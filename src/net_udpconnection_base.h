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
#include "ace/Time_Value.h"

#include "stream_statemachine_control.h"

#include "net_configuration.h"
#include "net_iconnectionmanager.h"
#include "net_socket_common.h"
#include "net_streamconnection_base.h"
#include "net_transportlayer_udp.h"

template <ACE_SYNCH_DECL, // 'send' lock strategy
          typename SocketHandlerType, // implements Net_UDPSocketHandler_T
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType>
class Net_UDPConnectionBase_T
 : public Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                                     SocketHandlerType,
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     Net_UDPSocketConfiguration_t,
                                     Net_UDPSocketConfiguration_t,
                                     Net_ListenerConfiguration_T<ConfigurationType,
                                                                 NET_TRANSPORTLAYER_UDP>,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     TimerManagerType,
                                     UserDataType>
 , public Net_TransportLayer_UDP
{
  typedef Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                                     SocketHandlerType,
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     Net_UDPSocketConfiguration_t,
                                     Net_UDPSocketConfiguration_t,
                                     Net_ListenerConfiguration_T<ConfigurationType,
                                                                 NET_TRANSPORTLAYER_UDP>,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     TimerManagerType,
                                     UserDataType> inherited;
  typedef Net_TransportLayer_UDP inherited2;

  friend class ACE_Connector<Net_UDPConnectionBase_T<ACE_SYNCH_USE,
                                                     SocketHandlerType,
                                                     ConfigurationType,
                                                     StateType,
                                                     StatisticContainerType,
                                                     StreamType,
                                                     TimerManagerType,
                                                     UserDataType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  // convenient types
  typedef Net_StreamConnectionBase_T<ACE_SYNCH_USE,
                                     SocketHandlerType,
                                     ACE_INET_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     Net_UDPSocketConfiguration_t,
                                     Net_UDPSocketConfiguration_t,
                                     Net_ListenerConfiguration_T<ConfigurationType,
                                                                 NET_TRANSPORTLAYER_UDP>,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     TimerManagerType,
                                     UserDataType> STREAM_CONNECTION_BASE_T;

  Net_UDPConnectionBase_T (bool); // managed ?
  inline virtual ~Net_UDPConnectionBase_T () {}

  // implement (part of) Net_ITransportLayer_T
  // *TODO*: these shouldn't be necessary, remove ASAP
  inline virtual enum Common_EventDispatchType dispatch () { return inherited2::dispatch_; }
  inline virtual enum Net_ClientServerRole role () { return inherited2::role_; }
  inline virtual void set (enum Net_ClientServerRole role_in) { inherited2::role_ = role_in; }
  inline virtual bool initialize (enum Common_EventDispatchType dispatch_in, enum Net_ClientServerRole role_in, const Net_UDPSocketConfiguration_t&) { inherited2::dispatch_ = dispatch_in; inherited2::role_ = role_in; return true; }
  inline virtual void finalize () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual enum Net_TransportLayerType transportLayer () { return inherited2::transportLayer_; }

  // override some ACE_Event_Handler methods
  // *NOTE*: stream any received data for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: send stream data to the peer
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);

  // implement (part of) Net_IStreamConnection_T
  // *IMPORTANT NOTE*: for write-only connections, this returns the outbound
  //                   socket handle, else the inbound socket handle
  virtual void info (ACE_HANDLE&,           // return value: handle
                     ACE_INET_Addr&,        // return value: local SAP
                     ACE_INET_Addr&) const; // return value: peer SAP

  // implement Common_IReset
  virtual void reset ();

 protected:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Net_UDPConnectionBase_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_UDPConnectionBase_T (const Net_UDPConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_UDPConnectionBase_T& operator= (const Net_UDPConnectionBase_T&))

  // helper method(s)
#if defined (ACE_LINUX)
  void processErrorQueue ();
#endif
};

//////////////////////////////////////////

