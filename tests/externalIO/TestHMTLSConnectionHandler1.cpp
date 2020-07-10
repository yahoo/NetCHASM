// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.#ifndef TESTS_LIBRARYTESTS_TESTHMLIBRARY_H_
#include "TestHMTLSConnectionHandler1.h"

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
#include "HMConnectionHandler.h"

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
dns.statictype: none\n\
dns.lookuptype: none\n\
http.type: curl\n\
ftp.type: curl\n\
tcp.type: rawsocket\n\
dnsvc.type: ares\n\
none.type: none\n\
db.type: mdbm\n\
db.path: healthmon.mdbm\n\
log.path: hm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
control-server-linux: on\n\
control-socket-check-portv4 : 10053\n\
control-server-ipv4 : on\n\
control-server-ipv6 : off\n\
enable-secure-remote : on\n\
cert-file: " + std::string(CERT_FOLDER) + "/server.crt\n\
key-file: " + std::string(CERT_FOLDER) + "/server.key\n\
ca-file: " + std::string(CERT_FOLDER) + "/ca.crt\n\
socket.path: test_sock" << endl;

    fout3 << "-   name: config.parse1.healthmon.net\n\
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
        - loadfb3.hm2.com\n\
    \n\
-   name: config1.parse1.healthmon.net\n\
    allow-hosts: any\n\
    ttl: 600000\n\
    failure-response: dns\n\
    check-type: none\n\
    check-port: 123\n\
    check-info: hm-hello\n\
    group-threshold: 120\n\
    dual-stack-mode: both\n\
    host:\n\
        - loadfb3.hm1.com\n\
        - loadfb3.hm2.com\n\
    \n\
