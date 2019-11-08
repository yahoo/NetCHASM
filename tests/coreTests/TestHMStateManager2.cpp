// Copyright (c) 2019 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.

#include "common.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fstream>

#include "TestHMStateManager2.h"
#include "HMStateManager.h"
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    mkdir("conf2", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("conf2/hm", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    std::ofstream fout1("conf2/dummy_master.yaml");
    std::ofstream fout2("conf2/hm/testconf.yaml");

    fout1 << "threads.max: 5\n\
threads.min: 2\n\
config.load-file: ./conf2/hm/testconf.yaml\n\
dns.type: ares\n\
http.type: curl\n\
ftp.type: curl\n\
tcp.type: rawsocket\n\
none.type: none\n\
db.type: notifier\n\
log.path: netchasm.log\n\
log.type: 0\n\
log.verbosity: 0\n\
socket.path: test_sock" << endl;

    fout2 << "-   name: nhg1\n\
    check-type: https-mark\n\
    check-info: /status.html\n\
    dns-type: static\n\
    check-port: 123\n\
    check-retries: 3\n\
    check-retry-delay: 0\n\
    timeout: 2000\n\
    ttl: 60000\n\
    source-address: 10.10.10.1\n\
    host:\n\
       - nhg1_172_20_10_1\n\
       - nhg1_172_20_10_2" << endl;

    fout1.close();
    fout2.close();
}

void TESTNAME::tearDown()
{

    remove("conf2/dummy_master.yaml");
    remove("conf2/hm/testconf.yaml");
    remove("conf2/hm");
    remove("conf2");
}

void TESTNAME::test_load_configs_backend()
{
    // Test basic loading of configs
    HMStateManager *sm = new HMStateManager();
    string master_config = "conf2/dummy_master.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm->loadDaemonState(master_config, HM_LOG_ERROR));
    shared_ptr<HMState> current;
    sm->updateState(current);
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_HTTP_CURL, (int)current->getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int)HM_STORAGE_NOTIFIER, (int)current->getStorageType());
    auto it = current->m_hostGroups.find("nhg1");
    CPPUNIT_ASSERT(it != current->m_hostGroups.end());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_MARK_HTTPS, it->second.getCheckType());
    CPPUNIT_ASSERT_EQUAL(123, (int)it->second.getCheckPort());
    current.reset();
    delete sm;
}
