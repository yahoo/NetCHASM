// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <string>
#include <vector>
#include <memory>

#include "HMStorageHostGroup.h"
#include "HMLogBase.h"
#include "HMAuxCache.h"
#include "HMState.h"
#include "HMDataHostGroup.h"
#include "HMDataHostCheck.h"
#include "HMDataCheckParams.h"

using namespace std;

void
HMStorageHostGroup::initResultsFromBackend(HMDataCheckList& checkList, HMDNSCache& dnsCache, HMAuxCache& auxCache)
{

    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to store result in read only mode");
        return;
    }

    // shutdown the write thread while re-loading
    m_shutdown = true;
    m_dataReadyCond.notify_one();
    if(m_thread.joinable())
    {
        m_thread.join();
    }
    m_shutdown = false;

    // Purge the internal data
    {
        lock_guard<shared_timed_mutex> lock(m_checkUpdateMutex);
        m_hostGroupResults.clear();
    }
    {
        lock_guard<shared_timed_mutex> lock(m_auxUpdateMutex);
        m_hostGroupAux.clear();
    }

    // Clear the write queues
    m_storeCheckQueue = queue<HMGroupCheckUpdate>();
    m_storeAuxQueue = queue<HMGroupAuxUpdate>();

    // bring in all info from the backend
    multimap<string, HMGroupCheckResult> backendChecks;
    multimap<string, HMGroupAuxResult> backendAux;

    // Fist sync all data to the cache
    set<string> groupNames;
    getHostGroupNames(groupNames);

    for(auto name = groupNames.begin(); name != groupNames.end(); ++name)
    {
        vector<HMGroupCheckResult> checkResults;
        vector<HMGroupAuxResult> auxResults;
        if(getHostGroupCheckResults(*name, checkResults))
        {
            for(auto it = checkResults.begin(); it != checkResults.end(); ++it)
            {
                backendChecks.emplace(*name, *it);
            }
        }

        if(getHostGroupGroupAuxInfo(*name, auxResults))
        {
            for(auto it = auxResults.begin(); it != auxResults.end(); ++it)
            {
                backendAux.emplace(*name, *it);
            }
        }
    }

    // Set of rotations that need pushed from the internal data
    set<string> healthCheckUpdateList;
    set<string> auxInfoUpdateList;
    map<pair<string, HMDNSLookup>,set<HMIPAddress>> dnsMap;
    map<pair<string, HMDNSLookup>,HMTimeStamp> dnsResTimeMap;

    // first we purge rotations that don't exist in the current backend
    for (auto it = backendChecks.begin(); it != backendChecks.end();)
    {
        auto hostGroup = m_hostGroupMap->find(it->first);

        // Purge hostGroup in the backend if it is not in the current configs
        if(hostGroup == m_hostGroupMap->end())
        {
            removeHostGroupCheckResults(it->first);
            it = backendChecks.erase(it);
            continue;
        }

        // Now check for a change in the hostGroup parameters
        // If the hostgroup info changed, save the IP Addresses and clear the checkResults
        // Note we will enter blank results later
        HMDataHostGroup backendHostGroupData(it->first);
        if(getGroupInfo(it->first, backendHostGroupData))
        {
            if (hostGroup->second != backendHostGroupData)
            {
                // save the IP mapping information for the host and blow away the hostGroup
                uint8_t dualstack = hostGroup->second.getDualstack();
                auto range = backendChecks.equal_range(it->first);
                for (auto iit = range.first; iit != range.second; ++iit)
                {
                    if((iit->second.m_address.getType() == AF_INET && dualstack == HM_DUALSTACK_IPV6_ONLY)
                            || (iit->second.m_address.getType() == AF_INET6 && dualstack == HM_DUALSTACK_IPV4_ONLY))
                    {
                        continue;
                    }
                    HMDNSLookup dnsHostCheck(hostGroup->second.getDnsCheckPlugin(), it->second.m_address.getType() == AF_INET6);
                    pair<string, HMDNSLookup> key = make_pair(it->second.m_hostName,  dnsHostCheck);
                    auto res = dnsMap.insert(make_pair(key, set<HMIPAddress>()));
                    res.first->second.insert(it->second.m_address);
                    if(dnsResTimeMap.find(key) == dnsResTimeMap.end())
                    {
                        dnsResTimeMap.insert(make_pair(key, it->second.m_result.m_checkTime));
                    }
                }
                // Remove the host group
                removeHostGroupCheckResults(it->first);
                it = backendChecks.erase(it);
                continue;
            }
        }
        // Purge the hostName from the backend if it does not exist in the current configs
        if (!hostGroup->second.isValidHost(it->second.m_hostName))
        {
            // Log a missing host group
            HMDataHostCheck dataCheck;
            HMDataCheckParams checkParams;
            backendHostGroupData.getHostCheck(dataCheck);
            backendHostGroupData.getCheckParameters(checkParams);
            HMLog(HM_LOG_DEBUG, "[STORE] %s - %s found in backend but not configs", it->first.c_str(), it->second.m_hostName.c_str());
            purgeCheckResult(it->second.m_hostName, it->second.m_address, dataCheck, checkParams);
            healthCheckUpdateList.insert(it->first);
            ++it;
            continue;
        }

        HMLog(HM_LOG_DEBUG, "[STORE] %s - %s reloaded into DNS cache and checklist", it->first.c_str(), it->second.m_hostName.c_str());

        // Cache the DNS mapping
        HMDNSLookup dnsHostCheck(hostGroup->second.getDnsCheckPlugin(), it->second.m_address.getType() == AF_INET6);
        pair<string, HMDNSLookup> key = make_pair(it->second.m_hostName, dnsHostCheck);
        auto res = dnsMap.insert(make_pair(key, set<HMIPAddress>()));
        res.first->second.insert(it->second.m_address);
        if(dnsResTimeMap.find(key) == dnsResTimeMap.end())
        {
            dnsResTimeMap.insert(make_pair(key, it->second.m_result.m_checkTime));
        }

        HMDataHostCheck hostCheck;
        HMDataCheckParams checkParams;
        hostGroup->second.getHostCheck(hostCheck);
        hostGroup->second.getCheckParameters(checkParams);
        checkList.updateCheck(HMCheckHeader(it->second.m_hostName, it->second.m_address, hostCheck, checkParams), it->second.m_result);
        ++it;
    }

    // so the aux fetch has a corresponding host check entry so we don't need to worry about IP Address mapping etc
    // Just remove the local aux info
    for (auto it = backendAux.begin(); it != backendAux.end();)
    {
        auto hostGroup = m_hostGroupMap->find(it->first);
        if(hostGroup == m_hostGroupMap->end())
        {
            it = backendAux.erase(it);
            continue;
        }

        // remove stored aux info if the hostGroup changed
        HMDataHostGroup backendHostGroupData(it->first);
        if(getGroupInfo(it->first, backendHostGroupData))
        {
            if (hostGroup->second != backendHostGroupData)
            {
                it = backendAux.erase(it);
                continue;
            }
        }
       // remove stored aux if the host was removed
        if (!hostGroup->second.isValidHost(it->second.m_hostName))
        {
            auxInfoUpdateList.insert(it->first);
            it = backendAux.erase(it);
            continue;
        }
        ++it;
    }

    // Now update the internal DNS Cache
    for(auto it = dnsMap.begin(); it != dnsMap.end(); ++it)
    {
        HMDNSResult v4Result;
        HMDNSResult v6Result;
        auto res = dnsResTimeMap.find(it->first);
        if (res != dnsResTimeMap.end() && !res->first.second.isIpv6())
        {
            v4Result.setResultTime(res->second);
        }
        else if (res != dnsResTimeMap.end() && res->first.second.isIpv6())
        {
            v6Result.setResultTime(res->second);
        }
        dnsCache.updateReloadDNSEntry(it->first.first, it->second, v4Result, v6Result, res->first.second.getPlugin());
    }

    // Now update the internal checkList
    for (auto it = backendChecks.begin(); it != backendChecks.end(); ++it)
    {
        auto hostGroup = m_hostGroupMap->find(it->first);
        HMDataHostCheck hostCheck;
        HMDataCheckParams checkParams;
        hostGroup->second.getHostCheck(hostCheck);
        hostGroup->second.getCheckParameters(checkParams);
        checkList.updateCheck(HMCheckHeader(it->second.m_hostName,
                        it->second.m_address,
                        hostCheck,
                        checkParams),
                        it->second.m_result);
        lock_guard<shared_timed_mutex> lock(m_checkUpdateMutex);
        m_hostGroupResults.insert(make_pair(it->first, it->second));
    }

    // Now update the internal AuxInfo
    for (auto it = backendAux.begin(); it != backendAux.end(); ++it)
    {
        auto hostGroup = m_hostGroupMap->find(it->first);
        auxCache.updateAuxInfo(it->second.m_hostName,
                hostGroup->second.getCheckInfo(),
                it->second.m_address,
                it->second.m_info);
        lock_guard<shared_timed_mutex> lock(m_auxUpdateMutex);
        m_hostGroupAux.insert(make_pair(it->first, it->second));
    }

    // Power up the storage thread again
    if(!m_readonly)
    {
        m_thread = thread(&HMStorageHostGroup::runStore, this);
    }

    // Now commit the healthChecks to the backend
    for (auto it = healthCheckUpdateList.begin(); it != healthCheckUpdateList.end(); ++it)
    {
        storeHostGroupCheckResults(*it);
    }

    // Now commit the AuxInfo to the backend
    for (auto it = auxInfoUpdateList.begin(); it != auxInfoUpdateList.end(); ++it)
    {
        storeHostGroupAuxInfo(*it);
    }
}

