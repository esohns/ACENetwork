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
#include "ace/Time_Value.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <wlanapi.h>

#include "common_tools.h"
#else
#include <ifaddrs.h>
#endif

#include "stream_defines.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::Net_WLANMonitor_T ()
 : inherited (ACE_TEXT_ALWAYS_CHAR (NET_WLANMONITOR_THREAD_NAME), // thread name
              NET_WLANMONITOR_THREAD_GROUP_ID,                    // group id
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              0,                                                  // # thread(s)
#else
              2,                                                  // # thread(s)
#endif
              false,                                              // auto-start ?
              ////////////////////////////
              //NULL)                                             // queue handle
              // *TODO*: this looks dodgy, but seems to work nonetheless...
              &queue_)                                            // queue handle
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , clientHandle_ (ACE_INVALID_HANDLE)
#else
 , connection_ (NULL)
 , identifierToObjectPath_ ()
 // , proxy_ (NULL)
#endif
 , configuration_ (NULL)
 , isActive_ (false)
 , isInitialized_ (false)
 , localSAP_ ()
 , peerSAP_ ()
 , subscribersLock_ ()
 , subscribers_ ()
 , userData_ (NULL)
 /////////////////////////////////////////
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , DBusDispatchStarted_ (false)
#endif
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::Net_WLANMonitor_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::~Net_WLANMonitor_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::~Net_WLANMonitor_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (clientHandle_ != ACE_INVALID_HANDLE)
  {
    DWORD result = WlanCloseHandle (clientHandle_, NULL);
    if (result != ERROR_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::WlanCloseHandle(0x%@): \"%s\", continuing\n"),
                  clientHandle_,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  } // end IF
#else
  if (isActive_)
  { ACE_ASSERT (connection_);
    dbus_connection_close (connection_);
    inherited::stop (true,
                     true);
  } // end IF
  if (connection_)
    dbus_connection_unref (connection_);
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::start"));

  // sanity check(s)
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF
  if (isActive_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("already started, returning\n")));
    return;
  } // end IF
  ACE_ASSERT (configuration_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ == ACE_INVALID_HANDLE);

  // *TODO*: support WinXP
  DWORD maximum_client_version =
    WLAN_API_MAKE_VERSION (2, 0); // *NOTE*: 1 for <= WinXP_SP2
  DWORD current_version = 0;
  DWORD result = 0;
  PWLAN_INTERFACE_INFO_LIST interface_list_p = NULL;
  DWORD notification_mask = WLAN_NOTIFICATION_SOURCE_ALL;
  DWORD previous_notification_mask = 0;
  result = WlanOpenHandle (maximum_client_version,
                           NULL,
                           &current_version,
                           &clientHandle_);
  if (result != ERROR_SUCCESS)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanOpenHandle(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registered WLAN API client (handle: 0x%@)\n"),
              clientHandle_));

  result = WlanRegisterNotification (clientHandle_,
                                     notification_mask,
                                     FALSE,
                                     configuration_->notificationCB,
                                     configuration_->notificationCBData,
                                     NULL,
                                     &previous_notification_mask);
  if (result != ERROR_SUCCESS)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanRegisterNotification(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (clientHandle_ != ACE_INVALID_HANDLE)
  {
    result = WlanCloseHandle (clientHandle_,
                              NULL);
    if (result != ERROR_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::WlanCloseHandle(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    clientHandle_ = ACE_INVALID_HANDLE;
  } // end IF

  return;

continue_:
#else
  // sanity check(s)
  ACE_ASSERT (!connection_);
//  ACE_ASSERT (!proxy_);

  struct DBusError error_s;
  dbus_error_init (&error_s);
  std::string device_path_string;
  std::string match_rule_string =
      ACE_TEXT_ALWAYS_CHAR ("type='signal',sender='");
  match_rule_string +=
      ACE_TEXT_ALWAYS_CHAR (NET_WLANMONITOR_DBUS_NETWORKMANAGER_SERVICE);
  match_rule_string += ACE_TEXT_ALWAYS_CHAR ("'");
  ACE_ASSERT (match_rule_string.size () <= DBUS_MAXIMUM_MATCH_RULE_LENGTH);
  connection_ = dbus_bus_get_private (DBUS_BUS_SYSTEM,
                                      &error_s);
  if (!connection_ ||
      dbus_error_is_set (&error_s))
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
  ACE_ASSERT (identifierToObjectPath_.empty ());
  // convert device identifier to object path ?
  if (!configuration_->interfaceIdentifier.empty ())
    identifierToObjectPath_.insert (std::make_pair (configuration_->interfaceIdentifier,
                                                    device_path_string));
  else
    identifierToObjectPath_ = getDevices ();
  for (INTERFACEIDENTIFIERS_ITERATOR_T iterator = identifierToObjectPath_.begin ();
       iterator != identifierToObjectPath_.end ();
       ++iterator)
  {
    (*iterator).second =
        Net_Common_Tools::deviceToDBusPath (connection_,
                                            (*iterator).first);
    if ((*iterator).second.empty ())
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::deviceToDBusPath(\"%s\"), continuing\n"),
                  ACE_TEXT ((*iterator).first).c_str ()));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": DBus object path is: \"%s\"\n"),
                  ACE_TEXT ((*iterator).first.c_str ()),
                  ACE_TEXT ((*iterator).second.c_str ())));
  } // end FOR

  // subscribe to all networkmanager signals
  if (!dbus_connection_add_filter (connection_,
                                   configuration_->notificationCB,
                                   configuration_->notificationCBData,
                                   NULL))
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
  if (dbus_error_is_set (&error_s))
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
  if (!inherited::isRunning ())
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
#endif

  isActive_ = true;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::stop (bool waitForCompletion_in,
                                       bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  if (!isActive_)
    return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);

  DWORD result = WlanCloseHandle (clientHandle_, NULL);
  if (result != ERROR_SUCCESS)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanCloseHandle(0x%@): \"%s\", returning\n"),
                clientHandle_,
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
  clientHandle_ = ACE_INVALID_HANDLE;
