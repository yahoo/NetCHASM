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
    cout << "Usage: "<< name<<" [options] ..." << endl << "Options:" << endl
            << "-s      <socket-path> [default: " << HM_DEFAULT_USD_PATH << "]" << endl
            << "-m      <master-config path> " << endl;
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


bool reloadConfig(int s, string& cmd)
{
    int result;
    if (!sendMessage(s, cmd))
    {
        return false;
    }
    read(s, &result, sizeof(result));
    if (result)
    {
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    int sock;
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
            if (optopt == 'c' || optopt == 'm')
                fprintf(stderr, "Option -%c requires an argument\n", optopt);
            else
                fprintf(stderr, "Unknown option:-%c\n", optopt);
            return 1;
        }

    }

    string command = HM_CMD_RELOAD;
    if (isDifMaster) {
        command += " ";
        command += master_path;
    }
    if(!createSocket(sock))
    {
        return -1;
    }
    if(!connectSocket(sock, server_path))
    {
        return -1;
    }
    if(!reloadConfig(sock, command))
    {
        return -1;
    }
    closeSocket(sock);
    return 0;
}
