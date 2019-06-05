// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <iostream>
#include "HMLogAPI.h"
#include "HMConstants.h"

using namespace std;

void
HMLogAPI::writeLog(LogEntry *entry)
{
    if (entry->length > 0 && entry->level <= m_logLevel)
    {
        lock_guard<mutex> lg(m_mutex);
        m_lastLogMessages.push_back(string(entry->entry));
    }
}
