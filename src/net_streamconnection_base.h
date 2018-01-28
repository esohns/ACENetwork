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

#ifndef NET_STREAMCONNECTION_BASE_H
#define NET_STREAMCONNECTION_BASE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "net_common.h"
#include "net_connection_base.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_ilistener.h"
#include "net_itransportlayer.h"

// forward declarations
class ACE_Time_Value;
class Stream_IAllocator;

template <ACE_SYNCH_DECL, // 'send' lock strategy
          typename HandlerType, // implements ACE_Svc_Handler
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename SocketConfigurationType,
          typename HandlerConfigurationType, // socket-
          typename ListenerConfigurationType,
          ////////////////////////////////
          typename StreamType,
          typename StreamStatusType, // state machine-
          ////////////////////////////////
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType>
class Net_StreamConnectionBase_T
 : public HandlerType
 , public Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               TimerManagerType,
                               UserDataType>
 , public Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  SocketConfigurationType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType>
{
  typedef HandlerType inherited;
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               TimerManagerType,
                               UserDataType> inherited2;

 public:
  // convenient types
  typedef AddressType addr_type; // required by ACE_Connector
  typedef HandlerType HANDLER_T;
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               TimerManagerType,
                               UserDataType> CONNECTION_BASE_T;
  typedef Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  SocketConfigurationType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType> ISTREAM_CONNECTION_T;

  virtual ~Net_StreamConnectionBase_T ();

  // override (part of) ACE_Svc_Handler
  virtual int open (void* = NULL); // argument
  virtual int close (u_long = 0); // reason

  // override some ACE_Event_Handler methods
  // *NOTE*: stream any received data for further processing
  //virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  //// *NOTE*: send stream data to the peer
  //virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() (see above) returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,                        // handle
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK); // event mask

  // implement (part of) Net_IStreamConnection_T
  //inline virtual unsigned int increase () { return CONNECTION_BASE_T::REFERENCECOUNTER_T::increase (); };
  //inline virtual unsigned int decrease () { return CONNECTION_BASE_T::REFERENCECOUNTER_T::decrease (); };
  inline virtual bool collect (StatisticContainerType& statistic_out) { return stream_.collect (statistic_out); }
  inline virtual void report () const { stream_.report (); }
  //using CONNECTION_BASE_T::getR;
  virtual void dump_state () const;
  virtual void info (ACE_HANDLE&,
                     AddressType&,
                     AddressType&) const;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::get_handle ()); }
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::get_handle ()); }
#endif
  inline virtual ACE_Notification_Strategy* notification () { return &(inherited::notificationStrategy_); }
  virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?
  // -------------------------------------
  virtual void set (enum Net_ClientServerRole);
  inline virtual const HandlerConfigurationType& getR_2 () const { ACE_ASSERT (inherited::configuration_); return *inherited::configuration_; }
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void send (ACE_Message_Block*&);
  // -------------------------------------
  inline virtual unsigned int flush (bool flushSessionMessages_in = false) { return stream_.flush (false, flushSessionMessages_in, false); }
  // *NOTE*: this waits for outbound (!) data only
  inline virtual void waitForIdleState () const { stream_.idle (); }
  inline virtual const StreamType& stream () const { return stream_; }
  virtual bool wait (StreamStatusType,
                     const ACE_Time_Value* = NULL); // timeout (absolute) ? : block

 protected:
  // convenient types
  typedef Net_IConnector_T<AddressType,
                           ConfigurationType> ICONNECTOR_T;
  typedef Net_IListener_T<ListenerConfigurationType,
                          ConfigurationType> ILISTENER_T;
  typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                   AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_StreamConnectionBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                              const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  // override (part of) Net_ISocketHandler
  virtual ACE_Message_Block* allocateMessage (unsigned int); // requested size

  // *IMPORTANT NOTE*: stub to facilitate connection handler encapsulation
  virtual void open (ACE_HANDLE, ACE_Message_Block&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return); }

  Stream_IAllocator* allocator_;
  StreamType         stream_;

  ACE_SYNCH_MUTEX_T  sendLock_;
  // *NOTE*: support partial writes
  ACE_Message_Block* writeBuffer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_StreamConnectionBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_StreamConnectionBase_T (const Net_StreamConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamConnectionBase_T& operator= (const Net_StreamConnectionBase_T&))

  using ISTREAM_CONNECTION_T::ISOCKET_CONNECTION_T::ITRANSPORT_LAYER_T::initialize;

  bool               notify_; // still to notify the processing stream ?
};

