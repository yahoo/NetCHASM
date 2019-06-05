// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMStorageHostGroupMDBM.h"
#include "common.h"
#include "HMState.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);
using namespace std;
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
TESTNAME::test_HMStorageHostGroupYForMDBM_Construction()
{
    HMConfigInfo configInfo;
    HMConfigInfo retConfigInfo;

    HMTimeStamp now;
    now.now();
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;

    string data;
    data.resize(configInfo.serialize(nullptr, 0));
    CPPUNIT_ASSERT_EQUAL(configInfo.serialize(&data.at(0), data.size()), (uint32_t)data.size());
    CPPUNIT_ASSERT(retConfigInfo.deserialize(&data.at(0), data.size()));
    CPPUNIT_ASSERT_EQUAL(configInfo.m_version , retConfigInfo.m_version);
    CPPUNIT_ASSERT_EQUAL(configInfo.m_configStatus, retConfigInfo.m_configStatus);
    CPPUNIT_ASSERT_EQUAL(configInfo.m_configLoadTime.getTimeSinceEpoch(), retConfigInfo.m_configLoadTime.getTimeSinceEpoch());

    HMDataHostGroup hostGroup("hostGroup1");
    HMDataHostGroup testHostGroup("hostGroup1");
    hostGroup.setMeasurementOptions(1);
    hostGroup.setCheckType(HM_CHECK_HTTP);
    hostGroup.setCheckTTL(60000);
    hostGroup.setCheckTimeout(1000);
    hostGroup.setGroupThreshold(5);
    hostGroup.setFlapThreshold(5);
    hostGroup.setMaxFlaps(5);
    hostGroup.setPassthroughInfo(3);
    hostGroup.setSmoothingWindow(10);
    hostGroup.setPort(80);
    hostGroup.setNumCheckRetries(3);
    hostGroup.setSlowThreshold(30);
    uint32_t size = hostGroup.serialize(nullptr, 0);
    data = "";
    data.resize(size);

    CPPUNIT_ASSERT_EQUAL(hostGroup.serialize(&data.at(0), size) , size);

    CPPUNIT_ASSERT(testHostGroup.deserialize(&data.at(0), data.size()));
    CPPUNIT_ASSERT(testHostGroup.getMeasurementOptions() == hostGroup.getMeasurementOptions());
    CPPUNIT_ASSERT(testHostGroup.getCheckType() == hostGroup.getCheckType());
    CPPUNIT_ASSERT(testHostGroup.getCheckTTL() == hostGroup.getCheckTTL());

    CPPUNIT_ASSERT(testHostGroup.getCheckTimeout() == hostGroup.getCheckTimeout());
    CPPUNIT_ASSERT(testHostGroup.getGroupThreshold() == hostGroup.getGroupThreshold());
    CPPUNIT_ASSERT(testHostGroup.getFlapThreshold() == hostGroup.getFlapThreshold());
    CPPUNIT_ASSERT(testHostGroup.getMaxFlaps() == hostGroup.getMaxFlaps());
    CPPUNIT_ASSERT(testHostGroup.getPassthroughInfo() == hostGroup.getPassthroughInfo());
    CPPUNIT_ASSERT(testHostGroup.getSmoothingWindow() == hostGroup.getSmoothingWindow());
    CPPUNIT_ASSERT(testHostGroup.getCheckPort() == hostGroup.getCheckPort());
    CPPUNIT_ASSERT(testHostGroup.getSlowThreshold() == hostGroup.getSlowThreshold());

    HMIPAddress address;
    HMTimeStamp timeStamp;
    timeStamp.now();
    address.set("192.168.0.1");
    HMDataCheckResult result;
    HMDataCheckResult testResult;
    result.m_address = address;
    result.m_responseTime = 1;
    result.m_totalResponseTime = 2;
    result.m_minResponseTime = 3;
    result.m_maxResponseTime = 4;
    result.m_smoothedResponseTime = 5;
    result.m_sumResponseTime = 6;
    result.m_numChecks = 7;
    result.m_numResponses = 8;
    result.m_numConnectFailures = 11;
    result.m_numFailures = 12;
    result.m_numTimeouts = 13;
    result.m_numFlaps = 15;
    result.m_numFailedChecks = 16;
    result.m_numSlowResponses = 17;
    result.m_status = HM_HOST_STATUS_UP;
    result.m_reason = HM_REASON_SUCCESS;
    result.m_port = 22;
    result.m_changeTime = timeStamp;
    result.m_checkTime = timeStamp;

    size = result.serialize(nullptr, 0);
    data = "";
    data.resize(size);
    CPPUNIT_ASSERT_EQUAL(result.serialize(&data.at(0), size), size);
    CPPUNIT_ASSERT(testResult.deserialize(&data.at(0), data.size()));
    CPPUNIT_ASSERT(testResult.m_address == address);
    CPPUNIT_ASSERT(testResult.m_responseTime == 1);
    CPPUNIT_ASSERT(testResult.m_totalResponseTime == 2);
    CPPUNIT_ASSERT(testResult.m_minResponseTime == 3);
    CPPUNIT_ASSERT(testResult.m_maxResponseTime == 4);
    CPPUNIT_ASSERT(testResult.m_smoothedResponseTime == 5);
    CPPUNIT_ASSERT(testResult.m_sumResponseTime == 6);
    CPPUNIT_ASSERT(testResult.m_numChecks == 7);
    CPPUNIT_ASSERT(testResult.m_numResponses == 8);
    CPPUNIT_ASSERT(testResult.m_numConnectFailures == 11);
    CPPUNIT_ASSERT(testResult.m_numFailures == 12);
    CPPUNIT_ASSERT(testResult.m_numTimeouts == 13);
    CPPUNIT_ASSERT(testResult.m_numFlaps == 15);
    CPPUNIT_ASSERT(testResult.m_numFailedChecks == 16);
    CPPUNIT_ASSERT(testResult.m_numSlowResponses == 17);
    CPPUNIT_ASSERT(testResult.m_status == HM_HOST_STATUS_UP);
    CPPUNIT_ASSERT(testResult.m_reason == HM_REASON_SUCCESS);
    CPPUNIT_ASSERT(testResult.m_port == 22);
    CPPUNIT_ASSERT(testResult.m_changeTime == timeStamp);
    CPPUNIT_ASSERT(testResult.m_checkTime == timeStamp);
}


