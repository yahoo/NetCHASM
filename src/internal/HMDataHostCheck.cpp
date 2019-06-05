// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <cstring>

#include "HMDataHostCheck.h"
#include "HMLogBase.h"

using namespace std;

bool
HMDataHostCheck::operator<(const HMDataHostCheck& k) const
{
	if(m_checkType == k.m_checkType)
	{
		if(m_port == k.m_port)
		{
			if(m_dualstack == k.m_dualstack)
			{
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
            && m_checkInfo == k.m_checkInfo)
    {
        return true;
    }
    return false;
}

void
HMDataHostCheck::setCheckParams(HM_CHECK_TYPE checkType,
                                HM_CHECK_PLUGIN_CLASS checkPlugin,
                                uint16_t port,
                                HM_DUALSTACK dualstack,
                                const string& checkInfo)
{
    m_checkType = checkType;
    m_port = port;
    m_dualstack = dualstack;
    m_checkInfo = checkInfo;
    m_checkPlugin = checkPlugin;
}

HM_CHECK_TYPE
HMDataHostCheck::getCheckType() const
{
    return m_checkType;
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
                && (m_checkType != (HM_CHECK_AUX_HTTP)))
        {
            // checkInfo //host/yyy
            hostname = "Host: " + checkInfoHost + ":" + to_string((uint64_t)port);
        }
        else
        {
            // checkInfo //host:port/yyy
            hostname = "Host: " + checkInfoHost;
            if ((m_checkType != HM_CHECK_HTTP) && (m_checkType != HM_CHECK_AUX_HTTP))
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

uint32_t
HMDataHostCheck::serialize(char* buf, uint32_t size) const
{
    if(buf == nullptr || size < (sizeof(SerStruct) + m_checkInfo.size()))
    {
        return sizeof(SerStruct) + m_checkInfo.size();
    }

    SerStruct* ptr = (SerStruct*)buf;

    ptr->m_checkType = m_checkType;
    ptr->m_port = m_port;
    ptr->m_dualStack = m_dualstack;
    ptr->m_stringSize = m_checkInfo.size();
    strncpy(buf + sizeof(SerStruct), &m_checkInfo.at(0), m_checkInfo.size());

    return sizeof(SerStruct) + m_checkInfo.size();
}

bool
HMDataHostCheck::deserialize(char* buf, uint32_t size)
{
    if(buf == nullptr || size < sizeof(SerStruct))
    {
        return false;
    }

    SerStruct* ptr = (SerStruct*)buf;
    m_checkType = HM_CHECK_TYPE(ptr->m_checkType);
    m_port = ptr->m_port;
    m_dualstack = (HM_DUALSTACK)ptr->m_dualStack;
    uint32_t stringsize = ptr->m_stringSize;

    if(size < sizeof(SerStruct) + stringsize)
    {
        return false;
    }

    m_checkInfo.resize(stringsize);
    strncpy(&m_checkInfo.at(0), buf + sizeof(SerStruct), stringsize);

    return true;
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
