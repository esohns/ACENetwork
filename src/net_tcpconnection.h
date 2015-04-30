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

#include "ace/Asynch_Acceptor.h"
#include "ace/Asynch_Connector.h"
#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/Event_Handler.h"
#include "ace/Acceptor.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_exports.h"
#include "net_socket_common.h"
//#include "net_stream_common.h"
#include "net_tcpconnection_base.h"

class Net_Export Net_TCPConnection
 : public Net_TCPConnectionBase_T<Net_Configuration_t,
                                  Net_UserData_t,
                                  Net_StreamSessionData_t,
                                  Net_TCPHandler_t>
{
 friend class ACE_Acceptor<Net_TCPConnection, ACE_SOCK_ACCEPTOR>;
 friend class ACE_Connector<Net_TCPConnection, ACE_SOCK_CONNECTOR>;

 public:
  Net_TCPConnection (ICONNECTION_MANAGER_T*, // connection manager handle
                     unsigned int = 0);      // statistics collecting interval (second(s))
                                             // 0 --> DON'T collect statistics
  virtual ~Net_TCPConnection ();

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
  typedef Net_TCPConnectionBase_T<Net_Configuration_t,
                                  Net_UserData_t,
                                  Net_StreamSessionData_t,
                                  Net_TCPHandler_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  Net_TCPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnection (const Net_TCPConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnection& operator= (const Net_TCPConnection&));
};

/////////////////////////////////////////

class Net_Export Net_AsynchTCPConnection
 : public Net_AsynchTCPConnectionBase_T<Net_Configuration_t,
                                        Net_UserData_t,
                                        Net_StreamSessionData_t,
                                        Net_AsynchTCPHandler_t>
{
 friend class ACE_Asynch_Acceptor<Net_AsynchTCPConnection>;
 friend class ACE_Asynch_Connector<Net_AsynchTCPConnection>;

 public:
  typedef Net_IConnectionManager_T<Net_Configuration_t,
                                   Net_UserData_t,
                                   Stream_Statistic_t,
                                   Net_IInetTransportLayer_t> ICONNECTION_MANAGER_T;

  Net_AsynchTCPConnection (ICONNECTION_MANAGER_T*, // connection manager handle
                           unsigned int = 0);      // statistics collecting interval (second(s))
                                                   // 0 --> DON'T collect statistics
  virtual ~Net_AsynchTCPConnection ();

  // override some ACE_Service_Handler members
  //virtual void open (ACE_HANDLE,          // handle
  //                   ACE_Message_Block&); // (initial) data (if any)
  ////virtual int close (u_long = 0); // args

  //  // *NOTE*: enqueue any received data onto our stream for further processing
  //   virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

 private:
  typedef Net_AsynchTCPConnectionBase_T<Net_Configuration_t,
                                        Net_UserData_t,
                                        Net_StreamSessionData_t,
                                        Net_AsynchTCPHandler_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  Net_AsynchTCPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection (const Net_AsynchTCPConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection& operator= (const Net_AsynchTCPConnection&));
};

#endif
