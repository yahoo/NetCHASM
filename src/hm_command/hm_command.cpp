// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <vector> 
#include <map>
#include <sstream>
#include <chrono>
#include <algorithm>

#include "HMControlBase.h"
#include "HMControlLinuxSocket.h"


using namespace std;


string server_path = HM_DEFAULT_USD_PATH;
bool verbose = false;
static void usage(char* name)
{
    string command = "man ";
    command.append(name);
    {
        cout << "Usage: "<< name <<" [options] command" << endl << "Options:" << endl
                << "-s      <socket-path> [default: " << server_path << "]" << endl
                << "-v      verbose " << endl
                << "Commands:"<<endl
                << "\t" <<"threadinfo\tReturns # of total and idle threads" <<endl
                << "\t" <<"workqueueinfo\tReturns length of WorkQueue" <<endl
                << "\t" <<"schdqueueinfo\tReturns length of Scheduler queue" <<endl
                << "\t" <<"hostschdinfo <hostgroup name> <hostname> \tReturns Scheduling info about host in the hostgroup" <<endl
                << "\t" <<"dnscheck <hostgroup name> <hostname>\tqueues dnscheck (hostname is optional)" <<endl
                << "\t" <<"healthcheck <hostgroup name> <hostname>\tqueues healthcheck (hostname is optional)" <<endl
                << "\t" <<"getloglevel \t get the current daemon log verbosity" <<endl
                << "\t" <<"setloglevel <level>\t set the daemon log verbosity" <<endl
                << "\t" <<"getconnectiontimeout \t get the current connection timeout for healthchecks" <<endl
                << "\t" <<"setconnectiontimeout <timeout in seconds>\t set the connection timeout for healthchecks" <<endl
                << "\t" <<"getstride \t get the current thread-reduce percent in threadpool monitoring" <<endl
                << "\t" <<"setstride <level>\t set the thread-reduce percent in threadpool monitoring"<< endl
                << "\t" <<"getmonitorfrequency \t get the current threadpool monitoring frequency" << endl
                << "\t" <<"setmonitorfrequency <time in seconds>\t set threadpool monitoring frequency" << endl
                << "\t" <<"getttlthreshold \t get the current monitoring TTL threshold for threadpool monitoring" << endl
                << "\t" <<"setttlthreshold <threshold in percent>\t set monitoring TTL threshold for threadpool monitoring" << endl
                << "\t" <<"getworkperthreadratio \t get the current work to thread ratio for each thread in threadpool monitoring" << endl
                << "\t" <<"setworkperthreadratio <num of work>\t set the work to thread ratio for each thread in threadpool monitoring" << endl
                << "\t" <<"getrecycle \t get the recycle of threads enable status" << endl
                << "\t" <<"setrecycle <on/off>\t enable/disable the threads recycling " << endl
                << "\t" <<"getremotequery \t get the remote query enable status of daemon" << endl
                << "\t" <<"setremotequery <on/off>\t enable/disable the remote query state of the daemon " << endl
                << "\t" <<"gethostmark <hostgroupname> <hostname> <ip-address>\tReturns host mark value for the host" <<endl
                << "\t" <<"removehostmark <hostgroupname> <hostname> <ip-address>\tRemove host mark values for a host" <<endl
                << "\t" <<"sethostmark <hostgroupname> <hostname> <ip-address> <value>\tSet host mark value for a host" <<endl;
    }
}

void handleError(char *prog, string &name)
{
    cerr<<prog<<":"<<"Missing parameters for command "<< name<<endl;
    cerr<<"Use \""<<prog<<" -h\" for usage information"<<endl;
    exit(-3);
}

string printWorkState(HM_API_WORK_STATE state)
{
    switch(state)
    {
    case HM_CHECK_INACTIVE:
        return "INACTIVE";
    case HM_CHECK_QUEUED:
        return "QUEUED";
    case HM_CHECK_IN_PROGRESS:
        return "IN PROGRESS";
    case HM_CHECK_FAILED:
        return "FAILED";
    default:
        return "Invalid Response";
    }
}

