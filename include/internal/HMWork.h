// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMWORK_H_
#define HMWORK_H_

#include <thread>
#ifdef USE_ARES
#include "ares.h"
#endif

#include "HMDataCheckParams.h"
#include "HMDataHostCheck.h"
#include "HMIPAddress.h"
#include "HMTimeStamp.h"
#include "HMAuxCache.h"

class HMStateManager;
class HMEventLoop;
class HMState;
class HMDataCheckList;
class HMDNSCache;

//! Class that stores state maintained by each worker thread
/*!
    This class holds the persistent state for the worker threads. Store items that require init and shutdown processing.
    The reload state is called when a config reload is conducted by the state manager. Thus, cleanup and initilization can be
    done on variables that change with a config reload.
    Currently, the work state includes the worker thread id, an initialized Ares DNS lookup channel, and a pointer to the state manager.
 */
class HMWorkState
{
public:
    #ifdef USE_ARES
    ares_channel m_channel;
    bool m_aresLoaded;

    HMWorkState() :
        m_channel(0),
        m_aresLoaded(false) {};

    HMWorkState(ares_channel channel, bool aresLoaded) :
        m_channel(channel),
        m_aresLoaded(aresLoaded) {};
    #endif

    //! Reload the state. Used during a reload to re-init libraries such as Ares that are init once.
    /*!
         Reload the state. Used during a reload to re-init libraries such as Ares that are init once.
         \param the current state manager.
         \param the current threadID.
     */
    void reloadState(HMStateManager* state, uint64_t threadID);

    //! Destroy the ares channel.
    /*!
         Destroy ares channel before closing daemon.
     */
    void destroyAres();
};

//! Base class for all work classes
/*!
   This class is the basis for all work classes supported by NetCHASM.
   The class provides access to the current state, event queue, and state manager. Each work type must specify the target
   hostname, ip address, and checktype. The class maintains the thread id for debugging purposes and the check reason and response codes.
   Base classes must override the following functions:
   processWork - This function is called to actually conduct the work for the worktype
   init - This function is called once when the work is created
   getWorkType - called to return the type of work we are processing.

   Additionally, all base classes should call updateState before accessing the internal statemanager and eventqueue pointers to
   make sure the state is updated after a possible configuration reload.
 */
class HMWork
{
public:
    HMWork() :
        m_hostname(""),
        m_response(HM_RESPONSE_NONE),
        m_reason(HM_REASON_NONE),
        m_ID(0),
        m_workStatus(HM_WORK_IDLE),
        m_stateManager(nullptr),
        m_eventLoop(nullptr),
        m_reschedule(true),
        m_storeResults(true),
        m_publish(true),
        m_mark(0) {};

    HMWork(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        m_hostname(hostname),
        m_ipAddress(ip),
        m_hostCheck(hostcheck),
        m_response(HM_RESPONSE_NONE),
        m_reason(HM_REASON_NONE),
        m_ID(0),
        m_workStatus(HM_WORK_IDLE),
        m_stateManager(nullptr),
        m_eventLoop(nullptr),
        m_reschedule(true),
        m_storeResults(true),
        m_publish(true),
        m_mark(0) {};

    //! Called to update the state manager and event loop in case of a reload.
    /*!
         Called to update the state manager and event loop in case of a reload.
         \param a pointer to the current state manager.
         \param a pointer to the cureent event loop.
     */
    void updateState(HMStateManager* stateManager, HMEventLoop* eventLoop);

    virtual ~HMWork() {};

    //! Called to process the work in the work order.
    /*!
         Called by the worker thread to conduct the work.
         \return The return specifies how the work should be processed. In the case of HM_WORK_COMPLETE or HM_WORK_IDLE, the work order is complete and destructed.
         HM_WORK_IN_PROGRESS moves the work order to the work map in the queue. The work must schedule a callback to have the queue move the work from the map back into the active queue to finish the processing (continuation).
     */
    virtual HM_WORK_STATUS processWork() = 0;

    //! Called to init and setup the work order.
    /*!
         Called to init and setup the work order.
         \param the work state to use for the work order.
     */
    virtual void init(HMWorkState& state) = 0;

    //! Called to get the work type.
    /*!
         Called to get the work type.
         \return the current HM_WORK_TYPE used to determine the appropriate processing task.
     */
    virtual HM_WORK_TYPE getWorkType() = 0;

    //! Called to set the mark value used to mark a socket.
    /*!
         Called to get mark value used to mark a socket.
         \param set the mark for the current work.
     */
    void setMark(int mark);

    //! Called to set the reschedule flag value .
    /*!
         Called to set if the reschedule is needed for the work.
         \param value to set the reschedule on/off .
     */
    void setReschedule(bool reschedule);

    //! Called to set the storage flag value .
    /*!
         Called to set if the storage to backend is needed for the work.
         \param value to set the storage on/off .
     */
    void setStoreResults(bool storeResults);

    //! Called to get the mark value used.
    /*!
         Called to get the mark value used for the work.
         \return the mark malue set for the work.
     */
    int getMark() const;

    //! Called to set the publish flag value .
    /*!
         Called to set if the publish is needed for the work.
         \param value to set the publish on/off .
     */
    void setPublish(bool publish);

    std::string m_hostname;
    HMIPAddress m_ipAddress;
    HMDataHostCheck m_hostCheck;
    HM_RESPONSE m_response;
    HM_REASON m_reason;
    HMTimeStamp m_start;
    HMTimeStamp m_end;
    uint64_t m_ID;
    HM_WORK_STATUS m_workStatus;
    std::map<HMDataCheckParams, HMDataCheckResult> m_hostResults;
    HMAuxInfo m_auxInfo;


protected:
    HMTimeStamp m_timeout;
    HMStateManager* m_stateManager;
    HMEventLoop* m_eventLoop;
    bool m_reschedule;
    bool m_storeResults;
    bool m_publish;
    int m_mark;
};

#endif /* HMWORKBASE_H_ */
