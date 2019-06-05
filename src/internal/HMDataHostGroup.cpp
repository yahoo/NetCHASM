// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <cstring>

#include "HMConstants.h"
#include "HMDataHostGroup.h"
#include "HMLogBase.h"

using namespace std;

bool HMDataHostGroup::operator<(const HMDataHostGroup& k) const
{
    if(m_groupName < k.m_groupName
            || m_measurementOptions < k.m_measurementOptions
            || m_dualstack < k.m_dualstack
            || m_checkType < k.m_checkType
            || m_port < k.m_port
            || m_checkInfo < k.m_checkInfo
            || m_numCheckRetries < k.m_numCheckRetries
            || m_checkRetryDelay < k.m_checkRetryDelay
            || m_smoothingWindow < k.m_smoothingWindow
            || m_groupThreshold < k.m_groupThreshold
            || m_slowThreshold < k.m_slowThreshold
            || m_maxFlaps < k.m_maxFlaps
            || m_checkTimeout < k.m_checkTimeout
            || m_checkTTL < k.m_checkTTL
            || m_flapThreshold < k.m_flapThreshold
            || m_passthroughInfo < k.m_passthroughInfo
            || m_checkPlugin < k.m_checkPlugin)
    {
        return true;
    }
    return false;
}

bool
HMDataHostGroup::operator==(const HMDataHostGroup& k) const
{
    if((m_groupName == k.m_groupName)
    && (m_measurementOptions == k.m_measurementOptions)
    && (m_dualstack == k.m_dualstack)
    && (m_checkType == k.m_checkType)
    && (m_port == k.m_port)
    && (m_checkInfo == k.m_checkInfo)
    && (m_numCheckRetries == k.m_numCheckRetries)
    && (m_checkRetryDelay == k.m_checkRetryDelay)
    && (m_smoothingWindow == k.m_smoothingWindow)
    && (m_groupThreshold == k.m_groupThreshold)
    && (m_slowThreshold == k.m_slowThreshold)
    && (m_maxFlaps == k.m_maxFlaps)
    && (m_checkTimeout == k.m_checkTimeout)
    && (m_checkTTL == k.m_checkTTL)
    && (m_flapThreshold == k.m_flapThreshold)
    && (m_passthroughInfo == k.m_passthroughInfo))
    {
        return true;
    }
    return false;
}

bool
HMDataHostGroup::operator!=(const HMDataHostGroup& k) const
{
    return !(*this == k);
}

bool
HMDataHostGroup::getHostCheck(HMDataHostCheck& check) const
{
    if(m_checkType <= HM_CHECK_AUX_HTTPS_NO_PEER_CHECK)
    {
        check.setCheckParams(m_checkType, m_checkPlugin, m_port, m_dualstack, m_checkInfo);
        return true;
    }
    return false;
}

bool
HMDataHostGroup::getCheckParameters(HMDataCheckParams& check) const
{
    check.setCheckParameters(m_numCheckRetries,
            m_checkRetryDelay,
            m_measurementOptions,
            m_smoothingWindow,
            m_groupThreshold,
            m_slowThreshold,
            m_maxFlaps,
            m_checkTimeout,
            m_checkTTL,
            m_flapThreshold,
            m_passthroughInfo);

    return true;
}

void
HMDataHostGroup::setHostGroupParameters(const HMDataHostGroup& hostGroup)
{
         m_measurementOptions = hostGroup.m_measurementOptions;
         m_dualstack = hostGroup.m_dualstack;
         m_dualstack = hostGroup.m_dualstack;
         m_port = hostGroup.m_port;
         m_checkType = hostGroup.m_checkType;
         m_checkInfo = hostGroup.m_checkInfo;
         m_numCheckRetries = hostGroup.m_numCheckRetries;
         m_checkRetryDelay = hostGroup.m_checkRetryDelay;
         m_smoothingWindow = hostGroup.m_smoothingWindow;
         m_groupThreshold = hostGroup.m_groupThreshold;
         m_slowThreshold = hostGroup.m_slowThreshold;
         m_maxFlaps = hostGroup.m_maxFlaps;
         m_checkTimeout = hostGroup.m_checkTimeout;
         m_checkTTL = hostGroup.m_checkTTL;
         m_flapThreshold = hostGroup.m_flapThreshold;
         m_passthroughInfo = hostGroup.m_passthroughInfo;
}

void
HMDataHostGroup::addHost(string& host)
{
    if(find(m_hosts.begin(),m_hosts.end(), host) == m_hosts.end())
    {
        m_hosts.push_back(host);
    }
}

void
HMDataHostGroup::setMeasurementOptions(uint16_t measurementOptions)
{
    m_measurementOptions = measurementOptions;
}

void
HMDataHostGroup::setDualStack(HM_DUALSTACK dualstack)
{
    m_dualstack = dualstack;
}

void
HMDataHostGroup::setCheckType(HM_CHECK_TYPE checkType)
{
    m_checkType = checkType;
}

