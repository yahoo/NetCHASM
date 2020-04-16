// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMREMOTEHOSTCACHE_H_
#define HMREMOTEHOSTCACHE_H_

#include <map>
#include <string>
#include <set>
#include <mutex>
#include <shared_mutex>
#include "HMConstants.h"
#include "HMRemoteResult.h"

class HMWorkQueue;
class HMEventLoop;
class HMCheckHeader;

//! The class that stores all of the Remote host checks.
class HMRemoteHostCache
{
public:
    HMRemoteHostCache() {};
    HMRemoteHostCache(HMRemoteHostCache&) = delete;

    //! Initialize the Remote Host cache.
    /*!
         Initialize the Remote Host cache.
         \return true if the Remote Host cache is ready.
     */
    bool init();

    //! Insert a blank entry into the Remote Host cache.
    /*!
         Insert a blank entry into the Remote Host cache.
         \param the name to resolve.
         \param the time-to-live of the entry before requiring another lookup.
         \param the timeout of the resolution request.
     */
    void insertRemoteEntry(const std::string& name, const HMDataHostCheck& dataHostCheck, uint64_t ttl, uint64_t timeout);

    //! Finish the Remote Host check.
    /*!
         Finish the Remote Host-group check. Sets the internal state to either HM_CHECK_INACTIVE or HM_CHECK_FAILED depending on success.
         \param the resolved name.
         \param true if the check was a success.
     */
    void finishCheck(std::string name, const HMDataHostCheck& dataHostCheck, bool success);

    //! Get the full Remote Host result for the given hostname.
    /*!
         Get the full Remote result for the given hostname. This includes the check status and timeout.
         \param the resolved name to fetch the info.
         \param a constant iterator that will point to the entry in the map.
         \return true if the entry was found and stored in result.
     */
    bool getRemoteResult(const std::string& name, const HMDataHostCheck& dataHostCheck, std::map<std::pair<std::string, HMDataHostCheck>,HMRemoteResult>::const_iterator& result) const;

    //! Check to see if we should schedule a Remote check for the given lookup.
    /*!
         Check to see if we should schedule a Remote check for the given lookup.
         \param the hostname to resolve.
         \return the schedule state. Either queue work, a timeout event or ignore.
     */
    HM_SCHEDULE_STATE checkNeeded(const std::string& name, const HMDataHostCheck& dataHostCheck) const;

    //! Get the next resolution time based on the timeout.
    /*!
         Get the next resolution time based on the timeout
         \param the host to resolve.
         \return the HMTimeStamp of the next time to schedule a Remote check.
     */
    HMTimeStamp nextCheckTime(const std::string& name, const HMDataHostCheck& dataHostCheck) const;

    //! Start a Remote check.
    /*!
         Start a Remote check. Update the internal check state to in progress.
         \param the host name to resolve.
         \return the true if succeeds.
     */
    bool startRemoteCheck(const std::string& name, const HMDataHostCheck& dataHostCheck);

    //! Queue the Remote check.
    /*!
         Queue the Remote check. Set the check state to Queued.
         \param the hostname to resolve.
         \param the work queue to insert the Remote check work.
         \return true on success
     */
    bool queueRemoteCheck(std::string name, const HMDataHostCheck& dataHostCheck, HMWorkQueue& queue);

    //! Queue all the Remote checks.
    /*!
         Queue all the Remote checks. Called at cold start to get all the hostnames resolved.
         \param the work queue to insert the resolutions.
         \param the event loop to insert any needed timeouts for Remote checks that were loaded from storage.
         \param true if this is a restart during a running daemon config reload.
     */
    void queueRemoteLookups(HMWorkQueue& queue, HMEventLoop& eventLoop, bool restart);

    //! Update the result time for Remote checks.
    /*!
         Update the result time for Remote checks. \\
         \param the hostgroupname.
         \param the result time.
     */
    void updateResultTime(const std::string& name, const HMDataHostCheck& dataHostCheck, const HMTimeStamp& resultTime);

private:
    std::map<std::pair<std::string, HMDataHostCheck>, HMRemoteResult> m_cache;
};

#endif /* HMREMOTEHOSTCACHE_H_ */
