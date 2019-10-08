// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlTLSSocket.h"

#include <sys/stat.h>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include "common.h"
#include "HMConstants.h"
#include "HMStateManager.h"
#include "HMControlTLSSocketClient.h"

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    mkdir("conf", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("conf/hm", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    std::ofstream fout1("conf/dummy_master.yaml");
    std::ofstream fout2("conf/dummy_master2.yaml");
    std::ofstream fout3("conf/hm/testconf.yaml");
    std::ofstream fout4("conf/dummy_master3.yaml");

    fout1 << "threads.max: 2\n\
threads.min: 1\n\
config.load-directory: ./conf/hm\n\
config.load-file: ./conf/hm/testconf.yaml\n\
dns.type: ares\n\
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
ca-file: " + std::string(CERT_FOLDER) + "/ca.crt\n\
socket.path: test_sock" << endl;

    fout2 << "threads.max: 3\n\
threads.min: 2\n\
connectiontimeout: 10\n\
config.load-directory: ./conf/hm\n\
dns.type: ares\n\
dns.ttl: 360\n\
dns.lookup-timeout: 60\n\
http.type: curl\n\
db.type: mdbm\n\
db.path: netchasm.mdbm\n\
log.path: hm.log\n\
socket.path: test_sock1" << endl;

    fout3 << "-   name: config.parse1.netchasm.net\n\
    allow-hosts: any\n\
    ttl: 60000\n\
    error-ttl: 2\n\
    source-address: 127.0.0.5\n\
    tos-value: 1\n\
    failure-response: all\n\
    check-type: tcp\n\
    check-port: 123\n\
    check-info: hm-hello\n\
    check-retries: 2\n\
    check-retry-delay: 3\n\
    timeout: 2000\n\
    group-threshold: 12\n\
    slow-threshold: 34\n\
    smoothing-window: 5\n\
    rt-mode: total\n\
    flap-threshold: 12000\n\
    max-flaps: 3\n\
    conn-check-interval: 23123\n\
    dual-stack-mode: both\n\
    host:\n\
        - loadfb3.hm1.com\n\
        - loadfb3.hm2.com\n\
-   name: config.parse2.netchasm.net\n\
    allow-hosts: any\n\
    check-info: //hm/checkinfo\n\
    check-port: 80\n\
    check-type: http\n\
    dual-stack-mode: ipv4-only\n\
    rt-mode: connect\n\
    failure-response: none\n\
    ttl: 60000\n\
    timeout: 10000\n\
    host:\n\
        - mrf1.hm1.com\n\
        - mrf1.hm2.com\n\
\n\
-   name: config.parse3.netchasm.net\n\
    dual-stack-mode: ipv6-only\n\
    check-info: //hm/checkInfo-ssl\n\
    check-port: 443\n\
    rt-mode: smoothed-total\n\
    check-type: https-no-peer-check\n\
    ttl: 300000\n\
    host:\n\
        - e1.hm.com\n\
        - e2.hm.com\n\
\n\
-   name: config.parse4.netchasm.net\n\
    ttl: 60000\n\
    timeout: 10000\n\
    rt-mode: smoothed-connect\n\
    check-type: dnsvc\n\
    check-info: healthcheck.hm.com\n\
    host:\n\
        - g4.hm.com\n\
\n\
-   name: config.parse5.netchasm.net\n\
    check-info: netchasm:y\\treexrzindaaR3zuqhnjpcqqtwM8xq@/\n\
    check-port: 21\n\
    check-type: ftp\n\
    timeout: 10000\n\
    host:\n\
        - dh1.hm.com\n\
        - dh2.hm.com\n\
\n\
-   name: config.parse6.netchasm.net\n\
    check-type: http\n\
    check-info: /a00.netchasm.net.xml\n\
    host:\n\
        - lfb3.hm1.com\n\
        - lfb3.hm2.com \n\
\n\
-   name: config.parse7.netchasm.net\n\
    check-type: none\n\
    host:\n\
        - lfb3.hm1.com\n\
        - lfb3.hm2.com" << endl;

    fout4 << "threads.max: 3\n\
config.load-directory: ./conf/hm\n\
dns.type: ares\n\
dns.ttl: 360\n\
dns.lookup-timeout: 60\n\
http.type: curl\n\
db.type: text\n\
db.path: netchasm.text\n\
log.path: yHealth.log\n\
socket.path: test_sock1" << endl;

    fout1.close();
    fout2.close();
    fout3.close();
    fout4.close();
    string ipaddr = "127.0.0.1";
    server.set(ipaddr);
    port = HM_CONTROL_SOCKET_DEFAULT_PORTV4;
    sm = new HMStateManager;
    certfile = std::string(CERT_FOLDER) + "/client.crt";
    keyfile = std::string(CERT_FOLDER) + "/client.key";
    caFile = std::string(CERT_FOLDER) + "/ca.crt";
    string master_config = "conf/dummy_master.yaml";
    sm_thr = std::thread(&HMStateManager::healthCheck, std::ref(*sm),
            master_config, HM_LOG_ERROR);
    cout << "Sleeping" << endl;
    std::this_thread::sleep_for(5s);
    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT(state);
    state.reset();
    cout << "Done sleeping" << endl;
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
    HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    HMIPAddress ip, ip_ret;
    in_addr addr;
    inet_aton("192.168.1.3", &addr);
    ip.set((char*) &addr, AF_INET);
    string cmd = "reload";
    shared_ptr<HMState> state;
    sm->updateState(state);
    state->setDNSServer(ip);
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(1, (int )state->getMinThreads());
    CPPUNIT_ASSERT_EQUAL(3000, (int )state->getConnectionTimeout());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    CPPUNIT_ASSERT_EQUAL(true, state->getDNSAddress(ip_ret));
    CPPUNIT_ASSERT(!ip_ret.toString().compare("192.168.1.3"));
    state.reset();
    CPPUNIT_ASSERT(socketAPI.reload());
    std::this_thread::sleep_for(1s);
    sm->updateState(state);
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    CPPUNIT_ASSERT_EQUAL(true, state->getDNSAddress(ip_ret));
    CPPUNIT_ASSERT(!ip_ret.toString().compare("192.168.1.1"));

}

void TESTNAME::test_cmdlstnr2()
{
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    shared_ptr<HMState> state;
    sm->updateState(state);
    string masterConfig = "conf/dummy_master2.yaml";
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(3000, (int )state->getConnectionTimeout());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    state.reset();
    CPPUNIT_ASSERT(socketAPI.reload(masterConfig));

    std::this_thread::sleep_for(1s);
    sm->updateState(state);
    CPPUNIT_ASSERT_EQUAL(3, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMinThreads());
    CPPUNIT_ASSERT_EQUAL(10, (int )state->getConnectionTimeout());
    CPPUNIT_ASSERT_EQUAL((int )HM_CHECK_PLUGIN_HTTP_CURL,
            (int )state->getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT_EQUAL(60, (int )state->getDNSLookupTimeout());
    CPPUNIT_ASSERT_EQUAL(60, (int )state->getDNSLookupTimeout());
}

void TESTNAME::test_cmdlstnr3()
{
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
	HMIPAddress ip, ip_ret;
    in_addr addr;
    inet_aton("192.168.1.3", &addr);
    ip.set((char*) &addr, AF_INET);
    string cmd = "dummy";
    shared_ptr<HMState> state;
    sm->updateState(state);
    std::this_thread::sleep_for(1s);
    state->setDNSServer(ip);
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    CPPUNIT_ASSERT_EQUAL(true, state->getDNSAddress(ip_ret));
    CPPUNIT_ASSERT(!ip_ret.toString().compare("192.168.1.3"));
    CPPUNIT_ASSERT(!socketAPI.reload(cmd));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    CPPUNIT_ASSERT_EQUAL(true, state->getDNSAddress(ip_ret));
    CPPUNIT_ASSERT(!ip_ret.toString().compare("192.168.1.3"));
}

void TESTNAME::test_cmdlstnr4()
{
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    shared_ptr<HMState> state;
    sm->updateState(state);
    string masterConfig = "conf/dummy_master3.yaml";
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    state.reset();
    CPPUNIT_ASSERT(socketAPI.reload(masterConfig));
    std::this_thread::sleep_for(1s);
    sm->updateState(state);
    CPPUNIT_ASSERT_EQUAL(3, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_CHECK_PLUGIN_HTTP_CURL,
            (int )state->getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_TEXT, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.text"));
}

void TESTNAME::test_cmdlstnr5()
{
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    vector<string> hostGroupList;
    vector<string> hgns = { "config.parse1.netchasm.net",
            "config.parse2.netchasm.net", "config.parse3.netchasm.net",
            "config.parse4.netchasm.net", "config.parse5.netchasm.net",
            "config.parse6.netchasm.net", "config.parse7.netchasm.net" };
    CPPUNIT_ASSERT(socketAPI.getHostGroupList(hostGroupList));
    CPPUNIT_ASSERT_EQUAL(hgns.size(), hostGroupList.size());
    for( uint64_t i=0; i< hgns.size(); i++)
    {
        CPPUNIT_ASSERT(hgns[i] == hostGroupList[i]);
    }
}

void TESTNAME::test_cmdlstnr6()
{
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    string hostGroupName = "config.parse1.netchasm.net";
    vector<string> result = {"loadfb3.hm1.com","loadfb3.hm2.com"};
    vector<string> hostList;
    CPPUNIT_ASSERT(socketAPI.getHostList(hostGroupName, hostList));
    CPPUNIT_ASSERT_EQUAL(result.size(), hostList.size());
    for (uint32_t i = 0; i < result.size(); i++)
    {
        CPPUNIT_ASSERT(result[i] == hostList[i]);
    }
}


void TESTNAME::test_cmdlstnr7()
{
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    string hostGroupName = "dummy.parse1.netchasm.net";
    vector<string> hostList;
    CPPUNIT_ASSERT(socketAPI.getHostList(hostGroupName, hostList));
}


void TESTNAME::test_cmdlstnr8()
{
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    string hostGroupName = "config.parse1.netchasm.net";
    vector<string> result = {"loadfb3.hm1.com","loadfb3.hm2.com"};
    HMAPICheckInfo checkInfo;
    vector<string> hosts;
    CPPUNIT_ASSERT(socketAPI.getHostGroupParams(hostGroupName, checkInfo, hosts));
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_TCP, (int )checkInfo.m_checkType);
    CPPUNIT_ASSERT_EQUAL(123, (int )checkInfo.m_port);
    CPPUNIT_ASSERT(checkInfo.m_ipv4);
    CPPUNIT_ASSERT(checkInfo.m_ipv6);
    CPPUNIT_ASSERT_EQUAL(5, (int )checkInfo.m_smoothingWindow);
    CPPUNIT_ASSERT_EQUAL(3, (int )checkInfo.m_maxFlaps);
    CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_flapThreshold);
    CPPUNIT_ASSERT_EQUAL(2, (int )checkInfo.m_numCheckRetries);
    CPPUNIT_ASSERT_EQUAL(3, (int )checkInfo.m_checkRetryDelay);
    CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(34, (int )checkInfo.m_slowThreshold);
    CPPUNIT_ASSERT_EQUAL(2000, (int )checkInfo.m_checkTimeout);
    CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
    CPPUNIT_ASSERT("127.0.0.5" == checkInfo.m_sourceAddress.toString());
    CPPUNIT_ASSERT_EQUAL(1, (int)checkInfo.m_TOSValue);
    CPPUNIT_ASSERT("hm-hello" == checkInfo.m_checkInfo);
    CPPUNIT_ASSERT_EQUAL(result.size(), hosts.size());
    for (uint32_t i = 0; i < result.size(); i++)
    {
        CPPUNIT_ASSERT(result[i] == hosts[i]);
    }
}


void TESTNAME::test_cmdlstnr9()
{
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
	string hostGroupName = "dummy.parse1.netchasm.net";
    HMAPICheckInfo checkInfo;
    vector<string> hosts;
    CPPUNIT_ASSERT(!socketAPI.getHostGroupParams(hostGroupName, checkInfo, hosts));
}


void TESTNAME::test_cmdlstnr10()
{
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    vector<string> hostGroupList;
    vector<string> hgns = { "config.parse1.netchasm.net",
            "config.parse2.netchasm.net", "config.parse3.netchasm.net",
            "config.parse4.netchasm.net", "config.parse5.netchasm.net",
            "config.parse6.netchasm.net", "config.parse7.netchasm.net" };
    CPPUNIT_ASSERT(socketAPI.getHostGroupList(hostGroupList));
    CPPUNIT_ASSERT_EQUAL(hgns.size(), hostGroupList.size());
    for (uint32_t i = 0; i < hgns.size(); i++)
    {
        CPPUNIT_ASSERT(hgns[i] == hostGroupList[i]);
    }
    string hostGroupName = "config.parse1.netchasm.net";
    vector<string> result = { "loadfb3.hm1.com", "loadfb3.hm2.com" };
    HMAPICheckInfo checkInfo;
    vector<string> hosts;
    CPPUNIT_ASSERT(socketAPI.getHostGroupParams(hostGroupName, checkInfo, hosts));
    CPPUNIT_ASSERT_EQUAL((int )HM_CHECK_TCP, (int )checkInfo.m_checkType);
    CPPUNIT_ASSERT_EQUAL(123, (int )checkInfo.m_port);
    CPPUNIT_ASSERT(checkInfo.m_ipv4);
    CPPUNIT_ASSERT(checkInfo.m_ipv6);
    CPPUNIT_ASSERT_EQUAL(5, (int )checkInfo.m_smoothingWindow);
    CPPUNIT_ASSERT_EQUAL(3, (int )checkInfo.m_maxFlaps);
    CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_flapThreshold);
    CPPUNIT_ASSERT_EQUAL(2, (int )checkInfo.m_numCheckRetries);
    CPPUNIT_ASSERT_EQUAL(3, (int )checkInfo.m_checkRetryDelay);
    CPPUNIT_ASSERT_EQUAL(12, (int )checkInfo.m_groupThreshold);
    CPPUNIT_ASSERT_EQUAL(34, (int )checkInfo.m_slowThreshold);
    CPPUNIT_ASSERT_EQUAL(2000, (int )checkInfo.m_checkTimeout);
    CPPUNIT_ASSERT_EQUAL(60000, (int )checkInfo.m_checkTTL);
    CPPUNIT_ASSERT_EQUAL(0, (int )checkInfo.m_passthroughInfo);
    CPPUNIT_ASSERT("127.0.0.5" == checkInfo.m_sourceAddress.toString());
    CPPUNIT_ASSERT_EQUAL(1, (int)checkInfo.m_TOSValue);
    CPPUNIT_ASSERT("hm-hello" == checkInfo.m_checkInfo);
    CPPUNIT_ASSERT_EQUAL(result.size(), hosts.size());
    for (uint32_t i = 0; i < result.size(); i++)
    {
        CPPUNIT_ASSERT(result[i] == hosts[i]);
    }
}