void
HMDataHostGroup::setPort(uint16_t port)
{
    m_port = port;
}

void
HMDataHostGroup::setCheckInfo(string& checkInfo)
{
    m_checkInfo = checkInfo;
}

void
HMDataHostGroup::setNumCheckRetries(uint8_t numCheckRetries)
{
    m_numCheckRetries = numCheckRetries;
}

void
HMDataHostGroup::setCheckRetryDelay(uint32_t checkRetryDelay)
{
    m_checkRetryDelay = checkRetryDelay;
}

void
HMDataHostGroup::setCheckTimeout(uint64_t checkTimeout)
{
    m_checkTimeout = checkTimeout;
}

void
HMDataHostGroup::setCheckTTL(uint64_t checkTTL)
{
    m_checkTTL = checkTTL;
}

void
HMDataHostGroup::setGroupThreshold(uint32_t groupThreshold)
{
    m_groupThreshold = groupThreshold;
}

void
HMDataHostGroup::setSmoothingWindow(uint32_t smoothingWindow)
{
    m_smoothingWindow = smoothingWindow;
}

void
HMDataHostGroup::setFlapThreshold(uint32_t flapThreshold)
{
    m_flapThreshold = flapThreshold;
}

void
HMDataHostGroup::setMaxFlaps(uint32_t maxFlaps)
{
    m_maxFlaps = maxFlaps;
}

void
HMDataHostGroup::setSlowThreshold(uint32_t slowThreshold)
{
    m_slowThreshold = slowThreshold;
}

void
HMDataHostGroup::setPassthroughInfo(uint32_t passthroughInfo)
{
    m_passthroughInfo = passthroughInfo;
}

void
HMDataHostGroup::setCheckPlugin(HM_CHECK_PLUGIN_CLASS checkPlugin)
{
    m_checkPlugin = checkPlugin;
}

string
HMDataHostGroup::getName() const
{
    return m_groupName;
}

uint64_t
HMDataHostGroup::getCheckTTL() const
{
    return m_checkTTL;
}


HM_CHECK_TYPE
HMDataHostGroup::getCheckType() const
{
    return m_checkType;
}

uint16_t
HMDataHostGroup::getCheckPort() const
{
  return m_port;
}

uint16_t
HMDataHostGroup::getMeasurementOptions() const
{
    return m_measurementOptions;
}

uint64_t
HMDataHostGroup::getCheckTimeout() const
{
    return m_checkTimeout;
}

uint32_t
HMDataHostGroup::getGroupThreshold() const
{
    return m_groupThreshold;
}

string
HMDataHostGroup::getCheckInfo() const
{
    return m_checkInfo;
}

uint32_t
HMDataHostGroup::getFlapThreshold() const
{
    return m_flapThreshold;
}

HM_DUALSTACK
HMDataHostGroup::getDualstack() const
{
    return m_dualstack;
}

uint32_t
HMDataHostGroup::getPassthroughInfo() const
{
    return m_passthroughInfo;
}

uint32_t
HMDataHostGroup::getMaxFlaps() const
{
    return m_maxFlaps;
}

uint32_t
HMDataHostGroup::getSmoothingWindow() const
{
    return m_smoothingWindow;
}

uint8_t
HMDataHostGroup::getNumCheckRetries() const
{
    return m_numCheckRetries;
}

uint32_t
HMDataHostGroup::getSlowThreshold() const
{
    return m_slowThreshold;
}

uint32_t
HMDataHostGroup::getCheckRetryDelay() const
{
    return m_checkRetryDelay;
}

const vector<string>*
HMDataHostGroup::getHostList() const
{
    return &m_hosts;
}

bool
HMDataHostGroup::isValidHost(string& host) const
{
    return find(m_hosts.begin(),m_hosts.end(),host) != m_hosts.end();
}

uint32_t
HMDataHostGroup::serialize(char* buf, uint32_t size) const
{
    uint32_t totalSize = sizeof(SerStruct) + m_groupName.size() + m_checkInfo.size();
    uint32_t hostsSize = 0;
    for(auto it = m_hosts.begin(); it != m_hosts.end(); ++it)
    {
        hostsSize += (sizeof(uint32_t) + it->size());
    }
    totalSize += hostsSize;

    if(buf == nullptr || size < totalSize)
    {
        return totalSize;
    }

    SerStruct* ptr = (SerStruct*)buf;

    ptr->m_measurementOptions = m_measurementOptions;
    ptr->m_dualstack = m_dualstack;
    ptr->m_checkType = m_checkType;
    ptr->m_port = m_port;
    ptr->m_numCheckRetries = m_numCheckRetries;
    ptr->m_checkRetryDelay = m_checkRetryDelay;
    ptr->m_smoothingWindow = m_smoothingWindow;
    ptr->m_groupThreshold = m_groupThreshold;
    ptr->m_slowThreshold = m_slowThreshold;
    ptr->m_maxFlaps = m_maxFlaps;
    ptr->m_checkTimeout = m_checkTimeout;
    ptr->m_checkTTL = m_checkTTL;
    ptr->m_flapThreshold = m_flapThreshold;
    ptr->m_passthroughInfo = m_passthroughInfo;

    ptr->m_groupNameSize = m_groupName.size();
    ptr->m_checkInfoSize = m_checkInfo.size();
    ptr->m_numHosts = m_hosts.size();
    ptr->m_totalHostSize = hostsSize;

    // now we serialize the strings
    char* target = buf + sizeof(SerStruct);
    strncpy(target, &m_groupName.at(0), m_groupName.size());
    target += m_groupName.size();

    if(ptr->m_checkInfoSize)
    {
        strncpy(target, &m_checkInfo.at(0), m_checkInfo.size());
        target += m_checkInfo.size();
    }

    // now add the hosts
    for(auto it = m_hosts.begin(); it != m_hosts.end(); ++it)
    {
        *(uint32_t *)target = (uint32_t)it->size();
        strncpy((target + sizeof(uint32_t)) , &(it->at(0)), it->size());
        target += (sizeof(uint32_t) + it->size());
    }

    return totalSize;
}

