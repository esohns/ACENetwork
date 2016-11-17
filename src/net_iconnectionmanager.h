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

#include "common_idumpstate.h"
#include "common_ilock.h"
#include "common_itask.h"

#include "net_iconnection.h"

class Net_IConnectionManagerBase
 : public Common_ITaskControl_T<ACE_MT_SYNCH>
 , public Common_ILock
 , public Common_IDumpState
{
 public:
  inline virtual ~Net_IConnectionManagerBase () {};

  virtual void abort (bool = false) = 0; // wait for completion ? (see wait())
  virtual unsigned int count () const = 0; // return value: # of connections
  // *IMPORTANT NOTE*: this API really makes sense only AFTER stop() has been
  //                   invoked, i.e. when new connections will be rejected;
  //                   otherwise this may block indefinetly
  virtual void wait () const = 0;

  // debugging
  virtual void abortLeastRecent () = 0;
  virtual void abortMostRecent () = 0;
};

//////////////////////////////////////////

template <typename AddressType,
          typename ConfigurationType,
          typename StateType,
          typename StatisticContainerType,
          ////////////////////////////////
          typename UserDataType>
class Net_IConnectionManager_T
 : public Net_IConnectionManagerBase
{
 public:
  // convenience types
  typedef Net_IConnection_T<AddressType,
                            ConfigurationType,
                            StateType,
                            StatisticContainerType> CONNECTION_T;

  virtual ~Net_IConnectionManager_T () {};

  virtual void set (const ConfigurationType&, // connection handler (default)
                                              // configuration
                    UserDataType*) = 0;       // (stream) user data
  virtual void get (ConfigurationType*&,  // return value: (default)
                                          // connection handler configuration
                    UserDataType*&) = 0;  // return value: (stream) user data

  // *WARNING*: if (!= NULL) callers must decrease() the returned handle
  virtual CONNECTION_T* operator[] (unsigned int) const = 0; // index
  virtual CONNECTION_T* get (ACE_HANDLE) const = 0; // socket handle
  virtual CONNECTION_T* get (const AddressType&,     // address
                             bool = true) const = 0; // peer address ? : local address

  // *NOTE*: 'register' is a reserved keyword
  virtual bool registerc (CONNECTION_T*) = 0; // connection handle
  virtual bool deregister (CONNECTION_T*) = 0; // connection handle
};

#endif
