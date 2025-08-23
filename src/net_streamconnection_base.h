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

#include "ace/Asynch_IO.h"
#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"
#include "net_connection_base.h"
#include "net_iconnection.h"
#include "net_iconnector.h"
#include "net_ilistener.h"

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
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          typename StreamStatusType, // state machine-
          ////////////////////////////////
          typename UserDataType>
class Net_StreamConnectionBase_T
 : public HandlerType
 , public Net_ConnectionBase_T<ACE_MT_SYNCH, // *TODO*: make this a template parameter
                               AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType>
 , public Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType>
{
  typedef HandlerType inherited;
  typedef Net_ConnectionBase_T<ACE_MT_SYNCH,
                               AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> inherited2;

 public:
  // convenient types
  typedef AddressType addr_type; // required by ACE_Connector
  typedef HandlerType HANDLER_T;
  typedef Net_ConnectionBase_T<ACE_MT_SYNCH,
                               AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> CONNECTION_BASE_T;
  typedef Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType> ISTREAM_CONNECTION_T;
  typedef StreamType STREAM_T;

  virtual ~Net_StreamConnectionBase_T ();

  // override (part of) ACE_Svc_Handler
  virtual int open (void* = NULL); // argument
  virtual int close (u_long = 0); // reason

  // override some ACE_Event_Handler methods
  // *NOTE*: this is called when:
  // - handle_xxx() (see above) returns -1
  virtual int handle_close (ACE_HANDLE = ACE_INVALID_HANDLE,                        // handle
                            ACE_Reactor_Mask = ACE_Event_Handler::ALL_EVENTS_MASK); // event mask

  // override (part of) ACE_Reactor_Notification_Strategy
  // increase the reference count when this is invoked; this prevents the
  // connection from going away when still registered with the reactor for
  // writing
  inline virtual int notify (void) { inherited2::increase (); return inherited::notify (); }

  using inherited2::initialize;

  // implement (part of) Net_IStreamConnection_T
  inline virtual bool initiate_read () { return inherited::registerWithReactor (); }

  virtual bool collect (StatisticContainerType&);
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
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ACE_Notification_Strategy* notification () { return static_cast<HandlerType*> (this); }
  virtual void abort ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?
  // -------------------------------------
  virtual void set (enum Net_ClientServerRole);
  inline virtual const HandlerConfigurationType& getR_2 () const { ACE_ASSERT (inherited::configuration_); return *inherited::configuration_; }
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void send (ACE_Message_Block*&);
  // -------------------------------------
  inline virtual int enqueue_head_i (ACE_Message_Block*, ACE_Time_Value* = 0) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); }
  inline virtual bool hasData () { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); }
  inline virtual unsigned int flush (bool flushSessionMessages_in = false) { return stream_.flush (true, flushSessionMessages_in, false); }
  inline virtual void reset () { ACE_ASSERT (false); ACE_NOTSUP; }
  inline virtual void signal () { ACE_ASSERT (false); ACE_NOTSUP; }
  inline virtual bool isShuttingDown () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); }
  // *NOTE*: this waits for outbound (!) data only
  inline virtual void waitForIdleState (bool waitForever_in = true) const { stream_.idle (waitForever_in, false); }
  // -------------------------------------
  inline virtual const StreamType& stream () const { return stream_; }
  virtual bool wait (StreamStatusType,
                     const ACE_Time_Value* = NULL); // timeout (absolute) ? : block

 protected:
  // *NOTE*: if there is no default ctor, this will not compile
  inline Net_StreamConnectionBase_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  Net_StreamConnectionBase_T (bool); // managed ?

  // override (part of) Net_ISocketHandler
  virtual ACE_Message_Block* allocateMessage (unsigned int); // requested size

  // *IMPORTANT NOTE*: stub to facilitate connection handler encapsulation
  virtual void open (ACE_HANDLE, ACE_Message_Block&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return); }

  Stream_IAllocator* allocator_;
  ACE_SYNCH_MUTEX_T  sendLock_;
  // *NOTE*: support partial writes
  StreamType         stream_;
  ACE_Message_Block* writeBuffer_;

 private:
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
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          typename StreamStatusType,
          ////////////////////////////////
          typename UserDataType>
class Net_AsynchStreamConnectionBase_T
 : public HandlerType
 , public Net_ConnectionBase_T<ACE_MT_SYNCH, // *TODO*: make this a template parameter
                               AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType>
 , public Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType>
{
  typedef HandlerType inherited;
  typedef Net_ConnectionBase_T<ACE_MT_SYNCH,
                               AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> inherited2;

 public:
  // convenient types
  typedef Net_ConnectionBase_T<ACE_MT_SYNCH,
                               AddressType,
                               ConfigurationType,
                               StateType,
                               StatisticContainerType,
                               UserDataType> CONNECTION_BASE_T;
  typedef Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType> ISTREAM_CONNECTION_T;
  typedef StreamType STREAM_T;

  inline virtual ~Net_AsynchStreamConnectionBase_T () {}

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

  using inherited2::initialize;

  // implement (part of) Net_IStreamConnection_T
  virtual bool collect (StatisticContainerType&);
  inline virtual void report () const { stream_.report (); }
  virtual void dump_state () const;
  //inline virtual void info (ACE_HANDLE& handle_out, AddressType& localSAP_out, AddressType& peerSAP_out) const { handle_out = inherited::handle (); localSAP_out = inherited::localSAP_; peerSAP_out = inherited::peerSAP_; }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual Net_ConnectionId_t id () const { return reinterpret_cast<Net_ConnectionId_t> (inherited::handle ()); }
#else
  inline virtual Net_ConnectionId_t id () const { return static_cast<Net_ConnectionId_t> (inherited::handle ()); }
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ACE_Notification_Strategy* notification () { return this; }
  virtual void abort ();
  virtual void waitForCompletion (bool = true); // wait for thread(s) ?
  // -------------------------------------
  virtual void set (enum Net_ClientServerRole);
  inline virtual const HandlerConfigurationType& getR_2 () const { ACE_ASSERT (inherited::configuration_); return *(inherited::configuration_); }
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void send (ACE_Message_Block*&);
  // -------------------------------------
  inline virtual int enqueue_head_i (ACE_Message_Block*, ACE_Time_Value* = 0) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); }
  inline virtual bool hasData () { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); }
  inline virtual unsigned int flush (bool flushSessionMessages_in = false) { return stream_.flush (true, flushSessionMessages_in, false); }
  inline virtual void reset () { ACE_ASSERT (false); ACE_NOTSUP; }
  inline virtual void signal () { ACE_ASSERT (false); ACE_NOTSUP; }
  inline virtual bool isShuttingDown () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); }
  // *NOTE*: this waits for outbound (!) data only
  virtual void waitForIdleState (bool = true) const; // wait forever ?
  // -------------------------------------
  inline virtual const StreamType& stream () const { return stream_; }
  virtual bool wait (StreamStatusType,
                     const ACE_Time_Value* = NULL); // timeout (absolute) ? : block

 protected:
  typedef HandlerType HANDLER_T;

  // *NOTE*: if there is no default ctor, this will not compile
  inline Net_AsynchStreamConnectionBase_T () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  Net_AsynchStreamConnectionBase_T (bool); // managed ?

  // override (part of) Net_ISocketHandler
  virtual ACE_Message_Block* allocateMessage (unsigned int); // requested size
  virtual bool initiate_read ();

  // *IMPORTANT NOTE*: supports synchronicity-agnostic connections
  inline virtual int open (void* = NULL) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int close (u_long = 0) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }

  Stream_IAllocator* allocator_;
  StreamType         stream_;

 private:
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
