// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifdef USE_ARES
#include <ares.h>
#endif
#include <cstring>

#include "HMWork.h"
#include "HMEventLoopQueue.h"
#include "HMStateManager.h"
#include "HMLogBase.h"

using namespace std;

void HMWorkState::reloadState(HMStateManager* stateManager, uint64_t threadID)
{
#ifdef USE_ARES
    shared_ptr<HMState> currentState;
    stateManager->updateState(currentState);
    HMIPAddress dnsServer;
    struct ares_options opts;
    opts.flags = 0;
    opts.timeout = currentState->getDNSLookupTimeout() / 1000;
    opts.tries = currentState->getDNSRetries();

    if (ares_init_options(&m_channel, &opts,
            ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES | ARES_OPT_FLAGS) != ARES_SUCCESS)
    {
        HMLog(HM_LOG_CRITICAL, "[WORKER] [%llu] Ares Init Error", threadID); // LCOV_EXCL_LINE : This cannot be reached in testing
    }

    m_aresLoaded = true;

    if (currentState->getDNSAddress(dnsServer))
    {
        struct ares_addr_node addrs;

        if (dnsServer.getType() == AF_INET)
        {
            addrs.next = nullptr;
            addrs.family = AF_INET;
            addrs.addr.addr4.s_addr = dnsServer.addr4();
            ares_set_servers(m_channel, &addrs);
        }
        else if (dnsServer.getType() == AF_INET6)
        {
            addrs.next = nullptr;
            addrs.family = AF_INET6;
            in6_addr ipv6addr = dnsServer.addr6();
            memcpy(&addrs.addr.addr6, &ipv6addr, sizeof(addrs.addr.addr6));
            ares_set_servers(m_channel, &addrs);
        }
    }
#else
    (void)stateManager;
    (void)threadID;
#endif
}

void HMWorkState::destroyAres()
{
#ifdef USE_ARES
    if(m_aresLoaded)
    {
        ares_destroy(m_channel);
    }
#endif
}

void HMWork::updateState(HMStateManager* stateManager, HMEventLoop* eventLoop)
{
    // Update the smart pointer if necessary
    m_stateManager = stateManager;
    m_eventLoop = eventLoop;
}

void HMWork::setMark(int mark)
{
    m_mark = mark;
}

void HMWork::setReschedule(bool reschedule)
{
    m_reschedule = reschedule;
}

void HMWork::setStoreResults(bool storeResults)
{
    m_storeResults = storeResults;
}

int HMWork::getMark() const
{
    return m_mark;
}

void HMWork::setPublish(bool publish)
{
    m_publish = publish;
}
