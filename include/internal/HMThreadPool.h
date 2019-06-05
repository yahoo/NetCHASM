// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMTHREADPOOL_H_
#define HMTHREADPOOL_H_

#include <vector>
#include <thread>

#include "HMThreadWorker.h"

typedef std::vector<std::pair<std::unique_ptr<HMThreadWorker>, std::thread> > HMWorkerPool;

//! The class that holds the thread pool for the worker threads.
class HMThreadPool
{
public:
    HMThreadPool(HMThreadPool&) = delete;
    HMThreadPool(HMStateManager* state, HMEventLoop* eventLoop) :
        m_nThreads(0),
        m_stridePercent(HM_DEFAULT_STRIDE_PERCENT),
        m_monitorFrequency(HM_DEFAULT_MONITOR_FREQUENCY),
        m_workPerThreadRatio(HM_WORK_PER_THREAD_RATIO),
        m_recycle(false),
        m_stateManager(state),
        m_eventLoop(eventLoop),
        m_shutdown(false) {};

    //! Resize the size of the thread pool.
    /*!
         Resize the thread pool. Callable anytime to adjust the number of threads.
         The resize will happen but is not guarenteed to be immediate upon reduction since the threads are allowed to finish their current task.
         \param the new target size for the thread pool.
     */
    void resize(uint64_t nThreads);

    //! Get the number of idle threads in the thread pool.
    /*!
         Get the number of idle threads in the thread pool.
         \return the number of idle threads in the pool.
     */
    uint64_t countIdle();

    //! Recycle threads with a high used count.
    void recycleThreads();
    //! Function called to run the worker pool monitoring in a new thread.
    void monitorThreads();
    //! Function to shutdow the worker pool, all workers and the thread monitor.
    void shutdown();

    //! Get the number of threads running in the work pool.
    /*!
         Get the number of threads running in the work pool.
         \return the number of threads running in the work pool.
     */
    uint64_t getNThreads();

    //! Get the current timeout between updating the thread monitoring stats.
    /*!
         Get the current timeout between updating the thread monitoring stats.
         \return the current timeout between updating the thread monitoring stats.
     */
    uint32_t getMonitorFrequency() const;

    //! Get the current stride percent.
    /*!
         Get the current stride percent for the work pool. The stride percent is the percentage of idle threads allowed before the Daemon adjusts the thread count lower.
         \return the current stride percent.
     */
    uint32_t getStridePercent() const;

    //! Get the current worker to thread ratio.
    /*!
         Get the current worker to thread ratio. The worker to thread ratio is used to calibrate the number of threads when the pool increases the thread count. It will increase to work queue length / worker per thread ratio.
         \return the current worker to thread ratio.
     */
    uint32_t getWorkPerThreadRatio() const;

    //! Get if the work pool is using thread recycling.
    /*!
         Get if the work pool is using thread recycling. Thread recycling will periodically flush thread state for long running Curl and Ares library state.
         \return true if thread recycling is on.
     */
    bool isRecycle() const;

    //! Set the current timeout between updating the thread monitoring stats.
    /*!
         Set the current timeout between updating the thread monitoring stats.
         \param the new time between updating the thread monitoring stats.
     */
    void setMonitorFrequency(uint32_t monitorFrequency);

    //! Set the current stride percent.
    /*!
         Set the current stride percent. The stride percent is the percentage of idle threads allowed before the Daemon adjusts the thread count lower.
         \param the new stride percent.
     */
    void setStridePercent(uint32_t stridePercent);

    //! Set the work per thread ratio.
    /*!
         Set the work per thread ratio.  The worker to thread ratio is used to calibrate the number of threads when the pool increases the thread count. It will increase to work queue length / worker per thread ratio.
         \param the new work per thread ratio.
     */
    void setWorkPerThreadRatio(uint32_t workPerThreadRatio);

    //! Set the current thread pool to use thread recycle.
    /*!
         Set the current thread pool to use thread recycling. Thread recycling will periodically flush thread state for long running Curl and Ares library state.
         \param true to enable thread recycling.
     */
    void setRecycle(bool recycle);

private:
    uint64_t m_nThreads;
    uint32_t m_stridePercent;
    uint32_t m_monitorFrequency;
    uint32_t m_workPerThreadRatio;
    bool m_recycle
    ;
    HMWorkerPool m_workers;
    HMStateManager* m_stateManager;
    HMEventLoop* m_eventLoop;
    bool m_shutdown;
};

#endif /* HMTHREADPOOL_H_ */