template <typename SocketHandlerType, // implements Net_AsynchUDPSocketHandler_T
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType>
class Net_AsynchUDPConnectionBase_T
 : public Net_AsynchStreamConnectionBase_T<SocketHandlerType,
                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           Net_UDPSocketConfiguration_t,
                                           Net_UDPSocketConfiguration_t,
                                           Net_ListenerConfiguration_T<ConfigurationType,
                                                                       NET_TRANSPORTLAYER_UDP>,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           TimerManagerType,
                                           UserDataType>
 , public Net_TransportLayer_UDP
{
  typedef Net_AsynchStreamConnectionBase_T<SocketHandlerType,
                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           Net_UDPSocketConfiguration_t,
                                           Net_UDPSocketConfiguration_t,
                                           Net_ListenerConfiguration_T<ConfigurationType,
                                                                       NET_TRANSPORTLAYER_UDP>,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           TimerManagerType,
                                           UserDataType> inherited;
  typedef Net_TransportLayer_UDP inherited2;

  friend class ACE_Asynch_Connector<Net_AsynchUDPConnectionBase_T<SocketHandlerType,
                                                                  ConfigurationType,
                                                                  StateType,
                                                                  StatisticContainerType,
                                                                  StreamType,
                                                                  TimerManagerType,
                                                                  UserDataType> >;

 public:
  // convenient types
  typedef Net_AsynchStreamConnectionBase_T<SocketHandlerType,
                                           ACE_INET_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           Net_UDPSocketConfiguration_t,
                                           Net_UDPSocketConfiguration_t,
                                           Net_ListenerConfiguration_T<ConfigurationType,
                                                                       NET_TRANSPORTLAYER_UDP>,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           TimerManagerType,
                                           UserDataType> STREAM_CONNECTION_BASE_T;

  Net_AsynchUDPConnectionBase_T (bool); // managed ?
  inline virtual ~Net_AsynchUDPConnectionBase_T () {}

  // implement (part of) Net_ITransportLayer_T
  // *TODO*: these shouldn't be necessary, remove ASAP
  inline virtual enum Common_EventDispatchType dispatch () { return inherited2::dispatch_; }
  inline virtual enum Net_ClientServerRole role () { return inherited2::role_; }
  inline virtual void set (enum Net_ClientServerRole role_in) { inherited2::role_ = role_in; }
  inline virtual bool initialize (enum Common_EventDispatchType dispatch_in, enum Net_ClientServerRole role_in, const Net_UDPSocketConfiguration_t&) { inherited2::dispatch_ = dispatch_in; inherited2::role_ = role_in; return true; }
  inline virtual void finalize () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual enum Net_TransportLayerType transportLayer () { return inherited2::transportLayer_; }

  // override (part of) ACE_Service_Handler
  virtual void open (ACE_HANDLE,          // handle
                     ACE_Message_Block&); // (initial) data (if any)

  // override some ACE_Event_Handler methods
  // *NOTE*: send stream data to the peer
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);

  // implement (part of) Net_IStreamConnection_T
  // *IMPORTANT NOTE*: for write-only connections, this returns the outbound
  //                   socket handle, else the inbound socket handle
  virtual void info (ACE_HANDLE&,           // return value: handle
                     ACE_INET_Addr&,        // return value: local SAP
                     ACE_INET_Addr&) const; // return value: peer SAP

  // implement Common_IReset
  virtual void reset ();

 protected:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Net_AsynchUDPConnectionBase_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPConnectionBase_T (const Net_AsynchUDPConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchUDPConnectionBase_T& operator= (const Net_AsynchUDPConnectionBase_T&))

  //using inherited2::initialize;

  // override (part of) Net_IAsynchSocketHandler
  virtual bool initiate_read ();

  // helper method(s)
#if defined (ACE_LINUX)
  void processErrorQueue ();
#endif
};

// include template definition
#include "net_udpconnection_base.inl"

#endif