bool runCommand(const vector<string> &strArgs)
{
    HMControlLinuxSocketClient socketAPI(server_path);
    if (!socketAPI.isConnected())
    {
        cerr << "Failed to connect to socket:" << server_path << endl;
    }
    if (strArgs[0] == HM_CMD_THREADINFO)
    {
        HMAPIThreadInfo threadInfo;
        bool status = socketAPI.getThreadInfo(threadInfo);
        cout << "Number of threads  = "  << threadInfo.m_numThreads << endl;
        cout << "Number of idle threads  = "  << threadInfo.m_numIdleThreads << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_WORKQUEUEINFO)
    {
        uint32_t workQueueLen = 0;
        bool status = socketAPI.getWorkQueue(workQueueLen);
        cout << "Work queue length = " << workQueueLen << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_SCHDQUEUEINFO)
    {
        uint64_t schQueueLen = 0;
        bool status = socketAPI.getSchQueue(schQueueLen);
        cout << "Schedule queue length = " << schQueueLen << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_HOSTSCHDINFO)
    {
        HMAPIDNSSchedInfo dns;
        dns.m_hasv4 = false;
        dns.m_hasv6 = false;
        socketAPI.getHostScheduleInfo(strArgs[1], strArgs[2], dns);
        if(!dns.m_hasv4 && !dns.m_hasv6)
        {
            cout<<"No results found.\nCheck Hostgroup name and host name"<<endl;
            return false;
        }
        if(dns.m_hasv4)
        {
            cout<<"===========DNS IPv4==========="<<endl;
            HMTimeStamp lastCheck, nextCheck;
            lastCheck.setTime(dns.m_v4LastCheckTime);
            nextCheck.setTime(dns.m_v4NextCheckTime);
            cout<<"\tState : " << printWorkState(dns.m_v4State)<<endl;
            cout<<"\tLast check time : " << lastCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
            cout<<"\tNext check time : " << nextCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
        }
        if(dns.m_hasv6)
        {
            cout<<"===========DNS IPv6==========="<<endl;
            HMTimeStamp lastCheck, nextCheck;
            lastCheck.setTime(dns.m_v6LastCheckTime);
            nextCheck.setTime(dns.m_v6NextCheckTime);
            cout<<"\tState : " << printWorkState(dns.m_v6State)<<endl;
            cout<<"\tLast check time : " << lastCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
            cout<<"\tNext check time : " << nextCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
        }
        cout << "=============Total #IPs : " << dns.m_hostScheduleInfo.size() << "=============" << endl;
        for (size_t i = 0; i < dns.m_hostScheduleInfo.size(); i++)
        {
            cout<<"===========Health Checks IP:"<< (i+1) <<"==========="<<endl;
            cout<<"\t"<<dns.m_hostScheduleInfo[i].m_address.toString()<<endl;
            HMTimeStamp lastCheck, nextCheck;
            lastCheck.setTime(dns.m_hostScheduleInfo[i].m_lastCheckTime);
            nextCheck.setTime(dns.m_hostScheduleInfo[i].m_nextCheckTime);
            cout << "\t\tState : "
                    << printWorkState(dns.m_hostScheduleInfo[i].m_state)
                    << endl;
            cout << "\t\tLast check time : "
                    << lastCheck.print("(%a %b %d %H:%M:%S %Y)") << endl;
            cout << "\t\tNext check time : "
                    << nextCheck.print("(%a %b %d %H:%M:%S %Y)") << endl;
        }
    }
    else if (strArgs[0] == HM_CMD_SETLOGLEVEL)
    {
        return socketAPI.setLogLevel(strArgs[1]);
    }
    else if (strArgs[0] == HM_CMD_SETMONFREQ)
    {
        uint32_t freq = std::stoul(strArgs[1], nullptr, 0);
        return socketAPI.setMonitoringFrequency(freq);
    }
    else if (strArgs[0] == HM_CMD_SETSTRIDE)
    {
        uint32_t stride = std::stoul(strArgs[1], nullptr, 0);
        return socketAPI.setStride(stride);
    }
    else if (strArgs[0] == HM_CMD_SETCONNECTIONTIMEOUT)
    {
        uint64_t connectionTimeOut = std::stoull(strArgs[1], nullptr, 0);
        return socketAPI.setConnectionTimeOut(connectionTimeOut);
    }
    else if (strArgs[0] == HM_CMD_SETTTLTRESH)
    {
        uint32_t ttlTresh = std::stoul(strArgs[1], nullptr, 0);
        return socketAPI.setTTLTreshold(ttlTresh);
    }
    else if (strArgs[0] == HM_CMD_SETWORKPERTHREAD)
    {
        uint32_t workPerThread = std::stoul(strArgs[1], nullptr, 0);
        return socketAPI.setWorkPerThread(workPerThread);
    }
    else if (strArgs[0] == HM_CMD_SETRECYCLE)
    {
        if (strArgs[1] == "on")
        {
            return socketAPI.setRecycleOn();
        }
        else
        {
            return socketAPI.setRecycleOff();
        }
    }
    else if (strArgs[0] == HM_CMD_SETREMOTEQUERY)
    {
        if (strArgs[1] == "on")
        {
            return socketAPI.setRemoteQueryOn();
        }
        else
        {
            return socketAPI.setRemoteQueryOff();
        }
    }
    else if (strArgs[0] == HM_CMD_GETLOGLEVEL)
    {
        string log;
        bool status = socketAPI.getLogLevel(log);
        cout << "Log Level = " << log << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_GETLOGLEVEL)
    {
        string log;
        bool status = socketAPI.getLogLevel(log);
        cout << "Log Level = " << log << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_GETCONNECTIONTIMEOUT)
    {
        uint64_t connectionTimeout = 0;
        bool status = socketAPI.getConnectionTimeout(connectionTimeout);
        cout << "Connection Timeout = " << connectionTimeout << " milliseconds"
                << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_GETMONFREQ)
    {
        uint32_t freq = 0;
        bool status = socketAPI.getMonitoringFrequency(freq);
        cout << "Monitoring Frequency = " << freq << " seconds" << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_GETSTRIDE)
    {
        uint32_t stride = 0;
        bool status = socketAPI.getStride(stride);
        cout << "Stride Percentage = " << stride << "%" << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_GETTTLTRESH)
    {
        uint32_t ttlTresh = 0;
        bool status = socketAPI.getTTLTreshold(ttlTresh);
        cout << "TTL Threshold = " << ttlTresh << "%" << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_GETWORKPERTHREAD)
    {
        uint32_t workPerThread = 0;
        bool status = socketAPI.getWorkPerThread(workPerThread);
        cout << "Work per Thread Ratio = " << workPerThread << endl;
        return status;
    }
    else if (strArgs[0] == HM_CMD_GETRECYCLE)
    {
        bool recycle = socketAPI.isRecycleOn();
        string status = recycle ? "Enabled" : "Disabled";
        cout << "Recycle Threads Status = " << status << endl;
        return true;
    }
    else if (strArgs[0] == HM_CMD_GETREMOTEQUERY)
    {
        bool remoteQueryStatus;
        if (socketAPI.getRemoteQueryOn(remoteQueryStatus))
        {
            string remoteQueryStatusString =
                    remoteQueryStatus ? "Enabled" : "Disabled";
            cout << "Remote query Enabled = " << remoteQueryStatusString
                    << endl;
            return true;
        }
        return false;
    }
    else if (strArgs[0] == HM_CMD_SETHOSTMARK)
    {
        HMAPIIPAddress address;
        if (!address.set(strArgs[3]))
        {
            cerr << "Invalid address:" << strArgs[3] << endl;
            return false;
        }
        return socketAPI.setHostMark(strArgs[1], strArgs[2], address,
                std::stoi(strArgs[4], NULL, 0));
    }
    else if (strArgs[0] == HM_CMD_REMOVEHOSTMARK)
    {
        HMAPIIPAddress address;
        if (!address.set(strArgs[3]))
        {
            cerr << "Invalid address:" << strArgs[3] << endl;
            return false;
        }
        return socketAPI.removeHostMark(strArgs[1], strArgs[2], address);
    }
    else if (strArgs[0] == HM_CMD_GETHOSTMARK)
    {
        HMAPIIPAddress address;
        if (!address.set(strArgs[3]))
        {
            cerr << "Invalid address:" << strArgs[3] << endl;
            return false;
        }
        int value;
        bool status = socketAPI.getHostMark(strArgs[1], strArgs[2], address,
                value);
        if (status)
        {
            cout << "Mark value for HostGroup: " << strArgs[1] << " host: "
                    << strArgs[2] << " Address: " << address.toString() << " = "
                    << value << endl;
        }
        else
        {
            cerr << "Failed to get Mark for HostGroup: " << strArgs[1]
                    << " host: " << strArgs[2] << " Address: "
                    << address.toString() << endl;
        }
        return status;
    }
    else if(strArgs[0] == HM_CMD_SETHOSTSTATUS)
    {
        if (strArgs[3] == "ON" || strArgs[3] == "on")
        {
            return socketAPI.setForceStatusDown(strArgs[1], strArgs[2]);
        }
        else
        {
            return socketAPI.unsetForceStatusDown(strArgs[1], strArgs[2]);
        }
    }
    return true;
}

