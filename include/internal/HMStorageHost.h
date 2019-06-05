// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HMSTORAGEHOST_H_
#define INCLUDE_HMSTORAGEHOST_H_

#include <queue>

#include "HMStorage.h"

// Base class to handle storage per host
/*!
     Base class to handle storage on a per host basis.
     This class handles the book keeping to allow the backend to store information using the individual host as the key.
     In particular, it maps host groups to individual hosts allowing the get host group functions to operate correctly.
     Classes derived from this class will implement the ability to store the information on a per host basis.
     Derived classes must implement:

     clearBackend - Clear the backend data store.
     storeConfigInfo - Store the configuration info class.
     getConfigInfo - Retrieve the configuration info class.

     storeHostGroupNames - Store all the host group names in the back end.
     getHostGroupNames - Get all host group names from the stored configs.
     removeGroupInfo - Remove the host group info for a given host group.

     storeGroupInfo - Store the host group info for a given host group.
     getGroupInfo - Get the group info for a specific group.

     openBackend - Open the backend data store.
     closeBackend - Close the backend data store.

     storeHostNames - Store all the hostnames that we have checks.
     getHostNames - Get all all the hostnames that we have checks.

     storeNameChecks - Store the host check information for a given host name.
     getNameChecks - Get the host check information for a given host name.
     removeNameChecks - Remove the host check information for a given host name.

     storeHostCheckResult - Store the health check results for a given host and check.
     getHostCheckResult - Get the health check results for a given host and check.
     removeHostCheckResult - Remove the health check results for a given host and check.

     storeHostAuxInfo - Store the Aux info for a given host and check.
     getHostAuxInfo - Get the Aux info for a given host and check.
     removeHostAuxInfo - Remove the Aux info for a given host and check.

     storeDNSResult - Store the DNS resolution for a host.
     getDNSResult - Get the DNS resolution for a host.
     removeDNSResult - Remove the DNs resolution for a host.

 */
class HMStorageHost : public HMStorage
{
public:
    HMStorageHost(HMDataHostGroupMap* hostGroupMap) :
        HMStorage(hostGroupMap) {}

    virtual ~HMStorageHost() {};

    //! Sync the local data caches with results from the backend.
    /*!
         Sync the local data caches with results from the backend. Fill in HealthChecks and AuxInfo results. This does not load the configs just the results. Purge results if they are no longer needed in the loaded configs.
          \param the checkList restore.
          \param the DNS cache to restore.
          \param the Aux Cached to restore.
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

    //! Internal function to store the host names to the backend.
    /*!
         Inernal function to store the host names to the backend.
         \param the set of all hostnames to store.
         \return true if the names were stored.
     */
    virtual bool storeHostNames(std::set<std::string>& hostNames) = 0;

    // Internal function to get the host names from the backend.
    /*!
         Internal function to get the host names from the backend.
         \param the set to fill with the hostnames.
         \return true if the hostnames have been stored in the set successfully.
     */
    virtual bool getHostNames(std::set<std::string>& hostNames) = 0;

    // Internal function to get all the host group names.
    /*!
         Internal function to get all the host group names.
         \param the set of all host group names.
         \return true if the host group names were stored successfully.
     */
    virtual bool storeHostGroupNames(std::set<std::string>& hostNames) = 0;

    // Internal function to store the health checks conducted per host name.
    /*!
         Internal function to store the health checks conducted per host name.
         \param host name to use as the store key.
         \param the vector of all health checks (HMCheckHeader)
         \return true if the checks were stored successfully.
     */
    virtual bool storeNameChecks(const std::string& hostName, std::vector<HMCheckHeader>& checks) = 0;

    // Internal function to get the health checks conducted per host name.
    /*!
         Internal function to get the health checks conducted per host name.
         \param host name to use as the store key.
         \param the vector of all health checks (HMCheckHeader)
         \return true if the checks were retrieved successfully.
     */
    virtual bool getNameChecks(const std::string& hostName, std::vector<HMCheckHeader>& checks) = 0;

