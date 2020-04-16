// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDATAHOSTGROUP_H_
#define HMDATAHOSTGROUP_H_

#include <vector>
#include <string>
#include <algorithm>

#include "HMDataHostCheck.h"
#include "HMDataCheckParams.h"
#include "HMConstants.h"
#include "HMStorage.h"
#include "HMHashMD5.h"

class HMHashMD5;
class HMDataHostGroup
{
public:
    HMDataHostGroup(const std::string& groupName) :
        m_groupName(groupName),
        m_measurementOptions(HM_RT_CONNECT),
        m_dualstack(HM_DUALSTACK_IPV4_ONLY),
        m_checkType(HM_CHECK_DEFAULT),
        m_remoteCheckType(HM_REMOTE_CHECK_NONE),
        m_port(0),
        m_numCheckRetries(0),
        m_checkRetryDelay(0),
        m_smoothingWindow(HM_DEFAULT_SMOOTHING_WINDOW),
        m_groupThreshold(HM_DEFAULT_GROUP_THRESHOLD),
        m_slowThreshold(HM_DEFAULT_SLOW_THRESHOLD),
        m_maxFlaps(HM_DEFAULT_MAX_FLAPS),
        m_checkTimeout(HM_DEFAULT_CHECK_TIMEOUT),
        m_checkTTL(HM_DEFAULT_TTL),
        m_flapThreshold(HM_DEFAULT_FLAP_THRESHOLD),
        m_passthroughInfo(0),
        m_distributedFallback(HM_DISTRIBUTED_FALLBACK_NONE),
        m_checkPlugin(HM_CHECK_PLUGIN_DEFAULT),
        m_DNSType(HM_DNS_TYPE_LOOKUP),
        m_TOSValue(0),
        m_flowType(HM_FLOW_DNS_HEALTH_TYPE) {};

    bool operator<(const HMDataHostGroup& k) const;
    bool operator==(const HMDataHostGroup& k) const;
    bool operator!=(const HMDataHostGroup& k) const;

    HMDataHostGroup(const std::string& groupName, HMAPICheckInfo& checkInfo);

    //! Fill in a host check data structure based on this host group info.
    /*!
         Fill in a host check data structure based on this host group info.
         \param a data host check class to fill in with the check parameters.
         \return true if the host check class is ready.
     */
    bool getHostCheck(HMDataHostCheck& check) const;

    //! Fill in a check params class based on the host group info.
    /*!
         Fill in a check params class based on this host group info.
         \param a check params data class to set with this check params.
         \return true if the check params is ready.
     */
    bool getCheckParameters(HMDataCheckParams& check) const;

    //! Set the host group parameters (shallow copy)
    /*!
         Set the host group parameters without copying the host list.
         \param the host group to copy from.
     */
    void setHostGroupParameters(const HMDataHostGroup& hostGroup);

    //! Add a host to the host list for this group.
    /*!
         Add a host to the host list for this group.
         \param the host to add to the host list.
     */
    void addHost(std::string& host);

    //! Set the measurement options for this group.
    /*!
         Set the measurement options for this group.
         \param the measurement options.
     */
    void setMeasurementOptions(uint16_t options);

    //! Set the dual stack for this group.
    /*!
         Set the dual stack for this group.
         \param the dual stack setting.
     */
    void setDualStack(HM_DUALSTACK dualstack);

    //! Set the check type for this group.
    /*!
         Set the check type for this group.
         \param the check type setting.
     */
    void setCheckType(HM_CHECK_TYPE checkType);

    //! Set the port to check for this group.
    /*!
         Set the port to check for this group.
         \param the port to check.
     */
    void setPort(uint16_t port);

    //! Set the check info for this group.
    /*!
         Set the check info for this group.
         \param the check info setting.
     */
    void setCheckInfo(const std::string& checkInfo);

    //! Set the check retries for this group.
    /*!
         Set the check retries for this group.
         \param the check retries setting.
     */
    void setNumCheckRetries(uint8_t numCheckRetries);

    //! Set the retry delay for this group.
    /*!
         Set the retry delay for this group.
         \param the retry delay setting.
     */
    void setCheckRetryDelay(uint32_t checkRetryDelay);

    //! Set the check timeout for this group.
    /*!
         Set the check timeout for this group.
         \param the check timeout setting.
     */
    void setCheckTimeout(uint64_t checkTimeout);

    //! Set the check TTL for this group.
    /*!
         Set the check TTL for this group.
         \param the check TTL setting.
     */
    void setCheckTTL(uint64_t checkTTL);

    //! Set the group threshold for this group.
    /*!
         Set the group threshold for this group.
         \param the group threshold setting.
     */
    void setGroupThreshold(uint32_t groupThreshold);

    //! Set the smoothing window for this group.
    /*!
         Set the smoothing window for this group.
         \param the smoothing window setting.
     */
    void setSmoothingWindow(uint32_t smoothingWindow);

    //! Set the flap threshold for this group.
    /*!
         Set the flap threshold for this group.
         \param the flap threshold setting.
     */
    void setFlapThreshold(uint32_t flapThreshold);

