// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSTORAGEHOSTGROUPMDBM_H_
#define HMSTORAGEHOSTGROUPMDBM_H_

#include <mdbm.h>
#include <mdbm_handle_pool.h>

#include "HMStorage.h"
#include "HMAuxCache.h"
#include "HMStorageHostGroup.h"

//! The default MDBM locking scheme to use.
#define MDBM_DEFAULT_LOCKING MDBM_RW_LOCKS
//! The GLOBAL MDBM locking scheme to use.
#define MDBM_GLOBAL_LOCKING 0x0
//! The default open flags for the MDBM database.
#define MDBM_OPEN_FLAGS (MDBM_CREATE_V3 | MDBM_LARGE_OBJECTS)
//! The default MDBM path in NetCHASM
#define DEFAULT_DBNAME "/home/y/var/run/netchasm.mdbm"

//! The prefix to use for the config data in the MDBM key.
const std::string HM_MDBM_CONFIG = "hm:config";
//! The prefix to use for the group names in the MDBM key.
const std::string HM_MDBM_GROUP_NAMES = "hm:groups";
//! The prefix to use for the group information in the MDBM key.
const std::string HM_MDBM_GROUP_PREFIX = "hm:group:";
//! The prefix to use for the health check information in the MDBM key.
const std::string HM_MDBM_CHECK_PREFIX = "hm:check:";
//! The prefix to use for the aux info information in the MDBM key.
const std::string HM_MDBM_AUX_PREFIX = "hm:aux:";

//! Convenience class to track an MDBM handle.
/*!
     Convenience class to track and MDBM handle.
     Wraps all the locking/unlocking functionality for all locking types.
     Automatically cleans up the handle upon shutdown for smart pointer use.
     The locks are auto-aquired for the store and delete operations.
     The lock is auto release when the handle goes out of scope for the handle.
     Warning the m_kv retains a read lock during read operations until object destruction.
     Thus, the data should be copied immediately to avoid lock contention.
 */
class MDBMHandle
{
public:

    MDBMHandle(uint32_t lockScheme) :
        m_handle(nullptr),
        m_mdbmPool(nullptr),
        m_locked(false),
        m_lockScheme(lockScheme)  {}

    ~MDBMHandle();

    //! Initialize the handle using the given MDBM pool.
    /*!
         Initialize the handle using the given MDBM pool.
         \param a pointer to the MDBM pool to use to get the handle.
         \return true if the handle is ready for operations.
     */
    bool init(mdbm_pool_t* pool);

    //! Lock the handle for an MDBM operation.
    /*!
         Lock the handle for an MDBM operation.
         Wraps the lock/unlock logic for the given lock strategy passed on construction.
         \param true if the handle is read only.
         \return true if the lock was aquired.
     */
    bool lock(bool readOnly);

    //! Unlock the handle.
    /*!
         Unlock the handle.
         \return true if the handle is unlocked.
     */
    bool unlock();

    //! Store the data to MDBM.
    /*!
         Store the data to MDBM as stored in the m_kv variable.
         m_kv.key contains the key and m_kv.data is the pointer to the data to store.
         \return true if the data was stored.
     */
    bool mdbmStore();

    //! Get the data from MDBM.
    /*!
        Get the data drom MDBM and store the data in m_kv.value. Expects the key to be prefilled in m_kv.key.
        Locks the database until the unlock is called or the handle goes out of scope.
        The m_kv is no longer accessible after the database is unlocked.
        \return true if the data was fetched.
     */
    bool mdbmFetch();

    //! Remove the data from MDBM.
    /*!
         Remove the data from key m_kv.key from the MDBM database.
         \return true indicating the data was deleted.
     */
    bool mdbmRemove();

    //! Delete all data from the MDBM database.
    /*!
         Delete all the data from the MDBM database.
         \return true if the data was deleted.
     */
    bool mdbmClear();

    kvpair m_kv;

private:
    MDBM* m_handle;
    mdbm_pool_t* m_mdbmPool;
    bool m_locked;
    uint32_t m_lockScheme;
};

//! Convenience function to wrap the MDBM pool.
/*!
     Convenience function to wrap the MDBM pool. Handles shutting down the MDBM pool on destruction.
 */
class MDBMPool
{
public:

    MDBMPool() :
        m_mdbm(nullptr),
        m_mdbmPool(nullptr),
        m_lockScheme(MDBM_DEFAULT_LOCKING) {}

    MDBMPool(HM_STORAGE_LOCK_POLICY defaultLocking) :
        m_mdbm(nullptr),
        m_mdbmPool(nullptr)
    {
        switch (defaultLocking) {
        case HM_STORAGE_GLOBAL_LOCKS:
            m_lockScheme = MDBM_GLOBAL_LOCKING;
            break;
        case HM_STORAGE_RW_LOCKS:
            m_lockScheme = MDBM_RW_LOCKS;
            break;
        case HM_STORAGE_PARTITION_LOCKS:
            m_lockScheme = MDBM_PARTITIONED_LOCKS;
            break;
        };
    }

    ~MDBMPool();

    //! Check to see if the MDBM pool is ready.
    /*!
         Check to see if the MDBM pool is ready.
         \return true if the MDBM pool is ready to use.
     */
    bool isInit() { return m_mdbm != nullptr && m_mdbmPool != nullptr; }

    //! Create a new MDBM pool.
    /*!
         Creeat a new MDBM pool.
         \param the path to the MDBM file on the disk.
         \param true if the MDBM pool should be read only.
     */
    bool init(std::string& mdbmPath, bool readOnly);

    //! Get a new Handle to the MDBM pool.
    /*!
         Get a new Handle to the MDBM pool.
         \return a unique pointer to an MDBM handle read for operations.
     */
    std::unique_ptr<MDBMHandle> getHandle();

private:

