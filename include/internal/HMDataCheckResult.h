// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDATACHECKRESULT_H_
#define HMDATACHECKRESULT_H_

#include <cstdint>

#include "HMIPAddress.h"
#include "HMTimeStamp.h"
#include "HMConstants.h"

//! The class to hold the results of a health check.
class HMDataCheckResult
{
public:
    bool operator==(const HMDataCheckResult& k) const
    {
        if(m_address == k.m_address
                && m_start == k.m_start
                && m_end == k.m_end
                && m_responseTime == k.m_responseTime
                && m_totalResponseTime == k.m_totalResponseTime
                && m_minResponseTime == k.m_minResponseTime
                && m_maxResponseTime == k.m_maxResponseTime
                && m_smoothedResponseTime == k.m_smoothedResponseTime
                && m_sumResponseTime == k.m_sumResponseTime
                && m_numChecks == k.m_numChecks
                && m_numResponses == k.m_numResponses
                && m_numConnectFailures == k.m_numConnectFailures
                && m_numFailures == k.m_numFailures
                && m_numTimeouts == k.m_numTimeouts
                && m_numFlaps == k.m_numFlaps
                && m_status == k.m_status
                && m_response == k.m_response
                && m_reason == k.m_reason
                && m_numFailedChecks == k.m_numFailedChecks
                && m_numSlowResponses == k.m_numSlowResponses
                && m_port == k.m_port
                && m_changeTime == k.m_changeTime
                && m_flapTime == k.m_flapTime
                && m_queueCheckTime == k.m_queueCheckTime
                && m_checkTime == k.m_checkTime
                && m_queryState == k.m_queryState)
        {
            return true;
        }
        return false;
    }

    HMDataCheckResult() :
        m_responseTime(0),
        m_totalResponseTime(0),
        m_minResponseTime(0),
        m_maxResponseTime(0),
        m_smoothedResponseTime(0),
        m_sumResponseTime(0),
        m_numChecks(0),
        m_numResponses(0),
        m_numConnectFailures(0),
        m_numFailures(0),
        m_numTimeouts(0),
        m_numFlaps(0),
        m_status(HM_HOST_STATUS_NONE),
        m_softStatus(HM_HOST_STATUS_NONE),
        m_flapStatus(HM_HOST_STATUS_NONE),
        m_response(HM_RESPONSE_NONE),
        m_reason(HM_REASON_NONE),
        m_numFailedChecks(0),
        m_numSlowResponses(0),
        m_port(0),
        m_forceHostDown(0),
        m_queryState(HM_CHECK_INACTIVE),
        m_statusChanged(false) {};

    HMDataCheckResult(uint64_t checkTimeout) :
        m_responseTime(checkTimeout),
        m_totalResponseTime(checkTimeout),
        m_minResponseTime(checkTimeout),
        m_maxResponseTime(checkTimeout),
        m_smoothedResponseTime(checkTimeout),
        m_sumResponseTime(0),
        m_numChecks(0),
        m_numResponses(0),
        m_numConnectFailures(0),
        m_numFailures(0),
        m_numTimeouts(0),
        m_numFlaps(0),
        m_status(HM_HOST_STATUS_NONE),
        m_softStatus(HM_HOST_STATUS_NONE),
        m_flapStatus(HM_HOST_STATUS_NONE),
        m_response(HM_RESPONSE_NONE),
        m_reason(HM_REASON_NONE),
        m_numFailedChecks(0),
        m_numSlowResponses(0),
        m_port(0),
        m_forceHostDown(0),
        m_queryState(HM_CHECK_INACTIVE),
        m_statusChanged(false) {};

