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

#include "ace/Log_Msg.h"

#include "net_macros.h"

template <typename ConfigurationType>
Net_InetTransportLayerBase_T<ConfigurationType>::Net_InetTransportLayerBase_T (enum Net_TransportLayerType transportLayer_in)
 : dispatch_ (COMMON_EVENT_DISPATCH_INVALID)
 , role_ (NET_ROLE_INVALID)
 , transportLayer_ (transportLayer_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayerBase_T::Net_InetTransportLayerBase_T"));

}

template <typename ConfigurationType>
bool
Net_InetTransportLayerBase_T<ConfigurationType>::initialize (enum Common_EventDispatchType dispatch_in,
                                                             enum Net_ClientServerRole role_in,
                                                             const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_InetTransportLayerBase_T::initialize"));

  ACE_UNUSED_ARG (configuration_in);

  dispatch_ = dispatch_in;
  role_ = role_in;

  return true;
}
