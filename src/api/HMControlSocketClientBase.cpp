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
#include "HMControlSocketClientBase.h"
#include "HMControlBase.h"

using namespace std;

map<int8_t, string> loglevels  = {
        { -2, "LOG ERROR" },
        { -1, "none" },
        { 0, "emergency" },
        { 1, "alert" },
        { 2, "critical" },
        { 3, "error" },
        { 4, "warning" },
        { 5, "notice" },
        { 6, "info" },
        { 7, "debug" },
        { 8, "debug2" },
        { 9, "debug3" },
};

void
HMControlSocketClientBase::setError(string msg)
{
    m_errorMsg = msg;
}

const std::string&
HMControlSocketClientBase::getErrorMsg() const
{
    return m_errorMsg;
}

bool
HMControlSocketClientBase::isConnected() const
{
    return m_connected;
}

HMControlSocketClientBase::~HMControlSocketClientBase()
{

}

HMControlSocketClientBase::HMControlSocketClientBase() : m_connected(false)
{

}

void
HMControlSocketClientBase::extract(string &str, vector<string> &list)
{

    std::istringstream stream(str);
    string word;
    while (getline(stream, word, ','))
    {
        list.push_back(word);
    }
}


bool HMControlSocketClientBase::receivePacket(unique_ptr<char[]>& recvbuf, uint64_t& packetSize, bool acceptEmptyResult)
{
    packetSize = 0;
    if (recvMessage((char*) &packetSize, sizeof(packetSize)))
    {
        if (packetSize == 0)
        {
            return acceptEmptyResult;
        }
        packetSize = HMDataPacking::ntoh64(packetSize);
        recvbuf = make_unique<char[]>(packetSize);
        if (recvMessage((char*) recvbuf.get(), packetSize))
        {
            return true;
        }
    }
    return false;
}

template <typename T>
bool HMControlSocketClientBase::receiveUInt(T& x)
{
    uint64_t packetSize = 0;
    if (recvMessage((char*) &packetSize, sizeof(packetSize)))
    {
        if (packetSize == 0)
        {
            x = 0;
            return true;
        }
        packetSize = HMDataPacking::ntoh64(packetSize);
        unique_ptr<char[]> recvbuf = make_unique<char[]>(packetSize);
        if (recvMessage((char*) recvbuf.get(), packetSize))
        {
            return dataPacking->unpackUInt(recvbuf, packetSize, x);
        }
    }
    return false;
}

