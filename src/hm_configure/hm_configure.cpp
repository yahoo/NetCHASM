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

map<int8_t, string> loglevels  = {
        { -2, "LOG ERROR" },
        { -1, "NONE" },
        { 0, "EMERGENCY" },
        { 1, "ALERT" },
        { 2, "CRITICAL" },
        { 3, "ERROR" },
        { 4, "WARNING" },
        { 5, "NOTICE" },
        { 6, "INFO" },
        { 7, "DEBUG" },
        { 8, "DEBUG2" },
        { 9, "DEBUG3" },
};

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
            << "\t" <<"setrecycle <on/off>\t enable/disable the threads recycling " << endl;
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

template <class T>
bool recv(int s,  T &var, const string &cmd)
{
    int n = read(s, &var, sizeof(var));
    if (n < 0)
    {
        printError("Failed to recv on socket for "+ cmd + ", error desc: ");
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

    if (strArgs[0] == HM_CMD_GETLOGLEVEL)
    {
        string log = "UNKNOWN";
        int8_t loglevel = 0;
        recv<int8_t>(s, loglevel, strArgs[0]);
        map<int8_t, string>::iterator it;
        if((it = loglevels.find(loglevel)) != loglevels.end()){
            log = it->second;
        }
        cout << "Log Level = " << log << endl;
    }

    if (strArgs[0] == HM_CMD_GETCONNECTIONTIMEOUT)
    {
        uint64_t connectionTimeout = 0;
        recv<uint64_t>(s, connectionTimeout, strArgs[0]);
        cout << "Connection Timeout = " << connectionTimeout << " milliseconds" << endl;
    }

    if (strArgs[0] == HM_CMD_GETMONFREQ)
    {
        uint32_t freq = 0;
        recv<uint32_t>(s, freq, strArgs[0]);
        cout << "Monitoring Frequency = " << freq << " seconds" << endl;
    }

    if (strArgs[0] == HM_CMD_GETSTRIDE)
    {
        uint32_t stride = 0;
        recv<uint32_t>(s, stride, strArgs[0]);
        cout << "Stride Percentage = " << stride << "%"<< endl;
    }

    if (strArgs[0] == HM_CMD_GETTTLTRESH)
    {
        uint32_t ttlTresh = 0;
        recv<uint32_t>(s, ttlTresh, strArgs[0]);
        cout << "TTL Threshold = " << ttlTresh << "%"<< endl;
    }

    if (strArgs[0] == HM_CMD_GETWORKPERTHREAD)
    {
        uint32_t workPerThread = 0;
        recv<uint32_t>(s, workPerThread, strArgs[0]);
        cout << "Work per Thread Ratio = " << workPerThread << endl;
    }
    if (strArgs[0] == HM_CMD_GETRECYCLE)
    {
        bool recycle = 0;
        recv<bool>(s, recycle, strArgs[0]);
        string status = recycle? "Enabled": "Disabled";
        cout << "Recycle Threads Status = " << status<< endl;
    }
    closeSocket(s);
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