#else
  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);
//  deviceDBusPath_.resize (0);
#endif

  isActive_ = false;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
const std::string&
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::get1R (const std::string& value_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::get1R"));

  std::string result;

  INTERFACEIDENTIFIERS_CONSTITERATOR_T iterator =
    std::find_if (identifierToObjectPath_.begin (), identifierToObjectPath_.end (),
                  std::bind2nd (INTERFACEIDENTIFIERS_FIND_S (),
                                value_in));
  if (iterator != identifierToObjectPath_.end ())
    result = (*iterator).first;
//  else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("device object path not found (was: \"%s\"), aborting\n"),
//                ACE_TEXT (value_in.c_str ())));

  return result;
}
#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
bool
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::initialize"));

  if (isInitialized_)
  {
    if (isActive_)
      stop (true,
            true);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_ASSERT (clientHandle_ == ACE_INVALID_HANDLE);
#else
    ACE_ASSERT (!connection_);
    identifierToObjectPath_.clear ();
#endif

    localSAP_.reset ();
    peerSAP_.reset ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    DBusDispatchStarted_ = false;
#endif

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  // *TODO*: remove type inference
  userData_ = configuration_->userData;

  // sanity check(s)
  if (!configuration_->notificationCB)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("WLAN API notification callback not specified, using default implementation\n")));
    configuration_->notificationCB = network_wlan_default_notification_cb;
    Net_IWLANCB* iwlan_cb_p = this;
    configuration_->notificationCBData = iwlan_cb_p;
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("D-Bus signal callback not specified, using default implementation\n")));
    configuration_->notificationCB = network_wlan_dbus_default_filter_cb;
    configuration_->notificationCBData =
        static_cast<Net_IWLANMonitorBase*> (this);
#endif
  } // end IF
  else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("WLAN API notification callback specified, disabled event subscription\n")));
#else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("D-Bus signal callback specified, disabled event subscription\n")));
#endif
  if (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      configuration_->SSID.empty () ||
      (configuration_->SSID.size () > DOT11_SSID_MAX_LENGTH))
#else
      configuration_->SSID.empty ())
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid configuration, aborting\n")));
    return false;
  } // end IF
  isInitialized_ = true;

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::subscribe (Net_IWLANCB* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::subscribe"));

  // sanity check(s)
  if (!interfaceHandle_in)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: 0x%@), returning\n"),
                interfaceHandle_in));
    return;
  } // end IF

  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    subscribers_.push_back (interfaceHandle_in);
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::unsubscribe (Net_IWLANCB* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::unsubscribe"));

  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    SUBSCRIBERS_ITERATOR_T iterator = subscribers_.begin ();
    for (;
         iterator != subscribers_.end ();
         iterator++)
      if ((*iterator) == interfaceHandle_in)
        break;

    if (iterator != subscribers_.end ())
      subscribers_.erase (iterator);
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: 0x%@), continuing\n"),
                  interfaceHandle_in));
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
bool
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                  UserDataType>::associate (REFGUID interfaceIdentifier_in,
#else
                  UserDataType>::associate (const std::string& interfaceIdentifier_in,
#endif
                                            const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::associate"));

  // sanity check(s)
  if (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      SSID_in.empty () ||
      (SSID_in.size () > DOT11_SSID_MAX_LENGTH))
#else
      SSID_in.empty ())
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF
  if (!isActive_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not running, aborting\n")));
    return false;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL))
