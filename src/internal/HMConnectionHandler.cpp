// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMConnectionHandler.h"
#include "HMSocketUtilTCP.h"
#include "HMSocketUtilTCPS.h"
#include "HMSocketUtilLinux.h"
using namespace std;

bool
HMConnectionCheck::operator <(const HMConnectionCheck& k) const
{
    if(m_socketPath == k.m_socketPath)
    {
        if(m_port == k.m_port)
        {
            if(m_address == k.m_address)
            {
                if(m_remoteCheckType == k.m_remoteCheckType)
                {
                    if(m_sourceAddress == k.m_sourceAddress)
                    {
                        return m_tos < k.m_tos;
                    }
                    return m_sourceAddress < k.m_sourceAddress;
                }
                return m_remoteCheckType < k.m_remoteCheckType;
            }
            return m_address < k.m_address;
        }
        return m_port < k.m_port;
    }
    return m_socketPath < k.m_socketPath;
}

bool
HMConnectionCheck::operator ==(const HMConnectionCheck& k) const
{
    if(m_socketPath == k.m_socketPath
            && m_port == k.m_port
            && m_address == k.m_address
            && m_remoteCheckType == k.m_remoteCheckType
            && m_sourceAddress == k.m_sourceAddress
            && m_tos == k.m_tos)
    {
        return true;
    }
    return false;
}

bool
HMConnectionCheck::operator !=(const HMConnectionCheck& k) const
{
    return !(*this == k);
}

const
HMIPAddress& HMConnectionCheck::getAddress() const
{
    return m_address;
}

uint16_t
HMConnectionCheck::getPort() const
{
    return m_port;
}

HM_REMOTE_CHECK_TYPE
HMConnectionCheck::getRemoteCheckType() const
{
    return m_remoteCheckType;
}

const string&
HMConnectionCheck::getSocketPath() const
{
    return m_socketPath;
}

shared_ptr<HMSocketUtilBase>
HMConnectionHandler::createConnection(const HMConnectionCheck& check)
{
    shared_ptr<HMSocketUtilBase> conn;
    if(check.getRemoteCheckType() == HM_REMOTE_SHARED_CHECK_TCP)
    {
        conn = make_shared<HMSocketUtilTCP>(check.getAddress(), check.getPort(), m_tv, check.getSourceAddress(), check.getTos(), true);
        conn->connectServer();
    }
    else if(check.getRemoteCheckType() == HM_REMOTE_SHARED_CHECK_TCPS)
    {
        if (m_ctx != NULL)
        {
            conn = make_shared<HMSocketUtilTCPS>(m_ctx, check.getAddress(),
                    check.getPort(), m_tv, check.getSourceAddress(), check.getTos(), true);
            conn->connectServer();
        }
    }
    else if(check.getRemoteCheckType() == HM_REMOTE_SHARED_CHECK_LINUX)
    {
        conn = make_shared<HMSocketUtilLinux>(check.getSocketPath(), true);
        conn->connectServer();
    }
    else
    {
        HMLog(HM_LOG_ERROR, "Invalid shared remote check type, remote check type:", printRemoteCheckType(check.getRemoteCheckType()));

    }
    return std::move(conn);
}

uint64_t
HMConnectionHandler::getConnectionSize(const HMConnectionCheck& check)
{
    return m_connections.count(check);
}

uint64_t
HMConnectionHandler::getConnectionSize()
{
    return m_connections.size();
}

bool
HMConnectionHandler::findConnection(const HMConnectionCheck& check, shared_ptr<HMSocketUtilBase>& socketAPI)
{
    auto ret = m_connections.equal_range(check);
    // Assign to a random connection.
    std::srand(std::time(nullptr)); // use current time as seed for random generator
    uint8_t random_value = std::rand() % m_maxConnections;
    for (auto it = ret.first; it != ret.second; ++it, --random_value)
    {
        if (random_value == 0)
        {
            socketAPI = it->second.m_socketAPI;
            return true;
        }
    }
    return false;
}

bool
HMConnectionHandler::getConnection(const HMConnectionCheck& check, shared_ptr<HMSocketUtilBase>& socketAPI)
{
    if (m_connections.count(check) >= m_maxConnections)
    {
        shared_lock<std::shared_timed_mutex> lock(m_mutex);
        return findConnection(check, socketAPI);
    }
    else
    {
        unique_lock<std::shared_timed_mutex> lock(m_mutex);
        if (m_connections.count(check) >= m_maxConnections)
        {
            return findConnection(check, socketAPI);
        }

        auto it = m_connections.emplace(check,
                HMConnectionInterface(createConnection(check)));
        socketAPI = it->second.m_socketAPI;
        return true;
    }
    return false;
}

const HMIPAddress& HMConnectionCheck::getSourceAddress() const
{
    return m_sourceAddress;
}

uint8_t HMConnectionCheck::getTos() const
{
    return m_tos;
}
