// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HMSTORAGEHOSTGROUP_H_
#define INCLUDE_HMSTORAGEHOSTGROUP_H_

#include <string>
#include <vector>

#include "HMStorage.h"

// Base class to handle storage per host group
/*!
     Base class to handle storage per host group.
     This class handles the book keeping to update the host group each time an individial host check is done.
     The backend should store all info based on the hostgroup as the key.
     The class keeps a cache of check information keyed on the hostgroup. Updates can be done per check or per check round.
     Classes derived from this class will implement a backend that stores the host information by group key.
     The should implement the following functions:

     clearBackend - Clear the backend data store.
     storeConfigInfo - Store the configuration info class.
     getConfigInfo - Retrieve the configuration info class.

     storeHostGroupNames - Store all the host group names in the back end.
     getHostGroupNames - Get all host group names from the stored configs.
     removeGroupInfo - Remove the host group info for a given host group.

     getGroupInfo - Get the group info for a specific group.

     openBackend - Open the backend data store.
     closeBackend - Close the backend data store.

     storeHostGroupCheckResults - Store the host check results to the backend.
     getHostGroupCheckResults - get the host check restults from the backend.
     removeHostGroupCheckResults - Remove the host check results from the backend.

     storeHostGroupAuxInfo - Store the Aux info to the backend.
     getHostGroupGroupAuxInfo - Get the Aux info from the backend.
     removeHostGroupGroupAuxInfo - Remove the Aux info from the backend.
 */
class HMStorageHostGroup : public HMStorage
{
public:
    HMStorageHostGroup(HMDataHostGroupMap* hostGroupMap, HMDNSCache* dnsCache) :
        HMStorage(hostGroupMap, dnsCache) {}

    virtual ~HMStorageHostGroup() {};

    //! sync and prepare the backend for use
    /*!
      Sync and Prepare the backend for use. Checklist, DNSCache and auxCache are used as ground truth.
      Stale entries are deleted from the backend. CheckResults and AuxInfo are restored for valid entries.
      Blank entries are inserted into the backend for valid keys.
      \param The active checkList
      \param The active DNSCache
      \param The active auxCache
     */
    void initResultsFromBackend(HMDataCheckList& checkList, HMDNSCache& dnsCache, HMAuxCache& auxCache);

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
    bool storeConfigs(HMState& checkState);

    //! Get the configs from the backend.
    /*!
         Get the configs from the backend.
         \param the HMState to fill with the config info from the backend.
         \return true if the config information was loaded into the passed HMState successfully.
     */
    bool getConfigs(HMState& checkState);

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
    bool storeCheckResult(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            const HMDataCheckResult& checkResult);

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
    bool getCheckResult(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            HMDataCheckResult& checkResult);

    //! Purge the check result from the backend.
    /*!
         Purge the check result from the backend.
         \param the hostname of the check to purge.
         \param the IP address of the check to purge.
         \param the host check to purge.
         \param the check params to purge.
         \return true if the result was purged from the backend.
     */
    bool purgeCheckResult(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams);

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
    bool storeAuxInfo(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            const HMAuxInfo& auxInfo);

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
    bool getAuxInfo(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            HMAuxInfo& auxInfo);

    //! Purge the Aux Info from the backend.
    /*!
         Purge the Aux Info from the backend.
         \param the host name associated with the Aux info.
         \param the IP address associated with the Aux info.
         \param the host check associated with the Aux info.
         \param the check params associated with the Aux info.
         \return true if the Aux info was purged.
     */
    bool purgeAuxInfo(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams);

    //! Get the IPs associated with a given hostname.
    /*!
         Get the IPs associated with the given hostname.
         \param the hostname to lookup.
         \param the set of HMIPAddress to fill with the results.
         \return true if the IP addresses were looked up correctly.
     */
    bool getDNS(const std::string& hostname, std::set<HMIPAddress>& ips);

    //! Update any internal check results caches.
    /*!
         Update any internal check results. Called during a reload to move state between store classes. Copy the given result into the internal cache without updating the backend.
         \param the check header to update. (HMCheckHeader)
         \param the result to update for the given check header.
         \result true if the result was updated successfully.
     */
    bool updateCheckResultCache(HMCheckHeader& header, HMDataCheckResult& result);

