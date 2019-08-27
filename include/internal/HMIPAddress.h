// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMIPADDRESS_H_
#define HMIPADDRESS_H_

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <cstdint>
#include <string.h>

class HMAPIIPAddress;
class HMIPAddress
{
public:
    HMIPAddress();
    HMIPAddress(uint8_t type);
    bool operator<(const HMIPAddress& k) const;
    bool operator==(const HMIPAddress &other) const;
    bool operator!=(const HMIPAddress &other) const;

    //! Set the IPAddress
    /*!
         Set the IPAddress using a raw byte-by-byte copy.
         \param buffer with raw address information.
         \param type of the address.
         \return true if the address was set successfully.
     */
    bool set(char* addr, int addrType);

    //! Set the IPAddress
    /*!
         Set the IPAddress
         \param an IP Address in human readable string format.
         \return true if the address was set successfully.
     */
    bool set(std::string addr);

    //! Set the IPAddress
    /*!
         Set the IPAddress
         \param IPv4 address struct.
         \return true if the address was set successfully.
     */
    bool set(in_addr_t &addr);

    //! Set the IPAddress
    /*!
         Set the IPAddress
         \param IPv6 address struct.
         \return true if the address was set successfully.
     */
    bool set(struct in6_addr &addr);

    //! Set the IPAddress
    /*!
         Set the IPAddress
         \param API IP address class .
         \return true if the address was set successfully.
     */
    void set(const HMAPIIPAddress &addr);

    //! Set the IPAddress
    /*!
         Set the IPAddress
         \param addrinfo struct.
         \return true if the address was set successfully.
     */
    bool set(addrinfo &addr);

    //! Check to see if the HMIPAddress has been set.
    /*!
         Check to see if the HMIPAddress has been set.
         \return true if the type is not AF_UNSPEC.
     */
    bool isSet() const;

    //! Get the type of address stored.
    /*!
         Get the type of address stored.
         \return the type code AF_INET, AF_INET6, AF_UNSPEC.
     */
    uint8_t getType() const;

    //! Get the IPv4 Address.
    /*!
         Get the IPv4 in_addr_t struct.
         \return the in_addr_t struct (0 on failure)
     */
    in_addr_t addr4() const;

    //! Get the IPv6 Address.
    /*!
         Get the IPv6 in6_addr struct.
         \return the in6_addr struct (0 on failure)
     */
    struct in6_addr addr6() const;

    //! Convert the address to a string.
    /*!
         Convert the address to a string.
         \return the string representing the IPAdddress.
     */
    std::string toString() const;

    //! Get a sockaddr struct set to the current address.
    /*!
         Get a sockaddr struct set to the current address.
         \param a pointer to the sockaddr to fill.
         \param an int to fill with the size of the sockaddr.
         \param the port to fill into the struct.
         \return the pointer to the sockadd struct.
     */
    struct sockaddr* getSockaddr (struct sockaddr_storage* sa, socklen_t* salen, int port) const;

private:
    uint8_t m_type;

    union
    {
        in_addr_t addr;
        struct in6_addr addr6;
    } m_ip;
};

#endif /* HMIPADDRESS_H_ */
