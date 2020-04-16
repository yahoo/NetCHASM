// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <string>
#include <list>

#include "HMStorageHost.h"
#include "HMLogBase.h"
#include "HMAuxCache.h"
#include "HMState.h"

using namespace std;

void
HMStorageHost::initResultsFromBackend(HMDataCheckList& checkList, HMDNSCache& dnsCache, HMAuxCache& auxCache, HMRemoteHostGroupCache& remoteCache, HMRemoteHostCache& remoteHostCache)
{
    (void)remoteCache;
    set<string> backendNames;
    set<string> localNames;

    map<pair<string, HMDNSLookup>,set<HMIPAddress>> dnsMap;
    map<pair<string, HMDNSLookup>,HMTimeStamp> dnsResTimeMap;

    HMIPAddress blank(AF_INET);
    HMIPAddress blankv6(AF_INET6);

    list<HMCheckHeader> localChecks;
    vector<HMCheckHeader> getChecks;

    if(!getHostNames(backendNames))
    {
        HMLog(HM_LOG_ERROR, "Failed to fetch host names from backend");
        return;
    }

    checkList.getAllChecks(getChecks);
    for(auto it = getChecks.begin(); it != getChecks.end(); ++it)
    {
        localNames.insert(it->m_hostname);
        localChecks.push_back(*it);
    }

    // We have three groups to deal with:
    // 1 The names in both datasets (check by check update to local and removal from backend)
    // 2 Names in the backend but not local (delete)
    // 3 Names in the local but not backend (blank entries needed for IPAddress)
    set<string> both;

    for(auto it = backendNames.begin(); it != backendNames.end(); )
    {
        if(localNames.find(*it) != localNames.end())
        {
            both.insert(*it);
            localNames.erase(*it);
            it = backendNames.erase(it);
            continue;
        }
        ++it;
    }

    // Delete the extra names in the backend
    for(auto it = backendNames.begin(); it != backendNames.end(); ++it)
    {
        removeName(*it);
    }

    // Now insert the local checks that are not present in the backend
    for(auto it = localNames.begin(); it != localNames.end(); ++it)
    {
        vector<HMCheckHeader> headers;
        bool v6, v4 = false;
        HMCheckHeader checkHeader;
        for(auto check = localChecks.begin(); check != localChecks.end(); )
        {
            if(*it == check->m_hostname)
            {
                // The check does not exist in the backend
                v4 = (check->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV4_ONLY) ? true : v4;
                v6 = (check->m_hostCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY) ? true : v6;
                checkHeader.m_hostCheck = check->m_hostCheck;
                checkHeader.m_hostname = check->m_hostname;
                checkHeader.m_checkParams = check->m_checkParams;
                checkHeader.m_address = check->m_address;
                headers.push_back(*check);
                HMCheckData cd(check->m_hostname, check->m_address, check->m_hostCheck, check->m_checkParams, HMDataCheckResult());
                HMAuxInfo empty;
                HMAuxData ca(check->m_hostname, check->m_address, check->m_hostCheck, check->m_checkParams, empty);

                if(!storeHostCheckResult(cd))
                {
                    HMLog(HM_LOG_ERROR, "Failed to store check entry for %s", check->m_hostname.c_str());
                }

                if(!storeHostAuxInfo(ca))
                {
                    HMLog(HM_LOG_ERROR, "Failed to store aux entry for %s", check->m_hostname.c_str());
                }

                check = localChecks.erase(check);
                continue;
            }
            ++check;
        }

        // Store the headers
        if(!storeNameChecks(*it, headers))
        {
            HMLog(HM_LOG_ERROR, "Failed to store headers for %s", it->c_str());
        }

        // Insert blank DNS entry
        set<HMIPAddress> addresses;
        uint8_t dualstack = (v4 ? HM_DUALSTACK_IPV4_ONLY : 0) | (v6 ? HM_DUALSTACK_IPV6_ONLY : 0);
        HMDNSLookup dnsHostCheck(checkHeader.m_hostCheck.getDnsType(), checkHeader.m_hostCheck.getRemoteCheck());
        dnsCache.getAddresses(*it, dualstack, dnsHostCheck, addresses);
        if(addresses.size() == 0)
        {
            if(dualstack & HM_DUALSTACK_IPV4_ONLY)
            {
                addresses.insert(blank);
            }
            if(dualstack & HM_DUALSTACK_IPV6_ONLY)
            {
                addresses.insert(blankv6);
            }
        }

        if(!storeDNSResult(*it, addresses))
        {
            HMLog(HM_LOG_ERROR, "Failed to store DNS for %s", it->c_str());
        }
    }

    // Now we need to deal with the overlap
    set<HMCheckHeader> filledChecks;
    for(auto it = both.begin(); it != both.end(); ++it)
    {
        vector<HMCheckHeader> checks;
        if(!getNameChecks(*it, checks))
        {
            HMLog(HM_LOG_ERROR, "Failed to fetch host checks for %s from backend", it->c_str());
            continue;
        }

        // We need to either copy the results from the backend or remove
        for(auto check = checks.begin(); check != checks.end(); ++check)
        {
            bool found = false;

            for(auto localCheck = localChecks.begin(); localCheck != localChecks.end(); ++localCheck)
            {
                if(localCheck->m_hostname == check->m_hostname
                        && localCheck->m_hostCheck == check->m_hostCheck
                        && localCheck->m_checkParams == check->m_checkParams)
                {
                    // The check exists in both local and backend
                    // Copy the data from the backend and remove from the localChecks
                    found = true;

                    HMDataCheckResult result;
                    HMAuxInfo auxInfo;

                    if(getHostCheckResult(*check, result))
                    {
                        checkList.updateCheck(*check, result);
                    }

                    if(getHostAuxInfo(*check, auxInfo))
                    {
                        auxCache.updateAuxInfo(check->m_hostname, check->m_hostCheck.getCheckInfo(), check->m_address, auxInfo);
                    }

                    HMDNSLookup dnsHostCheck(check->m_hostCheck.getDnsType(), check->m_address.getType() == AF_INET6, check->m_hostCheck.getRemoteCheck());
                    pair<string, HMDNSLookup> key = make_pair(check->m_hostname, dnsHostCheck);
                    auto res = dnsMap.insert(make_pair(key, set<HMIPAddress>()));
                    res.first->second.insert(check->m_address);
                    if(dnsResTimeMap.find(key) == dnsResTimeMap.end())
                    {
                        dnsResTimeMap.insert(make_pair(key, result.m_checkTime));
                    }
                    if(check->m_hostCheck.getFlowType() == HM_FLOW_REMOTE_HOST_TYPE)
                    {
                        remoteHostCache.updateResultTime(check->m_hostname, check->m_hostCheck, result.m_checkTime);
                    }
                    filledChecks.insert(*localCheck);
                    break;
                }
            }

            // Check exists in the backend but not local
            if(!found)
            {
                removeHostCheckResult(*check);
                removeHostAuxInfo(*check);
            }
        }
    }

    // Backfill the DNS cache
    for(auto it = dnsMap.begin(); it != dnsMap.end(); ++it)
    {
        HMDNSResult v4Result;
        HMDNSResult v6Result;
        auto res = dnsResTimeMap.find(it->first);
        if (res != dnsResTimeMap.end())
        {
            v4Result.setResultTime(res->second);
        }
        res = dnsResTimeMap.find(it->first);
        if (res != dnsResTimeMap.end())
        {
            v6Result.setResultTime(res->second);
        }
        dnsCache.updateReloadDNSEntry(it->first.first, it->second, v4Result, v6Result, res->first.second);
    }

    // Fill missing entries in the backend
    for(auto check = localChecks.begin(); check != localChecks.end(); ++check)
    {
        if(filledChecks.find(*check) == filledChecks.end())
        {
            // No backend entry for this
            vector<HMCheckHeader> headers;
            getNameChecks(check->m_hostname, headers);
            headers.push_back(*check);
            storeNameChecks(check->m_hostname, headers);

            HMCheckData cd(check->m_hostname, check->m_address, check->m_hostCheck, check->m_checkParams, HMDataCheckResult());
            HMAuxInfo empty;
            HMAuxData ca(check->m_hostname, check->m_address, check->m_hostCheck, check->m_checkParams, empty);

            if(!storeHostCheckResult(cd))
            {
                HMLog(HM_LOG_ERROR, "Failed to store check entry for %s", check->m_hostname.c_str());
            }

            if(!storeHostAuxInfo(ca))
            {
                HMLog(HM_LOG_ERROR, "Failed to store aux entry for %s", check->m_hostname.c_str());
            }

            set<HMIPAddress> addresses;
            getDNSResult(check->m_hostname, addresses);

            bool v4found = false;
            bool v6found = false;

            for(auto address = addresses.begin(); address != addresses.end(); ++address)
            {
                v4found = (address->getType() == AF_INET) ? true : v4found;
                v6found = (address->getType() == AF_INET6) ? true : v6found;
            }

            uint8_t dualstack = check->m_hostCheck.getDualStack();
            if((dualstack & HM_DUALSTACK_IPV4_ONLY) && !v4found)
            {
                addresses.insert(blank);
            }
            if((dualstack & HM_DUALSTACK_IPV6_ONLY) && !v6found)
            {
                addresses.insert(blankv6);
            }
            storeDNSResult(check->m_hostname, addresses);
        }
    }

    // Commit the hostNames
    set<string> hostNames;

    for(auto it = both.begin(); it != both.end(); ++it)
    {
        hostNames.insert(*it);
    }

    for(auto it = localNames.begin(); it != localNames.end(); ++it)
    {
        hostNames.insert(*it);
    }

    storeHostNames(hostNames);
}

