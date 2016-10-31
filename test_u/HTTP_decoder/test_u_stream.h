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

#ifndef TEST_U_STREAM_H
#define TEST_U_STREAM_H

#include <string>

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_statemachine_control.h"

#include "stream_module_io.h"

#include "test_u_common.h"
#include "test_u_common_modules.h"
#include "test_u_session_message.h"
#include "test_u_message.h"

class Test_U_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        int,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_StreamState,
                        struct Test_U_StreamConfiguration,
                        HTTP_RuntimeStatistic_t,
                        struct Stream_ModuleConfiguration,
                        struct Test_U_ModuleHandlerConfiguration,
                        struct Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Test_U_Message,
                        Test_U_SessionMessage>
{
 public:
  Test_U_Stream (const std::string&); // name
  virtual ~Test_U_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const struct Test_U_StreamConfiguration&, // configuration
                           bool = true,                              // setup pipeline ?
                           bool = true);                             // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Net_RuntimeStatistic_t&); // return value: statistic data
  virtual void report () const;

  // *TODO*: re-consider this API
  void ping ();

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        int,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_U_StreamState,
                        struct Test_U_StreamConfiguration,
                        HTTP_RuntimeStatistic_t,
                        struct Stream_ModuleConfiguration,
                        struct Test_U_ModuleHandlerConfiguration,
                        struct Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Test_U_Message,
                        Test_U_SessionMessage> inherited;

  typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                       ACE_Message_Block,
                                       Test_U_Message,
                                       Test_U_SessionMessage,
                                       struct Test_U_ModuleHandlerConfiguration,
                                       int,
                                       enum Stream_SessionMessageType,
                                       struct Test_U_StreamState,
                                       struct Test_U_StreamSessionData,
                                       Test_U_StreamSessionData_t,
                                       HTTP_RuntimeStatistic_t,
                                       ACE_INET_Addr,
                                       Test_U_ConnectionManager_t> WRITER_T;
  typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_ModuleHandlerConfiguration,
                                       ACE_Message_Block,
                                       Test_U_Message,
                                       Test_U_SessionMessage,
                                       struct Test_U_StreamSessionData,
                                       Test_U_StreamSessionData_t,
                                       ACE_INET_Addr,
                                       Test_U_ConnectionManager_t> READER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,                             // task synch type
                                Common_TimePolicy_t,                      // time policy
                                Stream_SessionId_t,                       // session id type
                                struct Test_U_StreamSessionData,          // session data type
                                enum Stream_SessionMessageType,           // session event type
                                struct Stream_ModuleConfiguration,        // module configuration type
                                struct Test_U_ModuleHandlerConfiguration, // module handler configuration type
                                Test_U_IStreamNotify_t,                   // stream notification interface type
                                READER_T,                                 // reader type
                                WRITER_T> IO_MODULE_T;                    // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream (const Test_U_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream& operator= (const Test_U_Stream&))

  // finalize stream
  // *NOTE*: need this to clean up queued modules if something goes wrong during
  //         initialize () !
  bool finalize (const Stream_Configuration&); // configuration

  // modules
  IO_MODULE_T                          IO_;
  Test_U_Module_FileWriter_Module      dump_; // <-- raw HTTP output
  Test_U_Module_Marshal_Module         marshal_;
  Test_U_Module_StatisticReport_Module statisticReport_;
  Test_U_Module_FileWriter_Module      fileWriter_; // <-- entity (HTML) output
};

#endif
