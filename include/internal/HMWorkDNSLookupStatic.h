// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMDNSCache.h"
#include "HMWorkDNSLookup.h"

// LCOV_EXCL_START; Tested in functional testing

#ifndef HMWORKDNSLOOKUPSTATIC_H_
#define HMWORKDNSLOOKUPSTATIC_H_

class HMWorkDNSLookupStatic : public HMWorkDNSLookup
{

public:

    HMWorkDNSLookupStatic(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck, const HMDNSLookup& dnsHostCheck) :
        HMWorkDNSLookup(hostname, ip, hostcheck, dnsHostCheck) {};

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

private:

};

#endif /* HMWORKDNSLOOKUPSTATIC_H_ */
// LCOV_EXCL_STOP; Tested in functional testing