#else
  if (!interfaceIdentifier_in.empty ())
#endif
  {
    if (SSID () == SSID_in)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (!Net_Common_Tools::setDeviceSettingBool (clientHandle_,
                                                   interfaceIdentifier_in,
                                                   wlan_intf_opcode_background_scan_enabled,
                                                   false))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to Net_Common_Tools::setDeviceSettingBool(0x%@,%d,true), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                    clientHandle_, wlan_intf_opcode_background_scan_enabled));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("\"%s\": disabled background scans\n"),
      //              ACE_TEXT (Net_Common_Tools::interfaceToString (clientHandle_, interfaceIdentifier_in).c_str ())));
      if (!Net_Common_Tools::setDeviceSettingBool (clientHandle_,
                                                   interfaceIdentifier_in,
                                                   wlan_intf_opcode_media_streaming_mode,
                                                   true))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to Net_Common_Tools::setDeviceSettingBool(0x%@,%d,true), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                    clientHandle_, wlan_intf_opcode_media_streaming_mode));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("\"%s\": enabled streaming mode\n"),
      //              ACE_TEXT (Net_Common_Tools::interfaceToString (clientHandle_, interfaceIdentifier_in).c_str ())));
#endif
      return true; // already associated, nothing to do
    } // end IF
  } // end IF

  INTERFACEIDENTIFIERS_T devices;
  INTERFACEIDENTIFIERS_ITERATOR_T iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL))
#else
  if (interfaceIdentifier_in.empty ())
#endif
    devices = getDevices ();
  else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    devices.push_back (interfaceIdentifier_in);

  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);

  DWORD result = 0;
  DWORD flags =
    (WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES        |
     WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES);
  //struct _WLAN_BSS_LIST* wlan_bss_list_p = NULL;
  struct _WLAN_AVAILABLE_NETWORK_LIST* wlan_network_list_p = NULL;
#if defined (_DEBUG)
  //std::string phy_type_string;
  std::string SSID_string;
  std::string bss_network_type_string;
