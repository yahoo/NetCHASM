// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSTORAGE_H_
#define HMSTORAGE_H_

//! Default size for MD5 hash
#define HASH_MAX_SIZE 64 //Max MD5 length EVP_MAX_MD_SIZE


#include <string>
#include <openssl/evp.h>

#include "HMIPAddress.h"
#include "HMDataHostGroup.h"
#include "HMDataHostCheck.h"
#include "HMDataCheckParams.h"
#include "HMDataCheckResult.h"
#include "HMDataCheckList.h"
#include "HMDNSCache.h"
#include "HMUtilitySpinLock.h"
#include "HMAuxCache.h"

class HMDataCheckList;
class HMStorageHostGroup;
class HMState;

//! Convenience class to pass an entire host group check result.
/*!
      Convenience class to pass an entire host group check result.
      Includes the host name, IP address, and check result, and a flag if the backend is not synced.
 */
class HMGroupCheckResult
{
public:
    HMGroupCheckResult() :
        m_backendStale(false) {}
    HMGroupCheckResult(const std::string& hostName, const HMIPAddress& address, const HMDataCheckResult& result) :
        m_hostName(hostName),
        m_address(address),
        m_backendStale(false),
        m_result(result) { m_commitTime = HMTimeStamp::now(); }

    std::string m_hostName;
    HMIPAddress m_address;
    bool m_backendStale;
    HMTimeStamp m_commitTime;
    HMDataCheckResult m_result;
};

//! Convenience class to pass an entire host group check result update.
/*!
     Convenience class to pass an entire host group check result update.
     Include the host group name, the host name, the IP address and the check result.
 */
class HMGroupCheckUpdate
{
    public:
    HMGroupCheckUpdate(const std::string& hostGroup,
            const std::string& hostName,
            const HMIPAddress& address,
            const HMDataCheckResult& result) :
        m_hostGroup(hostGroup),
        m_hostName(hostName),
        m_address(address),
        m_result(result) {}

    std::string m_hostGroup;
    std::string m_hostName;
    HMIPAddress m_address;
    HMDataCheckResult m_result;
};

//! Convenience class to pass an entire host group Aux Info.
/*!
     Convenience class to pass an entire host group Aux Info.
     Includes the host name, IP address, and Aux info data, and a flag if the backend is not synced.
 */
class HMGroupAuxResult
{
public:
    HMGroupAuxResult() :
        m_backendStale(false) {};

    HMGroupAuxResult(const std::string& hostName,
            const HMIPAddress& address,
            const HMAuxInfo& info) :
                m_hostName(hostName),
                m_address(address),
                m_backendStale(false),
                m_info(info) { m_commitTime = HMTimeStamp::now(); }

    std::string m_hostName;
    HMIPAddress m_address;
    bool m_backendStale;
    HMTimeStamp m_commitTime;
    HMAuxInfo m_info;
};

//! Convenience class to pass an entire host goup aux info update.
/*!
     Convenience class to pass an entire host goup aux info update.
     Include the host group name, the host name, the IP address, and the Aux info data.
 */
class HMGroupAuxUpdate
{
public:
    HMGroupAuxUpdate(const std::string& hostGroup,
            const std::string& hostName,
            const HMIPAddress& address,
            const HMAuxInfo& info) :
                m_hostGroup(hostGroup),
                m_hostName(hostName),
                m_address(address),
                m_info(info) {}

    std::string m_hostGroup;
    std::string m_hostName;
    HMIPAddress m_address;
    HMAuxInfo m_info;
};

//! Convenience class to hold an entire host check result.
/*!
     Convenience class to hold an entire host check result.
     Include the host name, IP address, host check, check param, and results.
 */
class HMCheckData
{
public:
    HMCheckData(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            const HMDataCheckResult& result) :
                m_hostname(hostname),
                m_address(address),
                m_hostCheck(hostCheck),
                m_checkParams(checkParams),
                m_result(result) {}

    std::string m_hostname;
    HMIPAddress m_address;
    HMDataHostCheck m_hostCheck;
    HMDataCheckParams m_checkParams;
    HMDataCheckResult m_result;
};


//! Convenience class to hold an Aux data result.
/*!
    Convenience class to hold an Aux data result.
    Includes the host name, IP address, host check, check params and Aux data.
 */
class HMAuxData
{
public:
    HMAuxData(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            const HMAuxInfo& info) :
                m_hostname(hostname),
                m_address(address),
                m_hostCheck(hostCheck),
                m_checkParams(checkParams),
                m_info(info) {}

    std::string m_hostname;
    HMIPAddress m_address;
    HMDataHostCheck m_hostCheck;
    HMDataCheckParams m_checkParams;
    HMAuxInfo m_info;
};

