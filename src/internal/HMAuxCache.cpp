// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <vector>
#include <limits.h>
#include <shared_mutex>
#include <memory>
#include <set>
#include <errno.h>
#include <iostream>

#include "HMAuxCache.h"
#include "HMLogBase.h"
#include "HMAuxParser.h"

#ifdef USE_RAPIDXML
#include "HMAuxParserRapidXML.h"
#endif

using namespace std;

unique_ptr<HMAuxBase>
HMAuxBase::deserialize(char* buf, uint32_t size)
{
    unique_ptr<HMAuxBase> aux;
    if(buf == nullptr || size < sizeof(uint8_t))
    {
        return nullptr;
    }

    if(buf[0] == HM_OOB_FILE)
    {
        aux = make_unique<HMAuxOOB>();
    }
    else if(buf[0] == HM_LOAD_FILE)
    {
        aux = make_unique<HMAuxLoadFB>();
    }
    return (aux->internalDeserialize(buf, size)) ? move(aux) : unique_ptr<HMAuxBase>();
}

unique_ptr<HMAuxBase>
HMAuxLoadFB::clone() const
{
    unique_ptr<HMAuxLoadFB> pt = make_unique<HMAuxLoadFB>();
    pt->m_type = m_type;
    pt->m_host = m_host;
    pt->m_resource = m_resource;
    pt->m_ip = m_ip;
    pt->m_ts = m_ts;

    pt->m_datacenter = m_datacenter;
    pt->m_load = m_load;
    pt->m_target = m_target;
    pt->m_max = m_max;

    return move(pt);
}

unique_ptr<HMAuxBase>
HMAuxLoadFB::transfer() const
{
    unique_ptr<HMAuxLoadFB> pt = make_unique<HMAuxLoadFB>();
    pt->m_type = m_type;
    pt->m_host = move(m_host);
    pt->m_resource = move(m_resource);
    pt->m_ip = m_ip;
    pt->m_ts = m_ts;

    pt->m_datacenter = move(m_datacenter);
    pt->m_load = m_load;
    pt->m_target = m_target;
    pt->m_max = m_max;

    return move(pt);
}

uint32_t
HMAuxLoadFB::serialize(char* buf, uint32_t size) const
{
    uint32_t totalSize = sizeof(SerStruct) + m_host.size() + m_resource.size() + m_datacenter.size();
    if(buf == nullptr || size < totalSize)
    {
        return totalSize;
    }

    SerStruct* ptr = (SerStruct*)buf;

    ptr->m_type = m_type;
    ptr->m_hostSize = m_host.size();
    ptr->m_resourceSize = m_resource.size();
    ptr->m_ip = m_ip;
    ptr->m_ts = m_ts.getTimeSinceEpoch();
    ptr->m_datacenterSize = m_datacenter.size();
    ptr->m_load = m_load;
    ptr->m_target = m_target;
    ptr->m_max = m_max;

    char* target = buf + sizeof(SerStruct);

    if (ptr->m_hostSize > 0)
    {
        strncpy(target, &m_host.at(0), m_host.size());
        target += m_host.size();
    }
    if (ptr->m_resourceSize > 0)
    {
        strncpy(target, &m_resource.at(0), m_resource.size());
        target += m_resource.size();
    }
    if (ptr->m_datacenterSize > 0)
    {
        strncpy(target, &m_datacenter.at(0), m_datacenter.size());
    }

    return totalSize;
}

string
HMAuxLoadFB::print()
{
    stringstream ss;
    ss << "LoadFB Entry\n"
       << "Host:\t" << m_host << "\n"
       << "IP:\t" << m_ip.toString() << "\n"
       << "Resource:\t" << m_resource << "\n"
       << "Datacenter:\t" << m_datacenter << "\n"
       << "Load:\t" << m_load << "\n"
       << "Target:\t" << m_target << "\n"
       << "Max:\t" << m_max << "\n"
       << "Time Stamp:\t" << m_ts.print("(%a %b %d %H:%M:%S %Y)");
    return ss.str();
}

HMAuxLoadFB*
HMAuxLoadFB::getLFB()
{
    return this;
}