int main(int argc, char* argv[])
{
    bool bSuccess = false;
    int opt;

    while ((opt = getopt(argc, argv, "t:s:vh")) != -1)
    {
        switch (opt) {
        case 'v':
            verbose = true;
            break;
        case 's':
            server_path = std::string(optarg);
            break;
        case 'h':
            usage(argv[0]);
            exit(0);
        case '?':
            usage(argv[0]);
            exit(1);
        }
    }

    vector<string> strArgs;
    for (int arg = optind; arg < argc; arg++)
    {
        const string strArg = argv[arg];
        strArgs.push_back(strArg);
    }

    if (strArgs.empty())
    {
        cerr<<"Missing command"<<endl;
        cerr<<"Use \"hm_command -h\" for usage information"<<endl;
        return -1;
    }

    if (HMCommandListenerBase::isValidCommand(strArgs[0]) == false)
    {
        cerr << "Invalid command: " << strArgs[0] << endl;
        return -2;
    }
    switch(HMCommandListenerBase::convert(strArgs[0]))
    {
    case WORKQUEUEINFO:
    case THREADINFO:
    case SCHDQUEUEINFO:
    case GETLOGLEVEL:
    case GETCONNECTIONTIMEOUT:
    case GETMONFREQ:
    case GETSTRIDE:
    case GETTTLTRESH:
    case GETWORKPERTHREAD:
    case GETRECYCLE:
    case GETREMOTEQUERY:
        bSuccess = runCommand(strArgs);
        return (bSuccess) ? 0 : 1;
    case HEALTHCHECK:
    case DNSCHECK:
    case SETLOGLEVEL:
    case SETCONNECTIONTIMEOUT:
    case SETMONFREQ:
    case SETSTRIDE:
    case SETTTLTRESH:
    case SETWORKPERTHREAD:
    case SETRECYCLE:
    case SETREMOTEQUERY:
        if (strArgs.size() < 2)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        return (bSuccess) ? 0 : 1;
    case HOSTSCHDINFO:
        if(strArgs.size() < 3)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        return (bSuccess) ? 0 : 1;
    case REMOVEHOSTMARK:
    case GETHOSTMARK:
    case SETHOSTSTATUS:
        if (strArgs.size() < 4)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        break;
    case SETHOSTMARK:
        if (strArgs.size() < 5)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        break;
    case ADDDNSADDRESSES:
    case REMOVEDNSADDRESSES:
    case GETDNSADDRESSES:
        cerr << "This command is supported in hm_staticdns" << endl;
        return -1;
    case RELOAD:
    case HOSTGROUPINFO:
    case LOADFBINFO:
    case HOSTGROUPLIST:
    case HOSTCHECK:
    case HOSTGROUPPARAMS:
    case HOSTLIST:
    case HOSTRESULTS:
    case HOSTIPRESULTS:
    case LOADFBINFOIP:
    case UNDEFINED:
        cerr << "Not all commands are supported at this time" << endl;
        return -3;
    }
    if (!bSuccess)
    {
        cerr << "Failure executing command " << strArgs[0] << endl;
    }
    return (bSuccess) ? 0 : 1;
}

