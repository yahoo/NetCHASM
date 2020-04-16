#include "TestHMControlLinuxSocket6.h"

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

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    mkdir("conf", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("conf/hm", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    std::ofstream fout1("conf/dummy_master.yaml");
    std::ofstream fout3("conf/hm/testconf.yaml");

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
db.path: healthmon.mdbm\n\
log.path: hm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
enable-secure-remote : off\n\
socket.path: test_sock" << endl;

    fout3 << "-   name: config.parse1.healthmon.net\n\
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
        - loadfb3.hm1.com\n\
        - loadfb3.hm2.com\n\
-   name: config.parse2.healthmon.net\n\
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
-   name: config.parse3.healthmon.net\n\
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
-   name: config.parse4.healthmon.net\n\
    ttl: 60000\n\
    timeout: 10000\n\
    rt-mode: smoothed-connect\n\
    check-type: dnsvc\n\
    check-info: healthcheck.hm.com\n\
    host:\n\
        - g4.hm.com\n\
\n\
-   name: config.parse5.healthmon.net\n\
    check-info: healthmon:y\\treexrzindaaR3zuqhnjpcqqtwM8xq@/\n\
    check-port: 21\n\
    check-type: ftp\n\
    timeout: 10000\n\
    host:\n\
        - dh1.hm.com\n\
        - dh2.hm.com\n\
\n\
-   name: config.parse6.healthmon.net\n\
    check-type: http\n\
    check-info: /a00.healthmon.net.xml\n\
    host:\n\
        - lfb3.hm1.com\n\
        - lfb3.hm2.com \n\
\n\
-   name: config.parse7.healthmon.net\n\
    check-type: none\n\
    host:\n\
        - lfb3.hm1.com\n\
        - lfb3.hm2.com" << endl;


    fout1.close();
    fout3.close();
    sm = new HMStateManager;
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
    remove("healthmon.mdbm");
    sm->shutdown();
    std::this_thread::sleep_for(1s);
    sm_thr.join();
    delete(sm);
}

void TESTNAME::test_cmdlstnr1()
{
    string socketPath = "test_sock";
    HMControlLinuxSocketClient socketAPI(socketPath);
    HMIPAddress ip, ip_ret;
    in_addr addr;
    inet_aton("192.168.1.3", &addr);
    ip.set((char*) &addr, AF_INET);
    string cmd = "reload";
    shared_ptr<HMState> state;
    sm->updateState(state);
    shared_ptr<HMState> tState;
    sm->updateTransactionState(tState);
    state->setDNSServer(ip);
    string hostGroupName = "config.parse6.healthmon.net";
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(1, (int )state->getMinThreads());
    CPPUNIT_ASSERT_EQUAL(3000, (int )state->getConnectionTimeout());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("healthmon.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    CPPUNIT_ASSERT_EQUAL(true, state->getDNSAddress(ip_ret));
    CPPUNIT_ASSERT(
            state->m_hostGroups.find(hostGroupName)
                    != state->m_hostGroups.end());
    CPPUNIT_ASSERT(
                tState->m_hostGroups.find(hostGroupName)
                        != tState->m_hostGroups.end());
    CPPUNIT_ASSERT(socketAPI.removeHostGroup(hostGroupName));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    CPPUNIT_ASSERT(
            tState->m_hostGroups.find(hostGroupName)
                    == tState->m_hostGroups.end());

    CPPUNIT_ASSERT(
            state->m_hostGroups.find(hostGroupName)
                    != state->m_hostGroups.end());
}

void TESTNAME::test_cmdlstnr2()
{
    string socketPath = "test_sock";
    string host1 = "test_host1";
    string host2 = "test_host2";
    string checkInfo = "test_check_info";
    HMControlLinuxSocketClient socketAPI(socketPath);
    shared_ptr<HMState> state;
    sm->updateState(state);
    shared_ptr<HMState> tState;
    sm->updateTransactionState(tState);
    string hostGroupName = "config.parse6.healthmon.net";
    HMAPICheckInfo hostGroup;
    hostGroup.m_checkType = HM_API_CHECK_FTP;
    hostGroup.m_checkInfo = checkInfo;
    hostGroup.m_passthroughInfo = 0;
    hostGroup.m_hosts.push_back(host1);
    hostGroup.m_hosts.push_back(host2);
    auto itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    auto itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    const vector<string>* hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");


    CPPUNIT_ASSERT(socketAPI.addHostGroup(hostGroupName, hostGroup));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_FTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == checkInfo);
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == host1);
    CPPUNIT_ASSERT((*hostList)[1] == host2);
}


