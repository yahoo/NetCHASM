// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMControlTLSSocket6.h"

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
#include "HMControlTLSSocketClient.h"

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    mkdir("conf", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("conf/hm", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    std::ofstream fout1("conf/dummy_master.yaml");

    fout1 << "threads.max: 2\n\
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
control-server-linux: off\n\
control-socket-check-portv4 : 10053\n\
control-server-ipv4 : on\n\
control-server-ipv6 : off\n\
enable-secure-remote : on\n\
cert-file: " + std::string(CERT_FOLDER) + "/server.crt\n\
key-file: " + std::string(CERT_FOLDER) + "/server.key\n\
ca-file: " + std::string(CERT_FOLDER) + "/ca.crt\n\
socket.path: test_sock" << endl;

    fout1.close();
    string ipaddr = "127.0.0.1";
    server.set(ipaddr);
    port = HM_CONTROL_SOCKET_DEFAULT_PORTV4;

    setupCommon();
    string mdbm = "netchasm.mdbm";
    remove(mdbm.c_str());
    HMDataHostGroupMap groupMap;
    HMConfigInfo configInfo;
    configInfo.m_version = HM_MDBM_VERSION;
    configInfo.m_configStatus = HM_CONFIG_STATUS_OK;
    configInfo.m_configLoadTime = HMTimeStamp::now();
    HMState checkState;
    HMDNSCache dnsCache;
    HMStorageHostGroupMDBM* store = new HMStorageHostGroupMDBM(mdbm, &groupMap, &dnsCache);
    CPPUNIT_ASSERT(store->openStore());
    CPPUNIT_ASSERT(store->storeConfigInfo(configInfo));
    std::this_thread::sleep_for(10ms);
    store->closeStore();
    delete store;
    teardownCommon();
    sm = new HMStateManager;
    certfile = std::string(CERT_FOLDER) + "/client_self.crt";
    keyfile = std::string(CERT_FOLDER) + "/client.key";
    caFile = std::string(CERT_FOLDER) + "/ca.crt";
    string master_config = "conf/dummy_master.yaml";
    sm_thr = std::thread(&HMStateManager::healthCheck, std::ref(*sm),
            master_config, HM_LOG_ERROR);
    std::this_thread::sleep_for(4s);
}

void TESTNAME::tearDown()
{
    remove("conf/dummy_master.yaml");
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
	CPPUNIT_ASSERT(!socketAPI.isConnected());
	CPPUNIT_ASSERT(socketAPI.getErrorMsg().find("Failed to connect socket ssl, error:") != string::npos);
}


void TESTNAME::test_cmdlstnr2()
{
    keyfile = std::string(CERT_FOLDER) + "/client_mismatch.key";
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    CPPUNIT_ASSERT(!socketAPI.isConnected());
    CPPUNIT_ASSERT(!socketAPI.getErrorMsg().compare("Failed loading private key file, err:error:0B080074:x509 certificate routines:X509_check_private_key:key values mismatch"));
}

void TESTNAME::test_cmdlstnr3()
{
    certfile = "";
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
	CPPUNIT_ASSERT(!socketAPI.isConnected());
	CPPUNIT_ASSERT(!socketAPI.getErrorMsg().compare("Failed loading certificate file, err:error:02001002:system library:fopen:No such file or directory"));
}

void TESTNAME::test_cmdlstnr4()
{
    keyfile = "";
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    CPPUNIT_ASSERT(!socketAPI.isConnected());
    CPPUNIT_ASSERT(!socketAPI.getErrorMsg().compare("Failed loading private key file, err:error:20074002:BIO routines:file_ctrl:system lib"));
}


void TESTNAME::test_cmdlstnr5()
{
    caFile = "";
	HMControlTLSSocketClient socketAPI(server, port, certfile, keyfile, caFile);
    CPPUNIT_ASSERT(!socketAPI.isConnected());
    CPPUNIT_ASSERT(!socketAPI.getErrorMsg().compare("failed loading CA certificate, err:error:02001002:system library:fopen:No such file or directory"));
}

void TESTNAME::test_cmdlstnr6()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL_CTX* ctx = SSL_CTX_new(TLSv1_2_method());
	int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        SSL_CTX_free(ctx);
        close(sock);
        CPPUNIT_ASSERT(false);
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t len = sizeof(sockaddr_in);
    if (connect(sock, (struct sockaddr *) &addr, len) == -1)
    {
        close(sock);
        SSL_CTX_free(ctx);
        CPPUNIT_ASSERT(false);
    }
    SSL* ssl = SSL_new(ctx);
    if (!ssl)
    {
        SSL_CTX_free(ctx);
        close(sock);
        CPPUNIT_ASSERT(false);

    }
    SSL_set_fd(ssl, sock);
    CPPUNIT_ASSERT(SSL_connect(ssl) <= 0);
    string error = "error:14094410:SSL routines:ssl3_read_bytes:sslv3 alert handshake failure";
    CPPUNIT_ASSERT(!error.compare(ERR_error_string(ERR_get_error(), NULL)));
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sock);
}
