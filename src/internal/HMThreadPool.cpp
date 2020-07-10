// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <tgmath.h>

#include "HMThreadPool.h"

using namespace std;

void
HMThreadPool::resize(uint64_t nThreads)
{
    // increase threads
    if(nThreads > m_nThreads)
    {
        m_workers.resize(nThreads);
        for(uint32_t i = m_nThreads; i < nThreads; i++)
        {
            unique_ptr<HMThreadWorker> mw = make_unique<HMThreadWorker>(HMThreadWorker(m_stateManager, m_eventLoop));
            thread newThread(&HMThreadWorker::runThread, mw.get());
            m_workers[i] = move(make_pair(move(mw), move(newThread)));
        }
    }
    // decrease threads
    else
    {
        for(uint32_t i = m_nThreads - 1; i >= nThreads; --i)
        {
            m_workers[i].first->shutDown();
        }
        m_stateManager->m_workQueue.cycleThreads();
        for(uint32_t i = m_nThreads - 1; i >= nThreads; --i)
        {
            m_workers[i].second.join();
        }
        m_workers.resize(nThreads);
    }
    m_nThreads = nThreads;
}

uint64_t
HMThreadPool::countIdle()
{
    uint64_t total = 0;
    for(auto it = m_workers.begin(); it != m_workers.end(); ++it)
    {
        if(it->first->isIdle())
        {
            total++;
        }
    }
    return total;
}

void
HMThreadPool::recycleThreads()
{
    HMLog(HM_LOG_NOTICE, "[CORE] Recycling of threads started");
    for (uint32_t i = 0; i < m_nThreads; i++)
    {
        if (m_workers[i].first->getUsedCounter() > 10000)
        {
            m_workers[i].first->shutDown();
            m_workers[i].second.join();
            unique_ptr<HMThreadWorker> mw = make_unique<HMThreadWorker>(HMThreadWorker(m_stateManager, m_eventLoop));
            thread newThread(&HMThreadWorker::runThread, mw.get());
            m_workers[i] = move(make_pair(move(mw), move(newThread)));
        }
    }
    HMLog(HM_LOG_NOTICE, "[CORE] Recycling of threads completed");
}

void
HMThreadPool::monitorThreads()
{
    std::shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);

    resize(currentState->getMaxThreads());
    currentState.reset();
    int confidence = 0;
    uint32_t exp = 0;
    uint64_t counter = 0;
    HMLog(HM_LOG_NOTICE, "[CORE] Starting threadpool monitor");
    while(!m_shutdown)
    {
        m_stateManager->updateState(currentState);
        exp+=1;
        uint64_t offSchedule = m_stateManager->m_workQueue.getNumOffSchedule() + m_stateManager->m_workQueue.queueSize();
        uint64_t idle = countIdle();
        uint32_t stride = (m_stridePercent * m_nThreads)/ 100;
        if(offSchedule > 0)
        {
            exp=1;
            m_stateManager->m_workQueue.setNumOffSchedule(0);
            uint64_t newNThreads = m_nThreads + ceil((double)offSchedule/(double)m_workPerThreadRatio);
            uint64_t nThreads = currentState->getMaxThreads();
            resize(newNThreads > nThreads ? nThreads : newNThreads);
        }
        else if(idle > stride)
        {
            exp=1;
            confidence+=1;
            if(confidence >= 3)
            {
                confidence = 0;
                uint64_t newNThreads =  m_nThreads - stride;
                uint64_t nThreads = currentState->getMinThreads();;
                resize(newNThreads < nThreads ? nThreads : newNThreads);
            }

        }
        else
        {
            confidence = 0;
        }
        counter+=m_monitorFrequency;
        lock_guard<mutex> lock();
        if (isRecycle() && counter >= 3600)
        {
            recycleThreads();
            counter = 0;
        }
        currentState.reset();
        sleep(m_monitorFrequency);
    }
    HMLog(HM_LOG_NOTICE, "[CORE] Shutting down threadpool monitor");
}

void
HMThreadPool::shutdown()
{
    m_shutdown = true;
    for(auto it = m_workers.begin(); it != m_workers.end(); ++it)
    {
        it->first->shutDown();
    }
    m_stateManager->m_workQueue.shutdown();
    for(auto it = m_workers.begin(); it!= m_workers.end(); ++it)
    {
        it->second.join();
    }
    m_workers.clear();
}

uint64_t
HMThreadPool::getNThreads()
{
    return m_nThreads;
}

uint32_t
HMThreadPool::getMonitorFrequency() const
{
    return m_monitorFrequency;
}

uint32_t
HMThreadPool::getStridePercent() const
{
    return m_stridePercent;
}

uint32_t
HMThreadPool::getWorkPerThreadRatio() const
{
    return m_workPerThreadRatio;
}

bool
HMThreadPool::isRecycle() const
{
    return m_recycle;
}

void
HMThreadPool::setMonitorFrequency(uint32_t monitorFrequency)
{
    m_monitorFrequency = monitorFrequency;
}

void
HMThreadPool::setStridePercent(uint32_t stridePercent)
{
    m_stridePercent = stridePercent;
}

void
HMThreadPool::setWorkPerThreadRatio(uint32_t workPerThreadRatio)
{
    m_workPerThreadRatio = workPerThreadRatio;
}

void
HMThreadPool::setRecycle(bool recycle)
{
    m_recycle = recycle;
}
