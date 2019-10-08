// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSTATEMANAGER_H_
#define HMSTATEMANAGER_H_

#include <memory>

#include "HMControlBase.h"
#include "HMEventLoopLibEvent.h"
#include "HMState.h"
#include "HMWorkQueue.h"
#include "HMConstants.h"
#include "HMDNSCache.h"
#include "HMLogBase.h"
#include "HMHostMark.h"

class HMThreadPool;
class HMCommandListenerBase;
class HMStateManager
{
public:

    HMStateManager();
    HMStateManager(HMStateManager&) = delete;
    ~HMStateManager();

    //! Grab the latest version of the loaded state.
    /*!
         Grab the latest version of the loaded state.
         Various classes in the codebase can keep a shared pointer on the state. Calling the update function at set times allows the work to finish and smoothly switch to a new state.
         During a relaod, the reloader willl only garbage collect when the work has updated the shared pointer.
     */
    bool updateState(std::shared_ptr<HMState>& current);

    //! Load the Daemon State and prepare the Daemon
    /*!
         Load the Daemon State and prepare the Daemon. Should only be called on initial load.
         \param the master config to parse.
         \param the HM_LOG_LEVEL to use for logging.
         \return true on success.
     */
    bool loadDaemonState(const std::string& masterConfig, HM_LOG_LEVEL logLevel);

    //! Load the certificate and key files
    /*!
         Load the certificate files and key files.
         \return true on success.
     */
    bool loadCertificates();

    //! Reaload the Daemon.
    /*!
         Trigger a reload of the Daemon state. The new state will re-parse the master and health check configs.
         The reload function will copy state from the current state to the new state replacing the new state to the current before returning.
         The function takes care of rescheduling health checks and DNS resolutions. It also garbage collects the old state before returning.
         \param the new master config to parse.
         \return true on a successful reload.
     */
    bool reloadDaemonConfigs(const std::string& masterConfig);

    //! Reload the Daemon.
    /*!
         Trigger a reload of the Daemon state. The new state will re-parse the current master and health check configs.
         The reload function will copy state from the current state to the new state replacing the new state to the current before returning.
         The function takes care of rescheduling health checks and DNS resolutions. It also garbage collects the old state before returning.
         \return true on a successful reload.
     */
    bool reloadDaemonConfigs();

    //! Main health check function.
    /*!
         Main health check function.
         This is the class that should be called from the main or guardian process.
         Runs the entire Daemon starting with a config load. Sets up the event handler, work queues, control sockets and all necessary threads to run the daemon.
         Catches interrupt signals to force the Daemon to shut down.
         \param the master config to parse when loading the Daemon.
         \parma the log level to use for all event logging.
         \return false upon failure.
     */
    bool healthCheck(std::string masterConfig, HM_LOG_LEVEL logLevel);

    //! Shutdown the Daemon.
    void shutdown();

    //! Start logging.
    /*!
         Start the logging. Either during initial setup or during a reload.
         Deals with starting the logging defined in the master config. If that logging fails, starts logging to syslog. If syslog fails, starts logging to stdout.
         If the stdout fails, it will drop back to logging to the previous source. If the previous logging and new logging are the same, the function does nothing.
         \param the last log type before the start logging call. (The type we are replacing).
         \param the last log path.
         \param the last log level.
         \return true if the logger is logging regardless of what type ended up being opened.
     */
    bool startLogging(uint32_t lastLogType, const std::string& lastLogPath, HM_LOG_LEVEL lastLogLevel);

    //! Get the number of idle threads in the work pool.
    /*!
         Get the number of idle threads in the work pool.
         \return the number of idle threads in the work pool.
     */
    uint64_t getIdleThreads();

    //! Get the number of timeouts in the event loop.
    /*!
         Get the number of timeout in the event loop.
         \return the number of events in the event loop.
     */
    uint64_t getEventQueueSize();

