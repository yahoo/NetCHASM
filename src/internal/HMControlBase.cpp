// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <vector>

#include "HMControlBase.h"
#include "HMStateManager.h"
#include "HMLogBase.h"

using namespace std;

map<string, HM_COMMAND_TASKS> commands =
{
    { HM_CMD_RELOAD, RELOAD },
    { HM_CMD_HOSTGROUP, HOSTGROUPINFO },
    { HM_CMD_LOADFB, LOADFBINFO },
    { HM_CMD_THREADINFO, THREADINFO },
    { HM_CMD_WORKQUEUEINFO, WORKQUEUEINFO },
    { HM_CMD_SCHDQUEUEINFO, SCHDQUEUEINFO },
    { HM_CMD_SETHOSTSTATUS, SETHOSTSTATUS },
    { HM_CMD_HOSTGROUPLIST, HOSTGROUPLIST },
    { HM_CMD_HOSTLIST, HOSTLIST },
    { HM_CMD_HOSTCHECK, HOSTCHECK },
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
    { HM_CMD_SETRECYCLE, SETRECYCLE }
};

HMCommandListenerBase::HMCommandListenerBase(string &socketPath ,HMStateManager &stateManager) :
        m_stateManager(stateManager),
        m_keepRunning(false),
        m_internalSocketClient(-1),
        m_internalSocketServer(-1),
        m_internalSocketSClient(-1),
        m_socketPath(socketPath) {}

