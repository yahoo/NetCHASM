// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCOMMANDTCPLISTENER_H_
#define HMCOMMANDTCPLISTENER_H_

#include <inttypes.h>
#include <string>
#include <thread>

#include "HMControlBase.h"
#include "HMConstants.h"

//! Default location of the Linux Control Socket
const std::string HM_DEFAULT_USD_PATH = "/home/y/var/run/netchasm/controlsocket";

class HMStateManager;
//! Class to handle linux socket based external communications
class HMControlLinuxSocket : public HMCommandListenerBase
{
public:

    HMControlLinuxSocket(std::string socketPath, HMStateManager& stateManager);
    ~HMControlLinuxSocket();

    HMControlLinuxSocket& operator=(const HMControlLinuxSocket&) = delete;        // Disallow copying
    HMControlLinuxSocket(const HMControlLinuxSocket&) = delete;

    //!  Function to initialize sockets
    void init();
    //!  Function to handle connections
    void run();
    //! Shutdown listeners
    void listernerShutDown();
    /*!
         Handle client communications
         \param pointer of client socket.
     */
    void handleClient(char* client);
    //! Handle client connections
    void handleConnections();

private:
    int m_socket;
    std::string m_socketPath;
    //! unlink socket path
    void unlinkSocket(std::string& socketPath);
};

#endif /* HMCOMMANDTCPLISTENER_H_ */
