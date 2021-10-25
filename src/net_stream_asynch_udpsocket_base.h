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

#ifndef NET_STREAM_ASYNCH_UDPSOCKET_BASE_H
#define NET_STREAM_ASYNCH_UDPSOCKET_BASE_H

#include "ace/Asynch_IO.h"
#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/SOCK_Dgram.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnection.h"
#if defined (NETLINK_SUPPORT)
#include "net_asynch_netlinksockethandler.h"
#include "net_netlinksockethandler.h"
#endif // NETLINK_SUPPORT

// forward declarations
class ACE_Notification_Strategy;

template <typename HandlerType,
          ///////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename UserDataType>
class Net_StreamAsynchUDPSocketBase_T
 : public HandlerType
 , public ACE_Svc_Handler<ACE_SOCK_DGRAM, ACE_NULL_SYNCH>
 , virtual public Net_ISocketConnection_T<AddressType,
                                          ConfigurationType,
                                          StateType,
                                          StatisticContainerType,
                                          HandlerConfigurationType>
{
  typedef HandlerType inherited;
  typedef ACE_Svc_Handler<ACE_SOCK_DGRAM, ACE_NULL_SYNCH> inherited2;

 public:
  inline virtual ~Net_StreamAsynchUDPSocketBase_T () {};

  // implement Common_IReset
  // *NOTE*: use this to modify the source/target address after initialization
  virtual void reset ();

  // override some ACE_Service_Handler methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  int close (u_long = 0); // reason

  // implement/override some ACE_Svc_Handler methods
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE); // (socket) handle
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,                        // (socket) handle
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK); // (select) mask

  // implement (part of) Net_ISocketConnection_T
  virtual void dump_state () const;
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
  virtual Net_ConnectionId_t id () const;
  inline virtual ACE_Notification_Strategy* notification () { return this; };
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?

 protected:
  // convenient types
  typedef HandlerType HANDLER_T;

  Net_StreamAsynchUDPSocketBase_T ();

 private:
  // convenient types
  typedef Net_ISocketConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  HandlerConfigurationType> ISOCKET_CONNECTION_T;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T (const Net_StreamAsynchUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T& operator= (const Net_StreamAsynchUDPSocketBase_T&))

  // helper methods
  virtual void handle_read_dgram (const ACE_Asynch_Read_Dgram::Result&); // result
  virtual void handle_write_dgram (const ACE_Asynch_Write_Dgram::Result&); // result
};

//////////////////////////////////////////

#if defined (NETLINK_SUPPORT)
// partial specialization (for Netlink)
template <typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename UserDataType>
class Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                      Net_Netlink_Addr,
                                      ConfigurationType,
                                      StateType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      HandlerConfigurationType,
                                      UserDataType>
 : public Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>
 , public Net_SOCK_Netlink
 , public ACE_Svc_Handler<Net_SOCK_Netlink, ACE_NULL_SYNCH>
 , virtual public Net_ISocketConnection_T<Net_Netlink_Addr,
                                          ConfigurationType,
                                          StateType,
                                          StatisticContainerType,
                                          HandlerConfigurationType>
{
  typedef Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType> inherited;
  typedef Net_SOCK_Netlink inherited2;
  typedef ACE_Svc_Handler<Net_SOCK_Netlink,
                          ACE_NULL_SYNCH> inherited3;

 public:
  inline virtual ~Net_StreamAsynchUDPSocketBase_T () {};

  // override some ACE_Service_Handler methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  int close (u_long = 0); // reason

  // implement/override some ACE_Event_Handler methods
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE); // (socket) handle
  virtual int handle_close (ACE_HANDLE,        // (socket) handle
                            ACE_Reactor_Mask); // (select) mask

  // implement (part of) Net_ISocketConnection_T
  virtual void dump_state () const;
  virtual void info (ACE_HANDLE&,              // return value: handle
                     Net_Netlink_Addr&,        // return value: local SAP
                     Net_Netlink_Addr&) const; // return value: remote SAP
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited2::get_handle ()); };
  inline virtual ACE_Notification_Strategy* notification () { return this; };
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for any worker
                                                // thread(s) ?

 protected:
  // convenient types
  typedef Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType> HANDLER_T;

  Net_StreamAsynchUDPSocketBase_T ();

 private:
  // convenient types
  typedef Net_ISocketConnection_T<Net_Netlink_Addr,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  HandlerConfigurationType> ISOCKET_CONNECTION_T;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T (const Net_StreamAsynchUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T& operator= (const Net_StreamAsynchUDPSocketBase_T&))

  // helper methods
  virtual void handle_read_dgram (const ACE_Asynch_Read_Dgram::Result&); // result
  virtual void handle_write_dgram (const ACE_Asynch_Write_Dgram::Result&); // result
};
#endif // NETLINK_SUPPORT

// include template definition
#include "net_stream_asynch_udpsocket_base.inl"

#endif
