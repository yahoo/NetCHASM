// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlLinuxSocket4.h"

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


    string mdbm = "testmdbm";
    string masterConfig = "master_config.yaml";
    string hmConfig = "test.yaml";
    string masterConfig1 = "master_config1.yaml";
    string hmConfig1 = "test1.yaml";

    remove(mdbm.c_str());
    remove(masterConfig.c_str());
    remove(hmConfig.c_str());

    setupCommon();
    string hostGroupName = "test.netchasm.net";
    HMAuxInfo auxInfo;
    HMAuxInfo auxInfo1;
    HMAuxInfo auxInfo2;
    HMAuxInfo auxInfo3;
    std::unique_ptr<HMAuxLoadFB> temp_aux = std::make_unique<HMAuxLoadFB>();
    temp_aux->m_type = HM_LOAD_FILE;
    temp_aux->m_datacenter = "sg3";
    temp_aux->m_host = "mra1.canary.hm.com";
    temp_aux->m_resource = "mra1-canary";
    temp_aux->m_load = 11;
    temp_aux->m_target = 2500;
    temp_aux->m_max = 5000;
    auxInfo.m_auxData.push_back(std::move(temp_aux));
    temp_aux = std::make_unique<HMAuxLoadFB>();
    temp_aux->m_type = HM_LOAD_FILE;
    temp_aux->m_datacenter = "SG3";
    temp_aux->m_host = "api.hm.com";
    temp_aux->m_resource = "api1_sats_cdn";
    temp_aux->m_load = 3;
    temp_aux->m_target = 38;
    temp_aux->m_max = 76;
    auxInfo.m_auxData.push_back(std::move(temp_aux));
    std::unique_ptr<HMAuxLoadFB> temp_aux1 = std::make_unique<HMAuxLoadFB>();
    temp_aux1->m_type = HM_LOAD_FILE;
    temp_aux1->m_datacenter = "SG3";
    temp_aux1->m_host = "api.hm.com";
    temp_aux1->m_resource = "canl";
    temp_aux1->m_load = 118;
    temp_aux1->m_target = 500;
    temp_aux1->m_max = 2000;
    auxInfo1.m_auxData.push_back(std::move(temp_aux1));
    std::unique_ptr<HMAuxOOB> temp_aux2 = std::make_unique<HMAuxOOB>();
    temp_aux2->m_type = HM_OOB_FILE;
    temp_aux2->m_host = "api.hm.com";
    temp_aux2->m_resource = "canl";
    temp_aux2->m_shed = 10;
    temp_aux2->m_forceDown = HM_OOB_FORCEDOWN_FALSE;
    auxInfo2.m_auxData.push_back(std::move(temp_aux2));
    temp_aux2 = std::make_unique<HMAuxOOB>();
    temp_aux2->m_type = HM_OOB_FILE;
    temp_aux2->m_host = "api1.hm.com";
    temp_aux2->m_resource = "canl";
    temp_aux2->m_shed = 20;
    temp_aux2->m_forceDown = HM_OOB_FORCEDOWN_TRUE;
    auxInfo2.m_auxData.push_back(std::move(temp_aux2));
    temp_aux2 = std::make_unique<HMAuxOOB>();
    temp_aux2->m_type = HM_OOB_FILE;
    temp_aux2->m_host = "api2.hm.com";
    temp_aux2->m_resource = "canl1";
    temp_aux2->m_shed = 30;
    temp_aux2->m_forceDown = HM_OOB_FORCEDOWN_TRUE;

    std::unique_ptr<HMAuxOOB> temp_aux3 = std::make_unique<HMAuxOOB>();
    temp_aux3->m_type = HM_OOB_FILE;
    temp_aux3->m_host = "api3.hm.com";
    temp_aux3->m_resource = "canl";
    temp_aux3->m_shed = 30;
    temp_aux3->m_forceDown = HM_OOB_FORCEDOWN_FALSE;
    auxInfo3.m_auxData.push_back(std::move(temp_aux3));
    temp_aux3 = std::make_unique<HMAuxOOB>();
    temp_aux3->m_type = HM_OOB_FILE;
    temp_aux3->m_host = "api4.hm.com";
    temp_aux3->m_resource = "canl";
    temp_aux3->m_shed = 40;
    temp_aux3->m_forceDown = HM_OOB_FORCEDOWN_TRUE;
    auxInfo3.m_auxData.push_back(std::move(temp_aux3));

    string sourceURL = "lfb.html";
    string xmlOut;

    HMDataHostGroupMap groupMap;
    string checkInfo = sourceURL;

    HMIPAddress address;
    address.set("1.2.3.4");

    HMIPAddress address1;
    address1.set("11.22.33.44");

    HMIPAddress addressv6;
    addressv6.set("2001::7334");

    HMIPAddress addressZero(AF_INET6);
    set<HMIPAddress> addresses;
    addresses.insert(address);
    HMDataHostGroup hostGroup(hostGroupName);
    hostGroup.setMeasurementOptions(0);
    hostGroup.setCheckType(HM_CHECK_HTTP);
    hostGroup.setDualStack(HM_DUALSTACK_BOTH);
    hostGroup.setGroupThreshold(50);
    hostGroup.setCheckTTL(30);
    hostGroup.setPort(8080);
    hostGroup.setCheckInfo(checkInfo);
    hostGroup.addHost(host1);
    hostGroup.addHost(host2);
    hostGroup.addHost(host3);
    HMHashMD5 hashMD5;
    hashMD5.init();
    hostGroup.getHash(hashMD5);
    hashMD5.final(hash);
    hash1 = hash;
    hash1.m_hashValue[2]+=10;
    groupMap.insert(make_pair(hostGroupName, hostGroup));
    HMDataHostCheck hostCheck;
    hostGroup.getHostCheck(hostCheck);

    HMDataCheckParams checkParams;
    hostGroup.getCheckParameters(checkParams);
    checkParams.addHostGroup(hostGroupName);

    now.now();

    HMDataCheckResult checkResult;
    checkResult.m_checkTime = now;
    checkResult.m_response = HM_RESPONSE_CONNECTED;
    checkResult.m_responseTime = 33;
    checkResult.m_totalResponseTime = 66;
    checkResult.m_smoothedResponseTime = 44;
    checkResult.m_address = address;

    HMDataCheckResult checkResult1;
    checkResult1.m_checkTime = now;
    checkResult1.m_response = HM_RESPONSE_FAILED;
    checkResult1.m_reason = HM_REASON_RESPONSE_TIMEOUT;
    checkResult1.m_address = addressv6;

    HMDataCheckResult checkResult2;
    checkResult2.m_checkTime = now;
    checkResult2.m_response = HM_RESPONSE_DNS_FAILED;
    checkResult2.m_reason = HM_REASON_DNS_FAILURE;
    checkResult2.m_address = addressZero;

    HMDataCheckResult checkResult3;
    checkResult3.m_checkTime = now;
    checkResult3.m_response = HM_RESPONSE_CONNECTED;
    checkResult3.m_address = address1;


    HMConfigInfo configInfo;
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;

    HMDataCheckResult testResult;
    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName, hostGroup));
    HMDNSCache dnsCache;
    HMDNSLookup dnsHostCheck(HM_DNS_TYPE_LOOKUP, false);
    HMDNSLookup dnsHostCheckv6(HM_DNS_TYPE_LOOKUP, true);
    set<HMIPAddress> addressess;
    addressess.insert(address);
    addressess.insert(address1);
    dnsCache.insertDNSEntry(host1, dnsHostCheck, 10000, 10000);
    dnsCache.updateDNSEntry(host1, dnsHostCheck, addressess);
    addressess.clear();
    addressess.insert(addressv6);
    addressess.insert(addressZero);
    dnsCache.insertDNSEntry(host2, dnsHostCheckv6, 10000, 10000);
    dnsCache.updateDNSEntry(host2, dnsHostCheckv6, addressess);
    addressess.clear();
    addressess.insert(addressZero);
    dnsCache.insertDNSEntry(host3, dnsHostCheckv6, 10000, 10000);
    dnsCache.updateDNSEntry(host3, dnsHostCheckv6, addressess);
    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(mdbm, &groupMap, &dnsCache);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigs(checkState));
    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address, hostCheck, checkParams, auxInfo2));
    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address1, hostCheck, checkParams, auxInfo3));
    CPPUNIT_ASSERT(store->storeAuxInfo(host2, addressv6, hostCheck, checkParams, auxInfo1));
    CPPUNIT_ASSERT(store->storeAuxInfo(host3, addressZero, hostCheck, checkParams, auxInfo1));
    CPPUNIT_ASSERT(store->storeCheckResult(host1, address, hostCheck, checkParams, checkResult));
    CPPUNIT_ASSERT(store->storeCheckResult(host1, address1, hostCheck, checkParams, checkResult3));
    CPPUNIT_ASSERT(store->storeCheckResult(host2, addressv6, hostCheck, checkParams, checkResult1));
    CPPUNIT_ASSERT(store->storeCheckResult(host2, addressZero, hostCheck, checkParams, checkResult2));
    CPPUNIT_ASSERT(store->storeCheckResult(host3, addressZero, hostCheck, checkParams, checkResult2));
    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));

    std::this_thread::sleep_for(10ms);

    store->closeStore();

    delete store;
    teardownCommon();

    // Create the master config for the test
    ofstream fout(masterConfig.c_str());
    ofstream hout(hmConfig.c_str());
    ofstream fout1(masterConfig1.c_str());
    ofstream hout1(hmConfig1.c_str());
    CPPUNIT_ASSERT(fout.is_open());
    CPPUNIT_ASSERT(hout.is_open());

    fout << "threads: 1" << endl
            << "config.load-file: " << hmConfig << endl
            << "db.type: mdbm" << endl
            << "db.path: " << mdbm << endl
            << "dns.statictype: none"<<endl
            << "dns.lookuptype: none"<<endl
            << "enable-secure-remote : off"<<endl
            << "socket.path: test_sock"<< endl;

    fout.close();

    fout1 << "threads: 1" << endl
            << "config.load-file: " << hmConfig1 << endl
            << "db.type: mdbm" << endl
            << "dns.type none" << endl
            << "db.path: " << mdbm << endl
            << "socket.path: test_sock"<< endl;

    fout1.close();


    hout << "-   name: " << hostGroupName << endl
            << "    ttl: 30" << endl
            << "    check-type: http" << endl
            << "    group-threshold: 50" << endl
            << "    check-port: 8080" << endl
            << "    check-info: " << checkInfo << endl
            << "    dual-stack-mode: both" << endl
            << "    host:" << endl
            << "        - " << host1 << endl
            << "        - " << host2 << endl
            << "        - " << host3 << endl;

    hout.close();

    hout1 << "-   ne: " << hostGroupName << endl
            << "    ttl: 30" << endl
            << "    check-type: http" << endl
            << "    group-threshold: 50" << endl
            << "    check-port: 8080" << endl
            << "    check-info: " << checkInfo << endl
            << "    host:" << endl
            << "        - " << host1 << endl
            << "        - " << host2 << endl
            << "        - " << host3 << endl;

    hout1.close();
    std::this_thread::sleep_for(10ms);
    sm = new HMStateManager;
    string master_config = "master_config.yaml";
    sm_thr = std::thread(&HMStateManager::healthCheck, std::ref(*sm),
            master_config, HM_LOG_ERROR);
    std::this_thread::sleep_for(5s);
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
    string path = "test_sock";
    HMControlLinuxSocketClient socketAPI(path);
    HMAPIIPAddress address;
    HMAPIIPAddress address1;
    string ipaddr = "1.2.3.4";
    string ipaddr2 = "11.22.33.44";
    string ipaddr1 = "2001::7334";
    address.set(ipaddr);
    address1.set(ipaddr2);
    HMAPIIPAddress addressv6;
    addressv6.set(ipaddr1);
    string hostGroupName = "test.netchasm.net";
    vector<HMAPIAuxInfo> auxInfos;
    CPPUNIT_ASSERT(socketAPI.getLoadFeedback(hostGroupName, auxInfos));
    CPPUNIT_ASSERT_EQUAL(3, (int)auxInfos.size());
    HMAPIAuxInfo auxInfo =  auxInfos[0];
    CPPUNIT_ASSERT(auxInfo.m_address == address);
    CPPUNIT_ASSERT(auxInfo.m_host == host1);
    CPPUNIT_ASSERT_EQUAL(30, (int)auxInfo.m_ttl);
    CPPUNIT_ASSERT_EQUAL(0, (int)auxInfo.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(2, (int)auxInfo.m_oob.size());
    CPPUNIT_ASSERT("api.hm.com" == auxInfo.m_oob[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(10, (int )auxInfo.m_oob[0].m_shed);
    CPPUNIT_ASSERT(!auxInfo.m_oob[0].m_forceDown);
    CPPUNIT_ASSERT("api1.hm.com" == auxInfo.m_oob[1].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[1].m_resource);
    CPPUNIT_ASSERT_EQUAL(20, (int )auxInfo.m_oob[1].m_shed);
    CPPUNIT_ASSERT(auxInfo.m_oob[1].m_forceDown);
    auxInfo = auxInfos[1];
    CPPUNIT_ASSERT(auxInfo.m_host == host1);
    CPPUNIT_ASSERT(auxInfo.m_address == address1);
    CPPUNIT_ASSERT_EQUAL(30, (int )auxInfo.m_ttl);
    CPPUNIT_ASSERT_EQUAL(0, (int )auxInfo.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(2, (int )auxInfo.m_oob.size());
    CPPUNIT_ASSERT("api3.hm.com" == auxInfo.m_oob[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(30, (int )auxInfo.m_oob[0].m_shed);
    CPPUNIT_ASSERT(!auxInfo.m_oob[0].m_forceDown);
    CPPUNIT_ASSERT("api4.hm.com" == auxInfo.m_oob[1].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[1].m_resource);
    CPPUNIT_ASSERT_EQUAL(40, (int )auxInfo.m_oob[1].m_shed);
    CPPUNIT_ASSERT(auxInfo.m_oob[1].m_forceDown);
    auxInfo = auxInfos[2];
    CPPUNIT_ASSERT(auxInfo.m_host == host2);
    CPPUNIT_ASSERT(auxInfo.m_address == addressv6);
    CPPUNIT_ASSERT_EQUAL(30, (int )auxInfo.m_ttl);
    CPPUNIT_ASSERT_EQUAL(1, (int )auxInfo.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(0, (int )auxInfo.m_oob.size());
    CPPUNIT_ASSERT("SG3" == auxInfo.m_lfb[0].m_datacenter);
    CPPUNIT_ASSERT("api.hm.com" == auxInfo.m_lfb[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_lfb[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(118, (int)auxInfo.m_lfb[0].m_load);
    CPPUNIT_ASSERT_EQUAL(500, (int)auxInfo.m_lfb[0].m_target);
    CPPUNIT_ASSERT_EQUAL(2000, (int)auxInfo.m_lfb[0].m_max);
}

void TESTNAME::test_cmdlstnr2()
{
    string path = "test_sock";
    HMControlLinuxSocketClient socketAPI(path);
    string hostGroupName = "dummy.netchasm.net";
    vector<HMAPIAuxInfo> auxInfos;
    CPPUNIT_ASSERT(!socketAPI.getLoadFeedback(hostGroupName, auxInfos));
}

void TESTNAME::test_cmdlstnr3()
{
    string path = "test_sock";
    HMControlLinuxSocketClient socketAPI(path);
    HMAPIIPAddress address;
    string ipaddr = "1.2.3.4";
    string ipaddr1 = "2001::7334";
    string sourceURL = "lfb.html";
    std::string host1 = "test.hm.com";
    std::string host2 = "test2.hm.com";
    address.set(ipaddr);
    HMAPIIPAddress addressv6;
    addressv6.set(ipaddr1);
    HMAPIAuxInfo auxInfo;
    CPPUNIT_ASSERT(socketAPI.getLoadFeedback(host1, sourceURL, address, auxInfo));
    CPPUNIT_ASSERT(auxInfo.m_address == address);
    CPPUNIT_ASSERT(auxInfo.m_host == host1);
    CPPUNIT_ASSERT_EQUAL(0, (int)auxInfo.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(2, (int)auxInfo.m_oob.size());
    CPPUNIT_ASSERT("api.hm.com" == auxInfo.m_oob[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(10, (int )auxInfo.m_oob[0].m_shed);
    CPPUNIT_ASSERT(!auxInfo.m_oob[0].m_forceDown);
    CPPUNIT_ASSERT("api1.hm.com" == auxInfo.m_oob[1].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[1].m_resource);
    CPPUNIT_ASSERT_EQUAL(20, (int )auxInfo.m_oob[1].m_shed);
    CPPUNIT_ASSERT(auxInfo.m_oob[1].m_forceDown);
    HMAPIAuxInfo auxInfo1;
    CPPUNIT_ASSERT(socketAPI.getLoadFeedback(host2, sourceURL, addressv6, auxInfo1));
    CPPUNIT_ASSERT(auxInfo1.m_host == host2);
    CPPUNIT_ASSERT(auxInfo1.m_address == addressv6);
    CPPUNIT_ASSERT_EQUAL(1, (int )auxInfo1.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(0, (int )auxInfo1.m_oob.size());
    CPPUNIT_ASSERT("SG3" == auxInfo1.m_lfb[0].m_datacenter);
    CPPUNIT_ASSERT("api.hm.com" == auxInfo1.m_lfb[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo1.m_lfb[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(118, (int)auxInfo1.m_lfb[0].m_load);
    CPPUNIT_ASSERT_EQUAL(500, (int)auxInfo1.m_lfb[0].m_target);
    CPPUNIT_ASSERT_EQUAL(2000, (int)auxInfo1.m_lfb[0].m_max);
}

void TESTNAME::test_cmdlstnr4()
{
    string path = "test_sock";
    HMControlLinuxSocketClient socketAPI(path);
    string ipaddr1 = "2001::7334";
    string sourceURL = "lfb.html";
    std::string host1 = "test.hm.com";
    HMAPIIPAddress addressv6;
    addressv6.set(ipaddr1);
    HMAPIAuxInfo auxInfo;
    CPPUNIT_ASSERT(!socketAPI.getLoadFeedback(host1, sourceURL, addressv6, auxInfo));
}

void TESTNAME::test_cmdlstnr5()
{
    string path = "test_sock";
    HMControlLinuxSocketClient socketAPI(path);
    HMAPIIPAddress address, address1;
    string ipaddr = "1.2.3.4";
    string ipaddr2 = "11.22.33.44";
    string ipaddr1 = "2001::7334";
    address.set(ipaddr);
    address1.set(ipaddr2);
    HMAPIIPAddress addressv6;
    addressv6.set(ipaddr1);
    string hostGroupName = "test.netchasm.net";
    vector<HMAPIAuxInfo> auxInfos;
    CPPUNIT_ASSERT(socketAPI.getLoadFeedback(hostGroupName, hash, auxInfos));
    CPPUNIT_ASSERT_EQUAL(3, (int)auxInfos.size());
    HMAPIAuxInfo auxInfo =  auxInfos[0];
    CPPUNIT_ASSERT(auxInfo.m_address == address);
    CPPUNIT_ASSERT(auxInfo.m_host == host1);
    CPPUNIT_ASSERT_EQUAL(30, (int)auxInfo.m_ttl);
    CPPUNIT_ASSERT_EQUAL(0, (int)auxInfo.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(2, (int)auxInfo.m_oob.size());
    CPPUNIT_ASSERT("api.hm.com" == auxInfo.m_oob[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(10, (int )auxInfo.m_oob[0].m_shed);
    CPPUNIT_ASSERT(!auxInfo.m_oob[0].m_forceDown);
    CPPUNIT_ASSERT("api1.hm.com" == auxInfo.m_oob[1].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[1].m_resource);
    CPPUNIT_ASSERT_EQUAL(20, (int )auxInfo.m_oob[1].m_shed);
    CPPUNIT_ASSERT(auxInfo.m_oob[1].m_forceDown);
    auxInfo = auxInfos[1];
    CPPUNIT_ASSERT(auxInfo.m_host == host1);
    CPPUNIT_ASSERT(auxInfo.m_address == address1);
    CPPUNIT_ASSERT_EQUAL(30, (int )auxInfo.m_ttl);
    CPPUNIT_ASSERT_EQUAL(0, (int )auxInfo.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(2, (int )auxInfo.m_oob.size());
    CPPUNIT_ASSERT("api3.hm.com" == auxInfo.m_oob[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(30, (int )auxInfo.m_oob[0].m_shed);
    CPPUNIT_ASSERT(!auxInfo.m_oob[0].m_forceDown);
    CPPUNIT_ASSERT("api4.hm.com" == auxInfo.m_oob[1].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[1].m_resource);
    CPPUNIT_ASSERT_EQUAL(40, (int )auxInfo.m_oob[1].m_shed);
    CPPUNIT_ASSERT(auxInfo.m_oob[1].m_forceDown);
    auxInfo = auxInfos[2];
    CPPUNIT_ASSERT(auxInfo.m_host == host2);
    CPPUNIT_ASSERT(auxInfo.m_address == addressv6);
    CPPUNIT_ASSERT_EQUAL(30, (int )auxInfo.m_ttl);
    CPPUNIT_ASSERT_EQUAL(1, (int )auxInfo.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(0, (int )auxInfo.m_oob.size());
    CPPUNIT_ASSERT("SG3" == auxInfo.m_lfb[0].m_datacenter);
    CPPUNIT_ASSERT("api.hm.com" == auxInfo.m_lfb[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_lfb[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(118, (int)auxInfo.m_lfb[0].m_load);
    CPPUNIT_ASSERT_EQUAL(500, (int)auxInfo.m_lfb[0].m_target);
    CPPUNIT_ASSERT_EQUAL(2000, (int)auxInfo.m_lfb[0].m_max);

    CPPUNIT_ASSERT(!socketAPI.getLoadFeedback(hostGroupName, hash1, auxInfos));
}


void TESTNAME::test_cmdlstnr6()
{
    string path = "test_sock";
    HMControlLinuxSocketClient socketAPI(path);
    HMAPIIPAddress address, address1;
    string ipaddr = "1.2.3.4";
    string ipaddr2 = "11.22.33.44";
    address.set(ipaddr);
    address1.set(ipaddr2);
    vector<HMAPIAuxInfo> auxInfos;
    HMAPIDataHostCheck apiDataHostCheck;
    string sourceURL = "lfb.html";
    apiDataHostCheck.m_checkType = HM_API_CHECK_HTTP;
    apiDataHostCheck.m_port = 8080;
    apiDataHostCheck.m_dnsCheckType = HM_API_DNS_LOOKUP;
    apiDataHostCheck.m_checkInfo = sourceURL;
    apiDataHostCheck.m_ipv4 = true;
    apiDataHostCheck.m_ipv6 = true;
    CPPUNIT_ASSERT(socketAPI.getLoadFeedback(host1, apiDataHostCheck, auxInfos));
    CPPUNIT_ASSERT_EQUAL(2, (int)auxInfos.size());
    HMAPIAuxInfo auxInfo =  auxInfos[0];
    CPPUNIT_ASSERT(auxInfo.m_address == address);
    CPPUNIT_ASSERT(auxInfo.m_host == host1);
    CPPUNIT_ASSERT_EQUAL(0, (int)auxInfo.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(2, (int)auxInfo.m_oob.size());
    CPPUNIT_ASSERT("api.hm.com" == auxInfo.m_oob[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(10, (int )auxInfo.m_oob[0].m_shed);
    CPPUNIT_ASSERT(!auxInfo.m_oob[0].m_forceDown);
    CPPUNIT_ASSERT("api1.hm.com" == auxInfo.m_oob[1].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[1].m_resource);
    CPPUNIT_ASSERT_EQUAL(20, (int )auxInfo.m_oob[1].m_shed);
    CPPUNIT_ASSERT(auxInfo.m_oob[1].m_forceDown);
    auxInfo = auxInfos[1];
    CPPUNIT_ASSERT(auxInfo.m_host == host1);
    CPPUNIT_ASSERT(auxInfo.m_address == address1);
    CPPUNIT_ASSERT_EQUAL(0, (int )auxInfo.m_lfb.size());
    CPPUNIT_ASSERT_EQUAL(2, (int )auxInfo.m_oob.size());
    CPPUNIT_ASSERT("api3.hm.com" == auxInfo.m_oob[0].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[0].m_resource);
    CPPUNIT_ASSERT_EQUAL(30, (int )auxInfo.m_oob[0].m_shed);
    CPPUNIT_ASSERT(!auxInfo.m_oob[0].m_forceDown);
    CPPUNIT_ASSERT("api4.hm.com" == auxInfo.m_oob[1].m_host);
    CPPUNIT_ASSERT("canl" == auxInfo.m_oob[1].m_resource);
    CPPUNIT_ASSERT_EQUAL(40, (int )auxInfo.m_oob[1].m_shed);
    CPPUNIT_ASSERT(auxInfo.m_oob[1].m_forceDown);
}
