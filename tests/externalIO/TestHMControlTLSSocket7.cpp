// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlTLSSocket7.h"

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

    std::ofstream fout2("conf/missing_ca_master.yaml");
    fout2 << "threads.max: 2\n\
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
control-socket-check-portv4 : 10053\n\
control-server-ipv4 : on\n\
control-server-ipv6 : off\n\
enable-secure-remote : on\n\
cert-file: " + std::string(CERT_FOLDER) + "/server.crt\n\
key-file: " + std::string(CERT_FOLDER) + "/server.key\n\
socket.path: test_sock" << endl;
    fout2.close();

    std::ofstream fout3("conf/missing_key_master.yaml");
    fout3 << "threads.max: 2\n\
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
control-socket-check-portv4 : 10053\n\
control-server-ipv4 : on\n\
control-server-ipv6 : off\n\
enable-secure-remote : on\n\
cert-file: " + std::string(CERT_FOLDER) + "/server.crt\n\
ca-file: " + std::string(CERT_FOLDER) + "/ca.crt\n\
socket.path: test_sock" << endl;
    fout3.close();

    std::ofstream fout4("conf/missing_cert_master.yaml");
    fout4 << "threads.max: 2\n\
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
control-socket-check-portv4 : 10053\n\
control-server-ipv4 : on\n\
control-server-ipv6 : off\n\
enable-secure-remote : on\n\
key-file: " + std::string(CERT_FOLDER) + "/server.key\n\
ca-file: " + std::string(CERT_FOLDER) + "/ca.crt\n\
socket.path: test_sock" << endl;
    fout4.close();

    std::ofstream fout5("conf/cert_key_mismatch_master.yaml");
    fout5 << "threads.max: 2\n\
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
control-socket-check-portv4 : 10053\n\
control-server-ipv4 : on\n\
control-server-ipv6 : off\n\
enable-secure-remote : on\n\
cert-file: " + std::string(CERT_FOLDER) + "/server.crt\n\
key-file: " + std::string(CERT_FOLDER) + "/client.key\n\
ca-file: " + std::string(CERT_FOLDER) + "/ca.crt\n\
socket.path: test_sock" << endl;
    fout5.close();

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
}

void TESTNAME::tearDown()
{
    remove("conf/missing_ca_master.yaml");
    remove("conf/missing_key_master.yaml");
    remove("conf/missing_cert_master.yaml");
    remove("conf/cert_key_mismatch_master.yaml");
    remove("conf");
    remove("netchasm.mdbm");
}


void TESTNAME::test_cmdlstnr1()
{
    string master = "conf/missing_ca_master.yaml";
    HMStateManager sm;
    CPPUNIT_ASSERT(!sm.healthCheck(master, HM_LOG_ERROR));
}

void TESTNAME::test_cmdlstnr2()
{
    string master = "conf/missing_key_master.yaml";
    HMStateManager sm;
    CPPUNIT_ASSERT(!sm.healthCheck(master, HM_LOG_ERROR));
}

void TESTNAME::test_cmdlstnr3()
{
    string master = "conf/missing_cert_master.yaml";
    HMStateManager sm;
    CPPUNIT_ASSERT(!sm.healthCheck(master, HM_LOG_ERROR));
}

void TESTNAME::test_cmdlstnr4()
{
    string master = "conf/cert_key_mismatch_master.yaml";
    HMStateManager sm;
    CPPUNIT_ASSERT(!sm.healthCheck(master, HM_LOG_ERROR));
}
