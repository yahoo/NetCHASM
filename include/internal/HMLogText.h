// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMLOGTEXT_H_
#define HMLOGTEXT_H_

#include <mutex>

#include "HMLogBase.h"

//! Class to hold a file handle. Will auto close when the smart pointer goes out of scope.
class HMLogTextFileHandle
{
public:
    HMLogTextFileHandle():
        m_fileHandle(0) {}

    ~HMLogTextFileHandle();

    //! Write the log line to the file.
    /*!
         Write the log line to the file. If the log did not open correctly, write to stdout.
         \param the bugger to write.
         \param the size of the buffer.
     */
    void flush(const char* buf, int bufSize);

    //! Open the log file for writing.
    /*!
         Open the log file for writing.
         \param the path to the log file.
         \return true if the log file is ready for writing.
     */
    bool init(std::string& logFile);

private:
    int m_fileHandle;
};

//! The Log class that write the log to a text file.
class HMLogText : public HMLogBase
{
public:
    HMLogText() {};

    //! Called when the log needs to handle a log rotation.
    /*!
         File rotation is handled by having a current file handle and a new file handle.
         When a rotate occurs, a the new file handle is created. Any writes using the old file handle will update on the next write cycle.
     */
    void rotate();

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

    std::string m_logFile;
    std::shared_ptr<HMLogTextFileHandle> m_file;
};

#endif /* HMLOGTEXT_H_ */
