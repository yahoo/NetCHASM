// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMConstants.h"
#include "HMDataHostCheck.h"
#include "HMLogBase.h"
using namespace std;

string
printCheckTypeConfigs(HM_CHECK_TYPE ct)
{
    switch(ct)
    {
    case HM_CHECK_NONE:
        return "none";

    case HM_CHECK_HTTP:
        return "http";

    case HM_CHECK_TCP:
        return "tcp";

    case HM_CHECK_HTTPS:
        return "https";

    case HM_CHECK_HTTPS_NO_PEER_CHECK:
        return "https-no-peer-check";

    case HM_CHECK_FTP:
        return "ftp";

    case HM_CHECK_DNS:
        return "dns";

    case HM_CHECK_DNSVC:
        return "dnsvc";

    case HM_CHECK_FTPS_IMPLICIT:
        return "ftps-implicit";

    case HM_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK:
        return "ftps-implicit-no-peer-check";

    case HM_CHECK_FTPS_EXPLICIT:
        return "ftps-explicit";

    case HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK:
        return "ftps-explicit-no-peer-check";

    case HM_CHECK_AUX_HTTP:
        return "http";

    case HM_CHECK_AUX_HTTPS:
        return "https";

    case HM_CHECK_AUX_HTTPS_NO_PEER_CHECK:
        return "https-no-peer-check";

    case HM_CHECK_MARK_HTTP:
        return "http-mark";

    case HM_CHECK_MARK_HTTPS:
        return "https-mark";

    case HM_CHECK_MARK_HTTPS_NO_PEER_CHECK:
        return "https-mark-no-peer-check";

    default:
        return "default (empty) value";
    }
}


string
printCheckType(HM_CHECK_TYPE ct)
{
    switch(ct)
    {
    case HM_CHECK_NONE:
        return "none";

    case HM_CHECK_HTTP:
        return "http";

    case HM_CHECK_TCP:
        return "tcp";

    case HM_CHECK_TCPS:
        return "tcps";

    case HM_CHECK_HTTPS:
        return "https";

    case HM_CHECK_HTTPS_NO_PEER_CHECK:
        return "https-no-peer-check";

    case HM_CHECK_FTP:
        return "ftp";

    case HM_CHECK_DNS:
        return "dns";

    case HM_CHECK_DNSVC:
        return "dnsvc";

    case HM_CHECK_FTPS_IMPLICIT:
        return "ftps-implicit";

    case HM_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK:
        return "ftps-implicit-no-peer-check";

    case HM_CHECK_FTPS_EXPLICIT:
        return "ftps-explicit";

    case HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK:
        return "ftps-explicit-no-peer-check";

    case HM_CHECK_AUX_HTTP:
        return "aux http";

    case HM_CHECK_AUX_HTTPS:
        return "aux https";

    case HM_CHECK_AUX_HTTPS_NO_PEER_CHECK:
        return "aux https-no-peer-check";

    case HM_CHECK_MARK_HTTP:
        return "http-mark";

    case HM_CHECK_MARK_HTTPS:
        return "https-mark";

    case HM_CHECK_MARK_HTTPS_NO_PEER_CHECK:
        return "https-mark-no-peer-check";

    default:
        return "default (empty) value";
    }
}

string
printWorkType(HM_WORK_TYPE work)
{
    switch(work)
    {
    case HM_WORK_NONE:
        return "none";
    case HM_WORK_HEALTHCHECK:
        return "Health Check";
    case HM_WORK_DNSLOOKUP:
        return "DNS Lookup";
    case HM_WORK_AUXFETCH:
        return "Aux Fetch";
    default:
        return "Invalid Work Type";
    }
}

string
printMeasurementOptions(uint16_t rtMode)
{
    if (rtMode == HM_RT_CONNECT)
    {
        return "connect";
    }
    else if (rtMode == HM_RT_SMOOTHED_CONNECT)
    {
        return "smoothed-connect";
    }
    else if (rtMode == HM_RT_TOTAL)
    {
        return "total";
    }
    else if (rtMode == HM_RT_SMOOTHED_TOTAL)
    {
        return "smoothed-total";
    }
    return "connect";
}

