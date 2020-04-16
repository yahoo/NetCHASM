// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMREMOTERESULT_H_
#define HMREMOTERESULT_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <shared_mutex>
#include <vector>
#include <cstdint>

#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMIPAddress.h"

//! Class to hold the Remote results in the DNS cache. Also tracks the state of a resolution.
/*!
     HMRemoteResult holds the results of the Remote checks inside the Remote cache.
     All data is protected by a mutex at the result level. This allows concurrent threads to access the data safely.
 */
class HMRemoteResult
{
public:

    HMRemoteResult() :
        m_checkState(HM_CHECK_INACTIVE),
        m_remoteTimeout(HM_DEFAULT_CHECK_TIMEOUT),
        m_remoteTTL(HM_DEFAULT_TTL) {};

    HMRemoteResult(HMRemoteResult&& k)
    {
        m_remoteTTL = k.m_remoteTTL;
        m_remoteTimeout = k.m_remoteTimeout;
        m_checkState = k.m_checkState;
        m_checkTime = k.m_checkTime;
        m_resultTime = k.m_resultTime;
    }

    HMRemoteResult(uint64_t ttl, uint64_t timeout) :
        m_checkState(HM_CHECK_INACTIVE),
        m_remoteTimeout(timeout),
        m_remoteTTL(ttl) {};

    //! Update the internal timeout values after updating an entry.
    /*!
         Update the internal timeout values after updating an entry based on the TTL and timeout of the current name resolution.
         \param the time to live of the request.
         \param the timeout of the lookup.
     */
    void updateTimeouts(uint64_t ttl, uint64_t timeout);

	//! Put the check into the queued state.
	/*!
	     Put the check into the queued state. Sets the internal state parameter to HM_CHECK_QUEUED.
	 */
	void queueCheck();

	//! Start the Remote check for this result.
	/*!
	     Start the Remote check for this result.
	     Sets the internal state to HM_CHECK_IN_PROGRESS.
	     \return the HMTimeStamp with the check timeout value. (To determine when to reschedule in case of timeout.)
	 */
	HMTimeStamp startCheck();

	//! Finish the Remote check for this result.
	/*!
	     Finish the Remote check for this result. Set the internal state to either HM_CHECK_IDLE or HM_CHECK_FAILED.
	     \param true if the check was a success.
	 */
	void finishCheck(bool success);

	//! Determine if the Remote check needs to be run now.
	/*!
	     Determine if the Remote check needs to be run now.
	     \return true if the check is expired and needs to be run now.
	 */
	bool checkNeeded() const;

	//! Determine the time of the next required Remote check for this entry.
	/*!
	     Determine the time of the next required Remote check for this entry.
	     \return HMTimeStamp of the next required Remote check for this entry.
	 */
	HMTimeStamp nextCheckTime() const;

	//! Get the TTL of this Remote check.
	/*!
	     Get the TTL of this Remote check.
	     \return the TTL in seconds for this entry.
	 */
	uint64_t getCheckTTL() const;

	//! Get the current state of the internal state machine.
	/*!
	     Get the current state of the internal state machine.
	     \return the HM_WORK_STATE of the check.
	 */
    HM_WORK_STATE getCheckState() const;

    //! Get the last time the Remote check occurred.
    /*!
         Get the last time the Remote check occurred.
         \return the HMTimeStamp of the last resolution.
     */
    const HMTimeStamp& getResultTime() const;

    //! Set the Remote check time.
    /*!
         Set the Remote check time.
         \param the HMTimeStamp to set as the Remote check time.
     */
    void setResultTime(const HMTimeStamp& resultTime);

private:

	mutable std::shared_timed_mutex m_resultLock;

	HM_WORK_STATE m_checkState;
	HMTimeStamp m_checkTime;
	HMTimeStamp m_resultTime;

	uint64_t m_remoteTimeout;
	uint64_t m_remoteTTL;
};

#endif /* HMREMOTERESULT_H_ */
