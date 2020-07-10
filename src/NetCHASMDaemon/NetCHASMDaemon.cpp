// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <iostream>
#include <unistd.h>

#include "HMLogBase.h"
#include "HMConstants.h"
#include "HMStateManager.h"
#include "HMGuardian.h"

using namespace std;

void usage()
{
    // Standard printout of usage
    cout << "NetCHASM (Coordinated Health and Status Monitor) Utility" << endl;
    cout << "Usage: NetCHASM <options> <master config>" << endl;
    cout << "Options:" << endl;
    cout << "-d: Run process in the background with a guardian process" << endl;
    cout << "-v <1-9>: Guardian Logging Verbosity" << endl;
}

int main(int argc, char* argv[])
{
    bool runAsDaemon = false;
    string masterConfig;
    HM_LOG_LEVEL guardianLogLevel = DEFAULT_GUARDIAN_LOG_LEVEL;

    int c;
    while ((c = getopt(argc, argv, "dhv:")) != -1)
    {
        switch (c)
        {
        case 'v':
            guardianLogLevel =  static_cast<HM_LOG_LEVEL>(atoi(optarg));
            break;

        case 'd':
            runAsDaemon = true;
            break;

        case 'h':
            usage();
            exit(0);

         case '?':
            usage();
            exit(1);
        }
    }

    if (optind < argc)
    {
        masterConfig = argv[optind];
    }

    if (runAsDaemon)
    {
        HMGuardian guardian;
        return guardian.runGuardian(guardianLogLevel, masterConfig);
    }
    else
    {
        HMGuardian guardian;
        guardian.startNetCHASM(masterConfig, HM_LOG_ERROR);
    }
}

