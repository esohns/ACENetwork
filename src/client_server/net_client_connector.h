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
#include "ace/Netlink_Addr.h"
#include "ace/SOCK_Connector.h"

#include "stream_common.h"

#include "net_connection_manager_common.h"
#include "net_iconnector.h"
#include "net_netlinkconnection.h"
#include "net_udpconnection.h"

template <typename HandlerType,
          ///////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          ///////////////////////////////
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename UserDataType>
class Net_Client_Connector_T
 : public ACE_Connector<HandlerType,
                        ACE_SOCK_CONNECTOR>
 , public Net_IConnector_T<AddressType,
                           HandlerConfigurationType>
{
 public:
  typedef Net_IConnectionManager_T<AddressType,
                                   ConfigurationType,
                                   StateType,
                                   Stream_Statistic,
                                   StreamType,
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_Client_Connector_T (ICONNECTION_MANAGER_T*, // connection manager handle
                          unsigned int = 0);      // statistics collecting interval (second(s)) [0: off]
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  //virtual bool useReactor () const; // ? : uses proactor

  // *NOTE*: handlers retrieve the configuration object with get ()
  virtual bool initialize (const HandlerConfigurationType&);
  virtual const HandlerConfigurationType& get () const;

  virtual void abort ();
  virtual ACE_HANDLE connect (const AddressType&);

 protected:
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);

 private:
  typedef ACE_Connector<HandlerType,
                        ACE_SOCK_CONNECTOR> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  HandlerConfigurationType configuration_;

  ICONNECTION_MANAGER_T*   connectionManager_;
  unsigned int             statisticCollectionInterval_;
};

/////////////////////////////////////////

// partial specialization (for UDP)
template <typename HandlerType,
          ///////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          ///////////////////////////////
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename UserDataType>
class Net_Client_Connector_T<Net_UDPConnection_T<HandlerType,

                                                 StateType,

                                                 HandlerConfigurationType,

                                                 UserDataType>,
                             ////////////
                             ACE_INET_Addr,
                             ConfigurationType,
                             StateType,
                             StreamType,
                             ////////////
                             HandlerConfigurationType,
                             ////////////
                             UserDataType>
 : public Net_IConnector_T<ACE_INET_Addr,
                           HandlerConfigurationType>
{
  public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   ConfigurationType,
                                   StateType,
                                   Stream_Statistic,
                                   StreamType,
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;
  typedef Net_UDPConnection_T<HandlerType,
                              ///////////
                              StateType,
                              ///////////
                              HandlerConfigurationType,
                              ///////////
                              UserDataType> CONNECTION_T;

  Net_Client_Connector_T (ICONNECTION_MANAGER_T*, // connection manager handle
                          unsigned int = 0);      // statistics collecting interval (second(s)) [0: off]
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  //virtual bool useReactor () const; // ? : uses proactor

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
  // override default instantiation strategy
  virtual int make_svc_handler (CONNECTION_T*&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  HandlerConfigurationType configuration_;

  ICONNECTION_MANAGER_T*   connectionManager_;
  unsigned int             statisticCollectionInterval_;
};

/////////////////////////////////////////

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
// partial specialization (for Netlink)
template <typename HandlerType,
          ///////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename StreamType,
          ///////////////////////////////
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename UserDataType>
class Net_Client_Connector_T<HandlerType,
                             ////////////
                             ACE_Netlink_Addr,
                             ConfigurationType,
                             StateType,
                             StreamType,
                             ////////////
                             HandlerConfigurationType,
                             ////////////
                             UserDataType>
 : public Net_IConnector_T<ACE_Netlink_Addr,
                           HandlerConfigurationType>
{
 public:
  typedef Net_IConnectionManager_T<ACE_Netlink_Addr,
                                   ConfigurationType,
                                   StateType,
                                   Stream_Statistic,
                                   StreamType,
                                   //////
                                   UserDataType> ICONNECTION_MANAGER_T;

  Net_Client_Connector_T (ICONNECTION_MANAGER_T*, // connection manager handle
                          unsigned int = 0);      // statistics collecting interval (second(s)) [0: off]
  virtual ~Net_Client_Connector_T ();

  // implement Net_Client_IConnector_T
  //virtual bool useReactor () const; // ? : uses proactor

  // *NOTE*: handlers retrieve the configuration object with get ()
  virtual bool initialize (const HandlerConfigurationType&);
  virtual const HandlerConfigurationType& get () const;
  // *NOTE*: this is just a stub
  virtual void abort ();
  virtual ACE_HANDLE connect (const ACE_Netlink_Addr&);

 protected:
  // override default instantiation strategy
  virtual int make_svc_handler (HandlerType*&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T (const Net_Client_Connector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_Connector_T& operator= (const Net_Client_Connector_T&))

  HandlerConfigurationType configuration_;

  ICONNECTION_MANAGER_T*   connectionManager_;
  unsigned int             statisticCollectionInterval_;
};
#endif

// include template implementation
#include "net_client_connector.inl"

#endif
