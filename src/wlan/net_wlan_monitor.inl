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

#include "ace/Log_Msg.h"
#include "ace/POSIX_Proactor.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"
#include "ace/Time_Value.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <ifaddrs.h>
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_tools.h"
#endif

#include "stream_defines.h"

#include "net_common_tools.h"
#include "net_macros.h"
#include "net_packet_headers.h"

#include "net_wlan_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_THREAD_NAME), // thread name
              NET_WLAN_MONITOR_THREAD_GROUP_ID,                    // group id
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              0,                                                  // # thread(s)
#else
              1,                                                  // # thread(s)
#endif
              false,                                              // auto-start ?
              ////////////////////////////
              //NULL)                                             // queue handle
              // *TODO*: this looks dodgy, but seems to work nonetheless...
              &queue_)                                            // queue handle
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , clientHandle_ (ACE_INVALID_HANDLE)
#else
 , buffer_ (NULL)
 , bufferSize_ (0)
 , handle_ (ACE_INVALID_HANDLE)
// , isRegistered_ (false)
 , range_ ()
#endif
 , configuration_ (NULL)
 , interfaceIdentifiers_ ()
 , isActive_ (false)
 , isInitialized_ (false)
 , localSAP_ ()
 , peerSAP_ ()
 , subscribersLock_ ()
 , subscribers_ ()
 , userData_ (NULL)
 /////////////////////////////////////////
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
 , SSIDsToInterfaceIdentifier_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

  inherited::reactor (ACE_Reactor::instance ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_OS::memset (&range_, 0, sizeof (struct iw_range));

  isc_result_t status_i = dhcpctl_initialize ();
  if (status_i != ISC_R_SUCCESS)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_initialize(): \"%s\", continuing\n"),
                ACE_TEXT (isc_result_totext (status_i))));
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::~Net_WLAN_Monitor_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::~Net_WLAN_Monitor_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (clientHandle_ != ACE_INVALID_HANDLE))
  {
    DWORD result = WlanCloseHandle (clientHandle_, NULL);
    if (result != ERROR_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::WlanCloseHandle(0x%@): \"%s\", continuing\n"),
                  clientHandle_,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  } // end IF
#else
  if (unlikely (isActive_))
    inherited::stop (true,
                     true);

  int result = -1;
//  if (unlikely (isRegistered_))
//  {
//    ACE_Reactor* reactor_p = inherited::reactor ();
//    ACE_ASSERT (reactor_p);
//    result = reactor_p->remove_handler (this,
//                                        ACE_Event_Handler::READ_MASK);
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d,ACE_Event_Handler::READ_MASK): \"%m\", continuing\n"),
//                  handle_));
//  } // end IF

  if (unlikely (handle_ != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::close (handle_);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  handle_));
  } // end IF

  if (unlikely (buffer_))
    ACE_OS::free (buffer_);
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::start"));

  // sanity check(s)
  if (unlikely (!isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF
  if (unlikely (isActive_))
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
  if (unlikely (result != ERROR_SUCCESS))
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
  if (unlikely (result != ERROR_SUCCESS))
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
  ACE_ASSERT (handle_ == ACE_INVALID_HANDLE);

  handle_ = ACE_OS::socket (AF_INET,
                            SOCK_DGRAM,
                            0);
  if (unlikely (handle_ == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(AF_INET,SOCK_DGRAM,0): \"%m\", returning\n")));
    return;
  } // end IF
//  inherited::set_handle (handle_);

  int result = iw_get_range_info (handle_,
                                  configuration_->interfaceIdentifier.c_str (),
                                  &range_);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_get_range_info(%d,\"%s\"): \"%m\", returning\n"),
                handle_,
                ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
    return;
  } // end IFs
  // verify that the interface supports scanning
  if (range_.we_version_compiled < 14)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("interface (was: \"%s\") does not support scanning, returning\n"),
                ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
    return;
  } // end IF

  // monitor the interface in a dedicated thread
  inherited::open (NULL);
  if (unlikely (!inherited::isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", returning\n")));
    return;
  } // end IF
#endif

  isActive_ = true;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  if (!isActive_)
    return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);

  DWORD result = WlanCloseHandle (clientHandle_, NULL);
  if (unlikely (result != ERROR_SUCCESS))
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

  int result = -1;
//  if (likely (isRegistered_))
//  {
//    ACE_Reactor* reactor_p = inherited::reactor ();
//    ACE_ASSERT (reactor_p);
//    result = reactor_p->remove_handler (this,
//                                        ACE_Event_Handler::READ_MASK);
//    if (unlikely (result == -1))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d,ACE_Event_Handler::READ_MASK): \"%m\", continuing\n"),
//                  handle_));
//    isRegistered_ = false;
//  } // end IF

  if (likely (handle_ != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::close (handle_);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  handle_));
    handle_ = ACE_INVALID_HANDLE;
  } // end IF

  if (likely (buffer_))
  {
    ACE_OS::free (buffer_);
    buffer_ = NULL;
  } // end IF
  bufferSize_ = 0;
#endif

  isActive_ = false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  MonitorAPI_e,
                  UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (isInitialized_))
  {
    if (isActive_)
      stop (true,
            true);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_ASSERT (clientHandle_ == ACE_INVALID_HANDLE);
#else
//    ACE_OS::memset (&range_, 0, sizeof (struct iw_range));
#endif

    configuration_ = NULL;
    interfaceIdentifiers_.clear ();
    localSAP_.reset ();
    peerSAP_.reset ();

    { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
      subscribers_.clear ();
    } // end lock scope

    userData_ = NULL;
    unsigned int result = queue_.flush (false);
    ACE_UNUSED_ARG (result);
    SSIDsToInterfaceIdentifier_.clear ();

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  // *TODO*: remove type inference
  userData_ = configuration_->userData;

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!configuration_->notificationCB)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("WLAN API notification callback not specified, using default implementation\n")));
    configuration_->notificationCB = network_wlan_default_notification_cb;
    configuration_->notificationCBData =
        static_cast<Net_WLAN_IMonitorCB*> (this);
  } // end IF
  else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("WLAN API notification callback specified, disabled event subscription\n")));
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (configuration_->SSID.empty () ||
                (configuration_->SSID.size () > DOT11_SSID_MAX_LENGTH)))