template <typename T>
bool HMControlSocketClientBase::receiveInt(T& x)
{
    uint64_t packetSize = 0;
    if (recvMessage((char*) &packetSize, sizeof(packetSize)))
    {
        if (packetSize == 0)
        {
            x = 0;
            return true;
        }
        packetSize = HMDataPacking::ntoh64(packetSize);
        unique_ptr<char[]> recvbuf = make_unique<char[]>(packetSize);
        if (recvMessage((char*) recvbuf.get(), packetSize))
        {
            return dataPacking->unpackInt(recvbuf, packetSize, x);
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getThreadInfo(HMAPIThreadInfo& threadInfo)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_THREADINFO;
    if (sendMessage(cmd))
    {
        unique_ptr<char[]> data;
        uint64_t dataSize = 0;
        if (receivePacket(data, dataSize))
        {
            dataPacking->unpackThreadInfo(data, dataSize, threadInfo);
            return true;
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getWorkQueue(uint32_t &workQLen)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_WORKQUEUEINFO;
    if( sendMessage(cmd))
    {
        return receiveUInt(workQLen);
    }
    return false;
}

bool
HMControlSocketClientBase::getHostScheduleInfo(const string& hostGroupName, const string& hostName, HMAPIDNSSchedInfo& dns)
{
    string cmd =  to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_HOSTSCHDINFO + " " + hostGroupName + " " + hostName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize = 0;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            if (dataPacking->unpackHostSchedInfo(recvbuf, packetSize,
                    dns))
            {
                return true;
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getRemoteScheduleInfo(const string& hostGroupName, HMAPIHostSchedInfo& hostGroupSchdInfo)
{
    string cmd =  to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_REMOTESCHDINFO + " " + hostGroupName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize = 0;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            if (dataPacking->unpackRemoteHostGroupSchedInfo(recvbuf, packetSize,
                    hostGroupSchdInfo))
            {
                return true;
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getRemoteScheduleInfo(const string& hostGroupName, const string& hostName, HMAPIHostSchedInfo& hostGroupSchdInfo)
{
    string cmd =  to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_REMOTESCHDINFO + " " + hostGroupName + " " + hostName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize = 0;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            if (dataPacking->unpackRemoteHostGroupSchedInfo(recvbuf, packetSize,
                    hostGroupSchdInfo))
            {
                return true;
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getSchQueue(uint64_t& schQLen)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SCHDQUEUEINFO;
    if (sendMessage(cmd))
    {
        return receiveUInt(schQLen);
    }
    return false;
}

bool
HMControlSocketClientBase::getLogLevel(string& logLevel)
{
    logLevel = "UNKNOWN";
    int8_t log = 0;
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETLOGLEVEL;
    if (sendMessage(cmd))
    {
        if (receiveInt(log))
        {
            map<int8_t, string>::iterator it;
            if ((it = loglevels.find(log)) != loglevels.end())
            {
                logLevel = it->second;
                return true;
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getConnectionTimeout(uint64_t& connectionTimeOut)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETCONNECTIONTIMEOUT;
    if (sendMessage(cmd))
    {
        return receiveUInt(connectionTimeOut);
    }
    return false;
}

bool
HMControlSocketClientBase::getMonitoringFrequency(uint32_t& monitoringFrequency)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETMONFREQ;
    if(sendMessage(cmd))
    {
        return receiveUInt(monitoringFrequency);
    }
    return false;
}

bool
HMControlSocketClientBase::getStride(uint32_t& stride)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETSTRIDE;
    if (sendMessage(cmd))
    {
        return receiveUInt(stride);
    }
    return false;
}

bool
HMControlSocketClientBase::getTTLTreshold(uint32_t& ttlTreshold)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETTTLTRESH;
    if (sendMessage(cmd))
    {
        return receiveUInt(ttlTreshold);
    }
    return false;
}

bool
HMControlSocketClientBase::getWorkPerThread(uint32_t& workPerThread)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETWORKPERTHREAD;
    if (sendMessage(cmd))
    {
        return receiveUInt(workPerThread);
    }
    return false;
}

bool
HMControlSocketClientBase::isRecycleOn()
{
    bool recycle = false;
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETRECYCLE;
    if(sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            recycle = dataPacking->unpackBool(recvbuf, packetSize);
        }
    }
    return recycle;
}

bool
HMControlSocketClientBase::setLogLevel(const string& logLevel)
{
    // TODO: handle case match
    bool validLogLevel = false;
    for (auto it = loglevels.begin(); it != loglevels.end(); ++it)
    {
        if (it->second == logLevel)
        {
            validLogLevel = true;
            break;
        }
    }
    if(validLogLevel)
    {
        string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETLOGLEVEL + " " + logLevel;
        return sendMessage(cmd);
    }
    return false;
}

bool
HMControlSocketClientBase::setConnectionTimeOut(uint64_t connectionTimeOut)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETCONNECTIONTIMEOUT + " " + to_string(connectionTimeOut);
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::setMonitoringFrequency(uint32_t monitoringFrequency)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETMONFREQ + " " + to_string(monitoringFrequency);
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::setStride(uint32_t stride)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETSTRIDE + " " + to_string(stride);
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::setTTLTreshold(uint32_t ttlTreshold)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETTTLTRESH + " " + to_string(ttlTreshold);
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::setWorkPerThread(uint32_t workPerThread)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETWORKPERTHREAD + " " + to_string(workPerThread);
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::setRecycleOn()
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETRECYCLE + " on";
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::setRecycleOff()
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETRECYCLE + " off";
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::setForceStatusDown(const string& hostGroupName, const string& hostName)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETHOSTSTATUS + " " + hostGroupName + " " + hostName + " 1";
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::setForceStatusDown(const string& hostGroupName, HMAPIIPAddress& address)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETHOSTSTATUS + " " + hostGroupName + " " + address.toString() +  " 1";
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::unsetForceStatusDown(const string& hostGroupName, const string& hostName)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETHOSTSTATUS + " " + hostGroupName + " " + hostName + " 0";
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::unsetForceStatusDown(const string& hostGroupName, HMAPIIPAddress& address)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETHOSTSTATUS + " " + hostGroupName + " " + address.toString() + " 0";
    return sendMessage(cmd);
}


bool
HMControlSocketClientBase::reload(const string& masterConfig)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_RELOAD + " " + masterConfig;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            return dataPacking->unpackBool(recvbuf, packetSize);
        }
    }
    return false;
}

bool
HMControlSocketClientBase::reload()
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_RELOAD;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            return dataPacking->unpackBool(recvbuf, packetSize);
        }
    }
    return false;
}