    //! Set the max flaps for this group.
    /*!
         Set the max flaps for this group.
         \param the max flaps setting.
     */
    void setMaxFlaps(uint32_t maxFlaps);

    //! Set the slow threshold for this group.
    /*!
         Set the slow threshold for this group.
         \param the slow threshold setting.
     */
    void setSlowThreshold(uint32_t slowThreshold);

    //! Set the passthrough info for this group.
    /*!
         Set the passthrough info for this group.
         \param the passthrough info setting.
     */
    void setPassthroughInfo(uint32_t info);

    //! Set fallback mode for the remote check.
    /*!
         Set fallback mechanism for remote check.
         \param remote fallback value
     */
    void setDistributedFallback(HM_DISTRIBUTED_FALLBACK distributedFallback);

    //! Unset fallback mode for the remote check.
    /*!
         UnSet fallback mechanism for remote check.
         \param remote fallback value
     */
    void unsetDistributedFallback(HM_DISTRIBUTED_FALLBACK distributedFallback);


    //! Set the check plugin for this group.
    /*!
         Set the check plugin for this group.
         \param the check plugin setting.
     */
    void setCheckPlugin(HM_CHECK_PLUGIN_CLASS checkPlugin);

    //! Set the remote check type for this group.
    /*!
         Set the remote check type for this group.
         \param the remote check type setting.
     */
    void setRemoteCheckType(HM_REMOTE_CHECK_TYPE checkType);

    //! Get the name of the group.
    /*!
         Get the name of the group.
         \return the name of the group.
     */
    std::string getName() const;

    //! Get the check TTL for the group.
    /*!
         Get the check TTL for the group.
         \return the check TTL for the group.
     */
    uint64_t getCheckTTL() const;

    //! Get the check type for the group.
    /*!
         Get the check type for the group.
         \return the check type for the group.
     */
    HM_CHECK_TYPE getCheckType() const;

    //! Get the remote check type for the group.
    /*!
         Get the remote check type for the group.
         \return the remote check type for the group.
     */
    HM_REMOTE_CHECK_TYPE getRemoteCheckType() const;

    //! Get the check port for the group.
    /*!
         Get the check port for the group.
         \return the check port for the group.
     */
    uint16_t getCheckPort() const;

    //! Get the measurement options for the group.
    /*!
         Get the measurement options for the group.
         \return the measurement options for the group.
     */
    uint16_t getMeasurementOptions() const;

    //! Get the check timeout for the group.
    /*!
         Get the check timeout for the group.
         \return the check timeout for the group.
     */
    uint64_t getCheckTimeout() const;

    //! Get the group threshold for the group.
    /*!
         Get the group threshold for the group.
         \return the group threshold for the group.
     */
    uint32_t getGroupThreshold() const;

    //! Get the check info for the group.
    /*!
         Get the check info for the group.
         \return the check info for the group.
     */
    std::string getCheckInfo() const;

    //! Get the flap threshold for the group.
    /*!
         Get the flap threshold for the group.
         \return the flap threshold for the group.
     */
    uint32_t getFlapThreshold() const;

    //! Get the dual stack setting for the group.
    /*!
         Get the dual stack setting for the group.
         \return the dual stack setting for the group.
     */
    HM_DUALSTACK getDualstack() const;

    //! Get the passthrough info for the group.
    /*!
         Get the passthrough info for the group.
         \return the passthrough info for the group.
     */
    uint32_t getPassthroughInfo() const;

    //! Get the max flaps for the group.
    /*!
         Get the max flaps for the group.
         \return the max flaps for the group.
     */
    uint32_t getMaxFlaps() const;

    //! Get the smoothing window for the group.
    /*!
         Get the smoothing window for the group.
         \return the smoothing window for the group.
     */
    uint32_t getSmoothingWindow() const;

    //! Get the check retries setting for the group.
    /*!
         Get the check retries setting for the group.
         \return the check retries setting for the group.
     */
    uint8_t getNumCheckRetries() const;

    //! Get the slow threshold for the group.
    /*!
         Get the slow threshold for the group.
         \return the slow threshold for the group.
     */
    uint32_t getSlowThreshold() const;

    //! Get the check retry delay for the group.
    /*!
         Get the check retry delay for the group.
         \return the check retry delay for the group.
     */
    uint32_t getCheckRetryDelay() const;

    //! Get the host list for the group.
    /*!
         Get the host list for the group.
         \return a read only pointer to the host list for the group.
     */
    const std::vector<std::string>* getHostList() const;

    //! Check if the given host is in the host group.
    /*!
         Check if the given host is in the host group.
         \param the host to check.
         \return true if the host is present in the host list.
     */
    bool isValidHost(std::string& host) const;

