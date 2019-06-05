// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDES_HMTHREADWORKER_H_
#define INCLUDES_HMTHREADWORKER_H_

#include <cstdint>
#include <ares.h>
#include <thread>
#include <atomic>

#include "HMWork.h"
#include "HMEventLoop.h"
#include "HMStateManager.h"

//! The thread worker class maintains all state for a worker thread. It will run tasks from the work queue indefinitely.
class HMThreadWorker
{

public:
    HMThreadWorker(HMStateManager* stateManager, HMEventLoop* eventLoop) :
        m_idle(false),
        m_running(false),
        m_nUsed(0),
        m_shutdown(false),
        m_threadID(0),
        m_stateManager(stateManager),
        m_eventLoop(eventLoop) {};

    HMThreadWorker(HMThreadWorker&& k) :
        m_idle(k.m_idle.load()),
        m_running(k.m_running.load()),
        m_nUsed(k.m_nUsed),
        m_shutdown(k.m_shutdown),
        m_threadID(k.m_threadID),
        m_stateManager(k.m_stateManager),
        m_eventLoop(k.m_eventLoop),
        m_workState(k.m_workState) { m_order = std::move(k.m_order); };

    //! The function to run the thread called in the spawn thread.
    /*!
         The function to run the thread called in the spawn thread.
         Pops tasks off the work queue and conducts the work.
     */
    void runThread();

    //! Shutdown the thread.
    void shutDown();

    //! Get if the thread is idle.
    /*!
         Get if the thread is idle.
         \return true if the thread is idle.
     */
    bool isIdle();

    //! Get the number of tasks the current thread has completed.
    /*!
         Get the number of tasks the current thread has completed.
         \return the number of tasks the current thread has completed.
     */
    uint64_t getUsedCounter();

private:
    std::atomic<bool> m_idle;
    std::atomic<bool> m_running;
    uint64_t m_nUsed;
    bool m_shutdown;
    uint64_t m_threadID;
    HMStateManager* m_stateManager;
    HMEventLoop* m_eventLoop;
    std::unique_ptr<HMWork> m_order;
    HMWorkState m_workState;

};

#endif /* INCLUDES_HMTHREADWORKER_H_ */
