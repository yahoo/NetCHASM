// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDATAHOSTCHECK_H_
#define HMDATAHOSTCHECK_H_

#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>

#include "HMConstants.h"
#include "HMAPI.h"
#include "HMIPAddress.h"

class HMDataHostCheck
{
public:
    HMDataHostCheck() :
        m_checkType(HM_CHECK_DEFAULT),
        m_port(0),
        m_dualstack(HM_DUALSTACK_IPV4_ONLY),
        m_distributedFallback(HM_DISTRIBUTED_FALLBACK_NONE),
        m_DNSType(HM_DNS_TYPE_LOOKUP),
        m_checkPlugin(HM_CHECK_PLUGIN_DEFAULT),
        m_remoteCheckType(HM_REMOTE_CHECK_NONE),
        m_TOSValue(0),
        m_flowType(HM_FLOW_DNS_HEALTH_TYPE){};

    HMDataHostCheck(HM_DNS_TYPE dnsType) :
            m_checkType(HM_CHECK_DEFAULT),
            m_port(0),
            m_dualstack(HM_DUALSTACK_IPV4_ONLY),
            m_distributedFallback(HM_DISTRIBUTED_FALLBACK_NONE),
            m_DNSType(dnsType),
            m_checkPlugin(HM_CHECK_PLUGIN_DEFAULT),
            m_remoteCheckType(HM_REMOTE_CHECK_NONE),
            m_TOSValue(0),
            m_flowType(HM_FLOW_DNS_HEALTH_TYPE){};
	HMDataHostCheck(const HMAPIDataHostCheck&);
	bool operator<(const HMDataHostCheck& k) const;
	bool operator!=(const HMDataHostCheck& k) const;
	bool operator==(const HMDataHostCheck& k) const;

	//! Set the check params.
	/*!
	     Set the check params.
	     \param structure containing host group params.
	 */
	void setCheckParams(const HMDataHostGroup& dataHostGroup);

	//! Get the host check type code.
	/*!
	     Get the host check type code (HM_CHECK_TYPE)
	     \return HM_CHECK_TYPE the check type.
	 */
	HM_CHECK_TYPE getCheckType() const;

    //! Get the host remote check type code.
    /*!
         Get the host remote check type code (HM_REMOTE_CHECK_TYPE)
         \return HM_REMOTE_CHECK_TYPE the check type.
     */
    HM_REMOTE_CHECK_TYPE getRemoteCheckType() const;

	//! Get the port used for this check.
	/*!
	     Get the port used for this check.
	     \return the port to use in the check.
	 */
	uint16_t getPort() const;

	//! Get the dualstack setting for the host check.
	/*!
	     Get the dualstack setting for the host check.
	     \return the HM_DUALSTACK indicating whether to check IPv4, IPv6 or both.
	 */
	HM_DUALSTACK getDualStack() const;

	//! Get the check info used for the check.
	/*!
	     Get the check info used for the check.
	     \return the check info string.
	 */
	std::string getCheckInfo() const;

    //! Check mode of fallback for the remote check.
    /*!
         Fallback mechanism mode for remote check.
         \return retruns the mode of fallback.
     */
	HM_DISTRIBUTED_FALLBACK getDistributedFallBack() const;

	//! Get the plugin class used for this check.
	/*!
	     Get the plugin class used for this check.
	     \return HM_CHECK_PLUGIN_CLASS for the check.
	 */
	HM_CHECK_PLUGIN_CLASS getCheckPlugin() const;

	//! Parse the check info string and update the check host which is set in the hostname header.
	/*!
	     Parse the check info string and update the hostname that should be set in the request headers
	     The checkinfo string can contain a number of wildcards to override how the check header is formed.
	     //<host>, //<host:port> adds the current host and or port to the header.
	     \param hostname being checked.
	     \param the port being checked.
	     \param the check info in the check.
	     \return the hostname to set in the hostname header.
	 */
	std::string parseCheckInfo(const std::string& host, uint32_t& port, std::string& checkInfoHost);

	//! Output the host check parameters to a human readable string.
	/*!
	     Output the host check parameters to a human readable string.
	     \param a character to use as the delimiter between fields.
	     \param true to print the label to each field.
	     \return the human readable form of the host check.
	 */
	std::string printEntry(char delim, bool label) const;

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

    //! Get the source ip specified for the check.
    /*!
         Get the source ip specified for the check.
         \return the IP address.
     */
    const HMIPAddress& getSourceAddress() const;

    //! Get the type of service value.
    /*!
         Get the type of service specified for the check.
         \return the TOS value.
     */
    uint8_t getTOSValue() const;

    //! Get the type of DNS check.
    /*!
         Get the type of DNS check for the health check.
         \return the DNS check type.
     */
    HM_DNS_TYPE getDnsType() const;

    //! Get the type of flow type.
    /*!
         Get the type of flow type for the host group.
         \return the flow type.
     */
    HM_FLOW_TYPE getFlowType() const;

    //! Set the dns type used for the check.
    /*!
         Set the dns type used for the check.
         \param the dns plugin type.
     */
    void setDNSType(HM_DNS_TYPE dnsPlugin);

private:
    HM_CHECK_TYPE m_checkType;
    uint16_t m_port;
    HM_DUALSTACK m_dualstack;
    std::string m_checkInfo;
    std::string m_remoteCheck;
    HM_DISTRIBUTED_FALLBACK m_distributedFallback;
    HM_DNS_TYPE m_DNSType;
    HM_CHECK_PLUGIN_CLASS m_checkPlugin;
    HM_REMOTE_CHECK_TYPE m_remoteCheckType;
    HMIPAddress m_sourceAddress;
    uint8_t m_TOSValue;
    HM_FLOW_TYPE m_flowType;
};

#endif /* HMDATAHOSTCHECK_H_ */
