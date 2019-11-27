// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HMSTATE_H_
#define INCLUDE_HMSTATE_H_

#include <memory>
#include <vector>
#include <openssl/ossl_typ.h>
#include <openssl/ssl.h>
#include "HMPublisherBase.h"
#include "HMResultPublisher.h"
#include "HMConstants.h"
#include "HMStorage.h"
#include "HMDataCheckList.h"
#include "HMLogBase.h"
#include "HMAuxCache.h"
#include "HMControlLinuxSocket.h"
#include "HMHashMD5.h"

//! The core state class for NetCHASM.
/*!
     The core stat class for NetCHASM.
     All internal state loaded from the configs both master and healthcheck should go here.
     This class also contains all check information, the DNS cache and host group information.
     All configuration parsing and reload logic is inside this class.
 */
class HMStorageHostGroupMDBM;
class HMState
{
public:
    HMState() :
        m_ctx(NULL),
        m_useBackendConfigs(false),
        m_masterConfigLoaded(false),
        m_configsLoaded(false),
        m_stateActive(false),
        m_storageClass(HM_STORAGE_MDBM),
        m_eventClass(HM_EVENT_QUEUE),
        m_httpDefaultCheckClass(HM_CHECK_PLUGIN_HTTP_CURL),
        m_ftpDefaultCheckClass(HM_CHECK_PLUGIN_FTP_CURL),
        m_tcpDefaultCheckClass(HM_CHECK_PLUGIN_TCP_RAW),
        m_tcpsDefaultCheckClass(HM_CHECK_PLUGIN_TCPS_RAW),
        m_dnsDefaultCheckClass(HM_CHECK_PLUGIN_DNS_ARES),
        m_noneDefaultCheckClass(HM_CHECK_PLUGIN_DEFAULT),
        m_auxDefaultCheckClass(HM_CHECK_PLUGIN_AUX_CURL),
        m_markDefaultCheckClass(HM_CHECK_PLUGIN_MARK_CURL),
        m_dnsLookupTimeout(HM_DEFAULT_DNS_RESOLUTION_TIMEOUT),
        m_dnsRetries(HM_DEFAULT_DNS_RETRIES),
        m_nMaxThreads(1),
        m_nMinThreads(1),
        m_connectionTimeout(3000),
        m_logClass(HM_LOG_PLUGIN_TEXT),
        m_logLevel(HM_LOG_NOTICE),
        m_logPath(HM_DEFAULT_LOG_PATH),
        m_socketPath(HM_DEFAULT_USD_PATH),
        m_maxLogQueue(0),
        m_auxPolicy(HM_STORAGE_COMMIT_ALWAYS),
        m_healthCheckPolicy(HM_STORAGE_COMMIT_ALWAYS),
        m_storageLockPolicy(HM_STORAGE_RW_LOCKS),
        m_controlSocketCheckPortv4(HM_CONTROL_SOCKET_DEFAULT_PORTV4),
        m_controlSocketCheckPortv6(HM_CONTROL_SOCKET_DEFAULT_PORTV6),
        m_enableSecureRemote(false),
        m_enableMutualAuth(true),
        m_libEventEnabled(false)
    {
        m_control_socket.push_back(HM_CONTROL_SOCKET_LINUX);
    }
    HMState(HMState&) = delete;

    ~HMState()
    {
        closeBackend(); m_datastore.reset();
        SSL_CTX_free(m_ctx);
     }

    //! Check the configurations to make sure they are valid.
    /*!
         Check the configurations to make sure they are valid. This does not actually load the configs into the state.
         \param the master config to check for validity.
         \return true if the master config and all health check configs load without errors.
     */
    bool validateConfigs(const std::string& masterConfig);

    //! Parse the master configuration file.
    /*
         Parse the master configuration file. Does not parse the health check configs. Sets the internal variables for the state according to the passed master config.
         \param the path to the master config file.
         \return true if the master config loaded correctly.
     */
    bool parseMasterConfig(const std::string& masterConfig);

    //! Parse the health check configs and setup the Daemon state.
    /*
         Parse the health check configs. The function will use the paths loaded from the master config from the previous call to parseMasterConfig. It will also prepare the DNS cache and the check list classes.
         \return true if the health check configs parsed correctly.
     */
    bool setupDaemonstate();

    //! Computes Hash and stores the config info.
    /*
         Computes hash  for the entire config and stores in the config Info.
         \return true if the successful.
     */
    bool storeConfigInfo();

    //! Get the current master config loaded.
    /*!
         Get the current master config loaded.
         \return the current master config.
     */
    const std::string& getMasterConfig() const;

    //! Get the current storage type.
    /*!
         Get the current storage type.
         \return the current storage type.
     */
    HM_STORAGE_CLASS getStorageType() const;

