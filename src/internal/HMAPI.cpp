// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMAPI.h"
#include "HMDataHostGroup.h"
#include "HMIPAddress.h"

using namespace std;

HMAPICheckResult::HMAPICheckResult() :
                m_start(0),
                m_end(0),
                m_responseTime(0),
                m_totalResponseTime(0),
                m_minResponseTime(0),
                m_maxResponseTime(0),
                m_smoothedResponseTime(0),
                m_sumResponseTime(0),
                m_numChecks(0),
                m_numResponses(0),
                m_numConnectFailures(0),
                m_numFailures(0),
                m_numTimeouts(0),
                m_numFlaps(0),
                m_status(0),
                m_response(0),
                m_reason(0),
                m_numFailedChecks(0),
                m_numSlowResponses(0),
                m_port(0),
                m_changeTime(0),
                m_prevTime(0),
                m_forceHostDown(false),
                m_queueCheckTime(0),
                m_checkTime(0) {}

HMAPICheckResult::HMAPICheckResult(HMDataCheckResult& k)
{
    m_address.m_type = k.m_address.getType();
    if(k.m_address.getType() == AF_INET)
    {
        m_address.m_ip.addr = k.m_address.addr4();
    }
    else if(k.m_address.getType() == AF_INET6)
    {
        m_address.m_ip.addr6 = k.m_address.addr6();
    }
    m_start = k.m_start.getTimeSinceEpoch();
    m_end = k.m_end.getTimeSinceEpoch();
    m_responseTime = k.m_responseTime;
    m_totalResponseTime = k.m_totalResponseTime;
    m_minResponseTime = k.m_minResponseTime;
    m_maxResponseTime = k.m_maxResponseTime;
    m_smoothedResponseTime = k.m_smoothedResponseTime;
    m_sumResponseTime = k.m_sumResponseTime;
    m_numChecks = k.m_numChecks;
    m_numResponses = k.m_numResponses;
    m_numConnectFailures = k.m_numConnectFailures;
    m_numFailures = k.m_numFailures;
    m_numTimeouts = k.m_numTimeouts;
    m_numFlaps = k.m_numFlaps;
    m_status = k.m_status;
    m_response = k.m_response;
    m_reason = k.m_reason;
    m_softReason = k.m_softReason;
    m_numFailedChecks = k.m_numFailedChecks;
    m_numSlowResponses = k.m_numSlowResponses;
    m_port = k.m_port;
    m_changeTime = k.m_changeTime.getTimeSinceEpoch();
    m_prevTime = k.m_flapTime.getTimeSinceEpoch();
    m_forceHostDown = k.m_forceHostDown;
    m_queueCheckTime = k.m_queueCheckTime.getTimeSinceEpoch();
    m_checkTime = k.m_checkTime.getTimeSinceEpoch();
}

HMAPICheckResult::HMAPICheckResult(HMGroupCheckResult& k)
{
    m_host = k.m_hostName;
    m_address.m_type = k.m_result.m_address.getType();
    if(k.m_address.getType() == AF_INET)
    {
        m_address.m_ip.addr = k.m_result.m_address.addr4();
    }
    else if(k.m_result.m_address.getType() == AF_INET6)
    {
        m_address.m_ip.addr6 = k.m_result.m_address.addr6();
    }
    m_start = k.m_result.m_start.getTimeSinceEpoch();
    m_end = k.m_result.m_end.getTimeSinceEpoch();
    m_responseTime = k.m_result.m_responseTime;
    m_totalResponseTime = k.m_result.m_totalResponseTime;
    m_minResponseTime = k.m_result.m_minResponseTime;
    m_maxResponseTime = k.m_result.m_maxResponseTime;
    m_smoothedResponseTime = k.m_result.m_smoothedResponseTime;
    m_sumResponseTime = k.m_result.m_sumResponseTime;
    m_numChecks = k.m_result.m_numChecks;
    m_numResponses = k.m_result.m_numResponses;
    m_numConnectFailures = k.m_result.m_numConnectFailures;
    m_numFailures = k.m_result.m_numFailures;
    m_numTimeouts = k.m_result.m_numTimeouts;
    m_numFlaps = k.m_result.m_numFlaps;
    m_status = k.m_result.m_status;
    m_response = k.m_result.m_response;
    m_reason = k.m_result.m_reason;
    m_softReason = k.m_result.m_softReason;
    m_numFailedChecks = k.m_result.m_numFailedChecks;
    m_numSlowResponses = k.m_result.m_numSlowResponses;
    m_port = k.m_result.m_port;
    m_changeTime = k.m_result.m_changeTime.getTimeSinceEpoch();
    m_prevTime = k.m_result.m_flapTime.getTimeSinceEpoch();
    m_forceHostDown = k.m_result.m_forceHostDown;
    m_queueCheckTime = k.m_result.m_queueCheckTime.getTimeSinceEpoch();
    m_checkTime = k.m_result.m_checkTime.getTimeSinceEpoch();
}


