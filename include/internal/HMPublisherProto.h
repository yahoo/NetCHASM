// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMPUBLISHER_H_
#define HMPUBLISHERPROTO_H_
#include <iostream>
#include <set>
#include <vector>
#include <memory>
#include "HMPublisherBase.h"

class HMPublisherProto : public HMPublisherBase
{
public:
    ~HMPublisherProto() {}
    //! This function is called to publish the check information to the publisher.
    /*
         This function is called to publish the check information to the publisher.
         \param host name;
         \param check result of the host.
         \param mark value used 0 Otherwise
         \param hostgroups associated with the check
     */
    void publish(const std::string& hostName, const HMDataCheckResult& dataCheckResult, const int mark, std::set<std::string>& hostGroups) const;
protected:
    //! This function is called to publish the check information to the publisher.
    /*
         This function is called to publish the check information to the publisher.
         \param pointer to data;
         \param size of data.
     */
    virtual void publish(char* data, const size_t datalen) const = 0;
};

#endif /* HMPUBLISHERPROTO_H_ */
