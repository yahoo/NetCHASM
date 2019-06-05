// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMIPAddress.h"

#include <cstring>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include "cstdint"

#include "HMIPAddress.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp() {

}

void TESTNAME::tearDown() {

}

void TESTNAME::test_v4_sets() {

    HMIPAddress ip_empty;
    CPPUNIT_ASSERT(!ip_empty.isSet());

    string sAddress = "192.168.0.1";
    string sAddress2 = "0.0.0.1";
    string sAddress3 = "Bad Address";

    in_addr_t iAddress1 = 0x0100A8C0;
    in_addr_t iAddress2 = 0x01000000;
    in_addr_t iAddress3 = 0x00000000;

    addrinfo aiAddress1;
    addrinfo aiAddress2;
    addrinfo aiAddress3;

    struct sockaddr_in siAddress1;
    struct sockaddr_in siAddress2;

    memset(&siAddress1, 0, sizeof(struct sockaddr));
    memset(&siAddress2, 0, sizeof(struct sockaddr));
    memset(&aiAddress1, 0, sizeof(struct addrinfo));
    memset(&aiAddress2, 0, sizeof(struct addrinfo));
    memset(&aiAddress3, 0, sizeof(struct addrinfo));

    siAddress1.sin_family = AF_INET;
    siAddress1.sin_port = 80;
    siAddress1.sin_addr.s_addr = iAddress1;

    siAddress2.sin_family = AF_INET;
    siAddress2.sin_port = 50000;
    siAddress2.sin_addr.s_addr = iAddress2;

    aiAddress1.ai_family = AF_INET;
    aiAddress1.ai_socktype = SOCK_DGRAM;
    aiAddress1.ai_flags = AI_PASSIVE;
    aiAddress1.ai_protocol = 0;
    aiAddress1.ai_canonname = NULL;
    aiAddress1.ai_addr = (sockaddr*)&siAddress1;
    aiAddress1.ai_next = NULL;

    aiAddress2.ai_family = AF_INET;
    aiAddress2.ai_socktype = SOCK_DGRAM;
    aiAddress2.ai_flags = AI_PASSIVE;
    aiAddress2.ai_protocol = 0;
    aiAddress2.ai_canonname = NULL;
    aiAddress2.ai_addr = (sockaddr*)&siAddress2;
    aiAddress2.ai_next = NULL;

    aiAddress3.ai_family = AF_INET;
    aiAddress3.ai_socktype = SOCK_DGRAM;
    aiAddress3.ai_flags = AI_PASSIVE;
    aiAddress3.ai_protocol = 0;
    aiAddress3.ai_canonname = NULL;
    aiAddress3.ai_addr = NULL;
    aiAddress3.ai_next = NULL;

    // Begin the Actual Testing
    HMIPAddress ip(AF_INET);
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress3, ip.addr4());

    // Test: bool set(char* addr, int addrType);
    CPPUNIT_ASSERT(ip.set((char*)(&iAddress1), AF_INET));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress1, ip.addr4());


    CPPUNIT_ASSERT(ip.set((char*)(&iAddress2), AF_INET));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress2, ip.addr4());

    CPPUNIT_ASSERT(!ip.set((char*)(&iAddress1), 0));
    CPPUNIT_ASSERT(!ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_UNSPEC, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress3, ip.addr4());

    // Test: bool set(std::string addr);
    CPPUNIT_ASSERT(ip.set(sAddress));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress1, ip.addr4());


    CPPUNIT_ASSERT(ip.set(sAddress2));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress2, ip.addr4());

    CPPUNIT_ASSERT(!ip.set(sAddress3));
    CPPUNIT_ASSERT(!ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_UNSPEC, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress3, ip.addr4());

    // Test: bool set(in_addr_t &addr);
    CPPUNIT_ASSERT(ip.set(iAddress1));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress1, ip.addr4());

    CPPUNIT_ASSERT(ip.set(iAddress2));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress2, ip.addr4());

    // Test: bool set(addrinfo &addr);
    CPPUNIT_ASSERT(ip.set(aiAddress1));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress1, ip.addr4());

    CPPUNIT_ASSERT(ip.set(aiAddress2));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress2, ip.addr4());

    CPPUNIT_ASSERT(!ip.set(aiAddress3));
    CPPUNIT_ASSERT(!ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_UNSPEC, (int)ip.getType());
    CPPUNIT_ASSERT_EQUAL(iAddress3, ip.addr4());

    CPPUNIT_ASSERT(ip.set(aiAddress2));
    CPPUNIT_ASSERT(ip.isSet());
    struct sockaddr_storage ss;
    socklen_t salen = sizeof(ss);
    struct sockaddr* sa = ip.getSockaddr(&ss,&salen,32);
    CPPUNIT_ASSERT_EQUAL((int)sa->sa_family,AF_INET);

}