bool
HMControlSocketClientBase::refresh()
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_REFRESH;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            return dataPacking->unpackBool(recvbuf, packetSize);
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getHostGroupList(vector<string>& hostGroupNames)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_HOSTGROUPLIST;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
                unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize, true))
        {
            return dataPacking->unpackList(recvbuf, packetSize,
                    hostGroupNames);
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getHostList(const string& hostGroupName, vector<string>& hostList)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_HOSTLIST + " " + hostGroupName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize, true))
        {
            return dataPacking->unpackList(recvbuf, packetSize,
                    hostList);
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getHostGroupParams(const string& hostGroupName, HMAPICheckInfo& checkInfo)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_HOSTGROUPPARAMS + " " + hostGroupName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            return dataPacking->unpackDataHostGroup(recvbuf, packetSize, checkInfo);
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getHostGroupResults(const string& hostGroupName, HMAPICheckInfo& checkInfo, vector<HMAPICheckResult>& hostResults)
{
    hostResults.clear();
    vector<string> hosts;
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_HOSTGROUP + " " + hostGroupName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            if (dataPacking->unpackHostGroupInfo(recvbuf, packetSize,
                    checkInfo, hostResults))
            {
                return true;
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getHostGroupResults(const string& hostGroupName, const HMAPIHash& hash, HMAPICheckInfo& checkInfo, vector<HMAPICheckResult>& hostResults)
{
    hostResults.clear();
    uint64_t dataSize = 0;
    unique_ptr<char[]> data = dataPacking->packHash(hostGroupName, hash, dataSize);
    vector<string> hosts;
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_HOSTGROUP + " " + hostGroupName + " " + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize))
            {
                if (dataPacking->unpackHostGroupInfo(recvbuf, packetSize,
                        checkInfo, hostResults))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getHostResults(const string& hostGroupName, string& hostName, vector<HMAPICheckResult>& hostResults)
{
    hostResults.clear();
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_HOSTCHECK + " " + hostGroupName + " " + hostName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            dataPacking->unpackDataCheckResults(recvbuf, packetSize,
                    hostResults);
            return true;
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getLoadFeedback(const string& hostGroupName, vector<HMAPIAuxInfo>& auxInfo)
{
    auxInfo.clear();
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_LOADFB + " " + hostGroupName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            if (dataPacking->unpackAuxInfo(recvbuf, packetSize,
                    auxInfo))

            {
                return true;
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getLoadFeedback(const string& hostGroupName, const HMAPIHash& hash, vector<HMAPIAuxInfo>& auxInfo)
{
    auxInfo.clear();
    uint64_t dataSize = 0;
    unique_ptr<char[]> data = dataPacking->packHash(hostGroupName, hash, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_LOADFB + " " + hostGroupName + " " + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize))
            {
                if (dataPacking->unpackAuxInfo(recvbuf, packetSize, auxInfo))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getLoadFeedback(const string& hostName, string& sourceURL, HMAPIIPAddress& address, HMAPIAuxInfo& auxInfo)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_LOADFBIP + " " + hostName + " " + sourceURL + " " + address.toString();
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            if (dataPacking->unpackAuxInfo(recvbuf, packetSize,
                    auxInfo))

            {
                return true;
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getLoadFeedback(const string& hostName, string& sourceURL, HMAPIIPAddress& address, HMAuxInfo& auxInfo)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_LOADFBIP + " " + hostName + " " + sourceURL + " " + address.toString();
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            if (dataPacking->unpackAuxInfo(recvbuf, packetSize,
                    auxInfo))

            {
                return true;
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getLoadFeedback(const string& hostName, const HMAPIDataHostCheck& apiDataHostCheck, vector<HMAPIAuxInfo>& auxResults)
{
    auxResults.clear();
    HMDataHostCheck dataHostCheck(apiDataHostCheck);
    uint64_t dataSize = 0;
    unique_ptr<char[]> data = dataPacking->packDataHostCheck(dataHostCheck, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_LOADFBHOST + " " + hostName + " " + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize))
            {
                if (dataPacking->unpackAuxInfo(recvbuf, packetSize, auxResults))

                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getHostResults(const string& hostName, HMAPIDataHostCheck& apiDataHostCheck, vector<pair<HMAPICheckInfo, HMAPICheckResult>>& hostResults)
{
    hostResults.clear();
    HMDataHostCheck dataHostCheck(apiDataHostCheck);
    uint64_t dataSize = 0;
    unique_ptr<char[]> data = dataPacking->packDataHostCheck(dataHostCheck, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_HOSTRESULTS + " " + hostName + " "
            + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize))
            {
                return dataPacking->unpackHostResults(recvbuf,
                        packetSize, hostResults);
            }
        }
    }
	return false;
}


bool
HMControlSocketClientBase::getHostResults(const string& hostName, HMAPIIPAddress& address,HMAPIDataHostCheck& apiDataHostCheck, vector<pair<HMAPICheckInfo, HMAPICheckResult>>& hostResults)
{
    hostResults.clear();
    HMDataHostCheck dataHostCheck(apiDataHostCheck);
    uint64_t dataSize = 0;
    unique_ptr<char[]> data = dataPacking->packDataHostCheck(dataHostCheck, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_HOSTIPRESULTS + " " + hostName + " " + address.toString() + " "
            + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize))
            {
                return dataPacking->unpackHostResults(recvbuf,
                        packetSize, hostResults);
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getRemoteQueryOn(bool& remoteQueryStatus)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETREMOTEQUERY;
    if(sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize, true))
        {
            remoteQueryStatus = dataPacking->unpackBool(recvbuf, packetSize);
            return true;
        }
    }
    return false;
}


bool
HMControlSocketClientBase::setRemoteQueryOn()
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETREMOTEQUERY + " on";
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::setRemoteQueryOff()
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_SETREMOTEQUERY + " off";
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::getHandlerThreadCount(uint64_t& count)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_GETHANDLERTHREADSCOUNT;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize, false))
        {
            return dataPacking->unpackUInt(recvbuf, packetSize, count);
        }
    }
    return false;
}
bool
HMControlSocketClientBase::addHostGroup(const string& hostGroupName, HMAPICheckInfo& checkInfo)
{
    uint64_t dataSize = 0;
    HMDataHostGroup hostGroup(hostGroupName, checkInfo);
    unique_ptr<char[]> data = dataPacking->packDataHostGroup(hostGroup, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_ADDHOSTGROUP + " " + hostGroupName + " "
            + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize))
            {
                return dataPacking->unpackBool(recvbuf, packetSize);
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::removeHostGroup(const string& hostGroupName)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_REMOVEHOSTGROUP + " " + hostGroupName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            return dataPacking->unpackBool(recvbuf, packetSize);
        }
    }
    return false;
}

