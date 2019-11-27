// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_INTERNAL_HMDATAPACKING_H_
#define INCLUDE_INTERNAL_HMDATAPACKING_H_

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "HMAPI.h"
#include "HMConstants.h"
#include "HMDataHostGroup.h"
#include "netchasm/threadinfo.pb.h"
#include "netchasm/datahostgroup.pb.h"
#include "netchasm/ipaddress.pb.h"
#include "netchasm/datacheckresult.pb.h"
#include "netchasm/hostresults.pb.h"
#include "netchasm/hostgroupinfo.pb.h"
#include "netchasm/auxinfo.pb.h"
#include "netchasm/hostschdinfo.pb.h"
#include "netchasm/datahostcheck.pb.h"
#include "netchasm/generalparams.pb.h"

class HMDataPacking
{
public:
    virtual ~HMDataPacking() {}
    static uint64_t hton64(uint64_t);
    static uint64_t ntoh64(uint64_t);
    virtual void packIPAddress(const HMIPAddress& address, netchasm::IPAddress* pAddress) const;
    virtual void packIPAddress(const HMAPIIPAddress& address, netchasm::IPAddress* pAddress);
    virtual void unpackIPAddress(const netchasm::IPAddress& pAddress, HMAPIIPAddress& address);
    virtual void unpackIPAddress(const netchasm::IPAddress& pAddress, HMIPAddress& address) const;
    virtual void packDataCheckResult(const HMDataCheckResult& dataCheckResult, netchasm::DataCheckResult* pDataCheckResult) const;
    virtual void unpackDataCheckResult(const netchasm::DataCheckResult& pDataCheckResult, HMAPICheckResult& dataCheckResult);
    virtual void unpackDataCheckResult(const netchasm::DataCheckResult& pDataCheckResult, HMDataCheckResult& dataCheckResult) const;
    virtual void packDataCheckParam(const HMDataCheckParams& dataCheckParam, netchasm::DataCheckParam* pDataCheckParam);
    virtual void unpackDataCheckParam(const netchasm::DataCheckParam& pDataCheckParams, HMAPICheckInfo& dataCheckInfo);
    virtual void unpackDataCheckParam(const netchasm::DataCheckParam& pDataCheckParams, HMDataCheckParams& dataCheckParams);
    virtual void packCheckParamCheckResult(const HMDataCheckParams& dataCheckParams, const HMDataCheckResult& dataCheckResult, netchasm::CheckParamsCheckResult* pCheckParamCheckResult);
    virtual void unpackCheckParamCheckResult(netchasm::CheckParamsCheckResult& pCheckParamsCheckResults, HMAPICheckInfo& apiCheckInfo, HMAPICheckResult& apiCheckResult);
    virtual void unpackCheckParamCheckResult(netchasm::CheckParamsCheckResult& pCheckParamsCheckResults, HMDataCheckParams& dataCheckInfo, HMDataCheckResult& dataCheckResult);
    virtual void packDataHostGroup(const HMDataHostGroup& dataHostGroup, netchasm::DataHostGroup* pDataHostGroup);
    virtual void unpackDataHostGroup(const netchasm::DataHostGroup pDataHostGroup, HMAPICheckInfo& apiCheckInfo, std::vector<std::string>& hosts);
    virtual std::unique_ptr<char[]> packThreadInfo(HMAPIThreadInfo& tInfo, uint64_t& dataSize);
    virtual bool unpackThreadInfo(std::unique_ptr<char[]>& data, uint64_t dataSize, HMAPIThreadInfo& tInfo);
    virtual std::unique_ptr<char[]> packDataHostGroup(HMDataHostGroup& dataGroupInfo, uint64_t& dataSize);
    virtual bool unpackDataHostGroup(std::unique_ptr<char[]>& data, uint64_t dataSize, HMAPICheckInfo& dataGroupInfo, std::vector<std::string>& hosts);
    virtual std::unique_ptr<char[]> packDataCheckResults(std::vector<HMDataCheckResult>& dataCheckResults, uint64_t& dataSize);
    virtual bool unpackDataCheckResults(std::unique_ptr<char[]>& data, uint64_t dataSize, std::vector<HMAPICheckResult>& apiResults, std::string& hostName);
    virtual std::unique_ptr<char[]> packHostResults(std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>>& hostResults, uint64_t& dataSize);
    virtual std::unique_ptr<char[]> packHostResults(std::multimap<HMDataCheckParams, HMDataCheckResult>& hostResults, uint64_t& dataSize);
    virtual bool unpackHostResults(std::unique_ptr<char[]>& data, uint64_t dataSize, std::vector<std::pair<HMAPICheckInfo, HMAPICheckResult>>& hostResults);
    virtual bool unpackHostResults(std::unique_ptr<char[]>& data, uint64_t dataSize, std::multimap<HMDataCheckParams, HMDataCheckResult>& hostResults);
    virtual bool unpackHostResults(std::unique_ptr<char[]>& data, uint64_t dataSize, std::map<HMDataCheckParams, HMDataCheckResult>& hostResults);
    virtual std::unique_ptr<char[]> packHostGroupInfo(HMDataHostGroup& group,  std::vector<HMGroupCheckResult>& results,uint64_t& dataSize);
    virtual bool unpackHostGroupInfo(std::unique_ptr<char[]>& data, uint64_t dataSize, HMAPICheckInfo& apiCheckInfo, std::vector<std::string>& hosts, std::vector<HMAPICheckResult>& apiCheckResults);
    virtual std::unique_ptr<char[]> packAuxInfo(HMAuxInfo& auxInfo, std::string& hostName, HMIPAddress& addr, uint64_t& dataSize);
    virtual bool unpackAuxInfo(std::unique_ptr<char[]>& data, uint64_t dataSize, HMAPIAuxInfo& apiAuxInfoResult);
    virtual bool unpackAuxInfo(std::unique_ptr<char[]>& data, uint64_t dataSize, HMAuxInfo& auxInfoResult);