bool
HMStorageHostGroup::storeConfigs(HMState& checkState)
{
    set<string> hostNames;
    for(auto it = checkState.m_hostGroups.begin(); it != checkState.m_hostGroups.end(); ++it)
    {
        hostNames.insert(it->first);
        if(!storeGroupInfo(it->first, it->second))
        {
            return false;
        }
    }
    return storeHostGroupNames(hostNames);
}

bool
HMStorageHostGroup::getConfigs(HMState& checkState)
{
    checkState.m_hostGroups.clear();

    set<string> groupNames;
    getHostGroupNames(groupNames);
    HMConfigInfo configInfo;
    if(getConfigInfo(configInfo))
    {
        checkState.setHash(configInfo.m_hash);
    }
    for(auto it = groupNames.begin(); it != groupNames.end(); ++it)
    {
        HMDataHostGroup group(*it);
        if(!getGroupInfo(*it, group))
        {
            return false;
        }
        checkState.m_checkList.addHostGroup(group);
        checkState.m_hostGroups.insert(make_pair(*it, move(group)));
    }
    if(groupNames.size() == 0)
    {
        HMLog(HM_LOG_WARNING, "[STORE] No Hostgroups present in backend while loading configs from backend");
    }
    return true;
}

bool
HMStorageHostGroup::storeCheckResult(const string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            const HMDataCheckResult& checkResult)
{
    // This parameter is not needed for the Host Group Class
    (void)hostCheck;

    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to store check result in read only mode");
        return false;
    }

    vector<string> hostGroups;
    checkParams.getHostGroups(hostGroups);

    if(hostGroups.size() == 0)
    {
        HMLog(HM_LOG_WARNING, "[STORE] Attempting to store check result with no results");
        return false;
    }

    for(auto it = hostGroups.begin(); it != hostGroups.end(); ++it)
    {
        HMLog(HM_LOG_DEBUG, "[STORE] Inserting into store check queue hostgroup %s of host %s", it->c_str(), hostname.c_str());
        HMGroupCheckUpdate update(*it, hostname, address, checkResult);
        // Now add this to the update list
        m_checkQueueSpinLock.lock();
        m_storeCheckQueue.push(update);
        m_checkQueueSpinLock.unlock();

        lock_guard<mutex> lk(m_dataReadyMutex);
        m_dataReadyCond.notify_one();
    }
    return true;
}

