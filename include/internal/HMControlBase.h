// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCOMMANDLISTENERBASE_H
#define HMCOMMANDLISTENERBASE_H

#include <unistd.h>
#include <memory>

#include "HMConstants.h"
#include "HMLogBase.h"
#include "HMStorage.h"

const std::string HM_CMD_RELOAD = "reload";
const std::string HM_CMD_HOSTGROUP = "hostgroup";
const std::string HM_CMD_LOADFB = "loadfb";
const std::string HM_CMD_THREADINFO = "threadinfo";
const std::string HM_CMD_WORKQUEUEINFO = "workqueueinfo";
const std::string HM_CMD_SCHDQUEUEINFO = "schdqueueinfo";
const std::string HM_CMD_SETHOSTSTATUS = "host_set";
const std::string HM_CMD_HOSTGROUPLIST = "hostgrouplist";
const std::string HM_CMD_HOSTLIST = "hostlist";
const std::string HM_CMD_HOSTCHECK = "hostcheck";
const std::string HM_CMD_HOSTGROUPPARAMS = "hostgroupparams";
const std::string HM_CMD_HOSTSCHDINFO = "hostschdinfo";
const std::string HM_CMD_HEALTHCHECK = "healthcheck";
const std::string HM_CMD_DNSCHECK = "dnscheck";

const std::string HM_CMD_GETLOGLEVEL = "getloglevel";
const std::string HM_CMD_SETLOGLEVEL = "setloglevel";
const std::string HM_CMD_GETCONNECTIONTIMEOUT = "getconnectiontimeout";
const std::string HM_CMD_SETCONNECTIONTIMEOUT = "setconnectiontimeout";
const std::string HM_CMD_SETMONFREQ = "setmonitorfrequency";
const std::string HM_CMD_GETMONFREQ = "getmonitorfrequency";
const std::string HM_CMD_GETTTLTRESH = "getttlthreshold";
const std::string HM_CMD_SETTTLTRESH = "setttlthreshold";
const std::string HM_CMD_GETSTRIDE = "getstride";
const std::string HM_CMD_SETSTRIDE = "setstride";
const std::string HM_CMD_GETWORKPERTHREAD = "getworkperthreadratio";
const std::string HM_CMD_SETWORKPERTHREAD = "setworkperthreadratio";
const std::string HM_CMD_SETRECYCLE = "setrecycle";
const std::string HM_CMD_GETRECYCLE = "getrecycle";

class HMStateManager;

typedef struct hm_threadInfo_s
{
    uint64_t numThreads;
    uint64_t numIdleThreads;
} hm_threadInfo_t;

typedef struct hm_dns_sched_s
{
    bool hasv4;
    bool hasv6;
    uint64_t v4LastCheckTime;
    uint64_t v4NextCheckTime;
    HM_WORK_STATE v4State;
    uint64_t v6LastCheckTime;
    uint64_t v6NextCheckTime;
    HM_WORK_STATE v6State;
    uint32_t count;
} hm_dns_sched_t;

typedef struct hm_hc_sched_s
{
    uint8_t     addrtype;
    union
    {
        in_addr_t s_addr;
        struct in6_addr s_addr6;
    } addr;
    uint64_t lastCheckTime;
    uint64_t nextCheckTime;
    HM_WORK_STATE state;
} hm_hc_sched_t;

typedef struct hm_nameinfo_s
{
    int      ni_numhost;
    bool     ni_check_status;
    int      ni_errno;
    size_t      ni_size;
    unsigned int    ni_ttl;
    unsigned int    ni_group_threshold;
    unsigned int    ni_mode;
} hm_nameinfo_sock_t;

typedef struct hm_hostinfo2_s
{
    uint16_t        hi_size;    /* size of struct */
    uint8_t     pad[1];
    uint8_t     hi_addrtype;    /* address family */
    union {
    in_addr_t   s_addr;     /* v4 host address */
    struct in6_addr s_addr6;    /* v6 host address */
    }           hi_addr;
    unsigned int    hi_status;  /* current status (see above) */
    unsigned int    hi_reason;  /* failure reason code */
    unsigned long   hi_connect_rt;  /* time to connect socket on last check (usec) */
    unsigned long   hi_smoothed_connect_rt; /* smoothed connect time */
    unsigned long   hi_total_rt;    /* time to receive response on last check (us) */
    unsigned long long  hi_statustime;  /* timeofday when host last changed state */
    char        hi_hostname[0]; /* hostname */
} hm_hostinfo2_t;

