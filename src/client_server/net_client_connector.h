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

#ifndef NET_CLIENT_CONNECTOR_H
#define NET_CLIENT_CONNECTOR_H

#include "ace/Connector.h"
#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_timer_manager_common.h"

#include "stream_statemachine_control.h"

#include "net_common.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_udpconnection_base.h"

template <ACE_SYNCH_DECL, // 'send' lock strategy
          typename HandlerType, // implements Net_ConnectionBase_T
          typename ConnectorType, // ACE_SOCK_CONNECTOR
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType, // connection-
          typename StatisticContainerType,
          ////////////////////////////////
          typename SocketConfigurationType,
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_Client_Connector_T
 : public ACE_Connector<HandlerType,
                        ConnectorType>
 , public Net_IConnector_T<AddressType,
                           ConfigurationType>
{
  typedef ACE_Connector<HandlerType,
                        ConnectorType> inherited;

 public:
  // convenient types
  typedef AddressType ADDRESS_T;
  typedef StreamType STREAM_T;
  typedef ACE_Connector<HandlerType,
                        ConnectorType> CONNECTOR_T;

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

  typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                   AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> CONNECTION_MANAGER_T;
  typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                   AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  typedef Net_IConnector_T<AddressType,
                           ConfigurationType> ICONNECTOR_T;
  typedef Net_IAsynchConnector_T<AddressType,
                                 ConfigurationType> IASYNCH_CONNECTOR_T;

  Net_Client_Connector_T (ICONNECTION_MANAGER_T* = NULL,                 // connection manager handle
                          const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  virtual enum Net_TransportLayerType transportLayer () const;
  inline virtual bool useReactor () const { return true; };
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_); return *configuration_; };
  inline virtual bool initialize (const ConfigurationType& configuration_in) { configuration_ = &const_cast<ConfigurationType&> (configuration_in); configuration_->socketHandlerConfiguration.connectionConfiguration = configuration_; return true; };
  virtual ACE_HANDLE connect (const AddressType&);

 protected:
  // override default activation strategy
  virtual int activate_svc_handler (HandlerType*);
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);

 private:
  // convenient types
  typedef Net_Client_Connector_T<ACE_SYNCH_USE,
                                 HandlerType,
                                 ConnectorType,
                                 AddressType,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 SocketConfigurationType,
                                 HandlerConfigurationType,
                                 StreamType,
                                 UserDataType> OWN_TYPE_T;
  typedef Net_ITransportLayer_T<SocketConfigurationType> ITRANSPORTLAYER_T;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  ConfigurationType*     configuration_; // connection-

  ICONNECTION_MANAGER_T* connectionManager_;
  ACE_Time_Value         statisticCollectionInterval_;
};

//////////////////////////////////////////