HMAuxOOB*
HMAuxLoadFB::getOOB()
{
    return nullptr;
}

bool
HMAuxLoadFB::internalDeserialize(char* buf, uint32_t size)
{
    if(buf == nullptr || size < sizeof(SerStruct))
    {
        return false;
    }

    SerStruct* ptr = (SerStruct*)buf;

    if(size < sizeof(SerStruct) + ptr->m_hostSize + ptr->m_resourceSize + ptr->m_datacenterSize)
    {
        return false;
    }

    m_type = HM_AUX_TYPE(ptr->m_type);
    m_ip = ptr->m_ip;
    m_ts.setTime(ptr->m_ts);
    m_load = ptr->m_load;
    m_target = ptr->m_target;
    m_max = ptr->m_max;

    char* src = buf + sizeof(SerStruct);

    if (ptr->m_hostSize > 0)
    {
        m_host.resize(ptr->m_hostSize);
        strncpy(&m_host.at(0), src, ptr->m_hostSize);
        src += ptr->m_hostSize;
    }
    if (ptr->m_resourceSize > 0)
    {
        m_resource.resize(ptr->m_resourceSize);
        strncpy(&m_resource.at(0), src, ptr->m_resourceSize);
        src += ptr->m_resourceSize;
    }
    if (ptr->m_datacenterSize > 0)
    {
        m_datacenter.resize(ptr->m_datacenterSize);
        strncpy(&m_datacenter.at(0), src, ptr->m_datacenterSize);
    }

    return true;
}

unique_ptr<HMAuxBase>
HMAuxOOB::clone() const
{
    unique_ptr<HMAuxOOB> pt = make_unique<HMAuxOOB>();
    pt->m_type = m_type;
    pt->m_host = m_host;
    pt->m_resource = m_resource;
    pt->m_ip = m_ip;
    pt->m_ts = m_ts;

    pt->m_forceDown = m_forceDown;
    pt->m_shed = m_shed;

    return move(pt);
}

unique_ptr<HMAuxBase>
HMAuxOOB::transfer() const
{
    unique_ptr<HMAuxOOB> pt = make_unique<HMAuxOOB>();
    pt->m_type = m_type;
    pt->m_host = move(m_host);
    pt->m_resource = move(m_resource);
    pt->m_ip = m_ip;
    pt->m_ts = m_ts;

    pt->m_forceDown = m_forceDown;
    pt->m_shed = m_shed;

    return move(pt);
}

uint32_t
HMAuxOOB::serialize(char* buf, uint32_t size) const
{
    uint32_t totalSize = sizeof(SerStruct) + m_host.size() + m_resource.size();
    if(buf == nullptr || size < totalSize)
    {
        return totalSize;
    }

    SerStruct* ptr = (SerStruct*)buf;

    ptr->m_type = m_type;
    ptr->m_hostSize = m_host.size();
    ptr->m_resourceSize = m_resource.size();
    ptr->m_ip = m_ip;
    ptr->m_ts = m_ts.getTimeSinceEpoch();
    ptr->m_forceDown = m_forceDown;
    ptr->m_shed = m_shed;

    char* target = buf + sizeof(SerStruct);

    if (ptr->m_hostSize > 0)
    {
        strncpy(target, &m_host.at(0), m_host.size());
        target += m_host.size();
    }
    if (ptr->m_resourceSize > 0)
    {
        strncpy(target, &m_resource.at(0), m_resource.size());
    }

    return totalSize;
}

string
HMAuxOOB::print()
{
    stringstream ss;
    ss << "Out-of-Band(OOB) Entry\n"
            << "Host:\t" << m_host << "\n"
            << "IP:\t" << m_ip.toString() << "\n"
            << "Resource:\t" << m_resource << "\n"
            << "Force Down:\t" << ((m_forceDown) ? "true\n" : "false\n")
            << "Shed:\t" << m_shed
            << "Time Stamp:\t" << m_ts.print("(%a %b %d %H:%M:%S %Y)");
    return ss.str();
}

