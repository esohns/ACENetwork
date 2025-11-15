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

#ifndef NET_STREAM_UDPSOCKET_BASE_H
#define NET_STREAM_UDPSOCKET_BASE_H

#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnection.h"
#if defined (NETLINK_SUPPORT)
#include "net_netlinksockethandler.h"
#endif // NETLINK_SUPPORT
#include "net_udpsockethandler.h"

// forward declarations
class ACE_Notification_Strategy;

template <typename HandlerType,
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename UserDataType>
class Net_StreamUDPSocketBase_T
 : public HandlerType
 , virtual public Net_ISocketConnection_T<AddressType,
                                          ConfigurationType,
                                          StateType,
                                          StatisticContainerType,
                                          HandlerConfigurationType>
{
  typedef HandlerType inherited;

  friend class ACE_Connector<Net_StreamUDPSocketBase_T<HandlerType,
                                                       AddressType,
                                                       ConfigurationType,
                                                       StateType,
                                                       StatisticContainerType,
                                                       TimerManagerType,
                                                       HandlerConfigurationType,
                                                       UserDataType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  virtual ~Net_StreamUDPSocketBase_T ();

  // implement/override some ACE_Event_Handler methods
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE,
                            ACE_Reactor_Mask);

  // implement (part of) Net_ISocketConnection_T
  virtual void dump_state () const;
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); }
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); }
#endif
  inline virtual ACE_Notification_Strategy* notification () { ACE_ASSERT (false); return NULL; }
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?

  using inherited::initialize;
  using inherited::send;

 protected:
  // convenient types
  typedef HandlerType HANDLER_T;

  Net_StreamUDPSocketBase_T ();

  // override some ACE_Svc_Handler members
  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // args (reason)

  // helper method(s)
#if defined (ACE_LINUX)
  void processErrorQueue ();
#endif // ACE_LINUX

  ACE_Message_Block* writeBuffer_;
  ACE_SYNCH_MUTEX    sendLock_;
  // *IMPORTANT NOTE*: in a threaded environment, workers may (!) dispatch the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  bool               serializeOutput_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T (const Net_StreamUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T& operator= (const Net_StreamUDPSocketBase_T&))
};

//////////////////////////////////////////

// partial specialization (for connected sockets)
template <typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename UserDataType>
class Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                       HandlerConfigurationType,
                                                       Net_SOCK_CODgram>,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>
 : public Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                 HandlerConfigurationType,
                                 Net_SOCK_CODgram>
 , virtual public Net_ISocketConnection_T<AddressType,
                                          ConfigurationType,
                                          StateType,
                                          StatisticContainerType,
                                          HandlerConfigurationType>
{
  typedef Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                 HandlerConfigurationType,
                                 Net_SOCK_CODgram> inherited;

  friend class ACE_Connector<Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                                                              HandlerConfigurationType,
                                                                              Net_SOCK_CODgram>,
                                                       AddressType,
                                                       ConfigurationType,
                                                       StateType,
                                                       StatisticContainerType,
                                                       TimerManagerType,
                                                       HandlerConfigurationType,
                                                       UserDataType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  virtual ~Net_StreamUDPSocketBase_T ();

  // implement/override some ACE_Event_Handler methods
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE,
                            ACE_Reactor_Mask);

  // implement (part of) Net_ISocketConnection_T
  virtual void dump_state () const;
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); }
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); }
#endif
  inline virtual ACE_Notification_Strategy* notification () { return inherited::configuration_->streamConfiguration->configuration_.notificationStrategy; }
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?

  using inherited::initialize;

 protected:
  // convenient types
  typedef Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                                 HandlerConfigurationType,
                                 Net_SOCK_CODgram> HANDLER_T;

  Net_StreamUDPSocketBase_T ();

  // override some ACE_Svc_Handler members
  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // args (reason)

  // helper method(s)
#if defined (ACE_LINUX)
  void processErrorQueue ();
#endif // ACE_LINUX

  ACE_Message_Block* writeBuffer_;
  ACE_SYNCH_MUTEX    sendLock_;
  // *IMPORTANT NOTE*: in a threaded environment, workers may (!) dispatch the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  bool               serializeOutput_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T (const Net_StreamUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T& operator= (const Net_StreamUDPSocketBase_T&))
};

//////////////////////////////////////////

#if defined (NETLINK_SUPPORT)
// partial specialization (for Netlink)
template <typename ConfigurationType, // connection-
          typename StateType,
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename UserDataType>
class Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                                Net_Netlink_Addr,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                TimerManagerType,
                                HandlerConfigurationType,
                                UserDataType>
 : public Net_NetlinkSocketHandler_T<HandlerConfigurationType>
 , virtual public Net_ISocketConnection_T<Net_Netlink_Addr,
                                          ConfigurationType,
                                          StateType,
                                          StatisticContainerType,
                                          HandlerConfigurationType>
{
  typedef Net_NetlinkSocketHandler_T<HandlerConfigurationType> inherited;

  friend class ACE_Connector<Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                                                       Net_Netlink_Addr,
                                                       ConfigurationType,
                                                       StateType,
                                                       StatisticContainerType,
                                                       TimerManagerType,
                                                       HandlerConfigurationType,
                                                       UserDataType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  virtual ~Net_StreamUDPSocketBase_T ();

  // implement/override some ACE_Event_Handler methods
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE,
                            ACE_Reactor_Mask);

  // implement (part of) Net_ISocketConnection_T
  virtual void dump_state () const;
  virtual void info (ACE_HANDLE&,              // return value: handle
                     Net_Netlink_Addr&,        // return value: local SAP
                     Net_Netlink_Addr&) const; // return value: remote SAP
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); }
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); }
#endif
  inline virtual ACE_Notification_Strategy* notification () { return &(inherited::notificationStrategy_); }
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?

  using inherited::initialize;

 protected:
  // convenient types
  typedef Net_NetlinkSocketHandler_T<HandlerConfigurationType> HANDLER_T;

  Net_StreamUDPSocketBase_T ();

  // override some ACE_Svc_Handler members
  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // args (reason)

  // helper method(s)
#if defined (ACE_LINUX)
  void processErrorQueue ();
#endif // ACE_LINUX

  ACE_Message_Block* writeBuffer_;
  ACE_SYNCH_MUTEX    sendLock_;
  // *IMPORTANT NOTE*: in a threaded environment, workers may (!) dispatch the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  bool               serializeOutput_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T (const Net_StreamUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T& operator= (const Net_StreamUDPSocketBase_T&))
};
#endif // NETLINK_SUPPORT

// include template definition
#include "net_stream_udpsocket_base.inl"

#endif
