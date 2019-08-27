// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMEVENTLOOPQUEUE_H_
#define HMEVENTLOOPQUEUE_H_

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "HMWork.h"
#include "HMTimeStamp.h"
#include "HMDataCheckList.h"
#include "HMDataHostCheck.h"
#include "HMStateManager.h"
#include "HMWorkQueue.h"
#include "HMConstants.h"

//! Event loop implementation using a simple priority queue.
class HMEventLoopQueue : public HMEventLoop
{
public:
    HMEventLoopQueue(HMEventLoopQueue&) = delete;
    HMEventLoopQueue(HMStateManager* state) :
        m_keepRunning(true),
        m_emptyTimeout(300000),
        m_stateManager(state),
        m_wakeup(false) {};

    //! Add a new DNS timeout.
    /*!
         Add a new DNS timeout to the event loop.
         \param the hostname to resolve.
         \param true to resolve an IPv6 address.
         \param the time Stamp of when the DNS resolution should take place.
     */
    void addDNSTimeout(const std::string& hostname, const HMDNSLookup& dnsHostCheck, HMTimeStamp timeStamp);

    //! Add a new health check timeout.
    /*!
         Add a new health check timeout to the event loop.
         \param the hostname to health check.
         \param the IP address to health check.
         \param the host check to conduct.
         \param the time stamp of when the health check should take place.
     */
    void addHealthCheckTimeout(const std::string& hostname, const HMIPAddress& address, const HMDataHostCheck hostCheck, HMTimeStamp timeStamp);

    //! Wakeup the tracker.
    /*!
         Wakeup the tracker. Called if the new event timeout is earlier than the latest or the tracker needs  to check for shutdown condition.
     */
    void wakeupTracker();

    //! Shutdown the event loop and cleanup.
    /*!
         Shutdown the event loop and cleanup.
     */
    void shutDown();

    //! Get the number of events in the event loop.
    /*!
         Get the number of events in the event loop.
         \return the number of events in the event loop.
     */
    uint64_t getTimeOutQueueSize();

    //! Init and startup the thread for the event loop.
     /*!
          Init and startup the thread for the event loop.
      */
    void runThread();

private:

    //! The internal run function.
    /*!
         This function should run the event loop in a new thread.
     */
    void run();

    std::condition_variable m_sleepCond;
    std::mutex m_sleepMutex;
    std::mutex m_queueMutex;

    bool m_keepRunning;
    uint64_t m_emptyTimeout;

    //! The type of timeout that fired.
    enum TimeoutType
    {
        HEALTHCHECK_TIMEOUT,
        DNS_TIMEOUT,
        DNSV6_TIMEOUT
    };

    //! The class to hold parameters about the timeout that occurred.
    /*!
         The class to hold the parameters about the timeout that occurred.
         This class passes all information required to schedule the work when the timeout occurs.
     */
    class Timeout
    {
    public:
        std::string m_hostname;
        HMDataHostCheck m_hostCheck;
        HMTimeStamp m_timeout;
        TimeoutType m_type;
        HM_DNS_PLUGIN_CLASS m_dnsPlugin;
        HMIPAddress m_address;

        Timeout(const std::string& host, HM_DNS_PLUGIN_CLASS plugin, bool ipv6, const HMTimeStamp expiration)
        {
            m_hostname = host;
            m_timeout = expiration;
            m_type = ipv6?DNSV6_TIMEOUT:DNS_TIMEOUT;
            m_dnsPlugin = plugin;
        }

        Timeout(const std::string& host, const HMIPAddress& address, const HMDataHostCheck check, const HMTimeStamp expiration)
        {
            m_hostname = host;
            m_hostCheck = check;
            m_timeout = expiration;
            m_type = HEALTHCHECK_TIMEOUT;
            m_address = address;
            m_dnsPlugin = check.getDnsPlugin();
        }

        bool operator< (const Timeout& t) const
        {
            // note the inverted order to get the lowest timestamp on top
            return m_timeout > t.m_timeout;
        }

    };

    typedef std::priority_queue<Timeout> TimeoutQueue;
    TimeoutQueue m_timeouts;

    HMStateManager* m_stateManager;
    std::shared_ptr<HMState> m_currentState;
    bool m_wakeup;
};

#endif /* HMEVENTQUEUE_H_ */
