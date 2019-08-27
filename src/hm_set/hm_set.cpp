// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "HMConstants.h"
#include "HMControlLinuxSocket.h"

using namespace std;
static void usage(char* name)
{
    cout << "Usage: "<< name <<" [options] rotation host/ip 0/1[force_down off/force_down on]" << endl << "Options:" << endl
            << "-s      <socket-path> [default: " << HM_DEFAULT_USD_PATH << "]" << endl;
}

void printError(string msg)
{
    cerr << msg << ":" << strerror(errno) << endl;
}


int main(int argc, char**argv)
{
    string command = "host_set";
    int opt;
    string server_path = HM_DEFAULT_USD_PATH;

    while ((opt = getopt(argc, argv, "s:h")) != -1)
    {
        switch (opt)
        {
            case 's':
                server_path = optarg;
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

    if (strArgs.size() < 3)
    {
        cout << "Insufficient options - rotation hostname 0/1[force_down off/force_down on]";
    }
    HMControlLinuxSocketClient socketAPI(server_path);
    if(strArgs[2] == "1")
    {
        socketAPI.setForceStatusDown(strArgs[0], strArgs[1]);
    }
    else
    {
        socketAPI.unsetForceStatusDown(strArgs[0], strArgs[1]);
    }
    return 0;
}
