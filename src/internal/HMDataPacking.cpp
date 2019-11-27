#include "HMDataPacking.h"

using namespace google::protobuf::io;
using namespace std;

uint64_t HMDataPacking::hton64(uint64_t x)
{
    const uint32_t high = htonl(static_cast<uint32_t>(x >> 32));
    const uint32_t low = htonl(static_cast<uint32_t>(x & 0x00000000FFFFFFFF));
    return (static_cast<uint64_t>(low) << 32) | high;
}

uint64_t HMDataPacking::ntoh64(uint64_t x)
{
    const uint32_t high = ntohl(static_cast<uint32_t>(x >> 32));
    const uint32_t low = ntohl(static_cast<uint32_t>(x & 0x00000000FFFFFFFF));
    return (static_cast<uint64_t>(low) << 32) | high;
}

void
HMDataPacking::packIPAddress(const HMIPAddress& address, netchasm::IPAddress* pAddress) const
{
    pAddress->set_type(address.getType());
    if(address.getType() == AF_INET)
    {
        pAddress->set_addr0(address.addr4());
    }
    if(address.getType() == AF_INET6)
    {
        pAddress->set_addr0(address.addr6().__in6_u.__u6_addr32[0]);
        pAddress->set_addr1(address.addr6().__in6_u.__u6_addr32[1]);
        pAddress->set_addr2(address.addr6().__in6_u.__u6_addr32[2]);
        pAddress->set_addr3(address.addr6().__in6_u.__u6_addr32[3]);
    }
}


void
HMDataPacking::packIPAddress(const HMAPIIPAddress& address, netchasm::IPAddress* pAddress)
{
    pAddress->set_type(address.m_type);
    if(address.m_type == AF_INET)
    {
        pAddress->set_addr0(address.m_ip.addr);
    }
    if(address.m_type == AF_INET6)
    {
        pAddress->set_addr0(address.m_ip.addr6.__in6_u.__u6_addr32[0]);
        pAddress->set_addr1(address.m_ip.addr6.__in6_u.__u6_addr32[1]);
        pAddress->set_addr2(address.m_ip.addr6.__in6_u.__u6_addr32[2]);
        pAddress->set_addr3(address.m_ip.addr6.__in6_u.__u6_addr32[3]);
    }
}

void
HMDataPacking::unpackIPAddress(const netchasm::IPAddress& pAddress, HMAPIIPAddress& address)
{
    address.m_type = pAddress.type();
    if(address.m_type == AF_INET)
    {
        address.m_ip.addr = pAddress.addr0();
    }
    if(address.m_type == AF_INET6)
    {
        address.m_ip.addr6.__in6_u.__u6_addr32[0] = pAddress.addr0();
        address.m_ip.addr6.__in6_u.__u6_addr32[1] = pAddress.addr1();
        address.m_ip.addr6.__in6_u.__u6_addr32[2] = pAddress.addr2();
        address.m_ip.addr6.__in6_u.__u6_addr32[3] = pAddress.addr3();
    }
}

void
HMDataPacking::unpackIPAddress(const netchasm::IPAddress& pAddress, HMIPAddress& address) const
{
    if(pAddress.type() == AF_INET)
    {
        in_addr_t addr = pAddress.addr0();
        address.set(addr) ;
    }
    else if(pAddress.type() == AF_INET6)
    {
        in6_addr addr;
        addr.__in6_u.__u6_addr32[0] = pAddress.addr0();
        addr.__in6_u.__u6_addr32[1] = pAddress.addr1();
        addr.__in6_u.__u6_addr32[2] = pAddress.addr2();
        addr.__in6_u.__u6_addr32[3] = pAddress.addr3();
        address.set(addr);
    }
}

void
HMDataPacking::packDataCheckResult(const HMDataCheckResult& dataCheckResult, netchasm::DataCheckResult* pDataCheckResult) const
{
    netchasm::IPAddress *address = new netchasm::IPAddress;
    packIPAddress(dataCheckResult.m_address, address);
    pDataCheckResult->set_allocated_address(address);
    pDataCheckResult->set_start(dataCheckResult.m_start.getTimeSinceEpoch());
    pDataCheckResult->set_end(dataCheckResult.m_end.getTimeSinceEpoch());
    pDataCheckResult->set_responsetime(dataCheckResult.m_responseTime);
    pDataCheckResult->set_totalresponsetime(dataCheckResult.m_totalResponseTime);
    pDataCheckResult->set_minresponsetime(dataCheckResult.m_minResponseTime);
    pDataCheckResult->set_maxresponsetime(dataCheckResult.m_maxResponseTime);
    pDataCheckResult->set_smoothedresponsetime(dataCheckResult.m_smoothedResponseTime);
    pDataCheckResult->set_sumresponsetime(dataCheckResult.m_sumResponseTime);
    pDataCheckResult->set_numchecks(dataCheckResult.m_numChecks);
    pDataCheckResult->set_numresponses(dataCheckResult.m_numResponses);
    pDataCheckResult->set_numconnectfailures(dataCheckResult.m_numConnectFailures);
    pDataCheckResult->set_numfailures(dataCheckResult.m_numFailures);
    pDataCheckResult->set_numtimeouts(dataCheckResult.m_numTimeouts);
    pDataCheckResult->set_numflaps(dataCheckResult.m_numFlaps);
    pDataCheckResult->set_status(dataCheckResult.m_status);
    pDataCheckResult->set_response(dataCheckResult.m_response);
    pDataCheckResult->set_reason(dataCheckResult.m_reason);
    pDataCheckResult->set_softreason(dataCheckResult.m_softReason);
    pDataCheckResult->set_numfailedchecks(dataCheckResult.m_numFailedChecks);
    pDataCheckResult->set_numslowresponses(dataCheckResult.m_numSlowResponses);
    pDataCheckResult->set_port(dataCheckResult.m_port);
    pDataCheckResult->set_changetime(dataCheckResult.m_changeTime.getTimeSinceEpoch());
    pDataCheckResult->set_forcehostdown(dataCheckResult.m_forceHostDown);
    pDataCheckResult->set_queuechecktime(dataCheckResult.m_queueCheckTime.getTimeSinceEpoch());
    pDataCheckResult->set_checktime(dataCheckResult.m_checkTime.getTimeSinceEpoch());
}

