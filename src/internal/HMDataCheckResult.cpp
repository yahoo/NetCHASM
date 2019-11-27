// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMDataCheckResult.h"
#include "HMConstants.h"

using namespace std;

uint32_t
HMDataCheckResult::serialize(char* buf, uint32_t size) const
{
    if(buf == nullptr || size < sizeof(SerStruct))
    {
        return sizeof(SerStruct);
    }

    SerStruct* ptr = (SerStruct*)buf;

    ptr->m_address = m_address;
    ptr->m_start = m_start.getTimeSinceEpoch();
    ptr->m_end = m_end.getTimeSinceEpoch();
    ptr->m_responseTime = m_responseTime;
    ptr->m_totalResponseTime = m_totalResponseTime;
    ptr->m_minResponseTime = m_minResponseTime;
    ptr->m_maxResponseTime = m_maxResponseTime;
    ptr->m_smoothedResponseTime = m_smoothedResponseTime;
    ptr->m_sumResponseTime = m_sumResponseTime;
    ptr->m_numChecks = m_numChecks;
    ptr->m_numResponses = m_numResponses;
    ptr->m_numConnectFailures = m_numConnectFailures;
    ptr->m_numFailures = m_numFailures;
    ptr->m_numTimeouts = m_numTimeouts;
    ptr->m_numFlaps = m_numFlaps;
    ptr->m_status = m_status;
    ptr->m_response = m_response;
    ptr->m_reason = m_reason;
    ptr->m_softReason = m_softReason;
    ptr->m_numFailedChecks = m_numFailedChecks;
    ptr->m_numSlowResponses = m_numSlowResponses;
    ptr->m_port = m_port;
    ptr->m_changeTime = m_changeTime.getTimeSinceEpoch();
    ptr->m_prevTime = m_flapTime.getTimeSinceEpoch();
    ptr->m_forceHostDown = m_forceHostDown;
    ptr->m_queueCheckTime = m_queueCheckTime.getTimeSinceEpoch();
    ptr->m_checkTime = m_checkTime.getTimeSinceEpoch();

    return sizeof(SerStruct);
}

bool
HMDataCheckResult::deserialize(char* buf, uint32_t size)
{
    if(buf == nullptr || size < sizeof(SerStruct))
    {
        return false;
    }

    SerStruct* ptr = (SerStruct*)buf;

    m_address = ptr->m_address;
    m_start.setTime(ptr->m_start);
    m_end.setTime(ptr->m_end);
    m_responseTime = ptr->m_responseTime;
    m_totalResponseTime = ptr->m_totalResponseTime;
    m_minResponseTime = ptr->m_minResponseTime;
    m_maxResponseTime = ptr->m_maxResponseTime;
    m_smoothedResponseTime = ptr->m_smoothedResponseTime;
    m_sumResponseTime = ptr->m_sumResponseTime;
    m_numChecks = ptr->m_numChecks;
    m_numResponses = ptr->m_numResponses;
    m_numConnectFailures = ptr->m_numConnectFailures;
    m_numFailures = ptr->m_numFailures;
    m_numTimeouts = ptr->m_numTimeouts;
    m_numFlaps = ptr->m_numFlaps;
    m_status = HM_HOST_STATUS(ptr->m_status);
    m_response = HM_RESPONSE(ptr->m_response);
    m_reason = HM_REASON(ptr->m_reason);
    m_softReason = HM_REASON(ptr->m_softReason);
    m_numFailedChecks = ptr->m_numFailedChecks;
    m_numSlowResponses = ptr->m_numSlowResponses;
    m_port = ptr->m_port;
    m_changeTime.setTime(ptr->m_changeTime);
    m_flapTime.setTime(ptr->m_prevTime);
    m_forceHostDown = ptr->m_forceHostDown;
    m_queueCheckTime.setTime(ptr->m_queueCheckTime);
    m_checkTime.setTime(ptr->m_checkTime);

    return true;
}
