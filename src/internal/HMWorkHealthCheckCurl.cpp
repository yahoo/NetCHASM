// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "curl/curl.h"

#include "HMWorkHealthCheckCurl.h"
#include "HMWork.h"
#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMLogBase.h"
#include "HMStateManager.h"

// LCOV_EXCL_START; Tested in functional testing

using namespace std;

//cert and rs load
static CURLcode testsslctxkey_function(CURL* curl, void* sslctx, void* mykey)
{
    (void)curl; /* avoid warnings */
    (void)mykey; /* avoid warnings */


    auto l =  (std::pair<std::string,std::string>*) mykey;

    BIO* kbio = NULL;
    RSA* rsa = NULL;
    int ret;

    (void)curl; /* avoid warnings */
    (void)mykey; /* avoid warnings */

    char* mypem = (char*)l->first.c_str();/*key*/

    kbio = BIO_new_mem_buf(mypem, -1);

    if(kbio == NULL)
    {
        HMLog(HM_LOG_ERROR,"[CURLCHECK]BIO_new_mem_buf failed");
    }

    /*read the key bio into an RSA object*/
    rsa = PEM_read_bio_RSAPrivateKey(kbio, NULL, 0, NULL);
    if(rsa == NULL)
    {
        HMLog(HM_LOG_ERROR,"[CURLCHECK]Failed to create key bio");
    }

    /*tell SSL to use the RSA key from memory*/
    ret = SSL_CTX_use_RSAPrivateKey((SSL_CTX*)sslctx, rsa);
    if(ret != 1)
    {
        HMLog(HM_LOG_ERROR,"[CURLCHECK]RSA  Key failed");
    }

    /*cert*/
    X509* cert = NULL;
    BIO* bio;

    char* mycert = (char*)l->second.c_str();/*cert*/

    bio=BIO_new_mem_buf(mycert, -1);
    if(bio == NULL)
    {
        HMLog(HM_LOG_ERROR,"[CURLCHECK]BIO_new_mem_buf failed");
    }

    cert = PEM_read_bio_X509(bio, NULL, 0, NULL);
    if(cert == NULL)
    {
        HMLog(HM_LOG_ERROR,"[CURLCHECK]Failed to create cert bio");
    }

    /*tell SSL to use the X509 certificate*/ 
    ret = SSL_CTX_use_certificate((SSL_CTX*)sslctx, cert);
    if(ret != 1)
    {
        HMLog(HM_LOG_ERROR,"Use certificate failed\n");
    }
 
    if(kbio)
    {
        BIO_free(kbio);
    }
    if(rsa)
    {
        RSA_free(rsa);
    }
    if(bio)
    {
        BIO_free(bio);
    }
    if(cert)
    { 
       X509_free(cert);
    }

    delete l;

    /* all set to go */
    return CURLE_OK;
}

size_t
curl_Callback(void* ptr, size_t size, size_t nmemb, HMWorkHealthCheckCurl* check)
{
    check->updateBuffer((char*)ptr,nmemb);
    return size * nmemb;
}

int
sockopt_callback(void *sockopt, curl_socket_t curlfd,
                            curlsocktype purpose)
{
  (void)purpose;
  int val = *(int *)sockopt;
  setsockopt(curlfd, IPPROTO_IP, IP_TOS, (const char *)&val, sizeof(val));
  return CURL_SOCKOPT_OK;
}

void
HMWorkHealthCheckCurl::updateBuffer(char* buf, uint32_t length)
{
    rcvdBuffer = rcvdBuffer + string(buf, length);
}

