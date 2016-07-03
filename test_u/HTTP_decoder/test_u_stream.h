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

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

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
 : public Stream_Base_T<ACE_SYNCH_MUTEX,
                        //////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        //////////////////
                        int,
                        int,
                        Stream_StateMachine_ControlState,
                        Test_U_StreamState,
                        //////////////////
                        Test_U_StreamConfiguration,
                        //////////////////
                        HTTP_RuntimeStatistic_t,
                        //////////////////
                        Stream_ModuleConfiguration,
                        Test_U_ModuleHandlerConfiguration,
                        //////////////////
                        Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        Test_U_SessionMessage,
                        Test_U_Message>
{
 public:
  Test_U_Stream (const std::string&); // name
  virtual ~Test_U_Stream ();

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_U_StreamConfiguration&, // configuration
                           bool = true,                       // setup pipeline ?
                           bool = true);                      // reset session data ?

  // *TODO*: re-consider this API
  void ping ();

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Net_RuntimeStatistic_t&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                        //////////////////
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        //////////////////
                        int,
                        int,
                        Stream_StateMachine_ControlState,
                        Test_U_StreamState,
                        //////////////////
                        Test_U_StreamConfiguration,
                        //////////////////
                        HTTP_RuntimeStatistic_t,
                        //////////////////
                        Stream_ModuleConfiguration,
                        Test_U_ModuleHandlerConfiguration,
                        //////////////////
                        Test_U_StreamSessionData,   // session data
                        Test_U_StreamSessionData_t, // session data container (reference counted)
                        Test_U_SessionMessage,
                        Test_U_Message> inherited;

  typedef Stream_Module_Net_IOWriter_T<ACE_SYNCH_MUTEX,
                                       ///
                                       Test_U_SessionMessage,
                                       Test_U_Message,
                                       ///
                                       Test_U_ModuleHandlerConfiguration,
                                       ///
                                       int,
                                       int,
                                       Test_U_StreamState,
                                       ///
                                       Test_U_StreamSessionData,
                                       Test_U_StreamSessionData_t,
                                       ///
                                       HTTP_RuntimeStatistic_t,
                                       ///
                                       ACE_INET_Addr,
                                       Test_U_ConnectionManager_t> WRITER_T;
  typedef Stream_Module_Net_IOReader_T<Test_U_SessionMessage,
                                       Test_U_Message,
                                       ///
                                       Test_U_StreamConfiguration,
                                       ///
                                       Test_U_ModuleHandlerConfiguration,
                                       ///
                                       Test_U_StreamSessionData,
                                       Test_U_StreamSessionData_t,
                                       ///
                                       ACE_INET_Addr,
                                       Test_U_ConnectionManager_t> READER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,                      // task synch type
                                Common_TimePolicy_t,               // time policy
                                Stream_ModuleConfiguration,        // module configuration type
                                Test_U_ModuleHandlerConfiguration, // module handler configuration type
                                READER_T,                          // reader type
                                WRITER_T> IO_MODULE_T;             // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream (const Test_U_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_Stream& operator= (const Test_U_Stream&))

  // finalize stream
  // *NOTE*: need this to clean up queued modules if something goes wrong during
  //         initialize () !
  bool finalize (const Stream_Configuration&); // configuration

  // modules
  IO_MODULE_T                           IO_;
  Test_U_Module_FileWriter_Module       dump_; // <-- raw HTTP output
  Test_U_Module_Marshal_Module          marshal_;
  Test_U_Module_RuntimeStatistic_Module runtimeStatistic_;
  Test_U_Module_FileWriter_Module       fileWriter_; // <-- entity (HTML) output
};

#endif
