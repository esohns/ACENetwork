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

#include <ace/config-macros.h>
//#include <ace/Connector.h>
#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>
//#include <ace/SOCK_Connector.h>
#include <ace/Time_Value.h>

#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_ilistener.h"

template <typename HandlerType,
          ///////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ///////////////////////////////
          typename SocketConfigurationType,
          typename HandlerConfigurationType, // socket-
          ///////////////////////////////
          typename StreamType,
          typename StreamStatusType,
          ///////////////////////////////
          typename UserDataType>
class Net_StreamConnectionBase_T
 : public HandlerType
 , public Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  SocketConfigurationType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType>
{
 public:
  virtual ~Net_StreamConnectionBase_T ();

  // override some task-based members
  virtual int open (void* = NULL); // arg
  virtual int close (u_long = 0); // arg (reason)
//  virtual int svc (void);

  // implement (part of) Net_ISocketConnection_T
  //virtual const HandlerConfigurationType& get () const;
  virtual const StreamType& stream () const;
  // *IMPORTANT NOTE*: fire-and-forget API
  //virtual bool send (ACE_Message_Block*&);
  virtual void send (ACE_Message_Block*&);
  virtual bool wait (StreamStatusType,
                     const ACE_Time_Value* = NULL); // timeout (absolute) ? : block
  virtual const HandlerConfigurationType& get ();
  virtual bool initialize (const HandlerConfigurationType&); // handler configuration

  virtual void set (Net_ClientServerRole);
  //virtual void finalize ();

//  // *NOTE*: enqueue any received data onto our stream for further processing
//  virtual int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE);
//  // *NOTE*: this is called when:
//  // handle_xxx() returns -1
//  virtual int handle_close (ACE_HANDLE,
//                            ACE_Reactor_Mask);

 // convenient typedefs
 typedef Net_ITransportLayer_T<SocketConfigurationType> ITRANSPORTLAYER_T;

 protected:
  typedef StreamType STREAM_T;
  //typedef ACE_Connector<HandlerType,
  //                      ACE_SOCK_CONNECTOR> ACE_CONNECTOR_T;
  typedef Net_IConnector_T<AddressType,
                           HandlerConfigurationType> ICONNECTOR_T;
  typedef Net_IListener_T<ConfigurationType,
                          HandlerConfigurationType> ILISTENER_T;
  typedef Net_IConnectionManager_T<AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_StreamConnectionBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                              const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  // helper methods
  // *IMPORTANT NOTE*: dummy stub to integrate asynch/synch connection handlers
  virtual void open (ACE_HANDLE,          // handle
                     ACE_Message_Block&); // (initial) data (if any)

  // *TODO*: move this into the handler
  HandlerConfigurationType configuration_;

 private:
  typedef HandlerType inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_StreamConnectionBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_StreamConnectionBase_T (const Net_StreamConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_StreamConnectionBase_T& operator= (const Net_StreamConnectionBase_T&))

  // helper methods
//  void shutdown ();
};

/////////////////////////////////////////

template <typename HandlerType,
          ///////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ///////////////////////////////
          typename SocketConfigurationType,
          typename HandlerConfigurationType, // socket-
          ///////////////////////////////
          typename StreamType,
          typename StreamStatusType,
          ///////////////////////////////
          typename UserDataType>
class Net_AsynchStreamConnectionBase_T
 : public HandlerType
 , public Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  SocketConfigurationType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  StreamStatusType>
{
 public:
  virtual ~Net_AsynchStreamConnectionBase_T ();

  // override some ACE_Service_Handler members
  using HandlerType::open;
  //virtual void open (ACE_HANDLE,          // handle
  //                   ACE_Message_Block&); // (initial) data (if any)
  virtual void act (const void*); // act

  // implement (part of) Net_IStreamConnection_T
  //virtual const HandlerConfigurationType& get () const;
  virtual const StreamType& stream () const;
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void send (ACE_Message_Block*&);
  virtual bool wait (StreamStatusType,
                     const ACE_Time_Value* = NULL); // timeout (absolute) ? : block
  virtual const HandlerConfigurationType& get ();
  virtual bool initialize (const HandlerConfigurationType&); // handler configuration

  virtual void set (Net_ClientServerRole);
  //virtual void finalize ();

  // convenient typedefs
  typedef Net_ITransportLayer_T<SocketConfigurationType> ITRANSPORTLAYER_T;

 protected:
  typedef StreamType STREAM_T;
  typedef Net_IConnector_T<AddressType,
                           HandlerConfigurationType> ICONNECTOR_T;
  typedef Net_IListener_T<ConfigurationType,
                          HandlerConfigurationType> ILISTENER_T;
  typedef Net_IConnectionManager_T<AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_AsynchStreamConnectionBase_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                                    const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]

  // helper methods
  // *IMPORTANT NOTE*: dummy stub to integrate asynch/synch connection handlers
  virtual int open (void*); // arg

  // *TODO*: move this into the handler
  HandlerConfigurationType configuration_;

 private:
  typedef HandlerType inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_AsynchStreamConnectionBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchStreamConnectionBase_T (const Net_AsynchStreamConnectionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_AsynchStreamConnectionBase_T& operator= (const Net_AsynchStreamConnectionBase_T&))
};

// include template definition
#include "net_streamconnection_base.inl"

#endif
