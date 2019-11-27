// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include "HMPublisherBase.h"
#include "HMPublisherProto.h"
#include "HMPubSubDataPacking.h"
#include "HMLogBase.h"

using namespace std;


void
HMPublisherProto::publish(const string& hostName, const HMDataCheckResult& dataCheckResult, const int mark, set<string>& hostGroups) const
{
        HMPubSubDataPacking dataPacking;
        uint64_t buflen;
        unique_ptr<char[]> data = dataPacking.packPublishResults(hostName, mark, hostGroups, dataCheckResult, buflen);
        publish(data.get(), buflen);
}
