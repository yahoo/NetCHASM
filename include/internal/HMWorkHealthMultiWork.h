// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMWORKHEALTHMULTIWORK_H_
#define HMWORKHEALTHMULTIWORK_H_

#include <string>
#include <cstdint>

#include "HMWorkHealthCheck.h"

// LCOV_EXCL_START; Tested in functional testing

//! Class to implement queuing up of multiple health checks.
class HMWorkHealhMultiWork : public HMWorkHealthCheck
{
public:
    HMWorkHealhMultiWork(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        HMWorkHealthCheck(hostname, ip, hostcheck) {};


    //! Main function to actually conduct the health check.
    /*!
         Main function to actually conduct the health check.
         Should set the start, end and response codes.
         \return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
     */
    HM_WORK_STATUS healthCheck();

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

};

#endif /* HMWORKHEALTHMULTIWORK_H_ */
// LCOV_EXCL_STOP; Tested in functional testing
