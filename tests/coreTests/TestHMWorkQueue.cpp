// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMWorkQueue.h"
#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMEventLoopQueue.h"
#include "HMWorkDNSLookupStatic.h"
#include "common.h"
#include <unistd.h>

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp() {
    setupCommon();

}

void TESTNAME::tearDown() {
    teardownCommon();
}

void TESTNAME::test_basic_workqueue() {
    const string hostname = "dummy.hm.com";
    const HMIPAddress ip;
    const HMDataHostCheck host_check;
    HMDNSLookup dnsHostCheckF(HM_DNS_PLUGIN_STATIC, false);
    HMWorkDNSLookupStatic dns_lookup(hostname, ip, host_check, dnsHostCheckF);
    std::unique_ptr<HMWork> work = std::make_unique<HMWorkDNSLookupStatic>(
            dns_lookup);
    HMWorkQueue work_queue;
    work_queue.insertWork(work);
    CPPUNIT_ASSERT_EQUAL(1, (int )work_queue.queueSize());
    std::unique_ptr<HMWork> work_temp;
    bool threadStatus = false;
    CPPUNIT_ASSERT_EQUAL(true, work_queue.getWork(work_temp, threadStatus));
    CPPUNIT_ASSERT_EQUAL(dns_lookup.m_hostname, work_temp->m_hostname);
}

void TESTNAME::test_workqueue() {
    HMWorkQueue work_queue;
    const string hostname1 = "dummy1.hm.com";
    const string hostname2 = "dummy2.hm.com";
    const string hostname3 = "dummy3.hm.com";
    const HMIPAddress ip;
    const HMDataHostCheck host_check;
    HMDNSLookup dnsHostCheckF(HM_DNS_PLUGIN_STATIC, false);
    HMWorkDNSLookupStatic dns_lookup1(hostname1, ip, host_check, dnsHostCheckF);
    HMWorkDNSLookupStatic dns_lookup2(hostname2, ip, host_check, dnsHostCheckF);
    HMWorkDNSLookupStatic dns_lookup3(hostname3, ip, host_check, dnsHostCheckF);
    std::unique_ptr<HMWork> work = std::make_unique<HMWorkDNSLookupStatic>(
            dns_lookup3);
    work_queue.insertWork(work);
    CPPUNIT_ASSERT_EQUAL(1, (int )work_queue.queueSize());
    work = std::make_unique<HMWorkDNSLookupStatic>(dns_lookup2);
    work_queue.insertWork(work);
    CPPUNIT_ASSERT_EQUAL(2, (int )work_queue.queueSize());
    work = std::make_unique<HMWorkDNSLookupStatic>(dns_lookup1);
    work_queue.insertWork(work);
    CPPUNIT_ASSERT_EQUAL(3, (int )work_queue.queueSize());

    std::unique_ptr<HMWork> work_temp;
    bool threadStatus = false;
    CPPUNIT_ASSERT_EQUAL(true, work_queue.getWork(work_temp, threadStatus));
    CPPUNIT_ASSERT_EQUAL(dns_lookup3.m_hostname, work_temp->m_hostname);
    CPPUNIT_ASSERT_EQUAL(true, work_queue.getWork(work_temp, threadStatus));
    CPPUNIT_ASSERT_EQUAL(dns_lookup2.m_hostname, work_temp->m_hostname);
    CPPUNIT_ASSERT_EQUAL(true, work_queue.getWork(work_temp, threadStatus));
    CPPUNIT_ASSERT_EQUAL(dns_lookup1.m_hostname, work_temp->m_hostname);
}

void TESTNAME::test_notify_workqueue() {
    const string hostname = "dummy.hm.com";
    const HMIPAddress ip;
    const HMDataHostCheck host_check;
    HMDNSLookup dnsHostCheckF(HM_DNS_PLUGIN_STATIC, false);
    HMWorkDNSLookupStatic dns_lookup(hostname, ip, host_check, dnsHostCheckF);
    std::unique_ptr<HMWork> work = std::make_unique<HMWorkDNSLookupStatic>(
            dns_lookup);
    std::unique_ptr<HMWork> work_temp;
    HMWorkQueue *work_queue = new HMWorkQueue;
    bool threadStatus = false;
    std::thread get_thread(&HMWorkQueue::getWork, work_queue, std::ref(work_temp), std::ref(threadStatus));
    work_queue->insertWork(work);
    get_thread.join();
    CPPUNIT_ASSERT_EQUAL(dns_lookup.m_hostname, work_temp->m_hostname);
    delete work_queue;
}

void TESTNAME::test_shutdown_workqueue() {
    std::unique_ptr<HMWork> work_temp;
    bool threadStatus = false;
    HMWorkQueue *work_queue = new HMWorkQueue;
    std::thread get_thread(&HMWorkQueue::getWork, work_queue,
            std::ref(work_temp), std::ref(threadStatus));
    work_queue->shutdown();
    get_thread.join();
    CPPUNIT_ASSERT(true);
    delete work_queue;
}

void TESTNAME::test_multi_insert() {

    const string hostname1 = "dummy1.hm.com";
    const string hostname2 = "dummy2.hm.com";
    const string hostname3 = "dummy3.hm.com";
    const HMIPAddress ip;
    const HMDataHostCheck host_check;
    HMDNSLookup dnsHostCheckF(HM_DNS_PLUGIN_STATIC, false);
    HMWorkDNSLookupStatic dns_lookup1(hostname1, ip, host_check, dnsHostCheckF);
    HMWorkDNSLookupStatic dns_lookup2(hostname2, ip, host_check, dnsHostCheckF);
    HMWorkDNSLookupStatic dns_lookup3(hostname3, ip, host_check, dnsHostCheckF);
    std::unique_ptr<HMWork> work1 = std::make_unique<HMWorkDNSLookupStatic>(
            dns_lookup1);
    std::unique_ptr<HMWork> work2 = std::make_unique<HMWorkDNSLookupStatic>(
            dns_lookup2);
    std::unique_ptr<HMWork> work3 = std::make_unique<HMWorkDNSLookupStatic>(
            dns_lookup3);
    HMWorkQueue *work_queue = new HMWorkQueue;
    std::thread get_thread1(&HMWorkQueue::insertWork, work_queue,
            std::ref(work1));
    std::thread get_thread2(&HMWorkQueue::insertWork, work_queue,
            std::ref(work2));
    std::thread get_thread3(&HMWorkQueue::insertWork, work_queue,
            std::ref(work3));
    get_thread1.join();
    get_thread2.join();
    get_thread3.join();
    CPPUNIT_ASSERT_EQUAL(3, (int )work_queue->queueSize());
    delete work_queue;
}
