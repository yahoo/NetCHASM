// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <openssl/ssl.h>
#include <openssl/asn1.h>
#include <openssl/x509v3.h>

#include <event2/bufferevent_ssl.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/http.h>

#include "HMWorkHealthCheckLibEvent.h"
#include "HMEventLoopLibEvent.h"
#include "HMStateManager.h"
#include "HMLogBase.h"

using namespace std;

HM_WORK_STATUS
HMWorkHealthCheckLibEvent::healthCheck()
{
    if (m_hostCheck.getCheckType() == HM_CHECK_HTTP
            || m_hostCheck.getCheckType() == HM_CHECK_HTTPS
            || m_hostCheck.getCheckType() == HM_CHECK_HTTPS_NO_PEER_CHECK)
    {
        string uri;
        string url;
        string checkInfoHost;
        bool sni = false;
        uint32_t port = m_hostCheck.getPort();
        uint32_t checkInfoPort;
        string headerHostname = "";
        string hostsni = "";

        // check to make sure we have a lib event instance
        HMEventLoopLibEvent* le = m_stateManager->getLibEvent();

        if(le == nullptr)
        {
            HMLog(HM_LOG_ERROR,"[LIBHTTP] LibEvent is not initialized");
            m_response = HM_RESPONSE_FAILED;
            m_reason = HM_REASON_INTERNAL_ERROR;
            return HM_WORK_COMPLETE;
        }

        struct event_base* base = le->getEventBase();
        if(base == nullptr)
        {
            HMLog(HM_LOG_ERROR,"[LIBHTTP] LibEvent is not initialized");
            m_response = HM_RESPONSE_FAILED;
            m_reason = HM_REASON_INTERNAL_ERROR;
            return HM_WORK_COMPLETE;
        }

        if (m_hostCheck.getCheckType() == HM_CHECK_HTTPS_NO_PEER_CHECK)
        {
            m_verifyPeer = false;
        }

        // setup the HTTP parameters
        // we need to parse the checkInfo parameter
        // Check to see if we have the format: //hostname/uriCURLOPT_NOSIGNAL
        string checkInfo = m_hostCheck.getCheckInfo();
        if (checkInfo.size() > 2 && checkInfo[0] == '/' && (checkInfo)[1] == '/')
        {
            int hostLen = 0;
            size_t index;
            if ((index = checkInfo.find("/", 2)) != std::string::npos)
            {
                //checkinfo //xxxx/yy hostLen = 4 (xxxx)
                hostLen = checkInfo.substr(2, (checkInfo.find("/", 2) - 2)).length();
            }
            else
            {
                //checkinfo //xxx
                hostLen = checkInfo.substr(2).length();
            }
            if (hostLen)
            {
                sni = true;
                //check for special cases checkinfo //<host>, //<host:port>, //xx
                headerHostname = m_hostCheck.parseCheckInfo(m_hostname, checkInfoPort,
                        checkInfoHost);
                HMLog(HM_LOG_DEBUG3,
                        "[LIBHTTP] Curl slist %s for %s : %d at %s with checkinfo %s",
                        headerHostname.c_str(), m_hostname.c_str(), port,
                        m_ipAddress.toString().c_str(), checkInfo.c_str());
                //slist = curl_slist_append(slist, hostname.c_str());

                // checkinfo //xxx/yyy, uri = yyy
                //checkinfo //xxx ,uri is empty
                if (index != std::string::npos)
                {
                    uri = checkInfo.substr(index);
                    HMLog(HM_LOG_DEBUG3,
                            "[LIBHTTP] Curl uri %s for %s : %d at %s with checkinfo %s",
                            uri.c_str(), m_hostname.c_str(), port,
                            m_ipAddress.toString().c_str(), checkInfo.c_str());
                }
            }
            else
            {
                // checkInfo /// uri = /,  checkinfo ///xx uri = /xx
                uri = checkInfo.substr(2); // checkInfo ///
                HMLog(HM_LOG_DEBUG3,
                        "[LIBHTTP] Curl uri %s for %s : %d at %s with checkinfo %s",
                        uri.c_str(), m_hostname.c_str(), port,
                        m_ipAddress.toString().c_str(), checkInfo.c_str());
            }
        }
        else
        {
            uri = checkInfo;
        }

        // Now we setup SNI and the URL
        url = ((m_hostCheck.getCheckType() == HM_CHECK_HTTP) ?
                "http://" : "https://");

        //if the mode is http or https with checkInfo /xxx or ///xxxx
        if ((sni == false) || (m_hostCheck.getCheckType() == HM_CHECK_HTTP))
        {
            if (m_ipAddress.getType() == AF_INET6)
            {
                url = url + "[" + m_ipAddress.toString() + "]:"
                        + std::to_string((uint64_t) port);
            }
            else
            {
                url = url + m_ipAddress.toString() + ":"
                        + std::to_string((uint64_t) port);
            }
        }
        else
        {
            //sni enabled https and https_no_peer_check
            if (checkInfoHost.empty())
            {
                //checkinfo //<host>/ or //<host:port>/
                if (m_ipAddress.getType() == AF_INET6)
                {
                    hostsni = m_hostname + ":" + std::to_string((uint64_t) port)
                    + ":[" + m_ipAddress.toString() + "]" + ":"
                    + std::to_string((uint64_t) port);
                }
                else
                {
                    hostsni = m_hostname + ":" + std::to_string((uint64_t) port)
                    + ":" + m_ipAddress.toString() + ":"
                    + std::to_string((uint64_t) port);
                }

                if (port == 443)
                {
                    // if default port then only the host name is passed in the url
                    url = url + m_hostname;
                }
                else
                {
                    url = url + m_hostname + ":"
                            + std::to_string((uint64_t) port);
                }
            }
            else
            {
                //checkinfo //hostname:port/ or //hostname/
                if (m_ipAddress.getType() == AF_INET6)
                {
                    //checkinfohost:checkinfoport:[ip]:checkport
                    hostsni = checkInfoHost + ":"
                            + std::to_string((uint64_t) checkInfoPort) + ":["
                            + m_ipAddress.toString() + "]" + ":"
                            + std::to_string((uint64_t) port);
                }
                else
                {
                    //checkinfohost:checkinfoport:ip:checkport
                    hostsni = checkInfoHost + ":"
                            + std::to_string((uint64_t) checkInfoPort) + ":"
                            + m_ipAddress.toString() + ":"
                            + std::to_string((uint64_t) port);
                }

                if (checkInfoPort == 443)
                {
                    // if default port then only the host name is passed in the url
                    url = url + checkInfoHost;
                }
                else
                {
                    url = url + checkInfoHost + ":"
                            + std::to_string((uint64_t) checkInfoPort);

                }
            }
            HMLog(HM_LOG_DEBUG3,
                    "[LIBHTTP] curl SNI host header %s for CurlCheck for %s at %s with checkinfo %s",
                    hostsni.c_str(), m_hostname.c_str(),
                    m_ipAddress.toString().c_str(), checkInfo.c_str());
        }

        if (uri.empty())
        {
            url = url + "/status.html";
        }
        else
        {
            url = url + uri;
        }

        // Now we start to setup the connection
        if( m_hostCheck.getCheckType() == HM_CHECK_HTTPS
            || m_hostCheck.getCheckType() == HM_CHECK_HTTPS_NO_PEER_CHECK)
        {
            // now get the cert store
            SSL_CTX* ssl_ctx = le->getSSLCertStore();
            if(ssl_ctx == nullptr)
            {
                HMLog(HM_LOG_ERROR,"[LIBHTTP] LibEvent Cert Store is not initialized");
                m_response = HM_RESPONSE_FAILED;
                m_reason = HM_REASON_INTERNAL_ERROR;
                return HM_WORK_COMPLETE;
            }
            SSL_CTX_set_cert_verify_callback(ssl_ctx, certifyCallback, (void *) this);

            // Create OpenSSL bufferevent and stack evhttp on top of it
            m_ssl = SSL_new(ssl_ctx);
            if (m_ssl == NULL) {
                HMLog(HM_LOG_ERROR,"[LIBHTTP] LibEvent SSL failed to initialize");
                m_response = HM_RESPONSE_FAILED;
                m_reason = HM_REASON_INTERNAL_ERROR;
                return HM_WORK_COMPLETE;
            }

            if(sni)
            {
                SSL_set_tlsext_host_name(m_ssl, hostsni.c_str());
            }

            m_bev = bufferevent_openssl_socket_new(base, -1, m_ssl,
                                BUFFEREVENT_SSL_CONNECTING,
                                BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
        }

        {
            m_bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
        }

        if (m_bev == NULL)
        {
            HMLog(HM_LOG_ERROR,"[LIBHTTP] LibEvent Socket failed to initialize");
            if (m_ssl)
            {
                SSL_free(m_ssl);
            }
            m_response = HM_RESPONSE_FAILED;
            m_reason = HM_REASON_INTERNAL_ERROR;
            return HM_WORK_COMPLETE;
        }

        bufferevent_setcb(m_bev, NULL, NULL, connectCallback, this);

        bufferevent_openssl_set_allow_dirty_shutdown(m_bev, 1);

        m_evcon = evhttp_connection_base_bufferevent_new(base, NULL, m_bev, m_ipAddress.toString().c_str(), port);
        if (m_evcon == nullptr)
        {
            if (m_ssl)
            {
                SSL_free(m_ssl);
            }
            m_response = HM_RESPONSE_FAILED;
            m_reason = HM_REASON_INTERNAL_ERROR;
            return HM_WORK_COMPLETE;
        }


        evhttp_connection_set_retries(m_evcon, 1);
        evhttp_connection_set_timeout(m_evcon, m_timeout.getTimeSinceEpoch() - HMTimeStamp::now().getTimeSinceEpoch());


        // Fire off the request
        m_req = evhttp_request_new(requestDone, this);

        if (m_req == NULL) {
            if (m_evcon)
            {
                evhttp_connection_free(m_evcon);
            }
            if (m_ssl)
            {
                SSL_free(m_ssl);
            }

            m_response = HM_RESPONSE_FAILED;
            m_reason = HM_REASON_INTERNAL_ERROR;
            return HM_WORK_COMPLETE;
        }

        evhttp_request_set_error_cb(m_req, errorCallback);

        m_headers = evhttp_request_get_output_headers(m_req);

        if(!headerHostname.empty())
        {
            evhttp_add_header(m_headers, "Host", headerHostname.c_str());
        }
        else
        {
            evhttp_add_header(m_headers, "Host", m_ipAddress.toString().c_str());
        }
        evhttp_add_header(m_headers, "Accept", "*/*");
        evhttp_add_header(m_headers, "Connection", "close");
        evhttp_add_header(m_headers, "User-Agent", "YahooFOR/1.0");

        m_end.setTime(0);
        m_start = HMTimeStamp::now();
        if(evhttp_make_request(m_evcon, m_req, EVHTTP_REQ_GET, uri.c_str()) != 0)
        {

            if(m_req)
            {
                evhttp_request_free(m_req);
            }
            if (m_evcon)
            {
                evhttp_connection_free(m_evcon);
            }
            if (m_ssl)
            {
                SSL_free(m_ssl);
            }

            m_response = HM_RESPONSE_FAILED;
            m_reason = HM_REASON_INTERNAL_ERROR;
            return HM_WORK_COMPLETE;
        }

        return HM_WORK_IN_PROGRESS;
    }
    else
    {
        HMLog(HM_LOG_ERROR, "[LIBHTTP] Invalid Check type for CurlCheck %s",
                printCheckType(m_hostCheck.getCheckType()).c_str());
        m_response = HM_RESPONSE_FAILED;
        m_reason = HM_REASON_INTERNAL_ERROR;
        return HM_WORK_COMPLETE;
    }
}

void
HMWorkHealthCheckLibEvent::requestDone(struct evhttp_request *req, void* arg)
{

    HMWorkHealthCheckLibEvent* work = (HMWorkHealthCheckLibEvent*) arg;
    if(work->m_end.getTimeSinceEpoch() == 0)
    {
        work->m_end = HMTimeStamp::now();
    }

    // If the request is null than libevent did something stupid
    if(req == nullptr)
    {
        work->m_response = HM_RESPONSE_FAILED;
        work->m_reason = HM_REASON_NONE;
    }
    else
    {
        int httpCode = evhttp_request_get_response_code(req);
        work->m_response = HM_RESPONSE_CONNECTED;

        if (httpCode == 200)
        {
            work->m_reason = HM_REASON_SUCCESS;

            // read the returned data
            char buffer[256];
            size_t nread;
            work->rcvdBuffer.clear();
            while ((nread = evbuffer_remove(evhttp_request_get_input_buffer(req), buffer, sizeof(buffer))) > 0)
            {
                work->rcvdBuffer = work->rcvdBuffer + std::string(buffer, nread);
            }
        }
        else
        {
            if (httpCode >= 300 && httpCode < 400)
            {
                work->m_reason = HM_REASON_RESPONSE_3XX;
            }
            else if (httpCode == 403)
            {
                work->m_reason = HM_REASON_RESPONSE_403;
            }
            else if (httpCode == 404)
            {
                work->m_reason = HM_REASON_RESPONSE_404;
            }
            else if (httpCode >= 500 && httpCode < 600)
            {
                work->m_reason = HM_REASON_RESPONSE_5XX;
            }
            else
            {
                work->m_reason = HM_REASON_RESPONSE_DOWN;
            }
        }

    }

    HMLog(HM_LOG_DEBUG3,
            "[LIBHTTP] LibEvent fetching http url for hostname: %s:%d  with check info %s returned reason %s ",
            work->m_hostname.c_str(), work->m_hostCheck.getPort(),
            work->m_hostCheck.getCheckInfo().c_str(), printReason(work->m_reason).c_str());

    work->m_workStatus = HM_WORK_COMPLETE;

    if (work->m_evcon)
    {
        evhttp_connection_free(work->m_evcon);
    }
    if (work->m_ssl)
    {
        SSL_free(work->m_ssl);
    }

    work->m_stateManager->m_workQueue.addWork((HMWork *) work);
}

void
HMWorkHealthCheckLibEvent::errorCallback(evhttp_request_error error, void* arg)
{
    HMWorkHealthCheckLibEvent* work = (HMWorkHealthCheckLibEvent*) arg;
    work->m_workStatus = HM_WORK_COMPLETE;

    if(error == EVREQ_HTTP_TIMEOUT)
    {
        work->m_reason = HM_REASON_RESPONSE_TIMEOUT;
    }
    else if (error == EVREQ_HTTP_EOF)
    {
        work->m_reason = HM_REASON_CONNECT_FAILURE;
    }
    else
    {
        work->m_reason = HM_REASON_RESPONSE_FAILURE;
    }
}

void
HMWorkHealthCheckLibEvent::connectCallback(struct bufferevent* bev, short events, void* arg)
{
    (void) bev;
    HMWorkHealthCheckLibEvent* work = (HMWorkHealthCheckLibEvent*)arg;
    if(events & BEV_EVENT_CONNECTED)
    {
        work->m_end = HMTimeStamp::now();
    }
}

int
HMWorkHealthCheckLibEvent::certifyCallback(X509_STORE_CTX *x509_ctx, void *arg)
{
    HMWorkHealthCheckLibEvent* work = (HMWorkHealthCheckLibEvent*)arg;
    X509 *serverCert = NULL;

    if (!work->m_verifyPeer)
    {
        return 1;
    }

    if(!X509_verify_cert(x509_ctx))
    {
        HMLog(HM_LOG_DEBUG3, "X509 cert verification failed for %s", work->m_hostname);
        return 0;
    }

    if(!work->m_verifyHost)
    {
        return 1;
    }

    serverCert = X509_STORE_CTX_get_current_cert(x509_ctx);
    if(serverCert == nullptr)
    {
        HMLog(HM_LOG_DEBUG3, "X509 cert error failed for %s", work->m_hostname);
        return 0;
    }

    int numNames = -1;
    STACK_OF(GENERAL_NAME)* names = nullptr;

    // Try to extract the names within the SAN extension from the certificate
    names = (stack_st_GENERAL_NAME*)X509_get_ext_d2i((X509 *) serverCert, NID_subject_alt_name, nullptr, nullptr);
    if (names != nullptr)
    {
        numNames = sk_GENERAL_NAME_num(names);

        // Check each name within the extension
        for (int i = 0; i < numNames; ++i)
        {
            const GENERAL_NAME* currentName = sk_GENERAL_NAME_value(names, i);

            if (currentName->type == GEN_DNS)
            {
                // Current name is a DNS name, let's check it
                const char *dns_name = (char *) ASN1_STRING_get0_data(currentName->d.dNSName);

                // Make sure there isn't an embedded NUL character in the DNS name
                if ((size_t)ASN1_STRING_length(currentName->d.dNSName) != strlen(dns_name))
                {
                    HMLog(HM_LOG_DEBUG3, "X509 malformed cert for %s", work->m_hostname);
                    sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);
                    return 0;
                }
                else
                { // Compare expected hostname with the DNS name
                    if (curlCertHostCheck(dns_name, work->m_hostname.c_str()) == 1)
                    {
                        HMLog(HM_LOG_DEBUG3, "X509 cert matched for %s", work->m_hostname);
                        sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);
                        return 1;
                    }
                }
            }
        }
        sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);
    }

    int commonNameIndex = -1;
    X509_NAME_ENTRY* commonNameEntry = nullptr;
    ASN1_STRING* commonNameASN1 = nullptr;

    // Find the position of the CN field in the Subject field of the certificate
    commonNameIndex = X509_NAME_get_index_by_NID(X509_get_subject_name((X509 *) serverCert), NID_commonName, -1);
    if (commonNameIndex < 0)
    {
        HMLog(HM_LOG_DEBUG3, "X509 cert error failed for %s", work->m_hostname);
        return 0;
    }

    // Extract the CN field
    commonNameEntry = X509_NAME_get_entry(X509_get_subject_name((X509 *) serverCert), commonNameIndex);
    if (commonNameEntry == NULL)
    {
        HMLog(HM_LOG_DEBUG3, "X509 cert error failed for %s", work->m_hostname);
        return 0;
    }

    // Convert the CN field to a C string
    commonNameASN1 = X509_NAME_ENTRY_get_data(commonNameEntry);
    if (commonNameASN1 == NULL)
    {
        HMLog(HM_LOG_DEBUG3, "X509 cert error failed for %s", work->m_hostname);
        return 0;
    }
    std::string commonName((char *) ASN1_STRING_get0_data(commonNameASN1));

    // Make sure there isn't an embedded NUL character in the CN
    if ((size_t)ASN1_STRING_length(commonNameASN1) != commonName.size())
    {
        HMLog(HM_LOG_DEBUG3, "X509 malformed cert for %s", work->m_hostname);
        return 0;
    }

    // Compare expected hostname with the CN
    if (curlCertHostCheck(commonName.c_str(), work->m_hostname.c_str()) == 1)
    {
        HMLog(HM_LOG_DEBUG3, "X509 cert matched for %s", work->m_hostname);
        return 1;
    }
    else
    {
        HMLog(HM_LOG_DEBUG3, "X509 cert no match found for %s", work->m_hostname);
        return 0;
    }
}

