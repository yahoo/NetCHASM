// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMWORKQUEUE_H_
#define HMWORKQUEUE_H_

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <utility>
#include <map>

#include "HMWork.h"

// The work queue class.
/*!
     Maintains the queue of work to do right now. Thread safe for insertion and retrieving work.
     Work threads will block in get work until work is available, shutdown is called, or thread cycles is called.
 */
class HMWorkQueue
{
public:
    HMWorkQueue() :
        m_totalTime(0),
        m_totalCount(0),
        m_numOffSchedule(0),
        m_ttlTreshold(HM_DEFAULT_TTL_THRESHOLD),
        m_shutdown(false) {};

    //! Cycle the work threads.
    /*!
         Cycle the work threads. Notifies them to stop blocking on the getWork call even though there is no work.
         Used to force idle threads to shutdown and/or check for updated state during a reload.
     */
    void cycleThreads();

    //! Insert work into the queue.
    /*!
         Insert work into the work queue. Main work insertion function to be called to add work.
         \param a unique pointer to the work to add. The work will be moved so the pointer will be null after calling.
     */
    void insertWork(std::unique_ptr<HMWork>& work);

    //! Insert the work into a map for work that requires callback continuations.
    /*!
         Insert the work into a map for work that requires callback continuations.
         For work that requires a callback and continuation, the unique pointer needs to be re-queued for work upon completion of the callback.
         Thus, work with continuation will do the following:
         Push the work unique pointer into a map keyed on the work pointer (This function)
         Scheduled the callback with the pointer value key.
         Upon callback, call addWork to move the work from the map to the active work queue.
         \param unique pointer to the work to store in the callback map.
     */
    void insertMap(std::unique_ptr<HMWork>& work);

    //! Add the given work from the callback map to the active work queue.
    /*!
         Add the given work from the callback map to the active work queue.
         \param the pointer to the work to move to the active work queue.
     */
    void addWork(HMWork* value);

    //! Get a piece of work to run.
    /*!
         Get a piece of work to run.
         Blocks if no work is available. Passes a bool indicating shutdown by reference to return immediately if the thread pool is shutdown.
         \param the work to fill in to run.
         \param a bool to track if the thread has been shutdown.
         \return true if work was retrieved and is ready to be run.
     */
    bool getWork(std::unique_ptr<HMWork>& work, bool& threadShutdown);

    //! Get the size of the work queue.
    /*!
         Get the size of the work queue.
         \return the current size of the work queue.
     */
    uint32_t queueSize();

    //! Get the number of work items started that have been scheduled beyond their TTL * TTLThreshold.
    /*!
         Get the number of work items started that have been scheduled beyond their TTL * TTLThreshold.
         Used to determine if more threads should be created to service the work on time.
         \return the number of work items that were started late since the last call.
     */
    uint64_t getNumOffSchedule() const;

    //! Get the TTL Threshold or percentage of the TTL for the work before it is considered late.
    /*!
         Get the TTL Threshold or percentage of the TTL for the work before it is considered late. Late entries are marked if their start time is greater than schedule time + (TTL * TTL Threshold) / 100.
         \return the TTL Threshold.
     */
    uint32_t getTtlTreshold() const;

    //! Set the Num of off schedule work orders.
    /*!
         Set the Num of off schedule work orders or work items started that have been scheduled beyond their TTL * TTLThreshold.
         Used to determine if more threads should be created to service the work on time.
         \param the new number of off schedule checks.
     */
    void setNumOffSchedule(uint64_t numOffSchedule);

    //! Set the TTL threshold.
    /*!
         Set the TTL Threshold or percentage of the TTL for the work before it is considered late. Late entries are marked if their start time is greater than schedule time + (TTL * TTL Threshold) / 100.
         \param the new ttlThreshold.
     */
    void setTtlTreshold(uint32_t ttlTreshold);

    //! Shutdown the work queue.
    void shutdown();

private:

    std::queue<std::unique_ptr<HMWork>> m_queue;
    std::mutex m_queueMutex;

    std::mutex m_notifyMutex;
    std::condition_variable m_notifyCond;

    std::atomic<uint64_t> m_totalTime;
    std::atomic<uint32_t> m_totalCount;
    std::atomic<uint64_t> m_numOffSchedule;
    uint32_t m_ttlTreshold;

    std::mutex m_mapMutex;

    std::map<HMWork*, std::unique_ptr<HMWork>> m_workMap;

    bool m_shutdown;
};

#endif /* HMWORKQUEUE_H_ */
