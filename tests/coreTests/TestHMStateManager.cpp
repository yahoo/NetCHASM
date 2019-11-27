// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "common.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fstream>

#include "TestHMStateManager.h"
#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMConstants.h"

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

    std::ofstream fout5("conf/dummy_master4.yaml");
    std::ofstream fout6("conf/dummy_master5.yaml");
    std::ofstream fout7("conf/dummy_master6.yaml");
    std::ofstream fout8("conf/dummy_master7.yaml");

    //verify HM_CONFIG_AUTO
    std::ofstream fout9("conf/dummy_master.yaml");
    std::ofstream fout10("conf/dummy_master8.yaml");

    //verify loading configs from backend
    std::ofstream fout11("conf/dummy_masterbackend.yaml");
    std::ofstream fout12("conf/dummy_masterbackendempty.yaml");
    
    //child host group
    std::ofstream fout13("conf/dummy_master9.yaml");
    std::ofstream fout14("conf/hm/testconf12.yaml");


    fout1 << "threads.max: 5\n\
threads.min: 2\n\
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
log.path: netchasm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
socket.path: test_sock" << endl;

    fout2 << "threads.max: 4\n\
threads.min: 3\n\
config.load-file: ./conf/hm/testconf.yaml\n\
dns.type: ares\n\
dns.ttl: 360\n\
dns.lookup-timeout: 60\n\
http.type: curl\n\
db.type: text\n\
db.path: netchasm.text\n\
log.path: netchasm.log\n" << endl;

    fout3 << "-   name: config.parse1.netchasm.net\n\
    allow-hosts: any\n\
    ttl: 60000\n\
    error-ttl: 2\n\
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
       - lfb3.hm1.com\n\
       - lfb3.hm2.com\n\
\n\
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
    check-type: http-auxfetch\n\
    check-info: /a00.netchasm.net.xml\n\
    host:\n\
       - lfb-l3.hm1.com\n\
       - lfb-l3.hm2.com\n\
\n\
-   name: config.parse8.netchasm.net\n\
    check-info: config.parse6.netchasm.net\n\
    check-type: indirect-host\n\
    host:\n\
       - lfb-l3.hm1.com\n\
       - lfb-l3.hm2.com\n\
\n\
-   name: config.parse9.netchasm.net\n\
    check-type: indirect-host\n\
    check-info: config.parse5.netchasm.net\n\
    host:\n\
       - lfb-l3.hm1.com\n\
       - lfb-l3.hm2.com\n\
-   name: config.parse10.netchasm.net\n\
    check-info: config.parse6.netchasm.net\n\
    check-type: http\n\
    host-group:\n\
       - config.parse3.netchasm.net\n\
       - config.parse4.netchasm.net" << endl;


    fout4 << "threads.max: 5\n\
config.load-file: ./conf/hm/testconf.yaml\n\
log.type: text\n\
dns.type: ares\n\
dns.host: invalid.ip\n\
dns.lookup-timeout: 60\n\
http.type: curl\n\
db.type: text\n\
db.path: netchasm.text\n\
log.path: netchasm.log\n" << endl;


    fout5 << "threads.max: 5\n\
threads.min: 2\n\
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
log.path: netchasm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
socket.path: test_sock" << endl;

    fout6 << "threads.max: 4\n\
threads.min: 3\n\
config.load-file: ./conf/hm/testconf.yaml\n\
dns.type: ares\n\
dns.ttl: 360\n\
dns.lookup-timeout: 60\n\
http.type: curl\n\
db.type: text\n\
db.path: netchasm.text\n\
log.path: netchasm.log\n" << endl;

    fout7 << "threads.max: 5\n\
config.load-file: ./conf/hm/testconf.yaml\n\
log.type: text\n\
dns.type: ares\n\
dns.host: invalid.ip\n\
dns.lookup-timeout: 60\n\
http.type: curl\n\
db.type: text\n\
db.path: netchasm.text\n\
log.path: netchasm.log\n" << endl;

    fout8 << "threads.max: 5\n\
config.load-directory ./conf/hmd\n\
log.path: netchasm.log\n" << endl;

    fout9 << "threads.max: 5\n\
threads.min: 2\n\
config.load-file: ./conf/hm/testconf.yaml\n\
\n\
dns.type: ares\n\
dns.host: 192.168.1.1\n\
dns.ttl: 360\n\
dns.lookup-timeout: 60\n\
http.type: curl\n\
ftp.type: curl\n\
tcp.type: rawsocket\n\
dnsvc.type: ares\n\
none.type: none\n\
\n\
db.type: mdbm\n\
db.path: netchasm.mdbm\n\
log.path: netchasm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
socket.path: test_sock" << endl;

    fout10 << "threads.max: 5\n\
threads.min: 2\n\
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
log.path: netchasm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
socket.path: test_sock" << endl;

    fout11 << "threads.max: 5\n\
threads.min: 2\n\
config.use-backend: on\n\
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
log.path: netchasm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
socket.path: test_sock" << endl;

    fout12 << "threads.max: 5\n\
threads.min: 2\n\
config.use-backend: on\n\
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
db.path: netchasm_empty.mdbm\n\
log.path: netchasm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
socket.path: test_sock" << endl;

    fout13 << "threads.max: 5\n\
threads.min: 2\n\
config.load-file: ./conf/hm/testconf12.yaml\n\
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
log.path: netchasm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
socket.path: test_sock" << endl;

    fout14 << "-   name: iconfig.parse3.netchasm.net\n\
    check-info: iconfig.parse6.netchasm.net\n\
    check-type: indirect-host\n\
    host:\n\
       - ilfb-l3.hm1.com\n\
       - lfb-l3.hm2.com\n\
\n\
-   name: iconfig.parse10.netchasm.net\n\
    check-info: iconfig.parse6.netchasm.net\n\
    check-type: http\n\
    host-group:\n\
       - iconfig.parse3.netchasm.net\n\
       - iconfig.parse4.netchasm.net" << endl;

    fout1.close();
    fout2.close();
    fout3.close();
    fout4.close();
    fout5.close();
    fout6.close();
    fout7.close();
    fout8.close();
    fout9.close();
    fout10.close();
    fout11.close();
    fout12.close();
    fout13.close();
    fout14.close();
}