    virtual std::unique_ptr<char[]> packAuxInfo(std::vector<HMGroupAuxResult>& auxResults, uint64_t checkTTL, uint64_t& dataSize);
    virtual bool unpackAuxInfo(std::unique_ptr<char[]>& data, uint64_t dataSize, std::vector<HMAPIAuxInfo>& apiAuxResults);
    virtual std::unique_ptr<char[]> packHostSchedInfo(HMAPIDNSSchedInfo& dnsSchedInfo, uint64_t& dataSize);
    virtual bool unpackHostSchedInfo(std::unique_ptr<char[]>& data, uint64_t dataSize, HMAPIDNSSchedInfo& dnsSchedInfo);
    virtual std::unique_ptr<char[]> packDataHostCheck(HMDataHostCheck& dataHostCheck, uint64_t& dataSize);
    virtual bool unpackDataHostCheck(std::unique_ptr<char[]>& data, uint64_t dataSize, HMDataHostCheck& dataHostCheck);
    virtual std::unique_ptr<char[]> packList(std::vector<std::string>& listItems, uint64_t& dataSize);
    virtual bool unpackList(std::unique_ptr<char[]>& data, uint64_t dataSize, std::vector<std::string>& listItems);
    virtual std::unique_ptr<char[]> packIPAddresses(std::set<HMIPAddress>& addresses, uint64_t& dataSize);
    virtual std::unique_ptr<char[]> packIPAddresses(std::vector<HMAPIIPAddress>& addresses, uint64_t& dataSize);
    virtual bool unpackIPAddresses(std::unique_ptr<char[]>& data, uint64_t dataSize, std::set<HMIPAddress>& addresses);
    virtual bool unpackIPAddresses(std::unique_ptr<char[]>& data, uint64_t dataSize, std::vector<HMAPIIPAddress>& addresses);
    virtual std::unique_ptr<char[]> packBool(bool x, uint64_t& dataSize);
    virtual bool unpackBool(std::unique_ptr<char[]>& data, uint64_t dataSize);
    virtual std::unique_ptr<char[]> packListInt64(const std::set<int>& listItems, uint64_t& dataSize);
    virtual bool unpackListInt64(std::unique_ptr<char[]>& data, uint64_t dataSize, std::set<int>& listItems);

    template <class T>
    std::unique_ptr<char[]> packInt(T x, uint64_t& dataSize)
    {
        netchasm::Int pInt;
        pInt.set_data(x);
        std::unique_ptr<char[]> data;
        if(!pInt.IsInitialized())
        {
            return data;
        }
        dataSize = pInt.ByteSize();
        data = std::make_unique<char[]>(dataSize);
        pInt.SerializeToArray(data.get(), dataSize);
        return data;
    }

    template <class T>
    bool unpackInt(std::unique_ptr<char[]>& data, uint64_t dataSize, T& x)
    {
        netchasm::Int pInt;
        if(pInt.ParseFromArray(data.get(), dataSize))
        {
            x =  pInt.data();
            return true;
        }
        return false;
    }

    template <class T>
    std::unique_ptr<char[]> packUInt(T x, uint64_t& dataSize)
    {
        netchasm::Uint pUint;
        pUint.set_data(x);
        std::unique_ptr<char[]> data;
        if(!pUint.IsInitialized())
        {
            return data;
        }
        dataSize = pUint.ByteSize();
        data = std::make_unique<char[]>(dataSize);
        pUint.SerializeToArray(data.get(), dataSize);
        return data;
    }

    template <class T>
    bool unpackUInt(std::unique_ptr<char[]>& data, uint64_t dataSize, T& x)
    {
        netchasm::Uint pUint;
        if(pUint.ParseFromArray(data.get(), dataSize))
        {
            x = pUint.data();
            return true;
        }
        return false;
    }
};

#endif /* INCLUDE_INTERNAL_HMDATAPACKING_H_ */
