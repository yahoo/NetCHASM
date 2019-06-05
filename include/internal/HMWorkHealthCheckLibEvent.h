// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HMWORKHEALTHCHECKMULTILIBEVENT_H_
#define INCLUDE_HMWORKHEALTHCHECKMULTILIBEVENT_H_

#include <event2/http.h>
#include <openssl/ssl.h>

#include "HMWorkHealthCheck.h"

//! Enum outlining the possible results from the HTTPS cert check.
enum HostnameValidationResult
{
    Error,
    MatchFound,
    MatchNotFound,
    NoSANPresent,
    MalformedCertificate,
};

//! Implementation of an HTTP/HTTPS check using libevent. Note* Currently experimental.
class HMWorkHealthCheckLibEvent : public HMWorkHealthCheck
{
public:
        HMWorkHealthCheckLibEvent(const std::string& hostname,
                const HMIPAddress& ip,
                const HMDataHostCheck& hostcheck,
                bool ignoreCert) :
        HMWorkHealthCheck(hostname, ip, hostcheck),
                m_ssl(nullptr),
                m_bev(nullptr),
                m_evcon(nullptr),
                m_req(nullptr),
                m_headers(nullptr),
                m_verifyPeer(true),
                m_verifyHost(ignoreCert){};


    //! Main function to actually conduct the health check.
    /*!
        Main function to actually conduct the health check.
        Should set the start, end and response codes.
        \return the appropriate HM_WORK_STATUS (IDLE or COMPLETE for done or IN PROGRESS if it needs a continuation)
    */
    HM_WORK_STATUS healthCheck();

    //! Called to init and setup the work order.
    /*!
         Called to init and setup the work order.
         \param the work state to use for the work order.
     */
    void init(HMWorkState& state) { (void)state; };

    //! LibEvent callback when the request is done.
    /*!
         LibEvent callback when the request is done. Update the request parameters and move the work to the work queue.
         \param the lib event http request structure.
         \param the pointer to the work to update.
     */
    static void requestDone(struct evhttp_request *req, void* arg);

    //! LibEvent callback to call if the request has an error.
    /*!
         LibEvent callback to call if the request has an error. Update the request parameters and move the work to the work queue.
         \param the LibEvent error struct.
         \param the pointer to the work to update.
     */
    static void errorCallback(evhttp_request_error error, void* arg);

    //! LibEvent callback to call when the connection is established.
    /*!
         LibEvent callback to call when the connection is established. Updates the timing parameters of the work.
         \param the buffer event that happened to trigger the call.
         \param the events that have happened up to this call.
         \param a pointer to the work to update.
     */
    static void connectCallback(struct bufferevent* bev, short events, void* arg);

    //! LibEvent callback to call to verify the server cert.
    /*!
         LibEvent callback to call to verify the server cert.
         \param the x509 cert store.
         \param a pointer to the work to update.
         \return 1 if the cert is valid 0 if it failed to verify.
     */
    static int certifyCallback(X509_STORE_CTX *x509_ctx, void *arg);


    //! Required functions to match and verify the certs.
    static int curlCertHostCheck(const char *match_pattern, const char *hostname);
    static int curlRawEqual(const char *first, const char *second);
    static int curlHostMatch(const char *hostname, const char *pattern);
    static int curlRawNEqual(const char *first, const char *second, size_t max);
    static inline char curlRawToUpper(char in);

private:

    std::string rcvdBuffer;
    SSL* m_ssl;
    struct bufferevent* m_bev;
    struct evhttp_connection* m_evcon;
    struct evhttp_request* m_req;
    struct evkeyvalq* m_headers;

    bool m_verifyPeer;
    bool m_verifyHost;

};

#endif /* INCLUDE_HMWORKHEALTHCHECKMULTILIBEVENT_H_ */
