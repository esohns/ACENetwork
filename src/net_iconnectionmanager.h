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

#include "common_icontrol.h"

#include "net_iconnection.h"

template <typename ConfigurationType,
          typename UserDataType,
          typename StatisticContainerType,
          typename ITransportLayerType>
class Net_IConnectionManager_T
 : public Common_IControl
{
 public:
  // convenience types
  typedef Net_IConnection_T<ConfigurationType,
                            StatisticContainerType,
                            ITransportLayerType> CONNECTION_T;

  virtual ~Net_IConnectionManager_T () {};

  // API
  // *TODO*: (possibly) move these to Common_ILockedGet_T
  //virtual void lock () = 0;
  //virtual void unlock () = 0;
  virtual void set (const ConfigurationType&, // connection handler (default)
                                              // configuration
                    UserDataType*) = 0;       // (stream) user data
  virtual void get (ConfigurationType&,  // return value: (default)
                                         // connection handler configuration
                    UserDataType*&) = 0; // return value: (stream) user data

  // *WARNING*: if (!= NULL) callers must decrease() the returned handle
  virtual CONNECTION_T* operator[] (unsigned int) const = 0; // session id

  // *NOTE*: 'register' is a reserved keyword
  virtual bool registerc (CONNECTION_T*) = 0; // connection handle
  virtual void deregister (CONNECTION_T*) = 0; // connection handle

  virtual unsigned int numConnections () const = 0; // return value: (current) number of connections
};

#endif
