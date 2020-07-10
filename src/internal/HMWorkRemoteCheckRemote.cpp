// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include "HMIPAddress.h"
#include "HMLogBase.h"
#include "HMWork.h"
#include "HMStateManager.h"
#include "HMSocketUtilTCP.h"
#include "HMSocketUtilTCPS.h"
#include "HMWorkRemoteCheckRemote.h"

// LCOV_EXCL_START; Tested in functional testing

using namespace std;

bool
HMWorkRemoteCheckRemote::getHostResult(HMState& state, const string& remoteHost, HMDataCheckResult& remote)
{
    timeval tv, tv_checkinfo;
    tv.tv_sec = state.getConnectionTimeout() / 1000;
    tv.tv_usec = 0;
    tv_checkinfo.tv_sec = 30;
    tv_checkinfo.tv_usec = 0;

    shared_ptr<HMSocketUtilBase> socketAPI;
    HMConnectionCheck check(remote.m_port, remote.m_address, m_hostCheck.getRemoteCheckType(), m_hostCheck.getSourceAddress(), m_hostCheck.getTOSValue());
    //Check if tcp connected successfully, if check-info is present check for returned data to match
    switch(m_hostCheck.getRemoteCheckType())
    {
    case HM_REMOTE_CHECK_TCP:
        socketAPI = make_shared<HMSocketUtilTCP>(remote.m_address, remote.m_port, tv, m_hostCheck.getSourceAddress(), m_hostCheck.getTOSValue(), false);
        break;
    case HM_REMOTE_CHECK_TCPS:
        socketAPI = make_shared<HMSocketUtilTCPS>(state.m_ctx->getCtx(), remote.m_address, remote.m_port, tv, m_hostCheck.getSourceAddress(), m_hostCheck.getTOSValue(), false);
        break;
    case HM_REMOTE_SHARED_CHECK_TCP:
    case HM_REMOTE_SHARED_CHECK_TCPS:
        state.m_connectionHandler->getConnection(check, socketAPI);
        break;
    case HM_REMOTE_SHARED_CHECK_LINUX:
    case HM_REMOTE_CHECK_NONE:
        HMLog(HM_LOG_ERROR,
                "[%s-%s] Invalid Remote CheckType-%s HostName = %s(%s) Reason: %s",
                printFlowType(m_hostCheck.getFlowType()).c_str(),
                printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(), remoteHost.c_str(), remote.m_address.toString().c_str(),
                printReason(socketAPI->getReason()).c_str());
        break;
    }
    if(!socketAPI)
    {
        HMLog(HM_LOG_ERROR,
            "[%s-%s] Failed to create socket instance to remote host HostName = %s(%s) Reason: %s",
            printFlowType(m_hostCheck.getFlowType()).c_str(),
            printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
            remoteHost.c_str(), remote.m_address.toString().c_str(),
            printReason(socketAPI->getReason()).c_str());
        return false;
    }

    // Connect to remote server for non shared checks
    switch(m_hostCheck.getRemoteCheckType())
    {
    case HM_REMOTE_CHECK_TCP:
    case HM_REMOTE_CHECK_TCPS:
        socketAPI->connectServer();
        if (socketAPI->getReason() != HM_REASON_SUCCESS)
        {
            HMLog(HM_LOG_ERROR,
                    "[%s-%s] Connection Failure to remote host HostName = %s(%s) Reason: %s",
                    printFlowType(m_hostCheck.getFlowType()).c_str(),
                    printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                    remoteHost.c_str(), remote.m_address.toString().c_str(),
                    printReason(socketAPI->getReason()).c_str());
            return false;
        }
        break;
    default:
        break;
    }

    if (socketAPI->getHostGroupResults(m_hostname, tv_checkinfo, m_hash, m_results) == HM_SOCK_DATA_OK)
    {
        if (this->m_hostCheck.getCheckType() != HM_CHECK_AUX_HTTP
                && this->m_hostCheck.getCheckType() != HM_CHECK_AUX_HTTPS
                && this->m_hostCheck.getCheckType()
                        != HM_CHECK_AUX_HTTPS_NO_PEER_CHECK)
        {
            return true;
        }
        HMLog(HM_LOG_DEBUG,
                "[%s-%s] Trying to get Load feedback Info from %s(%s) for hostgroup %s",
                printFlowType(m_hostCheck.getFlowType()).c_str(),
                printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                remoteHost.c_str(), remote.m_address.toString().c_str(), m_hostname.c_str());
        HM_SOCK_DATA_STATUS status = socketAPI->getLoadFeedback(m_hostname,
                tv_checkinfo, m_hash, m_auxResults);
        if (status == HM_SOCK_DATA_OK || status == HM_SOCK_DATA_EMPTY)
        {
            HMLog(HM_LOG_DEBUG,
                    "[%s-%s] Obtained Load feedback Info from %s (%s) for hostgroup %s",
                    printFlowType(m_hostCheck.getFlowType()).c_str(),
                    printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                    remoteHost.c_str(), remote.m_address.toString().c_str(), m_hostname.c_str());
            return true;
        }
        HMLog(HM_LOG_DEBUG,
                "[%s-%s] Failed to get Load feedback Info from %s (%s) for hostgroup %s",
                printFlowType(m_hostCheck.getFlowType()).c_str(),
                printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                remoteHost.c_str(), remote.m_address.toString().c_str(), m_hostname.c_str());
    }
    return false;
}