void TESTNAME::test_cmdlstnr3()
{
    string socketPath = "test_sock";
    string checkInfo = "test_check_info";
    HMControlLinuxSocketClient socketAPI(socketPath);
    shared_ptr<HMState> state;
    sm->updateState(state);
    shared_ptr<HMState> tState;
    sm->updateTransactionState(tState);
    string hostGroupName = "config.parse6.healthmon.net";
    HMAPICheckInfo hostGroup;
    hostGroup.m_checkType = HM_API_CHECK_FTP;
    hostGroup.m_checkInfo = checkInfo;
    hostGroup.m_ipv4 = true;
    hostGroup.m_passthroughInfo = 0;
    auto itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    auto itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    const vector<string>* hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");


    CPPUNIT_ASSERT(socketAPI.addHostGroup(hostGroupName, hostGroup));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_FTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == checkInfo);
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 0);

    //calculating hash of hostgroup
    HMDataHostGroup groupHashCalc(hostGroupName, hostGroup);
    HMHashMD5 hostGroupMD5;
    if (hostGroupMD5.init())
    {
        HMHash hostGroupHash;
        groupHashCalc.getHash(hostGroupMD5);
        hostGroupMD5.final(hostGroupHash);
        groupHashCalc.setHashValue(hostGroupHash);
    }
    HMDataHostGroup test = itTransactionalState->second;
    CPPUNIT_ASSERT(groupHashCalc.getHashValue() == itTransactionalState->second.getHashValue());
}

void TESTNAME::test_cmdlstnr4()
{
    string socketPath = "test_sock";
    string host1 = "test_host1";
    string host2 = "test_host2";
    string checkInfo = "test_check_info";
    HMControlLinuxSocketClient socketAPI(socketPath);
    shared_ptr<HMState> state;
    sm->updateState(state);
    shared_ptr<HMState> tState;
    sm->updateTransactionState(tState);
    string hostGroupName = "config.parse6.healthmon.net";
    HMAPICheckInfo hostGroup;
    hostGroup.m_checkType = HM_API_CHECK_FTP;
    hostGroup.m_checkInfo = checkInfo;
    hostGroup.m_passthroughInfo = 0;
    hostGroup.m_hosts.push_back(host1);
    hostGroup.m_hosts.push_back(host2);
    auto itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    auto itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    const vector<string>* hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");


    CPPUNIT_ASSERT(socketAPI.addHostGroup(hostGroupName, hostGroup));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_FTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == checkInfo);
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == host1);
    CPPUNIT_ASSERT((*hostList)[1] == host2);

    CPPUNIT_ASSERT(socketAPI.clearTransaction());
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());
    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

}



void TESTNAME::test_cmdlstnr5()
{
    string socketPath = "test_sock";
    string host1 = "test_host1";
    string host2 = "test_host2";
    string checkInfo = "test_check_info";
    HMControlLinuxSocketClient socketAPI(socketPath);
    shared_ptr<HMState> state;
    sm->updateState(state);
    shared_ptr<HMState> tState;
    sm->updateTransactionState(tState);
    string hostGroupName = "config.parse8.healthmon.net";
    HMAPICheckInfo hostGroup;
    hostGroup.m_checkType = HM_API_CHECK_FTP;
    hostGroup.m_checkInfo = checkInfo;
    hostGroup.m_passthroughInfo = 0;
    hostGroup.m_hosts.push_back(host1);
    hostGroup.m_hosts.push_back(host2);
    auto itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState == state->m_hostGroups.end());
    auto itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState == tState->m_hostGroups.end());

    CPPUNIT_ASSERT(socketAPI.addHostGroup(hostGroupName, hostGroup));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState == state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    HMHashMD5 configHash;
    HMHash hashValue;
    CPPUNIT_ASSERT(configHash.init());
    for (auto it = tState->m_hostGroups.begin(); it != tState->m_hostGroups.end(); ++it)
    {
        it->second.getHash(configHash);
    }
    configHash.final(hashValue);
    HMAPIHash hash(hashValue);
    HMAPIHash daemonHash;
    CPPUNIT_ASSERT(socketAPI.getTransactionConfigHash(daemonHash));
    CPPUNIT_ASSERT(hash == daemonHash);
    state.reset();
    tState.reset();
    CPPUNIT_ASSERT_EQUAL(HM_API_COMMIT_TRANSACTION_SUCCESS, socketAPI.commitTransaction(hash));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());
}