#else
  if (unlikely (configuration_->SSID.empty ()))
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid configuration, aborting\n")));
    return false;
  } // end IF
  if (unlikely (configuration_->subscriber))
    subscribe (configuration_->subscriber);

  isInitialized_ = true;

  inherited2::initialize ();

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::subscribe (Net_WLAN_IMonitorCB* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::subscribe"));

  // sanity check(s)
  if (unlikely (!interfaceHandle_in))
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
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::unsubscribe (Net_WLAN_IMonitorCB* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::unsubscribe"));

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
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::associate (REFGUID interfaceIdentifier_in,
#else
                   UserDataType>::associate (const std::string& interfaceIdentifier_in,
                                             const struct ether_addr& APMACAddress_in,
#endif
                                             const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::associate"));

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (SSID_in.empty () ||
                (SSID_in.size () > DOT11_SSID_MAX_LENGTH)))
#else
  if (unlikely (SSID_in.empty ()))
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF
  if (unlikely (!isActive_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not running, aborting\n")));
    return false;
  } // end IF
  std::string ssid_s = SSID ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (likely (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL)))
#else
  if (likely (!interfaceIdentifier_in.empty ()))
#endif
    if (unlikely (!ACE_OS::strcmp (ssid_s.c_str (),
                                   SSID_in.c_str ())))
      return true; // --> nothing to do

  if (!ssid_s.empty ())
    Net_WLAN_Tools::disassociate (interfaceIdentifier (),
                                  handle_);

  INTERFACEIDENTIFIERS_T interface_identifiers;
  INTERFACEIDENTIFIERS_ITERATOR_T iterator;
  SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator_2;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID interface_identifier = interfaceIdentifier_in;
#else
  std::string interface_identifier = interfaceIdentifier_in;
  struct ether_addr ap_mac_address = APMACAddress_in;
#endif
  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL))
#else
    if (interfaceIdentifier_in.empty ())
#endif
      interface_identifiers = interfaceIdentifiers_;
    else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      interface_identifiers.push_back (interfaceIdentifier_in);
  } // end lock scope

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

  for (INTERFACEIDENTIFIERS_ITERATOR_T iterator = interface_identifiers.begin ();
       iterator != interface_identifiers.end ();
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
    if (unlikely (result != ERROR_SUCCESS))
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
      if (unlikely (result != ERROR_SUCCESS))
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
  for (INTERFACEIDENTIFIERS_ITERATOR_T iterator = interface_identifiers.begin ();
       iterator != interface_identifiers.end ();
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
    if (unlikely (result != ERROR_SUCCESS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to ::WlanScan(0x%@): \"%s\", aborting\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (*iterator).c_str ()),
                  clientHandle_,
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
    interface_identifiers.push_back (interfaceIdentifier_in);
  } // end lock scope

  iterator = interface_identifiers.begin ();
  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
next:
    if (unlikely (iterator == interface_identifiers.end ()))
      goto error; // no (more) WLAN interfaces --> failed

    // check cache first ?
    if (unlikely (interfaceIdentifier_in.empty ()))
    {
      iterator_2 = SSIDsToInterfaceIdentifier_.find (SSID_in);
      if (iterator_2 != SSIDsToInterfaceIdentifier_.end ())
      {
        iterator = std::find (interface_identifiers.begin (),
                              interface_identifiers.end (),
                              (*iterator_2).second.first);
        ACE_ASSERT (iterator != interface_identifiers.end ());
        interface_identifier = (*iterator_2).second.first;
        ap_mac_address = (*iterator_2).second.second;
        goto associate_to_access_point;
      } // end IF
      ++iterator;
      goto next;
    } // end IF
  } // end lock scope
  ACE_ASSERT (!interface_identifier.empty ());

associate_to_access_point:
  if (unlikely (!Net_WLAN_Tools::associate (interface_identifier,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                                            ap_mac_address,
#endif
                                            SSID_in,
                                            handle_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::associate(\"%s\",%s,%s), returning\n"),
                ACE_TEXT (interface_identifier.c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
    goto error;
  } // end IF

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
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
std::string
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::SSID () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::SSID"));

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
#else
//  ACE_ASSERT (handle_ != ACE_INVALID_HANDLE);
#endif
  ACE_ASSERT (configuration_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return Net_WLAN_Tools::associatedSSID (clientHandle_,
                                         configuration_->interfaceIdentifier);
#else
  return Net_WLAN_Tools::associatedSSID (configuration_->interfaceIdentifier,
                                         handle_);
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
Net_WLAN_SSIDs_t
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::SSIDs () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::SSIDs"));

  Net_WLAN_SSIDs_t result;

  // sanity check(s)
  ACE_ASSERT (configuration_);

  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, result);
    for (SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator = SSIDsToInterfaceIdentifier_.begin ();
         iterator != SSIDsToInterfaceIdentifier_.end ();
         ++iterator)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (InlineIsEqualGUID (configuration_->interfaceIdentifier,
                             (*iterator).second.first))
#else
      if (!ACE_OS::strcmp (configuration_->interfaceIdentifier.c_str (),
                           (*iterator).second.first.c_str ()))
#endif
        result.push_back ((*iterator).first);
    } // end FOR
  } // end lock scope

  return result;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::onChange (enum Net_WLAN_MonitorState newState_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onChange"));

  // set state early
  { ACE_GUARD (ACE_SYNCH_NULL_MUTEX, aGuard, *inherited2::stateLock_);
    inherited2::state_ = newState_in;
  } // end lock scope

  switch (newState_in)
  {
    case NET_WLAN_MONITOR_STATE_INITIAL:
    {
      // sanity check(s)
      ACE_ASSERT (configuration_);
      if (unlikely (!isRunning ()))
        break; // not start()ed yet, nothing to do

      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        if (unlikely (interfaceIdentifiers_.empty ()))
          interfaceIdentifiers_ = getDevices ();
      } // end lock scope

      // retrieve current AP [E]SSID{/BSSID (i.e. AP MAC address)} (if any)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (ACE_OS::strcmp (configuration_->SSID.c_str (),
                                    Net_WLAN_Tools::associatedSSID (clientHandle_,
                                                                    configuration_->interfaceIdentifier).c_str ())))
#else
      if (unlikely (ACE_OS::strcmp (configuration_->SSID.c_str (),
                                    Net_WLAN_Tools::associatedSSID (configuration_->interfaceIdentifier,
                                                                    handle_).c_str ())))
#endif
      {
        // check cache whether the ESSID is known
        SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
            SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
        // SSID not found ? --> initiate scan
        if (iterator == SSIDsToInterfaceIdentifier_.end ())
        {
          inherited2::change (NET_WLAN_MONITOR_STATE_SCAN);
          break;
        } // end IF

        inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);
        break;
      } // end IF

      inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
      break;
    }
    case NET_WLAN_MONITOR_STATE_SCAN:
    {
      // sanity check(s)
      ACE_ASSERT (configuration_);

      SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator;
      int result = -1;
      int error = 0;
      ACE_Time_Value poll_interval (0, 100000); // 100ms
      ACE_Message_Block* message_block_p = NULL;
      ACE_Time_Value now = COMMON_TIME_NOW;
      bool done = false;

      do
      {
        // SSID found ? --> associate to AP
        { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
          iterator =
              SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
          if (likely (iterator != SSIDsToInterfaceIdentifier_.end ()))
            break;
        } // end lock scope

        // scan and poll for results
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        Net_WLAN_Tools::scan (clientHandle_,
                              configuration_->interfaceIdentifier,
                              configuration_->SSID);
#else
        Net_WLAN_Tools::scan (configuration_->interfaceIdentifier,
                              configuration_->SSID,
                              handle_,
                              false); // don't wait
        ACE_OS::last_error (0);
        // *TODO*: implement nl80211 support, rather than polling
        result = handle_input (handle_);
        ACE_UNUSED_ARG (result);
        error = ACE_OS::last_error ();
        if (unlikely (error == EAGAIN)) //11: result data not available yet
        {
#endif
          result = ACE_OS::sleep (poll_interval);
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                        &poll_interval));

          // done ?
          result = inherited::getq (message_block_p, &now);
          if (likely (result == -1))
          {
            error = ACE_OS::last_error ();
            if (unlikely ((error != EAGAIN) &&
                          (error != ESHUTDOWN)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", returning\n")));
              return;
            } // end IF
            continue;
          } // end IF
          ACE_ASSERT (message_block_p);
          if (likely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
          {
            done = true;
            message_block_p->release ();
            break; // done
          } // end IF
          message_block_p->release ();
          message_block_p = NULL;
          continue;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
        } // end IF
