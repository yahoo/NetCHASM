// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <thread>
#include <mutex>
#include <openssl/err.h>
#include "HMOpenSSL.h"
/*
 * Used code from
 * https://curl.haxx.se/libcurl/c/opensslthreadlock.html
 */

/* This array will store all of the mutexes available to OpenSSL. */
static std::mutex *mutex_buf = NULL;

static void locking_function(int mode, int n, const char *file, int line)
{
#if OPENSSL_VERSION_NUMBER < 0x010100000 /* 1.1.0 */
    (void) file;
    (void) line;
    if (mode & CRYPTO_LOCK)
        mutex_buf[n].lock();
    else
        mutex_buf[n].unlock();
#endif
}

static unsigned long id_function(void)
{
#if OPENSSL_VERSION_NUMBER < 0x010100000 /* 1.1.0 */
    return ((unsigned long) pthread_self());
#endif
}

int thread_setup(void)
{
#if OPENSSL_VERSION_NUMBER < 0x010100000 /* 1.1.0 */
    mutex_buf = new std::mutex[CRYPTO_num_locks()];
    if (!mutex_buf)
    {
        return 0;
    }
    CRYPTO_set_id_callback(id_function);
    CRYPTO_set_locking_callback(locking_function);
#endif
    return 1;
}

int thread_cleanup(void)
{
#if OPENSSL_VERSION_NUMBER < 0x010100000 /* 1.1.0 */
    if (!mutex_buf)
    {
        return 0;
    }
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    delete[] mutex_buf;
    mutex_buf = NULL;
#endif
    return 1;
}

