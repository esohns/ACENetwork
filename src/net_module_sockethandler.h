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

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
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
          typename ProtocolHeaderType,
          ////////////////////////////////
          typename UserDataType>
class Net_Module_TCPSocketHandler_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      UserDataType>
{
 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Net_Module_TCPSocketHandler_T (ISTREAM_T* = NULL, // stream handle
                                 bool = true);      // generate session messages ?
  virtual ~Net_Module_TCPSocketHandler_T ();

#if defined (__GNUG__) || defined (_MSC_VER)
  // *NOTE*: for some obscure reason, these base class members are not exposed
  //         (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                    Common_TimePolicy_t,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ConfigurationType,
                                    StreamControlType,
                                    StreamNotificationType,
                                    StreamStateType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    StatisticContainerType,
                                    UserDataType>::initialize;
  using Stream_StateMachine_Control_T<ACE_SYNCH_USE>::initialize;
#endif

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // info
  bool isInitialized () const;
//  unsigned int getSessionID () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      UserDataType> inherited;

//  ACE_UNIMPLEMENTED_FUNC (Net_Module_TCPSocketHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Module_TCPSocketHandler_T (const Net_Module_TCPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Module_TCPSocketHandler_T& operator= (const Net_Module_TCPSocketHandler_T&))

  // helper methods
  bool bisectMessages (DataMessageType*&); // return value: complete message (chain)
//   Net_Message* allocateMessage(const unsigned int&); // requested size
  bool putStatisticMessage (const StatisticContainerType&) const; // statistics info

  // protocol
  DataMessageType* currentBuffer_;
  DataMessageType* currentMessage_;
  unsigned int     currentMessageLength_;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
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
          typename UserDataType>
class Net_Module_UDPSocketHandler_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      UserDataType>
{
 public:
  Net_Module_UDPSocketHandler_T (ACE_SYNCH_MUTEX_T* = NULL, // lock handle (state machine)
                                 bool = false,              // auto-start ?
                                 bool = true);              // generate session messages ?
  virtual ~Net_Module_UDPSocketHandler_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // info
  bool isInitialized () const;
//  unsigned int getSessionID () const;

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      UserDataType> inherited;

//  ACE_UNIMPLEMENTED_FUNC (Net_Module_UDPSocketHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Module_UDPSocketHandler_T (const Net_Module_UDPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Module_UDPSocketHandler_T& operator= (const Net_Module_UDPSocketHandler_T&))

  //// helper methods
  //bool putStatisticMessage (const StatisticContainerType&) const; // statistic info
};

// include template definition
#include "net_module_sockethandler.inl"

#endif
