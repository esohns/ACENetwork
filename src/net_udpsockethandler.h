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

#ifndef Net_UDPSOCKETHANDLER_H
#define Net_UDPSOCKETHANDLER_H

#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/SOCK_Dgram.h"
//#include "ace/SOCK_Stream.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"
#include "ace/Reactor_Notification_Strategy.h"

//#include "common_referencecounter_base.h"
//#include "common_irefcount.h"

//#include "net_iconnection.h"
//#include "net_iconnectionmanager.h"

template <typename SocketType>
class Net_UDPSocketHandler_T
 : public SocketType
 //: public Common_IRefCount
// : public ACE_Event_Handler
 , public ACE_Svc_Handler<ACE_SOCK_DGRAM, ACE_MT_SYNCH>
// : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
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

//  // implement (part of) Net_IConnection
//  virtual void close ();
//  virtual void info (ACE_HANDLE&,           // return value: handle
//                     ACE_INET_Addr&,        // return value: local SAP
//                     ACE_INET_Addr&) const; // return value: remote SAP
//  virtual unsigned int id () const;

 protected:
  typedef ACE_Svc_Handler<ACE_SOCK_DGRAM, ACE_MT_SYNCH> SVC_HANDLER_T;

//  typedef Net_IConnectionManager<ConfigurationType,
//                                 StatisticsContainerType> MANAGER_T;
//  Net_UDPSocketHandler (MANAGER_T*);
  Net_UDPSocketHandler_T ();
  virtual ~Net_UDPSocketHandler_T ();

  ACE_Reactor_Notification_Strategy notificationStrategy_;
//  MANAGER_T*                        manager_;
  //ConfigurationType                 userData_;
//  bool                              isRegistered_;

 private:
  typedef SocketType inherited;
  //typedef Common_IRefCount inherited;
//  typedef ACE_Event_Handler inherited;
  typedef ACE_Svc_Handler<ACE_SOCK_DGRAM, ACE_MT_SYNCH> inherited2;
//  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH> inherited2;

  ACE_UNIMPLEMENTED_FUNC (Net_UDPSocketHandler_T (const Net_UDPSocketHandler_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_UDPSocketHandler_T& operator= (const Net_UDPSocketHandler_T&));

  //// implement Common_IRefCount
  //virtual void increase ();
  //virtual void decrease ();
  //virtual unsigned int count ();
  //// *NOTE*: this call should block IF the count is > 0 and wait
  //// until the count reaches 0 the next time
  //virtual void wait_zero ();
};

// include template implementation
#include "net_udpsockethandler.inl"

#endif