    // Internal function to remove the health checks conducted per host name.
    /*!
         Internal function to remove the health checks conducted per host name.
         \param host name to use as the store key.
         \param the vector of all health checks to remove (HMCheckHeader)
         \return true if the checks were removed successfully.
     */
    virtual bool removeNameChecks(const std::string& hostName, std::vector<HMCheckHeader>& checks) = 0;

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


    //! Internal function to store the host check result to the backend.
    /*!
         Internal function to store the host check result to the backend.
         \param the HMCheckData to commit to the backend.
         \return true if the check result was updated successfully.
     */
    virtual bool storeHostCheckResult(HMCheckData& checkData) = 0;

    //! Internal function to get the given host check result for the correct host check.
    /*!
         Internal function to get the given host check result for the correct host check.
         \param the HMCheckHeader to retrieve the check result.
         \param the HMDataCheckResult to fill with the check result.
         \return true if the passed check result was filled successfully.
     */
    virtual bool getHostCheckResult(HMCheckHeader& header, HMDataCheckResult& checkResult) = 0;

    //! Internal function to remove the host check result from the backend.
    /*!
         Internal function to remove the host check result from the backend.
         \param the HMCheckHeader to remove from the backend.
         \return true if the host check result was removed successfully.
     */
    virtual bool removeHostCheckResult(HMCheckHeader& header) = 0;

    //! Internal function to store the Aux info result to the backend.
    /*!
         Internal function to store the Aux info result to the backend.
         \param the HMAuxData to commit to the backend.
         \return true if the Aux info was updated successfully.
     */
    virtual bool storeHostAuxInfo(HMAuxData& checkData) = 0;

    //! Internal function to get the given Aux info result for the correct host check.
    /*!
         Internal function to get the given Aux info result for the correct host check.
         \param the HMCheckHeader to retrieve the Aux info.
         \param the HMAuxInfo to fill with the Aux info.
         \return true if the passed Aux info was filled successfully.
     */
    virtual bool getHostAuxInfo(HMCheckHeader& header, HMAuxInfo& auxInfo) = 0;

    //! Internal function to remove the Aux info result from the backend.
    /*!
         Internal function to remove the Aux info result from the backend.
         \param the HMCheckHeader to remove from the backend.
         \return true if the Aux info was removed successfully.
     */
    virtual bool removeHostAuxInfo(HMCheckHeader& header) = 0;

    //! Internal function to store the DNS resolution for the given hostname.
    /*!
         Internal function to store the DNS resolution for the given hostname.
         \param the hostname that was resolved.
         \param the set of HMIPAddresses resolved to that hostname.
         \return true if the hostname resolution was stored successfully.
     */
    virtual bool storeDNSResult(const std::string& hostname, std::set<HMIPAddress>& addresses) = 0;

    //! Internal function to get the DNS resolution for a given hostname.
    /*!
         Internal function to get the DNS resolution for a given hostname.
         \param the hostname to get the addresses.
         \param the set of HMIPAddress to fill.
         \return true if the addresses were retrieved successfully.
     */
    virtual bool getDNSResult(const std::string& hostname, std::set<HMIPAddress>& addresses) = 0;

    //! Internal function to remove the DNS resolution for a given hostname.
    /*!
         Internal function to remove the DNS resolution for a given hostname.
         \param the hostname to remove the addresses.
         \param the set of IPAddresses to remove.
         \return true if the addresses were removed from the backend.
     */
    virtual bool removeDNSResult(const std::string& hostname, std::set<HMIPAddress>& addresses) = 0;

    //! Internal function to remove all entries for the given hostname
    /*!
         Internal function to remove all entries for the given hostname.
         One shot to remove all checks, check results, aux info and DNS results for the given hostname.
         \param the hostname to remove completely from the backend.
     */
    void removeName(const std::string& hostname);

    //! Internal function called from the commit thread to have the derived class write out the saved health checks information to the backend.
    bool commitHealthCheck();
    //! Internal function called from the commit thread to have the derived class write out the savedAux info information to the backend.
    bool commitAuxInfo();

    std::queue<HMCheckData> m_storeCheckQueue;
    std::queue<HMAuxData> m_storeAuxQueue;
};

#endif /* INCLUDE_HMSTORAGEHOSTGROUP_H_ */