    //! The IPAddress that was checked.
    HMIPAddress m_address;
    //! The start time of the check.
    HMTimeStamp m_start;
    //! The end time of the check.
    HMTimeStamp m_end;
    //! The amount of time it took for the check to connect.
    uint32_t m_responseTime;
    //! The total time it took for the check from beginning to the end of the transfer.
    uint32_t m_totalResponseTime;
    //! The minimum response time seen for this check.
    uint32_t m_minResponseTime;
    //! The maximum response time seen for this check.
    uint32_t m_maxResponseTime;
    //! The smoothed response time over multiple check.
    uint32_t m_smoothedResponseTime;
    //! The sum of the response time over multiple checks.
    uint64_t m_sumResponseTime;
    //! The total number of times this check was conducted.
    uint32_t m_numChecks;
    //! The total number of responses for this check.
    uint32_t m_numResponses;
    //! The total number of connect failures for this check.
    uint32_t m_numConnectFailures;
    //! The total number of failures for this check.
    uint32_t m_numFailures;
    //! The total number of timeouts for this check.
    uint32_t m_numTimeouts;
    //! The total number of current flaps for this check.
    uint32_t m_numFlaps;
    //! The current status code of this check.
    HM_HOST_STATUS m_status;
    //! The retry and soft status code of this check.
    HM_HOST_STATUS m_softStatus;
    //! The flap status code of this check.
    HM_HOST_STATUS m_flapStatus;
    //! The current response code for this check.
    HM_RESPONSE m_response;
    //! The final reason code for the status after retries check.
    HM_REASON m_reason;
    //! The current reason code for the status of this check.
    HM_REASON m_softReason;
    //! The number of failed checks for any reason.
    uint8_t m_numFailedChecks;
    //! The number of slow responses for thsi check.
    uint8_t m_numSlowResponses;
    //! The port used in the check.
    uint16_t m_port;
    //! The last time the status of this check changed.
    HMTimeStamp m_changeTime;
    //! The check time for the previous time this check was done. (For Flapping)
    HMTimeStamp m_flapTime;
    //! The flag to force this host check to down.
    bool m_forceHostDown;
    //! The time this check was put into the work queue.
    HMTimeStamp m_queueCheckTime;
    //! The last time this check was conducted.
    HMTimeStamp m_checkTime;
    //! The current WORK_STATE to track the current state of the check.
    HM_WORK_STATE m_queryState;
    //! The last time this remote host was contacted.
    HMTimeStamp m_remoteCheckTime;
    //! Status showing Host went up to down or down to up
    bool m_statusChanged;
    //! Function to serialize the current check result info into a raw buffer.
    /*!
         The serialize function supports two types of calls designed to be called consecutively.
         When called with a null buf and size 0 serialize will return the required size of the buf to store the check result.
         When called with a non-null buf and the correct size, serialize will store the check result into the buf.
         \param buf pass nullptr to get the required size or a raw buffer to fill.
         \param size pass 0 to get the required size or the required size to fill the buffer.
         \return The required size of the buf or the number of bytes saved to the buffer.
     */
    uint32_t serialize(char* buf, uint32_t size) const;

    //! De-serialize the raw buffer.
    /*!
         This function is called to deserialize a check result. It fills in the class data from the raw buffer.
         \param buf raw buffer to deserialize.
         \param size the size of the raw buffer.
         \return true if the deserialize was a success.
     */
    bool deserialize(char* buf, uint32_t size);

private:

    struct SerStruct
    {
        HMIPAddress m_address;
        uint64_t m_start;
        uint64_t m_end;
        uint32_t m_responseTime;
        uint32_t m_totalResponseTime;
        uint32_t m_minResponseTime;
        uint32_t m_maxResponseTime;
        uint32_t m_smoothedResponseTime;
        uint64_t m_sumResponseTime;
        uint32_t m_numChecks;
        uint32_t m_numResponses;
        uint32_t m_numConnectFailures;
        uint32_t m_numFailures;
        uint32_t m_numTimeouts;
        uint32_t m_numFlaps;
        uint16_t m_status;
        uint8_t m_response;
        uint8_t m_reason;
        uint8_t m_softReason;
        uint8_t m_numFailedChecks;
        uint8_t m_numSlowResponses;
        uint16_t m_port;
        uint64_t m_changeTime;
        uint64_t m_prevTime;
        bool m_forceHostDown;
        // variables to track the last check information
        uint64_t m_queueCheckTime;
        uint64_t m_checkTime;
    };
};

#endif /* HMDATACHECKRESULT_H_ */