#endif
  //std::string interface_state_string;
  struct _DOT11_SSID ssid_s;
  ACE_OS::memset (&ssid_s, 0, sizeof (struct _DOT11_SSID));
  ssid_s.uSSIDLength = configuration_->SSID.size ();
  ACE_OS::memcpy (ssid_s.ucSSID,
                  SSID_in.c_str (),
                  SSID_in.size ());
  struct _WLAN_CONNECTION_PARAMETERS wlan_connection_parameters_s;
  bool done = false;

  for (INTERFACEIDENTIFIERS_ITERATOR_T iterator = devices.begin ();
       iterator != devices.end ();
       ++iterator)
  {
    //result = WlanGetNetworkBssList (handle_client,
    //                                &interface_info_p->InterfaceGuid,
    //                                &ssid_s,
    //                                dot11_BSS_type_any,
    //                                FALSE,
    //                                NULL,
    //                                &wlan_bss_list_p);
    result =
      WlanGetAvailableNetworkList (clientHandle_,
                                   &(*iterator),
                                   flags,
                                   NULL,
                                   &wlan_network_list_p);
    if (result != ERROR_SUCCESS)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanGetAvailableNetworkList(0x%@): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  clientHandle_,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    //ACE_ASSERT (wlan_bss_list_p);
    ACE_ASSERT (wlan_network_list_p);
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found %u BSSs for wireless adapter \"%s\"\n"),
    //            wlan_bss_list_p->dwNumberOfItems,
    //            ACE_TEXT_WCHAR_TO_TCHAR (interface_info_p->strInterfaceDescription)));
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("found %u network(s) for wireless adapter \"%s\"\n"),
    //            wlan_network_list_p->dwNumberOfItems,
    //            ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ())));

    for (DWORD i = 0;
         //i < wlan_bss_list_p->dwNumberOfItems;
         i < wlan_network_list_p->dwNumberOfItems;
         ++i)
    {
#if defined (_DEBUG)
      //switch (wlan_bss_entry_p->dot11BssPhyType)
      //{
      //  case dot11_phy_type_fhss:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("FHSS"); break;
      //  case dot11_phy_type_dsss:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("DSSS"); break;
      //  case dot11_phy_type_irbaseband:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("IR baseband"); break;
      //  case dot11_phy_type_ofdm:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("OFDM"); break;
      //  case dot11_phy_type_hrdsss:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("high-rate DSSS"); break;
      //  case dot11_phy_type_erp:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("ERP"); break;
      //  case dot11_phy_type_ht:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("802.11n PHY"); break;
      //  case dot11_phy_type_vht:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("802.11ac PHY"); break;
      //  case dot11_phy_type_IHV_start:
      //  case dot11_phy_type_IHV_end:
      //  case dot11_phy_type_unknown:
      //  //case dot11_phy_type_any:
      //  default:
      //    phy_type_string = ACE_TEXT_ALWAYS_CHAR ("unknown PHY type"); break;
      //} // end SWITCH
      SSID_string.assign (reinterpret_cast<CHAR*> (wlan_network_list_p->Network[i].dot11Ssid.ucSSID),
                          static_cast<std::string::size_type> (wlan_network_list_p->Network[i].dot11Ssid.uSSIDLength));
      switch (wlan_network_list_p->Network[i].dot11BssType)
      {
        case dot11_BSS_type_infrastructure:
          bss_network_type_string = ACE_TEXT_ALWAYS_CHAR ("infrastructure"); break;
        case dot11_BSS_type_independent:
          bss_network_type_string = ACE_TEXT_ALWAYS_CHAR ("ad-hoc"); break;
        case dot11_BSS_type_any:
        default:
          bss_network_type_string = ACE_TEXT_ALWAYS_CHAR ("unknown type"); break;
      } // end SWITCH
      // *TODO*: report all available information here
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("[#%u] PHY %u; type: %s: AP MAC: %s; AP type: %s; RSSI: %d (dBm); link quality %u%%; in region domain: %s; beacon interval (us): %u; channel center frequency (kHz): %u\n"),
      //            j + 1,
      //            wlan_bss_entry_p->uPhyId,
      //            ACE_TEXT (phy_type_string.c_str ()),
      //            ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (wlan_bss_entry_p->dot11Bssid,
      //                                                                  NET_LINKLAYER_802_11).c_str ()),
      //            ACE_TEXT (bss_network_type_string.c_str ()),
      //            ACE_TEXT (Common_Tools::GUIDToString (interface_info_p->InterfaceGuid).c_str ()),
      //            ACE_TEXT_WCHAR_TO_TCHAR (interface_info_p->strInterfaceDescription),
      //            ACE_TEXT (interface_state_string.c_str ()),
      //            wlan_bss_entry_p->lRssi, wlan_bss_entry_p->uLinkQuality,
      //            (wlan_bss_entry_p->bInRegDomain ? ACE_TEXT ("true") : ACE_TEXT ("false")),
      //            wlan_bss_entry_p->usBeaconPeriod * 1024,
      //            wlan_bss_entry_p->ulChCenterFrequency));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("[#%u] profile \"%s\"; SSID: %s; type: %s; connectable: \"%s\"%s; signal quality %d%% [RSSI: %d (dBm)]; security enabled: \"%s\"\n"),
                  i + 1,
                  (wlan_network_list_p->Network[i].strProfileName ? ACE_TEXT_WCHAR_TO_TCHAR (wlan_network_list_p->Network[i].strProfileName) : ACE_TEXT ("N/A")),
                  ACE_TEXT (SSID_string.c_str ()),
                  ACE_TEXT (bss_network_type_string.c_str ()),
                  (wlan_network_list_p->Network[i].bNetworkConnectable ? ACE_TEXT ("yes") : ACE_TEXT ("no")),
                  (wlan_network_list_p->Network[i].bNetworkConnectable ? ACE_TEXT ("") : ACE_TEXT (" [reason]")),
                  wlan_network_list_p->Network[i].wlanSignalQuality, (-100 + static_cast<int> (static_cast<float> (wlan_network_list_p->Network[i].wlanSignalQuality) * ::abs ((-100.0F - -50.0F) / 100.0F))),
                  (wlan_network_list_p->Network[i].bSecurityEnabled ? ACE_TEXT ("yes") : ACE_TEXT ("no"))));
