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

template <typename AllocatorConfigurationType,
          typename StreamConfigurationType,
          enum Net_TransportLayerType TransportLayerType>
Net_ConnectionConfiguration_T<AllocatorConfigurationType,
                              StreamConfigurationType,
                              TransportLayerType>::Net_ConnectionConfiguration_T ()
 : inherited ()
 , allocatorConfiguration_ ()
 , streamConfiguration_ (NULL)
 , isInitialized_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionConfiguration_T::Net_ConnectionConfiguration_T"));

}

template <typename AllocatorConfigurationType,
          typename StreamConfigurationType,
          enum Net_TransportLayerType TransportLayerType>
bool
Net_ConnectionConfiguration_T<AllocatorConfigurationType,
                              StreamConfigurationType,
                              TransportLayerType>::initialize (const AllocatorConfigurationType& allocatorConfiguration_in,
                                                               const StreamConfigurationType& streamConfiguration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_ConnectionConfiguration_T::initialize"));

  if (isInitialized_)
  {
    isInitialized_ = false;
  } // end IF

  allocatorConfiguration_ = allocatorConfiguration_in;
  streamConfiguration_ =
    &const_cast<StreamConfigurationType&> (streamConfiguration_in);
  isInitialized_ = true;

  return true;
}
