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
#include "stdafx.h"

#include "ace/Synch.h"
#include "net_wlan_monitor.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
//#if defined (DBUS_SUPPORT)
//#include "NetworkManager.h"
//#endif // DBUS_SUPPORT

//#include "net_configuration.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#if defined (DHCLIENT_SUPPORT)
void
net_wlan_dhclient_cb (dhcpctl_handle handle_in,
                      dhcpctl_status status_in,
                      void* userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::net_wlan_dhclient_cb"));

  // sanity check(s)
  ACE_ASSERT (handle_in != dhcpctl_null_handle);
  ACE_ASSERT (userData_in);

//  Net_WLAN_IMonitorCB* iwlanmonitor_cb_p =
//    static_cast<Net_WLAN_IMonitorCB*> (userData_in);

  if (unlikely (status_in != ISC_R_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dhclient result: \"%s\" (handle was: %@)\n"),
                ACE_TEXT (isc_result_totext (status_in)),
                handle_in));
  else
  {
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("dhclient result: \"%s\" (handle was: %@)\n"),
                ACE_TEXT (isc_result_totext (status_in)),
                handle_in));
#endif // _DEBUG
  } // end ELSE
}
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
