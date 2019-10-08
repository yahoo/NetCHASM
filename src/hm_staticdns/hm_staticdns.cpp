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
static void usage(char* name)
{
    string command = "man ";
    command.append(name);
    {
        cout << "Usage: "<< name <<" [options] command" << endl << "Options:" << endl
            << "-s      <socket-path> [default: " << server_path << "]" << endl
            << "Commands:"<<endl
            << "\t" <<"getdnsaddresses <hostname>\tReturns static DNS address for the host" <<endl
            << "\t" <<"removednsaddresses <hostname> <address1> <address2> ....\tRemove addresses from static DNS for a host" <<endl
            << "\t" <<"adddnsaddresses <hostname> <address1> <address2> ....\tAdd addresses to static DNS for a host" <<endl;
    }
}

void handleError(char *prog, string &name)
{
    cerr<<prog<<":"<<"Missing parameters for command "<< name<<endl;
    cerr<<"Use \""<<prog<<" -h\" for usage information"<<endl;
    exit(-3);
}

bool runCommand(const vector<string> &strArgs)
{
    HMControlLinuxSocketClient socketAPI(server_path);
    if(!socketAPI.isConnected())
    {
        cerr<<"Failed to connect to socket:"<<server_path<<endl;
    }
    if (strArgs[0] == HM_CMD_ADDDNSADDRESSES)
    {
        vector<HMAPIIPAddress> addresses;
        for (size_t i = 2; i < strArgs.size(); i++)
        {
            HMAPIIPAddress address;
            if (address.set(strArgs[i]))
            {
                addresses.push_back(address);
            }
            else
            {
                cerr << "Invalid address:" << strArgs[i] << endl;
            }
        }
        bool status = socketAPI.addDNSAddresses(strArgs[1], addresses);
        return status;

    }
    else if (strArgs[0] == HM_CMD_REMOVEDNSADDRESSES)
    {
        vector<HMAPIIPAddress> addresses;
        for (size_t i = 2; i < strArgs.size(); i++)
        {
            HMAPIIPAddress address;
            if (address.set(strArgs[i]))
            {
                addresses.push_back(address);
            }
            else
            {
                cerr << "Invalid address:" << strArgs[i] << endl;
            }
        }
        bool status = socketAPI.removeDNSAddresses(strArgs[1], addresses);

        return status;
    }
    else if (strArgs[0] == HM_CMD_GETDNSADDRESSES)
    {

        vector<HMAPIIPAddress> addresses;
        bool status = socketAPI.getDNSAddresses(strArgs[1], addresses);
        for (const HMAPIIPAddress& address : addresses)
        {
            cout << "Address " << address.toString() << endl;
        }
        return status;
    }
    return true;
}

int main(int argc, char* argv[])
{
    bool bSuccess = false;
    int opt;

    while ((opt = getopt(argc, argv, "s:h")) != -1)
    {
        switch (opt) {
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
    case ADDDNSADDRESSES:
    case REMOVEDNSADDRESSES:
        if (strArgs.size() < 2)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        break;
    case GETDNSADDRESSES:
        if (strArgs.size() < 1)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        break;
    case HOSTSCHDINFO:
    case WORKQUEUEINFO:
    case THREADINFO:
    case SCHDQUEUEINFO:
    case HEALTHCHECK:
    case DNSCHECK:
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
    case REMOVEHOSTMARK:
    case GETHOSTMARK:
    case SETHOSTMARK:
        cerr << "This command is supported in hm_command" << endl;
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

    if (!bSuccess)
    {
        cerr << "Failure executing command " << strArgs[0] << endl;
    }
    return (bSuccess) ? 0 : 1;
}

