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

#ifndef TEST_U_CONNECTION_STREAM_H
#define TEST_U_CONNECTION_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_session_manager.h"

#include "stream_net_io_stream.h"

#include "test_u_common.h"
#include "test_u_connection_manager_common.h"

#include "test_u_message.h"
#include "test_u_session_message.h"

struct WebSocket_Client_StreamState
 : Test_U_StreamState
{
  WebSocket_Client_StreamState ()
   : Test_U_StreamState ()
   //, sessionData (NULL)
   , userData (NULL)
  {}

  //struct WebSocket_Client_SessionData* sessionData;

  struct Stream_UserData*        userData;
};

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct WebSocket_Client_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;

extern const char stream_name_string_[];

////////////////////////////////////////////////////////////////////////////////

class WebSocket_Client_ConnectionStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct WebSocket_Client_StreamState,
                                        struct WebSocket_Client_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct WebSocket_Client_ModuleHandlerConfiguration,
                                        Test_U_SessionManager_t,
                                        Stream_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        WebSocket_Client_ConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct WebSocket_Client_StreamState,
                                        struct WebSocket_Client_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct WebSocket_Client_ModuleHandlerConfiguration,
                                        Test_U_SessionManager_t,
                                        Stream_ControlMessage_t,
                                        Test_U_Message,
                                        Test_U_SessionMessage,
                                        ACE_INET_Addr,
                                        WebSocket_Client_ConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  WebSocket_Client_ConnectionStream ();
  inline virtual ~WebSocket_Client_ConnectionStream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const inherited::CONFIGURATION_T&,
                           ACE_HANDLE);                       // socket handle

 private:
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Client_ConnectionStream (const WebSocket_Client_ConnectionStream&))
  ACE_UNIMPLEMENTED_FUNC (WebSocket_Client_ConnectionStream& operator= (const WebSocket_Client_ConnectionStream&))
};

#endif
