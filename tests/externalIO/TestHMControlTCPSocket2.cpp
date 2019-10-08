// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlTCPSocket2.h"

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
#include "HMControlTCPSocketClient.h"
using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    mkdir("conf", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("conf/hm", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    std::ofstream fout1("conf/dummy_master.yaml");
    std::ofstream fout3("conf/hm/testconf.yaml");

    fout1 << "threads: 2\n\
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
control-server-linux: off\n\
master-check-portv4 : 10052\n\
control-server-ipv4 : on\n\
control-server-ipv6 : off\n\
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
    host-group:\n\
        - config.parse2.netchasm.net\n\
    host:\n\
        - lfb3.hm1.com\n\
        - lfb3.hm2.com\n\
-   name: config.parse2.netchasm.net\n\
    allow-hosts: any\n\
    ttl: 60000\n\
    failure-response: dns\n\
    check-type: none\n\
    check-port: 123\n\
    check-info: hm-hello\n\
    group-threshold: 12\n\
    host:\n\
        - lfb3.hm1.com\n\
        - lfb3.hm2.com\n" << endl;

    fout1.close();
    fout3.close();
//    host-group:\n\
//            - test.hostgroup.net\n\
            
    string ipaddr = "127.0.0.1";
    port = 10052;
    server.set(ipaddr);
    setupCommon();
    string mdbm = "netchasm.mdbm";
    remove(mdbm.c_str());
    string hostGroupName = "config.parse1.netchasm.net";
    string host1 = "lfb3.hm1.com";
    string host2 = "lfb3.hm2.com";
    string hostgroup1 = "config.parse2.netchasm.net";
    HMDataHostGroupMap groupMap;
    string checkInfo = "hm-hello";


    HMIPAddress address;
    address.set("192.168.1.3");

    HMIPAddress address2;
    address2.set("192.168.5.1");

    HMIPAddress address3;
    address3.set("192.168.1.5");


    HMDataHostGroup hostGroup(hostGroupName);
    hostGroup.setPassthroughInfo(0);
    hostGroup.setCheckType(HM_CHECK_NONE);
    hostGroup.setGroupThreshold(12);
    hostGroup.setCheckTTL(60000);
    hostGroup.setPort(123);
    hostGroup.setCheckInfo(checkInfo);
    hostGroup.addHostGroup(hostgroup1);
    hostGroup.addHost(host1);
    hostGroup.addHost(host2);
    groupMap.insert(make_pair(hostGroupName, hostGroup));
    HMDataHostCheck hostCheck;
    hostGroup.getHostCheck(hostCheck);

    HMDataCheckParams checkParams;
    hostGroup.getCheckParameters(checkParams);
    checkParams.addHostGroup(hostGroupName);

    now = HMTimeStamp::now();

    HMDataCheckResult checkResult;
    checkResult.m_checkTime = now;
    checkResult.m_response = HM_RESPONSE_CONNECTED;
    checkResult.m_reason = HM_REASON_SUCCESS;
    checkResult.m_responseTime = 33;
    checkResult.m_totalResponseTime = 66;
    checkResult.m_smoothedResponseTime = 44;
    checkResult.m_address = address;

    HMDataCheckResult checkResult2;
    checkResult2.m_checkTime = now;
    checkResult2.m_response = HM_RESPONSE_CONNECTED;
    checkResult2.m_reason = HM_REASON_SUCCESS;
    checkResult2.m_responseTime = 33;
    checkResult2.m_totalResponseTime = 66;
    checkResult2.m_smoothedResponseTime = 44;
    checkResult2.m_address = address2;

    HMDataCheckResult checkResult3;
    checkResult3.m_checkTime = now - 100;
    checkResult3.m_response = HM_RESPONSE_CONNECTED;
    checkResult3.m_reason = HM_REASON_SUCCESS;
    checkResult3.m_responseTime = 34;
    checkResult3.m_totalResponseTime = 67;
    checkResult3.m_smoothedResponseTime = 45;
    checkResult3.m_address = address3;


    HMConfigInfo configInfo;
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;

    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName, hostGroup));
    HMDNSCache dnsCache;
    HMDNSLookup dnsHostCheck(HM_DNS_PLUGIN_ARES, false);
    set<HMIPAddress> addresses;
    addresses.insert(address);
    addresses.insert(address3);
    dnsCache.updateDNSEntry(host1, dnsHostCheck, addresses);
    addresses.clear();
    addresses.insert(address2);
    dnsCache.updateDNSEntry(host2, dnsHostCheck, addresses);
    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(mdbm, &groupMap, &dnsCache);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigs(checkState));
    CPPUNIT_ASSERT(
            store->storeCheckResult(host1, address, hostCheck, checkParams,
                    checkResult));
    CPPUNIT_ASSERT(
                store->storeCheckResult(host1, address3, hostCheck, checkParams,
                        checkResult3));
    CPPUNIT_ASSERT(
            store->storeCheckResult(host2, address2, hostCheck, checkParams,
                    checkResult2));
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
    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT(state);
    state.reset();
}

