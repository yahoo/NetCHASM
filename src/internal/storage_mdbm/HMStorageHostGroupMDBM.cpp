// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <mdbm.h>
#include <stdlib.h>
#include <cstring>
#include <climits>
#include <unistd.h>

#include "HMStorageHostGroupMDBM.h"
#include "HMAuxCache.h"
#include "HMLogBase.h"
#include "HMStorage.h"

using namespace std;

MDBMHandle::~MDBMHandle()
{
    if(m_handle != nullptr)
    {
        if(m_locked)
        {
            unlock();
        }
        mdbm_pool_release_handle(m_mdbmPool, m_handle);
    }
}

bool
MDBMHandle::init(mdbm_pool_t* pool)
{
    m_mdbmPool = pool;
    if(pool != nullptr && (m_handle  = mdbm_pool_acquire_handle(pool)) == nullptr)
    {
        //LCOV_EXCL_LINE; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] Failed to get handle from mdbm pool");
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }
    return true;
}

bool
MDBMHandle::lock(bool readOnly)
{
    uint32_t ret;
    if(m_locked)
    {
        //LCOV_EXCL_LINE; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] Failed attempting to double lock the database");
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    switch(m_lockScheme)
    {
    case MDBM_GLOBAL_LOCKING:
        ret = mdbm_lock(m_handle);
        break;
    case MDBM_RW_LOCKS:
        ret = (readOnly) ? mdbm_lock_shared(m_handle) : mdbm_lock(m_handle);
        break;
    case MDBM_PARTITIONED_LOCKS:
        ret = mdbm_plock(m_handle, &m_kv.key, 0);
        break;
    default:
        //LCOV_EXCL_LINE; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] Invalid Lock Type for MDBM");
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    if(ret != 1)
    {
        //LCOV_EXCL_START; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] mdbm_lock failed with %s", strerror(errno));
        return false;
        //LCOV_EXCL_STOP; can't be tested
    }

    m_locked = true;
    return true;
}

bool
MDBMHandle::unlock()
{
    uint32_t ret;
    if(!m_locked)
    {
        //LCOV_EXCL_LINE; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] Failed attempting to unlock the database: database not locked");
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    switch(m_lockScheme)
    {
    case MDBM_GLOBAL_LOCKING:
    case MDBM_RW_LOCKS:
        ret = mdbm_unlock(m_handle);
        break;
    case MDBM_PARTITIONED_LOCKS:
        ret = mdbm_punlock(m_handle, &m_kv.key, 0);
        break;
    default:
        //LCOV_EXCL_LINE; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] Invalid Lock Type for MDBM");
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    m_locked = false;
    return (ret == 1);
}

bool
MDBMHandle::mdbmStore()
{
    if(!lock(false))
    {
        //LCOV_EXCL_START; can't be tested
        return false;
        //LCOV_EXCL_STOP; can't be tested
    }
    if(mdbm_store(m_handle, m_kv.key, m_kv.val, MDBM_REPLACE) < 0)
    {
        //LCOV_EXCL_START; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] mdbm_store failed with %s", strerror(errno));
        return false;
        //LCOV_EXCL_STOP; can't be tested
    }
    HMLog(HM_LOG_DEBUG, "[STORE] Config Information Saved");

    return unlock();
}

bool
MDBMHandle::mdbmFetch()
{
    if (!lock(true))
    {
        //LCOV_EXCL_START; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] mdbm_lock failed");
        return false;
        //LCOV_EXCL_STOP; can't be tested
    }

    m_kv.val = mdbm_fetch(m_handle, m_kv.key);
    if (!m_kv.val.dptr)
    {
        HMLog(HM_LOG_INFO, "[STORE]: Not found: %s", m_kv.key.dptr);
        return false;
    }
    return true;
}

bool
MDBMHandle::mdbmRemove()
{
    if (!lock(false))
    {
        //LCOV_EXCL_START; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] mdbm_lock failed");
        return false;
        //LCOV_EXCL_STOP; can't be tested
    }

    mdbm_delete(m_handle, m_kv.key);
    HMLog(HM_LOG_DEBUG3, "[STORE] delete %s in mdbm", m_kv.key);

    return unlock();
}

bool
MDBMHandle::mdbmClear()
{
    if(!lock(false))
    {
        //LCOV_EXCL_START; can't be tested
        return false;
        //LCOV_EXCL_STOP; can't be tested
    }

    MDBM_ITER iter;
    m_kv = mdbm_first_r(m_handle, &iter);
    while (!(m_kv.key.dsize == 0))
    {
        mdbm_delete_r(m_handle, &iter);
        m_kv = mdbm_next_r(m_handle, &iter);
    }

    if (!unlock())
    {
        //LCOV_EXCL_START; can't be tested
        return false;
        //LCOV_EXCL_STOP; can't be tested
    }
    return true;
}

MDBMPool::~MDBMPool()
{
    if(m_mdbm)
    {
        mdbm_pool_destroy_pool(m_mdbmPool);
        mdbm_sync(m_mdbm);
        mdbm_close(m_mdbm);
        m_mdbm = nullptr;
        m_mdbmPool = nullptr;
    }
}

bool
MDBMPool::init(string& mdbmPath, bool readOnly)
{
    uint32_t flags = MDBM_OPEN_FLAGS | m_lockScheme;
    if(readOnly)
    {
        flags |= MDBM_O_RDONLY;
    }
    else
    {
        flags |= (MDBM_O_CREAT | MDBM_O_RDWR);
    }

    m_mdbm = mdbm_open(mdbmPath.c_str(), flags, 0666, 32*1024, 0);

    if (m_mdbm == nullptr)
    {
        HMLog(HM_LOG_CRITICAL, "[STORE] Failure Opening the backend data store");
        return false;
    }

    // If we are in read only mode, get the lock scheme
    if(m_lockScheme == MDBM_ANY_LOCKS)
    {
        m_lockScheme = mdbm_get_lockmode(m_mdbm);
    }

    // Again the number of threads should probably be a config parameter
    if (nullptr == (m_mdbmPool = mdbm_pool_create_pool(m_mdbm, 20)))
    {
        //LCOV_EXCL_START; can't be tested
        HMLog(HM_LOG_CRITICAL, "[STORE] Failed to create the mdbm handle pool");
        mdbm_close(m_mdbm);
        m_mdbm = nullptr;
        return false;
        //LCOV_EXCL_STOP; can't be tested
    }
    return true;
}

unique_ptr<MDBMHandle>
MDBMPool::getHandle()
{
    auto ret = make_unique<MDBMHandle>(m_lockScheme);
    return (ret->init(m_mdbmPool) ? move(ret) : nullptr);
}

bool
HMStorageHostGroupMDBM::clearBackend()
{
    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to clear backend in read only mode");
        return false;
    }

    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to clear backend when store is closed");
        return false;
    }

    HMLog(HM_LOG_DEBUG3, "[STORE] YMDBMStore::clearBackend");

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    return handle->mdbmClear();
}

bool
HMStorageHostGroupMDBM::storeConfigInfo(const HMConfigInfo& configInfo)
{
    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to save config info in read only mode");
        return false;
    }

    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to store config info when store is closed");
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string data;
    data.resize(configInfo.serialize(nullptr, 0));
    if(configInfo.serialize(&data.at(0), data.size()) != data.size())
    {
        //LCOV_EXCL_LINE; can't be tested
        HMLog(HM_LOG_ERROR, "[STORE] Failed to serialize config info");
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }
    string key = HM_MDBM_CONFIG;
    handle->m_kv.key.dptr = &key.at(0);
    handle->m_kv.key.dsize = key.length();
    handle->m_kv.val.dptr = &data.at(0);
    handle->m_kv.val.dsize = data.length();

    return handle->mdbmStore();
}

bool
HMStorageHostGroupMDBM::getConfigInfo(HMConfigInfo& configInfo)
{
    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to get config info when store is closed");
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string key = HM_MDBM_CONFIG;
    handle->m_kv.key.dptr = &key.at(0);
    handle->m_kv.key.dsize = key.length();

    if(!handle->mdbmFetch())
    {
        return false;
    }

    HMLog(HM_LOG_DEBUG, "[STORE] Config Information Read");

    if(!configInfo.deserialize(handle->m_kv.val.dptr, handle->m_kv.val.dsize))
    {
        //LCOV_EXCL_LINE; can't be tested
        HMLog(HM_LOG_CRITICAL, "[STORE] getConfigInfo: Failed to parse config info");
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }
    return true;
}

bool
HMStorageHostGroupMDBM::getHostGroupNames(set<string>& groupNames)
{
    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to get group names when store is closed");
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string key = HM_MDBM_GROUP_NAMES;
    handle->m_kv.key.dptr = &key.at(0);
    handle->m_kv.key.dsize = key.length();

    if(!handle->mdbmFetch())
    {
        return false;
    }

    char* ptr = handle->m_kv.val.dptr;
    char* endptr = ptr + handle->m_kv.val.dsize;
    uint32_t size = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    groupNames.clear();

    for(uint32_t i = 0; i < size; i++)
    {
        uint32_t stringsize = *(uint32_t*)ptr;
        // make sure the size is within the returned data size
        if(ptr + sizeof(uint32_t) + stringsize > endptr)
        {
            HMLog(HM_LOG_ERROR, "[STORE] invalid data in getHostGroupNames");
            return false;
        }

        string temp;
        temp.resize(stringsize);
        strncpy(&temp.at(0), ptr + sizeof(uint32_t), stringsize);
        groupNames.insert(temp);
        ptr += (sizeof(uint32_t) + stringsize);
    }

    return true;
}

bool
HMStorageHostGroupMDBM::getGroupInfo(const string& hostGroupName, HMDataHostGroup& hostGroup)
{
    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to get group info when store is closed");
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string key = HM_MDBM_GROUP_PREFIX + hostGroupName;
    handle->m_kv.key.dptr = &key.at(0);
    handle->m_kv.key.dsize = key.length();

    handle->mdbmFetch();

    if(!hostGroup.deserialize(handle->m_kv.val.dptr, handle->m_kv.val.dsize))
    {
        HMLog(HM_LOG_INFO, "[STORE] getGroupInfo: HostGroup deserialize failed: %s", hostGroupName.c_str());
        return false;
    }

    return true;
}

bool
HMStorageHostGroupMDBM::openBackend()
{
    if(m_mdbmPath.empty())
    {
        m_mdbmPath = DEFAULT_DBNAME;
    }

    m_pool = make_unique<MDBMPool>(m_lockPolicy);
    bool opened = m_pool->init(m_mdbmPath, m_readonly);
    HMLog(HM_LOG_DEBUG, "[STORE] MDBM open %s", (opened) ? "successful" : "failed");

    return opened;
}

bool
HMStorageHostGroupMDBM::closeBackend()
{
    return true;
}

bool
HMStorageHostGroupMDBM::storeHostGroupNames(set<string>& groupNames)
{
    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to save group names in read only mode");
        return false;
    }

    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to store group names when store is closed");
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    uint32_t totalSize = sizeof(uint32_t);

    for(auto it = groupNames.begin(); it != groupNames.end(); ++it)
    {
        totalSize += (sizeof(uint32_t) + it->size());
    }

    string data;
    data.resize(totalSize);

    char* target = &data.at(0);
    *(uint32_t *)target = (uint32_t)groupNames.size();
    target += sizeof(uint32_t);

    for(auto it = groupNames.begin(); it != groupNames.end(); ++it)
    {
        *(uint32_t *)target = it->size();
        strncpy(target + sizeof(uint32_t), &it->at(0), it->size());
        target += (sizeof(uint32_t) + it->size());
    }

    string key = HM_MDBM_GROUP_NAMES;
    handle->m_kv.key.dptr = &key.at(0);
    handle->m_kv.key.dsize = key.length();
    handle->m_kv.val.dptr = &data.at(0);
    handle->m_kv.val.dsize = data.length();

    return handle->mdbmStore();
}

