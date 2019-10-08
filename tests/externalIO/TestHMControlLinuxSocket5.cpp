// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlLinuxSocket5.h"

#include <sys/stat.h>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include "common.h"
#include "HMConstants.h"
#include "HMStateManager.h"
#include "HMControlLinuxSocket.h"
#include "HMStorageHostGroupMDBM.h"

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    mkdir("conf", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("conf/hm", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    std::ofstream fout1("conf/dummy_master.yaml");
    std::ofstream fout3("conf/hm/testconf.yaml");

    fout1 << "threads.max: 2\n\
config.load-directory: ./conf/hm\n\
dns.type: none\n\
dns.host: 192.168.1.1\n\
dns.ttl: 360\n\
dns.lookup-timeout: 60\n\
http.type: curl\n\
ftp.type: curl\n\
tcp.type: rawsocket\n\
dnsvc.type: ares\n\
none.type: none\n\
db.type: mdbm\n\
db.path: netchasm.mdbm\n\
log.path: hm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
enable-secure-remote : off\n\
socket.path: test_sock" << endl;

    fout3 << "-   name: config.parse1.netchasm.net\n\
    allow-hosts: any\n\
    ttl: 60000\n\
    failure-response: dns\n\
    check-type: none\n\
    check-port: 123\n\
    check-info: hm-hello\n\
    group-threshold: 12\n\
    dual-stack-mode: both\n\
    dns-type: none\n\
    host:\n\
        - loadfb3.hm1.com\n\
        - loadfb3.hm2.com\n\
    \n\
-   name: config1.parse1.netchasm.net\n\
    allow-hosts: any\n\
    ttl: 600000\n\
    failure-response: dns\n\
    check-type: none\n\
    check-port: 123\n\
    check-info: hm-hello\n\
    group-threshold: 120\n\
    dual-stack-mode: both\n\
    dns-type: none\n\
    host:\n\
        - loadfb3.hm1.com\n\
        - loadfb3.hm2.com\n\
    \n\
-   name: config2.parse1.netchasm.net\n\
    allow-hosts: any\n\
    ttl: 60000\n\
    failure-response: dns\n\
    check-type: none\n\
    check-port: 1234\n\
    check-info: hm-hello\n\
    group-threshold: 12\n\
    dual-stack-mode: both\n\
    dns-type: none\n\
    host:\n\
        - loadfb3.hm1.com\n\
        - loadfb3.hm2.com\n"<< endl;

    fout1.close();
    fout3.close();

    setupCommon();
    string mdbm = "netchasm.mdbm";
    remove(mdbm.c_str());
    string host1 = "loadfb3.hm1.com";
    string host2 = "loadfb3.hm2.com";
    HMDataHostGroupMap groupMap;
    string checkInfo = "hm-hello";

    string hostGroupName = "config.parse1.netchasm.net";
    string hostGroupName1 = "config1.parse1.netchasm.net";
    string hostGroupName2 = "config2.parse1.netchasm.net";

    HMIPAddress address;
    address.set("192.168.1.3");

    HMIPAddress address2;
    address2.set("2001::7334");
    HMIPAddress address3;
    address3.set("2002::7334");

    HMDataHostGroup hostGroup(hostGroupName);
    hostGroup.setMeasurementOptions(0);
    hostGroup.setCheckType(HM_CHECK_NONE);
    hostGroup.setGroupThreshold(12);
    hostGroup.setCheckTTL(60000);
    hostGroup.setPort(123);
    hostGroup.setDualStack(HM_DUALSTACK_BOTH);
    hostGroup.setCheckInfo(checkInfo);
    hostGroup.addHost(host1);
    hostGroup.addHost(host2);

	HMDataHostGroup hostGroup1(hostGroupName1);
	hostGroup1.setMeasurementOptions(0);
	hostGroup1.setCheckType(HM_CHECK_NONE);
	hostGroup1.setGroupThreshold(120);
	hostGroup1.setCheckTTL(600000);
	hostGroup1.setPort(123);
	hostGroup1.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup1.setCheckInfo(checkInfo);
	hostGroup1.addHost(host1);
	hostGroup1.addHost(host2);

	HMDataHostGroup hostGroup2(hostGroupName2);
	hostGroup2.setMeasurementOptions(0);
	hostGroup2.setCheckType(HM_CHECK_NONE);
	hostGroup2.setGroupThreshold(12);
	hostGroup2.setCheckTTL(60000);
	hostGroup2.setPort(1234);
	hostGroup2.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup2.setCheckInfo(checkInfo);
	hostGroup2.addHost(host1);
	hostGroup2.addHost(host2);

    groupMap.insert(make_pair(hostGroupName, hostGroup));
    groupMap.insert(make_pair(hostGroupName1, hostGroup1));
    groupMap.insert(make_pair(hostGroupName2, hostGroup2));

    HMDataHostCheck hostCheck;
    hostGroup.getHostCheck(hostCheck);
    HMDataCheckParams checkParams;
    hostGroup.getCheckParameters(checkParams);
    checkParams.addHostGroup(hostGroupName);


    HMDataHostCheck hostCheck1;
	hostGroup1.getHostCheck(hostCheck1);
	HMDataCheckParams checkParams1;
	hostGroup1.getCheckParameters(checkParams1);
	checkParams1.addHostGroup(hostGroupName1);

	HMDataHostCheck hostCheck2;
	hostGroup2.getHostCheck(hostCheck2);
	HMDataCheckParams checkParams2;
	hostGroup2.getCheckParameters(checkParams2);
	checkParams2.addHostGroup(hostGroupName2);


    HMTimeStamp now;
    now.now();

    HMDataCheckResult checkResultg1h11;
    checkResultg1h11.m_checkTime = now;
    checkResultg1h11.m_response = HM_RESPONSE_CONNECTED;
    checkResultg1h11.m_reason = HM_REASON_SUCCESS;
    checkResultg1h11.m_responseTime = 33;
    checkResultg1h11.m_totalResponseTime = 66;
    checkResultg1h11.m_smoothedResponseTime = 44;
    checkResultg1h11.m_address = address;

    HMDataCheckResult checkResultg1h12;
	checkResultg1h12.m_checkTime = now;
	checkResultg1h12.m_response = HM_RESPONSE_CONNECTED;
	checkResultg1h12.m_reason = HM_REASON_SUCCESS;
	checkResultg1h12.m_responseTime = 34;
	checkResultg1h12.m_totalResponseTime = 67;
	checkResultg1h12.m_smoothedResponseTime = 45;
	checkResultg1h12.m_address = address3;

    HMDataCheckResult checkResultg2h11;
    checkResultg2h11.m_checkTime = now;
    checkResultg2h11.m_response = HM_RESPONSE_CONNECTED;
    checkResultg2h11.m_reason = HM_REASON_SUCCESS;
    checkResultg2h11.m_responseTime = 43;
    checkResultg2h11.m_totalResponseTime = 76;
    checkResultg2h11.m_smoothedResponseTime = 44;
    checkResultg2h11.m_address = address;

    HMDataCheckResult checkResultg2h12;
	checkResultg2h12.m_checkTime = now;
	checkResultg2h12.m_response = HM_RESPONSE_CONNECTED;
	checkResultg2h12.m_reason = HM_REASON_SUCCESS;
	checkResultg2h12.m_responseTime = 44;
	checkResultg2h12.m_totalResponseTime = 77;
	checkResultg2h12.m_smoothedResponseTime = 45;
	checkResultg2h12.m_address = address3;


    HMDataCheckResult checkResultg3h11;
    checkResultg3h11.m_checkTime = now;
    checkResultg3h11.m_response = HM_RESPONSE_CONNECTED;
    checkResultg3h11.m_reason = HM_REASON_SUCCESS;
    checkResultg3h11.m_responseTime = 53;
    checkResultg3h11.m_totalResponseTime = 86;
    checkResultg3h11.m_smoothedResponseTime = 44;
    checkResultg3h11.m_address = address;

    HMDataCheckResult checkResultg3h12;
	checkResultg3h12.m_checkTime = now;
	checkResultg3h12.m_response = HM_RESPONSE_CONNECTED;
	checkResultg3h12.m_reason = HM_REASON_SUCCESS;
	checkResultg3h12.m_responseTime = 54;
	checkResultg3h12.m_totalResponseTime = 87;
	checkResultg3h12.m_smoothedResponseTime = 45;
	checkResultg3h12.m_address = address3;


    HMDataCheckResult checkResulth2;
    checkResulth2.m_checkTime = now;
    checkResulth2.m_response = HM_RESPONSE_CONNECTED;
    checkResulth2.m_reason = HM_REASON_SUCCESS;
    checkResulth2.m_responseTime = 33;
    checkResulth2.m_totalResponseTime = 66;
    checkResulth2.m_smoothedResponseTime = 44;
    checkResulth2.m_address = address2;

    HMConfigInfo configInfo;
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;
    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName, hostGroup));
    checkState.m_hostGroups.insert(make_pair(hostGroupName1, hostGroup1));
    checkState.m_hostGroups.insert(make_pair(hostGroupName2, hostGroup2));
    HMDNSCache dnsCache;
    HMDNSLookup dnsHostCheck(HM_DNS_PLUGIN_ARES, false);
    HMDNSLookup dnsHostCheckv6(HM_DNS_PLUGIN_ARES, true);
    set<HMIPAddress> addresses;
    addresses.insert(address);
    dnsCache.updateDNSEntry(host1, dnsHostCheck, addresses);
    addresses.clear();
    addresses.insert(address3);
    dnsCache.updateDNSEntry(host1, dnsHostCheckv6, addresses);
    addresses.clear();
    addresses.insert(address2);
    dnsCache.updateDNSEntry(host2, dnsHostCheckv6, addresses);

    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(mdbm, &groupMap, &dnsCache);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(
            store->storeCheckResult(host1, address, hostCheck, checkParams,
                    checkResultg1h11));
	CPPUNIT_ASSERT(
			store->storeCheckResult(host1, address3, hostCheck, checkParams,
					checkResultg1h12));
	CPPUNIT_ASSERT(
			store->storeCheckResult(host1, address, hostCheck1, checkParams1,
					checkResultg2h11));
	CPPUNIT_ASSERT(
			store->storeCheckResult(host1, address3, hostCheck1, checkParams1,
					checkResultg2h12));
	CPPUNIT_ASSERT(
			store->storeCheckResult(host1, address, hostCheck2, checkParams2,
					checkResultg3h11));
	CPPUNIT_ASSERT(
			store->storeCheckResult(host1, address3, hostCheck2, checkParams2,
					checkResultg3h12));
    CPPUNIT_ASSERT(
            store->storeCheckResult(host2, address2, hostCheck, checkParams,
                    checkResulth2));
    CPPUNIT_ASSERT(
                store->storeCheckResult(host2, address2, hostCheck1, checkParams1,
                        checkResulth2));
    CPPUNIT_ASSERT(
                store->storeCheckResult(host2, address2, hostCheck2, checkParams2,
                        checkResulth2));

    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));
    std::this_thread::sleep_for(10ms);
    store->closeStore();
    delete store;
    teardownCommon();
    sm = new HMStateManager;
    string master_config = "conf/dummy_master.yaml";
    sm_thr = std::thread(&HMStateManager::healthCheck, std::ref(*sm),
            master_config, HM_LOG_ERROR);
    std::this_thread::sleep_for(2s);
}