bool
HMControlSocketClientBase::clearTransaction()
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_CLEARTRANSACTION;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            return dataPacking->unpackBool(recvbuf, packetSize);
        }
    }
    return false;
}


bool
HMControlSocketClientBase::getTransactionalHashInfo(map<string, HMAPIHash>& hashinfo)
{
    hashinfo.clear();
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETHOSTGROUPHASH;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            return dataPacking->unpackHashInfo(recvbuf, packetSize,
                    hashinfo);
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getTransactionConfigHash(HMAPIHash& hash)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETTRANSCONFIGHASH;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize))
        {
            return dataPacking->unpackHash(recvbuf, packetSize,
                    hash);
        }
    }
    return false;
}

HM_API_COMMIT_TRANSACTION_STATUS
HMControlSocketClientBase::commitTransaction(const HMAPIHash& hash)
{
    uint64_t dataSize = 0;
    string configHash = "config";
    unique_ptr<char[]> data = dataPacking->packHash(configHash, hash, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_COMMITTRANSACTION + " " + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize))
            {
                uint8_t status;
                if(dataPacking->unpackUInt(recvbuf, packetSize, status))
                {
                    return (HM_API_COMMIT_TRANSACTION_STATUS)status;
                }
                setError("Failure unpacking status");
                return HM_API_COMMIT_TRANSACTION_FAILURE;
            }
        }
    }
    return HM_API_COMMIT_TRANSACTION_FAILURE;;
}