-   name: config2.parse1.healthmon.net\n\
    allow-hosts: any\n\
    ttl: 60000\n\
    failure-response: dns\n\
    check-type: none\n\
    check-port: 1234\n\
    check-info: hm-hello\n\
    group-threshold: 12\n\
    dual-stack-mode: both\n\
    host:\n\
        - loadfb3.hm1.com\n\
        - loadfb3.hm2.com\n"<< endl;

    fout1.close();
    fout3.close();
    string ipaddr = "127.0.0.1";
    server.set(ipaddr);
    port = HM_CONTROL_SOCKET_DEFAULT_PORTV4;

    setupCommon();
    string mdbm = "healthmon.mdbm";
    remove(mdbm.c_str());
    string host1 = "loadfb3.hm1.com";
    string host2 = "loadfb3.hm2.com";
    HMDataHostGroupMap groupMap;
    string checkInfo = "hm-hello";

    string hostGroupName = "config.parse1.healthmon.net";
    string hostGroupName1 = "config1.parse1.healthmon.net";
    string hostGroupName2 = "config2.parse1.healthmon.net";

    std::unique_ptr<HMAuxLoadFB> temp_aux = std::make_unique<HMAuxLoadFB>();
    temp_aux->m_type = HM_LOAD_FILE;
    temp_aux->m_datacenter = "d3";
    temp_aux->m_host = "h1.hm.com";
    temp_aux->m_resource = "r1";
    temp_aux->m_load = 11;
    temp_aux->m_target = 2500;
    temp_aux->m_max = 5000;
    auxInfo.m_auxData.push_back(std::move(temp_aux));
    temp_aux = std::make_unique<HMAuxLoadFB>();
    temp_aux->m_type = HM_LOAD_FILE;
    temp_aux->m_datacenter = "D3";
    temp_aux->m_host = "h2.hm.com";
    temp_aux->m_resource = "r2";
    temp_aux->m_load = 3;
    temp_aux->m_target = 38;
    temp_aux->m_max = 76;
    auxInfo.m_auxData.push_back(std::move(temp_aux));
    std::unique_ptr<HMAuxLoadFB> temp_aux1 = std::make_unique<HMAuxLoadFB>();
    temp_aux1->m_type = HM_LOAD_FILE;
    temp_aux1->m_datacenter = "D3";
    temp_aux1->m_host = "h3.hm.com";
    temp_aux1->m_resource = "r3";
    temp_aux1->m_load = 118;
    temp_aux1->m_target = 500;
    temp_aux1->m_max = 2000;
    auxInfo1.m_auxData.push_back(std::move(temp_aux1));
    std::unique_ptr<HMAuxOOB> temp_aux2 = std::make_unique<HMAuxOOB>();
    temp_aux2->m_type = HM_OOB_FILE;
    temp_aux2->m_host = "h4.hm.com";
    temp_aux2->m_resource = "r4";
    temp_aux2->m_shed = 0;
    temp_aux2->m_forceDown = HM_OOB_FORCEDOWN_FALSE;
    auxInfo2.m_auxData.push_back(std::move(temp_aux2));
    temp_aux2 = std::make_unique<HMAuxOOB>();
    temp_aux2->m_type = HM_OOB_FILE;
    temp_aux2->m_host = "h5.hm.com";
    temp_aux2->m_resource = "r5";
    temp_aux2->m_shed = 20;
    temp_aux2->m_forceDown = HM_OOB_FORCEDOWN_TRUE;
    auxInfo2.m_auxData.push_back(std::move(temp_aux2));

    HMIPAddress address;
    address.set("192.168.1.3");

    HMIPAddress address2;
    address2.set("2001::7334");
    HMIPAddress address3;
    address3.set("2002::7334");

    HMDataHostGroup hostGroup(hostGroupName);
    hostGroup.setPassthroughInfo(0);
    hostGroup.setCheckType(HM_CHECK_NONE);
    hostGroup.setGroupThreshold(12);
    hostGroup.setCheckTTL(60000);
    hostGroup.setPort(123);
    hostGroup.setDualStack(HM_DUALSTACK_BOTH);
    hostGroup.setCheckInfo(checkInfo);
    hostGroup.addHost(host1);
    hostGroup.addHost(host2);
    HMHashMD5 hashMD5;
    hashMD5.init();
    hostGroup.getHash(hashMD5);
    hashMD5.final(hash);

	HMDataHostGroup hostGroup1(hostGroupName1);
	hostGroup1.setPassthroughInfo(0);
	hostGroup1.setCheckType(HM_CHECK_NONE);
	hostGroup1.setGroupThreshold(120);
	hostGroup1.setCheckTTL(600000);
	hostGroup1.setPort(123);
	hostGroup1.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup1.setCheckInfo(checkInfo);
	hostGroup1.addHost(host1);
	hostGroup1.addHost(host2);
    HMHashMD5 hashMD5_1;
    hashMD5_1.init();
    hostGroup1.getHash(hashMD5_1);
    hashMD5_1.final(hash1);

	HMDataHostGroup hostGroup2(hostGroupName2);
	hostGroup2.setPassthroughInfo(0);
	hostGroup2.setCheckType(HM_CHECK_NONE);
	hostGroup2.setGroupThreshold(12);
	hostGroup2.setCheckTTL(60000);
	hostGroup2.setPort(1234);
	hostGroup2.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup2.setCheckInfo(checkInfo);
	hostGroup2.addHost(host1);
	hostGroup2.addHost(host2);
    HMHashMD5 hashMD5_2;
    hashMD5_2.init();
    hostGroup2.getHash(hashMD5_2);
    hashMD5_2.final(hash2);

    groupMap.insert(make_pair(hostGroupName, hostGroup));
    groupMap.insert(make_pair(hostGroupName1, hostGroup1));
    groupMap.insert(make_pair(hostGroupName2, hostGroup2));

    hostGroup.getHostCheck(hostCheck);
    hostGroup.getCheckParameters(checkParams);
    checkParams.addHostGroup(hostGroupName);

	hostGroup1.getHostCheck(hostCheck1);
	hostGroup1.getCheckParameters(checkParams1);
	checkParams1.addHostGroup(hostGroupName1);

	hostGroup2.getHostCheck(hostCheck2);
	hostGroup2.getCheckParameters(checkParams2);
	checkParams2.addHostGroup(hostGroupName2);


    HMTimeStamp now;
    now.now();

    checkResultg1h11.m_checkTime = now;
    checkResultg1h11.m_response = HM_RESPONSE_CONNECTED;
    checkResultg1h11.m_reason = HM_REASON_SUCCESS;
    checkResultg1h11.m_responseTime = 33;
    checkResultg1h11.m_totalResponseTime = 66;
    checkResultg1h11.m_smoothedResponseTime = 44;
    checkResultg1h11.m_address = address;

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


    checkResultg3h11.m_checkTime = now;
    checkResultg3h11.m_response = HM_RESPONSE_CONNECTED;
    checkResultg3h11.m_reason = HM_REASON_SUCCESS;
    checkResultg3h11.m_responseTime = 53;
    checkResultg3h11.m_totalResponseTime = 86;
    checkResultg3h11.m_smoothedResponseTime = 44;
    checkResultg3h11.m_address = address;

	checkResultg3h12.m_checkTime = now;
	checkResultg3h12.m_response = HM_RESPONSE_CONNECTED;
	checkResultg3h12.m_reason = HM_REASON_SUCCESS;
	checkResultg3h12.m_responseTime = 54;
	checkResultg3h12.m_totalResponseTime = 87;
	checkResultg3h12.m_smoothedResponseTime = 45;
	checkResultg3h12.m_address = address3;


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
    set<HMIPAddress> addresses;
    addresses.insert(address);
    HMDNSLookup dns4(HM_DNS_TYPE_LOOKUP, false);
    dns4.setPlugin(HM_DNS_PLUGIN_NONE);
    HMDNSLookup dns6(HM_DNS_TYPE_LOOKUP, true);
    dns6.setPlugin(HM_DNS_PLUGIN_NONE);
    dnsCache.insertDNSEntry(host1, dns4, 10000, 10000);
    dnsCache.insertDNSEntry(host1, dns6, 10000, 10000);
    dnsCache.insertDNSEntry(host2, dns4, 10000, 10000);
    dnsCache.insertDNSEntry(host2, dns6, 10000, 10000);

    dnsCache.updateDNSEntry(host1, dns4, addresses);

    addresses.clear();
    addresses.insert(address3);
    dnsCache.updateDNSEntry(host1, dns6, addresses);
    addresses.clear();
    addresses.insert(address2);
    dnsCache.updateDNSEntry(host2, dns6, addresses);;
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

    CPPUNIT_ASSERT(
            store->storeAuxInfo(host1, address, hostCheck, checkParams,
                    auxInfo));
    CPPUNIT_ASSERT(
            store->storeAuxInfo(host1, address3, hostCheck, checkParams,
                    auxInfo2));
    CPPUNIT_ASSERT(
            store->storeAuxInfo(host2, address2, hostCheck, checkParams,
                    emptyAuxInfo));
    CPPUNIT_ASSERT(
            store->storeAuxInfo(host1, address, hostCheck2, checkParams2,
                    auxInfo));
    CPPUNIT_ASSERT(
            store->storeAuxInfo(host1, address3, hostCheck2, checkParams2,
                    auxInfo2));
    CPPUNIT_ASSERT(
            store->storeAuxInfo(host2, address2, hostCheck2, checkParams2,
                    auxInfo1));

    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));
    std::this_thread::sleep_for(10ms);
    store->closeStore();
    delete store;

    //set up context
    certfile = std::string(CERT_FOLDER) + "/client.crt";
    keyfile = std::string(CERT_FOLDER) + "/client.key";
    caFile = std::string(CERT_FOLDER) + "/ca.crt";
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    m_ctx = SSL_CTX_new(TLSv1_2_method());
    if (SSL_CTX_use_certificate_file(m_ctx, certfile.c_str(),
    SSL_FILETYPE_PEM) <= 0)
    {
        CPPUNIT_ASSERT(false);
    }
    if (SSL_CTX_use_PrivateKey_file(m_ctx, keyfile.c_str(),
    SSL_FILETYPE_PEM) <= 0)
    {
        CPPUNIT_ASSERT(false);
    }
    if (!SSL_CTX_check_private_key(m_ctx))
    {
        CPPUNIT_ASSERT(false);
    }
    if (!SSL_CTX_load_verify_locations(m_ctx, caFile.c_str(), NULL))
    {
        CPPUNIT_ASSERT(false);
    }
    SSL_CTX_set_verify(m_ctx,
    SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);


    teardownCommon();
    sm = new HMStateManager;
    CPPUNIT_ASSERT(sm);
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
    remove("healthmon.mdbm");
    sm->shutdown();
    std::this_thread::sleep_for(1s);
    sm_thr.join();
    SSL_CTX_free(m_ctx);
    delete(sm);
}

