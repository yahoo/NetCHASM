// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "ares.h"

#include "HMWorkDNSLookupAres.h"
#include "HMDNSCache.h"
#include "HMIPAddress.h"
#include "HMLogBase.h"
#include "HMWork.h"
#include "HMStateManager.h"

// LCOV_EXCL_START; Tested in functional testing

using namespace std;

static void
aresCallback (void* arg, int status, int timeouts, struct hostent* host)
{
    (void)timeouts;
    HMWorkDNSLookupAres* check = (HMWorkDNSLookupAres*)arg;
    if (status == ARES_SUCCESS)
    {
        bool isIPEmpty = true;
        // now the ip addresses
        for(char** p = host->h_addr_list; *p != NULL; p++)
        {
            HMIPAddress ip;
            if(ip.set(*p,host->h_addrtype))
            {
                isIPEmpty = false;
                check->addIP(ip);
            }
        }
        if (isIPEmpty)
        {
            HMIPAddress ip(check->m_ipAddress.getType());
            check->addIP(ip);
            check->m_response = HM_RESPONSE_DNS_FAILED;
            check->m_reason = HM_REASON_DNS_FAILURE;
        }
    }
    else if (status == ARES_ETIMEOUT)
    {
        HMIPAddress ip(check->m_ipAddress.getType());
        check->addIP(ip);
        check->m_response = HM_RESPONSE_DNS_FAILED;
        check->m_reason = HM_REASON_DNS_TIMEOUT;
        string ip_version = check->m_ipAddress.getType() == AF_INET? "IPv4":"IPv6";
        HMLog(HM_LOG_DEBUG3,"[ARES] DNS lookup %s for %s: TIMEOUT",check->m_hostname.c_str(), ip_version.c_str());
    }
    else
    {
        HMIPAddress ip(check->m_ipAddress.getType());
        check->addIP(ip);
        check->m_reason = (ARES_ENOTFOUND == status)
                   ? HM_REASON_DNS_NOTFOUND : HM_REASON_DNS_FAILURE;
        check->m_response = HM_RESPONSE_DNS_FAILED;
        string ip_version = check->m_ipAddress.getType() == AF_INET? "IPv4":"IPv6";
        HMLog(HM_LOG_NOTICE,"[ARES] %s %s for host %s", ip_version.c_str(), printReason(check->m_reason).c_str(),check->m_hostname.c_str());
    }
    check->signalDone();
}

void
HMWorkDNSLookupAres::init(HMWorkState& state)
{
    m_channel = &state.m_channel;
    m_finished = false;
}

HM_WORK_STATUS
HMWorkDNSLookupAres::dnsLookup()
{

    m_finished = false;
    bool ipv6 = false;
    int nfds;
    fd_set readers, writers;
    timeval tv, *tvp;

    HMLog(HM_LOG_DEBUG3, "[ARES] Resolving %s" , m_hostname.c_str());

    if(m_ipAddress.getType() == AF_INET)
    {
        ares_gethostbyname(*m_channel,m_hostname.c_str(),AF_INET,aresCallback,(void*)this);
    }
    else
    {
        ares_gethostbyname(*m_channel,m_hostname.c_str(),AF_INET6,aresCallback,(void*)this);
        ipv6 = true;
    }

    do
    {
        FD_ZERO(&readers);
        FD_ZERO(&writers);
        nfds = ares_fds(*m_channel, &readers, &writers);
        if (nfds == 0)
        {
            break;
        }
        tvp = ares_timeout(*m_channel, NULL, &tv);
        select(nfds, &readers, &writers, NULL, tvp);
        ares_process(*m_channel, &readers, &writers);
    } while(!m_finished);

    // Now process the returned data
    m_start = HMTimeStamp::now();
    m_end = m_start;
    shared_ptr<HMState> current;
    m_stateManager->updateState(current);

    // If server times out we do not update the entries. The old entried will continue to exist. The same can be done for other error if needed
    if (m_reason != HM_REASON_DNS_TIMEOUT)
    {
        current->m_dnsCache.updateDNSEntry(m_hostname, ipv6, m_ips);
    }
    else
    {
        uint8_t dual_stack = HM_DUALSTACK_IPV4_ONLY;
        if (m_ipAddress.getType() == AF_INET6)
        {
            dual_stack = HM_DUALSTACK_IPV6_ONLY;
        }
        // If no valid IP already exist insert "0.0.0.0 or ::"
        set<HMIPAddress> tmp_result;
        if (!current->m_dnsCache.getAddresses(m_hostname, dual_stack, tmp_result))
        {
            current->m_dnsCache.updateDNSEntry(m_hostname, ipv6, m_ips);
        }
    }
    current->m_dnsCache.finishQuery(m_hostname, ipv6, true);
    return HM_WORK_COMPLETE;
}

void
HMWorkDNSLookupAres::addIP(HMIPAddress& ip)
{
    m_ips.insert(ip);
    HMLog(HM_LOG_DEBUG, "[ARES] Resolved IP: %s", ip.toString().c_str());
}

void
HMWorkDNSLookupAres::signalDone()
{
    m_finished = true;
}

// LCOV_EXCL_STOP; Tested in functional testing
