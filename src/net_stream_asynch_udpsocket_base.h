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

#include "ace/config-lite.h"
#include "ace/Asynch_IO.h"
#include "ace/Event_Handler.h"
#include "ace/Message_Block.h"

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "net_asynch_netlinksockethandler.h"
#endif
#include "net_connection_base.h"
#include "net_connection_common.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "net_netlinksockethandler.h"
#endif

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType,
          typename SocketType,
          typename SocketHandlerType>
class Net_StreamAsynchUDPSocketBase_T
 : public SocketHandlerType
 , public SocketType
 , public ACE_Event_Handler
 , public Net_ConnectionBase_T<AddressType,
                               SocketConfigurationType,
                               ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType>
{
 public:
  typedef Net_ConnectionBase_T<AddressType,
                               SocketConfigurationType,
                               ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType> CONNECTION_BASE_T;

  virtual ~Net_StreamAsynchUDPSocketBase_T ();

  // override some service methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)

  // implement (part of) Net_IConnection_T
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
  virtual unsigned int id () const;
  virtual void close ();
  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;
  virtual void dump_state () const;

 protected:
  typedef Net_IConnectionManager_T<AddressType,
                                   SocketConfigurationType,
                                   ConfigurationType,
                                   UserDataType,
                                   StatisticContainerType> ICONNECTION_MANAGER_T;

  Net_StreamAsynchUDPSocketBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                                   unsigned int = 0);      // statistics collecting interval (second(s))
                                                           // 0 --> DON'T collect statistics

  // helper methods
  virtual int handle_close (ACE_HANDLE handle_in,                                           // (socket) handle
                            ACE_Reactor_Mask mask_in = ACE_Event_Handler::ALL_EVENTS_MASK); // event mask

  virtual void handle_read_dgram (const ACE_Asynch_Read_Dgram::Result&); // result

  StreamType      stream_;
  // *TODO*: handle short writes (more) gracefully...
  //  ACE_Message_Block* buffer_;

 private:
  typedef SocketHandlerType inherited;
  typedef SocketType inherited2;
  typedef ACE_Event_Handler inherited3;
  typedef Net_ConnectionBase_T<AddressType,
                               SocketConfigurationType,
                               ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType> inherited4;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T (const Net_StreamAsynchUDPSocketBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T& operator= (const Net_StreamAsynchUDPSocketBase_T&));

  // helper methods
  virtual int handle_output (ACE_HANDLE); // (socket) handle
};

/////////////////////////////////////////

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
// partial specialization (for Netlink)
template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
class Net_StreamAsynchUDPSocketBase_T<AddressType,
                                      SocketConfigurationType,
                                      ConfigurationType,
                                      UserDataType,
                                      SessionDataType,
                                      StatisticContainerType,
                                      StreamType,
                                      ACE_SOCK_NETLINK,
                                      Net_AsynchNetlinkSocketHandler>
 : public Net_AsynchNetlinkSocketHandler
 , public ACE_SOCK_NETLINK
 , public ACE_Event_Handler
 , public Net_ConnectionBase_T<AddressType,
                               SocketConfigurationType,
                               ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType>
{
 public:
  typedef Net_ConnectionBase_T<AddressType,
                               SocketConfigurationType,
                               ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType> CONNECTION_BASE_T;

  virtual ~Net_StreamAsynchUDPSocketBase_T ();

  // override some service methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)

  // implement (part of) Net_IConnection_T
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
  virtual unsigned int id () const;
  virtual void close ();
  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;
  virtual void dump_state () const;

 protected:
  typedef Net_IConnectionManager_T<AdressType,
                                   SocketConfigurationType,
                                   ConfigurationType,
                                   UserDataType,
                                   StatisticContainerType> ICONNECTION_MANAGER_T;

  Net_StreamAsynchUDPSocketBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                                   unsigned int = 0);      // statistics collecting interval (second(s))
                                                           // 0 --> DON'T collect statistics

  // helper methods
  virtual int handle_close (ACE_HANDLE handle_in,                                           // (socket) handle
                            ACE_Reactor_Mask mask_in = ACE_Event_Handler::ALL_EVENTS_MASK); // event mask

  virtual void handle_read_dgram (const ACE_Asynch_Read_Dgram::Result&); // result

  // *TODO*: (try to) handle short writes gracefully...
  //  ACE_Message_Block* buffer_;
  StreamType         stream_;

 private:
  typedef Net_AsynchNetlinkSocketHandler inherited;
  typedef ACE_SOCK_NETLINK inherited2;
  typedef ACE_Event_Handler inherited3;
  typedef Net_ConnectionBase_T<AddressType,
                               SocketConfigurationType,
                               ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType> inherited4;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T (const Net_StreamAsynchUDPSocketBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchUDPSocketBase_T& operator= (const Net_StreamAsynchUDPSocketBase_T&));

  // helper methods
  virtual int handle_output (ACE_HANDLE); // (socket) handle
};
#endif

// include template definition
#include "net_stream_asynch_udpsocket_base.inl"

#endif
