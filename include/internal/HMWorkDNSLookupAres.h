// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMDNSCache.h"
#include "HMWorkDNSLookup.h"

#include <vector>
#include <string>
#include <set>
#include "ares.h"

// LCOV_EXCL_START; Tested in functional testing

#ifndef HMWORKDNSLOOKUPARES_H_
#define HMWORKDNSLOOKUPARES_H_

class HMWorkDNSLookupAres : public HMWorkDNSLookup
{

public:

    HMWorkDNSLookupAres(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck, const HMDNSLookup& dnsHostCheck) :
        HMWorkDNSLookup(hostname, ip, hostcheck, dnsHostCheck),
        m_finished(false),
        m_channel(nullptr) {};

    //! Called to init and setup the work order.
    /*!
         Called to init and setup the work order.
         \param the work state to use for the work order.
     */
    void init(HMWorkState& state);

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

    //! Called from the Ares callback to indicate that the resolution lookup is done.
    void signalDone();

private:

    bool m_finished;
    ares_channel* m_channel;
};

#endif /* HMWORKDNSLOOKUPARES_H_ */
// LCOV_EXCL_STOP; Tested in functional testing
