// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCONFIGPARSERBASE_H_
#define HMCONFIGPARSERBASE_H_

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

#include "HMConstants.h"

class HMState;

//! Class containing info of configuration outside of hostgroup info.
/*!
   master-slave params
   indirect host support to allow us to map a host check to another host check.
 */
class HMConfigParams
{
public:
    HMConfigParams() : m_masterMode(false) {}
    bool m_masterMode;
    //! Remote Domain-Master information
    std::map<std::string, std::string> m_remoteChecks;
    std::map<std::string, std::string> m_indirectHost;
};
//! Base class for all configuration parsing types. Derive a new config parser from this class.
/*!
   Base class for configuraiton parsing.
   Adds the indirect host support to allow us to map a host check to another host check.
   Adds the parseDirectory support which will automatically call the appropriate base class parse for each file.
 */
class HMConfigParserBase
{
public:
    HMConfigParserBase() :
        m_configsLoaded(false) {}


    virtual ~HMConfigParserBase() {}

    //! Main parse config function.
    /*!
         The function used to parse the file based on the type of config file.
         \param fileName the file to parse.
         \param state the state data structure to fill from the config information.
         \param class to fill info of config outside of Host-Groups
         \return the number of config parsing errors. Zero means a successful load.
     */
    static uint32_t parseConfigFile(const std::string& fileName, HMState& state, HMConfigParams& configParam);

    //! Parse the entire given directory for configs.
    /*!
        Parse the given directory loading all the config files.
        \param path the config directory path to load.
        \param state the configuration state to use when loading the configs.
        \param class to fill info of config outside of Host-Groups
        \return the number of errors encountered while loading. Zero indicates no problems.
     */
    static uint32_t parseDirectory(const std::string& path, HMState& state, HMConfigParams& configParams);

    //! Main parse config function.
    /*!
     The function used to parse a config file.
     \param fileName the file to parse.
     \param state the state data structure to fill from the config information.
     \param class to fill info of config outside of Host-Groups
     \return the number of config parsing errors. Zero means a successful load.
     */
    virtual uint32_t parseConfig(const std::string& fileName, HMState& state, HMConfigParams& configParams) = 0;

    //! Write the internal config state back to disk.
    /*!
         Write the internal config state back to disk.
         \param path the path to disk to write the config.
         \param state the config state to write back out to the disk.
         \return bool true if the configs were stored without an error.
     */
    virtual bool writeConfigs(HMState& state, std::string outFile) = 0;

protected:
    bool m_configsLoaded;
};

#endif /* HMCONFIGPARSERBASE_H_ */