typedef struct hm_loadfbfile_s
{
    unsigned int    file_size;
    char        file_buffer[0];
} hm_loadfbfile_t;

typedef struct hm_loadfbdata_s
{
    unsigned int    ld_size;
    unsigned int    ld_count;
    unsigned int    ld_ttl;
    unsigned int    ld_reserved0;
    unsigned long long  ld_updatetime;
    int      ni_errno;
    bool     ni_check_status;
    char        ld_filecontents[0];
} hm_loadfbdata_t;

typedef struct hm_hostcheck_s
{
    int32_t errnum;
    bool    check_status;
    uint32_t    status;  /* current status (see above) */
    uint32_t    reason;  /* failure reason code */
    uint64_t    connect_rt;  /* time to connect socket on last check (usec) */
    uint64_t    smoothed_connect_rt; /* smoothed connect time */
    uint64_t    total_rt;    /* time to receive response on last check (us) */
    uint64_t    statustime;  /* timeofday when host last changed state */
} hm_hostcheck_t;

typedef struct hm_grpcheckparams_s
{
    bool    check_status;
    HM_CHECK_TYPE checkType;
    uint16_t port;
    uint8_t dualStack;
    uint32_t checkInfoSize;
    uint32_t smoothingWindow;
    uint32_t maxFlaps;
    uint32_t flapThreshold;
    uint8_t numCheckRetries;
    uint32_t checkRetryDelay;
    uint32_t groupThreshold;
    uint32_t slowThreshold;
    uint64_t checkTimeout;
    uint64_t checkTTL;
    uint8_t mode;
    char    check_info[0];
} hm_grpcheckparams_t;

class HMCommandListenerBase
{
 public:
  HMCommandListenerBase(std::string& socketPath, HMStateManager& stateManager);
  virtual ~HMCommandListenerBase() {}

  HMCommandListenerBase& operator=(const HMCommandListenerBase&) = delete;        // Disallow copying
  HMCommandListenerBase(const HMCommandListenerBase&) = delete;

  virtual void init();
  void acceptThread();

  virtual void run() = 0;
  virtual void listernerShutDown() = 0;
  virtual void responseMessage(int, const void*, size_t) = 0;
  virtual void responseMessageVarLen(int, const char*, size_t) = 0;

  static HM_COMMAND_TASKS convert(const std::string& task);
  void handleCommands(std::string& command, int clientSock);
  void shutdown();
  void unlinkSocket(std::string& socketPath);
  void throwException(std::string errStr);

  bool getHostGroupInfo(const std::string& name, HMDataHostGroup& group);
  bool getHostSchdInfo(const std::string& hostgroup, std::string& host, hm_dns_sched_t& dns, std::vector<hm_hc_sched_t>& hcs);
  uint32_t getHostGroupResults(const std::string& name, std::vector<HMGroupCheckResult>& results);
  void createHostGroup(char* buf, size_t buflen, std::string& hostGroupName);
  std::unique_ptr<char[]> getloadfbdata (std::string rotationName, uint32_t& buflen);
  //host name, host ip, forcehostdown
  bool setHostStatus (const std::string& hostGroupName, const std::string& host, bool forceHostStatus);
  void getAllHostGroupNames(std::string& groupNames);
  void getHosts(const std::string& hostGroupName, std::string& hostNames);
  void getHostCheck(const std::string& hostGroupName, const std::string& hostName, hm_hostcheck_t* hostcheck);
  std::unique_ptr<char[]> getHostGroupParams(std::string& hostGroupName, uint32_t& buflen);

  void cleanHandlerThreads();

  static bool isValidCommand(const std::string& strCommand);

  void tokenize(std::string& command, std::vector<std::string>& tokens);

  // This object will be valid during the lifetime of the command listener
  HMStateManager& m_stateManager;
  bool m_keepRunning;
  std::thread m_mainThread;
  std::mutex m_handlerMutex;
  std::mutex m_exceptionMutex;
  std::vector<std::thread> m_handlerThreads;
  std::map<std::thread::id,bool> m_handlerThreadsStatus;
  int m_internalSocketClient;
  int m_internalSocketServer;
  int m_internalSocketSClient;
  std::string m_internalSocketPath;
  std::string m_socketPath;
};

#endif /* HMCOMMANDLISTENERBASE_H */
