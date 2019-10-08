// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <memory>
#include <limits.h>

#include "HMDataCheckList.h"
#include "HMAuxCache.h"
#include "HMStorage.h"
#include "HMWork.h"
#include "HMLogBase.h"
#include "HMWorkHealthCheckCurl.h"
#include "HMWorkQueue.h"
#include "HMWorkHealthCheckTCP.h"
#include "HMWorkHealthCheckDNS.h"
#include "HMWorkHealthCheckNone.h"
#include "HMWorkAuxFetchCurl.h"
#include "HMWorkHealthCheckRemote.h"
#include "HMWorkHealthCheckRemoteTLS.h"
#include "HMWorkHealthCheckTCPS.h"
#include "HMWorkMarkFetchCurl.h"
using namespace std;

HM_SCHEDULE_STATE
HMDataCheckList::checkNeeded(string& hostname, HMIPAddress& ip, HMDataHostCheck& hostCheck)
{
    if(ip.toString() == "0.0.0.0" || ip.toString() == "::")
    {
        return HM_SCHEDULE_IGNORE;
    }
    HM_SCHEDULE_STATE result = HM_SCHEDULE_EVENT;
    bool alreadyScheduled = true;
    bool isCheckTimeChanged = false;
    HMTimeStamp nextCheck = HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    auto key = make_pair(hostname, hostCheck);
    auto ret = m_checklist.equal_range(key);
    for (auto it = ret.first; it != ret.second; ++it)
    {
        if(!it->second.isValidIP(ip))
        {
            return HM_SCHEDULE_IGNORE;
        }
        HMTimeStamp checkTime = it->second.nextCheckTime(ip);
        HM_WORK_STATE query_state = it->second.getQueryState(ip);
        if((query_state == HM_CHECK_FAILED) || (query_state == HM_CHECK_INACTIVE))
        {
            alreadyScheduled = false;
            if (checkTime < nextCheck)
            {
                isCheckTimeChanged = true;
                nextCheck = checkTime;
            }
        }
    }
    if(alreadyScheduled || isCheckTimeChanged)
    {
        result = HM_SCHEDULE_IGNORE;
    }
    if(nextCheck <= HMTimeStamp::now())
    {
        result = HM_SCHEDULE_WORK;
    }
    return result;
}

HMTimeStamp
HMDataCheckList::nextCheckTime(string& hostname, const HMIPAddress& ip, HMDataHostCheck& hostCheck)
{
    HMTimeStamp nextCheck = HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    auto key = make_pair(hostname, hostCheck);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        HMTimeStamp checkTime = it->second.nextCheckTime(ip);
        if(checkTime < nextCheck)
        {
            nextCheck = checkTime;
        }
    }
    return nextCheck;
}

HMTimeStamp
HMDataCheckList::getCheckTimeout(const string& hostname, const HMIPAddress& ip, HMDataHostCheck& hostCheck)
{
    HMTimeStamp minCheckTimeout = HMTimeStamp::now() + HMTimeStamp::HOURINMS;
    auto key = make_pair(hostname, hostCheck);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        HMTimeStamp checkTime = it->second.getCheckTimeout(ip);
        if(checkTime < minCheckTimeout)
        {
            minCheckTimeout = checkTime;
        }
    }
    return minCheckTimeout;
}

