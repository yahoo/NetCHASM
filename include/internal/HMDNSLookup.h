// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDNSHOSTCHECK_H_
#define HMDNSHOSTCHECK_H_

#include <map>
#include <string>
#include <set>

#include "HMConstants.h"

class HMWorkQueue;
class HMEventLoop;
class HMCheckHeader;

//! The class to identify the type of DNS check type.
class HMDNSTypeMap
{
public:
    HMDNSTypeMap(std::string name, HM_DNS_PLUGIN_CLASS plugin) :
        m_name(name),
        m_plugin(plugin) {}
    bool operator<(const HMDNSTypeMap& k) const;
    bool operator!=(const HMDNSTypeMap& k) const;
    bool operator==(const HMDNSTypeMap& k) const;

private:
    std::string m_name;
    HM_DNS_PLUGIN_CLASS m_plugin;
};

class HMDNSLookup
{
public:
    HMDNSLookup() :
        m_plugin(HM_DNS_PLUGIN_ARES),
        m_ipv6(false) {}
    HMDNSLookup(bool ipv6) :
        m_plugin(HM_DNS_PLUGIN_ARES),
        m_ipv6(ipv6) {}
    HMDNSLookup(HM_DNS_PLUGIN_CLASS plugin) :
        m_plugin(plugin),
        m_ipv6(false) {}
    HMDNSLookup(HM_DNS_PLUGIN_CLASS plugin, bool ipv6) :
        m_plugin(plugin),
        m_ipv6(ipv6) {}

    bool operator<(const HMDNSLookup& k) const;
    bool operator!=(const HMDNSLookup& k) const;
    bool operator==(const HMDNSLookup& k) const;

    //! Set the check params.
    /*!
         Set the check params.
         \param structure containing host group params.
     */
    void setCheckParams(const HMDataHostGroup& dataHostGroup);

    //! Get the mode of DNS check.
    /*!
         Get the mode of DNS check (v4 or v6).
         \return the DNS check type.
     */
    bool isIpv6() const;
    //! Get the type of DNS check.
    /*!
         Get the type of DNS check for the health check.
         \return the DNS check type.
     */
    HM_DNS_PLUGIN_CLASS getPlugin() const;

private:
    HM_DNS_PLUGIN_CLASS m_plugin;
    bool m_ipv6;
};
#endif /* HMDNSHOSTCHECK_H_ */
