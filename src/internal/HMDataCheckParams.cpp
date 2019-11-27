// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <sstream>
#include <iostream>

#include "HMConstants.h"
#include "HMDataCheckParams.h"
#include "HMLogBase.h"

using namespace std;

bool
HMDataCheckParams::operator<(const HMDataCheckParams& k) const
{
    if(m_numCheckRetries != k.m_numCheckRetries)
    {
        return m_numCheckRetries < k.m_numCheckRetries;
    }
    if(m_checkRetryDelay != k.m_checkRetryDelay)
    {
        return m_checkRetryDelay < k.m_checkRetryDelay;
    }
    if(m_measurementOptions != k.m_measurementOptions)
    {
        return m_measurementOptions < k.m_measurementOptions;
    }
    if(m_smoothingWindow != k.m_smoothingWindow)
    {
        return m_smoothingWindow < k.m_smoothingWindow;
    }
    if(m_groupThreshold != k.m_groupThreshold)
    {
        return m_groupThreshold < k.m_groupThreshold;
    }
    if(m_slowThreshold != k.m_slowThreshold)
    {
        return m_slowThreshold < k.m_slowThreshold;
    }
    if(m_maxFlaps != k.m_maxFlaps)
    {
        return m_maxFlaps < k.m_maxFlaps;
    }
    if(m_checkTimeout != k.m_checkTimeout)
    {
        return m_checkTimeout < k.m_checkTimeout;
    }
    if(m_checkTTL != k.m_checkTTL)
    {
        return m_checkTTL < k.m_checkTTL;
    }
    if(m_flapThreshold != k.m_flapThreshold)
    {
        return m_flapThreshold < k.m_flapThreshold;
    }
    return m_passthroughInfo < k.m_passthroughInfo;
}

bool
HMDataCheckParams::operator==(const HMDataCheckParams& k) const
{
    return (m_numCheckRetries == k.m_numCheckRetries)
            && (m_checkRetryDelay == k.m_checkRetryDelay)
            && (m_measurementOptions == k.m_measurementOptions)
            && (m_smoothingWindow == k.m_smoothingWindow)
            && (m_groupThreshold == k.m_groupThreshold)
            && (m_slowThreshold == k.m_slowThreshold)
            && (m_maxFlaps == k.m_maxFlaps)
            && (m_checkTimeout == k.m_checkTimeout)
            && (m_checkTTL == k.m_checkTTL)
            && (m_flapThreshold == k.m_flapThreshold)
            && (m_passthroughInfo == k.m_passthroughInfo);
}

bool
HMDataCheckParams::operator!=(const HMDataCheckParams& k) const
{
    return !(*this == k);
}

void
HMDataCheckParams::setCheckParameters(uint8_t numCheckRetries,
        uint32_t checkRetryDelay,
        uint16_t measurementOptions,
        uint32_t smoothingWindow,
        uint32_t groupThreshold,
        uint32_t slowThreshold,
        uint32_t maxFlaps,
        uint64_t checkTimeout,
        uint64_t checkTTL,
        uint32_t flapThreshold,
        uint32_t passthroughInfo)
{
    m_numCheckRetries = numCheckRetries;
    m_checkRetryDelay = checkRetryDelay;
    m_measurementOptions = measurementOptions;
    m_smoothingWindow = (smoothingWindow == 0) ? HM_DEFAULT_SMOOTHING_WINDOW : smoothingWindow;
    m_groupThreshold = groupThreshold;
    m_slowThreshold = slowThreshold;
    m_maxFlaps = maxFlaps;
    m_checkTimeout = checkTimeout;
    m_checkTTL = checkTTL;
    m_flapThreshold = flapThreshold;
    m_passthroughInfo = passthroughInfo;
}

