// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMCOMMANDLISTENERCLIENTBASE_H_
#define HMCOMMANDLISTENERCLIENTBASE_H_

#include <inttypes.h>
#include <string>
#include <thread>
#include <iostream>
#include <vector>

#include "HMAPI.h"
#include "HMDataPacking.h"

class HMDataCheckParams;
//! Base client class to support socket communications
class HMControlSocketClientBase
{
public:

    HMControlSocketClientBase& operator=(const HMControlSocketClientBase&) = delete;        // Disallow copying
    HMControlSocketClientBase(const HMControlSocketClientBase&) = delete;

    /*!
     Get the thread pool info of the daemon.
     \param the hm_threadInfo_s struct to fill.
     \return true if the struct was filled correctly.
     */
    bool getThreadInfo(HMAPIThreadInfo& threadInfo);

    /*!
         Get the work queue length of the daemon.
         \param the variable to assign the length.
         \return true if the variable was filled correctly.
     */
    bool getWorkQueue(uint32_t &workQLen);

    /*!
         Get the schedule information of a host.
         \param hostGroupName to get the schedule info.
         \param hostName of the hostgroup to get the schedule info.
         \param the HMAPIDNSSchedInfo struct to fill.
         \return true if the struct was filled correctly.
     */
    bool getHostScheduleInfo(const std::string& hostGroupName, const std::string& hostName, HMAPIDNSSchedInfo &dns);

    /*!
         Get the schedule queue length of the daemon.
         \param the variable to assign the length.
         \return true if the variable was filled correctly.
    */
    bool getSchQueue(uint64_t& schQLen);

    /*!
        Get the log level of the daemon.
        \param the variable to assign the length.
        \return true if the variable was filled correctly.
    */
    bool getLogLevel(std::string& logLevel);

    /*!
         Get the connection timeout used by the daemon.
         \param the variable to assign the length.
         \return true if the variable was filled correctly.
    */
    bool getConnectionTimeout(uint64_t& connectionTimeOut);

    /*!
         Get the monitoring frequency of the thread pool in the daemon.
         \param the variable to assign the length.
         \return true if the variable was filled correctly.
    */
    bool getMonitoringFrequency(uint32_t& monitoringFrequency);

    /*!
         Get the percent of the threads to decrease in thread pool when threads are idle.
         \param the variable to assign the length.
         \return true if the variable was filled correctly.
    */
    bool getStride(uint32_t& stride);

    /*!
         Get the ttl threshold used by thread pool to identify a delayed check.
         \param the variable to assign the length.
         \return true if the variable was filled correctly.
     */
    bool getTTLTreshold(uint32_t& ttlTreshold);

    /*!
         Get the work per thread used by thread pool to increase threads when work increases.
         \param the variable to assign the length.
         \return true if the variable was filled correctly.
     */
    bool getWorkPerThread(uint32_t& workPerThread);

    /*!
         Get if recycling of threads is enabled.
         \return true if successful.
     */
    bool isRecycleOn();

    /*!
         Set the log level of the daemon.
         \param the variable containing the log level.
         \return true if successful.
    */
    bool setLogLevel(const std::string& logLevel);

    /*!
         Set the connection timeout used by the daemon.
         \param the variable containing the connection timeout.
         \return true if successful.
    */
    bool setConnectionTimeOut(uint64_t connectionTimeOut);

    /*!
         Set the monitoring frequency of the thread pool in the daemon.
         \param the variable containing the monitoring frequency.
         \return true if successful.
    */
    bool setMonitoringFrequency(uint32_t monitoringFrequency);

    /*!
         Set the stride percent to decrease threads of the thread pool.
         \param the variable containing the stride percent.
         \return true if successful.
    */
    bool setStride(uint32_t stride);

    /*!
         Set the ttl threshold to identify the delayed thread of the daemon.
         \param the variable containing the log level.
         \return true if successful.
    */
    bool setTTLTreshold(uint32_t ttlTreshold);

    /*!
         Set the work per thread to increase threads in thread pool.
         \param the variable containing the work per thread.
         \return true if successful.
    */
    bool setWorkPerThread(uint32_t workPerThread);

    /*!
         Enabling recycling of thread of the daemon.
         \return true if successful.
    */
    bool setRecycleOn();