int
HMWorkHealthCheckLibEvent::curlCertHostCheck(const char *match_pattern, const char *hostname)
{
    if(!match_pattern || !*match_pattern || !hostname || !*hostname)
    {
        return 0;
    }

    if(curlRawEqual(hostname, match_pattern))
    {
        return 1;
    }

    if(curlHostMatch(hostname,match_pattern) == 1)
    {
        return 1;
    }
    return 0;
}

int
HMWorkHealthCheckLibEvent::curlRawEqual(const char *first, const char *second)
{
    while(*first && *second) {
        if(curlRawToUpper(*first) != curlRawToUpper(*second))
            /* get out of the loop as soon as they don't match */
            break;
        first++;
        second++;
    }
    /* we do the comparison here (possibly again), just to make sure that if the
     loop above is skipped because one of the strings reached zero, we must not
     return this as a successful match */
    return (curlRawToUpper(*first) == curlRawToUpper(*second));
}

int
HMWorkHealthCheckLibEvent::curlRawNEqual(const char *first, const char *second, size_t max)
{
    while(*first && *second && max) {
        if(curlRawToUpper(*first) != curlRawToUpper(*second)) {
            break;
        }
        max--;
        first++;
        second++;
    }
    if(0 == max)
        return 1; /* they are equal this far */

    return curlRawToUpper(*first) == curlRawToUpper(*second);
}