HMAuxLoadFB*
HMAuxOOB::getLFB()
{
    return nullptr;
}

HMAuxOOB*
HMAuxOOB::getOOB()
{
    return this;
}

bool
HMAuxOOB::internalDeserialize(char* buf, uint32_t size)
{
    if(buf == nullptr || size < sizeof(SerStruct))
    {
        return false;
    }

    SerStruct* ptr = (SerStruct*)buf;

    if(size < sizeof(SerStruct) + ptr->m_hostSize + ptr->m_resourceSize)
    {
        return false;
    }

    m_type = HM_AUX_TYPE(ptr->m_type);
    m_ip = ptr->m_ip;
    m_ts.setTime(ptr->m_ts);
    m_forceDown = ptr->m_forceDown;
    m_shed = ptr->m_shed;

    char* src = buf + sizeof(SerStruct);

    if (ptr->m_hostSize > 0)
    {
        m_host.resize(ptr->m_hostSize);
        strncpy(&m_host.at(0), src, ptr->m_hostSize);
        src += ptr->m_hostSize;
    }
    if (ptr->m_resourceSize > 0)
    {
        m_resource.resize(ptr->m_resourceSize);
        strncpy(&m_resource.at(0), src, ptr->m_resourceSize);
    }

    return true;
}

bool
HMAuxKey::operator<(const HMAuxKey& k) const
{
    if(m_hostName != k.m_hostName)
    {
        return m_hostName < k.m_hostName;
    }
    if(m_address != k.m_address)
    {
        return m_address < k.m_address;
    }
    return m_sourceUrl < k.m_sourceUrl;
}

uint32_t
HMAuxKey::serialize(char* buf, uint32_t size) const
{
    uint32_t totalSize = sizeof(uint32_t) * 2 + m_hostName.size() + m_sourceUrl.size();
    if(buf == nullptr || size < totalSize)
    {
        return totalSize;
    }

    *buf = (uint32_t)m_hostName.size();
    *(buf + sizeof(uint32_t)) = (uint32_t)m_sourceUrl.size();
    buf += (sizeof(uint32_t) * 2);

    memcpy(buf, &m_address, sizeof(HMIPAddress));
    strncpy(buf + sizeof(HMIPAddress), &m_hostName.at(0), m_hostName.size());
    strncpy((buf + sizeof(HMIPAddress) + m_hostName.size()), &m_sourceUrl.at(0), m_sourceUrl.size());

    return totalSize;
}

bool
HMAuxKey::deserialize(char* buf, uint32_t size)
{
    if(buf == nullptr || size < (2 * sizeof(uint32_t)))
    {
        return false;
    }
    uint32_t hostSize = *buf;
    uint32_t sourceUrlSize = *(buf + sizeof(uint32_t));

    if(size < (2 * sizeof(uint32_t)) + sizeof(HMIPAddress) + hostSize + sourceUrlSize)
    {
        return false;
    }

    memcpy(&m_address, buf + (2 * sizeof(uint32_t)), sizeof(HMIPAddress));

    m_hostName.resize(hostSize);
    strncpy(&m_hostName.at(0), buf + sizeof(HMIPAddress) + (2 * sizeof(uint32_t)), hostSize);

    m_sourceUrl.resize(sourceUrlSize);
    strncpy(&m_sourceUrl.at(0), buf + sizeof(HMIPAddress) + (2 * sizeof(uint32_t)) + hostSize, sourceUrlSize);

    return true;
}

HMAuxInfo::HMAuxInfo(const HMAuxInfo& k)
{
    m_ts = k.m_ts;
    m_auxData.clear();
    for(auto it = k.m_auxData.begin(); it != k.m_auxData.end(); ++ it)
    {
        m_auxData.push_back(move((*it)->clone()));
    }
}

HMAuxInfo::HMAuxInfo(const HMAuxInfo&& k)
{
    m_ts = k.m_ts;
    m_auxData.clear();
    for(auto it = k.m_auxData.begin(); it != k.m_auxData.end(); ++ it)
    {
        m_auxData.push_back(move((*it)->transfer()));
    }
}

