// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <chrono>

#include "HMDNSCache.h"
#include "HMEventLoop.h"
#include "HMWorkQueue.h"
#include "HMConstants.h"
#include "HMWork.h"
#include "HMWorkDNSLookup.h"
#ifdef USE_ARES
#include "HMWorkDNSLookupAres.h"
#endif
#include "HMWorkDNSLookupStatic.h"
#include "HMLogBase.h"
#include "HMStorage.h"

using namespace std;

bool
HMDNSCache::init()
{
    return true;
}

void
HMDNSCache::insertDNSEntry(const string& name, HMDNSLookup& dnsHostCheck, uint64_t ttl, uint64_t timeout)
{
    auto key = make_pair(name, dnsHostCheck);
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
        const HMDNSResult& v6Result, const HMDNSLookup& dnsLookup)
{
    bool ipv4 = false;
    bool ipv6 = false;

    map<pair<string,HMDNSLookup>,HMDNSResult>::iterator v4Entry;
    map<pair<string,HMDNSLookup>,HMDNSResult>::iterator v6Entry;
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
        HMDNSLookup dnsHostCheck(dnsLookup.getType(), false, dnsLookup.getRemoteCheckGroup());
        auto key = make_pair(name, dnsHostCheck);
        v4Entry = m_cache.find(key);
        if(v4Entry == m_cache.end())
        {
            v4Entry = m_cache.insert(make_pair(key,HMDNSResult())).first;
        }
        v4Entry->second.setResultTime(v4Result.getResultTime());
        v4Entry->second.updateQuery(v4Address);
        if(dnsLookup.getType() == HM_DNS_TYPE_STATIC)
        {
            v4Address.erase(HMIPAddress(AF_INET));
            addStaticDNSAddress(name, v4Address);
        }
    }

    if(ipv6)
    {
        HMDNSLookup dnsHostCheck(dnsLookup.getType(), true, dnsLookup.getRemoteCheckGroup());
        auto key = make_pair(name, dnsHostCheck);
        v6Entry = m_cache.find(key);
        if(v6Entry == m_cache.end())
        {
            v6Entry = m_cache.insert(make_pair(key,HMDNSResult())).first;
        }
        v6Entry->second.setResultTime(v6Result.getResultTime());
        v6Entry->second.updateQuery(v6Address);
        if (dnsLookup.getType() == HM_DNS_TYPE_STATIC)
        {
            v6Address.erase(HMIPAddress(AF_INET6));
            addStaticDNSAddress(name, v6Address);
        }
    }
}

void
HMDNSCache::updateDNSEntry(const string& name, HMDNSLookup& dnsHostCheck, const set<HMIPAddress>& addresses)
{
    auto key = make_pair(name, dnsHostCheck);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing DNS entry in cache in function updateDNSEntry for host %s - DNS check type %s",
                name.c_str(), printDnsType(dnsHostCheck.getType()).c_str());
        return;
    }
    it->second.updateQuery(addresses);
}

void
HMDNSCache::finishQuery(string name, HMDNSLookup& dnsHostCheck, bool success)
{
    auto key = make_pair(name, dnsHostCheck);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing DNS entry in cache in function finishquery for host %s - DNS check type %s",
                name.c_str(), printDnsType(dnsHostCheck.getType()).c_str());
        return;

    }
    it->second.finishQuery(success);
}

bool
HMDNSCache::getAddresses(const string& name, uint8_t dualstack, const HMDNSLookup& dnsHostCheck, set<HMIPAddress>& vaddress) const
{
    map<pair<string, HMDNSLookup>,HMDNSResult>::const_iterator res;

    if(dualstack & HM_DUALSTACK_IPV4_ONLY)
    {
        HMDNSLookup dnsCheck(dnsHostCheck.getType(), false, dnsHostCheck.getRemoteCheckGroup());
        if(getDNSResult(name, dnsCheck, res))
        {
            res->second.getAddresses(vaddress);
        }
    }
    if(dualstack & HM_DUALSTACK_IPV6_ONLY)
    {
        HMDNSLookup dnsCheck(dnsHostCheck.getType(), true, dnsHostCheck.getRemoteCheckGroup());
        if(getDNSResult(name, dnsCheck, res))
        {
            res->second.getAddresses(vaddress);
        }
    }

    return (vaddress.size() > 0);
}

bool
HMDNSCache::getExpiredAddresses(const string& name, uint8_t dualstack, const HMDNSLookup& dnsHostCheck, set<HMIPAddress>& vaddress) const
{
    map<pair<string,HMDNSLookup>,HMDNSResult>::const_iterator res;
    if (dualstack & HM_DUALSTACK_IPV4_ONLY)
    {
        HMDNSLookup dnsCheck(dnsHostCheck.getType(), false, dnsHostCheck.getRemoteCheckGroup());
        if (getDNSResult(name, dnsCheck, res))
        {
            res->second.getExpiredAddresses(vaddress);
        }
    }
    if (dualstack & HM_DUALSTACK_IPV6_ONLY)
    {
        HMDNSLookup dnsCheck(dnsHostCheck.getType(), true, dnsHostCheck.getRemoteCheckGroup());
        if (getDNSResult(name, dnsCheck, res) )
        {
            res->second.getExpiredAddresses(vaddress);
        }
    }
    return (vaddress.size() > 0);
}

