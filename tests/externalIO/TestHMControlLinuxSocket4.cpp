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
    sock_fd = 0;
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
    std::unique_ptr<HMAuxOOB> temp_aux2 = std::make_unique<HMAuxOOB>();
    temp_aux2->m_type = HM_OOB_FILE;
    temp_aux2->m_host = "api.hm.com";
    temp_aux2->m_resource = "canl";
    temp_aux2->m_shed = 10;
    temp_aux2->m_forceDown = false;
    auxInfo2.m_auxData.push_back(std::move(temp_aux2));
    temp_aux2 = std::make_unique<HMAuxOOB>();
    temp_aux2->m_type = HM_OOB_FILE;
    temp_aux2->m_host = "api1.hm.com";
    temp_aux2->m_resource = "canl";
    temp_aux2->m_shed = 20;
    temp_aux2->m_forceDown = true;
    auxInfo2.m_auxData.push_back(std::move(temp_aux2));
    temp_aux2 = std::make_unique<HMAuxOOB>();
    temp_aux2->m_type = HM_OOB_FILE;
    temp_aux2->m_host = "api2.hm.com";
    temp_aux2->m_resource = "canl1";
    temp_aux2->m_shed = 30;
    temp_aux2->m_forceDown = true;

    string host1 = "test.hm.com";
    string host2 = "test2.hm.com";
    string host3 = "test3.hm.com";
    string sourceURL = "lfb.html";
    string xmlOut;

    HMDataHostGroupMap groupMap;
    string checkInfo = "/hm";

    HMIPAddress address;
    address.set("1.2.3.4");

    HMIPAddress addressv6;
    addressv6.set("2001::7334");

    HMIPAddress addressZero(AF_INET6);
    set<HMIPAddress> addresses;
    addresses.insert(address);
    HMDataHostGroup hostGroup(hostGroupName);
    hostGroup.setMeasurementOptions(0);
    hostGroup.setCheckType(HM_CHECK_HTTP);
    hostGroup.setGroupThreshold(50);
    hostGroup.setCheckTTL(30);
    hostGroup.setPort(8080);
    hostGroup.setCheckInfo(checkInfo);
    hostGroup.addHost(host1);
    hostGroup.addHost(host2);
    hostGroup.addHost(host3);
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

    HMConfigInfo configInfo;
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = now;

    HMDataCheckResult testResult;
    HMState checkState;
    checkState.m_hostGroups.insert(make_pair(hostGroupName, hostGroup));
    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(mdbm, &groupMap);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigs(checkState));
    CPPUNIT_ASSERT(store->storeAuxInfo(host1, address, hostCheck, checkParams, auxInfo));
    CPPUNIT_ASSERT(store->storeAuxInfo(host2, addressv6, hostCheck, checkParams, auxInfo1));
    CPPUNIT_ASSERT(store->storeAuxInfo(host3, addressZero, hostCheck, checkParams, auxInfo2));
    CPPUNIT_ASSERT(store->storeCheckResult(host1, address, hostCheck, checkParams, checkResult));
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
            << "dns.type: none" << endl
            << "db.path: " << mdbm << endl
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
    struct sockaddr_un server;
    string path = "";

    const int filesizeAry[2] = { 367, 297 };
    const char *fileCntAry[2] =
            {
                    "<load-file updated=\"0\">\n\
\t<resource name=\"mra1-canary\">\n\
\t\t<host name=\"mra1.canary.hm.com\">\n\
\t\t\t<time>0</time>\n\
\t\t\t<load>11</load>\n\
\t\t\t<target>2500</target>\n\
\t\t\t<max>5000</max>\n\
\t\t</host>\n\
\t</resource>\n\
\t<resource name=\"api1_sats_cdn\">\n\
\t\t<host name=\"api.hm.com\">\n\
\t\t\t<time>0</time>\n\
\t\t\t<load>3</load>\n\
\t\t\t<target>38</target>\n\
\t\t\t<max>76</max>\n\
\t\t</host>\n\
\t</resource>\n\
</load-file>\n\n",
                    "<oob-file updated=\"0\">\n\
\t<resource-oob name=\"canl\">\n\
\t\t<host name=\"api.hm.com\">\n\
\t\t\t<time>0</time>\n\
\t\t\t<force-down>false</force-down>\n\
\t\t\t<shed>10</shed>\n\
\t\t</host>\n\
\t\t<host name=\"api1.hm.com\">\n\
\t\t\t<time>0</time>\n\
\t\t\t<force-down>true</force-down>\n\
\t\t\t<shed>20</shed>\n\
\t\t</host>\n\
\t</resource-oob>\n\
</oob-file>\n\n" };
    string hostGroupName = "test.netchasm.net";
    string cmd = "loadfb " + hostGroupName;
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
    uint32_t bufSize;
    read(sock_fd, &bufSize, sizeof(bufSize));
    unique_ptr<char[]> buffer = make_unique<char[]>(bufSize);
    read(sock_fd, buffer.get(), bufSize);
    close(sock_fd);
    sock_fd = 0;
    hm_loadfbdata_t *ni = (hm_loadfbdata_t *) buffer.get();
    CPPUNIT_ASSERT_EQUAL(2U, ni->ld_count);
    CPPUNIT_ASSERT_EQUAL(30000U, ni->ld_ttl);
    CPPUNIT_ASSERT_EQUAL(true, ni->ni_check_status);
    CPPUNIT_ASSERT_EQUAL(0, (int )ni->ni_errno);
    char *filecontents = &ni->ld_filecontents[0];
    unsigned int size = sizeof(hm_loadfbdata_t);
    int index = 0;
    while (size < ni->ld_size)
    {
        hm_loadfbfile_t *ldfbfile = (hm_loadfbfile_s*) filecontents;
        char *file = (char*) ldfbfile->file_buffer;
        CPPUNIT_ASSERT_EQUAL(filesizeAry[index], (int )ldfbfile->file_size);
        string actual(file);
        string expected(fileCntAry[index]);
        int pos = actual.find(">");
        int subpos = expected.find(">");
        int len = actual.length() - pos;
        int sublen = expected.length() - subpos;
        CPPUNIT_ASSERT(!actual.compare(pos, len, expected, subpos, sublen));

        size += (sizeof(hm_loadfbfile_s) + ldfbfile->file_size + 1);
        filecontents += (sizeof(hm_loadfbfile_s) + ldfbfile->file_size + 1);
        index++;
    }
}


void TESTNAME::test_cmdlstnr2()
{
    struct sockaddr_un server;
    string hostGroupName = "dummy.netchasm.net";
    string cmd = "loadfb " + hostGroupName;
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
    uint32_t bufSize;
    read(sock_fd, &bufSize, sizeof(bufSize));
    unique_ptr<char[]> buffer = make_unique<char[]>(bufSize);
    read(sock_fd, buffer.get(), bufSize);
    close(sock_fd);
    sock_fd = 0;
    hm_loadfbdata_t *ni = (hm_loadfbdata_t *) buffer.get();
    CPPUNIT_ASSERT_EQUAL(false, ni->ni_check_status);
    CPPUNIT_ASSERT_EQUAL(ENOENT, (int )ni->ni_errno);
}

void TESTNAME::test_cmdlstnr3()
{

}
