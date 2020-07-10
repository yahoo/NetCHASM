// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HMCONNECTIONHANDLER_H_
#define INCLUDE_HMCONNECTIONHANDLER_H_
#include <openssl/ssl.h>
#include <mutex>
#include <shared_mutex>

#include "HMConstants.h"
#include "HMSocketUtilBase.h"
#include "HMIPAddress.h"
class HMDataCheckResult;
class HMDataCheckParams;
class HMAPIDataHostCheck;
class HMConnectionCheck
{
public:
    HMConnectionCheck(const std::string& socketPath, HM_REMOTE_CHECK_TYPE remoteCheckType)
            : m_port(0),
              m_remoteCheckType(remoteCheckType),
              m_socketPath(socketPath),
              m_tos(0)
    {}

    HMConnectionCheck(uint16_t port, HMIPAddress& address, HM_REMOTE_CHECK_TYPE remoteCheckType, const HMIPAddress& source, const uint8_t tos)
        : m_port(port),
          m_address(address),
          m_remoteCheckType(remoteCheckType),
          m_sourceAddress(source),
          m_tos(tos)
    {}

    bool operator<(const HMConnectionCheck& k) const;
    bool operator==(const HMConnectionCheck& k)const;
    bool operator!=(const HMConnectionCheck& k) const;

    const HMIPAddress& getAddress() const;
    uint16_t getPort() const;
    HM_REMOTE_CHECK_TYPE getRemoteCheckType() const;
    const std::string& getSocketPath() const;
    const HMIPAddress& getSourceAddress() const;
    uint8_t getTos() const;

private:
    uint16_t m_port;
    HMIPAddress m_address;
    HM_REMOTE_CHECK_TYPE m_remoteCheckType;
    std::string m_socketPath;
    HMIPAddress m_sourceAddress;
    uint8_t m_tos;
};

class HMConnectionInterface
{
public:
    HMConnectionInterface(std::shared_ptr<HMSocketUtilBase> socketAPI)
    {
        m_socketAPI = socketAPI;
    }
    std::shared_ptr<HMSocketUtilBase> m_socketAPI;
};

class HMConnectionHandler
{
public:
    HMConnectionHandler(uint8_t maxConnections, timeval& tv):
        m_tv(tv),
        m_ctx(NULL),
        m_maxConnections(maxConnections) { }

    HMConnectionHandler(SSL_CTX* ctx, uint8_t maxConnections,timeval& tv) :
        m_tv(tv),
        m_ctx(ctx),
        m_maxConnections(maxConnections){ }

    //! Get the number of connection established to a remote IP and port .
    uint64_t getConnectionSize(const HMConnectionCheck& check);
    //! Get the total number of remote connection established.
    uint64_t getConnectionSize();
    //! Called to get a existing remote persistent connections or create a new connection.
    bool getConnection(const HMConnectionCheck& check, std::shared_ptr<HMSocketUtilBase>& socketAPI);

private:
    timeval m_tv;
    SSL_CTX* m_ctx = NULL;
    std::shared_timed_mutex m_mutex;
    //! Called to create a remote persistent connections.
    std::shared_ptr<HMSocketUtilBase> createConnection(const HMConnectionCheck& check);
    //! Called to get a existing remote connections.
    bool findConnection(const HMConnectionCheck& check, std::shared_ptr<HMSocketUtilBase>& socketAPI);
    std::multimap<HMConnectionCheck, HMConnectionInterface> m_connections;
    uint8_t m_maxConnections;
};



#endif /* INCLUDE_HMCONNECTIONHANDLER_H_ */
