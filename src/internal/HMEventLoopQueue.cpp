// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMEventLoopQueue.h"
#include "HMLogBase.h"

using namespace std;

void
HMEventLoopQueue::addDNSTimeout(const string& hostname, const HMDNSLookup& dnslookup, HMTimeStamp timeStamp)
{
    auto queueLock = unique_lock<mutex> (m_queueMutex, defer_lock);
    HMLog(HM_LOG_DEBUG3, "[EVENT] Adding DNS scheduler timeout %llu", timeStamp.getTimeSinceEpoch());

    bool preempt = false;
    queueLock.lock();
    if(m_timeouts.empty() || timeStamp < m_timeouts.top().m_timeout)
    {
        preempt = true;
    }
    m_timeouts.push(Timeout(hostname, dnslookup, timeStamp));
    queueLock.unlock();

    // if the Timeout we are inserting is before the next timeout, kick the tracker
    if(preempt)
    {
        wakeupTracker();
    }
}

void
HMEventLoopQueue::addRemoteTimeout(const string& hostname, HMTimeStamp timeStamp)
{
    auto queueLock = unique_lock<mutex> (m_queueMutex, defer_lock);
    HMLog(HM_LOG_DEBUG3, "[EVENT] Adding Remote scheduler timeout %llu", timeStamp.getTimeSinceEpoch());

    bool preempt = false;
    queueLock.lock();
    if(m_timeouts.empty() || timeStamp < m_timeouts.top().m_timeout)
    {
        preempt = true;
    }
    m_timeouts.push(Timeout(hostname, timeStamp));
    queueLock.unlock();

    // if the Timeout we are inserting is before the next timeout, kick the tracker
    if(preempt)
    {
        wakeupTracker();
    }
}

void
HMEventLoopQueue::addRemoteHostTimeout(const std::string& hostname, const HMDataHostCheck& dataHostCheck,HMTimeStamp timeStamp)
{
    auto queueLock = unique_lock<mutex> (m_queueMutex, defer_lock);
    HMLog(HM_LOG_DEBUG3, "[EVENT] Adding Remote host scheduler timeout %llu", timeStamp.getTimeSinceEpoch());

    bool preempt = false;
    queueLock.lock();
    if(m_timeouts.empty() || timeStamp < m_timeouts.top().m_timeout)
    {
        preempt = true;
    }
    m_timeouts.push(Timeout(hostname, dataHostCheck, timeStamp));
    queueLock.unlock();

    // if the Timeout we are inserting is before the next timeout, kick the tracker
    if(preempt)
    {
        wakeupTracker();
    }
}


void
HMEventLoopQueue::addHealthCheckTimeout(const string& hostname, const HMIPAddress& address, const HMDataHostCheck check, HMTimeStamp timeStamp)
{
    auto queueLock = unique_lock<mutex> (m_queueMutex, defer_lock);
    bool preempt = false;

    HMLog(HM_LOG_DEBUG3, "[EVENT] Adding HealthCheck Scheduler timeout %llu for %s", timeStamp.getTimeSinceEpoch(),hostname.c_str());

    queueLock.lock();
    if(m_timeouts.empty() || timeStamp < m_timeouts.top().m_timeout)
    {
        preempt = true;
    }
    m_timeouts.push(Timeout(hostname, address, check, timeStamp));
    queueLock.unlock();

    // if the Timeout we are inserting is before the next timeout, kick the tracker
    if(preempt)
    {
        wakeupTracker();
    }
}

void
HMEventLoopQueue::wakeupTracker()
{
    lock_guard<mutex> lock(m_sleepMutex);
    m_wakeup = true;
    m_sleepCond.notify_all();
}

void
HMEventLoopQueue::shutDown()
{
    m_keepRunning = false;
    wakeupTracker();
    m_thread.join();
}

uint64_t
HMEventLoopQueue::getTimeOutQueueSize()
{
    return m_timeouts.size();
}

void
HMEventLoopQueue::runThread()
{
    m_keepRunning = true;
    m_thread = std::thread(&HMEventLoopQueue::run, this);
}

