// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <regex>
#include <yaml-cpp/yaml.h>

#include "HMState.h"
#include "HMConfigParserYaml.h"
#ifdef USE_MDBM
#include "HMStorageHostGroupMDBM.h"
#endif
#include "HMPubSubConfigParser.h"
#include "HMStorageHostText.h"
#include "HMPublisherBase.h"
#include "HMPublisherDefault.h"
#include "HMLogBase.h"

using namespace std;

HMSSLCtx::HMSSLCtx()
{
    m_ctx = SSL_CTX_new(TLSv1_2_method());
}

SSL_CTX*
HMSSLCtx::getCtx() const
{
    return m_ctx;
}

HMSSLCtx::~HMSSLCtx()
{
    SSL_CTX_free(m_ctx);
}

HMState::HMState(HMState& k) :
        m_ctx(NULL) ,
        m_configsLoaded(false),
        m_stateActive(false)
{
    m_configDirs = k.m_configDirs;
    m_configFiles = k.m_configFiles;
    m_useBackendConfigs = k.m_useBackendConfigs;
    m_masterConfigLoaded = k.m_masterConfigLoaded;
    m_storageClass = k.m_storageClass;
    m_storagePath = k.m_storagePath;
    m_eventClass = k.m_eventClass;
    m_httpDefaultCheckClass = k.m_httpDefaultCheckClass;
    m_ftpDefaultCheckClass = k.m_ftpDefaultCheckClass;
    m_tcpDefaultCheckClass = k.m_tcpDefaultCheckClass;
    m_tcpsDefaultCheckClass = k.m_tcpsDefaultCheckClass;
    m_dnsDefaultCheckClass = k.m_dnsDefaultCheckClass;
    m_noneDefaultCheckClass = k.m_noneDefaultCheckClass;
    m_auxDefaultCheckClass = k.m_auxDefaultCheckClass;
    m_markDefaultCheckClass = k.m_markDefaultCheckClass;
    m_dnsLookupTimeout = k.m_dnsLookupTimeout;
    m_dnsRetries = k.m_dnsRetries;
    m_nMaxThreads = k.m_nMaxThreads;
    m_nMinThreads = k.m_nMinThreads;
    m_connectionTimeout = k.m_connectionTimeout;
    m_logClass = k.m_logClass;
    m_logLevel = k.m_logLevel;
    m_logPath = k.m_logPath;
    m_storagePath = k.m_storagePath;
    m_dnsServer = k.m_dnsServer;
    m_socketPath = k.m_socketPath;
    m_logTimeFormat = k.m_logTimeFormat;
    m_maxLogQueue = k.m_maxLogQueue;
    m_auxPolicy = k.m_auxPolicy;
    m_healthCheckPolicy = k.m_healthCheckPolicy;
    m_storageLockPolicy = k.m_storageLockPolicy;
    m_controlSocketCheckPortv4 = k.m_controlSocketCheckPortv4;
    m_controlSocketCheckPortv6 = k.m_controlSocketCheckPortv6;
    m_hash = k.m_hash;
    m_masterConfig = k.m_masterConfig;
    m_healthCheckCAFile = k.m_healthCheckCAFile;
    m_healthCheckKey = k.m_healthCheckKey;
    m_healthCheckCert = k.m_healthCheckCert;
    m_control_socket = k.m_control_socket;
    m_enableSecureRemote = k.m_enableSecureRemote;
    m_certFile = k.m_certFile;
    m_keyFile = k.m_keyFile;
    m_caFile = k.m_caFile;
    m_PubSubConfigFile = k.m_PubSubConfigFile;
    m_hostGroups = k.m_hostGroups;
    m_enableMutualAuth = k.m_enableMutualAuth;
    m_enableSharedConnection = k.m_enableSharedConnection;
    m_maxConnections = k.m_maxConnections;
    m_libEventEnabled = k.m_libEventEnabled;
    m_flowType = k.m_flowType;
    m_dnsLookDefaultPlugin = k.m_dnsLookDefaultPlugin;
    m_dnsStaticDefaultPlugin = k.m_dnsStaticDefaultPlugin;
}

