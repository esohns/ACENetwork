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

#include <ace/Connector.h>
#include <ace/Event_Handler.h>
#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>
#include <ace/Notification_Strategy.h>
#include <ace/SOCK_Connector.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include "common_time_common.h"

#include "stream_imodule.h"

#include "net_connection_base.h"
#include "net_iconnectionmanager.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_netlinksockethandler.h"
#endif
#include "net_udpsockethandler.h"

template <typename HandlerType,
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType,
          ////////////////////////////////
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
class Net_StreamUDPSocketBase_T
 : public HandlerType
 , public Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType>
{
  friend class ACE_Connector<Net_StreamUDPSocketBase_T<HandlerType,
                                                       AddressType,
                                                       ConfigurationType,
                                                       StateType,
                                                       StatisticContainerType,
                                                       HandlerConfigurationType,
                                                       StreamType,
                                                       UserDataType,
                                                       ModuleConfigurationType,
                                                       ModuleHandlerConfigurationType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  virtual ~Net_StreamUDPSocketBase_T ();

  // *NOTE*: enqueue any received data onto our stream for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: send any enqueued data back to the client
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE,
                            ACE_Reactor_Mask);

  // implement (part of) Net_IConnection_T
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); };
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); };
#endif
  inline virtual ACE_Notification_Strategy* notification () { return &(inherited::notificationStrategy_); };
  inline virtual const StreamType& stream () const { return stream_; };
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for any worker
                                                // thread(s) ?

  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;
  virtual void dump_state () const;

  // convenient types
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> CONNECTION_BASE_T;

 protected:
  typedef Net_IConnectionManager_T<AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;
  typedef Stream_IModule_T<Stream_SessionId_t,
                           typename StreamType::SESSION_DATA_T,
                           enum Stream_SessionMessageType,
                           ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           ModuleConfigurationType,
                           ModuleHandlerConfigurationType> IMODULE_T;

  Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                             const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  // override some task-based members
  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // args (reason)

  // helper method(s)
  ACE_Message_Block* allocateMessage (unsigned int); // requested size
#if defined (ACE_LINUX)
  void processErrorQueue ();
#endif

  ACE_Message_Block* currentWriteBuffer_;
  ACE_SYNCH_MUTEX    sendLock_;
  // *IMPORTANT NOTE*: in a threaded environment, workers may (!) dispatch the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  bool               serializeOutput_;

  StreamType         stream_;

 private:
  typedef HandlerType inherited;
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> inherited2;

  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_svc_handler() method of ACE_Connector/ACE_Acceptor
  Net_StreamUDPSocketBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T (const Net_StreamUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T& operator= (const Net_StreamUDPSocketBase_T&))
};

//////////////////////////////////////////

