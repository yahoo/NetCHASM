// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCONSTANTS_H_
#define HMCONSTANTS_H_

#include <set>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <chrono>
#include <thread>

class HMDataHostGroup;
class HMDataHostCheck;
class HMDNSResult;
class HMThreadWorker;
class HMDataCheckParams;

//! STL Definition of the mapping between host group names and host group data.
typedef std::map<std::string,HMDataHostGroup> HMDataHostGroupMap;
//! STL Definitino of the mapping between the hostname and the associated checks.
typedef std::multimap<std::string,HMDataHostCheck> HMWaitList;

//! The Current Version of the NetCHASM mdbm structure. Used to verify backend database compatibility.
const uint8_t HM_MDBM_VERSION = 2;

//! The default path to create the log file.
const std::string HM_DEFAULT_LOG_PATH = "/home/y/logs/HM.log";

//! The default time to live of the DNS resolutions in the DNS cache.
#define HM_DEFAULT_DNS_TTL 360000
//! The default check time to live in the check list cache.
#define HM_DEFAULT_TTL 30000

//! The default timeout to resolve a DNS name in milliseconds.
#define HM_DEFAULT_DNS_RESOLUTION_TIMEOUT 60000
//! The default timeout of a health check to make a connection in ms
#define HM_DEFAULT_CHECK_TIMEOUT 10000
//! The default number of times to retry a dns lookup before waiting a check timeout.
#define HM_DEFAULT_DNS_RETRIES 3
//! The default port to use for HTTP checks.
#define HM_HTTP_DEFAULT_PORT 80
//! The default port to use for HTTPS checks.
#define HM_HTTPS_DEFAULT_PORT 443
//! The default port to use for TCP checks.
#define HM_TCP_DEFAULT_PORT 80
//! The default port to use for FTP checks.
#define HM_FTP_DEFAULT_PORT 21
//! The default port to use for FTPS checks.
#define HM_FTPS_DEFAULT_PORT 990
//! The default port to use for DNS health checks.
#define HM_DNSVC_DEFAULT_PORT 53
//! The default smoothing window for the smoothed health check measurement.
#define HM_DEFAULT_SMOOTHING_WINDOW 10

// TODO Ragha we need doxygen for these constants.
#define HM_DEFAULT_TTL_THRESHOLD 10
#define HM_DEFAULT_STRIDE_PERCENT 10
#define HM_DEFAULT_MONITOR_FREQUENCY 2
#define HM_WORK_PER_THREAD_RATIO 4

// We need to use milliseconds for group-threshold and milliseconds for
// slow-threshold because that's what the current configs expect

//! The default number of ms to consider the top measurements a tie. Used to calculate the need to conduct another healthcheck if higher than the slow threshold.
#define HM_DEFAULT_GROUP_THRESHOLD 20
//! The number of ms between flaps needed to reset the flap counter.
#define HM_DEFAULT_FLAP_THRESHOLD 60000
//! The max number of milliseconds slower this health check can be without causing a re-check.
#define HM_DEFAULT_SLOW_THRESHOLD 20
//! The number of flaps before a host is marked down.
#define HM_DEFAULT_MAX_FLAPS 4

//! The supported types of event classes.
enum HM_EVENT_PLUGIN_CLASS : uint8_t
{
    HM_EVENT_DEFAULT,
    HM_EVENT_QUEUE,
    HM_EVENT_LIBEVENT
};

enum HM_LOG_PLUGIN_CLASS : uint8_t
{
    HM_LOG_PLUGIN_DEFAULT,
    HM_LOG_PLUGIN_TEXT,
    HM_LOG_PLUGIN_STDOUT,
    HM_LOG_PLUGIN_SYSLOG
};

//! The supported types of DNS lookup classes.
enum HM_DNS_PLUGIN_CLASS : uint8_t
{
    HM_DNS_PLUGIN_DEFAULT,
    HM_DNS_PLUGIN_ARES,
    HM_DNS_PLUGIN_LIBEVENT,
    HM_DNS_PLUGIN_NONE
};

