// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <regex>

#include "HMConfigParserBase.h"
#include "HMConfigParserYaml.h"
#include "HMState.h"
#include "HMStateManager.h"
#include "HMLogBase.h"

using namespace std;


uint32_t
HMConfigParserBase::parseConfigFile(const string& fileName, HMState& state)
{
    unique_ptr<HMConfigParserBase> parser;
    HMLog(HM_LOG_INFO, "[CORE] Parse file %s", fileName.c_str());
    if (regex_match(fileName, regex("(.*)(.yaml)")))
    {
        parser = make_unique<HMConfigParserYAML>();
    }
    uint32_t nErrors =  parser->parseConfig(fileName, state);

    return nErrors;
}

uint32_t
HMConfigParserBase::parseDirectory(const string& folderName, HMState& state)
{
    uint32_t nErrors = 0;
    DIR* dp;
    struct dirent* d;

    HMLog(HM_LOG_DEBUG, "Parsing the %s directory",folderName.c_str());

    dp = opendir(folderName.c_str());

    if(dp == NULL)
    {
        char errbuf[2048];
        char *error = strerror_r(errno, errbuf, sizeof(errbuf));
        HMLog(HM_LOG_ERROR, "Error opening dir %s: error: %s",
            folderName.c_str(), error);
        return ++nErrors;
    }

    while((d = readdir(dp)))
    {
        if (string(d->d_name) == "." || string(d->d_name) == "..")
        {
            continue;
        }
        if(d->d_type == IS_FILE)
        {
            string fullpath = folderName + "/" + d->d_name;
            uint32_t errorCnt;
            if((errorCnt = parseConfigFile(fullpath, state)) > 0)
            {
                nErrors+=errorCnt;
            }
        }
        else if(d->d_type == IS_FOLDER)
        {
            string fullpath = folderName + "/" + d->d_name;
            nErrors += parseDirectory(fullpath, state);
        }
    }
    closedir(dp);
    return nErrors;
}


