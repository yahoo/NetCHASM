// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <cstdio>

#include "TestHMStorageText.h"
#include "common.h"
#include "HMState.h"
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void
TESTNAME::setUp()
{
    setupCommon();
}

void
TESTNAME::tearDown()
{
    teardownCommon();
}

void
TESTNAME::test_HMStorageText_StoreRetrieve()
{
    string hostname = "test.hm.com";
    HMIPAddress address;
    address.set("192.168.0.1");
    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams;
    HMDataCheckResult checkResult;
    HMDataHostGroupMap groupmap;
    checkResult.m_numChecks = 100;
    HMState checkState;
    string filename = "";
    set<HMIPAddress> addresses;
    addresses.insert(address);
    HMDNSCache dnsCache;
    HMStorageHostText* store = new HMStorageHostText(filename, &groupmap, &dnsCache);

    // First test with a bad filename
    CPPUNIT_ASSERT(!store->openStore(false));
    delete store;
    HMDNSLookup dnsHostCheck(HM_DNS_TYPE_LOOKUP, false);
    checkState.m_dnsCache.updateDNSEntry(hostname, dnsHostCheck, addresses);
    filename  = "testfile";
    remove(filename.c_str());
    store = new HMStorageHostText(filename, &groupmap, &dnsCache);

    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigs(checkState));
    CPPUNIT_ASSERT(store->storeCheckResult(hostname, address, hostCheck, checkParams, checkResult));
    CPPUNIT_ASSERT(!store->getCheckResult(hostname, address, hostCheck, checkParams, checkResult));
    store->closeStore();

    delete store;

    string line;
    ifstream fin(filename);

    CPPUNIT_ASSERT(fin.is_open());

    getline(fin, line);
    CPPUNIT_ASSERT(line.find("\t\ttest.hm.com\tCheck Type: default (empty) value\tCheck Info: \tPort: 0\tDual Stack: ipv4-only\t192.168.0.1\t0\t0\t0\t0") != string::npos);
    CPPUNIT_ASSERT(!fin.eof());
    getline(fin, line);
    CPPUNIT_ASSERT(line == "Check Result:\t\t0\t0\t100\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0");
    CPPUNIT_ASSERT(!fin.eof());
    getline(fin, line);
    CPPUNIT_ASSERT(line.find("\ttest.hm.com\t192.168.0.1") != string::npos);
    getline(fin, line);
    CPPUNIT_ASSERT(fin.eof());

    remove(filename.c_str());
}

void
TESTNAME::test_HMStorageText_ConfigStoreRetrieve()
{
    string filename  = "testfile";
    remove(filename.c_str());
    HMDataHostGroupMap hostGroup;
    HMDNSCache dnsCache;
    HMStorageHostText* store = new HMStorageHostText(filename, &hostGroup, &dnsCache);
    HMConfigInfo configInfo;
    HMTimeStamp now;
    now.now();

    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;

    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(!store->getConfigInfo(configInfo));
    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));

    store->closeStore();

    delete store;

    string line;
    ifstream fin(filename);

    CPPUNIT_ASSERT(fin.is_open());

    getline(fin, line);
    string result = "Config Info\t";
    result.append(std::to_string(HM_MDBM_VERSION));
    result.append("\t0\t0");
    CPPUNIT_ASSERT(line == result);
    getline(fin, line);
    CPPUNIT_ASSERT(fin.eof());

    remove(filename.c_str());
}

void
TESTNAME::test_HMStorageText_AuxStoreRetrieve()
{
    string filename  = "testfile";
        string filename1  = "testfile";
    remove(filename.c_str());
    HMDataHostGroupMap groupMap;
    HMDNSCache dnsCache;
    HMStorageHostText* store = new HMStorageHostText(filename, &groupMap, &dnsCache);

    CPPUNIT_ASSERT(store->openStore());

    string host1 = "host1.hm.com";
    string sourceurl = "testing";
    string hostGroupName = "testHostGroup";
    HMIPAddress address_1;
    address_1.set("192.168.0.1");
    HMTimeStamp ts;
    HMTimeStamp updateTime;
    ts.setTime("2018-2-1T0:3:1Z", "%Y-%m-%dT%H:%M:%SZ");
    std::unique_ptr<HMAuxLoadFB> r1 = make_unique<HMAuxLoadFB>();
    r1->m_host ="host1.hm.com";
    r1->m_type = HM_LOAD_FILE;
    r1->m_ip = address_1;
    r1->m_resource ="resource";
    r1->m_ts = ts;
    r1->m_datacenter = "datacenter1";
    r1->m_load = 3;
    r1->m_max = 400;
    r1->m_target =20;
    HMDataHostGroup hostGroup(hostGroupName);
    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams;

    hostGroup.getHostCheck(hostCheck);
    hostGroup.setCheckInfo(sourceurl);
    hostGroup.getCheckParameters(checkParams);
    HMAuxInfo auxInfo, recieveInfo;
    auxInfo.m_auxData.push_back(std::move(r1));

    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1, hostCheck, checkParams,  auxInfo));
    CPPUNIT_ASSERT(!store->getAuxInfo(host1, address_1, hostCheck, checkParams, recieveInfo));

    store->closeStore();
    delete store;

    store = new HMStorageHostText(filename1, &groupMap, &dnsCache);
    CPPUNIT_ASSERT(store->openStore());

    std::unique_ptr<HMAuxOOB> r2 = make_unique<HMAuxOOB>();
    r2->m_host ="host1.hm.com";
    r2->m_type = HM_OOB_FILE;
    r2->m_ip = address_1;
    r2->m_resource ="resource";
    r2->m_ts = ts;
    r2->m_forceDown = HM_OOB_FORCEDOWN_FALSE;
    r2->m_shed = 3;

    auxInfo.m_auxData.clear();
    auxInfo.m_auxData.push_back(std::move(r2));

    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1, hostCheck, checkParams, auxInfo));
    CPPUNIT_ASSERT(!store->getAuxInfo(host1, address_1, hostCheck, checkParams, recieveInfo));

    store->closeStore();
    delete store;

    remove(filename.c_str());
}

