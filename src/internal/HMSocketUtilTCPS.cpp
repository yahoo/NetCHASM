// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <future>
#include <fcntl.h>

#include "HMSocketUtilTCPS.h"

using namespace std;

void
HMSocketUtilTCPS::closeSocket()
{
    if(m_ssl)
    {
        SSL_free(m_ssl);
    }
    HMSocketUtilTCP::closeSocket();
}

bool
HMSocketUtilTCPS::connectSocket()
{
    m_ssl = SSL_new(m_ctx);
    if (m_ssl)
    {
        int status = 0;
        SSL_set_fd(m_ssl, m_socket);
        do
        {
            int ret;
            int cret = SSL_connect(m_ssl);
            if (cret > 0)
            {
                m_connectTime = HMTimeStamp::now();
                return true;
            }
            if (cret == 0)
            {
                return false;

            }
            fd_set fdsr, fdsw;
            FD_ZERO(&fdsr);
            FD_ZERO(&fdsw);
            FD_SET(m_socket, &fdsr);
            FD_SET(m_socket, &fdsw);
            switch (SSL_get_error(m_ssl, cret)) {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                ret = select(m_socket + 1, &fdsr, &fdsw, NULL,
                        &m_connectTimeInfo);
                if (ret < 0)
                {
                    m_reason = HM_REASON_CONNECT_FAILURE;
                }
                else if (ret == 0)
                {
                    m_reason = HM_REASON_CONNECT_TIMEOUT;
                }
                else
                {
                    int v = 0;
                    socklen_t vlen = sizeof(v);
                    if (FD_ISSET(m_socket,
                            &fdsr) || FD_ISSET(m_socket, &fdsw))
                    {
                        if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, &v,
                                &vlen) < 0)
                        {
                            m_reason = HM_REASON_INTERNAL_ERROR;
                        }
                        else if (v == ETIMEDOUT)
                        {
                            m_reason = HM_REASON_CONNECT_TIMEOUT;
                        }
                        else if (!v)
                        {
                            status = 1;
                        }
                        else
                        {
                            m_reason = HM_REASON_CONNECT_FAILURE;
                        }
                    }
                }
                break;
            default:
                status = 0;
                break;
            }
        }
        while (status == 1 && !SSL_is_init_finished(m_ssl));
    }

    return false;
}

HMSocketUtilTCPS::HMSocketUtilTCPS(SSL_CTX* ctx, HMIPAddress& address, uint16_t port, timeval &timeInfo, const HMIPAddress& sourceAddress, const uint8_t tosValue) :
        HMSocketUtilTCP(address, port, timeInfo, sourceAddress, tosValue),
        m_ctx(ctx),
        m_ssl(NULL)
{
    if (connectSocket())
    {
        m_connected = true;
    }
}

HMSocketUtilTCPS::HMSocketUtilTCPS(SSL* ssl) :
        HMSocketUtilTCP(SSL_get_fd(ssl)),
        m_ctx(NULL),
        m_ssl(ssl) { }

HMSocketUtilTCPS::~HMSocketUtilTCPS()
{
    closeSocket();
}

bool
HMSocketUtilTCPS::sendData(const char* buffer, uint64_t size)
{
    if (!m_connected)
    {
        return false;
    }
    if (SSL_write(m_ssl, buffer, size) == (int)size)
    {
        return true;
    }
    return false;
}

bool
HMSocketUtilTCPS::recvData(char* data, uint64_t size, timeval& tv)
{
    if (!m_connected)
    {
        return false;
    }
    uint64_t ret = 0;
    while (ret < size)
    {
        fd_set fdsr;
        FD_ZERO(&fdsr);
        FD_SET(m_socket, &fdsr);
        // either connected or in progress
        int s_ret = select(m_socket + 1, (&fdsr), NULL, NULL, &tv);
        if (s_ret < 0)
        {
            m_reason = HM_REASON_INTERNAL_ERROR;
            m_errorMsg = "select error ";
            return false;
        }
        else if (s_ret == 0)
        {
            m_reason = HM_REASON_RESPONSE_TIMEOUT;
            return false;
        }
        else if (FD_ISSET(m_socket, &fdsr))
        {

            int read_cont;
            do
            {
                int ssl_error;
                read_cont = 0;
                int tret = SSL_read(m_ssl, data + ret, size - ret);

                //check SSL errors
                switch (ssl_error = SSL_get_error(m_ssl, tret))
                {
                case SSL_ERROR_NONE:
                    ret += tret;
                    break;

                case SSL_ERROR_WANT_READ:
                    //the operation did not complete, block the read
                    read_cont = 1;
                    break;

                case SSL_ERROR_ZERO_RETURN:
                case SSL_ERROR_WANT_WRITE:
                case SSL_ERROR_SYSCALL:
                default:
                    m_reason = HM_REASON_INTERNAL_ERROR;
                    uint32_t error = ERR_get_error();
                    if(error)
                    {
                        HMLog(HM_LOG_ERROR,
                                "Error reading data from SSL socket, error desc: %s",
                                ERR_error_string(error, NULL));
                    }
                    else
                    {
                        HMLog(HM_LOG_DEBUG,
                                "No more data available for reading data from SSL socket");
                    }
                    return false;
                }
            } while (SSL_pending(m_ssl) && !read_cont);
        }
        if (ret == size)
        {
            m_reason = HM_REASON_SUCCESS;
        }
    }

    return ret == size;
}
