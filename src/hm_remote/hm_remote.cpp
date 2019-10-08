// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>

#include <vector> 
#include <map>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <fstream>

#include "HMControlBase.h"
#include "HMControlSocketClientBase.h"
#include "HMControlTCPSocketClient.h"
#include "HMControlTLSSocketClient.h"

using namespace std;


string server_address = "127.0.0.1";
HMAPIIPAddress server;
uint16_t server_port = HM_CONTROL_SOCKET_DEFAULT_PORTV4;
string mode = "tcp";
bool tcpMode = true;
string keyPath;
string certPath;
string caPath;
bool opt_state = false;
bool opt_load = false;
char delim = 0;

static void usage(char* name)
{
    string command = "man ";
    command.append(name);
    {
        cout << "Usage: "<< name <<" [options] command" << endl << "Options:" << endl
                << "-i      <socket-ip-address> [default: " << server_address << "]" << endl
                << "-p     <server-port> [default: " << server_port<< "]" <<endl
                << "-o     <tcp/tcps> [default: tcp"<< "]" <<endl
                << "-k      <key-file>"<<endl
                << "-c     <cert-file>"<<endl
                << "-a     <ca-file>"<<endl
                << "-d     <delimiter> [default: " <<delim<< "Show host status in bare delimited format"<<endl
                << "-L          Show load feedback information"<<endl
                << "-s          Show per-host state summary"<<endl
                << "Commands:"<<endl
                << "\t" <<"getremotequery \t get the remote query enable status of daemon" << endl
                << "\t" <<"setremotequery <on/off>\t enable/disable the remote query state of the daemon " << endl
                << "\t" <<"hostresults <host-group>\t get the host group results" << endl;
    }
}

void handleError(char *prog, string &name)
{
    cerr << prog << ":" << "Missing parameters for command " << name << endl;
    cerr<<"Use \""<<prog<<" -h\" for usage information"<<endl;
    exit(-3);
}



string
printTimeSinceNow(uint64_t timeCount)
{
    if(timeCount == 0)
    {
        return "-";
    }
    HMTimeStamp now =  HMTimeStamp::now();
    uint64_t t = (now.getTimeSinceEpoch() - timeCount) / 1000;
    string st;
    int days = t / (24 * 60 * 60);
    t -= days * ( 24 * 60 * 60);
    int hours = t / (60 * 60);
    t -= hours * (60 * 60);
    int mins = t / 60;
    t -= mins * 60;
    int secs = t;
    if (days)
    {
        st = to_string(days) + "d:" + to_string(hours);
    }
    else if (hours)
    {
        st = to_string(hours) + "h:" + to_string(mins);
    }
    else
    {
        st = to_string(mins) + ':' + to_string(secs);
    }
    return st;
}

static void printResult(stringstream& buf,
        HMAPICheckResult& hostGroupResult)
{

    char status;

    if (hostGroupResult.m_status & HM_HOST_STATUS_FORCE_DOWN)
    {
        status = '-';
    }
    else if (hostGroupResult.m_status & HM_HOST_STATUS_FORCE_USE)
    {
        status = '*';
    }
    else if (hostGroupResult.m_status & HM_HOST_STATUS_USE)
    {
        status = '+';
    }
    else if (hostGroupResult.m_status & HM_HOST_STATUS_UP)
    {
        status = ':';
    }
    else
    {
        status = ' ';
    }

    string niceAddress;
    size_t index = hostGroupResult.m_host.find(".yahoo.com");
    if(index == string::npos)
    {
        niceAddress = hostGroupResult.m_host;
    }
    else
    {
        niceAddress = hostGroupResult.m_host.substr(0, index);
    }

    stringstream tbuf;
    tbuf << niceAddress;
    niceAddress = tbuf.str();

    string strAddr = hostGroupResult.m_address.toString();
    if (strAddr == "0.0.0.0" || strAddr == "::")
    {
        strAddr = "";
    }
    buf << status
        <<  left <<setw(23) << strAddr
        <<setw(36) << niceAddress;

    if((hostGroupResult.m_status & HM_HOST_STATUS_UP))
    {
        buf << setw(6) << printTimeSinceNow(hostGroupResult.m_changeTime) << " ";
    }
    else
    {
        buf << setw(6) << "-" << " ";
    }

    buf <<setw(7) << printTimeSinceNow(hostGroupResult.m_changeTime) << " "
        << setw(6) << hostGroupResult.m_responseTime << " "
        << setw(6) << hostGroupResult.m_totalResponseTime << " "
        << setw(6) << hostGroupResult.m_minResponseTime << " "
        << setw(6) << hostGroupResult.m_maxResponseTime << " "
        << setw(6) << hostGroupResult.m_smoothedResponseTime<< " "
        << setw(6) << (unsigned long)(hostGroupResult.m_numResponses ?
                (hostGroupResult.m_sumResponseTime / hostGroupResult.m_numResponses)
                : 0) << " "
        << setw(8) << hostGroupResult.m_numChecks << " "
        << setw(7) << hostGroupResult.m_numResponses << " "
        << setw(7) << hostGroupResult.m_numFlaps << " "
        << setw(8) << hostGroupResult.m_numConnectFailures << " "
        << setw(8) << hostGroupResult.m_numFailures << " "
        << setw(7) << hostGroupResult.m_numTimeouts << " ";
    buf << "\n";

}

