// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include <limits.h>
#include <errno.h>
#include <iostream>
#include <string>

#include "HMLogBase.h"
#include "HMAuxParserRapidXML.h"
#include "rapidxml.h"
#include "rapidxml_print.h"
using namespace rapidxml;
using namespace std;

bool
HMAuxParserRapidXML::genAuxData(HMAuxInfo& auxInfo,
        const HM_AUX_TYPE type,
        const string& rotation,
        string& xmlOutput)
{
    stringstream ss;
    xml_document<> doc;
    xml_node<>* rootNode = nullptr;
    xml_node<>* dcNode = nullptr;
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
HMAuxParserRapidXML::parseAuxData(const string& hostname,
        const string& sourceURL,
        const HMIPAddress& address,
        string& auxStr,
        HMAuxInfo& auxInfo)
{
    HMLog(HM_LOG_DEBUG3,
            "[CURLCHECK] HMAuxCache::parseXML: url %s from host %s@%s", hostname.c_str(), sourceURL.c_str(), address.toString().c_str());
    // I hate this copy so much. But for now I will leave it to fix in profiling
    vector<char> buffer(auxStr.begin(), auxStr.end());
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
        return parseNewLFB(hostname, sourceURL, address, doc, auxInfo);
    }
    else if(rootName == "oob-file")
    {
        return parseOOB(hostname, sourceURL, address, doc, auxInfo);
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
HMAuxParserRapidXML::parseNewLFB(const string& hostname,
        const string& sourceURL,
        const HMIPAddress& address,
        xml_document<char>& doc,
        HMAuxInfo& auxInfo)
{
    HMTimeStamp filets;
    HMTimeStamp ts;

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
    doc.clear();
    return true;
}

bool
HMAuxParserRapidXML::parseOOB(const string& hostname,
        const string& sourceURL,
        const HMIPAddress& address,
        xml_document<char>& doc,
        HMAuxInfo& auxInfo)
{

    HMTimeStamp ts;
    HMTimeStamp filets;
    string resource;

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
    doc.clear();
    return true;
}