void
HMDataPacking::unpackDataCheckResult(const netchasm::DataCheckResult& pDataCheckResult, HMAPICheckResult& dataCheckResult)
{
    unpackIPAddress(pDataCheckResult.address(), dataCheckResult.m_address);
    dataCheckResult.m_start = pDataCheckResult.start();
    dataCheckResult.m_end = pDataCheckResult.end();
    dataCheckResult.m_responseTime = pDataCheckResult.responsetime();
    dataCheckResult.m_totalResponseTime = pDataCheckResult.totalresponsetime();
    dataCheckResult.m_minResponseTime = pDataCheckResult.minresponsetime();
    dataCheckResult.m_maxResponseTime = pDataCheckResult.maxresponsetime();
    dataCheckResult.m_smoothedResponseTime = pDataCheckResult.smoothedresponsetime();
    dataCheckResult.m_sumResponseTime = pDataCheckResult.sumresponsetime();
    dataCheckResult.m_numChecks = pDataCheckResult.numchecks();
    dataCheckResult.m_numResponses = pDataCheckResult.numresponses();
    dataCheckResult.m_numConnectFailures = pDataCheckResult.numconnectfailures();
    dataCheckResult.m_numFailures = pDataCheckResult.numfailures();
    dataCheckResult.m_numTimeouts = pDataCheckResult.numtimeouts();
    dataCheckResult.m_numFlaps = pDataCheckResult.numflaps();
    dataCheckResult.m_status = pDataCheckResult.status();
    dataCheckResult.m_response = pDataCheckResult.response();
    dataCheckResult.m_reason = pDataCheckResult.reason();
    dataCheckResult.m_numFailedChecks = pDataCheckResult.numfailedchecks();
    dataCheckResult.m_numSlowResponses = pDataCheckResult.numslowresponses();
    dataCheckResult.m_port = pDataCheckResult.port();
    dataCheckResult.m_changeTime = pDataCheckResult.changetime();
    dataCheckResult.m_forceHostDown = pDataCheckResult.forcehostdown();
    dataCheckResult.m_queueCheckTime = pDataCheckResult.queuechecktime();
    dataCheckResult.m_checkTime = pDataCheckResult.checktime();
}

void
HMDataPacking::unpackDataCheckResult(const netchasm::DataCheckResult& pDataCheckResult, HMDataCheckResult& dataCheckResult) const
{
    unpackIPAddress(pDataCheckResult.address(), dataCheckResult.m_address);
    HMTimeStamp time;
    time.setTime(pDataCheckResult.start());
    dataCheckResult.m_start = time;
    time.setTime(pDataCheckResult.end());
    dataCheckResult.m_end = time;
    dataCheckResult.m_responseTime = pDataCheckResult.responsetime();
    dataCheckResult.m_totalResponseTime = pDataCheckResult.totalresponsetime();
    dataCheckResult.m_minResponseTime = pDataCheckResult.minresponsetime();
    dataCheckResult.m_maxResponseTime = pDataCheckResult.maxresponsetime();
    dataCheckResult.m_smoothedResponseTime = pDataCheckResult.smoothedresponsetime();
    dataCheckResult.m_sumResponseTime = pDataCheckResult.sumresponsetime();
    dataCheckResult.m_numChecks = pDataCheckResult.numchecks();
    dataCheckResult.m_numResponses = pDataCheckResult.numresponses();
    dataCheckResult.m_numConnectFailures = pDataCheckResult.numconnectfailures();
    dataCheckResult.m_numFailures = pDataCheckResult.numfailures();
    dataCheckResult.m_numTimeouts = pDataCheckResult.numtimeouts();
    dataCheckResult.m_numFlaps = pDataCheckResult.numflaps();
    dataCheckResult.m_status = (HM_HOST_STATUS)pDataCheckResult.status();
    dataCheckResult.m_response = (HM_RESPONSE)pDataCheckResult.response();
    dataCheckResult.m_reason = (HM_REASON)pDataCheckResult.reason();
    dataCheckResult.m_softReason = (HM_REASON)pDataCheckResult.softreason();
    dataCheckResult.m_numFailedChecks = pDataCheckResult.numfailedchecks();
    dataCheckResult.m_numSlowResponses = pDataCheckResult.numslowresponses();
    dataCheckResult.m_port = pDataCheckResult.port();
    time.setTime(pDataCheckResult.changetime());
    dataCheckResult.m_changeTime = time;
    dataCheckResult.m_forceHostDown = pDataCheckResult.forcehostdown();
    time.setTime(pDataCheckResult.queuechecktime());
    dataCheckResult.m_queueCheckTime = time;
    time.setTime(pDataCheckResult.checktime());
    dataCheckResult.m_checkTime = time;
}

void
HMDataPacking::packDataCheckParam(const HMDataCheckParams& dataCheckParam, netchasm::DataCheckParam* pDataCheckParam)
{
    pDataCheckParam->set_numcheckretries(dataCheckParam.getNumCheckRetries());
    pDataCheckParam->set_checkretrydelay(dataCheckParam.getCheckRetryDelay());
    pDataCheckParam->set_measurementoptions(dataCheckParam.getMeasurementOptions());
    pDataCheckParam->set_smoothingwindow(dataCheckParam.getSmoothingWindow());
    pDataCheckParam->set_groupthreshold(dataCheckParam.getGroupThreshold());
    pDataCheckParam->set_slowthreshold(dataCheckParam.getSlowThreshold());
    pDataCheckParam->set_maxflaps(dataCheckParam.getMaxFlaps());
    pDataCheckParam->set_checktimeout(dataCheckParam.getTimeout());
    pDataCheckParam->set_checkttl(dataCheckParam.getTTL());
    pDataCheckParam->set_flapthreshold(dataCheckParam.getFlapThreshold());
    pDataCheckParam->set_passthroughinfo(dataCheckParam.getPassthroughInfo());
}

