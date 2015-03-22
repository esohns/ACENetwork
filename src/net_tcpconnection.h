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

#include "stream_common.h"

#include "net_configuration.h"
#include "net_exports.h"
#include "net_iconnectionmanager.h"
#include "net_itransportlayer.h"
#include "net_socket_common.h"
#include "net_socketconnection_base.h"
#include "net_stream_common.h"

// forward declarations
template <typename SVC_HANDLER,
          typename PEER_CONNECTOR> class ACE_Connector;
template <class HANDLER> class ACE_Asynch_Connector;

class Net_Export Net_TCPConnection
 : public Net_SocketConnectionBase_T<Net_TCPHandler_t,
                                     Net_IInetTransportLayer_t,
                                     Net_Configuration_t,
                                     Net_UserData_t,
                                     Net_StreamSessionData_t,
                                     Stream_Statistic_t>
{
  //friend class ACE_Connector<Net_TCPConnection, ACE_SOCK_CONNECTOR>;

 public:
  typedef Net_IConnectionManager_T<Net_Configuration_t,
                                   Net_UserData_t,
                                   Stream_Statistic_t,
                                   Net_IInetTransportLayer_t> ICONNECTION_MANAGER_T;

  Net_TCPConnection (ICONNECTION_MANAGER_T*, // connection manager handle
                     unsigned int = 0);      // statistics collecting interval (second(s))
                                             // 0 --> DON'T collect statistics
  virtual ~Net_TCPConnection ();

  // override / implement (part of) Net_IInetTransportLayer
  virtual bool initialize (Net_ClientServerRole_t,            // role
                           const Net_SocketConfiguration_t&); // socket configuration
  virtual void finalize ();
  virtual void info (ACE_HANDLE&,           // return value: handle
                     ACE_INET_Addr&,        // return value: local SAP
                     ACE_INET_Addr&) const; // return value: remote SAP
  virtual unsigned int id () const;
  virtual void dump_state () const;

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
                                     Net_IInetTransportLayer_t,
                                     Net_Configuration_t,
                                     Net_UserData_t,
                                     Net_StreamSessionData_t,
                                     Stream_Statistic_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnection ());
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnection (const Net_TCPConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnection& operator= (const Net_TCPConnection&));
};

/////////////////////////////////////////

class Net_Export Net_AsynchTCPConnection
 : public Net_AsynchSocketConnectionBase_T<Net_AsynchTCPHandler_t,
                                           Net_IInetTransportLayer_t,
                                           Net_Configuration_t,
                                           Net_UserData_t,
                                           Net_StreamSessionData_t,
                                           Stream_Statistic_t>
{
 friend class ACE_Asynch_Connector<Net_AsynchTCPConnection>;

 public:
  typedef Net_IConnectionManager_T<Net_Configuration_t,
                                   Net_UserData_t,
                                   Stream_Statistic_t,
                                   Net_IInetTransportLayer_t> ICONNECTION_MANAGER_T;

 // *WARNING*: need to make this available to Asynch_Connector
   //            (see: ace/Asynch_Connector.cpp:239)
//   Net_AsynchTCPConnection ();
  Net_AsynchTCPConnection (ICONNECTION_MANAGER_T*);

  // override / implement (part of) Net_IInetTransportLayer
  virtual bool initialize (Net_ClientServerRole_t,            // role
                           const Net_SocketConfiguration_t&); // socket configuration
  virtual void finalize ();
  virtual void info (ACE_HANDLE&,           // return value: handle
                     ACE_INET_Addr&,        // return value: local SAP
                     ACE_INET_Addr&) const; // return value: remote SAP
  virtual unsigned int id () const;
  virtual void dump_state () const;

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
  typedef Net_AsynchSocketConnectionBase_T<Net_AsynchTCPHandler_t,
                                           Net_IInetTransportLayer_t,
                                           Net_Configuration_t,
                                           Net_UserData_t,
                                           Net_StreamSessionData_t,
                                           Stream_Statistic_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  virtual ~Net_AsynchTCPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection ());
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection (const Net_AsynchTCPConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection& operator= (const Net_AsynchTCPConnection&));
};

#endif
