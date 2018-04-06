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

#include "net_wlan_tools.h"

#include <sys/capability.h>
#include <linux/capability.h>
//#include <linux/ieee80211.h>
#include <net/if_arp.h>
#include "ifaddrs.h"
//#if defined (WEXT_SUPPORT)
//#include "iwlib.h"
//#endif // WEXT_SUPPORT

#include "ace/Handle_Set.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch.h"

#include "common_tools.h"

#include "common_dbus_defines.h"
#include "common_dbus_tools.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "net_wlan_defines.h"

//////////////////////////////////////////

Net_InterfaceIdentifiers_t
Net_WLAN_Tools::getInterfaces (struct DBusConnection* connection_in,
                               int addressFamily_in,
                               int flags_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getInterfaces"));

  ACE_UNUSED_ARG (addressFamily_in);
  ACE_UNUSED_ARG (flags_in);

  // initialize return value(s)
  Net_InterfaceIdentifiers_t return_value;

  // sanity check(s)
  struct DBusConnection* connection_p = connection_in;
  bool release_connection_b = false;
  if (unlikely (!connection_p))
  {
    struct DBusError error_s;
    dbus_error_init (&error_s);
    connection_p = dbus_bus_get_private (DBUS_BUS_SYSTEM,
                                         &error_s);
    if (unlikely (!connection_p ||
                  dbus_error_is_set (&error_s)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_bus_get_private(DBUS_BUS_SYSTEM): \"%s\", returning\n"),
                  ACE_TEXT (error_s.message)));

      dbus_error_free (&error_s);

      return return_value;
    } // end IF
    dbus_connection_set_exit_on_disconnect (connection_p,
                                            false);

    release_connection_b = true;
  } // end IF

  struct DBusMessageIter iterator, iterator_2;
  char* object_path_p = NULL;
  Common_DBus_ObjectPaths_t interface_object_paths_a;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_OBJECT_PATH),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("GetDevices"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(GetDevices): \"%m\", aborting\n")));
    return return_value;
  } // end IF
  struct DBusMessage* reply_p =
      Common_DBus_Tools::exchange (connection_p,
                                   message_p,
                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF

  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                  DBUS_TYPE_ARRAY)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  do {
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_OBJECT_PATH);
    object_path_p = NULL;
    dbus_message_iter_get_basic (&iterator_2, &object_path_p);
    ACE_ASSERT (object_path_p);
    interface_object_paths_a.push_back (object_path_p);
  } while (dbus_message_iter_next (&iterator_2));
  dbus_message_unref (reply_p); reply_p = NULL;

  for (Common_DBus_ObjectPathsIterator_t iterator_3 = interface_object_paths_a.begin ();
       iterator_3 != interface_object_paths_a.end ();
       ++iterator_3)
    return_value.push_back (Net_WLAN_Tools::deviceDBusObjectPathToIdentifier (connection_p,
                                                                              *iterator_3));

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);
  if (release_connection_b)
  {
    dbus_connection_close (connection_p);
    dbus_connection_unref (connection_p);
  } // end IF

  return return_value;
}

std::string
Net_WLAN_Tools::associatedSSID (struct DBusConnection* connection_in,
                                const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  std::string interface_identifier = interfaceIdentifier_in;
  if (unlikely (interfaceIdentifier_in.empty ()))
  {
    Net_InterfaceIdentifiers_t interface_identifiers_a =
        Net_WLAN_Tools::getInterfaces (connection_in,
                                       AF_UNSPEC,
                                       0);
    if (interface_identifiers_a.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument, aborting\n")));
      return result;
    } // end IF
    interface_identifier = interface_identifiers_a.front ();
  } // end IF

  std::string device_path_string =
      Net_WLAN_Tools::deviceToDBusObjectPath (connection_in,
                                        interfaceIdentifier_in);
  ACE_ASSERT (!device_path_string.empty ());
  std::string access_point_path_string =
      Net_WLAN_Tools::deviceDBusObjectPathToActiveAccessPointDBusObjectPath (connection_in,
                                                                 device_path_string);
  if (access_point_path_string.empty ())
    return result;

  result =
      Net_WLAN_Tools::accessPointDBusObjectPathToSSID (connection_in,
                                                 access_point_path_string);

  return result;
}

