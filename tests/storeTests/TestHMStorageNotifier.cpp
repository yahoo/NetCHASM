// Copyright (c) 2019 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.

#include <cstdio>

#include "TestHMStorageNotifier.h"
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
TESTNAME::test_HMStorageNotifier_storeCheckResult()
{
    string hostGroupName = "nhg1";
    HMDataHostGroup hostGroup(hostGroupName);
    string host1 = "host1_172_20_10_1";
    hostGroup.addHost(host1);
    HMIPAddress address_1_1;
    address_1_1.set("192.168.0.1");

    HMDataHostGroupMap groupMap;
    groupMap.insert(make_pair(hostGroupName, hostGroup));

    HMDataHostCheck hostCheck;
    hostGroup.getHostCheck(hostCheck);

    HMDataCheckParams checkParams;
    hostGroup.getCheckParameters(checkParams);
    checkParams.addHostGroup(hostGroupName);

    HMDataCheckResult checkResultUp;
    checkResultUp.m_status = HM_HOST_STATUS_UP ;

    HMDNSCache dnsCache;

    HMStorageHostNotifier* store = new HMStorageHostNotifier(&groupMap, &dnsCache);

    CPPUNIT_ASSERT(store->openStore(false));

    // test observer type
    std::shared_ptr<ObserverText> ot = std::make_shared<ObserverText>();
    store->registerObserver( ot );

    CPPUNIT_ASSERT(store->storeCheckResult(host1, address_1_1, hostCheck, checkParams, checkResultUp));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(ot->m_text.find(host1) != std::string::npos);
    CPPUNIT_ASSERT(ot->m_text.find("up") != std::string::npos);
}

