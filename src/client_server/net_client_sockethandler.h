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

#ifndef RPG_NET_CLIENT_SOCKETHANDLER_H
#define RPG_NET_CLIENT_SOCKETHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Event_Handler.h"

#include "net_common.h"

#include "net_tcpsockethandler.h"

#include "net_client_exports.h"

class Net_Client_Export Net_Client_SocketHandler
 : public Net_TCPSocketHandler_T<Net_SocketHandlerConfiguration>
{
 public:
  Net_Client_SocketHandler ();
  virtual ~Net_Client_SocketHandler ();

  virtual int open (void* = NULL); // args
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

 private:
  typedef Net_TCPSocketHandler_T<Net_SocketHandlerConfiguration> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_SocketHandler (const Net_Client_SocketHandler&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_SocketHandler& operator= (const Net_Client_SocketHandler&))
};

#endif