void
HMDataCheckList::queueCheck(const string& hostname, const HMIPAddress& ip, HMDataHostCheck& check, HMWorkQueue& queue, bool enableRemoteCheck)
{
    HMLog(HM_LOG_DEBUG, "[CORE] Health Check QueueCheck for %s(%s)",
                                        hostname.c_str(), ip.toString().c_str());
    auto key = make_pair(hostname, check);
    auto ret = m_checklist.equal_range(key);
    for (auto it = ret.first; it != ret.second; ++it)
    {
        it->second.queueQuerry(ip);
    }

    unique_ptr<HMWork> healthCheck;
    if (enableRemoteCheck
            && check.getRemoteCheckType() != HM_REMOTE_CHECK_NONE)
    {
        switch (check.getRemoteCheckType())
        {
        case HM_REMOTE_CHECK_TCP:
            healthCheck = make_unique<HMWorkHealthCheckRemote>(
                    HMWorkHealthCheckRemote(hostname, ip, check));
            break;
        case HM_REMOTE_CHECK_TCPS:
        default:
            healthCheck = make_unique<HMWorkHealthCheckRemoteTLS>(
                    HMWorkHealthCheckRemoteTLS(hostname, ip, check));
            break;
        }
    }
    else
    {
        switch (check.getCheckType())
        {
        case HM_CHECK_HTTP:
        case HM_CHECK_HTTPS:
        case HM_CHECK_HTTPS_NO_PEER_CHECK:
            switch (check.getCheckPlugin())
            {
            case HM_CHECK_PLUGIN_DEFAULT:
            case HM_CHECK_PLUGIN_HTTP_CURL:
                healthCheck = make_unique<HMWorkHealthCheckCurl>(
                        HMWorkHealthCheckCurl(hostname, ip, check));
                break;
            default:
                HMLog(HM_LOG_ERROR,
                        "[CORE] Invalid Check Type Plugin for %s CheckType",
                        printCheckType(check.getCheckType()).c_str());
                return;
            }
            break;

        case HM_CHECK_FTP:
        case HM_CHECK_FTPS_IMPLICIT:
        case HM_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK:
        case HM_CHECK_FTPS_EXPLICIT:
        case HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK:
            switch (check.getCheckPlugin())
            {
            case HM_CHECK_PLUGIN_FTP_CURL:
            case HM_CHECK_PLUGIN_DEFAULT:
                healthCheck = make_unique<HMWorkHealthCheckCurl>(
                        HMWorkHealthCheckCurl(hostname, ip, check));
                break;

            default:
                HMLog(HM_LOG_ERROR,
                        "[CORE] Invalid Check Type Plugin for %s CheckType",
                        printCheckType(check.getCheckType()).c_str());
                return;
            }
            break;

        case HM_CHECK_TCP:
            switch (check.getCheckPlugin())
            {
            case HM_CHECK_PLUGIN_TCP_RAW:
            case HM_CHECK_PLUGIN_DEFAULT:
                healthCheck = make_unique<HMWorkHealthCheckTCP>(
                        HMWorkHealthCheckTCP(hostname, ip, check));
                break;
            default:
                HMLog(HM_LOG_ERROR,
                        "[CORE] Invalid Check Type Plugin for TCP CheckType");
                return;
            }
            break;
        case HM_CHECK_TCPS:
            switch (check.getCheckPlugin()) {
            case HM_CHECK_PLUGIN_TCPS_RAW:
            case HM_CHECK_PLUGIN_DEFAULT:
                healthCheck = make_unique<HMWorkHealthCheckTCPS>(
                        HMWorkHealthCheckTCPS(hostname, ip, check));
                break;
            default:
                HMLog(HM_LOG_ERROR,
                        "[CORE] Invalid Check Type Plugin for TCP CheckType");
                return;
            }
            break;
        case HM_CHECK_DNS:
        case HM_CHECK_DNSVC:
            switch (check.getCheckPlugin())
            {
            case HM_CHECK_PLUGIN_DNS_ARES:
            case HM_CHECK_PLUGIN_DEFAULT:
#ifdef USE_ARES
                healthCheck = make_unique<HMWorkHealthCheckDNS>(
                        HMWorkHealthCheckDNS(hostname, ip, check));
#else
                HMLog(HM_LOG_ERROR, "ARES disabled during build. Please enable it for the ARES to work");
#endif
                break;
            default:
                HMLog(HM_LOG_ERROR,
                        "[CORE] Invalid Check Type Plugin for %s CheckType",
                        printCheckType(check.getCheckType()).c_str());
                return;
            }
            break;

        case HM_CHECK_NONE:
        case HM_CHECK_DEFAULT:
            switch (check.getCheckPlugin())
            {
            case HM_CHECK_PLUGIN_DEFAULT:
                healthCheck = make_unique<HMWorkHealthCheckNone>(
                        HMWorkHealthCheckNone(hostname, ip, check));
                break;
            default:
                HMLog(HM_LOG_ERROR,
                        "[CORE] Invalid Check Type Plugin for None CheckType");
                return;
            }
            break;

        case HM_CHECK_AUX_HTTP:
        case HM_CHECK_AUX_HTTPS:
        case HM_CHECK_AUX_HTTPS_NO_PEER_CHECK:
            switch (check.getCheckPlugin())
            {
            case HM_CHECK_PLUGIN_AUX_CURL:
            case HM_CHECK_PLUGIN_DEFAULT:
                healthCheck = make_unique<HMWorkAuxFetchCurl>(
                        HMWorkAuxFetchCurl(hostname, ip, check));
                break;
            default:
                HMLog(HM_LOG_ERROR,
                        "[CORE] Invalid Check Type Plugin for %s CheckType",
                        printCheckType(check.getCheckType()).c_str());
                return;
            }
            break;

        case HM_CHECK_MARK_HTTP:
        case HM_CHECK_MARK_HTTPS:
        case HM_CHECK_MARK_HTTPS_NO_PEER_CHECK:
            switch (check.getCheckPlugin())
            {
            case HM_CHECK_PLUGIN_DEFAULT:
            case HM_CHECK_PLUGIN_MARK_CURL:
                healthCheck = make_unique<HMWorkMarkFetchCurl>(
                        HMWorkMarkFetchCurl(hostname, ip, check));
                break;
            default:
                HMLog(HM_LOG_ERROR,
                        "[CORE] Invalid Check Type Plugin for %s CheckType",
                        printCheckType(check.getCheckType()).c_str());
                return;
            }
            break;
        default:
            HMLog(HM_LOG_ERROR, "[CORE] Invalid Check Type %s",
                    hostname.c_str());
            return;
        }
    }
    // Setup the timing parameters
    healthCheck->m_start = HMTimeStamp::now();
    healthCheck->m_end = getCheckTimeout(hostname, ip, check);

    queue.insertWork(healthCheck);
}

