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

#ifndef NET_ITRANSPORTLAYER_H
#define NET_ITRANSPORTLAYER_H

#include "ace/config-macros.h"
#include "ace/INET_Addr.h"
#include "ace/Netlink_Addr.h"

#include "common_idumpstate.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_exports.h"

template <typename AddressType,
          typename ConfigurationType>
class Net_ITransportLayer_T
 : public Common_IDumpState
{
 public:
  virtual ~Net_ITransportLayer_T () {};

  virtual bool initialize (Net_ClientServerRole_t,        // role
                           const ConfigurationType&) = 0; // configuration
  virtual void finalize () = 0;

  virtual void ping () = 0; // ping the peer !

  virtual void info (ACE_HANDLE&,             // return value: I/O handle
                     AddressType&,            // return value: local SAP
                     AddressType&) const = 0; // return value: remote SAP
  virtual unsigned int id () const = 0;
};

typedef Net_ITransportLayer_T<ACE_INET_Addr,
                              Net_SocketConfiguration_t> Net_IInetTransportLayer_t;

#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
typedef Net_ITransportLayer_T<ACE_Netlink_Addr,
                              Net_SocketConfiguration_t> Net_INetlinkTransportLayer_t;
#endif
#endif