bool
HMStorageHost::storeConfigs(HMState& checkState)
{
    // Store the hostnames, the hostchecks and the host group information
    vector<HMCheckHeader> allChecks;
    set<string> allHosts;

    checkState.m_checkList.getAllChecks(allChecks);
    for(auto it = allChecks.begin(); it != allChecks.end(); ++it)
    {
        allHosts.insert(it->m_hostname);
    }

    if(!storeHostNames(allHosts))
    {
        return false;
    }

    // Now on the second pass, we store all the hostchecks
    for(auto it = allHosts.begin(); it != allHosts.end(); ++it)
    {
        vector<HMCheckHeader> checks;
        for(auto iit = allChecks.begin(); iit != allChecks.end(); ++iit)
        {
            if(iit->m_hostname == *it)
            {
                checks.push_back(*iit);
            }
        }
        if(!storeNameChecks(*it, checks))
        {
            return false;
        }
    }

    // Now store the host group info
    set<string> hostGroupNames;
    for(auto it = checkState.m_hostGroups.begin(); it != checkState.m_hostGroups.end(); ++it)
    {
        hostGroupNames.insert(it->first);
        if(!storeGroupInfo(it->first, it->second))
        {
            return false;
        }
    }

    return storeHostGroupNames(hostGroupNames);
}

