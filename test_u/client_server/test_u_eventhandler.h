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

#include <ace/Global_Macros.h>

#include "stream_common.h"

#include "test_u_message.h"
#include "test_u_sessionmessage.h"

#include "net_client_common.h"

class Test_U_EventHandler
 : public Test_U_ISessionNotify_t
{
 public:
  Test_U_EventHandler (Test_U_GTK_CBData*); // GTK state
  virtual ~Test_U_EventHandler ();

  // implement Common_INotify_T
  virtual void start (Stream_SessionId_t,
                      const Test_U_StreamSessionData&);
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler (const Test_U_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler& operator=(const Test_U_EventHandler&))

  Test_U_GTK_CBData* CBData_;
};

#endif
