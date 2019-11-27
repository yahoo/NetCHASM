// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.


#include "TestStorage.h"
using namespace std;


void TestStorage::initResultsFromBackend(HMDataCheckList& checkList,
        HMDNSCache& dnsCache, HMAuxCache& auxCache)
{
}

bool TestStorage::clearBackend()
{
    return true;
}

bool TestStorage::storeConfigInfo(const HMConfigInfo& configInfo)
{
    return m_readonly?false:true;
}

bool TestStorage::getConfigInfo(HMConfigInfo& configInfo)
{
    return true;
}


bool TestStorage::storeConfigs(HMState& checkState)
{
    return true;
}

bool TestStorage::getConfigs(HMState& checkState)
{
    return true;
}

bool
TestStorage::storeCheckResult(const std::string& hostname,
        const HMIPAddress& address,
        const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams,
        const HMDataCheckResult& checkResult)
{
    m_writeCount++;
    m_hostname = hostname;
    m_address = address;
    m_hostCheck = hostCheck;
    m_response = checkResult.m_response;
    m_reason = checkResult.m_reason;
    m_start = checkResult.m_start;
    m_end = checkResult.m_end;

    m_checkResult = checkResult;
    m_checkParams = checkParams;

    return m_readonly?false:true;
}

bool TestStorage::getCheckResult(const std::string& hostname,
        const HMIPAddress& address, const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams, HMDataCheckResult& checkResult)
{
    return true;
}

bool TestStorage::purgeCheckResult(const std::string& hostname,
        const HMIPAddress& address, const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams)
{
    return true;
}

bool TestStorage::storeAuxInfo(const std::string& hostname,
        const HMIPAddress& address, const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams, const HMAuxInfo& auxInfo)
{
    return true;
}

bool TestStorage::getAuxInfo(const std::string& hostname,
        const HMIPAddress& address, const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams, HMAuxInfo& auxInfo)
{
    return true;
}

bool TestStorage::purgeAuxInfo(const std::string& hostname,
        const HMIPAddress& address, const HMDataHostCheck& hostCheck,
        const HMDataCheckParams& checkParams)
{
    return true;
}

bool TestStorage::getDNS(const std::string& hostname, std::set<HMIPAddress>& ips)
{
    return true;
}

bool TestStorage::updateCheckResultCache(HMCheckHeader& header,
        HMDataCheckResult& result)
{
    return true;
}

bool TestStorage::updateAuxInfoCache(HMCheckHeader& header, HMAuxInfo& aux)
{
    return true;
}

void TestStorage::updateHostGroups(std::set<std::string>& hostGroups)
{

}

bool TestStorage::getGroupCheckResults(const std::string& groupName,
        bool noCache, bool onlyResolved,
        std::vector<HMGroupCheckResult>& results)
{
    return true;
}

bool TestStorage::getGroupAuxInfo(const std::string& groupName, bool noCache,
        bool onlyResolved, std::vector<HMGroupAuxResult>& results)
{
    return true;
}

bool TestStorage::getHostGroupNames(std::set<std::string>& groupNames)
{
    return true;
}

bool TestStorage::getGroupInfo(const std::string& hostGroupName,
        HMDataHostGroup& hostGroup)
{
    return true;
}

bool TestStorage::openBackend()
{
    return true;
}

bool TestStorage::closeBackend()
{
    return true;
}

bool TestStorage::commitHealthCheck()
{
     if(m_commitCalls > 0) {
         m_commitCalls--;
         return true;
     }
    return false;;
}

bool TestStorage::commitAuxInfo()
{
    return false;
}


