// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlLinuxSocket.h"

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
log.type: stdout\n\
log.verbosity: debug3\n\
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
    sm = new HMStateManager;
    string master_config = "conf/dummy_master.yaml";
    sm_thr = std::thread(&HMStateManager::healthCheck, std::ref(*sm),
            master_config, HM_LOG_ERROR);
    cout << "Sleeping" << endl;
    std::this_thread::sleep_for(5s);
    cout << "Done sleeping" << endl;
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
    HMIPAddress ip, ip_ret;
    in_addr addr;
    inet_aton("192.168.1.3", &addr);
    ip.set((char*) &addr, AF_INET);
    struct sockaddr_un server;
    string cmd = "reload";

    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT(sm);
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
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0) {
        close(sock_fd);
        CPPUNIT_ASSERT("connecting error");
    }
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    int result;
    if(read(sock_fd, &result, sizeof(result)) < 0)
        CPPUNIT_FAIL("reading on stream socket");
    CPPUNIT_ASSERT_EQUAL(0, result);
    close(sock_fd);
    sock_fd = 0;
    std::this_thread::sleep_for(1s);
    sm->updateState(state);
    CPPUNIT_ASSERT(sm);
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    CPPUNIT_ASSERT_EQUAL(true, state->getDNSAddress(ip_ret));
    CPPUNIT_ASSERT(!ip_ret.toString().compare("192.168.1.1"));

}

void TESTNAME::test_cmdlstnr2()
{
    struct sockaddr_un server;
    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT(sm);
    string cmd = "reload conf/dummy_master2.yaml";
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(3000, (int )state->getConnectionTimeout());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    state.reset();
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
    int result;
    if (read(sock_fd, &result, sizeof(result)) < 0)
        CPPUNIT_FAIL("reading on stream socket");
    CPPUNIT_ASSERT_EQUAL(0, result);

    close(sock_fd);
    sock_fd = 0;
    std::this_thread::sleep_for(1s);
    sm->updateState(state);
    CPPUNIT_ASSERT(sm);
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
    HMIPAddress ip, ip_ret;
    in_addr addr;
    inet_aton("192.168.1.3", &addr);
    ip.set((char*) &addr, AF_INET);
    struct sockaddr_un server;
    string cmd = "dummy";
    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT(sm);
    std::this_thread::sleep_for(1s);
    state->setDNSServer(ip);
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    CPPUNIT_ASSERT_EQUAL(true, state->getDNSAddress(ip_ret));
    CPPUNIT_ASSERT(!ip_ret.toString().compare("192.168.1.3"));
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
    close(sock_fd);
    sock_fd = 0;
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
    struct sockaddr_un server;
    shared_ptr<HMState> state;
    sm->updateState(state);
    CPPUNIT_ASSERT(sm);
    string cmd = "reload conf/dummy_master3.yaml";
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL(2, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_MDBM, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.mdbm"));
    CPPUNIT_ASSERT(!state->getSocketPath().compare("test_sock"));
    state.reset();
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
    int result;
    if (read(sock_fd, &result, sizeof(result)) < 0)
        CPPUNIT_FAIL("reading on stream socket");
    CPPUNIT_ASSERT_EQUAL(0, result);

    close(sock_fd);
    sock_fd = 0;
    std::this_thread::sleep_for(1s);
    sm->updateState(state);
    CPPUNIT_ASSERT(sm);
    CPPUNIT_ASSERT_EQUAL(3, (int )state->getMaxThreads());
    CPPUNIT_ASSERT_EQUAL((int )HM_CHECK_PLUGIN_HTTP_CURL,
            (int )state->getDefaultHTTPCheckype());
    CPPUNIT_ASSERT_EQUAL((int )HM_STORAGE_TEXT, (int )state->getStorageType());
    CPPUNIT_ASSERT(!state->getStoragePath().compare("netchasm.text"));
}

void TESTNAME::test_cmdlstnr5()
{
    struct sockaddr_un server;
    string cmd = "hostgrouplist";
    char result[] = "config.parse1.netchasm.net,config.parse2.netchasm.net,config.parse3.netchasm.net,config.parse4.netchasm.net,config.parse5.netchasm.net,config.parse6.netchasm.net,config.parse7.netchasm.net";
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
    size_t grpNamesSize;
    read(sock_fd,&grpNamesSize,sizeof(grpNamesSize));
    char *data = new char[grpNamesSize + 1];
    int n = read(sock_fd,data,grpNamesSize);
    data[n] = '\0';
    CPPUNIT_ASSERT_EQUAL(188, (int )grpNamesSize);
    CPPUNIT_ASSERT(!strncmp(data,result,strlen(data)));
    CPPUNIT_ASSERT(!strncmp(data,result,strlen(result)));
    delete[] data;
    close(sock_fd);
    sock_fd = 0;
}

void TESTNAME::test_cmdlstnr6()
{
    struct sockaddr_un server;
    string cmd = "hostlist config.parse1.netchasm.net";
    char result[] = "loadfb3.hm1.com,loadfb3.hm2.com";
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
    size_t hstNamesSize;
    read(sock_fd,&hstNamesSize,sizeof(hstNamesSize));
    char *data = new char[hstNamesSize + 1];
    int n = read(sock_fd,data,hstNamesSize);
    data[n] = '\0';
    CPPUNIT_ASSERT_EQUAL(31, (int )hstNamesSize);
    CPPUNIT_ASSERT(!strncmp(data,result,strlen(data)));
    CPPUNIT_ASSERT(!strncmp(data,result,strlen(result)));
    delete[] data;
    close(sock_fd);
    sock_fd = 0;
}


void TESTNAME::test_cmdlstnr7()
{
    struct sockaddr_un server;
    string cmd = "hostlist dummy.parse1.netchasm.net";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0) {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);
    size_t hstNamesSize;
    read(sock_fd,&hstNamesSize,sizeof(hstNamesSize));
    CPPUNIT_ASSERT_EQUAL(0, (int )hstNamesSize);
    close(sock_fd);
    sock_fd = 0;
}