char
HMWorkHealthCheckLibEvent::curlRawToUpper(char in)
{
    switch (in)
    {
    case 'a':
        return 'A';
    case 'b':
        return 'B';
    case 'c':
        return 'C';
    case 'd':
        return 'D';
    case 'e':
        return 'E';
    case 'f':
        return 'F';
    case 'g':
        return 'G';
    case 'h':
        return 'H';
    case 'i':
        return 'I';
    case 'j':
        return 'J';
    case 'k':
        return 'K';
    case 'l':
        return 'L';
    case 'm':
        return 'M';
    case 'n':
        return 'N';
    case 'o':
        return 'O';
    case 'p':
        return 'P';
    case 'q':
        return 'Q';
    case 'r':
        return 'R';
    case 's':
        return 'S';
    case 't':
        return 'T';
    case 'u':
        return 'U';
    case 'v':
        return 'V';
    case 'w':
        return 'W';
    case 'x':
        return 'X';
    case 'y':
        return 'Y';
    case 'z':
        return 'Z';
    }
    return in;
}

int
HMWorkHealthCheckLibEvent::curlHostMatch(const char *hostname, const char *pattern)
{
  const char *pattern_label_end, *pattern_wildcard, *hostname_label_end;
  int wildcard_enabled;
  size_t prefixlen, suffixlen;
  pattern_wildcard = strchr(pattern, '*');

  if(pattern_wildcard == NULL)
  {
    return curlRawEqual(pattern, hostname) ? 1 : 0;
  }

  /* We require at least 2 dots in pattern to avoid too wide wildcard
     match. */
  wildcard_enabled = 1;
  pattern_label_end = strchr(pattern, '.');

  if(pattern_label_end == NULL || strchr(pattern_label_end+1, '.') == NULL ||
     pattern_wildcard > pattern_label_end ||
     curlRawNEqual(pattern, "xn--", 4))
  {
    wildcard_enabled = 0;
  }
  if(!wildcard_enabled)
  {
    return curlRawEqual(pattern, hostname) ? 1 : 0;
  }

  hostname_label_end = strchr(hostname, '.');
  if(hostname_label_end == NULL ||
     !curlRawEqual(pattern_label_end, hostname_label_end))
  {
    return 0;
  }

  /* The wildcard must match at least one character, so the left-most
     label of the hostname is at least as large as the left-most label
     of the pattern. */
  if(hostname_label_end - hostname < pattern_label_end - pattern)
  {
    return 0;
  }

  prefixlen = pattern_wildcard - pattern;
  suffixlen = pattern_label_end - (pattern_wildcard + 1);

  return curlRawNEqual(pattern, hostname, prefixlen) &&
    curlRawNEqual(pattern_wildcard + 1, hostname_label_end - suffixlen, suffixlen) ? 1 : 0;
}