//! Convenience class to hold a check header.
/*!
     Convenience class to hold a check header.
     Check header includes the host name, IP address, host check and check params.
 */
class HMCheckHeader
{
public:
    HMCheckHeader() {}
    HMCheckHeader(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams) :
                m_hostname(hostname),
                m_address(address),
                m_hostCheck(hostCheck),
                m_checkParams(checkParams) {}

    bool operator<(const HMCheckHeader& k) const;

    std::string m_hostname;
    HMIPAddress m_address;
    HMDataHostCheck m_hostCheck;
    HMDataCheckParams m_checkParams;
};

//! Class to hold the hash value
/*!
     Class to hold the hash value
     HMHash stores the hash of configs loaded.
 */
class HMHash {
public:
    HMHash(): m_hashSize(0) { }
    unsigned char m_hashValue[HASH_MAX_SIZE];
    uint32_t m_hashSize;
    bool operator==(const HMHash& k) const;
    bool operator!=(const HMHash& k) const;
};

//! Class to hold the config Info
/*!
     Class to hold the config Info
     Config info stores the version of netchasm to check the backend storage compatibility, the status of the config, load time of the config and a hash for comparisons.
 */
class HMConfigInfo
{
public:
    uint8_t m_version;
    HM_CONFIG_STATUS m_configStatus;
    HMTimeStamp m_configLoadTime;
    HMHash m_hash;
    uint32_t serialize(char* buf, uint32_t size) const;
    bool deserialize(char* buf, uint32_t size);

private:
    struct SerStruct
    {
        uint8_t m_version;
        uint8_t m_configStatus;
        uint64_t m_configLoadTime;
        uint32_t m_hashSize;
        //variables to reconstruct HMHashMD5
        unsigned char md_value[HASH_MAX_SIZE];
    };
};

//! Backend storage base class.
/*!
     Base class for all backend storage functionality. Class supports read only mode. It also can open a background thread for writing without blocking.
     It implements the write queues for both health check and aux info storage.
     Derived classes should implement the ability to store data to the backend.

     They must implement the following functions:
     initResultsFromBackend - called upon initial load to fill the state with saved information.
     clearBackend - clear the backend database information.

     storeConfigInfo - store the data in HMConfigInfo to the back end.
     getConfigInfo - get the data from the backend filling in HMConfigInfo.
     storeConfigs - store the configuration needed to run based on the HMState.
     getConfigs - restore the stored configuration information from the backend into an HMState structure.

     storeCheckResult - store the check information to the backend.
     getCheckResult - get the check information from the backend.
     purgeCheckResult - purge the check information from the backend.
     storeAuxInfo - store the aux information to the backend.
     getAuxInfo - get the aux information from the backend.
     purgeAuxInfo - purge the aux information from the backend.

     getDNS - get the DNS entries for the given hostname.

     updateCheckResultCache - Update any internal state relating to the check results for the given entrties.
     updateAuxInfoCache - Update any internal state relating to aux info for the given entries.

     updateHostGroups - force the given host groups to be updated in the backend from the result information.

     getGroupCheckResults - return all the check results for a given host group.
     getGroupAuxInfo - return all the aux info for a given host group.

     getHostGroupNames - get all the host group names from the stored configs.
     getGroupInfo - get the configured group info for the given host group.

     openBackend - (internal) call when the backend is opened.
     closeBackend - (internal) call when the backend is closed.
     commitHealthCheck - (internal) called from the commit thread to have the derived class write out the saved health checks information to the backend.
     commitAuxInfo - (internal)  called from the commit thread to have the derived class write out the savedAux info information to the backend.
*/
class HMStorage
{
public:

    HMStorage(HMDataHostGroupMap* hostGroupMap, HMDNSCache* dnsCache) :
        m_shutdown(true),
        m_readonly(false),
        m_auxCommitPolicy(HM_STORAGE_COMMIT_ALWAYS),
        m_healthCheckCommitPolicy(HM_STORAGE_COMMIT_ALWAYS),
        m_lockPolicy(HM_STORAGE_RW_LOCKS),
        m_hostGroupMap(hostGroupMap),
        m_dnsCache(dnsCache){}

    virtual ~HMStorage() {};

    //! Open the backend store.
    /*!
         Open the backend store. Default open function to open the store in r/w mode.
         \return success if the data store was opened and is ready.
    */
    bool openStore();

    //! Open the backend store.
    /*!
        Open the backend store.
        \param true to open in read only mode.
        \return success if the backend store was opened and is ready.
     */
    bool openStore(bool readonly);

    //! Close the data store.
    /*!
        Close the data store.
    */
    void closeStore();

    //! Check the backend version for compatibility.
    /*!
          Check the version of the backend database. Clear the data if the version does not match.
          \param returns true if the database is ready for use.
    */
    bool validateDBVersion();

