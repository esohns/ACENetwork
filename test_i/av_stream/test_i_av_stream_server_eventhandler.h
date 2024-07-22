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

#ifndef TEST_I_AVSTREAM_SERVER_EVENTHANDLER_H
#define TEST_I_AVSTREAM_SERVER_EVENTHANDLER_H

#include "ace/config-lite.h"
#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "stream_isessionnotify.h"

#include "test_i_av_stream_server_common.h"

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
class Test_I_Server_EventHandler_T
 : public Stream_ISessionDataNotify_T<SessionDataType,
                                      SessionEventType,
                                      MessageType,
                                      SessionMessageType>
{
 public:
  Test_I_Server_EventHandler_T (CallbackDataType*); // GTK state
  inline virtual ~Test_I_Server_EventHandler_T () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,      // session id
                      const SessionDataType&); // session data
  inline virtual void notify (Stream_SessionId_t, const SessionEventType&, bool = false) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,       // session id
                       const MessageType&); // (protocol) data
  virtual void notify (Stream_SessionId_t,              // session id
                       const SessionMessageType&); // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Server_EventHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Server_EventHandler_T (const Test_I_Server_EventHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Server_EventHandler_T& operator= (const Test_I_Server_EventHandler_T&))

  CallbackDataType* CBData_;
  SessionDataType*  sessionData_;
};

// include template definition
#include "test_i_av_stream_server_eventhandler.inl"

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Server_EventHandler_T<Test_I_AVStream_Server_DirectShow_StreamSessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_AVStream_Server_DirectShow_Message,
                                     Test_I_AVStream_Server_DirectShow_SessionMessage,
                                     struct Test_I_AVStream_Server_DirectShow_UI_CBData> Test_I_AVStream_Server_DirectShow_EventHandler_t;

typedef Test_I_Server_EventHandler_T<Test_I_AVStream_Server_MediaFoundation_StreamSessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_AVStream_Server_MediaFoundation_Message,
                                     Test_I_AVStream_Server_MediaFoundation_SessionMessage,
                                     struct Test_I_AVStream_Server_MediaFoundation_UI_CBData> Test_I_AVStream_Server_MediaFoundation_EventHandler_t;
#else
typedef Test_I_Server_EventHandler_T<Test_I_AVStream_Server_StreamSessionData,
                                     enum Stream_SessionMessageType,
                                     Test_I_AVStream_Server_Message,
                                     Test_I_AVStream_Server_SessionMessage,
                                     struct Test_I_AVStream_Server_UI_CBData> Test_I_AVStream_Server_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
