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

#ifndef NET_EVENTHANDLER_H
#define NET_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "common_inotify.h"

#include "test_u_message.h"
#include "test_u_sessionmessage.h"
#include "test_u_stream_common.h"

// forward declaration(s)
struct Net_GTK_CBData;
struct Net_StreamSessionData;

class Net_EventHandler
 : public Net_ISessionNotify_t
{
 public:
  Net_EventHandler (Net_GTK_CBData*); // GTK state
  virtual ~Net_EventHandler ();

  // implement Common_INotify_T
  virtual void start (Stream_SessionId_t,
                      const Net_StreamSessionData&);
  virtual void notify (Stream_SessionId_t,
                       const Stream_SessionMessageType&);
  virtual void end (Stream_SessionId_t);
  virtual void notify (Stream_SessionId_t,
                       const Net_Message&);
  virtual void notify (Stream_SessionId_t,
                       const Net_SessionMessage&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Net_EventHandler (const Net_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Net_EventHandler& operator=(const Net_EventHandler&))

  Net_GTK_CBData* CBData_;
};

#endif