bool
HMStorageHost::getConfigs(HMState& checkState)
{
    set<string> hostNames;
    string hostGroup = "";
    set<HMIPAddress> ip;

    set<string> hostGroupNames;
    if(!getHostGroupNames(hostGroupNames))
    {
        return false;
    }

    for(auto it = hostGroupNames.begin(); it != hostGroupNames.end(); ++it)
    {
        HMDataHostGroup hostGroup(*it);
        if(!getGroupInfo(*it, hostGroup))
        {
            return false;
        }
        checkState.m_hostGroups.insert(make_pair(*it, hostGroup));
        checkState.m_checkList.addHostGroup(hostGroup);
    }

    if(!getHostNames(hostNames))
    {
        return false;
    }

    for(auto it = hostNames.begin(); it != hostNames.end(); ++it)
    {
        vector<HMCheckHeader> checks;
        if(!getNameChecks(*it, checks))
        {
            return false;
        }

        for(auto check = checks.begin(); check != checks.end(); ++check)
        {
            checkState.m_checkList.insertCheck(hostGroup, check->m_hostname, check->m_hostCheck, check->m_checkParams, ip);
        }
    }

    return true;
}

bool
HMStorageHost::storeCheckResult(const string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams,
        const HMDataCheckResult& checkResult)
{
    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to store result in read only mode");
        return false;
    }

    m_checkQueueSpinLock.lock();
    m_storeCheckQueue.push(HMCheckData(hostname, address, hostCheck, checkParams, checkResult));
    m_checkQueueSpinLock.unlock();

    lock_guard<mutex> lk(m_dataReadyMutex);
    m_dataReadyCond.notify_one();
    return true;
}

