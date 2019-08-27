// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include "HMStorageHostGroup.h"

#ifndef TESTSTORAGEHOSTGROUP_H_
#define TESTSTORAGEHOSTGROUP_H_

class TestStorageHostGroup : public HMStorageHostGroup
{
public:
    TestStorageHostGroup(HMDataHostGroupMap* hostMap, HMDNSCache *dnsCache) :
        HMStorageHostGroup(hostMap, dnsCache) {}

    bool test_getInternalHostGroupInfo(const std::string& hostGroupName, const HMDataHostGroup* &hostGroup)
        { return getInternalHostGroupInfo(hostGroupName, hostGroup); }

    uint32_t test_getInternalHostGroupResults(const std::string& hostGroupName, std::vector<HMGroupCheckResult>& results)
        { return  getInternalCheckResults(hostGroupName, results); }

    bool test_manual_add(HMGroupCheckUpdate& update);
    void populateHostGroup(bool toLocal);
    bool getHostGroupNames(std::set<std::string>& groupNames);
    bool getGroupInfo(const std::string& hostGroupName, HMDataHostGroup& hostGroup);
    bool getConfigInfo(HMConfigInfo& configInfo);

protected:
    bool storeConfigInfo(const HMConfigInfo& configInfo);
    bool retrieveConfigInfo(HMConfigInfo& configInfo);

    bool openBackend() { return true; }
    bool closeBackend() { return true; }

    bool storeHostGroup(const std::string& hostGroup);
    bool getHostGroup(const std::string& hostGroup);
    bool getGroupInfoBackend(const std::string& hostGroupName, HMDataHostGroup& hostGroup);
    bool storeAuxInfo(const std::string& hostname,
            const std::string& sourceURL,
            std::vector<std::unique_ptr<HMAuxBase>>& auxInfo);

    bool retrieveAuxInfo(const std::string& hostname,
            const std::string& sourceURL,
            std::vector<std::unique_ptr<HMAuxBase>>& auxInfo,
            HMTimeStamp& updateTime);

    bool removeHostGroup(const std::string& hostname);

    bool getAllHostGroupNamesBackend(std::vector<std::string>& groupNames);
    bool getHostGroup(const std::string& hostGroup, std::vector<HMGroupCheckResult>& results);
    bool clearBackend();
    bool storeHostGroupNames(std::set<std::string>& groupNames);

    bool storeGroupInfo(const std::string& hostGroupName,
            HMDataHostGroup& hostGroup);
    bool removeGroupInfo(const std::string& hostGroupName);

    bool storeHostGroupCheckResults(const std::string& hostGroup);
    bool getHostGroupCheckResults(const std::string& hostGroup);
    bool getHostGroupCheckResults(const std::string& hostGroup,
            std::vector<HMGroupCheckResult>& results);
    bool removeHostGroupCheckResults(const std::string& hostGroup);

    bool storeHostGroupAuxInfo(const std::string& hostGroup);
    bool getHostGroupGroupAuxInfo(const std::string& hostGroup);
    bool getHostGroupGroupAuxInfo(const std::string& hostGroup,
            std::vector<HMGroupAuxResult>& results);
    bool removeHostGroupGroupAuxInfo(const std::string& hostGroup);
    std::set<std::string> groupNames;
    std::multimap<std::string, HMGroupCheckResult> hostGroupResults;

};




#endif /* TESTSTORAGEHOSTGROUP_H_ */
