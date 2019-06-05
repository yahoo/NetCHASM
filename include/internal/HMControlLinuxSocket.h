// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCONTROLLINUXSOCKET_H_
#define HMCONTROLLINUXSOCKET_H_

#include <inttypes.h>
#include <string>
#include <thread>

#include "HMControlBase.h"
#include "HMConstants.h"

//! Default location of the Linux Control Socket
const std::string HM_DEFAULT_USD_PATH = "/home/y/var/run/netchasm/controlsocket";

class HMStateManager;
class HMControlLinuxSocket : public HMCommandListenerBase
{
public:

    HMControlLinuxSocket(std::string socketPath, HMStateManager& stateManager);
    ~HMControlLinuxSocket();

    HMControlLinuxSocket& operator=(const HMControlLinuxSocket&) = delete;        // Disallow copying
    HMControlLinuxSocket(const HMControlLinuxSocket&) = delete;

    void init();
    void run();
    void listernerShutDown();
    void handleClient(int clientSock);
    void handleConnections();

    void responseMessage(int clinetSock , const void* buffer, size_t size);
    void responseMessageVarLen(int clinetSock, const char* buffer, size_t size);

private:
    int m_socket;
};

#endif /* HMCONTROLLINUXSOCKET_H_ */