    //! Function to serialize the current host group info into a raw buffer.
    /*!
         The serialize function supports two types of calls designed to be called consecutively.
         When called with a null buf and size 0 serialize will return the required size of the buf to store the host group info.
         When called with a non-null buf and the correct size, serialize will store the host group info into the buf.
         \param buf pass nullptr to get the required size or a raw buffer to fill.
         \param size pass 0 to get the required size or the required size to fill the buffer.
         \return The required size of the buf or the number of bytes saved to the buffer.
     */
    uint32_t serialize(char* buf, uint32_t size) const;
    //! De-serialize the raw buffer.
    /*!
         This function is called to deserializethe host group info. It fills in the class data from the raw buffer.
         \param buf raw buffer to deserialize.
         \param size the size of the raw buffer.
         \return true if the deserialize was a success.
     */
    bool deserialize(char* buf, uint32_t size);

    //! Get the hash value of this host group info.
    /*!
         Get the hash value of this host group info.
         \param the hash to update with the state of this host group.
     */
    void getHash (HMHashMD5& hash);

    //! Get the remote host used for the check.
    /*!
         Get the remote host used for the check.
         \return the remote host string.
     */
    const std::string& getRemoteCheck() const;

    //! Set the remote host used for the check.
    /*!
         Set the remote host used for the check.
         \param the remote host string.
     */
    void setRemoteCheck(const std::string& remoteCheck);

    //! Add a childhostgroup to the hostgroup list for this group.
    /*!
         Add a childhostgroup to the  hostgroup list for this group.
         \param the hostgroup to add to the hostgroup list.
     */    
    void addHostGroup(std::string& hostGroup);
    
    //! Get the hostgroup list for the group.
    /*!
         Get the hostgroup list for the group.
         \return a read only pointer to the hostgroup list for the group.
     */    
    const std::vector<std::string>* getHostGroupList() const;

    //! Get the fallback for distributed check.
    /*!
         Get the fallback for distributed health check.
         \return a fallback mode.
     */
    HM_DISTRIBUTED_FALLBACK getDistributedFallback() const;

    const HMHash& getHashValue() const;

    void setHashValue(const HMHash& hashValue);
    //! Get the source ip specified for the check.
    /*!
         Get the source ip specified for the check.
         \return the IP address.
     */
    const HMIPAddress& getSourceAddress() const;

    //! Set the source ip for the check.
    /*!
         Set the source ip for the check.
         \param the IP address to be used as source IP.
     */
    void setSourceAddress(HMIPAddress& sourceAddress);

    //! Get the check plugin used for the check.
    /*!
         Get the check plugin used for the check.
         \return the check plugin.
     */
    HM_CHECK_PLUGIN_CLASS getCheckPlugin() const;

    //! Get the type of service value.
    /*!
         Get the type of service specified for the Host group.
         \return the TOS value.
     */
    uint8_t getTOSValue() const;

    //! Set the type of service value.
    /*!
         Set the type of service specified for the host group.
         \param TOS value.
     */
    void setTOSValue(uint8_t tosValue);

    //! Get the type of DNS check.
    /*!
         Get the type of DNS check for the host group.
         \return the DNS check type.
     */
    HM_DNS_TYPE getDNSType() const;

    //! Set the type of DNS check.
    /*!
         Set the type of DNS check for the health check.
         \param the DNS check type.
     */
    void setDNSType(HM_DNS_TYPE dnstype);

    //! Get the type of flow type.
    /*!
         Get the type of flow type for the host group.
         \return the flow type.
     */
    HM_FLOW_TYPE getFlowType() const;

    //! Set the type of flow type.
    /*!
         Set the type of flow type for the health check.
         \param the flow type.
     */
    void setFlowType(HM_FLOW_TYPE flowType);

private:

    std::string m_groupName;
    uint16_t m_measurementOptions;
    HM_DUALSTACK m_dualstack;
    HM_CHECK_TYPE m_checkType;
    HM_REMOTE_CHECK_TYPE m_remoteCheckType;
    uint16_t m_port;
    std::string m_checkInfo;
    std::string m_remoteCheck;
    HMIPAddress m_sourceAddress;
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
    HM_DISTRIBUTED_FALLBACK m_distributedFallback;
    HMHash m_hashValue;
    HM_CHECK_PLUGIN_CLASS m_checkPlugin;
    HM_DNS_TYPE m_DNSType;
    uint8_t m_TOSValue;
    HM_FLOW_TYPE m_flowType;
    std::vector<std::string> m_hostGroups;
    std::vector<std::string> m_hosts;

    struct SerStruct
    {
        uint16_t m_measurementOptions;
        uint8_t m_dualstack;
        uint8_t m_checkType;
        uint16_t m_port;
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
        uint8_t m_distributedFallback;
        uint32_t m_groupNameSize;
        uint32_t m_checkInfoSize;
        uint32_t m_numHosts;
        uint32_t m_remoteCheckSize;
        uint32_t m_totalHostSize;
        uint32_t m_numHostGroups;
        uint32_t m_totalHostGroupSize;
        HMIPAddress m_sourceAddress;
        uint8_t m_TOSValue;
        uint8_t m_DNSCheckPlugin;
        uint8_t m_flowType;
    };
};

#endif /* HMDATAHOSTGROUP_H_ */
