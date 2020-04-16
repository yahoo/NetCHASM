// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HMEVENTLOOP_H_
#define INCLUDE_HMEVENTLOOP_H_

#include <string>
#include <thread>

#include "HMDataHostCheck.h"
#include "HMIPAddress.h"
#include "HMTimeStamp.h"
#include "HMDNSCache.h"
//! The Base class for the Event Loop.
/*!
     The Base class for the Event Loop.
     Derived classes should implement the following functions:
     runThread - called to init and start the event loop.
     shutDown - called to shutdown and cleanup.
     wakeupTracker - called to force the event loop to check to make sure the next entry did not change.
     getTimeoutQueueSize - returns the number of events in the loop.
     addDNTimeout - deals with scheduling a DNS lookup timeout.
     addHealthCheckTimeout - deals with scheduling a health check timeout.
     run - the main run code
 */
class HMEventLoop
{
public:
    virtual ~HMEventLoop(){};

    //! Init and startup the thread for the event loop.
    /*!
         Init and startup the thread for the event loop.
     */
    virtual void runThread() = 0;

    //! Shutdown the event loop and cleanup.
    /*!
         Shutdown the event loop and cleanup.
     */
    virtual void shutDown() = 0;

    //! Wakeup the tracker.
    /*!
         Wakeup the tracker. Called if the new event timeout is earlier than the latest or the tracker needs  to check for shutdown condition.
     */
    virtual void wakeupTracker() = 0;

    //! Get the number of events in the event loop.
    /*!
         Get the number of events in the event loop.
         \return the number of events in the event loop.
     */
    virtual uint64_t getTimeOutQueueSize() = 0;

    //! Add a new DNS timeout.
    /*!
         Add a new DNS timeout to the event loop.
         \param the hostname to resolve.
         \param structure holding DNS type and address type(v4 or v6).
         \param the time Stamp of when the DNS resolution should take place.
     */
    virtual void addDNSTimeout(const std::string& hostname, const HMDNSLookup& dnsHostCheck, HMTimeStamp timeStamp) = 0;

    //! Add a new Remote timeout.
    /*!
         Add a new Remote timeout to the event loop.
         \param the hostgroupname to remote check.
         \param the time Stamp of when the Remote check should take place.
     */
    virtual void addRemoteTimeout(const std::string& hostname, HMTimeStamp timeStamp) = 0;

    //! Add a new Remote host timeout.
    /*!
         Add a new Remote timeout to the event loop.
         \param the hostname to remote check.
         \param the time Stamp of when the Remote check should take place.
     */
    virtual void addRemoteHostTimeout(const std::string& hostname, const HMDataHostCheck& dataHostCheck, HMTimeStamp timeStamp) = 0;

    //! Add a new health check timeout.
    /*!
         Add a new health check timeout to the event loop.
         \param the hostname to health check.
         \param the IP address to health check.
         \param the host check to conduct.
         \param the time stamp of when the health check should take place.
     */
    virtual void addHealthCheckTimeout(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck hostCheck,
            HMTimeStamp timeStamp) = 0;
protected:

    //! The internal run function.
    /*!
         This function should run the event loop in a new thread.
     */
    virtual void run() = 0;
    std::thread m_thread;
};



#endif /* INCLUDE_HMEVENT_H_ */
