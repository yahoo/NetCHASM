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

    HMSocketUtilLinux(int sock, std::string& sockPath);
    virtual ~HMSocketUtilLinux();

    HMSocketUtilLinux& operator=(const HMSocketUtilLinux&) = delete;        // Disallow copying
    HMSocketUtilLinux(const HMSocketUtilLinux&) = delete;


protected:
    std::string m_socketPath;

private:
    HMSocketUtilLinux();
    //! Called to close the socket.
    void closeSocket();
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
};

#endif /* HMSOCKETUTIL_LINUX_H_ */
