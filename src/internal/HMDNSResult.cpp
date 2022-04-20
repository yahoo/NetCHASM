// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <chrono>
#include <iostream>
#include <algorithm>

#include "HMDNSResult.h"
#include "HMLogBase.h"

using namespace std;

void
HMDNSResult::updateTimeouts(uint64_t ttl, uint64_t timeout)
{
    m_queryTimeout = (timeout < m_queryTimeout)?timeout:m_queryTimeout;
    m_dnsTimeout = (ttl < m_dnsTimeout)?ttl:m_dnsTimeout;
}

void
HMDNSResult::updateQuery(const set<HMIPAddress>& addresses)
{
    lock_guard<shared_timed_mutex> lock(m_resultLock);
    m_addrExp.clear();
    set_difference(m_addr.begin(), m_addr.end(),
            addresses.begin(), addresses.end(),
            inserter(m_addrExp, m_addrExp.begin()));
    m_addr = addresses;
}

void
HMDNSResult::queueQuery()
{
    lock_guard<shared_timed_mutex> lock(m_resultLock);
    m_queryState = HM_CHECK_QUEUED;
}

HMTimeStamp
HMDNSResult::startQuery(uint32_t version)
{
    lock_guard<shared_timed_mutex> lock(m_resultLock);
    m_queryState = HM_CHECK_IN_PROGRESS;
    m_queryTime = HMTimeStamp::now() + m_queryTimeout;
    m_queryVersion = version;
    return m_queryTime;
}

void
HMDNSResult::finishQuery(bool success)
{
    lock_guard<shared_timed_mutex> lock(m_resultLock);
    m_queryState = success?HM_CHECK_INACTIVE:HM_CHECK_FAILED;
    m_resultTime = HMTimeStamp::now();
}

bool
HMDNSResult::queryNeeded(uint32_t version) const
{
    return (nextQueryTime(version) <= HMTimeStamp::now());
}

HMTimeStamp
HMDNSResult::nextQueryTime(uint32_t version) const
{
    lock_guard<shared_timed_mutex> lock(m_resultLock);
    HMTimeStamp now = HMTimeStamp::now();
    if ( m_queryVersion != version ) {
       return now;
    }
    if(m_queryState == HM_CHECK_INACTIVE)
    {
        if((m_resultTime + m_dnsTimeout) < now)
        {
            return now;
        }
        else
        {
            return m_resultTime + m_dnsTimeout;
        }
    }
    else if(m_queryState == HM_CHECK_IN_PROGRESS)
    {
        if((m_queryTime + m_queryTimeout) < now)
        {
            return now;
        }
        else
        {
            return m_queryTime + m_queryTimeout;
        }
    }
    else if(m_queryState == HM_CHECK_FAILED)
    {
        return m_resultTime + m_dnsTimeout;
    }
    // return something really high since the check is in progress
    return now + HMTimeStamp::HOURINMS;
}

uint64_t
HMDNSResult::getDNSTTL() const
{
    return m_dnsTimeout;
}

bool
HMDNSResult::getAddresses(set<HMIPAddress>& addresses) const
{
    shared_lock<shared_timed_mutex> lock(m_resultLock);
    if(m_addr.size() == 0)
    {
        return false;
    }
    addresses.insert(m_addr.begin(),m_addr.end());
    return true;
}

bool
HMDNSResult::getAddress(HMIPAddress& address) const
{
    shared_lock<shared_timed_mutex> lock(m_resultLock);
    if(m_addr.size() == 0)
    {
        return false;
    }
    address = *m_addr.begin();
    return true;
}

bool
HMDNSResult::getExpiredAddresses(set<HMIPAddress>& addresses) const
{
    shared_lock<shared_timed_mutex> lock(m_resultLock);
    if(m_addrExp.size() == 0)
    {
        return false;
    }
    addresses.insert(m_addrExp.begin(), m_addrExp.end());
    return true;
}

bool
HMDNSResult::isValidAddress(HMIPAddress& address) const
{
    return (m_addr.find(address) != m_addr.end());
}


HM_WORK_STATE
HMDNSResult::getQueryState() const
{
    return m_queryState;
}

uint32_t
HMDNSResult::getQueryVersion() const
{
   return m_queryVersion;
}

const HMTimeStamp&
HMDNSResult::getResultTime() const
{
    return m_resultTime;
}

void
HMDNSResult::setResultTime(const HMTimeStamp& resultTime)
{
    m_resultTime = resultTime;
}

