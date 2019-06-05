// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <iostream>
#include <openssl/crypto.h>

#include "HMStateManager.h"
#include "HMEventLoopQueue.h"
#include "HMStorageHostGroupMDBM.h"
#include "HMStorageHostText.h"
#include "HMLogText.h"
#include "HMLogStdout.h"
#include "HMLogSyslog.h"
#include "HMThreadPool.h"
#include "HMConfigParserBase.h"
#include "HMConfigParserYaml.h"
#include "HMLogBase.h"
#include "HMControlLinuxSocket.h"

using namespace std;

static HMStateManager* monitor;
//LCOV_EXCL_START
void ctrlC_Callback(int s)
{
    (void)(s);
    // Signal the main process to quit
    monitor->shutdown();
}
//LCOV_EXCL_STOP

HMStateManager::HMStateManager() :
          m_keepRunning(true),
          m_eventLoop(nullptr),
          m_threadPool(nullptr),
          m_libEvent(nullptr)
{
    hlog = nullptr;
}


HMStateManager::~HMStateManager()
{
    if(hlog != nullptr)
    {
        hlog->shutDownLogging();
        delete hlog;
        hlog = nullptr;
    }

    delete (m_threadPool);

    if(m_eventLoop == m_libEvent)
    {
        delete m_libEvent;
        m_eventLoop = nullptr;
        m_libEvent = nullptr;
    }
    if(m_eventLoop)
    {
        delete m_eventLoop;
    }
    if(m_libEvent)
    {
        delete m_libEvent;
    }

}

bool
HMStateManager::updateState(shared_ptr<HMState>& current)
{
    if(current == m_currentState)
    {
        return false;
    }
    current = m_currentState;
    return true;
}

// The main Daemon HealthCheck Function
bool
HMStateManager::healthCheck(string masterConfig, HM_LOG_LEVEL logLevel)
{
    // Setup the exit conditions from Ctrl-c
    monitor = this;
    FIPS_mode_set(0);
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = ctrlC_Callback;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    struct sigaction sigTermHandler;
    sigTermHandler.sa_handler = ctrlC_Callback;
    sigemptyset(&sigTermHandler.sa_mask);
    sigTermHandler.sa_flags = 0;
    sigaction(SIGTERM, &sigTermHandler, NULL);

    signal(SIGPIPE, SIG_IGN);
    thread threadMonitor;
    // Main function to do all the health checking

    // Step 1. Load the Master Config that maintains how YHealthCheck runs
    if(!loadDaemonState(masterConfig, logLevel))
    {
        return false;
    }

    // Step 2. Initialize the event library
    if(m_currentState->getDefaultHTTPCheckype() == HM_CHECK_PLUGIN_HTTP_LIBEVENT
            || m_currentState->getDefaultDNSLookupType() == HM_DNS_PLUGIN_LIBEVENT
            || m_currentState->getDefaultEventType() == HM_EVENT_LIBEVENT)
    {
        m_libEvent = new HMEventLoopLibEvent(this);
    }

    if(m_currentState->getDefaultEventType() == HM_EVENT_LIBEVENT)
    {
        m_eventLoop = m_libEvent;
    }
    else
    {
        m_eventLoop = new HMEventLoopQueue(this);
    }

    // Step 3. Fill the initial work order Queue
    // Note #1. We always insert into the DNS callback since the checklist is by definition unique for each host/checktype
    // Note #2. This is only called at the beginning when we have no health check info saved. If we load cached DNS, this needs changed to handle existing DNS entries.
    shared_ptr<HMState> current;
    updateState(current);
    current->m_dnsCache.queueDNSLookups(m_workQueue, *m_eventLoop, true);
    const string socketPath = current->getSocketPath();
    current.reset();
    // Step 4: Create a socket to listen for external commands
    m_listener = unique_ptr<HMControlLinuxSocket>(new HMControlLinuxSocket(socketPath, *this));


    m_listener->init();


    // Step 5. Start the worker threads
    HMLog(HM_LOG_INFO, "[CORE] Starting Worker Threads");
    m_threadPool = new HMThreadPool(this, m_eventLoop);
    threadMonitor = thread(&HMThreadPool::monitorThreads, m_threadPool);

    // Step 6: Listen for commands on the socket
    m_listener->run();

    // Step 7. Start the Health Tracker (Run this in the primary thread)
    if(m_eventLoop != m_libEvent && m_libEvent)
    {
        m_libEvent->runThread();
    }

    m_eventLoop->runThread();

    // block until shutdown
    std::unique_lock<std::mutex> lk(m_shutdownMutex, std::defer_lock);
    while(m_keepRunning)
    {
        lk.lock();
        m_shutdownCond.wait(lk, [this](){return !m_keepRunning;});
        lk.unlock();
    }

    // Begin shutdown code
    m_eventLoop->shutDown();

    if(m_eventLoop != m_libEvent && m_libEvent)
    {
        m_libEvent->shutDown();
    }

    // Begin shutdown code
    // The tracker is now shutdown
    // Shutdown the threads
    HMLog(HM_LOG_NOTICE, "[CORE] Shutting Down");
    updateState(current);
    current->closeBackend();
    m_threadPool->shutdown();

    if(hlog != nullptr)
    {
        hlog->shutDownLogging();
        delete hlog;
        hlog = nullptr;
    }
    m_listener->shutdown();
    threadMonitor.join();
    return true;
}

