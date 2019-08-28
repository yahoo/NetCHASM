// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <regex>
#include <yaml-cpp/yaml.h>

#include "HMState.h"
#include "HMConfigParserYaml.h"
#include "HMStorageHostGroupMDBM.h"
#include "HMStorageHostText.h"
#include "HMLogBase.h"


using namespace std;

// I am changing this function to assume the logging is setup before calling
bool
HMState::validateConfigs(const string& masterConfig)
{
    if(m_stateActive)
    {
        return false;
    }

    if(!parseMasterConfig(masterConfig))
    {
        HMLog(HM_LOG_CRITICAL, "Failure to parse master config");
        return false;
    }

    // Parse the new config into a check state data structure
    if(!loadAllConfigs())
    {
        HMLog(HM_LOG_CRITICAL, "Failure to parse configs");
        return false;
    }
    return true;
}

bool
HMState::parseMasterConfig(const string& masterConfig)
{
    if(m_stateActive)
    {
        HMLog(HM_LOG_ERROR, "[CORE] Attempted to load master config while configuration was active");
        return false;
    }

    HMLog(HM_LOG_NOTICE, "[CORE] Loading Master Configuration %s", masterConfig.c_str());

    //master.yaml
    if(regex_match (masterConfig, regex("(.*)(.yaml)")))
    {
        return parseMasterYaml(masterConfig);
    }
    else
    {
        HMLog(HM_LOG_ERROR, "[CORE] Invalid master config %s type", masterConfig.c_str());
        return false;
    }
}

bool
HMState::setupDaemonstate()
{
    if(!m_masterConfigLoaded)
    {
        HMLog(HM_LOG_CRITICAL, "[CORE] No Master Config Loaded");
        return false;
    }

    bool bConfigLoadOk = loadAllConfigs();

    if(!bConfigLoadOk)
    {
       HMLog(HM_LOG_CRITICAL, "[CORE] Failure loading configs");
       return false;
    }
    generateHostCheckList();
    generateDNSCheckList();
    return true;
}

bool
 HMState::storeConfigInfo()
 {
    HMConfigInfo configInfo;
    HMHashMD5 configHash;
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configLoadTime = HMTimeStamp::now();
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    if (!configHash.init())
    {
        HMLog(HM_LOG_CRITICAL, "[CORE] Failure Initializing Hashing function");
         return false;
    }
    HashHostGroupMap(configHash, configInfo.m_hash);
    setHash(configInfo.m_hash);
    return m_datastore->storeConfigInfo(configInfo);
}

const std::string&
HMState::getMasterConfig() const
{
    return m_masterConfig;
}

HM_STORAGE_CLASS
HMState::getStorageType() const
{
    return m_storageClass;
}

HM_LOG_PLUGIN_CLASS
HMState::getDefaultLogType() const
{
    return m_logClass;
}

HM_DNS_PLUGIN_CLASS
HMState::getDefaultDNSLookupType() const
{
    return m_dnsLookupClass;
}

HM_EVENT_PLUGIN_CLASS
HMState::getDefaultEventType() const
{
    return m_eventClass;
}

HM_CHECK_PLUGIN_CLASS
HMState::getDefaultHTTPCheckype() const
{
    return m_httpDefaultCheckClass;
}

HM_CHECK_PLUGIN_CLASS
HMState::getDefaultFTPCheckype() const
{
    return m_ftpDefaultCheckClass;
}

HM_CHECK_PLUGIN_CLASS
HMState::getDefaultTCPCheckype() const
{
    return m_tcpDefaultCheckClass;
}

HM_CHECK_PLUGIN_CLASS
HMState::getDefaultDNSCheckype() const
{
    return m_dnsDefaultCheckClass;
}

HM_CHECK_PLUGIN_CLASS
HMState::getDefaultNoneCheckype() const
{
    return m_noneDefaultCheckClass;
}

string
HMState::getLogPath() const
{
    return m_logPath;
}

HM_LOG_LEVEL
HMState::getLogLevel() const
{
    return m_logLevel;
}

string
HMState::getLogFormat() const
{
    return m_logTimeFormat;
}

uint32_t
HMState::getMaxLogQueueLength() const
{
    return m_maxLogQueue;
}

uint64_t
HMState::getConnectionTimeout() const
{
    return m_connectionTimeout;
}

