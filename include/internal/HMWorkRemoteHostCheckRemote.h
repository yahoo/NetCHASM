// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMWorkRemoteHostCheck.h"

#include <vector>
#include <string>
#include <set>

// LCOV_EXCL_START; Tested in functional testing

#ifndef HMWORKREMOTEHOSTCHECKREMOTE_H_
#define HMWORKREMOTEHOSTCHECKREMOTE_H_

class HMWorkRemoteHostCheckRemote : public HMWorkRemoteHostCheck
{

public:

    HMWorkRemoteHostCheckRemote(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& datahostcheck) :
        HMWorkRemoteHostCheck(hostname, ip, datahostcheck) {};

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
    HM_WORK_STATUS remoteLookup();

    bool updateResults();

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

#endif /* HMWORKREMOTEHOSTCHECKREMOTE_H_ */
// LCOV_EXCL_STOP; Tested in functional testing