void
HMDataCheckParams::setHostStatus(const HMIPAddress& address, bool forceHostStatus)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);
    
    if (address.isSet())
    {
        //IP address is set if we want to force the status of a ip
        auto it = m_checkData.find(address);
        if (it != m_checkData.end())
        {
            it->second.m_forceHostDown = forceHostStatus;
            string setting = (forceHostStatus)?"true":"false";
            HMLog(HM_LOG_DEBUG, "[CHECK]Set ip: %s  m_forceHostDown to %s", address.toString().c_str(), setting.c_str());
        }
    }
    else
    {
        //When we want to force the status of a hostname, then all ip in the hostname is set down
        for (auto ip = m_checkData.begin(); ip != m_checkData.end(); ++ip)
        {
            ip->second.m_forceHostDown = forceHostStatus;
            string setting = (forceHostStatus)?"true":"false";
            HMLog(HM_LOG_DEBUG, "[CHECK]Set ip: %s  m_forceHostDown to %s", ip->first.toString().c_str(), setting.c_str());
        }
    }
}

bool
HMDataCheckParams::checkNeeded(HMIPAddress& address)
{
    return (nextCheckTime(address) <= HMTimeStamp::now());
}

HMTimeStamp
HMDataCheckParams::nextCheckTime(const HMIPAddress& address)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);
    HMTimeStamp now = HMTimeStamp::now();
    auto it = m_checkData.find(address);
    if(it == m_checkData.end())
    {
        return now;
    }
    
    HMTimeStamp nextchecktime;
    nextchecktime =  now + HMTimeStamp::HOURINMS;
    if(it->second.m_queryState == HM_CHECK_INACTIVE || it->second.m_queryState == HM_CHECK_FAILED)
    {
        if((it->second.m_checkTime + m_checkTTL) < now)
        {
            nextchecktime = now;
        }
        else
        {
            if ((it->second.m_numSlowResponses > 0)
                    || (!(it->second.m_softStatus & HM_HOST_STATUS_UP)
                            && (it->second.m_status & HM_HOST_STATUS_UP)
                            && ((it->second.m_numFailedChecks)
                                    <= m_numCheckRetries)))
            {
                nextchecktime = it->second.m_checkTime + m_checkRetryDelay;
            }
            else
            {
                nextchecktime = it->second.m_checkTime + m_checkTTL;
            }
        }
    }
    return nextchecktime;
}

HMTimeStamp
HMDataCheckParams::getCheckTimeout(const HMIPAddress& address)
{
    (void)address;
    return HMTimeStamp::now() + m_checkTTL;
}

void
HMDataCheckParams::queueQuerry(const HMIPAddress& address)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);
    auto it = m_checkData.find(address);
    if(it == m_checkData.end())
    {
        it = m_checkData.insert(make_pair(address,HMDataCheckResult(m_checkTimeout))).first;
    }
    it->second.m_queryState = HM_CHECK_QUEUED;
}

void
HMDataCheckParams::emptyQuery(const HMIPAddress& address)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);
    auto it = m_checkData.find(address);
    if(it == m_checkData.end())
    {
        it = m_checkData.insert(make_pair(address,HMDataCheckResult(m_checkTimeout))).first;
    }
    it->second.m_queryState = HM_CHECK_INACTIVE;
}

void
HMDataCheckParams::startQuery(HMIPAddress& address)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);
    auto it = m_checkData.find(address);
    if(it == m_checkData.end())
    {
        return;
    }
    it->second.m_checkTime = HMTimeStamp::now();
    it->second.m_queryState = HM_CHECK_IN_PROGRESS;
}

bool
HMDataCheckParams::isValidIP(const HMIPAddress& address)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);
    auto it = m_checkData.find(address);
    return (it == m_checkData.end()) ? false : true;
}

