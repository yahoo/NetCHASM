// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include "TestStorageHostGroup.h"

using namespace std;


bool
TestStorageHostGroup::storeHostGroup(const string& hostGroup)
{
    groupNames.insert(hostGroup);
    return true;
}

bool
TestStorageHostGroup::getHostGroup(const string& hostGroup)
{
    auto it = groupNames.find(hostGroup);
    return (it != groupNames.end());
}

bool TestStorageHostGroup::getHostGroupNames(std::set<std::string>& groupNames)
{
    groupNames.insert(this->groupNames.begin(), this->groupNames.end());
    return true;
}

bool TestStorageHostGroup::getGroupInfo(const std::string& hostGroupName,
        HMDataHostGroup& hostGroup)
{
    auto it = hostgroupmap->find(hostGroupName);
    if(it != hostgroupmap->end())
    {
        hostGroup = it->second;
        return true;
    }
    return false;
}

bool TestStorageHostGroup::getConfigInfo(HMConfigInfo& configInfo)
{
    return true;
}

bool
TestStorageHostGroup::getGroupInfoBackend(const string& hostGroupName, HMDataHostGroup& hostGroup)
{
    return true;
}

bool
TestStorageHostGroup::test_manual_add(HMGroupCheckUpdate& update)
{
    hostGroupResults.insert(make_pair(update.m_hostGroup, HMGroupCheckResult(update.m_hostName, update.m_address, update.m_result)));
    groupNames.insert(update.m_hostGroup);
    return true;
}

void
TestStorageHostGroup::populateHostGroup(bool toLocal)
{
    if(toLocal)
    {
        hostGroupResults = m_hostGroupResults;
    }
    else
    {
        m_hostGroupResults = hostGroupResults;
    }
}


bool TestStorageHostGroup::storeAuxInfo(const std::string& hostname,
        const std::string& sourceURL,
        std::vector<std::unique_ptr<HMAuxBase> >& auxInfo)
{
    return true;
}

bool TestStorageHostGroup::retrieveAuxInfo(const std::string& hostname,
        const std::string& sourceURL,
        std::vector<std::unique_ptr<HMAuxBase> >& auxInfo,
        HMTimeStamp& updateTime)
{
    return true;
}

bool TestStorageHostGroup::removeHostGroup(const std::string& hostname)
{
    return true;
}

bool TestStorageHostGroup::getAllHostGroupNamesBackend(
        std::vector<std::string>& groupNames)
{
    return true;
}

bool TestStorageHostGroup::getHostGroup(const std::string& hostGroup,
        std::vector<HMGroupCheckResult>& results)
{
    return true;
}

bool TestStorageHostGroup::clearBackend()
{
    return true;
}

bool TestStorageHostGroup::storeHostGroupNames(
        std::set<std::string>& groupNames)
{
    return true;
}

bool TestStorageHostGroup::storeGroupInfo(const std::string& hostGroupName,
        HMDataHostGroup& hostGroup)
{
    return true;
}

bool TestStorageHostGroup::removeGroupInfo(const std::string& hostGroupName)
{
    return true;
}

bool TestStorageHostGroup::storeHostGroupCheckResults(
        const std::string& hostGroup)
{
    return true;
}

bool TestStorageHostGroup::getHostGroupCheckResults(const std::string& hostGroup)
{
    groupNames.insert(hostGroup);
    auto range = hostGroupResults.equal_range(hostGroup);
    for (auto it = range.first; it != range.second; ++it)
    {
        m_hostGroupResults.insert(*it);
    }
    return true;
}


bool TestStorageHostGroup::getHostGroupCheckResults(
        const std::string& hostGroup, std::vector<HMGroupCheckResult>& results)
{
    if(groupNames.find(hostGroup) == groupNames.end())
    {
        return false;
    }
    auto range = hostGroupResults.equal_range(hostGroup);
    for (auto it = range.first; it != range.second; ++it)
    {
        results.push_back(it->second);
    }
    return true;
}

bool TestStorageHostGroup::removeHostGroupCheckResults(
        const std::string& hostGroup)
{
    return true;
}

bool TestStorageHostGroup::storeHostGroupAuxInfo(const std::string& hostGroup)
{
    return true;
}

bool TestStorageHostGroup::getHostGroupGroupAuxInfo(
        const std::string& hostGroup)
{
    return true;
}

bool TestStorageHostGroup::getHostGroupGroupAuxInfo(
        const std::string& hostGroup, std::vector<HMGroupAuxResult>& results)
{
    return true;
}

bool TestStorageHostGroup::removeHostGroupGroupAuxInfo(
        const std::string& hostGroup)
{
    return true;
}

bool TestStorageHostGroup::storeConfigInfo(const HMConfigInfo& configInfo){
        return true;
}