    //! Get the current logging class.
    /*!
        Get the current logging class.
        \return the current logging class.
     */
    HM_LOG_PLUGIN_CLASS  getDefaultLogType() const;

    //! Get the current event loop class.
    /*!
        Get the current event loop class.
        \return the current event loop class.
     */
    HM_EVENT_PLUGIN_CLASS getDefaultEventType() const;

    //! Get the current HTTP check class.
    /*!
        Get the current HTTP check class.
        \return the current HTTP check class.
     */
    HM_CHECK_PLUGIN_CLASS getDefaultHTTPCheckype() const;

    //! Get the current FTP check class.
    /*!
        Get the current FTP check class.
        \return the current FTP check class.
     */
    HM_CHECK_PLUGIN_CLASS getDefaultFTPCheckype() const;

    //! Get the current TCP check class.
    /*!
        Get the current TCP check class.
        \return the current TCP check class.
     */
    HM_CHECK_PLUGIN_CLASS getDefaultTCPCheckype() const;

    //! Get the current TCPS check class.
    /*!
        Get the current TCPS check class.
        \return the current TCPS check class.
     */
    HM_CHECK_PLUGIN_CLASS getDefaultTCPSCheckype() const;


    //! Get the current DNS check class.
    /*!
        Get the current DNS check class.
        \return the current DNS check class.
     */
    HM_CHECK_PLUGIN_CLASS getDefaultDNSCheckype() const;

    //! Get the current none check class.
    /*!
        Get the current none check class.
        \return the current none check class.
     */
    HM_CHECK_PLUGIN_CLASS getDefaultNoneCheckype() const;

    //! Get the current log path.
    /*!
        Get the current log patch.
        \return the current log path.
     */
    std::string getLogPath() const;

    //! Get the current log level.
    /*!
        Get the current log level.
        \return the current HM_LOG_LEVEL enum for the log level.
     */
    HM_LOG_LEVEL getLogLevel() const;

    //! Get the current log time stamp format string.
    /*!
        Get the current log time stamp format string.
        \return the current log time stamp format string.
     */
    std::string getLogFormat() const;

    //! Get the current max log queue length.
    /*!
        Get the current max log queue length. Logs beyond this length will be dropped until the logger catches up.
        \return the current max log queue length.
     */
    uint32_t getMaxLogQueueLength() const;

    //! Get the current default connection timeout for all checks.
    /*!
        Get the current default connection timeout for all checks.
        \return the current default connection timeout.
     */
    uint64_t getConnectionTimeout() const;

    //! Get the current max number of work threads.
    /*!
            Get the current max number of work threads. The thread pool will spawn up to max depending on the workload.
            \return the current max number of work threads.
     */
    uint64_t getMaxThreads();

    //! Get the current minimum number of work threads.
    /*!
            Get the current minimum number of work threads. The thread pool will shutdown to the minimum when we have not work.
            \return the current minimum number of work threads.
     */
    uint64_t getMinThreads();

    //! Get the current default DNS resolution timeout.
    /*!
            Get the current default DNS resolution timeout.
            \return the current default DNS resolution timeout.
     */
    uint32_t getDNSLookupTimeout() const;

    //! Get the current number of DNS lookup retries.
    /*!
            Get the current number of DNS lookup retries.
            \return the current number of DNS lookup retries.
     */
    uint32_t getDNSRetries() const;

    //! Get the current path to the backend storage.
    /*!
            Get the current path to the backend storage.
            \return the current path to the backend storage.
     */
    std::string getStoragePath() const;

    //! Get the current socket path.
    /*!
            Get the current socket path.
            \return the current socket path.
     */
    std::string getSocketPath() const;

    //! Get the current DNS server to query for DNS resolutions.
    /*!
             the current DNS server to query for DNS resolutions. If not set, the daemon will use the system default.
            \return the current DNS server to use for DNS resolutions.
     */
    bool getDNSAddress(HMIPAddress& addr) const;

    //! Set the path to load the master configuration.
    /*!
         Set the path to load the master configuration.
         \param the path to the master configuration.
     */
    void setMasterConfig(const std::string& masterConfig);

    //! Set the default connection timeout for health checks.
    /*!
         Set the default connection timeout for health checks.
         \param new health check timeout in milliseconds.
     */
    void setConnectionTimeout(uint64_t connectionTimeout);

    //! Set the DNS server to use for all resolutions.
    /*!
         Set the DNS server to use for all resolutions.
         \param the HMIPAddress of the DNS server.
     */
    void setDNSServer(HMIPAddress server);

