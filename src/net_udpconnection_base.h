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

#ifndef NET_UDPCONNECTION_BASE_H
#define NET_UDPCONNECTION_BASE_H

#include "ace/Asynch_Connector.h"
#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"

#include "net_iconnectionmanager.h"
#include "net_socketconnection_base.h"
#include "net_transportlayer_udp.h"

template <typename HandlerType,
          ///////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          ///////////////////////////////
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename UserDataType>
class Net_UDPConnectionBase_T
 : public Net_SocketConnectionBase_T<HandlerType,
                                     ////
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     StreamType,
                                     ////
                                     Net_SocketConfiguration,
                                     ////
                                     HandlerConfigurationType,
                                     ////
                                     UserDataType>
 , public Net_TransportLayer_UDP
{
  friend class ACE_Connector<Net_UDPConnectionBase_T<HandlerType,

                                                     ConfigurationType,
                                                     StateType,
                                                     StatisticContainerType,
                                                     StreamType,

                                                     HandlerConfigurationType,

                                                     UserDataType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_UDPConnectionBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                           unsigned int = 0);      // statistic collecting interval (second(s)) [0: off]
  virtual ~Net_UDPConnectionBase_T ();

  // override / implement (part of) Net_IInetTransportLayer
  //typedef Net_ConnectionBase_T<ACE_INET_Addr,
  //                             Net_Configuration,
  //                             StateType,
  //                             Stream_Statistic,
  //                             Net_Stream,
  //                             //////////
  //                             UserDataType> CONNECTION_BASE_T;
  //using CONNECTION_BASE_T::get;

  //using Net_SocketConnectionBase_T::inherited::Net_ConnectionBase_T::initialize;
  //using Net_SocketConnectionBase_T::inherited::Net_ConnectionBase_T::finalize;
//  using Net_SocketConnectionBase_T::info;
  //virtual void info (ACE_HANDLE&,           // return value: handle
  //                   ACE_INET_Addr&,        // return value: local SAP
  //                   ACE_INET_Addr&) const; // return value: remote SAP

  //// override / implement (part of) Net_IConnection_T
  //virtual int close (u_long = 0); // reason

 private:
  typedef Net_SocketConnectionBase_T<HandlerType,
                                     ////
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     StreamType,
                                     ////
                                     Net_SocketConfiguration,
                                     HandlerConfigurationType,
                                     ////
                                     UserDataType> inherited;
  typedef Net_TransportLayer_UDP inherited2;

  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_svc_handler() method of ACE_Connector/ACE_Acceptor
  Net_UDPConnectionBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_UDPConnectionBase_T (const Net_UDPConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_UDPConnectionBase_T& operator= (const Net_UDPConnectionBase_T&))
};

/////////////////////////////////////////

template <typename HandlerType,
          ///////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          ///////////////////////////////
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename UserDataType>
class Net_AsynchUDPConnectionBase_T
 : public Net_AsynchSocketConnectionBase_T<HandlerType,

                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           StreamType,

                                           Net_SocketConfiguration,

                                           HandlerConfigurationType,

                                           UserDataType>
 , public Net_TransportLayer_UDP
{
  friend class ACE_Asynch_Connector<Net_AsynchUDPConnectionBase_T<HandlerType,

                                                                  ConfigurationType,
                                                                  StateType,
                                                                  StatisticContainerType,
                                                                  StreamType,

                                                                  HandlerConfigurationType,

                                                                  UserDataType> >;

 public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_AsynchUDPConnectionBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                                 unsigned int = 0);      // statistic collecting interval (second(s)) [0:off]
  virtual ~Net_AsynchUDPConnectionBase_T ();

  // override / implement (part of) Net_IInetTransportLayer
  //using Net_AsynchSocketConnectionBase_T::inherited::Net_ConnectionBase_T::initialize;
  //using Net_AsynchSocketConnectionBase_T::inherited::Net_ConnectionBase_T::finalize;
//  using Net_AsynchSocketConnectionBase_T::info;
  //virtual void info (ACE_HANDLE&,           // return value: handle
  //                   ACE_INET_Addr&,        // return value: local SAP
  //                   ACE_INET_Addr&) const; // return value: remote SAP

 private:
  typedef Net_AsynchSocketConnectionBase_T<HandlerType,

                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           StreamType,

                                           Net_SocketConfiguration,

                                           HandlerConfigurationType,

                                           UserDataType> inherited;
  typedef Net_TransportLayer_UDP inherited2;

  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_handler() method of ACE_AsynchConnector/ACE_AsynchAcceptor
  Net_AsynchUDPConnectionBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPConnectionBase_T (const Net_AsynchUDPConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPConnectionBase_T& operator= (const Net_AsynchUDPConnectionBase_T&))
};

// include template implementation
#include "net_udpconnection_base.inl"

#endif