HMTimeStamp
HMDataCheckList::startCheck(string& hostname, HMIPAddress& ip, HMDataHostCheck& check)
{
    uint64_t maxCheckTimeout = 0;
    auto key = make_pair(hostname, check);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        if(it->second.isValidIP(ip))
        {
            it->second.startQuery(ip);
        }
        maxCheckTimeout = (it->second.getTimeout() > maxCheckTimeout) ? it->second.getTimeout() : maxCheckTimeout;
    }

    return HMTimeStamp().now() + maxCheckTimeout;
}

bool
HMDataCheckList::getHostGroups(const HMCheckHeader& header, vector<string>& hostGroups)
{
    bool found = false;
    auto key = make_pair(header.m_hostname, header.m_hostCheck);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        if(it->second == header.m_checkParams)
        {
            it->second.getHostGroups(hostGroups);
            found = true;
        }
    }
    return found;
}

bool
HMDataCheckList::updateCheck(const HMCheckHeader& header, const HMDataCheckResult& result)
{
    bool found = false;
    auto key = make_pair(header.m_hostname, header.m_hostCheck);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        if(it->second == header.m_checkParams)
        {
            it->second.updateCheck(header.m_address, result, true);
            found = true;
        }
    }
    return found;
}

void
HMDataCheckList::updateCheck(HMWork* work, map<HMDataCheckParams, HMDataCheckResult>& results)
{
    HMLog(HM_LOG_DEBUG, "[CORE] Remote results update for %s", work->m_hostname.c_str());
    auto key = make_pair(work->m_hostname, work->m_hostCheck);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        auto paramsIt = results.equal_range(it->second);
        if(paramsIt.first == paramsIt.second)
        {
            it->second.emptyQuery(work->m_ipAddress);
            it->second.updateCheck(work->m_hostname, work->m_ipAddress, HM_RESPONSE_REMOTE_FAILED, HM_REASON_REMOTE_NODATA, work->m_start, work->m_end, work->m_hostCheck.getPort());
            continue;
        }
        for(auto iit = paramsIt.first; iit != paramsIt.second; ++iit)
        {
            // need to copy status such that it does not fall into retry loop.
            iit->second.m_softStatus = iit->second.m_status;
            iit->second.m_checkTime = HMTimeStamp::now();
            iit->second.m_changeTime = HMTimeStamp::now();
            // reset query state to prevent the health check to go off schedule
            iit->second.m_queryState = HM_CHECK_INACTIVE;
            it->second.updateCheck(iit->second.m_address, iit->second, true);
        }
    }
    results.clear();
}

void
HMDataCheckList::updateCheck(HMWork* work, HMDataHostCheck& hostCheck)
{
    auto key = make_pair(work->m_hostname, hostCheck);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        if (it->second.isValidIP(work->m_ipAddress))
        {
            it->second.updateCheck(work->m_hostname,
                    work->m_ipAddress,
                    work->m_response,
                    work->m_reason,
                    work->m_start,
                    work->m_end,
                    hostCheck.getPort());
        }
    }
}

void
HMDataCheckList::storeCheck(HMWork* work, HMDataHostCheck& hostCheck, const HMIPAddress& address, HMStorage* store)
{
    auto key = make_pair(work->m_hostname, hostCheck);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        HMDataCheckResult result(it->second.getTimeout());
        bool status = it->second.getCheckResult(address, result);
        if (status)
        {
            store->storeCheckResult(work->m_hostname,
                    address,
                    hostCheck,
                    it->second, result);
        }
    }
}

