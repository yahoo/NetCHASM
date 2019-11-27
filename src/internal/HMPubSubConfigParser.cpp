// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <iostream>
#include <fstream>
#include <memory>
#include <regex>
#include <yaml-cpp/yaml.h>

#include "HMPubSubConfigParser.h"
#include "HMState.h"
#include "HMStateManager.h"
#include "HMConstants.h"
#include "HMLogBase.h"

#ifdef USE_KAFKA 
#include "HMPublisherKafka.h"
#endif 

using namespace std;

uint32_t
HMPubSubConfigParser::parseConfig(const string& fileName, HMState& checkState)
{
    string key, val;

    // track for error logging
    uint32_t nerr = 0;

    if (!regex_match (fileName, regex("(.*)(.yaml)")))
    {
        HMLog(HM_LOG_ERROR, "Requires yaml, Parsing config file format %s is not supported",fileName.c_str());
        return 0;
    }

    YAML::Node configNode;

    try
    {
        configNode = YAML::LoadFile(fileName);
    }
    catch(const YAML::ParserException &e)
    {
        HMLog(HM_LOG_ERROR, "Parsing config yaml file %s is not valid",fileName.c_str());
        return 0;
    }
    catch (const YAML::BadFile &e)
    {
        HMLog(HM_LOG_ERROR, "No yaml file %s exists",fileName.c_str());
        return 0;
    }

    if(!configNode.IsSequence())
    {
        HMLog(HM_LOG_ERROR, "%s(%d): Error Yaml is not a list",fileName.c_str(), configNode.Mark().line);
        return ++nerr;
    }

    for(auto publisherNode : configNode)
    {
        if(!publisherNode.IsMap())
        {
            HMLog(HM_LOG_ERROR, "%s(%d): Error Yaml is not a Map",fileName.c_str(), publisherNode.Mark().line);
            return ++nerr;
        }


        if (!(publisherNode["name"]))
        {
            nerr++;
            HMLog(HM_LOG_ERROR, "%s(%d): publisher name not present",
                    fileName.c_str(), publisherNode.Mark().line);
            continue;
        }
        string name = publisherNode["name"].Scalar();
        bool onChange = false;
        set<string> hostgroups;
        if (!(publisherNode["type"]))
        {
            nerr++;
            HMLog(HM_LOG_ERROR, "%s(%d): publisher type not present",
                    fileName.c_str(), publisherNode.Mark().line);
            continue;
        }

        if (publisherNode["hostgroups"])
        {
            YAML::Node hostsgroups = publisherNode["hostgroups"];
            if (!hostsgroups.IsSequence())
            {
                nerr++;
                HMLog(HM_LOG_ERROR, "%s(%d): Error: hostgroups not a list",
                        fileName.c_str(), hostsgroups.Mark().line);
            }
            for (auto h : hostsgroups)
            {
                val = h.Scalar();
                hostgroups.insert(val);
            }
        }

        if (publisherNode["onchange"])
        {
            onChange = publisherNode["onchange"].Scalar() == "true" ? true : false;
        }
        unique_ptr<HMPublisherBase> publisher;
        string type = publisherNode["type"].Scalar();
        if (type == "kafka")
        {
#ifdef USE_KAFKA
            HMKafkaConfig config;
            if (!(publisherNode["parameters"]))
            {
                nerr++;
                HMLog(HM_LOG_ERROR, "%s(%d): publisher name not present",
                        fileName.c_str(), publisherNode.Mark().line);
                continue;
            }
            auto kafkaNode = publisherNode["parameters"];
            if (!(kafkaNode["topic"]))
            {
                nerr++;
                HMLog(HM_LOG_ERROR, "%s(%d): publisher kakfa topic not present",
                        fileName.c_str(), kafkaNode.Mark().line);
                continue;
            }
            if (!(kafkaNode["brokerlist"]))
            {
                nerr++;
                HMLog(HM_LOG_ERROR,
                        "%s(%d): publisher kafka broker list not present",
                        fileName.c_str(), kafkaNode.Mark().line);
                continue;
            }
            string topic = kafkaNode["topic"].Scalar();
            YAML::Node brokerlist = kafkaNode["brokerlist"];
            if (!brokerlist.IsSequence())
            {
                HMLog(HM_LOG_ERROR, "%s(%d): Error: hostgroups not a list",
                        fileName.c_str(), brokerlist.Mark().line);
            }
            for (auto h : brokerlist)
            {
                val = h.Scalar();
                if (!config.brokers.empty())
                {
                    config.brokers += ',';
                }
                config.brokers += val;
            }
            publisher = make_unique<HMPublisherKafka>(config, topic);
#else
            HMLog(HM_LOG_ERROR, "%s(%d): Error: Kafka module disabled during build",
                                    fileName.c_str(), publisherNode.Mark().line);
            continue;
#endif
        }
        else
        {
            nerr++;
            HMLog(HM_LOG_ERROR, "%s(%d): Invalid publisher type",
                    fileName.c_str(), publisherNode.Mark().line);
            continue;
        }
        if (hostgroups.empty())
        {
            publisher->setPublishAll(true);
        }
        else
        {
            publisher->setPublishAll(false);
            publisher->addHostGroups(hostgroups);
        }
        publisher->setPublishOnChange(onChange);
        checkState.m_resultPublisher.registerPublisher(name, publisher);
    }
    return nerr;
}
