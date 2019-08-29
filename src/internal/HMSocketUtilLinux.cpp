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

HMSocketUtilLinux::HMSocketUtilLinux(int sock, std::string& sockPath) :
        HMSocketUtilBase(true, sock),
        m_socketPath(sockPath)
{
}

void
HMSocketUtilLinux::closeSocket()
{
    if(m_socket != -1)
    {
        close(m_socket);
    }
}

HMSocketUtilLinux::~HMSocketUtilLinux()
{
    closeSocket();
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

bool HMSocketUtilLinux::recvData(char* data, uint64_t size, timeval& tv)
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
            HMLog(HM_LOG_ERROR, "select Error, error desc: %s", buf);
            return false;
        }
        if (ret == 0)
        {
            HMLog(HM_LOG_DEBUG, "select timed out, closing Socket");
            return false;
        }
        if (FD_ISSET(m_socket, &fdset))
        {
            int n = recv(m_socket, (void*) data, size, 0);
            if (n == 0)
            {
                return false;
            }
            if (n < 0)
            {
                return false;
            }
            offset += n;
        }
    }
    while (offset < size);
    return true;
}