void TESTNAME::tearDown()
{
    //remove("conf/hm/testconf.yaml");
    //remove("conf/dummy_master.yaml");
    remove("conf/dummy_master2.yaml");
    remove("conf/dummy_master3.yaml");
    remove("conf/dummy_master4.yaml");
    remove("conf/dummy_master5.yaml");
    remove("conf/dummy_master6.yaml");
    remove("conf/dummy_master7.yaml");
    remove("conf/dummy_master.yaml");
    remove("conf/dummy_master8.yaml");
    remove("conf/dummy_masterbackend.yaml");
    remove("conf/dummy_masterbackendempty.yaml");
    //remove("conf/hm/testconf2.yaml");
    //remove("conf/dummy_master9.yaml");    
    remove("conf/hm");
    remove("conf");
    remove("netchasm.mdbm");
    remove("netchasm.text");
}

void TESTNAME::test_statemanager()
{
    HMState sm;
    CPPUNIT_ASSERT_EQUAL((int)HM_STORAGE_MDBM ,(int)sm.getStorageType());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_HTTP_CURL ,(int)sm.getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_FTP_CURL ,(int)sm.getDefaultFTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_TCP_RAW ,(int)sm.getDefaultTCPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_DNS_ARES ,(int)sm.getDefaultDNSCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_DEFAULT ,(int)sm.getDefaultNoneCheckype());
    CPPUNIT_ASSERT_EQUAL(HM_DEFAULT_DNS_RETRIES ,(int)sm.getDNSRetries());
    CPPUNIT_ASSERT_EQUAL(1 ,(int)sm.getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(HM_DEFAULT_DNS_RESOLUTION_TIMEOUT ,(int)sm.getDNSLookupTimeout());

}

void TESTNAME::test_master_conf_parse()
{
    HMState sm;
    string non_exist_config = "dummy.config";
    HMIPAddress ip;
    CPPUNIT_ASSERT_EQUAL(false, sm.parseMasterConfig(non_exist_config));
    string invaliddnsconfig = "conf/dummy_master3.yaml";
    CPPUNIT_ASSERT_EQUAL(false, sm.parseMasterConfig(invaliddnsconfig));
    string master_config = "conf/dummy_master.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.parseMasterConfig(master_config));
    CPPUNIT_ASSERT_EQUAL(5, (int)sm.getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(2, (int)sm.getMinThreads());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_HTTP_CURL, (int)sm.getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_FTP_CURL, (int)sm.getDefaultFTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_TCP_RAW, (int)sm.getDefaultTCPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_DNS_ARES, (int)sm.getDefaultDNSCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_DEFAULT, (int)sm.getDefaultNoneCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_STORAGE_MDBM, (int)sm.getStorageType());
    CPPUNIT_ASSERT(!sm.getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!sm.getSocketPath().compare("test_sock"));
    CPPUNIT_ASSERT_EQUAL(60, (int)sm.getDNSLookupTimeout());
    CPPUNIT_ASSERT_EQUAL(60, (int)sm.getDNSLookupTimeout());
    CPPUNIT_ASSERT_EQUAL(true,sm.getDNSAddress(ip));
    CPPUNIT_ASSERT(!ip.toString().compare("192.168.1.1"));

}


