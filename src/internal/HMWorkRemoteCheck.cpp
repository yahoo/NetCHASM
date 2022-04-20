// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMWorkRemoteCheck.h"
#include "HMEventLoopQueue.h"
#include "HMWork.h"
#include "HMLogBase.h"
#include "HMStateManager.h"
#include "HMRemoteHostGroupCache.h"

using namespace std;

HM_WORK_STATUS HMWorkRemoteCheck::processWork()
{
    HMLog(HM_LOG_DEBUG3, "[WORKER] [%llu] Remote hostgroup fetch for %s", m_ID, m_hostname.c_str());

    // Update the smart pointer
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);

    if(!currentState->m_remoteCache.startRemoteCheck(m_hostname))
    {
        return HM_WORK_COMPLETE;
    }

    m_workStatus = remoteLookup();
    currentState->m_remoteCache.finishCheck(m_hostname, true);

    if(m_workStatus == HM_WORK_COMPLETE_REMOTE)
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
    HMTimeStamp checkTime = currentState->m_remoteCache.nextCheckTime(
            m_hostname);
    if (checkTime <= HMTimeStamp::now())
    {
        currentState->m_remoteCache.queueRemoteCheck(m_hostname,
                m_stateManager->m_workQueue, currentState->m_hostGroups);
    }
    else
    {
        m_eventLoop->addRemoteTimeout(m_hostname, checkTime);
    }
    if (m_workStatus == HM_WORK_REMOTE_FALLBACK
            && (m_hostCheck.getDistributedFallBack()
                    & HM_DISTRIBUTED_FALLBACK_REMOTE))
    {
        auto it = currentState->m_hostGroups.find(m_hostname);
        if (it == currentState->m_hostGroups.end())
        {
            HMLog(HM_LOG_ERROR,
                    "[WORKER] Host group missing for remote fallback rescheduling  %s",
                    m_hostname.c_str());
            return HM_WORK_COMPLETE;
        }
        const vector<string>* hosts = it->second.getHostList();
        for(uint32_t i = 0; i < hosts->size(); i++)
        {
            if (m_hostCheck.getDualStack() & HM_DUALSTACK_IPV4_ONLY)
            {
                HMDNSLookup lookup(m_hostCheck.getDnsType(), false,
                        m_hostCheck.getRemoteCheck());
                lookup.setPlugin(currentState->getDNSPlugin(m_hostCheck.getDnsType()));
                currentState->m_dnsCache.queueDNSQuery((*hosts)[i], lookup,
                        m_stateManager->m_workQueue, currentState->getStateVersion());
            }
            if (m_hostCheck.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
            {
                HMDNSLookup lookup(m_hostCheck.getDnsType(), true,
                        m_hostCheck.getRemoteCheck());
                lookup.setPlugin(currentState->getDNSPlugin(m_hostCheck.getDnsType()));
                currentState->m_dnsCache.queueDNSQuery((*hosts)[i], lookup,
                        m_stateManager->m_workQueue, currentState->getStateVersion());
            }
        }
    }
    return m_workStatus;
}


bool
HMWorkRemoteCheck::updateResults()
{
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);
    for(auto& result : m_results)
    {
        HMCheckHeader checkHeader(result.m_hostName, result.m_address, m_hostCheck, m_dataCheckParams);
        result.m_result.m_remoteCheckTime = result.m_result.m_checkTime;
        result.m_result.m_checkTime = HMTimeStamp::now();
    }
    currentState->m_datastore->storeHostGroupCheckResult(m_hostname, m_results);
    m_results.clear();
    return true;
}

bool
HMWorkRemoteCheck::updateResultsNoData()
{
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);
    currentState->m_datastore->getGroupCheckResults(m_hostname, m_results);
    for(auto& result : m_results)
    {
        if (result.m_result.m_response != HM_RESPONSE_DNS_FAILED
                && result.m_result.m_response != HM_RESPONSE_NONE)
        {
            HMCheckHeader checkHeader(result.m_hostName, result.m_address,
                    m_hostCheck, m_dataCheckParams);
            result.m_result.m_reason = HM_REASON_REMOTE_NODATA;
            result.m_result.m_checkTime = HMTimeStamp::now();
        }
    }
    currentState->m_datastore->storeHostGroupCheckResult(m_hostname, m_results);
    m_results.clear();
    return true;
}

bool
HMWorkRemoteCheck::updateAuxResults()
{
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);
    for(auto& it : m_auxResults)
    {
        currentState->m_auxCache.storeAuxInfo(it.m_hostName, m_hostCheck.getCheckInfo(), it.m_address, it.m_info);
    }
    currentState->m_datastore->storeHostGroupAuxResult(m_hostname, m_auxResults);
    m_auxResults.clear();
    return true;
}

