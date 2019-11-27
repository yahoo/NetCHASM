// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDataCheckParams_H_
#define HMDataCheckParams_H_

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <shared_mutex>

#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMDataCheckResult.h"
#include "HMDataHostCheck.h"

class HMIPAddress;

//! Main class to hold the check parameters
/*!
     Main class to hold the check parameters.
     This class stores values that are used in the update and storage of the check.
     They must not cause a new check based on them, but can be used to configure how the check is updated.
     Check update and scheduling can be based on the check params differently for the check type.
 */
class HMDataCheckParams
{
public:
    HMDataCheckParams() :
        m_numCheckRetries(0),
        m_checkRetryDelay(0),
        m_measurementOptions(HM_RT_CONNECT),
        m_smoothingWindow(HM_DEFAULT_SMOOTHING_WINDOW),
        m_groupThreshold(HM_DEFAULT_GROUP_THRESHOLD),
        m_slowThreshold(HM_DEFAULT_SLOW_THRESHOLD),
        m_maxFlaps(HM_DEFAULT_MAX_FLAPS),
        m_checkTimeout(HM_DEFAULT_CHECK_TIMEOUT),
        m_checkTTL(HM_DEFAULT_TTL),
        m_flapThreshold(HM_DEFAULT_FLAP_THRESHOLD),
        m_passthroughInfo(0) {};

    HMDataCheckParams(const HMDataCheckParams& k)
    {
        m_numCheckRetries = k.m_numCheckRetries;
        m_checkRetryDelay = k.m_checkRetryDelay;
        m_measurementOptions = k.m_measurementOptions;
        m_smoothingWindow = k.m_smoothingWindow;
        m_groupThreshold = k.m_groupThreshold;
        m_slowThreshold = k.m_slowThreshold;
        m_maxFlaps = k.m_maxFlaps;
        m_checkTimeout = k.m_checkTimeout;
        m_checkTTL = k.m_checkTTL;
        m_flapThreshold = k.m_flapThreshold;
        m_passthroughInfo = k.m_passthroughInfo;
    }

    HMDataCheckParams& operator=(const HMDataCheckParams& k)
    {
        m_numCheckRetries = k.m_numCheckRetries;
        m_checkRetryDelay = k.m_checkRetryDelay;
        m_measurementOptions = k.m_measurementOptions;
        m_smoothingWindow = k.m_smoothingWindow;
        m_groupThreshold = k.m_groupThreshold;
        m_slowThreshold = k.m_slowThreshold;
        m_maxFlaps = k.m_maxFlaps;
        m_checkTimeout = k.m_checkTimeout;
        m_checkTTL = k.m_checkTTL;
        m_flapThreshold = k.m_flapThreshold;
        m_passthroughInfo = k.m_passthroughInfo;
        return *this;
    }

    HMDataCheckParams(const HMDataCheckParams&& k) = delete;

    bool operator<(const HMDataCheckParams& k) const;
    bool operator==(const HMDataCheckParams& k) const;
    bool operator!=(const HMDataCheckParams& k) const;

    //! Convenience function to set all the check params.
    /*
         Convenience function to set all the check params.
         \param numCheckRetries the number of times this check will retry.
         \param checkRetryDelay the time delay between check retries.
         \param measurementOptions the MEASUREMENT_OPTIONS define how the performance measurements are taken and aggregated.
         \param smoothingWindow sets the size of the window used to calculate the smoothed connection measurement.
         \param groupThreshold sets the maximum amount of time the measurement can be outside of the best group measurement to be considered up.
         \param slowThreshold sets the max performance that will trigger an automatic retry measurement.
         \param maxFlaps sets the maximum number of flaps to mark the host down.
         \param checkTimeout sets the connection timeout for the health check.
         \param checkTTL sets the time-to-live of the measurement.
         \param flapThreshold sets the amount of time that needs to transpire to reset the flap count.
         \param passthroughInfo sets the information for the rotation to send straight to the requester.
     */
    void setCheckParameters(uint8_t numCheckRetries,
            uint32_t checkRetryDelay,
            uint16_t measurementOptions,
            uint32_t smoothingWindow,
            uint32_t groupThreshold,
            uint32_t slowThreshold,
            uint32_t maxFlaps,
            uint64_t checkTimeout,
            uint64_t checkTTL,
            uint32_t flapThreshold,
            uint32_t passthroughInfo);

