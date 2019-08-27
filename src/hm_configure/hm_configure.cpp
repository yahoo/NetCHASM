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
            << "-v      verbose " << endl
            << "Commands:"<<endl
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
            << "\t" <<"setremotequery <on/off>\t enable/disable the remote query state of the daemon " << endl;

}

void handleError(char *prog, string &name)
{
    cout<<"Missing parameters for "<< name<<endl;
    usage(prog);
    exit(-3);
}

bool runCommand(const vector<string> &strArgs)
{
    HMControlLinuxSocketClient socketAPI(server_path);
    if (strArgs[0] == HM_CMD_SETLOGLEVEL)
    {
        return socketAPI.setLogLevel(strArgs[1]);
    }

    if (strArgs[0] == HM_CMD_SETMONFREQ)
    {
        uint32_t freq= std::stoul(strArgs[1], nullptr, 0);
        return socketAPI.setMonitoringFrequency(freq);
    }

    if (strArgs[0] == HM_CMD_SETSTRIDE)
    {
        uint32_t stride = std::stoul(strArgs[1], nullptr, 0);
        return socketAPI.setStride(stride);
    }

    if (strArgs[0] == HM_CMD_SETCONNECTIONTIMEOUT)
    {
        uint64_t connectionTimeOut = std::stoull(strArgs[1], nullptr, 0);
        return socketAPI.setConnectionTimeOut(connectionTimeOut);
    }

    if (strArgs[0] == HM_CMD_SETTTLTRESH)
    {
        uint32_t ttlTresh = std::stoul(strArgs[1], nullptr, 0);
        return socketAPI.setTTLTreshold(ttlTresh);
    }

    if (strArgs[0] == HM_CMD_SETWORKPERTHREAD)
    {
        uint32_t workPerThread = std::stoul(strArgs[1], nullptr, 0);
        return socketAPI.setWorkPerThread(workPerThread);
    }

    if (strArgs[0] == HM_CMD_SETRECYCLE)
    {
        if(strArgs[1] == "on")
        {
            return socketAPI.setRecycleOn();
        }
        else
        {
            return socketAPI.setRecycleOff();
        }
    }

    if (strArgs[0] == HM_CMD_SETREMOTEQUERY)
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


    if (strArgs[0] == HM_CMD_GETLOGLEVEL)
    {
        string log;
        bool status = socketAPI.getLogLevel(log);
        cout << "Log Level = " << log << endl;
        return status;
    }

    if (strArgs[0] == HM_CMD_GETLOGLEVEL)
    {
        string log;
        bool status = socketAPI.getLogLevel(log);
        cout << "Log Level = " << log << endl;
        return status;
    }

    if (strArgs[0] == HM_CMD_GETCONNECTIONTIMEOUT)
    {
        uint64_t connectionTimeout = 0;
        bool status = socketAPI.getConnectionTimeout(connectionTimeout);
        cout << "Connection Timeout = " << connectionTimeout << " milliseconds" << endl;
        return status;
    }

    if (strArgs[0] == HM_CMD_GETMONFREQ)
    {
        uint32_t freq = 0;
        bool status = socketAPI.getMonitoringFrequency(freq);
        cout << "Monitoring Frequency = " << freq << " seconds" << endl;
        return status;
    }

    if (strArgs[0] == HM_CMD_GETSTRIDE)
    {
        uint32_t stride = 0;
        bool status = socketAPI.getStride(stride);
        cout << "Stride Percentage = " << stride << "%"<< endl;
        return status;
    }

    if (strArgs[0] == HM_CMD_GETTTLTRESH)
    {
        uint32_t ttlTresh = 0;
        bool status = socketAPI.getTTLTreshold(ttlTresh);
        cout << "TTL Threshold = " << ttlTresh << "%" << endl;
        return status;
    }

    if (strArgs[0] == HM_CMD_GETWORKPERTHREAD)
    {
        uint32_t workPerThread = 0;
        bool status = socketAPI.getWorkPerThread(workPerThread);
        cout << "Work per Thread Ratio = " << workPerThread << endl;
        return status;
    }
    if (strArgs[0] == HM_CMD_GETRECYCLE)
    {
        bool recycle = socketAPI.isRecycleOn();
        string status = recycle? "Enabled": "Disabled";
        cout << "Recycle Threads Status = " << status<< endl;
        return true;
    }
    if (strArgs[0] == HM_CMD_GETREMOTEQUERY)
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
    return true;
}

int main(int argc, char* argv[])
{
    bool bSuccess = false;
    int opt;

    while ((opt = getopt(argc, argv, "s:vh")) != -1)
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
    case HOSTSCHDINFO:
    case WORKQUEUEINFO:
    case THREADINFO:
    case SCHDQUEUEINFO:
    case HEALTHCHECK:
    case DNSCHECK:
        cerr << "This command is supported in hm_command" << endl;
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

