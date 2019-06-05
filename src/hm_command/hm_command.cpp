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

void printError(string msg)
{
    cerr << msg << ":" << strerror(errno) << endl;
}

bool createSocket(int& s)
{
    if ((s = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1)
    {
        printError("Failed to create socket, error: ");
        return false;
    }
    return true;
}

void closeSocket(int s)
{
    if (s != -1)
    {
        close(s);
    }
}

bool connectSocket(int sock, string& server_path)
{
    int len;
    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, server_path.c_str());
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sock, (struct sockaddr *) &remote, len) == -1)
    {
        printError("Failed to connect socket to " + server_path + ", error: ");
        closeSocket(sock);
        return false;
    }
    return true;
}

bool sendMessage(int s, string cmd)
{
    if (send(s, cmd.c_str(), cmd.length(), 0) == -1)
    {
        printError("failed to send msg " + cmd + " on socket, error desc: ");
        closeSocket(s);
        return false;
    }
    return true;
}

bool recvWorkQueue(int s,  uint32_t &workQLen)
{
    int n = read(s, &workQLen, sizeof(workQLen));
    if (n < 0)
    {
        printError("Failed to recv on socket for hm_hostcheck_t, error desc: ");
        closeSocket(s);
        return false;
    }
    return true;
}

bool recvHostSchd(int s,  hm_dns_sched_t &dns, vector<hm_hc_sched_t> &hcs)
{
    int n = read(s, &dns, sizeof(dns));
    if (n < 0)
    {
        printError("Failed to recv on socket for hm_dns_sched_t, error desc: ");
        closeSocket(s);
        return false;
    }
    for (uint64_t i = 0; i < dns.count; i++)
    {
        hm_hc_sched_t hc;
        int n = read(s, &hc, sizeof(hc));
        if(n < 0)
        {
            printError("Failed to recv on socket for hm_hc_sched_t, error desc: ");
            closeSocket(s);
            return false;
        }
        hcs.push_back(hc);
    }
    return true;
}

bool recvSchQueue(int s,  uint64_t &schQLen)
{
    int n = read(s, &schQLen, sizeof(schQLen));
    if (n < 0)
    {
        printError("Failed to recv on socket for hm_hostcheck_t, error desc: ");
        closeSocket(s);
        return false;
    }
    return true;
}

bool recvThreadInfo(int s,  hm_threadInfo_s& threadInfo)
{
    int n = read(s, &threadInfo, sizeof(threadInfo));
    if (n < 0)
    {
        printError("Failed to recv on socket for hm_hostcheck_t, error desc: ");
        closeSocket(s);
        return false;
    }
    return true;
}


bool runCommand(const vector<string> &strArgs)
{
    string strCommand;
    bool bRemoveLastWS = false;
    for (size_t i = 0; i < strArgs.size(); i++)
    {
        strCommand += strArgs[i];
        strCommand += " ";
        bRemoveLastWS = true;
    }

    if (bRemoveLastWS)
    {
        strCommand.erase(strCommand.size()-1);
    }

    int s;
    createSocket (s);
    if (!connectSocket(s, server_path))
    {
        return false;
    }

    if (!sendMessage(s, strCommand))
    {
        return false;
    }

    if (strArgs[0] == HM_CMD_THREADINFO)
    {
        hm_threadInfo_t threadInfo;
        recvThreadInfo(s, threadInfo);
        cout << "Number of threads  = "  << threadInfo.numThreads << endl;
        cout << "Number of idle threads  = "  << threadInfo.numIdleThreads << endl;
    }
    else if (strArgs[0] == HM_CMD_WORKQUEUEINFO)
    {
        uint32_t workQueueLen = 0;
        recvWorkQueue(s, workQueueLen);
        cout << "Work queue length = " << workQueueLen << endl;
    }
    else if (strArgs[0] == HM_CMD_SCHDQUEUEINFO)
    {
        uint64_t schQueueLen = 0;
        recvSchQueue(s, schQueueLen);
        cout << "Schedule queue length = " << schQueueLen << endl;
    }
    else if (strArgs[0] == HM_CMD_HOSTSCHDINFO)
    {
        hm_dns_sched_t dns;
        vector<hm_hc_sched_t> hcs;
        dns.hasv4 = false;
        dns.hasv6 = false;
        recvHostSchd(s, dns, hcs);
        if(!dns.hasv4 && !dns.hasv6)
        {
            cout<<"No results found.\nCheck Hostgroup name and host name"<<endl;
            return false;
        }
        if(dns.hasv4)
        {
            cout<<"===========DNS IPv4==========="<<endl;
            HMTimeStamp lastCheck, nextCheck;
            lastCheck.setTime(dns.v4LastCheckTime);
            nextCheck.setTime(dns.v4NextCheckTime);
            cout<<"\tState : " << printWorkState(dns.v4State)<<endl;
            cout<<"\tLast check time : " << lastCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
            cout<<"\tNext check time : " << nextCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
        }
        if(dns.hasv6)
        {
            cout<<"===========DNS IPv6==========="<<endl;
            HMTimeStamp lastCheck, nextCheck;
            lastCheck.setTime(dns.v6LastCheckTime);
            nextCheck.setTime(dns.v6NextCheckTime);
            cout<<"\tState : " << printWorkState(dns.v6State)<<endl;
            cout<<"\tLast check time : " << lastCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
            cout<<"\tNext check time : " << nextCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
        }
        cout << "=============Total #IPs : " << dns.count << "=============" << endl;
        for (size_t i = 0; i < hcs.size(); i++)
        {
            cout<<"===========Health Checks IP:"<< (i+1) <<"==========="<<endl;
            HMIPAddress ip(hcs[i].addrtype);
            if(hcs[i].addrtype == AF_INET)
            {
                ip.set(hcs[i].addr.s_addr);
            }
            if(hcs[i].addrtype == AF_INET6)
            {
                ip.set(hcs[i].addr.s_addr6);
            }
            cout<<"\t"<<ip.toString()<<endl;
            HMTimeStamp lastCheck, nextCheck;
            lastCheck.setTime(hcs[i].lastCheckTime);
            nextCheck.setTime(hcs[i].nextCheckTime);
            cout<<"\t\tState : " << printWorkState(hcs[i].state)<<endl;
            cout<<"\t\tLast check time : " << lastCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
            cout<<"\t\tNext check time : " << nextCheck.print("(%a %b %d %H:%M:%S %Y)")<<endl;
        }
    }

    closeSocket(s);
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
        cerr << "This command is supported in hm_configure" << endl;
        return -1;
    case RELOAD:
    case HOSTGROUPINFO:
    case LOADFBINFO:
    case SETHOSTSTATUS:
    case HOSTGROUPLIST:
    case HOSTCHECK:
    case HOSTGROUPPARAMS:
    case HOSTLIST:
    case UNDEFINED:
        cerr << "Not all commands are supported at this time" << endl;
        return -3;
    }
}

