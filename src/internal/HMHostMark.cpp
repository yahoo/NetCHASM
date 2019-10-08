// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <map>
#include <shared_mutex>

#include "HMHostMark.h"

using namespace std;

bool
HMHostMarkOptKey::operator <(const HMHostMarkOptKey& k) const
{
    if (m_hostName == k.m_hostName)
    {
        if (m_address == k.m_address)
        {
            return m_hostCheck < k.m_hostCheck;
        }
        return m_address < k.m_address;
    }
    return m_hostName < k.m_hostName;
}

bool
HMHostMarkOptKey::operator !=(const HMHostMarkOptKey& k) const
{
    return !(*this == k);
}

bool
HMHostMarkOptKey::operator ==(const HMHostMarkOptKey& k) const
{
    if (m_hostName == k.m_hostName && m_address == k.m_address
            && m_hostCheck == k.m_hostCheck)
    {
        return true;
    }
    return false;
}


bool HMHostMark::setSocketOption(const std::string& hostName,
        const HMIPAddress& address, const HMDataHostCheck& hostCheck, int value)
{
    lock_guard<shared_timed_mutex> wLock(m_mutex);
    HMHostMarkOptKey key(hostName, address, hostCheck);
    auto it = m_hostSockOptMap.insert(make_pair(key, value));
    if(!it.second)
    {
        it.first->second = value;
    }
    return true;
}

bool HMHostMark::getSocketOption(const std::string& hostName,
        const HMIPAddress& address, const HMDataHostCheck& hostCheck,
        int& value)
{
    shared_lock<shared_timed_mutex> rlock(m_mutex);
    const HMHostMarkOptKey key(hostName, address, hostCheck);
    auto it = m_hostSockOptMap.find(key);
    if (it != m_hostSockOptMap.end())
    {
        value = it->second;
        return true;
    }
    return false;
}

bool HMHostMark::removeSocketOption(const std::string& hostName,
        const HMIPAddress& address, const HMDataHostCheck& hostCheck)
{
    lock_guard<shared_timed_mutex> wLock(m_mutex);
    const HMHostMarkOptKey key(hostName, address, hostCheck);
    return m_hostSockOptMap.erase(key);
}
