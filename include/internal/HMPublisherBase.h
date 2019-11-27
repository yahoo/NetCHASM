// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMPUBLISHERBASE_H_
#define HMPUBLISHERBASE_H_
#include <iostream>
#include <set>
#include <vector>

class HMDataCheckResult;
class HMPublisherBase
{
public:
    HMPublisherBase() : m_publishAll(true), m_publishOnChange(false) {}
    virtual ~HMPublisherBase() {}
    //! Get the publish all value
    bool isPublishAll() const;
    //! Set to publish all the results
    void setPublishAll(bool publishAll);
    //! Add host group to filter result publish
    void addHostGroup(const std::string hostGroup);
    //! Add host groups to filter result publish
    void addHostGroups(const std::set<std::string>& hostGroups);
    //! Remove added filter host group
    void removeHostGroup(const std::string hostGroup);
    //! Remove added filter host groups
    void removeHostGroups(const std::set<std::string>& hostGroups);
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
    void publishResult(const std::string& hostName, const HMDataCheckResult& dataCheckResult, const int mark, std::set<std::string>& hostGroups, bool changed) const;
    //! Check if the publish on state change is enabled
    bool isPublishOnChange() const;
    //! Enable publish on state change is enabled
    void setPublishOnChange(bool publishOnChange);
    //! Return added host groups
    const std::set<std::string>& getHostGroups() const;

protected:
    //! This function is called to publish the check information to the publisher.
    /*
         This function is called to publish the check information to the publisher.
         \param pointer to data;
         \param size of data.
     */
    virtual void publish(const std::string& hostName, const HMDataCheckResult& dataCheckResult, const int mark, std::set<std::string>& hostGroups) const = 0;
private:
    bool m_publishAll;
    bool m_publishOnChange;
    std::set<std::string> m_hostGroups;

    //! This function is called to check if the hostgroups associated with check match the filter
    /*
     This function is called to check if the hostgroups associated with check match the filter
     \param hostgroups associated with check
     \param return data structure to hold common hostgroups.
     */
    bool matchingHostGroups(const std::set<std::string>& hostGroups, std::vector<std::string>& resultHostGroups) const;
};

#endif /* HMPUBLISHERBASE_H_ */