bool
HMStorageHostGroupMDBM::storeGroupInfo(const string& hostGroupName, HMDataHostGroup& hostGroup)
{
    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to save group info in read only mode");
        return false;
    }

    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to store group info when store is closed");
        return false;
    }

    uint32_t size = hostGroup.serialize(nullptr, 0);
    string data;
    data.resize(size);
    if(hostGroup.serialize(&data.at(0), size) != size)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Failed to serialize hostGroupInfo: %s", hostGroupName.c_str());
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string key = HM_MDBM_GROUP_PREFIX + hostGroupName;
    handle->m_kv.key.dptr = &key.at(0);
    handle->m_kv.key.dsize = key.length();
    handle->m_kv.val.dptr = &data.at(0);
    handle->m_kv.val.dsize = data.length();

    return handle->mdbmStore();
}

bool
HMStorageHostGroupMDBM::removeGroupInfo(const string& hostGroupName)
{
    HMLog(HM_LOG_DEBUG, "[STORE] YMDBMStore::removeHostGroup Info: Group %s", hostGroupName.c_str());

    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to remove group info entry in read only mode");
        return false;
    }

    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to remove group info entry %s after store is closed", hostGroupName.c_str());
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string hostGroupNameCopy = HM_MDBM_GROUP_PREFIX + hostGroupName;

    handle->m_kv.key.dptr = &hostGroupNameCopy.at(0);
    handle->m_kv.key.dsize = hostGroupNameCopy.length();

    return handle->mdbmRemove();
}