// partial specialization (for connected sockets)
template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType,
          ////////////////////////////////
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
class Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                       HandlerConfigurationType>,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>
 : public Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                 HandlerConfigurationType>
 , public Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType>
{
  friend class ACE_Connector<Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                                                              HandlerConfigurationType>,
                                                       AddressType,
                                                       ConfigurationType,
                                                       StateType,
                                                       StatisticContainerType,
                                                       HandlerConfigurationType,
                                                       StreamType,
                                                       UserDataType,
                                                       ModuleConfigurationType,
                                                       ModuleHandlerConfigurationType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  virtual ~Net_StreamUDPSocketBase_T ();

  // override some task-based members
  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // args (reason)

  // *NOTE*: enqueue any received data onto our stream for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: send any enqueued data back to the client...
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE,
                            ACE_Reactor_Mask);

  // implement (part of) Net_IConnection_T
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); };
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); };
#endif
  inline virtual ACE_Notification_Strategy* notification () { return &(inherited::notificationStrategy_); };
  inline virtual const StreamType& stream () const { return stream_; };
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for any worker
                                                // thread(s) ?

  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;
  virtual void dump_state () const;

  // convenient types
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> CONNECTION_BASE_T;

 protected:
  typedef Net_IConnectionManager_T<AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;
  typedef Stream_IModule_T<Stream_SessionId_t,
                           typename StreamType::SESSION_DATA_T,
                           enum Stream_SessionMessageType,
                           ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           ModuleConfigurationType,
                           ModuleHandlerConfigurationType> IMODULE_T;

  Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                             const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  ACE_Message_Block* currentWriteBuffer_;
  ACE_SYNCH_MUTEX    sendLock_;
  // *IMPORTANT NOTE*: in a threaded environment, workers may (!) dispatch the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  bool               serializeOutput_;

  StreamType         stream_;

  // helper method(s)
  ACE_Message_Block* allocateMessage (unsigned int); // requested size
  void processErrorQueue ();

 private:
  typedef Net_UDPSocketHandler_T<Net_SOCK_CODgram,
                                 HandlerConfigurationType> inherited;
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> inherited2;

  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_svc_handler() method of ACE_Connector/ACE_Acceptor
  Net_StreamUDPSocketBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T (const Net_StreamUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T& operator= (const Net_StreamUDPSocketBase_T&))
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
// partial specialization (for Netlink)
template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType,
          ////////////////////////////////
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
class Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                                AddressType,
                                ConfigurationType,
                                StateType,
                                StatisticContainerType,
                                HandlerConfigurationType,
                                StreamType,
                                UserDataType,
                                ModuleConfigurationType,
                                ModuleHandlerConfigurationType>
 : public Net_NetlinkSocketHandler_T<HandlerConfigurationType>
 , public Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType>
{
  friend class ACE_Connector<Net_StreamUDPSocketBase_T<Net_NetlinkSocketHandler_T<HandlerConfigurationType>,
                                                       AddressType,
                                                       ConfigurationType,
                                                       StateType,
                                                       StatisticContainerType,
                                                       HandlerConfigurationType,
                                                       StreamType,
                                                       UserDataType,
                                                       ModuleConfigurationType,
                                                       ModuleHandlerConfigurationType>,
                             ACE_SOCK_CONNECTOR>;

 public:
  virtual ~Net_StreamUDPSocketBase_T ();

  // override some task-based members
  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // args (reason)

  // *NOTE*: enqueue any received data onto our stream for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: send any enqueued data back to the client...
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE,
                            ACE_Reactor_Mask);

  // implement (part of) Net_IConnection_T
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); };
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::SVC_HANDLER_T::get_handle ()); };
#endif
  inline virtual ACE_Notification_Strategy* notification () { return &(inherited::notificationStrategy_); };
  inline virtual const StreamType& stream () const { return stream_; };
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for any worker
                                                // thread(s) ?

  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;
  virtual void dump_state () const;

  // convenient types
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> CONNECTION_BASE_T;

 protected:
  typedef Net_IConnectionManager_T<AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;
  typedef Stream_IModule_T<Stream_SessionId_t,
                           typename StreamType::SESSION_DATA_T,
                           enum Stream_SessionMessageType,
                           ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           ModuleConfigurationType,
                           ModuleHandlerConfigurationType> IMODULE_T;

  Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                             const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  ACE_Message_Block* currentWriteBuffer_;
  ACE_SYNCH_MUTEX    sendLock_;
  // *IMPORTANT NOTE*: in a threaded environment, workers may (!) dispatch the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  bool               serializeOutput_;

  StreamType         stream_;

  // helper method(s)
  ACE_Message_Block* allocateMessage (unsigned int); // requested size
  void processErrorQueue ();

 private:
  typedef Net_NetlinkSocketHandler_T<HandlerConfigurationType> inherited;
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> inherited2;

  // *TODO*: if there is no default ctor, MSVC will not compile this code.
  //         For some reason, the compiler will not accept the overloaded
  //         make_svc_handler() method of ACE_Connector/ACE_Acceptor
  Net_StreamUDPSocketBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T (const Net_StreamUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T& operator= (const Net_StreamUDPSocketBase_T&))
};
#endif

// include template definition
#include "net_stream_udpsocket_base.inl"

#endif
