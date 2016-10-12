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

#ifndef Net_TRANSPORTLAYER_IP_CAST_H
#define Net_TRANSPORTLAYER_IP_CAST_H

#include <ace/Global_Macros.h>
//#include <ace/INET_Addr.h>
//#include <ace/SOCK_Dgram_Bcast.h>
//#include <ace/SOCK_Dgram_Mcast.h>

//#include "net_common.h"
#include "net_exports.h"
#include "net_transportlayer_base.h"

// forward declarations
enum Common_DispatchType;

//class Net_Export Net_TransportLayer_IP_Broadcast
class Net_TransportLayer_IP_Broadcast
 : public Net_InetTransportLayer_Base
// , public ACE_SOCK_Dgram_Bcast
{
 public:
  Net_TransportLayer_IP_Broadcast ();
  virtual ~Net_TransportLayer_IP_Broadcast ();

//  // override some transport layer-based members
//  virtual bool initialize (const Net_SocketConfiguration_t&); // configuration

 private:
  typedef Net_InetTransportLayer_Base inherited;
//  typedef ACE_SOCK_Dgram_Bcast inherited2;

  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_IP_Broadcast (const Net_TransportLayer_IP_Broadcast&))
  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_IP_Broadcast& operator= (const Net_TransportLayer_IP_Broadcast&))
};

/////////////////////////////////////////

//class Net_Export Net_TransportLayer_IP_Multicast
class Net_TransportLayer_IP_Multicast
 : public Net_InetTransportLayer_Base
// , public ACE_SOCK_Dgram_Mcast
{
 public:
  Net_TransportLayer_IP_Multicast ();
  virtual ~Net_TransportLayer_IP_Multicast ();

//  // override some transport layer-based members
//  virtual bool initialize (const Net_SocketConfiguration_t&); // configuration

 private:
  typedef Net_InetTransportLayer_Base inherited;
//  typedef ACE_SOCK_Dgram_Mcast inherited2;

  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_IP_Multicast (const Net_TransportLayer_IP_Multicast&))
  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_IP_Multicast& operator= (const Net_TransportLayer_IP_Multicast&))

//  bool joined_;
};

#endif
