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
#include "HMControlBase.h"

using namespace std;

HM_SOCK_DATA_STATUS HMSocketUtilBase::receiveCommand(std::string& command, timeval& tv)
{
    if(!m_connected)
    {
        return HM_SOCK_DATA_FAILED;
    }
    uint64_t stringSize = 0;
    HM_SOCK_DATA_STATUS status = recvData((char*) &stringSize, sizeof(stringSize), tv);
    if (status != HM_SOCK_DATA_OK)
    {
        return status;
    }
    if(stringSize == 0) return HM_SOCK_DATA_EMPTY;
    stringSize = HMDataPacking::ntoh64(stringSize);
    unique_ptr<char[]> data = make_unique<char[]>(stringSize + 1);
    status = recvData(data.get(), stringSize, tv);
    if (status != HM_SOCK_DATA_OK)
    {
        return status;
    }
    data[stringSize] = '\0';
    command = data.get();
    return HM_SOCK_DATA_OK;
}

bool
HMSocketUtilBase::sendCommand(const string& cmd)
{
   return sendMessage(cmd.c_str(), cmd.length());
}

HM_SOCK_DATA_STATUS
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

bool
HMSocketUtilBase::pingRemoteHost(timeval& tv)
{
    lock_guard<mutex> clock(m_mutex);
    HMDataPacking dataPacking;
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_GETREMOTEQUERY;
    if (sendCommand(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (recvData((char*) &packetSize, sizeof(packetSize), tv) == HM_SOCK_DATA_OK)
        {
            // when bool is false a empty packet is returned.
            if (packetSize == 0)
            {
                m_reason = HM_REASON_RESPONSE_DOWN;
                return false;
            }
            packetSize = dataPacking.ntoh64(packetSize);
            std::unique_ptr<char[]> recvdData = make_unique<char[]>(packetSize);
            if (recvData(recvdData.get(), packetSize, tv) == HM_SOCK_DATA_OK)
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

bool
HMSocketUtilBase::openPersistant()
{
    HMDataPacking dataPacking;
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_OPENPERSISTANT;
    if (sendCommand(cmd))
    {
        return true;
    }
    return false;
}


HM_SOCK_DATA_STATUS
HMSocketUtilBase::getLoadFeedback(const string& hostName, HMIPAddress& address, HMDataHostCheck& dataHostCheck, timeval &tv, HMAuxInfo& auxData)
{
    lock_guard<mutex> clock(m_mutex);
    HMDataPacking dataPacking;
    if (isConnectionReset() || (getReason() != HM_REASON_NONE && getReason() != HM_REASON_SUCCESS))
    {
        reconnect();
    }
    string cmd = std::to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_LOADFBIP + " " + hostName + " " + dataHostCheck.getCheckInfo() + " " + address.toString();
    if (sendCommand(cmd))
    {
        uint64_t packetSize = 0;
        if (recvData((char*) &packetSize, sizeof(packetSize), tv) == HM_SOCK_DATA_OK)
        {
            packetSize = dataPacking.ntoh64(packetSize);
            if (packetSize == 0)
            {
                HMLog(HM_LOG_DEBUG,
                        "[SocketUtil] Remote aux result fetch received packet size 0 for %s(%s)",
                        hostName.c_str(), address.toString().c_str());
                return HM_SOCK_DATA_EMPTY;
            }
            std::unique_ptr<char[]> recvdData = make_unique<char[]>(packetSize);
            if (recvData(recvdData.get(), packetSize, tv)  == HM_SOCK_DATA_OK)
            {
                if (dataPacking.unpackAuxInfo(recvdData, packetSize,
                        auxData))
                {
                    return HM_SOCK_DATA_OK;
                }
            }
        }
    }
    setConnectionReset(true);
    closeSocket();
    HMLog(HM_LOG_DEBUG, "[SocketUtil] Failed to receive aux results from remote host for %s(%s)", hostName.c_str(), address.toString().c_str());
    return HM_SOCK_DATA_FAILED;
}


HM_SOCK_DATA_STATUS
HMSocketUtilBase::getLoadFeedback(const string& hostName,  HMDataHostCheck& dataHostCheck, timeval &tv, vector<HMGroupAuxResult>& auxResults)
{
    lock_guard<mutex> clock(m_mutex);
    HMDataPacking dataPacking;
    uint64_t dataSize;
    if (isConnectionReset() || (getReason() != HM_REASON_NONE && getReason() != HM_REASON_SUCCESS))
    {
        reconnect();
    }
    std::unique_ptr<char[]> data = dataPacking.packDataHostCheck(dataHostCheck, dataSize);
    string cmd = std::to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_LOADFBHOST + " " + hostName + " " + to_string(dataSize);
    if (sendCommand(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize = 0;
            if (recvData((char*) &packetSize, sizeof(packetSize), tv)
                    == HM_SOCK_DATA_OK)
            {
                packetSize = dataPacking.ntoh64(packetSize);
                if (packetSize == 0)
                {
                    HMLog(HM_LOG_DEBUG,
                            "[SocketUtil] Remote aux result fetch received packet size 0 for %s",
                            hostName.c_str());
                    return HM_SOCK_DATA_EMPTY;
                }
                HMLog(HM_LOG_DEBUG,
                        "[SocketUtil] Remote result packet size %llu for %s",
                        packetSize, hostName.c_str());
                std::unique_ptr<char[]> recvdData = make_unique<char[]>(
                        packetSize);
                if (recvData(recvdData.get(), packetSize, tv)
                        == HM_SOCK_DATA_OK)
                {
                    if (dataPacking.unpackAuxInfo(recvdData, packetSize,
                            auxResults))
                    {
                        return HM_SOCK_DATA_OK;
                    }
                }
            }
        }
    }
    setConnectionReset(true);
    closeSocket();
    HMLog(HM_LOG_DEBUG, "[SocketUtil] Failed to receive aux results from remote host for %s", hostName.c_str());
    return HM_SOCK_DATA_FAILED;
}


HM_SOCK_DATA_STATUS
HMSocketUtilBase::getHostResults(string& hostName, HMIPAddress& address, HMDataHostCheck& dataHostCheck, timeval &tv, map<HMDataCheckParams, HMDataCheckResult>& hostResults)
{
    lock_guard<mutex> clock(m_mutex);
    hostResults.clear();
    if (isConnectionReset() || (getReason() != HM_REASON_NONE && getReason() != HM_REASON_SUCCESS))
    {
        reconnect();
    }
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
            if (recvData((char*) &packetSize, sizeof(packetSize), tv) == HM_SOCK_DATA_OK)
            {
                packetSize = dataPacking.ntoh64(packetSize);
                if (packetSize == 0)
                {
                    HMLog(HM_LOG_DEBUG,
                            "[SocketUtil] Remote result fetch received packet size 0 for %s(%s)",
                            hostName.c_str(), address.toString().c_str());
                    return HM_SOCK_DATA_EMPTY;
                }
                std::unique_ptr<char[]> recvdData = make_unique<char[]>(packetSize);
                if(recvData(recvdData.get(), packetSize, tv) == HM_SOCK_DATA_OK)
                {
                    if(dataPacking.unpackHostResults(recvdData, packetSize, hostResults))
                    {
                        return HM_SOCK_DATA_OK;
                    }
                }
            }
        }
    }
    setConnectionReset(true);
    closeSocket();
    HMLog(HM_LOG_DEBUG, "[SocketUtil] Failed to receive results from remote host for %s(%s)", hostName.c_str(), address.toString().c_str());
    return HM_SOCK_DATA_FAILED;
}

HM_SOCK_DATA_STATUS
HMSocketUtilBase::getHostResults(string& hostName, HMDataHostCheck& dataHostCheck, timeval &tv, multimap<HMDataCheckParams, HMDataCheckResult>& hostResults)
{
    lock_guard<mutex> clock(m_mutex);
    hostResults.clear();
    if (isConnectionReset() || (getReason() != HM_REASON_NONE && getReason() != HM_REASON_SUCCESS))
    {
        reconnect();
    }
    HMDataPacking dataPacking;
    uint64_t dataSize;
    std::unique_ptr<char[]> data = dataPacking.packDataHostCheck(dataHostCheck, dataSize);
    string cmd = std::to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_HOSTRESULTS + " " + hostName + " " + to_string(dataSize);
    if (sendCommand(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize = 0;
            if (recvData((char*) &packetSize, sizeof(packetSize), tv) == HM_SOCK_DATA_OK)
            {
                packetSize = dataPacking.ntoh64(packetSize);
                if (packetSize == 0)
                {
                    HMLog(HM_LOG_DEBUG,
                            "[SocketUtil] Remote result fetch received packet size 0 for %s",
                            hostName.c_str());
                    return HM_SOCK_DATA_EMPTY;
                }
                std::unique_ptr<char[]> recvdData = make_unique<char[]>(packetSize);
                HMLog(HM_LOG_DEBUG,
                        "[SocketUtil] Remote result packet size %llu for %s",
                        packetSize, hostName.c_str());
                if(recvData(recvdData.get(), packetSize, tv) == HM_SOCK_DATA_OK)
                {
                    if(dataPacking.unpackHostResults(recvdData, packetSize, hostResults))
                    {
                        return HM_SOCK_DATA_OK;
                    }
                }
            }
        }
    }
    setConnectionReset(true);
    closeSocket();
    HMLog(HM_LOG_DEBUG, "[SocketUtil] Failed to receive results from remote host for %s", hostName.c_str());
    return HM_SOCK_DATA_FAILED;
}


HM_SOCK_DATA_STATUS
HMSocketUtilBase::getLoadFeedback(string& hostGroupName, timeval &tv, const HMHash& hash, vector<HMGroupAuxResult>& auxResults)
{
    lock_guard<mutex> clock(m_mutex);
    HMDataPacking dataPacking;
    auxResults.clear();
    if (isConnectionReset() || (getReason() != HM_REASON_NONE && getReason() != HM_REASON_SUCCESS))
    {
        reconnect();
    }
    uint64_t dataSize;
    std::unique_ptr<char[]> data = dataPacking.packHash(hostGroupName, hash, dataSize);
    string cmd = std::to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_LOADFB + " " + hostGroupName + " " + to_string(dataSize);
    if (sendCommand(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize = 0;
            if (recvData((char*) &packetSize, sizeof(packetSize), tv)
                    == HM_SOCK_DATA_OK)
            {
                packetSize = dataPacking.ntoh64(packetSize);
                if (packetSize == 0)
                {
                    HMLog(HM_LOG_DEBUG,
                            "[SocketUtil] Remote aux result fetch received packet size 0 for hostgroup %s",
                            hostGroupName.c_str());
                    return HM_SOCK_DATA_EMPTY;
                }
                std::unique_ptr<char[]> recvdData = make_unique<char[]>(
                        packetSize);
                if (recvData(recvdData.get(), packetSize, tv)
                        == HM_SOCK_DATA_OK)
                {
                    if (dataPacking.unpackAuxInfo(recvdData, packetSize,
                            auxResults))
                    {
                        return HM_SOCK_DATA_OK;
                    }
                }
            }
        }
    }
    setConnectionReset(true);
    closeSocket();
    HMLog(HM_LOG_DEBUG, "[SocketUtil] Failed to receive aux results from remote hostgroup for %s", hostGroupName.c_str());
    return HM_SOCK_DATA_FAILED;
}


HM_SOCK_DATA_STATUS
HMSocketUtilBase::getHostGroupResults(string& hostGroupName, timeval &tv, const HMHash& hash, vector<HMGroupCheckResult>& results)
{
    lock_guard<mutex> clock(m_mutex);
    results.clear();
    if (isConnectionReset() || (getReason() != HM_REASON_NONE && getReason() != HM_REASON_SUCCESS))
    {
        reconnect();
    }
    HMDataPacking dataPacking;
    uint64_t dataSize;
    std::unique_ptr<char[]> data = dataPacking.packHash(hostGroupName, hash, dataSize);
    string cmd = std::to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_HOSTGROUP + " " + hostGroupName + " " + to_string(dataSize);
    if (sendCommand(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize = 0;
            if (recvData((char*) &packetSize, sizeof(packetSize), tv) == HM_SOCK_DATA_OK)
            {
                packetSize = dataPacking.ntoh64(packetSize);
                if (packetSize == 0)
                {
                    HMLog(HM_LOG_DEBUG,
                            "[SocketUtil] Remote result fetch received packet size 0 for %s",
                            hostGroupName.c_str());
                    return HM_SOCK_DATA_EMPTY;
                }
                std::unique_ptr<char[]> recvdData = make_unique<char[]>(packetSize);
                if(recvData(recvdData.get(), packetSize, tv) == HM_SOCK_DATA_OK)
                {
                    HMDataHostGroup hostgroup(hostGroupName);
                    if(dataPacking.unpackHostGroupInfo(recvdData, packetSize, hostgroup, results))
                    {
                        return HM_SOCK_DATA_OK;
                    }
                }
            }
        }
    }
    setConnectionReset(true);
    closeSocket();
    HMLog(HM_LOG_DEBUG, "[SocketUtil] Failed to receive results from remote hostgroup for %s", hostGroupName.c_str());
    return HM_SOCK_DATA_FAILED;
}


HM_REASON
HMSocketUtilBase::getReason() const
{
    return m_reason;
}

const std::string&
HMSocketUtilBase::getErrorMsg() const
{
    return m_errorMsg;
}

const HMTimeStamp&
HMSocketUtilBase::getConnectTime() const
{
    return m_connectTime;
}

bool HMSocketUtilBase::isConnectionReset() const
{
    return m_connectionReset;
}

void HMSocketUtilBase::setConnectionReset(bool connectionReset)
{
    m_connectionReset = connectionReset;
}

bool HMSocketUtilBase::isPersistent() const
{
    return m_persistent;
}

void HMSocketUtilBase::connectServer()
{
    lock_guard<mutex> clock(m_mutex);
    reconnect();
}

bool HMSocketUtilBase::strtoull(const std::string& sNumber, uint64_t& number)
{
    try
    {
        number = std::stoull(sNumber, NULL, 0);
        return true;
    }
    catch (const invalid_argument& ia)
    {
        HMLog(HM_LOG_ERROR,
                "Failed to convert size value for command :%s, error: %s",
                HM_CMD_SETHOSTMARK.c_str(), ia.what());
        sendMessage(nullptr, 0);
        return false;
    }
}

bool HMSocketUtilBase::strtoul(const std::string& sNumber, uint32_t& number)
{
    try
    {
        number = std::stoul(sNumber, NULL, 0);
        return true;
    }
    catch (const invalid_argument& ia)
    {
        HMLog(HM_LOG_ERROR,
                "Failed to convert size value for command :%s, error: %s",
                HM_CMD_SETHOSTMARK.c_str(), ia.what());
        sendMessage(nullptr, 0);
        return false;
    }
}
