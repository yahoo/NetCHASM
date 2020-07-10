// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSOCKETUTIL_TCP_H_
#define HMSOCKETUTIL_TCP_H_

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

//! Class to help tcp socket communication.
class HMSocketUtilTCP : public HMSocketUtilBase
{
public:

    HMSocketUtilTCP(const HMIPAddress& address, uint16_t port, timeval &timeInfo, const HMIPAddress& sourceAddress, const uint8_t tosValue, bool persistant) :
            HMSocketUtilBase(false, -1, persistant),
            m_connectTimeInfo(timeInfo),
            m_address(address),
            m_sourceAddress(sourceAddress),
            m_port(port),
            m_tos(tosValue),
            m_clientPort(-1)
    { }

    HMSocketUtilTCP(int sock) :
            HMSocketUtilBase(true, sock, false),
            m_port(0),
            m_tos(0),
            m_clientPort(-1)
    { }

    virtual ~HMSocketUtilTCP();
    HMSocketUtilTCP& operator=(const HMSocketUtilTCP&) = delete;        // Disallow copying
    HMSocketUtilTCP(const HMSocketUtilTCP&) = delete;

    /*!
         Called to receive checkinfo.
         \param string to receive.
         \param size of string.
         \param wait time for the data.
     */
    bool getCheckInfo(std::string& checkinfo, uint32_t size, timeval& tv);

    //! Called to get the close a socket.
    virtual void closeSocket();

protected:
    timeval m_connectTimeInfo;
    //! Called to get the connect to a socket.
    virtual bool connectSocket();

private:
    HMSocketUtilTCP();
    HMIPAddress m_address;
    HMIPAddress m_sourceAddress;
    uint16_t m_port;
    uint8_t m_tos;
    int m_clientPort;
    //! Called to get the create a socket.
    bool createSocket();
    //! Called to reset the connection.
    virtual void reconnect();
    /*!
         Called to send data across the socket.
         \param data buffer.
         \param size of the data buffer.
     */
    virtual bool sendData(const char* buffer, uint64_t size);
    /*!
         Called to receive data across the socket.
         \param data buffer.
         \param size of the data buffer.
         \param wait time for the data.
         \return Status of results fetch.
     */
    virtual HM_SOCK_DATA_STATUS recvData(char* data, uint64_t size, timeval tv);

};

#endif /* HMSOCKETUTIL_TCP_H_ */