#endif

      if (ACE_OS::memcmp (SSID_in.c_str (),
                          wlan_network_list_p->Network[i].dot11Ssid.ucSSID,
                          //wlan_bss_entry_p->dot11Ssid.ucSSID,
                          SSID_in.size ()))
        continue;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("\"%s\": found SSID (was: %s), associating\n"),
      //            ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
      //            ACE_TEXT (SSID_in.c_str ())));

      ACE_OS::memset (&wlan_connection_parameters_s,
                      0,
                      sizeof (struct _WLAN_CONNECTION_PARAMETERS));
      wlan_connection_parameters_s.dot11BssType =
        wlan_network_list_p->Network[i].dot11BssType;
      //wlan_connection_parameters_s.dwFlags = 0;
      //wlan_connection_parameters_s.pDesiredBssidList = NULL;
      wlan_connection_parameters_s.pDot11Ssid = &ssid_s;
      //wlan_connection_parameters_s.strProfile = NULL;
      // *TODO*: do the research here
      wlan_connection_parameters_s.wlanConnectionMode =
        wlan_connection_mode_discovery_unsecure;
      // *NOTE*: this returns immediately
      result = WlanConnect (clientHandle_,
                            &(*iterator),
                            &wlan_connection_parameters_s,
                            NULL);
      if (result != ERROR_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to ::WlanConnect(0x%@,%s): \"%s\", aborting\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                    clientHandle_,
                    ACE_TEXT (SSID_in.c_str ()),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": associating with SSID %s...\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));

      done = true;
      break;
    } // end FOR
    //WlanFreeMemory (wlan_bss_list_p);
    //wlan_bss_list_p = NULL;
    WlanFreeMemory (wlan_network_list_p);
    wlan_network_list_p = NULL;

    if (done)
      break;
  } // end FOR
  if (done)
    goto continue_;

  // SSID not found --> scan for networks and abort
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("SSID (was: %s) not found, scanning...\n"),
              ACE_TEXT (SSID_in.c_str ())));

  struct _WLAN_RAW_DATA raw_data_s;
  ACE_OS::memset (&raw_data_s, 0, sizeof (struct _WLAN_RAW_DATA));
  for (INTERFACEIDENTIFIERS_ITERATOR_T iterator = devices.begin ();
       iterator != devices.end ();
       ++iterator)
  {
    // *NOTE*: this returns immediately
    result = WlanScan (clientHandle_,
                       &(*iterator),
                       NULL, // *NOTE*: support WinXP
                       //&ssid_s,
                       NULL, // *NOTE*: support WinXP
                       //&raw_data_s,
                       NULL);
    if (result != ERROR_SUCCESS)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanScan(0x%@): \"%s\", aborting\n"),
                  clientHandle_,
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end FOR

error:
  //if (wlan_bss_list_p)
  //  WlanFreeMemory (wlan_bss_list_p);
  if (wlan_network_list_p)
    WlanFreeMemory (wlan_network_list_p);

  return false;
#else
    devices.insert (std::make_pair (interfaceIdentifier_in, ACE_TEXT_ALWAYS_CHAR ("")));

  // step1: retrieve available and matching connection profile(s)
  std::string device_object_path_string, connection_object_path_string;
  std::string access_point_object_path_string;
  INTERFACEIDENTIFIERS_ITERATOR_T iterator_2;
  iterator = devices.begin ();

next:
  if (iterator == devices.end ())
    goto error; // --> failed

  // check cache first
  iterator_2 = identifierToObjectPath_.find ((*iterator).first);
  if (iterator_2 != identifierToObjectPath_.end ())
  {
    device_object_path_string = (*iterator_2).second;
    goto retrieve_access_point;
  } // end IF
  device_object_path_string =
      Net_Common_Tools::SSIDToDeviceDBusPath (connection_,
                                              SSID_in);
  if (device_object_path_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::SSIDToDeviceDBusPath(0x%@,%s), continuing\n"),
                connection_,
                ACE_TEXT (SSID_in.c_str ())));
    ++iterator;
    goto next;
  } // end IF

retrieve_access_point:
  access_point_object_path_string =
      Net_Common_Tools::SSIDToAccessPointDBusPath (connection_,
                                                   device_object_path_string,
                                                   SSID_in);
  if (access_point_object_path_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::SSIDToAccessPointDBusPath(0x%@,\"%s\",%s), continuing\n"),
                connection_,
                ACE_TEXT (device_object_path_string.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
    ++iterator;
    goto next;
  } // end IF
  connection_object_path_string =
      Net_Common_Tools::SSIDToConnectionDBusPath (connection_,
                                                  device_object_path_string,
                                                  SSID_in);
  if (connection_object_path_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::SSIDToConnectionDBusPath(0x%@,\"%s\",%s), continuing\n"),
                connection_,
                ACE_TEXT (device_object_path_string.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
    ++iterator;
    goto next;
  } // end IF
  if (!Net_Common_Tools::activateConnection (connection_,
                                             connection_object_path_string,
                                             device_object_path_string,
                                             access_point_object_path_string))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::activateConnection(0x%@,\"%s\",\"%s\",\"%s\"), continuing\n"),
                connection_,
                ACE_TEXT (connection_object_path_string.c_str ()),
                ACE_TEXT ((*iterator).first.c_str ()),
                ACE_TEXT (access_point_object_path_string.c_str ())));
    ++iterator;
    goto next;
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("activated connection configuration \"%s\" (device: \"%s\", SSID: %s)\n"),
//              ACE_TEXT (connection_object_path_string.c_str ()),
//              ACE_TEXT ((*iterator).first.c_str ()),
//              ACE_TEXT (SSID_in.c_str ())));

  goto continue_;

