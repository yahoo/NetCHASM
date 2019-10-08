// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "HMConstants.h"
#include "HMControlLinuxSocket.h"

using namespace std;

static void usage(char* name)
{
    string command = "man ";
    command.append(name);
    {
        cout << "Usage: "<< name<<" [options] ..." << endl << "Options:" << endl
                << "-s      <socket-path> [default: " << HM_DEFAULT_USD_PATH << "]" << endl
                << "-m      <master-config path> " << endl;
    }
}

void printError(string msg)
{
    cerr << msg << ":" << strerror(errno) << endl;
}

int main(int argc, char *argv[])
{
    int opt;
    string server_path = HM_DEFAULT_USD_PATH;
    string master_path;
    bool isDifMaster = false;
    while ((opt = getopt(argc, argv, "s:m:h")) != -1) {
        switch (opt) {
        case 's':
            server_path = std::string(optarg);
            break;
        case 'm':
            isDifMaster = true;
            master_path = optarg;
            break;
        case 'h':
            usage(argv[0]);
            exit(0);
        case '?':
            usage(argv[0]);
            exit(0);
        }

    }
    bool status;
    HMControlLinuxSocketClient socketAPI(server_path);
    if (isDifMaster)
    {
        status = socketAPI.reload(master_path);
    }
    else
    {
        status = socketAPI.reload();
    }
    return status ? 0 : -1;
}
