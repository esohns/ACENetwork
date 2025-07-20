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

#include "net_wlan_monitor.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
//#if defined (DBUS_NM_SUPPORT)
//#include "NetworkManager.h"
//#endif // DBUS_NM_SUPPORT

//#include "net_configuration.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_LINUX)
#if defined (DHCLIENT_SUPPORT)
void
net_wlan_dhclient_connection_event_cb (dhcpctl_handle handle_in,
                                       dhcpctl_status status_in,
                                       void* userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::net_wlan_dhclient_connection_event_cb"));

  // sanity check(s)
  ACE_ASSERT (handle_in != dhcpctl_null_handle);
  if (unlikely (status_in != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dhclient result: \"%s\" (handle was: %@), returning\n"),
                ACE_TEXT (isc_result_totext (status_in)),
                handle_in));
    return;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("dhclient result: \"%s\" (handle was: %@)\n"),
              ACE_TEXT (isc_result_totext (status_in)),
              handle_in));
#endif // _DEBUG
  ACE_ASSERT (userData_in);

  struct Net_WLAN_dhclient_CBData* cb_data_p =
      static_cast<struct Net_WLAN_dhclient_CBData*> (userData_in);
  ACE_UNUSED_ARG (cb_data_p);
}

void
net_wlan_dhclient_connect_cb (dhcpctl_handle handle_in,
                              dhcpctl_status status_in,
                              void* userData_in)
{
  NETWORK_TRACE (ACE_TEXT ("::net_wlan_dhclient_connect_cb"));

  // sanity check(s)
  ACE_ASSERT (handle_in != dhcpctl_null_handle);
  if (unlikely (status_in != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dhclient result: \"%s\" (handle was: %@), returning\n"),
                ACE_TEXT (isc_result_totext (status_in)),
                handle_in));
    return;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("dhclient result: \"%s\" (handle was: %@)\n"),
              ACE_TEXT (isc_result_totext (status_in)),
              handle_in));
#endif // _DEBUG
  ACE_ASSERT (userData_in);
  struct Net_WLAN_dhclient_CBData* cb_data_p =
      static_cast<struct Net_WLAN_dhclient_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p->connection != dhcpctl_null_handle);
  ACE_ASSERT (cb_data_p->monitor);

  dhcpctl_data_string data_string_p = NULL;
  dhcpctl_status status_i = ISC_R_SUCCESS;
  std::string interface_identifier_string;
  bool reconnect_b = false;
  std::string leases_file_path =
      ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_LEASES_FILE);
  Net_WLAN_Monitor_IStateMachine_t* istate_machine_p = NULL;

  status_i =
      dhcpctl_get_value (&data_string_p,
                         handle_in,
                         ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_OBJECT_VALUE_NAME_STRING));
  if (unlikely (status_i != ISC_R_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_get_value(%@,\"%s\"): \"%s\", returning\n"),
                handle_in,
                ACE_TEXT (DHCP_DHCLIENT_OBJECT_VALUE_NAME_STRING),
                ACE_TEXT (isc_result_totext (status_i))));
    return;
  } // end IF
  ACE_ASSERT (data_string_p);
  interface_identifier_string.assign (reinterpret_cast<char*> (data_string_p->value),
                                      data_string_p->len);
  status_i = dhcpctl_data_string_dereference (&data_string_p, MDL);
  ACE_ASSERT (status_i == ISC_R_SUCCESS);
  data_string_p = NULL;

  if (!DHCP_Tools::hasState (handle_in,
                             ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_INTERFACE_STATE_UP_STRING)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": invalid state, retrying\n"),
                ACE_TEXT (interface_identifier_string.c_str ())));
    reconnect_b = true;
    goto continue_;
  } // end IF

  // interface is 'up'; verify that it has a valid active lease
//      if (unlikely (!Net_Common_Tools::hasActiveLease (cb_data_p->connection,
  if (unlikely (!DHCP_Tools::hasActiveLease (leases_file_path,
                                             interface_identifier_string)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to DHCP_Tools::hasActiveLease(%s), retrying\n"),
                ACE_TEXT (interface_identifier_string.c_str ()),
                ACE_TEXT (leases_file_path.c_str ())));
    reconnect_b = true;
    goto continue_;
  } // end IF

continue_:
  DHCP_Tools::disconnectDHClient (cb_data_p->connection);
//  cb_data_p->connection = NULL;

  istate_machine_p =
      dynamic_cast<Net_WLAN_Monitor_IStateMachine_t*> (cb_data_p->monitor);
  ACE_ASSERT (istate_machine_p);
  try {
    istate_machine_p->change ((reconnect_b ? NET_WLAN_MONITOR_STATE_CONNECT
                                           : NET_WLAN_MONITOR_STATE_CONNECTED));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStateMachine_T::change(NET_WLAN_MONITOR_STATE_ASSOCIATED), returning\n")));
    return;
  }
}
#endif // DHCLIENT_SUPPORT
#endif // ACE_LINUX
