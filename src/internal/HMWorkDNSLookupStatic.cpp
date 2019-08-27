// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMWorkDNSLookupStatic.h"
#include "HMDNSCache.h"
#include "HMIPAddress.h"
#include "HMLogBase.h"
#include "HMWork.h"
#include "HMStateManager.h"

// LCOV_EXCL_START; Tested in functional testing

using namespace std;

void
HMWorkDNSLookupStatic::init(HMWorkState& state)
{
    (void)state;
}

HM_WORK_STATUS
HMWorkDNSLookupStatic::dnsLookup()
{


    bool ipv6 = false;

    HMLog(HM_LOG_DEBUG3, "[STATIC] Resolving %s" , m_hostname.c_str());
    if(m_ipAddress.getType() == AF_INET6)
    {
        ipv6 = true;
    }

    // Now process the returned data
    m_start = HMTimeStamp::now();
    m_end = m_start;

    shared_ptr<HMState> current;
    m_stateManager->updateState(current);
    // If no valid IP already exist insert "0.0.0.0 or ::"
    if (current->m_dnsCache.getStaticDNSAddress(m_hostname, ipv6, m_ips))
    {
        current->m_dnsCache.updateDNSEntry(m_hostname, m_dnsHostCheck, m_ips);
    }
    else
    {
        m_response = HM_RESPONSE_DNS_FAILED;
        m_reason = HM_REASON_DNS_FAILURE;
        set<HMIPAddress> tmp_result;
        HMIPAddress ip(m_ipAddress.getType());
        tmp_result.insert(ip);
        current->m_dnsCache.updateDNSEntry(m_hostname, m_dnsHostCheck, tmp_result);
    }
    current->m_dnsCache.finishQuery(m_hostname, m_dnsHostCheck, true);
    return HM_WORK_COMPLETE;
}
// LCOV_EXCL_STOP; Tested in functional testing
