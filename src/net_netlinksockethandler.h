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

#include "ace/Global_Macros.h"
#include "ace/Reactor_Notification_Strategy.h"
#include "ace/SOCK_Netlink.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"

#include "net_configuration.h"
#include "net_sockethandler_base.h"

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
// *NOTE*: this should be added to ACE so Svc_Handler can be parametrized with
//         ACE_SOCK_Netlink
class Net_SOCK_Netlink
 : public ACE_SOCK_Netlink
{
 public:
  // = Meta-type info
  typedef Net_Netlink_Addr PEER_ADDR;
};

template <typename ConfigurationType>
class Net_NetlinkSocketHandler_T
 : public Net_SocketHandlerBase_T<ConfigurationType>
 , public ACE_Svc_Handler<Net_SOCK_Netlink, ACE_MT_SYNCH>
{
  typedef Net_SocketHandlerBase_T<ConfigurationType> inherited;
  typedef ACE_Svc_Handler<Net_SOCK_Netlink, ACE_MT_SYNCH> inherited2;

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
  typedef ACE_Svc_Handler<Net_SOCK_Netlink, ACE_MT_SYNCH> SVC_HANDLER_T;

  Net_NetlinkSocketHandler_T ();
  virtual ~Net_NetlinkSocketHandler_T ();

  ACE_Reactor_Notification_Strategy notificationStrategy_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkSocketHandler_T (const Net_NetlinkSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkSocketHandler_T& operator= (const Net_NetlinkSocketHandler_T&))
};

// include template definition
#include "net_netlinksockethandler.inl"
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT

#endif
