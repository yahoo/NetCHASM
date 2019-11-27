// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include <utility>

#include "HMPubSubDataPacking.h"

using namespace std;
unique_ptr<char[]>
HMPubSubDataPacking::packPublishResults(const string& hostName, const int mark, const set<string>& hostGroups, const HMDataCheckResult& dataCheckresult, uint64_t& dataSize) const
{
    netchasm::PublishResults pPublishResults;
    netchasm::DataCheckResult* result = new netchasm::DataCheckResult;
    pPublishResults.set_publisherversion(HM_PUBSUB_VERSION);
    m_dataPacking.packDataCheckResult(dataCheckresult, result);
    pPublishResults.set_allocated_results(result);
    pPublishResults.set_hostname(hostName);
    pPublishResults.set_mark(mark);
    for (const string& it : hostGroups)
    {
         pPublishResults.add_hostgroups(it);
    }
    unique_ptr<char[]> data;
    if(!pPublishResults.IsInitialized())
    {
        return data;
    }
    dataSize = pPublishResults.ByteSize();
    data = make_unique<char[]>(dataSize);
    pPublishResults.SerializeToArray(data.get(), dataSize);
    return data;
}


bool
HMPubSubDataPacking::unpackPublishResults(unique_ptr<char[]>& data, uint64_t& dataSize, uint8_t& publisherVersion,  string& hostName, int& mark, set<string>& hostGroups, HMDataCheckResult& dataCheckresult) const
{
    netchasm::PublishResults pPublishResults;
    if (pPublishResults.ParseFromArray(data.get(), dataSize))
    {
        publisherVersion = pPublishResults.publisherversion();
        hostName = pPublishResults.hostname();
        mark = pPublishResults.mark();
        m_dataPacking.unpackDataCheckResult(pPublishResults.results(), dataCheckresult);
        for(const string& hostgroup: pPublishResults.hostgroups())
        {
            hostGroups.insert(hostgroup);
        }
        return true;
    }
    return false;
}
