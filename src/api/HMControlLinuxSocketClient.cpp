// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <future>

#include "HMDataPacking.h"
#include "HMControlLinuxSocketClient.h"
#include "HMControlBase.h"

using namespace std;

bool
HMControlLinuxSocketClient::createSocket()
{
    if ((m_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1)
    {
        setError("Failed to create socket, error: " + std::string(strerror(errno)));
        m_socket = -1;
        return false;
    }
    return true;
}

void
HMControlLinuxSocketClient::closeSocket()
{
    if (m_socket != -1)
    {
        close(m_socket);
    }
}

bool
HMControlLinuxSocketClient::connectSocket()
{
    if(!createSocket())
    {
        return false;
    }
    int len;
    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    sprintf(remote.sun_path, "%s", m_server_path.c_str());
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(m_socket, (struct sockaddr *) &remote, len) == -1)
    {
        setError("Failed to connect socket to " + m_server_path + ", error: " + std::string(strerror(errno)));
        closeSocket();
        m_socket = -1;
        return false;
    }
    return true;
}

HMControlLinuxSocketClient::HMControlLinuxSocketClient(const string& socketPath) :
        m_server_path(socketPath)
{
    dataPacking = make_unique<HMDataPacking>();
    if (connectSocket())
    {
        m_connected = true;
    }

}

HMControlLinuxSocketClient::~HMControlLinuxSocketClient()
{
    closeSocket();
}

bool
HMControlLinuxSocketClient::sendMessage(const string& cmd)
{
    if(!m_connected)
    {
        setError("Socket not connected");
        return false;
    }
    uint64_t cmdLen = HMDataPacking::hton64(cmd.length());
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
HMControlLinuxSocketClient::sendData(const char* buffer, uint32_t size)
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
HMControlLinuxSocketClient::recvMessage(char* data, uint64_t size)
{
    if (!m_connected)
    {
        setError("Socket not connected");
        return false;
    }
    uint64_t offSize = 0;
    while (offSize < size)
    {
        int n = read(m_socket, data + offSize, size);
        if (n < 0)
        {
            setError("Failed to recv on socket, error desc: " + std::string(strerror(errno)));
            return false;
        }
        offSize += n;
    }
    return offSize == size;
}