bool
Net_WLAN_Tools::activateConnection (struct DBusConnection* connection_in,
                                    const std::string& connectionObjectPath_in,
                                    const std::string& deviceObjectPath_in,
                                    const std::string& accessPointObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::activateConnection"));

  // initialize return value(s)
  bool result = false;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!connectionObjectPath_in.empty ());
  ACE_ASSERT (!deviceObjectPath_in.empty ());
  ACE_ASSERT (!accessPointObjectPath_in.empty ());

  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_OBJECT_PATH),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("ActivateConnection"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(ActivateConnection): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  struct DBusMessageIter iterator;
  char* object_path_p = NULL;
  const char* argument_string_p = connectionObjectPath_in.c_str ();
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_OBJECT_PATH,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = deviceObjectPath_in.c_str ();
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_OBJECT_PATH,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = accessPointObjectPath_in.c_str ();
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_OBJECT_PATH,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF

  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator, &object_path_p);
  ACE_ASSERT (object_path_p);
  dbus_message_unref (reply_p); reply_p = NULL;
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s: activated connection profile \"%s\" (active connection is: \"%s\")\n"),
//              ACE_TEXT (Net_WLAN_Tools::deviceDBusObjectPathToIdentifier (connection_in, deviceObjectPath_in).c_str ()),
//              ACE_TEXT (connectionObjectPath_in.c_str ()),
//              ACE_TEXT (object_path_p)));

  result = true;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

  return false;

continue_:
  return result;
}

ACE_INET_Addr
Net_WLAN_Tools::getGateway (const std::string& interfaceIdentifier_in,
                            struct DBusConnection* connection_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getGateway"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (connection_in);

  ACE_INET_Addr result = Net_Common_Tools::getGateway (interfaceIdentifier_in);
  int result_2 = -1;

  // *IMPORTANT NOTE*: (as of kernel 3.16.0,x) dhclient apparently does not add
  //                   wireless gateway information to the routing table
  //                   reliably (i.e. Gateway is '*'). Specifically, when there
  //                   already is a gateway configured on a different interface
  //                   --> try DBus instead
  if (result.is_any ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to retrieve gateway from kernel, trying DBus...\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));

    std::string result_string =
        Net_WLAN_Tools::deviceToDBusObjectPath (connection_in,
                                          interfaceIdentifier_in);
    if (unlikely (result_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceToDBusObjectPath(\"%s\"), aborting\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
      return result;
    } // end IF
    result_string =
        Net_WLAN_Tools::deviceDBusObjectPathToIp4ConfigDBusObjectPath (connection_in,
                                                             result_string);
    if (unlikely (result_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceDBusObjectPathToIp4ConfigDBusObjectPath(\"%s\",\"%s\"), aborting\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ()),
                  ACE_TEXT (result_string.c_str ())));
      return result;
    } // end IF
    result_string =
        Net_WLAN_Tools::Ip4ConfigDBusObjectPathToGateway (connection_in,
                                                      result_string);
    if (unlikely (result_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::Ip4ConfigDBusObjectPathToGateway(\"%s\",\"%s\"), aborting\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ()),
                  ACE_TEXT (result_string.c_str ())));
      return result;
    } // end IF
    result_2 = result.set (0,
                           result_string.c_str (),
                           1,
                           AF_INET);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (result_string.c_str ())));
      return result;
    } // end IF
  } // end IF

  return result;
}

