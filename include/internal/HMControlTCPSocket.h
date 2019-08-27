// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCOMMANDLISTENERTCP_H_
#define HMCOMMANDLISTENERTCP_H_

#include <inttypes.h>
#include <string>
#include <thread>

#include "HMControlBase.h"
#include "HMConstants.h"
#include "HMSocketUtilTCP.h"

class HMStateManager;
//! Class to handle tcp socket based external communications
class HMControlTCPSocket : public HMCommandListenerBase
{
public:

    HMControlTCPSocket(bool ipv6, HMStateManager& stateManager);
    ~HMControlTCPSocket();

    HMControlTCPSocket& operator=(const HMControlTCPSocket&) = delete;        // Disallow copying
    HMControlTCPSocket(const HMControlTCPSocket&) = delete;

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
    bool m_ipv6;
};

#endif /* HMCOMMANDLISTENER_H_ */
