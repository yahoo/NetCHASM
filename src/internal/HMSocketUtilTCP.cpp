// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <future>
#include <fcntl.h>

#include "HMSocketUtilTCP.h"

using namespace std;

string getSystemError()
{
    error_code ec (errno,generic_category());
    return ec.message();
}


bool
HMSocketUtilTCP::createSocket()
{
    if ((m_socket = socket(m_address.getType(), SOCK_STREAM, 0)) == -1)
    {
        m_errorMsg = "tcp check socket() " + getSystemError();
        m_reason = HM_REASON_INTERNAL_ERROR;
        return false;
    }
    if (fcntl(m_socket, F_SETFL, O_NONBLOCK) < 0)
    {
        m_errorMsg = "tcp check fcntl() " + getSystemError();
        m_reason = HM_REASON_INTERNAL_ERROR;
        return false;
    }
    if(m_tos)
    {
        if (setsockopt(m_socket, IPPROTO_IP, IP_TOS, &m_tos, sizeof(m_tos)) < 0)
        {
            m_errorMsg = "Error setting TOS: ", getSystemError();
            m_reason = HM_REASON_INTERNAL_ERROR;
            return false;
        }
    }

    struct sockaddr_storage localAddress;
    bool needBind = false;
    if(m_sourceAddress.getType() == AF_INET6)
    {
        sockaddr_in6* addr = (sockaddr_in6*)&localAddress;
        addr->sin6_family = AF_INET6;
        addr->sin6_addr = m_sourceAddress.addr6();
        addr->sin6_port = 0;
        needBind = true;
    }
    else if(m_sourceAddress.getType() == AF_INET)
    {
        sockaddr_in* addr = (sockaddr_in*) &localAddress;
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = m_sourceAddress.addr4();
        addr->sin_port = 0;
        needBind = true;
    }
    if (needBind)
    {
        if (bind(m_socket, (struct sockaddr *) &localAddress, sizeof(localAddress)) < 0)
        {
            //LCOV_EXCL_START;
            HMLog(HM_LOG_ERROR, "[SocketUtil] Failed to bind source IP for %s", m_sourceAddress.toString().c_str());
            //LCOV_EXCL_STOP;
        }
    }
    return true;
}

void
HMSocketUtilTCP::closeSocket()
{
    if (m_socket != -1)
    {
        close(m_socket);
        m_connected = false;
        m_socket = -1;
    }
}

bool
HMSocketUtilTCP::getCheckInfo(std::string& checkinfo, uint32_t size, timeval& tv) {
    return recvData(&checkinfo.at(0), size, tv);
}

bool
HMSocketUtilTCP::connectSocket()
{
    if (!createSocket())
    {
        return false;
    }
    sockaddr_storage server;
    socklen_t len;
    if(m_address.getType() == AF_INET6)
    {
        sockaddr_in6 *addr = (sockaddr_in6 *)&server;
        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(m_port);
        addr->sin6_addr = m_address.addr6();
        len = sizeof(sockaddr_in6);
    }
    else
    {
        sockaddr_in *addr = (sockaddr_in *)&server;
        addr->sin_family = AF_INET;
        addr->sin_port = htons(m_port);
        addr->sin_addr.s_addr = m_address.addr4();
        len = sizeof(sockaddr_in);
    }
    if (connect(m_socket, (struct sockaddr *) &server, len)
            < 0&& errno != EINPROGRESS)
    {
        m_reason = HM_REASON_CONNECT_FAILURE;
        m_errorMsg = "tcp check connect " + getSystemError();
        return false;
    }
    int ret;
    fd_set fdrs, fdws;
    FD_ZERO(&fdrs);
    FD_ZERO(&fdws);
    FD_SET(m_socket, &fdrs);
    FD_SET(m_socket, &fdws);
    timeval tv = m_connectTimeInfo;
    ret = select(m_socket + 1, (&fdrs), (&fdws), NULL, &tv);
    if (ret < 0)
    {
        m_errorMsg = "select error " + getSystemError();
        m_reason = HM_REASON_CONNECT_FAILURE;
    }
    else if (ret == 0)
    {
        m_reason = HM_REASON_CONNECT_TIMEOUT;
    }
    else
    {
        int v = 0;
        socklen_t vlen = sizeof(v);
        if (FD_ISSET(m_socket, &fdrs) || FD_ISSET(m_socket, &fdws))
        {
            if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &v, &vlen) < 0)
            {
                m_reason = HM_REASON_INTERNAL_ERROR;
                m_errorMsg = "tcp check getsockopt " + getSystemError();
            }
            else if (v == ETIMEDOUT)
            {
                m_reason = HM_REASON_CONNECT_TIMEOUT;
            }
            else if (!v)
            {
                m_reason = HM_REASON_SUCCESS;
                struct sockaddr_in sin;
                socklen_t len = sizeof(sin);
                if (getsockname(m_socket, (struct sockaddr *)&sin, &len) == 0)
                {
                    m_clientPort = ntohs(sin.sin_port);
                }
                m_connectTime = HMTimeStamp::now();
                m_connected = true;
                return true;
            }
            else
            {
                m_reason = HM_REASON_CONNECT_FAILURE;
            }
        }
    }
    return false;
}

