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

#include <vector> 
#include <map>
#include <sstream>
#include <chrono>
#include <algorithm>

#include "HMControlBase.h"
#include "HMControlLinuxSocket.h"


using namespace std;


string server_path = HM_DEFAULT_USD_PATH;
int sleep_duration = 3;
bool verbose = false;
static void usage(char* name)
{
    cout << "Usage: "<< name <<" [options] command" << endl << "Options:" << endl
            << "-s      <socket-path> [default: " << server_path << "]" << endl
            << "-t      <time-in-seconds> [default: " << sleep_duration << "]" << endl
            << "-v      verbose " << endl
            << "Commands:"<<endl
            << "\t" <<"threadinfo\tReturns # of total and idle threads" <<endl
            << "\t" <<"workqueueinfo\tReturns length of WorkQueue" <<endl
            << "\t" <<"schdqueueinfo\tReturns length of Scheduler queue" <<endl
            << "\t" <<"hostschdinfo <hostgroup name> <hostname> \tReturns Scheduling info about host in the hostgroup" <<endl
            << "\t" <<"dnscheck <hostgroup name> <hostname>\tqueues dnscheck (hostname is optional)" <<endl
            << "\t" <<"healthcheck <hostgroup name> <hostname>\tqueues healthcheck (hostname is optional)" <<endl;
}

void handleError(char *prog, string &name)
{
    cout<<"Missing parameters for "<< name<<endl;
    usage(prog);
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
            cout<<"\t\tState : " << printWorkState(dns.m_hostScheduleInfo[i].m_state)<<endl;
            cout<<"\t\tLast check time : " << lastCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
            cout<<"\t\tNext check time : " << nextCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
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
        case 't':
            sleep_duration = atoi(optarg);
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
        usage(argv[0]);
        return 0;
    }

    if (HMCommandListenerBase::isValidCommand(strArgs[0]) == false)
    {
        cerr << "Invalid command: " << strArgs[0] << endl;
        return -2;
    }
    switch(HMCommandListenerBase::convert(strArgs[0]))
    {
    case HOSTSCHDINFO:
        if(strArgs.size() < 3)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        return (bSuccess) ? 0 : 1;

    case WORKQUEUEINFO:
    case THREADINFO:
    case SCHDQUEUEINFO:
        bSuccess = runCommand(strArgs);
        return (bSuccess) ? 0 : 1;
    case HEALTHCHECK:
    case DNSCHECK:
        if (strArgs.size() < 2)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        return (bSuccess) ? 0 : 1;

    case SETLOGLEVEL:
    case SETCONNECTIONTIMEOUT:
    case SETMONFREQ:
    case SETSTRIDE:
    case SETTTLTRESH:
    case SETWORKPERTHREAD:
    case SETRECYCLE:
    case GETLOGLEVEL:
    case GETCONNECTIONTIMEOUT:
    case GETMONFREQ:
    case GETSTRIDE:
    case GETTTLTRESH:
    case GETWORKPERTHREAD:
    case GETRECYCLE:
    case GETREMOTEQUERY:
    case SETREMOTEQUERY:
        cerr << "This command is supported in hm_configure" << endl;
        return -1;
    case ADDDNSADDRESSES:
    case REMOVEDNSADDRESSES:
    case GETDNSADDRESSES:
        cerr << "This command is supported in hm_staticdns" << endl;
        return -1;
    case RELOAD:
    case HOSTGROUPINFO:
    case LOADFBINFO:
    case SETHOSTSTATUS:
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
}