void
HMDataCheckParams::updateCheck(const HMIPAddress& address, const HMDataCheckResult& result, bool forceReplace)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);
    auto it = m_checkData.insert(make_pair(address,result));

    // If the entry existed then we replace if forceReplace is set or the checkTime in the cache is older
    if(!it.second && (forceReplace || it.first->second.m_checkTime < result.m_checkTime))
    {
        m_checkData.erase(address);
        m_checkData.insert(make_pair(address,result));
    }
}

void
HMDataCheckParams::updateCheck(string& hostname, const HMIPAddress& address, HM_RESPONSE response, HM_REASON reason, HMTimeStamp start, HMTimeStamp end, uint16_t port)
{

    lock_guard<shared_timed_mutex> lock(m_sharedMutex);

    auto it = m_checkData.find(address);
    if(it == m_checkData.end())
    {
        return;
    }

    HMLog(HM_LOG_DEBUG3,
            "[CHECK] %s: failed checks %d in checks  %d with retries %d , ttl %llu  ,  current status %s",
            it->first.toString().c_str(),
            it->second.m_numFailedChecks,
            it->second.m_numChecks,
            m_numCheckRetries,
            m_checkTTL,
            printStatus(it->second.m_status).c_str());


    if(reason != HM_REASON_REMOTE_NODATA)
    {
        it->second.m_softStatus = (HM_HOST_STATUS)(it->second.m_softStatus & ~(HM_HOST_STATUS_UP));
    }
    it->second.m_statusChanged = false;
    it->second.m_address = address;
    it->second.m_response = response;
    it->second.m_reason = reason;
    it->second.m_start = start;
    it->second.m_end = end;
    it->second.m_port = port;

    if(response == HM_RESPONSE_DNS_FAILED)
    {
        it->second.m_checkTime = start;
        return;
    }

    it->second.m_numChecks++;
    it->second.m_checkTime = HMTimeStamp::now();
    it->second.m_queryState = HM_CHECK_INACTIVE;

    if(response == HM_RESPONSE_CONNECTED)
    {
        it->second.m_reason = reason;

        if(reason == HM_REASON_SUCCESS)
        {
            // merge the results ie expend the call to setResponse
            setResponse(hostname, start, end, it);
        }

        if(it->second.m_reason != HM_REASON_SUCCESS)
        {
            it->second.m_numFailures++;
        }
    }
    else
    {
        it->second.m_reason = reason;

        if(reason == HM_REASON_CONNECT_FAILURE)
        {
            it->second.m_numConnectFailures++;
        }
        else if (reason == HM_REASON_RESPONSE_FAILURE)
        {
            it->second.m_numFailures++;
        }
        else if(reason == HM_REASON_CONNECT_TIMEOUT)
        {
            it->second.m_numTimeouts++;
        }
        else if(reason == HM_REASON_INTERNAL_ERROR)
        {
            it->second.m_numConnectFailures++;
        }
        else if(reason == HM_REASON_REMOTE_NODATA)
        {
            it->second.m_numConnectFailures++;
        }
    }
    it->second.m_softReason = it->second.m_reason;
    // Remote check failure will not go through retry or flapping logic
    if (reason == HM_REASON_REMOTE_NODATA)
    {
        return;
    }

    unsigned long flap = m_flapThreshold  ?m_flapThreshold : HM_DEFAULT_FLAP_THRESHOLD;
    
    //Failed on Retry
    if((it->second.m_status & HM_HOST_STATUS_UP)
            && (!(it->second.m_softStatus & HM_HOST_STATUS_UP))
            && ++(it->second.m_numFailedChecks) <= m_numCheckRetries) 
    {
        HMLog(HM_LOG_NOTICE,
                "[CHECK] Check failed for %s(%s). Check %d of %d failed reason %s for hostgroups %s ",
                hostname.c_str(), address.toString().c_str(),
                it->second.m_numFailedChecks,
                m_numCheckRetries + 1,
                printReason(it->second.m_reason).c_str(),
                printHostGroups().c_str());

       //reason is changed since previous is successful
       it->second.m_reason = HM_REASON_SUCCESS;
    }
    // Successful
    if(it->second.m_softStatus & HM_HOST_STATUS_UP)
    {
        it->second.m_status = (it->second.m_status | HM_HOST_STATUS_UP);
    }
    //Successful on retry
    if(!(it->second.m_status & HM_HOST_STATUS_UP)
            && (it->second.m_softStatus & HM_HOST_STATUS_UP)
            && (it->second.m_numFailedChecks > 0))
    {
        //Host is up , previously host was down
        HMLog(HM_LOG_NOTICE,
                "[CHECK] Check successful after retry (%d), target host=%s(%s) port=%d status=%s reason=%s for hostgroups %s",
                it->second.m_numFailedChecks, hostname.c_str(), address.toString().c_str(),
                it->second.m_port, printStatus(it->second.m_status).c_str(),
                printReason(it->second.m_reason).c_str(),
                printHostGroups().c_str());
        it->second.m_numFailedChecks = 0;
    }

    if (it->second.m_numFailedChecks > m_numCheckRetries)
    {
        HMLog(HM_LOG_NOTICE,
                "[CHECK] %s(%s) is marked DOWN: failed checks(%d) reached maximum number of retries(%d), Failed reason:%s for hostgroups %s",
                hostname.c_str(), address.toString().c_str(), it->second.m_numFailedChecks,
                m_numCheckRetries + 1, printReason(it->second.m_reason).c_str(),
                printHostGroups().c_str());
        it->second.m_status = (HM_HOST_STATUS) (it->second.m_status
                & ~HM_HOST_STATUS_UP);
        it->second.m_numFailedChecks = 0;
    }

    //Flap - previously up current down or previously down current up
    if (((it->second.m_softStatus & HM_HOST_STATUS_UP) != 0)
            != ((it->second.m_flapStatus & HM_HOST_STATUS_UP) != 0))
    {
        if (start - it->second.m_flapTime < flap)
        {
            it->second.m_numFlaps++;
        }
        else
        {
            it->second.m_numFlaps = 0;
        }

        if (it->second.m_numFlaps <= m_maxFlaps + 1)
        {
            string status =
                    (it->second.m_numFlaps > m_maxFlaps) ?
                            "FLAPPING" :
                            ((it->second.m_softStatus & HM_HOST_STATUS_UP) ?
                                    "UP" : "DOWN");
            it->second.m_statusChanged = true;
            HMLog(HM_LOG_NOTICE,
                    "[CHECK] Target %s host=%s(%s) port=%d reason=%s for hostgroups %s ",
                    status.c_str(), hostname.c_str(), address.toString().c_str(), it->second.m_port,
                    printReason(it->second.m_reason).c_str(),
                    printHostGroups().c_str());
        }

        it->second.m_flapTime = start;
        it->second.m_changeTime = start;

        if(it->second.m_numFailedChecks > m_numCheckRetries)
        {
            HMLog(HM_LOG_NOTICE,
                    "[CHECK] %s(%s) is marked DOWN: flaps (%d) exceeded maximum number of flaps(%d), Failed reason:%s for hostgroups %s",
                    hostname.c_str(), address.toString().c_str(),
                    it->second.m_numFlaps, m_maxFlaps,
                    printReason(it->second.m_reason).c_str(),
                    printHostGroups().c_str());
            it->second.m_status = (HM_HOST_STATUS)(it->second.m_status & ~HM_HOST_STATUS_UP);
            it->second.m_numFailedChecks = 0;
        }
    }
    else if(start - it->second.m_flapTime >= flap) 
    {
        it->second.m_numFlaps = 0;
    }

    it->second.m_flapStatus = it->second.m_softStatus;

    HMLog(HM_LOG_DEBUG,
            "[CHECK] UpdateCheck completed target host=%s(%s) port=%d status=%s reason=%s for hostgroups %s",
            hostname.c_str(),
            address.toString().c_str(),
            it->second.m_port,
            printStatus(it->second.m_status).c_str(),
            printReason(it->second.m_reason).c_str(),
            printHostGroups().c_str());
}


