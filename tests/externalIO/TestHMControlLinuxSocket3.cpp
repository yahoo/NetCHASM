// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlLinuxSocket3.h"

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
    sock_fd = 0;
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
dns.statictype: none\n\
dns.lookuptype: none\n\
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
    host:\n\
        - loadfb3.hm1.com\n\
        - loadfb3.hm2.com\n" << endl;

    fout1.close();
    fout3.close();

    setupCommon();
    string mdbm = "netchasm.mdbm";
    remove(mdbm.c_str());
    string hostGroupName = "config.parse1.netchasm.net";
    string host1 = "loadfb3.hm1.com";
    string host2 = "loadfb3.hm2.com";
    HMDataHostGroupMap groupMap;
    string checkInfo = "hm-hello";


    HMIPAddress address;
    address.set("192.168.1.3");

    HMIPAddress address2;
    address2.set("2001::7334");

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

    groupMap.insert(make_pair(hostGroupName, hostGroup));
    HMDataHostCheck hostCheck;
    hostGroup.getHostCheck(hostCheck);

    HMDataCheckParams checkParams;
    hostGroup.getCheckParameters(checkParams);
    checkParams.addHostGroup(hostGroupName);

    HMTimeStamp now;
    now.now();

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

    HMConfigInfo configInfo;
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;
    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName, hostGroup));
    HMDNSCache dnsCache;
    HMDNSLookup dnsHostCheck(HM_DNS_TYPE_LOOKUP, false);
    set<HMIPAddress> addresses;
    addresses.insert(address);
    dnsCache.insertDNSEntry(host1, dnsHostCheck, 10000, 10000);
    dnsCache.updateDNSEntry(host1, dnsHostCheck, addresses);
    addresses.clear();
    addresses.insert(address2);
    dnsCache.insertDNSEntry(host2, dnsHostCheck, 10000, 10000);
    dnsCache.updateDNSEntry(host2, dnsHostCheck, addresses);
    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(mdbm, &groupMap, &dnsCache);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(
            store->storeCheckResult(host1, address, hostCheck, checkParams,
                    checkResult));
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
    if(sock_fd != 0)
    {
        close(sock_fd);
    }
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
    struct sockaddr_un server;
    string cmd = "host_set";
    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    state.reset();
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0) {
        close(sock_fd);
        perror("connecting error");
        exit(1);
    }
    uint64_t cmdlen = cmd.length();
    cmdlen = HMDataPacking::hton64(cmdlen);
    if (write(sock_fd, &cmdlen, sizeof(cmdlen)) < 0)
        perror("writing on stream socket");
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        perror("writing on stream socket");
    close(sock_fd);
    sock_fd = 0;
    std::this_thread::sleep_for(3s);
    sm->updateState(state);
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_CHECK_PLUGIN_HTTP_CURL,
            (int )state->getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
}

