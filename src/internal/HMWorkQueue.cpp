// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "HMWorkQueue.h"
#include "HMConstants.h"
#include "HMLogBase.h"

using namespace std;

void
HMWorkQueue::cycleThreads()
{
    lock_guard<mutex> lg(m_notifyMutex);
    m_notifyCond.notify_all();
}

void
HMWorkQueue::insertWork(unique_ptr<HMWork>& work)
{
    HMLog(HM_LOG_DEBUG, "[CORE] Pushing Task to Queue checktype %s \"%s\" "
        "ds=%s port=%d host=%s at %s",
        printCheckType(work->m_hostCheck.getCheckType()).c_str(),
        work->m_hostCheck.getCheckInfo().c_str(),
        printDualStack(work->m_hostCheck.getDualStack()).c_str(),
        (uint32_t)work->m_hostCheck.getPort(),
        work->m_hostname.c_str(),
        work->m_ipAddress.toString().c_str());

    unique_lock<mutex> lock(m_queueMutex);
    m_queue.push(move(work));
    lock.unlock();
    lock_guard<mutex> lg(m_notifyMutex);
    m_notifyCond.notify_one();
}

void
HMWorkQueue::insertMap(unique_ptr<HMWork>& work)
{
    lock_guard<mutex> mg(m_mapMutex);
    m_workMap.emplace(work.get(),move(work));
}

void
HMWorkQueue::addWork(HMWork* value)
{
    unique_ptr<HMWork> work;
    {
        lock_guard<mutex> mg(m_mapMutex);
        work = move((m_workMap.at(value)));
        m_workMap.erase(value);
    }
    insertWork(work);
}

bool
HMWorkQueue::getWork(unique_ptr<HMWork>& work, bool& threadShutdown)
{
    unique_lock<mutex> queueLock(m_queueMutex, defer_lock);
    unique_lock<mutex> notifyLock(m_notifyMutex, defer_lock);
    while(!m_shutdown && !threadShutdown)
    {
        // retrieve work
        queueLock.lock();
        if(!m_queue.empty())
        {
            work = move(m_queue.front());
            m_queue.pop();
            queueLock.unlock();
            
            if(work->m_workStatus != HM_WORK_IN_PROGRESS)
            {
                // deal with timing issues here
                HMTimeStamp now = HMTimeStamp::now();

                uint64_t totalTime = now - work->m_start;
                m_totalCount++;
                m_totalTime += totalTime;

                if(m_totalCount == 1000)
                {
                    uint64_t avg = m_totalTime / 1000;
                    m_totalCount = 0;
                    m_totalTime = 0;

                    HMLog(HM_LOG_INFO, "[CORE] Average queue time for 1000 queries is %" PRIu64 " ms", avg);
                }

                if (now > (work->m_end - (((float)m_ttlTreshold/100.0) * (work->m_end - work->m_start))))
                {
                    m_numOffSchedule += 1;
                }

                if(now > work->m_end)
                {
                    uint64_t ttl = work->m_end - work->m_start;
                    HM_WORK_TYPE workType = work->getWorkType();
                    if(workType == HM_WORK_DNSLOOKUP)
                    {
                        HMLog(HM_LOG_WARNING, "[CORE] DNS Lookup Work order for %s in the queue for %" PRIu64" ms with a ttl of %" PRIu64,
                                work->m_hostname.c_str(),
                                totalTime,
                                ttl);
                    }
                    else
                    {
                        HMLog(HM_LOG_WARNING, "[CORE] %s Work order for %s checking %s in the queue for %" PRIu64" ms with a ttl of %" PRIu64,
                                printWorkType(workType).c_str(),
                                work->m_hostname.c_str(),
                                work->m_ipAddress.toString().c_str(),
                                totalTime,
                                ttl);
                    }
                }

                HMLog(HM_LOG_DEBUG3, "[CORE] Processing work order for %s which was queued for %llu",
                        work->m_hostname.c_str(),
                        totalTime);
            }
            HMLog(HM_LOG_DEBUG, "[CORE] Work Queue Length %d", m_queue.size());
            return true;
        }
        queueLock.unlock();

        notifyLock.lock();
        m_notifyCond.wait(notifyLock, [this, &threadShutdown](){return (!m_queue.empty() || m_shutdown || threadShutdown);});
        notifyLock.unlock();
    }
    return false;
}

uint32_t
HMWorkQueue::queueSize()
{
    lock_guard<mutex> lg(m_queueMutex);
    return m_queue.size();
}

uint64_t
HMWorkQueue::getNumOffSchedule() const
{
    return m_numOffSchedule;
}

uint32_t
HMWorkQueue::getTtlTreshold() const
{
    return m_ttlTreshold;
}

void
HMWorkQueue::setNumOffSchedule(uint64_t numOffSchedule)
{
    m_numOffSchedule = numOffSchedule;
}

void
HMWorkQueue::setTtlTreshold(uint32_t ttlTreshold)
{
    m_ttlTreshold = ttlTreshold;

}

void
HMWorkQueue::shutdown()
{
    m_shutdown = true;
    lock_guard<mutex> lg(m_notifyMutex);
    m_notifyCond.notify_all();
}


