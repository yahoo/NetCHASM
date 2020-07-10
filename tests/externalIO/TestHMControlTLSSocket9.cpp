// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlTLSSocket9.h"

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
#include "HMControlTLSSocketClient.h"

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    mkdir("conf", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("conf/hm", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    std::ofstream fout1("conf/dummy_master.yaml");

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
control-server-linux: off\n\
control-socket-check-portv4 : 10053\n\
control-server-ipv4 : on\n\
control-server-ipv6 : off\n\
enable-secure-remote : on\n\
enable-mutual-auth: off\n\
cert-file: " + std::string(CERT_FOLDER) + "/server.crt\n\
key-file: " + std::string(CERT_FOLDER) + "/server.key\n\
ca-file: " + std::string(CERT_FOLDER) + "/ca.crt\n\
socket.path: test_sock" << endl;

    fout1.close();
    string ipaddr = "127.0.0.1";
    server.set(ipaddr);
    port = HM_CONTROL_SOCKET_DEFAULT_PORTV4;

    setupCommon();
    string mdbm = "netchasm.mdbm";
    remove(mdbm.c_str());
    HMDataHostGroupMap groupMap;
    HMConfigInfo configInfo;
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = HMTimeStamp::now();
    HMState checkState;
    HMDNSCache dnsCache;
    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(mdbm, &groupMap, &dnsCache);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));
    std::this_thread::sleep_for(10ms);
    store->closeStore();
    delete store;
    teardownCommon();
    sm = new HMStateManager;
    certfile = std::string(CERT_FOLDER) + "/client_self.crt";
    keyfile = std::string(CERT_FOLDER) + "/client.key";
    caFile = std::string(CERT_FOLDER) + "/ca.crt";
    string master_config = "conf/dummy_master.yaml";
    sm_thr = std::thread(&HMStateManager::healthCheck, std::ref(*sm),
            master_config, HM_LOG_ERROR);
    std::this_thread::sleep_for(4s);
}

void TESTNAME::tearDown()
{
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
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
	CPPUNIT_ASSERT(socketAPI.isConnected());
    CPPUNIT_ASSERT_EQUAL(HM_LOG_NOTICE, sm->getLogLevel());
    string newLogLevel = "debug3";
    string logLevel;
    CPPUNIT_ASSERT(socketAPI.getLogLevel(logLevel));
    CPPUNIT_ASSERT(logLevel == "notice");
    CPPUNIT_ASSERT(socketAPI.setLogLevel(newLogLevel));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getLogLevel(logLevel));
    CPPUNIT_ASSERT(logLevel == "debug3");
    newLogLevel = "error";
    // testing the timeout
    std::this_thread::sleep_for(5s);
    CPPUNIT_ASSERT(socketAPI.setLogLevel(newLogLevel));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_ERROR, sm->getLogLevel());
    CPPUNIT_ASSERT(socketAPI.getLogLevel(logLevel));
    CPPUNIT_ASSERT(logLevel == "error");
}