void TESTNAME::test_cmdlstnr2()
{
    string sock_path = "test_sock";
    HMControlLinuxSocketClient socketAPI(sock_path);
    HMAPICheckInfo checkInfo;
    vector<HMAPICheckResult> results;

    HMAPIIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr1 = "2001::7334";
    addr.set(ipaddr);
    HMAPIIPAddress addr2;
    addr2.set(ipaddr1);
    string hostGroupName = "config.parse1.netchasm.net";
    CPPUNIT_ASSERT(socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
    CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL(2, (int )results.size());
    HMAPICheckResult infop = results[0];
    CPPUNIT_ASSERT_EQUAL((unsigned int)HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
    CPPUNIT_ASSERT(addr ==  infop.m_address);
    CPPUNIT_ASSERT(!infop.m_forceHostDown);
    HMAPICheckResult infop1 = results[1];
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS, (unsigned int )infop1.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm2.com" == infop1.m_host);
    CPPUNIT_ASSERT(addr2 == infop1.m_address);
    CPPUNIT_ASSERT(!infop1.m_forceHostDown);

    string hostName = "loadfb3.hm1.com";
    CPPUNIT_ASSERT(socketAPI.setForceStatusDown(hostGroupName, hostName));
    // TODO: needs to enabled after host_set fix
    /*std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
    CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
    CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL(2, (int )results.size());
    infop = results[0];
    CPPUNIT_ASSERT_EQUAL((unsigned int)HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
    CPPUNIT_ASSERT(addr ==  infop.m_address);
    CPPUNIT_ASSERT(infop.m_forceHostDown);
    infop1 = results[1];
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS, (unsigned int )infop1.m_reason);
    CPPUNIT_ASSERT("loadfb3.hm2.com" == infop1.m_host);
    CPPUNIT_ASSERT(addr2 == infop1.m_address);
    CPPUNIT_ASSERT(!infop1.m_forceHostDown);
     */
}

void TESTNAME::test_cmdlstnr3()
{
	string hostGroupName = "config.parse1.netchasm.net";
	string sock_path = "test_sock";
	HMControlLinuxSocketClient socketAPI(sock_path);
	HMAPICheckInfo checkInfo;
	vector<HMAPICheckResult> results;
	HMAPIIPAddress addr;
	string ipaddr = "192.168.1.3";
	string ipaddr1 = "2001::7334";
	addr.set(ipaddr);
	HMAPIIPAddress addr2;
	addr2.set(ipaddr1);
	CPPUNIT_ASSERT(
			socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
	CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL(2, (int )results.size());
	HMAPICheckResult infop = results[0];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
	CPPUNIT_ASSERT(addr == infop.m_address);
	CPPUNIT_ASSERT(!infop.m_forceHostDown);
	HMAPICheckResult infop1 = results[1];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop1.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm2.com" == infop1.m_host);
	CPPUNIT_ASSERT(addr2 == infop1.m_address);
	CPPUNIT_ASSERT(!infop1.m_forceHostDown);
	CPPUNIT_ASSERT(socketAPI.setForceStatusDown(hostGroupName, addr2));
	// TODO: needs to enabled after host_set fix
	/*std::this_thread::sleep_for(1s);
	 CPPUNIT_ASSERT(socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
	 CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
	 CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
	 CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
	 CPPUNIT_ASSERT_EQUAL(2, (int )results.size());
	 infop = results[0];
	 CPPUNIT_ASSERT_EQUAL((unsigned int)HM_REASON_SUCCESS, (unsigned int )infop.m_reason);
	 CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
	 CPPUNIT_ASSERT(addr ==  infop.m_address);
	 CPPUNIT_ASSERT(!infop.m_forceHostDown);
	 infop1 = results[1];
	 CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS, (unsigned int )infop1.m_reason);
	 CPPUNIT_ASSERT("loadfb3.hm2.com" == infop1.m_host);
	 CPPUNIT_ASSERT(addr2 == infop1.m_address);
	 CPPUNIT_ASSERT(infop1.m_forceHostDown);*/
 }

void TESTNAME::test_cmdlstnr4()
{
	string hostGroupName = "config.parse1.netchasm.net";
	string sock_path = "test_sock";
	HMControlLinuxSocketClient socketAPI(sock_path);
	HMAPICheckInfo checkInfo;
	vector<HMAPICheckResult> results;
	HMAPIIPAddress addr;
	string ipaddr = "192.168.1.3";
	string ipaddr1 = "2001::7334";
	addr.set(ipaddr);
	HMAPIIPAddress addr2;
	addr2.set(ipaddr1);
	CPPUNIT_ASSERT(
			socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
	CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL(2, (int )results.size());
	HMAPICheckResult infop = results[0];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
	CPPUNIT_ASSERT(addr == infop.m_address);
	CPPUNIT_ASSERT(!infop.m_forceHostDown);
	HMAPICheckResult infop1 = results[1];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop1.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm2.com" == infop1.m_host);
	CPPUNIT_ASSERT(addr2 == infop1.m_address);
	CPPUNIT_ASSERT(!infop1.m_forceHostDown);
	HMAPIIPAddress address;
	string ipaddr2 = "102.131.120.134";
	address.set(ipaddr2);
	CPPUNIT_ASSERT(socketAPI.setForceStatusDown(hostGroupName, address));
	std::this_thread::sleep_for(1s);
	CPPUNIT_ASSERT(
			socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
	CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL(2, (int )results.size());
	infop = results[0];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
	CPPUNIT_ASSERT(addr == infop.m_address);
	CPPUNIT_ASSERT(!infop.m_forceHostDown);
	infop1 = results[1];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop1.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm2.com" == infop1.m_host);
	CPPUNIT_ASSERT(addr2 == infop1.m_address);
	CPPUNIT_ASSERT(!infop1.m_forceHostDown);
}

void TESTNAME::test_cmdlstnr5()
{
	string hostGroupName = "config.parse1.netchasm.net";
	string sock_path = "test_sock";
	HMControlLinuxSocketClient socketAPI(sock_path);
	HMAPICheckInfo checkInfo;
	vector<HMAPICheckResult> results;
	HMAPIIPAddress addr;
	string ipaddr = "192.168.1.3";
	string ipaddr1 = "2001::7334";
	addr.set(ipaddr);
	HMAPIIPAddress addr2;
	addr2.set(ipaddr1);
	CPPUNIT_ASSERT(
			socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
	CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL(2, (int )results.size());
	HMAPICheckResult infop = results[0];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
	CPPUNIT_ASSERT(addr == infop.m_address);
	CPPUNIT_ASSERT(!infop.m_forceHostDown);
	HMAPICheckResult infop1 = results[1];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop1.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm2.com" == infop1.m_host);
	CPPUNIT_ASSERT(addr2 == infop1.m_address);
	CPPUNIT_ASSERT(!infop1.m_forceHostDown);
	string hostName = "wrongload.hm2.com";
	CPPUNIT_ASSERT(socketAPI.setForceStatusDown(hostGroupName, hostName));
	std::this_thread::sleep_for(1s);
	CPPUNIT_ASSERT(
			socketAPI.getHostGroupResults(hostGroupName, checkInfo, results));
	CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
	CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
	CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
	CPPUNIT_ASSERT_EQUAL(2, (int )results.size());
	infop = results[0];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm1.com" == infop.m_host);
	CPPUNIT_ASSERT(addr == infop.m_address);
	CPPUNIT_ASSERT(!infop.m_forceHostDown);
	infop1 = results[1];
	CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
			(unsigned int )infop1.m_reason);
	CPPUNIT_ASSERT("loadfb3.hm2.com" == infop1.m_host);
	CPPUNIT_ASSERT(addr2 == infop1.m_address);
	CPPUNIT_ASSERT(!infop1.m_forceHostDown);

}