bool
HMStorageHostGroup::getCheckResult(const string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            HMDataCheckResult& checkResult)
{
    (void)hostCheck;

    // first lookup the hostgroup note the answer should be on all the active host groups
    vector<string> hostGroups;
    checkParams.getHostGroups(hostGroups);

    // iterate over active host maps
    for(auto it = hostGroups.begin(); it != hostGroups.end(); ++it)
    {
        if(getHostGroupCheckResults(*it))
        {
            // Now parse out the result and return
            shared_lock<shared_timed_mutex> lock(m_checkUpdateMutex);
            auto entries = m_hostGroupResults.equal_range(*it);
            for(auto iit = entries.first; iit != entries.second; ++iit)
            {
                if(iit->second.m_address == address && iit->second.m_hostName == hostname)
                {
                    // if we do, update the entry and stop looking
                    checkResult = iit->second.m_result;
                    return true;
                }
            }
        }
    }
    HMLog(HM_LOG_DEBUG, "[STORE] check results for %s at %s not found", hostname.c_str(), address.toString().c_str());
    return false;
}

bool
HMStorageHostGroup::purgeCheckResult(const string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams)
{
    (void) hostCheck;
    // first lookup the hostgroup note the answer should be on all the active host groups
    bool found = false;
    string hostgroup;

    vector<string> hostGroups;
    checkParams.getHostGroups(hostGroups);
    // iterate over active host maps
    for (auto it = hostGroups.begin(); it != hostGroups.end(); ++it)
    {
        if (getHostGroupCheckResults(*it))
        {
            // Now parse out the result and return
            lock_guard<shared_timed_mutex> lock(m_checkUpdateMutex);
            auto entries = m_hostGroupResults.equal_range(*it);
            for (auto iit = entries.first; iit != entries.second; ++iit)
            {
                if (iit->second.m_address == address
                        && iit->second.m_hostName == hostname)
                {
                    m_hostGroupResults.erase(iit);
                    hostgroup = *it;
                    found = true;
                    break;
                }
            }
        }
        if (found)
        {
            storeHostGroupCheckResults(hostgroup);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "[STORE] check results for %s at %s not found to delete", hostname.c_str(), address.toString().c_str());
        }
    }
    return true;
}

