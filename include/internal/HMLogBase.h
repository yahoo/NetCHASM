// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMLOGBASE_H_
#define HMLOGBASE_H_

#include <string>
#include <syslog.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <sys/types.h>
#include <condition_variable>

#include "HMUtilitySpinLock.h"
#include "HMTimeStamp.h"

//! The length of the date buffer.
#define DATE_LENGTH 18
//! The default max buffer for a log line.
#define DEFAULT_MAX_BUFFER 2056
//! The default date string to use in the logging.
#define DEFAULT_DATE_STRING "%Y%m%d.%Hh%Mm%Ss"
//! The default max queue size for the logging. If the queue backs up more, logs are dropped.
#define DEFAULT_MAX_QUEUE 8256

//! The pre-defined log levels
enum HM_LOG_LEVEL : int8_t
{
    HM_ERROR = -2,
    HM_LOG_NONE = -1,
    HM_LOG_OFF = -1,

    HM_LOG_EMERGENCY = LOG_EMERG,
    HM_LOG_ALERT = LOG_ALERT,
    HM_LOG_CRITICAL = LOG_CRIT,
    HM_LOG_ERROR = LOG_ERR,
    HM_LOG_WARNING = LOG_WARNING,
    HM_LOG_NOTICE = LOG_NOTICE,
    HM_LOG_INFO = LOG_INFO,
    HM_LOG_DEBUG = LOG_DEBUG,
    HM_LOG_DEBUG2 = LOG_DEBUG+1,
    HM_LOG_DEBUG3 = LOG_DEBUG+2,
    HM_LOG_MAXLEVEL,

    HM_LOG_ABORT = LOG_EMERG,
    HM_LOG_FATAL = LOG_ALERT
};

//! The default log level for the guardian process.
const HM_LOG_LEVEL DEFAULT_GUARDIAN_LOG_LEVEL = HM_LOG_INFO;

//! The max size of the print log level array.
#define MAX_LOG_PRINT 8
//! Human readable representation of the various log levels.
const char * const PRINT_LOG_LEVEL[] =
{
        "[EMERG]",
        "[ALERT]",
        "[CRIT]",
        "[ERROR]",
        "[WARN]",
        "[NOTICE]",
        "[INFO]",
        "[DEBUG]",
        "[DEBUG2]",
        "[DEBUG3]"
};

//! The main base class for the logging.
/*!
     The main base class for the logging.

     All logging types need to be derived from this class and implement the following functions:
     rotate - do any work needed to re-open file handles after the current log has been rotated out.
     openLog - setup and open the log file.
     closeLog - tear down and close the current logging.
     writeLog - write the log entry to the log.
 */
class HMLogBase
{

public:
    //! Basic log entry. Contains all necessary information to write the log line.
    struct LogEntry
    {
        char* entry;
        int length;
        HM_LOG_LEVEL level;
        pid_t tid;
        bool ready;
        struct timeval tv;
        struct LogEntry* next;
    };

    virtual ~HMLogBase() {}

    //! Initialize the logging.
    /*!
         Initialize the logging. This function calls the open log and does all the work to get the logging ready to go.
         \param the logfile to create/write to.
         \param the HM_LOG_LEVEL to log.
         \param true to write the log in a dedicated thread.
         \param true to install sigaction for log rotate
         \return true when the log is ready to go.
     */
    bool initLogging(std::string logFile, HM_LOG_LEVEL level, bool threaded,
          bool installSigAction=true);

    //! Initialize the logging.
    /*!
         Initialize the logging. This function calls the open log and does all the work to get the logging ready to go. This will write to the default log location.
         \param the HM_LOG_LEVEL to log.
         \param true to write the log in a dedicated thread.
         \param true to install sigaction for log rotate
         \return true when the log is ready to go.
     */
    bool initLogging(HM_LOG_LEVEL level, bool threaded, bool installSigAction=true);