std::string
Net_WLAN_Tools::activeConnectionDBusObjectPathToIp4ConfigDBusObjectPath (struct DBusConnection* connection_in,
                                                               const std::string& activeConnectionObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::activeConnectionDBusObjectPathToIp4ConfigDBusObjectPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!activeConnectionObjectPath_in.empty ());

  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    activeConnectionObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(IPv4Config): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  struct DBusMessageIter iterator;
  char* object_path_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_CONNECTIONACTIVE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Ip4Config");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF

  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator, &object_path_p);
  ACE_ASSERT (object_path_p);
  result = object_path_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::activeConnectionDBusObjectPathToDeviceDBusObjectPath (struct DBusConnection* connection_in,
                                                                      const std::string& activeConnectionObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::activeConnectionDBusObjectPathToDeviceDBusObjectPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!activeConnectionObjectPath_in.empty ());

  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    activeConnectionObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Devices): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  struct DBusMessageIter iterator, iterator_2;
  char* object_path_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_CONNECTIONACTIVE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Devices");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_ARRAY);
  dbus_message_iter_recurse (&iterator, &iterator_2);
  do {
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_OBJECT_PATH);
    dbus_message_iter_get_basic (&iterator_2, &object_path_p);
    ACE_ASSERT (object_path_p);
    if (unlikely (!result.empty ()))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("active connection \"%s\" uses several devices, returning the first one\n"),
                  ACE_TEXT (activeConnectionObjectPath_in.c_str ())));
      break;
    } // end IF
    result = object_path_p;
  } while (dbus_message_iter_next (&iterator));
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::deviceToDBusObjectPath (struct DBusConnection* connection_in,
                                        const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::deviceToDBusObjectPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_OBJECT_PATH),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("GetDeviceByIpIface"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(GetDeviceByIpIface): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator;
  char* object_path_p = NULL;
  dbus_message_iter_init_append (message_p, &iterator);
  const char* device_identifier_p = interfaceIdentifier_in.c_str ();
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &device_identifier_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                  DBUS_TYPE_OBJECT_PATH)))
    goto error;
  dbus_message_iter_get_basic (&iterator, &object_path_p);
  ACE_ASSERT (object_path_p);
  result = object_path_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::Ip4ConfigDBusObjectPathToGateway (struct DBusConnection* connection_in,
                                                  const std::string& Ip4ConfigObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::Ip4ConfigDBusObjectPathToGateway"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!Ip4ConfigObjectPath_in.empty ());

  struct DBusMessage* message_p =
  dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                Ip4ConfigObjectPath_in.c_str (),
                                ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Gateway): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  struct DBusMessageIter iterator, iterator_2;
  char* string_p = NULL;
//  char character_c = 0;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_IP4CONFIG_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Gateway");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF

//  character_c = dbus_message_iter_get_arg_type (&iterator);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_VARIANT);
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_STRING);
  dbus_message_iter_get_basic (&iterator_2, &string_p);
  ACE_ASSERT (string_p);
  result = string_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::deviceDBusObjectPathToActiveAccessPointDBusObjectPath (struct DBusConnection* connection_in,
                                                                       const std::string& deviceObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::deviceDBusObjectPathToActiveAccessPointDBusObjectPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!deviceObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    deviceObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char* object_path_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_DEVICEWIRELESS_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("ActiveAccessPoint");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                  DBUS_TYPE_VARIANT)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator_2, &object_path_p);
  ACE_ASSERT (object_path_p);
  result = object_path_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  // *NOTE*: the D-Bus protocol defines "/" to be an invalid/unkown object path
  if (unlikely (!ACE_OS::strcmp (result.c_str (), ACE_TEXT_ALWAYS_CHAR ("/"))))
    result.resize (0);

  return result;
}

std::string
Net_WLAN_Tools::deviceDBusObjectPathToIp4ConfigDBusObjectPath (struct DBusConnection* connection_in,
                                                      const std::string& deviceObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::deviceDBusObjectPathToIp4ConfigDBusObjectPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!deviceObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    deviceObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char* object_path_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_DEVICE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Ip4Config");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  // *NOTE*: contrary to the documentation, the type is 'vo', not 'o'
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_VARIANT);
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator_2, &object_path_p);
  ACE_ASSERT (object_path_p);
  result = object_path_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  // *NOTE*: the D-Bus protocol defines "/" to be an invalid/unkown object path
  if (unlikely (!ACE_OS::strcmp (result.c_str (), ACE_TEXT_ALWAYS_CHAR ("/"))))
    result.resize (0);

  return result;
}

