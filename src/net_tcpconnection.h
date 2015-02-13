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

#include "net_exports.h"
#include "net_socket_common.h"
#include "net_socketconnection_base.h"
#include "net_transportlayer_tcp.h"

#include "ace/Event_Handler.h"

class Net_Export Net_TCPConnection
 : public Net_SocketConnectionBase_T<Net_TCPHandler_t,
                                     Net_TransportLayer_TCP,
                                     Net_StreamProtocolConfigurationState_t,
                                     Net_RuntimeStatistic_t>
{
 public:
  Net_TCPConnection ();

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
                                     Net_StreamProtocolConfigurationState_t,
                                     Net_RuntimeStatistic_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  virtual ~Net_TCPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnection (const Net_TCPConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnection& operator= (const Net_TCPConnection&));
};

/////////////////////////////////////////

class Net_Export Net_AsynchTCPConnection
 : public Net_SocketConnectionBase_T<Net_AsynchTCPHandler_t,
                                     Net_TransportLayer_TCP,
                                     Net_StreamProtocolConfigurationState_t,
                                     Net_RuntimeStatistic_t>
{
 public:
  Net_AsynchTCPConnection ();

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
                                     Net_StreamProtocolConfigurationState_t,
                                     Net_RuntimeStatistic_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  virtual ~Net_AsynchTCPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection (const Net_AsynchTCPConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection& operator= (const Net_AsynchTCPConnection&));
};

#endif
