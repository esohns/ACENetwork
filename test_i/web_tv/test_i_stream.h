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

#include "test_i_common.h"

// forward declarations
class Test_I_Message;
class Test_I_SessionMessage;

extern const char stream_name_string_2[];

class Test_I_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_WebTV_StreamState_2,
                        struct Test_I_WebTV_StreamConfiguration_2,
                        struct Stream_Statistic,
                        struct Test_I_WebTV_ModuleHandlerConfiguration_2,
                        Test_I_WebTV_SessionData_2, // session data
                        Test_I_WebTV_SessionData_2_t,      // session data container (reference counted)
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_SessionMessage_2>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        stream_name_string_2,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_WebTV_StreamState_2,
                        struct Test_I_WebTV_StreamConfiguration_2,
                        struct Stream_Statistic,
                        struct Test_I_WebTV_ModuleHandlerConfiguration_2,
                        Test_I_WebTV_SessionData_2, // session data
                        Test_I_WebTV_SessionData_2_t,      // session data container (reference counted)
                        Stream_ControlMessage_t,
                        Test_I_Message,
                        Test_I_SessionMessage_2> inherited;

 public:
  Test_I_Stream ();
  inline virtual ~Test_I_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // i/o value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream (const Test_I_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream& operator= (const Test_I_Stream&))
};

#endif
