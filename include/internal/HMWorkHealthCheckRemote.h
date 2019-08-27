#ifndef HMWORKHEALTHCHECKREMOTE_H_
#define HMWORKHEALTHCHECKREMOTE_H_

// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <string>
#include <cstdint>

#include "HMWorkHealthCheck.h"

// LCOV_EXCL_START; Tested in functional testing
//! Health Check implementation to do a TCP connection.
/*!

 */
class HMWorkHealthCheckRemote : public HMWorkHealthCheck
{
public:
    HMWorkHealthCheckRemote(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        HMWorkHealthCheck(hostname, ip, hostcheck) {};

    //! Throw an error mesage to the work processing upon a critical TCP socket error.
    /*!
          Throw an error mesage to the work processing upon a critical TCP socket error.
          \param the string to include in the thrown error.
     */
    void errorMsg(std::string strBuf);


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

private:
    /*!
         Called to get the results from remote host.
         \param current state.
         \param remote host name.
         \param remote result data structure.
     */
    bool getHostResult(HMState& state, const std::string& remoteHost, HMDataCheckResult& remote);

    //! Called to get the connect and communicate from remote host.
    bool remoteCheck(HMState& state, HMDataHostGroup& dataHostGroup);
};

#endif /* HMWORKHEALTHCHECKREMOTE_H_ */
// LCOV_EXCL_STOP; Tested in functional testing 

