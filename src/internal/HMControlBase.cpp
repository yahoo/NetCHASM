// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <vector>

#include "HMControlBase.h"
#include "HMStateManager.h"
#include "HMLogBase.h"
#include "HMDataPacking.h"

using namespace std;

map<string, HM_COMMAND_TASKS> commands =
{
    { HM_CMD_RELOAD, RELOAD },
    { HM_CMD_HOSTGROUP, HOSTGROUPINFO },
    { HM_CMD_LOADFB, LOADFBINFO },
    { HM_CMD_LOADFBIP, LOADFBINFOIP },
    { HM_CMD_THREADINFO, THREADINFO },
    { HM_CMD_WORKQUEUEINFO, WORKQUEUEINFO },
    { HM_CMD_SCHDQUEUEINFO, SCHDQUEUEINFO },
    { HM_CMD_SETHOSTSTATUS, SETHOSTSTATUS },
    { HM_CMD_HOSTGROUPLIST, HOSTGROUPLIST },
    { HM_CMD_HOSTLIST, HOSTLIST },
    { HM_CMD_HOSTCHECK, HOSTCHECK },
    { HM_CMD_HOSTRESULTS, HOSTRESULTS },
    { HM_CMD_HOSTIPRESULTS, HOSTIPRESULTS },
    { HM_CMD_HOSTGROUPPARAMS, HOSTGROUPPARAMS },
    { HM_CMD_HOSTSCHDINFO, HOSTSCHDINFO },
    { HM_CMD_HEALTHCHECK, HEALTHCHECK },
    { HM_CMD_DNSCHECK, DNSCHECK },
    { HM_CMD_SETLOGLEVEL, SETLOGLEVEL },
    { HM_CMD_GETLOGLEVEL, GETLOGLEVEL },
    { HM_CMD_SETCONNECTIONTIMEOUT, SETCONNECTIONTIMEOUT },
    { HM_CMD_GETCONNECTIONTIMEOUT, GETCONNECTIONTIMEOUT },
    { HM_CMD_SETMONFREQ, SETMONFREQ },
    { HM_CMD_GETMONFREQ, GETMONFREQ },
    { HM_CMD_SETTTLTRESH, SETTTLTRESH },
    { HM_CMD_GETTTLTRESH, GETTTLTRESH },
    { HM_CMD_SETSTRIDE, SETSTRIDE },
    { HM_CMD_GETSTRIDE, GETSTRIDE },
    { HM_CMD_SETWORKPERTHREAD, SETWORKPERTHREAD },
    { HM_CMD_GETWORKPERTHREAD, GETWORKPERTHREAD },
    { HM_CMD_GETRECYCLE, GETRECYCLE },
    { HM_CMD_SETRECYCLE, SETRECYCLE },
    { HM_CMD_SETREMOTEQUERY, SETREMOTEQUERY },
    { HM_CMD_GETREMOTEQUERY, GETREMOTEQUERY },
    { HM_CMD_ADDDNSADDRESSES, ADDDNSADDRESSES },
    { HM_CMD_REMOVEDNSADDRESSES, REMOVEDNSADDRESSES },
    { HM_CMD_GETDNSADDRESSES, GETDNSADDRESSES },
    { HM_CMD_SETHOSTMARK, SETHOSTMARK },
    { HM_CMD_REMOVEHOSTMARK, REMOVEHOSTMARK },
    { HM_CMD_GETHOSTMARK, GETHOSTMARK }
};

HMCommandListenerBase::HMCommandListenerBase(HMStateManager &stateManager) :
        m_stateManager(stateManager),
        m_keepRunning(false) {}

void
HMCommandListenerBase::init()
{
    if (pipe(m_pipesfd) < 0) {
        throwException("Failed to create pipe, error desc: ");
    }
}

HM_COMMAND_TASKS
HMCommandListenerBase::convert(const string& task)
{
    map<string, HM_COMMAND_TASKS>::iterator it;
    if((it = commands.find(task)) != commands.end())
    {
        return it->second;
    }
    return UNDEFINED;
}

unique_ptr<HMDataPacking> getPackingVersion(string& strVersion)
{
    try
    {
        unsigned long version = std::stoul(strVersion, nullptr, 0);
        switch (version)
        {
        case HM_CONTROL_SOCKET_VERSION:
            return make_unique<HMDataPacking>();
        }
    }
    catch(std::invalid_argument& e)
    {
        return nullptr;
    }
    return nullptr;
}

