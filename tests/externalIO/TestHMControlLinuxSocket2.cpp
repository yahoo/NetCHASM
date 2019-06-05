// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlLinuxSocket2.h"

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
socket.path: test_sock" << endl;

    fout3 << "-   name: config.parse1.netchasm.net\n\
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

    setupCommon();
    string mdbm = "netchasm.mdbm";
    remove(mdbm.c_str());
    string hostGroupName = "config.parse1.netchasm.net";
    string host1 = "lfb3.hm1.com";
    string host2 = "lfb3.hm2.com";
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
    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(mdbm, &groupMap);
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
    std::this_thread::sleep_for(1s);
}

void TESTNAME::tearDown()
{
    if(sock_fd != 0)
    {
        close(sock_fd);
    }
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
    char buffer[65535];
    in_addr addr;
    inet_aton("192.168.1.3", &addr);
    in_addr addr2;
    inet_pton(AF_INET,"192.168.5.1",&addr2);
    in_addr addr3;
    inet_pton(AF_INET, "192.168.1.5", &addr3);

    struct sockaddr_un server;
    string cmd = "hostgroup config.parse1.netchasm.net";

    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");

    std::this_thread::sleep_for(1s);
    read(sock_fd, buffer, sizeof(buffer));
    close(sock_fd);
    sock_fd = 0;
    hm_nameinfo_sock_t* ni = (hm_nameinfo_sock_t*) buffer;
    CPPUNIT_ASSERT_EQUAL(true, ni->ni_check_status);
    CPPUNIT_ASSERT_EQUAL(12, (int )ni->ni_group_threshold);
    CPPUNIT_ASSERT_EQUAL(0, (int )ni->ni_mode);
    CPPUNIT_ASSERT_EQUAL(3, (int )ni->ni_numhost);
    CPPUNIT_ASSERT_EQUAL(60000, (int )ni->ni_ttl);
    hm_hostinfo2_t* infop = (hm_hostinfo2_t*) (buffer + ni->ni_size);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )infop->hi_reason);
    CPPUNIT_ASSERT_EQUAL((unsigned int)AF_INET,
            (unsigned int )infop->hi_addrtype);
    CPPUNIT_ASSERT(!(strcmp(infop->hi_hostname, "lfb3.hm1.com")));
    CPPUNIT_ASSERT_EQUAL(addr.s_addr, infop->hi_addr.s_addr);
    hm_hostinfo2_t* infop1 = (hm_hostinfo2_t*) (buffer + ni->ni_size
            + infop->hi_size);
    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )infop1->hi_reason);
    CPPUNIT_ASSERT_EQUAL((unsigned int)AF_INET,
            (unsigned int )infop1->hi_addrtype);
    CPPUNIT_ASSERT(
            !(strcmp(infop1->hi_hostname, "lfb3.hm1.com")));
    CPPUNIT_ASSERT_EQUAL(addr3.s_addr, infop1->hi_addr.s_addr);
    hm_hostinfo2_t* infop2 = (hm_hostinfo2_t*) (buffer + ni->ni_size
            + infop->hi_size + infop1->hi_size);

    CPPUNIT_ASSERT_EQUAL((unsigned int )HM_REASON_SUCCESS,
            (unsigned int )infop2->hi_reason);
    CPPUNIT_ASSERT_EQUAL((unsigned int)AF_INET,
            (unsigned int )infop2->hi_addrtype);
    CPPUNIT_ASSERT(
            !(strcmp(infop2->hi_hostname, "lfb3.hm2.com")));
    CPPUNIT_ASSERT_EQUAL(addr2.s_addr, infop2->hi_addr.s_addr);
}


void TESTNAME::test_cmdlstnr2()
{
    char buffer[65535];
    struct sockaddr_un server;
    string cmd = "hostgroup config.parse2.netchasm.net";

    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");

    std::this_thread::sleep_for(1s);
    read(sock_fd, buffer, sizeof(buffer));
    close(sock_fd);
    sock_fd = 0;
    hm_nameinfo_sock_t* ni = (hm_nameinfo_sock_t*) buffer;
    CPPUNIT_ASSERT_EQUAL(false, ni->ni_check_status);
    CPPUNIT_ASSERT_EQUAL(ENOENT, (int )ni->ni_errno);
}

void TESTNAME::test_cmdlstnr3()
{
    struct sockaddr_un server;
    string cmd = "hostcheck config.parse1.netchasm.net lfb3.hm1.com";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(2s);
    hm_hostcheck_t hostcheck;
    read(sock_fd,&hostcheck,sizeof(hostcheck));
    CPPUNIT_ASSERT_EQUAL(true, hostcheck.check_status);
    CPPUNIT_ASSERT_EQUAL(34, (int)hostcheck.connect_rt);
    CPPUNIT_ASSERT_EQUAL(1, (int)hostcheck.reason);
    CPPUNIT_ASSERT_EQUAL(45, (int)hostcheck.smoothed_connect_rt);
    CPPUNIT_ASSERT_EQUAL(0, (int)hostcheck.status);
    CPPUNIT_ASSERT_EQUAL(now.getTimeSinceEpoch()-100, (uint64_t)hostcheck.statustime);
    close(sock_fd);
    sock_fd = 0;
}
