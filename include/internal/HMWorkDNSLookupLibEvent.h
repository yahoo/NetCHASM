// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMWORKDNSLOOKUPLIBEVENT_H_
#define HMWORKDNSLOOKUPLIBEVENT_H_

#include <vector>
#include <string>
#include <set>

#include "HMDNSCache.h"
#include "HMWorkDNSLookup.h"

// LCOV_EXCL_START; Tested in functional testing
class HMWorkDNSLookupLibEvent : public HMWorkDNSLookup
{

public:

    HMWorkDNSLookupLibEvent(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        HMWorkDNSLookup(hostname, ip, hostcheck) {};

    //! Called to init and setup the work order.
    /*!
         Called to init and setup the work order.
         \param the work state to use for the work order.
     */
    void init(HMWorkState& state) { (void)state; }

    //! Main function to actually conduct the DNS resolution.
    /*!
         Main function to actually conduct the DNS resolution.
         Should set the the hostname/IP Addresses and result codes.
         \return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
     */
    HM_WORK_STATUS dnsLookup();

    //! Called from the Ares callback to add an IP address to the set of resolved IP Addresses.
    /*!
         Called from the Ares callback to add an IP address to the set of resolved IP Addresses.
         \param the IP Address to add.
     */
    void addIP(HMIPAddress& ip);

    //! The LibEvent callback upon completion of a DNS resolution. Update the results and moves the work order to the active work queue.
    /*!
         The LibEvent callback upon completion of a DNS resolution.
         \param the result of the LibEvent lookup.
         \param the type IPv4 or IPv6 of the resolution.
         \param the number of IPs in the record.
         \param the ttl of the lookup.
         \param the array of IP Addresses from the lookup
         \param the pointer to the work order to update with the IP Addresses.
     */
    static void libEventDNSCallback(int result, char type, int count, int ttl, void* addrs, void* arg);
};

#endif /* HMWORKDNSLOOKUPARES_H_ */
// LCOV_EXCL_STOP; Tested in functional testing
