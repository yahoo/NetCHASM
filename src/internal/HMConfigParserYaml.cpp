// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <iostream>
#include <fstream>
#include <memory>
#include <regex>
#include <yaml-cpp/yaml.h>

#include "HMConfigParserYaml.h"
#include "HMState.h"
#include "HMStateManager.h"
#include "HMConstants.h"
#include "HMDataHostGroup.h"
#include "HMLogBase.h"

using namespace std;

uint32_t
HMConfigParserYAML::parseConfig(const string& fileName, HMState& checkState, HMConfigParams& configParams)
{
    auto currentHostGroup = checkState.m_hostGroups.end();
    auto currentMasterDomain = configParams.m_remoteChecks.end();
    string key, val;

    // track for error logging
    size_t temp;
    uint32_t nerr = 0;
    temp = HM_RT_CONNECT;

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

    for(auto hostNode : configNode)
    {
        if(!hostNode.IsMap())
        {
            HMLog(HM_LOG_ERROR, "%s(%d): Error Yaml is not a Map",fileName.c_str(), hostNode.Mark().line);
            return ++nerr;
        }
        if (hostNode["master-check-mode"])
        {
            string val = hostNode["master-check-mode"].Scalar();
            if(val == "slave")
            {
                configParams.m_masterMode = false;
            }
            else if(val == "master")
            {
                configParams.m_masterMode = true;
            }
            continue;
        }

        if ((hostNode["master-check-domain"]))
        {
            string name = hostNode["master-check-domain"].Scalar();
            currentMasterDomain = configParams.m_remoteChecks.find(name);
            if (currentMasterDomain == configParams.m_remoteChecks.end())
            {
                currentMasterDomain = configParams.m_remoteChecks.insert(
                        pair<string, string>(name, "")).first;
            }
            for (auto n : hostNode)
            {
                key = n.first.Scalar();
                val = n.second.Scalar();

                if (key == "master-check-domain")
                {
                    continue;
                }
                else if (key == "master-check-host")
                {
                    currentMasterDomain->second = val;
                }
                else
                {
                    nerr++;
                    HMLog(HM_LOG_ERROR, "%s(%d):Unknown option %s : %s",
                            fileName.c_str(), hostNode.Mark().line, key.c_str(),
                            val.c_str());
                }
            }
            continue;
        }
        if (!(hostNode["name"]))
        {
            nerr++;
            HMLog(HM_LOG_ERROR, "%s(%d): hostGroup name not present",
                    fileName.c_str(), hostNode.Mark().line);
            continue;
        }
        string name = hostNode["name"].Scalar();
        currentHostGroup = checkState.m_hostGroups.find(name);
        if(currentHostGroup == checkState.m_hostGroups.end())
        {
            currentHostGroup = checkState.m_hostGroups.insert(pair<string,HMDataHostGroup>(name,HMDataHostGroup(name))).first;
        }

        for (auto n : hostNode)
        {               
            key = n.first.Scalar();
            val = n.second.Scalar();

            if (key == "name")
            {
                continue;
            }

            // Start checking for name properties
            else if (key == "rt-mode")
            {
                if (val == "connect")
                {
                    temp = HM_RT_CONNECT;
                }
                else if (val == "smoothed-connect")
                {
                    temp = HM_RT_SMOOTHED_CONNECT;
                }
                else if (val == "total")
                {
                    temp = HM_RT_TOTAL;
                }
                else if (val == "smoothed-total")
                {
                    temp = HM_RT_SMOOTHED_TOTAL;
                }
                else
                {
                    nerr++;
                    HMLog(HM_LOG_ERROR, "%s(%d): Invalid rt-mode",
                            fileName.c_str(), n.second.Mark().line);
                }
                currentHostGroup->second.setMeasurementOptions(temp);
            }
            else if (key == "host")
            {
                YAML::Node hosts = n.second;
                if(!hosts.IsSequence())
                {
                    HMLog(HM_LOG_ERROR, "%s(%d): Error hosts not a list",
                            fileName.c_str(), hosts.Mark().line);   
                }
                for (auto h : hosts)
                {
                    val = h.Scalar();
                    currentHostGroup->second.addHost(val);
                }
            }
            else if (key == "dual-stack-mode")
            {
                if (val == "ipv4-only")
                {
                    currentHostGroup->second.setDualStack(HM_DUALSTACK_IPV4_ONLY);
                }
                else if (val == "both")
                {
                    currentHostGroup->second.setDualStack(HM_DUALSTACK_BOTH);
                }
                else if (val == "ipv6-only")
                {
                    currentHostGroup->second.setDualStack(HM_DUALSTACK_IPV6_ONLY);
                }
                else
                {
                    nerr++;
                    HMLog(HM_LOG_ERROR, "%s(%d): Invalid dual-stack-mode",
                            fileName.c_str(), n.second.Mark().line);
                }
            }
            else if (key == "check-type")
            {
                if (val == "http")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_HTTP);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTP_DEFAULT_PORT);
                    }
                }
                else if (val == "https")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_HTTPS);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTPS_DEFAULT_PORT);
                    }
                }
                else if (val == "tcp")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_TCP);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_TCP_DEFAULT_PORT);
                    }
                }
                else if (val == "tcps")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_TCPS);
                    if (!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_TCP_DEFAULT_PORT);
                    }
                }
                else if (val == "none")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_NONE);
                }
                else if (val == "ftp")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_FTP);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_FTP_DEFAULT_PORT);
                    }
                }
                else if (val == "dns")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_DNS);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_DNSVC_DEFAULT_PORT);
                    }
                }
                else if (val == "dnsvc")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_DNSVC);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_DNSVC_DEFAULT_PORT);
                    }
                }
                else if (val == "https-no-peer-check")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_HTTPS_NO_PEER_CHECK);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTPS_DEFAULT_PORT);
                    }
                }
                else if (val == "ftps-implicit")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_FTPS_IMPLICIT);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_FTPS_DEFAULT_PORT);
                    }
                }
                else if (val == "ftps-implicit-no-peer-check")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_FTPS_DEFAULT_PORT);
                    }
                }
                else if (val == "ftps" || val == "ftps-explicit")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_FTPS_EXPLICIT);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_FTP_DEFAULT_PORT);
                    }
                }
                else if (val == "ftps-explicit-no-peer-check")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_FTP_DEFAULT_PORT);
                    }
                }
                else if (val == "http-auxfetch")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_AUX_HTTP);
                    if (!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTP_DEFAULT_PORT);
                    }
                }
                else if (val == "https-auxfetch")
                {

                    currentHostGroup->second.setCheckType(HM_CHECK_AUX_HTTPS);

                    if (!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTPS_DEFAULT_PORT);
                    }
                }
                else if (val == "https-no-peer-check-auxfetch")
                {
                    currentHostGroup->second.setCheckType(
                            HM_CHECK_AUX_HTTPS_NO_PEER_CHECK);

                    if (!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTPS_DEFAULT_PORT);
                    }
                }
                else if (val == "indirect-host")
                {
                    configParams.m_indirectHost.insert(make_pair(currentHostGroup->second.getName(),currentHostGroup->second.getCheckInfo()));
                }
                else if (val == "http-mark")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_MARK_HTTP);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTP_DEFAULT_PORT);
                    }
                }
                else if (val == "https-mark")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_MARK_HTTPS);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTPS_DEFAULT_PORT);
                    }
                }
                else if (val == "https-mark-no-peer-check")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_MARK_HTTPS_NO_PEER_CHECK);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTPS_DEFAULT_PORT);
                    }
                }
                else
                {
                    HMLog(HM_LOG_ERROR, "%s(%d): Invalid check-type",
                            fileName.c_str(), n.second.Mark().line);
                    nerr++;
                }
            }
            else if (key == "check-port")
            {
                int port = atoi(val.c_str());
                if (port >= 0 && port <= 65535)
                {
                    currentHostGroup->second.setPort(port);
                }
                else
                {
                    HMLog(HM_LOG_ERROR, "%s(%d): Invalid port number %d",
                            fileName.c_str(), n.second.Mark().line, port);
                    nerr++;
                }
            }
            else if (key == "uri" || key == "check-info")
            {
                if (key == "uri")
                {
                    currentHostGroup->second.setCheckType(HM_CHECK_HTTP);
                    if(!currentHostGroup->second.getCheckPort())
                    {
                        currentHostGroup->second.setPort(HM_HTTP_DEFAULT_PORT);
                    }
                }
                currentHostGroup->second.setCheckInfo(val);
                auto res = configParams.m_indirectHost.find(currentHostGroup->second.getName());
                if(res != configParams.m_indirectHost.end())
                {
                    res->second = val;
                }
            }
            else if (key == "check-retries")
            {
                currentHostGroup->second.setNumCheckRetries(atoi(val.c_str()));
            }
            else if (key == "check-retry-delay")
            {
                currentHostGroup->second.setCheckRetryDelay(atoi(val.c_str()));
            }
            else if (key == "timeout")
            {
                currentHostGroup->second.setCheckTimeout(atoi(val.c_str()));
            }
            else if (key == "conn-check-interval")
            {
                HMLog(HM_LOG_DEBUG3,
                            "Depricated feature conn-check-interval with value %s found in config %s %d",
                             val.c_str(), fileName.c_str(), n.second.Mark().line);
            }
            else if (key == "ttl")
            {
                currentHostGroup->second.setCheckTTL(atoi(val.c_str()));
            }
            else if (key == "error-ttl")
            {
                HMLog(HM_LOG_DEBUG3,
                            "Depricated feature error-ttl found in config %s %d",
                            fileName.c_str(), n.second.Mark().line);
            }
            else if (key == "group-threshold")
            {
                currentHostGroup->second.setGroupThreshold(atoi(val.c_str()));
            }
            else if (key == "smoothing-window")
            {
                currentHostGroup->second.setSmoothingWindow(atoi(val.c_str()));
            }
            else if (key == "flap-threshold")
            {
                currentHostGroup->second.setFlapThreshold(atoi(val.c_str()) / 1000);
            }
            else if (key == "max-flaps")
            {
                currentHostGroup->second.setMaxFlaps(atoi(val.c_str()));
            }
            else if (key == "slow-threshold")
            {
                currentHostGroup->second.setSlowThreshold(atoi(val.c_str()));
            }
            else if (key == "remote-fallback")
            {
                if(val == "on")
                {
                    currentHostGroup->second.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_REMOTE);
                }
                else if(val =="off")
                {
                    currentHostGroup->second.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_REMOTE);
                }
                else
                {
                    nerr++;
                    HMLog(HM_LOG_ERROR, "%s(%d): Invalid remote fallback mode",
                            fileName.c_str(), n.second.Mark().line);
                }
            }
            else if (key == "local-fallback")
            {
                if (val == "on")
                {
                    currentHostGroup->second.setDistributedFallback(
                            HM_DISTRIBUTED_FALLBACK_LOCAL);
                }
                else if (val == "off")
                {
                    currentHostGroup->second.unsetDistributedFallback(
                            HM_DISTRIBUTED_FALLBACK_LOCAL);
                }
                else
                {
                    nerr++;
                    HMLog(HM_LOG_ERROR, "%s(%d): Invalid local fallback mode",
                            fileName.c_str(), n.second.Mark().line);
                }
            }
            else if (key == "source-address")
            {
                HMIPAddress address;
                if (address.set(val))
                {
                    currentHostGroup->second.setSourceAddress(address);
                }
                else
                {
                    nerr++;
                    HMLog(HM_LOG_ERROR, "%s(%d): Invalid source address",
                            fileName.c_str(), n.second.Mark().line);
                }
            }
            else if (key == "tos-value")
            {
                int tos = strtoul(val.c_str(), NULL, 0);
                if (tos <= 255)
                {
                    currentHostGroup->second.setTOSValue(tos);
                }
                else
                {
                    nerr++;
                    HMLog(HM_LOG_ERROR, "%s(%d): Invalid tos value",
                            fileName.c_str(), n.second.Mark().line);
                }
            }
            else if (key == "dns-type")
            {
                if (val == "none")
                {
                    currentHostGroup->second.setDnsCheckPlugin(
                            HM_DNS_PLUGIN_NONE);
                }
                else if (val == "ares")
                {
                    currentHostGroup->second.setDnsCheckPlugin(
                            HM_DNS_PLUGIN_ARES);
                }
                else if(val == "static")
                {
                    currentHostGroup->second.setDnsCheckPlugin(
                            HM_DNS_PLUGIN_STATIC);
                }
                else
                {
                    nerr++;
                    HMLog(HM_LOG_ERROR, "%s(%d): Invalid dns mode",
                            fileName.c_str(), n.second.Mark().line);
                }
            }
            else if (key == "host-group")
            {
                YAML::Node hostgroups = n.second;
                if(!hostgroups.IsSequence())
                {
                    HMLog(HM_LOG_ERROR, "%s(%d): Error hostgroups not a list",
                            fileName.c_str(), hostgroups.Mark().line);
                }
                for (auto h : hostgroups)
                {
                    val = h.Scalar();
                    currentHostGroup->second.addHostGroup(val);
                }
            }
            else if (key == "failure-response")
            {
                HMLog(HM_LOG_DEBUG3,
                            "Depricated feature failure-response mode found in config %s %d",
                            fileName.c_str(), n.second.Mark().line);
            }
            else if(key == "allow-hosts")
            {
                // do nothing since all the configs set this to any
            }
            else if(key == "auth-domain")
            {
                HMLog(HM_LOG_WARNING,
                        "Depricated feature auth-domain mode found in config %s %d",
                        fileName.c_str(), hostNode.Mark().line);
            }
            else
            {
                nerr++;
                HMLog(HM_LOG_ERROR, "%s(%d):Unknown option %s : %s",
                        fileName.c_str(), hostNode.Mark().line, key.c_str(), val.c_str());
            }
        }//ending of each host grop forloop (fo auto n:hostgroup)
    }//ending all host hostgroups for(auto hostgroup : confignode)
    return nerr;
}

