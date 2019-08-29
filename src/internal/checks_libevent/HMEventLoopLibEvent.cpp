// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "HMEventLoopLibEvent.h"
#include "HMStateManager.h"
#include "HMLogBase.h"

using namespace std;

// Deal with using the C++ standard for locking
/** XXX: implement handle recursive locking! */
static void*
cxx_lock_alloc(unsigned /*locktype*/)
{
    return reinterpret_cast<void *>(new std::recursive_mutex);
}

static void
cxx_lock_free(void *lock_, unsigned /*locktype*/)
{
    std::recursive_mutex *m = reinterpret_cast<std::recursive_mutex *>(lock_);
    delete m;
}

static int
cxx_lock_lock(unsigned /*mode*/, void *lock_)
{
    std::recursive_mutex *m = reinterpret_cast<std::recursive_mutex *>(lock_);
    m->lock();
    return 0;
}

static int
cxx_lock_unlock(unsigned /*mode*/, void *lock_)
{
    std::recursive_mutex *m = reinterpret_cast<std::recursive_mutex *>(lock_);
    m->unlock();
    return 0;
}

static unsigned long
cxx_get_id(void)
{
    // Note this is technically not guaranteed to be correct....we might want to drop to pthread calls
    return (unsigned long)std::hash<std::thread::id>()(std::this_thread::get_id());
}

static void*
cxx_cond_alloc(unsigned condflags)
{
    (void) condflags;
    return reinterpret_cast<void *>(new std::condition_variable_any);
}

static void
cxx_cond_free(void* cond_)
{
    condition_variable_any* cond = reinterpret_cast<std::condition_variable_any*>(cond_);
    delete cond;
}

static int
cxx_cond_signal(void *cond_, int broadcast)
{
    condition_variable_any* cond = reinterpret_cast<std::condition_variable_any*>(cond_);
    if (broadcast)
    {
        cond->notify_all();
    }
    else
    {
        cond->notify_one();
    }
    return 0;
}

static int
cxx_cond_wait(void *cond_, void *lock_, const struct timeval *tv)
{
    cv_status r;
    condition_variable_any* cond = reinterpret_cast<std::condition_variable_any*>(cond_);
    std::recursive_mutex *m = reinterpret_cast<std::recursive_mutex *>(lock_);
    auto lck = std::unique_lock<std::recursive_mutex> (*m);

    if (tv)
    {
        HMTimeStamp ts;
        ts.setTime(tv->tv_sec * 1000 + tv->tv_usec);
        ts = ts + HMTimeStamp::now();

        r = cond->wait_until(lck, ts.m_timeStamp);
        if (r == cv_status::timeout)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        cond->wait(lck);
        return 0;
    }
}

static int
use_cxx_threads(void)
{
    struct evthread_lock_callbacks cbs =
    {
            EVTHREAD_LOCK_API_VERSION,
            EVTHREAD_LOCKTYPE_RECURSIVE,
            cxx_lock_alloc,
            cxx_lock_free,
            cxx_lock_lock,
            cxx_lock_unlock,
    };

    struct evthread_condition_callbacks cond_cbs =
    {
            EVTHREAD_CONDITION_API_VERSION,
            cxx_cond_alloc,
            cxx_cond_free,
            cxx_cond_signal,
            cxx_cond_wait
    };

    evthread_set_lock_callbacks(&cbs);
    evthread_set_condition_callbacks(&cond_cbs);
    evthread_set_id_callback(cxx_get_id);

    return 0;
}

HMEventLoopLibEvent::HMEventLoopLibEvent(HMStateManager* stateManager) :
                m_keepRunning(true),
                m_timeoutEvents(0),
                m_base(nullptr),
                m_dnsBase(nullptr),
                m_ssl_ctx(nullptr)
{
    use_cxx_threads();
    m_base = event_base_new();
    m_stateManager = stateManager;
    m_dnsBase = (m_base == nullptr) ? nullptr : evdns_base_new(m_base,EVDNS_BASE_INITIALIZE_NAMESERVERS | EVDNS_BASE_DISABLE_WHEN_INACTIVE);

    m_ssl_ctx = SSL_CTX_new(SSLv23_method());
    if(!m_ssl_ctx)
    {
        HMLog(HM_LOG_ERROR, "Open SSL Error initializing the cert struct");
        return;
    }
    if(SSL_CTX_load_verify_locations(m_ssl_ctx, CERTS, NULL) != 1)
    {
        HMLog(HM_LOG_ERROR, "Open SSL Error initializing the cert store");
        return;
    }
    SSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_PEER, NULL);
}

HMEventLoopLibEvent::~HMEventLoopLibEvent()
{
    if(m_ssl_ctx)
    {
        SSL_CTX_free(m_ssl_ctx);
    }

    if(m_dnsBase)
    {
        evdns_base_free(m_dnsBase, 0);
    }

    event_base_free(m_base);
}

void
HMEventLoopLibEvent::addDNSTimeout(const string& hostname, const HMDNSLookup& dnsHostCheck, HMTimeStamp timeStamp)
{
    DNSTimeout* data = new DNSTimeout(hostname, dnsHostCheck);
    //struct event* ev = event_new(m_base, -1, 0, &HMEventLoopLibEvent::handleDNSTimeout, data);
    struct event* ev =  evtimer_new(m_base, &HMEventLoopLibEvent::handleDNSTimeout, data);
    struct timeval tv = timeStamp.getTimeout();
    HMLog(HM_LOG_DEBUG3, "[EVENT] Adding DNS timeout %d.%d for %s", tv.tv_sec, tv.tv_usec,hostname.c_str());
    data->m_ev = ev;
    data->m_stateManager = m_stateManager;
    event_add(ev,&tv);

    wakeupTracker();
}