void TESTNAME::test_cmdlstnr6()
{
    string socketPath = "test_sock";
    string host1 = "test_host1";
    string host2 = "test_host2";
    string checkInfo = "test_check_info";
    HMControlLinuxSocketClient socketAPI(socketPath);
    shared_ptr<HMState> state;
    sm->updateState(state);
    shared_ptr<HMState> tState;
    sm->updateTransactionState(tState);
    string hostGroupName = "config.parse6.healthmon.net";
    HMAPICheckInfo hostGroup;
    hostGroup.m_checkType = HM_API_CHECK_FTP;
    hostGroup.m_checkInfo = checkInfo;
    hostGroup.m_passthroughInfo = 0;
    hostGroup.m_hosts.push_back(host1);
    hostGroup.m_hosts.push_back(host2);
    auto itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    auto itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    const vector<string>* hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");


    CPPUNIT_ASSERT(socketAPI.addHostGroup(hostGroupName, hostGroup));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_FTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == checkInfo);
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == host1);
    CPPUNIT_ASSERT((*hostList)[1] == host2);

    HMHashMD5 configHash;
    HMHash hashValue;
    CPPUNIT_ASSERT(configHash.init());
    for (auto it = tState->m_hostGroups.begin();
            it != tState->m_hostGroups.end(); ++it)
    {
        it->second.getHash(configHash);
    }
    configHash.final(hashValue);
    HMAPIHash hash(hashValue);
    HMAPIHash daemonHash;
    CPPUNIT_ASSERT(socketAPI.getTransactionConfigHash(daemonHash));
    CPPUNIT_ASSERT(hash == daemonHash);
    state.reset();
    tState.reset();
    CPPUNIT_ASSERT_EQUAL(HM_API_COMMIT_TRANSACTION_SUCCESS,
            socketAPI.commitTransaction(hash));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());
    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_FTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == checkInfo);
    hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == host1);
    CPPUNIT_ASSERT((*hostList)[1] == host2);

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_FTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == checkInfo);
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == host1);
    CPPUNIT_ASSERT((*hostList)[1] == host2);
}


void TESTNAME::test_cmdlstnr7()
{
    string socketPath = "test_sock";
    string host1 = "test_host1";
    string host2 = "test_host2";
    string checkInfo = "test_check_info";
    HMControlLinuxSocketClient socketAPI(socketPath);
    shared_ptr<HMState> state;
    sm->updateState(state);
    shared_ptr<HMState> tState;
    sm->updateTransactionState(tState);
    string hostGroupName = "config.parse6.healthmon.net";
    HMAPICheckInfo hostGroup;
    hostGroup.m_checkType = HM_API_CHECK_FTP;
    hostGroup.m_checkInfo = checkInfo;
    hostGroup.m_passthroughInfo = 0;
    hostGroup.m_hosts.push_back(host1);
    hostGroup.m_hosts.push_back(host2);
    auto itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    auto itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    const vector<string>* hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");


    CPPUNIT_ASSERT(socketAPI.addHostGroup(hostGroupName, hostGroup));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_FTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == checkInfo);
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == host1);
    CPPUNIT_ASSERT((*hostList)[1] == host2);

    HMHashMD5 configHash;
    HMHash hashValue;
    CPPUNIT_ASSERT(configHash.init());
    for (auto it = tState->m_hostGroups.begin();
            it != tState->m_hostGroups.end(); ++it)
    {
        it->second.getHash(configHash);
    }
    configHash.final(hashValue);

    HMAPIHash hash(hashValue);

    //corrupt hash
    hash.m_hashValue[0] +=1;

    HMAPIHash daemonHash;
    socketAPI.getTransactionConfigHash(daemonHash);
    CPPUNIT_ASSERT(hash != daemonHash);

    state.reset();
    tState.reset();
    CPPUNIT_ASSERT_EQUAL(HM_API_COMMIT_TRANSACTION_HASH_MISMATCH,
            socketAPI.commitTransaction(hash));
    sm->updateState(state);
    sm->updateTransactionState(tState);
    itCurrentState = state->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itCurrentState != state->m_hostGroups.end());
    itTransactionalState = tState->m_hostGroups.find(hostGroupName);
    CPPUNIT_ASSERT(itTransactionalState != tState->m_hostGroups.end());

    //current State
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getCheckType(), HM_CHECK_HTTP);
    CPPUNIT_ASSERT_EQUAL(itCurrentState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itCurrentState->second.getCheckInfo() == "/a00.healthmon.net.xml");
    hostList = itCurrentState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == "lfb3.hm1.com");
    CPPUNIT_ASSERT((*hostList)[1] == "lfb3.hm2.com");

    // transactional state
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getCheckType(), HM_CHECK_FTP);
    CPPUNIT_ASSERT_EQUAL(itTransactionalState->second.getPassthroughInfo(), (uint32_t)0);
    CPPUNIT_ASSERT(itTransactionalState->second.getCheckInfo() == checkInfo);
    hostList = itTransactionalState->second.getHostList();
    CPPUNIT_ASSERT_EQUAL((int)hostList->size(), 2);
    CPPUNIT_ASSERT((*hostList)[0] == host1);
    CPPUNIT_ASSERT((*hostList)[1] == host2);
}