void TESTNAME::test_cmdlstnr8()
{
    struct sockaddr_un server;
    string cmd = "hostgroupparams config.parse1.netchasm.net";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0) {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);
    size_t buflen;
    read(sock_fd,&buflen,sizeof(buflen));
    CPPUNIT_ASSERT_EQUAL(73, (int )buflen);
    char *buf = new char[buflen];
    read(sock_fd,buf,buflen);
    hm_grpcheckparams_t *param = (hm_grpcheckparams_t*)buf;
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_TCP, (int )param->checkType);
    CPPUNIT_ASSERT_EQUAL(123, (int )param->port);
    CPPUNIT_ASSERT_EQUAL(3, (int )param->dualStack);
    CPPUNIT_ASSERT_EQUAL(9, (int )param->checkInfoSize);
    CPPUNIT_ASSERT_EQUAL(5, (int )param->smoothingWindow);
    CPPUNIT_ASSERT_EQUAL(3, (int )param->maxFlaps);
    CPPUNIT_ASSERT_EQUAL(12, (int )param->flapThreshold);
    CPPUNIT_ASSERT_EQUAL(2, (int )param->numCheckRetries);
    CPPUNIT_ASSERT_EQUAL(3, (int )param->checkRetryDelay);
    CPPUNIT_ASSERT_EQUAL(12, (int )param->groupThreshold);
    CPPUNIT_ASSERT_EQUAL(34, (int )param->slowThreshold);
    CPPUNIT_ASSERT_EQUAL(2000, (int )param->checkTimeout);
    CPPUNIT_ASSERT_EQUAL(60000, (int )param->checkTTL);
    CPPUNIT_ASSERT_EQUAL(0, (int )param->mode);
    CPPUNIT_ASSERT(!strncmp(param->check_info,"hm-hello",11));
    delete[] buf;
    close(sock_fd);
    sock_fd = 0;
}


void TESTNAME::test_cmdlstnr9()
{
    struct sockaddr_un server;
    string cmd = "hostgroupparams dummy.parse1.netchasm.net";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0) {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);
    size_t buflen;
    read(sock_fd,&buflen,sizeof(buflen));
    CPPUNIT_ASSERT_EQUAL(0, (int )buflen);
    close(sock_fd);
    sock_fd = 0;
}


