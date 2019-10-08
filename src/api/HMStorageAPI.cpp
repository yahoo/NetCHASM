// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>

#include "HMStorageAPI.h"
#include "HMTimeStamp.h"
#include "HMLogBase.h"
#include "HMState.h"
#include "HMLogBase.h"
#include "HMLogAPI.h"
#include "HMConstants.h"
#include "HMStorage.h"
#include "HMStateManager.h"

using namespace std;

HMStorageAPI::HMStorageAPI() :
    m_loaded(false)
{
    log = new HMLogAPI();
    log->initLogging(HM_LOG_ERROR, false);
    log->clearError();
    log->setAsDefaultLogger();
}

HMStorageAPI::~HMStorageAPI()
{
    m_currentState.reset();
    log->shutDownLogging();
    log->unsetAsDefaultLogger();
    delete log;
}

bool
HMStorageAPI::init(const string& masterConfig, bool localConfigs)
{
    // Parse the new config into a check state data structure
    log->clearError();
    m_currentState = make_shared<HMState>();

    if(!m_currentState->parseMasterConfig(masterConfig))
    {
        m_currentState.reset();
        return false;
    }
    m_currentState->setMasterConfig(masterConfig);

    // deal with the backend setup
    if(!m_currentState->openBackend(true))
    {
        HMLog(HM_LOG_CRITICAL, "Failure Opening the backend data store");
        m_currentState.reset();
        return false;
    }
    if(localConfigs)
    {
        // Parse the new config into a check state data structure
        if (!m_currentState->loadAllConfigs())
        {
            HMLog(HM_LOG_CRITICAL, "Failure to parse configs");
            m_currentState.reset();
            return false;
        }
        m_currentState->generateHostCheckList();
        m_currentState->generateDNSCheckList();
        HMHashMD5 configHash;
        HMConfigInfo configInfo;
        configInfo.m_version = HM_MDBM_VERSION;
        configInfo.m_configLoadTime = HMTimeStamp::now();
        configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
        if (configHash.init())
        {
            m_currentState->HashHostGroupMap(configHash, configInfo.m_hash);
        }
        m_currentState->setHash(configInfo.m_hash);
    }
    else
    {
        if(!m_currentState->m_datastore->getConfigs(*m_currentState))
        {
            HMLog(HM_LOG_CRITICAL, "Failure to retrieve config info from backend");
            m_currentState.reset();
            return false;
        }
    }
    m_loaded = true;
    return true;
}

bool
HMStorageAPI::updateState(shared_ptr<HMState>& current)
{
    if(current == m_currentState)
    {
        return false;
    }
    current = m_currentState;
    return true;
}

bool
HMStorageAPI::getConfigInfo(HMAPIConfigInfo& result)
{
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    HMConfigInfo configInfo; 
    if(!currentState->m_datastore->getConfigInfo(configInfo))
    {
        return false;
    }

    result.m_version = configInfo.m_version;
    result.m_configError = (configInfo.m_configStatus == HM_CONFIG_STATUS_ERROR);
    result.m_timestamp = configInfo.m_configLoadTime.getTimeSinceEpoch();
    std::memcpy(result.m_hash, configInfo.m_hash.m_hashValue, HASH_MAX_SIZE);
    return true;
}