bool
HMStorageHostGroup::storeAuxInfo(const string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams,
        const HMAuxInfo& auxInfo)
{
    // This parameter is not needed for the Host Group Class
    (void)hostCheck;

    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to store result in read only mode");
        return false;
    }

    vector<string> hostGroups;
    checkParams.getHostGroups(hostGroups);

    if(hostGroups.size() == 0)
    {
        HMLog(HM_LOG_WARNING, "[STORE] Attempting to store result with no results");
        return false;
    }

    for(auto it = hostGroups.begin(); it != hostGroups.end(); ++it)
    {
        HMLog(HM_LOG_DEBUG, "[STORE] Inserting into store aux queue hostgroup %s of host %s", it->c_str(), hostname.c_str());

        HMGroupAuxUpdate update(*it, hostname, address, auxInfo);

        // Now add this to the update list
        m_auxQueueSpinLock.lock();
        m_storeAuxQueue.push(update);
        m_auxQueueSpinLock.unlock();

        lock_guard<mutex> lk(m_dataReadyMutex);
        m_dataReadyCond.notify_one();
    }
    return true;
}

bool
HMStorageHostGroup::getAuxInfo(const string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams,
        HMAuxInfo& auxInfo)
{
    (void)hostCheck;

    // first lookup the hostgroup note the answer should be on all the active host groups
    vector<string> hostGroups;
    checkParams.getHostGroups(hostGroups);

    // iterate over active host maps
    for(auto it = hostGroups.begin(); it != hostGroups.end(); ++it)
    {
        if(getHostGroupGroupAuxInfo(*it))
        {
            // Now parse out the result and return
            shared_lock<shared_timed_mutex> lock(m_auxUpdateMutex);
            auto entries = m_hostGroupAux.equal_range(*it);
            for(auto iit = entries.first; iit != entries.second; ++iit)
            {
                if(iit->second.m_address == address && iit->second.m_hostName == hostname)
                {
                    auxInfo = iit->second.m_info;
                    return true;
                }
            }
        }
    }
    HMLog(HM_LOG_DEBUG, "[STORE] get auxinfo %s at %s not found", hostname.c_str(), address.toString().c_str());
    return false;
}

bool
HMStorageHostGroup::purgeAuxInfo(const string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams)
{
    (void) hostCheck;
    // first lookup the hostgroup note the answer should be on all the active host groups
    bool found = false;
    string hostgroup;

    vector<string> hostGroups;
    checkParams.getHostGroups(hostGroups);
    // iterate over active host maps
    for (auto it = hostGroups.begin(); it != hostGroups.end(); ++it)
    {
        if (getHostGroupGroupAuxInfo(*it))
        {
            // Now parse out the result and return
            lock_guard<shared_timed_mutex> lock(m_auxUpdateMutex);
            auto entries = m_hostGroupAux.equal_range(*it);
            for (auto iit = entries.first; iit != entries.second; ++iit)
            {
                if (iit->second.m_address == address && iit->second.m_hostName == hostname)
                {
                    m_hostGroupAux.erase(iit);
                    hostgroup = *it;
                    found = true;
                    break;
                }
            }
        }
        if (found)
        {
            storeHostGroupAuxInfo(hostgroup);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "[STORE] auxinfo %s at %s not found to delete", hostname.c_str(), address.toString().c_str());
        }
    }
    return true;
}

