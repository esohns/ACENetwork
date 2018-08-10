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

#ifndef TEST_I_GTK_EVENTHANDLER_H
#define TEST_I_GTK_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "common_ui_gtk_eventhandler.h"

#include "stream_isessionnotify.h"

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
class Test_I_GTK_EventHandler_T
 : public Comon_UI_GTK_EventHandler_T<CallbackDataType>
 , public Stream_ISessionDataNotify_T<SessionIdType,
                                      SessionDataType,
                                      SessionEventType,
                                      MessageType,
                                      SessionMessageType>
{
  typedef Comon_UI_GTK_EventHandler_T<CallbackDataType> inherited;

 public:
  Test_I_GTK_EventHandler_T (struct Common_UI_GTK_EventConfiguration*,
                             CallbackDataType*); // GTK state
  inline virtual ~Test_I_GTK_EventHandler_T () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (SessionIdType,           // session id
                      const SessionDataType&); // session data
  virtual void notify (SessionIdType,            // session id
                       const SessionEventType&); // event (state/status change, ...)
  virtual void end (SessionIdType); // session id
  virtual void notify (SessionIdType,       // session id
                       const MessageType&); // (protocol) data
  virtual void notify (SessionIdType,              // session id
                       const SessionMessageType&); // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_GTK_EventHandler_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_GTK_EventHandler_T (const Test_I_GTK_EventHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_GTK_EventHandler_T& operator= (const Test_I_GTK_EventHandler_T&))

  SessionDataType*  sessionData_;
};

// include template definition
#include "test_i_gtk_eventhandler.inl"

#endif