bool
HMDataCheckParams::invalidateResult(const HMIPAddress& address, HMDataCheckResult& result)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);

    auto it = m_checkData.find(address);
    if(it == m_checkData.end())
    {
        return false;
    }
    result = it->second;
    m_checkData.erase(address);
    return true;
}

bool
HMDataCheckParams::getCheckResult(const HMIPAddress& address, HMDataCheckResult& result)
{
    shared_lock<shared_timed_mutex> lock(m_sharedMutex);

    auto it = m_checkData.find(address);
    if(it == m_checkData.end())
    {
        return false;
    }
    result = it->second;
    return true;
}

HMTimeStamp
HMDataCheckParams::getCheckTime(HMIPAddress& address)
{
    shared_lock<shared_timed_mutex> lock(m_sharedMutex);

    auto it = m_checkData.find(address);
    if(it == m_checkData.end())
    {
        return HMTimeStamp();
    }
    return it->second.m_checkTime;
}

uint8_t
HMDataCheckParams::getNumCheckRetries() const
{
    return m_numCheckRetries;
}

uint32_t
HMDataCheckParams::getCheckRetryDelay() const
{
    return m_checkRetryDelay;
}

uint16_t
HMDataCheckParams::getMeasurementOptions() const
{
    return m_measurementOptions;
}

