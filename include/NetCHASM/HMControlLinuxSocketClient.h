// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCOMMANDLISTENERCLIENT_H_
#define HMCOMMANDLISTENERCLIENT_H_

#include <inttypes.h>
#include <string>
#include <thread>
#include <iostream>
#include <vector>

#include "HMControlSocketClientBase.h"
//! Client class to support linux socket communications
class HMControlLinuxSocketClient : public HMControlSocketClientBase
{
    const std::string HM_DEFAULT_USD_PATH = "/home/y/var/run/healthmon/controlsocket";
public:

    HMControlLinuxSocketClient() : HMControlLinuxSocketClient(HM_DEFAULT_USD_PATH) {}
    HMControlLinuxSocketClient(const std::string& socketPath);
    virtual ~HMControlLinuxSocketClient();

    HMControlLinuxSocketClient& operator=(const HMControlLinuxSocketClient&) = delete;        // Disallow copying
    HMControlLinuxSocketClient(const HMControlLinuxSocketClient&) = delete;

private:
    int m_socket;
    std::string m_server_path;
    /*!
         Send command over the socket.
         \param command to send.
         \return true if successful.
     */
    bool sendMessage(const std::string& cmd);
    /*!
          Send data over the socket.
          \param data to send.
          \param length of data.
          \return true if successful.
    */
    bool sendData(const char* buffer, uint32_t size);
    /*!
          Receive data over the socket.
          \param data to receive.
          \param length of data to receive.
          \return true if successful.
    */
    bool recvMessage(char* data, uint64_t size);
    //! Creates a socket
    bool createSocket();
    // closes a socket
    void closeSocket();
    // connects a socket
    bool connectSocket();
};

#endif /* HMCOMMANDLISTENERCLIENT_H_ */