void TESTNAME::test_master_conf_parse1()
{
    HMState sm;
    HMIPAddress ip,ip_ret;
    in_addr addr;
    inet_aton("192.168.1.1",&addr);
    ip.set((char*)&addr,AF_INET);
    string master_config = "conf/dummy_master2.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.parseMasterConfig(master_config));
    CPPUNIT_ASSERT_EQUAL(4, (int)sm.getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(3, (int)sm.getMinThreads());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_HTTP_CURL, (int)sm.getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_STORAGE_TEXT, (int)sm.getStorageType());
    CPPUNIT_ASSERT(!sm.getStoragePath().compare("netchasm.text"));
    CPPUNIT_ASSERT_EQUAL(60, (int)sm.getDNSLookupTimeout());
    CPPUNIT_ASSERT_EQUAL(60, (int)sm.getDNSLookupTimeout());
    CPPUNIT_ASSERT_EQUAL(false,sm.getDNSAddress(ip));
    sm.setDNSServer(ip);
    CPPUNIT_ASSERT_EQUAL(true,sm.getDNSAddress(ip_ret));
    CPPUNIT_ASSERT(!ip_ret.toString().compare("192.168.1.1"));

}

void TESTNAME::test_set_state()
{
    HMStateManager sm;
    shared_ptr<HMState> state1_ptr = make_shared<HMState>();
    shared_ptr<HMState> state2_ptr = make_shared<HMState>();
    sm.setState(state1_ptr);
    CPPUNIT_ASSERT_EQUAL(false,sm.updateState(state1_ptr));
    CPPUNIT_ASSERT_EQUAL(true,sm.updateState(state2_ptr));
    CPPUNIT_ASSERT(state1_ptr == state2_ptr);
}


void TESTNAME::test_config_parse()
{
    HMStateManager sm;
    string no_master_config = "dummy_master.yaml";
    CPPUNIT_ASSERT_EQUAL(false, sm.loadDaemonState(no_master_config, HM_LOG_ERROR));
    string invalidhmconfig = "conf/dummy_master3.yaml";
    CPPUNIT_ASSERT_EQUAL(false, sm.loadDaemonState(invalidhmconfig, HM_LOG_ERROR));
    string master_config = "conf/dummy_master.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.loadDaemonState(master_config, HM_LOG_ERROR));
    shared_ptr<HMState> state1_ptr = make_shared<HMState>();
}

void TESTNAME::test_config_parse1()
{
    HMStateManager sm;
    string no_master_config = "dummy_master.yaml";
    CPPUNIT_ASSERT_EQUAL(false, sm.loadDaemonState(no_master_config, HM_LOG_ERROR));
    string master_config = "conf/dummy_master2.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.loadDaemonState(master_config, HM_LOG_ERROR));
    shared_ptr<HMState> state1_ptr = make_shared<HMState>();
    shared_ptr<HMState> state2_ptr = make_shared<HMState>();
    CPPUNIT_ASSERT_EQUAL(true, sm.updateState(state1_ptr));
}

void TESTNAME::test_indirect_host()
{
    HMStateManager sm;
    string master_config = "conf/dummy_master.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.loadDaemonState(master_config, HM_LOG_ERROR));
    shared_ptr<HMState> current;
    sm.updateState(current);
    auto it = current->m_hostGroups.find("config.parse8.netchasm.net");
    CPPUNIT_ASSERT(it != current->m_hostGroups.end());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_AUX_HTTP, it->second.getCheckType());
}

void TESTNAME::test_indirect_host1()
{
    HMStateManager sm;
    string master_config = "conf/dummy_master.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.loadDaemonState(master_config, HM_LOG_ERROR));
    shared_ptr<HMState> current;
    sm.updateState(current);
    auto it = current->m_hostGroups.find("config.parse9.netchasm.net");
    CPPUNIT_ASSERT(it != current->m_hostGroups.end());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_FTP, it->second.getCheckType());
    CPPUNIT_ASSERT_EQUAL(21, (int)it->second.getCheckPort());
}

