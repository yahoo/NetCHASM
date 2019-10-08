// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMDATACHECKLIST_H_
#define HMDATACHECKLIST_H_

#include <map>
#include <string>
#include <memory>
#include <vector>

#include "HMDataCheckResult.h"
#include "HMDataCheckParams.h"
#include "HMDataHostCheck.h"
#include "HMDataHostGroup.h"
#include "HMDNSCache.h"

class HMWork;
class HMStorage;
class HMDataHostCheck;
class HMCheckHeader;
class HMAuxCache;
class HMAuxInfo;
//! Class to hold the list of health checks to perform
/*!
 * This is the main class that holds all health checks that NetCHASM will perform.
 * Checks are stored in a tiered structure to minimize the number of repeat checks.
 * HMDataCheckList -> Top level class holding all checks. Query by HostName, IPAddress and hostDataCheck.
 * Core data structure is a mapping of host name and hostDataChecks to the checkParams for the check.
 * HMDataHostCheck -> Stores the check URL, checktype and all information required to perform the check work.
 * Thus, the internal map contains a key/value pair for each individual piece of check "work" to perform.
 *
 * HMDataCheckParams -> Stores the variables for check timing, check updating and storage. This allows each check to be performed
 * with all individual rotation's check to be updated correctly even with different check parameters, averaging etc.
 *
 * HMDataCheckResults -> Stores the actual results of the checks
 */
class HMDataCheckList
{
public:
    HMDataCheckList() : m_guard(false) {};
    HMDataCheckList(HMDataCheckList& k) = delete;

    //! CheckNeeded determines if the specific check in question should be conducted now.
    /*!
        This function determines the checkNeeded (HM_SCHEDULE_STATE) according to the requirements of all check parameters
        \param hostname to check.
        \param ip to check.
        \param hostCheck parameters to use for the check.
        \return the schedule state. Either queue work, a timeout event or ignore.
     */
    HM_SCHEDULE_STATE checkNeeded(std::string& hostname, HMIPAddress& ip, HMDataHostCheck& hostCheck);

    //! nextCheckTime determines the next timeStamp to conduct the given check.
    /*!
         This function determines the next check time according to the requirements of all check parameters.
         \param hostname to check.
         \param ip address to check.
         \param hostCheck parameters to be used for the check.
         \return An HMTimeStamp of the time the next check should occur.
     */
    HMTimeStamp nextCheckTime(std::string& hostname, const HMIPAddress& ip, HMDataHostCheck& hostCheck);

    //! Get the check timeout based on the TTL of this check.
    /*!
         This function gets the TTL of this check based on the minimum TTL of all the chec params for this check.
         \param hostname to check.
         \param ip address to check.
         \param hostCheck to use for the check parameters.
         \return the timestamp of the minimum TTL for this check.
     */
    HMTimeStamp getCheckTimeout(const std::string& hostname, const HMIPAddress& ip, HMDataHostCheck& hostCheck);

    //! This function is called to insert the check in to the work queue.
    /*
         This function is called to insert the check in to the work queue.
         The function changes the internal state to HM_CHECK_QUEUED and creates a work order inserting it into the work queue.
         \param hostname to check.
         \param ip to check.
         \param hostCheck data to be used for the check.
         \param the work queue to insert the check.
         \param enable remote check(default is false)
     */
    void queueCheck(const std::string& hostname, const HMIPAddress& ip, HMDataHostCheck& check, HMWorkQueue& queue, bool remote = true);

    //! This function is called by the worker thread when the check is removed from the work queue and is executed.
    /*
         This function is called by the worker thread when the check is removed from the work queue and is executed.
         The function updates the internal state machine to HM_CHECK_IN_PROGRESS.
         \param hostname of the check.
         \param ip of the check.
         \param hostCheck data to use for the check.
         \return the timeout of the check. It can be rescheduled if this timeout elapses.
     */
    HMTimeStamp startCheck(std::string& hostname, HMIPAddress& ip, HMDataHostCheck& check);