    //! Update any internal aux info caches.
    /*!
         Update any internal aux info caches. Called during a reload to move state between store classes. Copy the given result into the internal cache without updating the backend.
         \param the check header to update. (HMCheckHeader)
         \param the aux info to update for the given check header.
         \result true if the result was updated successfully.
     */
    bool updateAuxInfoCache(HMCheckHeader& header, HMAuxInfo& aux);

    //! Force the given hosts to update to the backend storage now.
    /*!
         Force the given hosts to update to the backend storage now.
         Used during a reload to make the storage class update data from the host groups which may have changed.
         \param the set of host groups to force an update.
     */
    void updateHostGroups(std::set<std::string>& hostGroups);

    //! Get the health check results for a given host group.
    /*!
         Get the health check results for a given host group.
         \param the group name to get the results.
         \param true to force the backend to not use internally cached results.
         \param true to only return the health checks for resolved hosts.
         \param results vector to store the check results (HMGroupCheckResult)
         \return true if the results vector contains the results.
     */
    bool getGroupCheckResults(const std::string& groupName,
            bool noCache,
            bool onlyResolved,
            std::vector<HMGroupCheckResult>& results);

    //! Get the aux info for a given host group.
    /*!
         Get the aux info for a given host group.
         \param the group name to get the aux info.
         \param true to force the backend to not use internally cached results.
         \param true to only return the aux info for resolved hosts.
         \param results vector to store the aux info (HMGroupAuxResult)
         \return true if the results vector contains the results.
     */
    bool getGroupAuxInfo(const std::string& groupName,
            bool noCache,
            bool onlyResolved,
            std::vector<HMGroupAuxResult>& results);

    //! Get all the host group names from the stored configuration info.
    /*!
         Get all the host group names from the stored configuration info.
         \param the set to store the group names from the backend.
         \return true if the set contains the host group names.
     */
    virtual bool getHostGroupNames(std::set<std::string>& groupNames) = 0;

    //! Get the host group info from the backend for a given host group name.
    /*!
         Get the host group info from the backend for a given host group name.
         \param the host group name to lookup the host group info.
         \param the HMDataHostGroup class to fill with the host group info.
         \return true if the host group info is in the HMDataHostGroup.
     */
    virtual bool getGroupInfo(const std::string& hostGroupName, HMDataHostGroup& hostGroup) = 0;

protected:
    //! Internal function to handle opening the database and initializing the data structures.
    /*!
         Internal function to handle opening the database and initializing the data structures.
         \return true if the datastore was opened successfully.
     */
    virtual bool openBackend() = 0;

    //! Internal function to handle closing the backend storage.
    virtual bool closeBackend() = 0;

    //! Internal function to update the check results cache.
    /*!
         Internal function to update the check results cache. Place the result directly in the internal cache. Do not update the backend.
         \param the HMCheckHeader for the check.
         \param the check result to store for the given group.
         \return true if the result was updated successfully.
     */
    bool updateCheckResultsCache(HMCheckHeader& header, HMDataCheckResult& result);

    // Internal function to get all the host group names.
    /*!
         Internal function to get all the host group names.
         \param the set of all host group names.
         \return true if the host group names were stored successfully.
     */
    virtual bool storeHostGroupNames(std::set<std::string>& groupNames) = 0;

    // Internal function to store the group info per host group.
    /*!
         Internal function to store the group info per host group.
         \param the host group name.
         \param the host group info.
         \return true if the host group info was stored successfully.
     */
    virtual bool storeGroupInfo(const std::string& hostGroupName, HMDataHostGroup& hostGroup) = 0;

    // Internal function to remove the group info per host group.
    /*!
         Internal function to remove the group info per host group.
         \param the host group name.
         \return true if the host group info was removed successfully.
     */
    virtual bool removeGroupInfo(const std::string& hostGroupName) = 0;

    // Internal function to store the host group results.
    /*!
         Internal function to store the host group results to the backend.
         Function assumes the host group results are in the cache.
         \param the host group to store.
         \return true if the host group was stored successfully.
     */
    virtual bool storeHostGroupCheckResults(const std::string& hostGroup) = 0;

