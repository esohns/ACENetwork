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

#ifndef NET_NETLINKCONNECTION_H
#define NET_NETLINKCONNECTION_H

#include "ace/Asynch_Connector.h"
#include "ace/Connector.h"
#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/SOCK_Connector.h"

#include "common_timer_manager_common.h"

#include "stream_statemachine_control.h"

#include "net_common.h"
#include "net_iconnectionmanager.h"
#include "net_netlinksockethandler.h"
#include "net_streamconnection_base.h"
#include "net_transportlayer_netlink.h"

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
template <typename HandlerType,
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_NetlinkConnection_T
 : public Net_StreamConnectionBase_T<ACE_NULL_SYNCH,
                                     HandlerType,
                                     Net_Netlink_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     Net_NetlinkSocketConfiguration_t,
                                     Net_NetlinkSocketConfiguration_t,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     UserDataType>
 , public Net_TransportLayer_Netlink
{
  typedef Net_StreamConnectionBase_T<ACE_NULL_SYNCH,
                                     HandlerType,
                                     Net_Netlink_Addr,
                                     ConfigurationType,
                                     StateType,
                                     StatisticContainerType,
                                     Net_NetlinkSocketConfiguration_t,
                                     Net_NetlinkSocketConfiguration_t,
                                     StreamType,
                                     enum Stream_StateMachine_ControlState,
                                     UserDataType> inherited;
  typedef Net_TransportLayer_Netlink inherited2;

  friend class ACE_Connector<Net_NetlinkConnection_T<HandlerType,
                                                     ConfigurationType,
                                                     StateType,
                                                     StatisticContainerType,
                                                     StreamType,
                                                     UserDataType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  Net_NetlinkConnection_T (bool); // managed ?
  inline virtual ~Net_NetlinkConnection_T () {}

  // override some task-based members
//   virtual int open (void* = NULL); // args
   //  // *NOTE*: enqueue any received data onto our stream for further processing
   //   virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
   //  // *NOTE*: send any enqueued data back to the client...
   //  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
   // *NOTE*: this is called when:
   // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

   // override / implement (part of) Net_INetlinkTransportLayer
//   virtual bool initialize (Common_DispatchType,             // dispatch
//                            Net_ClientServerRole,            // role
//                            const Net_SocketConfiguration&); // socket configuration
//   virtual void finalize ();
  virtual void info (ACE_HANDLE&,              // return value: handle
                     Net_Netlink_Addr&,        // return value: local SAP
                     Net_Netlink_Addr&) const; // return value: remote SAP
  virtual Net_ConnectionId_t id () const;
  virtual void dump_state () const;

 protected:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Net_NetlinkConnection_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkConnection_T (const Net_NetlinkConnection_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_NetlinkConnection_T& operator= (const Net_NetlinkConnection_T&))
};

//////////////////////////////////////////

template <typename HandlerType,
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_AsynchNetlinkConnection_T
 : public Net_AsynchStreamConnectionBase_T<HandlerType,
                                           Net_Netlink_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           Net_NetlinkSocketConfiguration_t,
                                           Net_NetlinkSocketConfiguration_t,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           UserDataType>
 , public Net_TransportLayer_Netlink
{
  typedef Net_AsynchStreamConnectionBase_T<HandlerType,
                                           Net_Netlink_Addr,
                                           ConfigurationType,
                                           StateType,
                                           StatisticContainerType,
                                           Net_NetlinkSocketConfiguration_t,
                                           Net_NetlinkSocketConfiguration_t,
                                           StreamType,
                                           enum Stream_StateMachine_ControlState,
                                           UserDataType> inherited;
  typedef Net_TransportLayer_Netlink inherited2;

  friend class ACE_Asynch_Connector<Net_AsynchNetlinkConnection_T<HandlerType,
                                                                  ConfigurationType,
                                                                  StateType,
                                                                  StatisticContainerType,
                                                                  StreamType,
                                                                  UserDataType> >;

 public:
  Net_AsynchNetlinkConnection_T (bool); // managed ?
  inline virtual ~Net_AsynchNetlinkConnection_T () {}

  // implement (part of) Net_INetlinkTransportLayer
//  virtual bool initialize (Common_DispatchType,             // dispatch
//                           Net_ClientServerRole,            // role
//                           const Net_SocketConfiguration&); // configuration
//  virtual void finalize ();
//  virtual void info (ACE_HANDLE&,              // return value: handle
//                     Net_Netlink_Addr&,        // return value: local SAP
//                     Net_Netlink_Addr&) const; // return value: remote SAP
  virtual Net_ConnectionId_t id () const;
  virtual void dump_state () const;

  // override some ACE_Service_Handler members
  virtual void open (ACE_HANDLE,          // handle
                     ACE_Message_Block&); // (initial) data (if any)
  //  // *NOTE*: enqueue any received data onto our stream for further processing
  //   virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
  //  // *NOTE*: send any enqueued data back to the client...
  //  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
    // *NOTE*: this is called when:
    // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

 protected:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Net_AsynchNetlinkConnection_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchNetlinkConnection_T (const Net_AsynchNetlinkConnection_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchNetlinkConnection_T& operator= (const Net_AsynchNetlinkConnection_T&))
};

// include template definition
#include "net_netlinkconnection.inl"
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT

#endif
