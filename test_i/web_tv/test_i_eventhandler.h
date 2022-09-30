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

#ifndef TEST_I_EVENTHANDLER_H
#define TEST_I_EVENTHANDLER_H

#include <map>

#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "test_i_stream_common.h"

#include "test_i_message.h"
#include "test_i_session_message.h"

class Test_I_EventHandler // master m3u
 : public Test_I_ISessionNotify_t
{
 public:
  Test_I_EventHandler (
#if defined (GUI_SUPPORT)
                       struct Test_I_WebTV_UI_CBData*); // UI state
#else
                      );
#endif // GUI_SUPPORT
  inline virtual ~Test_I_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,                      // session id
                      const struct Test_I_WebTV_SessionData&); // session data
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);                // session id
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_I_Message&);          // (protocol) message
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_I_SessionMessage&);   // session message

 private:
#if defined (GUI_SUPPORT)
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler ())
#endif // GUI_SUPPORT
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler (const Test_I_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler& operator= (const Test_I_EventHandler&))

  typedef std::map<unsigned int, struct Test_I_WebTV_SessionData*> SESSION_DATA_MAP_T;
  typedef SESSION_DATA_MAP_T::iterator SESSION_DATA_MAP_ITERATOR_T;

#if defined (GUI_SUPPORT)
  struct Test_I_WebTV_UI_CBData* CBData_;
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_T             sessionDataMap_;
};

//////////////////////////////////////////

class Test_I_EventHandler_2 // playlist m3u
 : public Test_I_ISessionNotify_t
{
 public:
  Test_I_EventHandler_2 (
#if defined (GUI_SUPPORT)
                         struct Test_I_WebTV_UI_CBData*); // UI state
#else
                        );
#endif // GUI_SUPPORT
  inline virtual ~Test_I_EventHandler_2 () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,               // session id
                      const Test_I_WebTV_SessionData&); // session data
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,     // session id
                       const Test_I_Message&); // (protocol) message
  virtual void notify (Stream_SessionId_t,            // session id
                       const Test_I_SessionMessage&); // session message

 private:
#if defined (GUI_SUPPORT)
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_2 ())
#endif // GUI_SUPPORT
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_2 (const Test_I_EventHandler_2&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_2& operator= (const Test_I_EventHandler_2&))

  typedef std::map<unsigned int, Test_I_WebTV_SessionData*> SESSION_DATA_MAP_T;
  typedef SESSION_DATA_MAP_T::iterator SESSION_DATA_MAP_ITERATOR_T;

#if defined (GUI_SUPPORT)
  struct Test_I_WebTV_UI_CBData* CBData_;
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_T             sessionDataMap_;
};

//////////////////////////////////////////

class Test_I_EventHandler_3 // content
 : public Test_I_ISessionNotify_3_t
{
 public:
  Test_I_EventHandler_3 (
#if defined (GUI_SUPPORT)
                         struct Test_I_WebTV_UI_CBData*); // UI state
#else
                        );
#endif // GUI_SUPPORT
  inline virtual ~Test_I_EventHandler_3 () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,                 // session id
                      const Test_I_WebTV_SessionData_3&); // session data
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,     // session id
                       const Test_I_Message&); // (protocol) message
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_I_SessionMessage_3&); // session message

 private:
#if defined (GUI_SUPPORT)
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_3 ())
#endif // GUI_SUPPORT
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_3 (const Test_I_EventHandler_3&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_EventHandler_3& operator= (const Test_I_EventHandler_3&))

  typedef std::map<unsigned int, Test_I_WebTV_SessionData_3*> SESSION_DATA_MAP_T;
  typedef SESSION_DATA_MAP_T::iterator SESSION_DATA_MAP_ITERATOR_T;

#if defined (GUI_SUPPORT)
  struct Test_I_WebTV_UI_CBData* CBData_;
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_T             sessionDataMap_;
};

#endif
