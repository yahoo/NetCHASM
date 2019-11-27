// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCOMMANDLISTENERBASE_H
#define HMCOMMANDLISTENERBASE_H

#include <unistd.h>
#include <memory>

#include "HMConstants.h"
#include "HMLogBase.h"
#include "HMStorage.h"
#include "HMDataPacking.h"
#include "HMControlLinuxSocketClient.h"
#include "HMSocketUtilBase.h"

const std::string HM_CMD_RELOAD = "reload";
const std::string HM_CMD_HOSTGROUP = "hostgroup";
const std::string HM_CMD_LOADFB = "loadfb";
const std::string HM_CMD_LOADFBIP = "loadfbip";
const std::string HM_CMD_THREADINFO = "threadinfo";
const std::string HM_CMD_WORKQUEUEINFO = "workqueueinfo";
const std::string HM_CMD_SCHDQUEUEINFO = "schdqueueinfo";
const std::string HM_CMD_SETHOSTSTATUS = "hostset";
const std::string HM_CMD_HOSTGROUPLIST = "hostgrouplist";
const std::string HM_CMD_HOSTLIST = "hostlist";
const std::string HM_CMD_HOSTCHECK = "hostcheck";
const std::string HM_CMD_HOSTRESULTS = "hostresults";
const std::string HM_CMD_HOSTIPRESULTS = "hostipresults";
const std::string HM_CMD_HOSTGROUPPARAMS = "hostgroupparams";
const std::string HM_CMD_HOSTSCHDINFO = "hostschdinfo";
const std::string HM_CMD_HEALTHCHECK = "healthcheck";
const std::string HM_CMD_DNSCHECK = "dnscheck";

const std::string HM_CMD_GETLOGLEVEL = "getloglevel";
const std::string HM_CMD_SETLOGLEVEL = "setloglevel";
const std::string HM_CMD_GETCONNECTIONTIMEOUT = "getconnectiontimeout";
const std::string HM_CMD_SETCONNECTIONTIMEOUT = "setconnectiontimeout";
const std::string HM_CMD_SETMONFREQ = "setmonitorfrequency";
const std::string HM_CMD_GETMONFREQ = "getmonitorfrequency";
const std::string HM_CMD_GETTTLTRESH = "getttlthreshold";
const std::string HM_CMD_SETTTLTRESH = "setttlthreshold";
const std::string HM_CMD_GETSTRIDE = "getstride";
const std::string HM_CMD_SETSTRIDE = "setstride";
const std::string HM_CMD_GETWORKPERTHREAD = "getworkperthreadratio";
const std::string HM_CMD_SETWORKPERTHREAD = "setworkperthreadratio";
const std::string HM_CMD_SETRECYCLE = "setrecycle";
const std::string HM_CMD_GETRECYCLE = "getrecycle";
const std::string HM_CMD_SETREMOTEQUERY = "setremotequery";
const std::string HM_CMD_GETREMOTEQUERY = "getremotequery";
const std::string HM_CMD_ADDDNSADDRESSES = "adddnsaddresses";
const std::string HM_CMD_REMOVEDNSADDRESSES = "removednsaddresses";
const std::string HM_CMD_GETDNSADDRESSES = "getdnsaddresses";
const std::string HM_CMD_SETHOSTMARK = "sethostmark";
const std::string HM_CMD_REMOVEHOSTMARK = "removehostmark";
const std::string HM_CMD_GETHOSTMARK = "gethostmark";

class HMStateManager;
//! Class to handle external communications
class HMCommandListenerBase
{
 public:
  HMCommandListenerBase(HMStateManager& stateManager);
  virtual ~HMCommandListenerBase() {}

  HMCommandListenerBase& operator=(const HMCommandListenerBase&) = delete;        // Disallow copying
  HMCommandListenerBase(const HMCommandListenerBase&) = delete;

  //! Base function to initialize sockets
  virtual void init();

  //! Base function to handle connections
  virtual void run() = 0;
  //! Base function to shutdown sockets
  virtual void listernerShutDown() = 0;

  //! Convert the string message to HM_COMMAND_TASKS enum
  static HM_COMMAND_TASKS convert(const std::string& task);

  /*!
       Called to handle socket commands.
       \param command received.
       \param Utility base class to send and receive messages.
  */
  void handleCommands(std::string& command, HMSocketUtilBase& utilBase);

  //! function to shutdown base socket
  void shutDown();

  //! function to throw exception
  void throwException(std::string errStr);

  /*!
       Called to get the host group in for a particular hostgroup.
       \param name of hostgroup.
       \param HMDataHostGroup to store results.
       \return true if results are found.
  */
  bool getHostGroupInfo(const std::string& name, HMDataHostGroup& group);

  /*!
       Called to get the packed scheduling for a particular host in hostgroup.
       \param unique ptr of the data packing class.
       \param name of the hostgroup.
       \param name of the host.
       \param size of the packed data.
       \return unique pointer containing the data.
    */
  std::unique_ptr<char[]> getHostSchdInfo(std::unique_ptr<HMDataPacking>& datapacking, const std::string& hostgroup,
            std::string& host, uint64_t& buflen);

  /*!
       Called to get the health check results for a particular hostgroup.
       \param name of hostgroup.
       \param vector to store results.
       \return true if results are found.
    */
  uint32_t getHostGroupResults(const std::string& name, std::vector<HMGroupCheckResult>& results);

