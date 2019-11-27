// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <stdlib.h>
#include <assert.h>

#include "HMStorageHostText.h"
#include "HMAuxCache.h"
#include "HMTimeStamp.h"

using namespace std;

bool
HMStorageHostText::clearBackend()
{
    m_fout.close();
    m_fout.open(m_outputfile.c_str(), ios::out);
    return m_fout.is_open();
}

bool
HMStorageHostText::storeConfigInfo(const HMConfigInfo& configInfo)
{
    if(!m_fout.is_open())
    {
        if(!openBackend())
        {
            return false;
        }
    }

    m_fout << "Config Info" << "\t"
           << (uint32_t)configInfo.m_version << "\t"
           << (uint32_t)configInfo.m_configStatus << "\t"
           << configInfo.m_configLoadTime.getTimeSinceEpoch() << "\n";
    return true;
}

bool
HMStorageHostText::getConfigInfo(HMConfigInfo& configInfo)
{
    (void)configInfo;
    return false;
}

bool
HMStorageHostText::getHostGroupNames(set<string>& hostGroupNames)
{
    (void)hostGroupNames;
    return false;
}

bool
HMStorageHostText::getGroupInfo(const string& hostGroupName, HMDataHostGroup& hostGroup)
{
    (void)hostGroupName;
    (void)hostGroup;
    return false;
}

bool
HMStorageHostText::openBackend()
{
    m_fout.open(m_outputfile.c_str(), ios::ate);
    if(m_fout.is_open())
    {
        return true;
    }
    return false;
}

bool
HMStorageHostText::closeBackend()
{
    m_fout.close();
    return true;
}

bool
HMStorageHostText::storeHostNames(set<string>& hostNames)
{
    if(!m_fout.is_open())
    {
        if(!openBackend())
        {
            return false;
        }
    }

    m_fout << "Storing Host Names:\n";

    for(auto it = hostNames.begin(); it != hostNames.end(); ++it)
    {
        m_fout << *it << "\n";
    }
    m_fout << endl;
    return true;
}

bool
HMStorageHostText::getHostNames(set<string>& hostNames)
{
    (void)hostNames;
    return false;
}

bool
HMStorageHostText::storeHostGroupNames(set<string>& hostGroupNames)
{
    if(!m_fout.is_open())
    {
        if(!openBackend())
        {
            return false;
        }
    }

    m_fout << "Storing Host Group Names:\n";

    for(auto it = hostGroupNames.begin(); it != hostGroupNames.end(); ++it)
    {
        m_fout << *it << "\n";
    }
    m_fout << endl;
    return true;
}

bool
HMStorageHostText::storeNameChecks(const string& hostName, vector<HMCheckHeader>& checks)
{
    if(!m_fout.is_open())
    {
        if(!openBackend())
        {
            return false;
        }
    }

    m_fout << "Storing the check header for " << hostName << "\n";

    for(auto it = checks.begin(); it != checks.end(); ++it)
    {
        m_fout << "HostName:\t" << it->m_hostname << "\n"
               << "IP:\t" << it->m_address.toString() << "\n"
               << "Host Check:\n"
               << "Check Type:\n" << printCheckType(it->m_hostCheck.getCheckType()) << "\n"
               << "Port:\t" << it->m_hostCheck.getPort() << "\n"
               << "Dual Stack:\t" << printDualStack(it->m_hostCheck.getDualStack()) << "\n"
               << "Check Info:\t" << it->m_hostCheck.getCheckInfo() << "\n"
               << "Check Params:\n"
               << it->m_checkParams.printEntry()
               << "Check Params HostGroups:\n"
               << it->m_checkParams.printHostGroups();
    }
    m_fout << endl;
    return true;
}

bool
HMStorageHostText::getNameChecks(const string& hostName, vector<HMCheckHeader>& checks)
{
    (void)hostName;
    (void) checks;
    return false;
}

bool
HMStorageHostText::removeNameChecks(const string& hostName, vector<HMCheckHeader>& checks)
{
    (void)hostName;
    (void) checks;
    return false;
}

bool
HMStorageHostText::storeGroupInfo(const string& hostGroupName, HMDataHostGroup& hostGroup)
{
    if(!m_fout.is_open())
    {
        if(!openBackend())
        {
            return false;
        }
    }

    m_fout << "Storing Group Info for " << hostGroupName << "\n"
           << "Name:\t" << hostGroup.getName() << "\n"
           << "Check Type:\n" << printCheckType(hostGroup.getCheckType()) << "\n"
           << "TTL:\t" << hostGroup.getCheckTTL() << "\n"
           << "Flags:\t" << printMeasurementOptions(hostGroup.getMeasurementOptions()) << "\n"
           << "Dual Stack:\t" << printDualStack(hostGroup.getDualstack()) << "\n"
           << "Port:\t" << hostGroup.getCheckPort() << "\n"
           << "Check Info:\t" << hostGroup.getCheckInfo() << "\n"
           << "Retries:\t" << hostGroup.getNumCheckRetries() << "\n"
           << "Retry Delay:\t" << hostGroup.getCheckRetryDelay() << "\n"
           << "Check Timneout:\t" << hostGroup.getCheckTimeout() << "\n"
           << "Group Threshold:\t" << hostGroup.getGroupThreshold() << "\n"
           << "Smoothing Window:\t" << hostGroup.getSmoothingWindow() << "\n"
           << "Flap Theshold:\t" << hostGroup.getFlapThreshold() << "\n"
           << "Max Flaps:\t" << hostGroup.getMaxFlaps() << "\n"
           << "Slow Threshold:\t" << hostGroup.getSlowThreshold() << "\n"
           << "Mode:\t" << hostGroup.getPassthroughInfo() << "\n"
           << "Hosts:\n";

    for(auto it = hostGroup.getHostList()->begin(); it != hostGroup.getHostList()->end(); ++it)
    {
        m_fout << *it << "\n";
    }
    m_fout << endl;
    return true;
}