uint64_t
HMState::getMaxThreads()
{
    return m_nMaxThreads;
}

uint64_t
HMState::getMinThreads()
{
    return m_nMinThreads;
}

uint32_t
HMState::getDNSLookupTimeout() const
{
    return m_dnsLookupTimeout;
}

uint32_t
HMState::getDNSRetries() const
{
    return m_dnsRetries;
}

string
HMState::getStoragePath() const
{
    return m_storagePath;
}

string
HMState::getSocketPath() const
{
  return m_socketPath;
}

bool
HMState::getDNSAddress(HMIPAddress& addr) const
{
    if(!m_dnsServer.isSet())
    {
        return false;
    }
    addr = m_dnsServer;
    return true;
}

void
HMState::setMasterConfig(const string& masterConfig)
{
    m_masterConfig = masterConfig;
}

void
HMState::setConnectionTimeout(uint64_t connectionTimeout)
{
    m_connectionTimeout = connectionTimeout;
}

void
HMState::setDNSServer(HMIPAddress server)
{
    m_dnsServer = server;
}

void
HMState::setSocketPath(const string& path)
{
    m_socketPath = path;
}

bool
HMState::loadAllConfigs()
{
    if (!m_useBackendConfigs)
    {
        // parse directories
        HMLog(HM_LOG_INFO, "[CORE] Loading Configuration Files");
        for (auto it = m_configDirs.begin(); it != m_configDirs.end(); ++it)
        {
            HMLog(HM_LOG_INFO, "[CORE] Parse directory %s", it->c_str());
            if (HMConfigParserBase::parseDirectory(*it, *this)
                    > 0)
            {
                return false;
            }
        }

        // parse files
        for (auto it = m_configFiles.begin(); it != m_configFiles.end(); ++it)
        {
            if (HMConfigParserBase::parseConfigFile(*it, *this)
                    > 0)
            {
                return false;
            }
        }
    }
    else
    {
        HMLog(HM_LOG_NOTICE, "Loaded config from backend");
        if (!m_datastore->getConfigs(*this))
        {
            HMLog(HM_LOG_CRITICAL,
                    "Failure to retrieve config info from backend");
            return false;
        }
    }

    // Set the default ports
    for(auto it =m_hostGroups.begin(); it != m_hostGroups.end(); ++it)
    {
        if(it->second.getCheckPort() == 0)
        {
            switch(it->second.getCheckType())
            {
            case HM_CHECK_NONE:
                break;
            case HM_CHECK_DEFAULT:
            case HM_CHECK_HTTP:
            case HM_CHECK_AUX_HTTP:
                it->second.setPort(HM_HTTP_DEFAULT_PORT);
                break;
            case HM_CHECK_HTTPS:
            case HM_CHECK_HTTPS_NO_PEER_CHECK:
            case HM_CHECK_AUX_HTTPS:
            case HM_CHECK_AUX_HTTPS_NO_PEER_CHECK:
                it->second.setPort(HM_HTTPS_DEFAULT_PORT);
                break;
            case HM_CHECK_TCP:
                it->second.setPort(HM_TCP_DEFAULT_PORT);
                break;
            case HM_CHECK_DNS:
            case HM_CHECK_DNSVC:
                it->second.setPort(HM_DNSVC_DEFAULT_PORT);
                break;
            case HM_CHECK_FTP:
                it->second.setPort(HM_FTP_DEFAULT_PORT);
                break;
            case HM_CHECK_FTPS_IMPLICIT:
            case HM_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK:
            case HM_CHECK_FTPS_EXPLICIT:
            case HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK:
                it->second.setPort(HM_FTPS_DEFAULT_PORT);
                break;
            }
        }
    }

    m_configsLoaded = true;
    return true;
}

