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

#ifndef NET_UDPCONNECTION_H
#define NET_UDPCONNECTION_H

#include "ace/Asynch_Connector.h"
#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_connection_manager_common.h"
#include "net_socketconnection_base.h"
#include "net_stream_common.h"
#include "net_transportlayer_udp.h"

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
class Net_UDPConnection_T
 : public Net_SocketConnectionBase_T<ACE_INET_Addr,
                                     Net_SocketConfiguration_t,
                                     HandlerType,
                                     Net_Configuration_t,
                                     Net_SocketHandlerConfiguration_t,
                                     UserDataType,
                                     SessionDataType,
                                     Stream_Statistic_t>
 , public Net_TransportLayer_UDP
{
  friend class ACE_Connector<Net_UDPConnection_T<UserDataType,
                                                 SessionDataType,
                                                 HandlerType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   Net_SocketConfiguration_t,
                                   Net_Configuration_t,
                                   UserDataType,
                                   Stream_Statistic_t> ICONNECTION_MANAGER_T;

  Net_UDPConnection_T (ICONNECTION_MANAGER_T*, // connection manager handle
                       unsigned int = 0);      // statistics collecting interval (second(s))
                                               // 0 --> DON'T collect statistics
  virtual ~Net_UDPConnection_T ();

  // override / implement (part of) Net_IInetTransportLayer
  //using Net_SocketConnectionBase_T::inherited::Net_ConnectionBase_T::initialize;
  //using Net_SocketConnectionBase_T::inherited::Net_ConnectionBase_T::finalize;
//  using Net_SocketConnectionBase_T::info;
  //virtual void info (ACE_HANDLE&,           // return value: handle
  //                   ACE_INET_Addr&,        // return value: local SAP
  //                   ACE_INET_Addr&) const; // return value: remote SAP

  // override / implement (part of) Net_IConnection_T
  virtual void close ();

 private:
  typedef Net_SocketConnectionBase_T<ACE_INET_Addr,
                                     Net_SocketConfiguration_t,
                                     HandlerType,
                                     Net_Configuration_t,
                                     Net_SocketHandlerConfiguration_t,
                                     UserDataType,
                                     SessionDataType,
                                     Stream_Statistic_t> inherited;
  typedef Net_TransportLayer_UDP inherited2;

  ACE_UNIMPLEMENTED_FUNC (Net_UDPConnection_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_UDPConnection_T (const Net_UDPConnection_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_UDPConnection_T& operator= (const Net_UDPConnection_T&));
};

///////////////////////////////////////////

template <typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
class Net_AsynchUDPConnection_T
 : public Net_AsynchSocketConnectionBase_T<ACE_INET_Addr,
                                           Net_SocketConfiguration_t,
                                           HandlerType,
                                           Net_Configuration_t,
                                           Net_SocketHandlerConfiguration_t,
                                           UserDataType,
                                           SessionDataType,
                                           Stream_Statistic_t>
 , public Net_TransportLayer_UDP
{
  friend class ACE_Asynch_Connector<Net_AsynchUDPConnection_T<UserDataType,
                                                              SessionDataType,
                                                              HandlerType> >;

 public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   Net_SocketConfiguration_t,
                                   Net_Configuration_t,
                                   UserDataType,
                                   Stream_Statistic_t> ICONNECTION_MANAGER_T;

  Net_AsynchUDPConnection_T (ICONNECTION_MANAGER_T*, // connection manager handle
                             unsigned int = 0);      // statistics collecting interval (second(s))
                                                     // 0 --> DON'T collect statistics
  virtual ~Net_AsynchUDPConnection_T ();

  // override / implement (part of) Net_IInetTransportLayer
  //using Net_AsynchSocketConnectionBase_T::inherited::Net_ConnectionBase_T::initialize;
  //using Net_AsynchSocketConnectionBase_T::inherited::Net_ConnectionBase_T::finalize;
//  using Net_AsynchSocketConnectionBase_T::info;
  //virtual void info (ACE_HANDLE&,           // return value: handle
  //                   ACE_INET_Addr&,        // return value: local SAP
  //                   ACE_INET_Addr&) const; // return value: remote SAP

 private:
  typedef Net_AsynchSocketConnectionBase_T<ACE_INET_Addr,
                                           Net_SocketConfiguration_t,
                                           HandlerType,
                                           Net_Configuration_t,
                                           Net_SocketHandlerConfiguration_t,
                                           UserDataType,
                                           SessionDataType,
                                           Stream_Statistic_t> inherited;
  typedef Net_TransportLayer_UDP inherited2;

  Net_AsynchUDPConnection_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPConnection_T (const Net_AsynchUDPConnection_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPConnection_T& operator= (const Net_AsynchUDPConnection_T&));
};

// include template implementation
#include "net_udpconnection.inl"

#endif
