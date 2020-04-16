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

#include "HMSocketUtilLinux.h"

using namespace std;

bool
HMSocketUtilLinux::createSocket()
{
    if ((m_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1)
    {
        m_errorMsg = "Failed to create socket, error: " + std::string(strerror(errno));
        m_socket = -1;
        m_reason = HM_REASON_INTERNAL_ERROR;
        return false;
    }
    return true;
}

bool
HMSocketUtilLinux::connectSocket()
{
    if(!createSocket())
    {
        return false;
    }
    int len;
    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    sprintf(remote.sun_path, "%s", m_socketPath.c_str());
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(m_socket, (struct sockaddr *) &remote, len) == -1)
    {
        m_errorMsg = "Failed to connect socket to " + m_socketPath + ", error: " + std::string(strerror(errno));
        closeSocket();
        m_socket = -1;
        m_reason = HM_REASON_CONNECT_FAILURE;
        return false;
    }
    m_connected = true;
    return true;
}

void
HMSocketUtilLinux::closeSocket()
{
    if(m_socket != -1)
    {
        close(m_socket);
        m_connected = false;
        m_socket = -1;
    }
}

HMSocketUtilLinux::~HMSocketUtilLinux()
{
    closeSocket();
}

void
HMSocketUtilLinux::reconnect()
{
    if (connectSocket())
    {
        setConnectionReset(false);
        if (isPersistent())
        {
            openPersistant();
        }
    }
}

bool
HMSocketUtilLinux::sendData(const char* buffer, uint64_t size)
{
    const uint32_t chunk = 65535;
    size_t offset = 0;
    size_t chunkSize = 0;
    for (; size > 0; size -= chunkSize)
    {
        chunkSize = (size < chunk) ? size : chunk;
        const char *data = buffer + offset;
        int n = send(m_socket, data, chunkSize, 0);
        if (n < 0)
        {
            HMLog(HM_LOG_DEBUG, "Failed to send message %s, error code:%d",
                    m_socketPath.c_str(), errno);
            return false;
        }

        offset += chunkSize;
    }
    return true;
}

HM_SOCK_DATA_STATUS HMSocketUtilLinux::recvData(char* data, uint64_t size, timeval tv)
{
    size_t offset = 0;
    fd_set fdset;
    do
    {
        FD_ZERO(&fdset);
        FD_SET(m_socket, &fdset);
        int ret = select(m_socket  + 1, &fdset, NULL, NULL, &tv);
        if (ret < 0)
        {
            char buf[1024];
            strerror_r(errno, buf, sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';
            HMLog(HM_LOG_ERROR, "select Error, error %d desc: %s",errno, buf);
            return HM_SOCK_DATA_FAILED;
        }
        if (ret == 0)
        {
            HMLog(HM_LOG_DEBUG, "select timed out, closing Socket");
            return HM_SOCK_DATA_TIMEOUT;
        }
        if (FD_ISSET(m_socket, &fdset))
        {
            int n = recv(m_socket, (void*) data, size, 0);
            if (n == 0)
            {
                return HM_SOCK_DATA_EMPTY;
            }
            if (n < 0)
            {
                return HM_SOCK_DATA_FAILED;
            }
            offset += n;
        }
    }
    while (offset < size);
    return HM_SOCK_DATA_OK;
}
