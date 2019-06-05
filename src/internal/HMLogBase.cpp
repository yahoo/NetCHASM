// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <algorithm>

#include "HMLogBase.h"

using namespace std;

HMLogBase* hlog = nullptr;

static void usr1Handler(int s)
{
    (void)s;
    hlog->rotate();
}

bool
HMLogBase::initLogging(string logfile, HM_LOG_LEVEL level, bool threaded)
{
    m_logFile = logfile;
    return initLogging(level,threaded);
}

bool
HMLogBase::initLogging(HM_LOG_LEVEL level, bool threaded)
{
    struct sigaction sigUsr1Handler;
    sigUsr1Handler.sa_handler = usr1Handler;
    sigemptyset(&sigUsr1Handler.sa_mask);
    sigUsr1Handler.sa_flags = 0;
    sigaction(SIGUSR1, &sigUsr1Handler, NULL);
    m_threaded = threaded;
    m_logLevel = level;

    m_dateString = DEFAULT_DATE_STRING;
    m_maxQueue = DEFAULT_MAX_QUEUE;

    if(!openLog(m_logFile))
    {
        return false;
    }

    LogEntry* entry;
    entry = new LogEntry;
    entry->entry = (char *)malloc(15);
    if(entry->entry == nullptr)
    {
        //LCOV_EXCL_START
        entry->entry = nullptr;
        entry->length = 0;
        entry->level = HM_LOG_NONE;
        //LCOV_EXCL_STOP
    }
    else
    {
        sprintf(entry->entry, "Begin Logging");
        entry->length = 14;
        entry->level = HM_LOG_NOTICE;
    }
    // Because of a glibc bug we have to use syscall
    entry->tid = syscall(SYS_gettid,0);
    gettimeofday(&entry->tv, NULL);
    entry->ready = true;
    m_droppedLogs = 0;
    if (m_threaded)
    {
        // init the log linked list
        m_writeHead = entry;
        m_readHead = m_writeHead;
        m_keepRunning = true;

        // start the logging thread
        m_thread = thread(&HMLogBase::flushBuffer, this);
        m_current = 0;
        m_last = 0;
    }
    else
    {
        writeLog(entry);
        free(entry->entry);
        delete entry;
    }
    return true;
}

void
HMLogBase::setDateString(string& dateString)
{
    m_dateString = dateString;
}

void
HMLogBase::setMaxLogQueue(uint32_t max)
{
    m_maxQueue = max;
}

void
HMLogBase::shutDownLogging()
{
    if (m_threaded)
    {
        unique_lock<mutex> lock(m_dataReadyLock);
        m_keepRunning = false;
        m_dataReadyCond.notify_one();
        lock.unlock();
        m_thread.join();
    }
    closeLog();
}


void
HMLogBase::log(HM_LOG_LEVEL level, const char* buf, ...)
{
    LogEntry* entry;
    int ret;

    if(level > m_logLevel)
    {
        return;
    }

    if(m_threaded)
    {
        // Add an entry to the linked list
        getEntry(entry);
    }
    else
    {
        entry = new LogEntry;
    }
    // If this is null, then we are too big give up
    if(entry == nullptr)
    {
        m_droppedLogs++;
        return;
    }

    va_list args;
    va_start(args,buf);

    // check to see if we were dropping lines
    if(m_droppedLogs.load() > 0)
    {
        ret = asprintf(&(entry->entry), "Max log queue length exceeded %d logs dropped", m_droppedLogs.load() + 1);
        m_droppedLogs = 0;
        entry->length = ret;
        entry->level = HM_LOG_WARNING;
    }
    // Format the log line string
    else if((ret = vasprintf(&(entry->entry), buf, args)) == -1)
    {
        //LCOV_EXCL_START
        entry->entry = nullptr;
        entry->length = 0;
        entry->level = HM_LOG_NONE;
        //LCOV_EXCL_STOP
    }
    else
    {
        entry->length = ret;
        entry->level = level;
    }
    va_end(args);
    // Because of a glibc bug we have to use syscall
    entry->tid = syscall(SYS_gettid,0);
    gettimeofday(&entry->tv, NULL);
    entry->ready = true;

    if(m_threaded)
    {
        // Signal the read thread that new data is ready
        lock_guard<mutex> lk(m_dataReadyLock);
        m_dataReadyCond.notify_one();
    }
    else
    {
        writeLog(entry);
        free(entry->entry);
        delete entry;
    }
}