void
HMCommandListenerBase::handleCommands(string& command, HMSocketUtilBase& socketBase)
{
    unique_ptr<HMDataPacking> dataPacking;
    vector<string> cmd_args;
    tokenize(command, cmd_args);
    if(cmd_args.size() <= 0)
    {
        HMLog(HM_LOG_DEBUG, "No command Received after Connect");//LCOV_EXCL_LINE;
        return; //LCOV_EXCL_LINE;
    }
    dataPacking = getPackingVersion(cmd_args[0]);
    if (dataPacking == nullptr)
    {
        socketBase.sendMessage(NULL, 0);
        return;
    }
    cmd_args.erase(cmd_args.begin());
    if (cmd_args.size() <= 0)
    {
        HMLog(HM_LOG_DEBUG, "Command name missing"); //LCOV_EXCL_LINE;
        return; //LCOV_EXCL_LINE;
    }
    HM_COMMAND_TASKS task = convert(cmd_args[0]);
    bool result = true;
    uint64_t buflen;
    unique_ptr<char[]> returnResult;
    vector<string> returnList;
    shared_ptr<HMState> current;
    switch (task)
    {
    case RELOAD:
        if(cmd_args.size() > 1)
        {
            result = m_stateManager.reloadDaemonConfigs(cmd_args[1]);
        }
        else
        {
            result = m_stateManager.reloadDaemonConfigs();
        }
        returnResult = dataPacking->packBool(result, buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case HOSTGROUPINFO:
        if(cmd_args.size() > 1)
        {
            returnResult = createHostGroup(dataPacking, cmd_args[1], buflen);
            socketBase.sendMessage(returnResult.get(), buflen);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name for command:%s", HM_CMD_HOSTGROUP.c_str());
        }
        break;
    case LOADFBINFO:
        if(cmd_args.size() > 1)
        {
            returnResult = getloadfbdata(dataPacking, cmd_args[1], buflen);
            socketBase.sendMessage(returnResult.get(), buflen);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name for command:%s", HM_CMD_LOADFB.c_str());
        }
        break;
    case THREADINFO:
        HMAPIThreadInfo info;
        info.m_numIdleThreads = m_stateManager.getIdleThreads();
        info.m_numThreads = m_stateManager.getNThreads();
        returnResult = dataPacking->packThreadInfo(info, buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case WORKQUEUEINFO:
        returnResult = dataPacking->packUInt(m_stateManager.m_workQueue.queueSize(), buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case SCHDQUEUEINFO:
        returnResult = dataPacking->packUInt(m_stateManager.getEventQueueSize(), buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case SETHOSTSTATUS:
        if(cmd_args.size() > 3)
        {
            string hostGroupName = cmd_args[1];
            string host = cmd_args[2];
            string status = cmd_args[3];
            int hoststatus = stoi(status);
            HMLog(HM_LOG_DEBUG3, "[COMMANDLISTENERBASE] SethostStatus for HostGroup %s host Name %s to Status:%d",
                    hostGroupName.c_str(),
                    host.c_str(),
                    hoststatus);
            setHostStatus(hostGroupName, host,hoststatus);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name, host Name and Status for command:%s",HM_CMD_SETHOSTSTATUS);
        }
        break;
    case HOSTGROUPLIST:
        getAllHostGroupNames(returnList);
        returnResult = dataPacking->packList(returnList, buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case HOSTLIST:
        if(cmd_args.size() > 1)
        {
            getHosts(cmd_args[1], returnList);
            returnResult = dataPacking->packList(returnList, buflen);
            socketBase.sendMessage(returnResult.get(), buflen);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name for command:%s", HM_CMD_HOSTGROUPLIST.c_str());
        }
        break;
    case HOSTCHECK:
        if(cmd_args.size() > 2)
        {
            vector<HMDataCheckResult> hostResults;
            if(getHostCheck(cmd_args[1], cmd_args[2], hostResults))
            {
                returnResult = dataPacking->packDataCheckResults(hostResults, buflen);
            }
            socketBase.sendMessage(returnResult.get(), buflen);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name and HostName for command:%s", HM_CMD_HOSTCHECK.c_str());
        }
        break;
    case HOSTRESULTS:
        if (cmd_args.size() > 2)
        {
            uint32_t size = std::stoul(cmd_args[2], NULL, 0);
            unique_ptr<char[]> data = make_unique<char[]>(size);
            HMDataHostCheck dataHostCheck;
            if (socketBase.receiveMessage(data.get(), size)) {
                if(dataPacking->unpackDataHostCheck(data, size, dataHostCheck))
                {
                    returnResult = getHostResults(dataPacking, cmd_args[1], dataHostCheck, buflen);
                    socketBase.sendMessage(returnResult.get(), buflen);
                }
            } else {
                socketBase.sendMessage(nullptr, 0);
            }
        } else {
            HMLog(HM_LOG_DEBUG, "Missing HostName, size of data for command:%s",
                    HM_CMD_HOSTRESULTS.c_str());
        }
        break;
    case LOADFBINFOIP:
        if (cmd_args.size() > 3)
        {
            HMIPAddress address;
            if (address.set(cmd_args[3]))
            {
                returnResult = getloadfbdata(dataPacking, cmd_args[1],
                        cmd_args[2], address, buflen);
                socketBase.sendMessage(returnResult.get(), buflen);
            }
            else
            {
                socketBase.sendMessage(nullptr, 0);
                HMLog(HM_LOG_DEBUG, "Invalid IP address for command:%s",
                        HM_CMD_HOSTIPRESULTS.c_str());
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostName, size of data for command:%s",
                    HM_CMD_HOSTRESULTS.c_str());
        }
        break;
    case HOSTIPRESULTS:
        if (cmd_args.size() > 3)
        {
            uint32_t size = std::stoul(cmd_args[3], NULL, 0);
            unique_ptr<char[]> data = make_unique<char[]>(size);
            HMDataHostCheck dataHostCheck;
            HMIPAddress address;
            if (socketBase.receiveMessage(data.get(), size))
            {
                if(dataPacking->unpackDataHostCheck(data, size, dataHostCheck))
                {
                    if(address.set(cmd_args[2]))
                    {
                        returnResult = getHostResults(dataPacking, cmd_args[1], address, dataHostCheck, buflen);
                        socketBase.sendMessage(returnResult.get(), buflen);
                    }
                    else
                    {
                        socketBase.sendMessage(nullptr, 0);
                        HMLog(HM_LOG_DEBUG, "Invalid IP address for command:%s",
                                            HM_CMD_HOSTIPRESULTS.c_str());
                    }
                }
            }
            else
            {
                socketBase.sendMessage(nullptr, 0);
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostName, size of data for command:%s",
                    HM_CMD_HOSTRESULTS.c_str());
        }
        break;
    case HOSTGROUPPARAMS:
        if(cmd_args.size() > 1)
        {
            buflen = 0;
            HMDataHostGroup group(cmd_args[1]);
            if (getHostGroupInfo(cmd_args[1], group))
            {
                returnResult = dataPacking->packDataHostGroup(group, buflen);
            }
            socketBase.sendMessage(returnResult.get(), buflen);
        } else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name for command:%s", HM_CMD_HOSTGROUPPARAMS.c_str());
        }
        break;
    case HOSTSCHDINFO:
        if(cmd_args.size() > 2)
        {
            returnResult = getHostSchdInfo(dataPacking, cmd_args[1], cmd_args[2], buflen);
            socketBase.sendMessage((char*)returnResult.get(), buflen);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name and Host name for command:%s", HM_CMD_HOSTSCHDINFO.c_str());
        }
        break;
    case SETLOGLEVEL:
        if(cmd_args.size() > 1)
        {
            m_stateManager.setLogLevel(HMLogBase::parseLogLevel(cmd_args[1]));
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing loglevel for command:%s", HM_CMD_SETLOGLEVEL.c_str());
        }
        break;
    case GETLOGLEVEL:
           returnResult = dataPacking->packInt(m_stateManager.getLogLevel(), buflen);
           socketBase.sendMessage(returnResult.get(), buflen);
           break;
    case SETCONNECTIONTIMEOUT:
        if(cmd_args.size() > 1)
        {
            shared_ptr<HMState> current;
            m_stateManager.updateState(current);
            current->setConnectionTimeout(strtoul(cmd_args[1].c_str(), NULL, 0));
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing time<in seconds> for command:%s", HM_CMD_SETCONNECTIONTIMEOUT.c_str());
        }
        break;
    case GETCONNECTIONTIMEOUT:
        m_stateManager.updateState(current);
        returnResult = dataPacking->packUInt(current->getConnectionTimeout(), buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case HEALTHCHECK:
        if(cmd_args.size() > 2)
        {
            current->forceHealthCheck(cmd_args[1], cmd_args[2], m_stateManager.m_workQueue);
        }
        else if(cmd_args.size() > 1)
        {
            current->forceHealthCheck(cmd_args[1], m_stateManager.m_workQueue);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing hostgroup name for command:%s", HM_CMD_HEALTHCHECK.c_str());
        }
        break;
    case DNSCHECK:
        if(cmd_args.size() > 2)
        {
            current->forceDNSCheck(cmd_args[1], cmd_args[2], m_stateManager.m_workQueue);
        }
        else if(cmd_args.size() > 1)
        {
            current->forceDNSCheck(cmd_args[1], m_stateManager.m_workQueue);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing hostgroup name for command:%s", HM_CMD_DNSCHECK.c_str());
        }
        break;
    case SETMONFREQ:
        if(cmd_args.size() > 1)
        {
            m_stateManager.setMonitorFrequency(stoul(cmd_args[1]));
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing time<in seconds> for command:%s", HM_CMD_SETMONFREQ.c_str());
        }
        break;
    case GETMONFREQ:
        returnResult = dataPacking->packUInt(m_stateManager.getMonitorFrequency(), buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case SETSTRIDE:
        if(cmd_args.size() > 1)
        {
            m_stateManager.setStridePercent(stoul(cmd_args[1]));
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing time<in seconds> for command:%s", HM_CMD_SETSTRIDE.c_str());
        }
        break;
    case GETSTRIDE:
        returnResult = dataPacking->packUInt(m_stateManager.getStridePercent(), buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case SETWORKPERTHREAD:
        if(cmd_args.size() > 1)
        {
            m_stateManager.setWorkPerThreadRatio(stoul(cmd_args[1]));
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing time<in seconds> for command:%s", HM_CMD_SETWORKPERTHREAD.c_str());
        }
        break;
    case GETWORKPERTHREAD:
        returnResult = dataPacking->packUInt(m_stateManager.getWorkPerThreadRatio(), buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case SETTTLTRESH:
        if(cmd_args.size() > 1)
        {
            m_stateManager.m_workQueue.setTtlTreshold(stoul(cmd_args[1]));
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing time<in seconds> for command:%s", HM_CMD_SETTTLTRESH.c_str());
        }
        break;
    case GETTTLTRESH:
        returnResult = dataPacking->packUInt(m_stateManager.m_workQueue.getTtlTreshold(), buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case SETRECYCLE:
        if(cmd_args.size() > 1)
        {
            if(cmd_args[1] == "on" || cmd_args[1] == "ON" || cmd_args[1] == "On")
            {
                m_stateManager.setRecycle(true);
            }
            else if(cmd_args[1] == "off" || cmd_args[1] == "OFF" || cmd_args[1] == "Off")
            {
                m_stateManager.setRecycle(false);
            }
            else
            {
                HMLog(HM_LOG_DEBUG, "wrong argument for command:%s. It has to be <on/off>", HM_CMD_SETRECYCLE.c_str());
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing <on/off> for command:%s", HM_CMD_SETRECYCLE.c_str());
        }
        break;
    case GETRECYCLE:
        returnResult = dataPacking->packBool(m_stateManager.isRecycle(), buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case SETREMOTEQUERY:
        if (cmd_args.size() > 1)
        {
            if (cmd_args[1] == "on" || cmd_args[1] == "ON"
                    || cmd_args[1] == "On")
            {
                HMLog(HM_LOG_NOTICE, "NetCHASM server turned up for remote checks");
                m_stateManager.setEnableRemoteQueryReply(true);
            }
            else if (cmd_args[1] == "off" || cmd_args[1] == "OFF"
                    || cmd_args[1] == "Off")
            {
                HMLog(HM_LOG_NOTICE, "NetCHASM server turned down for remote checks");
                m_stateManager.setEnableRemoteQueryReply(false);
            }
            else
            {
                HMLog(HM_LOG_DEBUG,
                        "wrong argument for command:%s. It has to be <on/off>",
                        HM_CMD_SETREMOTEQUERY.c_str());
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing <on/off> for command:%s",
                    HM_CMD_SETREMOTEQUERY.c_str());
        }
        break;
    case GETREMOTEQUERY:
        returnResult = dataPacking->packBool(m_stateManager.isEnableRemoteQueryReply(),
                buflen);
        socketBase.sendMessage(returnResult.get(), buflen);
        break;
    case ADDDNSADDRESSES:
        if (cmd_args.size() > 2)
        {
            uint32_t size = std::stoul(cmd_args[2], NULL, 0);
            unique_ptr<char[]> data = make_unique<char[]>(size);
            set<HMIPAddress> addresses;
            if (socketBase.receiveMessage(data.get(), size))
            {
                if (dataPacking->unpackIPAddresses(data, size, addresses))
                {
                    m_stateManager.updateState(current);
                    result = current->m_dnsCache.addStaticDNSAddress(
                            cmd_args[1], addresses);
                    current->forceDNSCheck(cmd_args[1], HM_DNS_PLUGIN_STATIC, addresses, m_stateManager.m_workQueue);
                    returnResult = dataPacking->packBool(result, buflen);
                    socketBase.sendMessage(returnResult.get(), buflen);
                    current.reset();
                }
                else
                {
                    HMLog(HM_LOG_ERROR, "Failed to unpack data for command:%s",
                                        HM_CMD_ADDDNSADDRESSES.c_str());
                }
            }
            else
            {
                socketBase.sendMessage(nullptr, 0);
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostName, size of data for command:%s",
                    HM_CMD_ADDDNSADDRESSES.c_str());
        }
        break;
    case REMOVEDNSADDRESSES:
        if (cmd_args.size() > 2)
        {
            uint32_t size = std::stoul(cmd_args[2], NULL, 0);
            unique_ptr<char[]> data = make_unique<char[]>(size);
            set<HMIPAddress> addresses;
            if (socketBase.receiveMessage(data.get(), size))
            {
                if (dataPacking->unpackIPAddresses(data, size, addresses))
                {
                    m_stateManager.updateState(current);
                    result = current->m_dnsCache.removeStaticDNSAddress(
                            cmd_args[1], addresses);
                    current->forceDNSCheck(cmd_args[1], HM_DNS_PLUGIN_STATIC, addresses, m_stateManager.m_workQueue);
                    returnResult = dataPacking->packBool(result, buflen);
                    socketBase.sendMessage(returnResult.get(), buflen);
                    current.reset();
                }
                else
                {
                    HMLog(HM_LOG_ERROR, "Failed to unpack data for command:%s",
                            HM_CMD_ADDDNSADDRESSES.c_str());
                }
            }
            else
            {
                socketBase.sendMessage(nullptr, 0);
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostName, size of data for command:%s",
                    HM_CMD_REMOVEDNSADDRESSES.c_str());
        }
        break;
    case GETDNSADDRESSES:
        if (cmd_args.size() > 1)
        {
            set<HMIPAddress> addresses, addressesv6;
            m_stateManager.updateState(current);
            current->m_dnsCache.getStaticDNSAddress(cmd_args[1], false, addresses);
            current->m_dnsCache.getStaticDNSAddress(cmd_args[1], true, addressesv6);
            addresses.insert(addressesv6.begin(), addressesv6.end());
            returnResult = dataPacking->packIPAddresses(addresses, buflen);
            socketBase.sendMessage(returnResult.get(), buflen);
            current.reset();
        }
        break;

    case SETHOSTMARK:
        if (cmd_args.size() > 4)
        {
            HMIPAddress address;
            if (address.set(cmd_args[3]))
            {
                uint32_t size;
                try
                {
                    size = std::stoul(cmd_args[4], NULL, 0);
                }
                catch (const invalid_argument& ia)
                {
                    HMLog(HM_LOG_ERROR, "Failed to convert size value for command :%s, error: %s",
                                                    HM_CMD_SETHOSTMARK.c_str(), ia.what());
                    socketBase.sendMessage(nullptr, 0);
                    return;
                }
                set<int> values;
                unique_ptr<char[]> data = make_unique<char[]>(size);
                if (socketBase.receiveMessage(data.get(), size))
                {
                    if (dataPacking->unpackListInt64(data, size, values))
                    {
                        result = setHostMark(cmd_args[1], cmd_args[2], address,
                                values);
                        returnResult = dataPacking->packBool(result, buflen);
                        socketBase.sendMessage(returnResult.get(), buflen);
                    }
                    else
                    {
                        HMLog(HM_LOG_ERROR, "Failed to unpack mark values for command:%s",
                                HM_CMD_SETHOSTMARK.c_str());
                    }
                }
                else
                {
                    HMLog(HM_LOG_ERROR, "Failed to receive mark values for command %s", HM_CMD_SETHOSTMARK.c_str());
                    socketBase.sendMessage(nullptr, 0);
                }
            }
            else
            {
                socketBase.sendMessage(nullptr, 0);
                HMLog(HM_LOG_ERROR, "Invalid IP address for command:%s",
                                                            HM_CMD_SETHOSTMARK.c_str());
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroupName, HostName, IP-Address, size for command:%s",
                    HM_CMD_SETHOSTMARK.c_str());
        }
        break;
    case REMOVEHOSTMARK:
        if (cmd_args.size() > 4)
        {
            HMIPAddress address;
            if (address.set(cmd_args[3]))
            {
                uint32_t size = std::stoul(cmd_args[4], NULL, 0);
                set<int> values;
                unique_ptr<char[]> data = make_unique<char[]>(size);
                if (socketBase.receiveMessage(data.get(), size))
                {
                    if (dataPacking->unpackListInt64(data, size, values))
                    {
                        result = removeHostMark(cmd_args[1], cmd_args[2],
                                address, values);
                        returnResult = dataPacking->packBool(result, buflen);
                        socketBase.sendMessage(returnResult.get(), buflen);
                    }
                    else
                    {
                        HMLog(HM_LOG_ERROR,
                                "Failed to unpack mark values for command:%s",
                                HM_CMD_SETHOSTMARK.c_str());
                    }
                }
                else
                {
                    HMLog(HM_LOG_ERROR,
                            "Failed to receive mark values for command %s",
                            HM_CMD_SETHOSTMARK.c_str());
                    socketBase.sendMessage(nullptr, 0);
                }
            }
            else
            {
                socketBase.sendMessage(nullptr, 0);
                HMLog(HM_LOG_ERROR, "Invalid IP address for command:%s",
                        HM_CMD_SETHOSTMARK.c_str());
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroupName, HostName, IP-Address, size for command:%s",
                    HM_CMD_REMOVEHOSTMARK.c_str());
        }
        break;
    case GETHOSTMARK:
        if (cmd_args.size() > 3)
        {
            HMIPAddress address;
            if (address.set(cmd_args[3]))
            {
                returnResult = getHostMark(dataPacking, buflen, cmd_args[1], cmd_args[2], address);
                socketBase.sendMessage(returnResult.get(), buflen);
            }
            else
            {
                socketBase.sendMessage(nullptr, 0);
                HMLog(HM_LOG_ERROR, "Invalid IP address for command:%s",
                        HM_CMD_SETHOSTMARK.c_str());
            }
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroupName, HostName, IP-Address for command:%s",
                    HM_CMD_GETHOSTMARK.c_str());
        }
        break;
    case UNDEFINED:
        HMLog(HM_LOG_DEBUG, "[COMMANDLISTENERBASE] Undefined command to handle");
    }
    if(!result)
    {
        HMLog(HM_LOG_DEBUG, "%s : failed", cmd_args[0].c_str());//LCOV_EXCL_LINE;
    }
}

void
HMCommandListenerBase::shutDown()
{
    m_keepRunning = false;
    listernerShutDown();
    for(auto it = m_handlerThreads.begin(); it != m_handlerThreads.end(); ++it)
    {
        it->join();
    }
    m_handlerThreads.clear();
}

//LCOV_EXCL_START; can't be tested
void
HMCommandListenerBase::throwException(string errStr)
{
    lock_guard<mutex> lg(m_exceptionMutex);
    error_code ec(errno, generic_category());
    throw system_error(ec, errStr);
}
//LCOV_EXCL_STOP;

bool
HMCommandListenerBase::getHostGroupInfo(const string& name, HMDataHostGroup& group)
{
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    auto hgi = current->m_hostGroups.find(name);
    if(hgi == current->m_hostGroups.end())
    {
        return false;
    }
    group = hgi->second;
    return true;
}

unique_ptr<char[]>
HMCommandListenerBase::getHostSchdInfo(unique_ptr<HMDataPacking>& dataPacking, const string& hostgroup, string& host, uint64_t& buflen)
{
    buflen = 0;
    unique_ptr<char[]> data;
    HMAPIDNSSchedInfo dns;
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    HMDataHostGroupMap::iterator hostGroupInfo;
    if ((hostGroupInfo = current->m_hostGroups.find(hostgroup))
            == current->m_hostGroups.end())
    {
        return data;
    }
    dns.m_hasv4 = false;
    dns.m_hasv6 = false;
    HMDataHostCheck check;
    HMDataCheckParams params;
    hostGroupInfo->second.getHostCheck(check);
    HMDNSLookup dnsCheck(check.getDnsPlugin());
    HMDNSLookup dnsCheckV4(check.getDnsPlugin(), false);
    HMDNSLookup dnsCheckV6(check.getDnsPlugin(), true);
    hostGroupInfo->second.getCheckParameters(params);
    map<pair<string, HMDNSLookup>, HMDNSResult>::const_iterator v4Result;
    map<pair<string, HMDNSLookup>, HMDNSResult>::const_iterator v6Result;
    if ((check.getDualStack() & HM_DUALSTACK_IPV4_ONLY)
            && current->m_dnsCache.getDNSResult(host, dnsCheckV4, v4Result))
    {
        dns.m_hasv4 = true;
        dns.m_v4LastCheckTime =
                v4Result->second.getResultTime().getTimeSinceEpoch();
        dns.m_v4State = HM_API_WORK_STATE(v4Result->second.getQueryState());
        dns.m_v4NextCheckTime =
                v4Result->second.nextQueryTime().getTimeSinceEpoch();
    }
    if ((check.getDualStack() & HM_DUALSTACK_IPV6_ONLY)
            && current->m_dnsCache.getDNSResult(host, dnsCheckV6, v6Result))
    {
        dns.m_hasv6 = true;
        dns.m_v6LastCheckTime =
                v6Result->second.getResultTime().getTimeSinceEpoch();
        dns.m_v6State = HM_API_WORK_STATE(v6Result->second.getQueryState());
        dns.m_v6NextCheckTime =
                v6Result->second.nextQueryTime().getTimeSinceEpoch();
    }
    set<HMIPAddress> addresses;
    if (current->m_dnsCache.getAddresses(host, check.getDualStack(), dnsCheck, addresses))
    {
        for (auto it = addresses.begin(); it != addresses.end(); ++it)
        {
            HMIPAddress ip(*it);
            HMCheckHeader header(host, ip, check, params);
            HMDataCheckResult hcResult;
            current->m_checkList.getCheckResult(header, hcResult);
            HMAPIHostSchedInfo hc;
            hc.m_lastCheckTime = hcResult.m_checkTime.getTimeSinceEpoch();
            hc.m_state = HM_API_WORK_STATE(hcResult.m_queryState);
            hc.m_nextCheckTime = current->m_checkList.nextCheckTime(host, ip,
                    check).getTimeSinceEpoch();
            hc.m_address.m_type = ip.getType();
            if (hc.m_address.m_type == AF_INET)
            {
                hc.m_address.m_ip.addr = ip.addr4();
            } else
            {
                hc.m_address.m_ip.addr6 = ip.addr6();
            }
            dns.m_hostScheduleInfo.push_back(hc);
        }
    }
    return dataPacking->packHostSchedInfo(dns, buflen);
}

uint32_t
HMCommandListenerBase::getHostGroupResults(const string& name, vector<HMGroupCheckResult>& results)
{
    HMLog(HM_LOG_DEBUG3, "Getting getHostGroupResults in commandlistener for %s", name.c_str());
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    current->m_datastore.get()->getGroupCheckResults(name, true, false, results);
    return results.size();
}

unique_ptr<char[]>
HMCommandListenerBase::createHostGroup(unique_ptr<HMDataPacking>& dataPacking, string& hostGroupName, uint64_t& buflen)
{
    buflen = 0;
    HMDataHostGroup group(hostGroupName);
    vector<HMGroupCheckResult> results;
    if(!getHostGroupInfo(hostGroupName, group))
    {
        std::unique_ptr<char[]> data;
        return data;
    }
    getHostGroupResults(hostGroupName, results);
    return dataPacking->packHostGroupInfo(group, results, buflen);
}


unique_ptr<char[]>
HMCommandListenerBase::getloadfbdata (unique_ptr<HMDataPacking>& dataPacking, string& rotationName, uint64_t& buflen)
{
    buflen = 0;
    unique_ptr<char[]> data;
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    HMDataHostGroup group(rotationName);
    if(!getHostGroupInfo(rotationName, group))
    {
        return data;
    }
    std::vector<HMGroupAuxResult> results;
    current->m_datastore->getGroupAuxInfo(rotationName, true, true, results);
    return dataPacking->packAuxInfo(results, group.getCheckTTL(), buflen);
}

unique_ptr<char[]>
HMCommandListenerBase::getloadfbdata (unique_ptr<HMDataPacking>& dataPacking, string& hostName, string& sourceURL, HMIPAddress& address, uint64_t& buflen)
{
    buflen = 0;
    unique_ptr<char[]> data;
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    HMAuxInfo auxInfo;
    if(!current->m_auxCache.getAuxInfo(hostName, sourceURL, address, auxInfo))
    {
        return data;
    }
    return dataPacking->packAuxInfo(auxInfo, hostName, address, buflen);
}

bool
HMCommandListenerBase::setHostStatus(const string& hostGroupName, const string& host, bool forceHostStatus)
{
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);

    auto hgi = current->m_hostGroups.find(hostGroupName);
    if(hgi == current->m_hostGroups.end())
    {
        return false;
    }
    HMDataHostGroup group = hgi->second;   //got hostgroup info
    HMDataHostCheck check;
    group.getHostCheck(check);
    HMIPAddress ip;

    // if we have an ip and no host we get the host list
    if(ip.set(host))
    {
        const vector<string> *hosts = group.getHostList();
        for(auto hostname = hosts->begin(); hostname != hosts->end(); ++hostname)
        {
            current->m_checkList.setForceHostStatus(*hostname, check, ip, forceHostStatus);
        }
    }
    else
    {
        current->m_checkList.setForceHostStatus(host, check, ip, forceHostStatus);
    }
    return true;
}

void
HMCommandListenerBase::getAllHostGroupNames(vector<string>& groupNames)
{
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    for(auto it = current->m_hostGroups.begin(); it != current->m_hostGroups.end(); ++it)
    {
        groupNames.push_back(it->first);
    }
}

void
HMCommandListenerBase::getHosts(const string& hostGroupName, vector<string>& hostNames)
{
    HMDataHostGroup group(hostGroupName);
    getHostGroupInfo(hostGroupName, group);
    const vector<string>* hosts = group.getHostList();
    hostNames.reserve(hosts->size());
    for(auto it = hosts->begin(); it != hosts->end(); ++it)
    {
        hostNames.push_back(*it);
    }
}

bool
HMCommandListenerBase::getHostCheck(const string& hostGroupName, const string& hostName, vector<HMDataCheckResult>& hostResults)
{
    string groupName = hostGroupName;
    HMDataHostGroup group(hostGroupName);
    vector<HMGroupCheckResult> results;
    vector<HMDataCheckResult> finalResults;
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);

    auto hgi = current->m_hostGroups.find(groupName);
    if(hgi == current->m_hostGroups.end())
    {
        return false;
    }
    group = hgi->second;
    HMDataHostCheck check;
    HMDataCheckParams checkparams;
    group.getHostCheck(check);
    group.getCheckParameters(checkparams);

    current->m_datastore->getGroupCheckResults(hostGroupName, false, false, results);
    for(HMGroupCheckResult result : results)
    {
        if(result.m_hostName == hostName)
        {
            finalResults.push_back(result.m_result);
        }
    }
    if(finalResults.size() == 0)
    {
        return false;
    }
    hostResults.reserve(finalResults.size());
    for (HMDataCheckResult result : finalResults)
    {
        hostResults.push_back(result);
    }
    return true;
}

unique_ptr<char[]>
HMCommandListenerBase::getHostResults(unique_ptr<HMDataPacking>& dataPacking, const string& hostName, HMDataHostCheck& hostCheck, uint64_t& dataSize)
{
    dataSize = 0;
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    vector<pair<HMDataCheckParams, HMDataCheckResult>> tempResults;
    multimap<HMDataCheckParams, HMDataCheckResult> finalResults;
    set<HMIPAddress> addresses;
    HMDNSLookup dnsHostCheck(hostCheck.getDnsPlugin());
    current->m_dnsCache.getAddresses(hostName, hostCheck.getDualStack(), dnsHostCheck, addresses);
    for( const HMIPAddress address: addresses)
    {
        current->m_checkList.getCheckResults(hostName, hostCheck,
                address, tempResults);
        for (auto it : tempResults)
        {
            finalResults.insert(make_pair(it.first, it.second));
        }
    }
    return dataPacking->packHostResults(finalResults, dataSize);
}

unique_ptr<char[]>
HMCommandListenerBase::getHostResults(unique_ptr<HMDataPacking>& dataPacking, const string& hostName, const HMIPAddress& address, HMDataHostCheck& hostCheck, uint64_t& dataSize)
{
    dataSize = 0;
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    vector<pair<HMDataCheckParams, HMDataCheckResult>> finalResults;
    current->m_checkList.getCheckResults(hostName, hostCheck, address,
            finalResults);
    return dataPacking->packHostResults(finalResults, dataSize);
}

bool
HMCommandListenerBase::setHostMark(const string& hostGroupName, const string& hostName, const HMIPAddress& address, const set<int>& values)
{
    string groupName = hostGroupName;
    HMDataHostGroup group(hostGroupName);
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);

    auto hgi = current->m_hostGroups.find(groupName);
    if(hgi == current->m_hostGroups.end())
    {
        return false;
    }
    group = hgi->second;
    HMDataHostCheck check;
    if(group.getHostCheck(check))
    {
        return m_stateManager.m_hostMark.setSocketOptionValues(hostName, address, check, values);
    }
    return false;
}

bool
HMCommandListenerBase::removeHostMark(const string& hostGroupName, const string& hostName, const HMIPAddress& address, const set<int>& values)
{
    string groupName = hostGroupName;
    HMDataHostGroup group(hostGroupName);
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);

    auto hgi = current->m_hostGroups.find(groupName);
    if(hgi == current->m_hostGroups.end())
    {
        return false;
    }
    group = hgi->second;
    HMDataHostCheck check;
    if (group.getHostCheck(check))
    {
        return m_stateManager.m_hostMark.removeSocketOptionValues(hostName, address, check, values);
    }
    return false;
}

unique_ptr<char[]>
HMCommandListenerBase::getHostMark(unique_ptr<HMDataPacking>& dataPacking, uint64_t& dataSize, const string& hostGroupName, const string& hostName, const HMIPAddress& address)
{
    unique_ptr<char[]> data;
    dataSize = 0;
    string groupName = hostGroupName;
    HMDataHostGroup group(hostGroupName);
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    auto hgi = current->m_hostGroups.find(groupName);
    if(hgi == current->m_hostGroups.end())
    {
        return data;
    }
    group = hgi->second;
    HMDataHostCheck check;
    set<int> values;
    if (group.getHostCheck(check))
    {
        if(m_stateManager.m_hostMark.getSocketOptionValues(hostName, address,check, values))
        {
            return dataPacking->packListInt64(values, dataSize);
        }
    }
    return data;
}

void
HMCommandListenerBase::cleanHandlerThreads()
{
    lock_guard<std::mutex> lg(m_handlerMutex);
    for(auto it = m_handlerThreads.begin(); it != m_handlerThreads.end();)
    {
        map<thread::id, bool>::iterator its = m_handlerThreadsStatus.find(it->get_id());
        if(its != m_handlerThreadsStatus.end() && its->second == true)
        {
            it->join();
            it = m_handlerThreads.erase(it);
            m_handlerThreadsStatus.erase(its);
        }
        else
        {
            ++it;
        }
    }
}

bool
HMCommandListenerBase::isValidCommand(const string& strCommand)
{
    return HMCommandListenerBase::convert(strCommand) != HM_COMMAND_TASKS::UNDEFINED;
}

void
HMCommandListenerBase::tokenize(string& command, vector<string>& tokens)
{
    istringstream stream(command);
    string word;
    while (getline(stream, word, ' '))
    {
        tokens.push_back(word);
    }
}