    //! Sync the local data caches with results from the backend.
    /*!
         Sync the local data caches with results from the backend. Fill in HealthChecks and AuxInfo results. This does not load the configs just the results. Purge results if they are no longer needed in the loaded configs.
          \param the checkList restore.
          \param the DNS cache to restore.
          \param the Aux Cached to restore.
    */
    virtual void initResultsFromBackend(HMDataCheckList& checkList, HMDNSCache& dnsCache, HMAuxCache& auxCache) = 0;

    //! Clear the backend datastore.
    /*!
         clear the backend datastore and any local caches in the store class.
           \return true if the clear was a success.
     */
    virtual bool clearBackend() = 0;

    //! Store the config info into the data store.
    /*!
         Store the config info into the data store.
         \param HMConfigInfo structure
         \return true on success.
    */
    virtual bool storeConfigInfo(const HMConfigInfo& configInfo) = 0;

    //! Get the config info from the data store.
    /*!
          Get the config info int the data store.
          \param HMConfigInfo structure
          \return true if the passed structure has the config info from the data store.
    */
    virtual bool getConfigInfo(HMConfigInfo& configInfo) = 0;

    //! Store the configs into the backend.
    /*!
         Store the configs into the backend.
         \param the HMState containing the config info to store.
         \return true if the configs were stored.
     */
    virtual bool storeConfigs(HMState& checkState) = 0;

    //! Get the configs from the backend.
    /*!
         Get the configs from the backend.
         \param the HMState to fill with the config info from the backend.
         \return true if the config information was loaded into the passed HMState successfully.
     */
    virtual bool getConfigs(HMState& checkState) = 0;

    //! Store the check result into the backend.
    /*!
         Store the check result into the backend.
         \param the hostname of the check result.
         \param the IP address of the check result.
         \param the host check conducted on the host/IP
         \param the check params used to conduct the host check.
         \param the check result to store.
         \return true if the check info was stored successfully.
     */
    virtual bool storeCheckResult(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            const HMDataCheckResult& checkResult) = 0;

    //! Get the check result from the backend.
    /*!
         Get the check result fromthe backend.
         \param the hostname of the check to retrieve.
         \param the IP address of the check to retrieve.
         \param the host check to retrieve the check.
         \param the check params to retrieve the check.
         \param the check result data structure to store the results.
         \return true if the passed check result contains the check results from the backend.
     */
    virtual bool getCheckResult(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            HMDataCheckResult& checkResult) = 0;

    //! Purge the check result from the backend.
    /*!
         Purge the check result from the backend.
         \param the hostname of the check to purge.
         \param the IP address of the check to purge.
         \param the host check to purge.
         \param the check params to purge.
         \return true if the result was purged from the backend.
     */
    virtual bool purgeCheckResult(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams) = 0;

    //! Store the Aux Info to the backend.
    /*!
         Store the Aux Info to the backend.
         \param the host name associated with the Aux info.
         \param the IP address associated with the Aux info.
         \param the host check associated with the Aux info.
         \param the check params associated with the Aux info.
         \param the Aux info to store.
         \return true if the Aux info was stored.
     */
    virtual bool storeAuxInfo(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            const HMAuxInfo& auxInfo) = 0;

    //! Get the Aux Info to the backend.
    /*!
         Get the Aux Info to the backend.
         \param the host name associated with the Aux info.
         \param the IP address associated with the Aux info.
         \param the host check associated with the Aux info.
         \param the check params associated with the Aux info.
         \param the Aux info data structure to fill with the Aux info.
         \return true if the Aux info was retrieved and ready in the passed structure.
     */
    virtual bool getAuxInfo(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            HMAuxInfo& auxInfo) = 0;

    //! Purge the Aux Info from the backend.
    /*!
         Purge the Aux Info from the backend.
         \param the host name associated with the Aux info.
         \param the IP address associated with the Aux info.
         \param the host check associated with the Aux info.
         \param the check params associated with the Aux info.
         \return true if the Aux info was purged.
     */
    virtual bool purgeAuxInfo(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams) = 0;

    //! Get the IPs associated with a given hostname.
    /*!
         Get the IPs associated with the given hostname.
         \param the hostname to lookup.
         \param the set of HMIPAddress to fill with the results.
         \return true if the IP addresses were looked up correctly.
     */
    virtual bool getDNS(const std::string& hostname, std::set<HMIPAddress>& ips) = 0;

    //! Update any internal check results caches.
    /*!
         Update any internal check results. Called during a reload to move state between store classes. Copy the given result into the internal cache without updating the backend.
         \param the check header to update. (HMCheckHeader)
         \param the result to update for the given check header.
         \result true if the result was updated successfully.
     */
    virtual bool updateCheckResultCache(HMCheckHeader& header, HMDataCheckResult& result) = 0;

