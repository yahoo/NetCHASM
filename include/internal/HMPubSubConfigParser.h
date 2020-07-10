// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMPUBSUBCONFIGPARSER_H_
#define HMPUBSUBCONFIGPARSER_H_

#include <string>
#include <cstdint>

#include "HMConfigParserBase.h"

#define IS_FOLDER 0x4
#define IS_FILE 0x8

class HMPubSubConfigParser
{
public:
    HMPubSubConfigParser() {}

    //! Main parse config function.
    /*!
        The function used to parse a config file.
        \param fileName the file to parse.
        \param state the state data structure to fill from the config information.
        \return the number of config parsing errors. Zero means a successful load.
     */
    uint32_t parseConfig(const std::string& fileName, HMState& state);
};

#endif /* HMPUBSUBCONFIGPARSER_H_ */