void TESTNAME::test_v6_sets() {

    HMIPAddress ip_empty;
    CPPUNIT_ASSERT(!ip_empty.isSet());

    string sAddress = "fdab:0010:abcd::";
    string sAddress2 = "::1";
    string sAddress3 = "Bad Address";

    struct in6_addr iAddress1;
    iAddress1.__in6_u.__u6_addr32[0] = 0x1000ABFD;
    iAddress1.__in6_u.__u6_addr32[1] = 0x0000CDAB;
    iAddress1.__in6_u.__u6_addr32[2] = 0x00000000;
    iAddress1.__in6_u.__u6_addr32[3] = 0x00000000;

    struct in6_addr iAddress2;
    iAddress2.__in6_u.__u6_addr32[0] = 0x00000000;
    iAddress2.__in6_u.__u6_addr32[1] = 0x00000000;
    iAddress2.__in6_u.__u6_addr32[2] = 0x00000000;
    iAddress2.__in6_u.__u6_addr32[3] = 0x01000000;

    struct in6_addr iAddress3;
    iAddress3.__in6_u.__u6_addr32[0] = 0x00000000;
    iAddress3.__in6_u.__u6_addr32[1] = 0x00000000;
    iAddress3.__in6_u.__u6_addr32[2] = 0x00000000;
    iAddress3.__in6_u.__u6_addr32[3] = 0x00000000;

    addrinfo aiAddress1;
    addrinfo aiAddress2;
    addrinfo aiAddress3;

    struct sockaddr_in6 siAddress1;
    struct sockaddr_in6 siAddress2;
    struct in6_addr temp;

    memset(&siAddress1, 0, sizeof(struct sockaddr));
    memset(&siAddress2, 0, sizeof(struct sockaddr));
    memset(&aiAddress1, 0, sizeof(struct addrinfo));
    memset(&aiAddress2, 0, sizeof(struct addrinfo));
    memset(&aiAddress3, 0, sizeof(struct addrinfo));

    siAddress1.sin6_family = AF_INET6;
    siAddress1.sin6_port = 80;
    siAddress1.sin6_addr = iAddress1;

    siAddress2.sin6_family = AF_INET;
    siAddress2.sin6_port = 50000;
    siAddress2.sin6_addr = iAddress2;

    aiAddress1.ai_family = AF_INET6;
    aiAddress1.ai_socktype = SOCK_DGRAM;
    aiAddress1.ai_flags = AI_PASSIVE;
    aiAddress1.ai_protocol = 0;
    aiAddress1.ai_canonname = NULL;
    aiAddress1.ai_addr = (sockaddr*)&siAddress1;
    aiAddress1.ai_next = NULL;

    aiAddress2.ai_family = AF_INET6;
    aiAddress2.ai_socktype = SOCK_DGRAM;
    aiAddress2.ai_flags = AI_PASSIVE;
    aiAddress2.ai_protocol = 0;
    aiAddress2.ai_canonname = NULL;
    aiAddress2.ai_addr = (sockaddr*)&siAddress2;
    aiAddress2.ai_next = NULL;

    aiAddress3.ai_family = AF_INET6;
    aiAddress3.ai_socktype = SOCK_DGRAM;
    aiAddress3.ai_flags = AI_PASSIVE;
    aiAddress3.ai_protocol = 0;
    aiAddress3.ai_canonname = NULL;
    aiAddress3.ai_addr = NULL;
    aiAddress3.ai_next = NULL;

    // Begin the Actual Testing
    HMIPAddress ip(AF_INET6);
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET6, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress3, &temp, sizeof(in6_addr)) == 0);

    // Test: bool set(char* addr, int addrType);
    CPPUNIT_ASSERT(ip.set((char*)(&iAddress1), AF_INET6));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET6, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress1, &temp, sizeof(in6_addr)) == 0);


    CPPUNIT_ASSERT(ip.set((char*)(&iAddress2), AF_INET6));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET6, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress2, &temp, sizeof(in6_addr)) == 0);

    CPPUNIT_ASSERT(!ip.set((char*)(&iAddress1), 0));
    CPPUNIT_ASSERT(!ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_UNSPEC, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress3, &temp, sizeof(in6_addr)) == 0);

    // Test: bool set(std::string addr);
    CPPUNIT_ASSERT(ip.set(sAddress));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET6, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress1, &temp, sizeof(in6_addr)) == 0);

    CPPUNIT_ASSERT(ip.set(sAddress2));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET6, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress2, &temp, sizeof(in6_addr)) == 0);

    CPPUNIT_ASSERT(!ip.set(sAddress3));
    CPPUNIT_ASSERT(!ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_UNSPEC, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress3, &temp, sizeof(in6_addr)) == 0);

    // Test: bool set(in_addr_t &addr);
    CPPUNIT_ASSERT(ip.set(iAddress1));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET6, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress1, &temp, sizeof(in6_addr)) == 0);

    CPPUNIT_ASSERT(ip.set(iAddress2));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET6, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress2, &temp, sizeof(in6_addr)) == 0);

    // Test: bool set(addrinfo &addr);
    CPPUNIT_ASSERT(ip.set(aiAddress1));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET6, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress1, &temp, sizeof(in6_addr)) == 0);

    CPPUNIT_ASSERT(ip.set(aiAddress2));
    CPPUNIT_ASSERT(ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_INET6, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress2, &temp, sizeof(in6_addr)) == 0);

    CPPUNIT_ASSERT(!ip.set(aiAddress3));
    CPPUNIT_ASSERT(!ip.isSet());
    CPPUNIT_ASSERT_EQUAL(AF_UNSPEC, (int)ip.getType());
    temp = ip.addr6();
    CPPUNIT_ASSERT(memcmp(&iAddress3, &temp, sizeof(in6_addr)) == 0);

    CPPUNIT_ASSERT(ip.set(aiAddress2));
    CPPUNIT_ASSERT(ip.isSet());
    struct sockaddr_storage ss6;
    socklen_t salen6 = sizeof(ss6);
    struct sockaddr* sa6 = ip.getSockaddr(&ss6,&salen6,3);
    CPPUNIT_ASSERT_EQUAL((int)sa6->sa_family,AF_INET6);

}