error:
  return false;
#endif

continue_:
  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
std::string
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::SSID () const
{
  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
#endif
  ACE_ASSERT (configuration_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return Net_Common_Tools::associatedSSID (clientHandle_,
                                           configuration_->interfaceIdentifier);
#else
  return Net_Common_Tools::associatedSSID (configuration_->interfaceIdentifier);
#endif
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                  UserDataType>::onAssociate (REFGUID interfaceIdentifier_in,
#else
                  UserDataType>::onAssociate (const std::string& interfaceIdentifier_in,
#endif
                                              const std::string& SSID_in,
                                              bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::onAssociate"));

  // synch access
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // *NOTE*: this works because the lock is recursive
    // *WARNING* if callees unsubscribe() within the callback bad things
    //           happen, as the current iterator is invalidated
    //           --> use a slightly modified for-loop (advance first before
    //               invoking the callback (works for MOST containers...)
    for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_.begin ();
         iterator != subscribers_.end ();
         )
    {
      try {
        (*(iterator++))->onAssociate (interfaceIdentifier_in,
                                      SSID_in,
                                      success_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_IWLANCB::onAssociate(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  // sanity check(s)
  if (!success_in)
  { // *TODO*: this also happens when the device has been disconnected
    //         - manually (i.e. by the user)
    //         - while it is switching to a different SSID (auto-connect)
    //         - ...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to associate with SSID %s, retrying...\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to associate with SSID %s, retrying...\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#endif

    if (!associate (interfaceIdentifier_in,
                    configuration_->SSID))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), returning\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (configuration_->SSID.c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), returning\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ()),
                  ACE_TEXT (configuration_->SSID.c_str ())));
#endif
    return;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!Net_Common_Tools::setDeviceSettingBool (clientHandle_,
                                               interfaceIdentifier_in,
                                               wlan_intf_opcode_background_scan_enabled,
                                               !success_in))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to Net_Common_Tools::setDeviceSettingBool(0x%@,%d,true), continuing\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                clientHandle_, wlan_intf_opcode_background_scan_enabled));
  //else
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("\"%s\": %s background scans\n"),
  //              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
  //              (success_in ? ACE_TEXT ("disabled") : ACE_TEXT ("enabled"))));
  if (!Net_Common_Tools::setDeviceSettingBool (clientHandle_,
                                               interfaceIdentifier_in,
                                               wlan_intf_opcode_media_streaming_mode,
                                               success_in))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": failed to Net_Common_Tools::setDeviceSettingBool(0x%@,%d,true), continuing\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                clientHandle_, wlan_intf_opcode_media_streaming_mode));
  //else
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("\"%s\": %s streaming mode\n"),
  //              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
  //              (success_in ? ACE_TEXT ("enabled") : ACE_TEXT ("disabled"))));
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": associated with SSID %s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": associated with SSID %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#endif
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                  UserDataType>::onConnect (REFGUID interfaceIdentifier_in,
#else
                  UserDataType>::onConnect (const std::string& interfaceIdentifier_in,
#endif
                                            const std::string& SSID_in,
                                            bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::onConnect"));

  // synch access
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // *NOTE*: this works because the lock is recursive
    // *WARNING* if callees unsubscribe() within the callback bad things
    //           happen, as the current iterator is invalidated
    //           --> use a slightly modified for-loop (advance first before
    //               invoking the callback (works for MOST containers...)
    for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_.begin ();
         iterator != subscribers_.end ();
         )
    {
      try {
        (*(iterator++))->onConnect (interfaceIdentifier_in,
                                    SSID_in,
                                    success_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_IWLANCB::onConnect(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  //// sanity check(s)
  //if (!success_in)
  //  return;

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  if (!Net_Common_Tools::interfaceToIPAddress (Common_Tools::GUIDToString (interfaceIdentifier_in),
//#else
//  if (!Net_Common_Tools::interfaceToIPAddress (interfaceIdentifier_in,
//#endif
//                                               localSAP_,
//                                               peerSAP_))
//  {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
//                ACE_TEXT (Net_Common_Tools::interfaceToString (clientHandle_, interfaceIdentifier_in).c_str ())));
//#else
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
//                ACE_TEXT (interfaceIdentifier_in.c_str ())));
//#endif
//    return;
//  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": connected to SSID %s: %s <---> %s\n"),
              //ACE_TEXT (Net_Common_Tools::WLANInterfaceToString (clientHandle_, interfaceIdentifier_in).c_str ()),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (SSID_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peerSAP_).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": connected to SSID %s: %s <---> %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (SSID_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peerSAP_).c_str ())));
#endif
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                  UserDataType>::onHotPlug (REFGUID interfaceIdentifier_in,
#else
                  UserDataType>::onHotPlug (const std::string& interfaceIdentifier_in,
#endif
                                            bool enabled_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::onHotPlug"));

  // synch access
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // *NOTE*: this works because the lock is recursive
    // *WARNING* if callees unsubscribe() within the callback bad things
    //           happen, as the current iterator is invalidated
    //           --> use a slightly modified for-loop (advance first before
    //               invoking the callback (works for MOST containers...)
    for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_.begin ();
         iterator != subscribers_.end ();
         )
    {
      try {
        (*(iterator++))->onHotPlug (interfaceIdentifier_in,
                                    enabled_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_IWLANCB::onHotPlug(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": interface %s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              (enabled_in ? ACE_TEXT ("enabled") : ACE_TEXT ("disabled/removed"))));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": interface %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              (enabled_in ? ACE_TEXT ("enabled") : ACE_TEXT ("disabled/removed"))));
#endif
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                  UserDataType>::onScanComplete (REFGUID interfaceIdentifier_in)
#else
                  UserDataType>::onScanComplete (const std::string& interfaceIdentifier_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::onScanComplete"));

  // synch access
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // *NOTE*: this works because the lock is recursive
    // *WARNING* if callees unsubscribe() within the callback bad things
    //           happen, as the current iterator is invalidated
    //           --> use a slightly modified for-loop (advance first before
    //               invoking the callback (works for MOST containers...)
    for (SUBSCRIBERS_ITERATOR_T iterator = subscribers_.begin ();
         iterator != subscribers_.end ();
         )
    {
      try {
        (*(iterator++))->onScanComplete (interfaceIdentifier_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_IWLANCB::onScanComplete(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  // sanity check(s)
  if (!isInitialized_ || !isActive_)
    return;
  ACE_ASSERT (configuration_);
  if ((SSID () == configuration_->SSID) ||
      !configuration_->autoAssociate)
    return;

  if (!associate (interfaceIdentifier_in,
                  configuration_->SSID))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), returning\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), returning\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#endif
}

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
int
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::svc (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::svc"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): worker thread (id: %t) starting\n"),
              ACE_TEXT (inherited::threadName_.c_str ())));

  DBusDispatchStatus dispatch_status = DBUS_DISPATCH_COMPLETE;

  { ACE_GUARD_RETURN (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_, -1);
    if (DBusDispatchStarted_)
      goto monitor_thread;
    DBusDispatchStarted_ = true;
  } // end lock scope

//dbus_dispatch_thread:
  // sanity check(s)
  ACE_ASSERT (connection_);

  // *IMPORTANT NOTE*: do NOT block in dbus_connection_read_write_dispatch until
  //                   there is some way to wake it up externally
  //  while (dbus_connection_read_write_dispatch (connection_,
//                                              -1)) // block
  do
  {
    if (unlikely (!dbus_connection_read_write_dispatch (connection_,
                                                        10))) // timeout (ms)
      break; // done

    do
    {
      dispatch_status = dbus_connection_dispatch (connection_);
      if (dispatch_status == DBUS_DISPATCH_DATA_REMAINS)
        continue; // <-- process more data
      if (unlikely (dispatch_status != DBUS_DISPATCH_COMPLETE))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("(%s): worker thread (id: %t) failed to dbus_connection_dispatch() (status was: %d), continuing\n"),
                    ACE_TEXT (inherited::threadName_.c_str ()),
                    dispatch_status));
      break;
    } while (true);
  } while (true);

  // clean up
  dbus_connection_unref (connection_);
  connection_ = NULL;
  { ACE_GUARD_RETURN (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_, -1);
    DBusDispatchStarted_ = false;
  } // end lock scope

  return 0;

monitor_thread:
  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (!configuration_->SSID.empty ());

  int result = -1;
  ACE_Time_Value interval (1, 0);
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value now = COMMON_TIME_NOW;
  do
  {
    if (configuration_->SSID != Net_Common_Tools::associatedSSID (configuration_->interfaceIdentifier))
    {
      // SSID not found ? --> initiate scan
      if (unlikely (!Net_Common_Tools::hasSSID (configuration_->interfaceIdentifier,
                                                configuration_->SSID)))
      {
        Net_Common_Tools::scan (configuration_->interfaceIdentifier);
        goto sleep;
      } // end IF

      if (unlikely (!associate (configuration_->interfaceIdentifier,
                                configuration_->SSID)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("(%s): worker thread (id: %t) failed to Net_IWLANMonitor_T::associate(\"%s\",%s), retrying in %#T...\n"),
                    ACE_TEXT (inherited::threadName_.c_str ()),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ()),
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
      message_block_p->release ();

      break; // done
    } // end IF
    message_block_p->release ();
    message_block_p = NULL;
  } while (true);

  return 0;
}
#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
std::vector<struct _GUID>
#else
std::map<std::string, std::string>
#endif
Net_WLANMonitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  UserDataType>::getDevices () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLANMonitor_T::getDevices"));

  INTERFACEIDENTIFIERS_T result;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);

  PWLAN_INTERFACE_INFO_LIST interface_list_p = NULL;
  DWORD result_2 = WlanEnumInterfaces (clientHandle_,
                                       NULL,
                                       &interface_list_p);
  if (result_2 != ERROR_SUCCESS)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanEnumInterfaces(0x%@): \"%s\", aborting\n"),
                clientHandle_,
                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (interface_list_p);
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("found %u WLAN interface(s)\n"),
  //            interface_list_p->dwNumberOfItems));

#if defined (_DEBUG)
  std::string interface_state_string;
#endif
  for (DWORD i = 0;
       i < interface_list_p->dwNumberOfItems;
       ++i)
  {
#if defined (_DEBUG)
    switch (interface_list_p->InterfaceInfo[i].isState)
    {
      case wlan_interface_state_not_ready:
        interface_state_string = ACE_TEXT_ALWAYS_CHAR ("not ready"); break;
      case wlan_interface_state_connected:
        interface_state_string = ACE_TEXT_ALWAYS_CHAR ("connected"); break;
      case wlan_interface_state_ad_hoc_network_formed:
        interface_state_string = ACE_TEXT_ALWAYS_CHAR ("first node in a ad hoc network"); break;
      case wlan_interface_state_disconnecting:
        interface_state_string = ACE_TEXT_ALWAYS_CHAR ("disconnecting"); break;
      case wlan_interface_state_disconnected:
        interface_state_string = ACE_TEXT_ALWAYS_CHAR ("not connected"); break;
      case wlan_interface_state_associating:
        interface_state_string = ACE_TEXT_ALWAYS_CHAR ("attempting to associate with a network"); break;
      case wlan_interface_state_discovering:
        interface_state_string = ACE_TEXT_ALWAYS_CHAR ("auto configuration is discovering settings for the network"); break;
      case wlan_interface_state_authenticating:
        interface_state_string = ACE_TEXT_ALWAYS_CHAR ("in process of authenticating"); break;
      default:
        interface_state_string = ACE_TEXT_ALWAYS_CHAR ("unknown state"); break;
    } // end SWITCH
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("[#%u] %s: \"%s\": %s\n"),
                i + 1,
                ACE_TEXT (Common_Tools::GUIDToString (interface_list_p->InterfaceInfo[i].InterfaceGuid).c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR (interface_list_p->InterfaceInfo[i].strInterfaceDescription),
                ACE_TEXT (interface_state_string.c_str ())));
#endif
    result.push_back (interface_list_p->InterfaceInfo[i].InterfaceGuid);
  } // end FOR
  WlanFreeMemory (interface_list_p);
#else
#if defined (ACE_HAS_GETIFADDRS)
  struct ifaddrs* ifaddrs_p = NULL;
  int result_2 = ::getifaddrs (&ifaddrs_p);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to ::getifaddrs(): \"%m\", aborting\n")));
    return result;
  } // end IF
  ACE_ASSERT (ifaddrs_p);

  for (struct ifaddrs* ifaddrs_2 = ifaddrs_p;
       ifaddrs_2;
       ifaddrs_2 = ifaddrs_2->ifa_next)
  {
//    if ((ifaddrs_2->ifa_flags & IFF_UP) == 0)
//      continue;
    if (!ifaddrs_2->ifa_addr)
      continue;
    if (ifaddrs_2->ifa_addr->sa_family != AF_INET)
      continue;
    if (!Net_Common_Tools::interfaceIsWLAN (ifaddrs_2->ifa_name))
      continue;

    result.insert (std::make_pair (ifaddrs_2->ifa_name,
                                   ACE_TEXT_ALWAYS_CHAR ("")));
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */
#endif

  return result;
}
