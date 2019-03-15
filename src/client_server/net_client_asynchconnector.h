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

#ifndef NET_CLIENT_ASYNCHCONNECTOR_H
#define NET_CLIENT_ASYNCHCONNECTOR_H

#include "ace/Asynch_Connector.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_timer_manager_common.h"

#include "stream_statemachine_common.h"

#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_udpconnection_base.h"

template <typename HandlerType,
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType, // connection-
          typename StatisticContainerType,
          ////////////////////////////////
          typename SocketConfigurationType, // *NOTE*: ATM these two are the same type
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_Client_AsynchConnector_T
 : public ACE_Asynch_Connector<HandlerType>
 , public Net_IAsynchConnector_T<AddressType,
                                 ConfigurationType>
{
  typedef ACE_Asynch_Connector<HandlerType> inherited;

 public:
  typedef AddressType ADDRESS_T;
  typedef StreamType STREAM_T;

  typedef Net_IConnection_T<AddressType,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> ICONNECTION_T;
  typedef Net_IStreamConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  SocketConfigurationType,
                                  HandlerConfigurationType,
                                  StreamType,
                                  enum Stream_StateMachine_ControlState> ISTREAM_CONNECTION_T;

  typedef Net_IAsynchConnector_T<AddressType,
                                 ConfigurationType> ICONNECTOR_T;
  typedef ICONNECTOR_T IASYNCH_CONNECTOR_T;

  Net_Client_AsynchConnector_T (bool = true); // managed ?
  virtual ~Net_Client_AsynchConnector_T ();

  // implement Net_IAsynchConnector_T
  // *NOTE*: handlers receive the configuration object via
  //         ACE_Service_Handler::act ()
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_); return *configuration_; }
  inline virtual bool initialize (const ConfigurationType& configuration_in) { configuration_ = &const_cast<ConfigurationType&> (configuration_in); return true; }
  virtual enum Net_TransportLayerType transportLayer () const;
  inline virtual bool useReactor () const { return false; }
  virtual ACE_HANDLE connect (const AddressType&);
  virtual void abort ();
  virtual int wait (ACE_HANDLE,                                    // connect handle
                    const ACE_Time_Value& = ACE_Time_Value::zero); // block : (relative-) timeout
  virtual void onConnect (ACE_HANDLE, // connect handle
                          int);       // success ? 0 : errno

 protected:
  // override default connect strategy
  virtual void handle_connect (const ACE_Asynch_Connect::Result&);
  // override default creation strategy
  virtual HandlerType* make_handler (void);

  ConfigurationType*     configuration_; // connection-

  typedef std::map<ACE_HANDLE, int> HANDLE_TO_ERROR_MAP_T;
  typedef HANDLE_TO_ERROR_MAP_T::iterator HANDLE_TO_ERROR_MAP_ITERATOR_T;

  HANDLE_TO_ERROR_MAP_T  handles_;
  bool                   managed_;
  AddressType            SAP_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T (const Net_Client_AsynchConnector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T& operator= (const Net_Client_AsynchConnector_T&))

  // convenient types
  typedef Net_Client_AsynchConnector_T<HandlerType,
                                       AddressType,
                                       ConfigurationType,
                                       StateType,
                                       StatisticContainerType,
                                       SocketConfigurationType,
                                       HandlerConfigurationType,
                                       StreamType,
                                       UserDataType> OWN_TYPE_T;
  typedef Net_ITransportLayer_T<SocketConfigurationType> ITRANSPORTLAYER_T;

  // override default connect strategy
  // *TODO*: currently tailored for TCP only (see implementation)
  virtual int connect (const AddressType&,                                         // remote address
                       const AddressType& = (const AddressType&)ACE_Addr::sap_any, // local address
                       int = 1,                                                    // SO_REUSEADDR ?
                       const void* = 0);                                           // ACT
  // override default validation strategy
  virtual int validate_connection (const ACE_Asynch_Connect::Result&, // result
                                   const AddressType&,                // remote address
                                   const AddressType&);               // local address

  ACE_SYNCH_CONDITION    condition_;
  ACE_SYNCH_MUTEX        lock_;
};

//////////////////////////////////////////

