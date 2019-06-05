// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "HMLogText.h"

using namespace std;

HMLogTextFileHandle::~HMLogTextFileHandle()
{
    if(m_fileHandle)
    {
        close(m_fileHandle);
        m_fileHandle = 0;
    }
}

void
HMLogTextFileHandle::flush(const char* buf, int bufSize)
{
    if(m_fileHandle > 0)
    {
        write(m_fileHandle, buf, bufSize);
    }
    else
    {
        printf(buf);
    }
}

bool
HMLogTextFileHandle::init(string& logFile)
{
    int oFlags = O_WRONLY|O_CREAT|O_APPEND;
    m_fileHandle = open(logFile.c_str(), oFlags,0644);
    if(m_fileHandle >= 0)
    {
        return true;
    }
    m_fileHandle = 0;
    return false;
}

void
HMLogText::rotate()
{
    auto newFile = make_shared<HMLogTextFileHandle>();
    newFile->init(m_logFile);
    m_file = newFile;
}

bool
HMLogText::openLog(string logFile)
{
    m_logFile = logFile;
    m_file = make_shared<HMLogTextFileHandle>();

    if(!m_file->init(m_logFile))
    {
        m_lastLogMessages.push_back("error opening " + m_logFile + " for logging! " + string(strerror(errno)));
        return false;
    }
    return true;
}

void
HMLogText::closeLog()
{
    m_file.reset();
}

void
HMLogText::writeLog(LogEntry *entry)
{
    shared_ptr<HMLogTextFileHandle> file = m_file;
    char buf[DATE_LENGTH + MAX_LOG_PRINT + DEFAULT_MAX_BUFFER] = {};
    char timeBuf[DATE_LENGTH] = {};
    struct tm* timeinfo;
    int bufsize;

    if(entry->length > 0 && entry->level <= m_logLevel )
    {
        // format the date string
        int32_t msecs = static_cast<int>(entry->tv.tv_usec / 1000);
        timeinfo = localtime(&(entry->tv.tv_sec));
        strftime (timeBuf, DATE_LENGTH, m_dateString.c_str(), timeinfo);

        // deal with long log lines in a slower manner
        if(entry->length > DEFAULT_MAX_BUFFER)
        {
            char* tbuf;
            bufsize = asprintf(&tbuf, "%s.%d [%d] %s %s\n",
                timeBuf,
                msecs,
                entry->tid,
                PRINT_LOG_LEVEL[entry->level],
                entry->entry);
            file->flush(buf, bufsize);
            free(tbuf);
        }
        else
        {
            bufsize = snprintf(buf, DATE_LENGTH + MAX_LOG_PRINT + DEFAULT_MAX_BUFFER +3,
                "%s.%d [%d] %s %s\n",
                timeBuf,
                msecs,
                entry->tid,
                PRINT_LOG_LEVEL[entry->level],
                entry->entry);
            file->flush(buf, bufsize);
        }
    }
}