void
HMState::copyMasterParams(HMState& k)
{
    m_configDirs = k.m_configDirs;
    m_configFiles = k.m_configFiles;
    m_useBackendConfigs = k.m_useBackendConfigs;
    m_masterConfigLoaded = k.m_masterConfigLoaded;
    m_storageClass = k.m_storageClass;
    m_storagePath = k.m_storagePath;
    m_eventClass = k.m_eventClass;
    m_httpDefaultCheckClass = k.m_httpDefaultCheckClass;
    m_ftpDefaultCheckClass = k.m_ftpDefaultCheckClass;
    m_tcpDefaultCheckClass = k.m_tcpDefaultCheckClass;
    m_tcpsDefaultCheckClass = k.m_tcpsDefaultCheckClass;
    m_dnsDefaultCheckClass = k.m_dnsDefaultCheckClass;
    m_noneDefaultCheckClass = k.m_noneDefaultCheckClass;
    m_auxDefaultCheckClass = k.m_auxDefaultCheckClass;
    m_markDefaultCheckClass = k.m_markDefaultCheckClass;
    m_dnsLookupTimeout = k.m_dnsLookupTimeout;
    m_dnsRetries = k.m_dnsRetries;
    m_nMaxThreads = k.m_nMaxThreads;
    m_nMinThreads = k.m_nMinThreads;
    m_connectionTimeout = k.m_connectionTimeout;
    m_logClass = k.m_logClass;
    m_logLevel = k.m_logLevel;
    m_logPath = k.m_logPath;
    m_socketPath = k.m_socketPath;
    m_storagePath = k.m_storagePath;
    m_dnsServer = k.m_dnsServer;
    m_socketPath = k.m_socketPath;
    m_logTimeFormat = k.m_logTimeFormat;
    m_maxLogQueue = k.m_maxLogQueue;
    m_auxPolicy = k.m_auxPolicy;
    m_healthCheckPolicy = k.m_healthCheckPolicy;
    m_storageLockPolicy = k.m_storageLockPolicy;
    m_controlSocketCheckPortv4 = k.m_controlSocketCheckPortv4;
    m_controlSocketCheckPortv6 = k.m_controlSocketCheckPortv6;
    m_hash = k.m_hash;
    m_masterConfig = k.m_masterConfig;
    m_healthCheckCAFile = k.m_healthCheckCAFile;
    m_healthCheckKey = k.m_healthCheckKey;
    m_healthCheckCert = k.m_healthCheckCert;
    m_control_socket = k.m_control_socket;
    m_enableSecureRemote = k.m_enableSecureRemote;
    m_certFile = k.m_certFile;
    m_keyFile = k.m_keyFile;
    m_caFile = k.m_caFile;
    m_PubSubConfigFile = k.m_PubSubConfigFile;
    m_enableMutualAuth = k.m_enableMutualAuth;
    m_enableSharedConnection = k.m_enableSharedConnection;
    m_maxConnections = k.m_maxConnections;
    m_libEventEnabled = k.m_libEventEnabled;
    m_flowType = k.m_flowType;
    m_dnsLookDefaultPlugin = k.m_dnsLookDefaultPlugin;
    m_dnsStaticDefaultPlugin = k.m_dnsStaticDefaultPlugin;
}


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

void
HMState::generateCheckList()
{
    generateHostCheckList();
    generateDNSCheckList();
    generateRemoteHostCheckList();
    generateRemoteCheckList();
}

bool
HMState::hashStoreConfig()
{
    this->hashHostGroupMap();
    if(!this->hashConfigs())
    {
        HMLog(HM_LOG_CRITICAL, "[CORE] Failure Initializing Hashing function");
        return false;
    }
    this->m_datastore->storeConfigs(*this);
    this->storeConfigInfo();
    return true;
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
    generateCheckList();
    hashStoreConfig();

    //To avoid inserting checks in the middle of the run
    m_checkList.setGuard(true);
    return true;
}

