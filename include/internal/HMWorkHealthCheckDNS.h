// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMWORKHEALTHCHECKDNS_H_
#define HMWORKHEALTHCHECKDNS_H_

#include <string>
#include <cstdint>

#include "HMConstants.h"
#include "HMWorkHealthCheck.h"

// LCOV_EXCL_START; Tested in functional testing
//! Class to conduct a health check through DNS using the Ares Library
class HMWorkHealthCheckDNS : public HMWorkHealthCheck
{
public:
    HMWorkHealthCheckDNS(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        HMWorkHealthCheck(hostname, ip, hostcheck),
        m_finishCallback(false),
        m_ok(HM_WORK_IDLE) {};

    //! Main function to actually conduct the health check.
    /*!
         Main function to actually conduct the health check.
         Should set the start, end and response codes.
         \return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
     */
    HM_WORK_STATUS healthCheck();

    //! Used in the Ares Callback to signal the DNS resolution is done.
    void signalDone();

    //! Used in the Ares Callback to update the reason and error message string.
    /*!
         Used in the Ares Callback to update the reason and error message string.
         \param the REASON code from the Ares response.
         \param the error string for error reporting.
     */
    void status(HM_REASON ret, const std::string& msg);

    //! Called to init and setup the work order.
    /*!
         Called to init and setup the work order.
         \param the work state to use for the work order.
     */
    void init(HMWorkState& state)
    {
      // To suppress compiler warning
      (void)state;
    };

private:
    bool m_finishCallback;
    HM_WORK_STATUS m_ok;
};

#endif /* HMWORKHEALTHCHECKDNS_H_ */
// LCOV_EXCL_STOP; Tested in functional testing

