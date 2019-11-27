// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMWorkHealthCheck.h"
#include "HMEventLoopQueue.h"
#include "HMState.h"
#include "HMConstants.h"
#include "HMWork.h"
#include "HMLogBase.h"

using namespace std;

HM_WORK_STATUS
HMWorkHealthCheck::processWork()
{
    shared_ptr<HMState> currentState;

    if(m_workStatus == HM_WORK_IDLE)
    {
        string isRemote = m_hostCheck.getRemoteCheck().empty() ? "" : "remote";
        // tell the code we are starting a check
        HMLog(HM_LOG_DEBUG3, "[WORKER] Starting %s %s Health Check %s for %s at %s",
                isRemote.c_str(),
                printCheckType(m_hostCheck.getCheckType()).c_str(),
                m_hostCheck.getCheckInfo().c_str(),
                m_hostname.c_str(),
                m_ipAddress.toString().c_str());

        // Update the smart pointer if necessary

        m_stateManager->updateState(currentState);

        // now conduct the check.
        m_timeout = currentState->m_checkList.startCheck(m_hostname, m_ipAddress, m_hostCheck);

        m_workStatus = healthCheck();
    }

    if (m_workStatus == HM_WORK_COMPLETE
            || m_workStatus == HM_WORK_COMPLETE_REMOTE
            || (m_workStatus == HM_WORK_REMOTE_FALLBACK
                    && !(m_hostCheck.getDistributedFallBack() & HM_DISTRIBUTED_FALLBACK_REMOTE)))
    {
        // process the results
        // Update the smart pointer if necessary
        m_stateManager->updateState(currentState);

        // check to see if this check is complete
        if (m_hostCheck.getRemoteCheckType() != HM_REMOTE_CHECK_NONE
                && (m_workStatus == HM_WORK_COMPLETE_REMOTE
                        || (m_workStatus == HM_WORK_REMOTE_FALLBACK
                                && !(m_hostCheck.getDistributedFallBack() & HM_DISTRIBUTED_FALLBACK_REMOTE))))
        {
            currentState->m_checkList.updateCheck(this, this->m_hostResults);
            if(this->m_hostCheck.getCheckType() == HM_CHECK_AUX_HTTP || this->m_hostCheck.getCheckType() == HM_CHECK_AUX_HTTPS)
            {
                currentState->m_checkList.storeAux(this, this->m_hostCheck, this->m_ipAddress, this->m_auxInfo, currentState->m_datastore.get(), currentState->m_auxCache);
            }
        }
        else
        {
            currentState->m_checkList.updateCheck(this, this->m_hostCheck);
        }
        if (m_publish)
        {
            currentState->m_checkList.publishCheck(this, this->m_hostCheck, this->m_ipAddress, currentState->m_resultPublisher);
        }
        if (m_storeResults)
        {
            currentState->m_checkList.storeCheck(this, this->m_hostCheck, this->m_ipAddress, currentState->m_datastore.get());
        }
        if (m_reschedule)
        {
            HMTimeStamp checkTime = currentState->m_checkList.nextCheckTime(m_hostname, m_ipAddress, m_hostCheck);
            HMDNSLookup dnsHostCheck(m_hostCheck.getDnsPlugin(), m_ipAddress.getType() == AF_INET6);
            bool isValidAddress = currentState->m_dnsCache.isValidAddress(m_hostname, m_hostCheck.getDualStack(), dnsHostCheck, m_ipAddress);
            if (isValidAddress)
            {
                if (checkTime <= HMTimeStamp::now())
                {
                    currentState->m_checkList.queueCheck(m_hostname, m_ipAddress, m_hostCheck, m_stateManager->m_workQueue);
                }
                else
                {
                    m_eventLoop->addHealthCheckTimeout(m_hostname, m_ipAddress, m_hostCheck, checkTime);
                }
            }
        }
    }
    if (m_workStatus == HM_WORK_REMOTE_FALLBACK
            && (m_hostCheck.getDistributedFallBack() & HM_DISTRIBUTED_FALLBACK_REMOTE))
    {
        // Update the smart pointer if necessary
        m_stateManager->updateState(currentState);
        if (!m_hostCheck.getRemoteCheck().empty())
        {
            currentState->m_checkList.queueCheck(m_hostname, m_ipAddress,
                    m_hostCheck, m_stateManager->m_workQueue, false);
        }
    }
   return m_workStatus;
}