void TESTNAME::test_comparisonsv4() {
    HMIPAddress ip1,ip2,ip3;
    CPPUNIT_ASSERT(!ip1.isSet());
    CPPUNIT_ASSERT(!ip2.isSet());
    CPPUNIT_ASSERT(!ip3.isSet());
    
    string sAddress1 = "192.168.1.11";
    string sAddress2 = "192.168.1.10";

    CPPUNIT_ASSERT(ip1.set(sAddress1));
    CPPUNIT_ASSERT(ip1.isSet());
    CPPUNIT_ASSERT(ip2.set(sAddress2));
    CPPUNIT_ASSERT(ip2.isSet());
    CPPUNIT_ASSERT(ip3.set(sAddress2));
    CPPUNIT_ASSERT(ip3.isSet());
    
    CPPUNIT_ASSERT_EQUAL(ip2 < ip1,true);
    CPPUNIT_ASSERT_EQUAL(ip1 < ip2,false);
    CPPUNIT_ASSERT_EQUAL(ip1 < ip1,false);
    CPPUNIT_ASSERT_EQUAL(ip2 == ip1,false);
    CPPUNIT_ASSERT_EQUAL(ip2 == ip3,true);
}

void TESTNAME::test_comparisonsv6() {
    HMIPAddress ip1,ip2,ip3;
    CPPUNIT_ASSERT(!ip1.isSet());
    CPPUNIT_ASSERT(!ip2.isSet());
    CPPUNIT_ASSERT(!ip3.isSet());
    
    string sAddress1 = "1010:0010:abcd::";
    string sAddress2 = "1111:0011:abcd::";

    CPPUNIT_ASSERT(ip1.set(sAddress1));
    CPPUNIT_ASSERT(ip1.isSet());
    CPPUNIT_ASSERT(ip2.set(sAddress2));
    CPPUNIT_ASSERT(ip2.isSet());
    CPPUNIT_ASSERT(ip3.set(sAddress2));
    CPPUNIT_ASSERT(ip3.isSet());
    
    CPPUNIT_ASSERT_EQUAL(ip1 < ip2,true);
    CPPUNIT_ASSERT_EQUAL(ip2 < ip1,false);
    CPPUNIT_ASSERT_EQUAL(ip2 == ip1,false);
    CPPUNIT_ASSERT_EQUAL(ip2 == ip3,true);
}

void TESTNAME::test_printing() {
    HMIPAddress ip1;
    CPPUNIT_ASSERT(!ip1.isSet());
    
    string sAddress1 = "192.168.1.11";
    string sAddressv61 = "1010:0010:abcd::";
    string sAddressv6 = "1010:10:abcd::";

    CPPUNIT_ASSERT(ip1.set(sAddress1));
    CPPUNIT_ASSERT(ip1.isSet());
    CPPUNIT_ASSERT_EQUAL(sAddress1, ip1.toString());
        
    CPPUNIT_ASSERT(ip1.set(sAddressv61));
    CPPUNIT_ASSERT(ip1.isSet());    
    CPPUNIT_ASSERT_EQUAL(sAddressv6, ip1.toString());
}