    //! This function retrieves the host groups associated with the given check and check params.
    /*
         This function retrieves the host groups associated with the given check and check params.
         \param check header specifying the hostname, ip, hostcheck and check params of the check.
         \param vector of hostGroups (string) associated with the given check params.
         \return true if the vector is filled false if the lookup failed.
     */
    bool getHostGroups(const HMCheckHeader& header, std::vector<std::string>& hostGroups);

    //! This function updates the result based on a reload from the backend data store.
    /*
         This function updates the result based on a reload from the backend data store.
         \param check header specifying the hostname, ip, hostcheck and check params of the check.
         \param the result to directly insert into the cache for the check.
         \return true if the result was inserted successfully.
     */
    bool updateCheck(const HMCheckHeader& header, const HMDataCheckResult& result);

    //! This function updates the result based on a reload from the backend data store.
    /*
         This function updates the result based on a reload from the backend data store.
         \param work pointer to the work base class that contains the results of the check and parameters of the check conducted.
         \param the result to directly insert into the map of check params and check results.
         \return true if the result was inserted successfully.
    */
    void updateCheck(HMWork* work, std::map<HMDataCheckParams, HMDataCheckResult>& results);

    //! This function is called by the work thread updating the internal check information.
    /*
         This function is called by the work thread updating the internal check information.
         The function parses the results and timing parameters from the work results updating the internal check information for each check params.
         It sets the internal state of the check to HM_CHECK_IDLE.
         \param work pointer to the work base class that contains the results of the check and parameters of the check conducted.
         \param hostCheck to update.
     */
    void updateCheck(HMWork* work, HMDataHostCheck &hostCheck);

    //! This function is called by the worker thread to commit the check information to the back end data store.
    /*
         This function is called by the worker thread to commit the check information to the back end data store.
         It takes care of updating all results associated with the check including all affected host groups.
        \param work pointer to the work base class defining the check parameters that need updated in the backend storage.
        \param hostCheck to update in the backend store.
        \param ip address to update in the backend store.
        \param pointer to the current backend data storage class.
     */
    void storeCheck(HMWork* work, HMDataHostCheck& hostCheck, const HMIPAddress& address, HMStorage* store);

    //! This function is called by the worker thread to commit the aux information to the backend data store.
    /*
         This function is called by the worker thread to commit the aux information to the backend data store and internal cache.
         It takes care of updating all results associated with the check including all affected host groups.
        \param work pointer to the work base class defining the check parameters that need updated.
        \param hostCheck to update.
        \param ip address to update.
        \param the aux data to parse in the aux cache parser.
        \param pointer to the current backend data storage class.
        \param format of aux data string.
        \param the current aux cache.
     */
    void storeAux(HMWork* work, HMDataHostCheck& hostCheck, const HMIPAddress& address, std::string auxData, HMStorage* store, HMAuxCache& aux, HM_AUX_DATA_TYPE auxDataType);

    //! This function is called by the worker thread to commit the aux information to the backend data store.
    /*
         This function is called by the worker thread to commit the aux information to the backend data store and internal cache.
         It takes care of updating all results associated with the check including all affected host groups.
        \param work pointer to the work base class defining the check parameters that need updated.
        \param hostCheck to update.
        \param ip address to update.
        \param the aux Info data structure.
        \param pointer to the current backend data storage class.
        \param the current aux cache.
     */
    void storeAux(HMWork* work, HMDataHostCheck& hostCheck, const HMIPAddress& address, HMAuxInfo& auxInfo, HMStorage* store, HMAuxCache& aux);

    //! Returns all the current health checks loaded in the core.
    /*
        Returns all the current health checks in the core.
        Returns a vector of HMCheckHeader (hostName, ipAddress, HostCheck, CheckParams).
        \param vector of HMCheckHeader to fill with the health check information.
        \return true if the vector contains all the health checks.
     */
    bool getAllChecks(std::vector<HMCheckHeader>& allChecks);

