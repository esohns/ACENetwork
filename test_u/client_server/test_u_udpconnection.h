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

#include "test_u_network_common.h"

#include "test_u_configuration.h"
#include "test_u_connection_common.h"
#include "test_u_stream.h"

#include "net_client_common.h"

class Test_U_UDPConnection
 : public Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                  Net_UDPSocketHandler_t,
                                  Test_U_UDPConnectionConfiguration,
                                  struct Net_StreamConnectionState,
                                  Net_StreamStatistic_t,
                                  Test_U_Stream_T<Test_U_UDPConnectionManager_t>,
                                  struct Net_UserData>
 , public Net_IPing
{
  typedef Net_UDPConnectionBase_T<ACE_NULL_SYNCH,
                                  Net_UDPSocketHandler_t,
                                  Test_U_UDPConnectionConfiguration,
                                  struct Net_StreamConnectionState,
                                  Net_StreamStatistic_t,
                                  Test_U_Stream_T<Test_U_UDPConnectionManager_t>,
                                  struct Net_UserData> inherited;

  friend class ACE_Connector<Test_U_UDPConnection, ACE_SOCK_CONNECTOR>;

 public:
  Test_U_UDPConnection (bool); // managed ?
  inline virtual ~Test_U_UDPConnection () {}

  // implement Net_IPing
  inline virtual void ping () { inherited::stream_.ping (); }

 private:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Test_U_UDPConnection () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  ACE_UNIMPLEMENTED_FUNC (Test_U_UDPConnection (const Test_U_UDPConnection&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_UDPConnection& operator= (const Test_U_UDPConnection&))
};

//////////////////////////////////////////

class Test_U_AsynchUDPConnection
 : public Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                        Test_U_UDPConnectionConfiguration,
                                        struct Net_StreamConnectionState,
                                        Net_StreamStatistic_t,
                                        Test_U_Stream_T<Test_U_UDPConnectionManager_t>,
                                        struct Net_UserData>
 , public Net_IPing
{
  typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                        Test_U_UDPConnectionConfiguration,
                                        struct Net_StreamConnectionState,
                                        Net_StreamStatistic_t,
                                        Test_U_Stream_T<Test_U_UDPConnectionManager_t>,
                                        struct Net_UserData> inherited;

 friend class ACE_Asynch_Connector<Test_U_AsynchUDPConnection>;

 public:
  Test_U_AsynchUDPConnection (bool); // managed ?
  inline virtual ~Test_U_AsynchUDPConnection () {}

  // implement Net_IPing
  inline virtual void ping () { inherited::stream_.ping (); }

 private:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Test_U_AsynchUDPConnection () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  ACE_UNIMPLEMENTED_FUNC (Test_U_AsynchUDPConnection (const Test_U_AsynchUDPConnection&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_AsynchUDPConnection& operator= (const Test_U_AsynchUDPConnection&))
};

#endif
