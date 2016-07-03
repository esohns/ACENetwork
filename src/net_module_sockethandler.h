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
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

// forward declarations
class Stream_IAllocator;

template <typename LockType,                 // connection stream state machine lock
          ////////////////////////////////
          typename SessionMessageType,
          typename ProtocolMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          ////////////////////////////////
          typename ProtocolHeaderType>
class Net_Module_SocketHandler_T
 : public Stream_HeadModuleTaskBase_T<LockType,
                                      ////
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ////
                                      ConfigurationType,
                                      ////
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      ////
                                      SessionDataType,
                                      SessionDataContainerType,
                                      ////
                                      StatisticContainerType>
{
 public:
  Net_Module_SocketHandler_T (LockType* = NULL); // lock handle (state machine)
  virtual ~Net_Module_SocketHandler_T ();

#if defined (__GNUG__) || defined (_MSC_VER)
  // *NOTE*: for some obscure reason, these base class members are not exposed
  //         (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<LockType,
                                    ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    SessionMessageType,
                                    ProtocolMessageType,
                                    ConfigurationType,
                                    StreamControlType,
                                    StreamNotificationType,
                                    StreamStateType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    StatisticContainerType>::initialize;
  using Stream_StateMachine_Control_T<LockType>::initialize;
#endif

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // info
  bool isInitialized () const;
//  unsigned int getSessionID () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ////
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ////
                                      ConfigurationType,
                                      ////
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      ////
                                      SessionDataType,
                                      SessionDataContainerType,
                                      ////
                                      StatisticContainerType> inherited;

//  ACE_UNIMPLEMENTED_FUNC (Net_Module_SocketHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Module_SocketHandler_T (const Net_Module_SocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Module_SocketHandler_T& operator= (const Net_Module_SocketHandler_T&))

  // helper methods
  bool bisectMessages (ProtocolMessageType*&); // return value: complete message (chain)
//   Net_Message* allocateMessage(const unsigned int&); // requested size
  bool putStatisticMessage (const StatisticContainerType&) const; // statistics info

  // protocol
  ProtocolMessageType* currentBuffer_;
  ProtocolMessageType* currentMessage_;
  unsigned int         currentMessageLength_;
};

//////////////////////////////////////////

template <typename LockType,
          ////////////////////////////////
          typename SessionMessageType,
          typename ProtocolMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType>
class Net_Module_UDPSocketHandler_T
 : public Stream_HeadModuleTaskBase_T<LockType,
                                      ////
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ////
                                      ConfigurationType,
                                      ////
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      ////
                                      SessionDataType,
                                      SessionDataContainerType,
                                      ////
                                      StatisticContainerType>
{
 public:
  Net_Module_UDPSocketHandler_T (LockType* = NULL); // lock handle (state machine)
  virtual ~Net_Module_UDPSocketHandler_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // info
  bool isInitialized () const;
//  unsigned int getSessionID () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ////
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ////
                                      ConfigurationType,
                                      ////
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      ////
                                      SessionDataType,
                                      SessionDataContainerType,
                                      ////
                                      StatisticContainerType> inherited;

//  ACE_UNIMPLEMENTED_FUNC (Net_Module_UDPSocketHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Module_UDPSocketHandler_T (const Net_Module_UDPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Module_UDPSocketHandler_T& operator= (const Net_Module_UDPSocketHandler_T&))

  //// helper methods
  //bool putStatisticsMessage (const StatisticContainerType&) const; // statistics info
};

// include template implementation
#include "net_module_sockethandler.inl"

#endif
