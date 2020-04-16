// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <chrono>
#ifdef USE_ARES 
#include "ares.h"
#endif

#include "HMDataHostGroup.h"
#include "HMDNSLookup.h"

using namespace std;

bool
HMDNSLookup::operator <(const HMDNSLookup& k) const
{
    if (m_type == k.m_type)
    {
        if(m_remoteCheckGroup == k.m_remoteCheckGroup)
        {
            return m_ipv6 < k.m_ipv6;
        }
        return m_remoteCheckGroup < k.m_remoteCheckGroup;
    }
    return m_type < k.m_type;
}

bool
HMDNSLookup::operator !=(const HMDNSLookup& k) const
{
    return !(*this == k);
}

bool
HMDNSLookup::operator ==(const HMDNSLookup& k) const
{
    if(m_type == k.m_type
            && m_ipv6 == k.m_ipv6)
    {
        return true;
    }
    return false;
}


void
HMDNSLookup::setCheckParams(const HMDataHostGroup& dataHostGroup)
{
    m_type = dataHostGroup.getDNSType();
    m_ipv6 = dataHostGroup.getDualstack() == AF_INET6;
}

bool
HMDNSLookup::isIpv6() const
{
    return m_ipv6;
}

HM_DNS_TYPE
HMDNSLookup::getType() const
{
    return m_type;
}

bool HMDNSTypeMap::operator <(const HMDNSTypeMap& k) const
{
    if (m_name == k.m_name)
    {
        return m_type < k.m_type;
    }
    return m_name < k.m_name;
}

bool HMDNSTypeMap::operator !=(const HMDNSTypeMap& k) const
{
    return !(*this == k);
}

bool HMDNSTypeMap::operator ==(const HMDNSTypeMap& k) const
{
    if (m_name == k.m_name && m_type == k.m_type)
    {
        return true;
    }
    return false;
}

const std::string& HMDNSLookup::getRemoteCheckGroup() const
{
    return m_remoteCheckGroup;
}

HM_DNS_PLUGIN_CLASS HMDNSLookup::getPlugin() const
{
    return m_plugin;
}

void HMDNSLookup::setPlugin(HM_DNS_PLUGIN_CLASS plugin)
{
    m_plugin = plugin;
}
