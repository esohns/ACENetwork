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

#include <algorithm>
#include <functional>

#include "net/ethernet.h"

#if defined (DHCLIENT_SUPPORT)
extern "C"
{
#include "dhcpctl/dhcpctl.h"
}
#endif // DHCLIENT_SUPPORT

#if defined (_DEBUG)
#include "ace/High_Res_Timer.h"
#endif // _DEBUG
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"
#include "ace/Time_Value.h"

#include "common_tools.h"

#if defined (_DEBUG)
#include "common_timer_tools.h"
#endif // _DEBUG

#include "stream_defines.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"
#include "net_packet_headers.h"

#include "net_wlan_common.h"
#include "net_wlan_defines.h"
#include "net_wlan_tools.h"

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited ()
 , connection_ (NULL)
 // , proxy_ (NULL)
 , objectPathCache_ ()
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

  inherited::TASK_T::threadCount_ = 2;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::~Net_WLAN_Monitor_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::~Net_WLAN_Monitor_T"));

  if (unlikely (inherited::isActive_))
  { ACE_ASSERT (connection_);
    dbus_connection_close (connection_);
    inherited::stop (true,  // wait ?
                     false, // high priority ?
                     true); // locked access ?
  } // end IF
  if (unlikely (connection_))
    dbus_connection_unref (connection_);
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::start"));

  // sanity check(s)
  if (unlikely (!inherited::isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF
  if (unlikely (inherited::isActive_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("already started, returning\n")));
    return;
  } // end IF
  ACE_ASSERT (inherited::configuration_);

  // sanity check(s)
  ACE_ASSERT (!connection_);
//  ACE_ASSERT (!proxy_);

  struct DBusError error_s;
  dbus_error_init (&error_s);
  std::string device_path_string;
  std::string match_rule_string =
      ACE_TEXT_ALWAYS_CHAR ("type='signal',sender='");
  match_rule_string +=
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE);
  match_rule_string += ACE_TEXT_ALWAYS_CHAR ("'");
  ACE_ASSERT (match_rule_string.size () <= DBUS_MAXIMUM_MATCH_RULE_LENGTH);
  connection_ = dbus_bus_get_private (DBUS_BUS_SYSTEM,
                                      &error_s);
  if (unlikely (!connection_ ||
                dbus_error_is_set (&error_s)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_bus_get_private(DBUS_BUS_SYSTEM): \"%s\", returning\n"),
                ACE_TEXT (error_s.message)));
    dbus_error_free (&error_s);
    return;
  } // end IF
  dbus_connection_set_exit_on_disconnect (connection_,
                                          false);

  // initialize cache
  ACE_ASSERT (objectPathCache_.empty ());
  Net_InterfaceIdentifiers_t interface_identifiers_a;
  std::string device_object_path;
  // convert device identifier to object path ?
  if (!inherited::configuration_->interfaceIdentifier.empty ())
    interface_identifiers_a.push_back (inherited::configuration_->interfaceIdentifier);
  else
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (connection_,
                                                             AF_UNSPEC,
                                                             0);
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    device_object_path =
        Net_WLAN_Tools::deviceToDBusObjectPath (connection_,
                                                *iterator);
    if (unlikely (device_object_path.empty ()))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceToDBusObjectPath(\"%s\"), continuing\n"),
                  ACE_TEXT ((*iterator).c_str ())));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": DBus object path is: \"%s\"\n"),
                  ACE_TEXT ((*iterator).c_str ()),
                  ACE_TEXT (device_object_path.c_str ())));
    objectPathCache_.insert (std::make_pair (*iterator,
                                             device_object_path));
  } // end FOR

  // subscribe to all networkmanager signals
  if (unlikely (!dbus_connection_add_filter (connection_,
                                             inherited::configuration_->notificationCB,
                                             inherited::configuration_->notificationCBData,
                                             NULL)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_connection_add_filter(): \"%m\", returning\n")));
    goto error;
  } // end IF
  // *NOTE*: according to the API documentation, this call should block until
  //         the dbus server acknowleges the request (which should actually
  //         hang, as the message processing loop has not started yet, see:
  //         svc()). However, this seems to work as needed anyway
  // *TODO*: find out why
  dbus_bus_add_match (connection_,
                      match_rule_string.c_str (),
                      &error_s);
  if (unlikely (dbus_error_is_set (&error_s)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_bus_add_match(): \"%s\", returning\n"),
                ACE_TEXT (error_s.message)));
    dbus_error_free (&error_s);
    goto error;
  } // end IF

  //  proxy_ =
  //      dbus_g_proxy_new_for_name (connection_,
  //                                 ACE_TEXT_ALWAYS_CHAR (NET_WLANMONITOR_DBUS_NETWORKMANAGER_SERVICE),
  //                                 ACE_TEXT_ALWAYS_CHAR (NET_WLANMONITOR_DBUS_NETWORKMANAGER_OBJECT_PATH),
  //                                 ACE_TEXT_ALWAYS_CHAR (NET_WLANMONITOR_DBUS_NETWORKMANAGER_INTERFACE));
  //  if (!proxy_)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to dbus_g_proxy_new_for_name(): \"%m\", returning\n")));
  //    goto error;
  //  } // end IF
  //  dbus_g_proxy_add_signal (proxy_,
  //                           ACE_TEXT_ALWAYS_CHAR ("StateChange"),
  //                           G_TYPE_UINT,
  //                           G_TYPE_INVALID);
  //  dbus_g_proxy_connect_signal (proxy_,
  //                               ACE_TEXT_ALWAYS_CHAR ("StateChange"),
  //                               G_CALLBACK (media_key_pressed),
  //                               NULL,
  //                               NULL);

