// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDNSCACHE_H_
#define HMDNSCACHE_H_

#include <map>
#include <string>
#include <set>
#include <mutex>
#include <shared_mutex>
#include "HMConstants.h"
#include "HMDNSResult.h"
#include "HMDNSLookup.h"

class HMWorkQueue;
class HMEventLoop;
class HMCheckHeader;

//! The class that stores all of the DNS resolutions.
class HMDNSCache
{
public:
    HMDNSCache() {};
    HMDNSCache(HMDNSCache&) = delete;

    //! Initialize the DNS cache.
    /*!
         Initialize the DNS cache.
         \return true if the DNS cache is ready.
     */
    bool init();

    //! Insert a blank entry into the DNS cache.
    /*!
         Insert a blank entry into the DNS cache.
         \param the name to resolve.
         \param structure holding DNS type and address type(v4 or v6).
         \param the time-to-live of the entry before requiring another lookup.
         \param the timeout of the resolution request.
     */
    void insertDNSEntry(const std::string& name, HMDNSLookup& dnsHostCheck, uint64_t ttl, uint64_t timeout);

    //! Insert DNS resolutions during a reload into a new instance of the cache.
    /*!
         Insert DNS resolutions during a reload into a new instance of the cache. For the given name, copy over all IPv4 and IPv6 (if any) along with the correct timeouts.
         \param the resolved hostname.
         \param the set of all IP addresses resolved to the host. This contains both the IPv4 and IPv6 IPs
         \param the timeouts associated with the last V4 resolution.
         \param the timeouts associated with the last V6 resolution.
         \param type of DNS lookup.
     */
    void updateReloadDNSEntry(const std::string& name, std::set<HMIPAddress>& addresses, const HMDNSResult& v4Result, const HMDNSResult& v6Result, HM_DNS_PLUGIN_CLASS plugin);

    //! Update a single DNS entry after resolving the name.
    /*!
         Update a single DNS entry after resolving the name.
         \param the resolved name.
         \param structure holding DNS type and address type(v4 or v6).
         \param the set of IPs resolved for this name.
     */
    void updateDNSEntry(std::string name, HMDNSLookup& dnsHostCheck, std::set<HMIPAddress>& addresses);

    //! Finish the DNS query.
    /*!
         Finish the DNS query. Sets the internal state to either HM_CHECK_INACTIVE or HM_CHECK_FAILED depending on success.
         \param the resolved name.
         \param structure holding DNS type and address type(v4 or v6).
         \param true if the check was a success.
     */
    void finishQuery(std::string name, HMDNSLookup& dnsHostCheck, bool success);

    //! Get the Addresses for a given host from the cache.
    /*!
         Get the Addresses for a given host from the cache. Returns IPv4,IPv6 or both. Only returns entries that are not expired.
         \param the resolved name to fetch the addresses.
         \param the dualstack setting one of IPv4, IPv6 or both.
         \param structure holding DNS type.
         \param set of HMIPAddress. Will be filled with the addresses for the given name.
         \return true if the set contains the IPAddresses.
     */
    bool getAddresses(const std::string& name, uint8_t dualstack, const HMDNSLookup& dnsHostCheck, std::set<HMIPAddress>& vaddress) const;

    //! Get the Addresses for a given host from the cache.
    /*!
         Get the Addresses for a given host from the cache. Returns IPv4,IPv6 or both. Returns expired entries.
         \param the resolved name to fetch the addresses.
         \param the dualstack setting one of IPv4, IPv6 or both.
         \param structure holding DNS type.
         \param set of HMIPAddress. Will be filled with the addresses for the given name.
         \return true if the set contains the IPAddresses.
     */
    bool getExpiredAddresses(const std::string& name, uint8_t dualstack, const HMDNSLookup& dnsHostCheck, std::set<HMIPAddress>& vaddress) const;

    //! Get the full DNS result for the given hostname.
    /*!
         Get the full DNS result for the given hostname. This includes the check status and timeout.
         \param the resolved name to fetch the info.
         \param structure holding DNS type and address type(v4 or v6).
         \param a constant iterator that will point to the entry in the map.
         \return true if the entry was found and stored in result.
     */
    bool getDNSResult(const std::string& name, const HMDNSLookup& dnsHostCheck, std::map<std::pair<std::string,HMDNSLookup>,HMDNSResult>::const_iterator& result) const;

