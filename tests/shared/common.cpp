// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "common.h"

using namespace std;

void setupCommon()
{
    string logPath = "HM.log";
    HM_LOG_LEVEL logLevel = HM_LOG_DEBUG3;
    hlog = new HMLogStdout();
    hlog->initLogging(logPath, logLevel, true);
}

void teardownCommon()
{
    hlog->shutDownLogging();
    delete hlog;
    hlog = nullptr;
}