void
TESTNAME::test_HMStorageHostGroupYForMDBM_TestRT()
{
    string hostGroupName = "group_Name";

    HMDataHostGroupMap groupMap;

    // Now create the hosts and add them to the group
    string host1 = "host1.hm.com";
    string host2 = "host2.hm.com";
    string host3 = "host3.hm.com";
    string host4 = "host4.hm.com";

    HMIPAddress address_1_1;
    HMIPAddress address_1_2;
    HMIPAddress address_2_1;
    HMIPAddress address_2_2;
    HMIPAddress address_3_1;
    HMIPAddress address_3_2;
    HMIPAddress address_4_1;
    HMIPAddress address_4_2;

    address_1_1.set("192.168.0.1");
    address_1_2.set("fda0::1");
    address_2_1.set("192.168.0.2");
    address_2_2.set("fda0::2");
    address_3_1.set("192.168.0.3");
    address_3_2.set("fda0::3");
    address_4_1.set("192.168.0.4");
    address_4_2.set("fda0::4");

    HMDataHostGroup hostGroup(hostGroupName);
    hostGroup.setPassthroughInfo(0);
    hostGroup.setMaxFlaps(3);
    hostGroup.setFlapThreshold(10);
    hostGroup.setGroupThreshold(10);
    hostGroup.addHost(host1);
    hostGroup.addHost(host2);
    hostGroup.addHost(host3);
    hostGroup.addHost(host4);
    groupMap.insert(make_pair(hostGroupName, hostGroup));

    HMDataHostCheck hostCheck;
    hostGroup.getHostCheck(hostCheck);

    HMDataCheckParams checkParams;
    hostGroup.getCheckParameters(checkParams);
    checkParams.addHostGroup(hostGroupName);

    HMTimeStamp now;
    now.now();

    HMDataCheckResult checkResultDown;
    checkResultDown.m_checkTime = now;
    checkResultDown.m_numChecks = 10;
    checkResultDown.m_numResponses = 10;
    checkResultDown.m_status = HM_HOST_STATUS_UP;
    checkResultDown.m_response = HM_RESPONSE_FAILED;
    checkResultDown.m_reason = HM_REASON_CONNECT_FAILURE;

    HMDataCheckResult checkResultUp;
    checkResultUp.m_checkTime = now;
    checkResultUp.m_maxResponseTime = 10;
    checkResultUp.m_minResponseTime = 1;
    checkResultUp.m_numChecks = 10;
    checkResultUp.m_numResponses = 10;
    checkResultUp.m_status = HM_HOST_STATUS_UP ;
    checkResultUp.m_response = HM_RESPONSE_CONNECTED;
    checkResultUp.m_reason = HM_REASON_SUCCESS;
    checkResultUp.m_responseTime = 5;
    checkResultUp.m_smoothedResponseTime = 5;
    checkResultUp.m_sumResponseTime = 5;
    checkResultUp.m_totalResponseTime = 5;

    HMDataCheckResult checkResultSlow;
    checkResultSlow.m_checkTime = now;
    checkResultSlow.m_maxResponseTime = 100;
    checkResultSlow.m_minResponseTime = 100;
    checkResultSlow.m_numChecks = 10;
    checkResultSlow.m_numResponses = 10;
    checkResultSlow.m_status = HM_HOST_STATUS_UP;
    checkResultSlow.m_response = HM_RESPONSE_CONNECTED;
    checkResultSlow.m_reason = HM_REASON_SUCCESS;
    checkResultSlow.m_responseTime = 100;
    checkResultSlow.m_smoothedResponseTime = 100;
    checkResultSlow.m_sumResponseTime = 100;
    checkResultSlow.m_totalResponseTime = 100;

    HMDataCheckResult checkResultFlapping;
    checkResultFlapping.m_checkTime = now;
    checkResultFlapping.m_maxResponseTime = 10;
    checkResultFlapping.m_minResponseTime = 1;
    checkResultFlapping.m_numChecks = 10;
    checkResultFlapping.m_numResponses = 10;
    checkResultFlapping.m_status = HM_HOST_STATUS_UP;
    checkResultFlapping.m_response = HM_RESPONSE_CONNECTED;
    checkResultFlapping.m_reason = HM_REASON_SUCCESS;
    checkResultFlapping.m_responseTime = 5;
    checkResultFlapping.m_smoothedResponseTime = 5;
    checkResultFlapping.m_sumResponseTime = 5;
    checkResultFlapping.m_totalResponseTime = 5;
    checkResultFlapping.m_numFlaps = 20;

    HMDataCheckResult testResult;

    string filename = "";
    filename  = "mdbm_yfor";
    remove(filename.c_str());

    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(filename, &groupMap);

    CPPUNIT_ASSERT(store->openStore());

    checkResultUp.m_address = address_1_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host1, address_1_1, hostCheck, checkParams, checkResultUp));
    checkResultUp.m_address = address_1_2;
    CPPUNIT_ASSERT(store->storeCheckResult(host1, address_1_2, hostCheck, checkParams, checkResultUp));
    checkResultDown.m_address = address_2_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host2, address_2_1, hostCheck, checkParams, checkResultDown));
    checkResultDown.m_address = address_2_2;
    CPPUNIT_ASSERT(store->storeCheckResult(host2, address_2_2, hostCheck, checkParams, checkResultDown));
    checkResultSlow.m_address = address_3_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host3, address_3_1, hostCheck, checkParams, checkResultSlow));
    checkResultSlow.m_address = address_3_2;
    CPPUNIT_ASSERT(store->storeCheckResult(host3, address_3_2, hostCheck, checkParams, checkResultSlow));
    checkResultFlapping.m_address = address_4_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host4, address_4_1, hostCheck, checkParams, checkResultFlapping));
    checkResultFlapping.m_address = address_4_2;
    CPPUNIT_ASSERT(store->storeCheckResult(host4, address_4_2, hostCheck, checkParams, checkResultFlapping));

    std::this_thread::sleep_for(10ms);

    // TODO these unit tests need expanded to test the various host selection techniques
    CPPUNIT_ASSERT(store->getCheckResult(host1, address_1_1, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultUp);
    CPPUNIT_ASSERT(store->getCheckResult(host1, address_1_2, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultUp);
    CPPUNIT_ASSERT(store->getCheckResult(host2, address_2_1, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultDown);
    CPPUNIT_ASSERT(store->getCheckResult(host2, address_2_2, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultDown);
    CPPUNIT_ASSERT(store->getCheckResult(host3, address_3_1, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultSlow);
    CPPUNIT_ASSERT(store->getCheckResult(host3, address_3_2, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultSlow);
    CPPUNIT_ASSERT(store->getCheckResult(host4, address_4_1, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultFlapping);
    CPPUNIT_ASSERT(store->getCheckResult(host4, address_4_2, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultFlapping);

    store->closeStore();

    delete store;

    //remove(filename.c_str());
}


void
TESTNAME::test_HMStorageHostGroupYForMDBM_BackendTest()
{
    string hostGroupName1 = "group_Name1";
    string hostGroupName2 = "group_Name2";
    string hostGroupName3 = "group_Name3";
    string hostGroupName4 = "group_Name4";
    vector<string> actual = {hostGroupName1, hostGroupName3, hostGroupName4, hostGroupName2};
    HMDataHostGroupMap groupMap;

    // Now create the hosts and add them to the group
    string host1 = "host1.hm.com";
    string host2 = "host2.hm.com";
    string host3 = "host3.hm.com";
    string host4 = "host4.hm.com";

    HMIPAddress address_1_1;
    HMIPAddress address_2_1;
    HMIPAddress address_3_1;
    HMIPAddress address_4_1;

    address_1_1.set("192.168.0.1");
    address_2_1.set("192.168.0.2");
    address_3_1.set("192.168.0.3");
    address_4_1.set("192.168.0.4");

    HMDataHostGroup hostGroup1(hostGroupName1);
    hostGroup1.setPassthroughInfo(0);
    hostGroup1.setMaxFlaps(3);
    hostGroup1.setFlapThreshold(10);
    hostGroup1.setGroupThreshold(20);
    hostGroup1.addHost(host1);

    HMDataHostGroup hostGroup2(hostGroupName2);
    hostGroup2.setPassthroughInfo(0);
    hostGroup2.setMaxFlaps(3);
    hostGroup2.setFlapThreshold(10);
    hostGroup2.setGroupThreshold(10);
    hostGroup2.addHost(host2);
    hostGroup2.addHost(host3);
    hostGroup2.addHost(host4);

    HMDataHostGroup hostGroup3(hostGroupName3);
    hostGroup3.setPassthroughInfo(0);
    hostGroup3.setMaxFlaps(3);
    hostGroup3.setFlapThreshold(10);
    hostGroup3.setGroupThreshold(10);
    hostGroup3.addHost(host3);

    HMDataHostGroup hostGroup4(hostGroupName4);
    hostGroup4.setPassthroughInfo(0);
    hostGroup4.setMaxFlaps(3);
    hostGroup4.setFlapThreshold(10);
    hostGroup4.setGroupThreshold(10);
    hostGroup4.addHost(host4);

    groupMap.insert(make_pair(hostGroupName1, hostGroup1));
    groupMap.insert(make_pair(hostGroupName2, hostGroup2));
    groupMap.insert(make_pair(hostGroupName3, hostGroup3));
    groupMap.insert(make_pair(hostGroupName4, hostGroup4));

    HMDataHostCheck hostCheck;
    hostGroup1.getHostCheck(hostCheck);

    HMDataCheckParams checkParams;
    hostGroup2.getCheckParameters(checkParams);

    HMDataCheckParams checkParams1;
    hostGroup1.getCheckParameters(checkParams1);

    checkParams1.addHostGroup(hostGroupName1);
    checkParams.addHostGroup(hostGroupName2);
    checkParams.addHostGroup(hostGroupName3);
    checkParams.addHostGroup(hostGroupName4);

    HMTimeStamp now;
    now.now();

    HMDataCheckResult checkResultDown;
    checkResultDown.m_checkTime = now;
    checkResultDown.m_numChecks = 10;
    checkResultDown.m_numResponses = 10;
    checkResultDown.m_status = HM_HOST_STATUS_UP;
    checkResultDown.m_response = HM_RESPONSE_FAILED;
    checkResultDown.m_reason = HM_REASON_CONNECT_FAILURE;

    HMDataCheckResult checkResultUp;
    checkResultUp.m_checkTime = now;
    checkResultUp.m_maxResponseTime = 10;
    checkResultUp.m_minResponseTime = 1;
    checkResultUp.m_numChecks = 10;
    checkResultUp.m_numResponses = 10;
    checkResultUp.m_status = HM_HOST_STATUS_UP;
    checkResultUp.m_response = HM_RESPONSE_CONNECTED;
    checkResultUp.m_reason = HM_REASON_SUCCESS;
    checkResultUp.m_responseTime = 5;
    checkResultUp.m_smoothedResponseTime = 5;
    checkResultUp.m_sumResponseTime = 5;
    checkResultUp.m_totalResponseTime = 5;

    HMDataCheckResult checkResultSlow;
    checkResultSlow.m_checkTime = now;
    checkResultSlow.m_maxResponseTime = 100;
    checkResultSlow.m_minResponseTime = 100;
    checkResultSlow.m_numChecks = 10;
    checkResultSlow.m_numResponses = 10;
    checkResultSlow.m_status = HM_HOST_STATUS_UP ;
    checkResultSlow.m_response = HM_RESPONSE_CONNECTED;
    checkResultSlow.m_reason = HM_REASON_SUCCESS;
    checkResultSlow.m_responseTime = 100;
    checkResultSlow.m_smoothedResponseTime = 100;
    checkResultSlow.m_sumResponseTime = 100;
    checkResultSlow.m_totalResponseTime = 100;

    HMDataCheckResult checkResultFlapping;
    checkResultFlapping.m_checkTime = now;
    checkResultFlapping.m_maxResponseTime = 10;
    checkResultFlapping.m_minResponseTime = 1;
    checkResultFlapping.m_numChecks = 10;
    checkResultFlapping.m_numResponses = 10;
    checkResultFlapping.m_status = HM_HOST_STATUS_UP;
    checkResultFlapping.m_response = HM_RESPONSE_CONNECTED;
    checkResultFlapping.m_reason = HM_REASON_SUCCESS;
    checkResultFlapping.m_responseTime = 5;
    checkResultFlapping.m_smoothedResponseTime = 5;
    checkResultFlapping.m_sumResponseTime = 5;
    checkResultFlapping.m_totalResponseTime = 5;
    checkResultFlapping.m_numFlaps = 20;

    HMDataCheckResult testResult;

    HMConfigInfo configInfo;
    HMConfigInfo testInfo;

    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;

    string sourceurl = "testing";

    HMAuxInfo auxInfo;
    HMIPAddress address_1;
    address_1.set("192.168.0.1");
    HMTimeStamp ts;
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

    auxInfo.m_auxData.push_back(std::move(r1));

    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName1, hostGroup1));
    checkState.m_hostGroups.insert(make_pair(hostGroupName2, hostGroup2));
    checkState.m_hostGroups.insert(make_pair(hostGroupName3, hostGroup3));
    checkState.m_hostGroups.insert(make_pair(hostGroupName4, hostGroup4));
    string filename = "";
    filename  = "mdbm_yfor";
    remove(filename.c_str());

    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(filename, &groupMap);

    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigs(checkState));
    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));

    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1_1, hostCheck, checkParams1, auxInfo));
    std::this_thread::sleep_for(10ms);

    checkResultUp.m_address = address_1_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host1, address_1_1, hostCheck, checkParams1, checkResultUp));
    checkResultDown.m_address = address_2_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host2, address_2_1, hostCheck, checkParams, checkResultDown));
    checkResultSlow.m_address = address_3_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host3, address_3_1, hostCheck, checkParams, checkResultSlow));
    checkResultFlapping.m_address = address_4_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host4, address_4_1, hostCheck, checkParams, checkResultFlapping));

    std::this_thread::sleep_for(10ms);

    // TODO these unit tests need expanded to test the various host selection techniques
    CPPUNIT_ASSERT(store->getCheckResult(host1, address_1_1, hostCheck, checkParams1, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultUp);
    HMDataCheckResult actualResult = testResult;
    //CPPUNIT_ASSERT(testResult == checkResultUp);
    CPPUNIT_ASSERT(store->getCheckResult(host2, address_2_1, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultDown);

    //CPPUNIT_ASSERT(testResult == checkResultDown);
    CPPUNIT_ASSERT(store->getCheckResult(host3, address_3_1, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultSlow);

    //CPPUNIT_ASSERT(testResult == checkResultSlow);
    CPPUNIT_ASSERT(store->getCheckResult(host4, address_4_1, hostCheck, checkParams, testResult));
    //CPPUNIT_ASSERT(testResult == checkResultFlapping);

    //CPPUNIT_ASSERT(testResult == checkResultFlapping);

    set<string> hostGroups;
    store->getHostGroupNames(hostGroups);
    CPPUNIT_ASSERT_EQUAL(actual.size(), hostGroups.size());
    for (uint32_t i = 0; i < hostGroups.size(); i++)
    {
        CPPUNIT_ASSERT(hostGroups.find(actual[i]) != hostGroups.end());
    }
    vector<HMGroupCheckResult> results;
    CPPUNIT_ASSERT(store->getGroupCheckResults(hostGroupName1, true, true, results));
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    CPPUNIT_ASSERT(results[0].m_address == address_1_1);
    CPPUNIT_ASSERT(results[0].m_hostName == host1);
    CPPUNIT_ASSERT(results[0].m_result == actualResult);
    HMDataHostGroup hostGroup(hostGroupName1);
    CPPUNIT_ASSERT(store->getGroupInfo(hostGroupName1, hostGroup));
    CPPUNIT_ASSERT(hostGroup == hostGroup1);
    CPPUNIT_ASSERT(hostGroup.getPassthroughInfo() == hostGroup1.getPassthroughInfo());

    store->closeStore();
    delete store;

    remove(filename.c_str());
}


void
TESTNAME::test_HMStorageHostGroupYForMDBM_ConfigStoreRetrieve()
{
    string filename  = "testmdbm";
    remove(filename.c_str());

    HMDataHostGroupMap groupMap;
    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(filename, &groupMap);
    HMConfigInfo configInfo;
    HMConfigInfo testInfo;
    HMTimeStamp now;
    now.now();

    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;

    CPPUNIT_ASSERT(store->openStore());

    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));
    CPPUNIT_ASSERT(store->getConfigInfo(testInfo));

    store->closeStore();

    CPPUNIT_ASSERT(configInfo.m_version == testInfo.m_version);
    CPPUNIT_ASSERT(configInfo.m_configStatus == testInfo.m_configStatus);
    CPPUNIT_ASSERT(configInfo.m_configLoadTime == testInfo.m_configLoadTime);
    delete store;

    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore(true));
    CPPUNIT_ASSERT(!store->storeConfigInfo(configInfo));
    CPPUNIT_ASSERT(store->getConfigInfo(testInfo));

    store->closeStore();

    CPPUNIT_ASSERT(configInfo.m_version == testInfo.m_version);
    CPPUNIT_ASSERT(configInfo.m_configStatus == testInfo.m_configStatus);
    CPPUNIT_ASSERT(configInfo.m_configLoadTime == testInfo.m_configLoadTime);
    delete store;

    remove(filename.c_str());
}

