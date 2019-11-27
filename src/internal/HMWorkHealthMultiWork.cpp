// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "curl/curl.h"

#include "HMWorkMarkFetchCurl.h"
#include "HMWorkHealthMultiWork.h"
#include "HMWork.h"
#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMLogBase.h"
#include "HMStateManager.h"

// LCOV_EXCL_START; Tested in functional testing

using namespace std;

HM_WORK_STATUS
HMWorkHealhMultiWork::healthCheck()
{
    unique_ptr<HMWork> healthCheck;
    set<int> values;
    HMLog(HM_LOG_DEBUG,
            "[MultiQueueCheck] Preparing to schedule child Curl Mark checks for host %s ip %s",
            m_hostname.c_str(), m_ipAddress.toString().c_str());
    m_stateManager->m_hostMark.getSocketOptionValues(m_hostname, m_ipAddress,
            m_hostCheck, values);
    for (int value : values)
    {
        HMLog(HM_LOG_DEBUG,
                "[MultiQueueCheck] Scheduling Curl Mark check check for host %s ip %s with mark %d",
                m_hostname.c_str(), m_ipAddress.toString().c_str(), value);
        healthCheck = make_unique<HMWorkMarkFetchCurl>(
                HMWorkMarkFetchCurl(m_hostname, m_ipAddress, m_hostCheck));
        healthCheck->setMark(value);
        healthCheck->setReschedule(false);
        healthCheck->setStoreResults(false);
        m_stateManager->m_workQueue.insertWork(healthCheck);
    }
    return HM_WORK_COMPLETE;
}
// LCOV_EXCL_STOP; Tested in functional testing
