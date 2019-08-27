// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <future>

#include "HMControlTCPSocketClient.h"
#include "HMControlBase.h"

using namespace std;

bool
HMControlTCPSocketClient::createSocket()
{
    if ((m_socket = socket(m_address.m_type, SOCK_STREAM, 0)) == -1)
    {
        setError("Failed to create socket, error: " + std::string(strerror(errno)));
        m_socket = -1;
        return false;
    }
    return true;
}

void
HMControlTCPSocketClient::closeSocket()
{
    if (m_socket != -1)
    {
        close(m_socket);
    }
}

bool
HMControlTCPSocketClient::connectSocket()
{
    if (!createSocket())
    {
        return false;
    }
    sockaddr_storage server;
    socklen_t len;
    if(m_address.m_type == AF_INET6)
    {
        sockaddr_in6 *addr = (sockaddr_in6 *)&server;
        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(m_port);
        addr->sin6_addr = m_address.m_ip.addr6;
        len = sizeof(sockaddr_in6);
    }
    else
    {
        sockaddr_in *addr = (sockaddr_in *)&server;
        addr->sin_family = AF_INET;
        addr->sin_port = htons(m_port);
        addr->sin_addr.s_addr = m_address.m_ip.addr;
        len = sizeof(sockaddr_in);
    }
    if (connect(m_socket, (struct sockaddr *) &server, len) == -1)
    {
        setError("Failed to connect socket, error: " + std::string(strerror(errno)));
        closeSocket();
        m_socket = -1;
        return false;
    }
    return true;
}

HMControlTCPSocketClient::HMControlTCPSocketClient(HMAPIIPAddress& address, uint16_t port) :
        m_socket(-1),
        m_address(address),
        m_port(port)
{
    if (connectSocket())
    {
        m_connected = true;
    }
}

HMControlTCPSocketClient::~HMControlTCPSocketClient()
{
    closeSocket();
}

bool
HMControlTCPSocketClient::sendMessage(const string& cmd)
{
    if(!m_connected)
    {
        setError("Socket not connected");
        return false;
    }
    uint64_t cmdLen = cmd.length();
    cmdLen = HMDataPacking::hton64(cmdLen);
    if (send(m_socket, &cmdLen, sizeof(cmdLen), 0) == sizeof(cmdLen))
    {
        if (send(m_socket, cmd.c_str(), cmd.length(), 0) == (int32_t)cmd.length())
        {
            return true;
        }
    }
    if ( errno == EPIPE)
    {
        if(!connectSocket())
        {
            return false;
        }
        if (send(m_socket, &cmdLen, sizeof(cmdLen), 0) == sizeof(cmdLen))
        {
            if (send(m_socket, cmd.c_str(), cmd.length(), 0) == (int32_t)cmd.length())
            {
                return true;
            }
        }
    }
    setError("failed to send msg " + cmd + " on socket, error desc: " + std::string(strerror(errno)));
    closeSocket();
    return false;
}

bool
HMControlTCPSocketClient::sendData(const char* buffer, uint32_t size)
{
    if (!m_connected)
    {
        setError("Socket not connected");
        return false;
    }
    if (send(m_socket, buffer, size, 0) == -1)
    {
        if ( errno == EPIPE )
        {
            connectSocket();
        }
        setError("failed to send msg on socket, error desc: " + std::string(strerror(errno)));
        closeSocket();
        return false;
    }
    return true;
}

bool
HMControlTCPSocketClient::recvMessage(char* data, uint64_t size)
{
    if (!m_connected)
    {
        setError("Socket not connected");
        return false;
    }
    int n = read(m_socket, data, size);
    return n == (int)size;
}
