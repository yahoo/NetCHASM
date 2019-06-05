// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSTORAGETEXT_H_
#define HMSTORAGETEXT_H_

#include <sstream>
#include <queue>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "HMStorage.h"
#include "HMStorageHost.h"
#include "HMUtilitySpinLock.h"
#include "HMDataCheckParams.h"


// Storage class to dump to a text file. Primarily useful for debugging and as an example of a storage host class.
/*!
     Dumps to a text file. Is a write only storage class.
 */
class HMStorageHostText : public HMStorageHost
{

public:
    HMStorageHostText(std::string filename, HMDataHostGroupMap* hostGroupMap) :
        HMStorageHost(hostGroupMap),
        m_outputfile(filename) {}

    ~HMStorageHostText(){ closeStore(); }

    //! Clear the backend datastore.
    /*!
         clear the backend datastore and any local caches in the store class.
           \return true if the clear was a success.
     */
    bool clearBackend();

    //! Store the config info into the data store.
    /*!
         Print the config info into the data store.
         \param HMConfigInfo structure
         \return true on success.
    */
    bool storeConfigInfo(const HMConfigInfo& configInfo);

    //! Get the config info from the data store.
    /*!
          Get the config info int the data store.
          \param HMConfigInfo structure
          \return returns false since write only.
    */
    bool getConfigInfo(HMConfigInfo& configInfo);

    //! Get all the host group names from the stored configuration info.
    /*!
         Get all the host group names from the stored configuration info.
         \param the set to store the group names from the backend.
         \return returns false since this is write only data store.
     */
    bool getHostGroupNames(std::set<std::string>& hostGroupNames);

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

    //! Internal function to print the host names to the backend.
    /*!
         Internal function to print the host names to the backend.
         \param the set of all hostnames to print.
         \return true if the names were printed.
     */
    bool storeHostNames(std::set<std::string>& hostNames);

    // Internal function to get the host names from the backend.
    /*!
         Internal function to get the host names from the backend.
         \param the set to fill with the hostnames.
         \return false since the storage class is write only.
     */
    bool getHostNames(std::set<std::string>& hostNames);

    // Internal function to get all the host group names.
    /*!
         Internal function to get all the host group names.
         \param the set of all host group names.
         \return true if the host group names were stored successfully.
     */
    bool storeHostGroupNames(std::set<std::string>& hostGroupNames);

    // Internal function to print the health checks conducted per host name.
    /*!
         Internal function to print the health checks conducted per host name.
         \param host name to use as the store key.
         \param the vector of all health checks (HMCheckHeader)
         \return true if the checks were printed successfully.
     */
    bool storeNameChecks(const std::string& hostName, std::vector<HMCheckHeader>& checks);

    // Internal function to get the health checks conducted per host name.
    /*!
         Internal function to get the health checks conducted per host name.
         \param host name to use as the store key.
         \param the vector of all health checks (HMCheckHeader)
         \return false since this storage class is write only.
     */
    bool getNameChecks(const std::string& hostName, std::vector<HMCheckHeader>& checks);

    // Internal function to remove the health checks conducted per host name.
    /*!
         Internal function to remove the health checks conducted per host name.
         \param host name to use as the store key.
         \param the vector of all health checks to remove (HMCheckHeader)
         \return false since this storage class is write only.
     */
    bool removeNameChecks(const std::string& hostName, std::vector<HMCheckHeader>& checks);

    // Internal function to print the group info per host group.
    /*!
         Internal function to print the group info per host group.
         \param the host group name.
         \param the host group info.
         \return true if the host group info was printed successfully.
     */
    bool storeGroupInfo(const std::string& hostGroupName, HMDataHostGroup& hostGroup);

    // Internal function to remove the group info per host group.
    /*!
         Internal function to remove the group info per host group.
         \param the host group name.
         \return false since this storage class is write only.
     */
    bool removeGroupInfo(const std::string& hostGroupName);

    //! Print the host check result to the backend.
    /*!
         Print the host check result to the backend.
         \param the HMCheckData to commit to the backend.
         \return true if the check result was printed successfully.
     */
    bool storeHostCheckResult(HMCheckData& checkData);

    //! Get the given host check result for the correct host check.
    /*!
         Get the given host check result for the correct host check.
         \param the HMCheckHeader to retrieve the check result.
         \param the HMDataCheckResult to fill with the check result.
         \return false since this storage class is write only.
     */
    bool getHostCheckResult(HMCheckHeader& header, HMDataCheckResult& checkResult);

    //! Remove the host check result from the backend.
    /*!
         Remove the host check result from the backend.
         \param the HMCheckHeader to remove from the backend.
         \return false since this storage class is write only.
     */
    bool removeHostCheckResult(HMCheckHeader& header);

    //! Print the Aux info result to the backend.
    /*!
         Print the Aux info result to the backend.
         \param the HMAuxData to commit to the backend.
         \return true if the Aux info was printed successfully.
     */
    bool storeHostAuxInfo(HMAuxData& checkData);

    //! Get the given Aux info result for the correct host check.
    /*!
         Get the given Aux info result for the correct host check.
         \param the HMCheckHeader to retrieve the Aux info.
         \param the HMAuxInfo to fill with the Aux info.
         \return false since this storage class is write only.
     */
    bool getHostAuxInfo(HMCheckHeader& header, HMAuxInfo& auxInfo);

    //! Remove the Aux info result from the backend.
    /*!
         Remove the Aux info result from the backend.
         \param the HMCheckHeader to remove from the backend.
         \return false since this storage class is write only.
     */
    bool removeHostAuxInfo(HMCheckHeader& header);

    //! Print the DNS resolution for the given hostname.
    /*!
         Print the DNS resolution for the given hostname.
         \param the hostname that was resolved.
         \param the set of HMIPAddresses resolved to that hostname.
         \return true if the hostname resolution was printed successfully.
     */
    bool storeDNSResult(const std::string& hostname, std::set<HMIPAddress>& addresses);

    //! Get the DNS resolution for a given hostname.
    /*!
         Get the DNS resolution for a given hostname.
         \param the hostname to get the addresses.
         \param the set of HMIPAddress to fill.
         \return false since this storage class is write only.
     */
    bool getDNSResult(const std::string& hostname, std::set<HMIPAddress>& addresses);

    //! Remove the DNS resolution for a given hostname.
    /*!
         Remove the DNS resolution for a given hostname.
         \param the hostname to remove the addresses.
         \param the set of IPAddresses to remove.
         \return false since this storage class is write only.
     */
    bool removeDNSResult(const std::string& hostname, std::set<HMIPAddress>& addresses);

private:
    std::string m_outputfile;
    std::ofstream m_fout;
};

#endif /* HMSTORAGETEXT_H_ */