// specialization (for UDP)
template <ACE_SYNCH_DECL, // 'send' lock strategy
          typename HandlerType, // implements Net_ConnectionBase_T
          typename ConnectorType, // ACE_SOCK_CONNECTOR
          ////////////////////////////////
          typename ConfigurationType, // connection-
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_Client_Connector_T<ACE_SYNCH_USE,
                             HandlerType,
                             ConnectorType,
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             struct Net_UDPSocketConfiguration,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>
 : public Net_IConnector_T<ACE_INET_Addr,
                           ConfigurationType>
{
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
                                  struct Net_UDPSocketConfiguration,
                                  HandlerConfigurationType,
                                  StreamType,
                                  enum Stream_StateMachine_ControlState> ISTREAM_CONNECTION_T;

  typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                   ACE_INET_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> CONNECTION_MANAGER_T;
  typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                   ACE_INET_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  typedef Net_IConnector_T<ACE_INET_Addr,
                           ConfigurationType> ICONNECTOR_T;
  typedef Net_IAsynchConnector_T<ACE_INET_Addr,
                                 ConfigurationType> IASYNCH_CONNECTOR_T;

  Net_Client_Connector_T (ICONNECTION_MANAGER_T* = NULL,                 // connection manager handle
                          const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  inline virtual ~Net_Client_Connector_T () {};

  // implement Net_Client_IConnector_T
  inline virtual enum Net_TransportLayerType transportLayer () const { return NET_TRANSPORTLAYER_UDP; };
  inline virtual bool useReactor () const { return true; };
  // *NOTE*: handlers retrieve the configuration object with get ()
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_); return *configuration_; };
  inline virtual bool initialize (const ConfigurationType& configuration_in) { configuration_ = &const_cast<ConfigurationType&> (configuration_in); configuration_->socketHandlerConfiguration.connectionConfiguration = configuration_; return true; };
  // specialize (part of) Net_IConnector_T
  virtual ACE_HANDLE connect (const ACE_INET_Addr&);

 protected:
  // override default activation strategy
  virtual int activate_svc_handler (CONNECTION_T*);
  // override default instantiation strategy
  virtual int make_svc_handler (CONNECTION_T*&);
  //virtual int connect_svc_handler (CONNECTION_T*&,
  //                                 const ACE_SOCK_Connector::PEER_ADDR&,
  //                                 ACE_Time_Value*,
  //                                 const ACE_SOCK_Connector::PEER_ADDR&,
  //                                 int,
  //                                 int,
  //                                 int);
  //// *TODO*: it's not quite clear what this API does (see Connector.h:514),
  ////         needs overriding to please the compiler...
  //virtual int connect_svc_handler (CONNECTION_T*&,
  //                                 CONNECTION_T*&,
  //                                 const ACE_SOCK_Connector::PEER_ADDR&,
  //                                 ACE_Time_Value*,
  //                                 const ACE_SOCK_Connector::PEER_ADDR&,
  //                                 int,
  //                                 int,
  //                                 int);

 private:
  // convenient types
  typedef Net_Client_Connector_T<ACE_SYNCH_USE,
                                 HandlerType,
                                 ConnectorType,
                                 ACE_INET_Addr,
                                 ConfigurationType,
                                 StateType,
                                 StatisticContainerType,
                                 struct Net_UDPSocketConfiguration,
                                 HandlerConfigurationType,
                                 StreamType,
                                 UserDataType> OWN_TYPE_T;

  //ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  // convenient types
  typedef Net_ITransportLayer_T<struct Net_UDPSocketConfiguration> ITRANSPORTLAYER_T;

  ConfigurationType*     configuration_; // connection-

  ICONNECTION_MANAGER_T* connectionManager_;
  ACE_Time_Value         statisticCollectionInterval_;
};

//////////////////////////////////////////

#if defined (ACE_HAS_NETLINK)
// specialization (for Netlink)
template <ACE_SYNCH_DECL, // 'send' lock strategy
          typename HandlerType, // implements Net_ConnectionBase_T
          typename ConnectorType, // ACE_SOCK_CONNECTOR
          ////////////////////////////////
          typename ConfigurationType, // connection-
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename StreamType,
          ////////////////////////////////
          typename UserDataType>
class Net_Client_Connector_T<ACE_SYNCH_USE,
                             HandlerType,
                             ConnectorType,
                             Net_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             struct Net_NetlinkSocketConfiguration,
                             HandlerConfigurationType,
                             StreamType,
                             UserDataType>
 : public Net_IConnector_T<Net_Netlink_Addr,
                           ConfigurationType>
{
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
                                  struct Net_NetlinkSocketConfiguration,
                                  HandlerConfigurationType,
                                  StreamType,
                                  enum Stream_StateMachine_ControlState> ISTREAM_CONNECTION_T;

  typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                   Net_Netlink_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> CONNECTION_MANAGER_T;
  typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                   Net_Netlink_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   UserDataType> ICONNECTION_MANAGER_T;

  typedef Net_IConnector_T<Net_Netlink_Addr,
                           ConfigurationType> ICONNECTOR_T;
  typedef Net_IAsynchConnector_T<Net_Netlink_Addr,
                                 ConfigurationType> IASYNCH_CONNECTOR_T;

  Net_Client_Connector_T (ICONNECTION_MANAGER_T* = NULL,                 // connection manager handle
                          const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  inline virtual ~Net_Client_Connector_T () {};

  // implement Net_IConnector_T
  inline virtual enum Net_TransportLayerType transportLayer () const { return NET_TRANSPORTLAYER_NETLINK; };
  inline virtual bool useReactor () const { return true; };
  // *NOTE*: handlers retrieve the configuration object with get ()
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_); return *configuration_; };
  inline virtual bool initialize (const ConfigurationType& configuration_in) { configuration_ = &const_cast<ConfigurationType&> (configuration_in); configuration_->socketHandlerConfiguration.connectionConfiguration = configuration_; return true; };
  virtual ACE_HANDLE connect (const Net_Netlink_Addr&);

 protected:
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  // convenient types
  typedef Net_ITransportLayer_T<struct Net_NetlinkSocketConfiguration> ITRANSPORTLAYER_T;

  ConfigurationType*     configuration_; // connection-

  ICONNECTION_MANAGER_T* connectionManager_;
  ACE_Time_Value         statisticCollectionInterval_;
};
#endif

// include template definition
#include "net_client_connector.inl"

#endif
