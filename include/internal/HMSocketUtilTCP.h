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

    HMSocketUtilTCP(HMIPAddress& address, uint16_t port, timeval &timeInfo, const HMIPAddress& sourceAddress, const uint8_t tosValue);
    HMSocketUtilTCP(int sock);
    virtual ~HMSocketUtilTCP();

    HMSocketUtilTCP& operator=(const HMSocketUtilTCP&) = delete;        // Disallow copying
    HMSocketUtilTCP(const HMSocketUtilTCP&) = delete;

    //! Called to get the reason of socket connection failure.
    HM_REASON getReason() const;
    //! Called to get the error message.
    const std::string& getErrorMsg() const;
    //! Called to get the connect time.
    const HMTimeStamp& getConnectTime() const;

    /*!
         Called to receive checkinfo.
         \param string to receive.
         \param size of string.
         \param wait time for the data.
     */
    bool getCheckInfo(std::string& checkinfo, uint32_t size, timeval& tv);

    /*!
     Ping the remote host for status.
     \param wait time for the data.
     \returns true on success
     */
    bool pingRemoteHost(timeval& tv);
    /*!
         Called to receive Host results from remote host.
         \param name of the host.
         \param address of the remote host.
         \param datahost check of the host.
         \param timeout period
         \param Datastructure to store results
     */
    HM_SOCK_DATA_STATUS getHostResults(std::string& hostName, HMIPAddress& address, HMDataHostCheck& dataHostCheck,
            timeval &tv,
            std::map<HMDataCheckParams, HMDataCheckResult>& hostResults);

    /*!
         Called to receive Load Feedback results from remote host.
         \param name of the host.
         \param address of the remote host.
         \param datahost check of the host.
         \param timeout period
         \param Datastructure to store results
    */
    HM_SOCK_DATA_STATUS getLoadFeedback(std::string& hostName, HMIPAddress& address,
            HMDataHostCheck& dataHostCheck, timeval &tv, HMAuxInfo& auxData);

protected:
    HM_REASON m_reason;
    std::string m_errorMsg;
    timeval m_connectTimeInfo;
    HMTimeStamp m_connectTime;
    //! Called to get the connect to a socket.
    virtual bool connectSocket();

protected:
    //! Called to get the close a socket.
    virtual void closeSocket();

private:
    HMSocketUtilTCP();
    HMIPAddress m_address;
    HMIPAddress m_sourceAddress;
    uint16_t m_port;
    uint8_t m_tos;
    //! Called to get the create a socket.
    bool createSocket();
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
     */
    virtual bool recvData(char* data, uint64_t size, timeval& tv);
};

#endif /* HMSOCKETUTIL_TCP_H_ */
