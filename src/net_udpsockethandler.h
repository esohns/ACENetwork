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

#ifndef NET_UDP_SOCKETHANDLER_T_H
#define NET_UDP_SOCKETHANDLER_T_H

#include "ace/Global_Macros.h"
#include "ace/Reactor_Notification_Strategy.h"
//#include "ace/SOCK_CODgram.h"
#include "ace/SOCK_Dgram.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"

#include "net_sockethandler_base.h"

/////////////////////////////////////////

class Net_SOCK_Dgram
 : public ACE_SOCK_Dgram
{
 public:
  int get_remote_addr (ACE_Addr&) const;

 private:
  typedef ACE_SOCK_Dgram inherited;
};
//typedef ACE_SOCK_CODgram Net_SOCK_Dgram;

/////////////////////////////////////////

template <typename SocketType,
          typename ConfigurationType>
class Net_UDPSocketHandler_T
 : public Net_SocketHandlerBase_T<ConfigurationType>
 , public ACE_Svc_Handler<SocketType, ACE_MT_SYNCH>
{
 public:
  //// override some event handler methods
  //virtual ACE_Event_Handler::Reference_Count add_reference (void);
  //// *IMPORTANT NOTE*: this API works as long as the reactor doesn't manage
  //// the lifecycle of the event handler. To avoid unforseen behavior, make sure
  //// that the event handler has been properly deregistered from the reactor
  //// before removing the last reference (delete on zero).
  //virtual ACE_Event_Handler::Reference_Count remove_reference (void);

  virtual int open (void* = NULL); // args

  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,                        // handle
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK); // event mask

  // resolve ambiguity between ACE_Event_Handler and ACE_Svc_Handler
  using ACE_Svc_Handler<SocketType, ACE_MT_SYNCH>::get_handle;
  using ACE_Svc_Handler<SocketType, ACE_MT_SYNCH>::set_handle;

 protected:
  typedef ACE_Svc_Handler<SocketType, ACE_MT_SYNCH> SVC_HANDLER_T;

  Net_UDPSocketHandler_T ();
  virtual ~Net_UDPSocketHandler_T ();

  ACE_Reactor_Notification_Strategy notificationStrategy_;

 private:
  typedef Net_SocketHandlerBase_T<ConfigurationType> inherited;
  typedef ACE_Svc_Handler<SocketType, ACE_MT_SYNCH> inherited2;

  ACE_UNIMPLEMENTED_FUNC (Net_UDPSocketHandler_T (const Net_UDPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_UDPSocketHandler_T& operator= (const Net_UDPSocketHandler_T&))
};

// include template implementation
#include "net_udpsockethandler.inl"

#endif