uint32_t
HMDataCheckParams::getSmoothingWindow() const
{
    return m_smoothingWindow;
}

uint32_t
HMDataCheckParams::getGroupThreshold() const
{
    return m_groupThreshold;
}

uint32_t
HMDataCheckParams::getSlowThreshold() const
{
    return m_slowThreshold;
}

uint32_t
HMDataCheckParams::getMaxFlaps() const
{
    return m_maxFlaps;
}

uint64_t
HMDataCheckParams::getTimeout() const
{
    return m_checkTimeout;
}

uint64_t
HMDataCheckParams::getTTL() const
{
    return m_checkTTL;
}

uint32_t
HMDataCheckParams::getFlapThreshold() const
{
    return m_flapThreshold;
}

uint32_t
HMDataCheckParams::getPassthroughInfo() const
{
    return m_passthroughInfo;
}

string
HMDataCheckParams::printEntry() const
{
    stringstream output;
    output << "Check Timeout: "     << m_checkTimeout
           << "\nCheck TTL:"          << m_checkTTL
           << "\nNumber Check Retries: "  << (uint32_t)m_numCheckRetries
           << "\nCheck Retry Delay: "     << m_checkRetryDelay
           << "\nMeasurement Options: "     << printMeasurementOptions(m_measurementOptions)
           << "\nSmoothing Window: "  << m_smoothingWindow
           << "\nGroup Threshold: "   << m_groupThreshold
           << "\nSlow Threshold: "    << m_slowThreshold
           << "\nMax Flaps: "         << m_maxFlaps << "\n";
    return output.str();
}

string
HMDataCheckParams::printHostGroups() const
{
    string hostGroups = "";
    for (auto it = m_hostGroups.begin(); it != m_hostGroups.end(); ++it)
    {
        hostGroups = hostGroups.empty() ? *it:hostGroups + "," + *it;
    }
    return hostGroups;
}

bool
HMDataCheckParams::getHostGroups(vector<string>& hostGroups) const
{
    hostGroups = m_hostGroups;
    return true;
}

bool
HMDataCheckParams::getHostGroups(set<string>& hostGroups) const
{
    hostGroups.insert(m_hostGroups.begin(), m_hostGroups.end());
    return true;
}

void
HMDataCheckParams::addHostGroup(string name)
{
    m_hostGroups.push_back(name);
}