//  dbus_connection_set_wakeup_main_function (connection_,
//                                            network_wlan_dbus_main_wakeup_cb,
//                                            connection_,
//                                            NULL);

  // dispatch DBus messages in a dedicated thread
  inherited::open (NULL);
  if (unlikely (!inherited::isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", returning\n")));
    goto error;
  } // end IF
  dbus_connection_flush (connection_);

  goto continue_;

error:
  if (connection_)
  {
    dbus_connection_close (connection_);
    dbus_connection_unref (connection_);
    connection_ = NULL;
  } // end IF

  return;

continue_:
  inherited::isActive_ = true;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool highPriority_in,
                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  // sanity check(s)
  if (!inherited::isActive_)
    return;

  inherited::stop (waitForCompletion_in,
                   highPriority_in,
                   lockedAccess_in);
//  deviceDBusObjectPath_.resize (0);

  inherited::isActive_ = false;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    ACE_ASSERT (!connection_);
    objectPathCache_.clear ();

    userData_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  userData_ = configuration_in.userData;

  // sanity check(s)
  if (!configuration_in.notificationCB)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("D-Bus signal callback not specified, using default implementation\n")));
    const_cast<ConfigurationType&> (configuration_in).notificationCB =
        network_wlan_dbus_default_filter_cb;
    const_cast<ConfigurationType&> (configuration_in).notificationCBData =
        static_cast<Net_WLAN_IMonitorBase*> (this);
  } // end IF
  else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("D-Bus signal callback specified, disabled event subscription\n")));

  return inherited::initialize (configuration_in);
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
const std::string&
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::get1RR_2 (const std::string& value_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::get1RR_2"));

  std::string result;

  INTERFACEIDENTIFIER_TO_OBJECTPATH_CONSTITERATOR_T iterator =
    std::find_if (objectPathCache_.begin (), objectPathCache_.end (),
                  std::bind2nd (INTERFACEIDENTIFIER_TO_OBJECTPATH_FIND_PREDICATE (),
                                value_in));
  if (iterator != objectPathCache_.end ())
    return (*iterator).first;
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("device object path not found (was: \"%s\"), aborting\n"),
//                ACE_TEXT (value_in.c_str ())));

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::set2R (const std::string& interfaceIdentifier_in,
                                         const std::string& objectPath_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::set2R"));

  // sanity check(s)
  INTERFACEIDENTIFIER_TO_OBJECTPATH_CONSTITERATOR_T iterator =
    objectPathCache_.find (interfaceIdentifier_in);
  ACE_ASSERT (iterator == objectPathCache_.end ());

  objectPathCache_.insert (std::make_pair (interfaceIdentifier_in,
                                           objectPath_in));
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::do_associate (const std::string& interfaceIdentifier_in,
                                                const struct ether_addr& APMACAddress_in,
                                                const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::do_associate"));

  ACE_UNUSED_ARG (APMACAddress_in);

  // sanity check(s)
  if (unlikely (SSID_in.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF
  if (unlikely (!inherited::isActive_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not running, aborting\n")));
    return false;
  } // end IF

  // step1: translate SSID to the corresponding device object path
  // check cache first
  std::string device_object_path_string;
  Net_WLAN_AccessPointCacheConstIterator_t iterator =
      inherited::SSIDCache_.find (SSID_in);
  if (likely (iterator != inherited::SSIDCache_.end ()))
  {
    // check cache first
    INTERFACEIDENTIFIER_TO_OBJECTPATH_ITERATOR_T iterator_2 =
        objectPathCache_.find ((*iterator).second.first);
    if (likely (iterator_2 != objectPathCache_.end ()))
      device_object_path_string = (*iterator_2).second;
    else
    {
      device_object_path_string =
          Net_WLAN_Tools::deviceToDBusObjectPath (connection_,
                                            (*iterator).second.first);
      if (unlikely (device_object_path_string.empty ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::deviceToDBusObjectPath(0x%@,\"%s\"), aborting\n"),
                    connection_,
                    ACE_TEXT ((*iterator).second.first.c_str ())));
        return false;
      } // end IF
    } // end ELSE
  } // end IF
  else
  {
    device_object_path_string =
        Net_WLAN_Tools::SSIDToDeviceDBusObjectPath (connection_,
                                              SSID_in);
    if (unlikely (device_object_path_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToDeviceDBusObjectPath(0x%@,%s), aborting\n"),
                  connection_,
                  ACE_TEXT (SSID_in.c_str ())));
      return false;
    } // end IF
  } // end ELSE
  ACE_ASSERT (!device_object_path_string.empty ());

  // step2: retrieve corresponding access point object path
  std::string access_point_object_path_string =
      Net_WLAN_Tools::SSIDToAccessPointDBusObjectPath (connection_,
                                                       device_object_path_string,
                                                       SSID_in);
  if (unlikely (access_point_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToAccessPointDBusObjectPath(0x%@,\"%s\",%s), aborting\n"),
                connection_,
                ACE_TEXT (device_object_path_string.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
    return false;
  } // end IF

  // step3: retrieve available and matching connection profile(s)
  std::string connection_object_path_string =
      Net_WLAN_Tools::SSIDToConnectionDBusObjectPath (connection_,
                                                      device_object_path_string,
                                                      SSID_in);
  if (unlikely (connection_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToConnectionDBusObjectPath(0x%@,\"%s\",%s), aborting\n"),
                connection_,
                ACE_TEXT (device_object_path_string.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
    return false;
  } // end IF

  // step4: activate the most suitable connection profile
  if (unlikely (!Net_WLAN_Tools::activateConnection (connection_,
                                                     connection_object_path_string,
                                                     device_object_path_string,
                                                     access_point_object_path_string)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::activateConnection(0x%@,\"%s\",\"%s\",\"%s\"), aborting\n"),
                connection_,
                ACE_TEXT (connection_object_path_string.c_str ()),
                ACE_TEXT (device_object_path_string.c_str ()),
                ACE_TEXT (access_point_object_path_string.c_str ())));
    return false;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": activated connection configuration \"%s\" (SSID was: %s)\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (connection_object_path_string.c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#endif

  return true;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::do_scan (const std::string& interfaceIdentifier_in,
                                           const struct ether_addr& APMACAddress_in,
                                           const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::do_scan"));

  // sanity check(s)
  if (unlikely (!inherited::isActive_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not running, returning\n")));
    return;
  } // end IF

  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (unlikely (interfaceIdentifier_in.empty ()))
    interface_identifiers_a =
        Net_WLAN_Tools::getInterfaces (connection_, // DBus connection handle
                                       AF_UNSPEC,   // address family
                                       0);          // flag(s)
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);

  // *TODO*
  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
int
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::svc (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::svc"));

  DBusDispatchStatus dispatch_status = DBUS_DISPATCH_COMPLETE;

  { ACE_GUARD_RETURN (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_, -1);
    if (inherited::dispatchStarted_)
      goto monitor_thread;
    inherited::dispatchStarted_ = true;
  } // end lock scope

//dbus_dispatch_thread:
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): D-Bus dispatch (group: %d, thread id: %t) starting\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));
#endif // _DEBUG

  // sanity check(s)
  ACE_ASSERT (connection_);

  do
  {
    if (unlikely (!dbus_connection_read_write_dispatch (connection_,
                                                        -1))) // timeout (ms)
      break; // done

    do
    {
      dispatch_status = dbus_connection_dispatch (connection_);
      if (dispatch_status == DBUS_DISPATCH_DATA_REMAINS)
        continue; // <-- process more data
      if (unlikely (dispatch_status != DBUS_DISPATCH_COMPLETE))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dbus_connection_dispatch() (status was: %d), continuing\n"),
                    dispatch_status));
      break;
    } while (true);
  } while (true);

  // clean up
  dbus_connection_unref (connection_);
  connection_ = NULL;
  { ACE_GUARD_RETURN (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_, -1);
    inherited::dispatchStarted_ = false;
  } // end lock scope

  return 0;

monitor_thread:
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): monitor (group: %d, thread id: %t) starting\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));
#endif // _DEBUG

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (!inherited::configuration_->SSID.empty ());

  int result = -1;
  ACE_Time_Value interval (1, 0);
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value now = COMMON_TIME_NOW;
  struct ether_addr ap_mac_address_s_s;
  do
  {
    if (inherited::configuration_->SSID != Net_WLAN_Tools::associatedSSID (connection_,
                                                                           inherited::configuration_->interfaceIdentifier))
    {
      // SSID not found ? --> initiate scan
      if (unlikely (!Net_WLAN_Tools::hasSSID (connection_,
                                              inherited::configuration_->interfaceIdentifier,
                                              inherited::configuration_->SSID)))
      {
        Net_WLAN_Tools::scan (connection_,
                              inherited::configuration_->interfaceIdentifier,
                              ap_mac_address_s_s,
                              inherited::configuration_->SSID,
                              false,
                              false,
                              false);
        goto sleep;
      } // end IF

      if (unlikely (!inherited::associate (inherited::configuration_->interfaceIdentifier,
                                           ap_mac_address_s_s,
                                           inherited::configuration_->SSID)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), retrying in %#T...\n"),
                    ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (inherited::configuration_->SSID.c_str ()),
                    &interval));
    } // end IF

sleep:
    result = ACE_OS::sleep (interval);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &interval));

    // done ?
    result = inherited::getq (message_block_p, &now);
    if (likely (result == -1))
    {
      int error = ACE_OS::last_error ();
      if (unlikely ((error != EAGAIN) &&
                    (error != ESHUTDOWN)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
        return -1;
      } // end IF
      continue;
    } // end IF
    ACE_ASSERT (message_block_p);
    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      // clean up
      if (connection_)
      { // *NOTE*: wakes up the other thread (see above)
        dbus_connection_close (connection_);
      } // end IF
      message_block_p->release (); message_block_p = NULL;
      break; // done
    } // end IF
    message_block_p->release (); message_block_p = NULL;
  } while (true);

  return 0;
}
