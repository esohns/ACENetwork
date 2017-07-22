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

#include "ace/OS.h"

#include "net_macros.h"

template <typename ConfigurationType>
Net_SocketHandlerBase_T<ConfigurationType>::Net_SocketHandlerBase_T ()
 : configuration_ (NULL)
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketHandlerBase_T::Net_SocketHandlerBase_T"));

}

template <typename ConfigurationType>
Net_SocketHandlerBase_T<ConfigurationType>::~Net_SocketHandlerBase_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketHandlerBase_T::~Net_SocketHandlerBase_T"));

}

template <typename ConfigurationType>
bool
Net_SocketHandlerBase_T<ConfigurationType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_SocketHandlerBase_T::initialize"));

  if (isInitialized_)
  {
    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  isInitialized_ = true;

  return true;
}
