// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMPUBSUBDATAPACKING_H_
#define HMPUBSUBDATAPACKING_H_

#include <set>

#include "HMDataPacking.h"

class HMPubSubDataPacking
{
public:
    std::unique_ptr<char[]> packPublishResults(const std::string& hostName, const int mark, const std::set<std::string>& hostGroups, const HMDataCheckResult& dataCheckresult, uint64_t& dataSize) const ;
    bool unpackPublishResults(std::unique_ptr<char[]>& data, uint64_t& dataSize, uint8_t& publisherVersion,  std::string& hostName, int& mark, std::set<std::string>& hostGroups, HMDataCheckResult& dataCheckresult) const;

private:
    HMDataPacking m_dataPacking;
};

#endif /* HMPUBSUBDATAPACKING_H_ */