bool
HMStorageHostGroupMDBM::storeHostGroupCheckResults(const string& hostGroupName)
{
    HMLog(HM_LOG_DEBUG3, "[STORE] YMDBMStore::updateHostGroup: Group %s", hostGroupName.c_str());

    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to save group info in read only mode");
        return false;
    }

    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to store hostgroup %s after store is closed", hostGroupName.c_str());
        return false;
    }

    // Internal data format:
    // | total size (bytes) | number of records (count) |
    //(| size of hostname (bytes) | hostname | IPAddress | results |) --repeated count times
    uint32_t size = 2 * sizeof(uint32_t);
    uint32_t count = 0;

    shared_lock<shared_timed_mutex> lock(m_checkUpdateMutex);
    auto range = m_hostGroupResults.equal_range(hostGroupName);
    for(auto it = range.first; it != range.second; ++it)
    {
        count++;
        size += (sizeof(uint32_t) + it->second.m_hostName.size() + sizeof(HMIPAddress) + it->second.m_result.serialize(nullptr, 0));
    }
    string data;
    data.resize(size);

    char* target = &data.at(0);
    char* end = target + size;

    *(uint32_t*)target = size;
    *(uint32_t*)(target + sizeof(uint32_t)) = count;
    target += (2 * sizeof(uint32_t));

    for(auto it = range.first; it != range.second; ++it)
    {
        if(it->second.m_hostName.size() == 0)
        {
            HMLog(HM_LOG_ERROR, "[STORE] Hostname size is zero for hostgroup %s", hostGroupName.c_str());
            return false;
        }
        *(uint32_t*)target = (uint32_t)it->second.m_hostName.size();
        strncpy(target + sizeof(uint32_t), it->second.m_hostName.c_str(), it->second.m_hostName.size());
        memcpy(target + sizeof(uint32_t) + it->second.m_hostName.size(), &it->second.m_address, sizeof(HMIPAddress));
        target += (sizeof(uint32_t) + it->second.m_hostName.size() + sizeof(HMIPAddress));
        target += it->second.m_result.serialize(target, end - target);
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string hostGroupNameCopy = HM_MDBM_CHECK_PREFIX + hostGroupName;

    handle->m_kv.key.dptr = &hostGroupNameCopy.at(0);
    handle->m_kv.key.dsize = hostGroupNameCopy.length();
    handle->m_kv.val.dptr = &data.at(0);
    handle->m_kv.val.dsize = size;

    HMLog(HM_LOG_DEBUG3, "[STORE] Store hostgroup %s in mdbm", hostGroupNameCopy.c_str());

    return handle->mdbmStore();
}