#endif
      } while (true);
      if (unlikely (done))
        break;

      inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);

      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATE:
    {
      // sanity check(s)
      ACE_ASSERT (configuration_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _GUID interface_identifier = GUID_NULL;
#else
      std::string interface_identifier_string;
#endif
      struct ether_addr ap_mac_address, ether_addr_s;
      int result = -1;
      int error = 0;
      ACE_Time_Value poll_interval (0, 100000); // 100ms
      ACE_Message_Block* message_block_p = NULL;
      ACE_Time_Value now = COMMON_TIME_NOW;
      bool done = false;
      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
            SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
        ACE_ASSERT (iterator != SSIDsToInterfaceIdentifier_.end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        interface_identifier = (*iterator).second.first;
#else
        interface_identifier_string = (*iterator).second.first;
#endif
        ap_mac_address = (*iterator).second.second;
      } // end lock scope
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (InlineIsEqualGUID (interface_identifier,
                                       configuration_->interfaceIdentifier)))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("found SSID (was: %s) on interface \"%s\" (configured interface was: \"%s\"), continuing\n"),
                    ACE_TEXT (configuration_->SSID.c_str ()),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ())));
#else
      if (unlikely (ACE_OS::strcmp (interface_identifier_string.c_str (),
                                    configuration_->interfaceIdentifier.c_str ())))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("found SSID (was: %s) on interface \"%s\" (configured interface was: \"%s\"), continuing\n"),
                    ACE_TEXT (configuration_->SSID.c_str ()),
                    ACE_TEXT (interface_identifier_string.c_str ()),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (!associate (interface_identifier,
#else
      if (!associate (interface_identifier_string,
                      ap_mac_address,
#endif
                      configuration_->SSID))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_IMonitor_T::associate(\"%s\",%s), returning\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_IMonitor_T::associate(\"%s\",%s,%s), returning\n"),
                    ACE_TEXT (interface_identifier_string.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
#endif
        return;
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      // obtain an IP address via DHCP
      dhcpctl_handle connection_h = dhcpctl_null_handle;
      dhcpctl_handle authenticator_h = dhcpctl_null_handle;
      dhcpctl_handle interface_h = dhcpctl_null_handle;
      dhcpctl_status wait_status = -1;
      dhcpctl_data_string result_string;
      dhcpctl_status status_i =
          dhcpctl_connect (&connection_h,
                           ACE_TEXT_ALWAYS_CHAR ("192.168.0.1"),
                           7911,
                           authenticator_h);
      if (status_i != ISC_R_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_connect(%s:%u): \"%s\", returning\n"),
                    ACE_TEXT ("192.168.0.1"), 7911,
                    ACE_TEXT (isc_result_totext (status_i))));
        return;
      } // end IF
      ACE_ASSERT (connection_h != dhcpctl_null_handle);

      status_i = dhcpctl_new_object (&interface_h,
                                     connection_h,
                                     ACE_TEXT_ALWAYS_CHAR ("interface"));
      if (status_i != ISC_R_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_new_object(0x%@,%s): \"%s\", returning\n"),
                    connection_h,
                    ACE_TEXT ("interface"),
                    ACE_TEXT (isc_result_totext (status_i))));
        return;
      } // end IF
      ACE_ASSERT (interface_h != dhcpctl_null_handle);
      status_i =
          dhcpctl_set_string_value (interface_h,
                                    ACE_TEXT_ALWAYS_CHAR (interface_identifier_string.c_str ()),
                                    ACE_TEXT_ALWAYS_CHAR ("name"));
      if (status_i != ISC_R_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_set_string_value(0x%@,%s,\"%s\"): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (interface_identifier_string.c_str ()),
                    ACE_TEXT ("name"),
                    ACE_TEXT (isc_result_totext (status_i))));
        return;
      } // end IF
      status_i =
          dhcpctl_open_object (interface_h,
                               connection_h,
                               DHCPCTL_CREATE | DHCPCTL_EXCL);
      if (status_i != ISC_R_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_open_object(0x%@,0x%@,%d): \"%s\", returning\n"),
                    interface_h,
                    connection_h,
                    DHCPCTL_CREATE | DHCPCTL_EXCL,
                    ACE_TEXT (isc_result_totext (status_i))));
        return;
      } // end IF

      status_i = dhcpctl_wait_for_completion (interface_h,
                                              &wait_status);
      if (status_i != ISC_R_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_wait_for_completion(0x%@): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (isc_result_totext (status_i))));
        return;
      } // end IF
      if (wait_status != ISC_R_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_open_object(0x%@,create): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (isc_result_totext (wait_status))));
        return;
      } // end IF

      ACE_OS::memset (&result_string, 0, sizeof (dhcpctl_data_string));
      status_i = dhcpctl_get_value (&result_string,
                                    interface_h,
                                    ACE_TEXT_ALWAYS_CHAR ("state"));
      if (status_i != ISC_R_SUCCESS)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_get_value(0x%@,state): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (isc_result_totext (wait_status))));
        return;
      } // end IF

      dhcpctl_data_string_dereference (&result_string, MDL);
