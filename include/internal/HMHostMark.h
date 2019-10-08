// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_INTERNAL_HMHOSTMARK_H_
#define INCLUDE_INTERNAL_HMHOSTMARK_H_

#include "HMDataHostCheck.h"
//! Key class to the host mark map
class HMHostMarkOptKey
{
public:
    HMHostMarkOptKey(const std::string& hostName, const HMIPAddress& address, const HMDataHostCheck& hostCheck) :
            m_hostName(hostName), m_address(address), m_hostCheck(hostCheck)
    { }

    bool operator<(const HMHostMarkOptKey& k) const;
    bool operator!=(const HMHostMarkOptKey& k) const;
    bool operator==(const HMHostMarkOptKey& k) const;


private:
    std::string m_hostName;
    HMIPAddress m_address;
    HMDataHostCheck m_hostCheck;
};

//! Class holding information to mask that needs to be set for a particular host.
class HMHostMark
{
public:

    /*!
         set mark to a particular host and datahostcheck.
         \param hostname to add the mark
         \param address of the host to set the mark
         \param datahostcheck for the mark
         \param mark value
         \return true if successful.
     */
    bool setSocketOption(const std::string& hostName, const HMIPAddress& address, const HMDataHostCheck& hostCheck, int value);

    /*!
         get mark that is set to a particular host and datahostcheck.
         \param hostname to get the mark
         \param address of the host to get the mark
         \param datahostcheck for the mark
         \param variable to store the mark value
         \return true if successful.
     */
    bool getSocketOption(const std::string& hostName, const HMIPAddress& address, const HMDataHostCheck& hostCheck, int& value);

    /*!
         remove mark to a particular host and datahostcheck.
         \param hostname to remove the mark
         \param address of the host to remove the mark
         \param datahostcheck for the mark
         \return true if successful.
     */
    bool removeSocketOption(const std::string& hostName, const HMIPAddress& address, const HMDataHostCheck& hostCheck);
private:
    std::map<HMHostMarkOptKey, int> m_hostSockOptMap;
    std::shared_timed_mutex m_mutex;
};


#endif /* INCLUDE_INTERNAL_HMHOSTMARK_H_ */
