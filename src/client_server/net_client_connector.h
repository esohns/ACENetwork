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

#include "stream_statemachine_control.h"

#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_netlinksockethandler.h"
#endif
#include "net_udpconnection_base.h"

template <typename HandlerType,
          typename ConnectorType, // ACE_SOCK_CONNECTOR
          ///////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          ///////////////////////////////
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename UserDataType>
class Net_Client_Connector_T
 : public ACE_Connector<HandlerType,
                        ConnectorType>
 , public Net_IConnector_T<AddressType,
                           HandlerConfigurationType>
{
 public:
  typedef Net_IConnector_T<AddressType,
                           HandlerConfigurationType> ICONNECTOR_T;
  typedef StreamType STREAM_T;
  typedef Net_IConnection_T<AddressType,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> ICONNECTION_T;
  typedef Net_ISocketConnection_T<AddressType,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  StreamType,
                                  Stream_StateMachine_ControlState,
                                  ///////
                                  Net_SocketConfiguration,
                                  ///////
                                  HandlerConfigurationType> ISOCKET_CONNECTION_T;
  typedef Net_IConnectionManager_T<AddressType,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;
  typedef Net_IConnector_T<AddressType,
                           HandlerConfigurationType> INTERFACE_T;

  Net_Client_Connector_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                          const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  virtual bool useReactor () const; // ? : uses proactor

  // *NOTE*: handlers retrieve the configuration object with get ()
  virtual bool initialize (const HandlerConfigurationType&);
  virtual const HandlerConfigurationType& get () const;

  virtual void abort ();
  virtual ACE_HANDLE connect (const AddressType&);

 protected:
  // override default activation strategy
  virtual int activate_svc_handler (HandlerType*);
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);

 private:
  typedef ACE_Connector<HandlerType,
                        ConnectorType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  HandlerConfigurationType* configuration_;

  ICONNECTION_MANAGER_T*    connectionManager_;
  ACE_Time_Value            statisticCollectionInterval_;
};

/////////////////////////////////////////

// (partial) specializations (for UDP)
template <typename HandlerType,
          typename ConnectorType, // ACE_SOCK_CONNECTOR
          ///////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          ///////////////////////////////
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename UserDataType>
class Net_Client_Connector_T<Net_UDPConnectionBase_T<HandlerType,

                                                     ConfigurationType,
                                                     StateType,
                                                     StatisticContainerType,
                                                     StreamType,

                                                     HandlerConfigurationType,

                                                     UserDataType>,
                             ConnectorType,
                             ////////////
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             StreamType,
                             ////////////
                             HandlerConfigurationType,
                             ////////////
                             UserDataType>
 : public Net_IConnector_T<ACE_INET_Addr,
                           HandlerConfigurationType>
{
 public:
  typedef Net_IConnector_T<ACE_INET_Addr,
                           HandlerConfigurationType> ICONNECTOR_T;
  typedef StreamType STREAM_T;
  typedef Net_IConnection_T<ACE_INET_Addr,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> ICONNECTION_T;
  typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  StreamType,
                                  Stream_StateMachine_ControlState,
                                  ///////
                                  Net_SocketConfiguration,
                                  ///////
                                  HandlerConfigurationType> ISOCKET_CONNECTION_T;
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;
  typedef Net_UDPConnectionBase_T<HandlerType,
                                  ///////
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  StreamType,
                                  ///////
                                  HandlerConfigurationType,
                                  ///////
                                  UserDataType> CONNECTION_T;
  typedef Net_IConnector_T<ACE_INET_Addr,
                           HandlerConfigurationType> INTERFACE_T;

  Net_Client_Connector_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                          const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  virtual bool useReactor () const; // ? : uses proactor

  // *NOTE*: handlers retrieve the configuration object with get ()
  virtual bool initialize (const HandlerConfigurationType&);
  // *TODO*: why is it necessary to provide an implementation when there is (a
  //         more generic) one available ? (gcc complains about abort() and
  //         gets())
  virtual const HandlerConfigurationType& get () const;
  // *NOTE*: this is just a stub
  virtual void abort ();
  // specialize (part of) Net_Client_IConnector_T
  virtual ACE_HANDLE connect (const ACE_INET_Addr&);

 protected:
  //typedef ACE_Connector<CONNECTION_T,
  //                      ACE_SOCK_CONNECTOR> ACE_CONNECTOR_T;

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
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  HandlerConfigurationType* configuration_;

  ICONNECTION_MANAGER_T*    connectionManager_;
  ACE_Time_Value            statisticCollectionInterval_;
};

/////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
// partial specialization (for Netlink)
template <typename HandlerType,
          typename ConnectorType, // ACE_SOCK_CONNECTOR
          ///////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          typename StreamType,
          ///////////////////////////////
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename UserDataType>
class Net_Client_Connector_T<HandlerType,
                             ConnectorType,
                             ////////////
                             Net_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StatisticContainerType,
                             StreamType,
                             ////////////
                             HandlerConfigurationType,
                             ////////////
                             UserDataType>
 : public Net_IConnector_T<Net_Netlink_Addr,
                           HandlerConfigurationType>
{
 public:
  typedef Net_IConnector_T<Net_Netlink_Addr,
                           HandlerConfigurationType> ICONNECTOR_T;
  typedef StreamType STREAM_T;
  typedef Net_IConnection_T<Net_Netlink_Addr,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> ICONNECTION_T;
  typedef Net_ISocketConnection_T<Net_Netlink_Addr,
                                  ConfigurationType,
                                  StateType,
                                  StatisticContainerType,
                                  StreamType,
                                  Stream_StateMachine_ControlState,
                                  ///////
                                  Net_SocketConfiguration,
                                  ///////
                                  HandlerConfigurationType> ISOCKET_CONNECTION_T;
  typedef Net_IConnectionManager_T<Net_Netlink_Addr,
                                   ConfigurationType,
                                   StateType,
                                   StatisticContainerType,
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;
  typedef Net_IConnector_T<Net_Netlink_Addr,
                           HandlerConfigurationType> INTERFACE_T;

  Net_Client_Connector_T (ICONNECTION_MANAGER_T*,                        // connection manager handle
                          const ACE_Time_Value& = ACE_Time_Value::zero); // statistic collecting interval [ACE_Time_Value::zero: off]
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  virtual bool useReactor () const; // ? : uses proactor

  // *NOTE*: handlers retrieve the configuration object with get ()
  virtual bool initialize (const HandlerConfigurationType&);
  virtual const HandlerConfigurationType& get () const;
  // *NOTE*: this is just a stub
  virtual void abort ();
  virtual ACE_HANDLE connect (const Net_Netlink_Addr&);

 protected:
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  HandlerConfigurationType* configuration_;

  ICONNECTION_MANAGER_T*    connectionManager_;
  ACE_Time_Value            statisticCollectionInterval_;
};
#endif

// include template implementation
#include "net_client_connector.inl"

#endif
