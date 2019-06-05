// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMCONFIGPARSERBASE_H_
#define TEST_HMCONFIGPARSERBASE_H_

#include "HMStateManager.h"
#include "HMConfigParserBase.h"

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#define TESTNAME TestHMConfigParserBase

class TestConfigParser : public HMConfigParserBase
{
public:
    ~TestConfigParser() {}
    uint32_t parseConfig(const std::string& fileName, HMState& state) { return 0; }
    bool writeConfigs(HMState& state, std::string outFile)
    {
        return true;
    }

};

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_parseDirectory);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_parseDirectory();

    HMStateManager* parse;
};

#endif // TEST_HMCONFIGPARSERBASE_H_
