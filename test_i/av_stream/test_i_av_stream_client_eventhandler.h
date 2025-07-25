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

#ifndef TEST_I_AVSTREAM_CLIENT_EVENTHANDLER_H
#define TEST_I_AVSTREAM_CLIENT_EVENTHANDLER_H

#include <map>

#include "ace/Global_Macros.h"

#include "stream_isessionnotify.h"

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
class Test_I_AVStream_Client_EventHandler_T
 : public Stream_ISessionDataNotify_T<SessionDataType,
                                      SessionEventType,
                                      MessageType,
                                      SessionMessageType>
{
 public:
  Test_I_AVStream_Client_EventHandler_T (CallbackDataType*); // UI state
  inline virtual ~Test_I_AVStream_Client_EventHandler_T () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,      // session id
                      const SessionDataType&); // session data
  virtual void notify (Stream_SessionId_t,      // session id
                       const SessionEventType&, // event (state/status change, ...)
                       bool = false);           // expedite ?
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,  // session id
                       const MessageType&); // (protocol) data
  virtual void notify (Stream_SessionId_t,         // session id
                       const SessionMessageType&); // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_EventHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_EventHandler_T (const Test_I_AVStream_Client_EventHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_AVStream_Client_EventHandler_T& operator= (const Test_I_AVStream_Client_EventHandler_T&))

  typedef std::map<Stream_SessionId_t, SessionDataType*> SESSIONDATA_MAP_T;
  typedef typename SESSIONDATA_MAP_T::iterator SESSIONDATA_MAP_ITERATOR_T;

  CallbackDataType* CBData_;
  ACE_Thread_Mutex  lock_;
  SESSIONDATA_MAP_T sessionData_;
};

// include template definition
#include "test_i_av_stream_client_eventhandler.inl"

#endif