    /*!
         Disabling recycling of threads of the daemon.
         \return true if successful.
    */
    bool setRecycleOff();

    /*!
         Force set the host down .
         \param hostGroupName of the host.
         \param hostName of the hostgroup to force set it down.
         \return true if successful.
    */
    bool setForceStatusDown(const std::string& hostGroupName, const std::string& hostName);

	/*!
	 Force set the host down .
		 \param hostGroupName of the host.
		 \param ipaddress of the host in the hostgroup to force set it down.
		 \return true if successful.
	*/
	bool setForceStatusDown(std::string& hostGroupName, HMAPIIPAddress& address);


    /*!
         Unset the force set host down .
         \param hostGroupName of the host.
         \param hostName of the hostgroup to unset the force set down.
         \return true if successful.
    */
    bool unsetForceStatusDown(const std::string& hostGroupName, const std::string& hostName);

    /*!
         Unset the force set host down .
         \param hostGroupName of the host.
         \param ipaddress of the host in the hostgroup to unset the force set down.
         \return true if successful.
    */
    bool unsetForceStatusDown(std::string& hostGroupName, HMAPIIPAddress& address);


    /*!
         Reload the daemon .
         \param master config used for reload.
         \return true if successful.
    */
    bool reload(std::string& masterConfig);

    /*!
         Reload the daemon .
         \return true if successful.
    */
    bool reload();

    /*!
         Get all host group names.
         \param vector to get all the host groups names.
         \return true if successful.
    */
    bool getHostGroupList(std::vector<std::string>& hostGroupNames);

    /*!
         Get all host names.
         \param hostgroup to get the host names.
         \param vector to get all the host names.
         \return true if successful.
    */
    bool getHostList(std::string& hostGroupName,
            std::vector<std::string>& hostList);

    /*!
         Get all host group params and host names.
         \param hostgroup to get the host params.
         \param vector to get all the host names and structure to fill host group params.
         \return true if successful.
    */
    bool getHostGroupParams(std::string& hostGroupName,
            HMAPICheckInfo& checkInfo, std::vector<std::string>& hosts);

    /*!
         Get host group health check results.
         \param hostgroup to get the result.
         \param structure to fill host group results.
         \return true if successful.
    */
    bool getHostGroupResults(std::string& hostGroupName,
            HMAPICheckInfo& checkInfo,
            std::vector<HMAPICheckResult>& hostResults);

    /*!
         Get host health check results.
         \param hostgroup to get the result.
         \param host of the hostgroup to get the result.
         \param structure to fill host results.
         \return true if successful.
    */
    bool getHostResults(std::string& hostGroupName, std::string& hostName,
            std::vector<HMAPICheckResult>& hostResults);

    /*!
         Get aux results.
         \param hostgroup to get the aux result.
         \param structure to fill aux results.
         \return true if successful.
    */
    bool getLoadFeedback(std::string& hostGroupName,
            std::vector<HMAPIAuxInfo>& auxInfo);

    /*!
         Get aux results for host and corresponding IP.
         \param host to get the aux result.
         \param source URL for Host group.
         \param IPAddress of the host
         \param structure to fill aux results.
         \return true if successful.
     */
    bool getLoadFeedback(std::string& hostName, std::string& sourceURL,
            HMAPIIPAddress& address, HMAPIAuxInfo& auxInfo);
    /*!
         Get aux results for host and corresponding IP.
         \param host to get the aux result.
         \param source URL for Host group.
         \param IPAddress of the host
         \param structure to fill aux results.
         \return true if successful.
     */
    bool getLoadFeedback(std::string& hostName,
            std::string& sourceURL, HMAPIIPAddress& address, HMAuxInfo& auxInfo);

    /*!
         Get check results.
         \param hostname to get the health check result.
         \param structure to fill host check details results.
         \return true if successful.
     */
    bool getHostResults(std::string& hostName,
            HMAPIDataHostCheck& apiDataHostCheck,
            std::vector<std::pair<HMAPICheckInfo, HMAPICheckResult>>& hostResults);