void
TESTNAME::test_HMStorageHostGroupYForMDBM_HMLoadFile_StoreRetrieve()
{
    string filename  = "testmdbm";
    remove(filename.c_str());

    HMDataHostGroupMap groupMap;
    HMStorageHostGroupMDBM* store;
    // Now create the hosts and add them to the group
    string host1 = "host1.hm.com";
    string sourceurl = "testing";
    string hostGroupName1 = "testHostGroup";
    HMAuxInfo auxInfo, recieveInfo;
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

    HMDataHostGroup hostGroup1(hostGroupName1);
    hostGroup1.setPassthroughInfo(0);
    hostGroup1.setMaxFlaps(3);
    hostGroup1.setFlapThreshold(10);
    hostGroup1.setGroupThreshold(20);
    hostGroup1.addHost(host1);
    hostGroup1.setCheckInfo(sourceurl);
    HMDataHostCheck dataHostCheck;
    HMDataCheckParams dataCheckParams;
    hostGroup1.getHostCheck(dataHostCheck);
    hostGroup1.getCheckParameters(dataCheckParams);
    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName1, hostGroup1));
    groupMap.insert(make_pair(hostGroupName1, hostGroup1));
    dataCheckParams.addHostGroup(hostGroupName1);

    auxInfo.m_auxData.push_back(std::move(r1));
    //Tests HM_LOAD_FILE, host stores only 1 of data
    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigs(checkState));
    CPPUNIT_ASSERT(
            store->storeAuxInfo(host1, address_1, dataHostCheck,
                    dataCheckParams, auxInfo));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, recieveInfo));

    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == auxInfo.m_auxData.size());
    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == 1);

    HMAuxLoadFB* lfb = dynamic_cast<HMAuxLoadFB*>(recieveInfo.m_auxData[0].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    CPPUNIT_ASSERT(lfb->m_host == "host1.hm.com");
    CPPUNIT_ASSERT(lfb->m_ip == address_1);
    CPPUNIT_ASSERT(lfb->m_resource == "resource");
    CPPUNIT_ASSERT(lfb->m_ts == ts);
    CPPUNIT_ASSERT(lfb->m_datacenter == "datacenter1");
    CPPUNIT_ASSERT(lfb->m_load == 3);
    CPPUNIT_ASSERT(lfb->m_target == 20);
    CPPUNIT_ASSERT(lfb->m_max == 400);
    store->closeStore();

    delete store;
    recieveInfo.m_auxData.clear();

    std::unique_ptr<HMAuxLoadFB> r2 = make_unique<HMAuxLoadFB>();

    HMIPAddress address_2;
    address_2.set("192.168.0.2");
    ts.setTime("2018-2-1T0:3:1Z", "%Y-%m-%dT%H:%M:%SZ");

    r2->m_host ="host1.hm.com";
    r2->m_type = HM_LOAD_FILE;
    r2->m_ip = address_1;
    r2->m_resource ="resource";
    r2->m_ts = ts;
    r2->m_datacenter = "datacenter2";
    r2->m_load = 13;
    r2->m_max = 1400;
    r2->m_target =120;

    auxInfo.m_auxData.push_back(std::move(r2));

    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    //Tests HM_LOAD_FILE, host stores more than 1  data  can be used if
    //we want multiple type values
    CPPUNIT_ASSERT(store->openStore());

    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, auxInfo));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, recieveInfo));

    CPPUNIT_ASSERT_EQUAL(auxInfo.m_auxData.size(), recieveInfo.m_auxData.size());
    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == 2);

    lfb = dynamic_cast<HMAuxLoadFB*>(recieveInfo.m_auxData[0].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    CPPUNIT_ASSERT(lfb->m_host == "host1.hm.com");
    CPPUNIT_ASSERT(lfb->m_ip == address_1);
    CPPUNIT_ASSERT(lfb->m_resource == "resource");
    CPPUNIT_ASSERT(lfb->m_ts == ts);
    CPPUNIT_ASSERT(lfb->m_datacenter == "datacenter1");
    CPPUNIT_ASSERT(lfb->m_load == 3);
    CPPUNIT_ASSERT(lfb->m_target == 20);
    CPPUNIT_ASSERT(lfb->m_max == 400);
    store->closeStore();

    lfb = dynamic_cast<HMAuxLoadFB*>(recieveInfo.m_auxData[1].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    CPPUNIT_ASSERT(lfb->m_host == "host1.hm.com");
    CPPUNIT_ASSERT(lfb->m_ip == address_1);
    CPPUNIT_ASSERT(lfb->m_resource == "resource");
    CPPUNIT_ASSERT(lfb->m_ts == ts);
    CPPUNIT_ASSERT(lfb->m_datacenter == "datacenter2");
    CPPUNIT_ASSERT(lfb->m_load == 13);
    CPPUNIT_ASSERT(lfb->m_target == 120);
    CPPUNIT_ASSERT(lfb->m_max == 1400);
    store->closeStore();
    delete store;

    remove(filename.c_str());
}

void
TESTNAME::test_HMStorageHostGroupYForMDBM_HMAuxOOB_StoreRetrieve()
{
    string filename  = "testmdbm";
    remove(filename.c_str());

    HMDataHostGroupMap groupMap;
    HMStorageHostGroupMDBM* store;
    // Now create the hosts and add them to the group
    string host1 = "host1.hm.com";
    string sourceurl = "testing";
    string hostGroupName1 = "testHostGroup";

    HMDataHostGroup hostGroup1(hostGroupName1);
    hostGroup1.setPassthroughInfo(0);
    hostGroup1.setMaxFlaps(3);
    hostGroup1.setFlapThreshold(10);
    hostGroup1.setGroupThreshold(20);
    hostGroup1.addHost(host1);
    hostGroup1.setCheckInfo(sourceurl);
    HMDataHostCheck dataHostCheck;
    HMDataCheckParams dataCheckParams;
    hostGroup1.getHostCheck(dataHostCheck);
    hostGroup1.getCheckParameters(dataCheckParams);
    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName1, hostGroup1));
    groupMap.insert(make_pair(hostGroupName1, hostGroup1));
    dataCheckParams.addHostGroup(hostGroupName1);


    HMAuxInfo auxInfo, recieveInfo;
    HMIPAddress address_1;
    address_1.set("192.168.0.1");
    HMTimeStamp ts,ts1;
    HMTimeStamp updateTime;
    ts.setTime("2018-2-1T0:3:1Z", "%Y-%m-%dT%H:%M:%SZ");
    std::unique_ptr<HMAuxOOB> r1 = make_unique<HMAuxOOB>();
    r1->m_host ="host1.hm.com";
    r1->m_type = HM_OOB_FILE;
    r1->m_ip = address_1;
    r1->m_resource ="resource";
    r1->m_ts = ts;
    r1->m_forceDown = false;

    auxInfo.m_auxData.push_back(std::move(r1));
    //Tests HM_OOB_FILE, host stores only 1 of data
    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigs(checkState));
    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, auxInfo));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, recieveInfo));

    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == auxInfo.m_auxData.size());
    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == 1);

    HMAuxOOB* oob = dynamic_cast<HMAuxOOB*>(recieveInfo.m_auxData[0].get());
    CPPUNIT_ASSERT(oob != nullptr);
    CPPUNIT_ASSERT(oob->m_host == "host1.hm.com");
    CPPUNIT_ASSERT(oob->m_ip == address_1);
    CPPUNIT_ASSERT(oob->m_type == HM_OOB_FILE);
    CPPUNIT_ASSERT(oob->m_resource == "resource");
    CPPUNIT_ASSERT(oob->m_ts == ts);
    CPPUNIT_ASSERT(oob->m_forceDown == false);
    CPPUNIT_ASSERT(oob->m_shed == 0);
    store->closeStore();

    delete store;

    std::unique_ptr<HMAuxOOB> r2 = make_unique<HMAuxOOB>();
    r2->m_host ="host2.hm.com";
    r2->m_type = HM_OOB_FILE;
    r2->m_ip = address_1;
    r2->m_resource ="resource2";
    r2->m_ts = ts;
    r2->m_forceDown = false;
    r2->m_shed = 30;

    auxInfo.m_auxData.clear();
    recieveInfo.m_auxData.clear();
    auxInfo.m_auxData.push_back(std::move(r2));

    //Tests HM_OOB_FILE, host stores only 1 of data
    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore());

    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, auxInfo));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, recieveInfo));

    CPPUNIT_ASSERT_EQUAL(auxInfo.m_auxData.size(), recieveInfo.m_auxData.size());
    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == 1);

    oob = dynamic_cast<HMAuxOOB*>(recieveInfo.m_auxData[0].get());
    CPPUNIT_ASSERT(oob != nullptr);
    CPPUNIT_ASSERT(oob->m_host == "host2.hm.com");
    CPPUNIT_ASSERT(oob->m_ip == address_1);
    CPPUNIT_ASSERT(oob->m_resource == "resource2");
    CPPUNIT_ASSERT(oob->m_ts == ts);
    CPPUNIT_ASSERT(oob->m_forceDown == false);
    CPPUNIT_ASSERT(oob->m_shed == 30);
    store->closeStore();

    delete store;

    std::unique_ptr<HMAuxOOB> r3 = make_unique<HMAuxOOB>();
    r3->m_type = HM_OOB_FILE;

    recieveInfo.m_auxData.clear();
    auxInfo.m_auxData.clear();
    auxInfo.m_auxData.push_back(std::move(r3));

    //Tests HM_OOB_FILE, host stores only 1 of data
    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore());

    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, auxInfo));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, recieveInfo));

    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == auxInfo.m_auxData.size());
    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == 1);

    oob = dynamic_cast<HMAuxOOB*>(recieveInfo.m_auxData[0].get());
    CPPUNIT_ASSERT(oob != nullptr);
    CPPUNIT_ASSERT(oob->m_host.empty());
    CPPUNIT_ASSERT((oob->m_ip.toString()).empty());
    CPPUNIT_ASSERT(oob->m_resource.empty());
    CPPUNIT_ASSERT(oob->m_ts == ts1);
    CPPUNIT_ASSERT(!oob->m_forceDown);
    CPPUNIT_ASSERT(oob->m_shed == 0);
    store->closeStore();

    delete store;

    remove(filename.c_str());
}