    //! Retrieve a set of check results for a given host.
    /*
         Retrive a set of check results for a given host. Gets all checks for a given hostname, address, host check. Currently only used in testing.
         \param hostname to get results.
         \param hostcheck to get results.
         \param address to get results.
         \param vector of pairs of check params and check results to fill with all associated check params and correlated results.
         \return true if the results vector is filled with all check params results pairs.
     */
    bool getCheckResults(const std::string& hostname, const HMDataHostCheck& check, const HMIPAddress& address, std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>>& results);

    //! Retrieves the check result for the given check header
    /*
         Retrieves the check result for the given check header.
         Check header includes the hostname, address, check data and check params.
         \param check header to lookup results
         \param result the result structure to fill with the correlated check results.
         \return true if the result contains the check results for the passed check header.
     */
    bool getCheckResult(const HMCheckHeader& check, HMDataCheckResult& result);

    //! This function sets the host down overriding all health check information. 
    /*!
         This function sets the host down overriding all health check information.
         This is set through the socket interface and is only used during debugging on the host.
         It should never be left set in normal production.
         \param hostName to force down.
         \param the associated host check to force down.
         \param the associated address to force down.
         \param the force host status either true (down) or false to clear.
     */
    void setForceHostStatus(const std::string& hostName, HMDataHostCheck& check, HMIPAddress& address, bool forceHostStatus);
    
    //! Add a hostGroup to the internal state.
    /*
         Add a hostGroup to the internal state. This function generates a check for each host and/or populates the check params list of groups if the check already existed.
         Only call this during config load. Once the checks start, this can no longer safely be called.
         \param the host group to add.
         \return the number of checks that will be conducted for this group.
     */
    uint32_t addHostGroup(HMDataHostGroup& hostGroup);

    //! Initiate the DNS cache based on the internal list of hostChecks.
    /*
         Initiate the DNS cache based on the internal list of hostChecks.
         This function creates a DNS cache entry for each host name that needs to be looked up.
         It also fills the dnsWaitList which maps host names to their host checks. This data structure is used by the DNS callback to schedule health checks when the DNS entry is updated.
         Once the checks start, this can no longer be called safely.
         \param the currest DNS cache to fill.
         \param the current dnsWaitList to fill.
     */
    void initDNSCache(HMDNSCache& cache, HMWaitList& dnsWaitList);

    //! Remove the check from the cache and backend store.
    /*
         Remove the check from the cache and backend store.
         \param hostname to remove.
         \param ip address to remove.
         \param the host check to remove.
         \param a pointer to the current backend storage class to purge the check result.
     */
    void invalidateCheck(std::string& hostname, const HMIPAddress& ipAddress, HMDataHostCheck& hostCheck, HMStorage* store);

    //! Insert the given check.
    /*
         Insert the given check. This is not safe to call once the checks begin running. This is currently a debug function for unit testing only.
         \param the host group to use in the check insertions.
         \param the hostname to insert.
         \param the host check to insert.
         \param the check params to insert.
         \param the set of ips to insert.
     */
    void insertCheck(std::string hostGroup, std::string hostname, HMDataHostCheck& hostCheck, HMDataCheckParams& checkParams, std::set<HMIPAddress>& ips);

    //! Insert an empty query in case of a failed DNS resolution.
    /*
         Insert an empty query in case of a failed DNS resolution.
         \param a pointer to the work order of the failed DNS resultion.
         \param the host check to update.
         \param a blank IPAddress to insert.
     */
    void insertEmptyQuery(HMWork* work, HMDataHostCheck &hostCheck, const HMIPAddress &address);

    //! Print all of the checks in the core.
    /*
         Print all of the checks in the core.
         \param true to also print all the check info.
         \return string containing all the check info in the core.
     */
    std::string printChecks(bool printCheckInfo) const;

    //!Sets the value for m_guard, to prevent insetring checks in the middle of run.
    void setGuard (bool guard);


private:
    bool m_guard;
    HMCheckList m_checklist;
    // Contains reference to m_checklist entries without the remoteCheck set in DataHostCheck
    std::multimap<std::pair<std::string,HMDataHostCheck>, HMCheckList::iterator> m_checklistReference;
};

#endif /* HMDATACHECKLIST_H_ */