#endif

      // *TODO*: implement nl80211 support, rather than polling
      do
      {
        ether_addr_s =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          Net_WLAN_Tools::associatedBSSID (clientHandle_,
                                           interface_identifier);
#else
          Net_WLAN_Tools::associatedBSSID (interface_identifier_string,
                                           handle_);
#endif
        if (likely (!ACE_OS::memcmp (&ether_addr_s,
                                     &ap_mac_address,
                                     sizeof (struct ether_addr))))
          break;

        result = ACE_OS::sleep (poll_interval);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                      &poll_interval));

        // done ?
        result = inherited::getq (message_block_p, &now);
        if (likely (result == -1))
        {
          error = ACE_OS::last_error ();
          if (unlikely ((error != EAGAIN) &&
                        (error != ESHUTDOWN)))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", returning\n")));
            return;
          } // end IF
          continue;
        } // end IF
        ACE_ASSERT (message_block_p);
        if (likely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
        {
          done = true;
          message_block_p->release ();
          break; // done
        } // end IF
        message_block_p->release ();
        message_block_p = NULL;
      } while (true); // *TODO*: add association timeout
      if (unlikely (done))
        break;

      inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);

      break;
    }
    case NET_WLAN_MONITOR_STATE_CONNECT:
    {
      inherited2::change (NET_WLAN_MONITOR_STATE_CONNECTED);
      break;
    }
    case NET_WLAN_MONITOR_STATE_DISCONNECT:
    {
      break;
    }
    case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
    {
      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATED:
    {
      // sanity check(s)
      ACE_ASSERT (configuration_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
                                                 configuration_->interfaceIdentifier,
                                                 wlan_intf_opcode_background_scan_enabled,
                                                 false))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,true), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    clientHandle_, wlan_intf_opcode_background_scan_enabled));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("\"%s\": disabled background scans\n"),
      //              ACE_TEXT (Net_Common_Tools::interfaceToString (clientHandle_, interfaceIdentifier_in).c_str ())));
      if (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
                                                 configuration_->interfaceIdentifier,
                                                 wlan_intf_opcode_media_streaming_mode,
                                                 true))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,true), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    clientHandle_, wlan_intf_opcode_media_streaming_mode));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("\"%s\": enabled streaming mode\n"),
      //              ACE_TEXT (Net_Common_Tools::interfaceToString (clientHandle_, interfaceIdentifier_in).c_str ())));
#endif

      try {
        onAssociate (configuration_->interfaceIdentifier,
                     configuration_->SSID,
                     true);
      } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
