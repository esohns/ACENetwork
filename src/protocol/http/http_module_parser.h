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

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_headmoduletask_base.h"
#include "stream_statistichandler.h"
#include "stream_task_base_asynch.h"
//#include "stream_task_base_synch.h"

#include "http_defines.h"
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
          typename SessionMessageType,
          ////////////////////////////////
          typename RecordType>
class HTTP_Module_Parser_T
 : public Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  Stream_SessionMessageType>
{
 public:
  HTTP_Module_Parser_T ();
  virtual ~HTTP_Module_Parser_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  typename SessionMessageType::DATA_T::DATA_T* sessionData_;

 private:
  typedef Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  Stream_SessionId_t,
                                  Stream_SessionMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_Parser_T (const HTTP_Module_Parser_T&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_Parser_T& operator= (const HTTP_Module_Parser_T&))

  // convenient types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;

  // helper methods
  DataMessageType* allocateMessage (unsigned int); // requested size

  Stream_IAllocator*                           allocator_;

  // driver
  bool                                         debugScanner_;
  bool                                         debugParser_;
  HTTP_ParserDriver<RecordType,
                    SessionMessageType>        driver_;
  DataMessageType*                             headFragment_;
  bool                                         isDriverInitialized_;
  //ACE_SYNCH_MUTEX                       lock_;
  //ACE_SYNCH_CONDITION                   condition_;

  bool                                         crunchMessages_;
  DATA_CONTAINER_T*                            dataContainer_;
  bool                                         initialized_;
};

////////////////////////////////////////////////////////////////////////////////

template <typename LockType,
          ////////////////////////////////
          ACE_SYNCH_DECL,
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
          typename StatisticContainerType,
          ////////////////////////////////
          typename RecordType>
class HTTP_Module_ParserH_T
 : public Stream_HeadModuleTaskBase_T<LockType,
                                      ACE_SYNCH_USE,
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
{
 public:
  HTTP_Module_ParserH_T ();
  virtual ~HTTP_Module_ParserH_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<LockType,
                                    ACE_SYNCH_USE,
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
  SessionDataType*  sessionData_;

 private:
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ACE_SYNCH_USE,
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

  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_ParserH_T (const HTTP_Module_ParserH_T&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_ParserH_T& operator= (const HTTP_Module_ParserH_T&))

  // convenience types
  typedef typename DataMessageType::DATA_T DATA_CONTAINER_T;
  typedef typename DataMessageType::DATA_T::DATA_T DATA_T;
  typedef HTTP_ParserDriver<RecordType,
                            SessionMessageType> PARSER_T;

  // helper methods
  //bool putStatisticMessage (const StatisticContainerType&) const;
  //DataMessageType* allocateMessage (unsigned int); // requested size

  // driver
  bool              debugScanner_;
  bool              debugParser_;
  PARSER_T          driver_;
  DataMessageType*  headFragment_;
  bool              isDriverInitialized_;

  bool              crunchMessages_;
  DATA_CONTAINER_T* dataContainer_;
};

// include template definition
#include "http_module_parser.inl"

#endif