void TESTNAME::test_cmdlstnr1()
{
    timeval tvc;
    tvc.tv_sec = 3;
    tvc.tv_usec = 0;
    string host = "test";
    string socketPath = "test_sock";
    HMIPAddress source;
    const HMConnectionCheck check(port, server, HM_REMOTE_SHARED_CHECK_TCPS, source, 0);
    const HMConnectionCheck checkl(socketPath, HM_REMOTE_SHARED_CHECK_LINUX);
    HMConnectionHandler handler(m_ctx, 5, tvc);

    HMIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMIPAddress addr2;
    addr2.set(ipaddr2);
    HMIPAddress addr3;
    addr3.set(ipaddr3);

    string hostName = "loadfb3.hm1.com";
    HMAPIDataHostCheck hostApiCheck;
    hostApiCheck.m_checkInfo = "hm-hello";
    hostApiCheck.m_port = 123;
    hostApiCheck.m_ipv4 = true;
    hostApiCheck.m_ipv6 = true;
    hostApiCheck.m_checkType = HM_API_CHECK_NONE;
    HMDataHostCheck dataHostCheck(hostApiCheck);
    map<HMDataCheckParams, HMDataCheckResult> hostResults;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    shared_ptr<HMSocketUtilBase> socketAPI;
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int)hostResults.size());
    CPPUNIT_ASSERT_EQUAL(1, (int)handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(1, (int)handler.getConnectionSize(check));
    CPPUNIT_ASSERT(handler.getConnection(checkl, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    CPPUNIT_ASSERT_EQUAL(2, (int )handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(1, (int )handler.getConnectionSize(check));

    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int)hostResults.size());
    CPPUNIT_ASSERT_EQUAL(3, (int)handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(2, (int)handler.getConnectionSize(check));
    CPPUNIT_ASSERT(handler.getConnection(checkl, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    CPPUNIT_ASSERT_EQUAL(4, (int )handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(2, (int )handler.getConnectionSize(check));

    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int)hostResults.size());
    CPPUNIT_ASSERT_EQUAL(5, (int)handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(3, (int)handler.getConnectionSize(check));
    CPPUNIT_ASSERT(handler.getConnection(checkl, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    CPPUNIT_ASSERT_EQUAL(6, (int )handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(3, (int )handler.getConnectionSize(check));

    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int)hostResults.size());
    CPPUNIT_ASSERT_EQUAL(7, (int)handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(4, (int)handler.getConnectionSize(check));
    CPPUNIT_ASSERT(handler.getConnection(checkl, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    CPPUNIT_ASSERT_EQUAL(8, (int )handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(4, (int )handler.getConnectionSize(check));

    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int)hostResults.size());
    CPPUNIT_ASSERT_EQUAL(9, (int)handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(5, (int)handler.getConnectionSize(check));
    CPPUNIT_ASSERT(handler.getConnection(checkl, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    CPPUNIT_ASSERT_EQUAL(10, (int )handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(5, (int )handler.getConnectionSize(check));

    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int)hostResults.size());
    CPPUNIT_ASSERT_EQUAL(10, (int)handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(5, (int)handler.getConnectionSize(check));
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    CPPUNIT_ASSERT_EQUAL(10, (int )handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(5, (int )handler.getConnectionSize(check));
    auto it = hostResults.find(checkParams);
    CPPUNIT_ASSERT(it != hostResults.end());
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(33, (int)it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(66, (int )it->second.m_totalResponseTime);
    it = hostResults.find(checkParams1);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(43, (int )it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(76, (int )it->second.m_totalResponseTime);
    CPPUNIT_ASSERT(handler.getConnection(checkl, socketAPI));
    CPPUNIT_ASSERT_EQUAL(HM_SOCK_DATA_OK, socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults));
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    CPPUNIT_ASSERT_EQUAL(10, (int )handler.getConnectionSize());
    CPPUNIT_ASSERT_EQUAL(5, (int )handler.getConnectionSize(check));
    it = hostResults.find(checkParams);
    CPPUNIT_ASSERT(it != hostResults.end());
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(33, (int)it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(66, (int )it->second.m_totalResponseTime);
    it = hostResults.find(checkParams1);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(43, (int )it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(76, (int )it->second.m_totalResponseTime);
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(
            socketAPI->getHostResults(hostName, addr3, dataHostCheck, tv,hostResults) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(2, (int )hostResults.size());
    it = hostResults.find(checkParams);
    CPPUNIT_ASSERT(it != hostResults.end());
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr3 == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(34, (int )it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(67, (int )it->second.m_totalResponseTime);
    it = hostResults.find(checkParams1);
    CPPUNIT_ASSERT(it != hostResults.end());
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr3 == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(44, (int )it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(77, (int )it->second.m_totalResponseTime);
}


void TESTNAME::test_cmdlstnr2()
{
    timeval tvc;
    tvc.tv_sec = 3;
    tvc.tv_usec = 0;
    string host = "test";
    string socketPath = "test_sock";
    HMIPAddress source;
    const HMConnectionCheck check(port, server, HM_REMOTE_SHARED_CHECK_TCPS, source, 0);
    const HMConnectionCheck checkl(socketPath, HM_REMOTE_SHARED_CHECK_LINUX);
    HMConnectionHandler handler(m_ctx, 5, tvc);

    HMIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMIPAddress addr2;
    addr2.set(ipaddr2);
    HMIPAddress addr3;
    addr3.set(ipaddr3);

    string hostName = "loadfb3.hm1.com";
    HMAPIDataHostCheck hostCheck;
    hostCheck.m_checkInfo = "hm-hello";
    hostCheck.m_port = 1234;
    hostCheck.m_ipv4 = true;
    hostCheck.m_ipv6 = true;
    hostCheck.m_checkType = HM_API_CHECK_NONE;
    HMDataHostCheck dataHostCheck(hostCheck);
    map<HMDataCheckParams, HMDataCheckResult> hostResults;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    shared_ptr<HMSocketUtilBase> socketAPI;
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(1, (int )hostResults.size());
    auto it = hostResults.find(checkParams);
    CPPUNIT_ASSERT(it != hostResults.end());
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(53, (int )it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(86, (int )it->second.m_totalResponseTime);

    CPPUNIT_ASSERT(handler.getConnection(checkl, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getHostResults(hostName, addr, dataHostCheck, tv, hostResults) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(1, (int )hostResults.size());
    it = hostResults.find(checkParams2);
    CPPUNIT_ASSERT(it != hostResults.end());
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(53, (int )it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(86, (int )it->second.m_totalResponseTime);


    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getHostResults(hostName, addr3, dataHostCheck, tv, hostResults) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(1, (int )hostResults.size());
    it = hostResults.find(checkParams2);
    CPPUNIT_ASSERT(it != hostResults.end());
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr3 == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(54, (int )it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(87, (int )it->second.m_totalResponseTime);

    CPPUNIT_ASSERT(handler.getConnection(checkl, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getHostResults(hostName, addr3, dataHostCheck, tv, hostResults) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(1, (int )hostResults.size());
    it = hostResults.find(checkParams2);
    CPPUNIT_ASSERT(it != hostResults.end());
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )it->second.m_reason);
    CPPUNIT_ASSERT(addr3 == it->second.m_address);
    CPPUNIT_ASSERT_EQUAL(54, (int )it->second.m_responseTime);
    CPPUNIT_ASSERT_EQUAL(87, (int )it->second.m_totalResponseTime);
}

void TESTNAME::test_cmdlstnr3()
{
    timeval tvc;
    tvc.tv_sec = 3;
    tvc.tv_usec = 0;
    string host = "test";
    string socketPath = "test_sock";
    HMIPAddress source;
    const HMConnectionCheck check(port, server, HM_REMOTE_SHARED_CHECK_TCPS, source, 0);
    const HMConnectionCheck checkl(socketPath, HM_REMOTE_SHARED_CHECK_LINUX);
    HMConnectionHandler handler(m_ctx, 5, tvc);
    HMIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMIPAddress addr2;
    addr2.set(ipaddr2);
    HMIPAddress addr3;
    addr3.set(ipaddr3);
    string hostName = "loadfb3.hm1.com";
    HMAPIDataHostCheck hostCheck;
    hostCheck.m_checkInfo = "hm-hello";
    hostCheck.m_port = 1234;
    hostCheck.m_ipv4 = true;
    hostCheck.m_ipv6 = true;
    hostCheck.m_checkType = HM_API_CHECK_NONE;
    HMDataHostCheck dataHostCheck(hostCheck);
    map<HMDataCheckParams, HMDataCheckResult> hostResults;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    shared_ptr<HMSocketUtilBase> socketAPI;
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getHostResults(hostName, addr2, dataHostCheck, tv, hostResults) == HM_SOCK_DATA_EMPTY);
    hostCheck.m_port = 12;
    CPPUNIT_ASSERT(socketAPI->getHostResults(hostName, addr2, dataHostCheck, tv, hostResults) == HM_SOCK_DATA_EMPTY);
}


void TESTNAME::test_cmdlstnr4()
{
    timeval tvc;
    tvc.tv_sec = 3;
    tvc.tv_usec = 0;
    string host = "test";
    string socketPath = "test_sock";
    HMIPAddress source;
    const HMConnectionCheck check(port, server, HM_REMOTE_SHARED_CHECK_TCPS, source, 0);
    HMConnectionHandler handler(m_ctx, 5, tvc);
    HMIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMIPAddress addr2;
    addr2.set(ipaddr2);
    HMIPAddress addr3;
    addr3.set(ipaddr3);
    string hostgroup = "config.parse1.healthmon.net";
    vector<HMGroupCheckResult> results;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    shared_ptr<HMSocketUtilBase> socketAPI;
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getHostGroupResults(hostgroup, tv, hash, results) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(3, (int)results.size());
    CPPUNIT_ASSERT("loadfb3.hm1.com" == results[0].m_hostName);
    CPPUNIT_ASSERT(addr == results[0].m_address);
    CPPUNIT_ASSERT(checkResultg1h11 == results[0].m_result);
    CPPUNIT_ASSERT("loadfb3.hm1.com" == results[1].m_hostName);
    CPPUNIT_ASSERT(addr3 == results[1].m_address);
    CPPUNIT_ASSERT(checkResultg1h12 == results[1].m_result);
    CPPUNIT_ASSERT("loadfb3.hm2.com" == results[2].m_hostName);
    CPPUNIT_ASSERT(addr2 == results[2].m_address);
    CPPUNIT_ASSERT(checkResulth2 == results[2].m_result);
}

void TESTNAME::test_cmdlstnr5()
{
    timeval tvc;
    tvc.tv_sec = 3;
    tvc.tv_usec = 0;
    string host = "test";
    string socketPath = "test_sock";
    HMIPAddress source;
    const HMConnectionCheck check(port, server, HM_REMOTE_SHARED_CHECK_TCPS, source, 0);
    HMConnectionHandler handler(m_ctx, 5, tvc);
    HMIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMIPAddress addr2;
    addr2.set(ipaddr2);
    HMIPAddress addr3;
    addr3.set(ipaddr3);
    string hostgroup = "config2.parse1.healthmon.net";
    vector<HMGroupCheckResult> results;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    shared_ptr<HMSocketUtilBase> socketAPI;
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getHostGroupResults(hostgroup, tv, hash2, results) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(3, (int)results.size());
    CPPUNIT_ASSERT("loadfb3.hm1.com" == results[0].m_hostName);
    CPPUNIT_ASSERT(addr == results[0].m_address);
    CPPUNIT_ASSERT(checkResultg3h11 == results[0].m_result);
    CPPUNIT_ASSERT("loadfb3.hm1.com" == results[1].m_hostName);
    CPPUNIT_ASSERT(addr3 == results[1].m_address);
    CPPUNIT_ASSERT(checkResultg3h12 == results[1].m_result);
    CPPUNIT_ASSERT("loadfb3.hm2.com" == results[2].m_hostName);
    CPPUNIT_ASSERT(addr2 == results[2].m_address);
    CPPUNIT_ASSERT(checkResulth2 == results[2].m_result);
}

void TESTNAME::test_cmdlstnr6()
{
    timeval tvc;
    tvc.tv_sec = 3;
    tvc.tv_usec = 0;
    string host = "test";
    string socketPath = "test_sock";
    HMIPAddress source;
    const HMConnectionCheck check(port, server, HM_REMOTE_SHARED_CHECK_TCPS, source, 0);
    HMConnectionHandler handler(m_ctx, 5, tvc);
    HMIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMIPAddress addr2;
    addr2.set(ipaddr2);
    HMIPAddress addr3;
    addr3.set(ipaddr3);
    string hostgroup = "config.parse1.healthmon.net";
    vector<HMGroupAuxResult> auxResults;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    shared_ptr<HMSocketUtilBase> socketAPI;
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getLoadFeedback(hostgroup, tv, hash, auxResults) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(3, (int)auxResults.size());


    CPPUNIT_ASSERT("loadfb3.hm1.com" == auxResults[0].m_hostName);
    CPPUNIT_ASSERT(addr == auxResults[0].m_address);
    CPPUNIT_ASSERT_EQUAL(2, (int)auxResults[0].m_info.m_auxData.size());
    unique_ptr<HMAuxBase> data = auxResults[0].m_info.m_auxData[0]->clone();
    CPPUNIT_ASSERT(data->m_host == "h1.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r1");
    CPPUNIT_ASSERT_EQUAL(HM_LOAD_FILE, data->m_type);
    data = auxResults[0].m_info.m_auxData[1]->clone();
    CPPUNIT_ASSERT(data->m_host == "h2.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r2");
    CPPUNIT_ASSERT_EQUAL(HM_LOAD_FILE, data->m_type);

    CPPUNIT_ASSERT("loadfb3.hm1.com" == auxResults[1].m_hostName);
    CPPUNIT_ASSERT(addr3 == auxResults[1].m_address);
    CPPUNIT_ASSERT_EQUAL(2, (int)auxResults[1].m_info.m_auxData.size());
    data = auxResults[1].m_info.m_auxData[0]->clone();
    CPPUNIT_ASSERT(data->m_host == "h4.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r4");
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FILE, data->m_type);
    HMAuxOOB* oob = data->getOOB();
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FORCEDOWN_FALSE, oob->m_forceDown);
    CPPUNIT_ASSERT_EQUAL(0, (int)oob->m_shed);
    data = auxResults[1].m_info.m_auxData[1]->clone();
    CPPUNIT_ASSERT(data->m_host == "h5.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r5");
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FILE, data->m_type);
    oob = data->getOOB();
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FORCEDOWN_TRUE, oob->m_forceDown);
    CPPUNIT_ASSERT_EQUAL(20, (int)oob->m_shed);
    CPPUNIT_ASSERT("loadfb3.hm2.com" == auxResults[2].m_hostName);
    CPPUNIT_ASSERT(addr2 == auxResults[2].m_address);
    CPPUNIT_ASSERT_EQUAL(0, (int)auxResults[2].m_info.m_auxData.size());
}

void TESTNAME::test_cmdlstnr7()
{
    timeval tvc;
    tvc.tv_sec = 3;
    tvc.tv_usec = 0;
    string host = "test";
    string socketPath = "test_sock";
    HMIPAddress source;
    const HMConnectionCheck check(port, server, HM_REMOTE_SHARED_CHECK_TCPS, source, 0);
    HMConnectionHandler handler(m_ctx, 5, tvc);
    HMIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMIPAddress addr2;
    addr2.set(ipaddr2);
    HMIPAddress addr3;
    addr3.set(ipaddr3);
    string hostgroup = "config2.parse1.healthmon.net";
    vector<HMGroupAuxResult> auxResults;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    shared_ptr<HMSocketUtilBase> socketAPI;
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getLoadFeedback(hostgroup, tv, hash2, auxResults) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(3, (int)auxResults.size());
    CPPUNIT_ASSERT("loadfb3.hm1.com" == auxResults[0].m_hostName);
    CPPUNIT_ASSERT(addr == auxResults[0].m_address);

    CPPUNIT_ASSERT_EQUAL(2, (int)auxResults[0].m_info.m_auxData.size());
    unique_ptr<HMAuxBase> data = auxResults[0].m_info.m_auxData[0]->clone();
    CPPUNIT_ASSERT(data->m_host == "h1.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r1");
    CPPUNIT_ASSERT_EQUAL(HM_LOAD_FILE, data->m_type);
    data = auxResults[0].m_info.m_auxData[1]->clone();
    CPPUNIT_ASSERT(data->m_host == "h2.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r2");
    CPPUNIT_ASSERT_EQUAL(HM_LOAD_FILE, data->m_type);
    CPPUNIT_ASSERT("loadfb3.hm1.com" == auxResults[1].m_hostName);
    CPPUNIT_ASSERT(addr3 == auxResults[1].m_address);

    CPPUNIT_ASSERT_EQUAL(2, (int)auxResults[1].m_info.m_auxData.size());
    data = auxResults[1].m_info.m_auxData[0]->clone();
    CPPUNIT_ASSERT(data->m_host == "h4.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r4");
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FILE, data->m_type);
    HMAuxOOB* oob = data->getOOB();
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FORCEDOWN_FALSE, oob->m_forceDown);
    CPPUNIT_ASSERT_EQUAL(0, (int)oob->m_shed);
    data = auxResults[1].m_info.m_auxData[1]->clone();
    CPPUNIT_ASSERT(data->m_host == "h5.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r5");
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FILE, data->m_type);
    oob = data->getOOB();
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FORCEDOWN_TRUE, oob->m_forceDown);
    CPPUNIT_ASSERT_EQUAL(20, (int)oob->m_shed);
    CPPUNIT_ASSERT("loadfb3.hm2.com" == auxResults[2].m_hostName);
    CPPUNIT_ASSERT(addr2 == auxResults[2].m_address);
    CPPUNIT_ASSERT_EQUAL(1, (int )auxResults[2].m_info.m_auxData.size());
    data = auxResults[2].m_info.m_auxData[0]->clone();
    CPPUNIT_ASSERT(data->m_host == "h3.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r3");
    CPPUNIT_ASSERT_EQUAL(HM_LOAD_FILE, data->m_type);

}

void TESTNAME::test_cmdlstnr8()
{
    timeval tvc;
    tvc.tv_sec = 3;
    tvc.tv_usec = 0;
    HMIPAddress source;
    const HMConnectionCheck check(port, server, HM_REMOTE_SHARED_CHECK_TCPS, source, 0);
    HMConnectionHandler handler(m_ctx, 5, tvc);
    HMIPAddress addr;
    string ipaddr = "192.168.1.3";
    addr.set(ipaddr);

    HMAuxInfo auxInfo;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    shared_ptr<HMSocketUtilBase> socketAPI;
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getLoadFeedback("loadfb3.hm1.com", addr, hostCheck, tv, auxInfo) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(2, (int)auxInfo.m_auxData.size());
    unique_ptr<HMAuxBase> data = auxInfo.m_auxData[0]->clone();
    CPPUNIT_ASSERT(data->m_host == "h1.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r1");
    CPPUNIT_ASSERT_EQUAL(HM_LOAD_FILE, data->m_type);
    HMAuxLoadFB* lfb = data->getLFB();
    CPPUNIT_ASSERT(lfb->m_datacenter == "d3");
    CPPUNIT_ASSERT_EQUAL(11, (int)lfb->m_load);
    CPPUNIT_ASSERT_EQUAL(2500, (int)lfb->m_target);
    CPPUNIT_ASSERT_EQUAL(5000, (int)lfb->m_max);
}

void TESTNAME::test_cmdlstnr9()
{
    timeval tvc;
    tvc.tv_sec = 3;
    tvc.tv_usec = 0;
    HMIPAddress source;
    const HMConnectionCheck check(port, server, HM_REMOTE_SHARED_CHECK_TCPS, source, 0);
    HMConnectionHandler handler(m_ctx, 5, tvc);
    HMIPAddress addr;
    string ipaddr = "192.168.1.3";
    string ipaddr2 = "2001::7334";
    string ipaddr3 = "2002::7334";
    addr.set(ipaddr);
    HMIPAddress addr2;
    addr2.set(ipaddr2);
    HMIPAddress addr3;
    addr3.set(ipaddr3);
    vector<HMGroupAuxResult> auxResults;
    timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    shared_ptr<HMSocketUtilBase> socketAPI;
    CPPUNIT_ASSERT(handler.getConnection(check, socketAPI));
    CPPUNIT_ASSERT(socketAPI->getLoadFeedback("loadfb3.hm1.com", hostCheck, tv, auxResults) == HM_SOCK_DATA_OK);
    CPPUNIT_ASSERT_EQUAL(2, (int)auxResults.size());
    CPPUNIT_ASSERT("loadfb3.hm1.com" == auxResults[0].m_hostName);
    CPPUNIT_ASSERT(addr == auxResults[0].m_address);

    CPPUNIT_ASSERT_EQUAL(2, (int)auxResults[0].m_info.m_auxData.size());
    unique_ptr<HMAuxBase> data = auxResults[0].m_info.m_auxData[0]->clone();
    CPPUNIT_ASSERT(data->m_host == "h1.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r1");
    CPPUNIT_ASSERT_EQUAL(HM_LOAD_FILE, data->m_type);
    HMAuxLoadFB* lfb = data->getLFB();
    CPPUNIT_ASSERT(lfb->m_datacenter == "d3");
    CPPUNIT_ASSERT_EQUAL(11, (int)lfb->m_load);
    CPPUNIT_ASSERT_EQUAL(2500, (int)lfb->m_target);
    CPPUNIT_ASSERT_EQUAL(5000, (int)lfb->m_max);
    data = auxResults[0].m_info.m_auxData[1]->clone();
    CPPUNIT_ASSERT(data->m_host == "h2.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r2");
    CPPUNIT_ASSERT_EQUAL(HM_LOAD_FILE, data->m_type);
    lfb = data->getLFB();
    CPPUNIT_ASSERT(lfb->m_datacenter == "D3");
    CPPUNIT_ASSERT_EQUAL(3, (int)lfb->m_load);
    CPPUNIT_ASSERT_EQUAL(38, (int)lfb->m_target);
    CPPUNIT_ASSERT_EQUAL(76, (int)lfb->m_max);

    CPPUNIT_ASSERT("loadfb3.hm1.com" == auxResults[1].m_hostName);
    CPPUNIT_ASSERT(addr3 == auxResults[1].m_address);
    CPPUNIT_ASSERT_EQUAL(2, (int)auxResults[1].m_info.m_auxData.size());
    data = auxResults[1].m_info.m_auxData[0]->clone();
    CPPUNIT_ASSERT(data->m_host == "h4.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r4");
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FILE, data->m_type);
    HMAuxOOB* oob = data->getOOB();
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FORCEDOWN_FALSE, oob->m_forceDown);
    CPPUNIT_ASSERT_EQUAL(0, (int)oob->m_shed);
    data = auxResults[1].m_info.m_auxData[1]->clone();
    CPPUNIT_ASSERT(data->m_host == "h5.hm.com");
    CPPUNIT_ASSERT(data->m_resource == "r5");
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FILE, data->m_type);
    oob = data->getOOB();
    CPPUNIT_ASSERT_EQUAL(HM_OOB_FORCEDOWN_TRUE, oob->m_forceDown);
    CPPUNIT_ASSERT_EQUAL(20, (int)oob->m_shed);
}
