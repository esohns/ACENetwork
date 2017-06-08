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

#include <map>
#include <set>

#include "ace/Global_Macros.h"
#include "ace/Stream_Modules.h"
#include "ace/Synch_Traits.h"

#include "common_icounter.h"
#include "common_istatistic.h"

#include "stream_common.h"
#include "stream_istreamcontrol.h"
#include "stream_resetcounterhandler.h"
#include "stream_statistichandler.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_synch.h"

#include "net_defines.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType> class Net_Module_Statistic_WriterTask_T;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
class Net_Module_Statistic_ReaderTask_T
 : public ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType>
{
 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;

  Net_Module_Statistic_ReaderTask_T (ISTREAM_T*); // stream handle
  virtual ~Net_Module_Statistic_ReaderTask_T ();

  virtual int put (ACE_Message_Block*,      // message
                   ACE_Time_Value* = NULL); // time

 private:
  typedef ACE_Thru_Task<ACE_SYNCH_USE,
                        TimePolicyType> inherited;
  typedef Net_Module_Statistic_WriterTask_T<ACE_SYNCH_USE,
                                            TimePolicyType,
                                            ConfigurationType,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            ProtocolCommandType,
                                            StatisticContainerType> WRITER_TASK_T;
  typedef DataMessageType Net_MessageType_t;
  typedef ProtocolCommandType Net_CommandType_t;

  ACE_UNIMPLEMENTED_FUNC (Net_Module_Statistic_ReaderTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Module_Statistic_ReaderTask_T (const Net_Module_Statistic_ReaderTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Module_Statistic_ReaderTask_T& operator= (const Net_Module_Statistic_ReaderTask_T&))
};

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ProtocolCommandType,
          typename StatisticContainerType>
class Net_Module_Statistic_WriterTask_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 Stream_UserData>
 , public Common_ICounter
 , public Common_IStatistic_T<StatisticContainerType>
{
 friend class Net_Module_Statistic_ReaderTask_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                ConfigurationType,
                                                ControlMessageType,
                                                DataMessageType,
                                                SessionMessageType,
                                                ProtocolCommandType,
                                                StatisticContainerType>;
 public:
  Net_Module_Statistic_WriterTask_T (ISTREAM_T*); // stream handle
  virtual ~Net_Module_Statistic_WriterTask_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL); // report cache usage ?

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?

  // implement this so we can print overall statistics after session completes...
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_ICounter
  virtual void reset ();

  // implement Common_IStatistic
  virtual bool collect (StatisticContainerType&); // return value: info
  // *NOTE*: this also implements locally triggered reporting !
  virtual void report () const;

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 Stream_UserData> inherited;

  // message type counters
//  typedef std::set<ProtocolCommandType> Net_Messages_t;
//  typedef typename Net_Messages_t::const_iterator Net_MessagesIterator_t;
  typedef std::map<ProtocolCommandType,
                   unsigned int> Net_MessageStatistic_t;
  typedef std::pair<ProtocolCommandType,
                    unsigned int> Net_MessageStatisticRecord_t;
  typedef typename Net_MessageStatistic_t::const_iterator Net_MessageStatisticIterator_t;

  ACE_UNIMPLEMENTED_FUNC (Net_Module_Statistic_WriterTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Module_Statistic_WriterTask_T (const Net_Module_Statistic_WriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Module_Statistic_WriterTask_T& operator= (const Net_Module_Statistic_WriterTask_T&))

  // helper method(s)
  void final_report () const;
  void fini_timers (bool = true); // cancel both timers ? (false --> cancel only localReportingHandlerID_)

  // timer stuff
  Stream_ResetCounterHandler        resetTimeoutHandler_;
  long                              resetTimeoutHandlerID_;
  Stream_StatisticHandler_Reactor_t localReportingHandler_;
  long                              localReportingHandlerID_;
  unsigned int                      reportingInterval_; // second(s) {0 --> OFF}
  bool                              printFinalReport_;

  // *GENERIC STATS*
  mutable ACE_SYNCH_MUTEX           lock_;
  unsigned int                      sessionID_;

  // *NOTE*: data messages == (myNumTotalMessages - myNumSessionMessages)
  unsigned int                      numInboundMessages_;
  unsigned int                      numOutboundMessages_;
  unsigned int                      numSessionMessages_;
  // used to compute message throughput
  unsigned int                      messageCounter_;
  // *NOTE: support asynchronous collecting/reporting of data...
  unsigned int                      lastMessagesPerSecondCount_;

  float                             numInboundBytes_;
  float                             numOutboundBytes_;
  // used to compute data throughput
  unsigned int                      byteCounter_;
  // *NOTE: support asynchronous collecting/reporting of data
  unsigned int                      lastBytesPerSecondCount_;

  // *MESSAGE TYPE STATS*
  Net_MessageStatistic_t            messageTypeStatistic_;
};

// include template definition
#include "net_module_runtimestatistic.inl"

#endif
