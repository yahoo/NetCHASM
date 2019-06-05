// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "HMLibYDNSAPI.h"
#include "HMStorageAPI.h"

using namespace std;


static void
usage (char* name)
{
    printf("\
Usage: %s [options] \n\
Options:\n\
    -f			<optional>config format<yaml/yfor>(default is yaml format)\n\
    -o			<optional> output file name(default is hm_conf.yaml)\n\
    -C <master config>  Load non-default Master Config\n", name);
    exit(1);
}

HM_API_CONFIG_CLASS getFormat(string& format) {
    if(format == "yfor") 
    {
        return HM_API_CONFIG_YFOR;
    }
    if(format == "yaml")
    {
        return HM_API_CONFIG_YAML;
    }
    printf("Invalid format %s, setting to default yaml format", format.c_str());
    return HM_API_CONFIG_DEFAULT;
	
}

int
main (int argc, char** argv)
{
    int opt;
    int ret;
    string defYaml = "/home/y/conf/netchasm/master.yaml";
    string defConfig = "/home/y/conf/netchasm/master.config";
    string config;
    string format;
    HM_API_CONFIG_CLASS confFormat = HM_API_CONFIG_DEFAULT;
    string fileName = "hm_conf.yaml";

    while ((opt = getopt(argc,argv,"C:f:o:")) != -1) {
        switch (opt) {
        case 'C':
            config = optarg;
            break;
        case 'f':
            format = optarg;
            confFormat = getFormat(format);
            break;
        case 'o':
            fileName = optarg;
            break;
	case 'h':
            usage(argv[0]);
            exit(0);
        case '?':
            usage(argv[0]);
            exit(1);
        default:
            usage(argv[0]);
        }
    }

    HMHandle* handle; 
    if(!config.empty())
    {
        handle = initLibHmonMaster(config.c_str());
    }
    else
    {
        handle = initLibHmonMaster(defYaml.c_str());
        
        if(handle == nullptr)
        {
            handle = initLibHmonMaster(defConfig.c_str());
        }
    }
    if(handle == nullptr)
    {
        fprintf(stderr,"yfor_stat: LibHmon init error\n");
        ret = 1;
        return ret;
    }
    HMStorageAPI *api = (HMStorageAPI*)handle->m_api;
    ret = api->writeConfigs(confFormat, fileName);
    closeLibHmon(handle);
    return ret;
}
