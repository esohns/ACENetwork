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

#include "ace/config-lite.h"
#include "ace/Asynch_IO.h"
#include "ace/Event_Handler.h"
#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_time_common.h"

#include "stream_imodule.h"

#include "net_connection_base.h"
#include "net_iconnectionmanager.h"

template <typename HandlerType,
          ///////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType, // (connection) state
          typename StatisticContainerType,
          typename StreamType,
          ///////////////////////////////
          typename UserDataType,
          ///////////////////////////////
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
class Net_StreamAsynchTCPSocketBase_T
 : public HandlerType
 , public ACE_Event_Handler
 , public Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               StreamType,
                               //////////
                               UserDataType>
{
 public:
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               StreamType,
                               //////////
                               UserDataType> CONNECTION_BASE_T;

  virtual ~Net_StreamAsynchTCPSocketBase_T ();

  // implement some task methods
  int close (u_long = 0); // reason
  // override some service handler methods
  virtual void open (ACE_HANDLE,          // (socket) handle
                     ACE_Message_Block&); // initial data (if any)
  //virtual void act (const void*); // (user) data handle
  // implement some event handler methods
  virtual int handle_close (ACE_HANDLE,        // (socket) handle
                            ACE_Reactor_Mask); // (select) mask
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE); // (socket) handle

  // implement (part of) Net_IConnection_T
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
  virtual size_t id () const;
  virtual ACE_Notification_Strategy* notification ();
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
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;
  typedef Stream_IModule_T<ACE_MT_SYNCH,
                           Common_TimePolicy_t,
                           ModuleConfigurationType,
                           ModuleHandlerConfigurationType> IMODULE_T;

  Net_StreamAsynchTCPSocketBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                                   const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  // implement some handler methods
  virtual void handle_read_stream (const ACE_Asynch_Read_Stream::Result&); // result
  virtual void handle_write_stream (const ACE_Asynch_Write_Stream::Result&); // result

  StreamType stream_;

 private:
  typedef HandlerType inherited;
  typedef ACE_Event_Handler inherited2;
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               StreamType,
                               //////////
                               UserDataType> inherited3;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchTCPSocketBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchTCPSocketBase_T (const Net_StreamAsynchTCPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamAsynchTCPSocketBase_T& operator= (const Net_StreamAsynchTCPSocketBase_T&))
};

// include template definition
#include "net_stream_asynch_tcpsocket_base.inl"

#endif
