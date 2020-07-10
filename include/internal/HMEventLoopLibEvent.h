// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HMEVENTLOOPLIBEVENT_H_
#define INCLUDE_HMEVENTLOOPLIBEVENT_H_

#include <string>
#include <mutex>
#include <condition_variable>
#include <event2/event.h>
#include <event2/thread.h>
#include <event2/event.h>
#include <event2/dns.h>
#include <event2/dns_struct.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "HMEventLoop.h"
#include "HMState.h"
#include "HMIPAddress.h"
#include "HMDataHostCheck.h"
#include "HMTimeStamp.h"

#define CERTS "/etc/ssl/certs/ca-certificates.crt"

class HMStateManager;

//! Types of events that can be sceduled inside lib event.
enum EVENT_TYPES
{
    EVENT_TIMEOUT,
    EVENT_HTTP,
    EVENT_DNS
};

//! Class to hold a DNS timeout within libevent
/*!
     Class to hold a DNS timeout within libevent.
     Contains all the data needed to schedule a DNS lookup work request.
 */
class DNSTimeout
{
public:
    DNSTimeout(const std::string& host, const HMDNSLookup& dnshostCheck) :
        m_hostname(host),
        m_dnsHostCheck(dnshostCheck),
        m_ev(nullptr),
        m_stateManager(nullptr) {}

    std::string m_hostname;
    HMDNSLookup m_dnsHostCheck;
    struct event* m_ev;
    HMStateManager* m_stateManager;
};

//! Class to hold a Remote timeout within libevent
/*!
     Class to hold a Remote timeout within libevent.
     Contains all the data needed to schedule a Remote check work request.
 */
class RemoteTimeout
{
public:
    RemoteTimeout(const std::string& host) :
        m_hostGroupName(host),
        m_ev(nullptr),
        m_stateManager(nullptr) {}

    std::string m_hostGroupName;
    struct event* m_ev;
    HMStateManager* m_stateManager;
};


//! Class to hold a Remote host timeout within libevent
/*!
     Class to hold a Remote host timeout within libevent.
     Contains all the data needed to schedule a Remote host check work request.
 */
class RemoteHostTimeout
{
public:
    RemoteHostTimeout(const std::string& host, const HMDataHostCheck& check) :
        m_hostName(host),
        m_hostCheck(check),
        m_ev(nullptr),
        m_stateManager(nullptr) {}

    std::string m_hostName;
    HMDataHostCheck m_hostCheck;
    struct event* m_ev;
    HMStateManager* m_stateManager;
};
//! Class to hold a health check timeout within libevent
/*!
     Class to hold a health check timeout within libevent.
     Contains all the data needed to schedule a health check work request.
 */
class HealthCheckTimeout
{
public:
    HealthCheckTimeout(const std::string& host, const HMIPAddress& address, const HMDataHostCheck& check) :
        m_hostname(host),
        m_hostCheck(check),
        m_address(address),
        m_ev(nullptr),
        m_stateManager(nullptr) {}

    std::string m_hostname;
    HMDataHostCheck m_hostCheck;
    HMIPAddress m_address;
    struct event* m_ev;
    HMStateManager* m_stateManager;
};

//! Event loop implementation using the LibEvent framework.
class HMEventLoopLibEvent : public HMEventLoop
{
public:

    HMEventLoopLibEvent(HMStateManager* stateManager);

    ~HMEventLoopLibEvent();

    //! Add a new DNS timeout.
    /*!
         Add a new DNS timeout to the event loop.
         \param the hostname to resolve.
         \param structure holding DNS type and address type(v4 or v6).
         \param the time Stamp of when the DNS resolution should take place.
     */
    void addDNSTimeout(const std::string& hostname, const HMDNSLookup& dnsHostCheck, HMTimeStamp timeStamp);

    //! Add a new Remote timeout.
    /*!
         Add a new Remote timeout to the event loop.
         \param the hostgroupname to remote check.
         \param the time Stamp of when the Remote check should take place.
     */
    void addRemoteTimeout(const std::string& hostGroupName, HMTimeStamp timeStamp);