void
HMStateManager::shutdown()
{
    std::lock_guard<mutex> lock(m_shutdownMutex);
    m_keepRunning = false;
    m_shutdownCond.notify_one();
}

bool
HMStateManager::loadDaemonState(const string& masterConfig, HM_LOG_LEVEL logLevel)
{
    lock_guard<mutex> lg(m_reloadMutex);

    // save a copy of the current backend storage
    string currentStoragePath;
    HM_LOG_PLUGIN_CLASS lastLogType;
    string lastLogPath;
    HM_LOG_LEVEL lastLogLevel;

    if(m_currentState)
    {
        currentStoragePath = m_currentState->getStoragePath();
        lastLogType = m_currentState->getDefaultLogType();
        lastLogPath = m_currentState->getLogPath();
        lastLogLevel = m_currentState->getLogLevel();
    }
    else
    {
        // This is a new load. Setup the syslog logger
        HMLogBase* newLog = new HMLogSyslog();
        if(newLog->initLogging(logLevel, false))
        {
            newLog->setAsDefaultLogger();
        }
        else
        {
            delete newLog;
            newLog = new HMLogStdout();
            if(newLog->initLogging(logLevel, false))
            {
                newLog->setAsDefaultLogger();
            }
            else
            {
                cout << "Critical Log failure: Could not open any loggers" << endl;
                delete newLog;
                return false;
            }
        }
    }

    // Parse the new config into a check state data structure
    m_newState = make_shared<HMState>();

    if(!m_newState->parseMasterConfig(masterConfig))
    {
        HMLog(HM_LOG_CRITICAL, "Failure loading master config: %s", masterConfig.c_str());
        m_newState.reset();
        return false;
    }
    m_newState->setMasterConfig(masterConfig);
    startLogging(lastLogType, lastLogPath, lastLogLevel);

    // deal with the backend setup
    if(!m_newState->openBackend(false))
    {
        HMLog(HM_LOG_CRITICAL, "Failure Opening the backend data store");
        m_newState.reset();
        return false;
    }

    if(!m_newState->setupDaemonstate())
    {
        HMLog(HM_LOG_CRITICAL, "Failure Parsing Configs");
        m_newState->m_datastore->closeStore();
        m_newState.reset();
        return false;
    }
    m_newState->m_datastore->storeConfigs(*m_newState);
    m_currentState.swap(m_newState);
    m_newState.reset();

    m_currentState->restoreStoredCheckState();
    return true;
}

bool
HMStateManager::reloadDaemonConfigs()
{
    return reloadDaemonConfigs(m_currentState->getMasterConfig());
}

bool
HMStateManager::reloadDaemonConfigs(const string& masterConfig)
{
    lock_guard<mutex> lg(m_reloadMutex);

    set<string> mHostGroup;
    set<string> mHostGroupIgn;

    // save a copy of the current backend db
    string currentStoragePath;
    HM_LOG_PLUGIN_CLASS lastLogType;
    string lastLogPath;
    HM_LOG_LEVEL lastLogLevel;
    string sockPath;

    if(m_currentState)
    {
        currentStoragePath = m_currentState->getStoragePath();
        lastLogType = m_currentState->getDefaultLogType();
        lastLogPath = m_currentState->getLogPath();
        lastLogLevel = m_currentState->getLogLevel();
        sockPath = m_currentState->getSocketPath();
    }

    // Parse the new config into a check state data structure
    m_newState = make_shared<HMState>();


    if(!m_newState->parseMasterConfig(masterConfig))
    {
        printf("Failure loading master config");
        m_newState.reset();
        return false;
    }

    m_newState->setMasterConfig(masterConfig);
    startLogging(lastLogType, lastLogPath, lastLogLevel);

    // TODO this seems fixable.....
    if(sockPath.compare(m_newState->getSocketPath()))
    {
        m_newState->setSocketPath(sockPath);
        HMLog(HM_LOG_WARNING, "Socket path cannot be changed");
    }

    // deal with the backend setup
    if(!m_newState->openBackend(false))
    {
        HMLog(HM_LOG_CRITICAL, "Failure Opening the backend data store");
        m_newState.reset();
        return false;
    }

    if(!m_newState->setupDaemonstate())
    {
        HMLog(HM_LOG_CRITICAL, "Failure Parsing Configs");
        m_newState->m_datastore->closeStore();
        m_newState.reset();
        return false;
    }
    m_newState->restoreRunningCheckState(m_currentState);

    m_currentState.swap(m_newState);
    m_currentState->m_datastore->storeConfigs(*m_currentState);
    m_currentState->resheduleDNSChecks(m_newState, m_workQueue);
    m_currentState->resheduleHealthChecks(m_newState, m_workQueue);
    m_currentState->m_dnsCache.queueDNSLookups(m_workQueue, *m_eventLoop, false);
    m_currentState->updateBackend(m_newState);
    // Kick State and Wait for us to have the last handle to the old state
    m_eventLoop->wakeupTracker();
    m_workQueue.cycleThreads();
    while(m_newState.use_count() > 1) {}

    // Load any results that were in flight
    m_currentState->restoreRunningCheckState(m_newState);

    // Now we destroy the state
    m_newState.reset();
    return true;
}

