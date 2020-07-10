// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <cstring>
#include <arpa/inet.h>

#include "HMIPAddress.h"
#include "HMAPI.h"

using namespace std;

HMIPAddress::HMIPAddress() :
m_type(AF_UNSPEC)
{
    memset(&m_ip, 0, sizeof(m_ip));
}

HMIPAddress::HMIPAddress(uint8_t type)
{
    memset(&m_ip.addr6, 0, sizeof(in6_addr));
    if(type == AF_INET)
    {
        m_type = AF_INET;
        m_ip.addr = 0x00000000;
    }
    else if(type == AF_INET6)
    {
        m_type = AF_INET6;
    }
    else
    {
        m_type = AF_UNSPEC;
    }
}

bool
HMIPAddress::operator<(const HMIPAddress& other) const
{
    if(m_type == other.m_type)
    {
        if (m_type == AF_INET)
        {
            return (memcmp(&m_ip, &other.m_ip, sizeof(in_addr_t)) < 0);
        }
        else if(m_type == AF_INET6)
        {
            return (memcmp(&m_ip, &other.m_ip, sizeof(in6_addr)) < 0);
        }
        else
        {
            return false;
        }
    }
    return m_type < other.m_type;
}


bool
HMIPAddress::operator ==(const HMIPAddress &other) const
{
    if (m_type == other.m_type)
    {
        if (m_type == AF_INET)
        {
            return (memcmp(&m_ip, &other.m_ip, sizeof(in_addr_t)) == 0);
        }
        else if(m_type == AF_INET6)
        {
            return (memcmp(&m_ip, &other.m_ip, sizeof(in6_addr)) == 0);
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
 }

bool
HMIPAddress::operator!=(const HMIPAddress &other) const
{
    return !(*this == other);
}

bool
HMIPAddress::set(char* addr, int addrType)
{
    if(addrType == AF_INET)
    {
        return set(*((in_addr_t*)addr));
    }
    else if(addrType == AF_INET6)
    {
        return set(*((in6_addr*)addr));
    }
    m_type = AF_UNSPEC;
    return false;
}

bool
HMIPAddress::set(string addr)
{
    if(inet_pton(AF_INET, addr.c_str(), &m_ip.addr) == 1)
    {
        m_type = AF_INET;
        return true;
    }
    else if(inet_pton(AF_INET6, addr.c_str(), &m_ip.addr6) == 1)
    {
        m_type = AF_INET6;
        return true;
    }
    else
    {
        m_type = AF_UNSPEC;
    }
    return false;
}

bool
HMIPAddress::set(in_addr_t &addr)
{
    m_type = AF_INET;
    m_ip.addr = addr;
    return true;
}

bool
HMIPAddress::set(struct in6_addr &addr)
{
    m_type = AF_INET6;
    m_ip.addr6 = addr;
    return true;
}

bool
HMIPAddress::set(addrinfo &addr)
{
    if(addr.ai_family == AF_INET)
    {
        m_type = AF_INET;
        sockaddr_in* sockaddr4 = (sockaddr_in*)addr.ai_addr;
        if(sockaddr4 != NULL)
        {
            m_ip.addr = sockaddr4->sin_addr.s_addr;
            return true;
        }
    }
    else if(addr.ai_family == AF_INET6)
    {
        m_type = AF_INET6;
        sockaddr_in6* sockaddr6 = (sockaddr_in6*)addr.ai_addr;
        if(sockaddr6 != NULL)
        {
            memcpy(&m_ip.addr6, &sockaddr6->sin6_addr, sizeof(in6_addr));
            return true;
        }
    }
    m_type = AF_UNSPEC;
    return false;
}

void
HMIPAddress::set(const HMAPIIPAddress &k)
{
    m_type = k.m_type;
    if (k.m_type == AF_INET)
    {
        m_ip.addr = k.m_ip.addr;
    }
    else if (k.m_type == AF_INET6)
    {
        m_ip.addr6 = k.m_ip.addr6;
    }
}

bool
HMIPAddress::isSet() const
{
    return m_type != AF_UNSPEC;
}

uint8_t
HMIPAddress::getType() const
{
    return m_type;
}

in_addr_t
HMIPAddress::addr4() const
{
    if(m_type == AF_INET)
    {
        return m_ip.addr;
    }
    return 0;
}

struct
in6_addr HMIPAddress::addr6() const
{
    if(m_type == AF_INET6)
    {
        return m_ip.addr6;
    }
    return {};
}

string
HMIPAddress::toString() const
{
    char buf[INET6_ADDRSTRLEN];
    if (m_type == AF_INET || m_type == AF_INET6)
    {
        inet_ntop(m_type,&m_ip,buf,INET6_ADDRSTRLEN);
        return buf;
    }
    return "";
}

struct sockaddr*
HMIPAddress::getSockaddr (struct sockaddr_storage* sa, socklen_t* salen, int port) const
{
    if (m_type == AF_INET)
    {
        struct sockaddr_in* sin;
        if (*salen < sizeof(*sin))
        {
            return NULL;
        }
        sin = (struct sockaddr_in*)sa;
        sin->sin_family = m_type;
        sin->sin_port = htons(port);
        sin->sin_addr.s_addr = m_ip.addr;
        *salen = sizeof(*sin);
    }
    else if (m_type == AF_INET6)
    {
        struct sockaddr_in6* sin;
        if (*salen < sizeof(*sin))
        {
            return NULL;
        }
        sin = (struct sockaddr_in6*)sa;
        sin->sin6_family = m_type;
        sin->sin6_port = htons(port);
        sin->sin6_addr = m_ip.addr6;
        *salen = sizeof(*sin);
    }
    return (struct sockaddr*)sa;
}