// partial specialization (for UDP)
template <typename HandlerType,
          ////////////////////////////////
          typename ConfigurationType, // connection-
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_Client_AsynchConnector_T<HandlerType,
                                   ACE_INET_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   Net_UDPSocketConfiguration_t,
                                   HandlerConfigurationType,
                                   StreamType,
                                   UserDataType>
 : public ACE_Asynch_Connector<HandlerType>
 , public Net_IAsynchConnector_T<ACE_INET_Addr,
                                 ConfigurationType>
{
  typedef ACE_Asynch_Connector<HandlerType> inherited;

 public:
  typedef ACE_INET_Addr ADDRESS_T;
  typedef StreamType STREAM_T;

  typedef Net_IConnection_T<ACE_INET_Addr,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> ICONNECTION_T;
  typedef HandlerType CONNECTION_T;
  typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  Net_UDPSocketConfiguration_t,
                                  HandlerConfigurationType,
                                  StreamType,
                                  enum Stream_StateMachine_ControlState> ISTREAM_CONNECTION_T;

  typedef Net_IAsynchConnector_T<ACE_INET_Addr,
                                 ConfigurationType> ICONNECTOR_T;
  typedef ICONNECTOR_T IASYNCH_CONNECTOR_T;

  Net_Client_AsynchConnector_T (bool = true); // managed ?
  inline virtual ~Net_Client_AsynchConnector_T () {}

  // implement Net_IAsynchConnector_T
  // *NOTE*: handlers receive the configuration object via
  //         ACE_Service_Handler::act ()
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_); return *configuration_; }
  inline virtual bool initialize (const ConfigurationType& configuration_in) { configuration_ = &const_cast<ConfigurationType&> (configuration_in); return true; }
  inline virtual enum Net_TransportLayerType transportLayer () const { return NET_TRANSPORTLAYER_UDP; }
  inline virtual bool useReactor () const { return false; }
  // *NOTE*: UDP is a datagram-based protocol. The Berkeley (datagram) socket
  //         API is essentially stateless from a user-space perspective (send/
  //         recv only) and thus does not translate to a client/server role
  //         concept very intuitively; some conceptual varnish is in order
  // *CONCEPT*: if the user provides a 'remote' socket address, then the
  //            connection will be 'write-only' (i.e. does not register with the
  //            proactor immediately), and assigned a 'server' role. Otherwise
  //            the connection is assumed to be 'read-write' and assigned a
  //            'client' role. Note that 'client' connections currently need two
  //            distinct sockets to implement 'read-write' semantics and may (!)
  //            therefore also maintain two socket handles
  virtual ACE_HANDLE connect (const ACE_INET_Addr&);
  inline virtual void abort () {}
  inline virtual int wait (ACE_HANDLE handle_in, const ACE_Time_Value& timeout_in = ACE_Time_Value::zero) { ACE_UNUSED_ARG (timeout_in); return ((handle_in != ACE_INVALID_HANDLE) ? 0 : -1); } // block : (relative-) timeout
  inline virtual void onConnect (ACE_HANDLE, int) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 protected:
  // override default creation strategy
  virtual HandlerType* make_handler (void);

  ConfigurationType*     configuration_; // connection-
  bool                   managed_;
  ACE_INET_Addr          SAP_;

 private:
  // convenient types
  typedef Net_ITransportLayer_T<Net_UDPSocketConfiguration_t> ITRANSPORTLAYER_T;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T (const Net_Client_AsynchConnector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T& operator= (const Net_Client_AsynchConnector_T&))

  // override default connect strategy
  virtual int validate_connection (const ACE_Asynch_Connect::Result&, // result
                                   const ACE_INET_Addr&,              // remote address
                                   const ACE_INET_Addr&);             // local address

  ACE_SYNCH_CONDITION    condition_;
  ACE_SYNCH_MUTEX        lock_;
};

//////////////////////////////////////////

#if defined (ACE_HAS_NETLINK) && defined (NETLINK_SUPPORT)
// partial specialization (for Netlink)
template <typename HandlerType,
          ////////////////////////////////
          typename ConfigurationType, // connection-
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_Client_AsynchConnector_T<HandlerType,
                                   Net_Netlink_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   Net_NetlinkSocketConfiguration_t,
                                   HandlerConfigurationType,
                                   StreamType,
                                   UserDataType>
 : public ACE_Asynch_Connector<HandlerType>
 , public Net_IAsynchConnector_T<Net_Netlink_Addr,
                                 ConfigurationType>
{
  typedef ACE_Asynch_Connector<HandlerType> inherited;

 public:
  typedef Net_Netlink_Addr ADDRESS_T;
  typedef StreamType STREAM_T;

  typedef Net_IConnection_T<Net_Netlink_Addr,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> ICONNECTION_T;
  typedef Net_IStreamConnection_T<Net_Netlink_Addr,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  Net_NetlinkSocketConfiguration_t,
                                  HandlerConfigurationType,
                                  StreamType,
                                  enum Stream_StateMachine_ControlState> ISTREAM_CONNECTION_T;

  typedef Net_IAsynchConnector_T<Net_Netlink_Addr,
                                 ConfigurationType> ICONNECTOR_T;
  typedef ICONNECTOR_T IASYNCH_CONNECTOR_T;

  Net_Client_AsynchConnector_T (bool = true); // managed ?
  inline virtual ~Net_Client_AsynchConnector_T () {}

  // implement Net_IAsynchConnector_T
  // *NOTE*: handlers receive the configuration object via
  //         ACE_Service_Handler::act ()
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_); return *configuration_; }
  inline virtual bool initialize (const ConfigurationType& configuration_in) { configuration_ = &const_cast<ConfigurationType&> (configuration_in); return true; }
  inline virtual enum Net_TransportLayerType transportLayer () const { return NET_TRANSPORTLAYER_NETLINK; }
  inline virtual bool useReactor () const { return false; }
  virtual ACE_HANDLE connect (const Net_Netlink_Addr&);
  inline virtual void abort () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual int wait (ACE_HANDLE, const ACE_Time_Value& = ACE_Time_Value::zero) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) } // block : (relative-) timeout
  inline virtual void onConnect (ACE_HANDLE, int) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 protected:
  // override default creation strategy
  virtual HandlerType* make_handler (void);

  ConfigurationType*     configuration_; // connection-
  bool                   managed_;
  Net_Netlink_Addr       SAP_;

 private:
  // convenient types
  typedef Net_ITransportLayer_T<Net_NetlinkSocketConfiguration_t> ITRANSPORTLAYER_T;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T (const Net_Client_AsynchConnector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T& operator= (const Net_Client_AsynchConnector_T&))

  // override default connect strategy
  virtual int validate_connection (const ACE_Asynch_Connect::Result&, // result
                                   const Net_Netlink_Addr&,           // remote address
                                   const Net_Netlink_Addr&);          // local address

  ACE_SYNCH_CONDITION    condition_;
  ACE_SYNCH_MUTEX        lock_;
};
#endif // ACE_HAS_NETLINK && NETLINK_SUPPORT

// include template definition
#include "net_client_asynchconnector.inl"

#endif