bool
HMStorageHostGroup::getDNS(const std::string& hostname, std::set<HMIPAddress>& ips)
{
    ips.clear();
    string foundGroup;
    set<string> groups;
    if(!getHostGroupNames(groups))
    {
        return false;
    }

    // First we need to find the host in a hostgroup
    for(auto group = groups.begin(); group != groups.end() && foundGroup.empty(); ++group)
    {
        HMDataHostGroup groupInfo(*group);
        if(getGroupInfo(*group, groupInfo))
        {
            for(auto it = groupInfo.getHostList()->begin(); it != groupInfo.getHostList()->end(); ++it)
            {
                if(*it == hostname)
                {
                    foundGroup = *group;
                    break;
                }
            }
        }
    }

    if(foundGroup.empty())
    {
        return false;
    }

    // Now get the results for this group and return the addresses
    vector<HMGroupCheckResult> results;
    if(!getHostGroupCheckResults(foundGroup, results))
    {
        return false;
    }

    for(HMGroupCheckResult result : results)
    {
        if(result.m_hostName == hostname)
        {
            ips.insert(result.m_address);
        }
    }
    return true;
}

bool
HMStorageHostGroup::updateCheckResultCache(HMCheckHeader& header, HMDataCheckResult& result)
{
    bool found = false;
    lock_guard<shared_timed_mutex> lock(m_checkUpdateMutex);
    vector<string> hostGroupNames;
    header.m_checkParams.getHostGroups(hostGroupNames);
    for (string groupName : hostGroupNames)
    {
        auto entries = m_hostGroupResults.equal_range(groupName);
        for (auto iit = entries.first; iit != entries.second; ++iit)
        {
            if (iit->second.m_hostName == header.m_hostname
                    && iit->second.m_address == result.m_address)
            {
                // if we do, update the entry and stop looking
                found = true;
                iit->second.m_result = result;
                iit->second.m_address = result.m_address;
                iit->second.m_result.m_address = result.m_address;
            }
        }
        if (!found)
        {
            m_hostGroupResults.insert(make_pair(groupName, HMGroupCheckResult(header.m_hostname, header.m_address, result)));
        }
    }

    return true;
}

bool
HMStorageHostGroup::updateAuxInfoCache(HMCheckHeader& header, HMAuxInfo& aux)
{
    bool found = false;
    lock_guard<shared_timed_mutex> lock(m_auxUpdateMutex);

    vector<string> hostGroupNames;
    header.m_checkParams.getHostGroups(hostGroupNames);
    for (string groupName : hostGroupNames)
    {
        auto entries = m_hostGroupAux.equal_range(groupName);
        for (auto iit = entries.first; iit != entries.second; ++iit)
        {
            if (iit->second.m_hostName == header.m_hostname && iit->second.m_address == header.m_address)
            {
                // if we do, update the entry and stop looking
                found = true;

                iit->second.m_address = header.m_address;
                iit->second.m_info = aux;
            }
        }
        if (!found)
        {
            m_hostGroupAux.insert(make_pair(groupName, HMGroupAuxResult(header.m_hostname, header.m_address, aux)));
        }
    }

    return true;
}

void
HMStorageHostGroup::updateHostGroups(set<string>& hostGroups)
{
    // iterate over active host groups
    shared_lock<shared_timed_mutex> lock(m_checkUpdateMutex);
    for (auto it = hostGroups.begin(); it != hostGroups.end(); ++it)
    {
        if (m_hostGroupResults.find(*it) != m_hostGroupResults.end())
        {
            HMLog(HM_LOG_DEBUG, "[STORE] Updated HostGroup %s", it->c_str());
            storeHostGroupCheckResults(*it);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "[STORE] Deleted HostGroup %s", it->c_str());
            removeHostGroupCheckResults(*it);
        }
    }
}