void
HMDataCheckList::storeAux(HMWork* work, HMDataHostCheck& hostCheck, const HMIPAddress& address, string  auxData, HMStorage* store, HMAuxCache& aux, HM_AUX_DATA_TYPE auxDataType)
{

    if(work->m_reason == HM_REASON_SUCCESS)
    {
        // Store a local copy of the Aux data
        aux.storeAuxInfo(work->m_hostname, hostCheck.getCheckInfo(), address, auxData, auxDataType);
    }

    // Commit it to the back store as well
    HMAuxInfo auxInfo;
    aux.getAuxInfo(work->m_hostname, hostCheck.getCheckInfo(), address,
            auxInfo);
    auto key = make_pair(work->m_hostname, hostCheck);
    auto ret = m_checklist.equal_range(key);
    for (auto it = ret.first; it != ret.second; ++it)
    {
        HMDataCheckResult result(it->second.getTimeout());
        // if the check time had not timed out then don't commit the result for this check
        bool status = it->second.getCheckResult(address, result);
        if (status)
        {
            store->storeAuxInfo(work->m_hostname, address, hostCheck,
                    it->second, auxInfo);
        }
    }
}

void
HMDataCheckList::storeAux(HMWork* work, HMDataHostCheck& hostCheck, const HMIPAddress& address, HMAuxInfo& auxInfo, HMStorage* store, HMAuxCache& aux)
{

    aux.storeAuxInfo(work->m_hostname, hostCheck.getCheckInfo(), address, auxInfo);
    auto key = make_pair(work->m_hostname, hostCheck);
    auto ret = m_checklist.equal_range(key);
    for (auto it = ret.first; it != ret.second; ++it)
    {
        HMDataCheckResult result(it->second.getTimeout());
        // if the check time had not timed out then don't commit the result for this check
        bool status = it->second.getCheckResult(address, result);
        if (status)
        {
            store->storeAuxInfo(work->m_hostname, address, hostCheck,
                    it->second, auxInfo);
        }
    }
}

bool
HMDataCheckList::getAllChecks(vector<HMCheckHeader>& allChecks)
{
    allChecks.clear();
    for(auto it = m_checklist.begin(); it != m_checklist.end(); ++it)
    {
        vector<string> hostGroups;
        it->second.getHostGroups(hostGroups);
        allChecks.push_back(HMCheckHeader(it->first.first, HMIPAddress(), it->first.second, it->second));
    }
    return true;
}

bool
HMDataCheckList::getCheckResults(const string& hostname, const HMDataHostCheck& check, const HMIPAddress& address, vector<pair<HMDataCheckParams, HMDataCheckResult>>& results)
{
    results.clear();
    auto key = make_pair(hostname, check);
    auto ret = m_checklistReference.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        HMDataCheckResult result(it->second->second.getTimeout());
        if(it->second->second.getCheckResult(address, result))
        {
            results.push_back(pair<HMDataCheckParams, HMDataCheckResult>(it->second->second, result));
        }
    }
    return (results.size() > 0);
}

bool
HMDataCheckList::getCheckResult(const HMCheckHeader& check, HMDataCheckResult& result)
{
    auto key = make_pair(check.m_hostname, check.m_hostCheck);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        if(it->second == check.m_checkParams)
        {
            if(it->second.getCheckResult(check.m_address, result))
            {
                return true;
            }
        }
    }
    return false;
}

void 
HMDataCheckList::setForceHostStatus(const string& hostName, HMDataHostCheck& check, HMIPAddress& address, bool forceHostStatus)
{
    auto key = make_pair(hostName, check);
    auto ret = m_checklist.equal_range(key);
    // if hostname is given , address will not be set, all the ip
    //belonging to the hostname will be forced to change status
    //if address is given , only the status of the ip is changed
    for (auto it = ret.first; it != ret.second; ++it)
    {
        it->second.setHostStatus(address, forceHostStatus);
    }
}