    /*!
         Get check results.
         \param hostname to get the health check result.
         \param ipaddress to get the health check result.
         \param structure to fill host check details results.
         \return true if successful.
     */
    bool getHostResults(std::string& hostName, HMAPIIPAddress& address,
            HMAPIDataHostCheck& apiDataHostCheck,
            std::vector<std::pair<HMAPICheckInfo, HMAPICheckResult>>& hostResults);

    /*!
         Get if remote query reply is enabled.
         \param variable to store remote query status
         \return true if successful.
     */
    bool getRemoteQueryOn(bool& remoteQueryStatus);

    /*!
         Enabling remote query reply for remote query.
         \return true if successful.
    */
    bool setRemoteQueryOn();

    /*!
         Disabling remote query reply for remote query.
         \return true if successful.
    */
    bool setRemoteQueryOff();

    /*!
         add address to specific host in static DNS.
         \param hostname to add the addresses to the static DNS
         \param vector of address
         \return true if successful.
     */
    bool addDNSAddresses(const std::string& hostName, std::vector<HMAPIIPAddress>& addresses);

    /*!
         remove address from specific host in static DNS.
         \param hostname to remove the addresses from the static DNS
         \param vector of address
         \return true if successful.
     */
    bool removeDNSAddresses(const std::string& hostName, std::vector<HMAPIIPAddress>& addresses);

    /*!
         get address of a particular host in static DNS.
         \param hostname to add the addresses for static DNS
         \param vector to hold addresses
         \return true if successful.
     */
    bool getDNSAddresses(const std::string& hostName, std::vector<HMAPIIPAddress>& addresses);

    /*!
         set mark to a particular host in a host-group.
         \param host-groupname  the host belongs
         \param host-name to sdd the mark
         \param address of the host to set the mark
         \param mark value
         \return true if successful.
     */
    bool setHostMark(const std::string& hostGroupName, const std::string& hostName, const HMAPIIPAddress& address, int value);

    /*!
         remove mark to a particular host in a host-group.
         \param host-groupname  the host belongs
         \param host-name to remove the mark
         \param address of the host to remove the mark
         \return true if successful.
     */
    bool removeHostMark(const std::string& hostGroupName, const std::string& hostName, const HMAPIIPAddress& address);

    /*!
         get mark that is set to a particular host in a host-group.
         \param host-groupname  the host belongs
         \param host-name to get the mark
         \param address of the host to get the mark
         \param variable to store the mark value
         \return true if successful.
     */
    bool getHostMark(const std::string& hostGroupName, const std::string& hostName, const HMAPIIPAddress& address, int& value);

    /*!
         Get error message on failure.
         \return error string.
     */
    const std::string& getErrorMsg() const;

    /*!
         check if connection is established.
         \return true is connected.
     */
    bool isConnected() const;
    HMDataPacking dataPacking;
    virtual ~HMControlSocketClientBase() { }
protected:
    HMControlSocketClientBase() : m_connected(false) { }
    void extract(std::string &str, std::vector<std::string> &list);
    void setError(std::string msg);
    bool m_connected;
private:
    /*!
         Send command over the socket.
         \param command to send.
         \return true if successful.
     */
    virtual bool sendMessage(const std::string& cmd) = 0;
    /*!
          Send data over the socket.
          \param data to send.
          \param length of data.
          \return true if successful.
    */
    virtual bool sendData(const char* buffer, uint32_t size) = 0;
    /*!
          Receive data over the socket.
          \param data to receive.
          \param length of data to receive.
          \return true if successful.
    */
    virtual bool recvMessage(char* data, uint64_t size) = 0;
    //! Creates a socket
    virtual bool createSocket() = 0;
    // closes a socket
    virtual void closeSocket() = 0;
    // connects a socket
    virtual bool connectSocket() = 0;
    /*!
          Receive data over the socket.
          \param unique ptr of data to receive.
          \param length of data to receive.
          \param accept empty packets(default is false)
          \return true if successful.
    */
    bool receivePacket(std::unique_ptr<char[]>& recvbuf, uint64_t& packetSize, bool acceptEmptyResult = false);
    //! Received Unsigned Integer
    template <typename T>
    bool receiveUInt(T& x);
    //! Received signed Integer
    template <typename T>
    bool receiveInt(T& x);
    std::string m_errorMsg;
};

#endif /* HMCOMMANDLISTENERCLIENTBASE_H_ */
