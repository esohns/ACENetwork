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

template <typename AddressType,
          typename ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
          ACE_SYNCH_DECL,
          typename TimePolicyType,
#endif // ACE_WIN32 || ACE_WIN64
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                   ACE_SYNCH_USE,
                   TimePolicyType,
#endif // ACE_WIN32 || ACE_WIN64
                   MonitorAPI_e,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited ()
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

}
