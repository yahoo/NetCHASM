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
        m_errorMsg = "tcp check socket() " + getSystemError();
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
    }
}

HM_REASON
HMSocketUtilTCP::getReason() const {
    return m_reason;
}

const std::string&
HMSocketUtilTCP::getErrorMsg() const {
    return m_errorMsg;
}

const HMTimeStamp&
HMSocketUtilTCP::getConnectTime() const {
    return m_connectTime;
}

bool
HMSocketUtilTCP::getCheckInfo(std::string& checkinfo, uint32_t size, timeval& tv) {
    return recvData(&checkinfo.at(0), size, tv);
}

bool
HMSocketUtilTCP::pingRemoteHost(timeval& tv)
{
    HMDataPacking dataPacking;
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_GETREMOTEQUERY;
    if (sendCommand(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (recvData((char*) &packetSize, sizeof(packetSize), tv))
        {
            // when bool is false a empty packet is returned.
            if (packetSize == 0)
            {
                m_reason = HM_REASON_RESPONSE_DOWN;
                return false;
            }
            packetSize = dataPacking.ntoh64(packetSize);
            std::unique_ptr<char[]> recvdData = make_unique<char[]>(packetSize);
            if (recvData(recvdData.get(), packetSize, tv))
            {
                if (dataPacking.unpackBool(recvdData, packetSize))
                {
                    m_reason = HM_REASON_SUCCESS;
                    return true;
                }
                else
                {
                    m_reason = HM_REASON_RESPONSE_DOWN;
                    return false;
                }
            }
        }
    }
    return false;
}

HM_SOCK_DATA_STATUS
HMSocketUtilTCP::getLoadFeedback(string& hostName, HMIPAddress& address, HMDataHostCheck& dataHostCheck, timeval &tv, HMAuxInfo& auxData)
{
    HMDataPacking dataPacking;
    uint64_t dataSize;
    std::unique_ptr<char[]> data = dataPacking.packDataHostCheck(dataHostCheck, dataSize);
    string cmd = std::to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_LOADFBIP + " " + hostName + " " + dataHostCheck.getCheckInfo() + " " + address.toString()
            + " " + to_string(dataSize);
    if (sendCommand(cmd))
    {
        uint64_t packetSize = 0;
        if (recvData((char*) &packetSize, sizeof(packetSize), tv))
        {
            if (packetSize == 0)
            {
                HMLog(HM_LOG_DEBUG,
                        "[SocketUtil] Remote aux result fetch received packet size 0 for %s(%s)",
                        hostName.c_str(), address.toString().c_str());
                return HM_SOCK_DATA_EMPTY;
            }
            packetSize = dataPacking.ntoh64(packetSize);
            std::unique_ptr<char[]> recvdData = make_unique<char[]>(packetSize);
            if (recvData(recvdData.get(), packetSize, tv))
            {
                if (dataPacking.unpackAuxInfo(recvdData, packetSize,
                        auxData))
                {
                    return HM_SOCK_DATA_OK;
                }
            }
        }
    }
    HMLog(HM_LOG_DEBUG, "[SocketUtil] Failed to receive aux results from remote host for %s(%s)", hostName.c_str(), address.toString().c_str());
    return HM_SOCK_DATA_FAILED;
}


HM_SOCK_DATA_STATUS
HMSocketUtilTCP::getHostResults(string& hostName, HMIPAddress& address, HMDataHostCheck& dataHostCheck, timeval &tv, map<HMDataCheckParams, HMDataCheckResult>& hostResults)
{
    hostResults.clear();
    HMDataPacking dataPacking;
    uint64_t dataSize;
    std::unique_ptr<char[]> data = dataPacking.packDataHostCheck(dataHostCheck, dataSize);
    string cmd = std::to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_HOSTIPRESULTS + " " + hostName + " " + address.toString()
            + " " + to_string(dataSize);
    if (sendCommand(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize = 0;
            if (recvData((char*) &packetSize, sizeof(packetSize), tv))
            {
                if (packetSize == 0)
                {
                    HMLog(HM_LOG_DEBUG, "[SocketUtil] Remote result fetch received packet size 0 for %s(%s)", hostName.c_str(), address.toString().c_str());
                    return HM_SOCK_DATA_EMPTY;
                }
                packetSize = dataPacking.ntoh64(packetSize);
                std::unique_ptr<char[]> recvdData = make_unique<char[]>(packetSize);
                if(recvData(recvdData.get(), packetSize, tv))
                {
                    if(dataPacking.unpackHostResults(recvdData, packetSize, hostResults))
                    {
                        return HM_SOCK_DATA_OK;
                    }
                }
            }
        }
    }
    HMLog(HM_LOG_DEBUG, "[SocketUtil] Failed to receive results from remote host for %s(%s)", hostName.c_str(), address.toString().c_str());
    return HM_SOCK_DATA_FAILED;
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
    ret = select(m_socket + 1, (&fdrs), (&fdws), NULL, &m_connectTimeInfo);
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
                m_connectTime = HMTimeStamp::now();
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

HMSocketUtilTCP::HMSocketUtilTCP(HMIPAddress& address, uint16_t port, timeval &timeInfo, const HMIPAddress& sourceAddress, const uint8_t tosValue) :
        HMSocketUtilBase(false, -1),
        m_reason(HM_REASON_NONE),
        m_connectTimeInfo(timeInfo),
        m_address(address),
        m_sourceAddress(sourceAddress),
        m_port(port),
        m_tos(tosValue)

{
    if (connectSocket())
    {
        m_connected = true;
    }
}

HMSocketUtilTCP::HMSocketUtilTCP(int sock) :
        HMSocketUtilBase(true, sock),
        m_reason(HM_REASON_NONE),
        m_port(-1),
        m_tos(0){ }

HMSocketUtilTCP::~HMSocketUtilTCP()
{
    closeSocket();
}


bool
HMSocketUtilTCP::sendData(const char* buffer, uint64_t size)
{
    if (!m_connected)
    {
        return false;
    }
    if (send(m_socket, buffer, size, 0) == (int)size)
    {
        return true;
    }
    return false;
}

bool
HMSocketUtilTCP::recvData(char* data, uint64_t size, timeval& tv)
{
    if (!m_connected)
    {
        return false;
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
            return false;
        }
        else if (s_ret == 0)
        {
            m_reason = HM_REASON_RESPONSE_TIMEOUT;
            return false;
        }
        else if (FD_ISSET(m_socket, &fdsr))
        {
            //HMLog(HM_LOG_DEBUG3, "[TCPCHECK] Ready to read");
            if ((tret = read(m_socket, data + ret, size - ret)) < 0)
            {
                m_reason = HM_REASON_INTERNAL_ERROR;
                return false;
                //errorMsg("[TCPCHECK] read failure ");
            }
            else if (tret == 0)
            {
                HMLog(HM_LOG_DEBUG3,
                        "[TCPNBAPI] TCP read - No more data to send ,shutdown on other end");
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

    return ret == size;
}
