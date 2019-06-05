// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMWorkHealthCheckNone.h"
#include "HMWork.h"
#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMLogBase.h"

// LCOV_EXCL_START; Tested in functional testing

using namespace std;

HM_WORK_STATUS
HMWorkHealthCheckNone::healthCheck()
{
    if (m_hostCheck.getCheckType() == HM_CHECK_NONE)
    {

        HMLog(HM_LOG_DEBUG3, "[NONECHECK]  Checking %s at %s for check type  None ",
                 m_hostname.c_str(),
                 m_ipAddress.toString().c_str());

        m_start = HMTimeStamp::now();
        m_response = HM_RESPONSE_CONNECTED;
        m_reason = HM_REASON_SUCCESS;
        m_end= HMTimeStamp::now();
        return HM_WORK_COMPLETE;
    }
    else
    {
        HMLog(HM_LOG_ERROR, "[NONECHECK] Invalid Check type %d accepts only check type - none",
                printCheckType(m_hostCheck.getCheckType()).c_str());

        return HM_WORK_COMPLETE;
    }
}
// LCOV_EXCL_STOP; Tested in functional testing
