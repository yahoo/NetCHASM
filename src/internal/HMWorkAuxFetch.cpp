// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMWorkAuxFetch.h"
#include "HMEventLoopQueue.h"
#include "HMState.h"
#include "HMConstants.h"
#include "HMWork.h"
#include "HMLogBase.h"
#include "HMStorage.h"

using namespace std;

HM_WORK_STATUS HMWorkAuxFetch::processWork()
{
    // tell the code we are starting a check
    HM_WORK_STATUS result;
    HMLog(HM_LOG_DEBUG, "[WORKER] [%llu] Starting %s Aux Health Check %s for %s at %s",
            m_ID,
            printCheckType(m_hostCheck.getCheckType()).c_str(),
            m_hostCheck.getCheckInfo().c_str(),
            m_hostname.c_str(),
            m_ipAddress.toString().c_str());

    // Update the smart pointer if necessary
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);

    // now conduct the check.
    m_timeout = currentState->m_checkList.startCheck(m_hostname, m_ipAddress, m_hostCheck);

    result = fetchAux();

    // process the results
    // Update the smart pointer if necessary
    m_stateManager->updateState(currentState);

    currentState->m_checkList.updateCheck(this, this->m_hostCheck);
    currentState->m_checkList.storeCheck(this, this->m_hostCheck, this->m_ipAddress, currentState->m_datastore.get());
    currentState->m_checkList.storeAux(this, this->m_hostCheck, this->m_ipAddress, this->m_auxData, currentState->m_datastore.get(), currentState->m_auxCache, this->getAuxDataType());

    // check to see if this check is complete
    HMTimeStamp checkTime = currentState->m_checkList.nextCheckTime(m_hostname, m_ipAddress, m_hostCheck);
    if(checkTime <= HMTimeStamp::now())
    {
        currentState->m_checkList.queueCheck(m_hostname, m_ipAddress, m_hostCheck, m_stateManager->m_workQueue);
    }
    else
    {
        m_eventLoop->addHealthCheckTimeout(m_hostname, m_ipAddress, m_hostCheck, checkTime);
    }

    return result;
}
