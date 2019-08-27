// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <csignal>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <future>

#include "HMControlTLSSocketClient.h"
#include "HMControlBase.h"

using namespace std;

bool
HMControlTLSSocketClient::createSocket()
{
    if ((m_socket = socket(m_address.m_type, SOCK_STREAM, 0)) == -1)
    {
        setError("Failed to create socket, error: " + std::string(strerror(errno)));
        m_socket = -1;
        return false;
    }
    return true;
}

void
HMControlTLSSocketClient::closeSocket()
{
    if (m_socket != -1)
    {
        close(m_socket);
    }
}

bool
HMControlTLSSocketClient::connectSocket()
{
    if (!createSocket())
    {
        return false;
    }
    sockaddr_storage server;
    socklen_t len;
    if (m_address.m_type == AF_INET6)
    {
        sockaddr_in6 *addr = (sockaddr_in6 *) &server;
        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(m_port);
        addr->sin6_addr = m_address.m_ip.addr6;
        len = sizeof(sockaddr_in6);
    }
    else
    {
        sockaddr_in *addr = (sockaddr_in *) &server;
        addr->sin_family = AF_INET;
        addr->sin_port = htons(m_port);
        addr->sin_addr.s_addr = m_address.m_ip.addr;
        len = sizeof(sockaddr_in);
    }
    if (connect(m_socket, (struct sockaddr *) &server, len) == -1)
    {
        setError("Failed to connect socket, error: " + std::string(strerror(errno)));
        closeSocket();
        m_socket = -1;
        return false;
    }
    if(m_ssl)
    {
        SSL_free(m_ssl);
    }
    m_ssl = SSL_new(m_ctx);
    if (!m_ssl)
    {
        setError("Failed to create ssl Instance" + std::string(ERR_error_string(ERR_get_error(), NULL)));
    }
    SSL_set_fd(m_ssl, m_socket);
    if (SSL_connect(m_ssl) <= 0)
    {
        setError("Failed to connect socket ssl, error: " + std::string(ERR_error_string(ERR_get_error(), NULL)));
        closeSocket();
        m_socket = -1;
        return false;
    }
    return true;
}

HMControlTLSSocketClient::HMControlTLSSocketClient(HMAPIIPAddress& address, uint16_t port, string& certFile, string& keyFile, string& caFile) :
        m_socket(-1),
        m_address(address),
        m_port(port),
        m_ssl(NULL)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    m_ctx = SSL_CTX_new(TLSv1_2_method());
    if (SSL_CTX_use_certificate_file(m_ctx, certFile.c_str(),
    SSL_FILETYPE_PEM) <= 0)
    {
        setError(
                "Failed loading certificate file, err:"
                        + std::string(ERR_error_string(ERR_get_error(), NULL)));
        return;
    }
    if (SSL_CTX_use_PrivateKey_file(m_ctx, keyFile.c_str(),
    SSL_FILETYPE_PEM) <= 0)
    {
        setError(
                "Failed loading private key file, err:"
                        + std::string(ERR_error_string(ERR_get_error(), NULL)));
        return;
    }
    if (!SSL_CTX_check_private_key(m_ctx))
    {
        setError("Private key does not match the public certificate");
        return;
    }
    if (!SSL_CTX_load_verify_locations(m_ctx, caFile.c_str(), NULL))
    {
        setError(
                "failed loading CA certificate, err:"
                        + std::string(ERR_error_string(ERR_get_error(), NULL)));
        return;
    }

    SSL_CTX_set_verify(m_ctx,
    SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

    if (connectSocket())
    {
        m_connected = true;
    }

}

HMControlTLSSocketClient::~HMControlTLSSocketClient()
{
    SSL_free(m_ssl);
    SSL_CTX_free(m_ctx);
    closeSocket();
}

bool
HMControlTLSSocketClient::sendMessage(const string& cmd)
{
    if (!m_connected)
    {
        setError("Socket not connected");
        return false;
    }
    uint64_t cmdLen = cmd.length();
    cmdLen = HMDataPacking::hton64(cmdLen);
    if (SSL_write(m_ssl, &cmdLen, sizeof(cmdLen)) == sizeof(cmdLen))
    {
        if (SSL_write(m_ssl, cmd.c_str(), cmd.length())
                == (int32_t) cmd.length())
        {
            return true;
        }
    }
    if ( errno == EPIPE)
    {
        if (!connectSocket())
        {
            return false;
        }
        if (SSL_write(m_ssl, &cmdLen, sizeof(cmdLen)) == sizeof(cmdLen))
        {
            if (SSL_write(m_ssl, cmd.c_str(), cmd.length())
                    == (int32_t) cmd.length())
            {
                return true;
            }
        }
    }
    setError("failed to send msg " + cmd + " on socket, error desc: " + std::string(strerror(errno)));
    closeSocket();
    return false;
}

bool
HMControlTLSSocketClient::sendData(const char* buffer, uint32_t size)
{
    if (!m_connected)
    {
        setError("Socket not connected");
        return false;
    }
    if (SSL_write(m_ssl, buffer, size) == -1)
    {
        if ( errno == EPIPE)
        {
            connectSocket();
        }
        setError("failed to send msg on socket, error desc: " + std::string(strerror(errno)));
        closeSocket();
        return false;
    }
    return true;
}

bool
HMControlTLSSocketClient::recvMessage(char* data, uint64_t size)
{
    if (!m_connected)
    {
        setError("Socket not connected");
        return false;
    }
    int n = SSL_read(m_ssl, data, size);
    return n == (int) size;
}
