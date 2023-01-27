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

#ifndef NET_TCP_SOCKETHANDLER_T_H
#define NET_TCP_SOCKETHANDLER_T_H

#include "ace/config-macros.h"
#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/Reactor_Notification_Strategy.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"

#if defined (SSL_SUPPORT)
#include "ace/SSL/SSL_SOCK_Stream.h"
#endif // SSL_SUPPORT

#include "net_sockethandler_base.h"

template <ACE_SYNCH_DECL,
          typename StreamType,
          typename ConfigurationType>
class Net_TCPSocketHandler_T
 : public Net_SocketHandlerBase_T<ConfigurationType>
 , public ACE_Svc_Handler<StreamType,
                          ACE_SYNCH_USE>
 , public ACE_Reactor_Notification_Strategy // *NOTE*: the 'write'-strategy
{
  typedef Net_SocketHandlerBase_T<ConfigurationType> inherited;
  typedef ACE_Svc_Handler<StreamType,
                          ACE_SYNCH_USE> inherited2;
  typedef ACE_Reactor_Notification_Strategy inherited3;

 public:
  // convenient types
  typedef ACE_Svc_Handler<StreamType,
                          ACE_SYNCH_USE> SVC_HANDLER_T;

  // override some task-based members
  virtual int open (void* = NULL); // args

  // override some event handler methods
  virtual int handle_close (ACE_HANDLE,        // handle
                            ACE_Reactor_Mask); // event mask

  // the connector complains
  using SVC_HANDLER_T::reactor;

 protected:
  Net_TCPSocketHandler_T ();
  inline virtual ~Net_TCPSocketHandler_T () {}

  inline bool registerWithReactor () { return (inherited2::open (NULL) == 0); }
  inline void deregisterFromReactor () { inherited2::shutdown (); }

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_TCPSocketHandler_T (const Net_TCPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_TCPSocketHandler_T& operator= (const Net_TCPSocketHandler_T&))
};

//////////////////////////////////////////

#if defined (SSL_SUPPORT)
// partial specialization (for SSL)
template <ACE_SYNCH_DECL,
          typename ConfigurationType>
class Net_TCPSocketHandler_T<ACE_SYNCH_USE,
                             ACE_SSL_SOCK_Stream,
                             ConfigurationType>
 : public Net_SocketHandlerBase_T<ConfigurationType>
 , public ACE_Svc_Handler<ACE_SSL_SOCK_Stream,
                          ACE_SYNCH_USE>
 , public ACE_Reactor_Notification_Strategy // *NOTE*: the 'write'-strategy
{
  typedef Net_SocketHandlerBase_T<ConfigurationType> inherited;
  typedef ACE_Svc_Handler<ACE_SSL_SOCK_Stream,
                          ACE_SYNCH_USE> inherited2;
  typedef ACE_Reactor_Notification_Strategy inherited3;

 public:
  // convenient types
  typedef ACE_Svc_Handler<ACE_SSL_SOCK_Stream,
                          ACE_SYNCH_USE> SVC_HANDLER_T;

  // override some task-based members
  virtual int open (void* = NULL); // args

  // override some event handler methods
  virtual int handle_close (ACE_HANDLE,        // handle
                            ACE_Reactor_Mask); // event mask

 protected:
  Net_TCPSocketHandler_T ();
  inline virtual ~Net_TCPSocketHandler_T () {}

  inline bool registerWithReactor () { return (inherited2::open (NULL) == 0); }
  inline void deregisterFromReactor () { inherited2::shutdown (); }

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_TCPSocketHandler_T (const Net_TCPSocketHandler_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_TCPSocketHandler_T& operator= (const Net_TCPSocketHandler_T&))
};
#endif // SSL_SUPPORT

// include template definition
#include "net_tcpsockethandler.inl"

#endif