string
print(string format, uint64_t timeCount)
{
    chrono::milliseconds dur(timeCount);
    chrono::time_point<chrono::system_clock, chrono::milliseconds> dt(dur);
    stringstream ss;
    time_t tt = chrono::system_clock::to_time_t(dt);
    tm *tm = gmtime(&tt);
    ss << put_time(tm, format.c_str());
    return ss.str();
}

void printStateResult(stringstream& buf,
        const string& groupName,
        vector<HMAPICheckResult>& results,
        vector<HMAPICheckResult>::iterator& hostGroupIterator,
        HMAPICheckInfo& group, char delim)
{

    HMAPICheckResult hostGroupResult = *hostGroupIterator;
    string state;
    string mod;
    if (hostGroupResult.m_status & HM_HOST_STATUS_FORCE_DOWN)
    {
        state = "down";
        mod = '-';
    }
    else
    {
        if (hostGroupResult.m_status & HM_HOST_STATUS_UP)
        {
            state = "up";
        }
        else
        {
            state = "down";
        }
        if (hostGroupResult.m_status & HM_HOST_STATUS_FORCE_USE)
        {
            mod = '*';
        }
        else if (hostGroupResult.m_status & HM_HOST_STATUS_USE)
        {
            mod = '+';
        } else
        {
            mod = ' ';
        }
    }

    size_t index = hostGroupResult.m_host.find(".yahoo.com");
    string cleanHost =
            (index == std::string::npos) ?
                    hostGroupResult.m_host.substr(0, index) : hostGroupResult.m_host;
    string astr = hostGroupResult.m_address.toString();
    if (astr == "0.0.0.0" || astr == "::")
    {
        astr = "";
    }
    string hostPort;

    if (delim)
    {
        buf << groupName << delim << cleanHost << delim << astr << delim
                << hostGroupResult.m_port << delim << state << delim
                << ((group.m_passthroughInfo & HM_RT_SMOOTHED_CONNECT) ?
                        hostGroupResult.m_smoothedResponseTime :
                        hostGroupResult.m_responseTime) << delim
                << hostGroupResult.m_totalResponseTime << delim
                << printReason(HM_REASON(hostGroupResult.m_reason));
       buf << "\n";
    }
    else
    {
        if (hostGroupResult.m_port)
        {
            stringstream tempbuf;
            tempbuf << hostGroupResult.m_host << ":" << hostGroupResult.m_port;
            hostPort = tempbuf.str();
        }
        else
        {
            hostPort = hostGroupResult.m_host;
        }
        // do some quick checking to see if we need to number this host
        int currentIndex = 0;
        bool numberNeeded = false;
        for (auto iit = results.begin(); iit != hostGroupIterator; ++iit)
        {
            if (hostGroupResult.m_host == iit->m_host)
            {
                currentIndex++;
            }
        }
        if (currentIndex == 1)
        {
            auto iit = hostGroupIterator;
            for (++iit; iit != results.end(); ++iit)
            {
                if (hostGroupResult.m_host == iit->m_host)
                {
                    numberNeeded = true;
                }
            }
        } else
        {
            numberNeeded = true;
        }

        if (numberNeeded)
        {
            stringstream tempbuf;
            tempbuf << hostGroupResult.m_host;
            hostPort = tempbuf.str();
        }

        stringstream msbuf;
        msbuf
                << ((group.m_passthroughInfo & HM_RT_SMOOTHED_CONNECT) ?
                        hostGroupResult.m_smoothedResponseTime :
                        hostGroupResult.m_responseTime) << "/"
                << hostGroupResult.m_totalResponseTime << "ms";

        buf << left << setw(23) << astr << " " << setw(32) << hostPort << " "
                << mod << setw(4) << state << " " << setw(5)
                << ((hostGroupResult.m_status & HM_HOST_STATUS_UP) ?
                        printTimeSinceNow(hostGroupResult.m_changeTime) : "-") << " "
                << setw(5) << printTimeSinceNow(hostGroupResult.m_changeTime)
                << " " << setw(5) << msbuf.str() << " " << setw(9)
                << printReason(HM_REASON(hostGroupResult.m_reason));
        buf << "\n";
    }

}