    //! Set the date format string.
    /*!
         Set the date format string using the format specified for strftime.
         \param the strftime compatible data string.
     */
    void setDateString(std::string& dateString);

    //! Set the max log queue.
    /*!
         Set the max log queue. The logger will drop log messages if the queue fills.
         \param the max queue size.
     */
    void setMaxLogQueue(uint32_t max);

    //! Shutdown the logging closing the file and flushing the buffer.
    void shutDownLogging();

    //! Write to the log.
    /*!
         Write to the log.
         \param the log level to write.
         \param the log message.
         \param the variadic arguments for the c style log string.
     */
    void log(HM_LOG_LEVEL level, const char* buf, ...);

    //! Get the current log level.
    /*!
         Get the current log level.
         \return the current HM_LOG_LEVEL.
     */
    HM_LOG_LEVEL getLevel();

    //! Set the current log level.
    /*!
         Set the current log level.
         \param the new HM_LOG_LEVEL.
     */
    void setLevel(HM_LOG_LEVEL);

    //! Called when the log needs to handle a log rotation.
    virtual void rotate() = 0;

    //! Parse the log level enum value from the given string.
    /*!
         Parse the log level enum value from the given human readable string. This function is static.
         \param human readable represenation of the log level.
         \return the HM_LOG_LEVEL.
     */
    static HM_LOG_LEVEL parseLogLevel(std::string& val);

    //! Get the last error stored in the internal log buffer.
    /*!
         Get the last error stored in the internal log buffer.
         \return the last error message string.
     */
    std::string getLastError();

    //! Clear any errors in the internal log buffer.
    /*!
         Clear any errors in the internal log buffer.
     */
    void clearError();


protected:

    //! Internal log open.
    /*!
         Internal log open function. Opens the log during the init.
         \param the log file name and/or location.
         \return true if the log was opened correctly.
     */
    virtual bool openLog(std::string) = 0;

    //! Internal log close.
    /*!
         Internal log close function. Closes the log during shutdown.
     */
    virtual void closeLog() = 0;

    //! Internal write log function.
    /*!
         Internal write log function.
         Writes the log entry in the derived classes.
         \param the log entry to write.
     */
    virtual void writeLog(LogEntry *entry) = 0;

    //! Get a new log entry struct on the queue.
    /*!
         Get a new log entry struct on the queue. The allocation is done all at once to not block other threads.
         \param the new log entry to use by the caller.
     */
    void getEntry(struct LogEntry*& entry);

    //! The dedicated log writing done in the new thread.
    void flushBuffer();

    HM_LOG_LEVEL m_logLevel;
    std::string m_logFile;

    struct LogEntry* m_writeHead;
    struct LogEntry* m_readHead;

    bool m_keepRunning;
    bool m_threaded;

    HMUtilitySpinLock m_spinLock;

    std::condition_variable m_dataReadyCond;
    std::mutex m_dataReadyLock;

    std::thread m_thread;
    uint32_t m_current;
    uint32_t m_last;

    std::string m_dateString;
    uint32_t m_maxQueue;
    std::atomic<uint32_t> m_droppedLogs;

    std::vector<std::string> m_lastLogMessages;
    std::mutex m_mutex;
};

extern std::shared_ptr<HMLogBase> hlog;
//! Set the current log object as the default logger
void setAsDefaultLogger(std::shared_ptr<HMLogBase> &newLog);

//! Remove the current log object as the default logger
void unsetAsDefaultLogger();

//!  Template allowing the HMLog convenience function.
/*
    This template is inline wrapping the log level so the variadic parameter are only evaluated if the log should be written.
 */
template <typename ... Args>
inline void HMLog(const HM_LOG_LEVEL level, const char* buf, Args const& ... args)
{
    if(hlog != nullptr)
    {
        if(level <= hlog->getLevel())
        {
            hlog->log(level, buf, args ...);
        }
    }
}
#endif /* HMLOGBASE_H_ */
