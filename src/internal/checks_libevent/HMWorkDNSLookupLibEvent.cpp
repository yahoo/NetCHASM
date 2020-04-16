// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <event2/dns.h>

#include "HMWorkDNSLookupLibEvent.h"
#include "HMStateManager.h"
#include "HMLogBase.h"

void
HMWorkDNSLookupLibEvent::libEventDNSCallback (int result, char type, int count, int ttl, void* addrs, void* arg)
{
    (void) ttl;
    HMWorkDNSLookupLibEvent* check = (HMWorkDNSLookupLibEvent*)arg;
    char* addresses = (char*)addrs;

    if (result == DNS_ERR_NONE)
    {
        if (count == 0)
        {
            HMIPAddress ip(check->m_ipAddress.getType());
            check->addIP(ip);
            check->m_response = HM_RESPONSE_DNS_FAILED;
            check->m_reason = HM_REASON_DNS_FAILURE;
        }
        else
        {
            for(int i = 0; i < count; ++i)
            {
                if(type == DNS_IPv4_A)
                {
                    HMIPAddress ip;
                    if(ip.set((char*)&addresses[i*4], AF_INET))
                    {
                        check->addIP(ip);
                    }
                }
                else if(type == DNS_IPv6_AAAA)
                {
                    HMIPAddress ip;
                    if(ip.set((char*)&addresses[i*16], AF_INET6))
                    {
                        check->addIP(ip);
                    }
                }
            }
        }
    }
    else if (result == DNS_ERR_TIMEOUT)
    {
        HMIPAddress ip(check->m_ipAddress.getType());
        check->addIP(ip);
        check->m_response = HM_RESPONSE_DNS_FAILED;
        check->m_reason = HM_REASON_DNS_TIMEOUT;
        HMLog(HM_LOG_DEBUG3,"[LIBEVENT] DNS lookup %s(%s): TIMEOUT",check->m_hostname.c_str(), check->m_ipAddress.toString().c_str());
    }
    else
    {
        HMIPAddress ip(check->m_ipAddress.getType());
        check->addIP(ip);
        check->m_reason = (result == DNS_ERR_NOTEXIST)
                   ? HM_REASON_DNS_NOTFOUND : HM_REASON_DNS_FAILURE;
        check->m_response = HM_RESPONSE_DNS_FAILED;
        HMLog(HM_LOG_INFO,"[LIBEVENT] LibEvent DNS failure %s on %s(%s)",printReason(check->m_reason).c_str(),check->m_hostname.c_str(), check->m_ipAddress.toString().c_str());
    }

    // move the work to the work queue
    check->m_workStatus = HM_WORK_COMPLETE;
    check->m_stateManager->m_workQueue.addWork((HMWork *) check);
}

HM_WORK_STATUS
HMWorkDNSLookupLibEvent::dnsLookup()
{
    m_start = HMTimeStamp::now();
    m_end = m_start;

    HMEventLoopLibEvent* le = m_stateManager->getLibEvent();

    if(le == nullptr)
    {
        HMLog(HM_LOG_ERROR,"[LIBEVENT] DNS resolver is not initialized");
        m_response = HM_RESPONSE_FAILED;
        m_reason = HM_REASON_DNS_FAILURE;
        return HM_WORK_COMPLETE;
    }
    struct evdns_base* dns = le->getDNSBase();


    if(m_ipAddress.getType() == AF_INET)
    {
        evdns_base_resolve_ipv4(dns, m_hostname.c_str(), 0, HMWorkDNSLookupLibEvent::libEventDNSCallback, this);
    }
    else if(m_ipAddress.getType() == AF_INET6)
    {
        evdns_base_resolve_ipv6(dns, m_hostname.c_str(), 0, HMWorkDNSLookupLibEvent::libEventDNSCallback, this);
    }
    else
    {
        HMLog(HM_LOG_ERROR,"[LIBEVENT] invalid address type given to DNS resolver");
    }

    le->wakeupTracker();

    return HM_WORK_IN_PROGRESS;
}

void
HMWorkDNSLookupLibEvent::addIP(HMIPAddress& ip)
{
    m_ips.insert(ip);
    HMLog(HM_LOG_DEBUG, "[LIBEVENT] Resolved IP: %s", ip.toString().c_str());
}