bool
HMState::storeConfigInfo()
{
    HMConfigInfo configInfo;
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configLoadTime = HMTimeStamp::now();
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_hash = getHash();
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
HMState::getDefaultTCPSCheckype() const
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
    HMConfigParams configParams;
    if (!m_useBackendConfigs)
    {
        // parse directories
        HMLog(HM_LOG_INFO, "[CORE] Loading Configuration Files");
        for (auto it = m_configDirs.begin(); it != m_configDirs.end(); ++it)
        {
            HMLog(HM_LOG_INFO, "[CORE] Parse directory %s", it->c_str());
            if (HMConfigParserBase::parseDirectory(*it, *this, configParams)
                    > 0)
            {
                return false;
            }
        }

        // parse files
        for (auto it = m_configFiles.begin(); it != m_configFiles.end(); ++it)
        {
            if (HMConfigParserBase::parseConfigFile(*it, *this, configParams)
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
    if (!configParams.m_masterMode)
    {
        for (auto& it : m_hostGroups)
        {
            for( auto remote : configParams.m_remoteChecks)
            {
                if (it.first.find(remote.first)
                        != std::string::npos)
                {
                    HMLog(HM_LOG_DEBUG3,
                                        "Added remote check %s to %s", remote.second.c_str(), it.first.c_str());
                    it.second.setRemoteCheck(remote.second);
                    if(m_flowType == HM_FLOW_REMOTE_HOSTGROUP_TYPE)
                    {
                        it.second.setFlowType(HM_FLOW_REMOTE_HOSTGROUP_TYPE);
                    }
                    else if(m_flowType == HM_FLOW_REMOTE_HOST_TYPE)
                    {
                        it.second.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
                    }
                }
            }
        }
    }
    for(auto res = configParams.m_indirectHost.begin(); res!= configParams.m_indirectHost.end(); ++res)
    {
        auto indirectHost = m_hostGroups.find(res->first);
        auto mappedHost = m_hostGroups.find(res->second);
        if(mappedHost != m_hostGroups.end() && indirectHost != m_hostGroups.end())
        {
            indirectHost->second.setHostGroupParameters(mappedHost->second);
        }
        else
        {
            if(mappedHost == m_hostGroups.end())
            {
                HMLog(HM_LOG_WARNING,
                        "[CORE] Missing Rotation(%s) mentioned in indirect-host checktype for %s file %s",
                        res->second.c_str(), res->first.c_str());
                m_hostGroups.erase(indirectHost);
            }
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
            case HM_CHECK_MARK_HTTP:
                it->second.setPort(HM_HTTP_DEFAULT_PORT);
                break;
            case HM_CHECK_HTTPS:
            case HM_CHECK_HTTPS_NO_PEER_CHECK:
            case HM_CHECK_AUX_HTTPS:
            case HM_CHECK_AUX_HTTPS_NO_PEER_CHECK:
            case HM_CHECK_MTLS_HTTPS:
            case HM_CHECK_MTLS_HTTPS_NO_PEER_CHECK:
            case HM_CHECK_AUX_MTLS_HTTPS:
            case HM_CHECK_AUX_MTLS_HTTPS_NO_PEER_CHECK:
            case HM_CHECK_MARK_HTTPS:
            case HM_CHECK_MARK_HTTPS_NO_PEER_CHECK:
                it->second.setPort(HM_HTTPS_DEFAULT_PORT);
                break;
            case HM_CHECK_TCP:
            case HM_CHECK_TCPS:
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
            case HM_CHECK_INVALID:
                break;
            }
        }
        
         //add hosts of child host groups
        for(auto jt = it->second.getHostGroupList()->begin();jt!= it->second.getHostGroupList()->end(); ++jt)
        {
            auto childhostgroup = m_hostGroups.find(jt->c_str());
            if(childhostgroup != m_hostGroups.end())
            {
                if (childhostgroup->second.getHostGroupList()->size() > 0)
                {
                    HMLog(HM_LOG_ERROR,
                            "Error Child host group(%s) is a meta hostgroup. Cannot be added as sub hostgroup in %s",
                            jt->c_str(), it->first.c_str());
                    return false;
                }
                for (auto st = childhostgroup->second.getHostList()->begin(); st != childhostgroup->second.getHostList()->end(); ++st)
                {
                    string host = *st;
                    it->second.addHost(host);
                    HMLog(HM_LOG_DEBUG, "adding host %s of child host group %s in Parent host group %s",
                                    host.c_str(), jt->c_str(), it->first.c_str());
                }
            }
            else
            {
                HMLog(HM_LOG_ERROR,
                        "Error %s is not a valid child host group in Parent host group %s",
                        jt->c_str(), it->first.c_str());
                return false;
            }
        }        
    }

    m_configsLoaded = true;
    return true;
}

bool
HMState::loadPubSubConfig()
{
    m_resultPublisher = make_shared<HMResultPublisher>();
    if(!m_resultPublisher)
    {
        HMLog(HM_LOG_ERROR, "Failed to create result publisher");
        return false;
    }
    HMPubSubConfigParser parser;
    unique_ptr<HMPublisherBase> defaultPublisher = make_unique<HMPublisherDefault>();
    this->m_resultPublisher->registerPublisher("default", defaultPublisher);
    if(!m_PubSubConfigFile.empty())
    {
        return parser.parseConfig(m_PubSubConfigFile, *this) == 0;
    }
    return true;
}


void
HMState::generateHostCheckList()
{
    HM_REMOTE_CHECK_TYPE remoteCheckType = HM_REMOTE_CHECK_TCP;
    if (isEnableSharedConnection())
    {
        remoteCheckType = HM_REMOTE_SHARED_CHECK_TCP;
    }
    if(isEnableSecureRemote())
    {
        remoteCheckType = HM_REMOTE_CHECK_TCPS;
        if(isEnableSharedConnection())
        {
            remoteCheckType = HM_REMOTE_SHARED_CHECK_TCPS;
        }
    }
    for(HMDataHostGroupMap::iterator it = m_hostGroups.begin(); it != m_hostGroups.end(); ++it)
    {

        HMLog(HM_LOG_DEBUG, "[CORE] Adding hosts from hostGroup %s", it->first.c_str());

        // Add the plugin type information to the check type here
        if(it->second.getCheckType() == HM_CHECK_HTTP
                || it->second.getCheckType() == HM_CHECK_HTTPS
                || it->second.getCheckType() == HM_CHECK_HTTPS_NO_PEER_CHECK
                || it->second.getCheckType() == HM_CHECK_MTLS_HTTPS
                || it->second.getCheckType() == HM_CHECK_MTLS_HTTPS_NO_PEER_CHECK)
        {
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
                || it->second.getCheckType() == HM_CHECK_AUX_HTTPS_NO_PEER_CHECK
                || it->second.getCheckType() == HM_CHECK_AUX_MTLS_HTTPS
                || it->second.getCheckType() == HM_CHECK_AUX_MTLS_HTTPS_NO_PEER_CHECK)
        {
            it->second.setCheckPlugin(m_auxDefaultCheckClass);
        }
        else if(it->second.getCheckType() == HM_CHECK_MARK_HTTP
                || it->second.getCheckType() == HM_CHECK_MARK_HTTPS
                || it->second.getCheckType() == HM_CHECK_MARK_HTTPS_NO_PEER_CHECK)
        {
            it->second.setCheckPlugin(m_markDefaultCheckClass);
        }
        else if(it->second.getCheckType() == HM_CHECK_TCP)
        {
           it->second.setCheckPlugin(m_tcpDefaultCheckClass);
        }
        else if (it->second.getCheckType() == HM_CHECK_TCPS)
        {
            it->second.setCheckPlugin(m_tcpsDefaultCheckClass);
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
        else if (it->second.getCheckType() == HM_CHECK_DEFAULT)
        {
            it->second.setCheckPlugin(m_noneDefaultCheckClass);
        }
        if(!it->second.getRemoteCheck().empty())
        {
            it->second.setRemoteCheckType(remoteCheckType);
        }
        m_checkList.addHostGroup(it->second);
    }
}

void
HMState::generateDNSCheckList()
{
    m_dnsCache.init();
    m_checkList.initDNSCache(m_dnsCache, m_dnsWaitList, m_dnsLookDefaultPlugin, m_dnsStaticDefaultPlugin);
}

void
HMState::generateRemoteHostCheckList()
{
    m_remoteHostCache.init();
    m_checkList.initRemoteCache(m_remoteHostCache);
}

void
HMState::generateRemoteCheckList()
{
    for (auto it = m_hostGroups.begin(); it != m_hostGroups.end(); ++it)
    {
        if(it->second.getFlowType() == HM_FLOW_REMOTE_HOSTGROUP_TYPE)
        {
            m_remoteCache.insertRemoteEntry(it->first, it->second.getCheckTTL(), it->second.getCheckTimeout());
        }
    }
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
                HMDNSLookup dnsCheck(dataCheck.getDnsType(), hostGroupInfo->second.getRemoteCheck());
                if(m_dnsCache.getAddresses(*it, dataCheck.getDualStack(), dnsCheck, addresses))
                {
                    for(auto address = addresses.begin(); address != addresses.end(); ++address)
                    {
                        if(*address == HMIPAddress(AF_INET) || *address == HMIPAddress(AF_INET6))
                        {
                            if(*address == HMIPAddress(AF_INET))
                            {
                                HMDNSLookup dnsHostCheck(dataCheck.getDnsType(), false, hostGroupInfo->second.getRemoteCheck());
                                m_dnsCache.queueDNSQuery(*it, dnsHostCheck, workQueue, getStateVersion());
                            }
                            else if(*address == HMIPAddress(AF_INET6))
                            {
                                HMDNSLookup dnsHostCheck(dataCheck.getDnsType(), true, hostGroupInfo->second.getRemoteCheck());
                                m_dnsCache.queueDNSQuery(*it, dnsHostCheck, workQueue, getStateVersion());
                            }
                        }
                        else
                        {
                            m_checkList.queueCheck(*it, *address, dataCheck, workQueue, getStateVersion());
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
            HMDNSLookup dnsCheck(dataCheck.getDnsType(), hostGroupInfo->second.getRemoteCheck());
            if(m_dnsCache.getAddresses(hostName, dataCheck.getDualStack(), dnsCheck, addresses))
            {
                for(auto address = addresses.begin(); address != addresses.end(); ++address)
                {
                    if(*address == HMIPAddress(AF_INET) || *address == HMIPAddress(AF_INET6))
                    {
                        if(*address == HMIPAddress(AF_INET))
                        {
                            HMDNSLookup dnsHostCheck(dataCheck.getDnsType(), false, hostGroupInfo->second.getRemoteCheck());
                            m_dnsCache.queueDNSQuery(hostName, dnsHostCheck, workQueue, getStateVersion());
                        }
                        else if(*address == HMIPAddress(AF_INET6))
                        {
                            HMDNSLookup dnsHostCheck(dataCheck.getDnsType(), true, hostGroupInfo->second.getRemoteCheck());
                            m_dnsCache.queueDNSQuery(hostName, dnsHostCheck, workQueue, getStateVersion());
                        }
                    }
                    else
                    {
                        m_checkList.queueCheck(hostName, *address, dataCheck, workQueue, getStateVersion());
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
                    HMDNSLookup dnsHostCheck(dataCheck.getDnsType(), false, hostGroupInfo->second.getRemoteCheck());
                    dnsHostCheck.setPlugin(getDNSPlugin(dataCheck.getDnsType()));
                    m_dnsCache.queueDNSQuery(*it, dnsHostCheck, workQueue, getStateVersion());
                }
                if(dataCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
                {
                    HMDNSLookup dnsHostCheck(dataCheck.getDnsType(), true, hostGroupInfo->second.getRemoteCheck());
                    dnsHostCheck.setPlugin(getDNSPlugin(dataCheck.getDnsType()));
                    m_dnsCache.queueDNSQuery(*it, dnsHostCheck, workQueue, getStateVersion());
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
                HMDNSLookup dnsHostCheck(dataCheck.getDnsType(), false, hostGroupInfo->second.getRemoteCheck());
                dnsHostCheck.setPlugin(getDNSPlugin(dataCheck.getDnsType()));
                m_dnsCache.queueDNSQuery(hostName, dnsHostCheck, workQueue, getStateVersion());
            }
            if(dataCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
            {
                HMDNSLookup dnsHostCheck(dataCheck.getDnsType(), true, hostGroupInfo->second.getRemoteCheck());
                dnsHostCheck.setPlugin(getDNSPlugin(dataCheck.getDnsType()));
                m_dnsCache.queueDNSQuery(hostName, dnsHostCheck, workQueue, getStateVersion());
            }
        }
    }
}

void
HMState::forceDNSCheck(const string &hostName,  HM_DNS_TYPE dnsType, const set<HMIPAddress>& addresses, HMWorkQueue& workQueue)
{
    bool ipv4=false, ipv6=false;
    for(const HMIPAddress& address: addresses)
    {
        if(address.getType() == AF_INET)
        {
            ipv4 = true;
        }
        if (address.getType() == AF_INET6)
        {
            ipv6 = true;
        }

    }
    map<pair<string,HMDNSLookup>,HMDNSResult>::const_iterator iter;
    if (ipv4)
    {
        //TODO remote check is provided empty. Need to implement if remote feature is needed for static checks
        HMDNSLookup dnsHostCheck(dnsType, getDNSPlugin(dnsType), false);

        if(m_dnsCache.getDNSResult(hostName, dnsHostCheck, iter))
        {
            m_dnsCache.queueDNSQuery(hostName, dnsHostCheck, workQueue, getStateVersion());
        }
        else
        {
            HMLog(HM_LOG_ERROR,
                    "Force pushing a DNS check for invalid host %s or invalid DNS check type %s",
                    hostName.c_str(),
                    printDnsType(dnsHostCheck.getType()).c_str());
        }
    }
    if (ipv6)
    {
        //TODO remote check is provided empty. Need to implement if remote feature is needed for static checks
        HMDNSLookup dnsHostCheck(dnsType, getDNSPlugin(dnsType), true);
        if (m_dnsCache.getDNSResult(hostName, dnsHostCheck, iter))
        {
            m_dnsCache.queueDNSQuery(hostName, dnsHostCheck, workQueue, getStateVersion());
        }
        else
        {
            HMLog(HM_LOG_ERROR,
                    "Force pushing a DNS check for invalid host %s or invalid DNS check type %s",
                    hostName.c_str(),
                    printDnsType(dnsHostCheck.getType()).c_str());
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
#ifdef USE_MDBM
        m_datastore = make_unique<HMStorageHostGroupMDBM>(m_storagePath, &m_hostGroups, &m_dnsCache);
#else
        HMLog(HM_LOG_ERROR, "MDBM disabled during build. Please enable it for the storage to work");
#endif
        break;
    }
    case HM_STORAGE_TEXT:
    default:
        m_datastore = make_unique<HMStorageHostText>(m_storagePath, &m_hostGroups, &m_dnsCache);
        break;
    }
    m_datastore->updateAuxCommitPolicy(m_auxPolicy);
    m_datastore->updateHealthCheckCommitPolicy(m_healthCheckPolicy);
    m_datastore->updateLockPolicy(m_storageLockPolicy);
    return m_datastore->openStore(readOnly);
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
            if(it->m_hostCheck.getFlowType() == HM_FLOW_DNS_HEALTH_TYPE)
            {
                map<pair<string,HMDNSLookup>,HMDNSResult>::const_iterator dnsResultV4;
                map<pair<string,HMDNSLookup>,HMDNSResult>::const_iterator dnsResultV6;
                HMDNSLookup dnsHostCheckV6(it->m_hostCheck.getDnsType(), true, it->m_hostCheck.getRemoteCheck());
                HMDNSLookup dnsHostCheckV4(it->m_hostCheck.getDnsType(), false, it->m_hostCheck.getRemoteCheck());
                src->m_dnsCache.getAddresses(it->m_hostname, HM_DUALSTACK_IPV4_ONLY, dnsHostCheckV4, addresses);
                src->m_dnsCache.getAddresses(it->m_hostname, HM_DUALSTACK_IPV6_ONLY, dnsHostCheckV6, addresses);
                src->m_dnsCache.getDNSResult(it->m_hostname, dnsHostCheckV4, dnsResultV4);
                src->m_dnsCache.getDNSResult(it->m_hostname, dnsHostCheckV6, dnsResultV6);
                HMDNSLookup lookup(it->m_hostCheck.getDnsType(), it->m_hostCheck.getRemoteCheck());
                m_dnsCache.updateReloadDNSEntry(it->m_hostname, addresses, dnsResultV4->second, dnsResultV6->second, lookup);
            }
            else if(it->m_hostCheck.getFlowType() == HM_FLOW_REMOTE_HOST_TYPE)
            {
                src->m_checkList.getCheckResultsAddress(it->m_hostname, it->m_hostCheck, it->m_hostCheck.getDualStack(), addresses);
            }
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
                        result.m_queryState = HM_CHECK_INACTIVE;
                        bool found = m_checkList.updateCheck(*it, result);
                        //update the last checktime in remote host cache
                        if(it->m_hostCheck.getFlowType() == HM_FLOW_REMOTE_HOST_TYPE)
                        {
                            map<pair<string, HMDataHostCheck>, HMRemoteResult>::const_iterator result;
                            if (src->m_remoteHostCache.getRemoteResult(it->m_hostname, it->m_hostCheck, result))
                            {
                                m_remoteHostCache.updateResultTime(it->m_hostname, it->m_hostCheck,
                                        result->second.getResultTime());
                            }
                        }
                        vector<string> dHgs;
                        m_checkList.getHostGroups(*it, dHgs);
                        if(found)
                        {
                            HMLog(HM_LOG_DEBUG, "[CORE] Updating previous results for host %s", it->m_hostname.c_str());
                            // checkparams will not have the hostgroups as we do not do a deep copy.
                            for(string groupName : dHgs)
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
    // Store remote hostgroup check times
    //update the last checktime in remote host group cache
    for(auto it = m_hostGroups.begin(); it != m_hostGroups.end(); ++it)
    {
        if(it->second.getFlowType() == HM_FLOW_REMOTE_HOSTGROUP_TYPE)
        {
            vector<HMGroupCheckResult> results;
            if(src->m_datastore->getGroupCheckResults(it->first, results))
            {
                for(auto result = results.begin(); result != results.end();)
                {
                    if(!it->second.isValidHost(result->m_hostName))
                    {
                        results.erase(result);
                        continue;
                    }
                    ++result;
                }
                m_datastore->storeHostGroupCheckResult(it->first, results);
            }
            map<string,HMRemoteResult>::const_iterator result;
            if(src->m_remoteCache.getRemoteResult(it->first, result))
            {
                m_remoteCache.updateResultTime(it->first, result->second.getResultTime());
            }
        }
    }
    m_dnsCache.setStaticDns(src->m_dnsCache.getStaticDns());
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
                    HMDNSLookup dnsHostCheck(nDataCheck.getDnsType(), nDataCheck.getRemoteCheck());
                    if(m_dnsCache.getAddresses(*hostname, nDataCheck.getDualStack(), dnsHostCheck, addresses))
                    {
                        for(auto address = addresses.begin(); address != addresses.end(); ++address)
                        {
                            m_checkList.queueCheck(hostName, *address, nDataCheck, workQueue, getStateVersion());
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
                    HMDNSLookup dnsHostCheck(nDataCheck.getDnsType(), nDataCheck.getRemoteCheck());
                    if(m_dnsCache.getAddresses(*hostname, nDataCheck.getDualStack(), dnsHostCheck, addresses))
                    {
                        string hostName = *hostname;
                        for(auto address = addresses.begin(); address != addresses.end(); ++address)
                        {
                            m_checkList.queueCheck(hostName, *address, nDataCheck, workQueue, getStateVersion());
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
                    HMDNSLookup dnsHostCheck(nDataCheck.getDnsType(), nDataCheck.getRemoteCheck());
                    if(m_dnsCache.getAddresses(*hostname, nDataCheck.getDualStack(), dnsHostCheck, addresses))
                    {
                        string hostName = *hostname;
                        for(auto address = addresses.begin(); address != addresses.end(); ++address)
                        {
                            m_checkList.queueCheck(hostName, *address, nDataCheck, workQueue, getStateVersion());
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
                HMDNSLookup dnsHostCheck(nDataCheck.getDnsType(), nDataCheck.getRemoteCheck());
                if(m_dnsCache.getAddresses(*hostname, nDataCheck.getDualStack(), dnsHostCheck, addresses))
                {
                    for(auto address = addresses.begin(); address != addresses.end(); ++address)
                    {
                        m_checkList.queueCheck(hostName, *address, nDataCheck, workQueue, getStateVersion());
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
                HMDNSLookup dnsHostCheck(it->m_hostCheck.getDnsType(), false, it->m_hostCheck.getRemoteCheck());
                dnsHostCheck.setPlugin(getDNSPlugin(it->m_hostCheck.getDnsType()));
                map<pair<string, HMDNSLookup>, HMDNSResult>::const_iterator resultSrcDNS;
                map<pair<string, HMDNSLookup>, HMDNSResult>::const_iterator resultDestDNS;

                if(src->m_dnsCache.getDNSResult(it->m_hostname, dnsHostCheck, resultSrcDNS)
                        && m_dnsCache.getDNSResult(it->m_hostname, dnsHostCheck, resultDestDNS))
                {
                    if(resultSrcDNS->second.getDNSTTL() < resultDestDNS->second.getDNSTTL())
                    {
                       m_dnsCache.queueDNSQuery(it->m_hostname, dnsHostCheck, workQueue, getStateVersion());
                    }
                }
            }
            if(it->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
            {
                HMDNSLookup dnsHostCheck(it->m_hostCheck.getDnsType(), true, it->m_hostCheck.getRemoteCheck());
                dnsHostCheck.setPlugin(getDNSPlugin(it->m_hostCheck.getDnsType()));
                map<pair<string, HMDNSLookup>, HMDNSResult>::const_iterator resultSrcDNS;
                map<pair<string, HMDNSLookup>, HMDNSResult>::const_iterator resultDestDNS;

                if(src->m_dnsCache.getDNSResult(it->m_hostname, dnsHostCheck, resultSrcDNS)
                        && m_dnsCache.getDNSResult(it->m_hostname, dnsHostCheck, resultDestDNS))
                {
                    if(resultSrcDNS->second.getDNSTTL() < resultDestDNS->second.getDNSTTL())
                    {
                        m_dnsCache.queueDNSQuery(it->m_hostname, dnsHostCheck, workQueue, getStateVersion());
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
    m_datastore->initResultsFromBackend(m_checkList, m_dnsCache, m_auxCache, m_remoteCache, m_remoteHostCache);
}

const HMHash& HMState::getHash() const
{
    return m_hash;
}

void HMState::setHash(const HMHash& hash)
{
    m_hash = hash;
}

uint16_t
HMState::getControlSocketCheckPortv4() const
{
    return m_controlSocketCheckPortv4;
}

uint16_t
HMState::getControlSocketCheckPortv6() const
{
    return m_controlSocketCheckPortv6;
}

const std::vector<HM_CONTROL_SOCKET>&
HMState::getControlSocket() const
{
    return m_control_socket;
}

bool HMState::isEnableSharedConnection() const
{
    return m_enableSharedConnection;
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
            HMLog(HM_LOG_NOTICE, "[CORE] DNS type is no longer a master config parameter. It is configured for each hostgroup as a hostgroup paramater");
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
        else if (key == "dns.lookuptype")
        {
            if(val == "ares")
            {
                m_dnsLookDefaultPlugin = HM_DNS_PLUGIN_ARES;
                HMLog(HM_LOG_NOTICE, "[CORE] Using ARES plugin for DNS look up Types");
            }
            else if(val == "none")
            {
                m_dnsLookDefaultPlugin = HM_DNS_PLUGIN_NONE;
                HMLog(HM_LOG_NOTICE, "[CORE] Using None plugin for DNS look up Types");
            }
            else
            {
                HMLog(HM_LOG_ERROR, "[CORE] Invalid DNS plugin type for dns.lookuptype %s",
                        val.c_str());
                return false;
            }
        }
        else if (key == "dns.statictype")
        {
            if(val == "static")
            {
                m_dnsStaticDefaultPlugin = HM_DNS_PLUGIN_STATIC;
                HMLog(HM_LOG_NOTICE, "[CORE] Using ARES plugin for DNS look up Types");
            }
            else if(val == "none")
            {
                m_dnsStaticDefaultPlugin = HM_DNS_PLUGIN_NONE;
                HMLog(HM_LOG_NOTICE, "[CORE] Using None plugin for DNS look up Types");
            }
            else
            {
                HMLog(HM_LOG_ERROR, "[CORE] Invalid DNS plugin type for dns.statictype %s",
                        val.c_str());
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
        else if (key == "tcps.type")
        {
            if (val == "rawsocket")
            {
                m_tcpDefaultCheckClass = HM_CHECK_PLUGIN_TCPS_RAW;
                HMLog(HM_LOG_NOTICE,
                        "[CORE] Using raw socket for TCPS Check Type");
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
        else if (key == "hc-cert-file")
        {
            ifstream cert(val);
            if(cert.is_open())
            {
                ostringstream ss;
                ss << cert.rdbuf(); // reading data
                m_healthCheckCert = ss.str();
                cert.close();
            }
            else
            {
                HMLog(HM_LOG_CRITICAL, "[CORE] %s not a valid file %s", key.c_str(), val.c_str());
            }
        }
        else if (key == "hc-key-file")
        {
            ifstream in(val);
            if(in.is_open())
            {   
                ostringstream ss;
                ss << in.rdbuf(); // reading data
                m_healthCheckKey = ss.str();
                in.close();
            }
            else
            {
                HMLog(HM_LOG_CRITICAL, "[CORE] %s not a valid file %s", key.c_str(), val.c_str());
            }
        }
        else if (key == "hc-ca-file")
        {
            m_healthCheckCAFile = val;
        }
        else if (key == "master-check-portv4")
        {
            int port = atoi(val.c_str());
            if (port >= 0 && port <= 65535)
            {
                m_controlSocketCheckPortv4 = port;
            }
            else
            {
                HMLog(HM_LOG_ERROR,
                        "Invalid port number for master-check-portv4, using default port %d",
                        HM_CONTROL_SOCKET_DEFAULT_PORTV4);
            }

        }
        else if (key == "master-check-portv6")
        {
            int port = atoi(val.c_str());
            if (port >= 0 && port <= 65535)
            {
                m_controlSocketCheckPortv6 = port;
            }
            else
            {
                HMLog(HM_LOG_ERROR,
                        "Invalid port number for master-check-portv6, using default port %d",
                        HM_CONTROL_SOCKET_DEFAULT_PORTV6);
            }

        }
        else if (key == "control-server-linux")
        {
            if (val == "off")
            {
                m_control_socket.erase(
                        std::remove(m_control_socket.begin(),
                                m_control_socket.end(),
                                HM_CONTROL_SOCKET_LINUX),
                        m_control_socket.end());
            }
            else if (val != "on")
            {
                HMLog(HM_LOG_ERROR,
                        "Invalid value for control-server-ipv6, setting to default value");
            }
        }
        else if (key == "control-server-ipv4")
        {
            if (val == "on")
            {
                if (m_enableSecureRemote)
                {
                    m_control_socket.push_back(HM_CONTROL_SOCKET_TLS_V4);
                }
                else
                {
                    m_control_socket.push_back(HM_CONTROL_SOCKET_TCP_V4);
                }
            }
            else if (val != "off")
            {
                HMLog(HM_LOG_ERROR,
                        "Invalid value for control-server-ipv4, setting to default value");
            }
        }
        else if (key == "control-server-ipv6")
        {
            if (val == "on")
            {
                if (m_enableSecureRemote)
                {
                    m_control_socket.push_back(HM_CONTROL_SOCKET_TLS_V6);
                }
                else
                {
                    m_control_socket.push_back(HM_CONTROL_SOCKET_TCP_V6);
                }
            }
            else if (val != "off")
            {
                HMLog(HM_LOG_ERROR,
                        "Invalid value for control-server-ipv6, setting to default value");
            }
        }
        else if (key == "enable-secure-remote")
        {
            if (val == "off")
            {
                if (std::find(m_control_socket.begin(), m_control_socket.end(),
                        HM_CONTROL_SOCKET_TLS_V4) != m_control_socket.end())
                {
                    m_control_socket.erase(
                            std::remove(m_control_socket.begin(),
                                    m_control_socket.end(),
                                    HM_CONTROL_SOCKET_TLS_V4),
                            m_control_socket.end());
                    m_control_socket.push_back(HM_CONTROL_SOCKET_TCP_V4);
                }
                if (std::find(m_control_socket.begin(), m_control_socket.end(),
                        HM_CONTROL_SOCKET_TLS_V6) != m_control_socket.end())
                {
                    m_control_socket.erase(
                            std::remove(m_control_socket.begin(),
                                    m_control_socket.end(),
                                    HM_CONTROL_SOCKET_TLS_V6),
                            m_control_socket.end());
                    m_control_socket.push_back(HM_CONTROL_SOCKET_TCP_V6);
                }
                m_enableSecureRemote = false;
            }
            else if (val == "on")
            {
                if (std::find(m_control_socket.begin(), m_control_socket.end(),
                        HM_CONTROL_SOCKET_TCP_V4) != m_control_socket.end())
                {
                    m_control_socket.erase(
                            std::remove(m_control_socket.begin(),
                                    m_control_socket.end(),
                                    HM_CONTROL_SOCKET_TCP_V4),
                            m_control_socket.end());
                    m_control_socket.push_back(HM_CONTROL_SOCKET_TLS_V4);
                }
                if (std::find(m_control_socket.begin(), m_control_socket.end(),
                        HM_CONTROL_SOCKET_TCP_V6) != m_control_socket.end())
                {
                    m_control_socket.erase(
                            std::remove(m_control_socket.begin(),
                                    m_control_socket.end(),
                                    HM_CONTROL_SOCKET_TCP_V6),
                            m_control_socket.end());
                    m_control_socket.push_back(HM_CONTROL_SOCKET_TLS_V6);
                }
                m_enableSecureRemote = true;
            }
        }
        else if (key == "cert-file")
        {
            m_certFile = val;
        }
        else if (key == "key-file")
        {
            m_keyFile = val;
        }
        else if (key == "ca-file")
        {
            m_caFile = val;
        }
        else if (key == "enable-mutual-auth")
        {
            if (val == "on")
            {
                m_enableMutualAuth = true;
            }
            else if (val == "off")
            {
                m_enableMutualAuth = false;
            }
            else
            {
                HMLog(HM_LOG_ERROR,
                        "Invalid value for enable-mutual-auth, setting to default value");
            }
        }
        else if (key == "enable-shared-connection")
        {
            if (val == "on")
            {
                m_enableSharedConnection = true;
            }
            else if (val == "off")
            {
                m_enableSharedConnection = false;
            }
            else
            {
                HMLog(HM_LOG_ERROR,
                        "Invalid value for enable-shared-connection, setting to default value");
            }
        }
        else if (key == "max-remote-connections")
        {
            int conn = atoi(val.c_str());
            if (conn >= 0 && conn <= HM_MAX_REMOTE_CONNECTIONS)
            {
                m_maxConnections = conn;
            }
            else
            {
                m_maxConnections = HM_MAX_REMOTE_CONNECTIONS;
                HMLog(HM_LOG_ERROR,
                        "max-remote-connections exceeds max number of parallel connection, using max value %d",
                        HM_MAX_REMOTE_CONNECTIONS);
            }
        }
        else if (key == "pubsub.config-file")
        {
            m_PubSubConfigFile = val;
        }
        else if (key == "remote-fetch")
        {
            if(val == "hostgroup")
            {
                m_flowType = HM_FLOW_REMOTE_HOSTGROUP_TYPE;
            }
            else if(val == "host")
            {
                m_flowType = HM_FLOW_REMOTE_HOST_TYPE;
            }
            else
            {
                HMLog(HM_LOG_ERROR,
                        "Invalid value for remote-hostgroup-fetch, setting to default value");
            }
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
HMState::hashHostGroupMap()
{
    for (auto it = m_hostGroups.begin(); it != m_hostGroups.end(); ++it)
    {
        HMHashMD5 hostGroupMD5;
        if (hostGroupMD5.init())
        {
            HMHash hostGroupHash;
            it->second.getHash(hostGroupMD5);
            hostGroupMD5.final(hostGroupHash);
            it->second.setHashValue(hostGroupHash);
        }
    }
}


bool
HMState::hashConfigs()
{
    HMHashMD5 configHash;
    HMHash hashValue;
    if (configHash.init())
    {
        for (auto it = m_hostGroups.begin(); it != m_hostGroups.end(); ++it)
        {
            it->second.getHash(configHash);
        }
        configHash.final(hashValue);
        setHash(hashValue);
        return true;
    }
    return false;
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

const std::string& HMState::getHealthCheckCAFile() const
{
    return m_healthCheckCAFile;
}

const std::string& HMState::getHealthCheckKey() const
{
    return m_healthCheckKey;
}

const std::string& HMState::getHealthCheckCert() const
{
    return m_healthCheckCert;
}

const std::string& HMState::getCertFile() const
{
    return m_certFile;
}

const std::string& HMState::getKeyFile() const
{
    return m_keyFile;
}

const std::string& HMState::getCaFile() const
{
    return m_caFile;
}

bool HMState::isEnableSecureRemote() const
{
    return m_enableSecureRemote;
}

bool HMState::isEnableMutualAuth() const
{
    return m_enableMutualAuth;
}

uint8_t HMState::getMaxConnections() const
{
    return m_maxConnections;
}

bool HMState::isLibEventEnabled() const
{
    return m_libEventEnabled;
}

void HMState::setLibEventEnabled(bool libEventEnabled)
{
    m_libEventEnabled = libEventEnabled;
}

HM_DNS_PLUGIN_CLASS HMState::getDNSPlugin(HM_DNS_TYPE dnsType)
{
    switch (dnsType)
    {
    case HM_DNS_TYPE_LOOKUP:
        return m_dnsLookDefaultPlugin;
    case HM_DNS_TYPE_STATIC:
        return m_dnsStaticDefaultPlugin;
    }
    HMLog(HM_LOG_ERROR, "[CORE] Invalid DNS type  %s", printDnsType(dnsType));
    return HM_DNS_PLUGIN_NONE;
}

void HMState::setStateVersion(uint32_t version)
{
   m_stateVersion = version;
}

uint32_t HMState::getStateVersion()
{
   return m_stateVersion;
}
