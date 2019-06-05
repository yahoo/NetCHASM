// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMWORKHEALTHCHECK_H_
#define HMWORKHEALTHCHECK_H_

#include "HMWork.h"

//! Base Class for the work classes which conduct health checks.
/*!
     This class implements all the functionality to conduct and store the health check information.
     All logic to conduct, maintain the state, update the values and maintain timing and schedule the next check
     is done in the processWork function.

     The derived classes must implement how to actually make the network call and update the start, end and result codes.

     They must implement the following functions:
     healthCheck - Conduct the health check. Upon completion fill in the start, end and response codes.
     Return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
     init - convenience function to init any needed state.
 */
class HMWorkHealthCheck: public HMWork
{
public:
    HMWorkHealthCheck(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        HMWork(hostname, ip, hostcheck) {};

    virtual ~HMWorkHealthCheck() {};

    //! Main function to actually conduct the health check.
    /*!
         Main function to actually conduct the health check.
         Should set the start, end and response codes.
         \return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
     */
    virtual HM_WORK_STATUS healthCheck() = 0;

    //! Called to init and setup the work order.
    /*!
         Called to init and setup the work order.
         \param the work state to use for the work order.
     */
    virtual void init(HMWorkState& state) = 0;

    //! Called to get the work type.
    /*!
         Called to get the work type.
         \return the current HM_WORK_TYPE used to determine the appropriate processing task.
     */
    HM_WORK_TYPE getWorkType() { return HM_WORK_HEALTHCHECK; }

    //! Called to process the work in the work order.
    /*!
         Called by the worker thread to conduct the work.
         \return The return specifies how the work should be processed. In the case of HM_WORK_COMPLETE or HM_WORK_IDLE, the work order is complete and destructed.
         HM_WORK_IN_PROGRESS moves the work order to the work map in the queue. The work must schedule a callback to have the queue move the work from the map back into the active queue to finish the processing (continuation).
     */
    HM_WORK_STATUS processWork();
};

#endif /* HMWORKHEALTHCHECK_H_ */
