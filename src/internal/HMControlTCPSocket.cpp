// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <future>
#include <arpa/inet.h>
#include <fcntl.h>
#include <HMControlTCPSocket.h>
#include <netinet/in.h>
#include "HMLogBase.h"
#include "HMStateManager.h"

using namespace std;

HMControlTCPSocket::HMControlTCPSocket(bool ipv6, HMStateManager &stateManager) :
        HMCommandListenerBase(stateManager),
        m_socket(-1),
        m_ipv6(ipv6){}

HMControlTCPSocket::~HMControlTCPSocket() {}

void
HMControlTCPSocket::init()
{
    HMCommandListenerBase::init();
    struct sockaddr_storage server;
    uint8_t type = AF_INET;
    if(m_ipv6)
    {
        type = AF_INET6;
    }
    m_socket = socket(type, SOCK_STREAM, IPPROTO_TCP);
    if(m_socket < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to create socket TCP error desc: ");
        //LCOV_EXCL_STOP;
    }

    if (fcntl(m_socket, F_SETFL, O_NONBLOCK) < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to set non blocking socket:");
        //LCOV_EXCL_STOP;
    }

    int tmp = 1;
    if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*) &tmp, sizeof tmp) < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to setsockopt REUSEADDR for TCP socket error desc: ");
        //LCOV_EXCL_STOP;
    }
    shared_ptr<HMState> current;
    m_stateManager.updateState(current);
    uint16_t portv4 = current->getControlSocketCheckPortv4();
    uint16_t portv6 = current->getControlSocketCheckPortv6();
    current.reset();
    if(m_ipv6)
    {
        sockaddr_in6* addr = (sockaddr_in6*)&server;
        addr->sin6_family = AF_INET6;
        addr->sin6_addr = in6addr_any;
        addr->sin6_port = htons(portv6);
    }
    else
    {
        sockaddr_in* addr = (sockaddr_in*) &server;
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = INADDR_ANY;
        addr->sin_port = htons(portv4);

    }
    if(bind(m_socket, (struct sockaddr *) &server, sizeof(server)) < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to bind TCP socket error desc: ");
        //LCOV_EXCL_STOP;
    }
    if (listen(m_socket, SOMAXCONN) < 0)
    {
        throwException("Failed to listen TCP socket error desc: ");
    }
}

void
HMControlTCPSocket::run()
{
    m_mainThread = thread(&HMControlTCPSocket::handleConnections,this);
}

void
HMControlTCPSocket::listernerShutDown()
{
    if(m_socket != -1)
    {
        close(m_socket);
    }
    if(write(m_pipesfd[1], "test",5) < 0)
    {
        HMLog(HM_LOG_ERROR, "[CORE] failed writing to shutdown listener"); //LCOV_EXCL_LINE
    }
    m_mainThread.join();
}

void
HMControlTCPSocket::handleClient(int clientSock)
{
    if(clientSock == -1)
    {
        //LCOV_EXCL_START
        string strBuf = "Failed to accept on socket, error desc: ";
        error_code ec(errno, generic_category());
        throw system_error(ec, strBuf);
        //LCOV_EXCL_STOP
    }
    HMSocketUtilTCP utilTcp(clientSock);
    bool keepOpen = false;
    bool done;
    do
    {
        done = false;
        struct timeval timeToWait;
        timeToWait.tv_sec = 3;
        timeToWait.tv_usec = 0;
        string command;
        HM_SOCK_DATA_STATUS status = utilTcp.receiveCommand(command, timeToWait);
        switch(status)
        {
        case HM_SOCK_DATA_EMPTY:
        case HM_SOCK_DATA_FAILED:
            keepOpen = false;
            done = true;
            break;
        case HM_SOCK_DATA_TIMEOUT:
            done = true;
            break;
        case HM_SOCK_DATA_OK:
            HMLog(HM_LOG_DEBUG, "[TCPSERVER] NetCHASM daemon socket received command = %s",
                    command.c_str());
            if (command == "quit")
            {
                keepOpen = false;
                done = true;
            }
            else if (command == HM_CMD_KEEPOPENSTR)
            {
                keepOpen = true;
            }
            else
            {
                handleCommands(command, utilTcp);
            }
        }
    } while (m_keepRunning && (keepOpen || !done));
    utilTcp.closeSocket();
    lock_guard<shared_timed_mutex> lg(m_handlerMutex);
    m_handlerThreadsStatus[this_thread::get_id()] = true;
    return;
}

void HMControlTCPSocket::handleConnections()
{
    m_keepRunning = true;
    fd_set fds;
    while (m_keepRunning)
    {
        FD_ZERO (&fds);
        FD_SET(m_socket, &fds);
        FD_SET(m_pipesfd[0], &fds);
        int max = m_socket>m_pipesfd[0]?m_socket:m_pipesfd[0];
        timeval tv { 3, 0};
        int res = select(max + 1, &fds, NULL, NULL, &tv);
        if(res < 0)
        {
            //LCOV_EXCL_START
            HMLog(HM_LOG_ERROR, "[CORE] HMCommandListener::Select Failed linux Socket");
            //LCOV_EXCL_STOP
        }
        if(FD_ISSET(m_socket, &fds))
        {
            int clientSock = accept(m_socket, NULL, 0);
            if(clientSock > 0)
            {
                cleanHandlerThreads();
                lock_guard<shared_timed_mutex> lg(m_handlerMutex);
                m_handlerThreads.push_back(
                        thread(&HMControlTCPSocket::handleClient, this,
                                clientSock));
                HMLog(HM_LOG_DEBUG, "[CORE] HMCommandListener::run accept on client sock = %d", clientSock);
            }
        }
        if(FD_ISSET(m_pipesfd[0], &fds))
        {
            close(m_pipesfd[1]);
            close(m_pipesfd[0]);
            HMLog(HM_LOG_INFO, "[CORE] Shutting down listener");
        }
    }
}