HM_WORK_STATUS
HMWorkHealthCheckCurl::healthCheck()
{
    if(m_hostCheck.getCheckType() == HM_CHECK_HTTP
            || m_hostCheck.getCheckType() == HM_CHECK_HTTPS
            || m_hostCheck.getCheckType() == HM_CHECK_HTTPS_NO_PEER_CHECK
            || m_hostCheck.getCheckType() == HM_CHECK_MTLS_HTTPS
            || m_hostCheck.getCheckType() == HM_CHECK_MTLS_HTTPS_NO_PEER_CHECK)
    {
        shared_ptr<HMState> currentState;
        m_stateManager->updateState(currentState);

        string uri;
        string url;
        string checkInfoHost;
        bool sni = false;
        CURL* curl;
        curl = curl_easy_init();
        curl_slist* slist = NULL;
        struct curl_slist *host = NULL;

        uint64_t timeout = m_timeout - HMTimeStamp::now();

        if(!curl)
        {
            HMLog(HM_LOG_ERROR, "[CURLCHECK] Failed to initialize CURL");
            return HM_WORK_COMPLETE;
        }

        uint32_t port = m_hostCheck.getPort();
        uint32_t checkInfoPort;

        // setup the HTTP parameters
        // we need to parse the checkInfo parameter
        // Check to see if we have the format: //hostname/uriCURLOPT_NOSIGNAL
        string checkInfo = m_hostCheck.getCheckInfo();
        if(checkInfo.size() > 2 && checkInfo[0] == '/' && (checkInfo)[1] == '/')
        {
            int hostLen = 0;
            size_t index;
            if((index = checkInfo.find("/", 2)) != string::npos)
            {
                //checkinfo //xxxx/yy hostLen = 4 (xxxx)
                hostLen =
                        checkInfo.substr(2, (checkInfo.find("/", 2) - 2)).length();
            }
            else
            {
                //checkinfo //xxx
                hostLen = checkInfo.substr(2).length();
            }
            if(hostLen)
            {

                string hostname;
                sni = true;
                //check for special cases checkinfo //<host>, //<host:port>, //xx
                hostname = m_hostCheck.parseCheckInfo(m_hostname, checkInfoPort, checkInfoHost);
                HMLog(HM_LOG_DEBUG3,
                        "[CURLCHECK] Curl slist %s for %s : %d at %s with checkinfo %s",
                        hostname.c_str(),
                        m_hostname.c_str(),
                        port,
                        m_ipAddress.toString().c_str(),
                        checkInfo.c_str());

                slist = curl_slist_append(slist, hostname.c_str());

                // checkinfo //xxx/yyy, uri = yyy
                //checkinfo //xxx ,uri is empty
                if(index != string::npos)
                {
                    uri = checkInfo.substr(index);
                    HMLog(HM_LOG_DEBUG3,
                            "[CURLCHECK] Curl uri %s for %s : %d at %s with checkinfo %s",
                            uri.c_str(),
                            m_hostname.c_str(),
                            port,
                            m_ipAddress.toString().c_str(),
                            checkInfo.c_str());
                }
            }
            else
            {
                // checkInfo /// uri = /,  checkinfo ///xx uri = /xx
                uri = checkInfo.substr(2); // checkInfo ///
                HMLog(HM_LOG_DEBUG3,
                        "[CURLCHECK] Curl uri %s for %s : %d at %s with checkinfo %s",
                        uri.c_str(),
                        m_hostname.c_str(),
                        port,
                        m_ipAddress.toString().c_str(),
                        checkInfo.c_str());
            }
        }
        else
        {
            uri = checkInfo;
        }

        slist = curl_slist_append(slist, "Connection: close");
        slist = curl_slist_append(slist, "User-Agent: YahooFOR/1.0");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
        curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT, currentState->getConnectionTimeout() / 1000);
        curl_easy_setopt(curl,CURLOPT_TIMEOUT, timeout / 1000);
        curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0);
        curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1);
        curl_easy_setopt(curl,CURLOPT_TCP_NODELAY,1);
        if (m_hostCheck.getSourceAddress().isSet())
        {
            curl_easy_setopt(curl, CURLOPT_INTERFACE,
                    m_hostCheck.getSourceAddress().toString().c_str());
        }

        if (m_hostCheck.getTOSValue())
        {
            uint8_t tos = m_hostCheck.getTOSValue();
            curl_easy_setopt(curl, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
            curl_easy_setopt(curl, CURLOPT_SOCKOPTDATA, &tos);
        }

        if(m_hostCheck.getCheckType() == HM_CHECK_HTTPS_NO_PEER_CHECK
            || m_hostCheck.getCheckType() == HM_CHECK_MTLS_HTTPS_NO_PEER_CHECK)
        {
            curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
            curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0);
        }

        url = ((m_hostCheck.getCheckType() == HM_CHECK_HTTP) ? "http://" : "https://");

        //if the mode is http or https with checkInfo /xxx or ///xxxx
        if((sni == false) || (m_hostCheck.getCheckType() == HM_CHECK_HTTP))
        {
            if(m_ipAddress.getType() == AF_INET6)
            {
                url = url + "[" + m_ipAddress.toString() + "]:" + to_string((uint64_t) port);
            }
            else
            {
                url = url + m_ipAddress.toString() +":" + to_string((uint64_t)port);
            }
        }
        else
        {
            //sni enabled https and https_no_peer_check
            string hostsni;
            if(checkInfoHost.empty())
            {
                //checkinfo //<host>/ or //<host:port>/
                if(m_ipAddress.getType() == AF_INET6)
                {
                    hostsni = m_hostname + ":" + to_string((uint64_t) port)
                            + ":[" + m_ipAddress.toString() + "]" + ":"
                            + to_string((uint64_t) port);
                }
                else
                {
                    hostsni = m_hostname + ":" + to_string((uint64_t) port)
                            + ":" + m_ipAddress.toString() + ":"
                            + to_string((uint64_t) port);
                }

                if(port == 443)
                {
                    // if default port then only the host name is passed in the url
                    url = url + m_hostname;
                }
                else
                {
                    url = url + m_hostname + ":" + to_string((uint64_t) port);
                }
            }
            else
            {
                //checkinfo //hostname:port/ or //hostname/
                if(m_ipAddress.getType() == AF_INET6)
                {
                    //checkinfohost:checkinfoport:[ip]:checkport
                    hostsni = checkInfoHost + ":"
                            + to_string((uint64_t) checkInfoPort) + ":["
                            + m_ipAddress.toString() + "]" + ":"
                            + to_string((uint64_t) port);
                }
                else
                {
                    //checkinfohost:checkinfoport:ip:checkport
                    hostsni = checkInfoHost + ":"
                            + to_string((uint64_t) checkInfoPort) + ":"
                            + m_ipAddress.toString() + ":"
                            + to_string((uint64_t) port);
                }

                if(checkInfoPort == 443)
                {
                    // if default port then only the host name is passed in the url
                    url = url + checkInfoHost;
                }
                else
                {
                    url = url + checkInfoHost + ":" + to_string((uint64_t) checkInfoPort);

                }
            }
            HMLog(HM_LOG_DEBUG3,
                "[CURLCHECK] curl SNI host header %s for CurlCheck for %s at %s with checkinfo %s",
                hostsni.c_str(),
                m_hostname.c_str(),
                m_ipAddress.toString().c_str(),
                checkInfo.c_str());

            host = curl_slist_append(NULL, hostsni.c_str());
            curl_easy_setopt(curl, CURLOPT_CONNECT_TO, host);
        }

        std::pair<std::string, std::string>* certkey;
        if ((m_hostCheck.getCheckType() == HM_CHECK_HTTPS
               || m_hostCheck.getCheckType() == HM_CHECK_MTLS_HTTPS)
                    && !currentState->getHealthCheckCAFile().empty())
        {
            curl_easy_setopt(curl, CURLOPT_CAINFO, currentState->getHealthCheckCAFile().c_str());
        }
        //mtls add key and cert
        if ((m_hostCheck.getCheckType() == HM_CHECK_MTLS_HTTPS
                || m_hostCheck.getCheckType() == HM_CHECK_MTLS_HTTPS_NO_PEER_CHECK)
                    && (!currentState->getHealthCheckCert().empty() 
                        && !currentState->getHealthCheckKey().empty()))
        {
            curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
            curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
            certkey = new std::pair<std::string, std::string>(currentState->getHealthCheckKey(), currentState->getHealthCheckCert());
            curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *testsslctxkey_function);
            curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA,certkey);
        }

        if(uri.empty())
        {
            url = url + "/status.html";
        }
        else
        {
            url = url + uri;
        }
        curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
        HMLog(HM_LOG_DEBUG3, "[CURLCHECK] curl fetching http url for CurlCheck %s",url.c_str());

        rcvdBuffer = "";
        curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, &curl_Callback);
        curl_easy_setopt(curl,CURLOPT_WRITEDATA, this);

        m_start = HMTimeStamp::now();
        CURLcode res = curl_easy_perform(curl);
        m_end = HMTimeStamp::now();

        long http_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);


        m_response = HM_RESPONSE_FAILED;
        m_reason = HM_REASON_NONE;

        if(res == CURLE_OK)
        {
            m_response = HM_RESPONSE_CONNECTED;
            if(http_code == 200)
            {
                double t;
                curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &t);
                m_end = m_start + ((uint64_t)(t*1000));
                m_reason = HM_REASON_SUCCESS;
            }
            else
            {
                if(http_code >= 300 && http_code < 400)
                {
                    m_reason = HM_REASON_RESPONSE_3XX;
                }
                else if(http_code == 403)
                {
                    m_reason = HM_REASON_RESPONSE_403;
                }
                else if(http_code == 404)
                {
                    m_reason = HM_REASON_RESPONSE_404;
                }
                else if(http_code >= 500 && http_code < 600)
                {
                    m_reason = HM_REASON_RESPONSE_5XX;
                }
                else
                {
                    m_reason = HM_REASON_RESPONSE_DOWN;
                }
            }
        }
        else if(res == CURLE_OPERATION_TIMEDOUT)
        {
            m_reason = HM_REASON_RESPONSE_TIMEOUT;
        }
        else if(res == CURLE_LOGIN_DENIED)
        {
            m_reason = HM_REASON_RESPONSE_403;
        }
        else if(res == CURLE_COULDNT_CONNECT)
        {
            m_reason = HM_REASON_CONNECT_FAILURE;
        }
        else
        {
            m_reason = HM_REASON_RESPONSE_FAILURE;
        }

               
        HMLog(HM_LOG_DEBUG3, "[CURLCHECK] curl fetching http url for CurlCheck %s returned reason %s ",
                 url.c_str(),
                 printReason(m_reason).c_str());


        curl_easy_cleanup(curl);
        curl_slist_free_all(host);
        curl_slist_free_all(slist);
        return HM_WORK_COMPLETE;
    }

    else if(m_hostCheck.getCheckType() == HM_CHECK_FTP
            || m_hostCheck.getCheckType() == HM_CHECK_FTPS_IMPLICIT
            || m_hostCheck.getCheckType() == HM_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK
            || m_hostCheck.getCheckType() == HM_CHECK_FTPS_EXPLICIT
            || m_hostCheck.getCheckType() == HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK)
    {

        HMLog(HM_LOG_DEBUG3, "[CURLCHECK]  Check type for CurlCheck FTP %s",
                 printCheckType(m_hostCheck.getCheckType()).c_str());

        shared_ptr<HMState> currentState;
        m_stateManager->updateState(currentState);

        string uri;
        string url;

        uint64_t timeout = (m_timeout - HMTimeStamp::now()) / 1000;

        CURL* curl = curl_easy_init();

        if(!curl)
        {
            HMLog(HM_LOG_ERROR, "[CURLCHECK] Failed to initialize CURL");
            return HM_WORK_COMPLETE;
        }

        // Check to see if we have the format: ftp://user:pwd@hostname:port/uri

        string checkInfo = m_hostCheck.getCheckInfo();
        uri = checkInfo;

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

        switch (m_hostCheck.getCheckType())
        {
        case HM_CHECK_FTP:
            url = "ftp://";
            break;
        case HM_CHECK_FTPS_IMPLICIT:
            url = "ftps://";
            break;
        case HM_CHECK_FTPS_IMPLICIT_NO_PEER_CHECK:
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
            url = "ftps://";
            break;
        case HM_CHECK_FTPS_EXPLICIT:
            url = "ftp://";
            curl_easy_setopt(curl,CURLOPT_USE_SSL,CURLUSESSL_ALL);
            curl_easy_setopt(curl,CURLOPT_FTPSSLAUTH,CURLFTPAUTH_TLS);
            break;
        case HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK:
            url = "ftp://";
            curl_easy_setopt(curl,CURLOPT_USE_SSL,CURLUSESSL_ALL);
            curl_easy_setopt(curl,CURLOPT_FTPSSLAUTH,CURLFTPAUTH_TLS);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
            break;
        default:
            break;
        }

        if (m_hostCheck.getSourceAddress().isSet())
        {
            curl_easy_setopt(curl, CURLOPT_INTERFACE,
                    m_hostCheck.getSourceAddress().toString().c_str());
        }

        if (m_hostCheck.getTOSValue())
        {
            uint8_t tos = m_hostCheck.getTOSValue();
            curl_easy_setopt(curl, CURLOPT_SOCKOPTFUNCTION, sockopt_callback);
            curl_easy_setopt(curl, CURLOPT_SOCKOPTDATA, &tos);
        }

        if(checkInfo.find("@") != string::npos)
        {
            url=url+checkInfo.substr(0,checkInfo.find("@")+1);
            uri=checkInfo.substr(checkInfo.find("@")+1);
        }

        uint32_t port = m_hostCheck.getPort();

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);


        if(m_ipAddress.getType() == AF_INET6)
        {
            url = url + "[" + m_ipAddress.toString() + "]:" + to_string((uint64_t) port);
        }
        else 
        {
            url = url + m_ipAddress.toString() + ":" + to_string((uint64_t) port);
        }

        if ((m_hostCheck.getCheckType() == HM_CHECK_FTPS_EXPLICIT
                || m_hostCheck.getCheckType() == HM_CHECK_FTPS_IMPLICIT)
                && !currentState->getHealthCheckCAFile().empty())
        {
            curl_easy_setopt(curl, CURLOPT_CAINFO, currentState->getHealthCheckCAFile().c_str());
        }

        if(uri.empty())
        {
            url = url + "/status.txt";
        }
        else 
        {
            url = url + uri;
        }

        rcvdBuffer = "";
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curl_Callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

        HMLog(HM_LOG_DEBUG3, "[CURLCHECK] curl fetching ftp url for CurlCheck %s", url.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        m_start = HMTimeStamp::now();
        CURLcode res = curl_easy_perform(curl);
        m_end = HMTimeStamp::now();

        long ftp_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ftp_code);

        m_response = HM_RESPONSE_FAILED;
        m_reason = HM_REASON_NONE;

        if(res == CURLE_OK)
        {
            m_response = HM_RESPONSE_CONNECTED;
            HMLog(HM_LOG_DEBUG3,
                    "[CURLCHECK] curl CONNECTED ftp url for CurlCheck %s",url.c_str());
            double t;
            curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &t);
            m_end = m_start + ((uint64_t) (t * 1000));
            m_reason = HM_REASON_SUCCESS;
        }
        else if(res == CURLE_OPERATION_TIMEDOUT)
        {
            m_reason = HM_REASON_RESPONSE_TIMEOUT;
        }
        else if(res == CURLE_LOGIN_DENIED)
        {
            m_reason = HM_REASON_RESPONSE_403;
        }
        else if(res == CURLE_COULDNT_CONNECT)
        {
            m_reason = HM_REASON_CONNECT_FAILURE;
        }
        else if(res == CURLE_FTP_COULDNT_RETR_FILE)
        {
            m_reason = HM_REASON_RESPONSE_404;
        }
        else 
        {
            m_reason = HM_REASON_RESPONSE_FAILURE;
        }
        HMLog(HM_LOG_DEBUG3,
                "[CURLCHECK] curl fetching ftp url for CurlCheck %s returned reason %s ",
                url.c_str(),
                printReason(m_reason).c_str());

        curl_easy_cleanup(curl);
    }
    else 
    {
        HMLog(HM_LOG_ERROR, "[CURLCHECK] Invalid Check type for CurlCheck %s",
                printCheckType(m_hostCheck.getCheckType()).c_str());
    }
    return HM_WORK_COMPLETE;
}
// LCOV_EXCL_STOP; Tested in functional testing