    MDBM* m_mdbm;
    mdbm_pool_t* m_mdbmPool;
    uint32_t m_lockScheme;
};

//! Main class to support storing information to an MDBM backend using the host group as the keys.
class HMStorageHostGroupMDBM : public HMStorageHostGroup
{
public:
    HMStorageHostGroupMDBM(std::string filename, HMDataHostGroupMap* hostGroupMap, HMDNSCache* dnsCache) :
        HMStorageHostGroup(hostGroupMap, dnsCache),
        m_mdbmPath(filename),
        m_openFlags(MDBM_OPEN_FLAGS) {}

    ~HMStorageHostGroupMDBM(){ closeStore(); }

    //! Clear the backend datastore.
    /*!
         clear the backend datastore and any local caches in the store class.
           \return true if the clear was a success.
     */
    bool clearBackend();

    //! Store the config info into the data store.
    /*!
         Store the config info into the data store.
         \param HMConfigInfo structure
         \return true on success.
    */
    bool storeConfigInfo(const HMConfigInfo& configInfo);

    //! Get the config info from the data store.
    /*!
          Get the config info int the data store.
          \param HMConfigInfo structure
          \return true if the passed structure has the config info from the data store.
    */
    bool getConfigInfo(HMConfigInfo& configInfo);

    //! Get all the host group names from the stored configuration info.
    /*!
         Get all the host group names from the stored configuration info.
         \param the set to store the group names from the backend.
         \return true if the set contains the host group names.
     */
    bool getHostGroupNames(std::set<std::string>& groupNames);

    //! Get the host group info from the backend for a given host group name.
    /*!
         Get the host group info from the backend for a given host group name.
         \param the host group name to lookup the host group info.
         \param the HMDataHostGroup class to fill with the host group info.
         \return true if the host group info is in the HMDataHostGroup.
     */
    bool getGroupInfo(const std::string& hostGroupName, HMDataHostGroup& hostGroup);

protected:
    //! Internal function to handle opening the database and initializing the data structures.
    /*!
         Internal function to handle opening the database and initializing the data structures.
         \return true if the datastore was opened successfully.
     */
    bool openBackend();

    //! Internal function to handle closing the backend storage.
    bool closeBackend();

    // Internal function to get all the host group names.
    /*!
         Internal function to get all the host group names.
         \param the set of all host group names.
         \return true if the host group names were stored successfully.
     */
    bool storeHostGroupNames(std::set<std::string>& groupNames);

    // Internal function to store the group info per host group.
    /*!
         Internal function to store the group info per host group.
         \param the host group name.
         \param the host group info.
         \return true if the host group info was stored successfully.
     */
    bool storeGroupInfo(const std::string& hostGroupName, HMDataHostGroup& hostGroup);

    // Internal function to remove the group info per host group.
    /*!
         Internal function to remove the group info per host group.
         \param the host group name.
         \return true if the host group info was removed successfully.
     */
    bool removeGroupInfo(const std::string& hostGroupName);

    // Internal function to store the host group results.
    /*!
         Internal function to store the host group results to the backend.
         Function assumes the host group results are in the cache.
         \param the host group to store.
         \return true if the host group was stored successfully.
     */
    bool storeHostGroupCheckResults(const std::string& hostGroup);

    // Internal function to get the check results from the backend for the given group.
    /*!
         Internal function to get the check results from the backend for the given group.
         Function stores the results into the local cache.
         \param the host group to get.
         \return true if the host group was retrieved successfully.
     */
    bool getHostGroupCheckResults(const std::string& hostGroup);

    // Internal function to get the check results from the backend for the given group.
    /*!
         Internal function to get the check results from the backend for the given group.
         Function stores the results into the passed vector. The internal cache is not updated.
         \param the host group to get.
         \param the vector to hold the HMGroupCheckResults.
         \return true if the host group was retrieved successfully.
     */
    bool getHostGroupCheckResults(const std::string& hostGroup, std::vector<HMGroupCheckResult>& results);

    // Internal function to remove the host group from the backend.
    /*!
         Internal function to remove the host group from the backend.
         \param the host group to remove.
         \return true if the host group was removed successfully.
     */
    bool removeHostGroupCheckResults(const std::string& hostGroupName);

    // Internal function to store the Aux info.
    /*!
         Internal function to store the Aux info to the backend.
         Function assumes the Aux info is in the cache.
         \param the host group to store.
         \return true if the Aux info was stored successfully.
     */
    bool storeHostGroupAuxInfo(const std::string& hostGroup);

    // Internal function to get the Aux info from the backend for the given group.
    /*!
         Internal function to get the Aux info from the backend for the given group.
         Function stores the results into the local cache.
         \param the host group to get.
         \return true if the Aux info was retrieved successfully.
     */
    bool getHostGroupGroupAuxInfo(const std::string& hostGroup);

    // Internal function to get the Aux info from the backend for the given group.
    /*!
         Internal function to get the Aux info from the backend for the given group.
         Function stores the results into the passed vector. The internal cache is not updated.
         \param the host group to get.
         \param a vector to hold the HMGroupResults for the host group.
         \return true if the Aux info was retrieved successfully.
     */
    bool getHostGroupGroupAuxInfo(const std::string& hostGroup, std::vector<HMGroupAuxResult>& results);

    // Internal function to remove the Aux info from the backend.
    /*!
         Internal function to remove the Aux info from the backend.
         \param the host group to remove.
         \return true if the host group was removed successfully.
     */
    bool removeHostGroupGroupAuxInfo(const std::string& hostGroup);

private:

    std::string m_mdbmPath;
    uint32_t m_openFlags;

    std::unique_ptr<MDBMPool> m_pool;
};

#endif /* HMSTORAGEHOSTGROUPYFORMDBM_H_ */