#endif
      }

      inherited2::change (NET_WLAN_MONITOR_STATE_CONNECT);

      break;
    }
    case NET_WLAN_MONITOR_STATE_CONNECTED:
    {
      try {
        onConnect (configuration_->interfaceIdentifier,
                   configuration_->SSID,
                   true);
      } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
#endif
      }

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid state switch: \"%s\" --> \"%s\" --> check implementation !, returning\n"),
                  ACE_TEXT (stateToString (inherited2::state_).c_str ()),
                  ACE_TEXT (stateToString (newState_in).c_str ())));
      break;
    }
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::onAssociate (REFGUID interfaceIdentifier_in,
#else
                   UserDataType>::onAssociate (const std::string& interfaceIdentifier_in,
#endif
                                               const std::string& SSID_in,
                                               bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onAssociate"));

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
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  // sanity check(s)
  ACE_ASSERT (configuration_);
  if (!success_in)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to associate to SSID %s, retrying...\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to associate to SSID %s, retrying...\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#endif
    return;
  } // end IF

  // --> associated successfully
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
//                                                       interfaceIdentifier_in,
//                                                       wlan_intf_opcode_background_scan_enabled,
//                                                       !success_in)))
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,true), continuing\n"),
//                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
//                clientHandle_, wlan_intf_opcode_background_scan_enabled));
//  //else
//  //  ACE_DEBUG ((LM_DEBUG,
//  //              ACE_TEXT ("\"%s\": %s background scans\n"),
//  //              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
//  //              (success_in ? ACE_TEXT ("disabled") : ACE_TEXT ("enabled"))));
//  if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
//                                                       interfaceIdentifier_in,
//                                                       wlan_intf_opcode_media_streaming_mode,
//                                                       success_in)))
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,true), continuing\n"),
//                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
//                clientHandle_, wlan_intf_opcode_media_streaming_mode));
//  //else
//  //  ACE_DEBUG ((LM_DEBUG,
//  //              ACE_TEXT ("\"%s\": %s streaming mode\n"),
//  //              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
//  //              (success_in ? ACE_TEXT ("enabled") : ACE_TEXT ("disabled"))));
//#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": associated to SSID %s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": associated to SSID %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#endif
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::onConnect (REFGUID interfaceIdentifier_in,
#else
                   UserDataType>::onConnect (const std::string& interfaceIdentifier_in,
#endif
                                             const std::string& SSID_in,
                                             bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onConnect"));

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
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  // sanity check(s)
  ACE_ASSERT (configuration_);
  if (!success_in)
    return;

  // --> connected successfully
  // *NOTE*: the (internet-)address type isn't specified at this level of
  //         abstraction --> resolution must be effected by super-classes

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

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("\"%s\": connected to SSID %s: %s <---> %s\n"),
//              //ACE_TEXT (Net_Common_Tools::WLANInterfaceToString (clientHandle_, interfaceIdentifier_in).c_str ()),
//              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
//              ACE_TEXT (SSID_in.c_str ()),
//              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_).c_str ()),
//              ACE_TEXT (Net_Common_Tools::IPAddressToString (peerSAP_).c_str ())));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("\"%s\": connected to SSID %s: %s <---> %s\n"),
//              ACE_TEXT (interfaceIdentifier_in.c_str ()),
//              ACE_TEXT (SSID_in.c_str ()),
//              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_).c_str ()),
//              ACE_TEXT (Net_Common_Tools::IPAddressToString (peerSAP_).c_str ())));
//#endif
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::onHotPlug (REFGUID interfaceIdentifier_in,
#else
                   UserDataType>::onHotPlug (const std::string& interfaceIdentifier_in,
#endif
                                             bool enabled_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onHotPlug"));

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
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onHotPlug(), continuing\n")));
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
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::onScanComplete (REFGUID interfaceIdentifier_in)
#else
                   UserDataType>::onScanComplete (const std::string& interfaceIdentifier_in)
#endif
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onScanComplete"));

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
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n")));
      }
    } // end FOR
  } // end lock scope
}

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
int
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::svc (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::svc"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): monitor (group: %d, thread id: %t) starting...\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

//  ACE_Asynch_Pseudo_Task* task_p = NULL;
//  ACE_Reactor* reactor_p = inherited::reactor ();
//  if (likely (!configuration_->useReactor))
//  {
//    ACE_Proactor* proactor_p = ACE_Proactor::instance ();
//    ACE_ASSERT (proactor_p);
//    ACE_Proactor_Impl* proactor_impl_p = proactor_p->implementation ();
//    ACE_ASSERT (proactor_impl_p);
//    ACE_POSIX_Proactor* posix_proactor_p =
//        dynamic_cast <ACE_POSIX_Proactor*> (proactor_impl_p);
//    ACE_ASSERT (posix_proactor_p);
//    task_p =
//        &posix_proactor_p->get_asynch_pseudo_task ();
//  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (!configuration_->SSID.empty ());
  ACE_ASSERT (handle_ != ACE_INVALID_HANDLE);
//  ACE_ASSERT (!isRegistered_);
//  ACE_ASSERT (reactor_p);

  int result = -1;

//  // register handle with the reactor for notification
//  if (likely (!configuration_->useReactor))
//    result = task_p->register_io_handler (handle_,
//                                          this,
//                                          ACE_Event_Handler::READ_MASK,
//                                          0);
//  else
//    result = reactor_p->register_handler (this,
//                                          ACE_Event_Handler::READ_MASK);
//  if (unlikely (result == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Reactor::register_handler(%d,ACE_Event_Handler::READ_MASK): \"%m\", aborting\n"),
//                handle_));
//    return -1;
//  } // end IF
//  isRegistered_ = true;

//  do
//  {
  change (NET_WLAN_MONITOR_STATE_INITIAL);
//  } while (true);

//  // clean up
//  if (likely (!configuration_->useReactor))
//    result = task_p->remove_io_handler (handle_);
//  else
//    result = reactor_p->remove_handler (this,
//                                        ACE_Event_Handler::READ_MASK);
//  if (unlikely (result == -1))
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d,ACE_Event_Handler::READ_MASK): \"%m\", aborting\n"),
//                handle_));
//  isRegistered_ = false;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): monitor (group: %d, thread id: %t) leaving...\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
int
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::handle_input"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (handle_in == handle_);

  int result = -1;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   configuration_->interfaceIdentifier.c_str (),
                   IFNAMSIZ);
  struct stream_descr stream_descr_s;
  ACE_OS::memset (&stream_descr_s, 0, sizeof (struct stream_descr));
  struct iw_event iw_event_s;
  ACE_OS::memset (&iw_event_s, 0, sizeof (struct iw_event));
  int error = 0;
  SSIDS_TO_INTERFACEIDENTIFIER_MAP_ITERATOR_T iterator;
  struct ether_addr ap_mac_address;
  ACE_OS::memset (&ap_mac_address, 0, sizeof (struct ether_addr));
  std::string essid_string;
  ACE_TCHAR buffer_a[BUFSIZ];

  if (!buffer_)
  { ACE_ASSERT (!bufferSize_);
    bufferSize_ = IW_SCAN_MAX_DATA;
    buffer_ = ACE_OS::malloc (bufferSize_);
    if (unlikely (!buffer_))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory (%u byte(s)): \"%m\", continuing\n"),
                  bufferSize_));
      goto continue_;
    } // end IF
  } // end IF
  iwreq_s.u.data.pointer = buffer_;
  iwreq_s.u.data.length = bufferSize_;