bool
HMWorkRemoteCheckRemote::remoteCheck(HMState& state, HMDataHostGroup& dataHostGroup)
{
    HMDataCheckParams checkParams;
    HMDataHostCheck dataHostCheck;
    dataHostGroup.getCheckParameters(checkParams);
    dataHostGroup.getHostCheck(dataHostCheck);
    for(auto host = dataHostGroup.getHostList()->begin(); host != dataHostGroup.getHostList()->end(); ++host)
    {
        HMLog(HM_LOG_DEBUG,
                "[%s-%s] Trying to fetch results for %s from hostgroup %s",
                printFlowType(m_hostCheck.getFlowType()).c_str(),
                printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                m_hostname.c_str(), host->c_str());
        set<HMIPAddress> addresses;
        HMDNSLookup dnsHostCheck(dataHostCheck.getDnsType(), m_ipAddress.getType() == AF_INET6, dataHostCheck.getRemoteCheck());
        state.m_dnsCache.getAddresses(*host, dataHostCheck.getDualStack(), dnsHostCheck,
                addresses);
        for (const HMIPAddress address : addresses)
        {
            HMLog(HM_LOG_DEBUG,
                    "[%s-%s] Trying to fetch results for %s from host %s (%s)",
                    printFlowType(m_hostCheck.getFlowType()).c_str(),
                    printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                    m_hostname.c_str(), host->c_str(),
                    address.toString().c_str());
            HMDataCheckResult result;
            HMCheckHeader header(*host, address, dataHostCheck, checkParams);
            if (state.m_checkList.getCheckResult(header, result))
            {

                if (result.m_response != HM_RESPONSE_CONNECTED)
                {
                    HMLog(HM_LOG_DEBUG,
                            "[%s-%s] Failed to get results from %s for hostgroup %s",
                            printFlowType(m_hostCheck.getFlowType()).c_str(),
                            printRemoteCheckType(
                                    m_hostCheck.getRemoteCheckType()).c_str(),
                            host->c_str(), m_hostname.c_str());
                    continue;
                }
                if (getHostResult(state, *host, result))
                {
                    HMLog(HM_LOG_DEBUG,
                            "[%s-%s] Obtained results from %s (%s) for hostgroup %s",
                            printFlowType(m_hostCheck.getFlowType()).c_str(),
                            printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                            host->c_str(), result.m_address.toString().c_str(),
                            m_hostname.c_str());
                    return true;
                }
                if (!(m_hostCheck.getDistributedFallBack()
                        & HM_DISTRIBUTED_FALLBACK_LOCAL))
                {
                    return false;
                }
            }
            else
            {
                HMLog(HM_LOG_DEBUG,
                        "[%s-%s] Failed to get results for hostgroup %s from remote host %s (%s)",
                        printFlowType(m_hostCheck.getFlowType()).c_str(),
                        printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                        m_hostname.c_str(),
                        host->c_str(), result.m_address.toString().c_str());
                continue;
            }
        }

    }
    return false;
}

void
HMWorkRemoteCheckRemote::init(HMWorkState& state)
{
    (void)state;
}

HM_WORK_STATUS
HMWorkRemoteCheckRemote::remoteLookup()
{

    if (!m_hostCheck.getRemoteCheck().empty())
    {
        m_start = HMTimeStamp::now();
        m_end = HMTimeStamp::now();
        m_reason = HM_REASON_NONE;
        m_response = HM_RESPONSE_FAILED;
        shared_ptr<HMState> currentState;
        m_stateManager->updateState(currentState);
        auto hostGroupIt = currentState->m_hostGroups.find(m_hostCheck.getRemoteCheck());
        if (hostGroupIt != currentState->m_hostGroups.end())
        {

            HMLog(HM_LOG_DEBUG3, "[%s-%s] Remote check for %s",
                    printFlowType(m_hostCheck.getFlowType()).c_str(),
                    printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                    printCheckType(m_hostCheck.getCheckType()).c_str());
            string url;
            string checkInfo = m_hostCheck.getCheckInfo();
            //Check if tcp connected successfully, if check-info is present check for returned data to match
            if (remoteCheck(*(currentState.get()), hostGroupIt->second))
            {
                HMLog(HM_LOG_DEBUG,
                        "[%s-%s] Obtained results for hostgroup %s",
                        printFlowType(m_hostCheck.getFlowType()).c_str(),
                        printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                        m_hostname.c_str());
                return HM_WORK_COMPLETE_REMOTE;
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG,
                    "[%s-%s] Host group missing for remote check of %s (%s) for hostgroup %s",
                    printFlowType(m_hostCheck.getFlowType()).c_str(),
                    printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                    m_hostname.c_str(), m_ipAddress.toString().c_str(),
                    m_hostCheck.getRemoteCheck().c_str());
        }
    }
    else
    {
        HMLog(HM_LOG_ERROR,
                "[%s-%s] Invalid Check type for TCPCheckClient %s",
                printFlowType(m_hostCheck.getFlowType()).c_str(),
                printRemoteCheckType(m_hostCheck.getRemoteCheckType()).c_str(),
                printCheckType(m_hostCheck.getCheckType()).c_str());
    }
    return HM_WORK_REMOTE_FALLBACK;
}

// LCOV_EXCL_STOP; Tested in functional testing