void
HMEventLoopLibEvent::addHealthCheckTimeout(const string& hostname, const HMIPAddress& address, const HMDataHostCheck hostCheck, HMTimeStamp timeStamp)
{
    HealthCheckTimeout* data = new HealthCheckTimeout(hostname, address, hostCheck);
    //struct event* ev = event_new(m_base, -1, 0, &HMEventLoopLibEvent::handleHealthCheckTimeout, data);
    struct event* ev =  evtimer_new(m_base, &HMEventLoopLibEvent::handleHealthCheckTimeout, data);

    struct timeval tv = timeStamp.getTimeout();
    HMLog(HM_LOG_DEBUG3, "[EVENT] Adding HealthCheck timeout %d.%d for %s", tv.tv_sec, tv.tv_usec, hostname.c_str());
    data->m_ev = ev;
    data->m_stateManager = m_stateManager;
    event_add(ev,&tv);

    wakeupTracker();
}

// static
void
HMEventLoopLibEvent::handleDNSTimeout(evutil_socket_t fd, short what, void* arg)
{
    (void) fd;
    (void) what;
    DNSTimeout* data = (DNSTimeout*)arg;
    HMStateManager* state = data->m_stateManager;
    std::shared_ptr<HMState> currentState;

    HMEventLoopLibEvent* event = state->getLibEvent();
    if(event == nullptr)
    {
        HMLog(HM_LOG_ERROR, "[EVENT] LibEvent pointer null failing");
        return;
    }

    state->updateState(currentState);
    HM_SCHEDULE_STATE check_state;

    HMLog(HM_LOG_DEBUG, "[EVENT] DNS Entry Timeout for %s",
            data->m_hostname.c_str());
    check_state = currentState->m_dnsCache.queryNeeded(
           data->m_hostname, data->m_dnsHostCheck);
    if (check_state == HM_SCHEDULE_WORK)
    {
        HMLog(HM_LOG_DEBUG3,
                "[DEBUG] DNS Health Check Schedule work for %s",
                data->m_hostname.c_str());
        currentState->m_dnsCache.queueDNSQuery(data->m_hostname,
                data->m_dnsHostCheck,
                state->m_workQueue);
    }
    else if (check_state == HM_SCHEDULE_EVENT)
    {
        HMLog(HM_LOG_DEBUG3,
                "[DEBUG] DNS Health Check Schedule event for %s",
                data->m_hostname.c_str());
        HMDataHostCheck temp;
        HMTimeStamp nextCheckTimeOut = currentState->m_checkList.nextCheckTime(data->m_hostname, HMIPAddress(), temp);
        event->addDNSTimeout(data->m_hostname, data->m_dnsHostCheck, nextCheckTimeOut);
    }

    event_del(data->m_ev);
    delete data;
}

// static
void
HMEventLoopLibEvent::handleHealthCheckTimeout(evutil_socket_t fd, short what, void* arg)
{
    (void) fd;
    (void) what;
    HealthCheckTimeout* data = (HealthCheckTimeout*)arg;
    HMStateManager* state = data->m_stateManager;
    std::shared_ptr<HMState> currentState;

    HMEventLoopLibEvent* event = state->getLibEvent();
    if(event == nullptr)
    {
        HMLog(HM_LOG_ERROR, "[EVENT] LibEvent pointer null failing");
        return;
    }

    state->updateState(currentState);
    HM_SCHEDULE_STATE check_state;

    HMLog(HM_LOG_DEBUG, "[EVENT] Health Check Timeout for %s",
                        data->m_hostname.c_str());

    check_state = currentState->m_checkList.checkNeeded(data->m_hostname,
            data->m_address, data->m_hostCheck);
    if (check_state == HM_SCHEDULE_WORK)
    {
        HMLog(HM_LOG_DEBUG3, "[DEBUG] Health Check Schedule work for %s",
                data->m_hostname.c_str());
        currentState->m_checkList.queueCheck(data->m_hostname,
                data->m_address, data->m_hostCheck,
                state->m_workQueue);
    }
    else if (check_state == HM_SCHEDULE_EVENT)
    {
        HMLog(HM_LOG_DEBUG3, "[DEBUG] Health Check Schedule event for %s",
                data->m_hostname.c_str());
        HMTimeStamp nextCheckTimeOut = currentState->m_checkList.nextCheckTime(data->m_hostname, data->m_address, data->m_hostCheck);
        event->addHealthCheckTimeout(data->m_hostname, data->m_address, data->m_hostCheck, nextCheckTimeOut);
    }

    event_del(data->m_ev);
    delete data;
}

void
HMEventLoopLibEvent::shutDown()
{

    m_keepRunning = false;
    wakeupTracker();
    event_base_loopbreak(m_base);
    m_thread.join();
}

void
HMEventLoopLibEvent::runThread()
{
    m_keepRunning = true;
    m_thread = std::thread(&HMEventLoopLibEvent::run, this);
}

void
HMEventLoopLibEvent::run()
{
    std::unique_lock<std::mutex> lk(m_requestReadyLock, std::defer_lock);
    while(m_keepRunning)
    {
        //cout << "Starting event dispatch" << endl;
        event_base_loop(m_base, EVLOOP_ONCE);

        // wait for new data
        //cout << "Starting condition wait" << endl;
        //lk.lock();
        //m_requestReadyCond.wait(lk);
        //lk.unlock();
    }
}

void
HMEventLoopLibEvent::wakeupTracker()
{
    HMLockGuard<std::mutex> lg(m_requestReadyLock);
    m_requestReadyCond.notify_one();
}
