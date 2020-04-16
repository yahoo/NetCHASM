// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include "HMWorkRemoteHostCheck.h"
#include "HMEventLoopQueue.h"
#include "HMWork.h"
#include "HMLogBase.h"
#include "HMStateManager.h"
#include "HMRemoteHostCache.h"

using namespace std;

HM_WORK_STATUS HMWorkRemoteHostCheck::processWork()
{
    HMLog(HM_LOG_DEBUG3, "[WORKER] [%llu] Remote hostgroup fetch for %s", m_ID, m_hostname.c_str());

    // Update the smart pointer
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);

    if(!currentState->m_remoteHostCache.startRemoteCheck(m_hostname, m_hostCheck))
    {
        return HM_WORK_COMPLETE;
    }

    m_workStatus = remoteLookup();
    currentState->m_remoteHostCache.finishCheck(m_hostname, m_hostCheck, true);

    if(m_workStatus == HM_WORK_COMPLETE_REMOTE )
    {
        updateResults();
        updateAuxResults();
    }
    else if(m_workStatus == HM_WORK_REMOTE_FALLBACK
            && !(m_hostCheck.getDistributedFallBack()
                    & HM_DISTRIBUTED_FALLBACK_REMOTE))
    {
        updateResultsNoData();
    }
    // Update the smart pointer if necessary
    m_stateManager->updateState(currentState);

    // Make sure we completed the query with success and don't need another
    HMTimeStamp checkTime = currentState->m_remoteHostCache.nextCheckTime(
            m_hostname, m_hostCheck);
    if (checkTime <= HMTimeStamp::now())
    {
        currentState->m_remoteHostCache.queueRemoteCheck(m_hostname, m_hostCheck,
                m_stateManager->m_workQueue);
    }
    else
    {
        m_eventLoop->addRemoteHostTimeout(m_hostname, m_hostCheck, checkTime);
    }
    if (m_workStatus == HM_WORK_REMOTE_FALLBACK
            && (m_hostCheck.getDistributedFallBack()
                    & HM_DISTRIBUTED_FALLBACK_REMOTE))
    {
        if (m_hostCheck.getDualStack() & HM_DUALSTACK_IPV4_ONLY)
        {
            HMDNSLookup lookup(m_hostCheck.getDnsType(), false,
                    m_hostCheck.getRemoteCheck());
            lookup.setPlugin(currentState->getDNSPlugin(m_hostCheck.getDnsType()));
            currentState->m_dnsCache.queueDNSQuery(m_hostname, lookup,
                    m_stateManager->m_workQueue);
        }
        if (m_hostCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
        {
            HMDNSLookup lookup(m_hostCheck.getDnsType(), true,
                    m_hostCheck.getRemoteCheck());
            lookup.setPlugin(currentState->getDNSPlugin(m_hostCheck.getDnsType()));
            currentState->m_dnsCache.queueDNSQuery(m_hostname, lookup,
                    m_stateManager->m_workQueue);
        }
    }
    return m_workStatus;
}


bool
HMWorkRemoteHostCheck::updateResults()
{
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);
    set<HMIPAddress> v4addresses, v6addresses;
    for(const auto& result : m_hostResults)
    {
        if (result.second.m_address.getType() == AF_INET6)
        {
            v6addresses.insert(result.second.m_address);
        }
        else if(result.second.m_address.getType() == AF_INET)
        {
            v4addresses.insert(result.second.m_address);
        }
    }

    currentState->m_checkList.updateCheck(this, m_hostResults);
    for(const auto& address : v4addresses)
    {
            currentState->m_checkList.storeCheck(this, m_hostCheck, address, currentState->m_datastore.get());
    }
    for(const auto& address : v6addresses)
    {
            currentState->m_checkList.storeCheck(this, m_hostCheck, address, currentState->m_datastore.get());
    }

    if(v4addresses.size() > 0)
    {
        set<HMIPAddress> expiredIps;
        set<HMIPAddress> addresses;
        currentState->m_checkList.getCheckResultsAddress(m_hostname, m_hostCheck, HM_DUALSTACK_IPV4_ONLY, addresses);
        set_difference(addresses.begin(), addresses.end(),
                v4addresses.begin(), v4addresses.end(),
                inserter(expiredIps, expiredIps.begin()));
        for (auto iit = expiredIps.begin(); iit != expiredIps.end(); ++iit)
        {
            HMLog(HM_LOG_DEBUG3, "[WORKER] Deleting checklist result for host %s with IP %s with DataHostCheck=%s", m_hostname.c_str(), (*iit).toString().c_str(), m_hostCheck.printEntry('|', false).c_str());
            currentState->m_checkList.invalidateCheck(m_hostname, *iit, m_hostCheck, currentState->m_datastore.get());
        }
    }

    if(v6addresses.size() > 0)
    {
        set<HMIPAddress> expiredIps;
        set<HMIPAddress> addresses;
        currentState->m_checkList.getCheckResultsAddress(m_hostname, m_hostCheck, HM_DUALSTACK_IPV6_ONLY, addresses);
        set_difference(addresses.begin(), addresses.end(),
                v6addresses.begin(), v6addresses.end(),
                inserter(expiredIps, expiredIps.begin()));
        for (auto iit = expiredIps.begin(); iit != expiredIps.end(); ++iit)
        {
            HMLog(HM_LOG_DEBUG3, "[WORKER] Deleting checklist result for host %s with IP %s with DataHostCheck=%s", m_hostname.c_str(), (*iit).toString().c_str(), m_hostCheck.printEntry('|', false).c_str());
            currentState->m_checkList.invalidateCheck(m_hostname, *iit, m_hostCheck, currentState->m_datastore.get());
        }
    }
    return true;
}

bool
HMWorkRemoteHostCheck::updateResultsNoData()
{
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);
    set<HMIPAddress> ips;
    currentState->m_checkList.getCheckResultsAddress(m_hostname, m_hostCheck, m_hostCheck.getDualStack(), ips);
    for(const HMIPAddress& address: ips)
    {
        vector<pair<HMDataCheckParams, HMDataCheckResult>> results;
        if (currentState->m_checkList.getCheckResults(m_hostname, m_hostCheck, address, results))
        {
            for(auto& it : results)
            {
                HMCheckHeader checkHeader(m_hostname, address, m_hostCheck,
                        it.first);
                it.second.m_checkTime = HMTimeStamp::now();
                it.second.m_reason = HM_REASON_REMOTE_NODATA;
                currentState->m_checkList.updateCheck(checkHeader, it.second);
                currentState->m_checkList.storeCheck(this,
                        m_hostCheck, address, currentState->m_datastore.get());
             }
        }
    }
    return true;
}


bool
HMWorkRemoteHostCheck::updateAuxResults()
{
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);

    for(auto result : m_auxResults)
    {
        currentState->m_checkList.storeAux(this, m_hostCheck, result.m_address, result.m_info, currentState->m_datastore.get(), currentState->m_auxCache);
    }
    return true;
}

