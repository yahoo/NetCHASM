// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMPUBLISHERDEFAULT_H_
#define HMPUBLISHERDEFAULT_H_
#include <iostream>
#include <set>
#include <vector>
#include <memory>
#include "HMPublisherBase.h"

class HMPublisherDefault : public HMPublisherBase
{
public:
    ~HMPublisherDefault() {}
    //! This function is called to publish the check information to the publisher.
    /*
         This function is called to publish the check information to the publisher.
         \param host name;
         \param check result of the host.
         \param mark value used 0 Otherwise
         \param hostgroups associated with the check
     */
    void publish(const std::string& hostName, const HMDataCheckResult& dataCheckResult, const int mark, std::set<std::string>& hostGroups) const;
};

#endif /* HMPUBLISHERDEFAULT_H_ */
