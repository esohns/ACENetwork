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

//#include <ace/Acceptor.h>
//#include <ace/Connector.h>
#include <ace/config-macros.h>
#include <ace/Event_Handler.h>
#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>
//#include <ace/SOCK_Acceptor.h>
//#include <ace/SOCK_Connector.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include "common_time_common.h"

#include "stream_imodule.h"

#include "net_connection_base.h"
#include "net_iconnectionmanager.h"

template <typename HandlerType,
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType, // connection-
          typename StatisticContainerType,
          typename StreamType,
          ////////////////////////////////
          typename UserDataType,
          ////////////////////////////////
          typename ModuleConfigurationType,
          typename ModuleHandlerConfigurationType>
class Net_StreamTCPSocketBase_T
 : public HandlerType
 , public Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType>
{
  //friend class ACE_Acceptor<Net_StreamTCPSocketBase_T<HandlerType,
  //                                                    AddressType,
  //                                                    ConfigurationType,
  //                                                    StateType,
  //                                                    StatisticContainerType,
  //                                                    StreamType,
  //                                                    UserDataType,
  //                                                    ModuleConfigurationType,
  //                                                    ModuleHandlerConfigurationType>,
  //                          ACE_SOCK_ACCEPTOR>;
  //friend class ACE_Connector<Net_StreamTCPSocketBase_T<HandlerType,
  //                                                     AddressType,
  //                                                     ConfigurationType,
  //                                                     StateType,
  //                                                     StatisticContainerType,
  //                                                     StreamType,
  //                                                     UserDataType,
  //                                                     ModuleConfigurationType,
  //                                                     ModuleHandlerConfigurationType>,
  //                           ACE_SOCK_CONNECTOR>;

 public:
  virtual ~Net_StreamTCPSocketBase_T ();

  // override some task members
  virtual int open (void* = NULL); // args
  virtual int close (u_long = 0); // args (reason)

  // override some ACE_Event_Handler methods
  // *NOTE*: enqueue any received data onto our stream for further processing
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: send any enqueued data back to the client...
  virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() returns -1
  virtual int handle_close (ACE_HANDLE,
                            ACE_Reactor_Mask);

  // (partially) override Common_IReferenceCount
  // --> use ACE_Event_Handler reference counting
  virtual unsigned int increase ();
  virtual unsigned int decrease ();
  //virtual ACE_Event_Handler::Reference_Count add_reference (void);
  //virtual ACE_Event_Handler::Reference_Count remove_reference (void);

  // implement (part of) Net_IConnection_T
  virtual void info (ACE_HANDLE&,         // return value: handle
                     AddressType&,        // return value: local SAP
                     AddressType&) const; // return value: remote SAP
  virtual Net_ConnectionId_t id () const;
  virtual ACE_Notification_Strategy* notification ();
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
  typedef HandlerType HANDLER_T;

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

  Net_StreamTCPSocketBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                             const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  ACE_Message_Block* currentReadBuffer_;
  ACE_Message_Block* currentWriteBuffer_;
  ACE_SYNCH_MUTEX    sendLock_;
  // *IMPORTANT NOTE*: in a threaded environment, workers may (!) dispatch the
  //                   reactor notification queue concurrently (most notably,
  //                   ACE_TP_Reactor) --> enforce proper serialization
  bool               serializeOutput_;
  StreamType         stream_;

  // helper method(s)
  ACE_Message_Block* allocateMessage (unsigned int); // requested size

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
  Net_StreamTCPSocketBase_T ();
  ACE_UNIMPLEMENTED_FUNC (Net_StreamTCPSocketBase_T (const Net_StreamTCPSocketBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamTCPSocketBase_T& operator= (const Net_StreamTCPSocketBase_T&))
};

// include template definition
#include "net_stream_tcpsocket_base.inl"

#endif
