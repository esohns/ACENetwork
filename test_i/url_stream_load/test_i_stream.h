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

#ifndef TEST_I_STREAM_H
#define TEST_I_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_session_manager.h"

#include "test_i_common.h"

// forward declarations
class Test_I_Message;
class Test_I_SessionMessage;
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Test_I_URLStreamLoad_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_I_SessionManager_t;

extern const char stream_name_string_2[];
extern const char stream_name_string_2b[];

//////////////////////////////////////////

class Test_I_AudioStream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_URLStreamLoad_StreamState,
                        struct Test_I_URLStreamLoad_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                        Test_I_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_SessionMessage,
                        struct Stream_UserData>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_URLStreamLoad_StreamState,
                        struct Test_I_URLStreamLoad_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                        Test_I_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_SessionMessage,
                        struct Stream_UserData> inherited;

 public:
  Test_I_AudioStream ();
  inline virtual ~Test_I_AudioStream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AudioStream (const Test_I_AudioStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AudioStream& operator= (const Test_I_AudioStream&))
};

//////////////////////////////////////////

class Test_I_AVStream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2b,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_URLStreamLoad_StreamState,
                        struct Test_I_URLStreamLoad_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                        Test_I_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_SessionMessage,
                        struct Stream_UserData>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2b,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_URLStreamLoad_StreamState,
                        struct Test_I_URLStreamLoad_StreamConfiguration,
                        struct Stream_Statistic,
                        struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                        Test_I_SessionManager_t,
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_SessionMessage,
                        struct Stream_UserData> inherited;

 public:
  Test_I_AVStream ();
  inline virtual ~Test_I_AVStream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream (const Test_I_AVStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream& operator= (const Test_I_AVStream&))
};

#endif
