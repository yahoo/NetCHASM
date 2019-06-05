// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <chrono>
#include "ares.h"

#include "HMDNSCache.h"
#include "HMEventLoop.h"
#include "HMWorkQueue.h"
#include "HMConstants.h"
#include "HMWork.h"
#include "HMWorkDNSLookup.h"
#include "HMWorkDNSLookupAres.h"
#include "HMLogBase.h"

using namespace std;

bool
HMDNSCache::init(uint32_t dnsType)
{
    m_dnsType = dnsType;
    return true;
}

void
HMDNSCache::insertDNSEntry(const string& name, bool ipv6, uint64_t ttl, uint64_t timeout)
{
    auto key = make_pair(name, ipv6);
    ttl = (ttl == 0) ? HM_DEFAULT_DNS_TTL : ttl;
    timeout = (timeout == 0) ? HM_DEFAULT_DNS_RESOLUTION_TIMEOUT : timeout;

    auto result = m_cache.insert(make_pair(key, HMDNSResult(ttl, timeout)));
    if(!result.second)
    {
        result.first->second.updateTimeouts(ttl, timeout);
    }
}

void
HMDNSCache::updateReloadDNSEntry(const string& name,
        set<HMIPAddress>& addresses,
        const HMDNSResult& v4Result,
        const HMDNSResult& v6Result)
{
    bool ipv4 = false;
    bool ipv6 = false;

    map<pair<string,bool>,HMDNSResult>::iterator v4Entry;
    map<pair<string,bool>,HMDNSResult>::iterator v6Entry;
    set<HMIPAddress> v4Address;
    set<HMIPAddress> v6Address;
    for(auto it = addresses.begin(); it != addresses.end(); ++it)
    {
        if(it->getType() == AF_INET)
        {
            ipv4 = true;
            v4Address.insert(*it);
        }
        else if(it->getType() == AF_INET6)
        {
            v6Address.insert(*it);
            ipv6 = true;
        }
    }

    if(ipv4)
    {
        auto key = make_pair(name, false);
        v4Entry = m_cache.find(key);
        if(v4Entry == m_cache.end())
        {
            v4Entry = m_cache.insert(make_pair(key,HMDNSResult())).first;
        }
        v4Entry->second.setResultTime(v4Result.getResultTime());
        v4Entry->second.updateQuery(v4Address);
    }

    if(ipv6)
    {
        auto key = make_pair(name, true);
        v6Entry = m_cache.find(key);
        if(v6Entry == m_cache.end())
        {
            v6Entry = m_cache.insert(make_pair(key,HMDNSResult())).first;
        }
        v6Entry->second.setResultTime(v6Result.getResultTime());
        v6Entry->second.updateQuery(v6Address);
    }
}

void
HMDNSCache::updateDNSEntry(string name, bool ipv6, set<HMIPAddress>& addresses)
{
    auto key = make_pair(name, ipv6);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        it = m_cache.insert(make_pair(key,HMDNSResult())).first;
    }
    it->second.updateQuery(addresses);
}

void
HMDNSCache::finishQuery(string name, bool ipv6, bool success)
{
    auto key = make_pair(name, ipv6);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        it = m_cache.insert(make_pair(key,HMDNSResult())).first;
    }
    it->second.finishQuery(success);
}

bool
HMDNSCache::getAddresses(const string& name, uint8_t dualstack, set<HMIPAddress>& vaddress) const
{
    map<pair<string,bool>,HMDNSResult>::const_iterator res;

    if(dualstack & HM_DUALSTACK_IPV4_ONLY)
    {
        if(getDNSResult(name, false, res))
        {
            res->second.getAddresses(vaddress);
        }
    }
    if(dualstack & HM_DUALSTACK_IPV6_ONLY)
    {
        if(getDNSResult(name, true, res))
        {
            res->second.getAddresses(vaddress);
        }
    }

    return (vaddress.size() > 0);
}

bool
HMDNSCache::getExpiredAddresses(const string& name, uint8_t dualstack, set<HMIPAddress>& vaddress) const
{
    map<pair<string,bool>,HMDNSResult>::const_iterator res;
    if (dualstack & HM_DUALSTACK_IPV4_ONLY)
    {
        if (getDNSResult(name, false, res))
        {
            res->second.getExpiredAddresses(vaddress);
        }
    }
    if (dualstack & HM_DUALSTACK_IPV6_ONLY)
    {
        if (getDNSResult(name, true, res) )
        {
            res->second.getExpiredAddresses(vaddress);
        }
    }
    return (vaddress.size() > 0);
}

