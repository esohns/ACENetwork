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

#ifndef NET_TCPCONNECTION_BASE_H
#define NET_TCPCONNECTION_BASE_H

#include "ace/Acceptor.h"
#include "ace/Asynch_Acceptor.h"
#include "ace/Asynch_Connector.h"
#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"

#include "stream_common.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnectionmanager.h"
#include "net_socketconnection_base.h"
#include "net_transportlayer_tcp.h"

template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType,
          typename HandlerType,
          typename HandlerConfigurationType>
class Net_TCPConnectionBase_T
 : public Net_SocketConnectionBase_T<ACE_INET_Addr,
                                     Net_SocketConfiguration,
                                     HandlerType,
                                     ConfigurationType,
                                     HandlerConfigurationType,
                                     UserDataType,
                                     SessionDataType,
                                     Stream_Statistic,
                                     StreamType>
 , public Net_TransportLayer_TCP
{
 friend class ACE_Acceptor<Net_TCPConnectionBase_T<ConfigurationType,
                                                   UserDataType,
                                                   SessionDataType,
                                                   StreamType,
                                                   HandlerType,
                                                   HandlerConfigurationType>,
                           ACE_SOCK_ACCEPTOR>;
 friend class ACE_Connector<Net_TCPConnectionBase_T<ConfigurationType,
                                                    UserDataType,
                                                    SessionDataType,
                                                    StreamType,
                                                    HandlerType,
                                                    HandlerConfigurationType>,
                            ACE_SOCK_CONNECTOR>;

 public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   Net_SocketConfiguration,
                                   ConfigurationType,
                                   UserDataType,
                                   Stream_Statistic,
                                   StreamType> ICONNECTION_MANAGER_T;

  Net_TCPConnectionBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                           unsigned int = 0);      // statistics collecting interval (second(s))
                                                   // 0 --> DON'T collect statistics
  virtual ~Net_TCPConnectionBase_T ();

 private:
  typedef Net_SocketConnectionBase_T<ACE_INET_Addr,
                                     Net_SocketConfiguration,
                                     HandlerType,
                                     ConfigurationType,
                                     HandlerConfigurationType,
                                     UserDataType,
                                     SessionDataType,
                                     Stream_Statistic,
                                     StreamType> inherited;

  Net_TCPConnectionBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnectionBase_T (const Net_TCPConnectionBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnectionBase_T& operator= (const Net_TCPConnectionBase_T&));
};

/////////////////////////////////////////

template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StreamType,
          typename HandlerType,
          typename HandlerConfigurationType>
class Net_AsynchTCPConnectionBase_T
 : public Net_AsynchSocketConnectionBase_T<ACE_INET_Addr,
                                           Net_SocketConfiguration,
                                           HandlerType,
                                           ConfigurationType,
                                           HandlerConfigurationType,
                                           UserDataType,
                                           SessionDataType,
                                           Stream_Statistic,
                                           StreamType>
 , public Net_TransportLayer_TCP
{
 friend class ACE_Asynch_Acceptor<Net_AsynchTCPConnectionBase_T<ConfigurationType,
                                                                UserDataType,
                                                                SessionDataType,
                                                                StreamType,
                                                                HandlerType,
                                                                HandlerConfigurationType> >;
 friend class ACE_Asynch_Connector<Net_AsynchTCPConnectionBase_T<ConfigurationType,
                                                                 UserDataType,
                                                                 SessionDataType,
                                                                 StreamType,
                                                                 HandlerType,
                                                                 HandlerConfigurationType> >;

 public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   Net_SocketConfiguration,
                                   ConfigurationType,
                                   UserDataType,
                                   Stream_Statistic,
                                   StreamType> ICONNECTION_MANAGER_T;

  Net_AsynchTCPConnectionBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                                 unsigned int = 0);      // statistics collecting interval (second(s))
                                                         // 0 --> DON'T collect statistics
  virtual ~Net_AsynchTCPConnectionBase_T ();

 private:
  typedef Net_AsynchSocketConnectionBase_T<ACE_INET_Addr,
                                           Net_SocketConfiguration,
                                           HandlerType,
                                           ConfigurationType,
                                           HandlerConfigurationType,
                                           UserDataType,
                                           SessionDataType,
                                           Stream_Statistic,
                                           StreamType> inherited;

  Net_AsynchTCPConnectionBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnectionBase_T (const Net_AsynchTCPConnectionBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnectionBase_T& operator= (const Net_AsynchTCPConnectionBase_T&));
};

#include "net_tcpconnection_base.inl"

#endif