    // Internal function to get the check results from the backend for the given group.
    /*!
         Internal function to get the check results from the backend for the given group.
         Function stores the results into the local cache.
         \param the host group to get.
         \return true if the host group was retrieved successfully.
     */
    virtual bool getHostGroupCheckResults(const std::string& hostGroup) = 0;

    // Internal function to get the check results from the backend for the given group.
    /*!
         Internal function to get the check results from the backend for the given group.
         Function stores the results into the passed vector. The internal cache is not updated.
         \param the host group to get.
         \param the vector to hold the HMGroupCheckResults.
         \return true if the host group was retrieved successfully.
     */
    virtual bool getHostGroupCheckResults(const std::string& hostGroup, std::vector<HMGroupCheckResult>& results) = 0;

    // Internal function to remove the host group from the backend.
    /*!
         Internal function to remove the host group from the backend.
         \param the host group to remove.
         \return true if the host group was removed successfully.
     */
    virtual bool removeHostGroupCheckResults(const std::string& hostGroup) = 0;

    // Internal function to store the Aux info.
    /*!
         Internal function to store the Aux info to the backend.
         Function assumes the Aux info is in the cache.
         \param the host group to store.
         \return true if the Aux info was stored successfully.
     */
    virtual bool storeHostGroupAuxInfo(const std::string& hostGroup) = 0;

    // Internal function to get the Aux info from the backend for the given group.
    /*!
         Internal function to get the Aux info from the backend for the given group.
         Function stores the results into the local cache.
         \param the host group to get.
         \return true if the Aux info was retrieved successfully.
     */
    virtual bool getHostGroupGroupAuxInfo(const std::string& hostGroup) = 0;

    // Internal function to get the Aux info from the backend for the given group.
    /*!
         Internal function to get the Aux info from the backend for the given group.
         Function stores the results into the passed vector. The internal cache is not updated.
         \param the host group to get.
         \param a vector to hold the HMGroupResults for the host group.
         \return true if the Aux info was retrieved successfully.
     */
    virtual bool getHostGroupGroupAuxInfo(const std::string& hostGroup, std::vector<HMGroupAuxResult>& results) = 0;

    // Internal function to remove the Aux info from the backend.
    /*!
         Internal function to remove the Aux info from the backend.
         \param the host group to remove.
         \return true if the host group was removed successfully.
     */
    virtual bool removeHostGroupGroupAuxInfo(const std::string& hostGroup) = 0;

    //! Internal function called from the commit thread to have the derived class write out the saved health checks information to the backend.
    bool commitHealthCheck();
    //! Internal function called from the commit thread to have the derived class write out the savedAux info information to the backend.
    bool commitAuxInfo();

    //! Internal function to get the cached host group info.
    /*!
         Internal function to get the cached host group info.
         \param the host group name to get the group info.
         \param a HMDataHostGroup to store the host group info.
         \return true if the host group info was saved in the passed HMDataHostGroup.
     */
    bool getInternalHostGroupInfo(const std::string& hostGroupName, const HMDataHostGroup* &hostGroup);

    //! Internal function to get the cached check results.
    /*!
         Internal function to get the cached check results. Does not hit the backend.
         \param the group name to get the results.
         \param a vector of HMGroupCheckResults to hold the results.
         \return true if the passed vector contains the results for the passed host group name.
     */
    uint32_t getInternalCheckResults(const std::string& hostGroupName, std::vector<HMGroupCheckResult>& results);

    std::shared_timed_mutex m_checkUpdateMutex;
    std::multimap<std::string, HMGroupCheckResult> m_hostGroupResults;
    std::queue<HMGroupCheckUpdate> m_storeCheckQueue;

    std::shared_timed_mutex m_auxUpdateMutex;
    std::multimap<std::string, HMGroupAuxResult> m_hostGroupAux;
    std::queue<HMGroupAuxUpdate> m_storeAuxQueue;
};

#endif /* INCLUDE_HMSTORAGEHOSTGROUP_H_ */
