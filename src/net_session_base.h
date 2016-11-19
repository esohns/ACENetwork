/***************************************************************************
*   Copyright (C) 2009 by Erik Sohns   *
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

#ifndef NET_SESSION_BASE_H
#define NET_SESSION_BASE_H

//#include <ace/Asynch_Connector.h>
//#include <ace/config-macros.h>
//#include <ace/Connector.h>
#include <ace/Global_Macros.h>
//#include <ace/SOCK_Connector.h>
#include <ace/Synch_Traits.h>

#include "net_common.h"
#include "net_defines.h"
//#include "net_iconnection.h"

// forward declarations
class ACE_Message_Block;

template <typename AddressType,
          typename ConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename SocketConfigurationType,
          typename HandlerConfigurationType, // socket-
          ////////////////////////////////
          typename ConnectionType, // derived from Net_IConnection_T
          typename ConnectionManagerType, // derived from Net_IConnectionManager_T
          typename ConnectorType,
          ////////////////////////////////
          typename StateType,
          typename SessionInterfaceType> // derived from Net_ISession_T
class Net_SessionBase_T
 : public SessionInterfaceType
// Net_ISession_T<AddressType,
//                                 ConfigurationType,
//                                 ConnectionStateType,
//                                 StatisticContainerType,
//                                 SocketConfigurationType,
//                                 HandlerConfigurationType,
//                                 typename ConnectionType::STREAM_T,
//                                 typename ConnectionType::STREAM_T::STATUS_T,
//                                 StateType>
{
 public:
  Net_SessionBase_T (const HandlerConfigurationType&, // socket handler configuration
                     ConnectionManagerType* = NULL,   // connection manager handle
                     bool = !NET_EVENT_USE_REACTOR);  // asynchronous ?
  virtual ~Net_SessionBase_T ();

  // implement (part of) Net_ISession_T
  inline virtual const StateType& state () const { return state_; };

  virtual void connect (const AddressType&); // peer address
  virtual void disconnect (const AddressType&); // peer address

  ////////////////////////////////////////
  // callbacks
  // *TODO*: remove ASAP
  virtual void connect (Net_ConnectionId_t); // connection id
  virtual void disconnect (Net_ConnectionId_t); // connection id

 protected:
  // convenient types
  typedef ConnectorType CONNECTOR_T;
//  typedef ACE_Connector<ConnectionType,
//                        ACE_SOCK_CONNECTOR> CONNECTOR_T;
//  typedef ACE_Asynch_Connector<ConnectionType> ASYNCH_CONNECTOR_T;
  typedef SessionInterfaceType ISESSION_T;

  HandlerConfigurationType* configuration_;
  ConnectionManagerType*    connectionManager_;
  bool                      isAsynch_;

  ACE_SYNCH_MUTEX           lock_;
  StateType                 state_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_SessionBase_T (const Net_SessionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_SessionBase_T& operator= (const Net_SessionBase_T&))

};

// include template definition
#include "net_session_base.inl"

#endif