//! The supported types of backend storage classes.
enum HM_STORAGE_CLASS : uint8_t
{
    HM_STORAGE_DEFAULT,
    HM_STORAGE_MDBM,
    HM_STORAGE_TEXT
};

//! The supported commit strategies to the backend
enum HM_STORAGE_COMMIT_POLICY
{
    HM_STORAGE_COMMIT_ALWAYS,
    HM_STORAGE_COMMIT_ON_FIRST,
    HM_STORAGE_COMMIT_ON_ALL_READY,
    HM_STORAGE_COMMIT_ON_TTL
};

//! The supported lock strategies to the backend
enum HM_STORAGE_LOCK_POLICY : uint8_t
{
    HM_STORAGE_GLOBAL_LOCKS,
    HM_STORAGE_RW_LOCKS,
    HM_STORAGE_PARTITION_LOCKS
};

//! The supported types of health check plugins.
enum HM_CHECK_PLUGIN_CLASS : uint8_t
{
    HM_CHECK_PLUGIN_DEFAULT,
    HM_CHECK_PLUGIN_DNS_ARES,
    HM_CHECK_PLUGIN_HTTP_CURL,
    HM_CHECK_PLUGIN_FTP_CURL,
    HM_CHECK_PLUGIN_TCP_RAW,
    HM_CHECK_PLUGIN_AUX_CURL,
    HM_CHECK_PLUGIN_HTTP_LIBEVENT
};

//! The supported health check types.
enum HM_CHECK_TYPE : uint8_t
{
    HM_CHECK_DEFAULT,
    HM_CHECK_NONE,
    HM_CHECK_HTTP,
    HM_CHECK_HTTPS,
    HM_CHECK_TCP,
    HM_CHECK_FTP,
    HM_CHECK_DNS,
    HM_CHECK_DNSVC,
    HM_CHECK_HTTPS_NO_PEER_CHECK,
    HM_CHECK_FTPS_EXPLICIT,
    HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK,
    HM_CHECK_FTPS_IMPLICIT,
    HM_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK,
    HM_CHECK_AUX_HTTP,
    HM_CHECK_AUX_HTTPS,
    HM_CHECK_AUX_HTTPS_NO_PEER_CHECK
};

//! The status of the configuration loading.
enum HM_CONFIG_STATUS :uint8_t
{
    HM_CONFIG_STATUS_OK = 0,
    HM_CONFIG_STATUS_ERROR = 1

};

//! The type of dual stack used to conduct the health check.
enum HM_DUALSTACK : uint8_t
{
    HM_DUALSTACK_UNDEFINED = 0x00,
    HM_DUALSTACK_IPV4_ONLY = 0x01,
    HM_DUALSTACK_IPV6_ONLY = 0x02,
    HM_DUALSTACK_BOTH  = 0x03
};

//! The current status of the host from the health check.
enum HM_HOST_STATUS : uint16_t
{
    HM_HOST_STATUS_NONE = 0x00,
    HM_HOST_STATUS_USE = 0x01,
    HM_HOST_STATUS_FORCE_DOWN = 0x02,
    HM_HOST_STATUS_FORCE_USE = 0x04,
    HM_HOST_STATUS_UP = 0x10,
};
inline HM_HOST_STATUS operator|(HM_HOST_STATUS a, HM_HOST_STATUS b)
{return static_cast<HM_HOST_STATUS>(static_cast<int>(a) | static_cast<int>(b));}

//! The current state of the work in the internal state machine.
enum HM_WORK_STATE : uint8_t
{
    HM_CHECK_INACTIVE,
    HM_CHECK_QUEUED,
    HM_CHECK_IN_PROGRESS,
    HM_CHECK_FAILED
};

//! The supported work types.
enum HM_WORK_TYPE : uint8_t
{
    HM_WORK_NONE,
    HM_WORK_HEALTHCHECK,
    HM_WORK_DNSLOOKUP,
    HM_WORK_AUXFETCH
};

//! The state of the event in the event queue.
enum HM_SCHEDULE_STATE : uint8_t
{
    HM_SCHEDULE_EVENT,
    HM_SCHEDULE_WORK,
    HM_SCHEDULE_IGNORE
};

