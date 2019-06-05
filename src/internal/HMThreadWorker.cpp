// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <signal.h>
#include <cstring>
#include <sstream>

#include "HMThreadWorker.h"
#include "HMWork.h"
#include "HMWorkDNSLookup.h"
#include "HMWorkDNSLookupAres.h"
#include "HMWorkHealthCheck.h"
#include "HMWorkHealthCheckCurl.h"
#include "HMLogBase.h"
#include "HMEventLoopQueue.h"
#include "HMState.h"

using namespace std;

void
HMThreadWorker::runThread()
{
    signal(SIGPIPE, SIG_IGN);

    stringstream ss;
    ss << this_thread::get_id();
    m_threadID = stoull(ss.str());
    m_running = true;
    m_shutdown = false;
    m_idle = true;

    m_workState.reloadState(m_stateManager, m_threadID);
    while(!m_shutdown)
    {
        // check for work
        if(m_stateManager->m_workQueue.getWork(m_order, m_shutdown))
        {
            m_idle = false;
            m_nUsed+=1;
            m_order->init(m_workState);
            m_order->updateState(m_stateManager, m_eventLoop);
            HM_WORK_STATUS result = m_order->processWork();
            if(result == HM_WORK_IN_PROGRESS)
            {
                m_stateManager->m_workQueue.insertMap(m_order);
            }
            m_idle = true;
        }
    }
    if(m_workState.m_aresLoaded)
    {
        ares_destroy(m_workState.m_channel);
    }
}

void
HMThreadWorker::shutDown()
{
    m_shutdown = true;
}

bool
HMThreadWorker::isIdle()
{
    return m_idle;
}

uint64_t
HMThreadWorker::getUsedCounter()
{
    return m_nUsed;
}

