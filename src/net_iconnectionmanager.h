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

#ifndef NET_ICONNECTIONMANAGER_H
#define NET_ICONNECTIONMANAGER_H

#include "ace/Global_Macros.h"

#include "common_idumpstate.h"
#include "common_ilock.h"
#include "common_isubscribe.h"
#include "common_itaskcontrol.h"

#include "net_common.h"
#include "net_iconnection.h"

template <ACE_SYNCH_DECL>
class Net_IConnectionManagerBase_T
 : public Common_ITaskControl_T<ACE_SYNCH_USE,
                                Common_ILock_T<ACE_SYNCH_USE> >
 , public Common_IDumpState
{
 public:
  // convenient types
  typedef Common_ITaskControl_T<ACE_SYNCH_USE,
                                Common_ILock_T<ACE_SYNCH_USE> > ITASKCONTROL_T;

  virtual void abort (enum Net_Connection_AbortStrategy) = 0;
  virtual void abort (bool = false) = 0; // wait for completion ? (see wait())

  virtual unsigned int count () const = 0; // return value: # of connections

  // *NOTE*: the inherited wait() API really makes sense only AFTER stop() has
  //         been invoked, i.e. when new connections will be rejected; otherwise
  //         this may block indefinitely
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename AddressType,
          typename ConfigurationType, // connection-
          typename StateType, // connection-
          typename StatisticContainerType,
          ////////////////////////////////
          typename UserDataType>
class Net_IConnectionManager_T
 : public Net_IConnectionManagerBase_T<ACE_SYNCH_USE>
 , public Common_IRegister_T<Net_IConnection_T<AddressType,
                                               //ConfigurationType,
                                               StateType,
                                               StatisticContainerType> >
 , public Common_IStatistic_T<StatisticContainerType>
{
 public:
  // convenience types
  typedef Net_IConnectionManagerBase_T<ACE_SYNCH_USE> BASE_T;
  typedef ConfigurationType CONFIGURATION_T;
  typedef Net_IConnection_T<AddressType,
                            //ConfigurationType,
                            StateType,
                            StatisticContainerType> CONNECTION_T;

  virtual void set (const ConfigurationType&, // connection handler (default)
                                              // configuration
                    UserDataType*) = 0;       // (stream) user data
  virtual void get (ConfigurationType*&,  // return value: (default)
                                          // connection handler configuration
                    UserDataType*&) = 0;  // return value: (stream) user data

  // *WARNING*: if (!= NULL) callers must decrease() the returned handle
  virtual CONNECTION_T* operator[] (unsigned int) const = 0; // index
  virtual CONNECTION_T* get (Net_ConnectionId_t) const = 0; // id
  virtual CONNECTION_T* get (const AddressType&,     // address
                             bool = true) const = 0; // peer address ? : local address
  virtual CONNECTION_T* get (ACE_HANDLE) const = 0; // socket handle
};

#endif
