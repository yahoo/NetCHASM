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
        // tell the code we are starting a check
        HMLog(HM_LOG_DEBUG3, "[WORKER] Starting %s Health Check %s for %s at %s",
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

    if(m_workStatus == HM_WORK_COMPLETE)
    {
        // process the results
        // Update the smart pointer if necessary
        m_stateManager->updateState(currentState);

        // check to see if this check is complete
        currentState->m_checkList.updateCheck(this, this->m_hostCheck);
        currentState->m_checkList.storeCheck(this, this->m_hostCheck, this->m_ipAddress, currentState->m_datastore.get());

        HMTimeStamp checkTime = currentState->m_checkList.nextCheckTime(m_hostname, m_ipAddress, m_hostCheck);
        bool isValidAddress = currentState->m_dnsCache.isValidAddress(m_hostname,m_hostCheck.getDualStack(),m_ipAddress);
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
   return m_workStatus;
}
