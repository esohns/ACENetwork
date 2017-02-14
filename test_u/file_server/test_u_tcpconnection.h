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

#ifndef TEST_U_TCPCONNECTION_H
#define TEST_U_TCPCONNECTION_H

#include <ace/Acceptor.h>
#include <ace/Asynch_Acceptor.h>
#include <ace/Asynch_Connector.h>
#include <ace/Connector.h>
#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Connector.h>
#include <ace/Time_Value.h>

#include "net_tcpconnection_base.h"

#include "test_u_configuration.h"
#include "test_u_socket_common.h"

#include "file_server_connection_common.h"

// forward declarations
class Test_U_Stream;

class Test_U_TCPConnection
 : public Net_TCPConnectionBase_T<Test_U_TCPHandler_t,
                                  struct FileServer_ConnectionConfiguration,
                                  struct FileServer_ConnectionState,
                                  Net_RuntimeStatistic_t,
                                  struct FileServer_SocketHandlerConfiguration,
                                  Test_U_Stream,
                                  struct FileServer_UserData>
{
  friend class ACE_Acceptor<Test_U_TCPConnection, ACE_SOCK_ACCEPTOR>;
  friend class ACE_Connector<Test_U_TCPConnection, ACE_SOCK_CONNECTOR>;

 public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   struct FileServer_ConnectionConfiguration,
                                   struct FileServer_ConnectionState,
                                   Net_RuntimeStatistic_t,
                                   struct FileServer_UserData> ICONNECTION_MANAGER_T;

  Test_U_TCPConnection (ICONNECTION_MANAGER_T*,                        // connection manager handle
                        const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  virtual ~Test_U_TCPConnection ();

  // implement (part of) Net_ITransportLayer_T
  inline virtual void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

 private:
  typedef Net_TCPConnectionBase_T<Test_U_TCPHandler_t,
                                  struct FileServer_ConnectionConfiguration,
                                  struct FileServer_ConnectionState,
                                  Net_RuntimeStatistic_t,
                                  struct FileServer_SocketHandlerConfiguration,
                                  Test_U_Stream,
                                  struct FileServer_UserData> inherited;

  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_svc_handler() method of ACE_Connector/ACE_Acceptor
  Test_U_TCPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Test_U_TCPConnection (const Test_U_TCPConnection&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_TCPConnection& operator= (const Test_U_TCPConnection&))
};

//////////////////////////////////////////

class Test_U_AsynchTCPConnection
 : public Net_AsynchTCPConnectionBase_T<Test_U_AsynchTCPHandler_t,
                                        struct FileServer_ConnectionConfiguration,
                                        struct FileServer_ConnectionState,
                                        Net_RuntimeStatistic_t,
                                        struct FileServer_SocketHandlerConfiguration,
                                        Test_U_Stream,
                                        struct FileServer_UserData>
{
 friend class ACE_Asynch_Acceptor<Test_U_AsynchTCPConnection>;
 friend class ACE_Asynch_Connector<Test_U_AsynchTCPConnection>;

 public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   struct FileServer_ConnectionConfiguration,
                                   struct FileServer_ConnectionState,
                                   Net_RuntimeStatistic_t,
                                   struct FileServer_UserData> ICONNECTION_MANAGER_T;

  Test_U_AsynchTCPConnection (ICONNECTION_MANAGER_T*,                        // connection manager handle
                              const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  virtual ~Test_U_AsynchTCPConnection ();

  // implement (part of) Net_ITransportLayer_T
  inline virtual void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

 private:
  typedef Net_AsynchTCPConnectionBase_T<Test_U_AsynchTCPHandler_t,
                                        struct FileServer_ConnectionConfiguration,
                                        struct FileServer_ConnectionState,
                                        Net_RuntimeStatistic_t,
                                        struct FileServer_SocketHandlerConfiguration,
                                        Test_U_Stream,
                                        struct FileServer_UserData> inherited;

  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_handler() method of ACE_AsynchConnector/ACE_AsynchAcceptor
  Test_U_AsynchTCPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Test_U_AsynchTCPConnection (const Test_U_AsynchTCPConnection&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AsynchTCPConnection& operator= (const Test_U_AsynchTCPConnection&))
};

#endif
