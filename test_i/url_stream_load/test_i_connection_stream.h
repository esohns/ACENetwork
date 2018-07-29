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

#ifndef TEST_I_CONNECTION_STREAM_H
#define TEST_I_CONNECTION_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"

#include "stream_net_io_stream.h"

#include "test_i_connection_manager_common.h"
#include "test_i_message.h"
#include "test_i_url_stream_load_common.h"
#include "test_i_url_stream_load_stream_common.h"

// forward declarations
class Stream_IAllocator;
class Test_I_SessionMessage;

extern const char stream_name_string_[];

class Test_I_ConnectionStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_URLStreamLoad_StreamState,
                                        struct Test_I_URLStreamLoad_StreamConfiguration,
                                        HTTP_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Common_FlexParserAllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                        struct Test_I_URLStreamLoad_SessionData,
                                        Test_I_URLStreamLoad_SessionData_t,
                                        Test_I_ControlMessage_t,
                                        Test_I_Message,
                                        Test_I_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_ConnectionManager_t,
                                        struct HTTP_Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_URLStreamLoad_StreamState,
                                        struct Test_I_URLStreamLoad_StreamConfiguration,
                                        HTTP_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Common_FlexParserAllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_URLStreamLoad_ModuleHandlerConfiguration,
                                        struct Test_I_URLStreamLoad_SessionData,
                                        Test_I_URLStreamLoad_SessionData_t,
                                        Test_I_ControlMessage_t,
                                        Test_I_Message,
                                        Test_I_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_ConnectionManager_t,
                                        struct HTTP_Stream_UserData> inherited;

 public:
  Test_I_ConnectionStream ();
  inline virtual ~Test_I_ConnectionStream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&,
                           ACE_HANDLE);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_ConnectionStream (const Test_I_ConnectionStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_ConnectionStream& operator= (const Test_I_ConnectionStream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

#endif
