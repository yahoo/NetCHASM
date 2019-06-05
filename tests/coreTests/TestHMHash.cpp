// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMHash.h"
#include "common.h"

#include <string>
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void
TESTNAME::setUp()
{}

void
TESTNAME::tearDown()
{}

void
TESTNAME::TestHash_match()
{
    string host1 = "test.dummy.hm.com";
    string host2 = "test2.dummy.hm2.com";
    string host3 = "test3.dummy.hm3.com";
    string  hostGroupName = "test.hmdummy.net";
    HMDataHostGroupMap groupMap;
    string checkInfo = "/dummyinfo";
    HMDataHostGroup hostGroup(hostGroupName);
    hostGroup.setPassthroughInfo(0);
    hostGroup.setCheckType(HM_CHECK_HTTP);
    hostGroup.setGroupThreshold(50);
    hostGroup.setCheckTTL(30);
    hostGroup.setPort(8080);
    hostGroup.setCheckInfo(checkInfo);
    hostGroup.addHost(host1);
    hostGroup.addHost(host2);
    hostGroup.addHost(host3);
    
    HMHashMD5 hash;
    hash.init();
    hostGroup.getHash(hash);
    
    HMDataHostGroup hostGroup2(hostGroupName);
    hostGroup2.setPassthroughInfo(0);
    hostGroup2.setCheckType(HM_CHECK_HTTP);
    hostGroup2.setGroupThreshold(50);
    hostGroup2.setCheckTTL(30);
    hostGroup2.setPort(8080);
    hostGroup2.setCheckInfo(checkInfo);
    hostGroup2.addHost(host1);
    hostGroup2.addHost(host2);
    hostGroup2.addHost(host3);
    
    HMHashMD5 hash2;
    HMHash h1,h2;
    hash2.init();
    hostGroup2.getHash(hash2);
    
    hash.final(h1);
    hash2.final(h2);

    CPPUNIT_ASSERT(h1 == h2);
}


void
TESTNAME::TestHash_mismatch()
{
    string host1 = "test.dummy.hm.com";
    string host2 = "test2.dummy.hm2.com";
    string host3 = "test3.dummy.hm3.com";
    string  hostGroupName = "test.hmdummy.net";
    HMDataHostGroupMap groupMap;
    string checkInfo = "/dummyinfo";
    HMDataHostGroup hostGroup(hostGroupName);
    hostGroup.setPassthroughInfo(0);
    hostGroup.setCheckType(HM_CHECK_HTTP);
    hostGroup.setGroupThreshold(50);
    hostGroup.setCheckTTL(30);
    hostGroup.setPort(8010);
    hostGroup.setCheckInfo(checkInfo);
    hostGroup.addHost(host1);
    hostGroup.addHost(host2);
    hostGroup.addHost(host3);
    HMHashMD5 hash;
    hash.init();
    hostGroup.getHash(hash);
    
    HMDataHostGroup hostGroup2(hostGroupName);
    hostGroup2.setPassthroughInfo(0);
    hostGroup2.setCheckType(HM_CHECK_HTTP);
    hostGroup2.setGroupThreshold(50);
    hostGroup2.setCheckTTL(60);
    hostGroup2.setPort(8050);
    hostGroup2.setCheckInfo(checkInfo);
    hostGroup2.addHost(host1);
    hostGroup2.addHost(host2);
    hostGroup2.addHost(host3);
    HMHashMD5 hash2;
    HMHash h1, h2;
    hash2.init();
    hostGroup2.getHash(hash2);
    hash.final(h1);
    hash2.final(h2);

    CPPUNIT_ASSERT(h1 != h2);
}