bool
HMConfigParserYAML::writeConfigs(HMState& state, std::string outFile)
{
    std::ofstream outFileStream(outFile);
    for(auto it: state.m_hostGroups)
    {
        outFileStream << "-   name: " << it.second.getName() << endl;
        outFileStream << "    ttl: " << it.second.getCheckTTL() << endl;
        outFileStream << "    check-type: " << printCheckTypeConfigs(it.second.getCheckType()) << endl;
        if (it.second.getCheckInfo().size() > 0)
        {
            outFileStream << "    check-info: " << it.second.getCheckInfo() << endl;
        }
        outFileStream << "    timeout: " << it.second.getCheckTimeout() << endl;
        outFileStream << "    check-port: " << it.second.getCheckPort() << endl;
        outFileStream << "    dual-stack-mode: " << printDualStack(it.second.getDualstack()) << endl;
        if(it.second.getNumCheckRetries() > 0)
        {
            outFileStream << "    check-retries: " << to_string(it.second.getNumCheckRetries()) << endl;
            outFileStream << "    check-retry-delay: " << it.second.getCheckRetryDelay() << endl;
        }
        outFileStream << "    group-threshold: " << it.second.getGroupThreshold() << endl;
        outFileStream << "    max-flaps: " << it.second.getMaxFlaps() << endl;
        outFileStream << "    flap-threshold: " << it.second.getFlapThreshold() * 1000 << endl;
        outFileStream << "    slow-threshold: " << it.second.getSlowThreshold() << endl;
        outFileStream << "    smoothing-window: " << it.second.getSmoothingWindow() << endl;
        outFileStream << "    rt-mode: " << printMeasurementOptions(it.second.getMeasurementOptions()) << endl;
        if(it.second.getSourceAddress().isSet())
        {
            outFileStream << "    source-address: " << it.second.getSourceAddress().toString() << endl;
        }
        if(it.second.getTOSValue())
        {
            outFileStream << "    tos-value: " << (int)it.second.getTOSValue() << endl;
        }
        const vector<string>* hosts = it.second.getHostList();
        if(hosts->size() > 0)
        {
            outFileStream << "    host: " << endl;
        }
        for(auto host = hosts->begin(); host!= hosts->end(); ++host)
        {
            outFileStream << "       - " << *host << endl;
        }
        //currently displaying child hostgroup names and hosts
        const vector<string>* hostgroups = it.second.getHostGroupList();
        if(hostgroups->size() > 0)
        {
            outFileStream << "    host-group: " << endl;
        }

        for(auto hostgroup = hostgroups->begin(); hostgroup!= hostgroups->end(); ++hostgroup)
        {
            outFileStream << "       - " << *hostgroup << endl;
        }
        outFileStream << endl;
    }
    outFileStream.close();
    return true;
}
