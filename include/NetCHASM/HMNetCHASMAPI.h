// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMNETCHASMAPI_H_
#define HMNETCHASMAPI_H_

#include <thread>
#include <iostream>

class HMStateManager;
class HMNetCHASMAPI
{
public:
    HMNetCHASMAPI() {}
    ~HMNetCHASMAPI() {}

    //! Factory function to create a scheduler instance.
    /*!
         Function to create a scheduler instance. The instance is started as a separate thread. The instance returns a shared pointer of the instance.
         \param the log level.
         \param path to the master file.
         \return shared pointer of instance if successful else a nullptr.
     */
    static std::shared_ptr<HMNetCHASMAPI> createHealthCheckInstance(std::string loglevel, const std::string masterConfig);

    //! Function to check if the scheduler is still running.
    /*!
        Function to check if the scheduler is still running.
        \return true is still running else false if stopped.
     */
    bool isAlive();

    //! Function to stop scheduler and stop background thread running the scheduler.
    /*!
        Function to stop scheduler and stop background thread running the scheduler.
     */
    void stopHealthCheckInstance();

private:
    std::thread m_thread;
    std::shared_ptr<HMStateManager> m_monitor = nullptr;
};


#endif /* HMNETCHASMAPI_H_ */
