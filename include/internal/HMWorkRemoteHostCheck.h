// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMWORKREMOTEHOSTCHECK_H_
#define HMWORKREMOTEHOSTCHECK_H_

#include "HMWork.h"
#include "HMHashMD5.h"
#include "HMDataHostGroup.h"

//! Base class for the work classes that conduct Remote host-group checks.
/*!
     This class implements all the functionality to conduct remote checks.
     All the logic to store the remote results.

     The derived classes must implement how to actually conduct the remote checks.
     They must implement the following functions.
     init - do the initialization work for the class.
     RemoteLookup - do the actual remote results fetch and update the results.
     Return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
 */

class HMGroupCheckResult;
class HMGroupAuxResult;
class HMHash;
class HMWorkRemoteHostCheck: public HMWork
{
public:
    HMWorkRemoteHostCheck(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& dataHostCheck) :
        HMWork(hostname, ip, dataHostCheck) { };

    virtual ~HMWorkRemoteHostCheck() {}

    //! Called to init and setup the work order.
    /*!
         Called to init and setup the work order.
         \param the work state to use for the work order.
     */
    virtual void init(HMWorkState& state) = 0;

    //! Main function to actually conduct the Remote checks.
    /*!
         Main function to actually conduct the Remote checks.
         \return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
     */
    virtual HM_WORK_STATUS remoteLookup() = 0;

    //! Called to get the work type.
    /*!
         Called to get the work type.
         \return the current HM_WORK_TYPE used to determine the appropriate processing task.
     */
    HM_WORK_TYPE getWorkType() { return HM_WORK_REMOTEHOSTCHECK; }

    //! Called to process the work in the work order.
    /*!
         Called by the worker thread to conduct the work.
         \return The return specifies how the work should be processed. In the case of HM_WORK_COMPLETE or HM_WORK_IDLE, the work order is complete and destructed.
         HM_WORK_IN_PROGRESS moves the work order to the work map in the queue. The work must schedule a callback to have the queue move the work from the map back into the active queue to finish the processing (continuation).
     */
    HM_WORK_STATUS processWork();

    //! Main function to actually conduct the results update.
    /*!
         Main function to actually conduct the results update.
         Should update the DNS cache and check list.
         \return status
     */
    bool updateResults();

    //!  Function to update results when data is not available.
    /*!
         Main function to actually conduct the results update.
         Should update the results with remote no data
         \return status
     */
    bool updateResultsNoData();

    //! Main function to actually conduct the aux results update.
    /*!
         Main function to actually conduct the aux results update.
         \return status
     */
    bool updateAuxResults();

protected:
    std::multimap<HMDataCheckParams, HMDataCheckResult> m_hostResults;
    std::vector<HMGroupAuxResult> m_auxResults;
};

#endif /* HMWORKREMOTEHOSTCHECK_H_ */