bool
HMDNSCache::getDNSResult(const string& name, const HMDNSLookup& dnsHostCheck, map<pair<string,HMDNSLookup>,HMDNSResult>::const_iterator& result) const
{
    auto key = make_pair(name, dnsHostCheck);
    auto it = m_cache.find(key);
    if(it != m_cache.end())
    {
        result = it;
        return true;
    }
    return false;
}

HM_SCHEDULE_STATE
HMDNSCache::queryNeeded(const string& name, const HMDNSLookup& dnsHostCheck) const
{
    HM_SCHEDULE_STATE result = HM_SCHEDULE_EVENT;
    bool alreadyScheduled = true;
    bool isCheckTimeChanged = false;
    HMTimeStamp nextCheck = HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    auto key = make_pair(name, dnsHostCheck);
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
    else
    {
        HMLog(HM_LOG_NOTICE,
                        "Missing DNS entry in cache in function queryNeeded for host %s - DNS check type %s",
                        name.c_str(), printDnsType(dnsHostCheck.getType()).c_str());
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
HMDNSCache::nextQueryTime(const string& name, const HMDNSLookup& dnsHostCheck) const
{
    auto key = make_pair(name, dnsHostCheck);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_ERROR,
                        "Missing DNS entry in cache in function nextQueryTime for host %s - DNS check type %s",
                        name.c_str(), printDnsType(dnsHostCheck.getType()).c_str());
        return HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    }
    return it->second.nextQueryTime();
}

bool
HMDNSCache::startDNSQuery(const string& name, HMDNSLookup& dnsHostCheck)
{
    auto key = make_pair(name, dnsHostCheck);
    auto it = m_cache.find(key);
    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing DNS entry in cache in function startDNSQuery for host %s - DNS check type %s",
                name.c_str(), printDnsType(dnsHostCheck.getType()).c_str());
        return false;
    }
    it->second.startQuery();
    return true;
}

void
HMDNSCache::queueDNSQuery(string name, HMDNSLookup& dnsHostCheck, HMWorkQueue& queue)
{
    HMLog(HM_LOG_DEBUG, "[DEBUG] DNS Health Check QueueCheck for %s",
            name.c_str());
    auto key = make_pair(name, dnsHostCheck);
    auto it = m_cache.find(key);

    if(it == m_cache.end())
    {
        HMLog(HM_LOG_NOTICE,
                "Missing DNS entry in cache in function queueDNSQuery for host %s - DNS check type %s",
                name.c_str(), printDnsType(dnsHostCheck.getType()).c_str());
        return;
    }
    unique_ptr<HMWork> dnslookup;
    switch(dnsHostCheck.getType())
    {
    case HM_DNS_TYPE_STATIC:
        switch (dnsHostCheck.getPlugin())
        {
        case HM_DNS_PLUGIN_NONE:
            return;
        case HM_DNS_PLUGIN_STATIC:
        default:
            dnslookup = make_unique<HMWorkDNSLookupStatic>(
                    HMWorkDNSLookupStatic(it->first.first,
                            HMIPAddress((it->first.second.isIpv6()) ?
                            AF_INET6 :
                                                                      AF_INET),
                            HMDataHostCheck(it->first.second.getType()),
                            it->first.second));
            break;
        }
        break;
    case HM_DNS_TYPE_LOOKUP:
        switch (dnsHostCheck.getPlugin())
        {
        case HM_DNS_PLUGIN_NONE:
            return;
        case HM_DNS_PLUGIN_ARES:
        default:
            {
#ifdef USE_ARES
            dnslookup = make_unique<HMWorkDNSLookupAres>(
                    HMWorkDNSLookupAres(name,
                            HMIPAddress((dnsHostCheck.isIpv6()) ?
                            AF_INET6 :
                                                                  AF_INET),
                            HMDataHostCheck(it->first.second.getType()),
                            it->first.second));
#else
            HMLog(HM_LOG_ERROR, "ARES disabled during build. Please enable it for the ARES to work");
            return;
#endif
            }
        }
    }
    if (!it->first.second.getRemoteCheckGroup().empty())
    {
        dnslookup->setReschedule(false);
    }
    dnslookup->m_start = HMTimeStamp::now();
    dnslookup->m_end = HMTimeStamp::now() + it->second.getDNSTTL();
    it->second.queueQuery();
    queue.insertWork(dnslookup);
}

