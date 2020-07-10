// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSOCKETUTIL_BASE_H_
#define HMSOCKETUTIL_BASE_H_

#include <mutex>

#include "HMConstants.h"
#include "HMTimeStamp.h"
#include "HMIPAddress.h"

class HMDataCheckResult;
class HMAuxInfo;
class HMGroupCheckResult;
class HMGroupAuxResult;
class HMHash;
//! Class to help socket communication.
class HMSocketUtilBase
{
public:
    HMSocketUtilBase(bool connected, int sock, bool persistant) :
        m_connected(connected),
        m_socket(sock),
        m_reason(HM_REASON_NONE),
        m_connectionReset(false),
        m_persistent(persistant)
        {}
    virtual ~HMSocketUtilBase() {}
    HMSocketUtilBase& operator=(const HMSocketUtilBase&) = delete;        // Disallow copying
    HMSocketUtilBase(const HMSocketUtilBase&) = delete;


    /*!
         Called to receive command.
         \param string to receive.
         \param wait time for the data.
     */
    HM_SOCK_DATA_STATUS receiveCommand(std::string& cmd, timeval& tv);


    /*!
         Called to send command.
         \param string to send.
     */
    bool sendCommand(const std::string& cmd);


    /*!
         Called to send a response message.
         \param data to send.
         \param size of data.
     */
    bool sendMessage(const char* data, uint64_t size);

    /*!
         Called to receive data over socket. wait time is 3 seconds
         \param string to receive.
         \param size of string.
     */
    HM_SOCK_DATA_STATUS receiveMessage(char* data, uint64_t size);

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
         \return Status of results fetch.
     */
    HM_SOCK_DATA_STATUS getHostResults(std::string& hostName,
            HMIPAddress& address,
            HMDataHostCheck& dataHostCheck,
            timeval &tv,
            std::map<HMDataCheckParams, HMDataCheckResult>& hostResults);

    /*!
         Called to receive Host results from remote host.
         \param name of the host.
         \param datahost check of the host.
         \param timeout period
         \param Datastructure to store results
         \return Status of results fetch.
     */
    HM_SOCK_DATA_STATUS getHostResults(std::string& hostName,
            HMDataHostCheck& dataHostCheck, timeval &tv,
            std::multimap<HMDataCheckParams, HMDataCheckResult>& hostResults);

    /*!
         Called to receive Host group results from remote host specified in check.
         \param name of the hostgroup.
         \param timeout period
         \param hash value expected
         \param Datastructure to store results
         \return Status of results fetch.
     */
    HM_SOCK_DATA_STATUS getHostGroupResults(std::string& hostGroupName,
            timeval &tv, const HMHash& hash,
            std::vector<HMGroupCheckResult>& results);

    /*!
         Called to receive Load Feedback results from remote host.
         \param name of the host.
         \param address of the remote host.
         \param datahost check of the host.
         \param timeout period
         \param Datastructure to store results
         \return Status of results fetch.
    */
    HM_SOCK_DATA_STATUS getLoadFeedback(const std::string& hostName,
            HMIPAddress& address,
            HMDataHostCheck& dataHostCheck,
            timeval &tv,
            HMAuxInfo& auxData);

    /*!
         Called to receive Load Feedback results from remote host.
         \param name of the host.
         \param datahost check of the host.
         \param timeout period
         \param Datastructure to store results
         \return Status of results fetch.
    */
    HM_SOCK_DATA_STATUS getLoadFeedback(const std::string& hostName,
            HMDataHostCheck& dataHostCheck, timeval &tv,
            std::vector<HMGroupAuxResult>& auxResults);

    /*!
         Called to receive Load Feedback results for hostgroup from remote host specified in the check.
         \param name of the hostgroup.
         \param timeout period
         \param hash value expected
         \param Datastructure to store results
         \return Status of results fetch.
    */
    HM_SOCK_DATA_STATUS getLoadFeedback(std::string& hostGroupName, timeval &tv,
            const HMHash& hash, std::vector<HMGroupAuxResult>& auxResults);

    //! Called to get the reason of socket connection failure.
    HM_REASON getReason() const;
    //! Called to get the error message.
    const std::string& getErrorMsg() const;
    //! Called to get the connect time.
    const HMTimeStamp& getConnectTime() const;

    //! Called to check if the connection needs to be reset.
    bool isConnectionReset() const;

    //! Called to set the connection-reset flag.
    void setConnectionReset(bool connectionReset);

    //! Called to check if the connection is a persistent.
    bool isPersistent() const;

    //! Called to connect to a server.
    void connectServer();

    //!Convert string to unsigned long long number.
    bool strtoull(const std::string& sNumber, uint64_t& number);

    //!Convert string to unsigned long number.
    bool strtoul(const std::string& sNumber, uint32_t& number);

protected:
    bool m_connected;
    int m_socket;
    HM_REASON m_reason;
    std::string m_errorMsg;
    HMTimeStamp m_connectTime;
    std::mutex m_mutex;

    //! Called to reset the connection.
    virtual void reconnect() = 0;
    //! Called to close the socket.
    virtual void closeSocket() = 0;
    bool openPersistant();

private:
    /*!
         Called to send data across the socket.
         \param data buffer.
         \param size of the data buffer.
     */
    virtual bool sendData(const char* buffer, uint64_t size) = 0;
    /*!
         Called to receive data across the socket.
         \param data buffer.
         \param size of the data buffer.
         \param wait time for the data.
         \return Status of results fetch.
     */
    virtual HM_SOCK_DATA_STATUS recvData(char* data, uint64_t size, timeval tv) = 0;

    //! variable to indicate the connections needs to be reset next time.
    bool m_connectionReset;
    //! variable to indicate if the connections needs to be a persistent connections
    bool m_persistent;
};

#endif /* HMSOCKETUTIL_BASE_H_ */