void
TESTNAME::test_HMStorageHostGroupYForMDBM_HMLoadObject_StoreRetrieve()
{
    string filename  = "testmdbm";
    string hostGroupName1 = "testHostGroup";

    remove(filename.c_str());
    HMAuxInfo auxInfo, recieveInfo;

    HMDataHostGroupMap groupMap;
    HMStorageHostGroupMDBM* store;

    // Now create the hosts and add them to the group
    string host1 = "host1.hm.com";
    string sourceurl = "testing";

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

    auxInfo.m_auxData.push_back(std::move(r1));
    //Tests HM_LOAD_OBJECT, host stores only 1 of data

    HMDataHostGroup hostGroup1(hostGroupName1);
    hostGroup1.setPassthroughInfo(0);
    hostGroup1.setMaxFlaps(3);
    hostGroup1.setFlapThreshold(10);
    hostGroup1.setGroupThreshold(20);
    hostGroup1.addHost(host1);
    hostGroup1.setCheckInfo(sourceurl);
    HMDataHostCheck dataHostCheck;
    HMDataCheckParams dataCheckParams;
    hostGroup1.getHostCheck(dataHostCheck);
    hostGroup1.getCheckParameters(dataCheckParams);
    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName1, hostGroup1));
    groupMap.insert(make_pair(hostGroupName1, hostGroup1));
    dataCheckParams.addHostGroup(hostGroupName1);
    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigs(checkState));
    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, auxInfo));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, recieveInfo));

    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == auxInfo.m_auxData.size());
    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == 1);

    HMAuxLoadFB* lfb = dynamic_cast<HMAuxLoadFB*>(recieveInfo.m_auxData[0].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    CPPUNIT_ASSERT(lfb->m_host == "host1.hm.com");
    CPPUNIT_ASSERT(lfb->m_ip == address_1);
    CPPUNIT_ASSERT(lfb->m_resource == "resource");
    CPPUNIT_ASSERT(lfb->m_ts == ts);
    CPPUNIT_ASSERT(lfb->m_datacenter == "datacenter1");
    CPPUNIT_ASSERT(lfb->m_load == 3);
    CPPUNIT_ASSERT(lfb->m_type == HM_LOAD_FILE);
    CPPUNIT_ASSERT(lfb->m_target == 20);
    CPPUNIT_ASSERT(lfb->m_max == 400);
    store->closeStore();

    delete store;

    //neg test
    std::unique_ptr<HMAuxLoadFB> r2 = make_unique<HMAuxLoadFB>();

    recieveInfo.m_auxData.clear();
    auxInfo.m_auxData.clear();
    auxInfo.m_auxData.push_back(std::move(r2));

    //Tests HM_OOB_FILE, host stores only 1 of data
    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore());

    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, auxInfo));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, recieveInfo));

    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == auxInfo.m_auxData.size());
    CPPUNIT_ASSERT(recieveInfo.m_auxData.size() == 1);
    HMTimeStamp ts1;

    lfb = dynamic_cast<HMAuxLoadFB*>(recieveInfo.m_auxData[0].get());
    CPPUNIT_ASSERT(lfb != nullptr);
    CPPUNIT_ASSERT(lfb->m_host.empty());
    CPPUNIT_ASSERT((lfb->m_ip.toString()).empty());
    CPPUNIT_ASSERT((lfb->m_resource.empty()));
    CPPUNIT_ASSERT(lfb->m_type == HM_LOAD_FILE);
    CPPUNIT_ASSERT((lfb->m_datacenter.empty()));
    CPPUNIT_ASSERT(lfb->m_ts == ts1);
    CPPUNIT_ASSERT(lfb->m_load == 0);
    CPPUNIT_ASSERT(lfb->m_target == 0);
    CPPUNIT_ASSERT(lfb->m_max == 0);

    store->closeStore();

    delete store;
    
    //neg tests
    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore(true));

    CPPUNIT_ASSERT(!store->storeAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, auxInfo));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, recieveInfo));
       
    store->closeStore();

    delete store;
    HMDataCheckParams params;
    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore(true));

    CPPUNIT_ASSERT(!store->storeAuxInfo(host1, address_1, dataHostCheck, dataCheckParams, auxInfo));
    std::this_thread::sleep_for(10ms);
    CPPUNIT_ASSERT(!store->getAuxInfo("host1", address_1, dataHostCheck, params, recieveInfo));
       
    store->closeStore();

    delete store;    

    remove(filename.c_str());

}

