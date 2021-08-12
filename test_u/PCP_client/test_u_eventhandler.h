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

#ifndef TEST_U_EVENTHANDLER_H
#define TEST_U_EVENTHANDLER_H

#include <map>

#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "test_u_stream_common.h"

#include "test_u_message.h"
#include "test_u_session_message.h"

// forward declarations
#if defined (GUI_SUPPORT)
struct PCPClient_UI_CBData;
#endif // GUI_SUPPORT

class Test_U_EventHandler
 : public PCPClient_ISessionNotify_t
{
 public:
#if defined (GUI_SUPPORT)
  Test_U_EventHandler (struct PCPClient_UI_CBData*); // UI callback data handle
#else
  Test_U_EventHandler ();
#endif // GUI_SUPPORT
  inline virtual ~Test_U_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,                    // session id
                      const struct PCPClient_SessionData&); // session data
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);                // session id
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_U_Message&);          // (protocol) message
  virtual void notify (Stream_SessionId_t,              // session id
                       const Test_U_SessionMessage&);   // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler (const Test_U_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler& operator= (const Test_U_EventHandler&))

  typedef std::map<unsigned int, struct PCPClient_SessionData*> SESSION_DATA_MAP_T;
  typedef SESSION_DATA_MAP_T::iterator SESSION_DATA_MAP_ITERATOR_T;

#if defined (GUI_SUPPORT)
  struct PCPClient_UI_CBData* CBData_;
#endif // GUI_SUPPORT
  SESSION_DATA_MAP_T          sessionDataMap_;
};

#endif
