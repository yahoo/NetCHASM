// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef INCLUDE_HMUTILITYSPINLOCK_H_
#define INCLUDE_HMUTILITYSPINLOCK_H_

#include <atomic>

//! A simple implementation of a spin lock.
class HMUtilitySpinLock
{
    std::atomic_flag locked = ATOMIC_FLAG_INIT;

    //! Lock the spin lock.
    public: void lock()
    {
        while (locked.test_and_set(std::memory_order_acquire))
        { ; }
    }
    //! Unlock the spin lock.
    void unlock()
    {
        locked.clear(std::memory_order_release);
    }
};

//! Simple template to automatically lock and unlock within a standard lock guard.
template<class Lock> class HMLockGuard
{
  public:
    HMLockGuard(Lock &lock):
    m_lock(lock)
    {
      m_lock.lock();
    }

    ~HMLockGuard()
    {
      m_lock.unlock();
    }

  private:
    Lock &m_lock;
};

#endif /* INCLUDE_HMUTILITYSPINLOCK_H_ */