void TESTNAME::test_HMStorageHostGroupYForMDBM_ZeroIp()
{
    string hostGroupName = "group_Name";

    HMDataHostGroupMap groupMap;

    // Now create the hosts and add them to the group
    string host1 = "host1.hm.com";

    HMIPAddress address_1_1(AF_INET);
    HMIPAddress address_1_2(AF_INET6);
    HMIPAddress address_1_3;

    address_1_3.set("192.168.0.1");

    HMDataHostGroup hostGroup(hostGroupName);
    hostGroup.setPassthroughInfo(0);
    hostGroup.setMaxFlaps(3);
    hostGroup.setFlapThreshold(10);
    hostGroup.setGroupThreshold(10);
    hostGroup.addHost(host1);
    groupMap.insert(make_pair(hostGroupName, hostGroup));

    HMDataHostCheck hostCheck;
    hostGroup.getHostCheck(hostCheck);

    HMDataCheckParams checkParams;
    hostGroup.getCheckParameters(checkParams);
    checkParams.addHostGroup(hostGroupName);

    HMTimeStamp now;
    now.now();

    HMDataCheckResult checkResultDown;
    checkResultDown.m_checkTime = now;
    checkResultDown.m_response = HM_RESPONSE_FAILED;
    checkResultDown.m_reason = HM_REASON_CONNECT_FAILURE;

    HMDataCheckResult checkResultUp;
    checkResultUp.m_checkTime = now;
    checkResultUp.m_response = HM_RESPONSE_CONNECTED;
    checkResultUp.m_reason = HM_REASON_SUCCESS;

    HMDataCheckResult checkResultSlow;
    checkResultSlow.m_checkTime = now;
    checkResultSlow.m_response = HM_RESPONSE_CONNECTED;
    checkResultSlow.m_reason = HM_REASON_CONNECT_TIMEOUT;

    HMDataCheckResult testResult;

    string filename = "";
    filename = "mdbm_yfor";
    remove(filename.c_str());

    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(filename,
            &groupMap);

    CPPUNIT_ASSERT(store->openStore());

    checkResultUp.m_address = address_1_1;
    CPPUNIT_ASSERT(
            store->storeCheckResult(host1, address_1_1, hostCheck, checkParams,
                    checkResultUp));
    checkResultDown.m_address = address_1_2;
    CPPUNIT_ASSERT(
            store->storeCheckResult(host1, address_1_2, hostCheck, checkParams,
                    checkResultDown));
    std::this_thread::sleep_for(10ms);

    CPPUNIT_ASSERT_EQUAL(true,
            store->getCheckResult(host1, address_1_1, hostCheck, checkParams,
                    testResult));

    CPPUNIT_ASSERT_EQUAL(true,
            store->getCheckResult(host1, address_1_2, hostCheck, checkParams,
                    testResult));

    CPPUNIT_ASSERT_EQUAL((int)HM_REASON_CONNECT_FAILURE, (int)testResult.m_reason);

    CPPUNIT_ASSERT(
            store->storeCheckResult(host1, address_1_3, hostCheck, checkParams,
                    checkResultSlow));
    std::this_thread::sleep_for(10ms);

    CPPUNIT_ASSERT_EQUAL(true,
                store->getCheckResult(host1, address_1_2, hostCheck, checkParams,
                        testResult));


    CPPUNIT_ASSERT_EQUAL(true,
                store->getCheckResult(host1, address_1_3, hostCheck, checkParams,
                        testResult));

    CPPUNIT_ASSERT_EQUAL((int)HM_REASON_CONNECT_TIMEOUT, (int)testResult.m_reason);

    store->closeStore();

    delete store;

}


