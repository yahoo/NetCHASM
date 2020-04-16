// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include "HMEventLoop.h"
#include "HMWorkQueue.h"
#include "HMConstants.h"
#include "HMWork.h"
#include "HMWorkRemoteCheck.h"
#include "HMWorkRemoteHostCheckRemote.h"
#include "HMLogBase.h"
#include "HMStorage.h"
#include "HMRemoteHostCache.h"

using namespace std;

bool
HMRemoteHostCache::init()
{
    return true;
}

void
HMRemoteHostCache::insertRemoteEntry(const string& name, const HMDataHostCheck& dataHostCheck, uint64_t ttl, uint64_t timeout)
{
    auto key = make_pair(name, dataHostCheck);
    ttl = (ttl == 0) ? HM_DEFAULT_DNS_TTL : ttl;
    timeout = (timeout == 0) ? HM_DEFAULT_DNS_RESOLUTION_TIMEOUT : timeout;

    auto result = m_cache.insert(make_pair(key, HMRemoteResult(ttl, timeout)));
    if(!result.second)
    {
        result.first->second.updateTimeouts(ttl, timeout);
    }
}

void
HMRemoteHostCache::finishCheck(string name,  const HMDataHostCheck& dataHostCheck, bool success)
{
    auto key = make_pair(name, dataHostCheck);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing remote entry in cache in function finishquery for host %s",
                name.c_str());
        return;

    }
    it->second.finishCheck(success);
}

bool
HMRemoteHostCache::getRemoteResult(const string& name, const HMDataHostCheck& dataHostCheck, map<pair<string, HMDataHostCheck>,HMRemoteResult>::const_iterator& result) const
{
    auto key = make_pair(name, dataHostCheck);
    auto it = m_cache.find(key);
    if(it != m_cache.end())
    {
        result = it;
        return true;
    }
    return false;
}

HM_SCHEDULE_STATE
HMRemoteHostCache::checkNeeded(const string& name, const HMDataHostCheck& dataHostCheck) const
{
    HM_SCHEDULE_STATE result = HM_SCHEDULE_EVENT;
    bool alreadyScheduled = true;
    bool isCheckTimeChanged = false;
    HMTimeStamp nextCheck = HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    auto key = make_pair(name, dataHostCheck);
    auto ret = m_cache.find(key);
    if(ret != m_cache.end())
    {
        HMTimeStamp checkTime = ret->second.nextCheckTime();
        HM_WORK_STATE query_state = ret->second.getCheckState();
        if ((query_state == HM_CHECK_FAILED)
                || (query_state == HM_CHECK_INACTIVE))
        {
            alreadyScheduled = false;
            if (checkTime < nextCheck)
            {
                isCheckTimeChanged = true;
                nextCheck = checkTime;
            }
        }
    }
    else
    {
        HMLog(HM_LOG_NOTICE,
                "Missing Remote entry in cache in function checkNeeded for host %s",
                name.c_str());
    }
    if (alreadyScheduled || isCheckTimeChanged)
    {
        result = HM_SCHEDULE_IGNORE;
    }
    if (nextCheck <= HMTimeStamp::now())
    {
        result = HM_SCHEDULE_WORK;
    }
    return result;
}

HMTimeStamp
HMRemoteHostCache::nextCheckTime(const string& name, const HMDataHostCheck& dataHostCheck) const
{
    auto key = make_pair(name, dataHostCheck);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_ERROR,
                        "Missing remote entry in cache in function nextCheckTime for host %s",
                        name.c_str());
        return HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    }
    return it->second.nextCheckTime();
}

bool
HMRemoteHostCache::startRemoteCheck(const string& name, const HMDataHostCheck& dataHostCheck)
{
    auto key = make_pair(name, dataHostCheck);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing Remote entry in cache in function startRemoteQuery for host %s",
                name.c_str());
        return false;
    }
    it->second.startCheck();
    return true;
}

bool
HMRemoteHostCache::queueRemoteCheck(string name, const HMDataHostCheck& dataHostCheck, HMWorkQueue& queue)
{
    HMLog(HM_LOG_DEBUG, "[DEBUG] Remote  Check QueueCheck for %s",
            name.c_str());
    auto key = make_pair(name, dataHostCheck);
    auto it = m_cache.find(key);

    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing Remote entry in cache in function queueRemtoeQuery for host %s",
                name.c_str());
        return false;
    }
    unique_ptr<HMWork> remotelookup;
    switch(dataHostCheck.getRemoteCheckType())
    {
        case HM_REMOTE_CHECK_TCP:
        case HM_REMOTE_CHECK_TCPS:
        case HM_REMOTE_SHARED_CHECK_TCP:
        case HM_REMOTE_SHARED_CHECK_TCPS:
            remotelookup = make_unique<HMWorkRemoteHostCheckRemote>(HMWorkRemoteHostCheckRemote(name,
                        HMIPAddress(AF_INET), dataHostCheck));
            break;
        default:
            HMLog(HM_LOG_ERROR, "% remote check type currently not supported", printRemoteCheckType(dataHostCheck.getRemoteCheckType()));
            return false;
    }
    remotelookup->m_start = HMTimeStamp::now();
    remotelookup->m_end = HMTimeStamp::now() + it->second.getCheckTTL();
    it->second.queueCheck();
    queue.insertWork(remotelookup);
    return true;
}

void
HMRemoteHostCache::queueRemoteLookups(HMWorkQueue& queue, HMEventLoop& eventLoop, bool restart)
{
    for(auto it = m_cache.begin(); it != m_cache.end(); ++it)
    {
        if(it->first.second.getFlowType() != HM_FLOW_REMOTE_HOST_TYPE)
        {
            continue;
        }
        // Check for a DNS check
        unique_ptr<HMWork> remotelookup;
        HM_SCHEDULE_STATE state = this->checkNeeded(it->first.first, it->first.second);
        if(state == HM_SCHEDULE_EVENT || state == HM_SCHEDULE_WORK)
        {
            // if there is no active query, then we add one
            switch (it->first.second.getRemoteCheckType())
            {
                case HM_REMOTE_CHECK_TCP:
                case HM_REMOTE_CHECK_TCPS:
                case HM_REMOTE_SHARED_CHECK_TCP:
                case HM_REMOTE_SHARED_CHECK_TCPS:
                    remotelookup = make_unique<HMWorkRemoteHostCheckRemote>(HMWorkRemoteHostCheckRemote(it->first.first,
                                HMIPAddress(AF_INET), it->first.second));
                    break;
                default:
                    HMLog(HM_LOG_ERROR, "% remote check type currently not supported", printRemoteCheckType(it->first.second.getRemoteCheckType()));
                    continue;

            }
            remotelookup->m_start = HMTimeStamp::now();
            remotelookup->m_end = HMTimeStamp::now() + it->second.getCheckTTL();
            it->second.queueCheck();
            queue.insertWork(remotelookup);
        }
        else if(restart && (state == HM_SCHEDULE_IGNORE))
        {
            HMTimeStamp checkTime = nextCheckTime(it->first.first, it->first.second);
            eventLoop.addRemoteHostTimeout(it->first.first, it->first.second, checkTime);
        }
    }
}

void
HMRemoteHostCache::updateResultTime(const string& name, const HMDataHostCheck& dataHostCheck, const HMTimeStamp& resultTime)
{
    auto key = make_pair(name, dataHostCheck);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing remote entry in cache in function finishquery for host %s",
                name.c_str());
        return;

    }
    it->second.setResultTime(resultTime);
}
