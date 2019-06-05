// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMWORKAUXFETCHCURL_H_
#define HMWORKAUXFETCHCURL_H_

#include "HMWorkAuxFetch.h"

//! Curl implementation to retrieve auxiliary information for NetCHASM
/*!
     This class implements the auxiliary information retrieval using CURL.
 */
class HMWorkAuxFetchCurl : public HMWorkAuxFetch
{
public:
    HMWorkAuxFetchCurl(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        HMWorkAuxFetch(hostname, ip, hostcheck) {};

    //! Callback function to update the receive buffer as data comes into CURL.
    /*!
          Callback function to update the receive buffer as data comes into CURL.
          \param buf of data received from the CURL callback.
          \param the length of the buffer.
     */
    void updateBuffer(char* buf, uint32_t length);

    //! Called to conduct the actual check.
    /*!
         Called to conduct the actual check.  Should store the check info into m_auxData.
         set: HM_RESPONSE_CONNECTED and HM_REASON_SUCCESS if success or
         HM_RESPONSE_FAILED with a failure reason if the check did not succeed.
         \return The HM_WORK_STATUS to determine if the work is complete or needs a continuation.
     */
    HM_WORK_STATUS fetchAux();

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
    std::string rcvdBuffer;
};

#endif /* HMWORKAUXFETCHCURL_H_ */