bool
HMStorageHostGroup::getGroupCheckResults(const string& groupName, bool noCache, bool onlyResolved, vector<HMGroupCheckResult>& results)
{
    results.clear();

    // No Cache will query the backend without overwriting the local cache
    if(noCache)
    {
        vector<HMGroupCheckResult> tmp_results;
        if(!getHostGroupCheckResults(groupName, tmp_results))
        {
            // Log hostgroup not found
            HMLog(HM_LOG_DEBUG, "[STORE] %s not found", groupName.c_str());
            return false;
        }

        for(auto it = tmp_results.begin(); it != tmp_results.end(); ++it)
        {
            if (!onlyResolved
                    || (onlyResolved
                            && it->m_result.m_response != HM_RESPONSE_DNS_FAILED
                            && it->m_result.m_response != HM_RESPONSE_NONE))
            {
                results.push_back(*it);
            }
        }
        return true;
    }

    if(!getHostGroupCheckResults(groupName))
    {
        // Log hostgroup not found
        HMLog(HM_LOG_DEBUG, "[STORE] %s not found", groupName.c_str());
        return false;
    }

    shared_lock<shared_timed_mutex> lock(m_checkUpdateMutex);
    auto range = m_hostGroupResults.equal_range(groupName);
    for(auto it = range.first; it != range.second; ++it)
    {
        if (!onlyResolved
                || (onlyResolved
                        && it->second.m_result.m_response != HM_RESPONSE_DNS_FAILED
                        && it->second.m_result.m_response != HM_RESPONSE_NONE))
        {
            results.push_back(it->second);
        }
    }
    return true;
}

bool
HMStorageHostGroup::getGroupAuxInfo(const string& groupName, bool noCache, bool onlyResolved, vector<HMGroupAuxResult>& results)
{
    results.clear();

    // No Cache will query the backend without overwriting the local cache
    if(noCache)
    {
        vector<HMGroupAuxResult> tmp_results;
        getHostGroupGroupAuxInfo(groupName, tmp_results);
        for(auto it = tmp_results.begin(); it != tmp_results.end(); ++it)
        {
            if (!onlyResolved
                    || (onlyResolved
                            && ((it->m_address.getType() == AF_INET6  && it->m_address != HMIPAddress(AF_INET6))
                                    || (it->m_address.getType() == AF_INET && it->m_address != HMIPAddress(AF_INET)))))
            {
                results.push_back(HMGroupAuxResult(it->m_hostName, it->m_address, it->m_info));
            }
        }
        return true;
    }

    getHostGroupGroupAuxInfo(groupName);
    shared_lock<shared_timed_mutex> lock(m_auxUpdateMutex);
    auto range = m_hostGroupAux.equal_range(groupName);
    for(auto it = range.first; it != range.second; ++it)
    {
        if (!onlyResolved
                || (onlyResolved
                        && ((it->second.m_address.getType() == AF_INET6 && it->second.m_address != HMIPAddress(AF_INET6))
                                || (it->second.m_address.getType() == AF_INET && it->second.m_address != HMIPAddress(AF_INET)))))
        {
           results.push_back(HMGroupAuxResult(it->second.m_hostName, it->second.m_address, it->second.m_info));
        }
    }
    return true;
}

bool
HMStorageHostGroup::updateCheckResultsCache(HMCheckHeader& header, HMDataCheckResult& result)
{
    bool found = false;
    lock_guard<shared_timed_mutex> lock(m_checkUpdateMutex);

    vector<string> hostGroupNames;
    header.m_checkParams.getHostGroups(hostGroupNames);
    for (string groupName : hostGroupNames)
    {
        auto entries = m_hostGroupResults.equal_range(groupName);
        for (auto iit = entries.first; iit != entries.second; ++iit)
        {
            if (iit->second.m_hostName == header.m_hostname
                    && iit->second.m_address == result.m_address)
            {
                // if we do, update the entry and stop looking
                found = true;
                iit->second.m_result = result;
                iit->second.m_address = result.m_address;
                iit->second.m_result.m_address = result.m_address;
            }
        }
        if (!found)
        {
            m_hostGroupResults.insert(make_pair(groupName, HMGroupCheckResult(header.m_hostname, header.m_address, result)));
        }
    }

    return true;
}

