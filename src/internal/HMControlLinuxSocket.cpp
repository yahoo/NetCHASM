// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <future>

#include "HMControlLinuxSocket.h"
#include "HMLogBase.h"
#include "HMStateManager.h"
#include "HMSocketUtilLinux.h"

using namespace std;

HMControlLinuxSocket::HMControlLinuxSocket(string socketPath, HMStateManager &stateManager) :
        HMCommandListenerBase(stateManager),
        m_socket(-1),
        m_socketPath(socketPath){}

HMControlLinuxSocket::~HMControlLinuxSocket() {}

void
HMControlLinuxSocket::init()
{
    HMCommandListenerBase::init();
    unlinkSocket(m_socketPath);
    struct sockaddr_un addr;
    if(m_socketPath.length() >= sizeof(addr.sun_path))
    {
        //LCOV_EXCL_START;
        string msg = "socket path " + m_socketPath + " exceeds max unix domain socket path";
        throw length_error(msg);
        //LCOV_EXCL_STOP;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;

    size_t len = m_socketPath.copy(addr.sun_path, m_socketPath.length(), 0);
    addr.sun_path[len] = '\0';
    // TODO: Should we use a SOCK_DGRAM socket, then we can't use accept,
    // listen etc.
    m_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if(m_socket < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to create socket " + m_socketPath + ", error desc: ");
        //LCOV_EXCL_STOP;
    }

    int tmp = 1;
    if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*) &tmp, sizeof tmp) < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to setsockopt REUSEADDR " + m_socketPath + ", error desc: ");
        //LCOV_EXCL_STOP;
    }

    if(bind(m_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        //LCOV_EXCL_START;
        throwException("Failed to bind socket " + m_socketPath + ", error desc: ");
        //LCOV_EXCL_STOP;
    }
}

void
HMControlLinuxSocket::run()
{
    // TODO: Move to a non blocking socket so that we can kill
    // this thread
    int rc = listen(m_socket, 10);
    if(rc == -1)
    {
        //LCOV_EXCL_START
        string strBuf = "Failed to listen on socket, error desc: ";
        error_code ec(errno, generic_category());
        throw system_error(ec, strBuf);
        //LCOV_EXCL_STOP
    }
    m_mainThread = thread(&HMControlLinuxSocket::handleConnections,this);
}

void
HMControlLinuxSocket::listernerShutDown()
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
    unlinkSocket(m_socketPath);
}

void
HMControlLinuxSocket::handleClient(int clientSock)
{
    if(clientSock == -1)
    {
        //LCOV_EXCL_START
        string strBuf = "Failed to accept on socket, error desc: ";
        error_code ec(errno, generic_category());
        throw system_error(ec, strBuf);
        //LCOV_EXCL_STOP
    }
    HMSocketUtilLinux utilLinux(clientSock, m_socketPath);
    bool keepOpen = false;
    bool done = false;
    do
    {
        done = false;
        struct timeval timeToWait;
        timeToWait.tv_sec = 3;
        timeToWait.tv_usec = 0;
        string command;
        HM_SOCK_DATA_STATUS status = utilLinux.receiveCommand(command, timeToWait);
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
            HMLog(HM_LOG_DEBUG, "[UNIXSERVER] NetCHASM daemon socket received command = %s",
                    command.c_str());

            if (command == "quit")
            {
                done = true;
                keepOpen = false;
            }
            else if (command == HM_CMD_KEEPOPENSTR)
            {
                keepOpen = true;
            }
            else
            {
                handleCommands(command, utilLinux);
            }
        }
    } while (m_keepRunning && (keepOpen || !done));
    utilLinux.closeSocket();
    lock_guard<shared_timed_mutex> lg(m_handlerMutex);
    m_handlerThreadsStatus[this_thread::get_id()] = true;
    return;
}

void HMControlLinuxSocket::handleConnections()
{
    struct sockaddr_un clientAddr;
    socklen_t len = sizeof(clientAddr);
    m_keepRunning = true;
    fd_set fds;
    while (m_keepRunning)
    {
        FD_ZERO (&fds);
        FD_SET(m_socket, &fds);
        FD_SET(m_pipesfd[0], &fds);
        int max = m_socket>m_pipesfd[1]?m_socket:m_pipesfd[0];
        int res = select(max + 1, &fds, NULL, NULL, NULL);
        if(res < 0)
        {
            //LCOV_EXCL_START
            HMLog(HM_LOG_ERROR, "[CORE] HMCommandListener::Select Failed linux Socket");
            //LCOV_EXCL_STOP
        }
        if(FD_ISSET(m_socket, &fds))
        {
            int clientSock = accept(m_socket, (struct sockaddr *) &clientAddr, &len);
            if(clientSock > 0)
            {
                cleanHandlerThreads();
                lock_guard<shared_timed_mutex> lg(m_handlerMutex);
                m_handlerThreads.push_back(
                        thread(&HMControlLinuxSocket::handleClient, this,
                               clientSock));
                HMLog(HM_LOG_DEBUG, "[CORE] HMCommandListener::run accept on client sock = %d", clientSock);
            }
        }
        if(FD_ISSET(m_pipesfd[0], &fds))
        {
            close(m_pipesfd[0]);
            close(m_pipesfd[1]);
            HMLog(HM_LOG_INFO, "[CORE] Shutting down listener");
        }
    }
}

void
HMControlLinuxSocket::unlinkSocket(string& socketPath)
{
    if(unlink(socketPath.c_str()) != 0 && errno != ENOENT)
    {
        //LCOV_EXCL_START;
        throwException("Failed to unlink socket path " + socketPath + ", error desc: ");
        //LCOV_EXCL_STOP;
    }
}
