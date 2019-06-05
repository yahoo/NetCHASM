// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMWORKDNSLOOKUP_H_
#define HMWORKDNSLOOKUP_H_

#include "HMWork.h"

//! Base class for the work classes that conduct DNS resolutions.
/*!
     This class implements all the functionality to conduct and store DNS resolutions.
     All the logic to store the IP Addresses and pass them to the DNS cachec are in this class.

     The derived classes must implement how to actually conduct the DNS resolution.
     They must implement the following functions.
     init - do the initialization work for the class.
     dnsLookup - do the actual DNS lookup and update the hostname/IP Addresses.
     Return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
 */
class HMWorkDNSLookup: public HMWork
{
public:
    HMWorkDNSLookup(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        HMWork(hostname, ip, hostcheck) {};

    virtual ~HMWorkDNSLookup() {}

    //! Called to init and setup the work order.
    /*!
         Called to init and setup the work order.
         \param the work state to use for the work order.
     */
    virtual void init(HMWorkState& state) = 0;

    //! Main function to actually conduct the DNS resolution.
    /*!
         Main function to actually conduct the DNS resolution.
         Should set the the hostname/IP Addresses and result codes.
         \return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
     */
    virtual HM_WORK_STATUS dnsLookup() = 0;

    //! Called to get the work type.
    /*!
         Called to get the work type.
         \return the current HM_WORK_TYPE used to determine the appropriate processing task.
     */
    HM_WORK_TYPE getWorkType() { return HM_WORK_DNSLOOKUP; }

    //! Called to process the work in the work order.
    /*!
         Called by the worker thread to conduct the work.
         \return The return specifies how the work should be processed. In the case of HM_WORK_COMPLETE or HM_WORK_IDLE, the work order is complete and destructed.
         HM_WORK_IN_PROGRESS moves the work order to the work map in the queue. The work must schedule a callback to have the queue move the work from the map back into the active queue to finish the processing (continuation).
     */
    HM_WORK_STATUS processWork();

protected:
    std::set<std::string> m_hostnames;
    std::set<HMIPAddress> m_ips;
};

#endif /* HMWORKDNSLOOKUP_H_ */
