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

#ifndef NET_STREAM_TCPSOCKET_BASE_H
#define NET_STREAM_TCPSOCKET_BASE_H

#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"
#include "net_iconnection.h"

// forward declarations
class ACE_Message_Block;
class ACE_Notification_Strategy;

template <ACE_SYNCH_DECL, // 'send' lock strategy
          typename HandlerType, // implements ACE_Svc_Handler
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType, // connection-
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename SocketConfigurationType,
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename UserDataType>
class Net_StreamTCPSocketBase_T
 : public HandlerType
 , virtual public Net_ISocketConnection_T<AddressType,
                                          ConfigurationType,
                                          StateType,
                                          StatisticContainerType,
                                          SocketConfigurationType,
                                          HandlerConfigurationType>
{
  typedef HandlerType inherited;
 
 public:
  virtual ~Net_StreamTCPSocketBase_T ();

  // override some task members
  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // args (reason)

  // override some ACE_Event_Handler methods
  // *NOTE*: enqueue any received data for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: send enqueued data to the peer
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() (see above) returns -1
  virtual int handle_close (ACE_HANDLE,
                            ACE_Reactor_Mask);

  // implement (part of) Net_ISocketConnection_T
  // *NOTE*: use ACE_Event_Handler reference counting
  virtual unsigned int increase ();
  virtual unsigned int decrease ();
  //virtual ACE_Event_Handler::Reference_Count add_reference (void);
  //virtual ACE_Event_Handler::Reference_Count remove_reference (void);
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::get_handle ()); };
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::get_handle ()); };
#endif
  inline virtual ACE_Notification_Strategy* notification () { return &(inherited::notificationStrategy_); };
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?

 protected:
  // convenient types
  typedef HandlerType HANDLER_T;

  Net_StreamTCPSocketBase_T ();

  ACE_Message_Block* readBuffer_;
  ACE_Message_Block* writeBuffer_;
  // *IMPORTANT NOTE*: iff the reactor notification pipe is being dispatched
  //                   concurrently (most notably, ACE_TP_Reactor), havoc ensues
  //                   --> enforce proper serialization
  ACE_SYNCH_MUTEX_T  sendLock_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_StreamTCPSocketBase_T (const Net_StreamTCPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamTCPSocketBase_T& operator= (const Net_StreamTCPSocketBase_T&))
};

// include template definition
#include "net_stream_tcpsocket_base.inl"

#endif
