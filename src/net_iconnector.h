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

#ifndef NET_ICONNECTOR_H
#define NET_ICONNECTOR_H

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"

#include "common_iget.h"
#include "common_iinitialize.h"

#include "net_common.h"

template <typename AddressType>
class Net_IConnectorBase_T
{
 public:
  inline virtual ~Net_IConnectorBase_T () {}

  virtual enum Net_TransportLayerType transportLayer () const = 0;
  virtual bool useReactor () const = 0; // ? : uses proactor (i.e. operation is asynchronous)

  virtual ACE_HANDLE connect (const AddressType&) = 0; // return value: (asynchronous ? connect- : socket-) handle
};

//////////////////////////////////////////

template <typename AddressType,
          typename ConfigurationType>
class Net_IAsynchConnector_T;
template <typename AddressType,
          typename ConfigurationType> // connection-
class Net_IConnector_T
 : public Net_IConnectorBase_T<AddressType>
 , public Common_IGetR_T<ConfigurationType>
 , public Common_IInitialize_T<ConfigurationType>
{
 public:
  // convenient types
  typedef ConfigurationType CONFIGURATION_T;
  typedef Net_IAsynchConnector_T<AddressType,
                                 ConfigurationType> IASYNCH_CONNECTOR_T;
};

template <typename AddressType,
          typename ConfigurationType>
class Net_IAsynchConnector_T
 : public Net_IConnector_T<AddressType,
                           ConfigurationType>
{
 public:
  virtual void abort () = 0; // --> cancel
  virtual int wait (ACE_HANDLE,                                        // connect handle
                    const ACE_Time_Value& = ACE_Time_Value::zero) = 0; // block : (relative-) timeout

  ////////////////////////////////////////
  virtual void onConnect (ACE_HANDLE, // connect handle
                          int) = 0;   // success ? 0 : errno
};

//////////////////////////////////////////

typedef Net_IConnectorBase_T<ACE_INET_Addr> Net_Inet_IConnector_t;

#endif