void TESTNAME::test_HMStorageHostGroupYForMDBM_VersionChange()
{


    string hostGroupName1 = "group_Name1";
    string hostGroupName2 = "group_Name2";
    vector<string> actual = {hostGroupName1, hostGroupName2};
    HMDataHostGroupMap groupMap;

    // Now create the hosts and add them to the group
    string host1 = "host1.hm.com";
    string host2 = "host2.hm.com";

    HMIPAddress address_1_1;
    HMIPAddress address_2_1;

    address_1_1.set("192.168.0.1");
    address_2_1.set("192.168.0.2");

    HMDataHostGroup hostGroup1(hostGroupName1);
    hostGroup1.addHost(host1);

    HMDataHostGroup hostGroup2(hostGroupName2);
    hostGroup2.addHost(host2);

    groupMap.insert(make_pair(hostGroupName1, hostGroup1));
    groupMap.insert(make_pair(hostGroupName2, hostGroup2));

    HMDataHostCheck hostCheck;
    hostGroup1.getHostCheck(hostCheck);

    HMDataCheckParams checkParams1;
    hostGroup1.getCheckParameters(checkParams1);

    HMDataCheckParams checkParams2;
    hostGroup2.getCheckParameters(checkParams2);


    checkParams1.addHostGroup(hostGroupName1);
    checkParams2.addHostGroup(hostGroupName2);

    HMTimeStamp now;
    now.now();

    HMDataCheckResult checkResultDown;
    checkResultDown.m_checkTime = now;
    checkResultDown.m_numChecks = 10;
    checkResultDown.m_numResponses = 10;
    checkResultDown.m_status = HM_HOST_STATUS_UP;
    checkResultDown.m_response = HM_RESPONSE_FAILED;
    checkResultDown.m_reason = HM_REASON_CONNECT_FAILURE;

    HMConfigInfo configInfo;
    HMConfigInfo testInfo;

    configInfo.m_version = 1;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;

    string sourceurl = "testing";

    HMAuxInfo auxInfo, recieveInfo;
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

    auxInfo.m_auxData.push_back(std::move(r1));


    string filename = "";
    filename  = "mdbm_yfor";
    remove(filename.c_str());

    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(filename, &groupMap);

    CPPUNIT_ASSERT(store->openStore());

    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));

    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1_1, hostCheck, checkParams1, auxInfo));
    std::this_thread::sleep_for(10ms);

    checkResultDown.m_address = address_1_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host1, address_1_1, hostCheck, checkParams1, checkResultDown));
    checkResultDown.m_address = address_2_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host2, address_2_1, hostCheck, checkParams2, checkResultDown));

    std::this_thread::sleep_for(10ms);

    HMDataCheckResult testResult;
    HMConfigInfo testConfigInfo;
    // TODO these unit tests need expanded to test the various host selection techniques
    CPPUNIT_ASSERT(store->getConfigInfo(testConfigInfo));
    CPPUNIT_ASSERT_EQUAL(configInfo.m_version, testConfigInfo.m_version);
    CPPUNIT_ASSERT(store->getCheckResult(host1, address_1_1, hostCheck, checkParams1, testResult));
    CPPUNIT_ASSERT(store->getCheckResult(host2, address_2_1, hostCheck, checkParams2, testResult));
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1_1, hostCheck, checkParams1, recieveInfo));
    vector<string> hostGroups;
    store->closeStore();
    delete store;

    store = new HMStorageHostGroupMDBM(filename, &groupMap);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(!store->getConfigInfo(testConfigInfo));
    CPPUNIT_ASSERT(!store->getCheckResult(host1, address_1_1, hostCheck, checkParams1, testResult));
    CPPUNIT_ASSERT(!store->getCheckResult(host2, address_2_1, hostCheck, checkParams2, testResult));
    CPPUNIT_ASSERT(!store->getAuxInfo(host1, address_1_1, hostCheck, checkParams1, recieveInfo));
    hostGroups.clear();
    store->closeStore();
    delete store;

    remove(filename.c_str());


}


