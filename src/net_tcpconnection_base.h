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

#include "ace/Asynch_Acceptor.h"
#include "ace/Asynch_Connector.h"
#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/Event_Handler.h"
#include "ace/Acceptor.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"

#include "stream_common.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnectionmanager.h"
#include "net_itransportlayer.h"
#include "net_socketconnection_base.h"

template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
class Net_TCPConnectionBase_T
 : public Net_SocketConnectionBase_T<HandlerType,
                                     Net_IInetTransportLayer_t,
                                     ConfigurationType,
                                     Net_SocketHandlerConfiguration_t,
                                     UserDataType,
                                     SessionDataType,
                                     Stream_Statistic_t>
{
 friend class ACE_Acceptor<Net_TCPConnectionBase_T<ConfigurationType,
                                                   UserDataType,
                                                   SessionDataType,
                                                   HandlerType>,
                           ACE_SOCK_ACCEPTOR>;
 friend class ACE_Connector<Net_TCPConnectionBase_T<ConfigurationType,
                                                    UserDataType,
                                                    SessionDataType,
                                                    HandlerType>,
                            ACE_SOCK_CONNECTOR>;

 public:
  typedef Net_IConnectionManager_T<ConfigurationType,
                                   UserDataType,
                                   Stream_Statistic_t,
                                   Net_IInetTransportLayer_t> ICONNECTION_MANAGER_T;

  Net_TCPConnectionBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                           unsigned int = 0);      // statistics collecting interval (second(s))
                                                   // 0 --> DON'T collect statistics
  virtual ~Net_TCPConnectionBase_T ();

  // override / implement (part of) Net_IInetTransportLayer
  virtual bool initialize (Net_ClientServerRole_t,            // role
                           const Net_SocketConfiguration_t&); // socket configuration
  virtual void finalize ();

  //// override some task-based members
  virtual int open (void* = NULL); // args
  //virtual int close (u_long = 0); // args

//  // *NOTE*: enqueue any received data onto our stream for further processing
//   virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

 private:
  typedef Net_SocketConnectionBase_T<HandlerType,
                                     Net_IInetTransportLayer_t,
                                     ConfigurationType,
                                     Net_SocketHandlerConfiguration_t,
                                     UserDataType,
                                     SessionDataType,
                                     Stream_Statistic_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  Net_TCPConnectionBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnectionBase_T (const Net_TCPConnectionBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_TCPConnectionBase_T& operator= (const Net_TCPConnectionBase_T&));
};

/////////////////////////////////////////

template <typename ConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename HandlerType>
class Net_AsynchTCPConnectionBase_T
 : public Net_AsynchSocketConnectionBase_T<HandlerType,
                                           Net_IInetTransportLayer_t,
                                           ConfigurationType,
                                           Net_SocketHandlerConfiguration_t,
                                           UserDataType,
                                           SessionDataType,
                                           Stream_Statistic_t>
{
 friend class ACE_Asynch_Acceptor<Net_AsynchTCPConnectionBase_T<ConfigurationType,
                                                                UserDataType,
                                                                SessionDataType,
                                                                HandlerType> >;
 friend class ACE_Asynch_Connector<Net_AsynchTCPConnectionBase_T<ConfigurationType,
                                                                 UserDataType,
                                                                 SessionDataType,
                                                                 HandlerType> >;

 public:
  typedef Net_IConnectionManager_T<ConfigurationType,
                                   UserDataType,
                                   Stream_Statistic_t,
                                   Net_IInetTransportLayer_t> ICONNECTION_MANAGER_T;

  Net_AsynchTCPConnectionBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                                 unsigned int = 0);      // statistics collecting interval (second(s))
                                                         // 0 --> DON'T collect statistics
  virtual ~Net_AsynchTCPConnectionBase_T ();

  // override / implement (part of) Net_IInetTransportLayer
  virtual bool initialize (Net_ClientServerRole_t,            // role
                           const Net_SocketConfiguration_t&); // socket configuration
  virtual void finalize ();

  // override some ACE_Service_Handler members
  virtual void open (ACE_HANDLE,          // handle
                     ACE_Message_Block&); // (initial) data (if any)
  //virtual int close (u_long = 0); // args

  //  // *NOTE*: enqueue any received data onto our stream for further processing
  //   virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK);

 private:
  typedef Net_AsynchSocketConnectionBase_T<HandlerType,
                                           Net_IInetTransportLayer_t,
                                           ConfigurationType,
                                           Net_SocketHandlerConfiguration_t,
                                           UserDataType,
                                           SessionDataType,
                                           Stream_Statistic_t> inherited;

  //// override some task-based members
  //virtual int svc (void);

  //// stop worker, if any
  //void shutdown ();

  Net_AsynchTCPConnectionBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnectionBase_T (const Net_AsynchTCPConnectionBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchTCPConnectionBase_T& operator= (const Net_AsynchTCPConnectionBase_T&));
};

#include "net_tcpconnection_base.inl"

#endif
