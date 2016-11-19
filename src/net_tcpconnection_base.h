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

#include <ace/Acceptor.h>
#include <ace/Asynch_Acceptor.h>
#include <ace/Asynch_Connector.h>
#include <ace/Connector.h>
#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Connector.h>
#include <ace/Time_Value.h>

#include "stream_statemachine_control.h"

#include "net_iconnectionmanager.h"
#include "net_sock_acceptor.h"
#include "net_sock_connector.h"
#include "net_streamconnection_base.h"
#include "net_transportlayer_tcp.h"

template <typename HandlerType,
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_TCPConnectionBase_T
 : public Net_StreamConnectionBase_T<HandlerType,
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     struct Net_SocketConfiguration,
                                     HandlerConfigurationType,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     UserDataType>
 , public Net_TransportLayer_TCP
{
  friend class ACE_Acceptor<Net_TCPConnectionBase_T<HandlerType,
                                                    ConfigurationType,
                                                    StateType,
                                                    StatisticContainerType,
                                                    HandlerConfigurationType,
                                                    StreamType,
                                                    UserDataType>,
                            ACE_SOCK_ACCEPTOR>;
  friend class ACE_Acceptor<Net_TCPConnectionBase_T<HandlerType,
                                                    ConfigurationType,
                                                    StateType,
                                                    StatisticContainerType,
                                                    HandlerConfigurationType,
                                                    StreamType,
                                                    UserDataType>,
                            Net_SOCK_Acceptor>;
  friend class ACE_Connector<Net_TCPConnectionBase_T<HandlerType,
                                                     ConfigurationType,
                                                     StateType,
                                                     StatisticContainerType,
                                                     HandlerConfigurationType,
                                                     StreamType,
                                                     UserDataType>,
                             ACE_SOCK_CONNECTOR>;
  friend class ACE_Connector<Net_TCPConnectionBase_T<HandlerType,
                                                     ConfigurationType,
                                                     StateType,
                                                     StatisticContainerType,
                                                     HandlerConfigurationType,
                                                     StreamType,
                                                     UserDataType>,
                             Net_SOCK_Connector>;

 public:
  typedef Net_StreamConnectionBase_T<HandlerType,
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     struct Net_SocketConfiguration,
                                     HandlerConfigurationType,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     UserDataType> STREAM_CONNECTION_BASE_T;

  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_TCPConnectionBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                           const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  virtual ~Net_TCPConnectionBase_T ();

 private:
  typedef Net_StreamConnectionBase_T<HandlerType,
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     struct Net_SocketConfiguration,
                                     HandlerConfigurationType,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     UserDataType> inherited;
  typedef Net_TransportLayer_TCP inherited2;

  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_svc_handler() method of ACE_Connector/ACE_Acceptor
  Net_TCPConnectionBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnectionBase_T (const Net_TCPConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnectionBase_T& operator= (const Net_TCPConnectionBase_T&))
};

//////////////////////////////////////////

template <typename HandlerType,
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_AsynchTCPConnectionBase_T
 : public Net_AsynchStreamConnectionBase_T<HandlerType,
                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           struct Net_SocketConfiguration,
                                           HandlerConfigurationType,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           UserDataType>
 , public Net_TransportLayer_TCP
{
  friend class ACE_Asynch_Acceptor<Net_AsynchTCPConnectionBase_T<HandlerType,
                                                                 ConfigurationType,
                                                                 StateType,
                                                                 StatisticContainerType,
                                                                 HandlerConfigurationType,
                                                                 StreamType,
                                                                 UserDataType> >;
  friend class ACE_Asynch_Connector<Net_AsynchTCPConnectionBase_T<HandlerType,
                                                                  ConfigurationType,
                                                                  StateType,
                                                                  StatisticContainerType,
                                                                  HandlerConfigurationType,
                                                                  StreamType,
                                                                  UserDataType> >;

 public:
  typedef Net_AsynchStreamConnectionBase_T<HandlerType,
                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           struct Net_SocketConfiguration,
                                           HandlerConfigurationType,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           UserDataType> STREAM_CONNECTION_BASE_T;

  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_AsynchTCPConnectionBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                                 const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  virtual ~Net_AsynchTCPConnectionBase_T ();

 private:
  typedef Net_AsynchStreamConnectionBase_T<HandlerType,
                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           struct Net_SocketConfiguration,
                                           HandlerConfigurationType,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           UserDataType> inherited;
  typedef Net_TransportLayer_TCP inherited2;

  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_handler() method of ACE_AsynchConnector/ACE_AsynchAcceptor
  Net_AsynchTCPConnectionBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnectionBase_T (const Net_AsynchTCPConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnectionBase_T& operator= (const Net_AsynchTCPConnectionBase_T&))
};

// include template definition
#include "net_tcpconnection_base.inl"

#endif
