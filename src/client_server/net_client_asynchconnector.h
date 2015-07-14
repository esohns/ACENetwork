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
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/Netlink_Addr.h"
#endif

#include "stream_common.h"

#include "net_connection_manager_common.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "net_netlinkconnection.h"
#endif
#include "net_udpconnection.h"

#include "net_client_iconnector.h"

template <typename AddressType,
          typename SocketConfigurationType,
          typename ConfigurationType,
          typename ConnectorConfigurationType,
          typename UserDataType,
          typename StateType,
          typename StreamType,
          typename HandlerType>
class Net_Client_AsynchConnector_T
 : public ACE_Asynch_Connector<HandlerType>
 , public Net_Client_IConnector_T<AddressType,
                                  ConnectorConfigurationType>
{
 public:
  typedef Net_IConnectionManager_T<AddressType,
                                   SocketConfigurationType,
                                   ConfigurationType,
                                   UserDataType,
                                   StateType,
                                   Stream_Statistic,
                                   StreamType> ICONNECTION_MANAGER_T;

  Net_Client_AsynchConnector_T ();
  virtual ~Net_Client_AsynchConnector_T ();

  // override default connect strategy
  virtual int validate_connection (const ACE_Asynch_Connect::Result&, // result
                                   const ACE_INET_Addr&,              // remote address
                                   const ACE_INET_Addr&);             // local address

  // implement Net_Client_IConnector_T

  // *IMPORTANT NOTE*: fire-and-forget API: the first (!) generated handler
  //                   assumes responsibility for the socket handler
  //                   configuration (member). The handler retrieves the
  //                   configuration object via act ()
  //                   --> instances must be (re)initialize()d for every new
  //                       connection
  virtual bool initialize (const ConnectorConfigurationType&);
  // *NOTE*: this is just a stub
  virtual const ConnectorConfigurationType& get () const;

  virtual bool useReactor () const; // ? : uses proactor

  virtual void abort ();
  virtual ACE_HANDLE connect (const AddressType&);

 protected:
  // override default creation strategy
  virtual HandlerType* make_handler (void);

 private:
  typedef ACE_Asynch_Connector<HandlerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T (const Net_Client_AsynchConnector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T& operator= (const Net_Client_AsynchConnector_T&))

  ConnectorConfigurationType configuration_;
};

/////////////////////////////////////////

// partial specialization (for UDP)
template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename ConnectorConfigurationType,
          typename UserDataType,
          typename StateType,
          typename StreamType>
class Net_Client_AsynchConnector_T<ACE_INET_Addr,
                                   SocketConfigurationType,
                                   ConfigurationType,
                                   ConnectorConfigurationType,
                                   UserDataType,
                                   StateType,
                                   StreamType,
                                   Net_AsynchUDPConnection_T<UserDataType,
                                                             StateType,
                                                             HandlerType,
                                                             ConnectorConfigurationType> >
 : public ACE_Asynch_Connector<Net_AsynchUDPConnection_T<UserDataType,
                                                         StateType,
                                                         HandlerType,
                                                         ConnectorConfigurationType> >
 , public Net_Client_IConnector_T<ACE_INET_Addr,
                                  ConnectorConfigurationType>
{
 public:
  typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                   SocketConfigurationType,
                                   ConfigurationType,
                                   UserDataType,
                                   StateType,
                                   Stream_Statistic,
                                   StreamType> ICONNECTION_MANAGER_T;
  typedef Net_AsynchUDPConnection_T<UserDataType,
                                    StateType,
                                    HandlerType,
                                    ConnectorConfigurationType> CONNECTION_T;

  Net_Client_AsynchConnector_T ();
  virtual ~Net_Client_AsynchConnector_T ();

  // override default connect strategy
  virtual int validate_connection (const ACE_Asynch_Connect::Result&, // result
                                   const ACE_INET_Addr&,              // remote address
                                   const ACE_INET_Addr&);             // local address

  // implement Net_Client_IConnector_T

  // *IMPORTANT NOTE*: fire-and-forget API: the first (!) generated handler
  //                   assumes responsibility for the socket handler
  //                   configuration (member). The handler retrieves the
  //                   configuration object via act ()
  //                   --> instances must be (re)initialize()d for every new
  //                       connection
  virtual bool initialize (const ConnectorConfigurationType&);
  // *NOTE*: this is just a stub
  virtual const ConnectorConfigurationType& get () const;

  virtual bool useReactor () const; // ? : uses proactor

  virtual void abort ();
  virtual ACE_HANDLE connect (const ACE_INET_Addr&);

 protected:
  // override default creation strategy
  //virtual CONNECTION_T* make_handler (void);
   virtual Net_AsynchUDPConnection_T<UserDataType,
                                     StateType,
                                     HandlerType,
                                     ConnectorConfigurationType>* make_handler (void);

 private:
  typedef ACE_Asynch_Connector<Net_AsynchUDPConnection_T<UserDataType,
                                                         StateType,
                                                         HandlerType,
                                                         ConnectorConfigurationType> > inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T (const Net_Client_AsynchConnector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T& operator= (const Net_Client_AsynchConnector_T&))

  ConnectorConfigurationType configuration_;
};

/////////////////////////////////////////

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
// partial specialization (for Netlink)
template <typename SocketConfigurationType,
          typename HandlerType,
          typename ConfigurationType,
          typename ConnectorConfigurationType,
          typename UserDataType,
          typename StateType,
          typename StreamType>
class Net_Client_AsynchConnector_T<ACE_Netlink_Addr,
                                   SocketConfigurationType,
                                   ConfigurationType,
                                   ConnectorConfigurationType,
                                   UserDataType,
                                   StateType,
                                   StreamType,
                                   HandlerType>
 : public ACE_Asynch_Connector<HandlerType>
 , public Net_Client_IConnector_T<ACE_Netlink_Addr,
                                  ConnectorConfigurationType>
{
 public:
  typedef Net_IConnectionManager_T<ACE_Netlink_Addr,
                                   SocketConfigurationType,
                                   ConfigurationType,
                                   UserDataType,
                                   StateType,
                                   Stream_Statistic,
                                   StreamType> ICONNECTION_MANAGER_T;

  Net_Client_AsynchConnector_T ();
  virtual ~Net_Client_AsynchConnector_T ();

  // override default connect strategy
  virtual int validate_connection (const ACE_Asynch_Connect::Result&, // result
                                   const ACE_Netlink_Addr&,           // remote address
                                   const ACE_Netlink_Addr&);          // local address

  // implement Net_Client_IConnector_T

  // *IMPORTANT NOTE*: fire-and-forget API: the first (!) generated handler
  //                   assumes responsibility for the socket handler
  //                   configuration (member). The handler retrieves the
  //                   configuration object via act ()
  //                   --> instances must be (re)initialize()d for every new
  //                       connection
  virtual bool initialize (const ConnectorConfigurationType&);
  // *NOTE*: this is just a stub
  virtual const ConnectorConfigurationType& get () const;

  //virtual bool useReactor () const; // ? : uses proactor

  virtual void abort ();
  virtual ACE_HANDLE connect (const ACE_Netlink_Addr&);

 protected:
  // override default creation strategy
  virtual HandlerType* make_handler (void);

 private:
  typedef ACE_Asynch_Connector<HandlerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T ())
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T (const Net_Client_AsynchConnector_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_Client_AsynchConnector_T& operator= (const Net_Client_AsynchConnector_T&))

  ConnectorConfigurationType configuration_;
};
#endif

// include template implementation
#include "net_client_asynchconnector.inl"

#endif
