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

#ifndef NET_SOCKETCONNECTION_BASE_H
#define NET_SOCKETCONNECTION_BASE_H

#include "ace/config-macros.h"
#include "ace/Event_Handler.h"

#include "net_connection_base.h"
#include "net_connection_manager.h"
#include "net_iconnection.h"

template <typename AddressType,
          typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
class Net_SocketConnectionBase_T
 : public HandlerType
 , public Net_ISocketConnection_T<AddressType,
                                  SocketConfigurationType,
                                  ConfigurationType,
                                  StatisticContainerType,
                                  StreamType>
{
 public:
  virtual ~Net_SocketConnectionBase_T ();

  // override some task-based members
  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // args
//  virtual int svc (void);

  //// implement (part of) Net_ITransportLayer_T
  //virtual bool initialize (Net_ClientServerRole_t,            // role
  //                         const Net_SocketConfiguration_t&); // socket configuration
  //virtual void finalize ();
  virtual void ping (); // ping the peer !

//  // *NOTE*: enqueue any received data onto our stream for further processing
//  virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
//  // *NOTE*: this is called when:
//  // handle_xxx() returns -1
//  virtual int handle_close (ACE_HANDLE,
//                            ACE_Reactor_Mask);

 protected:
  typedef Net_IConnectionManager_T<AddressType,
                                   SocketConfigurationType,
                                   ConfigurationType,
                                   UserDataType,
                                   StatisticContainerType,
                                   StreamType> ICONNECTION_MANAGER_T;

  Net_SocketConnectionBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                              unsigned int = 0);      // statistics collecting interval (second(s))
                                                      // 0 --> DON'T collect statistics

 private:
  typedef HandlerType inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_SocketConnectionBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_SocketConnectionBase_T (const Net_SocketConnectionBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_SocketConnectionBase_T& operator= (const Net_SocketConnectionBase_T&));

//  // helper methods
//  void shutdown ();
};

/////////////////////////////////////////

template <typename AddressType,
          typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename SocketHandlerConfigurationType,
          typename UserDataType,
          typename SessionDataType,
          typename StatisticContainerType,
          typename StreamType>
class Net_AsynchSocketConnectionBase_T
 : public HandlerType
 , public Net_ISocketConnection_T<AddressType,
                                  SocketConfigurationType,
                                  ConfigurationType,
                                  StatisticContainerType,
                                  StreamType>
{
 public:
  virtual ~Net_AsynchSocketConnectionBase_T ();

  // override some ACE_Service_Handler members
  virtual void open (ACE_HANDLE,          // handle
                     ACE_Message_Block&); // (initial) data (if any)
  //virtual void act (const void*); // (user) data handle

  // implement (part of) Net_ITransportLayer
  virtual void ping (); // ping the peer !

 protected:
  typedef Net_IConnectionManager_T<AddressType,
                                   SocketConfigurationType,
                                   ConfigurationType,
                                   UserDataType,
                                   StatisticContainerType,
                                   StreamType> ICONNECTION_MANAGER_T;

  Net_AsynchSocketConnectionBase_T (ICONNECTION_MANAGER_T*, // connection manager handle
                                    unsigned int = 0);      // statistics collecting interval (second(s))
                                                            // 0 --> DON'T collect statistics

  const ConfigurationType* configuration_;

 private:
  typedef HandlerType inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_AsynchSocketConnectionBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchSocketConnectionBase_T (const Net_AsynchSocketConnectionBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchSocketConnectionBase_T& operator= (const Net_AsynchSocketConnectionBase_T&));
};

#include "net_socketconnection_base.inl"

#endif
