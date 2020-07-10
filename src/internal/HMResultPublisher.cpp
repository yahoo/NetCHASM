// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include <utility>

#include "HMPublisherBase.h"
#include "HMResultPublisher.h"

using namespace std;
bool HMResultPublisher::registerPublisher(std::string name, unique_ptr<HMPublisherBase>& publisher)
{
    auto it = m_publishers.insert(make_pair(name, std::move(publisher)));
    return it.second;
}

bool HMResultPublisher::unregisterPublisher(std::string& name)
{
    m_publishers.erase(name);
    return true;

}

bool HMResultPublisher::publishResult(const string& hostName, const HMDataCheckResult& dataCheckResult, const int mark, set<string>& hostGroups, bool changed) const
{
    for(auto& it: m_publishers)
    {
        it.second->publishResult(hostName, dataCheckResult, mark, hostGroups, changed);
    }
    return true;
}

size_t HMResultPublisher::publishersCount()
{
    return m_publishers.size();
}

HMPublisherBase* HMResultPublisher::getpublisher(std::string name)
{
    auto it = m_publishers.find(name);
    if(it!=m_publishers.end())
    {
        return it->second.get();
    }
    return nullptr;
}
