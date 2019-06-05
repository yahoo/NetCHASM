// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMHASH_H_
#define TEST_HMHASH_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMState.h"
#include "HMConstants.h"
#include "HMHashMD5.h"

#define TESTNAME Test_HMHASH

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(TestHash_match);
    CPPUNIT_TEST(TestHash_mismatch);    
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void TestHash_match();
    void TestHash_mismatch();    
};

#endif // TEST_HMHASH_H_
 
