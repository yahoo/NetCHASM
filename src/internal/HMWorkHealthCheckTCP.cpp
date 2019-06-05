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

void
HMWorkHealthCheckTCP::errorMsg(string strBuf)
{
    error_code ec (errno,generic_category());
    throw system_error(ec, strBuf);
}

HM_WORK_STATUS
HMWorkHealthCheckTCP::healthCheck()
{
    if(m_hostCheck.getCheckType() == HM_CHECK_TCP)
    {
        shared_ptr<HMState> currentState;
        m_stateManager->updateState(currentState);

        HMLog(HM_LOG_DEBUG3, "[TCPCHECK] Raw Socket check TCP for %s", printCheckType(m_hostCheck.getCheckType()).c_str());


        string url;
        int sock;
        string checkInfo = m_hostCheck.getCheckInfo();

        struct sockaddr_storage ss;
        socklen_t salen = sizeof(ss);

        struct sockaddr* sa = m_ipAddress.getSockaddr(&ss,&salen,m_hostCheck.getPort());

        //Check if tcp connected successfully, if check-info is present check for returned data to match
        m_start = HMTimeStamp::now();
        m_end = HMTimeStamp::now();
        m_reason = HM_REASON_NONE;
        m_response = HM_RESPONSE_FAILED;
        sock = socket(m_ipAddress.getType(),SOCK_STREAM,0);
        try
        {
            if(sock < 0)
            {
                m_reason = HM_REASON_INTERNAL_ERROR;
                errorMsg("[TCPCHECK] tcp check socket() ");
            }
            if(fcntl(sock,F_SETFL,O_NONBLOCK) < 0)//NON-BLOCKING
            {
                m_reason = HM_REASON_INTERNAL_ERROR;
                errorMsg("[TCPCHECK] tcp check fcntl() ");
            }

            if((connect(sock,sa,salen)) < 0
                           && errno != EINPROGRESS)
            {
                HMLog(HM_LOG_ERROR,"[TCPCHECK] tcp check connect");
                m_reason = HM_REASON_CONNECT_FAILURE;
                errorMsg("[TCPCHECK] tcp check connect ");
            }
            int ret,tret ;
            timeval tv, tv_checkinfo;

            tv.tv_sec = currentState->getConnectionTimeout() / 1000;
            tv.tv_usec = 0;
            tv_checkinfo.tv_sec = 30;
            tv_checkinfo.tv_usec = 0;

            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(sock,&fds);
            // either connected or in progress

            // using a write file descriptor to control the timeout
            ret = select(sock + 1, NULL, (&fds), NULL, &tv);

            if(ret < 0)
            {
                m_reason = HM_REASON_CONNECT_FAILURE;
                errorMsg("[TCPCHECK] select error ");
            }
            else if(ret == 0)
            {
                m_reason = HM_REASON_CONNECT_TIMEOUT;
                HMLog(HM_LOG_DEBUG3, "[TCPCHECK] TCP connect timeout for host:%s, post:%hu", m_hostname.c_str(), m_hostCheck.getPort());
            }
            else
            {
                int v =0;
                socklen_t vlen = sizeof(v);
                int size_buf = checkInfo.length();
                unique_ptr<char[]> buf = unique_ptr<char[]>(new char [size_buf]);

                if(FD_ISSET(sock, &fds))
                {
                    if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &v, &vlen) < 0)
                    {
                        m_reason = HM_REASON_INTERNAL_ERROR;
                        errorMsg("[TCPCHECK] tcp check getsockopt ");
                    }
                    else if(v == ETIMEDOUT)
                    {
                        m_reason = HM_REASON_CONNECT_TIMEOUT;
                        HMLog(HM_LOG_DEBUG3, "[TCPCHECK] TCP connect timeout in select for host:%s, post:%hu",
                                m_hostname.c_str(),
                                m_hostCheck.getPort());
                    }
                    else if(!v)
                    {
                        m_response = HM_RESPONSE_CONNECTED;
                        m_reason = HM_REASON_SUCCESS;
                        m_end = HMTimeStamp::now();
                        HMLog(HM_LOG_DEBUG3, "[TCPCHECK] Connection successful to host:%s, post:%hu",
                                m_hostname.c_str(),
                                m_hostCheck.getPort());
                    }
                    else
                    {
                        m_reason = HM_REASON_CONNECT_FAILURE;
                        HMLog(HM_LOG_DEBUG3, "[TCPCHECK] TCP connect failed for host:%s, post:%hu",
                                m_hostname.c_str(),
                                m_hostCheck.getPort());
                    }

                    if((m_response == HM_RESPONSE_CONNECTED) && checkInfo.empty())
                    {
                        HMLog(HM_LOG_DEBUG3, "[TCPCHECK] Health check successful for host:%s, post:%hu",
                                m_hostname.c_str(),
                                m_hostCheck.getPort());
                    }
                    else if((m_response == HM_RESPONSE_CONNECTED) && !checkInfo.empty())
                    {
                        // We need to reset the reason and response for checkinfo matching
                        m_reason = HM_REASON_NONE;
                        m_response = HM_RESPONSE_FAILED;
                        HMLog(HM_LOG_DEBUG3,"[TCPCHECK] checkInfo is present and Ready to read");

                        ret = 0;

                        /*
                         * TCP socket doesn't read the full data in one shot
                         * keep reading until we read desired number of bytes(checkInfo size)
                         */

                        while (ret < size_buf)
                        {
                            fd_set fdsr;
                            FD_ZERO(&fdsr);
                            FD_SET(sock,&fdsr);
                            // either connected or in progress
                            int s_ret = select(sock+1,(&fdsr),NULL,NULL,&tv_checkinfo);

                            if(s_ret < 0)
                            {
                                m_reason = HM_REASON_INTERNAL_ERROR;
                                errorMsg("[TCPCHECK] select error ");
                                break;
                            }
                            else if(s_ret == 0)
                            {
                                m_reason = HM_REASON_RESPONSE_TIMEOUT;
                                HMLog(HM_LOG_DEBUG, "[TCPCHECK] TCP connect timeout for host:%s, post:%hu",
                                        m_hostname.c_str(),
                                        m_hostCheck.getPort());
                                break;
                            }
                            else if(FD_ISSET(sock, &fdsr))
                            {
                                HMLog(HM_LOG_DEBUG3, "[TCPCHECK] Ready to read");
                                if((tret = read(sock,buf.get()+ret,size_buf-ret)) < 0)
                                {
                                    m_reason = HM_REASON_INTERNAL_ERROR;
                                    errorMsg("[TCPCHECK] read failure ");
                                }
                                else if(tret == 0)
                                {
                                    HMLog(HM_LOG_DEBUG3, "[TCPCHECK] TCP read - No more data to send ,shutdown on other end");
                                    break;
                                }
                                else
                                {
                                    ret += tret;
                                    HMLog(HM_LOG_DEBUG3, "[TCPCHECK] TCP bytes %d received", tret);
                                }
                            }
                            else
                            {
                                HMLog(HM_LOG_DEBUG3, "[TCPCHECK] Not ready to read");
                            }
                        }

                        if(ret >= (int)checkInfo.length()
                            && !strncmp(buf.get(),checkInfo.c_str(),size_buf))
                        {
                            HMLog(HM_LOG_DEBUG3, "[TCPCHECK] Health check successful for host:%s, post:%hu, checkInfo:%s",
                                    m_hostname.c_str(),
                                    m_hostCheck.getPort(),
                                    m_hostCheck.getCheckInfo().c_str());
                            m_reason = HM_REASON_SUCCESS;
                            m_response = HM_RESPONSE_CONNECTED;
                        }
                        else
                        {
                            HMLog(HM_LOG_DEBUG3, "[TCPCHECK] tcp response for %s did not match CheckInfo %s",
                                    m_hostname.c_str(),
                                    checkInfo.c_str());
                        }
                    }
                }
            }
        }

        catch (system_error& ex)
        {
            string what((ex.what()));
            HMLog(HM_LOG_ERROR, "%s for %s with checkinfo %s : %d",
                    what.c_str(),
                    m_hostname.c_str(),
                    checkInfo.c_str(),
                    ex.code().value());
        }
        close(sock);
    }//tcp ends
    else
    {
        HMLog(HM_LOG_ERROR, "[TCPCHECK] Invalid Check type for TCPCheck %s",
                printCheckType(m_hostCheck.getCheckType()).c_str());
    }
    return HM_WORK_COMPLETE;
}
// LCOV_EXCL_STOP; Tested in functional testing
