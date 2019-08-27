// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <curl/curl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "HMWorkHealthCheckTCP.h"
#include "HMWork.h"
#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMLogBase.h"
#include "HMStateManager.h"

// LCOV_EXCL_START; Tested in functional testing

using namespace std;

HM_WORK_STATUS
HMWorkHealthCheckTCP::healthCheck()
{
    if(m_hostCheck.getCheckType() == HM_CHECK_TCP)
    {
        shared_ptr<HMState> currentState;
        m_stateManager->updateState(currentState);

        HMLog(HM_LOG_DEBUG3, "[TCPCHECK] Raw Socket check TCP for %s",
                printCheckType(m_hostCheck.getCheckType()).c_str());

        string url;
        string checkInfo = m_hostCheck.getCheckInfo();
        //Check if tcp connected successfully, if check-info is present check for returned data to match
        m_start = HMTimeStamp::now();
        m_end = HMTimeStamp::now();
        m_reason = HM_REASON_NONE;
        m_response = HM_RESPONSE_FAILED;
        timeval tv, tv_checkinfo;
        tv.tv_sec = currentState->getConnectionTimeout() / 1000;
        tv.tv_usec = 0;
        tv_checkinfo.tv_sec = 30;
        tv_checkinfo.tv_usec = 0;
        HMSocketUtilTCP socketApi(m_ipAddress, m_hostCheck.getPort(), tv, m_hostCheck.getSourceAddress(), m_hostCheck.getTOSValue());
        m_reason = socketApi.getReason();
        switch (m_reason)
        {
        case HM_REASON_INTERNAL_ERROR:
            HMLog(HM_LOG_ERROR, "[TCPCHECK] %s - HostName = %s(%s), checkInfo = %s",
                    socketApi.getErrorMsg().c_str(), m_hostname.c_str(), m_ipAddress.toString().c_str(),
                    checkInfo.c_str());
            break;
        case HM_REASON_CONNECT_TIMEOUT:
            HMLog(HM_LOG_DEBUG3,
                    "[TCPCHECK] TCP connect timeout for host:%s(%s), port:%hu",
                    m_hostname.c_str(), m_ipAddress.toString().c_str(), m_hostCheck.getPort());
            break;
        case HM_REASON_CONNECT_FAILURE:
            HMLog(HM_LOG_DEBUG3,
                    "[TCPCHECK] TCP connect failed for host:%s(%s), port:%hu",
                    m_hostname.c_str(), m_ipAddress.toString().c_str(), m_hostCheck.getPort());
            break;
        case HM_REASON_SUCCESS:
            m_response = HM_RESPONSE_CONNECTED;
            m_reason = HM_REASON_SUCCESS;
            m_end = socketApi.getConnectTime();
            HMLog(HM_LOG_DEBUG3,
                    "[TCPCHECK] Connection successful to host:%s(%s), port:%hu",
                    m_hostname.c_str(), m_ipAddress.toString().c_str(), m_hostCheck.getPort());
            if (checkInfo.empty())
            {
                HMLog(HM_LOG_DEBUG3,
                        "[TCPCHECK] Health check successful for host:%s(%s), port:%hu",
                        m_hostname.c_str(), m_ipAddress.toString().c_str(), m_hostCheck.getPort());
                break;
            }
            else if (checkInfo == HM_MASTER_HEALTH_CHECK_COMMAND)
            {
                m_reason = HM_REASON_REQUEST_FAILURE;
                m_response = HM_RESPONSE_FAILED;
                if (socketApi.pingRemoteHost(tv_checkinfo))
                {
                    HMLog(HM_LOG_DEBUG3,
                            "[TLSCHECK] Health check successful for host:%s(%s), port:%hu",
                            m_hostname.c_str(), m_ipAddress.toString().c_str(),
                            m_hostCheck.getPort());
                    m_response = HM_RESPONSE_CONNECTED;
                }
                m_reason = socketApi.getReason();
            }
            else
            {
                m_reason = HM_REASON_NONE;
                m_response = HM_RESPONSE_FAILED;
                HMLog(HM_LOG_DEBUG3,
                        "[TCPCHECK] checkInfo is present and Ready to read");
                string recvCheckInfo;
                recvCheckInfo.resize(m_hostCheck.getCheckInfo().length());
                socketApi.getCheckInfo(recvCheckInfo,
                        m_hostCheck.getCheckInfo().length(), tv_checkinfo);
                m_reason = socketApi.getReason();
                switch (m_reason)
                {
                case HM_REASON_INTERNAL_ERROR:
                    HMLog(HM_LOG_ERROR, "[TCPCHECK] %s - HostName = %s(%s), checkInfo = %s",
                            socketApi.getErrorMsg().c_str(), m_hostname.c_str(), m_ipAddress.toString().c_str(),
                            recvCheckInfo.c_str());
                    break;
                case HM_REASON_RESPONSE_TIMEOUT:
                    HMLog(HM_LOG_DEBUG,
                            "[TCPCHECK] TCP connect timeout for host:%s(%s), port:%hu",
                            m_hostname.c_str(), m_ipAddress.toString().c_str(),m_hostCheck.getPort());
                    break;
                case HM_REASON_SUCCESS:
                    if (recvCheckInfo == m_hostCheck.getCheckInfo())
                    {
                        HMLog(HM_LOG_DEBUG3,
                                "[TCPCHECK] Health check successful for host:%s(%s), port:%hu, checkInfo:%s",
                                m_hostname.c_str(), m_ipAddress.toString().c_str(), m_hostCheck.getPort(),
                                m_hostCheck.getCheckInfo().c_str());
                        m_response = HM_RESPONSE_CONNECTED;
                    }
                    else
                    {
                        HMLog(HM_LOG_DEBUG3,
                                "[TCPCHECK] tcp response for %s(%s) did not match CheckInfo %s",
                                m_hostname.c_str(), m_ipAddress.toString().c_str(), recvCheckInfo.c_str());
                        m_reason = HM_REASON_NONE;
                    }
                default:
                    break;
                };
            }
        default:
            break;
        }
    }//tcp ends
    else
    {
        HMLog(HM_LOG_ERROR, "[TCPCHECK] Invalid Check type for TCPCheck %s",
                printCheckType(m_hostCheck.getCheckType()).c_str());
    }
    return HM_WORK_COMPLETE;
}
// LCOV_EXCL_STOP; Tested in functional testing
