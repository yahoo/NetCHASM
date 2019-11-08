// Copyright (c) 2019 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.

#include <assert.h>
#include <set>
#include <stdlib.h>
#include "HMStorageHostNotifier.h"

using namespace std;

bool
HMStorageHostNotifier::clearBackend()
{
    return true;
}

bool
HMStorageHostNotifier::storeConfigInfo(const HMConfigInfo& configInfo)
{
    (void)configInfo;
    return true;
}

bool
HMStorageHostNotifier::getConfigInfo(HMConfigInfo& configInfo)
{
    (void)configInfo;
    return false;
}

bool
HMStorageHostNotifier::getHostGroupNames(set<string>& hostGroupNames)
{
    (void)hostGroupNames;
    return false;
}

bool
HMStorageHostNotifier::getGroupInfo(const string& hostGroupName, HMDataHostGroup& hostGroup)
{
    (void)hostGroupName;
    (void)hostGroup;
    return false;
}

bool
HMStorageHostNotifier::openBackend()
{
    return true;
}

bool
HMStorageHostNotifier::closeBackend()
{
    return true;
}

bool
HMStorageHostNotifier::storeHostNames(set<string>& hostNames)
{
    (void)hostNames;
    return true;
}

bool
HMStorageHostNotifier::getHostNames(set<string>& hostNames)
{
    (void)hostNames;
    return false;
}

bool
HMStorageHostNotifier::storeHostGroupNames(set<string>& hostGroupNames)
{
    (void)hostGroupNames;
    return true;
}

bool
HMStorageHostNotifier::storeNameChecks(const string& hostName, vector<HMCheckHeader>& checks)
{
    (void)hostName;
    (void)checks;
    return true;
}

bool
HMStorageHostNotifier::getNameChecks(const string& hostName, vector<HMCheckHeader>& checks)
{
    (void)hostName;
    (void)checks;
    return false;
}

bool
HMStorageHostNotifier::removeNameChecks(const string& hostName, vector<HMCheckHeader>& checks)
{
    (void)hostName;
    (void)checks;
    return false;
}

bool
HMStorageHostNotifier::storeGroupInfo(const string& hostGroupName, HMDataHostGroup& hostGroup)
{
    (void)hostGroupName;
    (void)hostGroup;
    return true;
}

bool
HMStorageHostNotifier::removeGroupInfo(const string& hostGroupName)
{
    (void)hostGroupName;
    return false;
}

bool
HMStorageHostNotifier::storeHostCheckResult(HMCheckData& checkData)
{
    lock_guard<mutex> lg(m_observerMutex);
    bool res = true;
    for(auto it = m_observer.begin(); it != m_observer.end(); ++it)
    {
       res |= (*it)->storeHostCheckResult(checkData);
    }
    return res;
}


bool
HMStorageHostNotifier::getHostCheckResult(HMCheckHeader& header, HMDataCheckResult& checkResult)
{
    (void)header;
    (void)checkResult;
    return false;
}

bool
HMStorageHostNotifier::removeHostCheckResult(HMCheckHeader& header)
{
    (void)header;
    return false;
}

bool
HMStorageHostNotifier::storeHostAuxInfo(HMAuxData& checkData)
{
    (void)checkData;
    return true;
}

bool
HMStorageHostNotifier::getHostAuxInfo(HMCheckHeader& header, HMAuxInfo& auxInfo)
{
    (void)header;
    (void)auxInfo;
    return false;
}

bool
HMStorageHostNotifier::removeHostAuxInfo(HMCheckHeader& header)
{
    (void)header;
    return false;
}

bool
HMStorageHostNotifier::storeDNSResult(const string& hostname, set<HMIPAddress>& addresses)
{
    (void)hostname;
    (void)addresses;
    return true;
}

bool
HMStorageHostNotifier::getDNSResult(const string& hostname, set<HMIPAddress>& addresses)
{
    (void)hostname;
    (void)addresses;
    return false;
}

bool
HMStorageHostNotifier::removeDNSResult(const string& hostname, set<HMIPAddress>& addresses)
{
    (void)hostname;
    (void)addresses;
    return false;
}
