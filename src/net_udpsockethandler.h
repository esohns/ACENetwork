
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
#include "ace/INET_Addr.h"
#include "ace/Reactor_Notification_Strategy.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"

#include "net_sock_dgram.h"
#include "net_sockethandler_base.h"

template <ACE_SYNCH_DECL,
          typename SocketType,
          typename ConfigurationType>
class Net_UDPSocketHandler_T
 : public Net_SocketHandlerBase_T<ConfigurationType>
 , public ACE_Svc_Handler<SocketType,
                          ACE_SYNCH_USE>
 , public ACE_Reactor_Notification_Strategy // *NOTE*: the 'write'-strategy
// *NOTE*: use this to modify the source/target address after initialization
 , public Common_IReset
{
  typedef Net_SocketHandlerBase_T<ConfigurationType> inherited;
  typedef ACE_Svc_Handler<SocketType,
                          ACE_SYNCH_USE> inherited2;
  typedef ACE_Reactor_Notification_Strategy inherited3;

 public:
  // convenient types
  typedef ACE_Svc_Handler<SocketType,
                          ACE_SYNCH_USE> SVC_HANDLER_T;

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
  using SVC_HANDLER_T::get_handle;
  using SVC_HANDLER_T::set_handle;

 protected:
  Net_UDPSocketHandler_T ();
  virtual ~Net_UDPSocketHandler_T ();

  inline bool registerWithReactor () { return (inherited2::open (NULL) == 0); }
  inline void deregisterFromReactor () { inherited2::shutdown (); }

  ACE_INET_Addr                     address_;
#if defined (ACE_LINUX)
  bool                              errorQueue_;
#endif // ACE_LINUX
  // *NOTE*: used for read-write connections (i.e. NET_ROLE_CLIENT) only
  ACE_HANDLE                        writeHandle_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_UDPSocketHandler_T (const Net_UDPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_UDPSocketHandler_T& operator= (const Net_UDPSocketHandler_T&))
};

/////////////////////////////////////////

// partial specialization (for connected sockets)
template <ACE_SYNCH_DECL,
          typename ConfigurationType>
class Net_UDPSocketHandler_T<ACE_SYNCH_USE,
                             Net_SOCK_CODgram,
                             ConfigurationType>
 : public Net_SocketHandlerBase_T<ConfigurationType>
 , public ACE_Svc_Handler<Net_SOCK_CODgram,
                          ACE_SYNCH_USE>
{
  typedef Net_SocketHandlerBase_T<ConfigurationType> inherited;
  typedef ACE_Svc_Handler<Net_SOCK_CODgram,
                          ACE_SYNCH_USE> inherited2;

 public:
  // convenient types
  typedef ACE_Svc_Handler<Net_SOCK_CODgram,
                          ACE_SYNCH_USE> SVC_HANDLER_T;

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
  using SVC_HANDLER_T::get_handle;
  using SVC_HANDLER_T::set_handle;

 protected:
  Net_UDPSocketHandler_T ();
  virtual ~Net_UDPSocketHandler_T ();

  inline bool registerWithReactor () { return (inherited2::open (NULL) == 0); }
  inline void deregisterFromReactor () { inherited2::shutdown (); }

  ACE_INET_Addr                     address_;
#if defined (ACE_LINUX)
  bool                              errorQueue_;
#endif // ACE_LINUX
  ACE_Reactor_Notification_Strategy notificationStrategy_;
  // *NOTE*: used for read-write connections (i.e. NET_ROLE_CLIENT) only
  ACE_HANDLE                        writeHandle_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_UDPSocketHandler_T (const Net_UDPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_UDPSocketHandler_T& operator= (const Net_UDPSocketHandler_T&))
};

// include template definition
#include "net_udpsockethandler.inl"

#endif
