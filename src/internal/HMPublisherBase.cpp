// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include <algorithm>
#include "HMPublisherBase.h"
using namespace std;

void
HMPublisherBase::addHostGroup(const string hostGroup)
{
    m_hostGroups.insert(hostGroup);
}

void
HMPublisherBase::addHostGroups(const set<string>& hostGroups)
{
    m_hostGroups.insert(hostGroups.begin(), hostGroups.end());
}

void
HMPublisherBase::removeHostGroup(const string hostGroup)
{
    m_hostGroups.erase(hostGroup);
}

void
HMPublisherBase::removeHostGroups(const set<string>& hostGroups)
{
    for(const string& hostGroup: hostGroups)
    {
        m_hostGroups.erase(hostGroup);
    }
}

bool HMPublisherBase::isPublishAll() const
{
    return m_publishAll;
}

void HMPublisherBase::setPublishAll(bool publishAll)
{
    m_publishAll = publishAll;
}

const std::set<std::string>& HMPublisherBase::getHostGroups() const
{
    return m_hostGroups;
}

bool
HMPublisherBase::matchingHostGroups(const set<string>& hostGroups, vector<string>& resultHostGroups) const
{
    set_intersection(m_hostGroups.begin(), m_hostGroups.end(), hostGroups.begin(), hostGroups.end(), std::back_inserter(resultHostGroups));
    return resultHostGroups.size();
}

void
HMPublisherBase::publishResult(const string& hostName, const HMDataCheckResult& dataCheckResult, const int mark, set<string>& hostGroups, bool changed) const
{
    vector<string> resultHostGroups;
    if(isPublishOnChange() && !changed)
    {
        return;
    }
    if(isPublishAll() || matchingHostGroups(hostGroups, resultHostGroups))
    {
        publish(hostName, dataCheckResult, mark, hostGroups);
    }
}

bool HMPublisherBase::isPublishOnChange() const
{
    return m_publishOnChange;
}

void HMPublisherBase::setPublishOnChange(bool publishOnChange)
{
    m_publishOnChange = publishOnChange;
}