bool
HMDataHostGroup::deserialize(char* buf, uint32_t size)
{
    if(buf == nullptr || size < sizeof(SerStruct))
    {
        return false;
    }

    SerStruct* ptr = (SerStruct*)buf;

    m_measurementOptions = ptr->m_measurementOptions;
    m_dualstack = HM_DUALSTACK(ptr->m_dualstack);
    m_checkType = HM_CHECK_TYPE(ptr->m_checkType);
    m_port = ptr->m_port;
    m_numCheckRetries = ptr->m_numCheckRetries;
    m_checkRetryDelay = ptr->m_checkRetryDelay;
    m_smoothingWindow = ptr->m_smoothingWindow;
    m_groupThreshold = ptr->m_groupThreshold;
    m_slowThreshold = ptr->m_slowThreshold;
    m_maxFlaps = ptr->m_maxFlaps;
    m_checkTimeout = ptr->m_checkTimeout;
    m_checkTTL = ptr->m_checkTTL;
    m_flapThreshold = ptr->m_flapThreshold;
    m_passthroughInfo = ptr->m_passthroughInfo;

    if(size < sizeof(SerStruct) + ptr->m_groupNameSize + ptr->m_checkInfoSize + ptr->m_totalHostSize)
    {
        return false;
    }

    char* src = buf + sizeof(SerStruct);

    m_groupName.resize(ptr->m_groupNameSize);
    strncpy(&m_groupName.at(0), src, ptr->m_groupNameSize);
    src += ptr->m_groupNameSize;

    if(ptr->m_checkInfoSize > 0)
    {
        m_checkInfo.resize(ptr->m_checkInfoSize);
        strncpy(&m_checkInfo.at(0), src, ptr->m_checkInfoSize);
        src += ptr->m_checkInfoSize;
    }
    m_hosts.resize(ptr->m_numHosts);
    for(uint32_t i = 0; i < ptr->m_numHosts; ++i)
    {
        uint32_t size = *(uint32_t*)src;
        m_hosts[i].resize(size);
        strncpy(&m_hosts[i].at(0), src + sizeof(uint32_t), size);
        src += (size + sizeof(uint32_t));
    }

    return true;
}

void 
HMDataHostGroup::getHash(HMHashMD5& hash)
{
    HMLog(HM_LOG_DEBUG3, "Hashing host group %s", m_groupName.c_str());
    hash.update(m_groupName.c_str(), (uint64_t) (m_groupName.length()));
    hash.update(&m_measurementOptions,
            (uint64_t) (sizeof(m_measurementOptions)));
    hash.update(&m_dualstack, (uint64_t) (sizeof(m_dualstack)));
    hash.update(&m_checkType, (uint64_t) (sizeof(m_checkType)));
    hash.update(&m_port, (uint64_t) (sizeof(m_port)));
    hash.update(&m_numCheckRetries, (uint64_t) (sizeof(m_numCheckRetries)));
    hash.update(&m_checkRetryDelay, (uint64_t) (sizeof(m_checkRetryDelay)));
    hash.update(&m_smoothingWindow, (uint64_t) (sizeof(m_smoothingWindow)));
    hash.update(&m_groupThreshold, (uint64_t) (sizeof(m_groupThreshold)));
    hash.update(&m_slowThreshold, (uint64_t) (sizeof(m_slowThreshold)));
    hash.update(&m_maxFlaps, (uint64_t) (sizeof(m_maxFlaps)));
    hash.update(&m_checkTimeout, (uint64_t) (sizeof(m_checkTimeout)));
    hash.update(&m_checkTTL, (uint64_t) (sizeof(m_checkTTL)));
    hash.update(&m_flapThreshold, (uint64_t) (sizeof(m_flapThreshold)));
    hash.update(&m_passthroughInfo, (uint64_t) (sizeof(m_passthroughInfo)));
    for (string host: m_hosts)
    {
        hash.update(host.c_str(), (uint64_t) (host.length()));
    }
}
