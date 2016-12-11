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

#ifndef HTTP_MODULE_PARSER_H
#define HTTP_MODULE_PARSER_H

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "stream_headmoduletask_base.h"
#include "stream_statistichandler.h"
#include "stream_task_base_asynch.h"
//#include "stream_task_base_synch.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_iparser.h"
#include "http_parser_driver.h"

// forward declaration(s)
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class HTTP_Module_Parser_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  Stream_SessionMessageType>
 , public HTTP_ParserDriver_T<SessionMessageType>
{
 public:
  HTTP_Module_Parser_T ();
  virtual ~HTTP_Module_Parser_T ();

  // override (part of) Stream_IModuleHandler_T
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
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  Stream_SessionMessageType> inherited;
  typedef HTTP_ParserDriver_T<SessionMessageType> inherited2;

  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_Parser_T (const HTTP_Module_Parser_T&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_Parser_T& operator= (const HTTP_Module_Parser_T&))

  // implement (part of) HTTP_IParser
  virtual void record (struct HTTP_Record*&); // data record
  inline virtual void encoding (const std::string&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  // convenient types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
//  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  // *NOTE*: 'strips' the http protocol data from the message buffer, leaving
  //         the 'document entity' content. The protocol data is then available
  //         only from the HTTP_Record (i.e. DATA_T)
  bool             crunch_;

//  // driver
//  bool             debugScanner_;
//  bool             debugParser_;
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
class HTTP_Module_ParserH_T
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
 , public HTTP_ParserDriver_T<SessionMessageType>
{
 public:
  HTTP_Module_ParserH_T ();
  virtual ~HTTP_Module_ParserH_T ();

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
  typedef HTTP_ParserDriver_T<SessionMessageType> inherited2;

  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_ParserH_T (const HTTP_Module_ParserH_T&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_ParserH_T& operator= (const HTTP_Module_ParserH_T&))

  // implement (part of) HTTP_IParser
  virtual void record (struct HTTP_Record*&); // data record
  inline virtual void encoding (const std::string&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  // convenience types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
//  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  // *NOTE*: 'strips' the http protocol data from the message buffer, leaving
  //         the 'document entity' content. The protocol data is then available
  //         only from the HTTP_Record (i.e. DATA_T)
  bool             crunch_;

//  // driver
//  bool             debugScanner_;
//  bool             debugParser_;
};

// include template definition
#include "http_module_parser.inl"

#endif