fetch_scan_result_data:
  result = ACE_OS::ioctl (handle_in,
                          SIOCGIWSCAN,
                          &iwreq_s);
  if (unlikely (result < 0))
  {
    error = ACE_OS::last_error ();
    if ((error == E2BIG) && // 7
        (range_.we_version_compiled > 16))
    { // buffer too small --> retry
      if (iwreq_s.u.data.length > bufferSize_)
        bufferSize_ = iwreq_s.u.data.length;
      else
        bufferSize_ *= 2; // grow dynamically
      goto retry;
    } // end IF

    if (unlikely (error == EAGAIN)) // 11
    { // result(s) not available yet
      goto continue_;
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCGIWSCAN): \"%m\", continuing\n"),
                handle_in));
    goto continue_;
  } // end IF

  // the driver may have reported the required buffer size
  if (unlikely (iwreq_s.u.data.length > bufferSize_))
  { // --> grow the buffer and retrys
    bufferSize_ = iwreq_s.u.data.length;
retry:
    buffer_ = ACE_OS::realloc (buffer_, bufferSize_);
    if (!buffer_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory (%u byte(s)): \"%m\", continuing\n"),
                  bufferSize_));
      goto continue_;
    } // end IF
    iwreq_s.u.data.pointer = buffer_;
    iwreq_s.u.data.length = bufferSize_;
    goto fetch_scan_result_data;
  } // end IF
  ACE_ASSERT (iwreq_s.u.data.length && (iwreq_s.u.data.length <= bufferSize_));

  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, 0);
    // clear cache
    do
    {
      iterator =
          std::find_if (SSIDsToInterfaceIdentifier_.begin (), SSIDsToInterfaceIdentifier_.end (),
                        std::bind2nd (SSIDS_TO_INTERFACEIDENTIFIER_FIND_S (),
                                      configuration_->interfaceIdentifier));
      if (iterator == SSIDsToInterfaceIdentifier_.end ())
        break;
      SSIDsToInterfaceIdentifier_.erase ((*iterator).first);
    } while (true);
  } // end lock scope

  // process the result data
  iw_init_event_stream (&stream_descr_s,
                        static_cast<char*> (buffer_),
                        static_cast<int> (iwreq_s.u.data.length));
  do
  {
    result = iw_extract_event_stream (&stream_descr_s,
                                      &iw_event_s,
                                      range_.we_version_compiled);
    if (result <= 0)
      break; // done

    switch (iw_event_s.cmd)
    {
      case SIOCGIWAP:
      {
        ACE_OS::memcpy (&ap_mac_address,
                        reinterpret_cast<void*> (iw_event_s.u.ap_addr.sa_data),
                        sizeof (struct ether_addr));
        break;
      }
      case SIOCGIWNWID:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        if (!iw_event_s.u.nwid.disabled)
          ACE_OS::sprintf (buffer_a,
                           ACE_TEXT_ALWAYS_CHAR ("NWID: %X"),
                           iw_event_s.u.nwid.value);
#endif
        break;
      }
      case SIOCGIWFREQ:
      {
        double frequency_d = iw_freq2float (&iw_event_s.u.freq);
        int channel =iw_freq_to_channel (frequency_d, &range_);
        ACE_UNUSED_ARG (channel);
        break;
      }
      case SIOCGIWMODE:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        ACE_OS::sprintf (buffer_a,
                         ACE_TEXT_ALWAYS_CHAR ("Mode: %s"),
                         iw_operation_mode[iw_event_s.u.mode]);
#endif
        break;
      }
      case SIOCGIWNAME:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        ACE_OS::sprintf (buffer_a,
                         ACE_TEXT_ALWAYS_CHAR ("Protocol: %-1.16s"),
                         iw_event_s.u.name);
#endif
        break;
      }
      case SIOCGIWESSID:
      {
        ACE_ASSERT (iw_event_s.u.essid.length && (iw_event_s.u.essid.length <= IW_ESSID_MAX_SIZE));
        ACE_ASSERT (iw_event_s.u.essid.pointer);
        essid_string.assign (reinterpret_cast<char*> (iw_event_s.u.essid.pointer),
                             iw_event_s.u.essid.length);
        if (iw_event_s.u.essid.flags)
        {
          /* Does it have an ESSID index ? */
          if ((iw_event_s.u.essid.flags & IW_ENCODE_INDEX) > 1);
//            printf("                    ESSID:\"%s\" [%d]\n", essid,
//                   (iw_event_s.u.essid.flags & IW_ENCODE_INDEX));
//          else
//            printf("                    ESSID:\"%s\"\n", essid);
        } // end IF
//        else
//          printf("                    ESSID:off/any/hidden\n");
        { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, 0);
          SSIDsToInterfaceIdentifier_.insert (std::make_pair (essid_string,
                                                              std::make_pair (configuration_->interfaceIdentifier,
                                                                              ap_mac_address)));
        } // end lock scope
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("\"%s\": found wireless access point (MAC address: %s, ESSID: %s)...\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address),
                                                                          NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT (essid_string.c_str ())));
#endif
        break;
      }
      case SIOCGIWENCODE:
      {
        ACE_TCHAR buffer_2[IW_ENCODING_TOKEN_MAX];
        ACE_OS::memset (buffer_2, 0, sizeof (ACE_TCHAR[IW_ENCODING_TOKEN_MAX]));
        if (iw_event_s.u.data.pointer)
        { ACE_ASSERT (iw_event_s.u.data.length <= IW_ENCODING_TOKEN_MAX);
          ACE_OS::memcpy (buffer_2,
                          iw_event_s.u.data.pointer,
                          iw_event_s.u.data.length);
//        else
//          iw_event_s.u.data.flags |= IW_ENCODE_NOKEY;
        } // end IF
        if (iw_event_s.u.data.flags & IW_ENCODE_DISABLED);
//          printf ("off\n");
        else
        {
#if defined (_DEBUG)
          iw_print_key (buffer_a,
                        sizeof (ACE_TCHAR[BUFSIZ]),
                        reinterpret_cast<unsigned char*> (iw_event_s.u.data.pointer),
                        iw_event_s.u.data.length,
                        iw_event_s.u.data.flags);
#endif
          /* Other info... */
          if ((iw_event_s.u.data.flags & IW_ENCODE_INDEX) > 1);
//            printf (" [%d]", iw_event_s.u.data.flags & IW_ENCODE_INDEX);
          if (iw_event_s.u.data.flags & IW_ENCODE_RESTRICTED);
//            printf ("   Security mode:restricted");
          if (iw_event_s.u.data.flags & IW_ENCODE_OPEN);
//            printf ("   Security mode:open");
        } // end ELSE
        break;
      }
      case SIOCGIWRATE:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        iw_print_bitrate (buffer_a,
                          sizeof (ACE_TCHAR[BUFSIZ]),
                          iw_event_s.u.bitrate.value);
