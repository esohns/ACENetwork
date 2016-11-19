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

#ifndef BITTORRENT_MODULE_PARSER_T_H
#define BITTORRENT_MODULE_PARSER_T_H

#include <vector>

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "stream_headmoduletask_base.h"
#include "stream_statistichandler.h"
#include "stream_task_base_asynch.h"

#include "bittorrent_defines.h"
#include "bittorrent_parser_driver.h"

// forward declaration(s)
class ACE_Message_Block;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class BitTorrent_Module_Parser_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  Stream_SessionMessageType>
 , public BitTorrent_ParserDriver_T<DataMessageType,
                                    SessionMessageType>
{
 public:
  BitTorrent_Module_Parser_T ();
  virtual ~BitTorrent_Module_Parser_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  DataMessageType* headFragment_;

 private:
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  Stream_SessionMessageType> inherited;
  typedef BitTorrent_ParserDriver_T<DataMessageType,
                                    SessionMessageType> inherited2;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_Parser_T (const BitTorrent_Module_Parser_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_Parser_T& operator= (const BitTorrent_Module_Parser_T&))

  // convenient types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  // implement (part of) BitTorrent_IParser_T
  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (struct BitTorrent_Record*&); // data record
  virtual void handshake (struct BitTorrent_PeerHandshake*&); // handshake

  // *NOTE*: strips the protocol data from the message buffer, leaving the
  //         'piece' content. This data is then available only from the message
  //         record (i.e. DATA_T)
  bool               crunch_;

  // driver
  bool               debugScanner_;
  bool               debugParser_;
};

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
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
          typename StatisticContainerType>
class BitTorrent_Module_ParserH_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType>
 , public BitTorrent_ParserDriver_T<DataMessageType,
                                    SessionMessageType>
{
 public:
  BitTorrent_Module_ParserH_T ();
  virtual ~BitTorrent_Module_ParserH_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ConfigurationType,
                                    StreamControlType,
                                    StreamNotificationType,
                                    StreamStateType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    StatisticContainerType>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: this reuses the interface to implement timer-based data collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 protected:
  DataMessageType* headFragment_;

 private:
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType> inherited;
  typedef BitTorrent_ParserDriver_T<DataMessageType,
                                    SessionMessageType> inherited2;

  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_ParserH_T (const BitTorrent_Module_ParserH_T&))
  ACE_UNIMPLEMENTED_FUNC (BitTorrent_Module_ParserH_T& operator= (const BitTorrent_Module_ParserH_T&))

  // convenient types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  // implement (part of) BitTorrent_IParser_T
  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (struct BitTorrent_Record*&); // data record
  virtual void handshake (struct BitTorrent_PeerHandshake*&); // handshake

  // *NOTE*: strips the protocol data from the message buffer, leaving the
  //         'piece' content. This data is then available only from the message
  //         record (i.e. DATA_T)
  bool               crunch_;

  // driver
  bool               debugScanner_;
  bool               debugParser_;
};

// include template definition
#include "bittorrent_module_parser.inl"

#endif