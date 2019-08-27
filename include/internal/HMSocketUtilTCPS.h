// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSOCKETUTIL_TCPS_H_
#define HMSOCKETUTIL_TCPS_H_

#include <inttypes.h>
#include <string>
#include <thread>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "HMConstants.h"
#include "HMTimeStamp.h"
#include "HMIPAddress.h"
#include "HMControlBase.h"
#include "HMSocketUtilTCP.h"

//! Class to help tls socket communication.
class HMSocketUtilTCPS : public HMSocketUtilTCP
{
public:

    HMSocketUtilTCPS(SSL_CTX* ctx, HMIPAddress& address, uint16_t port, timeval &timeInfo, const HMIPAddress& sourceAddress, const uint8_t tosValue);
    HMSocketUtilTCPS(SSL* ssl);
    virtual ~HMSocketUtilTCPS();

    HMSocketUtilTCPS& operator=(const HMSocketUtilTCPS&) = delete;        // Disallow copying
    HMSocketUtilTCPS(const HMSocketUtilTCPS&) = delete;

private:
    HMSocketUtilTCPS();
    SSL_CTX* m_ctx;
    SSL* m_ssl;
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
     */
    bool recvData(char* data, uint64_t size, timeval& tv);
    //! Called to connect to a socket.
    bool connectSocket();
    //! Called to close the socket.
    void closeSocket();
};

#endif /* HMSOCKETUTIL_TCPS_H_ */