void
printName (const string& groupName,
        HMAPICheckInfo& group,
        vector<HMAPICheckResult>& results,
        vector<string>& filenames,
        vector<string>& hosts,
        char delim)
{
    stringstream buf;
    if (opt_state)
    {
        if (!delim)
        {
            buf << "[" << groupName << "]"<<endl;
        }

        for (std::vector<std::string>::const_iterator jt = hosts.begin();
                jt != hosts.end(); ++jt)
        {
            for (auto it = results.begin(); it != results.end();)
            {
                if (it->m_host.compare(*jt) == 0)
                {
                    printStateResult(buf, groupName, results, it, group, delim);
                    it = results.erase(it);
                    continue;
                }
                ++it;
            }
        }
    }
    else
    {
        // get the last host check time
        uint64_t lastCheck = 0;
        for(auto it = results.begin(); it != results.end(); ++it)
        {
            if(it->m_checkTime > lastCheck)
            {
                lastCheck = it->m_checkTime;
            }
        }
        if (opt_load)
        {
            cout << "load-feedback URLs" << endl;
            for (vector<string>::iterator it = filenames.begin();
                    it != filenames.end(); ++it)
            {
                cout << "\t[" << *it << "]" << endl;

            }
        }
        buf << "[" << groupName << "]" << "\n"
            << " last check =      " << setw(4) << setprecision(2) << lastCheck << " "<<print("(%a %b %d %H:%M:%S %Y)", lastCheck) << "\n"
            << " ttl =             " << group.m_checkTTL << "ms\n"
            << " error-ttl =       " << "0" << "ms\n"
            << " timeout =         " << group.m_checkTimeout << "ms\n"
            << " group-threshold = " << group.m_groupThreshold << "ms\n"
            << " check-type =      " << printCheckType(HM_CHECK_TYPE(group.m_checkType)) << "\n"
            << " check-port =      " << group.m_port << "\n"
            << " check-info =      " << group.m_checkInfo << "\n"
            << left <<setw(23) << " address " << setw(36) << " hostname "
            << setw(6) << " up-t " << " " << setw(7) << " chg-t " << " " << setw(6)
            << " rt " << " " << setw(6) << " tot " << " " << setw(6) << " min " << " " << setw(6)
            << " max " << " " << setw(6) << " smth " << " " << setw(6) << " mean " << " "
            << setw(8) << " ntries " << " " << setw(7) << " nok " << " " << setw(7)
            << " nflap " << " "
            << setw(8) << " erconn " << " " << setw(8) << " erhttp " << " " << setw(7)
            << " timeo " << " ";
        buf << endl;

        // ok now print all the host info
        for (std::vector<std::string>::const_iterator jt = hosts.begin();
                jt != hosts.end(); ++jt)
        {
            for (auto it = results.begin(); it != results.end();)
            {
                if (it->m_host.compare(*jt) == 0)
                {
                    printResult(buf, *it);
                    it = results.erase(it);
                    continue;
                }
                ++it;
            }
        }
    }

    if (!delim) {
        buf << endl;
    }
    cout << buf.str();
}

bool writeXML(string& fileName, string& xml)
{
    ofstream fout(fileName.c_str());
    if(!fout.is_open())
    {
        // Log error
        return false;
    }

    fout << xml << "\n";
    fout.close();
    return true;
}