void
HMState::generateHostCheckList()
{

    for(HMDataHostGroupMap::iterator it = m_hostGroups.begin(); it != m_hostGroups.end(); ++it)
    {

        HMLog(HM_LOG_DEBUG, "[CORE] Adding hosts from hostGroup %s", it->first.c_str());

        // Add the plugin type information to the check type here
        if(it->second.getCheckType() == HM_CHECK_HTTP
                || it->second.getCheckType() == HM_CHECK_HTTPS
                || it->second.getCheckType() == HM_CHECK_HTTPS_NO_PEER_CHECK)
        {
            //(it->second.
            it->second.setCheckPlugin(m_httpDefaultCheckClass);
        }
        else if(it->second.getCheckType() == HM_CHECK_FTP
                || it->second.getCheckType() == HM_CHECK_FTPS_IMPLICIT
                || it->second.getCheckType() == HM_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK
                || it->second.getCheckType() == HM_CHECK_FTPS_EXPLICIT
                || it->second.getCheckType() == HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK)
        {
           it->second.setCheckPlugin(m_ftpDefaultCheckClass);
        }
        else if(it->second.getCheckType() == HM_CHECK_AUX_HTTP
                || it->second.getCheckType() == HM_CHECK_AUX_HTTPS
                || it->second.getCheckType() == HM_CHECK_AUX_HTTPS_NO_PEER_CHECK)
        {
            it->second.setCheckPlugin(m_auxDefaultCheckClass);
        }
        else if(it->second.getCheckType() == HM_CHECK_TCP)
        {
           it->second.setCheckPlugin(m_tcpDefaultCheckClass);
        }
        else if(it->second.getCheckType() == HM_CHECK_DNSVC
                || it->second.getCheckType() == HM_CHECK_DNS)
        {
           it->second.setCheckPlugin(m_dnsDefaultCheckClass);
        }
        else if(it->second.getCheckType() == HM_CHECK_NONE)
        {
           it->second.setCheckPlugin(m_noneDefaultCheckClass);
        }
        m_checkList.addHostGroup(it->second);
    }
}


void
HMState::generateDNSCheckList()
{
    m_dnsCache.init(m_dnsLookupClass);
    m_checkList.initDNSCache(m_dnsCache, m_dnsWaitList);
}

// force check for all the hosts in the host group
void
HMState::forceHealthCheck(const string& hostGroup, HMWorkQueue& workQueue)
{
    HMDataHostGroupMap::iterator hostGroupInfo;
    if((hostGroupInfo = m_hostGroups.find(hostGroup)) != m_hostGroups.end())
    {
        HMDataHostCheck dataCheck;
        if(hostGroupInfo->second.getHostCheck(dataCheck))
        {
            for(auto it = hostGroupInfo->second.getHostList()->begin(); it != hostGroupInfo->second.getHostList()->end(); ++it)
            {
                set<HMIPAddress> addresses;
                if(m_dnsCache.getAddresses(*it, dataCheck.getDualStack(), addresses))
                {
                    for(auto address = addresses.begin(); address != addresses.end(); ++address)
                    {
                        if(*address == HMIPAddress(AF_INET) || *address == HMIPAddress(AF_INET6))
                        {
                            m_dnsCache.queueDNSQuery(*it, address->getType() == AF_INET6 ? true : false, workQueue);
                        }
                        else
                        {
                            m_checkList.queueCheck(*it, *address, dataCheck, workQueue);
                        }
                    }
                }
            }
        }
    }
}

//force check for a particular host in the host group
void
HMState::forceHealthCheck(const string& hostGroup, const string& hostName, HMWorkQueue& workQueue)
{
    HMDataHostGroupMap::iterator hostGroupInfo;
    if((hostGroupInfo = m_hostGroups.find(hostGroup)) != m_hostGroups.end())
    {
        HMDataHostCheck dataCheck;
        if(hostGroupInfo->second.getHostCheck(dataCheck))
        {
            set<HMIPAddress> addresses;
            if(m_dnsCache.getAddresses(hostName, dataCheck.getDualStack(), addresses))
            {
                for(auto address = addresses.begin(); address != addresses.end(); ++address)
                {
                    if(*address == HMIPAddress(AF_INET) || *address == HMIPAddress(AF_INET6))
                    {
                        m_dnsCache.queueDNSQuery(hostName, address->getType() == AF_INET6 ? true : false, workQueue);
                    }
                    else
                    {
                        m_checkList.queueCheck(hostName, *address, dataCheck, workQueue);
                    }
                }
            }
        }
    }
}