/////////////////////////////////////////

template <typename HandlerType, // implements ACE_Service_Handler
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename SocketConfigurationType,
          typename HandlerConfigurationType, // socket-
          typename ListenerConfigurationType,
          ////////////////////////////////
          typename StreamType,
          typename StreamStatusType,
          ////////////////////////////////
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType>
class Net_AsynchStreamConnectionBase_T
 : public HandlerType
 , public Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               TimerManagerType,
                               UserDataType>
 , public Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  SocketConfigurationType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType>
{
  typedef HandlerType inherited;
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               TimerManagerType,
                               UserDataType> inherited2;


 public:
  // convenient types
  typedef Net_ConnectionBase_T<AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               TimerManagerType,
                               UserDataType> CONNECTION_BASE_T;
  typedef Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  SocketConfigurationType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType> ISTREAM_CONNECTION_T;

  inline virtual ~Net_AsynchStreamConnectionBase_T () {};

  // override (part of) ACE_Service_Handler
  virtual void act (const void*); // act
  virtual void open (ACE_HANDLE,          // handle
                     ACE_Message_Block&); // (initial) data (if any)

  // override some ACE_Event_Handler methods
  // *NOTE*: send stream data to the peer
  //virtual int handle_output (ACE_HANDLE = ACE_INVALID_HANDLE);
  // *NOTE*: this is called when:
  // - handle_xxx() (see above) returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,                        // handle
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK); // event mask

  // implement (part of) Net_IStreamConnection_T
  inline virtual bool collect (StatisticContainerType& statistic_out) { return stream_.collect (statistic_out); }; // return value: statistic data
  inline virtual void report () const { stream_.report (); };
  virtual void dump_state () const;
  //inline virtual void info (ACE_HANDLE& handle_out, AddressType& localSAP_out, AddressType& peerSAP_out) const { handle_out = inherited::handle (); localSAP_out = inherited::localSAP_; peerSAP_out = inherited::peerSAP_; };
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::handle ()); };
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::handle ()); };
#endif
  inline virtual ACE_Notification_Strategy* notification () { return this; };
  inline virtual void close ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?
  // -------------------------------------
  virtual void set (enum Net_ClientServerRole);
  inline virtual const HandlerConfigurationType& getR_2 () const { ACE_ASSERT (inherited::configuration_); return *(inherited::configuration_); };
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void send (ACE_Message_Block*&);
  // -------------------------------------
  inline virtual unsigned int flush (bool flushSessionMessages_in = false) { return stream_.flush (false, flushSessionMessages_in, false); };
  // *NOTE*: this waits for outbound (!) data only
  virtual void waitForIdleState () const;
  inline virtual const StreamType& stream () const { return stream_; };
  virtual bool wait (StreamStatusType,
                     const ACE_Time_Value* = NULL); // timeout (absolute) ? : block

 protected:
  typedef HandlerType HANDLER_T;
  typedef StreamType STREAM_T;
  typedef Net_IConnector_T<AddressType,
                           ConfigurationType> ICONNECTOR_T;
  typedef Net_IListener_T<ListenerConfigurationType,
                          ConfigurationType> ILISTENER_T;
  typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                   AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_AsynchStreamConnectionBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                                    const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  // override (part of) Net_ISocketHandler
  virtual ACE_Message_Block* allocateMessage (unsigned int); // requested size

  // *IMPORTANT NOTE*: supports synchronicity-agnostic connections
  inline virtual int open (void* = NULL) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int close (u_long = 0) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }

  Stream_IAllocator* allocator_;
  StreamType         stream_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchStreamConnectionBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchStreamConnectionBase_T (const Net_AsynchStreamConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchStreamConnectionBase_T& operator= (const Net_AsynchStreamConnectionBase_T&))

  using ISTREAM_CONNECTION_T::ISOCKET_CONNECTION_T::ITRANSPORT_LAYER_T::initialize;

  // override some ACE_Handler methods
  virtual void handle_read_stream (const ACE_Asynch_Read_Stream::Result&); // result
  virtual void handle_write_stream (const ACE_Asynch_Write_Stream::Result&); // result
  virtual void handle_read_dgram (const ACE_Asynch_Read_Dgram::Result&); // result
  virtual void handle_write_dgram (const ACE_Asynch_Write_Dgram::Result&); // result

  bool               notify_; // still to notify the processing stream ?
};

// include template definition
#include "net_streamconnection_base.inl"

#endif