bool getHostResult(unique_ptr<HMControlSocketClientBase>& socketAPI, string hostGroup, HMAPICheckInfo& checkInfo, vector<HMAPICheckResult>& hostResults, vector<string>& hosts)
{
    vector<HMAPICheckResult> temp_hostResults;
    if(!socketAPI->getHostGroupParams(hostGroup, checkInfo, hosts))
    {
        return false;
    }
    if(socketAPI->getHostGroupResults(hostGroup, checkInfo, temp_hostResults))
    {
        HMAPIDataHostCheck dataHostCheck;
        dataHostCheck.m_checkInfo = checkInfo.m_checkInfo;
        dataHostCheck.m_checkType = checkInfo.m_checkType;
        dataHostCheck.m_port = checkInfo.m_port;
        dataHostCheck.m_ipv4 = checkInfo.m_ipv4;
        dataHostCheck.m_ipv6 = checkInfo.m_ipv6;
        for(HMAPICheckResult& result: temp_hostResults)
        {
            vector<pair<HMAPICheckInfo, HMAPICheckResult>> temp_results;

            if(socketAPI->getHostResults(result.m_host, result.m_address, dataHostCheck, temp_results))
            {
                for( auto it: temp_results)
                {
                    it.first.m_checkInfo = checkInfo.m_checkInfo;
                    it.first.m_checkType = checkInfo.m_checkType;
                    it.first.m_port = checkInfo.m_port;
                    it.first.m_ipv4 = checkInfo.m_ipv4;
                    it.first.m_ipv6 = checkInfo.m_ipv6;
                    it.second.m_host = result.m_host;
                    it.second.m_address = result.m_address;
                    if( it.first == checkInfo)
                    {
                        hostResults.push_back(it.second);
                    }
                }
            }
        }
        return true;
    }
    return false;
}

bool getLFBInfo(unique_ptr<HMControlSocketClientBase>& socketAPI,
        string hostGroup, HMAPICheckInfo& checkInfo,
        vector<HMAPICheckResult>& hostResults, vector<string>& filenames)
{
    for (HMAPICheckResult& result : hostResults)
    {
        HMAuxInfo auxInfo;
        if (socketAPI->getLoadFeedback(result.m_host, checkInfo.m_checkInfo,
                result.m_address, auxInfo))
        {
            HMAuxCache auxCache;
            string xml;
            if(auxInfo.m_auxData.size() == 0)
            {
                continue;
            }
            if (auxInfo.m_auxData[0]->m_type == HM_LOAD_FILE)
            {
                if (auxCache.genAuxData(auxInfo, HM_LOAD_FILE, hostGroup, xml, HM_AUX_DATA_XML))
                {
                    string fileName = hostGroup + "_" + result.m_host
                            + "_LoadFile.xml";
                    filenames.push_back(fileName);
                    writeXML(fileName, xml);
                }
            }
            else if (auxInfo.m_auxData[0]->m_type == HM_OOB_FILE)
            {
                if (auxCache.genAuxData(auxInfo, HM_OOB_FILE, hostGroup, xml, HM_AUX_DATA_XML))
                {
                    string fileName = hostGroup + "_" + result.m_host
                            + "_OOBFile.xml";
                    filenames.push_back(fileName);
                    writeXML(fileName, xml);
                }
            }
        }
    }
    return true;
}

bool runCommand(const vector<string> &strArgs)
{
    unique_ptr<HMControlSocketClientBase> socketAPI;
    if(tcpMode)
    {
        socketAPI = make_unique<HMControlTCPSocketClient>(server, server_port);
    }
    else
    {
        socketAPI = make_unique<HMControlTLSSocketClient>(server, server_port, certPath, keyPath, caPath);
    }
    if(!socketAPI->isConnected())
    {
        cerr << "Failed to connect: " << socketAPI->getErrorMsg() << endl;
        return false;
    }
    if (strArgs[0] == HM_CMD_SETREMOTEQUERY)
    {
        if (strArgs[1] == "on")
        {
            return socketAPI->setRemoteQueryOn();
        }
        else
        {
            return socketAPI->setRemoteQueryOff();
        }
    }

    if (strArgs[0] == HM_CMD_GETREMOTEQUERY)
    {
        bool remoteQueryStatus;
        if (socketAPI->getRemoteQueryOn(remoteQueryStatus))
        {
            string remoteQueryStatusString =
                    remoteQueryStatus ? "Enabled" : "Disabled";
            cout << "Remote query Enabled = " << remoteQueryStatusString
                    << endl;
            return true;
        }
        return false;
    }
    if (strArgs[0] == HM_CMD_HOSTRESULTS)
    {
        HMAPICheckInfo checkInfo;
        vector<HMAPICheckResult> hostResults;
        vector<string> files;
        vector<string> hosts;
        if(getHostResult(socketAPI, strArgs[1], checkInfo, hostResults, hosts))
        {
            if (opt_load)
            {
                getLFBInfo(socketAPI, strArgs[1], checkInfo, hostResults, files);
            }
            printName(strArgs[1], checkInfo, hostResults, files, hosts, delim);
            return true;
        }
        return false;
    }
    return true;
}


