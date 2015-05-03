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

#ifndef NET_TCPCONNECTION_H
#define NET_TCPCONNECTION_H

#include "ace/Acceptor.h"
#include "ace/Asynch_Acceptor.h"
#include "ace/Asynch_Connector.h"
#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_exports.h"
#include "net_socket_common.h"
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
   typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                    Net_SocketConfiguration_t,
                                    Net_Configuration_t,
                                    Net_UserData_t,
                                    Stream_Statistic_t> ICONNECTION_MANAGER_T;

  Net_TCPConnection (ICONNECTION_MANAGER_T*, // connection manager handle
                     unsigned int = 0);      // statistics collecting interval (second(s))
                                             // 0 --> DON'T collect statistics
  virtual ~Net_TCPConnection ();

 private:
  typedef Net_TCPConnectionBase_T<Net_Configuration_t,
                                  Net_UserData_t,
                                  Net_StreamSessionData_t,
                                  Net_TCPHandler_t> inherited;

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
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   Net_SocketConfiguration_t,
                                   Net_Configuration_t,
                                   Net_UserData_t,
                                   Stream_Statistic_t> ICONNECTION_MANAGER_T;

  Net_AsynchTCPConnection (ICONNECTION_MANAGER_T*, // connection manager handle
                           unsigned int = 0);      // statistics collecting interval (second(s))
                                                   // 0 --> DON'T collect statistics
  virtual ~Net_AsynchTCPConnection ();

 private:
  typedef Net_AsynchTCPConnectionBase_T<Net_Configuration_t,
                                        Net_UserData_t,
                                        Net_StreamSessionData_t,
                                        Net_AsynchTCPHandler_t> inherited;

  Net_AsynchTCPConnection ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection (const Net_AsynchTCPConnection&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnection& operator= (const Net_AsynchTCPConnection&));
};

#endif
