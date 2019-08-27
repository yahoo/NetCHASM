// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <vector>

#include "HMStorage.h"
#include "HMAuxCache.h"
#include "HMLogBase.h"

using namespace std;

bool
HMCheckHeader::operator<(const HMCheckHeader& k) const
{
    if(m_hostname != k.m_hostname)
    {
        return m_hostname < k.m_hostname;
    }
    if(m_address != k.m_address)
    {
        return m_address < k.m_address;
    }
    if(m_hostCheck != k.m_hostCheck)
    {
        return m_hostCheck < k.m_hostCheck;
    }
    return m_checkParams < k.m_checkParams;
}

uint32_t
HMConfigInfo::serialize(char* buf, uint32_t size) const
{
    if(buf == nullptr || size < sizeof(SerStruct))
    {
        return sizeof(SerStruct);
    }

    SerStruct* ptr = (SerStruct*)buf;

    ptr->m_version = m_version;
    ptr->m_configStatus = m_configStatus;
    ptr->m_configLoadTime = m_configLoadTime.getTimeSinceEpoch();
    ptr->m_hashSize = m_hash.m_hashSize;
    memcpy(ptr->md_value, m_hash.m_hashValue, HASH_MAX_SIZE);
    return sizeof(SerStruct);
}

bool
HMConfigInfo::deserialize(char* buf, uint32_t size)
{
    if(buf == nullptr || size < sizeof(SerStruct))
    {
        return false;
    }

    SerStruct* ptr = (SerStruct*)buf;

    m_version = ptr->m_version;
    m_configStatus = HM_CONFIG_STATUS(ptr->m_configStatus);
    m_configLoadTime.setTime(ptr->m_configLoadTime);
    m_hash.m_hashSize = ptr->m_hashSize;
    memcpy(m_hash.m_hashValue, ptr->md_value, HASH_MAX_SIZE);
    return true;
}
bool
HMStorage::openStore()
{
    return openStore(false);
}

bool
HMStorage::openStore(bool readonly)
{
    m_shutdown = false;
    m_readonly = readonly;
    if(!openBackend())
    {
        return false;
    }
    if(!validateDBVersion())
    {
        return false;
    }
    if(!readonly)
    {
        m_thread = thread(&HMStorage::runStore, this);
    }

    return true;
}

void
HMStorage::closeStore()
{
    m_shutdown = true;
    if(!m_readonly)
    {
        {
            lock_guard<mutex> lg(m_dataReadyMutex);
            m_dataReadyCond.notify_all();
        }
        if(m_thread.joinable())
        {
            m_thread.join();
        }
    }
    closeBackend();
}

bool HMStorage::validateDBVersion()
{
    HMConfigInfo configInfo;
    if(getConfigInfo(configInfo))
    {
        if(configInfo.m_version != HM_MDBM_VERSION)
        {
             return clearBackend();
        }
    }
    return true;
}

void
HMStorage::updateAuxCommitPolicy(HM_STORAGE_COMMIT_POLICY commitPolicy)
{
    m_auxCommitPolicy = commitPolicy;
}

void
HMStorage::updateHealthCheckCommitPolicy(HM_STORAGE_COMMIT_POLICY commitPolicy)
{
    m_healthCheckCommitPolicy = commitPolicy;
}

void
HMStorage::updateLockPolicy(HM_STORAGE_LOCK_POLICY lockPolicy)
{
    m_lockPolicy = lockPolicy;
}

void
HMStorage::runStore()
{
    unique_lock<mutex> notifyLock(m_dataReadyMutex, defer_lock);
    while(!m_shutdown)
    {
        notifyLock.lock();
        m_dataReadyCond.wait(notifyLock, [this](){return commitHealthCheck() || commitAuxInfo() || m_shutdown;});
        notifyLock.unlock();
    }
    while(commitHealthCheck() || commitAuxInfo()) {}
}

bool HMHash::operator ==(const HMHash& k) const
{
    if(m_hashSize == 0 || k.m_hashSize == 0)
    {
        return true;
    }
    if(m_hashSize != k.m_hashSize)
    {
        return false;
    }
    if(memcmp(m_hashValue, k.m_hashValue, m_hashSize) != 0)
    {
        return false;
    }
    return true;
}

bool HMHash::operator !=(const HMHash& k) const
{
    return !(*this == k);
}

