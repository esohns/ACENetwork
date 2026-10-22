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

#ifndef IRC_CLIENT_EVENTHANDLER_H
#define IRC_CLIENT_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "IRC_client_gui_common.h"
#include "IRC_client_stream_common.h"
#include "IRC_client_sessionmessage.h"

class IRC_Client_EventHandler
 : public IRC_Client_ISessionNotify_t
{
 public:
  IRC_Client_EventHandler (struct IRC_Client_UI_ConnectionCBData*); // UI state
  inline virtual ~IRC_Client_EventHandler () {}

  // implement Stream_ISessionDataNotify_T
  virtual void start (Stream_SessionId_t,                     // session id
                      const struct IRC_Client_SessionData&);  // session data
  virtual void notify (Stream_SessionId_t,                    // session id
                       const enum Stream_SessionMessageType&, // message type
                       bool = false);                         // expedited ?
  virtual void end (Stream_SessionId_t); // session id
  virtual void notify (Stream_SessionId_t,  // session id
                       const IRC_Message&); // (protocol) message
  virtual void notify (Stream_SessionId_t,                // session id
                       const IRC_Client_SessionMessage&); // session message

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_EventHandler (const IRC_Client_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_EventHandler& operator= (const IRC_Client_EventHandler&))

  struct IRC_Client_UI_ConnectionCBData* CBData_;
};

#endif