bool
HMDNSCache::getDNSResult(const string& name, bool ipv6, map<pair<string,bool>,HMDNSResult>::const_iterator& result) const
{
    auto key = make_pair(name, ipv6);
    auto it = m_cache.find(key);
    if(it != m_cache.end())
    {
        result = it;
        return true;
    }
    return false;
}

HM_SCHEDULE_STATE
HMDNSCache::queryNeeded(const string& name, bool ipv6) const
{
    HM_SCHEDULE_STATE result = HM_SCHEDULE_EVENT;
    bool alreadyScheduled = true;
    bool isCheckTimeChanged = false;
    HMTimeStamp nextCheck = HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    auto key = make_pair(name, ipv6);
    auto ret = m_cache.find(key);
    if(ret != m_cache.end())
    {
        HMTimeStamp checkTime = ret->second.nextQueryTime();
        HM_WORK_STATE query_state = ret->second.getQueryState();
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
HMDNSCache::nextQueryTime(const string& name, bool ipv6) const
{
    auto key = make_pair(name, ipv6);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        return HMTimeStamp::now();
    }
    return it->second.nextQueryTime();
}

HMTimeStamp
HMDNSCache::startDNSQuery(const string& name, bool ipv6)
{
    auto key = make_pair(name, ipv6);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        it = m_cache.insert(make_pair(key, HMDNSResult())).first;
    }
    return it->second.startQuery();
}

void
HMDNSCache::queueDNSQuery(string name, bool ipv6, HMWorkQueue& queue)
{
    HMLog(HM_LOG_DEBUG, "[DEBUG] DNS Health Check QueueCheck for %s",
            name.c_str());
    auto key = make_pair(name, ipv6);
    auto it = m_cache.find(key);

    if(it == m_cache.end())
    {
        it = m_cache.insert(make_pair(key, HMDNSResult())).first;
    }

    switch(m_dnsType)
    {
    case HM_DNS_PLUGIN_NONE:
        break;
    case HM_DNS_PLUGIN_ARES:
    default:
    {
        unique_ptr<HMWork> dnslookup = make_unique<HMWorkDNSLookupAres>
            (HMWorkDNSLookupAres(name, HMIPAddress((ipv6)?AF_INET6:AF_INET), HMDataHostCheck()));
        dnslookup->m_start = HMTimeStamp::now();
        dnslookup->m_end = HMTimeStamp::now() + it->second.getDNSTTL();
        it->second.queueQuery();
        queue.insertWork(dnslookup);
    }
    }
}

void
HMDNSCache::queueDNSLookups(HMWorkQueue& queue, HMEventLoop& eventLoop, bool restart)
{
    for(auto it = m_cache.begin(); it != m_cache.end(); ++it)
    {
        // Check for a DNS check
        HM_SCHEDULE_STATE state = this->queryNeeded(it->first.first, it->first.second);
        if(state == HM_SCHEDULE_EVENT || state == HM_SCHEDULE_WORK)
        {
            // if there is no active query, then we add one
            switch (m_dnsType)
            {
            case HM_DNS_PLUGIN_NONE:
                break;
            case HM_DNS_PLUGIN_ARES:
            default:
            {
                unique_ptr<HMWork> dnslookup = make_unique<HMWorkDNSLookupAres>(HMWorkDNSLookupAres(it->first.first,
                                        HMIPAddress((it->first.second) ? AF_INET6 : AF_INET),
                                        HMDataHostCheck()));

                dnslookup->m_start = HMTimeStamp::now();
                dnslookup->m_end = HMTimeStamp::now() + it->second.getDNSTTL();
                it->second.queueQuery();
                queue.insertWork(dnslookup);
            }
            }
        }
        else if(restart && (state == HM_SCHEDULE_IGNORE))
        {
            HMTimeStamp checkTime = nextQueryTime(it->first.first, it->first.second);
            eventLoop.addDNSTimeout(it->first.first, it->first.second, checkTime);
        }
    }
}

uint32_t
HMDNSCache::getDNSCheckPluginType() const
{
    return m_dnsType;
}


bool
HMDNSCache::isValidAddress(const string& name, uint8_t dualstack, HMIPAddress& address) const
{
    map<pair<string,bool>,HMDNSResult>::const_iterator res;

    if(dualstack & HM_DUALSTACK_IPV4_ONLY)
    {
        if(getDNSResult(name, false, res) && res->second.isValidAddress(address))
        {
            return true;
        }
    }
    if(dualstack & HM_DUALSTACK_IPV6_ONLY)
    {
        if(getDNSResult(name, true, res) && res->second.isValidAddress(address))
        {
            return true;
        }
    }
    return false;
}
