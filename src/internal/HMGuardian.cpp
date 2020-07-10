// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <iostream>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "HMGuardian.h"
#include "HMLogBase.h"
#include "HMLogText.h"
#include "HMStateManager.h"

using namespace std;

// LCOV_EXCL_START; Tested in functional testing

static HMGuardian* guardian = nullptr;
static unique_ptr<HMStateManager> monitor = nullptr;

static void parentTermCallback(int s)
{
    (void)s;
    guardian->callback();
}

//LCOV_EXCL_START
static void ctrlC_Callback(int s)
{
    (void)(s);
    // Signal the main process to quit
    if(monitor)
    {
        monitor->shutdown();
    }
}
//LCOV_EXCL_STOP


void setupSignals()
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = ctrlC_Callback;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    struct sigaction sigTermHandler;
    sigTermHandler.sa_handler = ctrlC_Callback;
    sigemptyset(&sigTermHandler.sa_mask);
    sigTermHandler.sa_flags = 0;
    sigaction(SIGTERM, &sigTermHandler, NULL);

    struct sigaction sigPipeHandler;
    sigPipeHandler.sa_handler = SIG_IGN;
    sigemptyset(&sigPipeHandler.sa_mask);
    sigPipeHandler.sa_flags = 0;
    sigaction(SIGPIPE, &sigPipeHandler, NULL);
}

HMGuardian::~HMGuardian()
{
    // terminate the logging
    if(yGuardianLog != nullptr)
    {
        yGuardianLog->shutDownLogging();
        delete yGuardianLog;
    }
}

bool HMGuardian::startNetCHASM(const string& masterConfig,
        HM_LOG_LEVEL logLevel)
{
    monitor = make_unique<HMStateManager>();
    setupSignals();
    bool rc = monitor->healthCheck(masterConfig, logLevel);
    monitor.reset();
    return rc;
}

int
HMGuardian::runGuardian(HM_LOG_LEVEL logLevel, const string& masterConfig)
{
    if (!daemonise())
    {
        cerr << "Failed to daemonise the program, exiting...\n";
        return -3;
    }

    yGuardianLog = new HMLogSyslog();
    yGuardianLog->initLogging(logLevel, false);
    yGuardianLog->log(HM_LOG_INFO, "Started guardian process");
    if (createVarDir(HMVarDir) == false)
    {
        yGuardianLog->log(HM_LOG_ERROR, "Cannot create the var directory, exiting...");
        return -1;
    }

    if (createPidFile(HMVarDir, HMPidFile) == false)
    {
        yGuardianLog->log(HM_LOG_ERROR, "Cannot create the pidfile exiting...");
        return -2;
    }

    while (!bExitGuardian)
    {
        yGuardianLog->log(HM_LOG_INFO, "Spawning child process to do health checking");

        if (!(hmonPid = fork()))
        {
            bool rc = startNetCHASM(masterConfig, logLevel);
            exit(rc ? EXIT_SUCCESS : EXIT_FAILURE);
        }
        else if (hmonPid > 0)
        {
            guardian = this;
            struct sigaction sigTermHandler;
            sigTermHandler.sa_handler = parentTermCallback;
            sigemptyset(&sigTermHandler.sa_mask);
            sigTermHandler.sa_flags = 0;
            sigTermHandler.sa_flags = SA_RESTART;
            sigaction(SIGTERM, &sigTermHandler, NULL);

            yGuardianLog->log(HM_LOG_INFO, "Watching child pid %d for exit", hmonPid);
            int status;
            pid_t exitedPid = waitpid(hmonPid, &status, 0);
            if (exitedPid == -1)
            {
                // When the stop script sends us a TERM signal, the waitpid syscall
                // is interrupted and the signal handler is invoked.
                // If we don't check that the child is not reaped and it becomes a
                // zombie
                if (errno == EINTR && bExitGuardian)
                {
                    yGuardianLog->log(HM_LOG_INFO, "Exiting guardian process");
                }
                else
                {
                    string strErr = getErrnoStr();
                    yGuardianLog->log(HM_LOG_ERROR, "Failed to wait for child process, waitpid error: %s, exiting...", strErr.c_str());
                }
                break;
            }
            else if (exitedPid == hmonPid)
            {
                string strStatus;
                bool ret = getChildStatus(status, strStatus);
                if(ret)
                {
                    yGuardianLog->log(HM_LOG_INFO, "Child process (%d) exited: %s", hmonPid, strStatus.c_str());
                }
                else
                {
                    yGuardianLog->log(HM_LOG_ERROR, "Child process (%d) exited: %s", hmonPid, strStatus.c_str());
                }
                // if Child exits gracefully do not re create child;
                if(WIFEXITED(status))
                {
                    break;
                }
                // Sleep a little to avoid thrashing the CPU in case the
                // process is continuously crashing or exiting on errors
                sleep(1);
            }
        }
        else
        {
            yGuardianLog->log(HM_LOG_ERROR, "Failed to fork child process");
            return -5;
        }
    }
    yGuardianLog->log(HM_LOG_INFO, "Exiting guardian process");
    return 0;
}