    //! Set the socket path to use for the Linux socket.
    /*!
         Set the socket path to use for the Linux socket.
         \param the path to use for the Linux socket.
     */
    void setSocketPath(const std::string& path);

    //! Load all the configs.
    /*!
         Load all the configs as defined in the master config parse.
         \return true if the configs loaded successfully.
     */
    bool loadAllConfigs();

    //! Load all the pub-sub configuration.
    /*!
         Load all the pub-sub configuration.
         \return true if the configs loaded successfully.
     */
    bool loadPubSubConfig();

    //! Generate the host check list based on the loaded configs.
    /*!
         Generate the host list based on the loaded configs. Call after filling in the host groups and individual host checks to complete the initial set of health checks required.
         Also fills in the callback host groups in the check params data structure.
     */

    void generateHostCheckList();

    //! Generate the DNS checks required in the DNS check list.
    /*
         Generate the DNS checks required in the DNS check list. Populates the DNS cache with empty entries. Populates the DNS call back to reschedule the needed health checks when a DNS entry is updated.
     */
    void generateDNSCheckList();

    //! Force a health check for all hosts within a specific host group.
    /*!
         Force a health check for all hosts within a specific host group.
         \param the host group to force a check now.
         \param the work queue to schedule the check.
     */
    void forceHealthCheck(const std::string& hostGroup, HMWorkQueue& workQueue);

    //! Force a health check for a specific host.
    /*!
         Force a health check for a specific host.
         \param the host group to force a check now.
         \param the host within the host group to force the check.
         \param the work queue to schedule the check.
     */
    void forceHealthCheck(const std::string& hostGroup, const std::string& hostName, HMWorkQueue& workQueue);

    //! Force a DNS resolution for all hosts within a specific host group.
    /*!
         Force a DNS resolution for all hosts within a specific host group.
         \param the host group to force DNS resolutions now.
         \param the work queue to schedule the DNS lookups.
     */
    void forceDNSCheck(const std::string& hostGroup, HMWorkQueue& workQueue);

    //! Force a DNS resolution for a specific host.
    /*!
         Force a DNS resolution for a specific host.
         \param the host group to force DNS resolutions now.
         \param the host within the host group to force the DNS resolution.
         \param the work queue to schedule the check.
     */
    void forceDNSCheck(const std::string& hostGroup, const std::string& hostName, HMWorkQueue& workQueue);

    //! Force a DNS resolution for a specific host.
    /*!
         Force a DNS resolution for a specific host.
         \param the host to force DNS resolutions now.
         \param the DNS plugin type.
         \param set of addresses added
         \param the work queue to schedule the check.
     */
    void forceDNSCheck(const std::string &hostName,  HM_DNS_PLUGIN_CLASS dnsPlugin, const std::set<HMIPAddress>& addresses, HMWorkQueue& workQueue);


    //! Open the backend storage.
    /*!
         Open the backend storage.
         \param true to open the backend in read only mode.
         \return true if the backend opened and is ready.
     */
    bool openBackend(bool readOnly);

    //! Close the backend.
    void closeBackend();

    //! Copy the current check state.
    /*!
         Copy the current check state from the src to this check state instance. Copies the checklist and DNS cache results. Only copies the results. The configs should be pre-loaded in the target state class.
         Handles dropping results from stale configuration information in the source.
         \param the src HMState to copy results from.
     */
    void restoreRunningCheckState(std::shared_ptr<HMState> src);

    //! Reschedule HealthChecks on a reload.
    /*!
         Reschedule HealthChecks on a reload. Forces stale and updated health checks to be queued for checking upon completion of a reload.
         \param the src of the checks before reload.
         \param the current work queue.
     */
    void resheduleHealthChecks(std::shared_ptr<HMState> src, HMWorkQueue& workQueue);

    //! Reschedule DNS checks on a reload.
    /*!
         Reschedule DNS checks on a reload. Forces stale and updated DNS cache value to be queued for resolution on the completion of a reload.
         \param the src of the checks before the reload.
         \parm the work queue to schedule the checks.
     */
    void resheduleDNSChecks(std::shared_ptr<HMState> src, HMWorkQueue& workQueue);

    //! Force the backend to updated for all checks removed or changed during the reload.
    /*!
         Force the backend to updated for all checks removed or changed during the reload.
         \param the src of the check values before the reload.
     */
    void updateBackend(std::shared_ptr<HMState> src);

    //! Restore the results from the backend.
    /*!
         Restore the results from the backedn for the DNS cache and the CheckList. Called on the initial loading of the Daemon.
     */
    void restoreStoredCheckState();

    //! Hash the host group map.
    /*
         Hash the host group map.
         \param the hash to update with the hash of the host group map.
         \param the variable to store the hash.
     */
    void  HashHostGroupMap (HMHashMD5& hash, HMHash& hashValue);

