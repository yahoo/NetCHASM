// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#ifndef TESTSTORAGEHOST_H_
#define TESTSTORAGEHOST_H_

#include "HMStorageHost.h"
class TestStorageHost: public HMStorageHost {
public:
    TestStorageHost(HMDataHostGroupMap *groupMap) :
            HMStorageHost(groupMap)
    {
    }

    std::map<std::string, HMCheckData> results;
    std::multimap<std::string, HMIPAddress> dns;

    bool clearBackend();
    bool storeConfigInfo(const HMConfigInfo& configInfo);
    bool getConfigInfo(HMConfigInfo& configInfo);
    bool getHostGroupNames(std::set<std::string>& groupNames);
    bool getGroupInfo(const std::string& hostGroupName,
            HMDataHostGroup& hostGroup);

protected:
    bool openBackend();
    bool closeBackend();

    bool storeHostNames(std::set<std::string>& hostNames);
    bool getHostNames(std::set<std::string>& hostNames);

    bool storeHostGroupNames(std::set<std::string>& hostNames);

    bool storeNameChecks(const std::string& hostName,
            std::vector<HMCheckHeader>& checks);
    bool getNameChecks(const std::string& hostName,
            std::vector<HMCheckHeader>& checks);
    bool removeNameChecks(const std::string& hostName,
            std::vector<HMCheckHeader>& checks);

    bool storeGroupInfo(const std::string& hostGroupName,
            HMDataHostGroup& hostGroup);
    bool removeGroupInfo(const std::string& hostGroupName);

    bool storeHostCheckResult(HMCheckData& checkData);
    bool getHostCheckResult(HMCheckHeader& header,
            HMDataCheckResult& checkResult);
    bool removeHostCheckResult(HMCheckHeader& header);

    bool storeHostAuxInfo(HMAuxData& checkData);
    bool getHostAuxInfo(HMCheckHeader& header, HMAuxInfo& auxInfo);
    bool removeHostAuxInfo(HMCheckHeader& header);

    bool storeDNSResult(const std::string& hostname,
            std::set<HMIPAddress>& addresses);
    bool getDNSResult(const std::string& hostname,
            std::set<HMIPAddress>& addresses);
    bool removeDNSResult(const std::string& hostname,
            std::set<HMIPAddress>& addresses);

};

#endif /* TESTSTORAGEHOST_H_ */
