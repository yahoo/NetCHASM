// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMConstants.h"

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
        return "http-auxfetch";

    case HM_CHECK_AUX_HTTPS:
        return "https-auxfetch";

    case HM_CHECK_AUX_HTTPS_NO_PEER_CHECK:
        return "https-no-peer-check-auxfetch";

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
    default:
        return "Invalid Response";
    }
}

string
printWorkState(HM_WORK_STATE state)
{
    switch(state)
    {
    case HM_CHECK_INACTIVE:
        return "INACTIVE";
    case HM_CHECK_QUEUED:
        return "QUEUED";
    case HM_CHECK_IN_PROGRESS:
        return "IN PROGRESS";
    case HM_CHECK_FAILED:
        return "FAILED";
    default:
        return "Invalid Response";
    }
}
