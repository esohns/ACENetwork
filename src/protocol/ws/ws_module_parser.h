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

#ifndef WEBSOCKET_MODULE_PARSER_H
#define WEBSOCKET_MODULE_PARSER_H

#include <utility>
#include <vector>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_headmoduletask_base.h"
#include "stream_task_base_synch.h"

#include "ws_common.h"
#include "ws_defines.h"

extern const char libacenetwork_protocol_default_ws_parser_module_name_string[];

// forward declaration(s)
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType>
class WebSocket_Module_Parser_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public ParserDriverType
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef ParserDriverType inherited2;

 public:
  WebSocket_Module_Parser_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~WebSocket_Module_Parser_T ();

  // override some baseclass methods
  virtual int put (ACE_Message_Block*,
                   ACE_Time_Value* = NULL);

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  DataMessageType*                    headFragment_;

 private:
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Module_Parser_T ())
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Module_Parser_T (const WebSocket_Module_Parser_T&))
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Module_Parser_T& operator= (const WebSocket_Module_Parser_T&))

  // enqueue MB_STOP --> stop worker thread(s)
  virtual void stop (bool = true,   // wait for completion ?
                     bool = false); // high priority ? (i.e. do not wait for queued messages)

  // override (part of) ACE_Task_Base
  virtual int svc (void);

  // helper methods
  void dispatch (ACE_Message_Block*);
  size_t getPayloadLength ();

  // override (part of) Common_IScannerBase
  inline virtual ACE_Message_Block* head () { return headFragment_; }
  inline virtual void head (ACE_Message_Block* newHead_in) { ACE_ASSERT (newHead_in && !headFragment_); headFragment_ = static_cast<DataMessageType*> (newHead_in); }

  // implement (part of) WebSocket_(|Reflex|ANTLR)_IParser
  virtual void record (struct WebSocket_Record*&); // data record
  inline virtual ACE_UINT64 dataSize () { return dataSize_; }
  inline virtual ACE_UINT64 dataBytesToSkip () { return dataBytesToSkip_; }

  inline virtual void dataSize (ACE_UINT64 size_in) { dataSize_ = size_in; dataBytesToSkip_ = dataSize_; }
  inline virtual void dataBytesSkipped (ACE_UINT64 bytesSkipped_in) { dataBytesToSkip_ -= bytesSkipped_in; }

  // convenient types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  ACE_UINT64                          dataBytesToSkip_;
  ACE_UINT64                          dataSize_;
  typename inherited::MESSAGE_QUEUE_T queue_; // parser-
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
          typename StatisticContainerType,
          ////////////////////////////////
          typename SessionManagerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType,
          typename ParserDriverType>
class WebSocket_Module_ParserH_T
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      UserDataType>
 , public ParserDriverType
{
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      UserDataType> inherited;
  typedef ParserDriverType inherited2;

 public:
  WebSocket_Module_ParserH_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~WebSocket_Module_ParserH_T ();

  // *NOTE*: disambiguate Common_ISet_T::set()
  using Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ConfigurationType,
                                    StreamControlType,
                                    StreamNotificationType,
                                    StreamStateType,
                                    StatisticContainerType,
                                    SessionManagerType,
                                    TimerManagerType,
                                    UserDataType>::setP;

  // override some baseclass methods
  virtual int put (ACE_Message_Block*,
                   ACE_Time_Value* = NULL);

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

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
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Module_ParserH_T ())
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Module_ParserH_T (const WebSocket_Module_ParserH_T&))
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Module_ParserH_T& operator= (const WebSocket_Module_ParserH_T&))

  // override (part of) Common_IScannerBase
  inline virtual ACE_Message_Block* head () { return headFragment_; }
  inline virtual void head (ACE_Message_Block* newHead_in) { ACE_ASSERT (newHead_in && !headFragment_); headFragment_ = static_cast<DataMessageType*> (newHead_in); }

  // implement (part of) WebSocket_IParser
  virtual void record (struct WebSocket_Record*&); // data record
  inline virtual ACE_UINT64 dataSize () { return dataSize_; }
  inline virtual ACE_UINT64 dataBytesToSkip () { return dataBytesToSkip_; }

  inline virtual void dataSize (ACE_UINT64 size_in) { dataSize_ = size_in; dataBytesToSkip_ = dataSize_; }
  inline virtual void dataBytesSkipped (ACE_UINT64 bytesSkipped_in) { dataBytesToSkip_ -= bytesSkipped_in; }

  // convenience types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  ACE_UINT64       dataBytesToSkip_;
  ACE_UINT64       dataSize_;
};

// include template definition
#include "ws_module_parser.inl"

#endif
