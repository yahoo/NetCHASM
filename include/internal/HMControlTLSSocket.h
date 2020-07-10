// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCOMMANDLISTENERTLS_H_
#define HMCOMMANDLISTENERTLS_H_

#include <inttypes.h>
#include <string>
#include <thread>

#include "HMControlBase.h"
#include "HMConstants.h"


class HMStateManager;
//! Class to handle tls socket based external communications
class HMControlTLSSocket : public HMCommandListenerBase
{
public:

	HMControlTLSSocket(bool ipv6, HMStateManager& stateManager);
    ~HMControlTLSSocket();

    HMControlTLSSocket& operator=(const HMControlTLSSocket&) = delete;        // Disallow copying
    HMControlTLSSocket(const HMControlTLSSocket&) = delete;

    //!  Function to initialize sockets
    void init();
    //!  Function to handle connections
    void run();
    //! Shutdown listeners
    void listernerShutDown();
    /*!
          Handle client communications
          \param Initialized SSL* object.
     */
    void handleClient(SSL* ssl);
    //! Handle client connections
    void handleConnections();
    /*!
          Handle secure accept
          \param expiry time to wait for the connection to be established.
          \param connection socket descriptor
          \param client port for debug info
     */
    void secureAccept(HMTimeStamp expiry, int clientSock, int clientport);


private:
    int m_socket;
    bool m_ipv6;
};

#endif /* HMCOMMANDLISTENERTLS_H_ */