HM_LOG_LEVEL
HMLogBase::getLevel()
{
    return m_logLevel;
}

void
HMLogBase::setLevel(HM_LOG_LEVEL level)
{
    m_logLevel = level;
}

HM_LOG_LEVEL
HMLogBase::parseLogLevel(string& val)
{
    transform(val.begin(), val.end(), val.begin(), ::tolower);

    if(val == "emerg" || val == "emergency")
    {
        return HM_LOG_EMERGENCY;
    }
    else if(val == "alert")
    {
        return HM_LOG_ALERT;
    }
    else if(val == "crit" || val == "critical")
    {
        return HM_LOG_CRITICAL;
    }
    else if(val == "error")
    {
        return HM_LOG_ERROR;
    }
    else if(val == "warn" || val == "warning")
    {
        return HM_LOG_WARNING;
    }
    else if(val == "notice")
    {
        return HM_LOG_NOTICE;
    }
    else if(val == "info")
    {
        return HM_LOG_INFO;
    }
    else if(val == "debug")
    {
        return HM_LOG_DEBUG;
    }
    else if(val == "debug2")
    {
        return HM_LOG_DEBUG2;
    }
    else if(val == "debug3")
    {
        return HM_LOG_DEBUG3;
    }
    else if(val == "off")
    {
        return HM_LOG_OFF;
    }
    else if(val == "none")
    {
        return HM_LOG_NONE;
    }

    // Maintain support for the numeric values as well
    try
    {
        int value = stoi(val);
        if(value < -1 || value >= HM_LOG_MAXLEVEL)
        {
            return HM_ERROR;
        }
        return HM_LOG_LEVEL(value);
    }
    catch(...)
    {
        return HM_ERROR;
    }
}

string
HMLogBase::getLastError()
{
    lock_guard<mutex> lg(m_mutex);
    string m_lastError;
    for(auto it = m_lastLogMessages.begin(); it != m_lastLogMessages.end(); ++it)
    {
        m_lastError += *it;
    }
    return m_lastError;
}

void
HMLogBase::clearError()
{
    lock_guard<mutex> lg(m_mutex);
    m_lastLogMessages.clear();
}

void
HMLogBase::setAsDefaultLogger()
{
    if(hlog != nullptr)
    {
        hlog->shutDownLogging();
        delete hlog;
    }
    hlog = this;
}

void
HMLogBase::unsetAsDefaultLogger()
{
    hlog = nullptr;
}

void
HMLogBase::getEntry(struct LogEntry*& entry)
{
    // Spin lock since logging should grab a new entry quickly
    // Note we can do some pre-allocation if this becomes very slow
    HMLockGuard<HMUtilitySpinLock> lg(m_spinLock);

    if(m_last > m_current && (m_last - m_current) > m_maxQueue)
    {
        entry = nullptr;
        return;
    }

    m_writeHead->next = new LogEntry;
    entry = m_writeHead->next;
    m_writeHead = m_writeHead->next;
    m_writeHead->ready = false;
    m_writeHead->next = nullptr;
    m_last++;
}


void
HMLogBase::flushBuffer()
{
    unique_lock<mutex> lk(m_dataReadyLock, defer_lock);
    LogEntry* entry;

    while(m_keepRunning || m_readHead != m_writeHead)
    {
        while(m_readHead != m_writeHead)
        {
            entry = m_readHead;

            // Wait for the write to be complete if we are caught up
            if(!entry->ready)
            {
                continue;//LCOV_EXCL_LINE
            }

            m_current++;

            writeLog(entry);

            m_readHead = m_readHead->next;
            free(entry->entry);
            delete entry;
        }


        // wait for new data
        lk.lock();
        m_dataReadyCond.wait(lk, [this](){return m_readHead != m_writeHead || !m_keepRunning;});
        lk.unlock();
    }

    if(m_readHead != nullptr)
    {
        // write the last entry
        entry = m_readHead;

        // Wait for the write to be complete if we are caught up
        if(entry->ready)
        {
            m_current++;
            writeLog(entry);
        }
        free(entry->entry);
        delete entry;
    }
}