HMAuxInfo&
HMAuxInfo::operator=(const HMAuxInfo& k)
{
    m_ts = k.m_ts;
    m_auxData.clear();
    for(auto it = k.m_auxData.begin(); it != k.m_auxData.end(); ++ it)
    {
        m_auxData.push_back(move((*it)->clone()));
    }
    return *this;
}

HMAuxInfo&
HMAuxInfo::operator=(const HMAuxInfo&& k)
{
    m_ts = k.m_ts;
    m_auxData.clear();
    for(auto it = k.m_auxData.begin(); it != k.m_auxData.end(); ++ it)
    {
        m_auxData.push_back(move((*it)->transfer()));
    }
    return *this;
}

bool
HMAuxCache::storeAuxInfo(const string& hostname,
        const string& sourceURL,
        const HMIPAddress& address,
        string& auxStr,
        HM_AUX_DATA_TYPE auxDataType)
{
    HMAuxInfo auxInfo;
    if(parseAuxData(hostname, sourceURL, address, auxStr, auxInfo, auxDataType))
    {
        HMAuxKey key;
        key.m_hostName = hostname;
        key.m_sourceUrl = sourceURL;
        key.m_address = address;
        commitEntry(key, auxInfo);
        return true;
    }
    return false;
}

bool
HMAuxCache::storeAuxInfo(const string& hostname,
        const string& sourceURL,
        const HMIPAddress& address,
        HMAuxInfo& auxInfo)
{
    HMAuxKey key;
    key.m_hostName = hostname;
    key.m_sourceUrl = sourceURL;
    key.m_address = address;
    commitEntry(key, auxInfo);
    return true;
}
bool
HMAuxCache::updateAuxInfo(const string& hostname,
            const string& sourceURL,
            const HMIPAddress& address,
            HMAuxInfo& auxInfo)
{
    HMAuxKey key;
    key.m_hostName = hostname;
    key.m_sourceUrl = sourceURL;
    key.m_address = address;

    unique_lock<shared_timed_mutex> lock(m_sharedMutex);

    m_auxData.erase(key);
    m_auxData.insert(make_pair(key, auxInfo));

    return true;
}

bool
HMAuxCache::getAuxInfo(const string& hostname,
           const string& sourceURL,
           const HMIPAddress& address,
           HMAuxInfo& auxInfo)
{
    HMAuxKey key;
    key.m_hostName = hostname;
    key.m_sourceUrl = sourceURL;
    key.m_address = address;

    shared_lock<shared_timed_mutex> lock(m_sharedMutex);

    auto it = m_auxData.find(key);
    if(it != m_auxData.end())
    {
        auxInfo = it->second;
        return true;
    }

    return false;
}

bool HMAuxCache::genAuxData(HMAuxInfo& auxInfo,
        const HM_AUX_TYPE type,
        const std::string& hostGroup,
        std::string& output,
        HM_AUX_DATA_TYPE auxDataType)
{
    unique_ptr<HMAuxParser> parser;
    switch(auxDataType)
    {
#ifdef USE_RAPIDXML
    case HM_AUX_DATA_XML:
        parser = make_unique<HMAuxParserRapidXML>();
        return parser->genAuxData(auxInfo, type, hostGroup, output);
#endif
    }
    return false;

}

bool HMAuxCache::parseAuxData(const std::string& hostname,
        const std::string& sourceURL,
        const HMIPAddress& address,
        std::string& auxStr,
        HMAuxInfo& auxInfo,
        HM_AUX_DATA_TYPE auxDataType)
{
    unique_ptr<HMAuxParser> parser;
    switch (auxDataType)
    {
#ifdef USE_RAPIDXML
    case HM_AUX_DATA_XML:
        parser = make_unique<HMAuxParserRapidXML>();
        return parser->parseAuxData(hostname, sourceURL, address, auxStr, auxInfo);
#endif
    }
    return false;
}

void
HMAuxCache::commitEntry(HMAuxKey& key, HMAuxInfo& auxInfo)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);

    m_auxData.erase(key);
    m_auxData.insert(make_pair(key, auxInfo));
}