void
HMGuardian::callback()
{
    yGuardianLog->log(HM_LOG_INFO, "Guardian process got TERM signal, taking down child  pid %d\n", hmonPid);
    // Kill the child
    kill(hmonPid, SIGINT);
    bExitGuardian = true;
    rmPidFile(HMVarDir, HMPidFile);
}

bool
HMGuardian::daemonise()
{
    // Fork, allowing the parent process to terminate.
    pid_t pid = fork();
    if (pid == -1)
    {
        printf("failed to fork while daemonising (errno=%d)",errno);
        return false;
    }
    else if (pid != 0)
    {
        _exit(0);
    }

    // Start a new session for the daemon.
    if (setsid()==-1)
    {
        printf("failed to become a session leader while daemonising(errno=%d)",errno);
        return false;
    }

    // Fork again, allowing the parent process to terminate.
    signal(SIGHUP,SIG_IGN);
    pid=fork();
    if (pid == -1)
    {
        printf("failed to fork while daemonising (errno=%d)",errno);
        return false;
    }
    else if (pid != 0)
    {
        _exit(0);
    }

    // Set the current working directory to the root directory.
    if (chdir("/") == -1)
    {
        printf("failed to change working directory while daemonising (errno=%d)",errno);
        return false;
    }

    // Set the user file creation mask to zero.
    umask(0);

    // Close then reopen standard file descriptors.
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    if (open("/dev/null",O_RDONLY) == -1)
    {
        printf("failed to reopen stdin while daemonising (errno=%d)",errno);
        return false;
    }
    if (open("/dev/null",O_WRONLY) == -1)
    {
        printf("failed to reopen stdout while daemonising (errno=%d)",errno);
        return false;
    }
    if (open("/dev/null",O_RDWR) == -1)
    {
        printf("failed to reopen stderr while daemonising (errno=%d)",errno);
        return false;
    }

    return true;
}

// LCOV_EXCL_STOP; Tested in functional testing

bool
HMGuardian::createVarDir(const string& vardir)
{
  struct stat sbuf;

  int rc = stat(vardir.c_str(), &sbuf);
  if (rc)
  {
    // If errno is ENOENT, it means probably dir doesn't exist, so create it
    if (errno != ENOENT)
    {
      return false;    //LCOV_EXCL_LINE;
    }
    else
    {
      return (mkdir(vardir.c_str(), 0777) == 0);
    }
  }
  // Path exists, but not a directory, bail out
  else if (!S_ISDIR(sbuf.st_mode))
  {
    return false;
  }
  return true;
}

bool
HMGuardian::createPidFile(const string& vardir, const string& pidfilename)
{
  pid_t mypid = getpid();
  string pidfile = vardir + pidfilename;

  ofstream fh(pidfile.c_str());
  if (fh.is_open())
  {
    fh << mypid << "\n";
    return true;
  }

  return false; //LCOV_EXCL_LINE;
}

bool
HMGuardian::rmPidFile(const string& vardir, const string& pidfilename)
{
  string pidfile = vardir + pidfilename;
  return (unlink(pidfile.c_str()) == 0);
}

bool
HMGuardian::getChildStatus(int &status, string& strstatus)
{
  ostringstream oss;

  if (WIFEXITED(status))
  {
    int exitStatus = WEXITSTATUS(status);
    oss << "Child exited with status " << exitStatus;
    strstatus = oss.str();
    return exitStatus == 0 ? true : false;
  }

  if (WIFSIGNALED(status))
  {
    int termSignal = WTERMSIG(status);
    oss << "Child killed by signal " << termSignal;
    strstatus = oss.str();
    return false;
  }

  strstatus = "Child status unknown"; //LCOV_EXCL_LINE;
  return false;
}

string
HMGuardian::getErrnoStr()
{
  char buf[1024];
  char *pBuf = strerror_r(errno, buf, sizeof(buf));
  buf[sizeof(buf)-1] = '\0';

  string strBuf(pBuf);
  return strBuf;
}