bool
HMStorageHost::getCheckResult(const string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams,
        HMDataCheckResult& checkResult)
{
    HMCheckHeader header(hostname, address, hostCheck, checkParams);
    return getHostCheckResult(header, checkResult);
}

bool
HMStorageHost::purgeCheckResult(const string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams)
{
    HMCheckHeader header(hostname, address, hostCheck, checkParams);
    return removeHostCheckResult(header);
}

bool
HMStorageHost::storeAuxInfo(const string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams,
        const HMAuxInfo& auxInfo)
{
    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to store aux result in read only mode");
        return false;
    }

    m_auxQueueSpinLock.lock();
    m_storeAuxQueue.push(move(HMAuxData(hostname, address, hostCheck, checkParams, auxInfo)));
    m_auxQueueSpinLock.unlock();

    lock_guard<mutex> lk(m_dataReadyMutex);
    m_dataReadyCond.notify_one();
    return true;
}

bool
HMStorageHost::getAuxInfo(const string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams,
        HMAuxInfo& auxInfo)
{
    HMCheckHeader header(hostname, address, hostCheck, checkParams);
    return getHostAuxInfo(header, auxInfo);
}

bool
HMStorageHost::purgeAuxInfo(const string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams)
{
    HMCheckHeader header(hostname, address, hostCheck, checkParams);
    return removeHostAuxInfo(header);
}

bool
HMStorageHost::getDNS(const std::string& hostname, std::set<HMIPAddress>& ips)
{
    return getDNSResult(hostname, ips);
}

bool
HMStorageHost::updateCheckResultCache(HMCheckHeader& header, HMDataCheckResult& result)
{
    (void)header;
    (void)result;
    return true;
}

bool
HMStorageHost::updateAuxInfoCache(HMCheckHeader& header, HMAuxInfo& aux)
{
    (void)header;
    (void)aux;
    return true;
}

void
HMStorageHost::updateHostGroups(set<string>& hostGroups)
{
    (void)hostGroups;
    return;
}

bool
HMStorageHost::storeHostGroupCheckResult(const std::string& hostgroupname,
        std::vector<HMGroupCheckResult>& checkResult)
{
    (void)hostgroupname;
    (void)checkResult;
    return true;
}

bool
HMStorageHost::storeHostGroupAuxResult(const std::string& hostgroupname,
        std::vector<HMGroupAuxResult>& auxResult)
{
    (void)hostgroupname;
    (void)auxResult;
    return true;
}

bool
HMStorageHost::getGroupCheckResults(const string& groupName,
        bool noCache,
        bool onlyResolved,
        vector<HMGroupCheckResult>& results)
{
    (void) noCache;
    (void) onlyResolved;
    return getGroupCheckResults(groupName, results);
}

bool
HMStorageHost::getGroupCheckResults(const std::string& groupName,
            std::vector<HMGroupCheckResult>& results)
{
    // assume we have the group info in the group map
    HMCheckHeader header;
    HMDataCheckResult result;

    auto group = m_hostGroupMap->find(groupName);
    if(group == m_hostGroupMap->end())
    {
        HMLog(HM_LOG_ERROR, "Host group not found during check results lookup");
        return false;
    }

    group->second.getHostCheck(header.m_hostCheck);
    group->second.getCheckParameters(header.m_checkParams);

    // now we need the list of hosts
    auto hosts = group->second.getHostList();

    for(auto it = hosts->begin(); it != hosts->end(); ++it)
    {
        set<HMIPAddress> ips;
        if(!getDNSResult(*it, ips))
        {
            continue;
        }

        for(auto ip = ips.begin(); ip != ips.end(); ++ip)
        {
            header.m_hostname = *it;
            header.m_address = *ip;
            if(getHostCheckResult(header, result))
            {
                results.push_back(HMGroupCheckResult(header.m_hostname, header.m_address, result));
            }
        }
    }
    return true;
}

