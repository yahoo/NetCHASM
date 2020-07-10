// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCONFIGPARSERYAML_H_
#define HMCONFIGPARSERYAML_H_

#include <string>
#include <cstdint>

#include "HMConfigParserBase.h"

#define IS_FOLDER 0x4
#define IS_FILE 0x8

class HMConfigParserYAML : public HMConfigParserBase
{
public:
    HMConfigParserYAML() :
        m_loadCount(0) {};

    //! Main parse config function.
    /*!
        The function used to parse a config file.
        \param fileName the file to parse.
        \param state the state data structure to fill from the config information.
        \param class to fill info of config outside of Host-Groups
        \return the number of config parsing errors. Zero means a successful load.
     */
    uint32_t parseConfig(const std::string& fileName, HMState& state, HMConfigParams& configParams);

    //! Write the internal config state back to disk.
    /*!
        Write the internal config state back to disk.
        \param path the path to disk to write the config.
        \param state the config state to write back out to the disk.
        \return bool true if the configs were stored without an error.
     */

    bool writeConfigs(HMState& state, std::string outFile);

private:
    uint32_t m_loadCount;
};

#endif /* HMCONFIGPARSERYAML_H_ */

