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
static int nargs;

static void usage(char* name)
{
    cout << "Usage: "<< name <<" [options] rotation host/ip 0/1[force_down off/force_down on]" << endl << "Options:" << endl
            << "-s      <socket-path> [default: " << HM_DEFAULT_USD_PATH << "]" << endl;
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


int main(int argc, char**argv)
{
    int sd = -1;
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

    nargs = argc - optind;

    if (nargs == 0)
    {
        cout << "Insufficient options - rotation hostname 0/1[force_down off/force_down on]";
    }
    else
    {
        int arg = optind;
        while (nargs && arg < argc)
        {

            command = command + " " + (string(argv[arg]));
            arg++;
        }
    }

    do
    {
        if(!createSocket(sd))
        {
            return -1;
        }


        if(!connectSocket(sd,server_path))
        {
            return -1;
        }
        
        if(!sendMessage(sd, command))
        {
            return -1;
        }
    } while (false);

    closeSocket(sd);
    return 0;
}
