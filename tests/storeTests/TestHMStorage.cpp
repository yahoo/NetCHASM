// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMStorage.h"

#include "TestStorage.h"
#include "common.h"

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    setupCommon();
}

void TESTNAME::tearDown()
{
    teardownCommon();
}


void TESTNAME::test_HMStorage_Construction()
{
    string hostname = "test.hm.com";
    HMIPAddress address;
    address.set("192.168.0.1");

    HMConfigInfo configInfo;
    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams;
    HMDataCheckResult result;

    HMCheckData checkData(hostname, address, hostCheck, checkParams, result);
    CPPUNIT_ASSERT(checkData.m_address == address);
    CPPUNIT_ASSERT(checkData.m_checkParams == checkParams);
    CPPUNIT_ASSERT(checkData.m_hostCheck == hostCheck);
    CPPUNIT_ASSERT(checkData.m_hostname == hostname);
    CPPUNIT_ASSERT(checkData.m_result == result);

    HMCheckHeader checkHeader(hostname, address, hostCheck, checkParams);
    CPPUNIT_ASSERT(checkHeader.m_address == address);
    CPPUNIT_ASSERT(checkHeader.m_checkParams == checkParams);
    CPPUNIT_ASSERT(checkHeader.m_hostCheck == hostCheck);
    CPPUNIT_ASSERT(checkHeader.m_hostname == hostname);
    HMDataHostGroupMap hostGroupMap;
    HMDNSCache dnsCache;
    TestStorage myStorage(&hostGroupMap, &dnsCache);
}

void TESTNAME::test_HMStorage_OpenClose()
{
    string hostname = "test.hm.com";
    HMIPAddress address;
    address.set("192.168.0.1");

    HMDataCheckList checkList;
    HMDNSCache dnsCache;
    HMConfigInfo configInfo;
    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams;
    HMDataCheckResult result;
    HMDataHostGroupMap hostGroupMap;

    TestStorage* readOnlyStorage = new TestStorage(&hostGroupMap, &dnsCache);
    TestStorage* rwStorage = new TestStorage(&hostGroupMap, &dnsCache);

    readOnlyStorage->m_commitCalls = 10;
    CPPUNIT_ASSERT(readOnlyStorage->openStore(true));

    rwStorage->m_commitCalls = 10;
    CPPUNIT_ASSERT(rwStorage->openStore(false));

    std::this_thread::sleep_for(10ms);

    CPPUNIT_ASSERT(!readOnlyStorage->storeCheckResult(hostname, address, hostCheck, checkParams, result));
    CPPUNIT_ASSERT(readOnlyStorage->getCheckResult(hostname, address, hostCheck, checkParams, result));
    CPPUNIT_ASSERT(!readOnlyStorage->storeConfigInfo(configInfo));
    CPPUNIT_ASSERT(readOnlyStorage->getConfigInfo(configInfo));
    //CPPUNIT_ASSERT(readOnlyStorage->restoreResults(checkList, dnsCache));
    CPPUNIT_ASSERT(readOnlyStorage->m_commitCalls == 10);

    CPPUNIT_ASSERT(rwStorage->storeCheckResult(hostname, address, hostCheck, checkParams, result));
    CPPUNIT_ASSERT(rwStorage->getCheckResult(hostname, address, hostCheck, checkParams, result));
    CPPUNIT_ASSERT(rwStorage->storeConfigInfo(configInfo));
    CPPUNIT_ASSERT(rwStorage->getConfigInfo(configInfo));
    //CPPUNIT_ASSERT(rwStorage->restoreResults(checkList, dnsCache));

    CPPUNIT_ASSERT_EQUAL(0, (int)rwStorage->m_commitCalls);

    readOnlyStorage->closeStore();
    rwStorage->closeStore();

    delete readOnlyStorage;
    delete rwStorage;
}
