#ifndef TESTS_HMCONTROLTLSSOCKET8_H_
#define TESTS_HMCONTROLTLSSOCKET8_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMStateManager.h"


#define TESTNAME Test_HMControlTLSSocket8

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_cmdlstnr1);
    CPPUNIT_TEST(test_cmdlstnr2);
    CPPUNIT_TEST(test_cmdlstnr3);
    CPPUNIT_TEST(test_cmdlstnr4);
    CPPUNIT_TEST(test_cmdlstnr5);
    CPPUNIT_TEST(test_cmdlstnr6);
    CPPUNIT_TEST(test_cmdlstnr7);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_cmdlstnr1();
    void test_cmdlstnr2();
    void test_cmdlstnr3();
    void test_cmdlstnr4();
    void test_cmdlstnr5();
    void test_cmdlstnr6();
    void test_cmdlstnr7();
    HMStateManager *sm;
    std::thread sm_thr;
    HMTimeStamp now;
    HMAPIIPAddress server;
    uint16_t port;
    std::string certfile;
    std::string keyfile;
    std::string caFile;
};
#endif /* TESTS_HMCONTROLTLSSOCKET8_H_ */