string
printDnsType(HM_DNS_PLUGIN_CLASS dt)
{
    switch(dt)
    {
    case HM_DNS_PLUGIN_ARES:
        return "Ares";
    case HM_DNS_PLUGIN_LIBEVENT:
        return "Libevent";
    case HM_DNS_PLUGIN_NONE:
        return "None";
    case HM_DNS_PLUGIN_STATIC:
        return "Static";
    default:
        return "Invalid DNS type";
    }
}

string
printDualStack(HM_DUALSTACK ds)
{
    switch(ds)
    {
    case HM_DUALSTACK_IPV6_ONLY:
        return "ipv6-only";
    case HM_DUALSTACK_BOTH:
        return "both";
    case HM_DUALSTACK_UNDEFINED:
    case HM_DUALSTACK_IPV4_ONLY:
    default:
        return "ipv4-only";
    }
}

string
printReason(HM_REASON reason)
{
    switch(reason)
    {
    case HM_REASON_NONE:
        return "-";
    case HM_REASON_SUCCESS:
        return "Ok";
    case HM_REASON_DNS_NOTFOUND:
        return "DNS Not Found";
    case HM_REASON_DNS_TIMEOUT:
        return "DNS Timeout";
    case HM_REASON_DNS_FAILURE:
        return "DNS Failure";
    case HM_REASON_YNET_NOTFOUND:
        return "YNET Not Found";
    case HM_REASON_CONNECT_TIMEOUT:
        return  "Connect Timeout";
    case HM_REASON_CONNECT_FAILURE:
        return "Connect Failure";
    case HM_REASON_REQUEST_FAILURE:
        return "Request Failure";
    case HM_REASON_RESPONSE_TIMEOUT:
        return "Response Timeout";
    case HM_REASON_RESPONSE_FAILURE:
        return "Response Failure";
    case HM_REASON_RESPONSE_DOWN:
        return "Response Down";
    case HM_REASON_RESPONSE_404:
        return "Response 404";
    case HM_REASON_RESPONSE_403:
        return "Response 403";
    case HM_REASON_RESPONSE_3XX:
        return "Response 3xx";
    case HM_REASON_RESPONSE_5XX:
        return "Response 5xx";
    case HM_REASON_INTERNAL_ERROR:
        return "Internal Error";
    case HM_REASON_REMOTE_NODATA:
        return "Remote No Data";
    default:
        return "Invalid Response Code";
    }
}

string
printStatus(HM_HOST_STATUS status)
{
    string ret;
    if(status == HM_HOST_STATUS_NONE)
    {
        return "Status None";
    }

    if(status & HM_HOST_STATUS_USE)
    {
        ret = "USE";
    }

    if(status & HM_HOST_STATUS_FORCE_DOWN)
    {
        ret += (ret.empty())?"FORCEDOWN":"|FORCEDOWN";
    }

    if(status & HM_HOST_STATUS_FORCE_USE)
    {
        ret += (ret.empty())?"FORCEUSE":"|FORCEUSE";
    }

    if(status & HM_HOST_STATUS_UP)
    {
        ret += (ret.empty())?"UP":"|UP";
    }

    return std::move(ret);
}

string
printResponse(HM_RESPONSE response)
{
    switch(response)
    {
    case HM_RESPONSE_NONE:
        return "No Response";
    case HM_RESPONSE_CONNECTED:
        return "Connected";
    case HM_RESPONSE_FAILED:
        return "Failed";
    case HM_RESPONSE_DNS_FAILED:
        return "DNS Failed";
    case HM_RESPONSE_REMOTE_FAILED:
        return "Remote Failed";
    default:
        return "Invalid Response";
    }
}

bool CompareCheckList::operator()(const std::pair<std::string,HMDataHostCheck>& lhs,
        const std::pair<std::string,HMDataHostCheck>& rhs) const {

    if(lhs.first  == rhs.first)
    {
        if(lhs.second == rhs.second)
        {
            return (uint8_t)lhs.second.getDnsPlugin() < rhs.second.getDnsPlugin();
        }
        return lhs.second < rhs.second;
    }
    return lhs.first < rhs.first;
}