HMSocketUtilTCP::~HMSocketUtilTCP()
{
    closeSocket();
}

void HMSocketUtilTCP::reconnect()
{
    closeSocket();
    if (connectSocket())
    {
        setConnectionReset(false);
        if(isPersistent())
        {
            openPersistant();
        }
    }
}


bool
HMSocketUtilTCP::sendData(const char* buffer, uint64_t size)
{
    if (!m_connected)
    {
        return false;
    }
    int tsize = send(m_socket, buffer, size, 0);
    if ( tsize == (int)size)
    {
        return true;
    }
    HMLog(HM_LOG_ERROR, "[TCPNBAPI] Failed to send data [sent:%d, Expected:%d]", tsize, size);
    return false;
}

HM_SOCK_DATA_STATUS
HMSocketUtilTCP::recvData(char* data, uint64_t size, timeval tv)
{
    if (!m_connected)
    {
        return HM_SOCK_DATA_FAILED;
    }
    uint64_t ret = 0;
    int32_t tret = 0;
    while (ret < size)
    {
        fd_set fdsr;
        FD_ZERO(&fdsr);
        FD_SET(m_socket, &fdsr);
        // either connected or in progress
        int s_ret = select(m_socket + 1, (&fdsr), NULL, NULL, &tv);
        if (s_ret < 0)
        {
            m_reason = HM_REASON_INTERNAL_ERROR;
            m_errorMsg = "select error ";
            return HM_SOCK_DATA_FAILED;
        }
        else if (s_ret == 0)
        {
            m_reason = HM_REASON_RESPONSE_TIMEOUT;
            return HM_SOCK_DATA_TIMEOUT;
        }
        else if (FD_ISSET(m_socket, &fdsr))
        {
            //HMLog(HM_LOG_DEBUG3, "[TCPCHECK] Ready to read");
            if ((tret = read(m_socket, data + ret, size - ret)) < 0)
            {
                m_reason = HM_REASON_INTERNAL_ERROR;
                return HM_SOCK_DATA_FAILED;
                //errorMsg("[TCPCHECK] read failure ");
            }
            else if (tret == 0)
            {
                HMLog(HM_LOG_DEBUG3,
                        "[TCPNBAPI] TCP read - No more data to send ,shutdown on other end");
                return HM_SOCK_DATA_FAILED;
                break;
            }
            else
            {
                ret += tret;
                HMLog(HM_LOG_DEBUG3, "[TCPNBAPI] TCP bytes %d received", tret);
            }
        }
        if (ret == size)
        {
            m_reason = HM_REASON_SUCCESS;
        }
    }
    if(ret != size)
    {
        return HM_SOCK_DATA_FAILED;
    }
    return HM_SOCK_DATA_OK;
}
