// Copyright (c) 2019 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.

#ifndef HMSTORAGEOBSERVER_H
#define HMSTORAGEOBSERVER_H

#include "HMStorage.h"

// Base class for objects that want to be notified when a probe completes.
class HMStorageObserver
{
public:
   HMStorageObserver() {};

   virtual ~HMStorageObserver() {};

   virtual bool storeHostCheckResult(HMCheckData& checkData) = 0;
};



#endif // HMSTORAGEOBSERVER_H

