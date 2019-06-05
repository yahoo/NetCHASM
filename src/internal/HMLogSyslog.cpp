// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "HMLogSyslog.h"
#include "HMConstants.h"

using namespace std;

bool
HMLogSyslog::openLog(string logFile)
{
    (void)logFile;
    openlog(NULL, LOG_PID, LOG_USER);
    return true;
}

void
HMLogSyslog::closeLog()
{
    closelog();
}

void
HMLogSyslog::writeLog(LogEntry* entry)
{
    char buf[DATE_LENGTH + MAX_LOG_PRINT + DEFAULT_MAX_BUFFER] = { };

    if (entry->length > 0 && entry->level <= m_logLevel)
    {
        int sysLogLevel = getSyslogLevel(entry->level);
        // deal with long log lines in a slower manner
        if (entry->length > DEFAULT_MAX_BUFFER)
        {
            char* tbuf;
            asprintf(&tbuf, "%s %s\n", PRINT_LOG_LEVEL[entry->level], entry->entry);
            syslog(sysLogLevel, "%s", tbuf);
            free(tbuf);
        }
        else
        {
            snprintf(buf, DATE_LENGTH + MAX_LOG_PRINT + DEFAULT_MAX_BUFFER + 3, "%s %s\n", PRINT_LOG_LEVEL[entry->level], entry->entry);
            syslog(sysLogLevel, "%s", buf);
        }
    }
}

void
HMLogSyslog::rotate()
{

}

int
HMLogSyslog::getSyslogLevel(HM_LOG_LEVEL level)
{
    switch(level)
    {
    case HM_LOG_EMERGENCY:
        return LOG_EMERG;

    case HM_LOG_ALERT:
        return LOG_ALERT;

    case HM_LOG_CRITICAL:
        return LOG_CRIT;

    case HM_LOG_ERROR:
        return LOG_ERR;

    case HM_LOG_WARNING:
        return LOG_WARNING;

    case HM_LOG_NOTICE:
        return LOG_NOTICE;

    case HM_LOG_INFO:
        return LOG_INFO;

    case HM_LOG_DEBUG:
    case HM_LOG_DEBUG2:
    case HM_LOG_DEBUG3:
        return LOG_DEBUG;

        // Anything outside the range
    default:
        return LOG_INFO;
    }
}
