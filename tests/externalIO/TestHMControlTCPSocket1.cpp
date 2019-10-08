// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlTCPSocket1.h"

#include <sys/stat.h>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include "common.h"
#include "HMConstants.h"
#include "HMStateManager.h"
#include "HMControlTCPSocketClient.h"

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    mkdir("conf", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    std::ofstream fout2("conf/dummy_master2.yaml");

    fout2 << "threads: 3\n\
dns.type: none\n\
dns.ttl: 360\n\
dns.lookup-timeout: 60\n\
http.type: curl\n\
\n\
\n\
db.type: mdbm\n\
db.path: netchasm.mdbm\n\
log.path: yHealth.log\n\
control-server-linux: off\n\
control-socket-check-portv6 : 10054\n\
control-server-ipv4 : off\n\
control-server-ipv6 : on\n\
enable-secure-remote : off\n\
socket.path: test_sock1" << endl;


    fout2.close();
    string ipaddr = "::1";
    server.set(ipaddr);
    port = HM_CONTROL_SOCKET_DEFAULT_PORTV6;
    sm = new HMStateManager;
    string master_config = "conf/dummy_master2.yaml";
    sm_thr = std::thread(&HMStateManager::healthCheck, std::ref(*sm),
            master_config, HM_LOG_ERROR);
    std::this_thread::sleep_for(2s);
    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT(state);
    state.reset();
}

void TESTNAME::tearDown()
{
    remove("conf/dummy_master2.yaml");
    remove("conf");
    remove("netchasm.mdbm");
    sm->shutdown();
    std::this_thread::sleep_for(1s);
    sm_thr.join();
    delete(sm);
}

void TESTNAME::test_cmdlstnr6()
{
    HMControlTCPSocketClient socketAPI(server, port);
    uint32_t workQueueSize;
    CPPUNIT_ASSERT(socketAPI.getWorkQueue(workQueueSize));
}


void TESTNAME::test_cmdlstnr7()
{
    HMControlTCPSocketClient socketAPI(server, port);
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

void TESTNAME::test_cmdlstnr8()
{
    HMControlTCPSocketClient socketAPI(server, port);
    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT_EQUAL((uint64_t)3000, state->getConnectionTimeout());
    uint64_t timeout;
    CPPUNIT_ASSERT(socketAPI.getConnectionTimeout(timeout));
    CPPUNIT_ASSERT_EQUAL(3000, (int)timeout);
    CPPUNIT_ASSERT(socketAPI.setConnectionTimeOut(10));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT_EQUAL((uint64_t)10, state->getConnectionTimeout());
    CPPUNIT_ASSERT(socketAPI.getConnectionTimeout(timeout));
    CPPUNIT_ASSERT_EQUAL((uint64_t)10, timeout);
    // testing the timeout
    std::this_thread::sleep_for(5s);
    CPPUNIT_ASSERT(socketAPI.setConnectionTimeOut(5000));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT_EQUAL((uint64_t )5000, state->getConnectionTimeout());
    CPPUNIT_ASSERT(socketAPI.getConnectionTimeout(timeout));
    CPPUNIT_ASSERT_EQUAL((uint64_t )5000, timeout);
}

void TESTNAME::test_cmdlstnr9()
{
    HMControlTCPSocketClient socketAPI(server, port);
    uint32_t freq;
    CPPUNIT_ASSERT(socketAPI.getMonitoringFrequency(freq));
    CPPUNIT_ASSERT_EQUAL(2, (int)freq);
    CPPUNIT_ASSERT(socketAPI.setMonitoringFrequency(10));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getMonitoringFrequency(freq));
    CPPUNIT_ASSERT_EQUAL((uint32_t)10, freq);
    // testing the timeout
    std::this_thread::sleep_for(5s);
    CPPUNIT_ASSERT(socketAPI.setMonitoringFrequency(5));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getMonitoringFrequency(freq));
    CPPUNIT_ASSERT_EQUAL((uint32_t )5, freq);
}


void TESTNAME::test_cmdlstnr10()
{
    HMControlTCPSocketClient socketAPI(server, port);
    uint32_t ttl_tresh;
    CPPUNIT_ASSERT(socketAPI.getTTLTreshold(ttl_tresh));
    CPPUNIT_ASSERT_EQUAL(10, (int)ttl_tresh);
    CPPUNIT_ASSERT(socketAPI.setTTLTreshold(10));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getTTLTreshold(ttl_tresh));
    CPPUNIT_ASSERT_EQUAL((uint32_t)10, ttl_tresh);
    // testing the timeout
    std::this_thread::sleep_for(5s);
    CPPUNIT_ASSERT(socketAPI.setTTLTreshold(5));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getTTLTreshold(ttl_tresh));
    CPPUNIT_ASSERT_EQUAL((uint32_t )5, ttl_tresh);
}

void TESTNAME::test_cmdlstnr11()
{
    HMControlTCPSocketClient socketAPI(server, port);
    uint32_t stride;
    CPPUNIT_ASSERT(socketAPI.getStride(stride));
    CPPUNIT_ASSERT_EQUAL(10, (int)stride);
    CPPUNIT_ASSERT(socketAPI.setStride(10));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getStride(stride));
    CPPUNIT_ASSERT_EQUAL((uint32_t)10, stride);
    // testing the timeout
    std::this_thread::sleep_for(5s);
    CPPUNIT_ASSERT(socketAPI.setStride(5));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getStride(stride));
    CPPUNIT_ASSERT_EQUAL((uint32_t )5, stride);
}

void TESTNAME::test_cmdlstnr12()
{
    HMControlTCPSocketClient socketAPI(server, port);
    uint32_t workPerThread;
    CPPUNIT_ASSERT(socketAPI.getWorkPerThread(workPerThread));
    CPPUNIT_ASSERT_EQUAL(4, (int)workPerThread);
    CPPUNIT_ASSERT(socketAPI.setWorkPerThread(10));
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT(socketAPI.getWorkPerThread(workPerThread));
    CPPUNIT_ASSERT_EQUAL((uint32_t)10, workPerThread);
    // testing the timeout
    std::this_thread::sleep_for(5s);
    CPPUNIT_ASSERT(socketAPI.setWorkPerThread(5));
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.getWorkPerThread(workPerThread));
    CPPUNIT_ASSERT_EQUAL((uint32_t )5, workPerThread);
}



void TESTNAME::test_cmdlstnr13()
{
    HMControlTCPSocketClient socketAPI(server, port);
    CPPUNIT_ASSERT(!socketAPI.isRecycleOn());
    CPPUNIT_ASSERT(socketAPI.setRecycleOn());
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(socketAPI.isRecycleOn());
    // testing the timeout
    std::this_thread::sleep_for(5s);
    CPPUNIT_ASSERT(socketAPI.setRecycleOff());
    std::this_thread::sleep_for(1s);
    CPPUNIT_ASSERT(!socketAPI.isRecycleOn());
}