void TESTNAME::test_cmdlstnr11()
{
    HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    bool status;
    CPPUNIT_ASSERT(socketAPI.getRemoteQueryOn(status));
    CPPUNIT_ASSERT(status);
    CPPUNIT_ASSERT(socketAPI.setRemoteQueryOff());
    this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getRemoteQueryOn(status));
    CPPUNIT_ASSERT(!status);
    CPPUNIT_ASSERT(socketAPI.setRemoteQueryOn());
    this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getRemoteQueryOn(status));
    CPPUNIT_ASSERT(status);
}

void TESTNAME::test_cmdlstnr12()
{
    HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    string hostGroupName1 = "config.parse6.netchasm.net";
    string hostGroupName2 = "config.parse7.netchasm.net";
    string hostName1 = "lfb3.hm1.com";
    string hostName2 = "lfb3.hm2.com";
    string addr1 = "127.0.0.4";
    HMAPIIPAddress address1;
    address1.set(addr1);
    string addr2 = "::2";
    HMAPIIPAddress address2;
    address2.set(addr2);

    CPPUNIT_ASSERT(socketAPI.setHostMark(hostGroupName1, hostName1, address1, 1));
    CPPUNIT_ASSERT(socketAPI.setHostMark(hostGroupName2, hostName1, address1, 2));
    CPPUNIT_ASSERT(socketAPI.setHostMark(hostGroupName1, hostName2, address1, 3));
    CPPUNIT_ASSERT(socketAPI.setHostMark(hostGroupName2, hostName2, address1, 4));
    CPPUNIT_ASSERT(socketAPI.setHostMark(hostGroupName1, hostName1, address2, 5));
    CPPUNIT_ASSERT(socketAPI.setHostMark(hostGroupName2, hostName1, address2, 6));
    CPPUNIT_ASSERT(socketAPI.setHostMark(hostGroupName1, hostName2, address2, 7));
    CPPUNIT_ASSERT(socketAPI.setHostMark(hostGroupName2, hostName2, address2, 8));

    int value;
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName1, hostName1, address1, value));
    CPPUNIT_ASSERT_EQUAL(1, value);
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName2, hostName1, address1, value));
    CPPUNIT_ASSERT_EQUAL(2, value);
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName1, hostName2, address1, value));
    CPPUNIT_ASSERT_EQUAL(3, value);
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName2, hostName2, address1, value));
    CPPUNIT_ASSERT_EQUAL(4, value);
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName1, hostName1, address2, value));
    CPPUNIT_ASSERT_EQUAL(5, value);
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName2, hostName1, address2, value));
    CPPUNIT_ASSERT_EQUAL(6, value);
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName1, hostName2, address2, value));
    CPPUNIT_ASSERT_EQUAL(7, value);
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName2, hostName2, address2, value));
    CPPUNIT_ASSERT_EQUAL(8, value);

    //delete entries
    CPPUNIT_ASSERT(socketAPI.removeHostMark(hostGroupName1, hostName1, address1));
    CPPUNIT_ASSERT(socketAPI.removeHostMark(hostGroupName1, hostName2, address1));
    CPPUNIT_ASSERT(socketAPI.removeHostMark(hostGroupName2, hostName1, address2));
    CPPUNIT_ASSERT(socketAPI.removeHostMark(hostGroupName2, hostName2, address2));

    CPPUNIT_ASSERT(!socketAPI.getHostMark(hostGroupName1, hostName1, address1, value));
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName2, hostName1, address1, value));
    CPPUNIT_ASSERT_EQUAL(2, value);
    CPPUNIT_ASSERT(!socketAPI.getHostMark(hostGroupName1, hostName2, address1, value));
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName2, hostName2, address1, value));
    CPPUNIT_ASSERT_EQUAL(4, value);

    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName1, hostName1, address2, value));
    CPPUNIT_ASSERT_EQUAL(5, value);
    CPPUNIT_ASSERT(!socketAPI.getHostMark(hostGroupName2, hostName1, address2, value));
    CPPUNIT_ASSERT(socketAPI.getHostMark(hostGroupName1, hostName2, address2, value));
    CPPUNIT_ASSERT_EQUAL(7, value);
    CPPUNIT_ASSERT(!socketAPI.getHostMark(hostGroupName2, hostName2, address2, value));
}