int main(int argc, char* argv[])
{
    bool bSuccess = false;
    int opt;
    uint32_t port;
    while ((opt = getopt(argc, argv, "i:p:o:k:c:a:d:sLh")) != -1)
    {
        switch (opt) {
        case 'i':
            server_address = std::string(optarg);
            if(!server.set(server_address))
            {
                cerr << "Invalid server address :" << server_address << endl;
                usage(argv[0]);
                exit(0);
            }
            break;
        case 'k':
            keyPath = std::string(optarg);
            break;
        case 'c':
            certPath = std::string(optarg);
            break;
        case 's':
            opt_state = true;
            break;
        case 'L':
            opt_load = true;
            break;
        case 'd':
            delim = (char)*optarg;
            break;
        case 'a':
            caPath = std::string(optarg);
            break;
        case 'p':
            port = std::stoul(optarg);
            if (port > 65535)
            {
                cerr << "Invalid port value:" << port << endl;
                usage(argv[0]);
                exit(0);
            }
            server_port = port;
            break;
        case 'o':
            mode = std::string(optarg);
            if(mode != "tcp" && mode != "tcps")
            {
                cerr << "invalid mode" << endl;
                usage(argv[0]);
                exit(0);
            }
            if(mode == "tcps")
            {
                tcpMode = false;
            }
            break;
        case 'h':
            usage(argv[0]);
            exit(0);
        case '?':
            usage(argv[0]);
            exit(1);
        }
    }

    vector<string> strArgs;
    for (int arg = optind; arg < argc; arg++)
    {
        const string strArg = argv[arg];
        strArgs.push_back(strArg);
    }

    if (strArgs.empty())
    {
        usage(argv[0]);
        return 0;
    }

    if (HMCommandListenerBase::isValidCommand(strArgs[0]) == false)
    {
        cerr << "Invalid command: " << strArgs[0] << endl;
        return -2;
    }
    switch(HMCommandListenerBase::convert(strArgs[0]))
    {
    case SETREMOTEQUERY:
        if (strArgs.size() < 2)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        break;
    case GETREMOTEQUERY:
        bSuccess = runCommand(strArgs);
        break;
    case HOSTRESULTS:
        if (strArgs.size() < 2)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        break;
    case LOADFBINFO:
        if (strArgs.size() < 2)
        {
            handleError(argv[0], strArgs[0]);
        }
        bSuccess = runCommand(strArgs);
        break;
    case ADDDNSADDRESSES:
    case REMOVEDNSADDRESSES:
    case GETDNSADDRESSES:
        cerr << "This command is supported in hm_staticdns" << endl;
        return -1;
    case REMOVEHOSTMARK:
    case GETHOSTMARK:
    case SETHOSTMARK:
        cerr << "This command is supported in hm_hostmark" << endl;
        return -1;
    case HOSTSCHDINFO:
    case WORKQUEUEINFO:
    case THREADINFO:
    case SCHDQUEUEINFO:
    case HEALTHCHECK:
    case DNSCHECK:
    case RELOAD:
    case HOSTGROUPINFO:
    case LOADFBINFOIP:
    case SETHOSTSTATUS:
    case HOSTGROUPLIST:
    case HOSTCHECK:
    case HOSTGROUPPARAMS:
    case HOSTLIST:
    case HOSTIPRESULTS:
    case SETLOGLEVEL:
    case SETCONNECTIONTIMEOUT:
    case SETMONFREQ:
    case SETSTRIDE:
    case SETTTLTRESH:
    case SETWORKPERTHREAD:
    case SETRECYCLE:
    case GETLOGLEVEL:
    case GETCONNECTIONTIMEOUT:
    case GETMONFREQ:
    case GETSTRIDE:
    case GETTTLTRESH:
    case GETWORKPERTHREAD:
    case GETRECYCLE:
    case UNDEFINED:
        cerr << "Not all commands are supported at this time" << endl;
        return -3;
    }
    if(!bSuccess)
    {
        cerr << "Failure executing command " << strArgs[0] << endl;
    }
    return (bSuccess) ? 0 : 1;
}