    //! Add a new Remote host timeout.
    /*!
         Add a new Remote timeout to the event loop.
         \param the hostname to remote check.
         \param the time Stamp of when the Remote check should take place.
     */
    void addRemoteHostTimeout(const std::string& hostname, const HMDataHostCheck& dataHostCheck, HMTimeStamp timeStamp);

    //! Add a new health check timeout.
    /*!
         Add a new health check timeout to the event loop.
         \param the hostname to health check.
         \param the IP address to health check.
         \param the host check to conduct.
         \param the time stamp of when the health check should take place.
     */
    void addHealthCheckTimeout(const std::string& hostname, const HMIPAddress& address, const HMDataHostCheck hostCheck, HMTimeStamp timeStamp);

    //! Callback function when a DNS timeout fires.
    /*!
         Callback when a DNS timeout fires. Either schedule another timeout or a DNS resolution work.
         \param unused.
         \param unused.
         \param a pointer to a DNSTimeout class.
     */
    static void handleDNSTimeout(evutil_socket_t fd, short what, void* arg);

    //! Callback function when a Remote timeout fires.
    /*!
         Callback when a Remote timeout fires. Either schedule another timeout or a Remote check work.
         \param unused.
         \param unused.
         \param a pointer to a RemoteTimeout class.
     */
    static void handleRemoteTimeout(evutil_socket_t fd, short what, void* arg);

    //! Callback function when a Remote Host timeout fires.
    /*!
         Callback when a Remote host timeout fires. Either schedule another timeout or a Remote check work.
         \param unused.
         \param unused.
         \param a pointer to a RemoteHostTimeout class.
     */
    static void handleRemoteHostTimeout(evutil_socket_t fd, short what, void* arg);


    //! Callback function when a health check timeout fires.
    /*!
         Callback when a health check timeout fires. Either schedule another timeout or a health check work.
         \param unused.
         \param unused.
         \param a pointer to a HealthCheckTimeout class.
     */
    static void handleHealthCheckTimeout(evutil_socket_t fd, short what, void* arg);

    //! Get the event base to use for health checks through libevent
    /*!
         Get the event base to use for health checks through libevent.
         Can be used for HTTP/HTTPS health checking.
         \return an event base suitable for the HTTP/HTTPS library.
     */
    struct event_base* getEventBase() { return m_base; }

    //! Get the event base to use for DNS resolution through libevent
    /*!
         Get the event base to use for DNS resolution through libevent.
         Can be used for DNS resolution.
         \return an event base suitable for the DNS resolution library.
     */
    struct evdns_base* getDNSBase() { return m_dnsBase; }

    //! Function to retrieve the SSL store used in the libevent http library.
    /*!
         Function to retrieve the SSL store used in the libevent http library.
         \return the SSL_CTX pointer to the certificate store.
     */
    SSL_CTX* getSSLCertStore() { return m_ssl_ctx; }

    //! Get the number of events in the event loop.
    /*!
         Get the number of events in the event loop.
         \return the number of events in the event loop.
     */
    uint64_t getTimeOutQueueSize() { return m_timeoutEvents; }

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

    //! Init and startup the thread for the event loop.
    /*!
         Init and startup the thread for the event loop.
     */
    void runThread();

private:

    bool m_keepRunning;

    uint64_t m_timeoutEvents;

    HMStateManager* m_stateManager;
    std::shared_ptr<HMState> m_currentState;

    struct event_base* m_base;
    struct evdns_base* m_dnsBase;

    SSL_CTX* m_ssl_ctx;

    std::condition_variable m_requestReadyCond;
    std::mutex m_requestReadyLock;

    //! The internal run function.
    /*!
         This function should run the event loop in a new thread.
     */
    void run();
};



#endif /* INCLUDE_HMEVENTLOOPLIBEVENT_H_ */
