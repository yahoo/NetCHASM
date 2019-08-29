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

#include "HMSocketUtilBase.h"
#include "HMDataPacking.h"
using namespace std;

bool HMSocketUtilBase::receiveCommand(std::string& command, timeval& tv)
{
    if(!m_connected)
    {
        return false;
    }
    uint64_t stringSize;
    if (!recvData((char*) &stringSize, sizeof(stringSize), tv))
    {
        return false;
    }
    stringSize = HMDataPacking::ntoh64(stringSize);
    unique_ptr<char[]> data = make_unique<char[]>(stringSize + 1);
    if (!recvData(data.get(), stringSize, tv))
    {
        return false;
    }
    data[stringSize] = '\0';
    command = data.get();
    return true;
}

bool
HMSocketUtilBase::sendCommand(const string& cmd)
{
   return sendMessage(&cmd.at(0), cmd.length());
}

/*bool
HMSocketUtilTCP::sendBlob(const char* buffer, uint64_t size)
{
    if (!m_connected)
    {
        return false;
    }
    uint64_t sendSize = HMDataPacking::hton64(size);
    int n = sendData((char*)&sendSize, sizeof(sendSize));
    if (n < 0)
    {
        HMLog(HM_LOG_DEBUG, "Failed to send message TCP socket, error code:%d",
                errno);
        return false;
    }
    if (size > 0)
    {
        int n = sendData(buffer, size);
        if (n < 0)
        {
            HMLog(HM_LOG_DEBUG,
                    "Failed to send message TCP socket, error code:%d", errno);
            return false;
        }
    }
    return true;
}
*/
bool
HMSocketUtilBase::receiveMessage(char *data, uint64_t size)
{
    timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    return recvData(data, size ,tv);
}
bool
HMSocketUtilBase::sendMessage(const char* data, uint64_t size)
{
    uint64_t sizeSend = HMDataPacking::hton64(size);
    if(sendData((const char*)&sizeSend, sizeof(sizeSend)))
    {
        if(size == 0 || sendData(data, size))
        {
            return true;
        }
    }
    return false;
}


