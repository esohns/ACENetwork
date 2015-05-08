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

#ifndef NET_NETLINK_SOCKETHANDLER_H
#define NET_NETLINK_SOCKETHANDLER_H

#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/Netlink_Addr.h"
#include "ace/Reactor_Notification_Strategy.h"
#include "ace/SOCK_Netlink.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_exports.h"
#include "net_sockethandler_base.h"

// *NOTE*: should be added to ACE so Svc_Handler can be parametrized with
//         ACE_SOCK_Netlink
class ACE_SOCK_NETLINK
 : public ACE_SOCK_Netlink
{
 public:
  // = Meta-type info
  typedef ACE_Netlink_Addr PEER_ADDR;
};

class Net_Export Net_NetlinkSocketHandler
 : public Net_SocketHandlerBase<Net_SocketHandlerConfiguration_t>
 , public ACE_Svc_Handler<ACE_SOCK_NETLINK, ACE_MT_SYNCH>
{
 public:
  //// override some event handler methods
  //virtual ACE_Event_Handler::Reference_Count add_reference (void);
  //// *IMPORTANT NOTE*: this API works as long as the reactor doesn't manage
  //// the lifecycle of the event handler. To avoid unforseen behavior, make sure
  //// that the event handler has been properly deregistered from the reactor
  //// before removing the last reference (delete on zero).
  //virtual ACE_Event_Handler::Reference_Count remove_reference (void);

  // override some task-based members
  virtual int open (void* = NULL); // args

  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,                        // handle
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK); // event mask

 protected:
  typedef ACE_Svc_Handler<ACE_SOCK_NETLINK, ACE_MT_SYNCH> SVC_HANDLER_T;

  Net_NetlinkSocketHandler ();
  virtual ~Net_NetlinkSocketHandler ();

  ACE_Reactor_Notification_Strategy notificationStrategy_;

 private:
  typedef Net_SocketHandlerBase<Net_SocketHandlerConfiguration_t> inherited;
  typedef ACE_Svc_Handler<ACE_SOCK_NETLINK, ACE_MT_SYNCH> inherited2;

  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkSocketHandler (const Net_NetlinkSocketHandler&));
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkSocketHandler& operator= (const Net_NetlinkSocketHandler&));
};

#endif