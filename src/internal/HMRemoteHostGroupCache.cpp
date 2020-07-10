// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include <chrono>

#include "HMEventLoop.h"
#include "HMWorkQueue.h"
#include "HMConstants.h"
#include "HMWork.h"
#include "HMWorkRemoteCheck.h"
#include "HMLogBase.h"
#include "HMStorage.h"
#include "HMRemoteHostGroupCache.h"
#include "HMWorkRemoteCheckRemote.h"

using namespace std;

bool
HMRemoteHostGroupCache::init()
{
    return true;
}

void
HMRemoteHostGroupCache::insertRemoteEntry(const string& name, uint64_t ttl, uint64_t timeout)
{
    auto key = name;
    ttl = (ttl == 0) ? HM_DEFAULT_DNS_TTL : ttl;
    timeout = (timeout == 0) ? HM_DEFAULT_DNS_RESOLUTION_TIMEOUT : timeout;

    auto result = m_cache.insert(make_pair(key, HMRemoteResult(ttl, timeout)));
    if(!result.second)
    {
        result.first->second.updateTimeouts(ttl, timeout);
    }
}

void
HMRemoteHostGroupCache::finishCheck(string name, bool success)
{
    auto key = name;
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing remote entry in cache in function finishquery for hostgroup %s",
                name.c_str());
        return;

    }
    it->second.finishCheck(success);
}

bool
HMRemoteHostGroupCache::getRemoteResult(const string& name, map<string,HMRemoteResult>::const_iterator& result) const
{
    auto key = name;
    auto it = m_cache.find(key);
    if(it != m_cache.end())
    {
        result = it;
        return true;
    }
    return false;
}

HM_SCHEDULE_STATE
HMRemoteHostGroupCache::checkNeeded(const string& name) const
{
    HM_SCHEDULE_STATE result = HM_SCHEDULE_EVENT;
    bool alreadyScheduled = true;
    bool isCheckTimeChanged = false;
    HMTimeStamp nextCheck = HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    auto key = name;
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
                "Missing Remote entry in cache in function checkNeeded for hostgroup %s",
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
HMRemoteHostGroupCache::nextCheckTime(const string& name) const
{
    auto key = name;
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_ERROR,
                        "Missing remote entry in cache in function nextCheckTime for hostgroup %s",
                        name.c_str());
        return HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    }
    return it->second.nextCheckTime();
}

bool
HMRemoteHostGroupCache::startRemoteCheck(const string& name)
{
    auto key = name;
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing Remote entry in cache in function startRemoteQuery for hostgroup %s",
                name.c_str());
        return false;
    }
    it->second.startCheck();
    return true;
}

bool
HMRemoteHostGroupCache::queueRemoteCheck(string name, HMWorkQueue& queue, const HMDataHostGroupMap& hostGroupMap)
{
    HMLog(HM_LOG_DEBUG, "[DEBUG] Remote  Check QueueCheck for %s",
            name.c_str());
    auto key = name;
    auto it = m_cache.find(key);

    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing Remote entry in cache in function queueRemtoeQuery for hostgroup %s",
                name.c_str());
        return false;
    }
    auto hostGroupIt = hostGroupMap.find(it->first);
    if (hostGroupIt == hostGroupMap.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing Remote entry in hostgroup map in function queueRemoteLookups for hostgroup %s",
                it->first.c_str());
        return false;
    }
    HMDataHostCheck datahostCheck;
    hostGroupIt->second.getHostCheck(datahostCheck);
    unique_ptr<HMWork> remotelookup;
    switch(hostGroupIt->second.getRemoteCheckType())
    {
        case HM_REMOTE_CHECK_TCP:
        case HM_REMOTE_CHECK_TCPS:
        case HM_REMOTE_SHARED_CHECK_TCP:
        case HM_REMOTE_SHARED_CHECK_TCPS:
            remotelookup = make_unique<HMWorkRemoteCheckRemote>(HMWorkRemoteCheckRemote(it->first,
                        HMIPAddress(AF_INET), datahostCheck, hostGroupIt->second));
            break;
        default:
            HMLog(HM_LOG_ERROR, "% remote check type currently not supported", printRemoteCheckType(hostGroupIt->second.getRemoteCheckType()));
            return false;
    }
    remotelookup->m_start = HMTimeStamp::now();
    remotelookup->m_end = HMTimeStamp::now() + it->second.getCheckTTL();
    it->second.queueCheck();
    queue.insertWork(remotelookup);
    return true;
}

void
HMRemoteHostGroupCache::queueRemoteLookups(HMWorkQueue& queue, HMEventLoop& eventLoop, const HMDataHostGroupMap& hostGroupMap, bool restart)
{
    for(auto it = m_cache.begin(); it != m_cache.end(); ++it)
    {
        auto hostGroupIt = hostGroupMap.find(it->first);
        if(hostGroupIt == hostGroupMap.end())
        {
            HMLog(HM_LOG_NOTICE,
                    "Missing Remote entry in hostgroup map in function queueRemoteLookups for hostgroup %s",
                    it->first.c_str());
            return;
        }
        if(hostGroupIt->second.getFlowType() != HM_FLOW_REMOTE_HOSTGROUP_TYPE)
        {
            continue;
        }
        HMDataHostCheck datahostCheck;
        hostGroupIt->second.getHostCheck(datahostCheck);
        // Check for a DNS check
        unique_ptr<HMWork> remotelookup;
        HM_SCHEDULE_STATE state = this->checkNeeded(it->first);
        if(state == HM_SCHEDULE_EVENT || state == HM_SCHEDULE_WORK)
        {
            // if there is no active query, then we add one
            switch (hostGroupIt->second.getRemoteCheckType())
            {
                case HM_REMOTE_CHECK_TCP:
                case HM_REMOTE_CHECK_TCPS:
                case HM_REMOTE_SHARED_CHECK_TCP:
                case HM_REMOTE_SHARED_CHECK_TCPS:
                    remotelookup = make_unique<HMWorkRemoteCheckRemote>(HMWorkRemoteCheckRemote(it->first,
                                HMIPAddress(AF_INET), datahostCheck, hostGroupIt->second));
                    break;
                default:
                    HMLog(HM_LOG_ERROR, "% remote check type currently not supported", printRemoteCheckType(hostGroupIt->second.getRemoteCheckType()));
                    continue;

            }
            remotelookup->m_start = HMTimeStamp::now();
            remotelookup->m_end = HMTimeStamp::now() + it->second.getCheckTTL();
            it->second.queueCheck();
            queue.insertWork(remotelookup);
        }
        else if(restart && (state == HM_SCHEDULE_IGNORE))
        {
            HMTimeStamp checkTime = nextCheckTime(it->first);
            eventLoop.addRemoteTimeout(it->first, checkTime);
        }
    }
}

void
HMRemoteHostGroupCache::updateResultTime(const string& name, const HMTimeStamp& resultTime)
{
    auto key = name;
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing remote entry in cache in function finishquery for hostgroup %s",
                name.c_str());
        return;

    }
    it->second.setResultTime(resultTime);
}