    //! Dump the configs from backend to file.
    /*
         Dump the configs from backend to file.
         \param config format.
         \param filename where the configs need to be written.
     */
    bool writeConfigs(HM_CONFIG_PLUGIN_CLASS configs, const std::string &fileName);

    //! Get the current hash of configs.
    const HMHash& getHash() const;

    //! Set the config's hash to the state.
    /*!
         Set the config's hash to the current state.
         \param the hash value of the configs loaded.
     */
    void setHash(const HMHash& hash);

    //! Get the current(v4) check-port of control-socket.
    uint16_t getControlSocketCheckPortv4() const;

    //! Get the current(v6) check-port of control-socket.
    uint16_t getControlSocketCheckPortv6() const;

    //! Get all the control sockets enabled
    const std::vector<HM_CONTROL_SOCKET>& getControlSocket() const;

    //! Get the host certificate file
    const std::string& getCertFile() const;

    //! Get the host key file
    const std::string& getKeyFile() const;

    //! Get the CA certificate file
    const std::string& getCaFile() const;

    //! Enable encrypted data transfer
    bool isEnableSecureRemote() const;

    //! Enable tls mutual authentication
    bool isEnableMutualAuth() const;

    //! check if lib-event type check used in DNS checks
    bool isLibEventEnabled() const;

    //! Enable/Disbale lib-event type DNS check
    void setLibEventEnabled(bool libEventEnabled);

    //! The main DNS cache.
    HMDNSCache m_dnsCache;
    //! Master Structure of health check information
    HMDataCheckList m_checkList;
    //! Host Group information
    HMDataHostGroupMap m_hostGroups;
    //! Map used to queue health checks after a successful DNS query
    HMWaitList m_dnsWaitList;
    //! Storage for the aux Info
    HMAuxCache m_auxCache;
    //! Back end data store class
    std::unique_ptr<HMStorage> m_datastore;
    //! SSL context
    SSL_CTX* m_ctx;
    
    HMResultPublisher m_resultPublisher;

private:

    //! Parse the master config in the YAML format.
    /*!
         Parse the master config in the YAML format.
         \param the path to the YAML config.
         \return true on a successful load of the master config.
     */
    bool parseMasterYaml(const std::string& masterConfig);

    std::vector<std::string> m_configDirs;
    std::vector<std::string> m_configFiles;

    bool m_useBackendConfigs;
    bool m_masterConfigLoaded;
    bool m_configsLoaded;
    bool m_stateActive;

    HM_STORAGE_CLASS m_storageClass;

    HM_EVENT_PLUGIN_CLASS m_eventClass;

    HM_CHECK_PLUGIN_CLASS m_httpDefaultCheckClass;
    HM_CHECK_PLUGIN_CLASS m_ftpDefaultCheckClass;
    HM_CHECK_PLUGIN_CLASS m_tcpDefaultCheckClass;
    HM_CHECK_PLUGIN_CLASS m_tcpsDefaultCheckClass;
    HM_CHECK_PLUGIN_CLASS m_dnsDefaultCheckClass;
    HM_CHECK_PLUGIN_CLASS m_noneDefaultCheckClass;
    HM_CHECK_PLUGIN_CLASS m_auxDefaultCheckClass;
    HM_CHECK_PLUGIN_CLASS m_markDefaultCheckClass;

    uint32_t m_dnsLookupTimeout;
    uint32_t m_dnsRetries;

    uint32_t m_nMaxThreads;
    uint32_t m_nMinThreads;
    uint64_t m_connectionTimeout;

    HM_LOG_PLUGIN_CLASS m_logClass;
    HM_LOG_LEVEL m_logLevel;

    std::string m_logPath;
    std::string m_storagePath;
    HMIPAddress m_dnsServer;
    std::string m_socketPath;
    std::string m_logTimeFormat;
    uint32_t m_maxLogQueue;
    HM_STORAGE_COMMIT_POLICY m_auxPolicy;
    HM_STORAGE_COMMIT_POLICY m_healthCheckPolicy;
    HM_STORAGE_LOCK_POLICY m_storageLockPolicy;
    uint16_t m_controlSocketCheckPortv4;
    uint16_t m_controlSocketCheckPortv6;
    HMHash m_hash;
    std::string m_masterConfig;
    std::vector<HM_CONTROL_SOCKET> m_control_socket;
    bool m_enableSecureRemote;
    std::string m_certFile;
    std::string m_keyFile;
    std::string m_caFile;
    std::string m_PubSubConfigFile;
    bool m_enableMutualAuth;
    bool m_libEventEnabled;
};

#endif /* INCLUDE_HMSTATE_H_ */
