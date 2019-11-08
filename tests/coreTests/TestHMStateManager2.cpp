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

// dummy log class to test passing a log to HMStateManager on init
class HMLogDummy : public HMLogBase
{
public:
    ~HMLogDummy(){};
    std::string m_str;

private:
    bool openLog(std::string logFile) {
       (void)logFile;
       m_str += "openLog";
       return true;
    };

    void closeLog() {};

    void writeLog(LogEntry *entry) {
        if (entry->length > 0 && entry->level <= m_logLevel) {
            m_str += PRINT_LOG_LEVEL[entry->level];
            m_str += entry->entry;
            m_str += "\n";
        }
    };

    void rotate() {};
};

void TESTNAME::setUp()
{
    mkdir("conf2", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("conf2/hm", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    std::ofstream fout1("conf2/dummy_master.yaml");
    std::ofstream fout2("conf2/dummy_master2.yaml");
    std::ofstream fout3("conf2/hm/testconf.yaml");

    fout1 << "threads.max: 5\n\
threads.min: 2\n\
config.load-file: ./conf2/hm/testconf.yaml\n\
dns.type: ares\n\
http.type: curl\n\
ftp.type: curl\n\
tcp.type: rawsocket\n\
none.type: none\n\
db.type: notifier\n\
log.type: registered\n\
log.path : ""\n\
log.verbosity: debug\n\
socket.path: test_sock" << endl;

    fout2 << "threads.max: 5\n\
threads.min: 2\n\
config.load-file: ./conf2/hm/testconf.yaml\n\
dns.type: ares\n\
http.type: curl\n\
ftp.type: curl\n\
tcp.type: rawsocket\n\
none.type: none\n\
db.type: notifier\n\
log.type: text\n\
log.path : /tmp\n\
log.verbosity: debug\n\
socket.path: test_sock" << endl;

    fout3 << "-   name: nhg1\n\
    check-type: https-mark\n\
    check-info: /status.html\n\
    dns-type: static\n\
    check-port: 123\n\
    check-retries: 3\n\
    check-retry-delay: 0\n\
    timeout: 2\n\
    ttl: 6\n\
    source-address: 10.10.10.1\n\
    host:\n\
       - nhg1_172_20_10_1\n\
       - nhg1_172_20_10_2" << endl;

    fout1.close();
    fout2.close();
    fout3.close();
}

void TESTNAME::tearDown()
{
    remove("conf2/dummy_master.yaml");
    remove("conf2/dummy_master2.yaml");
    remove("conf2/hm/testconf.yaml");
    remove("conf2/hm");
    remove("conf2");
}

void TESTNAME::test_load_configs_backend()
{
    // Test basic loading of configs
    shared_ptr<HMLogBase> log = make_shared<HMLogDummy>();
    HMStateManager *sm = new HMStateManager();
    sm->registerLog(log);
    string masterConfig = "conf2/dummy_master.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm->loadDaemonState(masterConfig, HM_LOG_ERROR));
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

void TESTNAME::test_initShutdown()
{
    // Test the healthCheck init routine and shuting down the threads
    shared_ptr<HMLogBase> log = make_shared<HMLogDummy>();
    HMStateManager *sm = new HMStateManager();
    sm->registerLog(log);
    string masterConfig = "conf2/dummy_master.yaml";
    // Init shouldn't block
    CPPUNIT_ASSERT_EQUAL(true, sm->healthCheck(masterConfig, HM_LOG_DEBUG3, true));
    // Give time for the threads to stabilize
    sleep(5);
    // Shutdown all threads should terminate
    sm->shutdownThreads();
    delete sm;
}

void TESTNAME::test_initWithLog()
{
    HMLogDummy * dummyLog = new HMLogDummy();
    shared_ptr<HMLogBase> log;
    log.reset( dummyLog );
    HMStateManager *sm = new HMStateManager();
    // Test the healthCheck init routine with logs.
    // Tests reload making sure the log is not changed.
    sm->registerLog(log);
    string masterConfig = "conf2/dummy_master.yaml";
    // Init shouldn't block
    CPPUNIT_ASSERT_EQUAL(true, sm->healthCheck(masterConfig, HM_LOG_DEBUG3, true));
    // Check there are logs inside of the HMLogDummy
    CPPUNIT_ASSERT_EQUAL(true, dummyLog->m_str.length() > 0);
    // Make sure the logger is the log we created
    CPPUNIT_ASSERT_EQUAL(true, hlog == log );
    size_t len = dummyLog->m_str.length();
    // reload and make sure the log is still the same
    CPPUNIT_ASSERT_EQUAL(true, sm->reloadDaemonConfigs());
    CPPUNIT_ASSERT_EQUAL(true, hlog == log);
    CPPUNIT_ASSERT_EQUAL(true, dummyLog->m_str.length() > len);
    // change the loglevel
    sm->setLogLevel(HM_LOG_CRITICAL);
    CPPUNIT_ASSERT_EQUAL(true, hlog == log);
    CPPUNIT_ASSERT_EQUAL(true, hlog->getLevel() == HM_LOG_CRITICAL);
    // change the logtype with a reload
    string masterConfig2 = "conf2/dummy_master2.yaml";
    CPPUNIT_ASSERT_EQUAL(true, sm->reloadDaemonConfigs(masterConfig2));
    CPPUNIT_ASSERT_EQUAL(true, hlog != log);
    // reload again to use the registered log
    CPPUNIT_ASSERT_EQUAL(true, sm->reloadDaemonConfigs(masterConfig));
    CPPUNIT_ASSERT_EQUAL(true, hlog == log);
}
