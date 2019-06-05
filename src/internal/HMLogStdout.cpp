// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "HMLogStdout.h"

using namespace std;

bool
HMLogStdout::openLog(string logFile)
{
    (void)logFile;
    return true;
}

void
HMLogStdout::closeLog() { }

void
HMLogStdout::writeLog(LogEntry *entry)
{
    char buf[DATE_LENGTH + MAX_LOG_PRINT + DEFAULT_MAX_BUFFER] = {};
    int bufsize;

    if (entry->length > 0 && entry->level <= m_logLevel)
    {
        // deal with long log lines in a slower manner
        if (entry->length > DEFAULT_MAX_BUFFER)
        {
            char* tbuf;
            bufsize = asprintf(&tbuf, "%s %s\n", PRINT_LOG_LEVEL[entry->level], entry->entry);
            write(fileno(stdout), tbuf, bufsize);
            free(tbuf);
        }
        else
        {
            bufsize = snprintf(buf,
            DATE_LENGTH + MAX_LOG_PRINT + DEFAULT_MAX_BUFFER + 3, "%s %s\n", PRINT_LOG_LEVEL[entry->level], entry->entry);
            write(fileno(stdout), buf, bufsize);
        }
    }
}

void
HMLogStdout::rotate() {}