uint32_t
HMDataCheckList::addHostGroup(HMDataHostGroup& hostGroup)
{
    if (!m_guard)
    {
        HMDataHostCheck hostcheck;
        HMDataCheckParams check;
        auto vec = hostGroup.getHostList();

        if(!hostGroup.getHostCheck(hostcheck) || !hostGroup.getCheckParameters(check))
        {
            HMLog(HM_LOG_ERROR, "[CORE] Invalid Check type");
            return 0;
        }

        for(auto host = vec->begin(); host != vec->end(); ++host)
        {

            bool found = false;
            auto key = make_pair(*host, hostcheck);
            auto ret = m_checklist.equal_range(key);
            for(auto it = ret.first; it != ret.second; ++it)
            {

                if(it->second == check)
                {
                    it->second.addHostGroup(hostGroup.getName());
                    found = true;
                    break;
                }
            }

            if(!found)
            {
                auto it = m_checklist.insert(make_pair(key,check));
                it->second.addHostGroup(hostGroup.getName());
                string emptyRemoteHost = "";
                HMDataHostCheck tempHostCheck = hostcheck;
                tempHostCheck.setRemoteCheck(emptyRemoteHost);
                auto keyRemote = make_pair(*host, tempHostCheck);
                m_checklistReference.insert(make_pair(keyRemote, it));
            }
        }
        return vec->size();
    }
    return 0;
}

void
HMDataCheckList::initDNSCache(HMDNSCache& cache, HMWaitList& dnsWaitList)
{
    if (!m_guard)
    {
        for(auto it = m_checklist.begin(); it != m_checklist.end(); ++it)
        {
            uint8_t dualstack = it->first.second.getDualStack();
            if(dualstack & HM_DUALSTACK_IPV4_ONLY)
            {
                HMDNSLookup dnsHostCheck(it->first.second.getDnsPlugin(), false);
                cache.insertDNSEntry(it->first.first, dnsHostCheck, it->second.getTTL(), it->second.getTimeout());
            }
            if(dualstack & HM_DUALSTACK_IPV6_ONLY)
            {
                HMDNSLookup dnsHostCheck(it->first.second.getDnsPlugin(), true);
               cache.insertDNSEntry(it->first.first, dnsHostCheck, it->second.getTTL(), it->second.getTimeout());
            }
            dnsWaitList.insert(make_pair(HMDNSTypeMap(it->first.first, it->first.second.getDnsPlugin()), it->first.second));
        }
    }
}

void
HMDataCheckList::invalidateCheck(string& hostname, const HMIPAddress& ipAddress, HMDataHostCheck& hostCheck, HMStorage* store)
{
    auto key = make_pair(hostname, hostCheck);
    auto ret = m_checklist.equal_range(key);
    for(auto it = ret.first; it != ret.second; ++it)
    {
        HMDataCheckResult result;
        it->second.invalidateResult(ipAddress, result);
        store->purgeCheckResult(hostname,
                ipAddress,
                hostCheck,
                it->second);
    }
}

void
HMDataCheckList::insertCheck(string hostGroup, string host, HMDataHostCheck& hostCheck, HMDataCheckParams& checkParams, set<HMIPAddress>& ips)
{
    if (!m_guard)
    {
        auto key = make_pair(host, hostCheck);
        auto it = m_checklist.insert(make_pair(key,checkParams));
        for(set<HMIPAddress>::iterator iit = ips.begin();iit != ips.end(); ++iit)
        {
            it->second.emptyQuery(*iit);
        }
        if(!hostGroup.empty())
        {
            it->second.addHostGroup(hostGroup);
        }
        string emptyRemoteHost = "";
        HMDataHostCheck tempHostCheck = hostCheck;
        tempHostCheck.setRemoteCheck(emptyRemoteHost);
        auto keyRemote = make_pair(host, tempHostCheck);
        m_checklistReference.insert(make_pair(keyRemote, it));
    }
}

void
HMDataCheckList::insertEmptyQuery(HMWork *work, HMDataHostCheck& check, const HMIPAddress &address)
{
    auto key = make_pair(work->m_hostname, check);
    auto ret = m_checklist.equal_range(key);
    for (auto it = ret.first; it != ret.second; ++it)
    {
        it->second.emptyQuery(address);
        it->second.updateCheck(work->m_hostname,
                address,
                work->m_response,
                work->m_reason,
                work->m_start,
                work->m_end,
                check.getPort());
    }
}

string
HMDataCheckList::printChecks(bool printCheckInfo) const
{
    string output;
    for(auto it = m_checklist.begin(); it != m_checklist.end(); ++it)
    {
        if(printCheckInfo)
        {
            output = output + it->first.first + "\n" + it->first.second.printEntry('\t', true) + "\n" + it->second.printEntry() + "\n\n";
        }
        else
        {
            output = output + it->first.first + "\n" + it->first.second.printEntry('\t', true) + "\n\n";
        }
    }
    return output;
}

void
HMDataCheckList::setGuard (bool guard)
{
    m_guard = guard;
    HMLog(HM_LOG_DEBUG3, "[CORE] setGuard  value is set to %s", m_guard ? "true" : "false");
}

