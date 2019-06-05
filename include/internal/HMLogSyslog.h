// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMLOGSYSLOG_H_
#define HMLOGSYSLOG_H_

#include <string>
#include <syslog.h>

#include "HMLogBase.h"

class HMLogSyslog : public HMLogBase
{

private:
    //! Internal log open.
    /*!
         Internal log open function. Opens the log during the init.
         \param the log file name and/or location.
         \return true if the log was opened correctly.
     */
    bool openLog(std::string);

    //! Internal log close.
    /*!
         Internal log close function. Closes the log during shutdown.
     */
    void closeLog();

    //! Internal write log function.
    /*!
         Internal write log function.
         Writes the log entry in the derived classes.
         \param the log entry to write.
     */
    void writeLog(LogEntry* entry);

    //! Called when the log needs to handle a log rotation.
    void rotate();

    //! Convert the HM_LOG_LEVEL to sys log level code.
    /*!
         Convert the HM_LOG_LEVEL to sys log level code.
         \param HM_LOG_LOEVEL.
         \return sys log level.
     */
    int getSyslogLevel(HM_LOG_LEVEL level);
};

#endif /* HMLOGSYSLOG_H_ */