std::string
Net_WLAN_Tools::deviceDBusObjectPathToIdentifier (struct DBusConnection* connection_in,
                                                  const std::string& deviceObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::deviceDBusObjectPathToIdentifier"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!deviceObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    deviceObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char* device_identifier_p = NULL;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_DEVICE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Interface");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                        DBUS_TYPE_VARIANT)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_STRING);
  dbus_message_iter_get_basic (&iterator_2, &device_identifier_p);
  ACE_ASSERT (device_identifier_p);
  result = device_identifier_p;
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::accessPointDBusObjectPathToSSID (struct DBusConnection* connection_in,
                                                 const std::string& accessPointObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::accessPointDBusObjectPathToSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!accessPointObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    accessPointObjectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char character_c = 0;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_ACCESSPOINT_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Ssid");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                                   message_p,
                                                   -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                        DBUS_TYPE_VARIANT)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_ARRAY);
  dbus_message_iter_recurse (&iterator_2, &iterator);
  do {
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_BYTE);
    dbus_message_iter_get_basic (&iterator, &character_c);
    ACE_ASSERT (character_c);
    result += character_c;
  } while (dbus_message_iter_next (&iterator));
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::connectionDBusObjectPathToSSID (struct DBusConnection* connection_in,
                                                const std::string& connectionObjectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::connectionDBusObjectPathToSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!connectionObjectPath_in.empty ());

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
  dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                connectionObjectPath_in.c_str (),
                                ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SETTINGSCONNECTION_INTERFACE),
                                ACE_TEXT_ALWAYS_CHAR ("GetSettings"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(GetSettings): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2, iterator_3, iterator_4;
  struct DBusMessageIter iterator_5, iterator_6, iterator_7;
  const char* key_string_p, *key_string_2 = NULL;
  DBusBasicValue value_u;
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
  goto error;
  } // end IF
  // *NOTE*: the schema is a{sa{sv}}
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                  DBUS_TYPE_ARRAY)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  do {
    // connection settings --> wireless settings --> IPv4 configuration --> ...
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_DICT_ENTRY);
    dbus_message_iter_recurse (&iterator_2, &iterator_3);
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_3) == DBUS_TYPE_STRING);
    key_string_p = NULL;
    dbus_message_iter_get_basic (&iterator_3, &key_string_p);
    ACE_ASSERT (key_string_p);
    if (ACE_OS::strcmp (key_string_p, ACE_TEXT_ALWAYS_CHAR ("802-11-wireless")))
      continue;
    dbus_message_iter_next (&iterator_3);
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_3) == DBUS_TYPE_ARRAY);
    dbus_message_iter_recurse (&iterator_3, &iterator_4);
    do
    {
      ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_4) == DBUS_TYPE_DICT_ENTRY);
      dbus_message_iter_recurse (&iterator_4, &iterator_5);
      ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_5) == DBUS_TYPE_STRING);
      key_string_2 = NULL;
      dbus_message_iter_get_basic (&iterator_5, &key_string_2);
      ACE_ASSERT (key_string_2);
      dbus_message_iter_next (&iterator_5);
      ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_5) == DBUS_TYPE_VARIANT);
      dbus_message_iter_recurse (&iterator_5, &iterator_6);
      switch (dbus_message_iter_get_arg_type (&iterator_6))
      {
        case DBUS_TYPE_BOOLEAN:
        case DBUS_TYPE_BYTE:
        case DBUS_TYPE_INT16:
        case DBUS_TYPE_UINT16:
        case DBUS_TYPE_INT32:
        case DBUS_TYPE_UINT32:
        case DBUS_TYPE_INT64:
        case DBUS_TYPE_UINT64:
        case DBUS_TYPE_DOUBLE:
        case DBUS_TYPE_STRING:
        case DBUS_TYPE_OBJECT_PATH:
        case DBUS_TYPE_SIGNATURE:
        case DBUS_TYPE_UNIX_FD:
        {
          dbus_message_iter_get_basic (&iterator_6, &value_u);
          break;
        }
        case DBUS_TYPE_ARRAY:
        {
          dbus_message_iter_recurse (&iterator_6, &iterator_7);
          if (!ACE_OS::strcmp (key_string_2, ACE_TEXT_ALWAYS_CHAR ("ssid")))
          { ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_7) == DBUS_TYPE_BYTE);
            char character_c = 0;
            do
            {
              dbus_message_iter_get_basic (&iterator_7, &character_c);
              result += character_c;
            } while (dbus_message_iter_next (&iterator_7));

            goto done;
          } // end IF

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown argument type (was: %d), continuing\n"),
                      dbus_message_iter_get_arg_type (&iterator_6)));
          break;
        }
      } // end SWITCH
    } while (dbus_message_iter_next (&iterator_4));
  } while (dbus_message_iter_next (&iterator_2));
