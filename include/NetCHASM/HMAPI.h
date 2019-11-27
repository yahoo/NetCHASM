// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HEALTHMON_HMAPI_H_
#define INCLUDE_HEALTHMON_HMAPI_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <vector>

#define HASH_MD5_MAX_SIZE 64 //Max MD5 length EVP_MAX_MD_SIZE

//! Config format
enum HM_API_CONFIG_CLASS : uint8_t
{
    HM_API_CONFIG_DEFAULT,
    HM_API_CONFIG_YAML
};

//! Schedule Information
enum HM_API_WORK_STATE : uint8_t
{
    HM_API_CHECK_INACTIVE,
    HM_API_CHECK_QUEUED,
    HM_API_CHECK_IN_PROGRESS,
    HM_API_CHECK_FAILED
};

//! Check Type
enum HM_API_CHECK_TYPE : uint8_t
{
    HM_API_CHECK_DEFAULT,
    HM_API_CHECK_NONE,
    HM_API_CHECK_HTTP,
    HM_API_CHECK_HTTPS,
    HM_API_CHECK_TCP,
    HM_API_CHECK_FTP,
    HM_API_CHECK_DNS,
    HM_API_CHECK_DNSVC,
    HM_API_CHECK_HTTPS_NO_PEER_CHECK,
    HM_API_CHECK_FTPS_EXPLICIT,
    HM_API_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK,
    HM_API_CHECK_FTPS_IMPLICIT,
    HM_API_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK,
    HM_API_CHECK_AUX_HTTP,
    HM_API_CHECK_AUX_HTTPS,
    HM_API_CHECK_AUX_HTTPS_NO_PEER_CHECK
};


//! The supported types of DNS lookup classes.
enum HM_DNS_CHECK_TYPE : uint8_t
{
    HM_API_DNS_ARES,
    HM_API_DNS_LIBEVENT,
    HM_API_DNS_STATIC,
    HM_API_DNS_NONE
};


class HMState;
class HMLogAPI;
class HMDataCheckResult;
class HMDataHostGroup;
class HMGroupCheckResult;
class HMDataHostCheck;
class HMDataCheckParams;
class HMIPAddress;
//! API class to store the config information.
class HMAPIConfigInfo
{
public:
    uint8_t m_version;
    bool m_configError;
    uint64_t m_timestamp;
    char m_hash[HASH_MD5_MAX_SIZE];
};

//! API class to store an IPAddress.
class HMAPIIPAddress
{
public:
    HMAPIIPAddress() :
    m_type(AF_UNSPEC)
    {
        memset(&m_ip, 0, sizeof(m_ip));
    }

    uint8_t m_type;

    union
    {
        in_addr_t addr;
        struct in6_addr addr6;
    } m_ip;

    HMAPIIPAddress(uint8_t type);
    bool operator==(const HMAPIIPAddress &other) const;

    //! Set the Address based on a human readable string.
    /*!
         Set the Address based on a human readable string.
         \param the string to parse.
         \return true if the address was parsed successfully.
     */
    bool set(const std::string& addr);

    //! Set the Address from existing internal Address.
    /*!
         Set the Address from existing internal Address.
         \param the Internal Address Structure.
         \return true if the address was parsed successfully.
     */
    bool set(const HMIPAddress& addr);

    //! Print the address to human readable string.
    /*!
         Print the address to human readable string.
         \return human readable IP address.
     */
    std::string toString() const;

    // TODO probably set the output convenience functions
};


//! API class to store the check info for both a host group and per host.
class HMAPICheckInfo
{
public:
    HMAPICheckInfo();
    HMAPICheckInfo(HMDataHostGroup& );
    HMAPICheckInfo(HMDataHostCheck&, HMDataCheckParams&);
    HMAPICheckInfo& operator=(HMDataHostGroup&);
    bool operator==(const HMAPICheckInfo& k) const;
    bool operator!=(const HMAPICheckInfo& k) const;
    uint16_t m_measurementOptions;
    bool m_ipv4;
    bool m_ipv6;
    HM_API_CHECK_TYPE m_checkType;
    uint16_t m_port;
    std::string m_checkInfo;
    uint8_t m_numCheckRetries;
    uint32_t m_checkRetryDelay;
    uint32_t m_smoothingWindow;
    uint32_t m_groupThreshold;
    uint32_t m_slowThreshold;
    uint32_t m_maxFlaps;
    uint64_t m_checkTimeout;
    uint64_t m_checkTTL;
    uint32_t m_flapThreshold;
    uint32_t m_passthroughInfo;
    uint8_t m_TOSValue;
    HM_DNS_CHECK_TYPE m_dnsCheckType;
    HMAPIIPAddress m_sourceAddress;
    std::vector<std::string> m_hosts;
    std::vector<std::string> m_hostGroups;
    // TODO Print measurementOPtions and checktype
};

