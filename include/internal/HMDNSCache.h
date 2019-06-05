// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDNSCACHE_H_
#define HMDNSCACHE_H_

#include <map>
#include <string>
#include <set>

#include "HMConstants.h"
#include "HMDNSResult.h"

class HMWorkQueue;
class HMEventLoop;

//! The class that stores all of the DNS resolutions.
class HMDNSCache
{
public:
    HMDNSCache() :
        m_dnsType(HM_DNS_PLUGIN_DEFAULT) {};
    HMDNSCache(HMDNSCache&) = delete;

    //! Initialize the DNS cache.
    /*!
         Initialize the DNS cache.
         \param the DNS plugin to use for all DNS resolutions.
         \return true if the DNS cache is ready.
     */
    bool init(uint32_t dnsType);

    //! Insert a blank entry into the DNS cache.
    /*!
         Insert a blank entry into the DNS cache.
         \param the name to resolve.
         \param true if we are checking IPV6.
         \param the time-to-live of the entry before requiring another lookup.
         \param the timeout of the resolution request.
     */
    void insertDNSEntry(const std::string& name, bool ipv6, uint64_t ttl, uint64_t timeout);

    //! Insert DNS resolutions during a reload into a new instance of the cache.
    /*!
         Insert DNS resolutions during a reload into a new instance of the cache. For the given name, copy over all IPv4 and IPv6 (if any) along with the correct timeouts.
         \param the resolved hostname.
         \param the set of all IP addresses resolved to the host. This contains both the IPv4 and IPv6 IPs
         \param the timeouts associated with the last V4 resolution.
         \param the timeouts associated with the last V6 resolution.
     */
    void updateReloadDNSEntry(const std::string& name, std::set<HMIPAddress>& addresses, const HMDNSResult& v4Result, const HMDNSResult& v6Result);

    //! Update a single DNS entry after resolving the name.
    /*!
         Update a single DNS entry after resolving the name.
         \param the resolved name.
         \param true if this was resolved for IPv6
         \param the set of IPs resolved for this name.
     */
    void updateDNSEntry(std::string name, bool ipv6, std::set<HMIPAddress>& addresses);

    //! Finish the DNS query.
    /*!
         Finish the DNS query. Sets the internal state to either HM_CHECK_INACTIVE or HM_CHECK_FAILED depending on success.
         \param the resolved name.
         \param true if we are resolving IPv6.
         \param true if the check was a success.
     */
    void finishQuery(std::string name, bool ipv6, bool success);

    //! Get the Addresses for a given host from the cache.
    /*!
         Get the Addresses for a given host from the cache. Returns IPv4,IPv6 or both. Only returns entries that are not expired.
         \param the resolved name to fetch the addresses.
         \param the dualstack setting one of IPv4, IPv6 or both.
         \param set of HMIPAddress. Will be filled with the addresses for the given name.
         \return true if the set contains the IPAddresses.
     */
    bool getAddresses(const std::string& name, uint8_t dualstack, std::set<HMIPAddress>& vaddress) const;

    //! Get the Addresses for a given host from the cache.
    /*!
         Get the Addresses for a given host from the cache. Returns IPv4,IPv6 or both. Returns expired entries.
         \param the resolved name to fetch the addresses.
         \param the dualstack setting one of IPv4, IPv6 or both.
         \param set of HMIPAddress. Will be filled with the addresses for the given name.
         \return true if the set contains the IPAddresses.
     */
    bool getExpiredAddresses(const std::string& name, uint8_t dualstack, std::set<HMIPAddress>& vaddress) const;

    //! Get the full DNS result for the given hostname.
    /*!
         Get the full DNS result for the given hostname. This includes the check status and timeout.
         \param the resolved name to fetch the info.
         \param true to fetch the IPv6 entries.
         \param a constant iterator that will point to the entry in the map.
         \return true if the entry was found and stored in result.
     */
    bool getDNSResult(const std::string& name, bool ipv6, std::map<std::pair<std::string,bool>,HMDNSResult>::const_iterator& result) const;

    //! Check to see if we should schedule a DNS resolution for the given lookup.
    /*!
         Check to see if we should schedule a DNS resolution for the given lookup.
         \param the hostname to resolve.
         \param true if we should resolve IPv6
         \return the schedule state. Either queue work, a timeout event or ignore.
     */
    HM_SCHEDULE_STATE queryNeeded(const std::string& name, bool ipv6) const;

    //! Get the next resolution time based on the timeout.
    /*!
         Get the next resolution time based on the timeout
         \param the host to resolve.
         \param true to resolve IPv6.
         \return the HMTimeStamp of the next time to schedule a DNS resolution.
     */
    HMTimeStamp nextQueryTime(const std::string& name, bool ipv6) const;

    //! Start aDNS query.
    /*!
         Start a DNS query. Update the internal query state to in progress.
         \param the host name to resolve.
         \param true if we are resolving IPv6.
         \return the HMTimstamp of the timeout for the query.
     */
    HMTimeStamp startDNSQuery(const std::string& name, bool ipv6);

    //! Queue the DNS query.
    /*!
         Queue the DNS query. Set the query state to Queued.
         \param the hostname to resolve.
         \param true to resolve IPv6.
         \param the work queue to insert the DNS resolution work.
     */
    void queueDNSQuery(std::string name, bool ipv6, HMWorkQueue& queue);

    //! Queue all the DNS lookups.
    /*!
         Queue all the DNS lookups. Called at cold start to get all the hostnames resolved.
         \param the work queue to insert the resolutions.
         \param the event loop to insert any needed timeouts for DNS resolutions that were loaded from storage.
         \param true if this is a restart during a running daemon config reload.
     */
    void queueDNSLookups(HMWorkQueue& queue, HMEventLoop& eventLoop, bool restart);

    //! Get the DNS plugin used for the DNS resolutions.
    /*!
        Get the DNS plugin used for the DNS resolutions.
        \return the DNS class code.
     */
    uint32_t getDNSCheckPluginType() const;

    //! Check to see if the given IPAddress has been resolved for the hostname.
    /*!
         Check to see if the given IPAddress has been resolved for the hostname.
         \param the hostname to resolve.
         \param the dualstack to specify IPv4, IPv6 of both.
         \param the IP Address to check.
         \return true if the address is valid in the cache.
     */
    bool isValidAddress(const std::string& name, uint8_t dualstack, HMIPAddress& address) const;

private:
    std::map<std::pair<std::string,bool>,HMDNSResult> m_cache;

    uint32_t m_dnsType;
};

#endif /* HMDNSCACHE_H_ */
