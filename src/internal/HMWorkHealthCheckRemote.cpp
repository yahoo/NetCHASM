// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <curl/curl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <HMSocketUtilTCP.h>
#include <unistd.h>
#include <string.h>

#include "HMWork.h"
#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMLogBase.h"
#include "HMStateManager.h"
#include "HMControlBase.h"
#include "HMWorkHealthCheckRemote.h"
// LCOV_EXCL_START; Tested in functional testing

using namespace std;

bool
HMWorkHealthCheckRemote::getHostResult(HMState& state, const string& remoteHost, HMDataCheckResult& remote)
{
    timeval tv, tv_checkinfo;
    tv.tv_sec = state.getConnectionTimeout() / 1000;
    tv.tv_usec = 0;
    tv_checkinfo.tv_sec = 30;
    tv_checkinfo.tv_usec = 0;
    //Check if tcp connected successfully, if check-info is present check for returned data to match
    HMSocketUtilTCP socketAPI(remote.m_address, remote.m_port, tv, m_hostCheck.getSourceAddress(), m_hostCheck.getTOSValue());
    if(socketAPI.getReason() != HM_REASON_SUCCESS)
    {
        return false;
    }
    if (socketAPI.getHostResults(m_hostname, m_ipAddress, m_hostCheck, tv_checkinfo, m_hostResults) == HM_SOCK_DATA_OK)
    {
        if (this->m_hostCheck.getCheckType() != HM_CHECK_AUX_HTTP
                && this->m_hostCheck.getCheckType() != HM_CHECK_AUX_HTTPS
                && this->m_hostCheck.getCheckType()
                        != HM_CHECK_AUX_HTTPS_NO_PEER_CHECK)
        {
            return true;
        }
        HMLog(HM_LOG_DEBUG,
                "[TCPREMOTECHECK] Trying to get Load feedback Info from %s (%s) for host %s (%s)",
                remoteHost.c_str(), remote.m_address.toString().c_str(), m_hostname.c_str(),
                m_ipAddress.toString().c_str());
        HM_SOCK_DATA_STATUS status = socketAPI.getLoadFeedback(m_hostname,
                m_ipAddress, m_hostCheck, tv_checkinfo, m_auxInfo);
        if (status == HM_SOCK_DATA_OK || status == HM_SOCK_DATA_EMPTY)
        {
            HMLog(HM_LOG_DEBUG,
                    "[TCPREMOTECHECK] Obtained Load feedback Info from %s (%s) for host %s (%s)",
                    remoteHost.c_str(), remote.m_address.toString().c_str(), m_hostname.c_str(),
                    m_ipAddress.toString().c_str());
            return true;
        }
        HMLog(HM_LOG_DEBUG,
                "[TCPREMOTECHECK] Failed to get Load feedback Info from %s (%s) for host %s (%s)",
                remoteHost.c_str(), remote.m_address.toString().c_str(), m_hostname.c_str(),
                m_ipAddress.toString().c_str());
    }
    return false;
}

bool
HMWorkHealthCheckRemote::remoteCheck(HMState& state, HMDataHostGroup& dataHostGroup)
{
    HMDataCheckParams checkParams;
    HMDataHostCheck dataHostCheck;
    dataHostGroup.getCheckParameters(checkParams);
    dataHostGroup.getHostCheck(dataHostCheck);
    for(auto host = dataHostGroup.getHostList()->begin(); host != dataHostGroup.getHostList()->end(); ++host)
    {
        HMLog(HM_LOG_DEBUG,
                "[TCPREMOTECHECK] Trying to fetch results for %s (%s) from host %s",
                m_hostname.c_str(), m_ipAddress.toString().c_str(),
                host->c_str());
        set<HMIPAddress> addresses;
        HMDNSLookup dnsHostCheck(m_hostCheck.getDnsPlugin(), m_ipAddress.getType() == AF_INET6);
        state.m_dnsCache.getAddresses(*host, dataHostCheck.getDualStack(), dnsHostCheck,
                addresses);
        for (const HMIPAddress address : addresses)
        {
            HMLog(HM_LOG_DEBUG,
                    "[TCPREMOTECHECK] Trying to fetch results for %s (%s) from host %s (%s)",
                    m_hostname.c_str(), m_ipAddress.toString().c_str(),
                    host->c_str(), address.toString().c_str());
            HMDataCheckResult result;
            HMCheckHeader header(*host, address, dataHostCheck, checkParams);
            if (state.m_checkList.getCheckResult(header, result))
            {

                if (result.m_response != HM_RESPONSE_CONNECTED)
                {
                    HMLog(HM_LOG_DEBUG,
                            "[TCPREMOTECHECK] Failed to get results from %s (%s) for host %s (%s)",
                            host->c_str(), result.m_address.toString().c_str(),
                            m_hostname.c_str(), m_ipAddress.toString().c_str());
                    continue;
                }
                if (getHostResult(state, *host, result))
                {
                    HMLog(HM_LOG_DEBUG,
                            "[TCPREMOTECHECK] Obtained results from %s (%s) for host %s (%s)",
                            host->c_str(), result.m_address.toString().c_str(),
                            m_hostname.c_str(), m_ipAddress.toString().c_str());
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
                        "[TCPREMOTECHECK] Failed to get results for remote host %s (%s)",
                        host->c_str(), result.m_address.toString().c_str());
                continue;
            }
        }

    }
    return false;
}

HM_WORK_STATUS
HMWorkHealthCheckRemote::healthCheck()
{
    if (!m_hostCheck.getRemoteCheck().empty())
    {
        shared_ptr<HMState> currentState;
        m_stateManager->updateState(currentState);
        auto hostGroupIt = currentState->m_hostGroups.find(m_hostCheck.getRemoteCheck());
        if(hostGroupIt != currentState->m_hostGroups.end())
        {
            HMLog(HM_LOG_DEBUG3, "[TCPREMOTECHECK] Remote check for %s",
                    printCheckType(m_hostCheck.getCheckType()).c_str());
            string url;
            string checkInfo = m_hostCheck.getCheckInfo();
            //Check if tcp connected successfully, if check-info is present check for returned data to match
            m_start = HMTimeStamp::now();
            m_end = HMTimeStamp::now();
            m_reason = HM_REASON_NONE;
            m_response = HM_RESPONSE_FAILED;
            if (remoteCheck(*(currentState.get()), hostGroupIt->second))
            {
                return HM_WORK_COMPLETE_REMOTE;
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG,
                    "[TCPREMOTECHECK] Host group missing for remote check of %s (%s) for hostgroup %s",
                    m_hostname.c_str(), m_ipAddress.toString().c_str(),
                    m_hostCheck.getRemoteCheck().c_str());
        }
    }
    else
    {
        HMLog(HM_LOG_ERROR, "[TCPREMOTECHECK] Invalid Check type for TCPCheckClient %s",
                printCheckType(m_hostCheck.getCheckType()).c_str());
    }
    return HM_WORK_REMOTE_FALLBACK;
}
// LCOV_EXCL_STOP; Tested in functional testing
