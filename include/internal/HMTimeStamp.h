// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMTIMESTAMP_H_
#define HMTIMESTAMP_H_

#include <chrono>
#include <iomanip>

//! The core time stamp class used in NetCHASM.
/*!
     The core time stamp class used in NetCHASM. Basically a convenience wrapper around std::chronos.
     All time stamps are stored in milliseconds by default.
 */
class HMTimeStamp
{

public:

    static const uint64_t HOURINMS = 86400000;

    HMTimeStamp() :
        m_timeStamp(std::chrono::time_point<std::chrono::high_resolution_clock,std::chrono::milliseconds>()) {};

    HMTimeStamp(std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> point) :
        m_timeStamp(point) {};

    bool operator==(const HMTimeStamp& k) const;
    bool operator<(const HMTimeStamp& k) const;
    bool operator<=(const HMTimeStamp& k) const;
    bool operator>(const HMTimeStamp& k) const;

    //! Static function to return an HMTimeStamp initialized to now.
    /*!
         Static function to return an HMTimeStamp initialized to now.
         \return an HMTimeStamp set to now.
     */
    static HMTimeStamp now();

    //! Get the time since the epoch for this time stamp in seconds.
    /*!
         Get the time since epoch for this time stamp in seconds.
         \return the time since epoch in seconds.
     */
    uint64_t getTimeSinceEpoch() const;

    //! Get the amount of time from now to this time stamp as a timeval timeout.
    /*!
         Get the amount of time from now to this time stamp as a timeval timeout.
         \return a timeval representing the difference between this time stamp and now.
     */
    struct timeval getTimeout() const;

    //! Set the time for this time stamp.
    /*!
         Set the time for this time stamp.
         \param the time to set in seconds since epoch.
     */
    void setTime(uint64_t time);

    //! Set the time for this time stamp.
    /*!
         Set the time for this time stamp.
         \param the time to set as a human readable string.
         \param the format to use to parse the time.
         \return true if the time stamp was parsed successfully.
     */
    bool setTime(std::string time, std::string format);

    //! Print the time stamp.
    /*!
         Print the time stamp.
         \param the format to use to print the time stamp.
         \return the string representation of the time stamp.
     */
    std::string print(std::string format) const;

    //! Print the elapsed time from this time stamp to now.
    /*!
         Print the elapsed time from this time stamp to now.
         \return the time elapsed from this time stamp to now in the format day:hour:minute.
     */
    std::string printTimeSinceNow() const;

    friend HMTimeStamp operator + (const HMTimeStamp& ts, const HMTimeStamp& ts2);
    friend HMTimeStamp operator + (const HMTimeStamp& ts, uint64_t offset);
    friend HMTimeStamp operator + (uint64_t offset, const HMTimeStamp& ts);
    friend HMTimeStamp operator - (const HMTimeStamp& ts, uint64_t offset);
    friend uint64_t operator - (const HMTimeStamp& ts1, const HMTimeStamp& ts2);

    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> m_timeStamp;
};

#endif /* HMTIMESTAMP_H_ */