void TESTNAME::test_master_yaml_conf_parse()
{   
    HMState sm;
    string non_exist_config = "dummy.yaml";
    HMIPAddress ip;
    CPPUNIT_ASSERT_EQUAL(false, sm.parseMasterConfig(non_exist_config));

    string not_yamlconfig = "conf/dummy_master7.yaml";
    CPPUNIT_ASSERT_EQUAL(false, sm.parseMasterConfig(not_yamlconfig));

    string invaliddnsconfig = "conf/dummy_master6.yaml";
    CPPUNIT_ASSERT_EQUAL(false, sm.parseMasterConfig(invaliddnsconfig));
    string master_config = "conf/dummy_master4.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.parseMasterConfig(master_config));
    CPPUNIT_ASSERT_EQUAL(5, (int)sm.getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(2, (int)sm.getMinThreads());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_HTTP_CURL, (int)sm.getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_FTP_CURL, (int)sm.getDefaultFTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_TCP_RAW, (int)sm.getDefaultTCPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_DNS_ARES, (int)sm.getDefaultDNSCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_DEFAULT, (int)sm.getDefaultNoneCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_STORAGE_MDBM, (int)sm.getStorageType());
    CPPUNIT_ASSERT(!sm.getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!sm.getSocketPath().compare("test_sock"));
    CPPUNIT_ASSERT_EQUAL(60, (int)sm.getDNSLookupTimeout());
    CPPUNIT_ASSERT_EQUAL(60, (int)sm.getDNSLookupTimeout());
    CPPUNIT_ASSERT_EQUAL(true,sm.getDNSAddress(ip));
    CPPUNIT_ASSERT(!ip.toString().compare("192.168.1.1"));

}

void TESTNAME::test_master_yaml_conf_parse1()
{
    HMState sm;
    HMIPAddress ip,ip_ret;
    in_addr addr;
    inet_aton("192.168.1.1",&addr);
    ip.set((char*)&addr,AF_INET);
    string master_config = "conf/dummy_master5.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.parseMasterConfig(master_config));
    CPPUNIT_ASSERT_EQUAL(4, (int)sm.getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(3, (int)sm.getMinThreads());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_HTTP_CURL, (int)sm.getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_STORAGE_TEXT, (int)sm.getStorageType());
    CPPUNIT_ASSERT(!sm.getStoragePath().compare("netchasm.text"));
    CPPUNIT_ASSERT_EQUAL(60, (int)sm.getDNSLookupTimeout());
    CPPUNIT_ASSERT_EQUAL(60, (int)sm.getDNSLookupTimeout());
    CPPUNIT_ASSERT_EQUAL(false,sm.getDNSAddress(ip));
    sm.setDNSServer(ip);
    CPPUNIT_ASSERT_EQUAL(true,sm.getDNSAddress(ip_ret));
    CPPUNIT_ASSERT(!ip_ret.toString().compare("192.168.1.1"));

}

void TESTNAME::test_indirect_host2()
{
    HMStateManager sm;
    string master_config = "conf/dummy_master.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.loadDaemonState(master_config, HM_LOG_ERROR));
    shared_ptr<HMState> current;
    sm.updateState(current);
    auto it = current->m_hostGroups.find("config.parse8.netchasm.net");
    CPPUNIT_ASSERT(it != current->m_hostGroups.end());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_AUX_HTTP, it->second.getCheckType());
}

void TESTNAME::test_indirect_host3()
{
    HMStateManager sm;
    string master_config = "conf/dummy_master8.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.loadDaemonState(master_config, HM_LOG_ERROR));
    shared_ptr<HMState> current;
    sm.updateState(current);
    auto it = current->m_hostGroups.find("config.parse9.netchasm.net");
    CPPUNIT_ASSERT(it != current->m_hostGroups.end());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_FTP, it->second.getCheckType());
    CPPUNIT_ASSERT_EQUAL(21, (int)it->second.getCheckPort());
}


