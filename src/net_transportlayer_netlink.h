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

#ifndef NET_TRANSPORTLAYER_NETLINK_H
#define NET_TRANSPORTLAYER_NETLINK_H

#include "ace/Global_Macros.h"

#include "net_transportlayer_base.h"

#if defined (ACE_HAS_NETLINK)
class Net_TransportLayer_Netlink
 : public Net_NetlinkTransportLayer_Base
{
 public:
  Net_TransportLayer_Netlink ();
  inline virtual ~Net_TransportLayer_Netlink () {};

 private:
  typedef Net_NetlinkTransportLayer_Base inherited;

  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_Netlink (const Net_TransportLayer_Netlink&))
  ACE_UNIMPLEMENTED_FUNC (Net_TransportLayer_Netlink& operator= (const Net_TransportLayer_Netlink&))
};
#endif

#endif