done:
  dbus_message_unref (reply_p); reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::SSIDToAccessPointDBusObjectPath (struct DBusConnection* connection_in,
                                                 const std::string& deviceObjectPath_in,
                                                 const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::SSIDToAccessPointDBusObjectPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!SSID_in.empty ());

  std::string device_object_path_string =
      (deviceObjectPath_in.empty () ? Net_WLAN_Tools::SSIDToDeviceDBusObjectPath (connection_in,
                                                                              SSID_in)
                                    : deviceObjectPath_in);
  if (unlikely (device_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToDeviceDBusObjectPath(0x%@,%s), aborting\n"),
                connection_in,
                ACE_TEXT (SSID_in.c_str ())));
    return result;
  } // end IF

  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    device_object_path_string.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_DEVICEWIRELESS_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("GetAllAccessPoints"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(GetAllAccessPoints): \"%m\", aborting\n")));
    return result;
  } // end IF
  struct DBusMessageIter iterator, iterator_2;
  char* object_path_p = NULL;
  std::string SSID_string;
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                        DBUS_TYPE_ARRAY)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  do {
    if (unlikely (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_INVALID))
      break; // device exists, but no SSIDs found --> radio off ?
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_OBJECT_PATH);
    dbus_message_iter_get_basic (&iterator_2, &object_path_p);
    ACE_ASSERT (object_path_p);
    SSID_string = Net_WLAN_Tools::accessPointDBusObjectPathToSSID (connection_in,
                                                               object_path_p);
    if (unlikely (SSID_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::accessPointDBusObjectPathToSSID(\"%s\"), continuing\n"),
                  ACE_TEXT (object_path_p)));
      continue;
    } // end IF
    if (SSID_string == SSID_in)
    {
      result = object_path_p;
      break;
    } // end IF
  } while (dbus_message_iter_next (&iterator_2));
  dbus_message_unref (reply_p);
  reply_p = NULL;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

std::string
Net_WLAN_Tools::SSIDToDeviceDBusObjectPath (struct DBusConnection* connection_in,
                                            const std::string& SSID_in)
{
  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!SSID_in.empty ());

  // step1: retrieve all wireless adapaters
  std::vector<std::string> wireless_device_identifiers_a =
      Net_WLAN_Tools::getInterfaces ();

  // step2: retrieve the wireless adapter(s) that can see the SSID
  for (std::vector<std::string>::iterator iterator = wireless_device_identifiers_a.begin ();
       iterator != wireless_device_identifiers_a.end ();
       ++iterator)
    if (!Net_WLAN_Tools::hasSSID (*iterator,
                                  SSID_in))
      wireless_device_identifiers_a.erase (iterator);
  if (wireless_device_identifiers_a.empty ())
    return result;

  if (unlikely (wireless_device_identifiers_a.size () > 1))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("found several devices that can see SSID %s, choosing the first one\n"),
                ACE_TEXT (SSID_in.c_str ())));

  return Net_WLAN_Tools::deviceToDBusObjectPath (connection_in,
                                           wireless_device_identifiers_a.front ());
}