void TESTNAME::test_cmdlstnr10()
{
    struct sockaddr_un server;
    string cmd = "hostgrouplist";
    char result[] = "config.parse1.netchasm.net,config.parse2.netchasm.net,config.parse3.netchasm.net,config.parse4.netchasm.net,config.parse5.netchasm.net,config.parse6.netchasm.net,config.parse7.netchasm.net";
    sock_fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, "test_sock");
    if (connect(sock_fd, (struct sockaddr *) &server,
            sizeof(struct sockaddr_un)) < 0) {
        close(sock_fd);
        CPPUNIT_FAIL("connecting error");
    }
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
            CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);
    size_t grpNamesSize;
    read(sock_fd,&grpNamesSize,sizeof(grpNamesSize));
    char *data = new char[grpNamesSize + 1];
    int n = read(sock_fd,data,grpNamesSize);
    data[n] = '\0';
    CPPUNIT_ASSERT_EQUAL(188, (int )grpNamesSize);
    CPPUNIT_ASSERT_EQUAL(188, n);
    CPPUNIT_ASSERT(!strncmp(data,result,n));
    delete[] data;

    cmd = "hostlist config.parse1.netchasm.net";
    char result1[] = "loadfb3.hm1.com,loadfb3.hm2.com";
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);
    size_t hstNamesSize;
    read(sock_fd, &hstNamesSize, sizeof(hstNamesSize));
    data = new char[hstNamesSize + 1];
    n = read(sock_fd, data, hstNamesSize);
    data[n] = '\0';
    CPPUNIT_ASSERT_EQUAL(31, (int )hstNamesSize);
    CPPUNIT_ASSERT_EQUAL(31, n);
    CPPUNIT_ASSERT(!strncmp(data, result1, n));
    delete[] data;

    cmd = "hostgroupparams config.parse1.netchasm.net";
    if (write(sock_fd, cmd.c_str(), cmd.length()) < 0)
        CPPUNIT_FAIL("writing on stream socket");
    std::this_thread::sleep_for(1s);
    size_t buflen;
    read(sock_fd, &buflen, sizeof(buflen));
    CPPUNIT_ASSERT_EQUAL(73, (int )buflen);
    char *buf = new char[buflen];
    read(sock_fd, buf, buflen);
    hm_grpcheckparams_t *param = (hm_grpcheckparams_t*) buf;
    CPPUNIT_ASSERT_EQUAL((int )HM_CHECK_TCP, (int )param->checkType);
    CPPUNIT_ASSERT_EQUAL(123, (int )param->port);
    CPPUNIT_ASSERT_EQUAL(3, (int )param->dualStack);
    CPPUNIT_ASSERT_EQUAL(9, (int )param->checkInfoSize);
    CPPUNIT_ASSERT_EQUAL(5, (int )param->smoothingWindow);
    CPPUNIT_ASSERT_EQUAL(3, (int )param->maxFlaps);
    CPPUNIT_ASSERT_EQUAL(12, (int )param->flapThreshold);
    CPPUNIT_ASSERT_EQUAL(2, (int )param->numCheckRetries);
    CPPUNIT_ASSERT_EQUAL(3, (int )param->checkRetryDelay);
    CPPUNIT_ASSERT_EQUAL(12, (int )param->groupThreshold);
    CPPUNIT_ASSERT_EQUAL(34, (int )param->slowThreshold);
    CPPUNIT_ASSERT_EQUAL(2000, (int )param->checkTimeout);
    CPPUNIT_ASSERT_EQUAL(60000, (int )param->checkTTL);
    CPPUNIT_ASSERT_EQUAL(0, (int )param->mode);

    CPPUNIT_ASSERT(!strncmp(param->check_info, "hm-hello", 11));
    delete[] buf;
    std::this_thread::sleep_for(4s);
    n = write(sock_fd, cmd.c_str(), cmd.length());
    CPPUNIT_ASSERT_EQUAL(-1, n);
    CPPUNIT_ASSERT_EQUAL(EPIPE, errno);
    close(sock_fd);
    sock_fd = 0;
}
