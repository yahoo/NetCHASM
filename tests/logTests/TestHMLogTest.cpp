// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMLogTest.h"

#include "HMLogBase.h"
#include "HMLogText.h"
#include "HMLogSyslog.h"
#include <unistd.h>
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

int vsyslogInvoked = 0;

void TESTNAME::setUp()
{
    hlog = nullptr;
    vsyslogInvoked = 0;
}

void TESTNAME::tearDown()
{

}

void TESTNAME::test_logbase()
{
   std::shared_ptr<HMLogBase> log = std::make_shared<HMLogText>();

    std::string logfile = "testfile";

    log->initLogging(logfile, HM_LOG_NONE, false);
    setAsDefaultLogger(log);
    log->log(HM_LOG_DEBUG2, "sampletest");

    const char* buf = "testing";

    HMLog(HM_LOG_DEBUG2, buf);

    log->setLevel(HM_LOG_DEBUG2);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_DEBUG2, log->getLevel());

    log->setLevel(HM_LOG_EMERGENCY);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_EMERGENCY, log->getLevel());

    log->setLevel(HM_LOG_CRITICAL);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_CRITICAL, log->getLevel());

    log->setLevel(HM_LOG_ERROR);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_ERROR, log->getLevel());

    log->setLevel(HM_LOG_WARNING);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_WARNING, log->getLevel());

    log->setLevel(HM_LOG_NOTICE);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_NOTICE, log->getLevel());

    log->setLevel(HM_LOG_INFO);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_INFO, log->getLevel());

    log->setLevel(HM_LOG_DEBUG);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_DEBUG, log->getLevel());

    log->setLevel(HM_LOG_DEBUG3);
    CPPUNIT_ASSERT_EQUAL(HM_LOG_DEBUG3, log->getLevel());

    log->shutDownLogging();

}

void TESTNAME::test_syslog_level()
{
  std::shared_ptr<HMLogBase> hsyslog = std::make_shared<HMLogSyslog>();
  hsyslog->initLogging(HM_LOG_WARNING, false);
  setAsDefaultLogger(hsyslog);
  HM_LOG_LEVEL eLevel = hlog->getLevel();
  CPPUNIT_ASSERT_EQUAL(HM_LOG_WARNING, eLevel);

  hlog->setLevel(HM_LOG_DEBUG2);
  eLevel = hlog->getLevel();
  CPPUNIT_ASSERT_EQUAL(HM_LOG_DEBUG2, eLevel);
}

void TESTNAME::test_syslog_called()
{
  std::shared_ptr<HMLogBase> hsyslog = std::make_shared<HMLogSyslog>();
  hsyslog->initLogging(HM_LOG_WARNING, false);
  setAsDefaultLogger(hsyslog);
  hlog->log(HM_LOG_ERROR, "This is a test");
  CPPUNIT_ASSERT_EQUAL(1, vsyslogInvoked);
}

void TESTNAME::test_syslog_notcalled()
{
  std::shared_ptr<HMLogBase> hsyslog = std::make_shared<HMLogSyslog>();
  hsyslog->initLogging(HM_LOG_WARNING, false);
  setAsDefaultLogger(hsyslog);
  hlog->log(HM_LOG_DEBUG3, "This is a test");
  CPPUNIT_ASSERT_EQUAL(0, vsyslogInvoked);
}

void syslog(int priority, const char *format, ...)
{
  // No point in calling the actual vsyslog, whether the
  // message will be logged or not depends on syslog.conf
  // settings
  vsyslogInvoked = 1;
}
