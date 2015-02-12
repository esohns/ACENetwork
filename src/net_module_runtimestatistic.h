/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef NET_MODULE_RUNTIMESTATISTIC_H
#define NET_MODULE_RUNTIMESTATISTIC_H

#include <set>
#include <map>

#include "ace/Global_Macros.h"
#include "ace/Stream_Modules.h"
#include "ace/Synch.h"

#include "common_istatistic.h"

#include "stream_streammodule_base.h"
#include "stream_task_base_synch.h"

#include "net_defines.h"
#include "net_icounter.h"
#include "net_resetcounterhandler.h"
#include "net_statistichandler.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;
template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticsContainerType> class Net_Module_Statistic_T;

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticsContainerType>
class Net_Module_StatisticReader_T
 : public ACE_Thru_Task<TaskSynchType,
                        TimePolicyType>
{
 public:
  Net_Module_StatisticReader_T ();
  virtual ~Net_Module_StatisticReader_T ();

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  typedef ACE_Thru_Task<TaskSynchType,
                        TimePolicyType> inherited;
  typedef Net_Module_Statistic_T<TaskSynchType,
                                 TimePolicyType,
                                 SessionMessageType,
                                 ProtocolMessageType,
                                 ProtocolCommandType,
                                 StatisticsContainerType> Net_ModuleStatisticTask_t;
  typedef ProtocolMessageType Net_Module;
  typedef ProtocolCommandType COMMAND_TYPE;

  ACE_UNIMPLEMENTED_FUNC(RPG_Net_Module_RuntimeStatisticReader_t(const RPG_Net_Module_RuntimeStatisticReader_t&));
  ACE_UNIMPLEMENTED_FUNC(RPG_Net_Module_RuntimeStatisticReader_t& operator=(const RPG_Net_Module_RuntimeStatisticReader_t&));
};

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolCommandType,
          typename StatisticsContainerType>
class RPG_Net_Module_RuntimeStatistic_t
 : public RPG_Stream_TaskBaseSynch<TimePolicyType,
                                   SessionMessageType,
                                   ProtocolMessageType>,
   public RPG_Net_ICounter,
   public RPG_Common_IStatistic<StatisticsContainerType>
{
 friend class RPG_Net_Module_RuntimeStatisticReader_t<TaskSynchType,
                                                      TimePolicyType,
                                                      SessionMessageType,
                                                      ProtocolMessageType,
                                                      ProtocolCommandType,
                                                      StatisticsContainerType>;
 public:
  RPG_Net_Module_RuntimeStatistic_t();
  virtual ~RPG_Net_Module_RuntimeStatistic_t();

  // initialization
  bool init(const unsigned int& = RPG_NET_DEFAULT_STATISTICS_REPORTING_INTERVAL, // (local) reporting interval [seconds: 0 --> OFF]
            const bool& = false,                                                 // print final report ?
            const RPG_Stream_IAllocator* = NULL);                                // report cache usage ?

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage(ProtocolMessageType*&, // data message handle
                                 bool&);            // return value: pass message downstream ?

  // implement this so we can print overall statistics after session completes...
  virtual void handleSessionMessage(SessionMessageType*&, // session message handle
                                    bool&);               // return value: pass message downstream ?

  // implement RPG_Net_ICounter
  virtual void reset();

  // implement RPG_Common_IStatistic
  virtual bool collect(StatisticsContainerType&) const; // return value: info
  // *NOTE*: this also implements locally triggered reporting !
  virtual void report() const;

 private:
  typedef RPG_Stream_TaskBaseSynch<TimePolicyType,
                                   SessionMessageType,
                                   ProtocolMessageType> inherited;

  // message type counters
  typedef std::set<ProtocolCommandType> MESSAGETYPECONTAINER_TYPE;
  typedef typename MESSAGETYPECONTAINER_TYPE::const_iterator MESSAGETYPECONTAINER_CONSTITERATOR_TYPE;
  typedef std::map<ProtocolCommandType,
                   unsigned int> MESSAGETYPE2COUNT_TYPE;
  typedef std::pair<ProtocolCommandType,
                    unsigned int> MESSAGETYPE2COUNTPAIR_TYPE;
  typedef typename MESSAGETYPE2COUNT_TYPE::const_iterator MESSAGETYPE2COUNT_CONSTITERATOR_TYPE;

  // convenience types
  typedef StatisticsContainerType STATISTICINTERFACE_TYPE;
  typedef RPG_Net_StatisticHandler_Reactor<STATISTICINTERFACE_TYPE> STATISTICHANDLER_TYPE;

  ACE_UNIMPLEMENTED_FUNC(RPG_Net_Module_RuntimeStatistic_t(const RPG_Net_Module_RuntimeStatistic_t&));
  ACE_UNIMPLEMENTED_FUNC(RPG_Net_Module_RuntimeStatistic_t& operator=(const RPG_Net_Module_RuntimeStatistic_t&));

  // helper method(s)
  void final_report() const;
  void fini_timers(const bool& = true); // cancel both timers ? (false --> cancel only myLocalReportingHandlerID)

  bool                         myIsInitialized;

  // timer stuff
//  RPG_Net_TimerQueue_t        myTimerQueue;
  RPG_Net_ResetCounterHandler  myResetTimeoutHandler;
  long                         myResetTimeoutHandlerID;
  STATISTICHANDLER_TYPE        myLocalReportingHandler;
  long                         myLocalReportingHandlerID;
  unsigned int                 myReportingInterval; // second(s) {0 --> OFF}
  bool                         myPrintFinalReport;

  // *GENERIC STATS*
  mutable ACE_Thread_Mutex     myLock;
  unsigned int                 mySessionID;

  // *NOTE*: data messages == (myNumTotalMessages - myNumSessionMessages)
  unsigned int                 myNumInboundMessages;
  unsigned int                 myNumOutboundMessages;
  unsigned int                 myNumSessionMessages;
  // used to compute message throughput...
  unsigned int                 myMessageCounter;
  // *NOTE: support asynchronous collecting/reporting of data...
  unsigned int                 myLastMessagesPerSecondCount;

  float                        myNumInboundBytes;
  float                        myNumOutboundBytes;
  // used to compute data throughput...
  unsigned int                 myByteCounter;
  // *NOTE: support asynchronous collecting/reporting of data...
  unsigned int                 myLastBytesPerSecondCount;

  // *MESSAGE TYPE STATS*
  MESSAGETYPE2COUNT_TYPE       myMessageTypeStatistics;

  // *CACHE STATS*
  const RPG_Stream_IAllocator* myAllocator;
};

// include template implementation
#include "rpg_net_module_runtimestatistic.inl"

#endif
