// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCOMMANDLISTENERCLIENTTCP_H_
#define HMCOMMANDLISTENERCLIENTTCP_H_

#include <inttypes.h>
#include <string>
#include <thread>
#include <iostream>
#include <vector>

#include "HMControlSocketClientBase.h"
//! Client class to support tcp socket communications
class HMControlTCPSocketClient : public HMControlSocketClientBase
{
public:

    HMControlTCPSocketClient(HMAPIIPAddress& address, uint16_t port);
    virtual ~HMControlTCPSocketClient();

    HMControlTCPSocketClient& operator=(const HMControlTCPSocketClient&) = delete;        // Disallow copying
    HMControlTCPSocketClient(const HMControlTCPSocketClient&) = delete;

private:
    HMControlTCPSocketClient();
    int m_socket;
    HMAPIIPAddress m_address;
    uint16_t m_port;
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