    //! set the forcedown flag for the appropriate IP address in this check parameter
    /*!
         Set the force down flag for the hosts based on their IP address.
         \param address the IP address that should be marked down.
         \param forceHostStatus true (forcedown) or false (normal operation).
     */
    void setHostStatus(const HMIPAddress& address, bool forceHostStatus);

    //! Determine if this check needs to be conducted now
    /*!
         Does this check need to be conducted now.
         \param address the address to check
         \return bool true if the check should be conducted now.
     */
    bool checkNeeded(HMIPAddress& address);
    //! Get the next check time for these check parameters
    /*!
         Get the next check time for the specified address in this check params.
         \param address the address to get the next check time.
         \return HMTimeStamp the timestamp to schedule the next check for this check params and address.
     */
    HMTimeStamp nextCheckTime(const HMIPAddress& address);

    //! Get the check timeout based on the TTL of this check params.
    /*!
         Get the check timeout from now until the TTL.
         \param address the address to use to get the checktime.
         \return HMTimeStamp the timestamp of now plus the check TTL.
     */
    HMTimeStamp getCheckTimeout(const HMIPAddress& address);

    //! Set the state machine for this check to queued.
    /*!
        Set the state machine for this check to queued.
        \param the IP address to set
     */
    void queueQuerry(const HMIPAddress& address);

    //! Set the state machine for this check to be idle.
    /*!
        Set the state machine for this check to be idle.
        \param the IP address to set
     */
    void emptyQuery(const HMIPAddress& address);

    //! Set the state machine for this check to running.
    /*!
        Set the state machine for this check to running.
        \param the IP address to set
     */
    void startQuery(HMIPAddress& address);

    //! Check to see if this IP address is currently active.
    /*!
        Check to see if this IP address is currently active.
        \param the IP address to check
        \return true if the IP address is currently in the check for this check params.
     */
    bool isValidIP(const HMIPAddress& address);

    //! Update the check with the new check result.
    /*!
         Update the check with a new check result.
         \param address the address to replace in this check param.
         \param result the HMDataCheckResult to use to replace the current entry.
         \param forceReplace true if the new entry should replace regardless. False to replace the check timeout rules.
     */
    void updateCheck(const HMIPAddress& address, const HMDataCheckResult& result, bool forceReplace);

    //! Update the check with the new result.
    /*!
         Update the check with the new result. This is the function called from the Work responses.
         \param hostname the hostname from the check. Used primarily in logging.
         \param address the address to update in this check param.
         \param response the response code returned from the work check.
         \param reason the reason code returned from the work check.
         \param start the start time of the check.
         \param end the end time of the check.
         \param port the port used in the check.
     */
    void updateCheck(std::string& hostname, const HMIPAddress& address, HM_RESPONSE response, HM_REASON reason, HMTimeStamp start, HMTimeStamp end, uint16_t port);

    //! Invalidate and remove the check and address to clear it from the internal cache.
    /*!
         Invalidate and remove the check and address to clear it from the internal cache.
         \param address the address to remove from the cache.
         \param result the result to remove from the cache.
         \return true if the entry was removed successfully
     */
    bool invalidateResult(const HMIPAddress& address, HMDataCheckResult& result);

    //! Get the check result
    /*! Get the check result
         \param address the address to fetch the check.
         \param result the data structure to fill the check result.
         \return bool true if the check was copied into the check result.
     */
    bool getCheckResult(const HMIPAddress& address, HMDataCheckResult& result);

    //! Get the time this check was last completed.
    /*!
         Get the time this check was last completed.
         \param address the address to get the last check time.
         \return HMTimeStamp the timestamp this check was last completed.
     */
    HMTimeStamp getCheckTime(HMIPAddress& address);

    //! Get the configured number of check retries.
    /*
         Get the configured number of check retries.
         \return the number of check retries.
     */
    uint8_t getNumCheckRetries() const;

