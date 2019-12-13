// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_INTERNAL_HMOPENSSL_H_
#define INCLUDE_INTERNAL_HMOPENSSL_H_

//! Setup locks for openssl versions < 1.1.0
/*
      Manage locking for openssl versions 1.0.*.
      /return return 1 on success, 0 otherwise.
*/
int thread_setup(void);

//! Clean up the locks setup for openssl.
/*
      Clean up the locks established for older openssl versions(1.0.*).
      /return return 1 on success, 0 otherwise.
*/
int thread_cleanup(void);

#endif /* INCLUDE_INTERNAL_HMOPENSSL_H_ */