bool
HMStorageAPI::validateHash()
{
    if (!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    HMConfigInfo configInfo;
    if (!currentState->m_datastore->getConfigInfo(configInfo))
    {
        return false;
    }
    HMHash hash = currentState->getHash();
    if (configInfo.m_hash != hash)
    {
        return false;
    }
    return true;
}

bool
HMStorageAPI::updateConfigs()
{
    log->clearError();
    if (!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    if(!validateHash())
    {
        lock_guard<mutex> lg(m_reloadConfigsMutex);
        if(validateHash())
        {
            return true;
        }
        shared_ptr<HMState> newState = make_shared<HMState>();

        if (!newState->parseMasterConfig(m_currentState->getMasterConfig()))
        {
            newState.reset();
            return false;
        }
        newState->setMasterConfig(m_currentState->getMasterConfig());

        // deal with the backend setup
        if (!newState->openBackend(true))
        {
            HMLog(HM_LOG_CRITICAL, "Failure Opening the backend data store");
            newState.reset();
            return false;
        }
        if (!newState->m_datastore->getConfigs(*newState))
        {
            HMLog(HM_LOG_CRITICAL,
                    "Failure to retrieve config info from backend");
            newState.reset();
            return false;
        }
        newState.swap(m_currentState);
    }
    return true;
}


bool
HMStorageAPI::validateConfigs(const string& masterConfig, bool verbose)
{
    log->clearError();
    if(verbose)
    {
        log->setLevel(HM_LOG_DEBUG3);
    }

    auto tempState = make_unique<HMState>();


    if(!tempState->validateConfigs(masterConfig))
    {
        log->setLevel(HM_LOG_ERROR);
        return false;
    }
    log->setLevel(HM_LOG_ERROR);
    return true;
}

bool
HMStorageAPI::getAllHosts(std::set<std::string>& hosts)
{
    hosts.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    std::vector<HMCheckHeader> allChecks;
    if(!currentState->m_checkList.getAllChecks(allChecks))
    {
        return false;
    }

    for(auto it = allChecks.begin(); it != allChecks.end(); ++it)
    {
        hosts.insert(it->m_hostname);
    }
    return true;
}

bool
HMStorageAPI::getHostChecks(const std::string& host, std::vector<HMAPICheckInfo>& checks)
{
    checks.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    std::vector<HMCheckHeader> allChecks;
    if(!currentState->m_checkList.getAllChecks(allChecks))
    {
        return false;
    }

    for(auto it = allChecks.begin(); it != allChecks.end(); ++it)
    {
        if(host == it->m_hostname)
        {
            HMAPICheckInfo info;
            info.m_measurementOptions = 0;
            info.m_ipv4 = (HM_DUALSTACK_IPV4_ONLY & it->m_hostCheck.getDualStack());
            info.m_ipv6 = (HM_DUALSTACK_IPV6_ONLY & it->m_hostCheck.getDualStack());
            info.m_checkType = (HM_API_CHECK_TYPE)it->m_hostCheck.getCheckType();
            info.m_port = it->m_hostCheck.getPort();
            info.m_checkInfo = it->m_hostCheck.getCheckInfo();
            info.m_numCheckRetries = it->m_checkParams.getNumCheckRetries();
            info.m_checkRetryDelay = it->m_checkParams.getCheckRetryDelay();
            info.m_smoothingWindow = it->m_checkParams.getSmoothingWindow();
            info.m_groupThreshold = it->m_checkParams.getGroupThreshold();
            info.m_slowThreshold = it->m_checkParams.getSlowThreshold();
            info.m_maxFlaps = it->m_checkParams.getMaxFlaps();
            info.m_checkTimeout = it->m_checkParams.getTimeout();
            info.m_checkTTL = it->m_checkParams.getTTL();
            info.m_flapThreshold = it->m_checkParams.getFlapThreshold();
            info.m_passthroughInfo = it->m_checkParams.getPassthroughInfo();

            checks.push_back(info);
        }
    }
    return true;
}

bool
HMStorageAPI::getAllHostGroupNames(set<string>& groupNames)
{
    groupNames.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    for(auto it = currentState->m_hostGroups.begin(); it != currentState->m_hostGroups.end(); ++it)
    {
        groupNames.insert(it->first);
    }
    return true;
}

bool
HMStorageAPI::getHostGroupInfo(const string& group, HMAPICheckInfo& info, vector<string>& hosts)
{
    hosts.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    auto hgi = currentState->m_hostGroups.find(group);

    if(hgi == currentState->m_hostGroups.end())
    {
        HMLog(HM_LOG_CRITICAL, "Group %s not found in the backend", group.c_str());
        return false;
    }

    info.m_measurementOptions = hgi->second.getMeasurementOptions();
    info.m_ipv4 = (HM_DUALSTACK_IPV4_ONLY & hgi->second.getDualstack());
    info.m_ipv6 = (HM_DUALSTACK_IPV6_ONLY & hgi->second.getDualstack());
    info.m_checkType = (HM_API_CHECK_TYPE)hgi->second.getCheckType();
    info.m_port = hgi->second.getCheckPort();
    info.m_checkInfo = hgi->second.getCheckInfo();
    info.m_numCheckRetries = hgi->second.getNumCheckRetries();
    info.m_checkRetryDelay = hgi->second.getCheckRetryDelay();
    info.m_smoothingWindow = hgi->second.getSmoothingWindow();
    info.m_groupThreshold = hgi->second.getGroupThreshold();
    info.m_slowThreshold = hgi->second.getSlowThreshold();
    info.m_maxFlaps = hgi->second.getMaxFlaps();
    info.m_checkTimeout = hgi->second.getCheckTimeout();
    info.m_checkTTL = hgi->second.getCheckTTL();
    info.m_flapThreshold = hgi->second.getFlapThreshold();
    info.m_passthroughInfo = hgi->second.getPassthroughInfo();

    for(auto it = hgi->second.getHostList()->begin(); it != hgi->second.getHostList()->end(); ++it)
    {
        hosts.push_back(*it);
    }

    for(auto it = hgi->second.getHostGroupList()->begin(); it != hgi->second.getHostGroupList()->end(); ++it)
    {
        info.m_hostGroups.push_back(*it);
    }
    return true;
}


bool
HMStorageAPI::getHostResults(const std::string& name, std::vector<HMAPICheckResult>& results, bool onlyResolved)
{
    bool foundName = false;
    HMDataCheckResult result;

    results.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    set<HMIPAddress> ips;
    if(!currentState->m_datastore->getDNS(name, ips))
    {
        return false;
    }

    std::vector<HMCheckHeader> allChecks;
    if(!currentState->m_checkList.getAllChecks(allChecks))
    {
        return false;
    }

    for(auto check = allChecks.begin(); check != allChecks.end(); ++check)
    {
        if(check->m_hostname == name)
        {
            foundName = true;
            bool foundV4 = false;
            bool foundV6 = false;

            for(auto ip = ips.begin(); ip != ips.end(); ++ip)
            {
                if((check->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV4_ONLY)
                        && ip->getType() == AF_INET)
                {
                    if(currentState->m_datastore->getCheckResult(name, *ip, check->m_hostCheck, check->m_checkParams, result))
                    {
                        foundV4 = true;
                        HMAPICheckResult entry(result);
                        entry.m_host = name;
                        results.push_back(entry);
                    }
                }
                else if((check->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
                        && ip->getType() == AF_INET6)
                {
                    if(currentState->m_datastore->getCheckResult(name, *ip, check->m_hostCheck, check->m_checkParams, result))
                    {
                        foundV6 = true;
                        HMAPICheckResult entry(result);
                        entry.m_host = name;
                        results.push_back(entry);
                    }
                }
            }

            // Add empty entries here is needed
            if((check->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV4_ONLY)
                    && !foundV4
                    && !onlyResolved)
            {
                HMAPICheckResult ipv4;
                ipv4.m_address.m_type = AF_INET;
                ipv4.m_host = name;
                results.push_back(ipv4);
            }

            if((check->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
                    && !foundV6
                    && !onlyResolved)
            {
                HMAPICheckResult ipv6;
                ipv6.m_address.m_type = AF_INET6;
                ipv6.m_host = name;
                results.push_back(ipv6);
            }

        }
    }

    if(!foundName)
    {
        HMLog(HM_LOG_CRITICAL, "Host %s not found in the configs", name.c_str());
    }
    return foundName;
}

bool
HMStorageAPI::getHostAuxInfo(const std::string& name, std::vector<HMAPIAuxInfo>& auxInfo)
{
    bool foundName = false;

    auxInfo.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    set<HMIPAddress> ips;
    if(!currentState->m_datastore->getDNS(name, ips))
    {
        return false;
    }

    std::vector<HMCheckHeader> allChecks;
    if(!currentState->m_checkList.getAllChecks(allChecks))
    {
        return false;
    }

    for(auto check = allChecks.begin(); check != allChecks.end(); ++check)
    {
        if(check->m_hostname == name
                && ((check->m_hostCheck.getCheckType() == HM_CHECK_AUX_HTTP)
                    || (check->m_hostCheck.getCheckType() == HM_CHECK_AUX_HTTPS)
                    || (check->m_hostCheck.getCheckType() == HM_CHECK_AUX_HTTPS_NO_PEER_CHECK)))
        {
            foundName = true;

            for(auto ip = ips.begin(); ip != ips.end(); ++ip)
            {
                if(((check->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV4_ONLY) && ip->getType() == AF_INET)
                        || ((check->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY) && ip->getType() == AF_INET6))
                {
                    HMAuxInfo info;
                    if(currentState->m_datastore->getAuxInfo(name, *ip, check->m_hostCheck, check->m_checkParams, info))
                    {
                        HMAPIAuxInfo tempInfo;

                        tempInfo.m_ttl = check->m_checkParams.getTTL();
                        tempInfo.m_updatetime = info.m_ts.getTimeSinceEpoch();
                        tempInfo.m_address.m_type = ip->getType();
                        if(ip->getType() == AF_INET)
                        {
                            tempInfo.m_address.m_ip.addr = ip->addr4();
                        }
                        else if(ip->getType() == AF_INET6)
                        {
                            tempInfo.m_address.m_ip.addr6 = ip->addr6();
                        }
                        tempInfo.m_host = name;

                        for(auto it = info.m_auxData.begin(); it != info.m_auxData.end(); ++it)
                        {
                            HMAuxLoadFB* lfb = (*it)->getLFB();
                            if(lfb)
                            {
                                HMAPILFB entry;
                                entry.m_type = lfb->m_type;
                                entry.m_ts = lfb->m_ts.getTimeSinceEpoch();
                                entry.m_load = lfb->m_load;
                                entry.m_target = lfb->m_target;
                                entry.m_max = lfb->m_max;
                                entry.m_host = lfb->m_host;
                                entry.m_resource = lfb->m_resource;
                                entry.m_datacenter = lfb->m_datacenter;
                                tempInfo.m_lfb.push_back(entry);
                                continue;
                            }
                            HMAuxOOB* oob = (*it)->getOOB();
                            if(oob)
                            {
                                HMAPIOOB entry;
                                entry.m_type = oob->m_type;
                                entry.m_shed = oob->m_shed;
                                entry.m_ts = oob->m_ts.getTimeSinceEpoch();
                                entry.m_forceDown = oob->m_forceDown;
                                entry.m_host = oob->m_host;
                                entry.m_resource = oob->m_resource;
                                tempInfo.m_oob.push_back(entry);
                            }
                        }

                        auxInfo.push_back(move(tempInfo));
                    }
                }
            }
        }
    }

    if(!foundName)
    {
        HMLog(HM_LOG_CRITICAL, "Host %s not found in the configs", name.c_str());
    }
    return foundName;
}

bool
HMStorageAPI::getHostGroupResults(const std::string& name, std::vector<HMAPICheckResult>& results, bool onlyResolved)
{
    set<string> hostsPresent;
    set<string> actualHosts;
    results.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    auto hostGroupIter = currentState->m_hostGroups.find(name);
    if (hostGroupIter == currentState->m_hostGroups.end())
    {
        return false;
    }

    std::vector<HMGroupCheckResult> iresults;
    currentState->m_datastore->getGroupCheckResults(name, false, onlyResolved, iresults);
    for (auto it = iresults.begin(); it != iresults.end(); ++it)
    {
        hostsPresent.insert(it->m_hostName);
        HMAPICheckResult entry(it->m_result);
        entry.m_address.m_type = it->m_address.getType();
        if (it->m_address.getType() == AF_INET)
        {
            entry.m_address.m_ip.addr = it->m_address.addr4();
        } else if (it->m_address.getType() == AF_INET6)
        {
            entry.m_address.m_ip.addr6 = it->m_address.addr6();
        }
        entry.m_host = it->m_hostName;
        results.push_back(entry);
    }
    if (!onlyResolved)
    {
        set<string> missing;
        actualHosts.insert(hostGroupIter->second.getHostList()->begin(),
                hostGroupIter->second.getHostList()->end());
        set_difference(actualHosts.begin(),
                actualHosts.end(),
                hostsPresent.begin(), hostsPresent.end(),
                std::inserter(missing, missing.begin()));
        for(string host : missing)
        {
            HMAPICheckResult empty_result;
            if(hostGroupIter->second.getDualstack() & HM_DUALSTACK_IPV4_ONLY)
            {
                empty_result.m_host = host;
                empty_result.m_address = HMAPIIPAddress(AF_INET);
                results.push_back(empty_result);
            }
            else if (hostGroupIter->second.getDualstack() & HM_DUALSTACK_IPV6_ONLY)
            {
                empty_result.m_host = host;
                empty_result.m_address = HMAPIIPAddress(AF_INET6);
                results.push_back(empty_result);
            }

        }

    }
    return true;
}

bool
HMStorageAPI::getHostGroupAuxInfo(const std::string& name, std::vector<HMAPIAuxInfo>& auxInfo)
{
    auxInfo.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }

    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    // First we need to dig out the TTL
    auto it = currentState->m_hostGroups.find(name);
    if(it == currentState->m_hostGroups.end())
    {
        HMLog(HM_LOG_CRITICAL, "Host Group %s not found in configs", name.c_str());
        return false;
    }
    uint64_t ttl = it->second.getCheckTTL();

    std::vector<HMGroupAuxResult> results;
    if(!currentState->m_datastore->getGroupAuxInfo(name, false, true, results))
    {
        return false;
    }

    for(auto it = results.begin(); it != results.end(); ++it)
    {
        HMAPIAuxInfo tempInfo;

        tempInfo.m_ttl = ttl;
        tempInfo.m_updatetime = it->m_info.m_ts.getTimeSinceEpoch();
        tempInfo.m_address.m_type = it->m_address.getType();
        if(it->m_address.getType() == AF_INET)
        {
            tempInfo.m_address.m_ip.addr = it->m_address.addr4();
        }
        else if(it->m_address.getType() == AF_INET6)
        {
            tempInfo.m_address.m_ip.addr6 = it->m_address.addr6();
        }
        tempInfo.m_host = it->m_hostName;

        for(auto iit = it->m_info.m_auxData.begin(); iit != it->m_info.m_auxData.end(); ++iit)
        {
            HMAuxLoadFB* lfb = (*iit)->getLFB();
            if(lfb)
            {
                HMAPILFB entry;
                entry.m_type = lfb->m_type;
                entry.m_ts = lfb->m_ts.getTimeSinceEpoch();
                entry.m_load = lfb->m_load;
                entry.m_target = lfb->m_target;
                entry.m_max = lfb->m_max;
                entry.m_host = lfb->m_host;
                entry.m_resource = lfb->m_resource;
                entry.m_datacenter = lfb->m_datacenter;
                tempInfo.m_lfb.push_back(entry);
                continue;
            }
            HMAuxOOB* oob = (*iit)->getOOB();
            if(oob)
            {
                HMAPIOOB entry;
                entry.m_type = oob->m_type;
                entry.m_shed = oob->m_shed;
                entry.m_ts = oob->m_ts.getTimeSinceEpoch();
                entry.m_forceDown = oob->m_forceDown;
                entry.m_host = oob->m_host;
                entry.m_resource = oob->m_resource;
                tempInfo.m_oob.push_back(entry);
            }
        }

        auxInfo.push_back(move(tempInfo));
    }

    return true;
}

string
HMStorageAPI::getError()
{
    return log->getLastError();
}

bool
HMStorageAPI::generateAuxXML(const std::string& name, std::vector<std::string>& fileContents, uint64_t& ttl, uint64_t& timeout)
{
    fileContents.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    // First we need to dig out the TTL
    auto it = currentState->m_hostGroups.find(name);
    if(it == currentState->m_hostGroups.end())
    {
        HMLog(HM_LOG_CRITICAL, "Host Group %s not found in configs", name.c_str());
        return false;
    }
    ttl = it->second.getCheckTTL();
    timeout = 0;

    std::vector<HMGroupAuxResult> results;
    if(!currentState->m_datastore->getGroupAuxInfo(name, false, true, results))
    {
        return false;
    }

    for (auto auxResult = results.begin(); auxResult != results.end(); ++auxResult)
    {
        bool loadFile = false;
        bool oobFile = false;

        timeout = (timeout < auxResult->m_info.m_ts.getTimeSinceEpoch()) ? auxResult->m_info.m_ts.getTimeSinceEpoch() : timeout;

        for (auto auxData = auxResult->m_info.m_auxData.begin(); auxData != auxResult->m_info.m_auxData.end(); ++auxData)
        {
            if ((*auxData)->m_type == HM_LOAD_FILE)
            {
                loadFile = true;
            } else if ((*auxData)->m_type == HM_OOB_FILE)
            {
                oobFile = true;
            }
        }

        string xml;

        if (loadFile)
        {
            if(currentState->m_auxCache.genAuxData(auxResult->m_info, HM_LOAD_FILE, name, xml, HM_AUX_DATA_XML))
            {
                fileContents.push_back(xml);
            }
        }

        if (oobFile)
        {
            if(currentState->m_auxCache.genAuxData(auxResult->m_info, HM_OOB_FILE, name, xml, HM_AUX_DATA_XML))
            {
                fileContents.push_back(xml);
            }
        }
    }
    // overriding the value to current time. Needs to cleaned and handled later in libydns.
    timeout = HMTimeStamp::now().getTimeSinceEpoch();
    return true;
}

void
HMStorageAPI::writeAuxXML(const std::string& name, std::vector<std::string>& files)
{
    files.clear();
    log->clearError();
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    // First we need to dig out the TTL
    auto it = currentState->m_hostGroups.find(name);
    if(it == currentState->m_hostGroups.end())
    {
        HMLog(HM_LOG_CRITICAL, "Host Group %s not found in configs", name.c_str());
        return;
    }

    std::vector<HMGroupAuxResult> results;
    if(!currentState->m_datastore->getGroupAuxInfo(name, false, true, results))
    {
        return;
    }

    for (auto auxResult = results.begin(); auxResult != results.end(); ++auxResult)
    {
        bool loadFile = false;
        bool oobFile = false;

        for (auto auxData = auxResult->m_info.m_auxData.begin(); auxData != auxResult->m_info.m_auxData.end(); ++auxData)
        {
            if ((*auxData)->m_type == HM_LOAD_FILE)
            {
                loadFile = true;
            }
            else if ((*auxData)->m_type == HM_OOB_FILE)
            {
                oobFile = true;
            }
        }

        string xml;

        if (loadFile)
        {
            if(currentState->m_auxCache.genAuxData(auxResult->m_info, HM_LOAD_FILE, name, xml, HM_AUX_DATA_XML))
            {
                string fileName = name + "_" + auxResult->m_hostName + "_LoadFile.xml";
                files.push_back(fileName);
                writeXML(fileName, xml);
            }
        }

        if (oobFile)
        {
            if(currentState->m_auxCache.genAuxData(auxResult->m_info, HM_OOB_FILE, name, xml, HM_AUX_DATA_XML))
            {
                string fileName = name + "_" + auxResult->m_hostName + "_OOBFile.xml";
                files.push_back(fileName);
                writeXML(fileName, xml);
            }
        }
    }
}


bool
HMStorageAPI::generateConfigs(const std::string& configName)
{
    // TODO this needs implemented.
    HMLog(HM_LOG_CRITICAL, "Feature not yet implemented.");
    return false;
}

bool
HMStorageAPI::writeConfigs(HM_API_CONFIG_CLASS configs, const string& fileName)
{
    if(!m_loaded)
    {
        HMLog(HM_LOG_CRITICAL, "No backend loaded");
        return false;
    }
    std::shared_ptr<HMState> currentState;
    updateState(currentState);
    return currentState->writeConfigs((HM_CONFIG_PLUGIN_CLASS)configs, fileName);
}

bool
HMStorageAPI::writeXML(string& fileName, string& xml)
{
    ofstream fout(fileName.c_str());
    if(!fout.is_open())
    {
        // Log error
        return false;
    }

    fout << xml << "\n";
    fout.close();
    return true;
}
