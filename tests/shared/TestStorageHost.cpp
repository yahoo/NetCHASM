// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include "TestStorageHost.h"
using namespace std;

bool TestStorageHost::clearBackend()
{
    return true;
}

bool TestStorageHost::storeConfigInfo(const HMConfigInfo& configInfo)
{
    return true;
}

bool TestStorageHost::getConfigInfo(HMConfigInfo& configInfo)
{
    return true;
}

bool TestStorageHost::getHostGroupNames(std::set<std::string>& groupNames)
{
    return true;
}

bool TestStorageHost::getGroupInfo(const std::string& hostGroupName,
        HMDataHostGroup& hostGroup)
{
    return true;
}

bool TestStorageHost::openBackend()
{
    return true;
}

bool TestStorageHost::closeBackend()
{
    return true;
}

bool TestStorageHost::storeHostNames(std::set<std::string>& hostNames)
{
    return true;
}

bool TestStorageHost::getHostNames(std::set<std::string>& hostNames)
{
    return true;
}

bool TestStorageHost::storeHostGroupNames(std::set<std::string>& hostNames)
{
    return true;
}

bool TestStorageHost::storeNameChecks(const std::string& hostName,
        std::vector<HMCheckHeader>& checks)
{
    return true;
}

bool TestStorageHost::getNameChecks(const std::string& hostName,
        std::vector<HMCheckHeader>& checks)
{
    return true;
}

bool TestStorageHost::removeNameChecks(const std::string& hostName,
        std::vector<HMCheckHeader>& checks)
{
    return true;
}

bool TestStorageHost::storeGroupInfo(const std::string& hostGroupName,
        HMDataHostGroup& hostGroup)
{
    return true;
}

bool TestStorageHost::removeGroupInfo(const std::string& hostGroupName)
{
    return true;
}

bool TestStorageHost::storeHostCheckResult(HMCheckData& checkData)
{
    results.insert(make_pair(checkData.m_hostname, checkData));
    return true;
}

bool TestStorageHost::getHostCheckResult(HMCheckHeader& header,
        HMDataCheckResult& checkResult)
{
    auto it = results.find(header.m_hostname);
    if (it != results.end())
    {
        checkResult = it->second.m_result;
        return true;
    }
    return false;
}

bool TestStorageHost::removeHostCheckResult(HMCheckHeader& header)
{
    return true;
}

bool TestStorageHost::storeHostAuxInfo(HMAuxData& checkData)
{
    return true;
}

bool TestStorageHost::getHostAuxInfo(HMCheckHeader& header, HMAuxInfo& auxInfo)
{
    return true;
}

bool TestStorageHost::removeHostAuxInfo(HMCheckHeader& header)
{
    return true;
}

bool TestStorageHost::removeDNSResult(const std::string& hostname,
        std::set<HMIPAddress>& addresses)
{
    return true;
}

bool
TestStorageHost::storeDNSResult(const string& hostname,
        std::set<HMIPAddress>& addresses)
{
    for(HMIPAddress address: addresses)
    {
        dns.insert(make_pair(hostname, address));
    }
    return true;
}

bool
TestStorageHost::getDNSResult(const string& hostname, set<HMIPAddress>& addresses)
{
    addresses.clear();
    auto range = dns.equal_range(hostname);

    for(auto it = range.first; it != range.second; ++it)
    {
        addresses.insert(it->second);
    }

    return (addresses.size() > 0)?true:false;
}