void
HMDataPacking::unpackDataCheckParam(const netchasm::DataCheckParam& pDataCheckParams, HMAPICheckInfo& dataCheckInfo)
{
    dataCheckInfo.m_numCheckRetries = pDataCheckParams.numcheckretries();
    dataCheckInfo.m_checkRetryDelay = pDataCheckParams.checkretrydelay();
    dataCheckInfo.m_measurementOptions = pDataCheckParams.measurementoptions();
    dataCheckInfo.m_smoothingWindow = pDataCheckParams.smoothingwindow();
    dataCheckInfo.m_groupThreshold = pDataCheckParams.groupthreshold();
    dataCheckInfo.m_slowThreshold = pDataCheckParams.slowthreshold();
    dataCheckInfo.m_maxFlaps  = pDataCheckParams.maxflaps();
    dataCheckInfo.m_checkTimeout = pDataCheckParams.checktimeout();
    dataCheckInfo.m_checkTTL = pDataCheckParams.checkttl();
    dataCheckInfo.m_flapThreshold = pDataCheckParams.flapthreshold();
    dataCheckInfo.m_passthroughInfo = pDataCheckParams.passthroughinfo();
}

void
HMDataPacking::unpackDataCheckParam(const netchasm::DataCheckParam& pDataCheckParams, HMDataCheckParams& dataCheckParams)
{
    dataCheckParams.setCheckParameters(pDataCheckParams.numcheckretries(),
            pDataCheckParams.checkretrydelay(),
            pDataCheckParams.measurementoptions(),
            pDataCheckParams.smoothingwindow(),
            pDataCheckParams.groupthreshold(), pDataCheckParams.slowthreshold(),
            pDataCheckParams.maxflaps(), pDataCheckParams.checktimeout(),
            pDataCheckParams.checkttl(), pDataCheckParams.flapthreshold(),
            pDataCheckParams.passthroughinfo());
}

void
HMDataPacking::packCheckParamCheckResult(const HMDataCheckParams& dataCheckParams, const HMDataCheckResult& dataCheckResult, netchasm::CheckParamsCheckResult* pCheckParamCheckResult)
{

    netchasm::DataCheckParam *checkParams = new netchasm::DataCheckParam;
    netchasm::DataCheckResult *checkResult = new netchasm::DataCheckResult;
    packDataCheckParam(dataCheckParams, checkParams);
    packDataCheckResult(dataCheckResult, checkResult);
    pCheckParamCheckResult->set_allocated_checkparam(checkParams);
    pCheckParamCheckResult->set_allocated_checkresult(checkResult);
}

void
HMDataPacking::unpackCheckParamCheckResult(netchasm::CheckParamsCheckResult& pCheckParamsCheckResults, HMAPICheckInfo& apiCheckInfo, HMAPICheckResult& apiCheckResult)
{
    unpackDataCheckParam(pCheckParamsCheckResults.checkparam(), apiCheckInfo);
    unpackDataCheckResult(pCheckParamsCheckResults.checkresult(), apiCheckResult);
}

void
HMDataPacking::unpackCheckParamCheckResult(netchasm::CheckParamsCheckResult& pCheckParamsCheckResults, HMDataCheckParams& dataCheckInfo, HMDataCheckResult& dataCheckResult)
{
    unpackDataCheckParam(pCheckParamsCheckResults.checkparam(), dataCheckInfo);
    unpackDataCheckResult(pCheckParamsCheckResults.checkresult(), dataCheckResult);
}

void
HMDataPacking::packDataHostGroup(const HMDataHostGroup& dataHostGroup, netchasm::DataHostGroup* pDataHostGroup)
{
    pDataHostGroup->set_measurementoptions(dataHostGroup.getMeasurementOptions());
    pDataHostGroup->set_dualstack(dataHostGroup.getDualstack());
    pDataHostGroup->set_checktype(dataHostGroup.getCheckType());
    pDataHostGroup->set_port(dataHostGroup.getCheckPort());
    pDataHostGroup->set_numcheckretries(dataHostGroup.getNumCheckRetries());
    pDataHostGroup->set_checkretrydelay(dataHostGroup.getCheckRetryDelay());
    pDataHostGroup->set_smoothingwindow(dataHostGroup.getSmoothingWindow());
    pDataHostGroup->set_groupthreshold(dataHostGroup.getGroupThreshold());
    pDataHostGroup->set_slowthreshold(dataHostGroup.getSlowThreshold());
    pDataHostGroup->set_maxflaps(dataHostGroup.getMaxFlaps());
    pDataHostGroup->set_checktimeout(dataHostGroup.getCheckTimeout());
    pDataHostGroup->set_checkttl(dataHostGroup.getCheckTTL());
    pDataHostGroup->set_flapthreshold(dataHostGroup.getFlapThreshold());
    pDataHostGroup->set_passthroughinfo(dataHostGroup.getPassthroughInfo());
    pDataHostGroup->set_checkinfo(dataHostGroup.getCheckInfo());
    pDataHostGroup->set_tosvalue(dataHostGroup.getTOSValue());
    pDataHostGroup->set_dnstype(dataHostGroup.getDnsCheckPlugin());
    netchasm::IPAddress* address = new netchasm::IPAddress;
    packIPAddress(dataHostGroup.getSourceAddress(), address);
    pDataHostGroup->set_allocated_sourceaddress(address);
    for (auto it = dataHostGroup.getHostList()->begin(); it != dataHostGroup.getHostList()->end(); ++it)
    {
        pDataHostGroup->add_hosts(*it);
    }
    for (auto it = dataHostGroup.getHostGroupList()->begin();
            it != dataHostGroup.getHostGroupList()->end(); ++it)
    {
        pDataHostGroup->add_hostgroups(*it);
    }

}


