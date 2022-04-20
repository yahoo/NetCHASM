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
    HMDNSTypeMap(std::string name, HM_DNS_TYPE type, const std::string& remoteCheck) :
        m_name(name),
        m_type(type),
        m_remoteCheckGroup(remoteCheck) {}
    bool operator<(const HMDNSTypeMap& k) const;
    bool operator!=(const HMDNSTypeMap& k) const;
    bool operator==(const HMDNSTypeMap& k) const;

private:
    std::string m_name;
    HM_DNS_TYPE m_type;
    std::string m_remoteCheckGroup;
};

class HMDNSLookup
{
public:
    HMDNSLookup() :
        m_type(HM_DNS_TYPE_LOOKUP),
        m_ipv6(false),
        m_plugin(HM_DNS_PLUGIN_NONE){}
    HMDNSLookup(HM_DNS_TYPE type) :
        m_type(type),
        m_ipv6(false),
        m_plugin(HM_DNS_PLUGIN_NONE){}
    HMDNSLookup(HM_DNS_TYPE type, const std::string& remoteCheck) :
        m_type(type),
        m_ipv6(false),
        m_remoteCheckGroup(remoteCheck),
        m_plugin(HM_DNS_PLUGIN_NONE) {}
    HMDNSLookup(HM_DNS_TYPE type, bool ipv6) :
        m_type(type),
        m_ipv6(ipv6),
        m_plugin(HM_DNS_PLUGIN_NONE) {}
    HMDNSLookup(HM_DNS_TYPE type, HM_DNS_PLUGIN_CLASS plugin, bool ipv6) :
       m_type(type),
       m_ipv6(ipv6),
       m_plugin(plugin) {}
    HMDNSLookup(HM_DNS_TYPE type, bool ipv6, const std::string& remoteCheck) :
        m_type(type),
        m_ipv6(ipv6),
        m_remoteCheckGroup(remoteCheck),
        m_plugin(HM_DNS_PLUGIN_NONE){}

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
    HM_DNS_TYPE getType() const;

    //! Get the remote check.
    /*!
         Get the remote check for the health check.
         \return the remote check host-group.
     */

    const std::string& getRemoteCheckGroup() const;

    //! Get the type of DNS plugin.
    /*!
         Get the type of DNS plugin.
         \return the DNS check plugin.
     */
    HM_DNS_PLUGIN_CLASS getPlugin() const;

    //! Set the type of DNS plugin.
    /*!
         Set the type of DNS plugin.
         \param the DNS check plugin.
     */
    void setPlugin(HM_DNS_PLUGIN_CLASS plugin);

private:
    HM_DNS_TYPE m_type;
    bool m_ipv6;
    std::string m_remoteCheckGroup;
    HM_DNS_PLUGIN_CLASS m_plugin;
};
#endif /* HMDNSHOSTCHECK_H_ */
