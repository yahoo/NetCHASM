// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <chrono>
#include <iostream>
#include <algorithm>

#include "HMRemoteResult.h"
#include "HMLogBase.h"

using namespace std;

void
HMRemoteResult::updateTimeouts(uint64_t ttl, uint64_t timeout)
{
    m_remoteTimeout = (timeout < m_remoteTimeout)?timeout:m_remoteTimeout;
    m_remoteTTL = (ttl < m_remoteTTL)?ttl:m_remoteTTL;
}

void
HMRemoteResult::queueCheck()
{
    lock_guard<shared_timed_mutex> lock(m_resultLock);
    m_checkState = HM_CHECK_QUEUED;
}

HMTimeStamp
HMRemoteResult::startCheck()
{
    lock_guard<shared_timed_mutex> lock(m_resultLock);
    m_checkState = HM_CHECK_IN_PROGRESS;
    m_checkTime = HMTimeStamp::now() + m_remoteTimeout;
    return m_checkTime;
}

void
HMRemoteResult::finishCheck(bool success)
{
    lock_guard<shared_timed_mutex> lock(m_resultLock);
    m_checkState = success?HM_CHECK_INACTIVE:HM_CHECK_FAILED;
    m_resultTime = HMTimeStamp::now();
}

bool
HMRemoteResult::checkNeeded() const
{
    return (nextCheckTime() <= HMTimeStamp::now());
}

HMTimeStamp
HMRemoteResult::nextCheckTime() const
{
    lock_guard<shared_timed_mutex> lock(m_resultLock);
    HMTimeStamp now = HMTimeStamp::now();
    if(m_checkState == HM_CHECK_INACTIVE)
    {
        if((m_resultTime + m_remoteTTL) < now)
        {
            return now;
        }
        else
        {
            return m_resultTime + m_remoteTTL;
        }
    }
    else if(m_checkState == HM_CHECK_IN_PROGRESS)
    {
        if((m_checkTime + m_remoteTimeout) < now)
        {
            return now;
        }
        else
        {
            return m_checkTime + m_remoteTimeout;
        }
    }
    else if(m_checkState == HM_CHECK_FAILED)
    {
        return m_resultTime + m_remoteTTL;
    }
    // return something really high since the check is in progress
    return now + HMTimeStamp::HOURINMS;
}

uint64_t
HMRemoteResult::getCheckTTL() const
{
    return m_remoteTTL;
}

HM_WORK_STATE
HMRemoteResult::getCheckState() const
{
    return m_checkState;
}

const HMTimeStamp&
HMRemoteResult::getResultTime() const
{
    return m_resultTime;
}

void
HMRemoteResult::setResultTime(const HMTimeStamp& resultTime)
{
    m_resultTime = resultTime;
}

