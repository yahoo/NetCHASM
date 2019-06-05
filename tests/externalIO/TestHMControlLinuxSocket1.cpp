// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlLinuxSocket1.h"

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
    sock_fd = 0;
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
socket.path: test_sock1" << endl;


    fout2.close();
    sm = new HMStateManager;
    string master_config = "conf/dummy_master2.yaml";
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
    struct sockaddr_un server;
    string cmd = "workqueueinfo";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock1");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0) {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);
    uint32_t workQueueSize;
    read(sock_fd,&workQueueSize,sizeof(workQueueSize));
    CPPUNIT_ASSERT_EQUAL(0, (int )workQueueSize);
    close(sock_fd);
    sock_fd = 0;
}


void TESTNAME::test_cmdlstnr7()
{
    CPPUNIT_ASSERT_EQUAL(HM_LOG_NOTICE, sm->getLogLevel());
    struct sockaddr_un server;
    string cmd = "setloglevel debug3";
    string cmd_get = "getloglevel";
    string cmd1 = "setloglevel error";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock1");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    int8_t loglevel;
    read(sock_fd, &loglevel, sizeof(loglevel));
    CPPUNIT_ASSERT_EQUAL(HM_LOG_NOTICE, HM_LOG_LEVEL(loglevel));

    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL(HM_LOG_DEBUG3, sm->getLogLevel());
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");

    read(sock_fd,&loglevel,sizeof(loglevel));
    CPPUNIT_ASSERT_EQUAL(HM_LOG_DEBUG3, HM_LOG_LEVEL(loglevel));

    if (write(sock_fd, cmd1.c_str(), cmd1.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL(HM_LOG_ERROR, sm->getLogLevel());
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    read(sock_fd, &loglevel, sizeof(loglevel));
    CPPUNIT_ASSERT_EQUAL(HM_LOG_ERROR, HM_LOG_LEVEL(loglevel));

    close(sock_fd);
    sock_fd = 0;
}

void TESTNAME::test_cmdlstnr8()
{
    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT_EQUAL((uint64_t)3000, state->getConnectionTimeout());
    struct sockaddr_un server;
    string cmd = "setconnectiontimeout 10";
    string cmd1 = "setconnectiontimeout 5000";
    string cmd_get = "getconnectiontimeout";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock1");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    uint64_t timeout;
    read(sock_fd, &timeout, sizeof(timeout));
    CPPUNIT_ASSERT_EQUAL(3000, (int)timeout);

    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL((uint64_t)10, state->getConnectionTimeout());
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");

    read(sock_fd,&timeout,sizeof(timeout));
    CPPUNIT_ASSERT_EQUAL((uint64_t)10, timeout);

    if (write(sock_fd, cmd1.c_str(), cmd1.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL((uint64_t )5000, state->getConnectionTimeout());
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    read(sock_fd, &timeout, sizeof(timeout));
    CPPUNIT_ASSERT_EQUAL((uint64_t )5000, timeout);
    close(sock_fd);
    sock_fd = 0;
}

void TESTNAME::test_cmdlstnr9()
{
    struct sockaddr_un server;
    string cmd = "setmonitorfrequency 10";
    string cmd1 = "setmonitorfrequency 5";
    string cmd_get = "getmonitorfrequency";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock1");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    uint32_t freq;
    read(sock_fd, &freq, sizeof(freq));
    CPPUNIT_ASSERT_EQUAL(2, (int)freq);

    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");

    read(sock_fd,&freq,sizeof(freq));
    CPPUNIT_ASSERT_EQUAL((uint32_t)10, freq);

    if (write(sock_fd, cmd1.c_str(), cmd1.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    read(sock_fd, &freq, sizeof(freq));
    CPPUNIT_ASSERT_EQUAL((uint32_t )5, freq);
    close(sock_fd);
    sock_fd = 0;
}


void TESTNAME::test_cmdlstnr10()
{
    struct sockaddr_un server;
    string cmd = "setttlthreshold 10";
    string cmd1 = "setttlthreshold 5";
    string cmd_get = "getttlthreshold";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock1");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    uint32_t ttl_tresh;
    read(sock_fd, &ttl_tresh, sizeof(ttl_tresh));
    CPPUNIT_ASSERT_EQUAL(10, (int)ttl_tresh);

    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");

    read(sock_fd,&ttl_tresh,sizeof(ttl_tresh));
    CPPUNIT_ASSERT_EQUAL((uint32_t)10, ttl_tresh);

    if (write(sock_fd, cmd1.c_str(), cmd1.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    read(sock_fd, &ttl_tresh, sizeof(ttl_tresh));
    CPPUNIT_ASSERT_EQUAL((uint32_t )5, ttl_tresh);
    close(sock_fd);
    sock_fd = 0;
}

void TESTNAME::test_cmdlstnr11()
{
    struct sockaddr_un server;
    string cmd = "setstride 10";
    string cmd1 = "setstride 5";
    string cmd_get = "getstride";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock1");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    uint32_t stride;
    read(sock_fd, &stride, sizeof(stride));
    CPPUNIT_ASSERT_EQUAL(10, (int)stride);

    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");

    read(sock_fd,&stride,sizeof(stride));
    CPPUNIT_ASSERT_EQUAL((uint32_t)10, stride);

    if (write(sock_fd, cmd1.c_str(), cmd1.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    read(sock_fd, &stride, sizeof(stride));
    CPPUNIT_ASSERT_EQUAL((uint32_t )5, stride);
    close(sock_fd);
    sock_fd = 0;
}

void TESTNAME::test_cmdlstnr12()
{
    struct sockaddr_un server;
    string cmd = "setworkperthreadratio 10";
    string cmd1 = "setworkperthreadratio 5";
    string cmd_get = "getworkperthreadratio";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock1");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    uint32_t stride;
    read(sock_fd, &stride, sizeof(stride));
    CPPUNIT_ASSERT_EQUAL(4, (int)stride);

    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");

    read(sock_fd,&stride,sizeof(stride));
    CPPUNIT_ASSERT_EQUAL((uint32_t)10, stride);

    if (write(sock_fd, cmd1.c_str(), cmd1.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    read(sock_fd, &stride, sizeof(stride));
    CPPUNIT_ASSERT_EQUAL((uint32_t )5, stride);
    close(sock_fd);
    sock_fd = 0;
}



void TESTNAME::test_cmdlstnr13()
{
    struct sockaddr_un server;
    string cmd = "setrecycle on";
    string cmd1 = "setrecycle off";
    string cmd_get = "getrecycle";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock1");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0)
    {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    bool recycle;
    read(sock_fd, &recycle, sizeof(recycle));
    CPPUNIT_ASSERT(!recycle);

    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");

    read(sock_fd,&recycle,sizeof(recycle));
    CPPUNIT_ASSERT(recycle);

    if (write(sock_fd, cmd1.c_str(), cmd1.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);

    if (write(sock_fd, cmd_get.c_str(), cmd_get.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    read(sock_fd, &recycle, sizeof(recycle));
    CPPUNIT_ASSERT(!recycle);
    close(sock_fd);
    sock_fd = 0;
}