void TESTNAME::test_load_configs_backend()
{
    // Test basic loading of configs
    HMStateManager *sm = new HMStateManager();
    string master_config = "conf/dummy_master8.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm->loadDaemonState(master_config, HM_LOG_ERROR));
    shared_ptr<HMState> current;
    sm->updateState(current);
    auto it = current->m_hostGroups.find("config.parse9.netchasm.net");
    CPPUNIT_ASSERT(it != current->m_hostGroups.end());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_FTP, it->second.getCheckType());
    CPPUNIT_ASSERT_EQUAL(21, (int)it->second.getCheckPort());
    current.reset();
    delete sm;

    sm = new HMStateManager();
    string master_backend = "conf/dummy_masterbackend.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm->loadDaemonState(master_backend, HM_LOG_ERROR));
    sm->updateState(current);
    it = current->m_hostGroups.find("config.parse9.netchasm.net");
    CPPUNIT_ASSERT(it != current->m_hostGroups.end());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_FTP, it->second.getCheckType());
    CPPUNIT_ASSERT_EQUAL(21, (int)it->second.getCheckPort());
    const vector<string> *hosts;
    hosts = it->second.getHostList();
    CPPUNIT_ASSERT_EQUAL(2, (int)hosts->size());
    CPPUNIT_ASSERT(find(hosts->begin(), hosts->end(), "lfb-l3.hm1.com") != hosts->end());
    CPPUNIT_ASSERT(find(hosts->begin(), hosts->end(), "lfb-l3.hm2.com") != hosts->end());
    CPPUNIT_ASSERT(current->m_hostGroups.find("config.parse1.netchasm.net") != current->m_hostGroups.end());
    CPPUNIT_ASSERT(current->m_hostGroups.find("config.parse2.netchasm.net") != current->m_hostGroups.end());
    CPPUNIT_ASSERT(current->m_hostGroups.find("config.parse3.netchasm.net") != current->m_hostGroups.end());
    CPPUNIT_ASSERT(current->m_hostGroups.find("config.parse4.netchasm.net") != current->m_hostGroups.end());
    CPPUNIT_ASSERT(current->m_hostGroups.find("config.parse5.netchasm.net") != current->m_hostGroups.end());
    CPPUNIT_ASSERT(current->m_hostGroups.find("config.parse6.netchasm.net") != current->m_hostGroups.end());
    CPPUNIT_ASSERT(current->m_hostGroups.find("config.parse8.netchasm.net") != current->m_hostGroups.end());
    current.reset();
    delete sm;
    // should return true even on cold start when the mdbm is empty.
    sm = new HMStateManager();
    string master_backendempty = "conf/dummy_masterbackendempty.yaml";
    CPPUNIT_ASSERT_EQUAL(true,
            sm->loadDaemonState(master_backendempty, HM_LOG_ERROR));
    delete sm;
}

void TESTNAME::test_childhostgroup_1()
{
    string hostGroup0 = "config.parse3.netchasm.net";
    string hostGroup1 = "config.parse4.netchasm.net";
    string host0 = "e1.hm.com";
    string host1 = "e2.hm.com";
    string host2 = "g4.hm.com";

    HMStateManager sm;
    string master_config = "conf/dummy_master4.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm.loadDaemonState(master_config, HM_LOG_ERROR));
    shared_ptr<HMState> current;
    sm.updateState(current);
    auto it = current->m_hostGroups.find("config.parse10.netchasm.net");
    CPPUNIT_ASSERT(it != current->m_hostGroups.end());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_HTTP, it->second.getCheckType());
    const std::vector<std::string>* hostGroupslist = it->second.getHostGroupList();
    CPPUNIT_ASSERT_EQUAL(hostGroupslist->at(0), hostGroup0);
    CPPUNIT_ASSERT_EQUAL(hostGroupslist->at(1), hostGroup1);
    const std::vector<std::string>* hostslist = it->second.getHostList();
    CPPUNIT_ASSERT_EQUAL(hostslist->at(0), host0);
    CPPUNIT_ASSERT_EQUAL(hostslist->at(1), host1);
    CPPUNIT_ASSERT_EQUAL(hostslist->at(2), host2);
}

void TESTNAME::test_childhostgroup_2()
{
    string hostGroup0 = "config.parse3.netchasm.net";
    string hostGroup1 = "config.parse4.netchasm.net";
    string host0 = "e1.hm.com";
    string host1 = "e2.hm.com";
    string host2 = "g4.hm.com";

    HMStateManager sm;
    string master_config = "conf/dummy_master9.yaml";
    CPPUNIT_ASSERT_EQUAL(false, sm.loadDaemonState(master_config, HM_LOG_ERROR));

}
