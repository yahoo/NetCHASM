// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <cstring>

#include "HMDataHostCheck.h"
#include "HMDataHostGroup.h"
#include "HMLogBase.h"

using namespace std;

HMDataHostCheck::HMDataHostCheck(const HMAPIDataHostCheck& apiDataHostCheck)
{
    m_port = apiDataHostCheck.m_port;
    m_checkType = HM_CHECK_TYPE(apiDataHostCheck.m_checkType);
    m_checkInfo = apiDataHostCheck.m_checkInfo;
    m_dualstack = HM_DUALSTACK_UNDEFINED;
    if (apiDataHostCheck.m_ipv4)
    {
        m_dualstack = (HM_DUALSTACK) (m_dualstack | HM_DUALSTACK_IPV4_ONLY);
    }
    if (apiDataHostCheck.m_ipv6)
    {
        m_dualstack = (HM_DUALSTACK) (m_dualstack | HM_DUALSTACK_IPV6_ONLY);
    }
    m_checkPlugin = HM_CHECK_PLUGIN_DEFAULT;
    m_DNSPlugin = (HM_DNS_PLUGIN_CLASS)apiDataHostCheck.m_dnsCheckType;
    m_remoteCheckType = HM_REMOTE_CHECK_NONE;
    m_distributedFallback = HM_DISTRIBUTED_FALLBACK_NONE;
    m_sourceAddress.set(apiDataHostCheck.m_sourceAddress);
    m_remoteCheck = false;
    m_TOSValue = apiDataHostCheck.m_TOSValue;
}

bool
HMDataHostCheck::operator<(const HMDataHostCheck& k) const
{
	if(m_checkType == k.m_checkType)
	{
		if(m_port == k.m_port)
		{
			if(m_dualstack == k.m_dualstack)
			{
			    if(m_checkInfo == k.m_checkInfo)
			    {
			        if(m_remoteCheck == k.m_remoteCheck)
			        {
			            if(m_sourceAddress == k.m_sourceAddress)
			            {
			                if(m_TOSValue == k.m_TOSValue)
			                {
			                    return m_distributedFallback < k.m_distributedFallback;
			                }
			                return m_TOSValue < k.m_TOSValue;
			            }
			            return m_sourceAddress < k.m_sourceAddress;
			        }
			        return m_remoteCheck < k.m_remoteCheck;
			    }
				return m_checkInfo < k.m_checkInfo;
			}
			return m_dualstack < k.m_dualstack;
		}
		return m_port < k.m_port;
	}
	return m_checkType < k.m_checkType;
}

bool
HMDataHostCheck::operator!=(const HMDataHostCheck& k) const
{
    return !(*this == k);
}

bool
HMDataHostCheck::operator==(const HMDataHostCheck& k) const
{
    if(m_checkType == k.m_checkType
            && m_port == k.m_port
            && m_dualstack == k.m_dualstack
            && m_checkInfo == k.m_checkInfo
            && m_remoteCheck == k.m_remoteCheck
            && m_sourceAddress == k.m_sourceAddress
            && m_TOSValue == k.m_TOSValue
            && m_distributedFallback == k.m_distributedFallback)
    {
        return true;
    }
    return false;
}

void
HMDataHostCheck::setCheckParams(const HMDataHostGroup& dataHostGroup)
{
    m_checkType = dataHostGroup.getCheckType();
    m_port = dataHostGroup.getCheckPort();
    m_dualstack = dataHostGroup.getDualstack();
    m_checkInfo = dataHostGroup.getCheckInfo();
    m_remoteCheck = dataHostGroup.getRemoteCheck();
    m_checkPlugin = dataHostGroup.getCheckPlugin();
    m_remoteCheckType = dataHostGroup.getRemoteCheckType();
    m_distributedFallback = dataHostGroup.getDistributedFallback();
    m_sourceAddress = dataHostGroup.getSourceAddress();
    m_TOSValue = dataHostGroup.getTOSValue();
    m_DNSPlugin = dataHostGroup.getDnsCheckPlugin();
}

HM_CHECK_TYPE
HMDataHostCheck::getCheckType() const
{
    return m_checkType;
}

HM_REMOTE_CHECK_TYPE
HMDataHostCheck::getRemoteCheckType() const
{
    return m_remoteCheckType;
}

uint16_t
HMDataHostCheck::getPort() const
{
   return m_port;
}

