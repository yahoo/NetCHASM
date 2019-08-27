// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <chrono>
#include "ares.h"

#include "HMDataHostGroup.h"
#include "HMDNSLookup.h"

using namespace std;

bool
HMDNSLookup::operator <(const HMDNSLookup& k) const
{
    if (m_plugin == k.m_plugin)
    {
        return m_ipv6 < k.m_ipv6;
    }
    return m_plugin < k.m_plugin;
}

bool
HMDNSLookup::operator !=(const HMDNSLookup& k) const
{
    return !(*this == k);
}

bool
HMDNSLookup::operator ==(const HMDNSLookup& k) const
{
    if(m_plugin == k.m_plugin
            && m_ipv6 == k.m_ipv6)
    {
        return true;
    }
    return false;
}


void
HMDNSLookup::setCheckParams(const HMDataHostGroup& dataHostGroup)
{
    m_plugin = dataHostGroup.getDnsCheckPlugin();
    m_ipv6 = dataHostGroup.getDualstack() == AF_INET6;
}

bool
HMDNSLookup::isIpv6() const
{
    return m_ipv6;
}

HM_DNS_PLUGIN_CLASS
HMDNSLookup::getPlugin() const
{
    return m_plugin;
}

bool HMDNSTypeMap::operator <(const HMDNSTypeMap& k) const
{
    if (m_name == k.m_name)
    {
        return m_plugin < k.m_plugin;
    }
    return m_name < k.m_name;
}

bool HMDNSTypeMap::operator !=(const HMDNSTypeMap& k) const
{
    return !(*this == k);
}

bool HMDNSTypeMap::operator ==(const HMDNSTypeMap& k) const
{
    if (m_name == k.m_name && m_plugin == k.m_plugin)
    {
        return true;
    }
    return false;
}
