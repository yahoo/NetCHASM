// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMNetCHASMAPI.h"
#include "HMStateManager.h"

using namespace std;

shared_ptr<HMNetCHASMAPI>
HMNetCHASMAPI::createHealthCheckInstance(string loglevel, const string masterConfig)
{
    HM_LOG_LEVEL level = HMLogBase::parseLogLevel(loglevel);
    HM_LOG_LEVEL logLevel = (level == HM_ERROR)?HM_LOG_ERROR:level;
    shared_ptr<HMNetCHASMAPI> api = make_shared<HMNetCHASMAPI>();
    if (api)
    {
        api->m_monitor = make_shared<HMStateManager>();
        if (api->m_monitor)
        {
            api->m_thread = thread([api, logLevel, masterConfig]()
            {
                api->m_monitor->healthCheck(masterConfig, logLevel);
            });
            return api;
        }
    }
    return nullptr;
}

bool HMNetCHASMAPI::isAlive()
{
    return m_monitor->isActive();
}

void HMNetCHASMAPI::stopHealthCheckInstance()
{
    if(m_monitor)
    {
        m_monitor->shutdown();
        m_thread.join();
    }
}