bool
HMStateManager::startLogging(uint32_t lastLogClass, const string& lastLogPath, HM_LOG_LEVEL lastLogLevel)
{
    // Do we need a new logger?
    if(hlog == nullptr
            || lastLogClass != m_newState->getDefaultLogType()
            || lastLogPath != m_newState->getLogPath())
    {
        HMLogBase* oldLog = hlog;
        HMLogBase* newLog;
        switch(m_newState->getDefaultLogType())
        {
        case HM_LOG_PLUGIN_DEFAULT:
        case HM_LOG_PLUGIN_TEXT:
            newLog = new HMLogText();
            break;
        case HM_LOG_PLUGIN_STDOUT:
            newLog = new HMLogStdout();
            break;
        case HM_LOG_PLUGIN_SYSLOG:
            newLog = new HMLogSyslog();
            break;
        }

        if(!newLog->initLogging(m_newState->getLogPath(), m_newState->getLogLevel(), true))
        {
            // If this fails, then we want to try syslog
            string fail1 = newLog->getLastError();

            delete newLog;
            newLog = new HMLogSyslog();

            if(newLog->initLogging( m_newState->getLogLevel(), true))
            {
                newLog->log(HM_LOG_ERROR, "Failed to open backend logger: %s", fail1.c_str());
            }
            else
            {
                // Syslog failed too
                string fail2 = newLog->getLastError();
                delete newLog;

                newLog = new HMLogStdout();
                if(newLog->initLogging( m_newState->getLogLevel(), true))
                {
                    // Ok the stdout worked
                    newLog->log(HM_LOG_ERROR, "Failed to open backend logger: %s", fail1.c_str());
                    newLog->log(HM_LOG_ERROR, "Failed to open syslog logger: %s", fail2.c_str());
                }
                else
                {
                    // Something when seriously wrong. If we have an old logger we can stay with that
                    if(oldLog)
                    {
                        newLog->log(HM_LOG_ERROR, "Failed to open backend logger: %s", fail1.c_str());
                        newLog->log(HM_LOG_ERROR, "Failed to open syslog logger: %s", fail2.c_str());
                        newLog->log(HM_LOG_ERROR, "Failed to open stdout logger: %s", newLog->getLastError().c_str());
                        delete newLog;
                        newLog = oldLog;
                        oldLog = nullptr;
                    }
                    else
                    {
                        delete newLog;
                        return false;
                    }
                }
            }
        }

        if(!m_newState->getLogFormat().empty())
        {
            string logFormat = m_newState->getLogFormat();
            newLog->setDateString(logFormat);
        }

        if(m_newState->getMaxLogQueueLength() > 0)
        {
            newLog->setMaxLogQueue(m_newState->getMaxLogQueueLength());
        }

        newLog->setAsDefaultLogger();
    }
    else if(lastLogLevel != m_newState->getLogLevel())
    {
        hlog->setLevel(m_newState->getLogLevel());
    }
    return true;
}

uint64_t
HMStateManager::getIdleThreads()
{
    return m_threadPool->countIdle();
}

uint64_t
HMStateManager::getEventQueueSize()
{
    return m_eventLoop->getTimeOutQueueSize();
}

HM_LOG_LEVEL
HMStateManager::getLogLevel()
{
    if(hlog != NULL)
    {
        return hlog->getLevel();
    }
    return HM_ERROR;
}

uint32_t
HMStateManager::getNThreads() const
{
    return m_threadPool->getNThreads();
}

uint32_t
HMStateManager::getMonitorFrequency() const
{
    return m_threadPool->getMonitorFrequency();
}

uint32_t
HMStateManager::getStridePercent() const
{
    return m_threadPool->getStridePercent();
}

uint32_t
HMStateManager::getWorkPerThreadRatio() const
{
    return m_threadPool->getWorkPerThreadRatio();
}

bool
HMStateManager::isRecycle() const
{
    return m_threadPool->isRecycle();
}

void
HMStateManager::setLogLevel(HM_LOG_LEVEL level)
{
    if(hlog != NULL)
    {
        hlog->setLevel(level);
    }
}

void
HMStateManager::setMonitorFrequency(uint32_t monitorFrequency)
{
    m_threadPool->setMonitorFrequency(monitorFrequency);
}

void
HMStateManager::setStridePercent(uint32_t stridePercent)
{
    m_threadPool->setStridePercent(stridePercent);
}

void
HMStateManager::setWorkPerThreadRatio(uint32_t workPerThreadRatio)
{
    m_threadPool->setWorkPerThreadRatio(workPerThreadRatio);
}

void
HMStateManager::setRecycle(bool recycle)
{
    m_threadPool->setRecycle(recycle);
}

void
HMStateManager::setState(shared_ptr<HMState> debugState)
{
    m_currentState = debugState;
}

