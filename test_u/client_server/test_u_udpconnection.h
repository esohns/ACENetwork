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

#ifndef Test_U_UDPConnection_H
#define Test_U_UDPConnection_H

#include "ace/Acceptor.h"
#include "ace/Asynch_Acceptor.h"
#include "ace/Asynch_Connector.h"
#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_timer_manager_common.h"

#include "net_sock_dgram.h"
#include "net_udpconnection_base.h"

#include "net_client_common.h"

#include "test_u_configuration.h"
#include "test_u_connection_common.h"
#include "test_u_socket_common.h"

// forward declarations
class Test_U_Stream;

class Test_U_UDPConnection
 : public Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                  Test_U_UDPSocketHandler_t,
                                  Test_U_UDPConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_Statistic_t,
                                  Net_UDPSocketConfiguration_t,
                                  Test_U_Stream,
                                  Common_Timer_Manager_t,
                                  struct Net_UserData>
 , public Net_IPing
{
  typedef Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                  Test_U_UDPSocketHandler_t,
                                  Test_U_UDPConnectionConfiguration,
                                  struct Test_U_ConnectionState,
                                  Net_Statistic_t,
                                  struct Test_U_SocketHandlerConfiguration,
                                  Test_U_Stream,
                                  Common_Timer_Manager_t,
                                  struct Net_UserData> inherited;

  friend class ACE_Connector<Test_U_UDPConnection, ACE_SOCK_CONNECTOR>;

 public:
  Test_U_UDPConnection (bool = true); // managed ?
  inline virtual ~Test_U_UDPConnection () {}

  // implement Net_IPing
  inline virtual void ping () { stream_.ping (); }

 private:
  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_svc_handler() method of ACE_Connector/ACE_Acceptor
  Test_U_UDPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Test_U_UDPConnection (const Test_U_UDPConnection&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_UDPConnection& operator= (const Test_U_UDPConnection&))
};

//////////////////////////////////////////

class Test_U_AsynchUDPConnection
 : public Net_AsynchUDPConnectionBase_T<Test_U_AsynchUDPSocketHandler_t,
                                        Test_U_UDPConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_Statistic_t,
                                        struct Test_U_SocketHandlerConfiguration,
                                        Test_U_Stream,
                                        Common_Timer_Manager_t,
                                        struct Net_UserData>
 , public Net_IPing
{
  typedef Net_AsynchUDPConnectionBase_T<Test_U_AsynchUDPSocketHandler_t,
                                        Test_U_UDPConnectionConfiguration,
                                        struct Test_U_ConnectionState,
                                        Net_Statistic_t,
                                        struct Test_U_SocketHandlerConfiguration,
                                        Test_U_Stream,
                                        Common_Timer_Manager_t,
                                        struct Net_UserData> inherited;

 friend class ACE_Asynch_Connector<Test_U_AsynchUDPConnection>;

 public:
  Test_U_AsynchUDPConnection (bool = true); // managed ?
  inline virtual ~Test_U_AsynchUDPConnection () {}

  // implement Net_IPing
  inline virtual void ping () { stream_.ping (); }

 private:
  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_handler() method of ACE_AsynchConnector/ACE_AsynchAcceptor
  Test_U_AsynchUDPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Test_U_AsynchUDPConnection (const Test_U_AsynchUDPConnection&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AsynchUDPConnection& operator= (const Test_U_AsynchUDPConnection&))
};

#endif
