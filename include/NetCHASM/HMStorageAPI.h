// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSTORAGEAPI_H_
#define HMSTORAGEAPI_H_

#include <set>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

#include "HMAPI.h"

class HMStorageAPI
{
public:

    HMStorageAPI();
    ~HMStorageAPI();

    //! Init function for the storage API library.
    /*!
         Init function for the storage API library.
         Loads the configs into an internal state for queries. Can be called multiple times to sync config changes from backend.
         Can be used to load a local configuration or with the configuration from the backend.
         *Warning. Using a local configuration may not give accurate results from the backend. ie if the local configs do not
         contain the group names, the backend may not be queried.
         \param the master config to use to find the backend storage.
         \param true will force the init to use the configs in the master config. False will use the config information in the backend data store.
         \return true if the library loaded correctly and is ready.
     */
    bool init(const std::string& masterConfig, bool localConfigs);

    //! Grab the latest version of the loaded state.
    /*!
         Grab the latest version of the loaded state.
         Calling the update function at query times allows the api to finish and smoothly switch to a new state.
         During a update of configs, the reloader will only garbage collect when the work has updated the shared pointer.
     */
    bool updateState(std::shared_ptr<HMState>& current);

    //! Get the config info from the backend.
    /*!
         Get the config info from the backend.
         \param the HMAPIConfigInfo struct to fill.
         \return true if the struct was filled correctly.
     */
    bool getConfigInfo(HMAPIConfigInfo& result);

    /*!
     verifies if the hash API holds is the same as in backend.
     \return true if the both API and backend hash matches.
     */
    bool validateHash();

    /*!
     updates configs if they differ from what is stored in backend.
     \return true if the update of configs to backend configs is successful.
     */
    bool updateConfigs();

    //! Parse the local configs to see if they are valid.
    /*!
         Parse the local configs to see if they are valid.
         \param the master config to use to parse the configs.
         \bool true to set the debug output to debug3 (verbose). False sets to the default notification level.
     */
    bool validateConfigs(const std::string& masterConfig, bool verbose);

    //! Get all the hosts with data in the backend.
    /*!
         Get all the hosts with data in the backend.
         \param a set of strings to fill with the list of hosts.
         \returen true if the list was filled successfully.
     */
    bool getAllHosts(std::set<std::string>& hosts);

    //! Get all the host checks conducted for a specific host.
    /*!
         Get all the host checks conducted for a specific host.
         \param the host to get the host checks.
         \param a vector of HMAPICheckInfo to fill with the host check information.
         \return true if the vector was filled successfully.
     */
    bool getHostChecks(const std::string& host, std::vector<HMAPICheckInfo>& checks);

    //! Get all the host groups with data in the backend.
    /*!
         Get all the host groups with data in the backend.
         \param a set of strings to fill with the list of host groups.
         \returen true if the list was filled successfully.
     */
    bool getAllHostGroupNames(std::set<std::string>& groupNames);

    //! Get all the group's host checks conducted for a specific host.
    /*!
         Get all the group's host checks conducted for a specific host.
         \param the host group to get the host checks.
         \param a vector of HMAPICheckInfo to fill with the host check information.
         \return true if the vector was filled successfully.
     */
    bool getHostGroupInfo(const std::string& group, HMAPICheckInfo& groupInfo, std::vector<std::string>& hosts);

    //! Get the host check results for a given host.
    /*!
         Get the host check results for a given host.
         \param the host to retrieve the check results.
         \param a vector of HMAPICheckResult to store the check information.
         \param true if the API should only return the resolved names. False returns blank names for checks not completed yet.
         \return true if the vector is filled with check results.
     */
    bool getHostResults(const std::string& name, std::vector<HMAPICheckResult>& results, bool onlyResolved);

    //! Get the Aux info for a given host name.
    /*!
         Get the Aux info for a given host name.
         \param the host to use as the key to get the Aux info.
         \param a vector of HMAuxInfo to store the Aux Info.
         \return true if the vector is filled with the host's Aux info.
     */
    bool getHostAuxInfo(const std::string& name, std::vector<HMAPIAuxInfo>& auxInfo);

    //! Get the host check results for a given host group.
    /*!
         Get the host check results for a given host group.
         \param the host group to retrieve the check results.
         \param a vector of HMAPICheckResult to store the check information.
         \param true if the API should only return the resolved names. False returns blank names for checks not completed yet.
         \return true if the vector is filled with check results.
     */
    bool getHostGroupResults(const std::string& name, std::vector<HMAPICheckResult>& results, bool onlyResolved);

    //! Get the Aux info for a given host group.
    /*!
         Get the Aux info for a given host group.
         \param the host group to use as the key to get the Aux info.
         \param a vector of HMAuxInfo to store the Aux Info.
         \return true if the vector is filled with the host's Aux info.
     */
    bool getHostGroupAuxInfo(const std::string& name, std::vector<HMAPIAuxInfo>& auxInfo);

    //! Get the IP addresses resolved to a given host name.
    /*!
         Get the IP addresses resolved to a given host name.
         \param the host name to lookup.
         \param a set of HMAPIIPAddress to store the addresses.
         \return true if the struct is filled with the resolved addresses.
     */
    bool getDNS(const std::string& host, std::set<HMAPIIPAddress> ips);

    //! Get the errors associated with the last call.
    /*!
         Get the errors associated with the last call. All errors are stored in an internal buffer. If the function returns false,
         call this function to get the error that occurred.
         \return the error that occured during the previous function call.
     */
    std::string getError();

    //! Generate the Aux XML based on the group Aux Info in the backend.
    /*!
         Generate the Aux XML based on the group Aux Info in the backend.
         \param the group name to dump the XML.
         \param a vector of strings to save the XML contents generated by the call.
         \param the TTL of the XML files.
         \param the Timeout of the XML files.
         \return True if the data was output successfully.
     */
    bool generateAuxXML(const std::string& name, std::vector<std::string>& fileContents, uint64_t& ttl, uint64_t& timeout);

    //! Generate and write the Aux XML based on the group Aux Info in the disk.
    /*!
         Generate the Aux XML based on the group Aux Info in the backend.
         \param the group name to dump the XML.
         \param a vector of strings to save the XML files generated by the call.
     */
    void writeAuxXML(const std::string& name, std::vector<std::string>& fileNames);


    //! Create a copy of the stored configs to a file.
    /*!
         Create a copy of the stored configs to a file.
         \param the config path to create.
         \return true if the config was written correctly.
     */
    bool generateConfigs(const std::string& configName);

    //! Dump the configs from backend to file.
    /*
         Dump the configs from backend to file.
         \param config format.
         \param filename where the configs need to be written.
     */
    bool writeConfigs(HM_API_CONFIG_CLASS configs, const std::string& fileName);

private:

    //! Internal function to write the xml to a file.
    /*!
         Internal function to write the xml to a file.
         \param the file name to create.
         \param the xml to write to the file.
         \return true if the file was created correctly.
     */
    bool writeXML(std::string& fileName, std::string& xml);

private:

    bool m_loaded;
    std::shared_ptr<HMLogBase> log;
    std::shared_ptr<HMState> m_currentState;
    std::mutex m_reloadConfigsMutex;
};


#endif /* HMSTORAGEAPI_H_ */
