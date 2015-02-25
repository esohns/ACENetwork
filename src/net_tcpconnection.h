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

#ifndef Net_TCPCONNECTION_H
#define Net_TCPCONNECTION_H

#include "ace/Global_Macros.h"
#include "ace/Event_Handler.h"
#include "ace/SOCK_Connector.h"

//#include "net_connection_manager_common.h"
#include "net_exports.h"
#include "net_socket_common.h"
#include "net_socketconnection_base.h"
#include "net_transportlayer_tcp.h"

// forward declarations
template <typename SVC_HANDLER,
          typename PEER_CONNECTOR> class ACE_Connector;
template <class HANDLER> class ACE_Asynch_Connector;

class Net_Export Net_TCPConnection
 : public Net_SocketConnectionBase_T<Net_TCPHandler_t,
                                     Net_TransportLayer_TCP,
                                     Net_Configuration_t,
                                     Stream_SessionData_t,
                                     Stream_Statistic_t>
{
  //friend class ACE_Connector<Net_TCPConnection, ACE_SOCK_CONNECTOR>;

 public:
   // *NOTE*: consider encapsulating this (need to grant access to
   //         ACE_Connector however (see: ace/Connector.cpp:239))
   Net_TCPConnection ();
   //Net_TCPConnection (Net_IConnectionManager_t*);

  // implement (part of) Net_ITransportLayer
  virtual void info (ACE_HANDLE&,           // return value: handle
                     ACE_INET_Addr&,        // return value: local SAP
                     ACE_INET_Addr&) const; // return value: remote SAP

  //// override some task-based members
  //virtual int open (void* = NULL); // args
  //virtual int close (u_long = 0); // args

//  // *NOTE*: enqueue any received data onto our stream for further processing
//   virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

 private:
  typedef Net_SocketConnectionBase_T<Net_TCPHandler_t,
                                     Net_TransportLayer_TCP,
                                     Net_Configuration_t,
                                     Stream_SessionData_t,
                                     Stream_Statistic_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  virtual ~Net_TCPConnection ();
  //ACE_UNIMPLEMENTED_FUNC (Net_TCPConnection (const Net_TCPConnection&));
  //ACE_UNIMPLEMENTED_FUNC (Net_TCPConnection& operator= (const Net_TCPConnection&));
};

/////////////////////////////////////////

class Net_Export Net_AsynchTCPConnection
 : public Net_SocketConnectionBase_T<Net_AsynchTCPHandler_t,
                                     Net_TransportLayer_TCP,
                                     Net_Configuration_t,
                                     Stream_SessionData_t,
                                     Stream_Statistic_t>
{
 friend class ACE_Asynch_Connector<Net_AsynchTCPConnection>;

 public:
   // *WARNING*: need to make this available to Asynch_Connector
   //            (see: ace/Asynch_Connector.cpp:239)
   Net_AsynchTCPConnection ();
  //Net_AsynchTCPConnection (Net_IConnectionManager_t*);

  //// override some task-based members
  //virtual int open (void* = NULL); // args
  //virtual int close (u_long = 0); // args

  //  // *NOTE*: enqueue any received data onto our stream for further processing
  //   virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

 private:
  typedef Net_SocketConnectionBase_T<Net_AsynchTCPHandler_t,
                                     Net_TransportLayer_TCP,
                                     Net_Configuration_t,
                                     Stream_SessionData_t,
                                     Stream_Statistic_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  virtual ~Net_AsynchTCPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection (const Net_AsynchTCPConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection& operator= (const Net_AsynchTCPConnection&));
};

#endif
