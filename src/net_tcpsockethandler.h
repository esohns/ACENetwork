/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#ifndef NET_TCP_SOCKETHANDLER_H
#define NET_TCP_SOCKETHANDLER_H

#include "ace/config-macros.h"
#include "ace/Global_Macros.h"
#include "ace/Reactor_Notification_Strategy.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"
#include "ace/SOCK_Stream.h"

#include "net_configuration.h"
#include "net_exports.h"
#include "net_sockethandler_base.h"

class Net_Export Net_TCPSocketHandler
 : public Net_SocketHandlerBase<Net_SocketHandlerConfiguration_t>
 , public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{
 public:
  // override some task-based members
  virtual int open (void* = NULL); // args

  // override some event handler methods
  virtual int handle_close (ACE_HANDLE,        // handle
                            ACE_Reactor_Mask); // event mask

 protected:
  Net_TCPSocketHandler ();
  virtual ~Net_TCPSocketHandler ();

  ACE_Reactor_Notification_Strategy notificationStrategy_;

 private:
  typedef Net_SocketHandlerBase<Net_SocketHandlerConfiguration_t> inherited;
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH> inherited2;

  ACE_UNIMPLEMENTED_FUNC (Net_TCPSocketHandler (const Net_TCPSocketHandler&));
  ACE_UNIMPLEMENTED_FUNC (Net_TCPSocketHandler& operator= (const Net_TCPSocketHandler&));
};

#endif
