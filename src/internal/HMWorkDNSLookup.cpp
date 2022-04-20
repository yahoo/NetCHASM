// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMWorkDNSLookup.h"
#include "HMEventLoopQueue.h"
#include "HMWork.h"
#include "HMLogBase.h"
#include "HMStateManager.h"
#include "HMDNSCache.h"

using namespace std;

HM_WORK_STATUS HMWorkDNSLookup::processWork()
{
    bool result;
    HMLog(HM_LOG_DEBUG3, "[WORKER] [%llu] Resolving DNS for %s", m_ID, m_hostname.c_str());

    // Update the smart pointer
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);

    if(!currentState->m_dnsCache.startDNSQuery(m_hostname, m_dnsHostCheck, getStateVersion()))
    {
        return HM_WORK_COMPLETE;
    }

    result = dnsLookup();

    // Update the smart pointer if necessary
    m_stateManager->updateState(currentState);

    // Make sure we completed the query with success and don't need another
    HMTimeStamp checkTime = currentState->m_dnsCache.nextQueryTime(m_hostname, m_dnsHostCheck, currentState->getStateVersion());
    if (m_reschedule || m_response == HM_RESPONSE_DNS_FAILED)
    {
        if (checkTime <= HMTimeStamp::now())
        {
            currentState->m_dnsCache.queueDNSQuery(m_hostname, m_dnsHostCheck,
                    m_stateManager->m_workQueue, getStateVersion());
        }
        else
        {
            m_eventLoop->addDNSTimeout(m_hostname, m_dnsHostCheck, checkTime, getStateVersion());
        }
    }
    if(!result)
    {
        return HM_WORK_COMPLETE;
    }

    // Now kick all the health checks waiting for this DNS lookup
    set<HMIPAddress> ips;
    set<HMIPAddress> expiredIps;
    auto range = currentState->m_dnsWaitList.equal_range(HMDNSTypeMap(m_hostname, m_dnsHostCheck.getType(), m_dnsHostCheck.getRemoteCheckGroup()));
    for(auto it = range.first; it != range.second; ++it)
    {
        uint8_t dualStack;
        ips.clear();
        expiredIps.clear();
        HMDataHostCheck check = it->second;
        if (m_ipAddress.getType() == AF_INET)
        {
            dualStack = HM_DUALSTACK_IPV4_ONLY;
            if(!(check.getDualStack() & HM_DUALSTACK_IPV4_ONLY))
            {
                continue;
            }
        }
        else if (m_ipAddress.getType() == AF_INET6)
        {
            dualStack = HM_DUALSTACK_IPV6_ONLY;
            if(!(check.getDualStack() & HM_DUALSTACK_IPV6_ONLY))
            {
                continue;
            }
        }

        currentState->m_dnsCache.getAddresses(m_hostname, dualStack, m_dnsHostCheck, ips);
        bool status = currentState->m_dnsCache.getExpiredAddresses(m_hostname, dualStack, m_dnsHostCheck, expiredIps);
        if(status)
        {
            for (auto iit = expiredIps.begin(); iit != expiredIps.end(); ++iit)
            {
                HMLog(HM_LOG_NOTICE, "[WORKER] Deleting DNS Entry for host %s with IP %s", m_hostname.c_str(), (*iit).toString().c_str());
                currentState->m_checkList.invalidateCheck(m_hostname, *iit, check,currentState->m_datastore.get());
            }
        }
        HMLog(HM_LOG_DEBUG, "[WORKER] [%llu] IP List retrieved total size %d", m_ID, ips.size());
        for(auto iit = ips.begin(); iit != ips.end(); ++iit)
        {
            if (iit->toString() == "0.0.0.0" || iit->toString() == "::")
            {
                if(*iit == this->m_ipAddress)
                {
                    currentState->m_checkList.insertEmptyQuery(this, check, *iit);
                    currentState->m_checkList.storeCheck(this, check, *iit, currentState->m_datastore.get());
                }
                continue;
            }
            HMLog(HM_LOG_DEBUG3, "[WORKER] [%llu] IP %s", m_ID, iit->toString().c_str());
            checkTime = currentState->m_checkList.nextCheckTime(m_hostname, *iit, check, currentState->getStateVersion());
            HMLog(HM_LOG_DEBUG3, "[WORKER] [%llu] Next check time %llu for %s at %s", m_ID,
                    checkTime.getTimeSinceEpoch(),
                    m_hostname.c_str(),
                    iit->toString().c_str());
            if(checkTime <= HMTimeStamp::now())
            {
                currentState->m_checkList.queueCheck(m_hostname, *iit, check, m_stateManager->m_workQueue, getStateVersion());
            }
            else{
               // We will hit this path after a reload for hosts
               // that were already being probed if we change the probe type.
               m_eventLoop->addHealthCheckTimeout(m_hostname, *iit, check, checkTime, getStateVersion());
            }
        }
    }

    return HM_WORK_COMPLETE;
}