std::string
Net_WLAN_Tools::SSIDToConnectionDBusObjectPath (struct DBusConnection* connection_in,
                                                const std::string& deviceObjectPath_in,
                                                const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::SSIDToConnectionDBusObjectPath"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!SSID_in.empty ());

  std::string device_object_path_string =
      (deviceObjectPath_in.empty () ? Net_WLAN_Tools::SSIDToDeviceDBusObjectPath (connection_in,
                                                                                  SSID_in)
                                    : deviceObjectPath_in);
  if (unlikely (device_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToDeviceDBusObjectPath(0x%@,%s), aborting\n"),
                connection_in,
                ACE_TEXT (SSID_in.c_str ())));
    return result;
  } // end IF

  struct DBusMessage* reply_p = NULL;
  // *NOTE*: a better alternative would be to retrieve the
  //        'AvailableConnections' property of the device; this seems to be
  //        broken at the moment *TODO*
  //        --> retrieve all connections
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
//                                    device_object_path_string.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SETTINGS_OBJECT_PATH),
//                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SETTINGS_INTERFACE),
//                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
                                    ACE_TEXT_ALWAYS_CHAR ("ListConnections"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    return result;
  } // end IF
//  struct DBusMessageIter iterator;
  struct DBusMessageIter iterator_2, iterator_3;
  std::vector<std::string> connection_paths_a;
  std::vector<std::string>::const_iterator iterator_4;
  char* object_path_p = NULL;
  std::string SSID_string;
  //  const char* argument_string_p =
//      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_DEVICE_INTERFACE);
//  dbus_message_iter_init_append (message_p, &iterator);
//  if (!dbus_message_iter_append_basic (&iterator,
//                                       DBUS_TYPE_STRING,
//                                       &argument_string_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
//    goto error;
//  } // end IF
//  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("AvailableConnections");
//  dbus_message_iter_init_append (message_p, &iterator);
//  if (!dbus_message_iter_append_basic (&iterator,
//                                       DBUS_TYPE_STRING,
//                                       &argument_string_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
//    goto error;
//  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
//  if (!dbus_message_iter_init (reply_p, &iterator))
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
//  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_VARIANT);
//  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_ARRAY);
  dbus_message_iter_recurse (&iterator_2, &iterator_3);
  do {
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_3) == DBUS_TYPE_OBJECT_PATH);
    dbus_message_iter_get_basic (&iterator_3, &object_path_p);
    ACE_ASSERT (object_path_p);
    connection_paths_a.push_back (object_path_p);
  } while (dbus_message_iter_next (&iterator_3));
  dbus_message_unref (reply_p); reply_p = NULL;

  iterator_4 = connection_paths_a.begin ();
  for (;
       iterator_4 != connection_paths_a.end ();
       ++iterator_4)
  {
    SSID_string = Net_WLAN_Tools::connectionDBusObjectPathToSSID (connection_in,
                                                                  *iterator_4);
    if (SSID_string == SSID_in)
      break;
  } // end FOR
  if (unlikely (iterator_4 == connection_paths_a.end ()))
    goto error;

  result = *iterator_4;

  goto continue_;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

continue_:
  return result;
}

bool
Net_WLAN_Tools::isWPASupplicantManagingInterface (struct DBusConnection* connection_in,
                                                  const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::isWPASupplicantManagingInterface"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

  // initialize return value(s)
  Net_InterfaceIdentifiers_t interface_identifiers_a;

  // sanity check(s)
  struct DBusConnection* connection_p = connection_in;
  bool release_connection_b = false;
  if (unlikely (!connection_p))
  {
    struct DBusError error_s;
    dbus_error_init (&error_s);
    connection_p = dbus_bus_get_private (DBUS_BUS_SYSTEM,
                                         &error_s);
    if (unlikely (!connection_p ||
                  dbus_error_is_set (&error_s)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_bus_get_private(DBUS_BUS_SYSTEM): \"%s\", aborting\n"),
                  ACE_TEXT (error_s.message)));

      dbus_error_free (&error_s);

      return false; // *TODO*: avoid false negatives
    } // end IF
    dbus_connection_set_exit_on_disconnect (connection_p,
                                            false);

    release_connection_b = true;
  } // end IF

  Common_DBus_ObjectPaths_t interface_object_paths_a;
  struct DBusMessage* reply_p = NULL;
  struct DBusMessageIter iterator, iterator_2, iterator_3;
  char* string_p = NULL;
  char character_c = 0;
  const char* argument_string_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_SERVICE),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_OBJECT_PATH),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Interfaces");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_p,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF
//#if defined (_DEBUG)
//  Common_DBus_Tools::dump (*reply_p);
//#endif // _DEBUG

  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                  DBUS_TYPE_VARIANT)))
    goto error;
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_ARRAY);
  if (!dbus_message_iter_get_element_count (&iterator_2))
    goto continue_;
  dbus_message_iter_recurse (&iterator_2, &iterator_3);
  do
  {
    // *NOTE*: apparently, empty arrays have an 'INVALID' object type entry
    ACE_ASSERT ((dbus_message_iter_get_arg_type (&iterator_3) == DBUS_TYPE_INVALID) || (dbus_message_iter_get_arg_type (&iterator_3) == DBUS_TYPE_OBJECT_PATH));
    string_p = NULL;
    dbus_message_iter_get_basic (&iterator_3, &string_p);
    ACE_ASSERT (string_p);
    interface_object_paths_a.push_back (string_p);
  } while (dbus_message_iter_next (&iterator_3));
continue_:
  dbus_message_unref (reply_p); reply_p = NULL;

  for (Common_DBus_ObjectPathsIterator_t iterator_3 = interface_object_paths_a.begin ();
       iterator_3 != interface_object_paths_a.end ();
       ++iterator_3)
    interface_identifiers_a.push_back (Net_WLAN_Tools::dBusObjectPathToIdentifier (connection_p,
                                                                                   *iterator_3));

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);
  if (release_connection_b)
  {
    dbus_connection_close (connection_p);
    dbus_connection_unref (connection_p);
  } // end IF

  for (Net_InterfacesIdentifiersIterator_t iterator_3 = interface_identifiers_a.begin ();
       iterator_3 != interface_identifiers_a.end ();
       ++iterator_3)
    if (!ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
                         (*iterator_3).c_str ()))
      return true;

  return false;
}

bool
Net_WLAN_Tools::WPASupplicantManageInterface (struct DBusConnection* connection_in,
                                              const std::string& interfaceIdentifier_in,
                                              bool toggle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::WPASupplicantManageInterface"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  bool is_managing_interface_b =
      Net_WLAN_Tools::isWPASupplicantManagingInterface (connection_in,
                                                        interfaceIdentifier_in);
  if ((toggle_in && is_managing_interface_b)  ||
      (!toggle_in && !is_managing_interface_b))
    return true; // nothing to do

  struct DBusConnection* connection_p = connection_in;
  bool release_connection_b = false;
  if (unlikely (!connection_p))
  {
    struct DBusError error_s;
    dbus_error_init (&error_s);
    connection_p = dbus_bus_get_private (DBUS_BUS_SYSTEM,
                                         &error_s);
    if (unlikely (!connection_p ||
                  dbus_error_is_set (&error_s)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_bus_get_private(DBUS_BUS_SYSTEM): \"%s\", aborting\n"),
                  ACE_TEXT (error_s.message)));

      dbus_error_free (&error_s);

      return false; // *TODO*: avoid false negatives
    } // end IF
    dbus_connection_set_exit_on_disconnect (connection_p,
                                            false);

    release_connection_b = true;
  } // end IF

  struct DBusMessageIter iterator, iterator_2;
  std::string object_path_string;
  struct DBusMessage* reply_p = NULL;
  char* string_p = NULL;
