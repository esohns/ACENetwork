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

#ifndef NET_MODULE_SOCKETHANDLER_H
#define NET_MODULE_SOCKETHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"
#include "ace/Synch_Traits.h"

#include "common.h"
#include "common_istatistic.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"
#include "stream_iallocator.h"

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ProtocolHeaderType>
class Net_Module_SocketHandler_T
 : public Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      SessionMessageType,
                                      ProtocolMessageType>
 , public Common_IStatistic_T<Stream_Statistic_t>
{
 public:
  Net_Module_SocketHandler_T ();
  virtual ~Net_Module_SocketHandler_T ();

  // initialization
  // *TODO*: implement generic initialization (override Stream_TaskBase_T::initialize)
  bool initialize (const StreamStateType&, // state
                   Stream_IAllocator*,     // message allocator
                   bool = false,           // active object ?
                   unsigned int = 0);      // statistics collecting interval (second(s))
                                           // 0 --> DON'T collect statistics

  // user interface
  // info
  bool isInitialized () const;
//  unsigned int getSessionID () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistics collection
  virtual bool collect (Stream_Statistic_t&) const; // return value: (currently unused !)
  virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      SessionMessageType,
                                      ProtocolMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Module_SocketHandler_T (const Net_Module_SocketHandler_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_Module_SocketHandler_T& operator= (const Net_Module_SocketHandler_T&));

  // helper methods
  bool bisectMessages (ProtocolMessageType*&); // return value: complete message (chain)
//   Net_Message* allocateMessage(const unsigned int&); // requested size
  bool putStatisticsMessage (const Stream_Statistic_t&, // statistics info
                             const ACE_Time_Value&) const;  // statistics generation time

  bool                              isInitialized_;

  // timer stuff
  unsigned int                      statCollectionInterval_; // seconds
  Stream_StatisticHandler_Reactor_t statCollectHandler_;
  long                              statCollectHandlerID_;

  // protocol stuff
  unsigned int                      currentMessageLength_;
  ProtocolMessageType*              currentMessage_;
  ProtocolMessageType*              currentBuffer_;
};

/////////////////////////////////////////

template <typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
class Net_Module_UDPSocketHandler_T
 : public Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      SessionMessageType,
                                      ProtocolMessageType>
 , public Common_IStatistic_T<Stream_Statistic_t>
{
 public:
  Net_Module_UDPSocketHandler_T ();
  virtual ~Net_Module_UDPSocketHandler_T ();

  // initialization
  // *TODO*: implement generic initialization (override Stream_TaskBase_T::initialize)
  bool initialize (const StreamStateType&, // state
                   Stream_IAllocator*,     // message allocator
                   bool = false,           // active object ?
                   unsigned int = 0);      // statistics collecting interval (second(s))
                                           // 0 --> DON'T collect statistics

  // user interface
  // info
  bool isInitialized () const;
//  unsigned int getSessionID () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistics collection
  virtual bool collect (Stream_Statistic_t&) const; // return value: (currently unused !)
  virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      SessionMessageType,
                                      ProtocolMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Module_UDPSocketHandler_T (const Net_Module_UDPSocketHandler_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_Module_UDPSocketHandler_T& operator= (const Net_Module_UDPSocketHandler_T&));

  // helper methods
  bool putStatisticsMessage (const Stream_Statistic_t&, // statistics info
                             const ACE_Time_Value&) const;  // statistics generation time

  bool                              isInitialized_;

  // timer stuff
  unsigned int                      statCollectionInterval_; // seconds
  Stream_StatisticHandler_Reactor_t statCollectHandler_;
  long                              statCollectHandlerID_;
};

// include template implementation
#include "net_module_sockethandler.inl"

#endif