    //! Update any internal aux info caches.
    /*!
         Update any internal aux info caches. Called during a reload to move state between store classes. Copy the given result into the internal cache without updating the backend.
         \param the check header to update. (HMCheckHeader)
         \param the aux info to update for the given check header.
         \result true if the result was updated successfully.
     */
    virtual bool updateAuxInfoCache(HMCheckHeader& header, HMAuxInfo& aux) = 0;

    //! Force the given hosts to update to the backend storage now.
    /*!
         Force the given hosts to update to the backend storage now.
         Used during a reload to make the storage class update data from the host groups which may have changed.
         \param the set of host groups to force an update.
     */
    virtual void updateHostGroups(std::set<std::string>& hostGroups) = 0;

    //! Get the health check results for a given host group.
    /*!
         Get the health check results for a given host group.
         \param the group name to get the results.
         \param true to force the backend to not use internally cached results.
         \param true to only return the health checks for resolved hosts.
         \param results vector to store the check results (HMGroupCheckResult)
         \return true if the results vector contains the results.
     */
    virtual bool getGroupCheckResults(const std::string& groupName,
                bool noCache,
                bool onlyResolved,
                std::vector<HMGroupCheckResult>& results) = 0;

    //! Get the aux info for a given host group.
    /*!
         Get the aux info for a given host group.
         \param the group name to get the aux info.
         \param true to force the backend to not use internally cached results.
         \param true to only return the aux info for resolved hosts.
         \param results vector to store the aux info (HMGroupAuxResult)
         \return true if the results vector contains the results.
     */
    virtual bool getGroupAuxInfo(const std::string& groupName,
                bool noCache,
                bool onlyResolved,
                std::vector<HMGroupAuxResult>& results) = 0;

    //! Get all the host group names from the stored configuration info.
    /*!
         Get all the host group names from the stored configuration info.
         \param the set to store the group names from the backend.
         \return true if the set contains the host group names.
     */
    virtual bool getHostGroupNames(std::set<std::string>& groupNames) = 0;

    //! Get the host group info form the backend for a given host group name.
    /*!
         Get the host group info form the backend for a given host group name.
         \param the host group name to lookup the host group info.
         \param the HMDataHostGroup class to fill with the host group info.
         \return true if the host group info is in the HMDataHostGroup.
     */
    virtual bool getGroupInfo(const std::string& hostGroupName, HMDataHostGroup& hostGroup) = 0;

    //! Update the Aux commit policy
    /*!
         Update the Aux commit policy to one of HM_STORAGE_COMMIT_POLICY.
         \param the new HM_STORAGE_COMMIT_POLICY.
     */
    void updateAuxCommitPolicy(HM_STORAGE_COMMIT_POLICY commitPolicy);

    //! Update the health check commit policy
    /*!
         Update the health check commit policy to one of HM_STORAGE_COMMIT_POLICY.
         \param the new HM_STORAGE_COMMIT_POLICY.
     */
    void updateHealthCheckCommitPolicy(HM_STORAGE_COMMIT_POLICY commitPolicy);

    //! Update the storage lock policy
    /*!
         Update the storage lock policy to one of HM_STORAGE_LOCK_POLICY.
         \param the new HM_STORAGE_LOCK_POLICY.
     */
    void updateLockPolicy(HM_STORAGE_LOCK_POLICY lockPolicy);

protected:

    //! Internal function to handle opening the database and initializing the data structures.
    /*!
         Internal function to handle opening the database and initializing the data structures.
         \return true if the datastore was opened successfully.
     */
    virtual bool openBackend() = 0;

    //! Internal function to handle closing the backend storage.
    virtual bool closeBackend() = 0;

    //! Internal function called from the commit thread to have the derived class write out the saved health checks information to the backend.
    virtual bool commitHealthCheck() = 0;

    //! Internal function called from the commit thread to have the derived class write out the savedAux info information to the backend.
    virtual bool commitAuxInfo() = 0;

    //! The function running in a separate thread to commit the health check and aux info to the backend without blocking the main data path.
    void runStore();

    HMUtilitySpinLock m_checkQueueSpinLock;
    HMUtilitySpinLock m_auxQueueSpinLock;
    std::mutex m_dataReadyMutex;
    std::condition_variable m_dataReadyCond;
    std::thread m_thread;

    bool m_shutdown;
    bool m_readonly;

    HM_STORAGE_COMMIT_POLICY m_auxCommitPolicy;
    HM_STORAGE_COMMIT_POLICY m_healthCheckCommitPolicy;
    HM_STORAGE_LOCK_POLICY m_lockPolicy;

    HMDataHostGroupMap* m_hostGroupMap;
    HMDNSCache* m_dnsCache;
};

#endif /* HMSTORAGEBASE_H_ */
