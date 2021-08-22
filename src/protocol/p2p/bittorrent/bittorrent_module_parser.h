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

#ifndef BitTorrent_Module_PeerParser_T_H
#define BitTorrent_Module_PeerParser_T_H

#include <vector>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_headmoduletask_base.h"
//#include "stream_statistichandler.h"
#include "stream_task_base_asynch.h"

#include "bittorrent_defines.h"
#include "bittorrent_bencoding_parser_driver.h"
#include "bittorrent_parser_driver.h"

// forward declaration(s)
class ACE_Message_Block;

extern const char libacenetwork_protocol_bittorrent_default_peer_parser_module_name_string[];
extern const char libacenetwork_protocol_bittorrent_default_tracker_parser_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename UserDataType>
class BitTorrent_Module_PeerParser_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  UserDataType>
 , public BitTorrent_ParserDriver
{
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  UserDataType> inherited;
  typedef BitTorrent_ParserDriver inherited2;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  BitTorrent_Module_PeerParser_T (ISTREAM_T*);                     // stream handle
#else
  BitTorrent_Module_PeerParser_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~BitTorrent_Module_PeerParser_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  virtual int put (ACE_Message_Block*,
                   ACE_Time_Value* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerParser_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerParser_T (const BitTorrent_Module_PeerParser_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerParser_T& operator= (const BitTorrent_Module_PeerParser_T&))

  // convenient types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  // implement (part of) BitTorrent_IParser_T
  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (struct BitTorrent_PeerRecord*&); // data record
  virtual void handshake (struct BitTorrent_PeerHandShake*&); // handshake

  DataMessageType* headFragment_;
};

////////////////////////////////////////////////////////////////////////////////

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          ////////////////////////////////
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          ////////////////////////////////
//          typename ConfigurationType,
//          ////////////////////////////////
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          ////////////////////////////////
//          typename SessionDataType,          // session data
//          typename SessionDataContainerType, // session message payload (reference counted)
//          ////////////////////////////////
//          typename StatisticContainerType,
//          typename TimerManagerType, // implements Common_ITimer
//          ////////////////////////////////
//          typename UserDataType>
//class BitTorrent_Module_PeerParserH_T
// : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
//                                      TimePolicyType,
//                                      ControlMessageType,
//                                      DataMessageType,
//                                      SessionMessageType,
//                                      ConfigurationType,
//                                      StreamControlType,
//                                      StreamNotificationType,
//                                      StreamStateType,
//                                      SessionDataType,
//                                      SessionDataContainerType,
//                                      StatisticContainerType,
//                                      TimerManagerType,
//                                      UserDataType>
// , public BitTorrent_ParserDriver_T<DataMessageType,
//                                    SessionMessageType>
//{
//  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
//                                      TimePolicyType,
//                                      ControlMessageType,
//                                      DataMessageType,
//                                      SessionMessageType,
//                                      ConfigurationType,
//                                      StreamControlType,
//                                      StreamNotificationType,
//                                      StreamStateType,
//                                      SessionDataType,
//                                      SessionDataContainerType,
//                                      StatisticContainerType,
//                                      TimerManagerType,
//                                      UserDataType> inherited;
//
// public:
//  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  BitTorrent_Module_PeerParserH_T (ISTREAM_T*);                     // stream handle
//#else
//  BitTorrent_Module_PeerParserH_T (typename inherited::ISTREAM_T*); // stream handle
//#endif
//  virtual ~BitTorrent_Module_PeerParserH_T ();
//
//  // *NOTE*: disambiguate Common_ISet_T::set()
//  using Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
//                                    TimePolicyType,
//                                    ControlMessageType,
//                                    DataMessageType,
//                                    SessionMessageType,
//                                    ConfigurationType,
//                                    StreamControlType,
//                                    StreamNotificationType,
//                                    StreamStateType,
//                                    SessionDataType,
//                                    SessionDataContainerType,
//                                    StatisticContainerType,
//                                    TimerManagerType,
//                                    UserDataType>::setP;
//
//  // override (part of) Stream_IModuleHandler_T
//  virtual bool initialize (const ConfigurationType&,
//                           Stream_IAllocator* = NULL);
//
//  // implement (part of) Stream_ITaskBase
//  virtual void handleDataMessage (DataMessageType*&, // data message handle
//                                  bool&);            // return value: pass message downstream ?
//  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
//                                     bool&);               // return value: pass message downstream ?
//
//  // implement Common_IStatistic
//  // *NOTE*: this reuses the interface to implement timer-based data collection
//  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
//  //virtual void report () const;
//
// protected:
//  DataMessageType* headFragment_;
//
// private:
//  typedef BitTorrent_ParserDriver_T<DataMessageType,
//                                    SessionMessageType> inherited2;
//
//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerParserH_T ())
//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerParserH_T (const BitTorrent_Module_PeerParserH_T&))
//  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_PeerParserH_T& operator= (const BitTorrent_Module_PeerParserH_T&))
//
//  // convenient types
//  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
//  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;
//
//  // implement (part of) BitTorrent_IParser_T
//  ////////////////////////////////////////
//  // callbacks
//  // *IMPORTANT NOTE*: fire-and-forget API
//  virtual void record (struct BitTorrent_PeerRecord*&); // data record
//  virtual void handshake (struct BitTorrent_PeerHandShake*&); // handshake
//
//  // *NOTE*: strips the protocol data from the message buffer, leaving the
//  //         'piece' content. This data is then available only from the message
//  //         record (i.e. DATA_T)
//  bool             crunch_;
//};

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename UserDataType>
class BitTorrent_Module_TrackerParser_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  UserDataType>
 , public BitTorrent_Bencoding_ParserDriver
{
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  enum Stream_ControlType,
                                  enum Stream_SessionMessageType,
                                  UserDataType> inherited;
  typedef BitTorrent_Bencoding_ParserDriver inherited2;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  BitTorrent_Module_TrackerParser_T (ISTREAM_T*);                     // stream handle
#else
  BitTorrent_Module_TrackerParser_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~BitTorrent_Module_TrackerParser_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  DataMessageType* headFragment_;

 private:
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_TrackerParser_T ())
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_TrackerParser_T (const BitTorrent_Module_TrackerParser_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_TrackerParser_T& operator= (const BitTorrent_Module_TrackerParser_T&))

  // convenient types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  // implement (part of) BitTorrent_IParser_T
  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (Bencoding_Dictionary_t*&); // data record

  // *NOTE*: strips the protocol data from the message buffer, leaving the
  //         'piece' content. This data is then available only from the message
  //         record (i.e. DATA_T)
  bool             crunch_;
};

// include template definition
#include "bittorrent_module_parser.inl"

#endif
