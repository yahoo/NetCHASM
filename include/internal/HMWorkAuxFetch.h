// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HMWORKAUXFETCH_H_
#define INCLUDE_HMWORKAUXFETCH_H_

#include "HMWork.h"

//! Base class for the Work classes which fetch Auxiliary information
/*!
  This class implements all the functionality to store auxiliary information (LFB, OOB)
  All logic to conduct, maintain the state, update the values and maintain timing and schedule the next check
  is done in the processWork function.

  The only piece missing is the code that actually conducts the check and places the
  retrieved values in m_auxData.

  Base classes need to override the following two functions:
  init() - called when the work is created to do any optional init functions
  fetchAux() - called to conduct the actual check. Should store the check info into m_auxData and set:
  HM_RESPONSE_CONNECTED and HM_REASON_SUCCESS if success or
  HM_RESPONSE_FAILED with a failure reason if the check did not succeed.
*/

class HMWorkAuxFetch : public HMWork
{
public:
    HMWorkAuxFetch(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
         HMWork(hostname, ip, hostcheck), m_auxDataType(HM_AUX_DATA_XML) {};

     virtual ~HMWorkAuxFetch() {};

     //! Called to conduct the actual check.
     /*!
          Called to conduct the actual check.  Should store the check info into m_auxData.
          set: HM_RESPONSE_CONNECTED and HM_REASON_SUCCESS if success or
          HM_RESPONSE_FAILED with a failure reason if the check did not succeed.
          \return The HM_WORK_STATUS to determine if the work is complete or needs a continuation.
      */
     virtual HM_WORK_STATUS fetchAux() = 0;

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
     HM_WORK_TYPE getWorkType() { return HM_WORK_AUXFETCH; }

     //! Called to process the work in the work order.
     /*!
          Called by the worker thread to conduct the work.
          \return The return specifies how the work should be processed. In the case of HM_WORK_COMPLETE or HM_WORK_IDLE, the work order is complete and destructed.
          HM_WORK_IN_PROGRESS moves the work order to the work map in the queue. The work must schedule a callback to have the queue move the work from the map back into the active queue to finish the processing (continuation).
      */
     HM_WORK_STATUS processWork();

     //! Called to get the aux data type.
         /*!
          Called to get the aux data type.
          \return the current HM_AUX_DATA_TYPE used to determine the aux data received.
     */
     HM_AUX_DATA_TYPE getAuxDataType() const { return m_auxDataType; }

     //! Called to set the aux data type.
         /*!
          Called to set the aux data type.
          \param the current HM_AUX_DATA_TYPE used to set the aux data type.
     */
     void setAuxDataType(HM_AUX_DATA_TYPE auxDataType) { m_auxDataType = auxDataType; }

protected:
     std::string m_auxData;

private:
     HM_AUX_DATA_TYPE m_auxDataType;
};

#endif /* INCLUDE_HMWORKAUXFETCH_H_ */