bool
HMStorageHostGroupMDBM::getHostGroupCheckResults(const string& hostGroupName)
{
    vector<HMGroupCheckResult> results;

    if(!getHostGroupCheckResults(hostGroupName, results))
    {
        return false;
    }

    auto getGroup = m_hostGroupMap->find(hostGroupName);
    if(getGroup == m_hostGroupMap->end())
    {
        return false;
    }

    lock_guard<shared_timed_mutex> lock(m_checkUpdateMutex);
    m_hostGroupResults.erase(hostGroupName);
    for(auto it = results.begin(); it != results.end(); ++it)
    {
        m_hostGroupResults.insert(make_pair(hostGroupName, *it));
    }
    return true;
}

bool
HMStorageHostGroupMDBM::getHostGroupCheckResults(const string& hostGroupName, vector<HMGroupCheckResult>& results)
{
    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to get hostgroup results for %s after store is closed", hostGroupName.c_str());
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string hostGroupNameCopy = HM_MDBM_CHECK_PREFIX + hostGroupName;

    handle->m_kv.key.dptr = &hostGroupNameCopy.at(0);
    handle->m_kv.key.dsize = hostGroupNameCopy.length();

    if(!handle->mdbmFetch())
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    // Internal data format:
    // | total size (bytes) | number of records (count) |
    //(| size of hostname (bytes) | hostname | IPAddress | results |) --repeated count times
    if((handle->m_kv.val.dsize < (int32_t)sizeof(uint32_t)) || (handle->m_kv.val.dsize < (int32_t)*(uint32_t*)handle->m_kv.val.dptr))
    {
        HMLog(HM_LOG_INFO, "[STORE] getHostGroupCheckResults: Returned result incorrect size: %s", hostGroupName.c_str());
        return false;
    }

    uint32_t size = *(uint32_t*)handle->m_kv.val.dptr;
    int32_t entries = *(uint32_t*)(handle->m_kv.val.dptr + sizeof(uint32_t));

    char* src = handle->m_kv.val.dptr + (2 * sizeof(uint32_t));
    char* end = handle->m_kv.val.dptr + size;

    results.clear();
    results.resize(entries);

    for(int i = 0; i < entries; i++)
    {
        uint32_t stringsize = *(uint32_t*)src;
        if(src + sizeof(uint32_t) + stringsize + sizeof(HMIPAddress) > end)
        {
            HMLog(HM_LOG_INFO, "[STORE] getHostGroupCheckResults: Parse error incorrect record size: %s", hostGroupName.c_str());
            return false;
        }

        results[i].m_hostName.resize(*src);
        strncpy(&results[i].m_hostName.at(0), src + sizeof(uint32_t), stringsize);
        src += (sizeof(uint32_t) + stringsize);

        memcpy(&results[i].m_address, src,sizeof(HMIPAddress));
        src += sizeof(HMIPAddress);

        if(!results[i].m_result.deserialize(src, end - src))
        {
            HMLog(HM_LOG_INFO, "[STORE] getHostGroupCheckResults: Parse error incorrect hostname results size: %s", hostGroupName.c_str());
            return false;
        }
        src += results[i].m_result.serialize(nullptr, 0);
    }

    return true;
}

