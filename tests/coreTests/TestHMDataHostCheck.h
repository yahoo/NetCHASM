// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMDataHostCheck_H_
#define TEST_HMDataHostCheck_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkQueue.h"

#define TESTNAME Test_HMDataHostCheck

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_basic_data_host_check);
    CPPUNIT_TEST(test_operations_equal);
    CPPUNIT_TEST(test_operations_nequal_checktype);
    CPPUNIT_TEST(test_operations_nequal_checkport);
    CPPUNIT_TEST(test_operations_nequal_checkinfo);
    CPPUNIT_TEST(test_operations_nequal_dualstack);
    CPPUNIT_TEST(test_operations_equal_checkplugin);
    CPPUNIT_TEST(test_operations_nequal_remote_check);
    CPPUNIT_TEST(test_print_entry);
    CPPUNIT_TEST(test_operations_less_checktype);
    CPPUNIT_TEST(test_operations_less_port);
    CPPUNIT_TEST(test_operations_less_dualstack);
    CPPUNIT_TEST(test_operations_less_checkinfo);
    CPPUNIT_TEST(test_operations_less_remote_check);
    CPPUNIT_TEST(test_parseCheckInfo_1);
    CPPUNIT_TEST(test_parseCheckInfo_2);
    CPPUNIT_TEST(test_parseCheckInfo_3);
    CPPUNIT_TEST(test_parseCheckInfo_4);
    CPPUNIT_TEST(test_parseCheckInfo_5);
    CPPUNIT_TEST(test_parseCheckInfo_6);
    CPPUNIT_TEST(test_parseCheckInfo_7);
    CPPUNIT_TEST(test_parseCheckInfo_8);
    CPPUNIT_TEST(test_parseCheckInfo_9);
    CPPUNIT_TEST(test_parseCheckInfo_10);
    CPPUNIT_TEST(test_parseCheckInfo_11);
    CPPUNIT_TEST(test_parseCheckInfo_12);
    CPPUNIT_TEST(test_parseCheckInfo_13);
    CPPUNIT_TEST(test_parseCheckInfo_14);
    CPPUNIT_TEST(test_parseCheckInfo_15);
    CPPUNIT_TEST(test_parseCheckInfo_16);
    CPPUNIT_TEST(test_parseCheckInfo_17);
    CPPUNIT_TEST(test_parseCheckInfo_18);
    CPPUNIT_TEST(test_parseCheckInfo_19);
    CPPUNIT_TEST(test_parseCheckInfo_20);
    CPPUNIT_TEST(test_parseCheckInfo_21);
    CPPUNIT_TEST(test_parseCheckInfo_22);
    CPPUNIT_TEST(test_parseCheckInfo_23);
    CPPUNIT_TEST(test_parseCheckInfo_24);
    CPPUNIT_TEST(test_parseCheckInfo_25);
    CPPUNIT_TEST(test_parseCheckInfo_26);
    CPPUNIT_TEST(test_parseCheckInfo_27);
    CPPUNIT_TEST(test_parseCheckInfo_28);
    CPPUNIT_TEST(test_parseCheckInfo_29);
    CPPUNIT_TEST(test_parseCheckInfo_30);
    CPPUNIT_TEST(test_parseCheckInfo_31);
    CPPUNIT_TEST(test_parseCheckInfo_32);
    CPPUNIT_TEST(test_parseCheckInfo_33);
    CPPUNIT_TEST(test_parseCheckInfo_34);
    CPPUNIT_TEST(test_parseCheckInfo_35);
    CPPUNIT_TEST(test_parseCheckInfo_36);
    CPPUNIT_TEST(test_parseCheckInfo_37);
    CPPUNIT_TEST(test_parseCheckInfo_38);
    CPPUNIT_TEST(test_parseCheckInfo_39);
    CPPUNIT_TEST(test_parseCheckInfo_40);
    CPPUNIT_TEST(test_basic_data_host_check2);
    CPPUNIT_TEST_SUITE_END();


public:

    void setUp();
    void tearDown();
    void test_basic_data_host_check();
    void test_operations_equal();
    void test_operations_nequal_checktype();
    void test_operations_nequal_checkport();
    void test_operations_nequal_checkinfo();
    void test_operations_nequal_dualstack();
    void test_operations_equal_checkplugin();
    void test_operations_nequal_remote_check();
    void test_print_entry();
    void test_operations_less_checktype();
    void test_operations_less_port();
    void test_operations_less_dualstack();
    void test_operations_less_checkinfo();
    void test_operations_less_remote_check();
    void test_parseCheckInfo_1();
    void test_parseCheckInfo_2();
    void test_parseCheckInfo_3();
    void test_parseCheckInfo_4();
    void test_parseCheckInfo_5();
    void test_parseCheckInfo_6();
    void test_parseCheckInfo_7();
    void test_parseCheckInfo_8();
    void test_parseCheckInfo_9();
    void test_parseCheckInfo_10();
    void test_parseCheckInfo_11();
    void test_parseCheckInfo_12();
    void test_parseCheckInfo_13();
    void test_parseCheckInfo_14();
    void test_parseCheckInfo_15();
    void test_parseCheckInfo_16();
    void test_parseCheckInfo_17();
    void test_parseCheckInfo_18();
    void test_parseCheckInfo_19();
    void test_parseCheckInfo_20();
    void test_parseCheckInfo_21();
    void test_parseCheckInfo_22();
    void test_parseCheckInfo_23();
    void test_parseCheckInfo_24();
    void test_parseCheckInfo_25();
    void test_parseCheckInfo_26();
    void test_parseCheckInfo_27();
    void test_parseCheckInfo_28();
    void test_parseCheckInfo_29();
    void test_parseCheckInfo_30();
    void test_parseCheckInfo_31();
    void test_parseCheckInfo_32();
    void test_parseCheckInfo_33();
    void test_parseCheckInfo_34();
    void test_parseCheckInfo_35();
    void test_parseCheckInfo_36();
    void test_parseCheckInfo_37();
    void test_parseCheckInfo_38();
    void test_parseCheckInfo_39();
    void test_parseCheckInfo_40();
    void test_basic_data_host_check2();
protected:

};

#endif /* TEST_HMDataHostCheck_H_ */

