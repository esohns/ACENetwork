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

#ifndef NET_STREAM_ASYNCH_TCPSOCKET_BASE_H
#define NET_STREAM_ASYNCH_TCPSOCKET_BASE_H

#include "ace/Asynch_IO.h"
#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/SOCK_Stream.h"
#include "ace/Svc_Handler.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"
#include "net_iconnection.h"

// forward declarations
class ACE_Notification_Strategy;

template <typename HandlerType, // implements ACE_Service_Handler
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType, // connection-
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename SocketConfigurationType,
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename UserDataType>
class Net_StreamAsynchTCPSocketBase_T
 : public HandlerType
 , public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>
 , virtual public Net_ISocketConnection_T<AddressType,
                                          ConfigurationType,
                                          StateType,
                                          StatisticContainerType,
                                          SocketConfigurationType,
                                          HandlerConfigurationType>
{
  typedef HandlerType inherited;
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> inherited2;

 public:
  inline virtual ~Net_StreamAsynchTCPSocketBase_T () {};

  // override some ACE_Service_Handler methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  // implement some ACE_Task methods
  //inline virtual int close (u_long reason_in = 0) { close (); };

  // implement/override some ACE_Event_Handler methods
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE); // (socket) handle
  virtual int handle_close (ACE_HANDLE,        // (socket) handle
                            ACE_Reactor_Mask); // (select) mask

  // implement (part of) Net_ISocketConnection_T
  virtual void dump_state () const;
  inline virtual void info (ACE_HANDLE& handle_out, AddressType& localSAP_out, AddressType& peerSAP_out) const { handle_out = inherited::handle (); localSAP_out = inherited::localSAP_; peerSAP_out = inherited::peerSAP_; };
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::handle ()); };
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::handle ()); };
#endif
  inline virtual ACE_Notification_Strategy* notification () { return this; };
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?

 protected:
  // convenient types
  typedef HandlerType HANDLER_T;

  Net_StreamAsynchTCPSocketBase_T ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchTCPSocketBase_T (const Net_StreamAsynchTCPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchTCPSocketBase_T& operator= (const Net_StreamAsynchTCPSocketBase_T&))

  // implement some ACE_Handler methods
  virtual void handle_read_stream (const ACE_Asynch_Read_Stream::Result&); // result
  virtual void handle_write_stream (const ACE_Asynch_Write_Stream::Result&); // result
};

// include template definition
#include "net_stream_asynch_tcpsocket_base.inl"

#endif
