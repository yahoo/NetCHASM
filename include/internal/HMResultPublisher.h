// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMRESULTPUBLISHER_H_
#define HMRESULTPUBLISHER_H_

#include <map>
#include <memory>
#include <set>

#include "HMPubSubDataPacking.h"

class HMPublisherBase;
class HMDataCheckResult;
class HMResultPublisher
{
public:
    //! This function is called to register a publisher.
    /*
         This function is called to register a publisher.
         \param name of the publisher
         \param unique pointer of the publisher.
         \return true on success
     */
    bool registerPublisher(std::string name, std::unique_ptr<HMPublisherBase>& publisher);

    //! This function is called to unregister a publisher.
    /*
         This function is called to register a publisher.
         \param name of the publisher
         \return true on success
     */
    bool unregisterPublisher(std::string& name);
    //! This function is called to publish the check information to the publisher.
    /*
         This function is called to publish the check information to the publisher.
         \param host name;
         \param check result of the host.
         \param mark value used 0 Otherwise
         \param hostgroups associated with the check
         \param boolean specifying if the state changed
         \return true on success
     */
    bool publishResult(const std::string& hostName, const HMDataCheckResult& dataCheckResultlt, const int mark, std::set<std::string>& hostGroups, bool changed) const;
    //! Get the number of publishers enrolled
    size_t publishersCount();

    // Used for testing purpose. Not to be used inside the main code.
    HMPublisherBase* getpublisher(std::string name);
private:
    std::map<std::string, std::unique_ptr<HMPublisherBase>> m_publishers;
};

#endif /* HMRESULTPUBLISHER_H_ */