#endif
        break;
      }
      case SIOCGIWMODUL:
      {
        for (int i = 0;
             i < IW_SIZE_MODUL_LIST;
             ++i)
        {
          if ((iw_event_s.u.param.value & iw_modul_list[i].mask) == iw_modul_list[i].mask);
#if defined (_DEBUG)
          ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
          ACE_OS::sprintf (buffer_a,
                           ACE_TEXT_ALWAYS_CHAR ("Modulation: %s"),
                           iw_modul_list[i].cmd);
#endif
        } // end FOR
        break;
      }
      case IWEVQUAL:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        iw_print_stats (buffer_a, sizeof (ACE_TCHAR[BUFSIZ]),
                        &iw_event_s.u.qual,
                        &range_, 1);
#endif
        break;
      }
#ifndef WE_ESSENTIAL
      case IWEVGENIE:
      {
#if defined (_DEBUG)
//        iw_print_gen_ie (iw_event_s.u.data.pointer,
//                         iw_event_s.u.data.length);
#endif
        break;
      }
#endif /* WE_ESSENTIAL */
      case IWEVCUSTOM:
      {
#if defined (_DEBUG)
        ACE_TCHAR buffer_2[IW_CUSTOM_MAX + 1];
        ACE_OS::memset (buffer_2, 0, sizeof (ACE_TCHAR[IW_CUSTOM_MAX + 1]));
        ACE_ASSERT (iw_event_s.u.data.length <= IW_CUSTOM_MAX);
        if (iw_event_s.u.data.pointer && iw_event_s.u.data.length)
          ACE_OS::memcpy (buffer_2,
                          iw_event_s.u.data.pointer,
                          iw_event_s.u.data.length);
//        buffer_2[iw_event_s.u.data.length] = '\0';
#endif
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown WE event (was: %d), continuing\n"),
                    iw_event_s.cmd));
        break;
      }
    } // end SWITCH
  } while (true);

  try {
    onScanComplete (configuration_->interfaceIdentifier);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n")));
  }

