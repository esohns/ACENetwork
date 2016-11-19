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

#ifndef BITTORRENT_MODULE_HANDLER_H
#define BITTORRENT_MODULE_HANDLER_H

#include <map>

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_control_message.h"

#include "stream_misc_messagehandler.h"

// forward declaration(s)
template <typename SessionDataType>
class BitTorrent_Message_T;
template <typename SessionDataType>
class BitTorrent_SessionMessage_T;
template <typename AddressType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          typename StreamType,
          typename StreamStatusType,
          typename StateType>
class BitTorrent_ISession_T;

template <typename AddressType,
          typename ConfigurationType, // module-
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType, // socket-
          typename ConnectionConfigurationType, // connection-
          typename ConnectionStateType,
          typename SessionStateType,
          ////////////////////////////////
          typename CBDataType> // gtk ui feedback
class BitTorrent_Module_PeerHandler_T
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         ConfigurationType,
                                         ControlMessageType,
                                         MessageType,
                                         SessionMessageType,
                                         Stream_SessionId_t,
                                         SessionDataType>
 , public Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                      SessionDataType,
                                      enum Stream_SessionMessageType,
                                      MessageType,
                                      SessionMessageType>
{
 public:
  BitTorrent_Module_PeerHandler_T ();
  virtual ~BitTorrent_Module_PeerHandler_T ();

  // implement (part of) Common_IStateMachine_T
//  virtual bool wait (const ACE_Time_Value* = NULL);

  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,      // session id
                      const SessionDataType&); // session data
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,  // session id
                       const MessageType&); // (protocol) message
  virtual void notify (Stream_SessionId_t,         // session id
                       const SessionMessageType&); // session message

 private:
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         ConfigurationType,
                                         ControlMessageType,
                                         MessageType,
                                         SessionMessageType,
                                         Stream_SessionId_t,
                                         SessionDataType> inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerHandler_T (const BitTorrent_Module_PeerHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerHandler_T& operator= (const BitTorrent_Module_PeerHandler_T&))

  // implement (part of) Common_IStateMachine_T
//  virtual void onChange (BitTorrent_RegistrationState); // new state

//  // helper methods
//  BitTorrent_Message* allocateMessage (unsigned int); // requested size
//  BitTorrent_Record* allocateMessage (BitTorrent_Record::CommandType); // command

//  // *NOTE*: "fire-and-forget" - the argument is consumed
//  void sendMessage (BitTorrent_Record*&); // command handle

  // convenient types
  typedef BitTorrent_Module_PeerHandler_T<AddressType,
                                          ConfigurationType,
                                          StatisticContainerType,
                                          ControlMessageType,
                                          MessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          StreamType,
                                          StreamStatusType,
                                          SocketConfigurationType,
                                          HandlerConfigurationType,
                                          ConnectionConfigurationType,
                                          ConnectionStateType,
                                          SessionStateType,
                                          CBDataType> OWN_TYPE_T;
  typedef BitTorrent_ISession_T<AddressType,
                                ConnectionConfigurationType,
                                ConnectionStateType,
                                StatisticContainerType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                StreamType,
                                StreamStatusType,
                                SessionStateType> ISESSION_T;

  typedef std::map<Stream_SessionId_t,
                   SessionDataType*> SESSION_DATA_T;
  typedef typename SESSION_DATA_T::iterator SESSION_DATA_ITERATOR_T;

  CBDataType*    CBData_;
  ISESSION_T*    session_;
  // *NOTE*: contains the STREAM session data of all connections involved in the
  //         BITTORRENT session
  SESSION_DATA_T sessionData_;
};

//////////////////////////////////////////

template <typename AddressType,
          typename ConfigurationType, // module-
          typename StatisticContainerType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename StreamType,
          typename StreamStatusType,
          typename SocketConfigurationType,
          typename HandlerConfigurationType, // socket-
          typename ConnectionConfigurationType, // connection-
          typename ConnectionStateType,
          typename SessionStateType,
          ////////////////////////////////
          typename CBDataType> // gtk ui feedback
class BitTorrent_Module_TrackerHandler_T
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         ConfigurationType,
                                         ControlMessageType,
                                         MessageType,
                                         SessionMessageType,
                                         Stream_SessionId_t,
                                         SessionDataType>
 , public Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                      SessionDataType,
                                      enum Stream_SessionMessageType,
                                      MessageType,
                                      SessionMessageType>
{
 public:
  BitTorrent_Module_TrackerHandler_T ();
  virtual ~BitTorrent_Module_TrackerHandler_T ();

  // implement (part of) Common_IStateMachine_T
//  virtual bool wait (const ACE_Time_Value* = NULL);

  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IClone_T
  virtual ACE_Task<ACE_MT_SYNCH,
                   Common_TimePolicy_t>* clone ();

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,      // session id
                      const SessionDataType&); // session data
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,  // session id
                       const MessageType&); // (protocol) message
  virtual void notify (Stream_SessionId_t,         // session id
                       const SessionMessageType&); // session message

 private:
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         ConfigurationType,
                                         ControlMessageType,
                                         MessageType,
                                         SessionMessageType,
                                         Stream_SessionId_t,
                                         SessionDataType> inherited;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_TrackerHandler_T (const BitTorrent_Module_TrackerHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_TrackerHandler_T& operator= (const BitTorrent_Module_TrackerHandler_T&))

  // implement (part of) Common_IStateMachine_T
//  virtual void onChange (BitTorrent_RegistrationState); // new state

//  // helper methods
//  BitTorrent_Message* allocateMessage (unsigned int); // requested size
//  BitTorrent_Record* allocateMessage (BitTorrent_Record::CommandType); // command

//  // *NOTE*: "fire-and-forget" - the argument is consumed
//  void sendMessage (BitTorrent_Record*&); // command handle

  // convenient types
  typedef BitTorrent_Module_TrackerHandler_T<AddressType,
                                             ConfigurationType,
                                             StatisticContainerType,
                                             ControlMessageType,
                                             MessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             StreamType,
                                             StreamStatusType,
                                             SocketConfigurationType,
                                             HandlerConfigurationType,
                                             ConnectionConfigurationType,
                                             ConnectionStateType,
                                             SessionStateType,
                                             CBDataType> OWN_TYPE_T;
  typedef BitTorrent_ISession_T<AddressType,
                                ConnectionConfigurationType,
                                ConnectionStateType,
                                StatisticContainerType,
                                SocketConfigurationType,
                                HandlerConfigurationType,
                                StreamType,
                                StreamStatusType,
                                SessionStateType> ISESSION_T;

  typedef std::map<Stream_SessionId_t,
                   SessionDataType*> SESSION_DATA_T;
  typedef typename SESSION_DATA_T::iterator SESSION_DATA_ITERATOR_T;

  CBDataType*    CBData_;
  ISESSION_T*    session_;
  // *NOTE*: contains the STREAM session data of all connections involved in the
  //         BITTORRENT session
  SESSION_DATA_T sessionData_;
};

// include template definition
#include "bittorrent_module_handler.inl"

#endif