bool
HMStorageHostText::removeGroupInfo(const string& hostGroupName)
{
    (void)hostGroupName;
    return false;
}

bool
HMStorageHostText::storeHostCheckResult(HMCheckData& checkData)
{
    if(!m_fout.is_open())
    {   
        if(!openBackend())
        {
            return false;
        }
    }

    m_fout << HMTimeStamp::now().getTimeSinceEpoch() << "\t"
            << checkData.m_checkParams.printHostGroups() << "\t"
            << checkData.m_hostname << "\t"
            << checkData.m_hostCheck.printEntry('\t', true) << "\t"
            << checkData.m_address.toString() << "\t"
            << (uint32_t)checkData.m_result.m_response << "\t"
            << (uint32_t)checkData.m_result.m_reason << "\t"
            << (uint32_t)checkData.m_result.m_softReason << "\t"
            << checkData.m_result.m_start.getTimeSinceEpoch() << "\t"
            << checkData.m_result.m_end.getTimeSinceEpoch() << "\n"
            << "Check Result:" << "\t"
            << checkData.m_result.m_address.toString() << "\t"
            << checkData.m_result.m_maxResponseTime << "\t"
            << checkData.m_result.m_minResponseTime << "\t"
            << checkData.m_result.m_numChecks << "\t"
            << checkData.m_result.m_numConnectFailures << "\t"
            << (uint32_t)checkData.m_result.m_numFailedChecks << "\t"
            << checkData.m_result.m_numFailures << "\t"
            << checkData.m_result.m_numFlaps << "\t"

            << checkData.m_result.m_numResponses << "\t"
            << (uint32_t)checkData.m_result.m_numSlowResponses << "\t"
            << checkData.m_result.m_numTimeouts << "\t"
            << checkData.m_result.m_port << "\t"
            << (uint32_t)checkData.m_result.m_reason << "\t"
            << (uint32_t)checkData.m_result.m_softReason << "\t"
            << checkData.m_result.m_responseTime << "\t"
            << checkData.m_result.m_smoothedResponseTime << "\t"

            << (uint32_t)checkData.m_result.m_status << "\t"
            << checkData.m_result.m_checkTime.getTimeSinceEpoch() << "\t"
            << checkData.m_result.m_sumResponseTime << "\t"
            << checkData.m_result.m_totalResponseTime << "\t"
            << checkData.m_result.m_changeTime.getTimeSinceEpoch() << "\n";

    return true;
}


bool
HMStorageHostText::getHostCheckResult(HMCheckHeader& header, HMDataCheckResult& checkResult)
{
    (void)header;
    (void)checkResult;
    return false;
}

bool
HMStorageHostText::removeHostCheckResult(HMCheckHeader& header)
{
    (void)header;
    return false;
}

bool
HMStorageHostText::storeHostAuxInfo(HMAuxData& checkData)
{
    if(!m_fout.is_open())
    {
        if(!openBackend())
        {
            return false;
        }
    }

    m_fout << "Storing Aux Info for " << checkData.m_hostname << "\n"
           << "Address:\t" << checkData.m_address.toString() << "\n"
           << "URL:\t" << checkData.m_hostCheck.getCheckInfo() << "\nEntries:\n";
    for(auto it = checkData.m_info.m_auxData.begin(); it != checkData.m_info.m_auxData.end(); ++it)
    {
        m_fout << (*it)->print();
    }
    m_fout << endl;
    return true;
}

bool
HMStorageHostText::getHostAuxInfo(HMCheckHeader& header, HMAuxInfo& auxInfo)
{
    (void)header;
    (void)auxInfo;
    return false;
}

bool
HMStorageHostText::removeHostAuxInfo(HMCheckHeader& header)
{
    (void)header;
    return false;
}

bool
HMStorageHostText::storeDNSResult(const string& hostname, set<HMIPAddress>& addresses)
{
    if(!m_fout.is_open())
    {
        if(!openBackend())
        {
            return false;
        }
    }

    m_fout << "Storing Addresses for " << hostname << endl;
    for(auto it = addresses.begin(); it != addresses.end(); ++it)
    {
        m_fout << it->toString() << "\n";
    }
    return true;
}

bool
HMStorageHostText::getDNSResult(const string& hostname, set<HMIPAddress>& addresses)
{
    (void)hostname;
    (void)addresses;
    return false;
}

bool
HMStorageHostText::removeDNSResult(const string& hostname, set<HMIPAddress>& addresses)
{
    (void)hostname;
    (void)addresses;
    return false;
}

