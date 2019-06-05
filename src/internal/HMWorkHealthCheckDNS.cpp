// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <curl/curl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <ares.h>

#include "HMWorkHealthCheckDNS.h"
#include "HMWork.h"
#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMLogBase.h"

// LCOV_EXCL_START; Tested in functional testing

using namespace std;

static void
aresCallback (void* arg, int status, int timeouts, struct hostent* host)
{
    HM_REASON ret = HM_REASON_NONE;
    HMWorkHealthCheckDNS* check = (HMWorkHealthCheckDNS*)arg;
    string msg;
    if (status == ARES_SUCCESS)
    {
        HMIPAddress ip;
        if(ip.set(host->h_addr_list[0],host->h_addrtype))
        {
            msg = "Resolved address : " + string (host->h_name) + " with IP : "+ ip.toString();
        }
        ret = HM_REASON_SUCCESS;

    }
    else if (status == ARES_ENOTFOUND)
    {
        msg = "DNS lookup not found, error code :" + to_string(status);
        ret = HM_REASON_DNS_NOTFOUND;
    }
    else
    {
        msg = "Ares DNS failure, error code:" + to_string(status);
        ret = HM_REASON_DNS_FAILURE;
    }

    if (timeouts > 0)
    {
        msg = "Ares DNS timeout";

        ret = HM_REASON_DNS_TIMEOUT;
    }

    check->status(ret,msg);
    check->signalDone();
}

HM_WORK_STATUS
HMWorkHealthCheckDNS::healthCheck()
{
    if (m_hostCheck.getCheckType() == HM_CHECK_DNSVC || m_hostCheck.getCheckType() == HM_CHECK_DNS)
    {
        HMLog(HM_LOG_DEBUG3, "[DNSCHECK]  Checking %s at %s:%u for check type %s",
                 m_hostname.c_str(),
                 m_ipAddress.toString().c_str(),
                 m_hostCheck.getPort(),
                 printCheckType(m_hostCheck.getCheckType()).c_str());

        string url;
        string checkInfo = m_hostCheck.getCheckInfo();
        m_finishCallback = false;

        int nfds;
        fd_set readers, writers;
        timeval tv, *tvp, maxtv;

        uint64_t timeout = m_timeout - HMTimeStamp::now();
        maxtv.tv_sec = timeout / 1000;
        maxtv.tv_usec = 0;

        m_response = HM_RESPONSE_FAILED;
        m_reason = HM_REASON_NONE;

        struct ares_options opts;
        opts.flags = 0;

        // will be updated with host check values
        opts.tries = 1;
        opts.timeout = timeout;

        int optMask = 0;
        ares_channel channel;
        if(m_hostCheck.getCheckType() == HM_CHECK_DNSVC)
        {
            opts.flags |= ARES_FLAG_USEVC;
            opts.tcp_port = m_hostCheck.getPort();
            optMask = ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES|ARES_OPT_FLAGS | ARES_OPT_TCP_PORT;
        }
        else
        {
            opts.udp_port = m_hostCheck.getPort();
            optMask = ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES|ARES_OPT_FLAGS | ARES_OPT_UDP_PORT;
        }

        if(ares_init_options(&channel, &opts, optMask) != ARES_SUCCESS)
        {
            HMLog(HM_LOG_ERROR, "[DNSCHECK] Ares Init Error"); // LCOV_EXCL_LINE : This cannot be reached in testing
        }
        else
        {

            struct ares_addr_node addrs;
            if (m_ipAddress.getType() == AF_INET)
            {
                addrs.family = AF_INET;
                addrs.addr.addr4.s_addr = m_ipAddress.addr4();
                addrs.next = NULL;
                ares_set_servers(channel,&addrs);

            }
            else if (m_ipAddress.getType() == AF_INET6)
            {
                addrs.family = AF_INET6;
                in6_addr ipv6addr = m_ipAddress.addr6();
                memcpy(&addrs.addr.addr6,&ipv6addr,sizeof(addrs.addr.addr6));
                addrs.next = NULL;
                ares_set_servers(channel,&addrs);
            }


            m_start = HMTimeStamp::now();

            if(m_ipAddress.getType() == AF_INET)
            {
                ares_gethostbyname(channel,checkInfo.c_str(),AF_INET,aresCallback,(void*)this);
            }
            else
            {
                ares_gethostbyname(channel,checkInfo.c_str(),AF_INET6,aresCallback,(void*)this);
            }

            while(!m_finishCallback)
            {
                FD_ZERO(&readers);
                FD_ZERO(&writers);
                nfds = ares_fds(channel, &readers, &writers);
                if (nfds == 0)
                {
                    break;
                }
                tvp = ares_timeout(channel, &maxtv, &tv);
                select(nfds, &readers, &writers, NULL, tvp);
                ares_process(channel, &readers, &writers);
            }

            m_end = HMTimeStamp::now();

            ares_destroy(channel);
        }
        return m_ok;
    }

    else
    {
        HMLog(HM_LOG_ERROR, "[DNSCHECK] Invalid Check type for DNS and DNSVC %s",
                printCheckType(m_hostCheck.getCheckType()).c_str());
        return HM_WORK_COMPLETE;
    }
}

void
HMWorkHealthCheckDNS::signalDone()
{
    m_finishCallback = true;
}

void
HMWorkHealthCheckDNS::status(HM_REASON ret, const string& msg)
{
    m_reason = ret;
    if( m_reason == HM_REASON_SUCCESS)
    {
        m_response = HM_RESPONSE_CONNECTED;
        HMLog(HM_LOG_DEBUG3, "[DNSVCCHECK] %s for %s at %s ",
                msg.c_str(), m_hostname.c_str(),
                m_ipAddress.toString().c_str());
        m_ok = HM_WORK_COMPLETE;
    }
    else
    {
	m_response = HM_RESPONSE_FAILED;
        HMLog(HM_LOG_INFO, "[DNSVCCHECK] %s for %s at %s ",
                msg.c_str(), m_hostname.c_str(),
                m_ipAddress.toString().c_str());
	m_ok = HM_WORK_COMPLETE;
    }
}

// LCOV_EXCL_STOP; Tested in functional testing
