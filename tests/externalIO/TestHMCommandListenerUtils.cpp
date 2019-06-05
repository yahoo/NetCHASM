// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <sys/stat.h>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include "TestHMCommandListenerUtils.h"
#include "common.h"
#include "HMConstants.h"
#include "HMStateManager.h"
#include "HMControlLinuxSocket.h"

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{}

void TESTNAME::tearDown()
{}

void dummy_function(HMControlLinuxSocket *comm){
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> lg(comm->m_handlerMutex);
    comm->m_handlerThreadsStatus[std::this_thread::get_id()] = true;
}

void dummy_function1(HMControlLinuxSocket *comm){
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::lock_guard<std::mutex> lg(comm->m_handlerMutex);
    comm->m_handlerThreadsStatus[std::this_thread::get_id()] = true;
}

void TESTNAME::test_handlerThreads()
{
    string dummy = "dunmmy";
    HMStateManager sm;
    HMControlLinuxSocket *comm_base = new HMControlLinuxSocket(dummy, sm);
    comm_base->m_handlerThreads.push_back(
            std::thread(dummy_function, comm_base));
    comm_base->m_handlerThreads.push_back(
            std::thread(dummy_function1, comm_base));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    comm_base->cleanHandlerThreads();
    CPPUNIT_ASSERT_EQUAL(2, (int )comm_base->m_handlerThreads.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    comm_base->cleanHandlerThreads();
    CPPUNIT_ASSERT_EQUAL(1, (int )comm_base->m_handlerThreads.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(550));
    comm_base->cleanHandlerThreads();
    CPPUNIT_ASSERT_EQUAL(0, (int )comm_base->m_handlerThreads.size());
    CPPUNIT_ASSERT_EQUAL(0, (int )comm_base->m_handlerThreadsStatus.size());
    delete comm_base;
}