    //! Get a pointer to the lib event instance for HTTP and DNS event lib queries.
    /*!
         Get a pointer to the lib event instance for HTTP and DNS event lib queries.
         \return a pointer to the current HMEventLoopLibEvent instance defined for use.
     */
    HMEventLoopLibEvent* getLibEvent() { return m_libEvent; }

    //! Get the current log level.
    /*!
         Get the current log level for the running logger.
         \return the current log level for the running logger.
     */
    HM_LOG_LEVEL getLogLevel();

    //! Get the number of threads running in the work pool.
    /*!
         Get the number of threads running in the work pool.
         \return the number of threads running in the work pool.
     */
    uint32_t getNThreads() const;

    //! Get the current timeout between updating the thread monitoring stats.
    /*!
         Get the current timeout between updating the thread monitoring stats.
         \return the current timeout between updating the thread monitoring stats.
     */
    uint32_t getMonitorFrequency() const;

    //! Get the current stride percent.
    /*!
         Get the current stride percent for the work pool. The stride percent is the percentage of idle threads allowed before the Daemon adjusts the thread count lower.
         \return the current stride percent.
     */
    uint32_t getStridePercent() const;

    //! Get the current worker to thread ratio.
    /*!
         Get the current worker to thread ratio. The worker to thread ratio is used to calibrate the number of threads when the pool increases the thread count. It will increase to work queue length / worker per thread ratio.
         \return the current worker to thread ratio.
     */
    uint32_t getWorkPerThreadRatio() const;

    //! Get if the work pool is using thread recycling.
    /*!
         Get if the work pool is using thread recycling. Thread recycling will periodically flush thread state for long running Curl and Ares library state.
         \return true if thread recycling is on.
     */
    bool isRecycle() const;

    //! Set the current log level.
    /*!
         Set the current log level for the running logger.
         \param the new log level for the running logger.
     */
    void setLogLevel(HM_LOG_LEVEL level);

    //! Set the current timeout between updating the thread monitoring stats.
    /*!
         Set the current timeout between updating the thread monitoring stats.
         \param the new time between updating the thread monitoring stats.
     */
    void setMonitorFrequency(uint32_t monitorFrequency);

    //! Set the current stride percent.
    /*!
         Set the current stride percent. The stride percent is the percentage of idle threads allowed before the Daemon adjusts the thread count lower.
         \param the new stride percent.
     */
    void setStridePercent(uint32_t stridePercent);

    //! Set the work per thread ratio.
    /*!
         Set the work per thread ratio.  The worker to thread ratio is used to calibrate the number of threads when the pool increases the thread count. It will increase to work queue length / worker per thread ratio.
         \param the new work per thread ratio.
     */
    void setWorkPerThreadRatio(uint32_t workPerThreadRatio);


    //! Set the current thread pool to use thread recycle.
    /*!
         Set the current thread pool to use thread recycling. Thread recycling will periodically flush thread state for long running Curl and Ares library state.
         \param true to enable thread recycling.
     */
    void setRecycle(bool recycle);

    //! Manually set the state into the HMStateMaster instance.
    /*!
         Force the given state into the HMStateMaster.
         \param the state to use as the new HMState.
     */
    void setState(std::shared_ptr<HMState> debugState);

    //! check is remote query reply is enabled
    bool isEnableRemoteQueryReply() const;

    //! set remote query reply flag
    void setEnableRemoteQueryReply(bool enableRemoteQueryReply);

    //! The Work Queue
    HMWorkQueue m_workQueue;

    HMHostMark m_hostMark;

private:

    bool m_keepRunning;

    std::condition_variable m_shutdownCond;
    std::mutex m_shutdownMutex;

    HMEventLoop* m_eventLoop;
    HMThreadPool* m_threadPool;
    HMEventLoopLibEvent* m_libEvent;

    std::mutex m_reloadMutex;

    std::shared_ptr<HMState> m_currentState;
    std::shared_ptr<HMState> m_newState;
    std::vector<std::unique_ptr<HMCommandListenerBase>> m_listener;
    bool m_enableRemoteQueryReply;
};

#endif /* HMSTATEMANAGER_H_ */
