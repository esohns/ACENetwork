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

#ifndef ClientServer_UDPConnection_H
#define ClientServer_UDPConnection_H

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

class ClientServer_UDPConnection
 : public Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                  ClientServer_UDPSocketHandler_t,
                                  ClientServer_ConnectionConfiguration_t,
                                  struct ClientServer_ConnectionState,
                                  Net_Statistic_t,
                                  struct ClientServer_SocketHandlerConfiguration,
                                  Test_U_Stream,
                                  Common_Timer_Manager_t,
                                  struct Test_U_UserData>
 , public Net_IPing
{
  typedef Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                  ClientServer_UDPSocketHandler_t,
                                  ClientServer_ConnectionConfiguration_t,
                                  struct ClientServer_ConnectionState,
                                  Net_Statistic_t,
                                  struct ClientServer_SocketHandlerConfiguration,
                                  Test_U_Stream,
                                  Common_Timer_Manager_t,
                                  struct Test_U_UserData> inherited;

  friend class ACE_Connector<ClientServer_UDPConnection, ACE_SOCK_CONNECTOR>;

 public:
  typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                   ACE_INET_Addr,
                                   ClientServer_ConnectionConfiguration_t,
                                   struct ClientServer_ConnectionState,
                                   Net_Statistic_t,
                                   struct Test_U_UserData> ICONNECTION_MANAGER_T;

  ClientServer_UDPConnection (ICONNECTION_MANAGER_T*,                        // connection manager handle
                              const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  inline virtual ~ClientServer_UDPConnection () {}

  // implement Net_IPing
  inline virtual void ping () { inherited::stream_.ping (); }

 private:
  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_svc_handler() method of ACE_Connector/ACE_Acceptor
  ClientServer_UDPConnection ();
  ACE_UNIMPLEMENTED_FUNC (ClientServer_UDPConnection (const ClientServer_UDPConnection&))
  ACE_UNIMPLEMENTED_FUNC (ClientServer_UDPConnection& operator= (const ClientServer_UDPConnection&))
};

//////////////////////////////////////////

class ClientServer_AsynchUDPConnection
 : public Net_AsynchUDPConnectionBase_T<ClientServer_AsynchUDPSocketHandler_t,
                                        ClientServer_ConnectionConfiguration_t,
                                        struct ClientServer_ConnectionState,
                                        Net_Statistic_t,
                                        struct ClientServer_SocketHandlerConfiguration,
                                        Test_U_Stream,
                                        Common_Timer_Manager_t,
                                        struct Test_U_UserData>
 , public Net_IPing
{
  typedef Net_AsynchUDPConnectionBase_T<ClientServer_AsynchUDPSocketHandler_t,
                                        ClientServer_ConnectionConfiguration_t,
                                        struct ClientServer_ConnectionState,
                                        Net_Statistic_t,
                                        struct ClientServer_SocketHandlerConfiguration,
                                        Test_U_Stream,
                                        Common_Timer_Manager_t,
                                        struct Test_U_UserData> inherited;

 friend class ACE_Asynch_Connector<ClientServer_AsynchUDPConnection>;

 public:
  typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                   ACE_INET_Addr,
                                   ClientServer_ConnectionConfiguration_t,
                                   struct ClientServer_ConnectionState,
                                   Net_Statistic_t,
                                   struct Test_U_UserData> ICONNECTION_MANAGER_T;

  ClientServer_AsynchUDPConnection (ICONNECTION_MANAGER_T*,                        // connection manager handle
                                    const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  inline virtual ~ClientServer_AsynchUDPConnection () {}

  // implement Net_IPing
  inline virtual void ping () { inherited::stream_.ping (); }

 private:
  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_handler() method of ACE_AsynchConnector/ACE_AsynchAcceptor
  ClientServer_AsynchUDPConnection ();
  ACE_UNIMPLEMENTED_FUNC (ClientServer_AsynchUDPConnection (const ClientServer_AsynchUDPConnection&))
  ACE_UNIMPLEMENTED_FUNC (ClientServer_AsynchUDPConnection& operator= (const ClientServer_AsynchUDPConnection&))
};

#endif