void TESTNAME::tearDown()
{
    remove("conf/hm/testconf.yaml");
    remove("conf/dummy_master.yaml");
    remove("conf/hm");
    remove("conf");
    remove("netchasm.mdbm");
    sm->shutdown();
    std::this_thread::sleep_for(1s);
    sm_thr.join();
    delete(sm);
}


void TESTNAME::test_cmdlstnr1()
{
    string sock_path = "test_sock";
    HMControlLinuxSocketClient socketAPI(sock_path);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> results;

    HMAPIIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMAPIIPAddress addr2;
    addr2.set(ipaddr2);
    HMAPIIPAddress addr3;
    addr3.set(ipaddr3);
    string hostGroupName = "config.parse1.netchasm.net";
    string hostGroupName1 = "config1.parse1.netchasm.net";
    string hostGroupName2 = "config2.parse1.netchasm.net";
    CPPUNIT_ASSERT(socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
    CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL(3, (int )results.size());

    HMAPICheckResult infop = results[0];
    CPPUNIT_ASSERT_EQUAL((unsigned int)HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
    CPPUNIT_ASSERT(addr ==  infop.m_address);
    infop = results[1];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
	CPPUNIT_ASSERT(addr3 == infop.m_address);
    infop = results[2];
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm2.com" == infop.m_host);
    CPPUNIT_ASSERT(addr2 == infop.m_address);

    string hostName = "loadfb3.hm1.com";
    HMAPIDataHostCheck hostCheck;
    hostCheck.m_checkInfo = "hm-hello";
    hostCheck.m_port = 123;
    hostCheck.m_ipv4 = true;
    hostCheck.m_ipv6 = true;
    hostCheck.m_checkType = HM_API_CHECK_NONE;
    hostCheck.m_dnsCheckType = HM_API_DNS_NONE;
    vector<pair<HMAPICheckInfo, HMAPICheckResult>> hostResults;
    CPPUNIT_ASSERT(socketAPI.getHostResults(hostName, hostCheck, hostResults));
    CPPUNIT_ASSERT_EQUAL(4, (int)hostResults.size());
    pair<HMAPICheckInfo, HMAPICheckResult> testResult = hostResults[0];
	CPPUNIT_ASSERT_EQUAL(12, (int )testResult.first.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(60000, (int )testResult.first.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )testResult.second.m_reason);
	CPPUNIT_ASSERT(addr == testResult.second.m_address);
	CPPUNIT_ASSERT_EQUAL(33, (int)testResult.second.m_responseTime);
	CPPUNIT_ASSERT_EQUAL(66, (int )testResult.second.m_totalResponseTime);
    testResult = hostResults[1];
    CPPUNIT_ASSERT_EQUAL(12, (int )testResult.first.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )testResult.first.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )testResult.second.m_reason);
    CPPUNIT_ASSERT(addr3 == testResult.second.m_address);
    CPPUNIT_ASSERT_EQUAL(34, (int )testResult.second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(67, (int )testResult.second.m_totalResponseTime);
	testResult = hostResults[2];
	CPPUNIT_ASSERT_EQUAL(120, (int )testResult.first.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(600000, (int )testResult.first.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )testResult.second.m_reason);
	CPPUNIT_ASSERT(addr == testResult.second.m_address);
	CPPUNIT_ASSERT_EQUAL(43, (int )testResult.second.m_responseTime);
	CPPUNIT_ASSERT_EQUAL(76, (int )testResult.second.m_totalResponseTime);
	testResult = hostResults[3];
	CPPUNIT_ASSERT_EQUAL(120, (int )testResult.first.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(600000, (int )testResult.first.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )testResult.second.m_reason);
	CPPUNIT_ASSERT(addr3 == testResult.second.m_address);
	CPPUNIT_ASSERT_EQUAL(44, (int )testResult.second.m_responseTime);
	CPPUNIT_ASSERT_EQUAL(77, (int )testResult.second.m_totalResponseTime);
}


void TESTNAME::test_cmdlstnr2()
{
    string sock_path = "test_sock";
    HMControlLinuxSocketClient socketAPI(sock_path);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> results;

    HMAPIIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMAPIIPAddress addr2;
    addr2.set(ipaddr2);
    HMAPIIPAddress addr3;
    addr3.set(ipaddr3);
    string hostGroupName = "config.parse1.netchasm.net";
    string hostGroupName1 = "config1.parse1.netchasm.net";
    string hostGroupName2 = "config2.parse1.netchasm.net";
    CPPUNIT_ASSERT(socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
    CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL(3, (int )results.size());
    HMAPICheckResult infop = results[0];
    CPPUNIT_ASSERT_EQUAL((unsigned int)HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
    CPPUNIT_ASSERT(addr ==  infop.m_address);
    infop = results[1];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
	CPPUNIT_ASSERT(addr3 == infop.m_address);
    infop = results[2];
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm2.com" == infop.m_host);
    CPPUNIT_ASSERT(addr2 == infop.m_address);

    string hostName = "loadfb3.hm1.com";
    HMAPIDataHostCheck hostCheck;
    hostCheck.m_checkInfo = "hm-hello";
    hostCheck.m_port = 1234;
    hostCheck.m_ipv4 = true;
    hostCheck.m_ipv6 = true;
    hostCheck.m_checkType = HM_API_CHECK_NONE;
    hostCheck.m_dnsCheckType = HM_API_DNS_NONE;
    vector<pair<HMAPICheckInfo, HMAPICheckResult>> hostResults;
    CPPUNIT_ASSERT(socketAPI.getHostResults(hostName, hostCheck, hostResults));
	CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
	pair<HMAPICheckInfo, HMAPICheckResult> testResult = hostResults[0];
	CPPUNIT_ASSERT_EQUAL(12, (int )testResult.first.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(60000, (int )testResult.first.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )testResult.second.m_reason);
	CPPUNIT_ASSERT(addr == testResult.second.m_address);
	CPPUNIT_ASSERT_EQUAL(53, (int )testResult.second.m_responseTime);
	CPPUNIT_ASSERT_EQUAL(86, (int )testResult.second.m_totalResponseTime);
	testResult = hostResults[1];
	CPPUNIT_ASSERT_EQUAL(12, (int )testResult.first.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(60000, (int )testResult.first.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )testResult.second.m_reason);
	CPPUNIT_ASSERT(addr3 == testResult.second.m_address);
	CPPUNIT_ASSERT_EQUAL(54, (int )testResult.second.m_responseTime);
	CPPUNIT_ASSERT_EQUAL(87, (int )testResult.second.m_totalResponseTime);
}

void TESTNAME::test_cmdlstnr3()
{
    string sock_path = "test_sock";
    HMControlLinuxSocketClient socketAPI(sock_path);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> results;

    HMAPIIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMAPIIPAddress addr2;
    addr2.set(ipaddr2);
    HMAPIIPAddress addr3;
    addr3.set(ipaddr3);
    string hostGroupName = "config.parse1.netchasm.net";
    string hostGroupName1 = "config1.parse1.netchasm.net";
    string hostGroupName2 = "config2.parse1.netchasm.net";
    CPPUNIT_ASSERT(socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
    CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL(3, (int )results.size());
    CPPUNIT_ASSERT(socketAPI.getHostGroupResults(hostGroupName1, checkInfo, results));
    CPPUNIT_ASSERT_EQUAL(3, (int )results.size());
    HMAPICheckResult infop = results[0];
    CPPUNIT_ASSERT_EQUAL((unsigned int)HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
    CPPUNIT_ASSERT(addr ==  infop.m_address);
    infop = results[1];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
	CPPUNIT_ASSERT(addr3 == infop.m_address);
    infop = results[2];
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm2.com" == infop.m_host);
    CPPUNIT_ASSERT(addr2 == infop.m_address);

    string hostName = "loadfb3.hm1.com";
    HMAPIDataHostCheck hostCheck;
    hostCheck.m_checkInfo = "hm-hello";
    hostCheck.m_port = 12;
    hostCheck.m_ipv4 = true;
    hostCheck.m_ipv6 = true;
    hostCheck.m_checkType = HM_API_CHECK_NONE;
    hostCheck.m_dnsCheckType = HM_API_DNS_NONE;
    vector<pair<HMAPICheckInfo, HMAPICheckResult>> hostResults;
    CPPUNIT_ASSERT(!socketAPI.getHostResults(hostName, hostCheck, hostResults));
	CPPUNIT_ASSERT_EQUAL(0, (int )hostResults.size());
}


void TESTNAME::test_cmdlstnr4()
{
    string sock_path = "test_sock";
    HMControlLinuxSocketClient socketAPI(sock_path);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> results;

    HMAPIIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMAPIIPAddress addr2;
    addr2.set(ipaddr2);
    HMAPIIPAddress addr3;
    addr3.set(ipaddr3);

    string hostName = "loadfb3.hm1.com";
    HMAPIDataHostCheck hostCheck;
    hostCheck.m_checkInfo = "hm-hello";
    hostCheck.m_port = 123;
    hostCheck.m_ipv4 = true;
    hostCheck.m_ipv6 = true;
    hostCheck.m_checkType = HM_API_CHECK_NONE;
    hostCheck.m_dnsCheckType = HM_API_DNS_NONE;
    vector<pair<HMAPICheckInfo, HMAPICheckResult>> hostResults;
    CPPUNIT_ASSERT(socketAPI.getHostResults(hostName, addr, hostCheck, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int)hostResults.size());
    pair<HMAPICheckInfo, HMAPICheckResult> testResult = hostResults[0];
    CPPUNIT_ASSERT_EQUAL(12, (int )testResult.first.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )testResult.first.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )testResult.second.m_reason);
    CPPUNIT_ASSERT(addr == testResult.second.m_address);
    CPPUNIT_ASSERT_EQUAL(33, (int)testResult.second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(66, (int )testResult.second.m_totalResponseTime);
    testResult = hostResults[1];
    CPPUNIT_ASSERT_EQUAL(120, (int )testResult.first.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(600000, (int )testResult.first.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )testResult.second.m_reason);
    CPPUNIT_ASSERT(addr == testResult.second.m_address);
    CPPUNIT_ASSERT_EQUAL(43, (int )testResult.second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(76, (int )testResult.second.m_totalResponseTime);
    CPPUNIT_ASSERT(
            socketAPI.getHostResults(hostName, addr3, hostCheck, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    testResult = hostResults[0];
    CPPUNIT_ASSERT_EQUAL(12, (int )testResult.first.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )testResult.first.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )testResult.second.m_reason);
    CPPUNIT_ASSERT(addr3 == testResult.second.m_address);
    CPPUNIT_ASSERT_EQUAL(34, (int )testResult.second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(67, (int )testResult.second.m_totalResponseTime);
    testResult = hostResults[1];
    CPPUNIT_ASSERT_EQUAL(120, (int )testResult.first.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(600000, (int )testResult.first.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )testResult.second.m_reason);
    CPPUNIT_ASSERT(addr3 == testResult.second.m_address);
    CPPUNIT_ASSERT_EQUAL(44, (int )testResult.second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(77, (int )testResult.second.m_totalResponseTime);
}


void TESTNAME::test_cmdlstnr5()
{
    string sock_path = "test_sock";
    HMControlLinuxSocketClient socketAPI(sock_path);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> results;

    HMAPIIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMAPIIPAddress addr2;
    addr2.set(ipaddr2);
    HMAPIIPAddress addr3;
    addr3.set(ipaddr3);

    string hostName = "loadfb3.hm1.com";
    HMAPIDataHostCheck hostCheck;
    hostCheck.m_checkInfo = "hm-hello";
    hostCheck.m_port = 1234;
    hostCheck.m_ipv4 = true;
    hostCheck.m_ipv6 = true;
    hostCheck.m_checkType = HM_API_CHECK_NONE;
    hostCheck.m_dnsCheckType = HM_API_DNS_NONE;
    vector<pair<HMAPICheckInfo, HMAPICheckResult>> hostResults;
    CPPUNIT_ASSERT(socketAPI.getHostResults(hostName, addr, hostCheck, hostResults));
    CPPUNIT_ASSERT_EQUAL(1, (int )hostResults.size());
    pair<HMAPICheckInfo, HMAPICheckResult> testResult = hostResults[0];
    CPPUNIT_ASSERT_EQUAL(12, (int )testResult.first.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )testResult.first.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )testResult.second.m_reason);
    CPPUNIT_ASSERT(addr == testResult.second.m_address);
    CPPUNIT_ASSERT_EQUAL(53, (int )testResult.second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(86, (int )testResult.second.m_totalResponseTime);
    CPPUNIT_ASSERT(socketAPI.getHostResults(hostName, addr3, hostCheck, hostResults));
    CPPUNIT_ASSERT_EQUAL(1, (int )hostResults.size());
    testResult = hostResults[0];
    CPPUNIT_ASSERT_EQUAL(12, (int )testResult.first.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )testResult.first.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )testResult.first.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )testResult.second.m_reason);
    CPPUNIT_ASSERT(addr3 == testResult.second.m_address);
    CPPUNIT_ASSERT_EQUAL(54, (int )testResult.second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(87, (int )testResult.second.m_totalResponseTime);
}

void TESTNAME::test_cmdlstnr6()
{
    string sock_path = "test_sock";
    HMControlLinuxSocketClient socketAPI(sock_path);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> results;

    HMAPIIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMAPIIPAddress addr2;
    addr2.set(ipaddr2);
    HMAPIIPAddress addr3;
    addr3.set(ipaddr3);
    string hostName = "loadfb3.hm1.com";
    HMAPIDataHostCheck hostCheck;
    hostCheck.m_checkInfo = "hm-hello";
    hostCheck.m_port = 1234;
    hostCheck.m_ipv4 = true;
    hostCheck.m_ipv6 = true;
    hostCheck.m_checkType = HM_API_CHECK_NONE;
    hostCheck.m_dnsCheckType = HM_API_DNS_NONE;
    vector<pair<HMAPICheckInfo, HMAPICheckResult>> hostResults;
    CPPUNIT_ASSERT(socketAPI.getHostResults(hostName, hostCheck, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    CPPUNIT_ASSERT(!socketAPI.getHostResults(hostName, addr2, hostCheck, hostResults));
    hostCheck.m_port = 12;
    CPPUNIT_ASSERT(!socketAPI.getHostResults(hostName, hostCheck, hostResults));
    CPPUNIT_ASSERT(!socketAPI.getHostResults(hostName, addr2, hostCheck, hostResults));
}

void TESTNAME::test_cmdlstnr7()
{
    string sock_path = "test_sock";
    HMControlLinuxSocketClient socketAPI(sock_path);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> results;

    vector<HMAPIIPAddress> addrs, addrsret, addrsrem;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    HMAPIIPAddress addr;
    addr.set(ipaddr);
    addrs.push_back(addr);
    addrsrem.push_back(addr);
    HMAPIIPAddress addr2;
    addr2.set(ipaddr2);
    addrs.push_back(addr2);
    addrsrem.push_back(addr2);
    HMAPIIPAddress addr3;
    addr3.set(ipaddr3);
    addrs.push_back(addr3);
    string hostName = "test.hm1.com";

    CPPUNIT_ASSERT(socketAPI.addDNSAddresses(hostName, addrs));
    CPPUNIT_ASSERT(socketAPI.getDNSAddresses(hostName, addrsret));
    CPPUNIT_ASSERT_EQUAL(3, (int )addrsret.size());
    CPPUNIT_ASSERT(std::find(addrsret.begin(), addrsret.end(), addr) != addrsret.end());
    CPPUNIT_ASSERT(std::find(addrsret.begin(), addrsret.end(), addr2) != addrsret.end());
    CPPUNIT_ASSERT(std::find(addrsret.begin(), addrsret.end(), addr3) != addrsret.end());
    CPPUNIT_ASSERT(socketAPI.removeDNSAddresses(hostName, addrsrem));
    CPPUNIT_ASSERT(socketAPI.getDNSAddresses(hostName, addrsret));
    CPPUNIT_ASSERT_EQUAL(1, (int )addrsret.size());
    CPPUNIT_ASSERT(std::find(addrsret.begin(), addrsret.end(), addr3) != addrsret.end());
}
