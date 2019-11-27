// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTSTORAGE_H_
#define TESTSTORAGE_H_

#include "HMStorage.h"

class TestStorage : public HMStorage
{
public:
    TestStorage(HMDataHostGroupMap* hostGroupMap, HMDNSCache *dnsCache) :
            HMStorage(hostGroupMap, dnsCache){ m_commitCalls = 0;}

    virtual ~TestStorage() {};

    void initResultsFromBackend(HMDataCheckList& checkList, HMDNSCache& dnsCache, HMAuxCache& auxCache);

    //! Clear the backend datastore.
    /*! clear the backend datastore and any local caches in the store class.
       \return bool indicating success.
     */
    bool clearBackend();

    // Functions to store and retrieve config information

    //! Store the config info into the data store.
    /*! Store the config info int the data store.
      \param HMConfigInfo structure
      \return true on success.
     */
    bool storeConfigInfo(const HMConfigInfo& configInfo);

    //! Get the config info into the data store.
    /*! Get the config info int the data store.
          \param HMConfigInfo structure
          \return true on success.
     */
    bool getConfigInfo(HMConfigInfo& configInfo);

    bool storeConfigs(HMState& checkState);
    bool getConfigs(HMState& checkState);

    // Functions to hand health check information
    bool storeCheckResult(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            const HMDataCheckResult& checkResult);

    bool getCheckResult(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            HMDataCheckResult& checkResult);

    bool purgeCheckResult(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams);

    // Functions to deal with aux info
    // Function
    bool storeAuxInfo(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            const HMAuxInfo& auxInfo);

    bool getAuxInfo(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams,
            HMAuxInfo& auxInfo);

    bool purgeAuxInfo(const std::string& hostname,
            const HMIPAddress& address,
            const HMDataHostCheck& hostCheck,
            const HMDataCheckParams& checkParams);


    bool getDNS(const std::string& hostname, std::set<HMIPAddress>& ips);

    // These functions are for copying state to the internal data structures during a reload
    bool updateCheckResultCache(HMCheckHeader& header, HMDataCheckResult& result);
    bool updateAuxInfoCache(HMCheckHeader& header, HMAuxInfo& aux);

    void updateHostGroups(std::set<std::string>& hostGroups);

    bool getGroupCheckResults(const std::string& groupName,
            bool noCache,
            bool onlyResolved,
            std::vector<HMGroupCheckResult>& results);

    bool getGroupAuxInfo(const std::string& groupName,
                bool noCache,
                bool onlyResolved,
                std::vector<HMGroupAuxResult>& results);

    bool getHostGroupNames(std::set<std::string>& groupNames);
    bool getGroupInfo(const std::string& hostGroupName, HMDataHostGroup& hostGroup);

    uint32_t m_writeCount;
    std::string m_hostname;
    HMIPAddress m_address;
    HMDataHostCheck m_hostCheck;
    uint32_t m_response;
    uint32_t m_reason;
    HMTimeStamp m_start;
    HMTimeStamp m_end;

    HMDataCheckResult m_checkResult;
    HMDataCheckParams m_checkParams;

    uint32_t m_commitCalls;
protected:
    bool openBackend();
    bool closeBackend();
    bool commitHealthCheck();
    bool commitAuxInfo();
};


#endif /* TESTSTORAGE_H_ */