void
HMEventLoopQueue::run()
{
    auto queueLock = unique_lock<mutex> (m_queueMutex, defer_lock);
    auto sleepLock = unique_lock<mutex> (m_sleepMutex, defer_lock);
    std::shared_ptr<HMState> currentState;
    HMTimeStamp nextTimeout;
    // Initial timeout check
    queueLock.lock();
    nextTimeout = (m_timeouts.empty()?
            (HMTimeStamp::now() + m_emptyTimeout):(m_timeouts.top().m_timeout));
    queueLock.unlock();

    while(m_keepRunning)
    {

        // Wait if there is no work to do and we are not shutting down
        while(HMTimeStamp::now() < nextTimeout && m_keepRunning)
        {

            HMLog(HM_LOG_DEBUG3, "[EVENT] Event Queue Sleeping until %llu s", nextTimeout.getTimeSinceEpoch());

            // Sleep till the next timeout
            sleepLock.lock();
            if(!m_wakeup)
            {
                m_sleepCond.wait_until(sleepLock, nextTimeout.m_timeStamp);
            }
            else
            {
                m_wakeup = false;
            }
            sleepLock.unlock();
            if(!m_keepRunning)
            {
                return;
            }

            m_stateManager->updateState(currentState);

            // Check the latest timeout value
            queueLock.lock();
            nextTimeout = (m_timeouts.empty() ? (HMTimeStamp::now() + m_emptyTimeout) : (m_timeouts.top().m_timeout));
            queueLock.unlock();
        }

        HMLog(HM_LOG_DEBUG3, "[EVENT] Event Queue Waking Up");

        // Ok Wait again if we have not timeout event
        queueLock.lock();
        if(m_timeouts.empty())
        {
            nextTimeout = HMTimeStamp::now() + m_emptyTimeout;
            queueLock.unlock();
            continue;
        }
        Timeout timeout = m_timeouts.top();
        m_timeouts.pop();
        queueLock.unlock();

        // Update to the current state
        m_stateManager->updateState(currentState);
        HM_SCHEDULE_STATE check_state;

        switch(timeout.m_type)
        {
        case HEALTHCHECK_TIMEOUT:
            HMLog(HM_LOG_DEBUG, "[EVENT] Health Check Scheduler Timeout for %s", timeout.m_hostname.c_str());

            check_state = currentState->m_checkList.checkNeeded(timeout.m_hostname, timeout.m_address, timeout.m_hostCheck);
            if (check_state == HM_SCHEDULE_WORK)
            {
                HMLog(HM_LOG_DEBUG3, "[DEBUG] Health Check Schedule work for %s", timeout.m_hostname.c_str());
                currentState->m_checkList.queueCheck(timeout.m_hostname, timeout.m_address, timeout.m_hostCheck, m_stateManager->m_workQueue);
            }
            else if (check_state == HM_SCHEDULE_EVENT)
            {
                HMLog(HM_LOG_DEBUG3, "[DEBUG] Health Check Schedule event for %s", timeout.m_hostname.c_str());
                HMTimeStamp nextCheckTimeOut = currentState->m_checkList.nextCheckTime(timeout.m_hostname, timeout.m_address, timeout.m_hostCheck);
                addHealthCheckTimeout(timeout.m_hostname, timeout.m_address, timeout.m_hostCheck, nextCheckTimeOut);
            }
            break;
        case REMOTECHECK_TIMEOUT:
            HMLog(HM_LOG_DEBUG, "[EVENT] Remote Scheduler host group Check Timeout for %s", timeout.m_hostname.c_str());

            check_state = currentState->m_remoteCache.checkNeeded(timeout.m_hostname);
            if (check_state == HM_SCHEDULE_WORK)
            {
                HMLog(HM_LOG_DEBUG3, "[DEBUG] Remote Check Schedule work for %s", timeout.m_hostname.c_str());
                currentState->m_remoteCache.queueRemoteCheck(timeout.m_hostname, m_stateManager->m_workQueue, currentState->m_hostGroups);
            }
            else if (check_state == HM_SCHEDULE_EVENT)
            {
                HMLog(HM_LOG_DEBUG3, "[DEBUG] Remote Check Schedule event for %s", timeout.m_hostname.c_str());
                HMTimeStamp nextCheckTimeOut = currentState->m_remoteCache.nextCheckTime(timeout.m_hostname);
                addRemoteTimeout(timeout.m_hostname, nextCheckTimeOut);
            }
            break;
        case REMOTEHOSTCHECK_TIMEOUT:
            HMLog(HM_LOG_DEBUG, "[EVENT] Remote Scheduler host Check Timeout for %s", timeout.m_hostname.c_str());

            check_state = currentState->m_remoteHostCache.checkNeeded(timeout.m_hostname, timeout.m_hostCheck);
            if (check_state == HM_SCHEDULE_WORK)
            {
                HMLog(HM_LOG_DEBUG3, "[DEBUG] Remote host Check Schedule work for %s", timeout.m_hostname.c_str());
                currentState->m_remoteHostCache.queueRemoteCheck(timeout.m_hostname, timeout.m_hostCheck, m_stateManager->m_workQueue);
            }
            else if (check_state == HM_SCHEDULE_EVENT)
            {
                HMLog(HM_LOG_DEBUG3, "[DEBUG] Remote host Check Schedule event for %s", timeout.m_hostname.c_str());
                HMTimeStamp nextCheckTimeOut = currentState->m_remoteHostCache.nextCheckTime(timeout.m_hostname, timeout.m_hostCheck);
                addRemoteTimeout(timeout.m_hostname, nextCheckTimeOut);
            }
            break;
        case DNS_TIMEOUT:
        case DNSV6_TIMEOUT:
            HMLog(HM_LOG_DEBUG, "[EVENT] DNS Scheduler Entry Timeout for %s",  timeout.m_hostname.c_str());

            check_state = currentState->m_dnsCache.queryNeeded(timeout.m_hostname, timeout.m_dnsLookup);
            if (check_state == HM_SCHEDULE_WORK)
            {
                HMLog(HM_LOG_DEBUG3, "[DEBUG] DNS Health Check Schedule work for %s", timeout.m_hostname.c_str());
                currentState->m_dnsCache.queueDNSQuery(timeout.m_hostname, timeout.m_dnsLookup, m_stateManager->m_workQueue);
            }
            else if (check_state == HM_SCHEDULE_EVENT)
            {
                HMLog(HM_LOG_DEBUG3, "[DEBUG] DNS Health Check Schedule event for %s", timeout.m_hostname.c_str());
                HMTimeStamp nextCheckTimeOut = currentState->m_checkList.nextCheckTime(timeout.m_hostname, timeout.m_address, timeout.m_hostCheck);
                addDNSTimeout(timeout.m_hostname, timeout.m_dnsLookup, nextCheckTimeOut);
            }
            else
            {
                string ip_version = timeout.m_type == DNSV6_TIMEOUT? "IPv6":"IPv4";
                HMLog(HM_LOG_DEBUG3, "%s DNS Check dropped for %s, already in schedule", ip_version.c_str(), timeout.m_hostname.c_str());
            }
            break;

        }
        queueLock.lock();
        nextTimeout = (m_timeouts.empty() ? (HMTimeStamp::now() + m_emptyTimeout) : (m_timeouts.top().m_timeout));
        queueLock.unlock();
    }
}