    //! Get the configured check retry delay.
    /*
         Get the configured check retry delay.
         \return the check retry delay.
     */

    uint32_t getCheckRetryDelay() const;

    //! Get the current configured measurement options.
    /*
         Get the current configured measurement options.
         \return the measurement options.
     */
    uint16_t getMeasurementOptions() const;

    //! Get the configured smoothing window.
    /*
         Get the configured smoothing window.
         \return the smoothing window.
     */
    uint32_t getSmoothingWindow() const;

    //! Get the configured group threshold.
    /*
         Get the configured group threshold.
         \return the group threshold.
     */
    uint32_t getGroupThreshold() const;

    //! Get the configured slow threshold.
    /*
         Get the configured slow threshold.
         \return the slow threshold.
     */
    uint32_t getSlowThreshold() const;

    //! Get the configured max flaps.
    /*
         Get the configured max flaps.
         \return the max flaps.
     */
    uint32_t getMaxFlaps() const;

    //! Get the configured check connection timeout.
    /*
         Get the configured check connection timeout.
         \return the check connection timeout.
     */
    uint64_t getTimeout() const;

    //! Get the configured check time-to-live.
    /*
         Get the configured check time-to-live.
         \return the check time-to-live.
     */
    uint64_t getTTL() const;

    //! Get the configured flap threshold.
    /*
         Get the configured flap threshold.
         \return the flap threshold.
     */
    uint32_t getFlapThreshold() const;

    //! Get the configured passthrough information.
    /*
         Get the configured passthrough information.
         \return the passthrough information.
     */
    uint32_t getPassthroughInfo() const;

    //! Print the check params entry.
    /*
         Print the check params entry.
         \return the string representation of the check params.
     */
    std::string printEntry() const;

    //! Print the host groups that are associated with this check params structure.
    /*
         Print the host groups that are associated with this check params structure.
         \return the string representation of the host groups associated with the check params.
     */
    std::string printHostGroups() const;

    //! Get the host groups which are associated with this check params structure.
    /*
         Get the host groups which are associated with this check params structure.
         \param a vector of strings to store the host group list.
         \return true if the vector contains the host groups associates with this params structure.
     */
    bool getHostGroups(std::vector<std::string>& hostGroups) const;

    //! Get the host groups which are associated with this check params structure.
    /*
         Get the host groups which are associated with this check params structure.
         \param a set of strings to store the host group list.
         \return true if the set contains the host groups associates with this params structure.
     */
    bool getHostGroups(std::set<std::string>& hostGroups) const;

    //! Add a host group to be associated with this check params structure.
    /*
         Add a host group to be associated with this check params structure.
         \param the host group name to add.
     */
    void addHostGroup(std::string name);

    //! Get the state of the internal query state machine.
    /*
         Get the state of the internal query state machine.
         \param the ip address to retrieve the state.
         \return the work state associated with the passed address.
     */
    HM_WORK_STATE getQueryState(HMIPAddress& address);

private:

    //! This function updates the state for the current check based on the results of the work.
    /*!
         This function updates the internal check results based on the results of the check.
         \param the hostname checked in this check to update.
         \param the start time of the check for timing purposes.
         \param the end time of the check for timing puproses.
         \param an iterator to the current entry to update.
     */
    void setResponse(std::string& hostname,
            HMTimeStamp start,
            HMTimeStamp end,
            std::map<HMIPAddress, HMDataCheckResult>::iterator& it);

    uint8_t m_numCheckRetries;
    uint32_t m_checkRetryDelay;
    uint16_t m_measurementOptions;
    uint32_t m_smoothingWindow;
    uint32_t m_groupThreshold;
    uint32_t m_slowThreshold;
    uint32_t m_maxFlaps;
    uint64_t m_checkTimeout;
    uint64_t m_checkTTL;
    uint32_t m_flapThreshold;
    uint32_t m_passthroughInfo;

    mutable std::shared_timed_mutex m_sharedMutex;

    std::map<HMIPAddress,HMDataCheckResult> m_checkData;
    std::vector<std::string> m_hostGroups;
};

#endif /* HMDataCheckParams_H_ */