HMAPIIPAddress::HMAPIIPAddress(uint8_t type)
{
    memset(&m_ip.addr6, 0, sizeof(in6_addr));
    if(type == AF_INET)
    {
        m_type = AF_INET;
        m_ip.addr = 0x00000000;
    }
    else if(type == AF_INET6)
    {
        m_type = AF_INET6;
    }
    else
    {
        m_type = AF_UNSPEC;
    }
}


bool
HMAPIIPAddress::set(const string& addr)
{
    if (inet_pton(AF_INET, addr.c_str(), &m_ip.addr) == 1)
    {
        m_type = AF_INET;
        return true;
    }
    else if (inet_pton(AF_INET6, addr.c_str(), &m_ip.addr6) == 1)
    {
        m_type = AF_INET6;
        return true;
    }
    else
    {
        m_type = AF_UNSPEC;
    }
    return false;
}

bool
HMAPIIPAddress::set(const HMIPAddress& addr)
{
    m_type = addr.getType();
    if(m_type == AF_INET)
    {
        m_ip.addr = addr.addr4();
    }
    else if(m_type == AF_INET6)
    {
        m_ip.addr6 = addr.addr6();
    }
    return true;
}

bool
HMAPIIPAddress::operator ==(const HMAPIIPAddress &other) const
{
    if (m_type == other.m_type)
    {
        if (m_type == AF_INET)
        {
            return (memcmp(&m_ip, &other.m_ip, sizeof(in_addr_t)) == 0);
        }
        else if (m_type == AF_INET6)
        {
            return (memcmp(&m_ip, &other.m_ip, sizeof(in6_addr)) == 0);
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

string
HMAPIIPAddress::toString() const
{
    char buf[INET6_ADDRSTRLEN];
    if (m_type == AF_INET || m_type == AF_INET6)
    {
        inet_ntop(m_type,&m_ip,buf,INET6_ADDRSTRLEN);
        return buf;
    }
    return "";
}

HMAPICheckInfo::HMAPICheckInfo() :
        m_measurementOptions(0),
        m_ipv4(false),
        m_ipv6(false),
        m_checkType(HM_API_CHECK_DEFAULT),
        m_port(0),
        m_numCheckRetries(0),
        m_checkRetryDelay(0),
        m_smoothingWindow(0),
        m_groupThreshold(0),
        m_slowThreshold(0),
        m_maxFlaps(0),
        m_checkTimeout(0),
        m_checkTTL(0),
        m_flapThreshold(0),
        m_passthroughInfo(0),
        m_TOSValue(0),
        m_dnsCheckType(HM_API_DNS_ARES){}

HMAPICheckInfo::HMAPICheckInfo(HMDataHostGroup& hostGroup)
{
    m_measurementOptions = hostGroup.getMeasurementOptions();
    m_ipv4 = (HM_DUALSTACK_IPV4_ONLY & hostGroup.getDualstack());
    m_ipv6 = (HM_DUALSTACK_IPV6_ONLY & hostGroup.getDualstack());
    m_checkType = (HM_API_CHECK_TYPE)hostGroup.getCheckType();
    m_port = hostGroup.getCheckPort();
    m_checkInfo = hostGroup.getCheckInfo();
    m_numCheckRetries = hostGroup.getNumCheckRetries();
    m_checkRetryDelay = hostGroup.getCheckRetryDelay();
    m_smoothingWindow = hostGroup.getSmoothingWindow();
    m_groupThreshold = hostGroup.getGroupThreshold();
    m_slowThreshold = hostGroup.getSlowThreshold();
    m_maxFlaps = hostGroup.getMaxFlaps();
    m_checkTimeout = hostGroup.getCheckTimeout();
    m_checkTTL = hostGroup.getCheckTTL();
    m_flapThreshold = hostGroup.getFlapThreshold();
    m_passthroughInfo = hostGroup.getPassthroughInfo();
    m_sourceAddress.set(hostGroup.getSourceAddress());
    m_TOSValue = hostGroup.getTOSValue();
    m_dnsCheckType = (HM_DNS_CHECK_TYPE)hostGroup.getDnsCheckPlugin();
    if (hostGroup.getHostGroupList()->size() != 0)
    {
        m_hostGroups.resize(hostGroup.getHostGroupList()->size());
        std::copy (hostGroup.getHostGroupList()->begin(), hostGroup.getHostGroupList()->end(), m_hostGroups.begin());
    }
}

HMAPICheckInfo::HMAPICheckInfo(HMDataHostCheck& dataHostCheck, HMDataCheckParams& checkParams)
{
    m_measurementOptions = checkParams.getMeasurementOptions();
    m_ipv4 = (HM_DUALSTACK_IPV4_ONLY & dataHostCheck.getDualStack());
    m_ipv6 = (HM_DUALSTACK_IPV6_ONLY & dataHostCheck.getDualStack());
    m_checkType = (HM_API_CHECK_TYPE)dataHostCheck.getCheckType();
    m_port = dataHostCheck.getPort();
    m_checkInfo = dataHostCheck.getCheckInfo();
    m_dnsCheckType = (HM_DNS_CHECK_TYPE)dataHostCheck.getDnsPlugin();
    m_TOSValue = dataHostCheck.getTOSValue();
    m_numCheckRetries = checkParams.getNumCheckRetries();
    m_checkRetryDelay = checkParams.getCheckRetryDelay();
    m_smoothingWindow = checkParams.getSmoothingWindow();
    m_groupThreshold = checkParams.getGroupThreshold();
    m_slowThreshold = checkParams.getSlowThreshold();
    m_maxFlaps = checkParams.getMaxFlaps();
    m_checkTimeout = checkParams.getTimeout();
    m_checkTTL = checkParams.getTTL();
    m_flapThreshold = checkParams.getFlapThreshold();
    m_passthroughInfo = checkParams.getPassthroughInfo();
}

HMAPICheckInfo&
HMAPICheckInfo::operator=(HMDataHostGroup& hostGroup)
{
    m_measurementOptions = hostGroup.getMeasurementOptions();
    m_ipv4 = (HM_DUALSTACK_IPV4_ONLY & hostGroup.getDualstack());
    m_ipv6 = (HM_DUALSTACK_IPV6_ONLY & hostGroup.getDualstack());
    m_checkType = (HM_API_CHECK_TYPE)hostGroup.getCheckType();
    m_port = hostGroup.getCheckPort();
    m_checkInfo = hostGroup.getCheckInfo();
    m_numCheckRetries = hostGroup.getNumCheckRetries();
    m_checkRetryDelay = hostGroup.getCheckRetryDelay();
    m_smoothingWindow = hostGroup.getSmoothingWindow();
    m_groupThreshold = hostGroup.getGroupThreshold();
    m_slowThreshold = hostGroup.getSlowThreshold();
    m_maxFlaps = hostGroup.getMaxFlaps();
    m_checkTimeout = hostGroup.getCheckTimeout();
    m_checkTTL = hostGroup.getCheckTTL();
    m_flapThreshold = hostGroup.getFlapThreshold();
    m_passthroughInfo = hostGroup.getPassthroughInfo();
    m_sourceAddress.set(hostGroup.getSourceAddress());
    m_TOSValue = hostGroup.getTOSValue();
    if (hostGroup.getHostGroupList()->size() != 0)
    {
        m_hostGroups.resize(hostGroup.getHostGroupList()->size());
        std::copy (hostGroup.getHostGroupList()->begin(), hostGroup.getHostGroupList()->end(), m_hostGroups.begin());
    }
    return *this;
}

bool
HMAPICheckInfo::operator ==(const HMAPICheckInfo& k) const
{
    if((m_measurementOptions == k.m_measurementOptions)
        && (m_ipv4 == k.m_ipv4)
        && (m_ipv6 == k.m_ipv6)
        && (m_port == k.m_port)
        && (m_checkInfo == k.m_checkInfo)
        && (m_numCheckRetries == k.m_numCheckRetries)
        && (m_checkRetryDelay == k.m_checkRetryDelay)
        && (m_smoothingWindow == k.m_smoothingWindow)
        && (m_groupThreshold == k.m_groupThreshold)
        && (m_slowThreshold == k.m_slowThreshold)
        && (m_maxFlaps == k.m_maxFlaps)
        && (m_checkTimeout == k.m_checkTimeout)
        && (m_checkTTL == k.m_checkTTL)
        && (m_flapThreshold == k.m_flapThreshold)
        && (m_passthroughInfo == k.m_passthroughInfo)
        && (m_sourceAddress == k.m_sourceAddress)
        && (m_TOSValue == k.m_TOSValue))
    {
        return true;
    }
    return false;
}

bool
HMAPICheckInfo::operator !=(const HMAPICheckInfo& k) const
{
    return !(*this == k);
}