HM_WORK_STATE
HMDataCheckParams::getQueryState(HMIPAddress& address)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);

    auto it = m_checkData.find(address);
    if(it == m_checkData.end())
    {
        return HM_CHECK_INACTIVE;
    }

    return it->second.m_queryState;

}

void
HMDataCheckParams::setResponse(string& hostname,
        HMTimeStamp start,
        HMTimeStamp end,
        map<HMIPAddress,HMDataCheckResult>::iterator& it)
{
    uint64_t srt;
    uint64_t rt = end - start;
    uint64_t totrt = HMTimeStamp::now() - start;

    uint8_t rtmode = m_measurementOptions & HM_RT_BITS;

    if (rtmode == HM_RT_CONNECT || rtmode == HM_RT_SMOOTHED_CONNECT)
    {
        srt = rt;
    }
    else
    {
        srt = totrt;
    }

    //Test is successful if only rt is within timeout
    if (srt < m_checkTimeout)
    {
        //Overwriting smoothedResponseTime from timeout to 0
        if (it->second.m_numResponses == 0)
        {
            it->second.m_smoothedResponseTime = 0;
            it->second.m_maxResponseTime = rt;
        }
        if (it->second.m_smoothedResponseTime)
        {
            srt = (srt + (it->second.m_smoothedResponseTime * (m_smoothingWindow - 1))) / m_smoothingWindow;
        }

        it->second.m_reason = HM_REASON_SUCCESS;
        it->second.m_numResponses++;
        it->second.m_sumResponseTime += rt;

        if (!it->second.m_minResponseTime || rt < it->second.m_minResponseTime)
        {
            it->second.m_minResponseTime = rt;
        }

        if (rt > it->second.m_maxResponseTime)
        {
            it->second.m_maxResponseTime = rt;
        }

        uint32_t slowThreshold = (m_groupThreshold > m_slowThreshold) ? m_groupThreshold : m_slowThreshold;

        if (m_numCheckRetries > 0
                && it->second.m_smoothedResponseTime
                && rt >= it->second.m_smoothedResponseTime + slowThreshold
                && ++it->second.m_numSlowResponses <= m_numCheckRetries)
        {
            HMLog(HM_LOG_NOTICE,
                    "[CHECK]%s(%15s): skip slow response (%d of %d): rt = %u,  total rt= %u, (smth rt=%u,st=%u,d=%u)",
                    hostname.c_str(),
                    it->first.toString().c_str(),
                    it->second.m_numSlowResponses,
                    m_numCheckRetries + 1,
                    rt / 1000,
                    totrt / 1000,
                    it->second.m_smoothedResponseTime / 1000,
                    slowThreshold / 1000,
                    (rt - (it->second.m_smoothedResponseTime + slowThreshold)) / 1000);
        }
        else
        {
            it->second.m_numSlowResponses = 0;
            it->second.m_totalResponseTime = totrt;
            it->second.m_responseTime = rt;
            it->second.m_smoothedResponseTime = srt;

            HMLog(HM_LOG_DEBUG3,
                    "[CHECK] %s: check complete: rt = %us, smoothed rt = %us, total rt = %us with response %s",
                    it->first.toString().c_str(),
                    it->second.m_responseTime,
                    it->second.m_smoothedResponseTime,
                    it->second.m_totalResponseTime,
                    printReason(it->second.m_reason).c_str());
        }
        it->second.m_softStatus = (HM_HOST_STATUS)it->second.m_softStatus | HM_HOST_STATUS_UP;
    }
    else
    {
        it->second.m_reason = HM_REASON_RESPONSE_TIMEOUT;
        HMLog(HM_LOG_NOTICE,
              "[CHECK] %s: responded t=%us (rt =%us, total rt=%us)  after timeout: %ums with reason %s",
              it->first.toString().c_str(),
              srt,
              rt,
              totrt,
              m_checkTimeout,
              printReason(it->second.m_reason).c_str());
    }
}