void
HMDataPacking::unpackDataHostGroup(const netchasm::DataHostGroup pDataHostGroup, HMAPICheckInfo& apiCheckInfo, vector<string>& hosts)
{
    apiCheckInfo.m_measurementOptions = pDataHostGroup.measurementoptions();
    apiCheckInfo.m_ipv4 = pDataHostGroup.dualstack() & HM_DUALSTACK_IPV4_ONLY;
    apiCheckInfo.m_ipv6 = pDataHostGroup.dualstack() & HM_DUALSTACK_IPV6_ONLY;
    apiCheckInfo.m_checkType = (HM_API_CHECK_TYPE) pDataHostGroup.checktype();
    apiCheckInfo.m_port = pDataHostGroup.port();
    apiCheckInfo.m_numCheckRetries = pDataHostGroup.numcheckretries();
    apiCheckInfo.m_checkRetryDelay = pDataHostGroup.checkretrydelay();
    apiCheckInfo.m_smoothingWindow = pDataHostGroup.smoothingwindow();
    apiCheckInfo.m_groupThreshold = pDataHostGroup.groupthreshold();
    apiCheckInfo.m_slowThreshold = pDataHostGroup.slowthreshold();
    apiCheckInfo.m_maxFlaps = pDataHostGroup.maxflaps();
    apiCheckInfo.m_checkTimeout = pDataHostGroup.checktimeout();
    apiCheckInfo.m_checkTTL = pDataHostGroup.checkttl();
    apiCheckInfo.m_flapThreshold = pDataHostGroup.flapthreshold();
    apiCheckInfo.m_passthroughInfo = pDataHostGroup.passthroughinfo();
    apiCheckInfo.m_checkInfo = pDataHostGroup.checkinfo();
    apiCheckInfo.m_TOSValue = pDataHostGroup.tosvalue();
    apiCheckInfo.m_dnsCheckType = (HM_DNS_CHECK_TYPE)pDataHostGroup.dnstype();
    unpackIPAddress(pDataHostGroup.sourceaddress(), apiCheckInfo.m_sourceAddress);
    for (string host : pDataHostGroup.hostgroups())
    {
        apiCheckInfo.m_hostGroups.push_back(host);
    }
    for (string host : pDataHostGroup.hosts())
    {
        hosts.push_back(host);
    }

}
unique_ptr<char[]>
HMDataPacking::packThreadInfo(HMAPIThreadInfo& tInfo, uint64_t& dataSize)
{
    dataSize = 0;
    netchasm::ThreadInfo threadInfo;
    threadInfo.set_numidlethreads(tInfo.m_numIdleThreads);
    threadInfo.set_numthreads(tInfo.m_numThreads);
    unique_ptr<char[]> data;
    if(!threadInfo.IsInitialized())
    {
        return data;
    }
    dataSize = threadInfo.ByteSize();
    data = make_unique<char[]>(dataSize);
    threadInfo.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackThreadInfo(unique_ptr<char[]>& data, uint64_t dataSize, HMAPIThreadInfo& tInfo)
{
    netchasm::ThreadInfo threadInfo;
    if(threadInfo.ParseFromArray(data.get(), dataSize))
    {
        tInfo.m_numIdleThreads = threadInfo.numidlethreads();
        tInfo.m_numThreads = threadInfo.numthreads();
        return true;
    }
    return false;
}


unique_ptr<char[]>
HMDataPacking::packDataHostGroup(HMDataHostGroup& dataGroupInfo, uint64_t& dataSize)
{
    dataSize = 0;
    netchasm::DataHostGroup groupInfo;
    packDataHostGroup(dataGroupInfo, &groupInfo);
    unique_ptr<char[]> data;
    if(!groupInfo.IsInitialized())
    {
        return data;
    }
    dataSize = groupInfo.ByteSize();
    data = make_unique<char[]>(dataSize);
    groupInfo.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackDataHostGroup(unique_ptr<char[]>& data, uint64_t dataSize, HMAPICheckInfo& dataGroupInfo, vector<string>& hosts)
{
    netchasm::DataHostGroup groupInfo;
    if(groupInfo.ParseFromArray(data.get(), dataSize))
    {
        unpackDataHostGroup(groupInfo, dataGroupInfo, hosts);
        return true;
    }
    return false;
}


unique_ptr<char[]>
HMDataPacking::packDataCheckResults(vector<HMDataCheckResult>& dataCheckResults, uint64_t& dataSize)
{
    dataSize = 0;
    netchasm::DataCheckResults hostResults;
    for (HMDataCheckResult result : dataCheckResults)
    {
        netchasm::DataCheckResult *pResult = hostResults.add_results();
        packDataCheckResult(result, pResult);
    }
    unique_ptr<char[]> data;
    if(!hostResults.IsInitialized())
    {
        return data;
    }
    dataSize = hostResults.ByteSize();
    data = make_unique<char[]>(dataSize);
    hostResults.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackDataCheckResults(unique_ptr<char[]>& data, uint64_t dataSize, vector<HMAPICheckResult>& apiResults, string& hostName)
{
    netchasm::DataCheckResults hostResults;
    if(hostResults.ParseFromArray(data.get(), dataSize))
    {
        for (netchasm::DataCheckResult result : hostResults.results())
        {
            HMAPICheckResult apiResult;
            unpackDataCheckResult(result, apiResult);
            apiResult.m_host = hostName;
            apiResults.push_back(apiResult);
        }
        return true;
    }
    return false;
}

unique_ptr<char[]>
HMDataPacking::packHostResults(vector<pair<HMDataCheckParams, HMDataCheckResult>>& hostResults, uint64_t& dataSize)
{
    netchasm::HostResult pHostResults;
    for (auto it : hostResults)
    {
        netchasm::CheckParamsCheckResult* checkParamCheckResult = pHostResults.add_hostresults();
        packCheckParamCheckResult(it.first, it.second, checkParamCheckResult);
    }
    unique_ptr<char[]> data;
    if(!pHostResults.IsInitialized())
    {
        return data;
    }
    dataSize = pHostResults.ByteSize();
    data = make_unique<char[]>(dataSize);
    pHostResults.SerializeToArray(data.get(), dataSize);
    return data;
}

unique_ptr<char[]>
HMDataPacking::packHostResults(multimap<HMDataCheckParams, HMDataCheckResult>& hostResults, uint64_t& dataSize)
{
    netchasm::HostResult pHostResults;
    for (auto it : hostResults)
    {
        netchasm::CheckParamsCheckResult* checkParamCheckResult = pHostResults.add_hostresults();
        packCheckParamCheckResult(it.first, it.second, checkParamCheckResult);
    }
    unique_ptr<char[]> data;
    if(!pHostResults.IsInitialized())
    {
        return data;
    }
    dataSize = pHostResults.ByteSize();
    data = make_unique<char[]>(dataSize);
    pHostResults.SerializeToArray(data.get(), dataSize);
    return data;
}


bool
HMDataPacking::unpackHostResults(unique_ptr<char[]>& data, uint64_t dataSize, vector<pair<HMAPICheckInfo, HMAPICheckResult>>& hostResults)
{
    netchasm::HostResult pHostResults;
    if(pHostResults.ParseFromArray(data.get(), dataSize))
    {
        for (netchasm::CheckParamsCheckResult result : pHostResults.hostresults())
        {
            HMAPICheckInfo checkInfo;
            HMAPICheckResult checkResult;
            unpackCheckParamCheckResult(result, checkInfo, checkResult);
            hostResults.push_back(make_pair(std::move(checkInfo), std::move(checkResult)));
        }
        return true;
    }
    return false;
}

bool
HMDataPacking::unpackHostResults(unique_ptr<char[]>& data, uint64_t dataSize, multimap<HMDataCheckParams, HMDataCheckResult>& hostResults)
{
    netchasm::HostResult pHostResults;
    if(pHostResults.ParseFromArray(data.get(), dataSize))
    {
        for (netchasm::CheckParamsCheckResult result : pHostResults.hostresults())
        {
            HMDataCheckParams checkInfo;
            HMDataCheckResult checkResult;
            unpackCheckParamCheckResult(result, checkInfo, checkResult);
            hostResults.insert(make_pair(std::move(checkInfo), std::move(checkResult)));
        }
        return true;
    }
    return false;
}

bool
HMDataPacking::unpackHostResults(unique_ptr<char[]>& data, uint64_t dataSize, map<HMDataCheckParams, HMDataCheckResult>& hostResults)
{
    netchasm::HostResult pHostResults;
    if(pHostResults.ParseFromArray(data.get(), dataSize))
    {
        for (netchasm::CheckParamsCheckResult result : pHostResults.hostresults())
        {
            HMDataCheckParams checkInfo;
            HMDataCheckResult checkResult;
            unpackCheckParamCheckResult(result, checkInfo, checkResult);
            hostResults.insert(make_pair(std::move(checkInfo), std::move(checkResult)));
        }
        return true;
    }
    return false;
}

unique_ptr<char[]>
HMDataPacking::packHostGroupInfo(HMDataHostGroup& group,  vector<HMGroupCheckResult>& results, uint64_t& dataSize)
{
    netchasm::HostGroupInfo pHostGroupInfo;
    netchasm::DataHostGroup* pDataHostGroup = new netchasm::DataHostGroup;
    packDataHostGroup(group, pDataHostGroup);
    pHostGroupInfo.set_allocated_hostgroup(pDataHostGroup);
    for (auto it : results)
    {
         netchasm::HostGroupCheckResult* groupCheckResult = pHostGroupInfo.add_groupcheckresult();
         groupCheckResult->set_hostname(it.m_hostName);
         netchasm::DataCheckResult* result = new netchasm::DataCheckResult;
         packDataCheckResult(it.m_result, result);
         groupCheckResult->set_allocated_hostresults(result);
    }
    unique_ptr<char[]> data;
    if(!pHostGroupInfo.IsInitialized())
    {
        return data;
    }
    dataSize = pHostGroupInfo.ByteSize();
    data = make_unique<char[]>(dataSize);
    pHostGroupInfo.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackHostGroupInfo(unique_ptr<char[]>& data, uint64_t dataSize, HMAPICheckInfo& apiCheckInfo, vector<string>& hosts, vector<HMAPICheckResult>& apiCheckResults)
{
    netchasm::HostGroupInfo pHostGroupInfo;
    if(pHostGroupInfo.ParseFromArray(data.get(), dataSize))
    {
        unpackDataHostGroup(pHostGroupInfo.hostgroup(), apiCheckInfo, hosts);
        for(netchasm::HostGroupCheckResult pResult : pHostGroupInfo.groupcheckresult())
        {
            HMAPICheckResult result;
            result.m_host = pResult.hostname();
            unpackDataCheckResult(pResult.hostresults(), result);
            apiCheckResults.push_back(std::move(result));
        }
        return true;
    }
    return false;
}

unique_ptr<char[]>
HMDataPacking::packAuxInfo(HMAuxInfo& auxInfo, string& hostName, HMIPAddress& addr, uint64_t& dataSize)
{
    netchasm::AuxInfo pAuxInfo;
    pAuxInfo.set_host(hostName);
    pAuxInfo.set_ttl(0);
    pAuxInfo.set_updatetime(auxInfo.m_ts.getTimeSinceEpoch());
    netchasm::IPAddress* address = new netchasm::IPAddress;
    packIPAddress(addr, address);
    pAuxInfo.set_allocated_address(address);
    for (auto iit = auxInfo.m_auxData.begin();
            iit != auxInfo.m_auxData.end(); ++iit)
    {
        HMAuxLoadFB* lfb = (*iit)->getLFB();
        if (lfb)
        {
            netchasm::LFB *pLFB = pAuxInfo.add_lfb();
            pLFB->set_type(lfb->m_type);
            pLFB->set_ts(lfb->m_ts.getTimeSinceEpoch());
            pLFB->set_load(lfb->m_load);
            pLFB->set_target(lfb->m_target);
            pLFB->set_max(lfb->m_max);
            pLFB->set_host(lfb->m_host);
            pLFB->set_resource(lfb->m_resource);
            pLFB->set_datacenter(lfb->m_datacenter);
            continue;
        }
        HMAuxOOB* oob = (*iit)->getOOB();
        if (oob)
        {
            netchasm::OOB *pOOB = pAuxInfo.add_oob();
            pOOB->set_type(oob->m_type);
            pOOB->set_shed(oob->m_shed);
            pOOB->set_ts(oob->m_ts.getTimeSinceEpoch());
            pOOB->set_forcedown(oob->m_forceDown);
            pOOB->set_host(oob->m_host);
            pOOB->set_resource(oob->m_resource);
        }
    }
    unique_ptr<char[]> data;
    if(!pAuxInfo.IsInitialized())
    {
        return data;
    }
    dataSize = pAuxInfo.ByteSize();
    data = make_unique<char[]>(dataSize);
    pAuxInfo.SerializeToArray(data.get(), dataSize);
    return data;
}

bool HMDataPacking::unpackAuxInfo(unique_ptr<char[]>& data, uint64_t dataSize,
        HMAPIAuxInfo& apiAuxInfoResult)
{
    netchasm::AuxInfo pAuxInfoResult;
    if (pAuxInfoResult.ParseFromArray(data.get(), dataSize))
    {
        unpackIPAddress(pAuxInfoResult.address(), apiAuxInfoResult.m_address);
        apiAuxInfoResult.m_host = pAuxInfoResult.host();
        apiAuxInfoResult.m_ttl = pAuxInfoResult.ttl();
        apiAuxInfoResult.m_updatetime = pAuxInfoResult.updatetime();
        apiAuxInfoResult.m_lfb.clear();
        apiAuxInfoResult.m_oob.clear();
        for (netchasm::LFB lfb : pAuxInfoResult.lfb())
        {
            HMAPILFB apiLFB;
            apiLFB.m_datacenter = lfb.datacenter();
            apiLFB.m_host = lfb.host();
            apiLFB.m_load = lfb.load();
            apiLFB.m_max = lfb.max();
            apiLFB.m_resource = lfb.resource();
            apiLFB.m_target = lfb.target();
            apiLFB.m_ts = lfb.ts();
            apiLFB.m_type = lfb.type();
            apiAuxInfoResult.m_lfb.push_back(std::move(apiLFB));
        }
        for (netchasm::OOB oob : pAuxInfoResult.oob())
        {
            HMAPIOOB apiOOB;
            apiOOB.m_forceDown = oob.forcedown();
            apiOOB.m_host = oob.host();
            apiOOB.m_resource = oob.resource();
            apiOOB.m_shed = oob.shed();
            apiOOB.m_ts = oob.ts();
            apiOOB.m_type = oob.type();
            apiAuxInfoResult.m_oob.push_back(std::move(apiOOB));
        }
        return true;
    }
    return false;
}

bool HMDataPacking::unpackAuxInfo(unique_ptr<char[]>& data, uint64_t dataSize,
        HMAuxInfo& auxInfoResult)
{
    netchasm::AuxInfo pAuxInfoResult;
    if (pAuxInfoResult.ParseFromArray(data.get(), dataSize))
    {
        auxInfoResult.m_ts.setTime(pAuxInfoResult.updatetime());
        for (netchasm::LFB lfb : pAuxInfoResult.lfb())
        {
            HMAuxLoadFB lfbData;
            lfbData.m_datacenter = lfb.datacenter();
            lfbData.m_host = lfb.host();
            lfbData.m_load = lfb.load();
            lfbData.m_max = lfb.max();
            lfbData.m_resource = lfb.resource();
            lfbData.m_target = lfb.target();
            lfbData.m_ts.setTime(lfb.ts());
            lfbData.m_type = (HM_AUX_TYPE)lfb.type();
            auxInfoResult.m_auxData.push_back(lfbData.transfer());
        }
        for (netchasm::OOB oob : pAuxInfoResult.oob())
        {
            HMAuxOOB oobData;
            oobData.m_forceDown = oob.forcedown();
            oobData.m_host = oob.host();
            oobData.m_resource = oob.resource();
            oobData.m_shed = oob.shed();
            oobData.m_ts.setTime(oob.ts());
            oobData.m_type = (HM_AUX_TYPE)oob.type();
            auxInfoResult.m_auxData.push_back(oobData.transfer());
        }
        return true;
    }
    return false;
}

unique_ptr<char[]>
HMDataPacking::packAuxInfo(std::vector<HMGroupAuxResult>& auxResults, uint64_t checkTTL, uint64_t& dataSize)
{
    netchasm::AuxResults pAuxResults;
    for (auto it : auxResults)
    {
        netchasm::AuxInfo* pAuxInfo = pAuxResults.add_auxinfos();
        pAuxInfo->set_host(it.m_hostName);
        pAuxInfo->set_ttl(checkTTL);
        pAuxInfo->set_updatetime(it.m_info.m_ts.getTimeSinceEpoch());
        netchasm::IPAddress* address = new netchasm::IPAddress;
        packIPAddress(it.m_address, address);
        pAuxInfo->set_allocated_address(address);
        for (auto iit = it.m_info.m_auxData.begin();
                iit != it.m_info.m_auxData.end(); ++iit)
        {
            HMAuxLoadFB* lfb = (*iit)->getLFB();
            if (lfb)
            {
                netchasm::LFB *pLFB = pAuxInfo->add_lfb();
                pLFB->set_type(lfb->m_type);
                pLFB->set_ts(lfb->m_ts.getTimeSinceEpoch());
                pLFB->set_load(lfb->m_load);
                pLFB->set_target(lfb->m_target);
                pLFB->set_max(lfb->m_max);
                pLFB->set_host(lfb->m_host);
                pLFB->set_resource(lfb->m_resource);
                pLFB->set_datacenter(lfb->m_datacenter);
                continue;
            }
            HMAuxOOB* oob = (*iit)->getOOB();
            if (oob)
            {
                netchasm::OOB *pOOB = pAuxInfo->add_oob();
                pOOB->set_type(oob->m_type);
                pOOB->set_shed(oob->m_shed);
                pOOB->set_ts(oob->m_ts.getTimeSinceEpoch());
                pOOB->set_forcedown(oob->m_forceDown);
                pOOB->set_host(oob->m_host);
                pOOB->set_resource(oob->m_resource);
            }
        }
    }
    unique_ptr<char[]> data;
    if(!pAuxResults.IsInitialized())
    {
        return data;
    }
    dataSize = pAuxResults.ByteSize();
    data = make_unique<char[]>(dataSize);
    pAuxResults.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackAuxInfo(unique_ptr<char[]>& data, uint64_t dataSize, vector<HMAPIAuxInfo>& apiAuxResults)
{
    netchasm::AuxResults pAuxResults;
    if(pAuxResults.ParseFromArray(data.get(), dataSize))
    {
        for(netchasm::AuxInfo pAuxInfo : pAuxResults.auxinfos())
        {
            HMAPIAuxInfo auxInfo;
            unpackIPAddress(pAuxInfo.address(), auxInfo.m_address);
            auxInfo.m_host = pAuxInfo.host();
            auxInfo.m_ttl = pAuxInfo.ttl();
            auxInfo.m_updatetime = pAuxInfo.updatetime();
            for (netchasm::LFB lfb : pAuxInfo.lfb())
            {
                HMAPILFB apiLFB;
                apiLFB.m_datacenter = lfb.datacenter();
                apiLFB.m_host = lfb.host();
                apiLFB.m_load = lfb.load();
                apiLFB.m_max = lfb.max();
                apiLFB.m_resource = lfb.resource();
                apiLFB.m_target = lfb.target();
                apiLFB.m_ts = lfb.ts();
                apiLFB.m_type = lfb.type();
                auxInfo.m_lfb.push_back(std::move(apiLFB));
            }
            for (netchasm::OOB oob : pAuxInfo.oob())
            {
                HMAPIOOB apiOOB;
                apiOOB.m_forceDown = oob.forcedown();
                apiOOB.m_host = oob.host();
                apiOOB.m_resource = oob.resource();
                apiOOB.m_shed = oob.shed();
                apiOOB.m_ts = oob.ts();
                apiOOB.m_type = oob.type();
                auxInfo.m_oob.push_back(std::move(apiOOB));
            }
            apiAuxResults.push_back(auxInfo);
        }
        return true;
    }
    return false;
}


unique_ptr<char[]>
HMDataPacking::packHostSchedInfo(HMAPIDNSSchedInfo& dnsSchedInfo, uint64_t& dataSize)
{
    netchasm::DNSSchedInfo pDNSSchedInfo;
    pDNSSchedInfo.set_hasv4(dnsSchedInfo.m_hasv4);
    pDNSSchedInfo.set_hasv6(dnsSchedInfo.m_hasv6);
    pDNSSchedInfo.set_v4lastchecktime(dnsSchedInfo.m_v4LastCheckTime);
    pDNSSchedInfo.set_v6lastchecktime(dnsSchedInfo.m_v6LastCheckTime);
    pDNSSchedInfo.set_v4nextchecktime(dnsSchedInfo.m_v4NextCheckTime);
    pDNSSchedInfo.set_v6nextchecktime(dnsSchedInfo.m_v6NextCheckTime);
    pDNSSchedInfo.set_v4state(dnsSchedInfo.m_v4State);
    pDNSSchedInfo.set_v6state(dnsSchedInfo.m_v6State);
    for (auto it : dnsSchedInfo.m_hostScheduleInfo)
    {
        netchasm::HostSchedInfo* hostSchedInfo =  pDNSSchedInfo.add_hostschedinfo();
        hostSchedInfo->set_lastchecktime(it.m_lastCheckTime);
        hostSchedInfo->set_nextchecktime(it.m_nextCheckTime);
        hostSchedInfo->set_state(it.m_state);
        netchasm::IPAddress* address = new netchasm::IPAddress;
        packIPAddress(it.m_address, address);
        hostSchedInfo->set_allocated_address(address);
    }
    unique_ptr<char[]> data;
    if(!pDNSSchedInfo.IsInitialized())
    {
        return data;
    }
    dataSize = pDNSSchedInfo.ByteSize();
    data = make_unique<char[]>(dataSize);
    pDNSSchedInfo.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackHostSchedInfo(unique_ptr<char[]>& data, uint64_t dataSize, HMAPIDNSSchedInfo& dnsSchedInfo)
{
    netchasm::DNSSchedInfo pDNSSchedInfo;
    if(pDNSSchedInfo.ParseFromArray(data.get(), dataSize))
    {
        dnsSchedInfo.m_hasv4 = pDNSSchedInfo.hasv4();
        dnsSchedInfo.m_hasv6 = pDNSSchedInfo.hasv6();
        dnsSchedInfo.m_v4LastCheckTime = pDNSSchedInfo.v4lastchecktime();
        dnsSchedInfo.m_v6LastCheckTime = pDNSSchedInfo.v6lastchecktime();
        dnsSchedInfo.m_v4NextCheckTime = pDNSSchedInfo.v4nextchecktime();
        dnsSchedInfo.m_v6NextCheckTime = pDNSSchedInfo.v6nextchecktime();
        dnsSchedInfo.m_v4State = (HM_API_WORK_STATE)pDNSSchedInfo.v4state();
        dnsSchedInfo.m_v6State = (HM_API_WORK_STATE)pDNSSchedInfo.v6state();
        for(netchasm::HostSchedInfo pHostSchdInfo : pDNSSchedInfo.hostschedinfo())
        {
            HMAPIHostSchedInfo hostSchedInfo;
            unpackIPAddress(pHostSchdInfo.address(), hostSchedInfo.m_address);
            hostSchedInfo.m_lastCheckTime = pHostSchdInfo.lastchecktime();
            hostSchedInfo.m_nextCheckTime = pHostSchdInfo.nextchecktime();
            hostSchedInfo.m_state = (HM_API_WORK_STATE)pHostSchdInfo.state();
            dnsSchedInfo.m_hostScheduleInfo.push_back(hostSchedInfo);
        }
        return true;
    }
    return false;
}

unique_ptr<char[]>
HMDataPacking::packDataHostCheck(HMDataHostCheck& dataHostCheck, uint64_t& dataSize)
{
    netchasm::DataHostCheck pDataHostCheck;
    pDataHostCheck.set_port(dataHostCheck.getPort());
    pDataHostCheck.set_dualstack(dataHostCheck.getDualStack());
    pDataHostCheck.set_checktype(dataHostCheck.getCheckType());
    pDataHostCheck.set_checkinfo(dataHostCheck.getCheckInfo());
    pDataHostCheck.set_tosvalue(dataHostCheck.getTOSValue());
    pDataHostCheck.set_dnstype(dataHostCheck.getDnsPlugin());
    netchasm::IPAddress* address = new netchasm::IPAddress;
    packIPAddress(dataHostCheck.getSourceAddress(), address);
    pDataHostCheck.set_allocated_sourceaddress(address);
    unique_ptr<char[]> data;
    if(!pDataHostCheck.IsInitialized())
    {
        return data;
    }
    dataSize = pDataHostCheck.ByteSize();
    data = make_unique<char[]>(dataSize);
    pDataHostCheck.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackDataHostCheck(unique_ptr<char[]>& data, uint64_t dataSize, HMDataHostCheck& dataHostCheck)
{
    netchasm::DataHostCheck pDataHostCheck;
    if(pDataHostCheck.ParseFromArray(data.get(), dataSize))
    {
        HMDataHostGroup dataHostGroup("dummy");
        dataHostGroup.setCheckType((HM_CHECK_TYPE)pDataHostCheck.checktype());
        dataHostGroup.setPort(pDataHostCheck.port());
        dataHostGroup.setDualStack((HM_DUALSTACK)pDataHostCheck.dualstack());
        dataHostGroup.setCheckInfo(pDataHostCheck.checkinfo());
        dataHostGroup.setDnsCheckPlugin((HM_DNS_PLUGIN_CLASS)pDataHostCheck.dnstype());
        HMIPAddress address;
        unpackIPAddress(pDataHostCheck.sourceaddress(), address);
        dataHostGroup.setSourceAddress(address);
        dataHostGroup.setTOSValue(pDataHostCheck.tosvalue());
        dataHostCheck.setCheckParams(dataHostGroup);

        return true;
    }
    return false;
}

unique_ptr<char[]>
HMDataPacking::packList(vector<string>& listItems, uint64_t& dataSize)
{
    netchasm::List pListItems;
    for( const string& str : listItems)
    {
        pListItems.add_items(str);
    }
    unique_ptr<char[]> data;
    if(!pListItems.IsInitialized())
    {
        return data;
    }
    dataSize = pListItems.ByteSize();
    data = make_unique<char[]>(dataSize);
    pListItems.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackList(unique_ptr<char[]>& data, uint64_t dataSize, vector<string>& listItems)
{
    netchasm::List pListItems;
    if(pListItems.ParseFromArray(data.get(), dataSize))
    {
        for(const string&  str : pListItems.items())
        {
            listItems.push_back(str);
        }
        return true;
    }
    return false;
}


unique_ptr<char[]>
HMDataPacking::packIPAddresses(set<HMIPAddress>& addresses, uint64_t& dataSize)
{
    netchasm::IPAddresses pAddresses;
    for( const HMIPAddress& address : addresses)
    {
        netchasm::IPAddress *pAddress = pAddresses.add_addresses();
        packIPAddress(address, pAddress);
    }
    unique_ptr<char[]> data;
    if(!pAddresses.IsInitialized())
    {
        return data;
    }
    dataSize = pAddresses.ByteSize();
    data = make_unique<char[]>(dataSize);
    pAddresses.SerializeToArray(data.get(), dataSize);
    return data;
}

unique_ptr<char[]>
HMDataPacking::packIPAddresses(vector<HMAPIIPAddress>& addresses, uint64_t& dataSize)
{
    netchasm::IPAddresses pAddresses;
    for( const HMAPIIPAddress& address : addresses)
    {
        netchasm::IPAddress *pAddress = pAddresses.add_addresses();
        packIPAddress(address, pAddress);
    }
    unique_ptr<char[]> data;
    if(!pAddresses.IsInitialized())
    {
        return data;
    }
    dataSize = pAddresses.ByteSize();
    data = make_unique<char[]>(dataSize);
    pAddresses.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackIPAddresses(unique_ptr<char[]>& data, uint64_t dataSize, vector<HMAPIIPAddress>& addresses)
{
    netchasm::IPAddresses pAddresses;
    if(pAddresses.ParseFromArray(data.get(), dataSize))
    {
        for(const netchasm::IPAddress&  pAddress : pAddresses.addresses())
        {
            HMAPIIPAddress address;
            unpackIPAddress(pAddress, address);
            addresses.push_back(address);
        }
        return true;
    }
    return false;
}

bool
HMDataPacking::unpackIPAddresses(unique_ptr<char[]>& data, uint64_t dataSize, set<HMIPAddress>& addresses)
{
    netchasm::IPAddresses pAddresses;
    if(pAddresses.ParseFromArray(data.get(), dataSize))
    {
        for(const netchasm::IPAddress&  pAddress : pAddresses.addresses())
        {
            HMIPAddress address;
            unpackIPAddress(pAddress, address);
            addresses.insert(address);
        }
        return true;
    }
    return false;
}


unique_ptr<char[]>
HMDataPacking::packBool(bool x, uint64_t& dataSize)
{
    netchasm::Bool pBool;
    pBool.set_data(x);
    unique_ptr<char[]> data;
    if(!pBool.IsInitialized())
    {
        return data;
    }
    dataSize = pBool.ByteSize();
    data = make_unique<char[]>(dataSize);
    pBool.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackBool(unique_ptr<char[]>& data, uint64_t dataSize)
{
    netchasm::Bool pBool;
    if(pBool.ParseFromArray(data.get(), dataSize))
    {
        return pBool.data();
    }
    return false;
}

unique_ptr<char[]>
HMDataPacking::packListInt64(const set<int>& listItems, uint64_t& dataSize)
{
    netchasm::ListInt64 pListItems;
    for( const int value : listItems)
    {
        pListItems.add_items(value);
    }
    unique_ptr<char[]> data;
    if(!pListItems.IsInitialized())
    {
        return data;
    }
    dataSize = pListItems.ByteSize();
    data = make_unique<char[]>(dataSize);
    pListItems.SerializeToArray(data.get(), dataSize);
    return data;
}

bool
HMDataPacking::unpackListInt64(unique_ptr<char[]>& data, uint64_t dataSize, set<int>& listItems)
{
    netchasm::ListInt64 pListItems;
    if(pListItems.ParseFromArray(data.get(), dataSize))
    {
        for(const int value : pListItems.items())
        {
            listItems.insert(value);
        }
        return true;
    }
    return false;
}