    //! Check to see if we should schedule a DNS resolution for the given lookup.
    /*!
         Check to see if we should schedule a DNS resolution for the given lookup.
         \param the hostname to resolve.
         \param structure holding DNS type and address type(v4 or v6).
         \return the schedule state. Either queue work, a timeout event or ignore.
     */
    HM_SCHEDULE_STATE queryNeeded(const std::string& name, const HMDNSLookup& dnsHostCheck) const;

    //! Get the next resolution time based on the timeout.
    /*!
         Get the next resolution time based on the timeout
         \param the host to resolve.
         \param structure holding DNS type and address type(v4 or v6).
         \return the HMTimeStamp of the next time to schedule a DNS resolution.
     */
    HMTimeStamp nextQueryTime(const std::string& name, const HMDNSLookup& dnsHostCheck) const;

    //! Start aDNS query.
    /*!
         Start a DNS query. Update the internal query state to in progress.
         \param the host name to resolve.
         \param structure holding DNS type and address type(v4 or v6).
         \return the HMTimstamp of the timeout for the query.
     */
    HMTimeStamp startDNSQuery(const std::string& name, HMDNSLookup& dnsHostCheck);

    //! Queue the DNS query.
    /*!
         Queue the DNS query. Set the query state to Queued.
         \param the hostname to resolve.
         \param structure holding DNS type and address type(v4 or v6).
         \param the work queue to insert the DNS resolution work.
     */
    void queueDNSQuery(std::string name, HMDNSLookup& dnsHostCheck, HMWorkQueue& queue);

    //! Queue all the DNS lookups.
    /*!
         Queue all the DNS lookups. Called at cold start to get all the hostnames resolved.
         \param the work queue to insert the resolutions.
         \param the event loop to insert any needed timeouts for DNS resolutions that were loaded from storage.
         \param true if this is a restart during a running daemon config reload.
     */
    void queueDNSLookups(HMWorkQueue& queue, HMEventLoop& eventLoop, bool restart);

    //! Check to see if the given IPAddress has been resolved for the hostname.
    /*!
         Check to see if the given IPAddress has been resolved for the hostname.
         \param the hostname to resolve.
         \param the dualstack to specify IPv4, IPv6 of both.
         \param structure holding DNS type.
         \param the IP Address to check.
         \return true if the address is valid in the cache.
     */
    bool isValidAddress(const std::string& name, uint8_t dualstack, const HMDNSLookup& dnsHostCheck, HMIPAddress& address) const;

    //! Get address for a particular host in static DNS.
    /*!
         Get address for a particular host in static DNS.
         \param the hostname to resolve.
         \param true if ipv6.
         \param set to store addresses.
         \return true if successful.
     */
    bool getStaticDNSAddress(const std::string& host, bool ipv6, std::set<HMIPAddress>& addresses);

    //! Remove addresses from a particular host in static DNS.
    /*!
         Remove addresses from a particular host in static DNS.
         \param the hostname to remove addresses.
         \param set containing addresses to remove.
         \return true if removed successful
     */
    bool removeStaticDNSAddress(const std::string& host, std::set<HMIPAddress>& address);

    //! Add addresses to a particular host in static DNS.
    /*!
         Add addresses to a particular host in static DNS.
         \param the hostname to add adddesses.
         \param set contining addresses to add.
         \return true if the address is valid in the cache.
     */
    bool addStaticDNSAddress(const std::string& host, std::set<HMIPAddress>& addresses);

    //! Get static DNS map.
    /*!
         Get static DNS map.
         \return map containing the static DNS entries.
     */
    const std::map<std::pair<std::string, bool>, std::set<HMIPAddress> >& getStaticDns() const;

    //! Overwrite static DNS map.
    /*!
         Overwrite static DNS map.
         \param map containing the static DNS entries.
     */
    void setStaticDns(const std::map<std::pair<std::string, bool>, std::set<HMIPAddress> >& staticDns);

private:
    std::map<std::pair<std::string, HMDNSLookup>,HMDNSResult> m_cache;
    std::map<std::pair<std::string,bool>, std::set<HMIPAddress>> m_staticDNS;
    std::shared_timed_mutex m_staticDNSMutex;
};

#endif /* HMDNSCACHE_H_ */
