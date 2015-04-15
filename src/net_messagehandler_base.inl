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

#include "ace/Log_Msg.h"

#include "net_macros.h"

template <typename ConfigurationType,
          typename MessageType>
Net_MessageHandlerBase_T<ConfigurationType,
                         MessageType>::Net_MessageHandlerBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageHandlerBase_T::Net_MessageHandlerBase_T"));

}

template <typename ConfigurationType,
          typename MessageType>
Net_MessageHandlerBase_T<ConfigurationType,
                         MessageType>::~Net_MessageHandlerBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageHandlerBase_T::~Net_MessageHandlerBase_T"));

}

template <typename ConfigurationType,
          typename MessageType>
void
Net_MessageHandlerBase_T<ConfigurationType,
                         MessageType>::start (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageHandlerBase_T::start"));

  ACE_UNUSED_ARG (configuration_in);
}

template <typename ConfigurationType,
          typename MessageType>
void
Net_MessageHandlerBase_T<ConfigurationType,
                         MessageType>::notify (const MessageType& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageHandlerBase_T::notify"));

  ACE_UNUSED_ARG (message_in);
}

template <typename ConfigurationType,
          typename MessageType>
void
Net_MessageHandlerBase_T<ConfigurationType,
                         MessageType>::end ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_MessageHandlerBase_T::end"));

}
