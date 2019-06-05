// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDATAHOSTCHECK_H_
#define HMDATAHOSTCHECK_H_

#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>

#include "HMConstants.h"

class HMDataHostGroup;
class HMDataHostCheck
{
public:
    HMDataHostCheck() :
        m_checkType(HM_CHECK_DEFAULT),
        m_port(0),
        m_dualstack(HM_DUALSTACK_IPV4_ONLY),
        m_checkPlugin(HM_CHECK_PLUGIN_DEFAULT) {};

	bool operator<(const HMDataHostCheck& k) const;
	bool operator!=(const HMDataHostCheck& k) const;
	bool operator==(const HMDataHostCheck& k) const;

	//! Set the check params.
	/*!
	     Set the check params.
	     \param the HM_CHECK_TYPE.
	     \param HM_CHECK_PLUGIN_CLASS the plugin class to service the check.
	     \param the port to use for the check.
	     \param HM_DUALSTACK to specify whether to check IPv4, IPv6 or both.
	     \param the check info to use for the check.
	 */
	void setCheckParams(HM_CHECK_TYPE checkType,
	                    HM_CHECK_PLUGIN_CLASS checkPlugin,
	                    uint16_t port,
	                    HM_DUALSTACK dualstack,
	                    const std::string& checkInfo);

	//! Get the host check type code.
	/*!
	     Get the host check type code (HM_CHECK_TYPE)
	     \return HM_CHECK_TYPE the check type.
	 */
	HM_CHECK_TYPE getCheckType() const;

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

	//! Function to serialize the current check result info into a raw buffer.
	/*!
	     The serialize function supports two types of calls designed to be called consecutively.
	     When called with a null buf and size 0 serialize will return the required size of the buf to store the host check.
	     When called with a non-null buf and the correct size, serialize will store the host check info into the buf.
	     \param buf pass nullptr to get the required size or a raw buffer to fill.
	     \param size pass 0 to get the required size or the required size to fill the buffer.
	     \return The required size of the buf or the number of bytes saved to the buffer.
	 */
	uint32_t serialize(char* buf, uint32_t size) const;
	//! De-serialize the raw buffer.
	/*!
	     This function is called to deserialize a host check info. It fills in the class data from the raw buffer.
	     \param buf raw buffer to deserialize.
	     \param size the size of the raw buffer.
	     \return true if the deserialize was a success.
	 */
	bool deserialize(char* buf, uint32_t size);

	//! Output the host check parameters to a human readable string.
	/*!
	     Output the host check parameters to a human readable string.
	     \param a character to use as the delimiter between fields.
	     \param true to print the label to each field.
	     \return the human readable form of the host check.
	 */
	std::string printEntry(char delim, bool label) const;


private:
	HM_CHECK_TYPE m_checkType;
	uint16_t m_port;
	HM_DUALSTACK m_dualstack;
	std::string m_checkInfo;
	HM_CHECK_PLUGIN_CLASS m_checkPlugin;

	struct SerStruct
	{
	    uint8_t m_checkType;
	    uint16_t m_port;
	    uint8_t m_dualStack;
	    uint32_t m_stringSize;
	};
};

#endif /* HMDATAHOSTCHECK_H_ */