  /*!
       Called to get the packed health check results for a particular hostgroup.
       \param unique ptr of the data packing class.
       \param name of the hostgroup
       \param size of the packed data.
       \return unique pointer containing the data.
    */
  std::unique_ptr<char[]> createHostGroup(std::unique_ptr<HMDataPacking>& datapacking, std::string& hostGroupName, uint64_t& buflen);

  /*!
       Called to get the packed load feedback data for a particular hostgroup.
       \param unique ptr of the data packing class.
       \param name of the hostgroup
       \param size of the packed data.
       \return unique pointer containing the data.
    */
  std::unique_ptr<char[]> getloadfbdata (std::unique_ptr<HMDataPacking>& datapacking, std::string& rotationName, uint64_t& buflen);

  /*!
       Called to get the packed load feedback data for a particular hostgroup.
       \param unique ptr of the data packing class.
       \param name of the host
       \param source URL of the hostgroup
       \param ipaddress of the host
       \param size of the packed data.
       \return unique pointer containing the data.
    */
  std::unique_ptr<char[]> getloadfbdata (std::unique_ptr<HMDataPacking>& dataPacking, std::string& hostName, std::string& sourceURL, HMIPAddress& address, uint64_t& buflen);

  /*!
       Called to set status up/down for a particular host in a hostgroup.
       \param name of hostgroup.
       \param name of the host.
       \param status true=up, false=down.
       \return true if results are found.
    */

  bool setHostStatus (const std::string& hostGroupName, const std::string& host, bool forceHostStatus);

  /*!
       Called to get all the hostgroups
       \param vector to store results(hostgroup names).
   */
  void getAllHostGroupNames(std::vector<std::string>& groupNames);

  /*!
     Called to get all the hosts in a hostgroup.
     \param name of hostgroup.
     \param vector to store results(host names).
  */
  void getHosts(const std::string& hostGroupName, std::vector<std::string>& hostNames);

  /*!
     Called to get the health check results for a particular host in a hostgroup.
     \param name of hostgroup.
     \param name of the host.
     \param vector to store results.
     \return true if results are found.
  */
  bool getHostCheck(const std::string& hostGroupName, const std::string& hostName, std::vector<HMDataCheckResult>& hostResults);

  /*!
     Called to get the packed health check results for a particular host and datahostcheck.
     \param unique ptr of the data packing class.
     \param name of the host
     \param datahostcheck of the host
     \param size of the packed data.
     \return unique pointer containing the data.
  */
  std::unique_ptr<char[]> getHostResults(std::unique_ptr<HMDataPacking>& datapacking, const std::string& hostName, HMDataHostCheck& hostCheck, uint64_t& dataSize);

  /*!
     Called to get the packed health check results for a particular host, ip address and datahostcheck.
     \param unique ptr of the data packing class.
     \param name of the host
     \param ip address of the host
     \param datahostcheck of the host
     \param size of the packed data.
     \return unique pointer containing the data.
  */
  std::unique_ptr<char[]> getHostResults(std::unique_ptr<HMDataPacking>& dataPacking, const std::string& hostName, const HMIPAddress& address, HMDataHostCheck& hostCheck, uint64_t& dataSize);

  /*!
       set mark to a particular host in a host-group.
       \param host-groupname  the host belongs
       \param host-name to sdd the mark
       \param address of the host to set the mark
       \param mark value
       \return true if successful.
   */
  bool setHostMark(const std::string& hostGroupName, const std::string& hostName, const HMIPAddress& address, const std::set<int>& values);

  /*!
       remove mark to a particular host in a host-group.
       \param host-groupname  the host belongs
       \param host-name to remove the mark
       \param address of the host to remove the mark
       \param mark values to be removed
       \return true if successful.
   */
  bool removeHostMark(const std::string& hostGroupName, const std::string& hostName, const HMIPAddress& address, const std::set<int>& values);

  /*!
       get mark that is set to a particular host in a host-group.
       \param unique ptr of the data packing class.
       \param size of the packed data.
       \param host-groupname  the host belongs
       \param host-name to get the mark
       \param address of the host to get the mark
       \param variable to store the mark value
       \return true if successful.
   */
  std::unique_ptr<char[]> getHostMark(std::unique_ptr<HMDataPacking>& dataPacking, uint64_t& dataSize, const std::string& hostGroupName, const std::string& hostName, const HMIPAddress& address);

  //! clean the handler thread after communication completes
  void cleanHandlerThreads();

  /*!
       Called to validate the received command.
       \param command received.
       \return true is successful.
   */
  static bool isValidCommand(const std::string& strCommand);

  /*!
       Called to tokenize the command to individual tokens.
       \param command message.
       \param tokens extracted from the command.
   */
  void tokenize(std::string& command, std::vector<std::string>& tokens);

  // This object will be valid during the lifetime of the command listener
  HMStateManager& m_stateManager;
  bool m_keepRunning;
  std::thread m_mainThread;
  std::mutex m_handlerMutex;
  std::mutex m_exceptionMutex;
  std::vector<std::thread> m_handlerThreads;
  std::map<std::thread::id,bool> m_handlerThreadsStatus;
  int m_pipesfd[2];
};

#endif /* HMCOMMANDLISTENERBASE_H */
