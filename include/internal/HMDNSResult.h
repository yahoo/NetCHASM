// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDNSRESULT_H_
#define HMDNSRESULT_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <shared_mutex>
#include <vector>
#include <cstdint>

#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMIPAddress.h"

//! Class to hold the DNS results in the DNS cache. Also tracks the state of a resolution.
/*!
     HMDNSResult holds the results of the DNS lookups inside the DNS cache.
     All data is protected by a mutex at the result level. This allows concurrent threads to access the data safely.
 */
class HMDNSResult
{
public:

    HMDNSResult() :
        m_queryState(HM_CHECK_INACTIVE),
        m_queryVersion(0),
        m_queryTimeout(HM_DEFAULT_DNS_RESOLUTION_TIMEOUT),
        m_dnsTimeout(HM_DEFAULT_DNS_TTL) {};

    HMDNSResult(HMDNSResult&& k)
    {
        m_dnsTimeout = k.m_dnsTimeout;
        m_queryTimeout = k.m_queryTimeout;
        m_queryState = k.m_queryState;
        m_queryVersion = 0;
        m_queryTime = k.m_queryTime;
        m_resultTime = k.m_resultTime;
    }

    HMDNSResult(uint64_t ttl, uint64_t timeout) :
        m_queryState(HM_CHECK_INACTIVE),
        m_queryVersion(0),
        m_queryTimeout(timeout),
        m_dnsTimeout(ttl) {};

    //! Update the internal timeout values after updating an entry.
    /*!
         Update the internal timeout values after updating an entry based on the TTL and timeout of the current name resolution.
         \param the time to live of the request.
         \param the timeout of the lookup.
     */
    void updateTimeouts(uint64_t ttl, uint64_t timeout);

    //! Update the resolution with the new addresses.
    /*!
         Update the resolution result with the new addresses.
         \param a set of IPAddresses to store.
     */
	void updateQuery(const std::set<HMIPAddress>& addresses);

	//! Put the resolution into the queued state.
	/*!
	     Put the resolution into the queued state. Sets the internal state parameter to HM_CHECK_QUEUED.
	 */
	void queueQuery();

	//! Start the DNS resolution for this result.
	/*!
	     Start the DNS resolution for this result.
	     Sets the internal state to HM_CHECK_IN_PROGRESS.
             \param version of the query.
	     \return the HMTimeStamp with the check timeout value. (To determine when to reschedule in case of timeout.)
	 */
	HMTimeStamp startQuery(uint32_t version=0);

	//! Finish the DNS resolution for this result.
	/*!
	     Finish the DNS resolution for this result. Set the internal state to either HM_CHECK_IDLE or HM_CHECK_FAILED.
	     \param true if the lookup was a success.
             \param version of the query.
	 */
	void finishQuery(bool success);

	//! Determine if the DNS query needs to be run now.
	/*!
	     Determine if the DNS query needs to be run now.
             \param version of the current state.
	     \return true if the check is expired and needs to be run now.
	 */
	bool queryNeeded(uint32_t version=0) const;

	//! Determine the time of the next required DNS resolution for this entry.
	/*!
	     Determine the time of the next required DNS resolution for this entry.
             \param version of the current state.
	     \return HMTimeStamp of the next required DNS resolution for this entry.
	 */
	HMTimeStamp nextQueryTime(uint32_t version=0) const;

	//! Get the TTL of this DNS resolution.
	/*!
	     Get the TTL of this DNS resolution.
	     \return the TTL in seconds for this entry.
	 */
	uint64_t getDNSTTL() const;


	//! Get the addresses associated with this entry.
	/*!
	     Get the addresses associated with this entry. Returns addresses that are not expired.
	     \param a set of HMIPAddress to fill with the addresses of this entry.
	     \return true if the set contains the addresses.
	 */
	bool getAddresses(std::set<HMIPAddress>& addresses) const;

	//! Get the first address in the address list for this hostname.
	/*!
	     Get the first address in the address list for this hostname.
	     \param an HMIPAddress to fill with the address information.
	     \return true if the passed address contains the first entry.
	 */
	bool getAddress(HMIPAddress& address) const;

    //! Get all the addresses associated with this entry.
    /*!
         Get all the addresses associated with this entry. Returns addresses even if they are expired.
         \param a set of HMIPAddress to fill with the addresses of this entry.
         \return true if the set contains the addresses.
     */
	bool getExpiredAddresses(std::set<HMIPAddress>& addresses) const;

	//! Check to see if the given address is in the address set.
	/*!
	     Check to see if the given address is in the address set.
	     \param the address to check.
	     \return true if the address is resolved to this entry.
	 */
	bool isValidAddress(HMIPAddress& addresses) const;

	//! Get the current state of the internal state machine.
	/*!
	     Get the current state of the internal state machine.
	     \return the HM_WORK_STATE of the check.
	 */
    HM_WORK_STATE getQueryState() const;

    //! Get the version of the internal state machine.
    /*!
     *   Get the version of the internal state machine.
     *   \return the version of the check.
     */
    uint32_t getQueryVersion() const;

    //! Get the last time the DNS resolution occurred.
    /*!
         Get the last time the DNS resolution occurred.
         \return the HMTimeStamp of the last resolution.
     */
    const HMTimeStamp& getResultTime() const;

    //! Set the DNS resolution time.
    /*!
         Set the DNS resolution time.
         \param the HMTimeStamp to set as the DNS resolution time.
     */
    void setResultTime(const HMTimeStamp& resultTime);

private:

	mutable std::shared_timed_mutex m_resultLock;

	HM_WORK_STATE m_queryState;
        uint32_t m_queryVersion;
	HMTimeStamp m_queryTime;
	HMTimeStamp m_resultTime;

	uint64_t m_queryTimeout;
	uint64_t m_dnsTimeout;

	std::set<HMIPAddress> m_addr;
	std::set<HMIPAddress> m_addrExp;
};

#endif /* YDNSRESULT_H_ */