bool
HMStorageHostGroupMDBM::removeHostGroupCheckResults(const string& hostGroupName)
{
    HMLog(HM_LOG_DEBUG, "[STORE] YMDBMStore::removeHostGroup: Group %s", hostGroupName.c_str());

    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to remove group check results in read only mode");
        return false;
    }

    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to remove group check results %s after store is closed", hostGroupName.c_str());
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string hostGroupNameCopy = HM_MDBM_CHECK_PREFIX + hostGroupName;

    handle->m_kv.key.dptr = &hostGroupNameCopy.at(0);
    handle->m_kv.key.dsize = hostGroupNameCopy.length();

    return handle->mdbmRemove();
}

bool
HMStorageHostGroupMDBM::storeHostGroupAuxInfo(const string& hostGroupName)
{
    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to save aux info in read only mode");
        return false;
    }

    if(m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to save aux %s after store is closed", hostGroupName.c_str());
        return false;
    }

    /* Format is:
     | Total Size (bytes) | Number of Records |
     | HostNameSize (bytes) | HostNameString | IPAddress | UpdateTime | NumberOfAuxRecords | AuxRecords |
     */

    uint32_t size = 2 * sizeof(uint32_t);
    uint32_t count = 0;
    shared_lock<shared_timed_mutex> lock(m_auxUpdateMutex);
    auto range = m_hostGroupAux.equal_range(hostGroupName);
    for(auto it = range.first; it != range.second; ++it)
    {
        count++;
        size += ((2 * sizeof(uint32_t)) + it->second.m_hostName.size() + sizeof(HMIPAddress) + sizeof(uint64_t));

        for(auto iit = it->second.m_info.m_auxData.begin(); iit != it->second.m_info.m_auxData.end(); ++iit)
        {
            size += (*iit)->serialize(nullptr, 0);
        }
    }

    string data;
    data.resize(size);

    char* target = &data.at(0);
    char* end = target + size;

    *(uint32_t*)target = size;
    *(uint32_t*)(target + sizeof(uint32_t)) = count;
    target += (2 * sizeof(uint32_t));

    for(auto it = range.first; it != range.second; ++it)
    {
        *(uint32_t*)target = (uint32_t)it->second.m_hostName.size();
        if (it->second.m_hostName.size() == 0)
        {
            HMLog(HM_LOG_ERROR,
                    "[STORE] Hostname size is zero for hostgroup %s",
                    hostGroupName.c_str());
            return false;
        }
        strncpy(target + sizeof(uint32_t), it->second.m_hostName.c_str(),
                it->second.m_hostName.size());
        memcpy(target + sizeof(uint32_t) + it->second.m_hostName.size(), &it->second.m_address, sizeof(HMIPAddress));
        *(uint64_t*)(target + sizeof(uint32_t) + it->second.m_hostName.size() + sizeof(HMIPAddress)) = it->second.m_info.m_ts.getTimeSinceEpoch();
        *(uint32_t*)(target + sizeof(uint32_t) + it->second.m_hostName.size() + sizeof(HMIPAddress) + sizeof(uint64_t)) = (uint32_t)it->second.m_info.m_auxData.size();
        target += (sizeof(uint32_t) + it->second.m_hostName.size() + sizeof(HMIPAddress) + sizeof(uint64_t) + sizeof(uint32_t));

        for(auto iit = it->second.m_info.m_auxData.begin(); iit != it->second.m_info.m_auxData.end(); ++iit)
        {
            target += (*iit)->serialize(target, end - target);
        }
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string hostGroupNameCopy = HM_MDBM_AUX_PREFIX + hostGroupName;

    handle->m_kv.key.dptr = &hostGroupNameCopy.at(0);
    handle->m_kv.key.dsize = hostGroupNameCopy.length();
    handle->m_kv.val.dptr = &data.at(0);
    handle->m_kv.val.dsize = size;

    HMLog(HM_LOG_DEBUG3, "[STORE] Store auxinfo %s in mdbm", hostGroupNameCopy.c_str());

    return handle->mdbmStore();
}

