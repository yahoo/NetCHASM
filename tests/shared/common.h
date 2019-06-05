// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMLogStdout.h"

extern HMLogBase* ylog;

void setupCommon();
void teardownCommon();

#endif // TEST_COMMON_H_
