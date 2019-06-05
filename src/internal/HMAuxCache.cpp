// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <vector>
#include <limits.h>
#include <shared_mutex>
#include <memory>
#include <set>
#include <errno.h>
#include <iostream>

#include "rapidxml.h"
#include "rapidxml_print.h"

#include "HMAuxCache.h"
#include "HMLogBase.h"

using namespace rapidxml;
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
        string& auxInfo)
{
    if(parseXML(hostname, sourceURL, address, auxInfo))
    {
        return true;
    }
    return false;
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

bool
HMAuxCache::genAuxXML(HMAuxInfo& auxInfo,
        const HM_AUX_TYPE type,
        const string& rotation,
        string& xmlOutput)
{
    (void)rotation;
    stringstream ss;
    xml_document<> doc;
    xml_node<>* rootNode = nullptr;
    xml_node<>* resourceNode = nullptr;
    xml_node<>* hostNode = nullptr;
    xml_node<>* childNode = nullptr;
    xml_attribute<>* attr = nullptr;
    char* value = nullptr;
    string svalue;

    HMTimeStamp parseTime;

    map<string, xml_node<>*> datacenters;
    map<string, xml_node<>*> resources;

    switch(type)
    {
    case HM_LOAD_FILE:
        rootNode = doc.allocate_node(node_element, "load-file");
        ss.str(string());
        ss << auxInfo.m_ts.getTimeSinceEpoch();
        value = doc.allocate_string(ss.str().c_str());
        attr = doc.allocate_attribute("updated", value);
        rootNode->append_attribute(attr);
        doc.append_node(rootNode);

        for(auto entry = auxInfo.m_auxData.begin(); entry != auxInfo.m_auxData.end(); ++entry)
        {
            HMAuxBase* base = entry->get();
            if(base->m_type != HM_LOAD_FILE)
            {
                continue;
            }

            HMAuxLoadFB* lfb = dynamic_cast<HMAuxLoadFB*>(base);
            if(lfb == nullptr)
            {
                HMLog(HM_LOG_WARNING, "Failed to parse XML for %s host %s LFB type specified", base->m_host);
                continue;
            }

            auto it = resources.find(lfb->m_resource);
            if(it == resources.end())
            {
                resourceNode = doc.allocate_node(node_element, "resource");
                value = doc.allocate_string(lfb->m_resource.c_str());
                attr = doc.allocate_attribute("name", value);
                resourceNode->append_attribute(attr);
                rootNode->append_node(resourceNode);
                resources.insert(make_pair(lfb->m_resource, resourceNode));
            }
            else
            {
                resourceNode = it->second;
            }

            // now check for an existing node
            bool skip = false;
            hostNode = resourceNode->first_node("host");
            while(hostNode != nullptr)
            {
                attr = hostNode->first_attribute("name");
                svalue = attr->value();
                if(svalue == lfb->m_host)
                {
                    childNode = hostNode->first_node("time");
                    svalue = childNode->value();
                    parseTime.setTime(atol(svalue.c_str()));
                    if(parseTime.getTimeSinceEpoch() < lfb->m_ts.getTimeSinceEpoch())
                    {
                        // we need to update this value
                        resourceNode->remove_node(hostNode);
                    }
                    else
                    {
                        // no need to add the attribute
                        skip = true;
                    }
                    break;
                }
                hostNode = hostNode->next_sibling("host");
            }
            if(skip)
            {
                continue;
            }

            hostNode = doc.allocate_node(node_element, "host");
            value = doc.allocate_string(lfb->m_host.c_str());
            attr = doc.allocate_attribute("name", value);
            hostNode->append_attribute(attr);

            ss.str(string());
            ss << lfb->m_ts.getTimeSinceEpoch();
            value = doc.allocate_string(ss.str().c_str());
            childNode = doc.allocate_node(node_element, "time", value);
            hostNode->append_node(childNode);

            ss.str(string());
            ss << lfb->m_load;
            value = doc.allocate_string(ss.str().c_str());
            childNode = doc.allocate_node(node_element, "load", value);
            hostNode->append_node(childNode);

            ss.str(string());
            ss << lfb->m_target;
            value = doc.allocate_string(ss.str().c_str());
            childNode = doc.allocate_node(node_element, "target", value);
            hostNode->append_node(childNode);

            ss.str(string());
            ss << lfb->m_max;
            value = doc.allocate_string(ss.str().c_str());
            childNode = doc.allocate_node(node_element, "max", value);
            hostNode->append_node(childNode);
            resourceNode->append_node(hostNode);
        }

        break;
    case HM_OOB_FILE:
        rootNode = doc.allocate_node(node_element, "oob-file");
        ss.str(string());
        ss << auxInfo.m_ts.getTimeSinceEpoch();
        value = doc.allocate_string(ss.str().c_str());
        attr = doc.allocate_attribute("updated", value);
        rootNode->append_attribute(attr);
        doc.append_node(rootNode);

        for(auto entry = auxInfo.m_auxData.begin(); entry != auxInfo.m_auxData.end(); ++entry)
        {
            HMAuxBase* base = entry->get();
            if(base->m_type != HM_OOB_FILE)
            {
                continue;
            }

            HMAuxOOB* oob = dynamic_cast<HMAuxOOB*>(base);
            if(oob == nullptr)
            {
                HMLog(HM_LOG_WARNING, "Failed to parse XML for %s host %s LFB type specified", base->m_host);
                continue;
            }

            auto it = resources.find(oob->m_resource);
            if(it == resources.end())
            {
                resourceNode = doc.allocate_node(node_element, "resource-oob");
                value = doc.allocate_string(oob->m_resource.c_str());
                attr = doc.allocate_attribute("name", value);
                resourceNode->append_attribute(attr);
                rootNode->append_node(resourceNode);
                resources.insert(make_pair(oob->m_resource, resourceNode));
            }
            else
            {
                resourceNode = it->second;
            }

            // now check for an existing node
            bool skip = false;
            hostNode = resourceNode->first_node("host");
            while(hostNode != nullptr)
            {
                attr = hostNode->first_attribute("name");
                svalue = attr->value();
                if(svalue == oob->m_host)
                {
                    childNode = hostNode->first_node("time");
                    svalue = childNode->value();
                    parseTime.setTime(atol(svalue.c_str()));
                    if(parseTime.getTimeSinceEpoch() < oob->m_ts.getTimeSinceEpoch())
                    {
                        // we need to update this value
                        resourceNode->remove_node(hostNode);
                    }
                    else
                    {
                        // no need to add the attribute
                        skip = true;
                    }
                    break;
                }
                hostNode = hostNode->next_sibling("host");
            }
            if(skip)
            {
                continue;
            }

            hostNode = doc.allocate_node(node_element, "host");
            value = doc.allocate_string(oob->m_host.c_str());
            attr = doc.allocate_attribute("name", value);
            hostNode->append_attribute(attr);

            ss.str(string());
            ss << oob->m_ts.getTimeSinceEpoch();
            value = doc.allocate_string(ss.str().c_str());
            childNode = doc.allocate_node(node_element, "time", value);
            hostNode->append_node(childNode);

            ss.str(string());
            ss << ((oob->m_forceDown) ? "true" : "false");
            value = doc.allocate_string(ss.str().c_str());
            childNode = doc.allocate_node(node_element, "force-down", value);
            hostNode->append_node(childNode);
            if(oob->m_shed!=0)
            {
                ss.str(string());
                ss << oob->m_shed;
                value = doc.allocate_string(ss.str().c_str());
                childNode = doc.allocate_node(node_element, "shed", value);
                hostNode->append_node(childNode);
            }
            resourceNode->append_node(hostNode);
        }
        break;
    }

    // print to the string
    ss.str(string());
    ss << doc;
    xmlOutput = ss.str();
    doc.clear();
    return true;
}

bool
HMAuxCache::parseXML(const string& hostname,
        const string& sourceURL,
        const HMIPAddress& address,
        string& auxInfo)
{
    HMLog(HM_LOG_DEBUG3,
            "[CURLCHECK] HMAuxCache::parseXML: url %s from host %s@%s", hostname.c_str(), sourceURL.c_str(), address.toString().c_str());
    // I hate this copy so much. But for now I will leave it to fix in profiling
    vector<char> buffer(auxInfo.begin(), auxInfo.end());
    buffer.push_back('\0');

    xml_node<>* rootNode;
    xml_document<char> doc;

    try
    {
        doc.parse<0>(&buffer[0]);
    }
    catch (parse_error &e)
    {
        HMLog(HM_LOG_INFO,
                "[CURLCHECK] HMAuxCache::parseXML: parse_error exception "
                "parsing url %s/%s, reason = %s",
                hostname.c_str(), sourceURL.c_str(), e.what());
        doc.clear();
        return false;
    }
    catch (...)
    {
        HMLog(HM_LOG_INFO,
                "[CURLCHECK] HMAuxCache::parseXML: unknown exception "
                "parsing url %s@%s",
                hostname.c_str(), sourceURL.c_str());
        doc.clear();
        return false;
    }

    rootNode = doc.first_node();

    if(rootNode == nullptr)
    {
        doc.clear();
        return false;
    }

    string rootName(rootNode->name());
    if(rootName == "load-file")
    {
        return parseNewLFB(hostname, sourceURL, address, doc);
    }
    else if(rootName == "oob-file")
    {
        return parseOOB(hostname, sourceURL, address, doc);
    }
    else
    {
        HMLog(HM_LOG_INFO, "XML Type Not Supported when querying %s %s %s",
                hostname.c_str(),
                sourceURL.c_str(),
                address.toString().c_str());
        doc.clear();
        return false;
    }
}

bool
HMAuxCache::parseNewLFB(const string& hostname,
        const string& sourceURL,
        const HMIPAddress& address,
        xml_document<char>& doc)
{
    HMAuxInfo auxInfo;
    HMTimeStamp filets;
    HMTimeStamp ts;
    HMAuxKey key;
    key.m_hostName = hostname;
    key.m_sourceUrl = sourceURL;
    key.m_address = address;

    xml_attribute<>* attr;
    xml_node<>* rootNode = doc.first_node();
    xml_node<>* resourceNode = nullptr;
    xml_node<>* hostNode = nullptr;
    xml_node<>* currentNode = nullptr;

    char* endptr = nullptr;

    attr = rootNode->first_attribute("updated");
    if(attr == nullptr)
    {
        HMLog(HM_LOG_INFO,
                "XML Load File does not contain a timestamp attribute for %s with checkinfo %s",
                hostname.c_str(), sourceURL.c_str());
        doc.clear();
        return false;
    }

    errno = 0;
    int64_t tempTime = strtol((const char*) attr->value(), &endptr, 10);
    if ((errno == ERANGE && (tempTime == LONG_MAX || tempTime == LONG_MIN))
            || (errno != 0 && tempTime == 0))
    {
        HMLog(HM_LOG_INFO,
                "XML Load file time tag has invalid value for %s with checkinfo %s",
                hostname.c_str(),
                sourceURL.c_str());
        doc.clear();
        return false;
    }
    filets.setTime(tempTime);
    auxInfo.m_ts.setTime(tempTime);

    resourceNode = rootNode->first_node("resource");
    if(resourceNode == nullptr)
    {
        HMLog(HM_LOG_DEBUG,
                "XML Load file does not contain a resource tag for %s with checkinfo %s",
                hostname.c_str(), sourceURL.c_str());
        doc.clear();
        return false;
    }

    while(resourceNode != nullptr)
    {
        attr = resourceNode->first_attribute("name");
        if(attr == nullptr)
        {
            HMLog(HM_LOG_INFO,
                    "XML Load file resource node does not contain a name attribute for %s with checkinfo %s",
                    hostname.c_str(), sourceURL.c_str());
            doc.clear();
            return false;
        }

        HMAuxLoadFB lfb;
        lfb.m_type = HM_LOAD_FILE;
        lfb.m_ip = address;
        lfb.m_datacenter = "";
        lfb.m_resource = attr->value();
        lfb.m_load = 0;
        lfb.m_target = 0;
        lfb.m_max = 0;

        hostNode = resourceNode->first_node("host");
        if(hostNode == nullptr)
        {
            HMLog(HM_LOG_DEBUG,
                    "XML Load file resource node does not contain a host tag for %s with checkinfo %s",
                    hostname.c_str(), sourceURL.c_str());
            doc.clear();
            return false;
        }

        while(hostNode != nullptr)
        {
            attr =  hostNode->first_attribute("name");
            if (attr == nullptr)
            {
                HMLog(HM_LOG_INFO,
                        "XML Load file host node does not contain a name attribute for %s with checkinfo %s",
                        hostname.c_str(), sourceURL.c_str());
                doc.clear();
                return false;
            }
            lfb.m_host = attr->value();

            currentNode = hostNode->first_node("load");
            if(currentNode == nullptr)
            {
                HMLog(HM_LOG_DEBUG3,
                        "XML Load file resource node does not contain a load tag for %s with checkinfo %s",
                        hostname.c_str(), sourceURL.c_str());
            }
            else
            {
                errno = 0;
                lfb.m_load = strtol((const char*)currentNode->value(), &endptr, 10);
                if((errno == ERANGE && (lfb.m_load == LONG_MAX || lfb.m_load == LONG_MIN))
                        || (errno != 0 && lfb.m_load == 0))
                {
                    HMLog(HM_LOG_DEBUG3,
                            "XML Load file resource node load tag has invalid value for %s with checkinfo %s",
                            hostname.c_str(), sourceURL.c_str());
                    lfb.m_load = 0;
                }
            }

            currentNode = hostNode->first_node("target");
            if(currentNode == nullptr)
            {
                HMLog(HM_LOG_DEBUG3,
                        "XML Load file resource node does not contain a target tag for %s with checkinfo %s",
                        hostname.c_str(), sourceURL.c_str());
            }
            else
            {
                errno = 0;
                lfb.m_target = strtol((const char*)currentNode->value(), &endptr, 10);
                if((errno == ERANGE && (lfb.m_target == LONG_MAX || lfb.m_target == LONG_MIN))
                        || (errno != 0 && lfb.m_target == 0))
                {
                    HMLog(HM_LOG_DEBUG3,
                            "XML Load file resource node target tag has invalid value for %s with checkinfo %s",
                            hostname.c_str(), sourceURL.c_str());
                    lfb.m_target = 0;
                }
            }

            currentNode = hostNode->first_node("max");
            if(currentNode == nullptr)
            {
                HMLog(HM_LOG_DEBUG3,
                        "XML Load object resource node does not contain a max tag for %s with checkinfo %s",
                        hostname.c_str(), sourceURL.c_str());
            }
            else
            {
                errno = 0;
                lfb.m_max = strtol((const char*)currentNode->value(), &endptr, 10);
                if((errno == ERANGE && (lfb.m_max == LONG_MAX || lfb.m_max == LONG_MIN))
                        || (errno != 0 && lfb.m_max == 0))
                {
                    HMLog(HM_LOG_DEBUG3,
                            "XML Load object resource node target tag has an invalid max value for %s with checkinfo %s",
                            hostname.c_str(), sourceURL.c_str());
                    lfb.m_max = 0;
                }
            }

            currentNode = hostNode->first_node("time");
            if (currentNode == nullptr)
            {
                HMLog(HM_LOG_DEBUG3,
                        "XML Load file resource node does not contain a time tag for %s with checkinfo %s using file time",
                        hostname.c_str(), sourceURL.c_str());
                ts = filets;
            }
            else
            {
                errno = 0;
                tempTime = strtol((const char*) currentNode->value(), &endptr, 10);

                if ((errno == ERANGE && (tempTime == LONG_MAX || tempTime == LONG_MIN))
                        || (errno != 0 && tempTime == 0))
                {
                    HMLog(HM_LOG_DEBUG3,
                            "XML Load file resource node time tag has invalid value for %s with checkinfo %s using file time",
                            hostname.c_str(),
                            sourceURL.c_str());

                    ts = filets;
                }
                else
                {
                    ts.setTime(tempTime);
                }
            }
            // According to Dan, if the filets is older than the resource, record the filets for the entry
            lfb.m_ts = ts;
            auxInfo.m_auxData.push_back(make_unique<HMAuxLoadFB>(lfb));

            hostNode = hostNode->next_sibling("host");
        }
        resourceNode = resourceNode->next_sibling("resource");
    }
    commitEntry(key, auxInfo);
    doc.clear();
    return true;
}

bool
HMAuxCache::parseOOB(const string& hostname,
        const string& sourceURL,
        const HMIPAddress& address,
        xml_document<char>& doc)
{
    HMAuxInfo auxInfo;
    HMTimeStamp ts;
    HMTimeStamp filets;
    string resource;
    HMAuxKey key;
    key.m_hostName = hostname;
    key.m_sourceUrl = sourceURL;
    key.m_address = address;

    xml_attribute<>* attr;
    xml_node<>* rootNode = doc.first_node();
    xml_node<>* resourceNode = nullptr;
    xml_node<>* hostNode = nullptr;
    xml_node<>* currentNode = nullptr;

    char* endptr = nullptr;

    attr = rootNode->first_attribute("updated");
    if(attr == nullptr)
    {
        HMLog(HM_LOG_INFO,
                "XML OOB File does not contain a timestamp attribute for %s with checkinfo %s",
                hostname.c_str(),
                sourceURL.c_str());
        doc.clear();
        return false;
    }

    errno = 0;
    int64_t tempTime = strtol((const char*) attr->value(), &endptr, 10);
    if ((errno == ERANGE && (tempTime == LONG_MAX || tempTime == LONG_MIN))
            || (errno != 0 && tempTime == 0))
    {
        HMLog(HM_LOG_INFO,
                "XML OOB file time tag has invalid value for %s with checkinfo %s",
                hostname.c_str(),
                sourceURL.c_str());
        doc.clear();
        return false;
    }
    filets.setTime(tempTime);
    auxInfo.m_ts = filets;

    resourceNode = rootNode->first_node("resource-oob");
    if(resourceNode == nullptr)
    {
        HMLog(HM_LOG_DEBUG,
                "XML OOB object does not contain a the resource tag for %s with checkinfo %s",
                hostname.c_str(), sourceURL.c_str());
        doc.clear();
        return false;
    }

    while(resourceNode != nullptr)
    {
        attr = resourceNode->first_attribute("name");

        if(attr == nullptr)
        {
            HMLog(HM_LOG_INFO,
                    "XML OOB object resource tag does not contain a name attribute for %s with checkinfo %s",
                    hostname.c_str(), sourceURL.c_str());
            doc.clear();
            return false;
        }

        resource = attr->value();

        hostNode = resourceNode->first_node("host");
        if(hostNode == nullptr)
        {
            HMLog(HM_LOG_DEBUG,
                    "XML OOB object resource node does not contain a host tag for %s with checkinfo %s",
                    hostname.c_str(), sourceURL.c_str());
            doc.clear();
            return false;
        }

        while(hostNode != nullptr)
        {
            attr = hostNode->first_attribute("name");
            if(attr == nullptr)
            {
                HMLog(HM_LOG_INFO,
                        "XML OOB object host node does not contain a name attribute for %s with checkinfo %s",
                        hostname.c_str(), sourceURL.c_str());
                doc.clear();
                return false;
            }

            HMAuxOOB oob;
            oob.m_type = HM_OOB_FILE;
            oob.m_ip = address;

            oob.m_resource = resource;
            attr = hostNode->first_attribute("name");
            if (attr == nullptr) {
                HMLog(HM_LOG_INFO,
                        "XML Load file host node does not contain a name attribute for %s with checkinfo %s",
                        hostname.c_str(), sourceURL.c_str());
                doc.clear();
                return false;
            }
            oob.m_host = attr->value();

            currentNode =  hostNode->first_node("time");
            if(currentNode == nullptr)
            {
                HMLog(HM_LOG_DEBUG3,
                        "XML OOB object host node does not contain a time tag for %s with checkinfo %s using file time",
                        hostname.c_str(),
                        sourceURL.c_str());
                ts = filets;
            }
            else
            {
                errno = 0;
                tempTime = strtol((const char*)currentNode->value(), &endptr, 10);
                if((errno == ERANGE && (tempTime == LONG_MAX || tempTime == LONG_MIN))
                        || (errno != 0 && tempTime == 0))
                {
                    HMLog(HM_LOG_DEBUG3,
                        "XML OOB object resource node load tag has invalid value for %s with checkinfo %s using file time",
                        hostname.c_str(),
                        sourceURL.c_str());

                    ts = filets;
                }
                else
                {
                    ts.setTime(tempTime);
                }
            }
            oob.m_ts = ts;

            currentNode = hostNode->first_node("shed");
            if(currentNode != nullptr)
            {
                errno = 0;
                oob.m_shed = strtol((const char*)currentNode->value(), &endptr, 10);
                if((errno == ERANGE && (oob.m_shed == UINT_MAX || oob.m_shed == 0))
                        || (errno != 0 && oob.m_shed == 0)
                        || oob.m_shed > 100)
                {
                    HMLog(HM_LOG_DEBUG3,
                            "XML OOB object resource node target tag has invalid value for %s with checkinfo %s setting to 0",
                            hostname.c_str(),
                            sourceURL.c_str());

                    oob.m_shed = 0;
                }
            }
            else
            {
                oob.m_shed = 0;
            }

            currentNode = hostNode->first_node("force-down");
            if(currentNode != nullptr)
            {
                oob.m_forceDown = string(currentNode->value()) == "true";
            }

            auxInfo.m_auxData.push_back(make_unique<HMAuxOOB>(oob));

            hostNode = hostNode->next_sibling("host");
        }
        resourceNode = resourceNode->next_sibling("resource-oob");
    }

    commitEntry(key, auxInfo);
    doc.clear();
    return true;
}

void
HMAuxCache::commitEntry(HMAuxKey& key, HMAuxInfo& auxInfo)
{
    lock_guard<shared_timed_mutex> lock(m_sharedMutex);

    m_auxData.erase(key);
    m_auxData.insert(make_pair(key, auxInfo));
}