void
HMDNSCache::queueDNSLookups(HMWorkQueue& queue, HMEventLoop& eventLoop, bool restart)
{
    for(auto it = m_cache.begin(); it != m_cache.end(); ++it)
    {
        // Check for a DNS check
        unique_ptr<HMWork> dnslookup;
        if(!it->first.second.getRemoteCheckGroup().empty())
        {
            continue;
        }
        HM_SCHEDULE_STATE state = this->queryNeeded(it->first.first, it->first.second);
        if(state == HM_SCHEDULE_EVENT || state == HM_SCHEDULE_WORK)
        {
            // if there is no active query, then we add one
            switch (it->first.second.getType())
            {
            case HM_DNS_TYPE_STATIC:
                switch (it->first.second.getPlugin())
                {
                case HM_DNS_PLUGIN_NONE:
                    continue;
                case HM_DNS_PLUGIN_STATIC:
                default:
                    dnslookup = make_unique<HMWorkDNSLookupStatic>(
                            HMWorkDNSLookupStatic(it->first.first,
                                    HMIPAddress(
                                            (it->first.second.isIpv6()) ?
                                                    AF_INET6 : AF_INET),
                                    HMDataHostCheck(it->first.second.getType()),
                                    it->first.second));
                    break;
                }
                break;
            case HM_DNS_TYPE_LOOKUP:
                switch (it->first.second.getPlugin())
                {
                case HM_DNS_PLUGIN_NONE:
                    continue;
                case HM_DNS_PLUGIN_ARES:
                default:
                    {
#ifdef USE_ARES
                    dnslookup = make_unique<HMWorkDNSLookupAres>(
                            HMWorkDNSLookupAres(it->first.first,
                                    HMIPAddress(
                                            (it->first.second.isIpv6()) ?
                                                    AF_INET6 : AF_INET),
                                    HMDataHostCheck(it->first.second.getType()),
                                    it->first.second));
#else
                    HMLog(HM_LOG_ERROR, "ARES disabled during build. Please enable it for the ARES to work");
                    continue;
#endif
                    }
                    break;
                }
            }
            dnslookup->m_start = HMTimeStamp::now();
            dnslookup->m_end = HMTimeStamp::now() + it->second.getDNSTTL();
            it->second.queueQuery();
            queue.insertWork(dnslookup);
        }
        else if(restart && (state == HM_SCHEDULE_IGNORE))
        {
            HMTimeStamp checkTime = nextQueryTime(it->first.first, it->first.second);
            eventLoop.addDNSTimeout(it->first.first, it->first.second, checkTime);
        }
    }
}

bool
HMDNSCache::isValidAddress(const string& name, uint8_t dualstack, const HMDNSLookup& dnsHostCheck, HMIPAddress& address) const
{
    map<pair<string,HMDNSLookup>,HMDNSResult>::const_iterator res;

    if(dualstack & HM_DUALSTACK_IPV4_ONLY)
    {
        HMDNSLookup dnsCheck(dnsHostCheck.getType(), false, dnsHostCheck.getRemoteCheckGroup());
        if(getDNSResult(name, dnsCheck, res) && res->second.isValidAddress(address))
        {
            return true;
        }
    }
    if(dualstack & HM_DUALSTACK_IPV6_ONLY)
    {
        HMDNSLookup dnsCheck(dnsHostCheck.getType(), true, dnsHostCheck.getRemoteCheckGroup());
        if(getDNSResult(name, dnsCheck, res) && res->second.isValidAddress(address))
        {
            return true;
        }
    }
    return false;
}

bool HMDNSCache::getStaticDNSAddress(const std::string& host, bool ipv6,
        std::set<HMIPAddress>& addresses)
{
    shared_lock<shared_timed_mutex> rlock(m_staticDNSMutex);
    addresses.clear();
    auto key = make_pair(host, ipv6);
    auto ret = m_staticDNS.find(key);
    if( ret != m_staticDNS.end())
    {
        addresses.insert(ret->second.begin(), ret->second.end());
    }
    return addresses.size() > 0;
}

bool HMDNSCache::removeStaticDNSAddress(const std::string& host,
        set<HMIPAddress>& addresses)
{
    lock_guard<shared_timed_mutex> wLock(m_staticDNSMutex);

    for (const HMIPAddress& address : addresses)
    {
        auto key = make_pair(host, address.getType() == AF_INET6);
        auto ret = m_staticDNS.find(key);
        if (ret != m_staticDNS.end())
        {
            ret->second.erase(address);
        }
    }
    return true;
}

bool HMDNSCache::addStaticDNSAddress(const string& host, set<HMIPAddress>& addresses)
{
    std::lock_guard<std::shared_timed_mutex> wLock(m_staticDNSMutex);
    for(const HMIPAddress& address : addresses)
    {
        auto key = make_pair(host, address.getType() == AF_INET6);
        auto it = m_staticDNS.find(key);
        if(it == m_staticDNS.end())
        {
            set<HMIPAddress> adds;
            adds.insert(address);
            m_staticDNS.insert(make_pair(key, adds));
        }
        else
        {
            it->second.insert(address);
        }
    }
    return true;
}

const std::map<std::pair<std::string, bool>, std::set<HMIPAddress> >& HMDNSCache::getStaticDns() const
{
    return m_staticDNS;
}

void HMDNSCache::setStaticDns(
        const std::map<std::pair<std::string, bool>, std::set<HMIPAddress> >& staticDns)
{
    m_staticDNS = staticDns;
}
