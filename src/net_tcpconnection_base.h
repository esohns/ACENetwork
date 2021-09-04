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
#include "ace/Time_Value.h"

#if defined (SSL_SUPPORT)
#include "ace/SSL/SSL_SOCK_Connector.h"
#endif // SSL_SUPPORT

#include "stream_statemachine_control.h"

#include "net_iconnectionmanager.h"
#include "net_sock_acceptor.h"
#include "net_sock_connector.h"
#include "net_socket_common.h"
#include "net_streamconnection_base.h"
#include "net_transportlayer_tcp.h"

template <ACE_SYNCH_DECL, // 'send' lock strategy
          typename SocketHandlerType, // implements Net_TCPSocketHandler_T
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_TCPConnectionBase_T
 : public Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                                     SocketHandlerType,
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     Net_TCPSocketConfiguration_t,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     UserDataType>
 , public Net_TransportLayer_TCP
{
  typedef Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                                     SocketHandlerType,
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     Net_TCPSocketConfiguration_t,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     UserDataType> inherited;
  typedef Net_TransportLayer_TCP inherited2;

  friend class ACE_Acceptor<Net_TCPConnectionBase_T<ACE_SYNCH_USE,
                                                    SocketHandlerType,
                                                    ConfigurationType,
                                                    StateType,
                                                    StatisticContainerType,
                                                    StreamType,
                                                    UserDataType>,
                            ACE_SOCK_ACCEPTOR>;
  friend class ACE_Acceptor<Net_TCPConnectionBase_T<ACE_SYNCH_USE,
                                                    SocketHandlerType,
                                                    ConfigurationType,
                                                    StateType,
                                                    StatisticContainerType,
                                                    StreamType,
                                                    UserDataType>,
                            Net_SOCK_Acceptor>;
  friend class ACE_Connector<Net_TCPConnectionBase_T<ACE_SYNCH_USE,
                                                     SocketHandlerType,
                                                     ConfigurationType,
                                                     StateType,
                                                     StatisticContainerType,
                                                     StreamType,
                                                     UserDataType>,
                             ACE_SOCK_CONNECTOR>;
  friend class ACE_Connector<Net_TCPConnectionBase_T<ACE_SYNCH_USE,
                                                     SocketHandlerType,
                                                     ConfigurationType,
                                                     StateType,
                                                     StatisticContainerType,
                                                     StreamType,
                                                     UserDataType>,
                             Net_SOCK_Connector>;
#if defined (SSL_SUPPORT)
  friend class ACE_Connector<Net_TCPConnectionBase_T<ACE_SYNCH_USE,
                                                     SocketHandlerType,
                                                     ConfigurationType,
                                                     StateType,
                                                     StatisticContainerType,
                                                     StreamType,
                                                     UserDataType>,
                             ACE_SSL_SOCK_Connector>;
#endif // SSL_SUPPORT

 public:
  // convenient types
  typedef Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                                     SocketHandlerType,
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     Net_TCPSocketConfiguration_t,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     UserDataType> STREAM_CONNECTION_BASE_T;

  Net_TCPConnectionBase_T (bool); // managed ?
  inline virtual ~Net_TCPConnectionBase_T () {}

  // implement (part of) Net_ITransportLayer_T
  // *TODO*: these shouldn't be necessary, remove ASAP
  inline virtual enum Common_EventDispatchType dispatch () { return inherited2::dispatch_; }
  inline virtual enum Net_ClientServerRole role () { return inherited2::role_; }
  inline virtual void set (enum Net_ClientServerRole role_in) { inherited2::role_ = role_in; }
  inline virtual bool initialize (enum Common_EventDispatchType dispatch_in, enum Net_ClientServerRole role_in, const Net_TCPSocketConfiguration_t&) { inherited2::dispatch_ = dispatch_in; inherited2::role_ = role_in; return true; }
  inline virtual void finalize () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual enum Net_TransportLayerType transportLayer () { return inherited2::transportLayer_; }

  // override some ACE_Event_Handler methods
  // *NOTE*: stream any received data for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: send stream data to the peer
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);

  // connector complains
  using inherited::SVC_HANDLER_T::reactor;

 protected:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Net_TCPConnectionBase_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnectionBase_T (const Net_TCPConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnectionBase_T& operator= (const Net_TCPConnectionBase_T&))
};

//////////////////////////////////////////

template <typename SocketHandlerType, // implements Net_AsynchTCPSocketHandler_T
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_AsynchTCPConnectionBase_T
 : public Net_AsynchStreamConnectionBase_T<SocketHandlerType,
                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           Net_TCPSocketConfiguration_t,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           UserDataType>
 , public Net_TransportLayer_TCP
{
  typedef Net_AsynchStreamConnectionBase_T<SocketHandlerType,
                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           Net_TCPSocketConfiguration_t,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           UserDataType> inherited;
  typedef Net_TransportLayer_TCP inherited2;

  friend class ACE_Asynch_Acceptor<Net_AsynchTCPConnectionBase_T<SocketHandlerType,
                                                                 ConfigurationType,
                                                                 StateType,
                                                                 StatisticContainerType,
                                                                 StreamType,
                                                                 UserDataType> >;
  friend class ACE_Asynch_Connector<Net_AsynchTCPConnectionBase_T<SocketHandlerType,
                                                                  ConfigurationType,
                                                                  StateType,
                                                                  StatisticContainerType,
                                                                  StreamType,
                                                                  UserDataType> >;

 public:
  // convenient types
  typedef Net_AsynchStreamConnectionBase_T<SocketHandlerType,
                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           Net_TCPSocketConfiguration_t,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           UserDataType> STREAM_CONNECTION_BASE_T;

  Net_AsynchTCPConnectionBase_T (bool); // managed ?
  inline virtual ~Net_AsynchTCPConnectionBase_T () {}

  // implement (part of) Net_ITransportLayer_T
  // *TODO*: these shouldn't be necessary, remove ASAP
  inline virtual enum Common_EventDispatchType dispatch () { return inherited2::dispatch_; }
  inline virtual enum Net_ClientServerRole role () { return inherited2::role_; }
  inline virtual void set (enum Net_ClientServerRole role_in) { inherited2::role_ = role_in; }
  inline virtual bool initialize (enum Common_EventDispatchType dispatch_in, enum Net_ClientServerRole role_in, const Net_TCPSocketConfiguration_t&) { inherited2::dispatch_ = dispatch_in; inherited2::role_ = role_in; return true; }
  inline virtual void finalize () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual enum Net_TransportLayerType transportLayer () { return inherited2::transportLayer_; }

  // override (part of) ACE_Service_Handler
  virtual void open (ACE_HANDLE,          // handle
                     ACE_Message_Block&); // (initial) data (if any)

  // override some ACE_Event_Handler methods
  // *NOTE*: send stream data to the peer
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);

  // implement (part of) Net_IStreamConnection_T
  inline virtual void info (ACE_HANDLE& handle_out, ACE_INET_Addr& localSAP_out, ACE_INET_Addr& peerSAP_out) const { handle_out = inherited::handle (); localSAP_out = inherited::localSAP_; peerSAP_out = inherited::peerSAP_; };

 protected:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Net_AsynchTCPConnectionBase_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // *IMPORTANT NOTE*: supports synchronicity-agnostic connections
  inline virtual int open (void* = NULL) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnectionBase_T (const Net_AsynchTCPConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnectionBase_T& operator= (const Net_AsynchTCPConnectionBase_T&))
};

// include template definition
#include "net_tcpconnection_base.inl"

#endif