//! API class to hold the check result information.
class HMAPICheckResult
{
public:
    HMAPICheckResult();
    HMAPICheckResult(HMDataCheckResult&);
    HMAPICheckResult(HMGroupCheckResult&);

    std::string m_host;
    HMAPIIPAddress m_address;
    uint64_t m_start;
    uint64_t m_end;
    uint32_t m_responseTime;
    uint32_t m_totalResponseTime;
    uint32_t m_minResponseTime;
    uint32_t m_maxResponseTime;
    uint32_t m_smoothedResponseTime;
    uint64_t m_sumResponseTime;
    uint32_t m_numChecks;
    uint32_t m_numResponses;
    uint32_t m_numConnectFailures;
    uint32_t m_numFailures;
    uint32_t m_numTimeouts;
    uint32_t m_numFlaps;
    uint16_t m_status;
    uint8_t m_response;
    uint8_t m_reason;
    uint8_t m_softReason;
    uint8_t m_numFailedChecks;
    uint8_t m_numSlowResponses;
    uint16_t m_port;
    uint64_t m_changeTime;
    uint64_t m_prevTime;
    bool m_forceHostDown;
    // variables to track the last check information
    uint64_t m_queueCheckTime;
    uint64_t m_checkTime;

    // TODO print for status, response, reason
};

//! API class to hold load feedback information.
class HMAPILFB
{
public:
    uint8_t m_type;
    uint64_t m_ts;
    int64_t m_load;
    int64_t m_target;
    int64_t m_max;
    std::string m_host;
    std::string m_resource;
    std::string m_datacenter;
};

//! API class to hold the out of band information.
class HMAPIOOB
{
public:
    uint8_t m_type;
    uint32_t m_shed;
    uint64_t m_ts;
    bool m_forceDown;
    std::string m_host;
    std::string m_resource;
};

//! API class to hold the Aux Information including OOB and LFB.
class HMAPIAuxInfo
{
public:
    std::string m_host;
    HMAPIIPAddress m_address;
    uint64_t m_ttl;
    uint64_t m_updatetime;
    std::vector<HMAPIOOB> m_oob;
    std::vector<HMAPILFB> m_lfb;
};

//! API class to hold the scheduling information of host's heath check.
class HMAPIHostSchedInfo
{
public:
    HMAPIIPAddress m_address;
    uint64_t m_lastCheckTime;
    uint64_t m_nextCheckTime;
    HM_API_WORK_STATE m_state;
};

//! API class to hold the scheduling information of host's DNS check.
class HMAPIDNSSchedInfo
{
public:
    bool m_hasv4;
    bool m_hasv6;
    uint64_t m_v4LastCheckTime;
    uint64_t m_v4NextCheckTime;
    HM_API_WORK_STATE m_v4State;
    uint64_t m_v6LastCheckTime;
    uint64_t m_v6NextCheckTime;
    HM_API_WORK_STATE m_v6State;
    std::vector<HMAPIHostSchedInfo> m_hostScheduleInfo;
};

//! API class to hold the thread pool information.
class HMAPIThreadInfo
{
public:
    uint64_t m_numThreads;
    uint64_t m_numIdleThreads;
};

//! API class to hold the data host check information.
class HMAPIDataHostCheck
{
public:
    HMAPIDataHostCheck() :
            m_checkType(HM_API_CHECK_DEFAULT),
            m_port(0),
            m_ipv4(true),
            m_ipv6(false),
            m_TOSValue(0),
            m_dnsCheckType(HM_API_DNS_ARES){}
    HM_API_CHECK_TYPE m_checkType;
    uint16_t m_port;
    bool m_ipv4;
    bool m_ipv6;
    std::string m_checkInfo;
    HMAPIIPAddress m_sourceAddress;
    uint8_t m_TOSValue;
    HM_DNS_CHECK_TYPE m_dnsCheckType;
};


#endif /* INCLUDE_HEALTHMON_HMAPI_H_ */