bool
HMStorageHostGroup::commitHealthCheck()
{
    bool found = false;

    m_checkQueueSpinLock.lock();
    if(m_storeCheckQueue.size() == 0)
    {
        m_checkQueueSpinLock.unlock();
        return false;
    }

    HMGroupCheckUpdate update = m_storeCheckQueue.front();
    m_storeCheckQueue.pop();
    m_checkQueueSpinLock.unlock();

    // Get the check TTL
    uint64_t ttl;
    auto it = m_hostGroupMap->find(update.m_hostGroup);
    if(it == m_hostGroupMap->end())
    {
        return false;
    }
    else
    {
        HMDNSLookup dnsHostCheck(it->second.getDnsCheckPlugin(), update.m_address.getType() == AF_INET6);
        bool isValidAddress = m_dnsCache->isValidAddress(update.m_hostName,
                it->second.getDualstack(), dnsHostCheck, update.m_address);
        if (!isValidAddress)
        {
            return true;
        }
    }
    ttl = it->second.getCheckTTL();

    set<string> hosts;
    bool coldStart = false;
    bool allStale = true;
    HMTimeStamp oldest = HMTimeStamp::now();


    // Check to see if we already have an entry for this ip address
    { // establish scope for our lock
        lock_guard<shared_timed_mutex> lock(m_checkUpdateMutex);
        auto entries = m_hostGroupResults.equal_range(update.m_hostGroup);
        for(auto iit = entries.first; iit != entries.second; ++iit)
        {
            if (!found
                    && (iit->second.m_hostName == update.m_hostName)
                    && ((iit->second.m_address == update.m_address)
                            || (update.m_address.getType() == AF_INET6
                                    && iit->second.m_address == HMIPAddress(AF_INET6))
                            || (update.m_address.getType() == AF_INET
                                    && iit->second.m_address == HMIPAddress(AF_INET))))
            {
                // if we do, update the entry and stop looking
                found = true;
                iit->second.m_result = update.m_result;
                iit->second.m_address = update.m_address;
                iit->second.m_result.m_address = update.m_address;
                iit->second.m_backendStale = true;
            }
            allStale = (allStale && iit->second.m_backendStale);
            oldest = (iit->second.m_commitTime < oldest) ? iit->second.m_commitTime : oldest;
            hosts.insert(iit->second.m_hostName);
        }
    }
    if(!found)
    {
        coldStart  = true;
        hosts.insert(update.m_hostName);
        // If no entry is found add one
        lock_guard<shared_timed_mutex> lock(m_checkUpdateMutex);
        m_hostGroupResults.insert(make_pair(update.m_hostGroup, HMGroupCheckResult(update.m_hostName, update.m_address, update.m_result)));
    }

    // Note this is where we can insert a check to see if update on every cycle... ie a timeout or update only when the whole rotation is updated
    // Update only if all the hosts information are available
    auto hostGroupIt = m_hostGroupMap->find(update.m_hostGroup);
    if (hostGroupIt != m_hostGroupMap->end())
    {
        // If we don't have the full health check complete return
        if(hosts.size() < hostGroupIt->second.getHostList()->size())
        {
            return true;
        }

        // Now we choose if we should commit the entry.
        // No matter what our update strategy is, we push an update if this is the last host in the initial check cycle
        // Also update if we are in amways commit.
        if(coldStart
                || (m_healthCheckCommitPolicy == HM_STORAGE_COMMIT_ALWAYS)
                || ((m_healthCheckCommitPolicy == HM_STORAGE_COMMIT_ON_FIRST) && (update.m_hostName == hostGroupIt->second.getHostList()->front()))
                || ((m_healthCheckCommitPolicy == HM_STORAGE_COMMIT_ON_ALL_READY) && allStale)
                || ((m_healthCheckCommitPolicy == HM_STORAGE_COMMIT_ON_TTL) && (ttl < (HMTimeStamp::now() - oldest))))
        {
            storeHostGroupCheckResults(update.m_hostGroup);

            // Now set the proper cache params for the next commit

            lock_guard<shared_timed_mutex> lock(m_checkUpdateMutex);
            auto entries = m_hostGroupResults.equal_range(update.m_hostGroup);
            HMTimeStamp ts = HMTimeStamp::now();
            for(auto iit = entries.first; iit != entries.second; ++iit)
            {
                iit->second.m_backendStale = false;
                iit->second.m_commitTime = ts;
            }
        }
    }
    else
    {
        HMLog(HM_LOG_ERROR, "[STORE] HostGroup %s missing HostGroupMap ");
        return false;
    }
    return true;
}