bool
HMStorageHostGroupMDBM::getHostGroupGroupAuxInfo(const string& hostGroupName)
{
    vector<HMGroupAuxResult> results;

    if(!getHostGroupGroupAuxInfo(hostGroupName, results))
    {
        return false;
    }

    lock_guard<shared_timed_mutex> lock(m_auxUpdateMutex);
    m_hostGroupAux.erase(hostGroupName);
    for(auto it = results.begin(); it != results.end(); ++it)
    {
        m_hostGroupAux.insert(make_pair(hostGroupName, *it));
    }
    return true;
}

bool
HMStorageHostGroupMDBM::getHostGroupGroupAuxInfo(const string& hostGroupName, vector<HMGroupAuxResult>& results)
{
    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to get hostgroup aux info for %s after store is closed", hostGroupName.c_str());
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string hostGroupNameCopy = HM_MDBM_AUX_PREFIX + hostGroupName;

    handle->m_kv.key.dptr = &hostGroupNameCopy.at(0);
    handle->m_kv.key.dsize = hostGroupNameCopy.length();

    if(!handle->mdbmFetch())
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    /* Format is:
    | Total Size (bytes) | Number of Records |
    | HostNameSize (bytes) | HostNameString | IPAddress | UpdateTime | NumberOfAuxRecords | AuxRecords |
    */

    if(handle->m_kv.val.dsize < (int32_t)sizeof(uint32_t) || handle->m_kv.val.dsize < (int32_t)*handle->m_kv.val.dptr)
    {
        HMLog(HM_LOG_INFO, "[STORE] getHostGroupAuxInfo: Returned result incorrect size: %s", hostGroupName.c_str());
        return false;
    }

    uint32_t size = *(uint32_t*)handle->m_kv.val.dptr;
    uint32_t entries = *(uint32_t*)(handle->m_kv.val.dptr + sizeof(uint32_t));

    char* src = handle->m_kv.val.dptr + (2 * sizeof(uint32_t));
    char* end = handle->m_kv.val.dptr + size;

    results.clear();
    results.resize(entries);

    for(uint32_t i = 0; i < entries; i++)
    {
        uint32_t stringsize = *(uint32_t*)src;
        if(src + (2 * sizeof(uint32_t)) + stringsize + sizeof(HMIPAddress) + sizeof(uint64_t) > end)
        {
            HMLog(HM_LOG_INFO, "[STORE] getHostGroupAuxInfo: Parse error incorrect record size: %s", hostGroupName.c_str());
            return false;
        }

        results[i].m_hostName.resize(*src);
        strncpy(&results[i].m_hostName.at(0), src + sizeof(uint32_t), stringsize);
        src += (sizeof(uint32_t) + stringsize);

        memcpy(&results[i].m_address, src, sizeof(HMIPAddress));
        src += sizeof(HMIPAddress);

        results[i].m_info.m_ts.setTime(*(uint64_t*)src);
        src += sizeof(uint64_t);

        uint32_t numberRecords = *(uint32_t*)src;
        src += sizeof(uint32_t);

        for(uint32_t j = 0; j < numberRecords; j++)
        {
            auto entry = HMAuxBase::deserialize(src, end - src);
            if(!entry)
            {
                HMLog(HM_LOG_INFO, "[STORE] getHostGroupAuxInfo: Parse error incorrect hostname results size: %s", hostGroupName.c_str());
                return false;
            }
            src+= entry->serialize(nullptr, 0);
            results[i].m_info.m_auxData.push_back(move(entry->transfer()));
        }
    }

    return true;
}

bool
HMStorageHostGroupMDBM::removeHostGroupGroupAuxInfo(const string& hostGroupName)
{
    HMLog(HM_LOG_DEBUG, "[STORE] Remove Aux Info: Group %s", hostGroupName.c_str());

    if(m_readonly)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to remove group aux info in read only mode");
        return false;
    }

    if (m_shutdown)
    {
        HMLog(HM_LOG_ERROR, "[STORE] Attempting to remove group aux info %s after store is closed", hostGroupName.c_str());
        return false;
    }

    unique_ptr<MDBMHandle> handle = m_pool->getHandle();
    if(!handle)
    {
        //LCOV_EXCL_LINE; can't be tested
        return false;
        //LCOV_EXCL_LINE; can't be tested
    }

    string hostGroupNameCopy = HM_MDBM_AUX_PREFIX + hostGroupName;

    handle->m_kv.key.dptr = &hostGroupNameCopy.at(0);
    handle->m_kv.key.dsize = hostGroupNameCopy.length();

    return handle->mdbmRemove();
}