// force check for all the hosts in the host group
void
HMState::forceDNSCheck(const string &hostGroup, HMWorkQueue& workQueue)
{
    HMDataHostGroupMap::iterator hostGroupInfo;
    if((hostGroupInfo = m_hostGroups.find(hostGroup)) != m_hostGroups.end())
    {
        HMDataHostCheck dataCheck;
        if(hostGroupInfo->second.getHostCheck(dataCheck))
        {
            for(auto it = hostGroupInfo->second.getHostList()->begin(); it != hostGroupInfo->second.getHostList()->end(); ++it)
            {
                if(dataCheck.getDualStack() & HM_DUALSTACK_IPV4_ONLY)
                {
                    m_dnsCache.queueDNSQuery(*it, false, workQueue);
                }
                if(dataCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
                {
                    m_dnsCache.queueDNSQuery(*it, true, workQueue);
                }
            }
        }
    }
}

void
HMState::forceDNSCheck(const string &hostGroup, const string &hostName, HMWorkQueue& workQueue)
{
    HMDataHostGroupMap::iterator hostGroupInfo;
    if((hostGroupInfo = m_hostGroups.find(hostGroup)) != m_hostGroups.end())
    {
        HMDataHostCheck dataCheck;
        if(hostGroupInfo->second.getHostCheck(dataCheck))
        {
            if(dataCheck.getDualStack() & HM_DUALSTACK_IPV4_ONLY)
            {
                m_dnsCache.queueDNSQuery(hostName, false, workQueue);
            }
            if(dataCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
            {
                m_dnsCache.queueDNSQuery(hostName, true, workQueue);
            }
        }
    }
}

bool
HMState::openBackend(bool readOnly)
{
    switch(m_storageClass)
    {
    case HM_STORAGE_MDBM:
    {
        m_datastore = make_unique<HMStorageHostGroupMDBM>(m_storagePath, &m_hostGroups);
        break;
    }
    case HM_STORAGE_TEXT:
    default:
        m_datastore = make_unique<HMStorageHostText>(m_storagePath, &m_hostGroups);
        break;
    }
    m_datastore->updateAuxCommitPolicy(m_auxPolicy);
    m_datastore->updateHealthCheckCommitPolicy(m_healthCheckPolicy);
    m_datastore->updateLockPolicy(m_storageLockPolicy);
    return m_datastore->openStore(readOnly);
}

void
HMState::closeBackend()
{
    if(m_datastore != nullptr)
    {
        m_datastore->closeStore();
    }
}

// this function copies state from a current CheckState to a new CheckState
// We need to restore both the DNS and HealthCheck State
void
HMState::restoreRunningCheckState(shared_ptr<HMState> src)
{
    vector<HMCheckHeader> allChecks;
    vector<string> changedHost;
    if(src->m_checkList.getAllChecks(allChecks))
    {
        for(auto it = allChecks.begin(); it != allChecks.end(); ++it)
        {
            set<HMIPAddress> addresses;
            map<pair<string,bool>,HMDNSResult>::const_iterator dnsResultV4;
            map<pair<string,bool>,HMDNSResult>::const_iterator dnsResultV6;
            src->m_dnsCache.getAddresses(it->m_hostname, HM_DUALSTACK_IPV4_ONLY, addresses);
            src->m_dnsCache.getAddresses(it->m_hostname, HM_DUALSTACK_IPV6_ONLY, addresses);
            src->m_dnsCache.getDNSResult(it->m_hostname, false, dnsResultV4);
            src->m_dnsCache.getDNSResult(it->m_hostname, true, dnsResultV6);
            m_dnsCache.updateReloadDNSEntry(it->m_hostname, addresses, dnsResultV4->second, dnsResultV6->second);
            for(auto address = addresses.begin(); address != addresses.end(); ++address)
            {
                it->m_address = *address;
                HMDataCheckResult result(it->m_checkParams.getTimeout());
                if(src->m_checkList.getCheckResult(*it, result))
                {
                    HMDataCheckResult tmp_result(it->m_checkParams.getTimeout());
                    if(!m_checkList.getCheckResult(*it, tmp_result)
                            || tmp_result.m_checkTime < result.m_checkTime)
                    {
                        bool found = m_checkList.updateCheck(*it, result);
                        vector<string> sHgs;
                        vector<string> dHgs;
                        vector<string> common;
                        src->m_checkList.getHostGroups(*it, sHgs);
                        m_checkList.getHostGroups(*it, dHgs);
                        // We cannot store to all host groups in previous configs, which will leak removed hostgroupinfo to new storage which cannot be removed
                        // We cannot store to all host groups in current configs, which will leak results to new hostgroup which should not be the case
                        // We only copy the results to host which are common in both hosts groups. The new host healthchecks will be rescheduled by the rescheduleHealthCheck function
                        set_intersection(sHgs.begin(), sHgs.end(),
                                dHgs.begin(), dHgs.end(),
                                inserter(common, common.begin()));
                        if(found)
                        {
                            HMLog(HM_LOG_DEBUG, "[CORE] Updating previous results for host %s", it->m_hostname.c_str());
                            // checkparams will not have the hostgroups as we do not do a deep copy.
                            for(string groupName : common)
                            {
                                it->m_checkParams.addHostGroup(groupName);
                            }
                            m_datastore->updateCheckResultCache(*it, result);
                        }
                    }
                }
            }
        }
    }
}

void
HMState::resheduleHealthChecks(shared_ptr<HMState> src, HMWorkQueue& workQueue)
{
    // This function reschedules all healthchecks that have changes from the previous configs
    for(auto iter = m_hostGroups.begin(); iter != m_hostGroups.end(); ++iter)
    {
        // We first look at all the healthchecks that are in both previous and current configs
        auto it = src->m_hostGroups.find(iter->first);
        if(it != src->m_hostGroups.end())
        {
            HMDataHostCheck oDataCheck;
            HMDataHostCheck nDataCheck;
            HMDataCheckParams oParams;
            HMDataCheckParams nParams;
            it->second.getCheckParameters(oParams);
            iter->second.getCheckParameters(nParams);
            it->second.getHostCheck(oDataCheck);
            iter->second.getHostCheck(nDataCheck);
            // If the HMDataHostCheck is same but the HMDataCheckParams are different. Reschedule all the healthchecks
            if(oDataCheck == nDataCheck && oParams != nParams)
            {
                for(auto hostname = iter->second.getHostList()->begin(); hostname != iter->second.getHostList()->end(); ++hostname)
                {
                    set<HMIPAddress> addresses;
                    string hostName = *hostname;
                    map<pair<string, bool>, HMDNSResult>::const_iterator result;
                    if(m_dnsCache.getAddresses(*hostname, nDataCheck.getDualStack(), addresses))
                    {
                        for(auto address = addresses.begin(); address != addresses.end(); ++address)
                        {
                            m_checkList.queueCheck(hostName, *address, nDataCheck, workQueue);
                        }
                    }
                }
            }
            // If the HMDataHostCheck is same and the HMDataCheckParams are same, reschedule additional hosts
            else if(oDataCheck == nDataCheck && oParams == nParams)
            {
                vector<string> diff;
                set_difference(iter->second.getHostList()->begin(),
                        iter->second.getHostList()->end(),
                        it->second.getHostList()->begin(),
                        it->second.getHostList()->end(),
                        inserter(diff, diff.begin()));
                for(auto hostname = diff.begin(); hostname != diff.end(); ++hostname)
                {
                    set<HMIPAddress> addresses;
                    map<pair<string, bool>, HMDNSResult>::const_iterator result;
                    if(m_dnsCache.getAddresses(*hostname, nDataCheck.getDualStack(), addresses))
                    {
                        string hostName = *hostname;
                        for(auto address = addresses.begin(); address != addresses.end(); ++address)
                        {
                            m_checkList.queueCheck(hostName, *address, nDataCheck, workQueue);
                        }
                    }
                }
            }
            // If the HMDataHostCheck is different reschedule additional hosts(DNS would have already existed) so we need to reschedule the healthchecks)
            else if(oDataCheck != nDataCheck)
            {
                for(auto hostname = iter->second.getHostList()->begin();
                        hostname != iter->second.getHostList()->end();
                        ++hostname)
                {
                    string hostName = *hostname;
                    set<HMIPAddress> addresses;
                    map<pair<string, bool>, HMDNSResult>::const_iterator result;
                    if(m_dnsCache.getAddresses(*hostname, nDataCheck.getDualStack(), addresses))
                    {
                        string hostName = *hostname;
                        for(auto address = addresses.begin(); address != addresses.end(); ++address)
                        {
                            m_checkList.queueCheck(hostName, *address, nDataCheck, workQueue);
                        }
                    }
                }
            }
        }
        // It the hostgroup exists in the new config but missing in the old configs reschedule all the hosts.
        else
        {
            HMDataHostCheck nDataCheck;
            HMDataCheckParams nParams;
            iter->second.getCheckParameters(nParams);
            iter->second.getHostCheck(nDataCheck);

            for(auto hostname = iter->second.getHostList()->begin(); hostname != iter->second.getHostList()->end(); ++hostname)
            {
                set<HMIPAddress> addresses;
                string hostName = *hostname;
                map<pair<string, bool>, HMDNSResult>::const_iterator result;
                if(m_dnsCache.getAddresses(*hostname, nDataCheck.getDualStack(), addresses))
                {
                    for(auto address = addresses.begin(); address != addresses.end(); ++address)
                    {
                        m_checkList.queueCheck(hostName, *address, nDataCheck, workQueue);
                    }
                }
            }
        }
    }
}

void
HMState::resheduleDNSChecks(shared_ptr<HMState> src, HMWorkQueue& workQueue)
{
    // This function reschedules all dnschecks that have change in ttl value which may cause the dns check to go off schedule.
    vector<HMCheckHeader> allChecks;
    if(m_checkList.getAllChecks(allChecks))
    {
        for(auto it = allChecks.begin(); it != allChecks.end(); ++it)
        {
            if(it->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV4_ONLY)
            {
                map<pair<string, bool>, HMDNSResult>::const_iterator resultSrcDNS;
                map<pair<string, bool>, HMDNSResult>::const_iterator resultDestDNS;

                if(src->m_dnsCache.getDNSResult(it->m_hostname, false, resultSrcDNS)
                        && m_dnsCache.getDNSResult(it->m_hostname, false, resultDestDNS))
                {
                    if(resultSrcDNS->second.getDNSTTL() < resultDestDNS->second.getDNSTTL())
                    {
                       m_dnsCache.queueDNSQuery(it->m_hostname, false, workQueue);
                    }
                }
            }
            if(it->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
            {
                map<pair<string, bool>, HMDNSResult>::const_iterator resultSrcDNS;
                map<pair<string, bool>, HMDNSResult>::const_iterator resultDestDNS;

                if(src->m_dnsCache.getDNSResult(it->m_hostname, true, resultSrcDNS)
                        && m_dnsCache.getDNSResult(it->m_hostname, true, resultDestDNS))
                {
                    if(resultSrcDNS->second.getDNSTTL() < resultDestDNS->second.getDNSTTL())
                    {
                        m_dnsCache.queueDNSQuery(it->m_hostname, true, workQueue);
                    }
                }
            }
        }
    }
}

void
HMState::updateBackend(shared_ptr<HMState> src)
{
    set<string> mHosts;
    for(auto it = src->m_hostGroups.begin(); it != src->m_hostGroups.end(); ++it)
    {
        auto iter = m_hostGroups.find(it->first);
        // TODO I think the pass through comparison can be dropped now...we now track passthrough in the check params....
        if(iter == m_hostGroups.end() || (iter != m_hostGroups.end() && it->second.getPassthroughInfo() != iter->second.getPassthroughInfo() ))
        {
            mHosts.insert(it->first);
        }
        else
        {
            vector<string> diff;
            set_difference(it->second.getHostList()->begin(),
                    it->second.getHostList()->end(),
                    iter->second.getHostList()->begin(),
                    iter->second.getHostList()->end(),
                    inserter(diff, diff.begin()));
            set_difference(iter->second.getHostList()->begin(),
                    iter->second.getHostList()->end(),
                    it->second.getHostList()->begin(),
                    it->second.getHostList()->end(),
                    inserter(diff, diff.begin()));
            if(diff.size())
            {
                mHosts.insert(it->first);
            }
        }
    }
    m_datastore->updateHostGroups(mHosts);
}

void
HMState::restoreStoredCheckState()
{
    m_datastore->initResultsFromBackend(m_checkList, m_dnsCache, m_auxCache);
}

const HMHash& HMState::getHash() const
{
    return m_hash;
}

void HMState::setHash(const HMHash& hash)
{
    m_hash = hash;
}

bool
HMState::parseMasterYaml(const string& masterConfig)
{
    string line, key, val;
    vector<string> files, dirs;

    m_configDirs.clear();
    m_configFiles.clear();

    YAML::Node configNode;

    try
    {
        configNode = YAML::LoadFile(masterConfig);
    }
    catch(YAML::ParserException &e)
    {
        printf("Parsing master yaml %s is not valid a yaml\n",masterConfig.c_str());
        HMLog(HM_LOG_ERROR, "Parsing master yaml %s is not valid a yaml",masterConfig.c_str());
        return false;
    }
    catch (YAML::BadFile &e)
    {
        printf("No master yaml file %s exists\n",masterConfig.c_str());
        HMLog(HM_LOG_ERROR, "No master yaml file %s exists",masterConfig.c_str());
        return false;
    }

    if(!configNode.IsMap())
    {
        HMLog(HM_LOG_ERROR, "%s(%d): Error Yaml is not a Map",masterConfig.c_str(), configNode.Mark().line);
    }
    for(auto n : configNode)
    {
        key = n.first.Scalar();
        val = n.second.Scalar();
        if(key == "threads.max")
        {
            m_nMaxThreads = atoi(val.c_str());
            HMLog(HM_LOG_DEBUG, "[CORE] Maximum worker threads -> %d ", m_nMaxThreads);
        }
        else if(key == "threads.min")
        {
            m_nMinThreads = atoi(val.c_str());
            HMLog(HM_LOG_DEBUG, "[CORE] Minimum worker threads -> %d ", m_nMinThreads);
        }
        else if(key == "connectiontimeout")
        {
            m_connectionTimeout = atol(val.c_str());
            HMLog(HM_LOG_NOTICE, "[CORE] Connection Timeout %d ", m_connectionTimeout);
        }
        else if(key == "config.load-directory")
        {
            dirs.push_back(val);
        }
        else if(key == "config.load-file")
        {
            files.push_back(val);
        }
        else if (key == "config.use-backend")
        {
            if (val == "on")
            {
                m_useBackendConfigs = true;
            }
            else
            {
                m_useBackendConfigs = false;
            }
        }
        else if(key == "dns.type")
        {
            if(val == "ares")
            {
                m_dnsLookupClass = HM_DNS_PLUGIN_ARES;
                HMLog(HM_LOG_NOTICE, "[CORE] Using Ares DNS Library");
            }
            else
            {
                m_dnsLookupClass = HM_DNS_PLUGIN_NONE;
            }
        }
        else if(key == "dns.lookup-timeout")
        {
            m_dnsLookupTimeout = atoi(val.c_str());
            HMLog(HM_LOG_NOTICE, "[CORE] DNS Lookup Timeout set to %d ms", m_dnsLookupTimeout);
        }
        else if(key == "dns.host")
        {
            if(m_dnsServer.set(val))
            {
                HMLog(HM_LOG_NOTICE, "[CORE] DNS Host set to %s", m_dnsServer.toString().c_str());
            }
            else
            {
                HMLog(HM_LOG_ERROR, "[CORE] Invalid DNS Server %s", val.c_str());
                return false;
            }
        }
        else if(key == "http.type")
        {
            if(val == "curl")
            {
                m_httpDefaultCheckClass = HM_CHECK_PLUGIN_HTTP_CURL;
                HMLog(HM_LOG_NOTICE, "[CORE] Using Curl Library for HTTP/S Check Types");
            }
        }
        else if(key == "ftp.type")
        {
            if(val == "curl")
            {
                m_ftpDefaultCheckClass = HM_CHECK_PLUGIN_FTP_CURL;
                HMLog(HM_LOG_NOTICE, "[CORE] Using Curl Library for FTP/S Check Types");
            }
        }
        else if(key == "tcp.type")
        {
            if(val == "rawsocket")
            {
                m_tcpDefaultCheckClass = HM_CHECK_PLUGIN_TCP_RAW;
                HMLog(HM_LOG_NOTICE, "[CORE] Using raw socket for TCP Check Type");
            }
        }
        else if(key == "dnscheck.type")
        {
            if(val == "ares")
            {
                m_dnsDefaultCheckClass = HM_CHECK_PLUGIN_DNS_ARES;
                HMLog(HM_LOG_NOTICE, "[CORE] Using Ares Library for DNSVC Check Type");
            }
        }
        else if(key == "none.type")
        {
            if(val == "none")
            {
                m_noneDefaultCheckClass = HM_CHECK_PLUGIN_DEFAULT;
                HMLog(HM_LOG_NOTICE, "[CORE] Using none for NONE Check Type");
            }
        }
        else if (key == "db.lockPolicy")
        {
            if(val == "globallocks")
            {
                m_storageLockPolicy = HM_STORAGE_GLOBAL_LOCKS;
            }
            else if(val == "rwlocks")
            {
                m_storageLockPolicy = HM_STORAGE_RW_LOCKS;
            }
            else if(val == "partitionlocks")
            {
                m_storageLockPolicy = HM_STORAGE_PARTITION_LOCKS;
            }
        }
        else if(key == "db.type")
        {
            if(val == "mdbm")
            {
                m_storageClass = HM_STORAGE_MDBM;
                HMLog(HM_LOG_NOTICE, "[CORE] Using MDBM for the backend database");
            }
            else if(val == "text")
            {
                m_storageClass = HM_STORAGE_TEXT;
                HMLog(HM_LOG_NOTICE, "[CORE] Using a raw text file for the backend database");
            }
        }
        else if(key == "db.path")
        {
            m_storagePath = val;
        }

        else if(key == "db.auxCommitPolicy")
        {
            if(val == "always")
            {
                m_auxPolicy = HM_STORAGE_COMMIT_ALWAYS;
            }
            else if(val == "first")
            {
                m_auxPolicy = HM_STORAGE_COMMIT_ON_FIRST;
            }
            else if(val == "fullcycle")
            {
                m_auxPolicy = HM_STORAGE_COMMIT_ON_ALL_READY;
            }
            else if(val == "ttl")
            {
                m_auxPolicy = HM_STORAGE_COMMIT_ON_TTL;
            }
        }
        else if(key == "db.healthCheckCommitPolicy")
        {
            if(val == "always")
            {
                m_healthCheckPolicy = HM_STORAGE_COMMIT_ALWAYS;
            }
            else if(val == "first")
            {
                m_healthCheckPolicy = HM_STORAGE_COMMIT_ON_FIRST;
            }
            else if(val == "fullcycle")
            {
                m_healthCheckPolicy = HM_STORAGE_COMMIT_ON_ALL_READY;
            }
            else if(val == "ttl")
            {
                m_healthCheckPolicy = HM_STORAGE_COMMIT_ON_TTL;
            }
        }
        else if(key == "log.path")
        {
            m_logPath = val;
        }
        else if(key == "log.type")
        {
            if(val == "text")
            {
                m_logClass = HM_LOG_PLUGIN_TEXT;
            }
            else if(val == "stdout")
            {
                m_logClass = HM_LOG_PLUGIN_STDOUT;
            }
            else if(val == "syslog")
            {
                m_logClass = HM_LOG_PLUGIN_SYSLOG;
            }
        }
        else if(key == "log.verbosity")
        {
            HM_LOG_LEVEL level = HMLogBase::parseLogLevel(val);
            m_logLevel = (level == HM_ERROR)?m_logLevel:level;
        }
        else if(key == "log.timeFormat")
        {
            m_logTimeFormat = val;
        }
        else if(key == "log.maxQueue")
        {
            m_maxLogQueue = atoi(val.c_str());
        }
        else if(key == "socket.path")
        {
            m_socketPath = val;
        }
        else
        {
            HMLog(HM_LOG_ERROR, "[CORE] Unknown Parameter %s", key.c_str());
        }
    }

    //adding directories
    for( auto dir: dirs)
    {
        m_configDirs.push_back(dir);
    }
    //adding files
    for( auto file: files)
    {
        m_configFiles.push_back(file);
    }
    m_masterConfigLoaded = true;
    return true;
}

void 
HMState::HashHostGroupMap(HMHashMD5& hash, HMHash& hashValue)
{
      for(auto it = m_hostGroups.begin();it != m_hostGroups.end();++it)
      {
          it->second.getHash(hash);
      }
      hash.final(hashValue);
}


bool
HMState::writeConfigs(HM_CONFIG_PLUGIN_CLASS configs, const string &fileName)
{
    unique_ptr<HMConfigParserBase> parser;
    switch(configs)
    {
    case HM_CONFIG_YAML:
    case HM_CONFIG_DEFAULT:
        parser = make_unique<HMConfigParserYAML>();
        break;
    }
    return parser->writeConfigs(*this, fileName);
}