//  char character_c = 0;
  const char* argument_string_p = NULL;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_SERVICE),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_OBJECT_PATH),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_INTERFACE),
                                    (toggle_in ? ACE_TEXT_ALWAYS_CHAR ("CreateInterface")
                                               : ACE_TEXT_ALWAYS_CHAR ("RemoveInterface")));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(%s): \"%m\", aborting\n"),
                (toggle_in ? ACE_TEXT_ALWAYS_CHAR ("CreateInterface")
                           : ACE_TEXT_ALWAYS_CHAR ("RemoveInterface"))));
    goto error;
  } // end IF
  dbus_message_iter_init_append (message_p, &iterator);
  if (toggle_in)
  {
    ACE_ASSERT (false); // *TODO*
  } // end IF
  else
  {
    object_path_string =
        Net_WLAN_Tools::identifierToDBusObjectPath (connection_p,
                                                    interfaceIdentifier_in);
    if (unlikely (object_path_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::identifierToDBusObjectPath(%@,\"%s\"), aborting\n"),
                  connection_p,
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
      goto error;
    } // end IF
    argument_string_p = object_path_string.c_str ();
    if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                   DBUS_TYPE_OBJECT_PATH,
                                                   &argument_string_p)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
      goto error;
    } // end IF
  } // end ELSE
  reply_p = Common_DBus_Tools::exchange (connection_p,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF

  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (toggle_in)
  {
    ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_OBJECT_PATH);
    dbus_message_iter_get_basic (&iterator_2, &string_p);
    ACE_ASSERT (string_p);
  } // end IF
//  else
//  {

//  } // end ELSE
  dbus_message_unref (reply_p); reply_p = NULL;

  return true;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);
  if (release_connection_b)
  {
    dbus_connection_close (connection_p);
    dbus_connection_unref (connection_p);
  } // end IF

  return false;
}

std::string
Net_WLAN_Tools::dBusObjectPathToIdentifier (struct DBusConnection* connection_in,
                                            const std::string& objectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::dBusObjectPathToIdentifier"));

  // initialize return value(s)
  std::string return_value;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!objectPath_in.empty ());

  struct DBusMessageIter iterator, iterator_2;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_SERVICE),
                                    objectPath_in.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(GetDevices): \"%m\", aborting\n")));
    return return_value;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  char* string_p = NULL;
//  char character_c = 0;
  const char* argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_INTERFACE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Ifname");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF

  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                  DBUS_TYPE_STRING)))
    goto error;
  dbus_message_iter_get_basic (&iterator, &string_p);
  ACE_ASSERT (string_p);
  return_value = string_p;
  dbus_message_unref (reply_p); reply_p = NULL;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

  return return_value;
}

std::string
Net_WLAN_Tools::identifierToDBusObjectPath (struct DBusConnection* connection_in,
                                            const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::identifierToDBusObjectPath"));

  // initialize return value(s)
  std::string return_value;

  // sanity check(s)
  ACE_ASSERT (connection_in);
  ACE_ASSERT (!interfaceIdentifier_in.empty ());

  struct DBusMessageIter iterator, iterator_2;
  struct DBusMessage* message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_SERVICE),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_OBJECT_PATH),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_WPASUPPLICANT_INTERFACE),
                                    ACE_TEXT_ALWAYS_CHAR ("GetInterface"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(GetInterface): \"%m\", aborting\n")));
    return return_value;
  } // end IF
  struct DBusMessage* reply_p = NULL;
  char* string_p = NULL;
//  char character_c = 0;
  const char* argument_string_p = interfaceIdentifier_in.c_str ();
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_in,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto error;
  } // end IF

  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator) == DBUS_TYPE_OBJECT_PATH);
  dbus_message_iter_get_basic (&iterator, &string_p);
  ACE_ASSERT (string_p);
  return_value = string_p;
  dbus_message_unref (reply_p); reply_p = NULL;

error:
  if (message_p)
    dbus_message_unref (message_p);
  if (reply_p)
    dbus_message_unref (reply_p);

  return return_value;
}
