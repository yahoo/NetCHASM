// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMGUARDIAN_H_
#define HMGUARDIAN_H_

#include <sys/types.h>
#include <string>

#include "HMLogSyslog.h"
#include "HMConstants.h"
#include "HMLogBase.h"
#include "HMStateManager.h"

// LCOV_EXCL_START; Tested in functional testing

//! The default run directory for netchasm
const std::string DEFAULT_HM_VARDIR = "/home/y/var/run/netchasm/";
//! The default name ot he pid file to create.
const std::string DEFAULT_HM_PIDFILE = "netchasm.pid";

class HMLogBase;

//! The guardian class monitors the netchasm daemon process. It restarts in case of a crash.
class HMGuardian
{
public:
    HMGuardian() :
        yGuardianLog(nullptr),
        hmonPid(0),
        bExitGuardian(false),
        HMVarDir(DEFAULT_HM_VARDIR),
        HMPidFile(DEFAULT_HM_PIDFILE) {}

    ~HMGuardian();

    //! The main function to run the netchasm daemon being monitored by the guardian process.
    /*!
         The main function to run the netchasm daemon being monitored by the guardian process.
         The daemon can be shutdown upon receiving a TERM signal.
         \param The log level to use for the guardian messages.
         \param The path to the master configuration to configure the daemon.
         \return Negative indicates an error. Zero means success.
     */
    int runGuardian(HM_LOG_LEVEL logLevel, const std::string& masterConfig);

    //! Cleanly exit the daemon upon receiving a term signal.
    /*!
         Cleanly exit the daemon upon receiving a term signal.
     */
    void callback();

private:
    //! Fork the process and start NetCHASM as a daemon.
    /*!
          Fork the process and start netchasm as a daemon.
          \return true if the process forked and is running.
     */
    bool daemonise();

    //! Create the Var directory.
    /*!
         Create the Var directory if it does not exist.
         \param the directory to create.
         \return true if the directory is ready.
     */
    bool createVarDir(const std::string& vardir);

    //! Create the Pid file to record the pid of the current process.
    /*!
         Create the Pid file to record the pid of the current process.
         \param the directory to place the pid file.
         \param the name of the pid file to create.
         \return true on successfully crating the pid file.
     */
    bool createPidFile(const std::string& vardir, const std::string& pidfilename);

    //! Remove the pid file.
    /*!
         Remove the Pid file
         \param the directory containing the Pid file.
         \param the name of the Pid file.
         \return true upon successfully deleting the Pid file.
     */
    bool rmPidFile(const std::string& vardir, const std::string& pidfilename);

    //! Convert the child status code to a status string.
    /*!
         Convert the child status code to a status string.
         \param the status code to convert.
         \param a string to fill with the human readable status.
         \return true if the error string has been parsed and filled.
     */
    bool getChildStatus(int &status, std::string& strstatus);

    //! Convert an errno string to string.
    /*!
         Convert an errno string to a standard string.
         \return a standard string.
     */
    std::string getErrnoStr();

    HMStateManager monitor;
    HMLogBase* yGuardianLog;
    pid_t hmonPid; // child pid
    bool bExitGuardian = false;

    std::string HMVarDir;
    std::string HMPidFile;
};

#endif /* HMGUARDIAN_H_ */

// LCOV_EXCL_STOP; Tested in functional testing
