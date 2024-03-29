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

#include "ace/Condition_Thread_Mutex.h"
#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Thread_Mutex.h"

#include "net_common.h"
#include "net_defines.h"

// forward declarations
class ACE_Message_Block;

template <typename AddressType,
          typename ConnectionConfigurationType,
          typename ConnectionStateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename ConnectionType, // implements Net_IConnection_T
          typename ConnectionManagerType, // implements Net_IConnectionManager_T
          typename ConnectorType,
          ////////////////////////////////
          typename ConfigurationType,
          typename StateType,
          typename SessionInterfaceType> // implements Net_ISession_T
class Net_SessionBase_T
 : public SessionInterfaceType
{
 public:
  // convenient types
  typedef ConnectionType ICONNECTION_T;
//  typedef typename ConnectorType::ISTREAM_CONNECTION_T ISTREAM_CONNECTION_T;
  typedef SessionInterfaceType ISESSION_T;

  Net_SessionBase_T ();
  virtual ~Net_SessionBase_T ();

  // implement (part of) Net_ISession_T
  virtual bool initialize (const ConfigurationType&);
  inline virtual const StateType& state () const { return state_; };
  virtual void connect (const AddressType&); // peer address
  virtual void disconnect (const AddressType&); // peer address
  virtual void close (bool = false); // wait ?
  virtual void wait ();

  ////////////////////////////////////////
  // callbacks
  // *TODO*: these should be private
  virtual void connect (Net_ConnectionId_t); // connection id
  virtual void disconnect (Net_ConnectionId_t); // connection id

 protected:
  // convenient types
  typedef ConnectorType CONNECTOR_T;
  typedef ACE_Singleton<ConnectionManagerType,
                        ACE_SYNCH_MUTEX> CONNECTION_MANAGER_SINGLETON_T;

  ConfigurationType*         configuration_;
  bool                       isAsynch_;

  mutable ACE_Thread_Mutex   lock_;
  ACE_Condition_Thread_Mutex condition_;
  StateType                  state_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_SessionBase_T (const Net_SessionBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_SessionBase_T& operator= (const Net_SessionBase_T&))
};

// include template definition
#include "net_session_base.inl"

#endif