//! The supported types of measurement aggregation.
enum HM_MEASUREMENT_OPTIONS : uint16_t
{
    HM_RT_CONNECT = 0x0,
    HM_RT_SMOOTHED_CONNECT = 0x2,
    HM_RT_TOTAL = 0x10,
    HM_RT_SMOOTHED_TOTAL = 0x12,
    HM_RT_BITS = 0x12
};

//! The type of response from the health check.
enum HM_RESPONSE : uint8_t
{
    HM_RESPONSE_NONE = 0,
    HM_RESPONSE_CONNECTED = 1,
    HM_RESPONSE_FAILED = 2,
    HM_RESPONSE_DNS_FAILED = 3
};

//! The detailed reason code after the health check.
enum HM_REASON : uint8_t
{
    HM_REASON_NONE = 0,
    HM_REASON_SUCCESS = 1,
    HM_REASON_DNS_NOTFOUND = 2,
    HM_REASON_DNS_TIMEOUT = 3,
    HM_REASON_DNS_FAILURE = 4,
    HM_REASON_YNET_NOTFOUND = 5,
    HM_REASON_CONNECT_TIMEOUT = 6,
    HM_REASON_CONNECT_FAILURE = 7,
    HM_REASON_REQUEST_FAILURE = 8,
    HM_REASON_RESPONSE_TIMEOUT = 9,
    HM_REASON_RESPONSE_FAILURE = 10,
    HM_REASON_RESPONSE_DOWN = 11,
    HM_REASON_RESPONSE_404 = 12,
    HM_REASON_RESPONSE_403 = 13,
    HM_REASON_RESPONSE_3XX = 14,
    HM_REASON_RESPONSE_5XX = 15,
    HM_REASON_INTERNAL_ERROR = 16
};

//! The commands supported by the control API.
enum HM_COMMAND_TASKS
{
    RELOAD,
    HOSTGROUPINFO,
    LOADFBINFO,
    THREADINFO,
    WORKQUEUEINFO,
    SCHDQUEUEINFO,
    SETHOSTSTATUS,
    HOSTGROUPLIST,
    HOSTLIST,
    HOSTCHECK,
    HOSTGROUPPARAMS,
    HOSTSCHDINFO,
    HEALTHCHECK,
    DNSCHECK,
    GETLOGLEVEL,
    SETLOGLEVEL,
    SETCONNECTIONTIMEOUT,
    GETCONNECTIONTIMEOUT,
    SETMONFREQ,
    GETMONFREQ,
    GETTTLTRESH,
    SETTTLTRESH,
    GETSTRIDE,
    SETSTRIDE,
    GETWORKPERTHREAD,
    SETWORKPERTHREAD,
    GETRECYCLE,
    SETRECYCLE,
    UNDEFINED
};

//! The states in the work order's internal state machine.
enum HM_WORK_STATUS : uint8_t
{
    HM_WORK_IDLE = 0,
    HM_WORK_COMPLETE = 1,
    HM_WORK_IN_PROGRESS = 2
};

//! Config format
enum HM_CONFIG_PLUGIN_CLASS : uint8_t
{
    HM_CONFIG_DEFAULT,
    HM_CONFIG_YAML
};

//! Print the config parser readable check type .
std::string printCheckTypeConfigs(HM_CHECK_TYPE ct);
//! Print the human readable check type.
std::string printCheckType(HM_CHECK_TYPE ct);
//! Print the human readable work type.
std::string printWorkType(HM_WORK_TYPE work);
//! Print the human readable measurement options.
std::string printMeasurementOptions(uint16_t);
//! Print the human readable dual stack.
std::string printDualStack(HM_DUALSTACK ds);
//! Print the human readable mode.
std::string printMode(uint32_t mode);
//! Print the human readable version of the response code.
std::string printReason(HM_REASON reason);
//! Print the human readable version of the check status code.
std::string printStatus(HM_HOST_STATUS status);
//! Print the human readable version of the response code.
std::string printResponse(HM_RESPONSE response);
//! Print the human readable version of the work state code.
std::string printWorkState(HM_WORK_STATE state);

#endif /* HMCONSTANTS_H_ */
