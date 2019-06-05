// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "HMTimeStamp.h"

using namespace std;

bool
HMTimeStamp::operator==(const HMTimeStamp& k) const
{
    return m_timeStamp == k.m_timeStamp;
}

bool
HMTimeStamp::operator<(const HMTimeStamp& k) const
{
    return m_timeStamp < k.m_timeStamp;
}

bool
HMTimeStamp::operator<=(const HMTimeStamp& k) const
{
    return m_timeStamp <= k.m_timeStamp;
}

bool
HMTimeStamp::operator>(const HMTimeStamp& k) const
{
    return m_timeStamp > k.m_timeStamp;
}

HMTimeStamp
HMTimeStamp::now()
{
    return HMTimeStamp(chrono::time_point_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()));
}

uint64_t
HMTimeStamp::getTimeSinceEpoch() const
{
    return m_timeStamp.time_since_epoch().count();
}

struct timeval
HMTimeStamp::getTimeout() const
{
    int64_t timeout = m_timeStamp.time_since_epoch().count() - HMTimeStamp::now().getTimeSinceEpoch();
    timeout = (timeout < 0) ? 0 : timeout;
    return { (uint32_t)(timeout / 1000), (uint32_t)(timeout % 1000) };
}

void
HMTimeStamp::setTime(uint64_t time)
{
    chrono::milliseconds dur(time);
    chrono::time_point<chrono::system_clock, chrono::milliseconds> dt(dur);
    m_timeStamp = dt;
}

bool
HMTimeStamp::setTime(string time, string format)
{
    tm tm = {};
    if(strptime(time.c_str(),format.c_str(), &tm) == nullptr)
    {
        return false;
    }
    m_timeStamp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::from_time_t(mktime(&tm)));
    return true;
}

string
HMTimeStamp::print(string format)
{
    stringstream ss;
    time_t tt = chrono::system_clock::to_time_t(m_timeStamp);
    tm *tm = gmtime(&tt);
    ss << put_time(tm, format.c_str());
    return ss.str();
}

string
HMTimeStamp::printTimeSinceNow() const
{
    if(m_timeStamp.time_since_epoch().count() == 0)
    {
        return "-";
    }
    HMTimeStamp now =  HMTimeStamp::now();
    uint64_t t = (now - m_timeStamp) / 1000;
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

HMTimeStamp
operator + (const HMTimeStamp& ts, const HMTimeStamp& ts2)
{
    return ts.m_timeStamp + ts2.m_timeStamp;
}

HMTimeStamp
operator + (const HMTimeStamp& ts, uint64_t offset)
{
    return ts.m_timeStamp + chrono::milliseconds(offset);
}

HMTimeStamp
operator + (uint64_t offset, const HMTimeStamp& ts)
{
    return ts.m_timeStamp + chrono::milliseconds(offset);
}

HMTimeStamp
operator - (const HMTimeStamp& ts, uint64_t offset)
{
    return ts.m_timeStamp - chrono::milliseconds(offset);
}

uint64_t
operator - (const HMTimeStamp& ts1, const HMTimeStamp& ts2)
{
    return (ts1.m_timeStamp - ts2.m_timeStamp).count();
}
