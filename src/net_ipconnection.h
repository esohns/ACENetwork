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

#ifndef Net_IPCONNECTION_H
#define Net_IPCONNECTION_H

//#include "ace/Global_Macros.h"
//#include "ace/INET_Addr.h"

//#include "net_connection_manager_common.h"
//#include "net_exports.h"
//#include "net_socket_common.h"
//#include "net_socketconnection_base.h"
//#include "net_stream_common.h"
//#include "net_transportlayer_udp.h"

//class Net_Export Net_IPMulticastConnection
// : public Net_IPMulticastHandler,
//   public ACE_Task<ACE_MT_SYNCH>
//{
// public:
//  Net_IPMulticastConnection (Net_IIPConnectionManager_t*); // connection manager handle
//
//  //  // implement (part of) Net_IConnection
//  //  virtual void ping ();
//
//  // override some task-based members
//  virtual int open (void* = NULL); // args
//  virtual int close (u_long = 0); // args
//
//  //  // *NOTE*: enqueue any received data onto our stream for further processing
//  //   virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
//  // *NOTE*: this is called when:
//  // - handle_xxx() returns -1
//  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
//                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);
//
// private:
//  typedef Net_IPMulticastHandler inherited;
//  typedef ACE_Task<ACE_MT_SYNCH> inherited2;
//
//  virtual ~Net_IPMulticastConnection ();
//  ACE_UNIMPLEMENTED_FUNC (Net_IPMulticastConnection ());
//  ACE_UNIMPLEMENTED_FUNC (Net_IPMulticastConnection (const Net_IPMulticastConnection&));
//  ACE_UNIMPLEMENTED_FUNC (Net_IPMulticastConnection& operator= (const Net_IPMulticastConnection&));
//
//  bool joined_;
//};

///////////////////////////////////////////

//class Net_Export Net_IPBroadcastConnection
// : public Net_IPBroadcastHandler,
//   public ACE_Task<ACE_MT_SYNCH>
//{
// public:
//  Net_IPBroadcastConnection (Net_IIPConnectionManager_t*); // connection manager handle
//
//  //  // implement (part of) Net_IConnection
//  //  virtual void ping ();
//
//  // override some task-based members
//  virtual int svc (void);
//  virtual int open (void* = NULL); // args
//  virtual int close (u_long = 0); // args
//
//  //  // *NOTE*: enqueue any received data onto our stream for further processing
//  //   virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
//  // *NOTE*: this is called when:
//  // - handle_xxx() returns -1
//  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
//                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);
//
//  private:
//  typedef Net_IPBroadcastHandler inherited;
//  typedef ACE_Task<ACE_MT_SYNCH> inherited2;
//
//  // stop worker, if any
//  void shutdown ();
//
//  virtual ~Net_IPBroadcastConnection ();
//  ACE_UNIMPLEMENTED_FUNC (Net_IPBroadcastConnection ());
//  ACE_UNIMPLEMENTED_FUNC (Net_IPBroadcastConnection (const Net_IPBroadcastConnection&));
//  ACE_UNIMPLEMENTED_FUNC (Net_IPBroadcastConnection& operator= (const Net_IPBroadcastConnection&));
//};

#endif
