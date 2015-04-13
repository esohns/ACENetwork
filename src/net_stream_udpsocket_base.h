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

#include "ace/config-lite.h"
#include "ace/Event_Handler.h"
#include "ace/INET_Addr.h"
#include "ace/Message_Block.h"
#include "ace/Synch.h"

#include "net_connection_base.h"
#include "net_itransportlayer.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "net_netlinksockethandler.h"
#endif

template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename ITransportLayerType,
          typename StatisticContainerType,
          typename StreamType,
//          typename SocketType,
          typename SocketHandlerType>
class Net_StreamUDPSocketBase_T
 : /*public SocketType
 ,*/ public SocketHandlerType
 , public Net_ConnectionBase_T<ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType,
                               ITransportLayerType>
{
 public:
  virtual ~Net_StreamUDPSocketBase_T ();

  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // flags

  // *NOTE*: enqueue any received data onto our stream for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: send any enqueued data back to the client...
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

  // implement Common_IStatistic
  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;

  // implement (part of) Net_ITransportLayer_T
  virtual void info (ACE_HANDLE&,           // return value: handle
                     ACE_INET_Addr&,        // return value: local SAP
                     ACE_INET_Addr&) const; // return value: remote SAP
  virtual unsigned int id () const;
  virtual void dump_state () const;

 protected:
 typedef Net_IConnectionManager_T<ConfigurationType,
                                  UserDataType,
                                  StatisticContainerType,
                                  ITransportLayerType> ICONNECTION_MANAGER_T;
  typedef Net_ConnectionBase_T<ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType,
                               ITransportLayerType> CONNECTION_BASE_T;

  Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                             unsigned int = 0);      // statistics collecting interval (second(s))
                                                     // 0 --> DON'T collect statistics

  ACE_Message_Block* currentWriteBuffer_;
  ACE_Thread_Mutex   sendLock_;
  // *IMPORTANT NOTE*: in a threaded environment, workers MAY
  // dispatch the reactor notification queue concurrently (most notably,
  // ACE_TP_Reactor) --> enforce proper serialization
  bool               serializeOutput_;
  StreamType         stream_;

  // helper method(s)
  ACE_Message_Block* allocateMessage (unsigned int); // requested size

 private:
//  typedef SocketType inherited;
  typedef SocketHandlerType inherited2;
  typedef Net_ConnectionBase_T<ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType,
                               ITransportLayerType> inherited3;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T (const Net_StreamUDPSocketBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T& operator= (const Net_StreamUDPSocketBase_T&));
};

/////////////////////////////////////////

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
// partial specialization (for Netlink)
template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
class Net_StreamUDPSocketBase_T<ConfigurationType,
                                UserDataType,
                                SessionDataType,
                                Net_INetlinkTransportLayer_t,
                                StatisticContainerType,
                                StreamType,
                                Net_NetlinkSocketHandler>
 : /*public SocketType
 ,*/ public Net_NetlinkSocketHandler
 , public Net_ConnectionBase_T<ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType,
                               Net_INetlinkTransportLayer_t>
{
 public:
  virtual ~Net_StreamUDPSocketBase_T ();

  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // flags

  // *NOTE*: enqueue any received data onto our stream for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: send any enqueued data back to the client...
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

  // implement Common_IStatistic
  // *NOTE*: delegate these to the stream
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;

  // implement (part of) Net_ITransportLayer_T
  virtual void info (ACE_HANDLE&,           // return value: handle
                     ACE_INET_Addr&,        // return value: local SAP
                     ACE_INET_Addr&) const; // return value: remote SAP
  virtual unsigned int id () const;
  virtual void dump_state () const;

 protected:
 typedef Net_IConnectionManager_T<ConfigurationType,
                                  UserDataType,
                                  StatisticContainerType,
                                  Net_INetlinkTransportLayer_t> ICONNECTION_MANAGER_T;
  typedef Net_ConnectionBase_T<ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType,
                               Net_INetlinkTransportLayer_t> CONNECTION_BASE_T;

  Net_StreamUDPSocketBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                             unsigned int = 0);      // statistics collecting interval (second(s))
                                                     // 0 --> DON'T collect statistics

  ACE_Message_Block* currentWriteBuffer_;
  ACE_Thread_Mutex   sendLock_;
  // *IMPORTANT NOTE*: in a threaded environment, workers MAY
  // dispatch the reactor notification queue concurrently (most notably,
  // ACE_TP_Reactor) --> enforce proper serialization
  bool               serializeOutput_;
  StreamType         stream_;

  // helper method(s)
  ACE_Message_Block* allocateMessage (unsigned int); // requested size

 private:
//  typedef SocketType inherited;
  typedef Net_NetlinkSocketHandler inherited2;
  typedef Net_ConnectionBase_T<ConfigurationType,
                               UserDataType,
                               SessionDataType,
                               StatisticContainerType,
                               Net_INetlinkTransportLayer_t> inherited3;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T (const Net_StreamUDPSocketBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_StreamUDPSocketBase_T& operator= (const Net_StreamUDPSocketBase_T&));
};
#endif

// include template implementation
#include "net_stream_udpsocket_base.inl"

#endif
