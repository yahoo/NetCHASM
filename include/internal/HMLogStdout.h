// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMLOGSTDOUT_H_
#define HMLOGSTDOUT_H_

#include <mutex>

#include "HMLogBase.h"



class HMLogStdout : public HMLogBase
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
    void writeLog(LogEntry *entry);

    //! Called when the log needs to handle a log rotation.
    void rotate();
};

#endif /* HMLOGSTDOUT_H_ */
