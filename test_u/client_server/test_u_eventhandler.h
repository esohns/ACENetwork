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

#ifndef ClientServer_EventHandler_H
#define ClientServer_EventHandler_H

#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "test_u_message.h"
#include "test_u_sessionmessage.h"

#include "net_client_common.h"

// forward declarations
#if defined (GUI_SUPPORT)
struct Test_U_UI_CBData;
#endif // GUI_SUPPORT

class ClientServer_EventHandler
 : public ClientServer_ISessionNotify_t
{
 public:
  ClientServer_EventHandler (
#if defined (GUI_SUPPORT)
                             struct Test_U_UI_CBData* // UI callback data
#endif // GUI_SUPPORT
                            );
  inline virtual ~ClientServer_EventHandler () {}

  // implement Common_INotify_T
  virtual void start (Stream_SessionId_t,
                      const struct ClientServer_StreamSessionData&);
  virtual void notify (Stream_SessionId_t,
                       const enum Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Test_U_SessionMessage&);

 private:
#if defined (GUI_SUPPORT)
  ACE_UNIMPLEMENTED_FUNC (ClientServer_EventHandler ())
#endif // GUI_SUPPORT
  ACE_UNIMPLEMENTED_FUNC (ClientServer_EventHandler (const ClientServer_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (ClientServer_EventHandler& operator=(const ClientServer_EventHandler&))

#if defined (GUI_SUPPORT)
  struct Test_U_UI_CBData* CBData_;
#endif // GUI_SUPPORT
};

#endif
