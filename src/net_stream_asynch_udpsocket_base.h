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

#include <ace/config-lite.h>
#include <ace/Asynch_IO.h>
#include <ace/Event_Handler.h>
#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include "common_time_common.h"

#include "stream_imodule.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_asynch_netlinksockethandler.h"
#include "net_netlinksockethandler.h"
#endif
#include "net_connection_base.h"
#include "net_iconnectionmanager.h"

template <typename HandlerType,
          typename SocketType,
          ///////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ///////////////////////////////
          typename HandlerConfigurationType, // socket-
          ///////////////////////////////
          typename StreamType,
          ///////////////////////////////
          typename UserDataType,
          ///////////////////////////////
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
class Net_StreamAsynchUDPSocketBase_T
 : public HandlerType
 , public SocketType
 , public ACE_Event_Handler
 , public Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType>
{
 public:
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> CONNECTION_BASE_T;

  virtual ~Net_StreamAsynchUDPSocketBase_T ();

  // override some service methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  int close (u_long = 0); // reason
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE); // (socket) handle
  virtual int handle_close (ACE_HANDLE handle_in,      // (socket) handle
                            ACE_Reactor_Mask mask_in); // event mask

  // implement (part of) Net_IConnection_T
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
  virtual Net_ConnectionId_t id () const;
  inline virtual ACE_Notification_Strategy* notification () { return this; };
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for any worker
                                                // thread(s) ?

  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;
  virtual void dump_state () const;

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

  Net_StreamAsynchUDPSocketBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                                   const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  // helper methods
  virtual void handle_read_dgram (const ACE_Asynch_Read_Dgram::Result&); // result
  virtual void handle_write_dgram (const ACE_Asynch_Write_Dgram::Result&); // result

//  // *TODO*: handle short writes (more) gracefully...
//  ACE_Message_Block* buffer_;
  StreamType         stream_;
  bool               useThreadPerConnection_;

 private:
  typedef HandlerType inherited;
  typedef SocketType inherited2;
  typedef ACE_Event_Handler inherited3;
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> inherited4;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T (const Net_StreamAsynchUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T& operator= (const Net_StreamAsynchUDPSocketBase_T&))
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
// partial specialization (for Netlink)
template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ///////////////////////////////
          typename HandlerConfigurationType, // socket-
          ///////////////////////////////
          typename StreamType,
          ///////////////////////////////
          typename UserDataType,
          ///////////////////////////////
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
class Net_StreamAsynchUDPSocketBase_T<Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>,
                                      Net_SOCK_Netlink,
                                      AddressType,
                                      ConfigurationType,
                                      StateType,
                                      StatisticContainerType,
                                      HandlerConfigurationType,
                                      StreamType,
                                      UserDataType,
                                      ModuleConfigurationType,
                                      ModuleHandlerConfigurationType>
 : public Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>
 , public Net_SOCK_Netlink
 , public ACE_Event_Handler
 , public Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType>
{
 public:
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> CONNECTION_BASE_T;

  virtual ~Net_StreamAsynchUDPSocketBase_T ();

  // override some service methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  int close (u_long = 0); // reason
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE); // (socket) handle
  virtual int handle_close (ACE_HANDLE handle_in,      // (socket) handle
                            ACE_Reactor_Mask mask_in); // event mask

  // implement (part of) Net_IConnection_T
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
  virtual Net_ConnectionId_t id () const;
  inline virtual ACE_Notification_Strategy* notification () { return this; };
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for any worker
                                                // thread(s) ?

  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;
  virtual void dump_state () const;

 protected:
  typedef Net_IConnectionManager_T<AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_StreamAsynchUDPSocketBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                                   const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  // helper methods
  virtual void handle_read_dgram (const ACE_Asynch_Read_Dgram::Result&); // result

//  // *TODO*: handle short writes (more) gracefully...
//  ACE_Message_Block* buffer_;
  StreamType         stream_;
  bool               useThreadPerConnection_;

 private:
  typedef Net_AsynchNetlinkSocketHandler_T<HandlerConfigurationType>  inherited;
  typedef Net_SOCK_Netlink inherited2;
  typedef ACE_Event_Handler inherited3;
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> inherited4;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T (const Net_StreamAsynchUDPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T& operator= (const Net_StreamAsynchUDPSocketBase_T&))
};
#endif

// include template definition
#include "net_stream_asynch_udpsocket_base.inl"

#endif