void TESTNAME::tearDown()
{
    remove("conf/hm/testconf.yaml");
    remove("conf/dummy_master.yaml");
    remove("conf/dummy_master2.yaml");
    remove("conf/dummy_master3.yaml");
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
    in_addr addr;
    inet_aton("192.168.1.3", &addr);
    in_addr addr2;
    inet_pton(AF_INET,"192.168.5.1",&addr2);
    in_addr addr3;
    inet_pton(AF_INET, "192.168.1.5", &addr3);

    string hostGroupName = "config.parse1.netchasm.net";
    string hostgroup1 = "config.parse2.netchasm.net";
    HMControlTCPSocketClient socketAPI(server, port);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> hostResults;
    CPPUNIT_ASSERT(socketAPI.getHostGroupResults(hostGroupName, checkInfo, hostResults));
    CPPUNIT_ASSERT_EQUAL(12, (int)checkInfo.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(1, (int)checkInfo.m_hostGroups.size());
    CPPUNIT_ASSERT_EQUAL(hostgroup1, checkInfo.m_hostGroups[0]);
    CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL(3, (int )hostResults.size());
    HMAPICheckResult infop = hostResults[0];
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT_EQUAL((unsigned int)AF_INET,
            (unsigned int )infop.m_address.m_type);
    CPPUNIT_ASSERT("lfb3.hm1.com" == infop.m_host);
    CPPUNIT_ASSERT_EQUAL(addr.s_addr, infop.m_address.m_ip.addr);
    HMAPICheckResult infop1 = hostResults[1];
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )infop1.m_reason);
    CPPUNIT_ASSERT_EQUAL((unsigned int)AF_INET,
            (unsigned int )infop1.m_address.m_type);
    CPPUNIT_ASSERT("lfb3.hm1.com" == infop1.m_host);
    CPPUNIT_ASSERT_EQUAL(addr3.s_addr, infop1.m_address.m_ip.addr);
    HMAPICheckResult infop2 = hostResults[2];
    CPPUNIT_ASSERT("lfb3.hm2.com" == infop2.m_host);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )infop2.m_reason);
    CPPUNIT_ASSERT_EQUAL((unsigned int)AF_INET,
            (unsigned int )infop2.m_address.m_type);
    CPPUNIT_ASSERT_EQUAL(addr2.s_addr, infop2.m_address.m_ip.addr);
}


void TESTNAME::test_cmdlstnr2()
{
    string hostGroupName = "config.parse3.netchasm.net";
    HMControlTCPSocketClient socketAPI(server, port);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> hostResults;
    CPPUNIT_ASSERT(!
            socketAPI.getHostGroupResults(hostGroupName, checkInfo,
                    hostResults));

}

void TESTNAME::test_cmdlstnr3()
{
    in_addr addr;
    inet_aton("192.168.1.3", &addr);
    in_addr addr3;
    inet_pton(AF_INET, "192.168.1.5", &addr3);
    HMControlTCPSocketClient socketAPI(server, port);
    string hostGroupName = "config.parse1.netchasm.net";
    string hostName = "lfb3.hm1.com";
    vector<HMAPICheckResult> results;
    CPPUNIT_ASSERT(socketAPI.getHostResults(hostGroupName, hostName, results));
    CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
    HMAPICheckResult infop = results[0];
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT_EQUAL((unsigned int)AF_INET,
            (unsigned int )infop.m_address.m_type);
    CPPUNIT_ASSERT("lfb3.hm1.com" == infop.m_host);
    CPPUNIT_ASSERT_EQUAL(addr.s_addr, infop.m_address.m_ip.addr);
    CPPUNIT_ASSERT_EQUAL(33, (int)infop.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(44, (int)infop.m_smoothedResponseTime);
    CPPUNIT_ASSERT_EQUAL(0, (int)infop.m_status);
    CPPUNIT_ASSERT_EQUAL(now.getTimeSinceEpoch(), (uint64_t)infop.m_checkTime);
    HMAPICheckResult infop1 = results[1];
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )infop1.m_reason);
    CPPUNIT_ASSERT_EQUAL((unsigned int)AF_INET,
            (unsigned int )infop1.m_address.m_type);
    CPPUNIT_ASSERT("lfb3.hm1.com" == infop1.m_host);
    CPPUNIT_ASSERT_EQUAL(34, (int )infop1.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(45, (int )infop1.m_smoothedResponseTime);
    CPPUNIT_ASSERT_EQUAL(0, (int )infop1.m_status);
    CPPUNIT_ASSERT_EQUAL(addr3.s_addr, infop1.m_address.m_ip.addr);
    CPPUNIT_ASSERT_EQUAL(now.getTimeSinceEpoch() - 100,
            (uint64_t )infop1.m_checkTime);

}