bool
HMStorageHost::getGroupAuxInfo(const string& groupName,
        bool noCache,
        bool onlyResolved,
        vector<HMGroupAuxResult>& results)
{
    (void) noCache;
    (void) onlyResolved;
    // assume we have the group info in the group map
    HMCheckHeader header;
    HMAuxInfo result;

    auto group = m_hostGroupMap->find(groupName);
    if(group == m_hostGroupMap->end())
    {
        HMLog(HM_LOG_ERROR, "Host group not found during check results lookup");
        return false;
    }

    group->second.getHostCheck(header.m_hostCheck);
    group->second.getCheckParameters(header.m_checkParams);

    // now we need the list of hosts
    auto hosts = group->second.getHostList();

    for(auto it = hosts->begin(); it != hosts->end(); ++it)
    {
        set<HMIPAddress> ips;
        if(!getDNSResult(*it, ips))
        {
            continue;
        }

        for(auto ip = ips.begin(); ip != ips.end(); ++ip)
        {
            header.m_hostname = *it;
            header.m_address = *ip;
            if(getHostAuxInfo(header, result))
            {
                results.push_back(HMGroupAuxResult(header.m_hostname, header.m_address, result));
            }
        }
    }
    return true;
}

void
HMStorageHost::removeName(const string& hostname)
{
    vector<HMCheckHeader> checks;
    set<HMIPAddress> addresses;

    if(!getNameChecks(hostname, checks))
    {
        HMLog(HM_LOG_ERROR, "Failed to fetch host checks for %s from backend", hostname.c_str());
    }
    else
    {
        if(!removeNameChecks(hostname, checks))
        {
            HMLog(HM_LOG_ERROR, "Failed to remove host checks for %s from backend", hostname.c_str());
        }

        for(auto check = checks.begin(); check != checks.end(); ++check)
        {
            if(!removeHostCheckResult(*check))
            {
                HMLog(HM_LOG_ERROR, "Failed to remove host check %s for %s from backend", check->m_hostCheck.getCheckInfo().c_str(), hostname.c_str());
            }

            if(!removeHostAuxInfo(*check))
            {
                HMLog(HM_LOG_ERROR, "Failed to aux info %s for %s from backend", check->m_hostCheck.getCheckInfo().c_str(), hostname.c_str());
            }
        }
    }

    if(!getDNSResult(hostname, addresses))
    {
        HMLog(HM_LOG_ERROR, "Failed to fetch DNS for %s from backend", hostname.c_str());
    }

    if(!removeDNSResult(hostname, addresses))
    {
        HMLog(HM_LOG_ERROR, "Failed to remove DNS for %s from backend", hostname.c_str());
    }
}

bool
HMStorageHost::commitHealthCheck()
{
    m_checkQueueSpinLock.lock();
    if(m_storeCheckQueue.size() == 0)
    {
        m_checkQueueSpinLock.unlock();
        return false;
    }

    HMCheckData data = m_storeCheckQueue.front();
    m_storeCheckQueue.pop();
    m_checkQueueSpinLock.unlock();

    storeHostCheckResult(data);

    set<HMIPAddress> addresses;
    if(getDNSResult(data.m_hostname, addresses))
    {
        addresses.insert(data.m_address);
    }
    storeDNSResult(data.m_hostname, addresses);

    return true;
}

bool
HMStorageHost::commitAuxInfo()
{
    m_auxQueueSpinLock.lock();
    if(m_storeAuxQueue.size() == 0)
    {
        m_auxQueueSpinLock.unlock();
        return false;
    }

    HMAuxData data = move(m_storeAuxQueue.front());
    m_storeAuxQueue.pop();
    m_auxQueueSpinLock.unlock();

    storeHostAuxInfo(data);
    set<HMIPAddress> addresses;
    if(getDNSResult(data.m_hostname, addresses))
    {
        addresses.insert(data.m_address);
    }

    storeDNSResult(data.m_hostname, addresses);

    return true;
}



