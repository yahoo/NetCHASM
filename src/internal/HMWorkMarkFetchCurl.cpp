// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "curl/curl.h"

#include "HMWorkMarkFetchCurl.h"
#include "HMWork.h"
#include "HMTimeStamp.h"
#include "HMConstants.h"
#include "HMLogBase.h"
#include "HMStateManager.h"

// LCOV_EXCL_START; Tested in functional testing

using namespace std;

size_t
curl_Callback(void* ptr, size_t size, size_t nmemb, HMWorkMarkFetchCurl* check)
{
    check->updateBuffer((char*)ptr,nmemb);
    return size * nmemb;
}

int
sockopt_CallBack(void *sockopt, curl_socket_t curlfd,
                            curlsocktype purpose)
{
  (void)purpose;
  int val = *(int *)sockopt;
  setsockopt(curlfd, IPPROTO_IP, IP_TOS, (const char *)&val, sizeof(val));
  return CURL_SOCKOPT_OK;
}

int
sockopt_mark_callback(void *sockopt, curl_socket_t curlfd,
                            curlsocktype purpose)
{
    (void) purpose;
    int val = *(int *) sockopt;
    if (setsockopt(curlfd, SOL_SOCKET, SO_MARK, (const char *) &val,
            sizeof(val)) < 0)
    {
        if (errno == EPERM)
        {
            HMLog(HM_LOG_ERROR,
                    "[CURLCHECK] Failed to mark the socket. Operation not permitted. Need CAP_NET_ADMIN capability to set mark");
        }
        else
        {
            HMLog(HM_LOG_ERROR, "[CURLCHECK] Failed to mark the socket. %s",
                    strerror(errno));
        }
    }
    int val_verify;
    socklen_t len;
    getsockopt(curlfd, SOL_SOCKET, SO_MARK, (char *) &val_verify, &len);
    if (val != val_verify)
    {
        HMLog(HM_LOG_ERROR, "[CURLCHECK] Failed to mark the socket");
    }
    return CURL_SOCKOPT_OK;
}

void
HMWorkMarkFetchCurl::updateBuffer(char* buf, uint32_t length)
{
    rcvdBuffer = rcvdBuffer + string(buf, length);
}

HM_WORK_STATUS
HMWorkMarkFetchCurl::healthCheck()
{
    if(m_hostCheck.getCheckType() == HM_CHECK_MARK_HTTP
            || m_hostCheck.getCheckType() == HM_CHECK_MARK_HTTPS
            || m_hostCheck.getCheckType() == HM_CHECK_MARK_HTTPS_NO_PEER_CHECK)
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
            curl_easy_setopt(curl, CURLOPT_SOCKOPTFUNCTION, sockopt_CallBack);
            curl_easy_setopt(curl, CURLOPT_SOCKOPTDATA, &tos);
        }

        int mark;
        if (m_stateManager->m_hostMark.getSocketOption(m_hostname, m_ipAddress, m_hostCheck, mark))
        {
            curl_easy_setopt(curl, CURLOPT_SOCKOPTFUNCTION, sockopt_mark_callback);
            curl_easy_setopt(curl, CURLOPT_SOCKOPTDATA, &mark);
        }
        
        if(m_hostCheck.getCheckType() == HM_CHECK_MARK_HTTPS_NO_PEER_CHECK)
        {
            curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
            curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0);
        }

        url = ((m_hostCheck.getCheckType() == HM_CHECK_MARK_HTTP) ? "http://" : "https://");

        //if the mode is http or https with checkInfo /xxx or ///xxxx
        if((sni == false) || (m_hostCheck.getCheckType() == HM_CHECK_MARK_HTTP))
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

    else 
    {
        HMLog(HM_LOG_ERROR, "[CURLCHECK] Invalid Check type for CurlCheck %s",
                printCheckType(m_hostCheck.getCheckType()).c_str());
    }
    return HM_WORK_COMPLETE;
}
// LCOV_EXCL_STOP; Tested in functional testing