continue_:
  // *NOTE*: do not deregister from the callback
  return 0;
}
#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
std::vector<struct _GUID>
#else
std::vector<std::string>
#endif
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::getDevices () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::getDevices"));

  INTERFACEIDENTIFIERS_T result;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);

  PWLAN_INTERFACE_INFO_LIST interface_list_p = NULL;
  DWORD result_2 = WlanEnumInterfaces (clientHandle_,
                                       NULL,
                                       &interface_list_p);
  if (unlikely (result_2 != ERROR_SUCCESS))
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
  if (unlikely (result_2 == -1))
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
    if (!Net_WLAN_Tools::interfaceIsWLAN (ifaddrs_2->ifa_name))
      continue;

    result.push_back (ifaddrs_2->ifa_name);
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

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DBUS_SUPPORT)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_THREAD_NAME), // thread name
              NET_WLAN_MONITOR_THREAD_GROUP_ID,                    // group id
              2,                                                  // # thread(s)
              false,                                              // auto-start ?
              //NULL)                                             // queue handle
              // *TODO*: this looks dodgy, but seems to work nonetheless...
              &queue_)                                            // queue handle
 , connection_ (NULL)
 , identifierToObjectPath_ ()
 // , proxy_ (NULL)
 , configuration_ (NULL)
 , isActive_ (false)
 , isInitialized_ (false)
 , localSAP_ ()
 , peerSAP_ ()
 , subscribersLock_ ()
 , subscribers_ ()
 , userData_ (NULL)
 /////////////////////////////////////////
 , DBusDispatchStarted_ (false)
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::~Net_WLAN_Monitor_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::~Net_WLAN_Monitor_T"));

  if (unlikely (isActive_))
  { ACE_ASSERT (connection_);
    dbus_connection_close (connection_);
    inherited::stop (true,
                     true);
  } // end IF
  if (unlikely (connection_))
    dbus_connection_unref (connection_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::start"));

  // sanity check(s)
  if (unlikely (!isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF
  if (unlikely (isActive_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("already started, returning\n")));
    return;
  } // end IF
  ACE_ASSERT (configuration_);

  // sanity check(s)
  ACE_ASSERT (!connection_);
//  ACE_ASSERT (!proxy_);

  struct DBusError error_s;
  dbus_error_init (&error_s);
  std::string device_path_string;
  std::string match_rule_string =
      ACE_TEXT_ALWAYS_CHAR ("type='signal',sender='");
  match_rule_string +=
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DBUS_NETWORKMANAGER_SERVICE);
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
        Net_WLAN_Tools::deviceToDBusPath (connection_,
                                          (*iterator).first);
    if (unlikely ((*iterator).second.empty ()))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceToDBusPath(\"%s\"), continuing\n"),
                  ACE_TEXT ((*iterator).first).c_str ()));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": DBus object path is: \"%s\"\n"),
                  ACE_TEXT ((*iterator).first.c_str ()),
                  ACE_TEXT ((*iterator).second.c_str ())));
  } // end FOR

  // subscribe to all networkmanager signals
  if (unlikely (!dbus_connection_add_filter (connection_,
                                             configuration_->notificationCB,
                                             configuration_->notificationCBData,
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
  isActive_ = true;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  if (!isActive_)
    return;

  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);
//  deviceDBusPath_.resize (0);

  isActive_ = false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
const std::string&
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  NET_WLAN_MONITOR_API_DBUS,
                  UserDataType>::get1R (const std::string& value_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::get1R"));

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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  AddressType,
                  ConfigurationType,
                  NET_WLAN_MONITOR_API_DBUS,
                  UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (isInitialized_))
  {
    if (isActive_)
      stop (true,
            true);

    ACE_ASSERT (!connection_);
    identifierToObjectPath_.clear ();

    localSAP_.reset ();
    peerSAP_.reset ();

    DBusDispatchStarted_ = false;

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  // *TODO*: remove type inference
  userData_ = configuration_->userData;

  // sanity check(s)
  if (!configuration_->notificationCB)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("D-Bus signal callback not specified, using default implementation\n")));
    configuration_->notificationCB = network_wlan_dbus_default_filter_cb;
    configuration_->notificationCBData =
        static_cast<Net_WLAN_IMonitorBase*> (this);
  } // end IF
  else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("D-Bus signal callback specified, disabled event subscription\n")));
  if (unlikely (configuration_->SSID.empty ()))
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
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::subscribe (Net_WLAN_IMonitorCB* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::subscribe"));

  // sanity check(s)
  if (unlikely (!interfaceHandle_in))
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
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::unsubscribe (Net_WLAN_IMonitorCB* interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::unsubscribe"));

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
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::associate (const std::string& interfaceIdentifier_in,
                                             const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::associate"));

  // sanity check(s)
  if (unlikely (SSID_in.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return false;
  } // end IF
  if (unlikely (!isActive_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not running, aborting\n")));
    return false;
  } // end IF
  if (!interfaceIdentifier_in.empty ())
  {
    if (SSID () == SSID_in)
      return true; // already associated, nothing to do
  } // end IF

  INTERFACEIDENTIFIERS_T devices;
  INTERFACEIDENTIFIERS_ITERATOR_T iterator;
  if (interfaceIdentifier_in.empty ())
    devices = getDevices ();
  else
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
      Net_WLAN_Tools::SSIDToDeviceDBusPath (connection_,
                                            SSID_in);
  if (unlikely (device_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToDeviceDBusPath(0x%@,%s), continuing\n"),
                connection_,
                ACE_TEXT (SSID_in.c_str ())));
    ++iterator;
    goto next;
  } // end IF

retrieve_access_point:
  access_point_object_path_string =
      Net_WLAN_Tools::SSIDToAccessPointDBusPath (connection_,
                                                 device_object_path_string,
                                                 SSID_in);
  if (unlikely (access_point_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToAccessPointDBusPath(0x%@,\"%s\",%s), continuing\n"),
                connection_,
                ACE_TEXT (device_object_path_string.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
    ++iterator;
    goto next;
  } // end IF
  connection_object_path_string =
      Net_WLAN_Tools::SSIDToConnectionDBusPath (connection_,
                                                device_object_path_string,
                                                SSID_in);
  if (unlikely (connection_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToConnectionDBusPath(0x%@,\"%s\",%s), continuing\n"),
                connection_,
                ACE_TEXT (device_object_path_string.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
    ++iterator;
    goto next;
  } // end IF
  if (unlikely (!Net_WLAN_Tools::activateConnection (connection_,
                                                     connection_object_path_string,
                                                     device_object_path_string,
                                                     access_point_object_path_string)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::activateConnection(0x%@,\"%s\",\"%s\",\"%s\"), continuing\n"),
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

continue_:
  return true;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::onAssociate (const std::string& interfaceIdentifier_in,
                                               const std::string& SSID_in,
                                               bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onAssociate"));

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
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  // sanity check(s)
  if (!success_in)
  { // *TODO*: this also happens when the device has been disconnected
    //         - manually (i.e. by the user)
    //         - while it is switching to a different SSID (auto-connect)
    //         - ...
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to associate with SSID %s, retrying...\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));

    if (unlikely (!associate (interfaceIdentifier_in,
                              configuration_->SSID)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), returning\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ()),
                  ACE_TEXT (configuration_->SSID.c_str ())));
    return;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": associated with SSID %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::onConnect (const std::string& interfaceIdentifier_in,
                                             const std::string& SSID_in,
                                             bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onConnect"));

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
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  //// sanity check(s)
  //if (!success_in)
  //  return;

//  if (!Net_Common_Tools::interfaceToIPAddress (interfaceIdentifier_in,
//                                               localSAP_,
//                                               peerSAP_))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
//                ACE_TEXT (interfaceIdentifier_in.c_str ())));
//    return;
//  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": connected to SSID %s: %s <---> %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (SSID_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peerSAP_).c_str ())));
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::onHotPlug (const std::string& interfaceIdentifier_in,
                                             bool enabled_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onHotPlug"));

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
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onHotPlug(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": interface %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              (enabled_in ? ACE_TEXT ("enabled") : ACE_TEXT ("disabled/removed"))));
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::onScanComplete (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onScanComplete"));

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
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  // sanity check(s)
  if (unlikely (!isInitialized_ || !isActive_))
    return;
  ACE_ASSERT (configuration_);
  if ((SSID () == configuration_->SSID) ||
      !configuration_->autoAssociate)
    return;

  if (unlikely (!associate (interfaceIdentifier_in,
                            configuration_->SSID)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), returning\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
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
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::svc (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::svc"));

  DBusDispatchStatus dispatch_status = DBUS_DISPATCH_COMPLETE;

  { ACE_GUARD_RETURN (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_, -1);
    if (DBusDispatchStarted_)
      goto monitor_thread;
    DBusDispatchStarted_ = true;
  } // end lock scope

//dbus_dispatch_thread:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): D-Bus dispatch (group: %d, thread id: %t) starting...\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

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
    DBusDispatchStarted_ = false;
  } // end lock scope

  return 0;

monitor_thread:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): monitor (group: %d, thread id: %t) starting...\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (!configuration_->SSID.empty ());

  int result = -1;
  ACE_Time_Value interval (1, 0);
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value now = COMMON_TIME_NOW;
  do
  {
    if (configuration_->SSID != Net_WLAN_Tools::associatedSSID (configuration_->interfaceIdentifier))
    {
      // SSID not found ? --> initiate scan
      if (unlikely (!Net_WLAN_Tools::hasSSID (configuration_->interfaceIdentifier,
                                              configuration_->SSID)))
      {
        Net_WLAN_Tools::scan (configuration_->interfaceIdentifier);
        goto sleep;
      } // end IF

      if (unlikely (!associate (configuration_->interfaceIdentifier,
                                configuration_->SSID)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_IWLANMonitor_T::associate(\"%s\",%s), retrying in %#T...\n"),
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
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_DBUS,
                   UserDataType>::getDevices () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::getDevices"));

  INTERFACEIDENTIFIERS_T result;

#if defined (ACE_HAS_GETIFADDRS)
  struct ifaddrs* ifaddrs_p = NULL;
  int result_2 = ::getifaddrs (&ifaddrs_p);
  if (unlikely (result_2 == -1))
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
    if (!Net_WLAN_Tools::interfaceIsWLAN (ifaddrs_2->ifa_name))
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

  return result;
}
#endif /* DBUS_SUPPORT */
#endif /* ACE_WIN32 || ACE_WIN64 */