HM_DUALSTACK
HMDataHostCheck::getDualStack() const
{
    return m_dualstack;
}

string
HMDataHostCheck::getCheckInfo() const
{
    return m_checkInfo;
}

HM_DISTRIBUTED_FALLBACK
HMDataHostCheck::getDistributedFallBack() const
{
    return m_distributedFallback;
}

HM_CHECK_PLUGIN_CLASS
HMDataHostCheck::getCheckPlugin() const
{
    return m_checkPlugin;
}

string
HMDataHostCheck::parseCheckInfo(const string& host, uint32_t& port, string& checkInfoHost)
{
    string hostname;
    port = m_port;

    if (m_checkInfo.find("<host>") != string::npos)
    {
        //check info //<host>/x or //<host>
        //if port is not 443 for checkType https
        if ((port != 443) && (m_checkType != HM_CHECK_HTTP) && (m_checkType != HM_CHECK_AUX_HTTP))
        {
            hostname = "Host: " + host + ":" + to_string((uint64_t)port);
        }
        else
        {
            hostname = "Host: " + host;
        }
    }
    else if (m_checkInfo.find("<host:port>") != string::npos)
    {
        //check info //<host:port>/x or //<host:port>
        hostname = "Host: " + host + ":" + to_string((uint64_t)port);
    }
    else
    {
        size_t portindex, index;
        index = m_checkInfo.find("/", 2);
        // checkinfo //xxx:port/ checkInfoHost = xxx:port
        checkInfoHost = m_checkInfo.substr(2, index - 2);
        if (((portindex = checkInfoHost.find(":")) == string::npos)
                && (port != 443)
                && (m_checkType != HM_CHECK_HTTP)
                && (m_checkType != (HM_CHECK_AUX_HTTP))
                && (m_checkType != (HM_CHECK_MARK_HTTP)))
        {
            // checkInfo //host/yyy
            hostname = "Host: " + checkInfoHost + ":" + to_string((uint64_t)port);
        }
        else
        {
            // checkInfo //host:port/yyy
            hostname = "Host: " + checkInfoHost;
            if ((m_checkType != HM_CHECK_HTTP) 
                && (m_checkType != HM_CHECK_AUX_HTTP) 
                && (m_checkType != HM_CHECK_MARK_HTTP))
            {
                //if checkinfo port is different than check port
                if ((portindex != string::npos) && checkInfoHost.substr(portindex + 1) != to_string(port))
                {
                    string portnum = checkInfoHost.substr(portindex + 1);
                    HMLog(HM_LOG_DEBUG3,
                            "[CURLCHECK] Check info port %s is different than check-port %d for %s with checkinfo %s",
                            portnum.c_str(),
                            (uint64_t)port,
                            host.c_str(),
                            m_checkInfo.c_str());
                    port = stoi(portnum);
                }
                checkInfoHost = checkInfoHost.substr(0, portindex);
            }
        }
    }
    return hostname;
}

string
HMDataHostCheck::printEntry(char delim, bool label) const
{
	stringstream mstream;

	if(label)
	{
	    mstream << "Check Type: ";
	}

	mstream << printCheckType(m_checkType)
	        << delim
	        << "Check Info: "
	        << m_checkInfo
	        << delim
	        << "Port: "
	        << m_port
	        << delim
	        << "Dual Stack: ";

	switch(m_dualstack)
	{
	case HM_DUALSTACK_UNDEFINED:
    case HM_DUALSTACK_IPV4_ONLY:
        mstream << "ipv4-only";
        break;
    case HM_DUALSTACK_BOTH:
        mstream << "both";
        break;
    case HM_DUALSTACK_IPV6_ONLY:
        mstream << "ipv6-only";
        break;
    }

	return mstream.str();
}

const std::string& HMDataHostCheck::getRemoteCheck() const
{
    return m_remoteCheck;
}

void HMDataHostCheck::setRemoteCheck(const std::string& remoteCheck)
{
    m_remoteCheck = remoteCheck;
}

const HMIPAddress& HMDataHostCheck::getSourceAddress() const
{
    return m_sourceAddress;
}

uint8_t HMDataHostCheck::getTOSValue() const
{
    return m_TOSValue;
}

HM_DNS_PLUGIN_CLASS HMDataHostCheck::getDnsPlugin() const
{
    return m_DNSPlugin;
}
