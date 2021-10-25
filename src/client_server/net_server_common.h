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

#ifndef NET_SERVER_COMMON_H
#define NET_SERVER_COMMON_H

#include "ace/Basic_Types.h"

#include "net_common.h"
#include "net_connection_configuration.h"

//template <typename ConnectionConfigurationType,
//          enum Net_TransportLayerType TransportLayerType_e>
//class Net_ListenerConfiguration_T
// : public Net_ConnectionConfigurationBase_T<TransportLayerType_e>
//{
// public:
//  Net_ListenerConfiguration_T ()
////   : Net_ConnectionConfigurationBase_T ()
//   : addressFamily (ACE_ADDRESS_FAMILY_INET)
//   , connectionConfiguration (NULL)
//  {}

//  int                          addressFamily;
//  ConnectionConfigurationType* connectionConfiguration;
//};

//#if defined (NETLINK_SUPPORT)
//typedef Net_ListenerConfiguration_T<NET_TRANSPORTLAYER_NETLINK> Net_NetlinkListenerConfiguration_t;
//#endif // NETLINK_SUPPORT
//typedef Net_ListenerConfiguration_T<NET_TRANSPORTLAYER_TCP> Net_TCPListenerConfiguration_t;
//typedef Net_ListenerConfiguration_T<NET_TRANSPORTLAYER_UDP> Net_UDPListenerConfiguration_t;

#endif