bool
HMStorageHostGroup::commitAuxInfo()
{
    bool found = false;

    m_auxQueueSpinLock.lock();
    if(m_storeAuxQueue.size() == 0)
    {
        m_auxQueueSpinLock.unlock();
        return false;
    }

    HMGroupAuxUpdate update = m_storeAuxQueue.front();
    m_storeAuxQueue.pop();
    m_auxQueueSpinLock.unlock();

    // Get the check TTL
    uint64_t ttl;
    auto it = m_hostGroupMap->find(update.m_hostGroup);
    if(it == m_hostGroupMap->end())
    {
        return false;
    }
    else
    {
        HMDNSLookup dnsHostCheck(it->second.getDnsCheckPlugin(), update.m_address.getType() == AF_INET6);
        bool isValidAddress = m_dnsCache->isValidAddress(update.m_hostName,
                it->second.getDualstack(), dnsHostCheck, update.m_address);
        if (!isValidAddress)
        {
            return true;
        }
    }

    ttl = it->second.getCheckTTL();

    set<string> hosts;
    bool coldStart = false;
    bool allStale = true;
    HMTimeStamp oldest = HMTimeStamp::now();

    // Check to see if we already have an entry for this ip address
    { // establish scope for our lock
        lock_guard<shared_timed_mutex> lock(m_auxUpdateMutex);
        auto entries = m_hostGroupAux.equal_range(update.m_hostGroup);
        for(auto iit = entries.first; iit != entries.second; ++iit)
        {
            if (!found
                    && (iit->second.m_hostName == update.m_hostName)
                    && ((iit->second.m_address == update.m_address)
                            || (update.m_address.getType() == AF_INET6
                                    && iit->second.m_address == HMIPAddress(AF_INET6))
                            || (update.m_address.getType() == AF_INET
                                    && iit->second.m_address == HMIPAddress(AF_INET))))
            {
                // if we do, update the entry
                found = true;
                iit->second.m_hostName = update.m_hostName;
                iit->second.m_address = update.m_address;
                iit->second.m_info = update.m_info;
                iit->second.m_backendStale = true;
            }
            allStale = (allStale && iit->second.m_backendStale);
            oldest = (iit->second.m_commitTime < oldest) ? iit->second.m_commitTime : oldest;
            hosts.insert(iit->second.m_hostName);
        }
    }
    if(!found)
    {
        coldStart  = true;
        hosts.insert(update.m_hostName);

        // If no entry is found add one
        lock_guard<shared_timed_mutex> lock(m_auxUpdateMutex);

        m_hostGroupAux.insert(make_pair(update.m_hostGroup, HMGroupAuxResult(update.m_hostName, update.m_address, update.m_info)));
    }

    auto hostGroupIt = m_hostGroupMap->find(update.m_hostGroup);
    if (hostGroupIt != m_hostGroupMap->end())
    {
        // If we don't have the full health check complete return
        if(hosts.size() < hostGroupIt->second.getHostList()->size())
        {
            return true;
        }

        // Now we choose if we should commit the entry.
        // No matter what our update strategy is, we push an update if this is the last host in the initial check cycle
        // Also update if we are in amways commit.
        if(coldStart
                || (m_auxCommitPolicy == HM_STORAGE_COMMIT_ALWAYS)
                || ((m_auxCommitPolicy == HM_STORAGE_COMMIT_ON_FIRST) && (update.m_hostName == hostGroupIt->second.getHostList()->front()))
                || ((m_auxCommitPolicy == HM_STORAGE_COMMIT_ON_ALL_READY) && allStale)
                || ((m_auxCommitPolicy == HM_STORAGE_COMMIT_ON_TTL) && (ttl < (HMTimeStamp::now() - oldest))))
        {
            storeHostGroupAuxInfo(update.m_hostGroup);

            // Now set the proper cache params for the next commit
            lock_guard<shared_timed_mutex> lock(m_auxUpdateMutex);
            auto entries = m_hostGroupAux.equal_range(update.m_hostGroup);
            HMTimeStamp ts = HMTimeStamp::now();
            for(auto iit = entries.first; iit != entries.second; ++iit)
            {
                iit->second.m_backendStale = false;
                iit->second.m_commitTime = ts;
            }
        }
    }
    else
    {
        HMLog(HM_LOG_ERROR, "[STORE] HostGroup %s missing HostGroupMap ");
        return false;
    }
    return true;
}

bool
HMStorageHostGroup::getInternalHostGroupInfo(const string& hostGroupName, const HMDataHostGroup* &hostGroup)
{
    HMDataHostGroupMap::const_iterator getGroup = m_hostGroupMap->find(hostGroupName);
    if(getGroup == m_hostGroupMap->end())
    {
        return false;
    }
    hostGroup = &getGroup->second;
    return true;
}

uint32_t
HMStorageHostGroup::getInternalCheckResults(const string& hostGroupName, vector<HMGroupCheckResult>& results)
{
    uint32_t count = 0;
    results.clear();

    shared_lock<shared_timed_mutex> lock(m_checkUpdateMutex);
    auto rangeResults = m_hostGroupResults.equal_range(hostGroupName);
    for(auto it = rangeResults.first; it != rangeResults.second; ++it)
    {
        count++;
        results.push_back(it->second);
    }
    return count;
}