void
HMCommandListenerBase::init()
{
    m_internalSocketPath = m_socketPath + "_internal";
    unlinkSocket(m_internalSocketPath);
    struct sockaddr_un addr;
    if(m_internalSocketPath.length() >= sizeof(addr.sun_path))
    {
        //LCOV_EXCL_START; can't be tested
        string msg = "socket path " + m_internalSocketPath + " exceeds max unix domain socket path";
        throw length_error(msg);
        //LCOV_EXCL_STOP; can't be tested
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;

    size_t len = m_internalSocketPath.copy(addr.sun_path, m_internalSocketPath.length(), 0);
    addr.sun_path[len] = '\0';

    // TODO: Should we use a SOCK_DGRAM socket, then we can't use accept,
    // listen etc.
    m_internalSocketServer = socket(AF_UNIX, SOCK_STREAM, 0);
    if(m_internalSocketServer < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to create socket " + m_internalSocketPath + ", error desc: ");
        //LCOV_EXCL_STOP;
    }

    int tmp = 1;
    if(setsockopt(m_internalSocketServer, SOL_SOCKET, SO_REUSEADDR, (char*) &tmp, sizeof tmp) < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to setsockopt REUSEADDR " + m_internalSocketPath + ", error desc: ");
        //LCOV_EXCL_STOP;
    }

    if(bind(m_internalSocketServer, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to bind socket " + m_internalSocketPath + ", error desc: ");
        //LCOV_EXCL_STOP;
    }
    int rc = listen(m_internalSocketServer, 10);
    if(rc == -1)
    {
        throwException("Failed to listen on socket, error desc: "); //LCOV_EXCL_LINE; can't be tested
    }

    thread t(&HMCommandListenerBase::acceptThread, this);
    struct sockaddr_un server;
    m_internalSocketClient = socket(AF_UNIX, SOCK_STREAM, 0);
    if(m_internalSocketClient < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to create socket " + m_internalSocketPath + ", error desc: ");
        //LCOV_EXCL_STOP;
    }
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, m_internalSocketPath.c_str());
    if(connect(m_internalSocketClient, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0)
    {
        //LCOV_EXCL_START; can't be tested
        throwException("Failed to connect socket " + m_internalSocketPath + ", error desc: ");
        //LCOV_EXCL_STOP; can't be tested
    }
    t.join();
}

void
HMCommandListenerBase::acceptThread()
{
    m_internalSocketSClient = accept(m_internalSocketServer, NULL, NULL);
    if(m_internalSocketSClient < 0)
    {
        throwException("Failed to accept socket " + m_internalSocketPath + ", error desc: ");//LCOV_EXCL_LINE; can't be tested
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

void
HMCommandListenerBase::handleCommands(string& command, int clientSock)
{
    vector<string> cmd_args;
    tokenize(command, cmd_args);
    if(cmd_args.size() <= 0)
    {
        HMLog(HM_LOG_DEBUG, "No command Received after Connect");//LCOV_EXCL_LINE;
        return; //LCOV_EXCL_LINE;
    }
    HM_COMMAND_TASKS task = convert(cmd_args[0]);
    bool result = true;
    uint32_t buflen;
    unique_ptr<char[]> returnResult;
    string returnString;
    HM_LOG_LEVEL level;
    uint64_t result_64;
    uint32_t result_32;
    bool result_bool;
    int resultCode;

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
        resultCode = result ? 0 : 1;
        responseMessage(clientSock, &resultCode, sizeof(resultCode));
        break;
    case HOSTGROUPINFO:
        if(cmd_args.size() > 1)
        {
            char buf[65535];
            createHostGroup(buf, sizeof(buf), cmd_args[1]);
            responseMessage(clientSock, buf, sizeof(buf));
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name for command:%s", HM_CMD_HOSTGROUP.c_str());
        }
        break;
    case LOADFBINFO:
        if(cmd_args.size() > 1)
        {
            returnResult = getloadfbdata(cmd_args[1], buflen);
            responseMessageVarLen(clientSock, returnResult.get(), buflen);
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name for command:%s", HM_CMD_LOADFB.c_str());
        }
        break;
    case THREADINFO:
        hm_threadInfo_s info;
        info.numIdleThreads = m_stateManager.getIdleThreads();
        info.numThreads = m_stateManager.getNThreads();
        responseMessage(clientSock, &info, sizeof(info));
        break;
    case WORKQUEUEINFO:
        uint32_t workQSize;
        workQSize = m_stateManager.m_workQueue.queueSize();
        responseMessage(clientSock, &workQSize, sizeof(workQSize));
        break;
    case SCHDQUEUEINFO:
        uint64_t schdQSize;
        schdQSize = m_stateManager.getEventQueueSize();
        responseMessage(clientSock, &schdQSize, sizeof(schdQSize));
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
        getAllHostGroupNames(returnString);
        responseMessageVarLen(clientSock, returnString.c_str(), returnString.length());
        break;
    case HOSTLIST:
        if(cmd_args.size() > 1)
        {
            getHosts(cmd_args[1], returnString);
            responseMessageVarLen(clientSock, returnString.c_str(), returnString.length());
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name for command:%s", HM_CMD_HOSTGROUPLIST.c_str());
        }
        break;
    case HOSTCHECK:
        if(cmd_args.size() > 2)
        {
            hm_hostcheck_t hostcheck;
            hostcheck.statustime = 0;
            getHostCheck(cmd_args[1], cmd_args[2], &hostcheck);
            responseMessage(clientSock, &hostcheck, sizeof(hostcheck));
        }
        else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name and HostName for command:%s", HM_CMD_HOSTCHECK.c_str());
        }
        break;
    case HOSTGROUPPARAMS:
        if(cmd_args.size() > 1)
        {
            uint32_t buflen;
            returnResult = getHostGroupParams(cmd_args[1], buflen);
            responseMessageVarLen(clientSock, returnResult.get(), buflen);
        } else
        {
            HMLog(HM_LOG_DEBUG, "Missing HostGroup name for command:%s", HM_CMD_HOSTGROUPPARAMS.c_str());
        }
        break;
    case HOSTSCHDINFO:
        if(cmd_args.size() > 2)
        {
            hm_dns_sched_t dns;
            vector<hm_hc_sched_t> hcs;
            getHostSchdInfo(cmd_args[1], cmd_args[2], dns, hcs);
            responseMessage(clientSock, &dns, sizeof(dns));
            for(hm_hc_sched_t hc : hcs)
            {
                responseMessage(clientSock, &hc, sizeof(hc));
            }

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
           level = m_stateManager.getLogLevel();
           responseMessage(clientSock, &level, sizeof(level));
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
        result_64 = current->getConnectionTimeout();
        responseMessage(clientSock, &result_64, sizeof(result_64));
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
        result_32 = m_stateManager.getMonitorFrequency();
        responseMessage(clientSock, &result_32, sizeof(result_32));
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
        result_32 = m_stateManager.getStridePercent();
        responseMessage(clientSock, &result_32, sizeof(result_32));
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
        result_32 = m_stateManager.getWorkPerThreadRatio();
        responseMessage(clientSock, &result_32, sizeof(result_32));
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
        result_32 = m_stateManager.m_workQueue.getTtlTreshold();
        responseMessage(clientSock, &result_32, sizeof(result_32));
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
        result_bool = m_stateManager.isRecycle();
        responseMessage(clientSock, &result_bool, sizeof(result_bool));
        break;
    case UNDEFINED:
        HMLog(HM_LOG_DEBUG, "[COMMANDLISTENERBASE] Undefined command to handle");
    }
    if(!result)
    {
        HMLog(HM_LOG_DEBUG, "%s : failed", cmd_args[0]);//LCOV_EXCL_LINE;
    }
}

void
HMCommandListenerBase::shutdown()
{
    m_keepRunning = false;
    listernerShutDown();
    for(auto it = m_handlerThreads.begin(); it != m_handlerThreads.end(); ++it)
    {
        it->join();
    }
    m_handlerThreads.clear();
    unlinkSocket(m_internalSocketPath);
    unlinkSocket(m_socketPath);
}

void
HMCommandListenerBase::unlinkSocket(string& socketPath)
{
    if(unlink(socketPath.c_str()) != 0 && errno != ENOENT)
    {
        //LCOV_EXCL_START;
        throwException("Failed to unlink socket path " + socketPath + ", error desc: ");
        //LCOV_EXCL_STOP;
    }
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

bool
HMCommandListenerBase::getHostSchdInfo(const string& hostgroup, string& host, hm_dns_sched_t& dns, vector<hm_hc_sched_t>& hcs)
{
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    HMDataHostGroupMap::iterator hostGroupInfo;
    if((hostGroupInfo = current->m_hostGroups.find(hostgroup)) != current->m_hostGroups.end())
    {
        dns.hasv4 = false;
        dns.hasv6 = false;
        dns.count = 0;
        HMDataHostCheck check;
        HMDataCheckParams params;
        hostGroupInfo->second.getHostCheck(check);
        hostGroupInfo->second.getCheckParameters(params);
        map<pair<string, bool>, HMDNSResult>::const_iterator v4Result;
        map<pair<string, bool>, HMDNSResult>::const_iterator v6Result;
        if((check.getDualStack() & HM_DUALSTACK_IPV4_ONLY) && current->m_dnsCache.getDNSResult(host, false, v4Result))
        {
            dns.hasv4 = true;
            dns.v4LastCheckTime = v4Result->second.getResultTime().getTimeSinceEpoch();
            dns.v4State = v4Result->second.getQueryState();
            dns.v4NextCheckTime = v4Result->second.nextQueryTime().getTimeSinceEpoch();
        }
        if((check.getDualStack() & HM_DUALSTACK_IPV6_ONLY) && current->m_dnsCache.getDNSResult(host, true, v6Result))
        {
            dns.hasv6 = true;
            dns.v6LastCheckTime = v6Result->second.getResultTime().getTimeSinceEpoch();
            dns.v6State = v6Result->second.getQueryState();
            dns.v6NextCheckTime = v6Result->second.nextQueryTime().getTimeSinceEpoch();
        }
        set<HMIPAddress> addresses;
        if(current->m_dnsCache.getAddresses(host, check.getDualStack(), addresses))
        {
            for(auto it = addresses.begin(); it != addresses.end(); ++it)
            {
                HMIPAddress ip(*it);
                HMCheckHeader header(host, ip, check, params);
                HMDataCheckResult hcResult;
                current->m_checkList.getCheckResult(header, hcResult);
                hm_hc_sched_t hc;
                hc.lastCheckTime = hcResult.m_checkTime.getTimeSinceEpoch();
                hc.state = hcResult.m_queryState;
                hc.nextCheckTime = current->m_checkList.nextCheckTime(host, ip, check).getTimeSinceEpoch();

                hc.addrtype = ip.getType();
                if(hc.addrtype == AF_INET)
                {
                    hc.addr.s_addr = ip.addr4();
                }
                else if(hc.addrtype == AF_INET6)
                {
                    hc.addr.s_addr6 = ip.addr6();
                }
                hcs.push_back(hc);
                dns.count += 1;
            }
        }
    }
    return false;
}

uint32_t
HMCommandListenerBase::getHostGroupResults(const string& name, vector<HMGroupCheckResult>& results)
{
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    current->m_datastore.get()->getGroupCheckResults(name, true, false, results);
    return results.size();
}

void
HMCommandListenerBase::createHostGroup(char* buf, size_t buflen, string& hostGroupName)
{
    // TODO: This code looks like a copy paste of gethostinfo2 API call
    // We should refactor and combine the two functions
    string groupName = hostGroupName;
    HMDataHostGroup group(hostGroupName);
    vector<HMGroupCheckResult> results;
    hm_nameinfo_sock_t* ni = (hm_nameinfo_sock_t*) buf;
    if(!getHostGroupInfo(groupName, group))
    {
        ni->ni_errno = ENOENT;
        ni->ni_check_status = false;
        return;
    }

    if(!getHostGroupResults(groupName, results))
    {
        // LCOV_EXCL_START;
        ni->ni_errno = ENOENT;
        // LCOV_EXCL_STOP;
    }

    ni->ni_size = sizeof(*ni);
    ni->ni_ttl = group.getCheckTTL();
    ni->ni_group_threshold = group.getGroupThreshold() ? : HM_DEFAULT_GROUP_THRESHOLD;
    ni->ni_mode = group.getPassthroughInfo();
    ni->ni_numhost = results.size();
    buf = (char*) buf + ni->ni_size;
    buflen -= ni->ni_size;

    for(auto it = results.begin(); it != results.end(); ++it)
    {
        hm_hostinfo2_t* infop = (hm_hostinfo2_t*) buf;
        int hnlen = it->m_hostName.size();

        if(buflen < sizeof(*infop) + hnlen + 1)
        {
            // LCOV_EXCL_START;
            ni->ni_errno = ENOMEM;
            ni->ni_check_status = false;
            // LCOV_EXCL_STOP;
        }

        infop->hi_size = sizeof(*infop) + hnlen + 1;
        infop->hi_addrtype = it->m_address.getType();
        if(infop->hi_addrtype == AF_INET)
        {
            infop->hi_addr.s_addr = it->m_address.addr4();
        }
        else if(infop->hi_addrtype == AF_INET6)
        {
            infop->hi_addr.s_addr6 = it->m_address.addr6();
        }

        infop->hi_status = it->m_result.m_status;
        infop->hi_reason = it->m_result.m_reason;
        infop->hi_connect_rt = it->m_result.m_responseTime;
        infop->hi_smoothed_connect_rt = it->m_result.m_smoothedResponseTime;
        infop->hi_total_rt = it->m_result.m_totalResponseTime;
        infop->hi_statustime = it->m_result.m_changeTime.getTimeSinceEpoch();
        strncpy(infop->hi_hostname, it->m_hostName.c_str(), it->m_hostName.length());

        buf = (char*) buf + infop->hi_size;
        buflen -= infop->hi_size;
    }
    ni->ni_check_status = true;
}


unique_ptr<char[]>
HMCommandListenerBase::getloadfbdata (string rotationName, uint32_t& buflen)
{
    vector<string> names;
    HMAuxInfo results;
    vector<string> fileContents;
    HMTimeStamp updateTime;

    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    HMDataHostGroup group(rotationName);

    if(!getHostGroupInfo(rotationName, group))
    {
        uint32_t size = sizeof(hm_loadfbdata_t);
        unique_ptr<char[]> data = make_unique<char[]>(size);
        hm_loadfbdata_t* b = (hm_loadfbdata_t*) data.get();
        b->ni_check_status = false;
        b->ni_errno = ENOENT;
        b->ld_count = 0;
        b->ld_size = size;
        b->ld_ttl = 0;
        b->ld_updatetime = 0;
        buflen = size;
        return data;
    }

    // TODO shoehorn the new function but don't take advantage of the speedup until re-write
    vector<HMGroupAuxResult> results2;
    if(!current->m_datastore->getGroupAuxInfo(rotationName, false, false, results2))
    {
        // TODO deal with missing information
    }

    for(auto it = group.getHostList()->begin(); it != group.getHostList()->end(); ++it)
    {
        // now grab the current DNS
        set<HMIPAddress> addresses;
        if(!current->m_dnsCache.getAddresses(*it, HM_DUALSTACK_BOTH, addresses))
        {
            // No Addresses for that hostname
            continue;
        }
        for(auto address = addresses.begin(); address != addresses.end(); ++address)
        {
            results.m_auxData.clear();
            results.m_ts.setTime(0);
            if(!current->m_auxCache.getAuxInfo(*it, group.getCheckInfo(), *address, results))
            {
                // deal with missing host info
            }

            bool loadFile = false;
            bool oobFile = false;

            for(auto auxData = results.m_auxData.begin(); auxData != results.m_auxData.end(); ++auxData)
            {
               if((*auxData)->m_type == HM_LOAD_FILE)
                {
                    loadFile = true;
                }
                else if((*auxData)->m_type == HM_OOB_FILE)
                {
                    oobFile = true;
                }
            }

            string fileName;
            string xml;

            if(loadFile)
            {
                fileName = rotationName + "_" + *it + "_LoadFile.xml";
                current->m_auxCache.genAuxXML(results, HM_LOAD_FILE, rotationName, xml);
                fileContents.push_back(xml);
            }

            if(oobFile)
            {
                fileName = rotationName + "_" + *it + "_OOBFile.xml";
                current->m_auxCache.genAuxXML(results, HM_OOB_FILE, rotationName, xml);
                fileContents.push_back(xml);
            }
        }
    }

    // Now deal with the return struct crap
    uint32_t size = sizeof(hm_loadfbdata_t);
    for(auto it = fileContents.begin(); it != fileContents.end(); ++it)
    {
        size += (sizeof(hm_loadfbfile_t) + it->size()+1);
    }

    unique_ptr<char[]> data = make_unique<char[]>(size);
    hm_loadfbdata_t* b = (hm_loadfbdata_t*) data.get();
    b->ld_count = fileContents.size();
    b->ld_size = size;
    b->ni_check_status = true;
    b->ni_errno = 0;
    // TODO can we keep to ms only to avoid confusion?
    b->ld_ttl = group.getCheckTTL() * 1000;
    b->ld_updatetime = updateTime.getTimeSinceEpoch() * 1000;
    buflen = size;
    char* ptr = data.get();
    ptr = b->ld_filecontents;

    for(auto it = fileContents.begin(); it != fileContents.end(); ++it)
    {
        hm_loadfbfile_t* fileBuf = (hm_loadfbfile_t*)ptr;
        fileBuf->file_size = it->size();
        strncpy(fileBuf->file_buffer, it->c_str(), it->size()+1);
        ptr += (sizeof(hm_loadfbfile_t) + it->size()+1);
    }
    return data;
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
HMCommandListenerBase::getAllHostGroupNames(string& groupNames)
{
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    for(auto it = current->m_hostGroups.begin(); it != current->m_hostGroups.end(); ++it)
    {
        groupNames+=it->first;
        groupNames+=",";
    }
    if(groupNames.length() > 0)
    {
        groupNames = groupNames.substr(0,groupNames.length()-1);
    }
}

void
HMCommandListenerBase::getHosts(const string& hostGroupName, string& hostNames)
{
    HMDataHostGroup group(hostGroupName);
    getHostGroupInfo(hostGroupName, group);
    const vector<string>* hosts = group.getHostList();
    for(auto it = hosts->begin(); it != hosts->end(); ++it)
    {
        hostNames += *it;
        hostNames += ",";
    }
    if(hostNames.length() > 0)
    {
        hostNames = hostNames.substr(0, hostNames.length()-1);
    }
}

void
HMCommandListenerBase::getHostCheck(const string& hostGroupName, const string& hostName, hm_hostcheck_t* hostCheck)
{
    string groupName = hostGroupName;
    HMDataHostGroup group(hostGroupName);
    vector<HMGroupCheckResult> results;

    shared_ptr<HMState> current;
    m_stateManager.updateState(current);

    auto hgi = current->m_hostGroups.find(groupName);
    if(hgi == current->m_hostGroups.end())
    {
        hostCheck->errnum = ENOENT;
        hostCheck->check_status = false;
        return;
    }
    group = hgi->second;
    HMDataHostCheck check;
    HMDataCheckParams checkparams;
    group.getHostCheck(check);
    group.getCheckParameters(checkparams);

    current->m_datastore->getGroupCheckResults(hostGroupName, false, false, results);
    uint64_t minTime = UINT64_MAX;
    hostCheck->check_status = false;
    bool found = false;
    HMDataCheckResult minAddress(checkparams.getTimeout());

    for(auto result = results.begin(); result != results.end(); ++result)
    {
        if(result->m_hostName != hostName)
        {
            continue;
        }
        if(result->m_result.m_checkTime.getTimeSinceEpoch() < minTime)
        {
            found = true;
            minAddress = result->m_result;
            minTime = result->m_result.m_checkTime.getTimeSinceEpoch();
        }
    }
    if(found)
    {
        hostCheck->check_status = true;
        hostCheck->status = minAddress.m_status;
        hostCheck->reason = minAddress.m_reason;
        hostCheck->connect_rt = minAddress.m_responseTime;
        hostCheck->smoothed_connect_rt = minAddress.m_smoothedResponseTime;
        hostCheck->total_rt = minAddress.m_totalResponseTime;
        hostCheck->statustime = minAddress.m_checkTime.getTimeSinceEpoch();

    }
    else
    {
        hostCheck->check_status = false;
    }
}

unique_ptr<char[]>
HMCommandListenerBase::getHostGroupParams(string& hostGroupName, uint32_t& buflen)
{
    string groupName = hostGroupName;
    HMDataHostGroup group(groupName);
    bool found = true;
    if(!getHostGroupInfo(hostGroupName, group))
    {
        found = false;
    }

    unique_ptr<char[]> retData;
    if(found)
    {
        buflen = sizeof(hm_grpcheckparams_t) + group.getCheckInfo().length() + 1;
        retData = make_unique<char[]>(buflen);
        hm_grpcheckparams_t *checkparams = (hm_grpcheckparams_t*) (retData.get());
        checkparams->port = group.getCheckPort();
        checkparams->checkType = group.getCheckType();
        checkparams->dualStack = group.getDualstack();
        checkparams->smoothingWindow = group.getSmoothingWindow();
        checkparams->maxFlaps = group.getMaxFlaps();
        checkparams->flapThreshold = group.getFlapThreshold();
        checkparams->checkInfoSize = group.getCheckInfo().length()+1;
        checkparams->numCheckRetries = group.getNumCheckRetries();
        checkparams->checkRetryDelay = group.getCheckRetryDelay();
        checkparams->groupThreshold = group.getGroupThreshold();
        checkparams->slowThreshold = group.getSlowThreshold();
        checkparams->checkTimeout = group.getCheckTimeout();
        checkparams->checkTTL = group.getCheckTTL();
        checkparams->mode = group.getPassthroughInfo();

        strncpy(checkparams->check_info, group.getCheckInfo().c_str(), checkparams->checkInfoSize);
    }
    else
    {
        buflen = 0;
        retData = make_unique<char[]>(buflen);
    }
    return retData;
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