void TESTNAME::test_HMStorageHostGroupYForMDBM_ClearBackend()
{


    string hostGroupName1 = "group_Name1";
    string hostGroupName2 = "group_Name2";
    vector<string> actual = {hostGroupName1, hostGroupName2};
    HMDataHostGroupMap groupMap;

    // Now create the hosts and add them to the group
    string host1 = "host1.hm.com";
    string host2 = "host2.hm.com";

    HMIPAddress address_1_1;
    HMIPAddress address_2_1;

    address_1_1.set("192.168.0.1");
    address_2_1.set("192.168.0.2");

    HMDataHostGroup hostGroup1(hostGroupName1);
    hostGroup1.setPassthroughInfo(0);
    hostGroup1.setMaxFlaps(3);
    hostGroup1.setFlapThreshold(10);
    hostGroup1.setGroupThreshold(20);
    hostGroup1.addHost(host1);

    HMDataHostGroup hostGroup2(hostGroupName2);
    hostGroup2.setPassthroughInfo(0);
    hostGroup2.setMaxFlaps(3);
    hostGroup2.setFlapThreshold(10);
    hostGroup2.setGroupThreshold(10);
    hostGroup2.addHost(host2);

    groupMap.insert(make_pair(hostGroupName1, hostGroup1));
    groupMap.insert(make_pair(hostGroupName2, hostGroup2));

    HMDataHostCheck hostCheck;
    hostGroup1.getHostCheck(hostCheck);

    HMDataCheckParams checkParams1;
    hostGroup1.getCheckParameters(checkParams1);

    HMDataCheckParams checkParams2;
    hostGroup2.getCheckParameters(checkParams2);

    checkParams1.addHostGroup(hostGroupName1);
    checkParams2.addHostGroup(hostGroupName2);

    HMTimeStamp now;
    now.now();

    HMDataCheckResult checkResultDown;
    checkResultDown.m_checkTime = now;
    checkResultDown.m_numChecks = 10;
    checkResultDown.m_numResponses = 10;
    checkResultDown.m_status = HM_HOST_STATUS_UP;
    checkResultDown.m_response = HM_RESPONSE_FAILED;
    checkResultDown.m_reason = HM_REASON_CONNECT_FAILURE;

    HMConfigInfo configInfo;
    HMConfigInfo testInfo;

    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;

    string sourceurl = "testing";

    HMAuxInfo auxInfo, recieveInfo;
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

    auxInfo.m_auxData.push_back(std::move(r1));


    string filename = "";
    filename  = "mdbm_yfor";
    remove(filename.c_str());

    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName1, hostGroup1));
    checkState.m_hostGroups.insert(make_pair(hostGroupName2, hostGroup2));
    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(filename, &groupMap);

    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigs(checkState));
    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));

    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address_1_1, hostCheck, checkParams1, auxInfo));
    std::this_thread::sleep_for(10ms);

    checkResultDown.m_address = address_1_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host1, address_1_1, hostCheck, checkParams1, checkResultDown));
    checkResultDown.m_address = address_2_1;
    CPPUNIT_ASSERT(store->storeCheckResult(host2, address_2_1, hostCheck, checkParams2, checkResultDown));

    std::this_thread::sleep_for(10ms);

    HMDataCheckResult testResult;
    // TODO these unit tests need expanded to test the various host selection techniques
    HMConfigInfo testConfigInfo;
    CPPUNIT_ASSERT(store->getConfigInfo(testConfigInfo));
    CPPUNIT_ASSERT_EQUAL(configInfo.m_version, testConfigInfo.m_version);
    CPPUNIT_ASSERT(store->getAuxInfo(host1, address_1_1, hostCheck, checkParams1,recieveInfo));
    CPPUNIT_ASSERT(store->getCheckResult(host1, address_1_1, hostCheck, checkParams1, testResult));
    CPPUNIT_ASSERT(store->getCheckResult(host2, address_2_1, hostCheck, checkParams2, testResult));

    set<string> hostGroups;
    store->getHostGroupNames(hostGroups);
    CPPUNIT_ASSERT_EQUAL(actual.size(), hostGroups.size());

    vector<HMGroupCheckResult> results;
    CPPUNIT_ASSERT(store->getGroupCheckResults(hostGroupName1, true, true, results));
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    HMDataHostGroup hostGroup(hostGroupName1);
    CPPUNIT_ASSERT(store->getGroupInfo(hostGroupName1, hostGroup));
    CPPUNIT_ASSERT(hostGroup == hostGroup1);
    CPPUNIT_ASSERT(hostGroup.getPassthroughInfo() == hostGroup1.getPassthroughInfo());

    // clearing the
    CPPUNIT_ASSERT(store->clearBackend());
    CPPUNIT_ASSERT(!store->getConfigInfo(testConfigInfo));
    CPPUNIT_ASSERT(!store->getCheckResult(host1, address_1_1, hostCheck, checkParams1, testResult));
    CPPUNIT_ASSERT(!store->getCheckResult(host2, address_2_1, hostCheck, checkParams2, testResult));
    CPPUNIT_ASSERT(!store->getAuxInfo(host1, address_1_1, hostCheck, checkParams1, recieveInfo));
    hostGroups.clear();
    store->getHostGroupNames(hostGroups);
    CPPUNIT_ASSERT_EQUAL(0, (int)hostGroups.size());

    results.clear();
    CPPUNIT_ASSERT(!store->getGroupCheckResults(hostGroupName1, true, true, results));
    CPPUNIT_ASSERT_EQUAL(0, (int )results.size());
    CPPUNIT_ASSERT(!store->getGroupInfo(hostGroupName1, hostGroup));
    store->closeStore();
    delete store;

    remove(filename.c_str());


}

