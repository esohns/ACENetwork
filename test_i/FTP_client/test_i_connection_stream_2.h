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

#ifndef TEST_I_CONNECTION_STREAM_2_H
#define TEST_I_CONNECTION_STREAM_2_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"

#include "stream_net_io_stream.h"

#include "test_i_common.h"

#include "test_i_connection_manager_common.h"
#include "test_i_ftp_client_common.h"
#include "test_i_message.h"

// forward declarations
class Stream_IAllocator;
class Test_I_SessionMessage;

extern const char data_connection_stream_name_string_[];

class Test_I_ConnectionStream_2
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        data_connection_stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct FTP_Client_StreamState,
                                        struct FTP_Client_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct FTP_Client_ModuleHandlerConfiguration,
                                        struct FTP_Client_SessionData, // session data
                                        FTP_Client_SessionData_t,      // session data container (reference counted)
                                        Stream_ControlMessage_t,
                                        Test_I_Message,
                                        Test_I_SessionMessage,
                                        ACE_INET_Addr,
                                        FTP_Client_ConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        data_connection_stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct FTP_Client_StreamState,
                                        struct FTP_Client_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct FTP_Client_ModuleHandlerConfiguration,
                                        struct FTP_Client_SessionData,
                                        FTP_Client_SessionData_t,
                                        Stream_ControlMessage_t,
                                        Test_I_Message,
                                        Test_I_SessionMessage,
                                        ACE_INET_Addr,
                                        FTP_Client_ConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_I_ConnectionStream_2 ();
  inline virtual ~Test_I_ConnectionStream_2 () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_ConnectionStream_2 (const Test_I_ConnectionStream_2&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_ConnectionStream_2& operator= (const Test_I_ConnectionStream_2&))
};

#endif