bool
HMControlSocketClientBase::addDNSAddresses(const string& hostName, vector<HMAPIIPAddress>& addresses)
{
    uint64_t dataSize = 0;
    unique_ptr<char[]> data = dataPacking->packIPAddresses(addresses, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_ADDDNSADDRESSES + " " + hostName + " "
            + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize, true))
            {
                return dataPacking->unpackBool(recvbuf, packetSize);
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::removeDNSAddresses(const string& hostName, vector<HMAPIIPAddress>& addresses)
{
    uint64_t dataSize = 0;
    unique_ptr<char[]> data = dataPacking->packIPAddresses(addresses, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_REMOVEDNSADDRESSES + " " + hostName + " "
            + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize, true))
            {
                return dataPacking->unpackBool(recvbuf, packetSize);
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getDNSAddresses(const string& hostName, vector<HMAPIIPAddress>& addresses)
{
    addresses.clear();
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " " + HM_CMD_GETDNSADDRESSES + " " + hostName;
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize, true))
        {
            return dataPacking->unpackIPAddresses(recvbuf, packetSize, addresses);
        }
    }
    return false;
}


bool
HMControlSocketClientBase::addHostMark(const string& hostGroupName, const string& hostName, const HMAPIIPAddress& address, const set<int>& values)
{
    uint64_t dataSize = 0;
    unique_ptr<char[]> data = dataPacking->packListInt64(values, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_SETHOSTMARK + " " + hostGroupName + " " + hostName + " " + address.toString() + " " + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize, true))
            {
                return dataPacking->unpackBool(recvbuf, packetSize);
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::removeHostMark(const string& hostGroupName, const string& hostName, const HMAPIIPAddress& address, const set<int>& values)
{
    uint64_t dataSize = 0;
    unique_ptr<char[]> data = dataPacking->packListInt64(values, dataSize);
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_REMOVEHOSTMARK + " " + hostGroupName + " " + hostName+ " " + address.toString() + " " + to_string(dataSize);
    if (sendMessage(cmd))
    {
        if (sendData(data.get(), dataSize))
        {
            uint64_t packetSize;
            unique_ptr<char[]> recvbuf;
            if (receivePacket(recvbuf, packetSize, true))
            {
                return dataPacking->unpackBool(recvbuf, packetSize);
            }
        }
    }
    return false;
}

bool
HMControlSocketClientBase::getHostMark(const string& hostGroupName, const string& hostName, const HMAPIIPAddress& address, set<int>& values)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_GETHOSTMARK + " " + hostGroupName + " " + hostName + " " + address.toString();
    if (sendMessage(cmd))
    {
        uint64_t packetSize;
        unique_ptr<char[]> recvbuf;
        if (receivePacket(recvbuf, packetSize, false))
        {
            return dataPacking->unpackListInt64(recvbuf, packetSize, values);
        }
    }
    return false;
}



bool
HMControlSocketClientBase::forceDNSCheck(const string& hostGroupName, const string& hostName)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_DNSCHECK + " " + hostGroupName + " " + hostName;
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::forceDNSCheck(const string& hostGroupName)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_DNSCHECK + " " + hostGroupName;
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::forceHealthCheck(const string& hostGroupName, const string& hostName)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_HEALTHCHECK + " " + hostGroupName + " " + hostName;
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::forceHealthCheck(const string& hostGroupName)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_HEALTHCHECK + " " + hostGroupName;
    return sendMessage(cmd);
}

bool
HMControlSocketClientBase::forceRemoteHostGroupCheck(const string& hostGroupName)
{
    string cmd = to_string(HM_CONTROL_SOCKET_VERSION) + " "
            + HM_CMD_REMOTEHOSTGROUPCHECK + " " + hostGroupName;
    return sendMessage(cmd);
}
