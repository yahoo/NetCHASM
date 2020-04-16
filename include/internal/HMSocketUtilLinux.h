// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSOCKETUTIL_LINUX_H_
#define HMSOCKETUTIL_LINUX_H_

#include <inttypes.h>
#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <sys/time.h>

#include "HMConstants.h"
#include "HMTimeStamp.h"
#include "HMIPAddress.h"
#include "HMControlBase.h"
//! Class to help linux socket communication.
class HMSocketUtilLinux : public HMSocketUtilBase
{
public:

    HMSocketUtilLinux(const std::string& socketPath, const bool persistant) :
            HMSocketUtilBase(false, -1, persistant),
            m_socketPath(socketPath)
    { }

    HMSocketUtilLinux(int sock, std::string& sockPath) :
            HMSocketUtilBase(true, sock, false),
            m_socketPath(sockPath)
    { }

    virtual ~HMSocketUtilLinux();
    HMSocketUtilLinux& operator=(const HMSocketUtilLinux&) = delete;        // Disallow copying
    HMSocketUtilLinux(const HMSocketUtilLinux&) = delete;
    //! Called to close the socket.
    void closeSocket();
    //! Called to connect to a server.
    void connectServer();

protected:
    std::string m_socketPath;
    //! Called to get the connect to a socket.
    virtual bool connectSocket();

private:
    HMSocketUtilLinux();
    //! Called to get the create a socket.
    bool createSocket();
    //! Called to reset the connection.
    void reconnect();
    /*!
         Called to send data across the socket.
         \param data buffer.
         \param size of the data buffer.
     */
    bool sendData(const char* buffer, uint64_t size);
    /*!
         Called to receive data across the socket.
         \param data buffer.
         \param size of the data buffer.
         \param wait time for the data.
         \return Status of results fetch.
     */
    HM_SOCK_DATA_STATUS recvData(char* data, uint64_t size, timeval tv);
};

#endif /* HMSOCKETUTIL_LINUX_H_ */
