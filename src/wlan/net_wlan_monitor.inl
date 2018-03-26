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
#endif
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                   ACE_SYNCH_USE,
                   TimePolicyType,
#endif
                   MonitorAPI_e,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited ()
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

}

//template <typename AddressType,
//          typename ConfigurationType,
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//          ACE_SYNCH_DECL,
//          typename TimePolicyType,
//#endif
//          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
//          typename UserDataType>
//bool
//Net_WLAN_Monitor_T<AddressType,
//                   ConfigurationType,
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//                   ACE_SYNCH_USE,
//                   TimePolicyType,
//#endif
//                   MonitorAPI_e,
//                   UserDataType>::initialize (const ConfigurationType& configuration_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

//  if (unlikely (inherited::isInitialized_))
//  {
//    userData_ = NULL;
//  } // end IF

//  // *TODO*: remove type inference
//  userData_ = configuration_in.userData;

//  return inherited::initialize (configuration_in);
//}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename AddressType,
//          typename ConfigurationType,
//          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
//          typename UserDataType>
//void
//Net_WLAN_Monitor_T<ACE_SYNCH_USE,
//                   TimePolicyType,
//                   AddressType,
//                   ConfigurationType,
//                   MonitorAPI_e,
//                   UserDataType>::onChange (enum Net_WLAN_MonitorState newState_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onChange"));

//  switch (newState_in)
//  {
//    case NET_WLAN_MONITOR_STATE_INVALID:
//    {
//      ACE_ASSERT (false);
//      ACE_NOTSUP;
//      ACE_NOTREACHED (break;)
//    }
//    case NET_WLAN_MONITOR_STATE_IDLE:
//    {
//      // *NOTE*: actually 'set' intermediate states to support atomic state
//      //         transition notifications
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_IDLE;
//      } // end lock scope

//      if (!isActive_ ||              // stop()ped
//          inherited::hasShutDown ())
//      {
//        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_INITIALIZED);
//        break;
//      } // end IF
//      std::string SSID_string = SSID ();
//      if (!SSID_string.empty ()) // connected
//      {
//        if (!isFirstConnect_)
//          goto continue_;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//        // sanity check(s)
//        ACE_ASSERT (configuration_);

//        try {
//          onAssociate (configuration_->interfaceIdentifier,
//                       SSID_string,
//                       true);
//        } catch (...) {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(\"%s\",%s,true), continuing\n"),
//                      ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
//                      ACE_TEXT (SSID_string.c_str ())));
//        }
//#else
//        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
//        break;

//continue_:
//        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCAN);
//#endif // ACE_WIN32 || ACE_WIN64
//        break;
//      } // end IF

//      // not connected

//      // sanity check(s)
//      ACE_ASSERT (configuration_);

//      bool essid_is_cached = false;
//      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
//        // check cache whether the configured ESSID (if any) is known
//        Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t iterator =
//            SSIDCache_.find (configuration_->SSID);
//        essid_is_cached = (iterator != SSIDCache_.end ());
//        if (unlikely (!SSIDSeenBefore_ && essid_is_cached))
//          SSIDSeenBefore_ = true;
//      } // end lock scope

//      // *NOTE*: there is no need to reset the state when returning from a
//      //         transitional state, as the current state is not set until this
//      //         method returns. Note that this holds only iff the state hasn't
//      //         already changed; see: common_state_machine_base.inl:214
//      //         --> reset the state just in case

//      if (configuration_->SSID.empty () || // not configured
//          !essid_is_cached              || // configured SSID unknown (i.e. not cached yet)
//          !configuration_->autoAssociate)  // auto-associate disabled
//      {
//        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCAN);
//        goto reset_state;
//      } // end IF

//      ACE_ASSERT (SSID_string.empty () && !configuration_->SSID.empty () && essid_is_cached && configuration_->autoAssociate);

//      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);

//reset_state:
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_IDLE;
//      } // end lock scope

//      break;
//    }
//    case NET_WLAN_MONITOR_STATE_SCAN:
//    {
//      // *NOTE*: actually 'set' intermediate states to support atomic state
//      //         transition notifications
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_SCAN;
//      } // end lock scope

//      // sanity check(s)
//      ACE_ASSERT (configuration_);

//      // scan (and wait/poll for results)
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#if defined (_DEBUG)
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("\"%s\": [%T] scanning...\n"),
//                  ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ())));
//#endif // _DEBUG
//      Net_WLAN_Tools::scan (clientinherited::handle_,
//                            configuration_->interfaceIdentifier,
//                            (SSIDSeenBefore_ ? configuration_->SSID
//                                             : ACE_TEXT_ALWAYS_CHAR ("")));
//#else
//#if defined (_DEBUG)
//      ACE_Time_Value scan_time;
//      std::string scan_time_string;
//      ACE_High_Res_Timer timer;
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("\"%s\": scanning...\n"),
//                  ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
//      timer.start ();
//#endif // _DEBUG
//      Net_WLAN_Tools::scan (configuration_->interfaceIdentifier,
//                            (SSIDSeenBefore_ ? configuration_->SSID
//                                             : ACE_TEXT_ALWAYS_CHAR ("")),
//                            inherited::handle_,
//                            false); // don't wait

//      int result = -1;
//      int error = 0;
//      ACE_Time_Value result_poll_interval (0,
//                                           NET_WLAN_MONITOR_SCAN_DEFAULT_RESULT_POLL_INTERVAL * 1000);
//fetch_scan_result_data:
//      ACE_OS::last_error (0);
//      // *TODO*: implement nl80211 support, rather than polling
//      result = inherited::handle_input (inherited::handle_);
//      ACE_UNUSED_ARG (result);
//      error = ACE_OS::last_error ();
//      if (error == EAGAIN) // 11: result data not available yet
//      {
//        result = ACE_OS::sleep (result_poll_interval);
//        if (unlikely (result == -1))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
//                      &result_poll_interval));
//        if (inherited::hasShutDown ())
//          break;
//        goto fetch_scan_result_data;
//      } // end IF
//#if defined (_DEBUG)
//      timer.stop ();
//      timer.elapsed_time (scan_time);
//      Common_Timer_Tools::periodToString (scan_time,
//                                          scan_time_string);
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("\"%s\": scanning (h:m:s.us) %s...DONE\n"),
//                  ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
//                  ACE_TEXT (scan_time_string.c_str ())));
//#endif // _DEBUG

//      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCANNED);
//#endif // ACE_WIN32 || ACE_WIN64

//      break;
//    }
//    case NET_WLAN_MONITOR_STATE_ASSOCIATE:
//    {
//      // *NOTE*: actually 'set' intermediate states to support atomic state
//      //         transition notifications
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATE;
//      } // end lock scope

//      // sanity check(s)
//      ACE_ASSERT (configuration_);
//      ACE_ASSERT (!configuration_->SSID.empty ());
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      struct _GUID interface_identifier = GUID_NULL;
//#else
//      std::string interface_identifier_string;
//      struct ether_addr ap_mac_address_s, ether_addr_s;
//      bool result = false;
//      int result_2 = -1;
//      ACE_Time_Value result_poll_interval (0,
//                                           NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL * 1000);
//      ACE_Time_Value result_timeout (NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT,
//                                     0);
//      ACE_Time_Value deadline;
//      bool shutdown = false;
//#endif // ACE_WIN32 || ACE_WIN64
//      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
//        // check cache whether the configured ESSID (if any) is known
//        Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t iterator =
//          SSIDCache_.find (configuration_->SSID);
//        ACE_ASSERT (iterator != SSIDCache_.end ());
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//        interface_identifier = (*iterator).second.first;
//#else
//        interface_identifier_string = (*iterator).second.first;
//        ap_mac_address_s = (*iterator).second.second.accessPointLinkLayerAddress;
//#endif // ACE_WIN32 || ACE_WIN64
//      } // end lock scope
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (unlikely (!InlineIsEqualGUID (interface_identifier,
//                                        configuration_->interfaceIdentifier)))
//        ACE_DEBUG ((LM_WARNING,
//                    ACE_TEXT ("found SSID (was: %s) on interface \"%s\" (configured interface was: \"%s\"), continuing\n"),
//                    ACE_TEXT (configuration_->SSID.c_str ()),
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ())));
//#else
//      if (unlikely (ACE_OS::strcmp (interface_identifier_string.c_str (),
//                                    configuration_->interfaceIdentifier.c_str ())))
//        ACE_DEBUG ((LM_WARNING,
//                    ACE_TEXT ("found SSID (was: %s) on interface \"%s\" (configured interface was: \"%s\"), continuing\n"),
//                    ACE_TEXT (configuration_->SSID.c_str ()),
//                    ACE_TEXT (interface_identifier_string.c_str ()),
//                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
//#endif // ACE_WIN32 || ACE_WIN64

//      std::string SSID_string = SSID ();
//      if (!SSID_string.empty () &&
//          ACE_OS::strcmp (SSID_string.c_str (),
//                          configuration_->SSID.c_str ()))
//      {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//        if (unlikely (!Net_WLAN_Tools::disassociate (clientinherited::handle_,
//                                                     interface_identifier)))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to Net_WLAN_Tools::disassociate(0x%@,\"%s\"), continuing\n"),
//                      clientinherited::handle_,
//                      ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
//#else
//        if (unlikely (!Net_WLAN_Tools::disassociate (interface_identifier_string,
//                                                     inherited::handle_)))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to Net_WLAN_Tools::disassociate(\"%s\",%d), continuing\n"),
//                      ACE_TEXT (interface_identifier_string.c_str ()),
//                      inherited::handle_));
//#endif // ACE_WIN32 || ACE_WIN64
//      } // end IF

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (unlikely (!Net_WLAN_Tools::associate (clientinherited::handle_,
//                                                interface_identifier,
//                                                configuration_->SSID)))
//#else
//associate:
//      if (unlikely (!Net_WLAN_Tools::associate (interface_identifier_string,
//                                                ap_mac_address_s,
//                                                configuration_->SSID,
//                                                inherited::handle_)))
//#endif // ACE_WIN32 || ACE_WIN64
//      {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Net_WLAN_Tools::associate(0x%@,\"%s\",%s), returning\n"),
//                    clientinherited::handle_,
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
//                    ACE_TEXT (configuration_->SSID.c_str ())));
//#else
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Net_WLAN_Tools::associate(\"%s\",%s,%s,%d), returning\n"),
//                    ACE_TEXT (interface_identifier_string.c_str ()),
//                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s)).c_str ()),
//                    ACE_TEXT (configuration_->SSID.c_str ()),
//                    inherited::handle_));
//#endif // ACE_WIN32 || ACE_WIN64
//        break;
//      } // end IF

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      break;
//#else
//      // *TODO*: implement nl80211 support, rather than polling
//      deadline = COMMON_TIME_NOW + result_timeout;
//      do
//      {
//        ether_addr_s =
//          Net_WLAN_Tools::associatedBSSID (configuration_->interfaceIdentifier,
//                                           inherited::handle_);
//        if (!ACE_OS::memcmp (&ether_addr_s.ether_addr_octet,
//                             &ap_mac_address_s.ether_addr_octet,
//                             ETH_ALEN))
//        {
//          result = true;
//          break;
//        } // end IF

//        result_2 = ACE_OS::sleep (result_poll_interval);
//        if (unlikely (result_2 == -1))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
//                      &result_poll_interval));
//        shutdown = inherited::hasShutDown ();
//      } while (!shutdown && (COMMON_TIME_NOW <= deadline));
//      if (unlikely (shutdown))
//        break;
//      if (unlikely (!result))
//      {
//        if (retries_ == NET_WLAN_MONITOR_AP_ASSOCIATION_RETRIES)
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("\"%s\": failed to associate with access point (MAC was: %s; SSID was: %s): timed out (was: %#T), giving up\n"),
//                      ACE_TEXT (interface_identifier_string.c_str ()),
//                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s)).c_str ()),
//                      ACE_TEXT (configuration_->SSID.c_str ()),
//                      &result_poll_interval));
//          retries_ = 0;
//          inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
//          break;
//        } // end IF
//        else
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("\"%s\": failed to associate with access point (MAC was: %s; SSID was: %s): timed out (was: %#T), retrying...\n"),
//                      ACE_TEXT (interface_identifier_string.c_str ()),
//                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s)).c_str ()),
//                      ACE_TEXT (configuration_->SSID.c_str ()),
//                      &result_poll_interval));
//          ++retries_;
//          goto associate;
//        } // end ELSE
//      } // end IF

//      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
//      break;
//#endif // ACE_WIN32 || ACE_WIN64
//    }
//    case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
//    {
//      // *NOTE*: actually 'set' intermediate states to support atomic state
//      //         transition notifications
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_DISASSOCIATE;
//      } // end lock scope

//      // sanity check(s)
////      std::string SSID_string = SSID ();
////      ACE_ASSERT (!SSID_string.empty ()); // associated
//      ACE_ASSERT (configuration_);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      ACE_ASSERT (clientinherited::handle_ != ACE_INVALID_HANDLE);
//      Net_WLAN_Tools::disassociate (clientinherited::handle_,
//                                    configuration_->interfaceIdentifier);
//#else
//      ACE_ASSERT (inherited::handle_ != ACE_INVALID_HANDLE);
//      Net_WLAN_Tools::disassociate (configuration_->interfaceIdentifier,
//                                    inherited::handle_);
//#endif

//      bool essid_is_cached = false;
//      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
//        // check cache whether the configured ESSID (if any) is known
//        Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t iterator =
//            SSIDCache_.find (configuration_->SSID);
//        essid_is_cached = (iterator != SSIDCache_.end ());
//      } // end lock scope

//      if (configuration_->SSID.empty () || // not configured
//          !essid_is_cached)                // configured SSID unknown (i.e. not cached)
//      {
//        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
//        break;
//      } // end IF

//      // --> configured && configured SSID known (i.e. cached)
//      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCANNED);
//      break;
//    }
//    case NET_WLAN_MONITOR_STATE_CONNECT:
//    {
//      // *NOTE*: actually 'set' intermediate states to support atomic state
//      //         transition notifications
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_CONNECT;
//      } // end lock scope

//      // sanity check(s)
//      ACE_ASSERT (configuration_);
//      ACE_ASSERT (!configuration_->SSID.empty ());
//      //{ ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
//      //  // check cache whether the configured ESSID (if any) is known
//      //  Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t iterator =
//      //    SSIDCache_.find (configuration_->SSID);
//      //  ACE_ASSERT (iterator != SSIDCache_.end ());
//      //} // end lock scope
//      std::string SSID_string = SSID ();
//      ACE_ASSERT (!SSID_string.empty ());
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      // *NOTE*: apparently, the Win32 API currently does not let developers
//      //         interfere with the WLAN DHCP addressing process
//      //         programmatically
//      //ACE_ASSERT (!ACE_OS::strcmp (configuration_->SSID.c_str (),
//      //                             SSID_string.c_str ()));
//#else
////      bool result = false;
//      ACE_Time_Value result_poll_interval (0,
//                                           NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL * 1000);
//      ACE_Time_Value result_timeout (NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT,
//                                     0);
//      ACE_Time_Value deadline;
////      bool shutdown = false;
//#if defined (DHCLIENT_SUPPORT)
//      // sanity check(s)
//      ACE_ASSERT (Common_Tools::getProcessId (ACE_TEXT_ALWAYS_CHAR ("dhclient")));
//      // *NOTE*: query the local dhclient for the current DHCP lease on the
//      //         configured interface
//dhcpctl:
//      dhcpctl_handle connection_h = dhcpctl_null_handle;
//      dhcpctl_handle authenticator_h = dhcpctl_null_handle;
//      dhcpctl_handle interface_h = dhcpctl_null_handle;
//      dhcpctl_status wait_status = -1;
//      dhcpctl_data_string result_string;
//      dhcpctl_status status_i =
//          dhcpctl_connect (&connection_h,
//                           ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DHCLIENT_LOCALHOST_IP_STRING),
//                           NET_WLAN_MONITOR_DHCLIENT_OMAPI_PORT,
//                           authenticator_h);
//      if (unlikely (status_i != ISC_R_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ::dhcpctl_connect(%s:%u): \"%s\", returning\n"),
//                    ACE_TEXT (NET_WLAN_MONITOR_DHCLIENT_LOCALHOST_IP_STRING), NET_WLAN_MONITOR_DHCLIENT_OMAPI_PORT,
//                    ACE_TEXT (isc_result_totext (status_i))));
//        break;
//      } // end IF
//      ACE_ASSERT (connection_h != dhcpctl_null_handle);

//      status_i = dhcpctl_new_object (&interface_h,
//                                     connection_h,
//                                     ACE_TEXT_ALWAYS_CHAR ("interface"));
//      if (unlikely (status_i != ISC_R_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ::dhcpctl_new_object(0x%@,%s): \"%s\", returning\n"),
//                    connection_h,
//                    ACE_TEXT ("interface"),
//                    ACE_TEXT (isc_result_totext (status_i))));
//        break;
//      } // end IF
//      ACE_ASSERT (interface_h != dhcpctl_null_handle);
//      status_i =
//          dhcpctl_set_string_value (interface_h,
//                                    ACE_TEXT_ALWAYS_CHAR (configuration_->interfaceIdentifier.c_str ()),
//                                    ACE_TEXT_ALWAYS_CHAR ("name"));
//      if (unlikely (status_i != ISC_R_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ::dhcpctl_set_string_value(0x%@,%s,\"%s\"): \"%s\", returning\n"),
//                    interface_h,
//                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
//                    ACE_TEXT ("name"),
//                    ACE_TEXT (isc_result_totext (status_i))));
//        break;
//      } // end IF
//      status_i =
//          dhcpctl_open_object (interface_h,
//                               connection_h,
//                               DHCPCTL_CREATE | DHCPCTL_EXCL);
//      if (unlikely (status_i != ISC_R_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ::dhcpctl_open_object(0x%@,0x%@,%d): \"%s\", returning\n"),
//                    interface_h,
//                    connection_h,
//                    DHCPCTL_CREATE | DHCPCTL_EXCL,
//                    ACE_TEXT (isc_result_totext (status_i))));
//        break;
//      } // end IF
//      // *TODO*: add a timeout here
//      status_i = dhcpctl_wait_for_completion (interface_h,
//                                              &wait_status);
//      if (unlikely (status_i != ISC_R_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ::dhcpctl_wait_for_completion(0x%@): \"%s\", returning\n"),
//                    interface_h,
//                    ACE_TEXT (isc_result_totext (status_i))));
//        break;
//      } // end IF
//      if (unlikely (wait_status != ISC_R_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ::dhcpctl_open_object(0x%@,create|excl): \"%s\", returning\n"),
//                    interface_h,
//                    ACE_TEXT (isc_result_totext (wait_status))));
//        break;
//      } // end IF
//      ACE_OS::memset (&result_string, 0, sizeof (dhcpctl_data_string));
//      status_i = dhcpctl_get_value (&result_string,
//                                    interface_h,
//                                    ACE_TEXT_ALWAYS_CHAR ("state"));
//      if (unlikely (status_i != ISC_R_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ::dhcpctl_get_value(0x%@,state): \"%s\", returning\n"),
//                    interface_h,
//                    ACE_TEXT (isc_result_totext (wait_status))));
//        break;
//      } // end IF

//      dhcpctl_data_string_dereference (&result_string, MDL);
//      ACE_ASSERT (false); // *TODO*
//#endif // DHCLIENT_SUPPORT
//#endif // ACE_WIN32 || ACE_WIN64
//      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_CONNECTED);
//      break;
//    }
//    case NET_WLAN_MONITOR_STATE_DISCONNECT:
//    {
//      // *NOTE*: actually 'set' intermediate states to support atomic state
//      //         transition notifications
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_DISCONNECT;
//      } // end lock scope

//      // sanity check(s)
//      ACE_ASSERT (configuration_);
//      ACE_ASSERT (!configuration_->SSID.empty ());
//      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
//        // check cache whether the configured ESSID (if any) is known
//        Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t iterator =
//          SSIDCache_.find (configuration_->SSID);
//        ACE_ASSERT (iterator != SSIDCache_.end ());
//      } // end lock scope
//      std::string SSID_string = SSID ();
//      ACE_ASSERT (!SSID_string.empty ());
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      // *NOTE*: apparently, the Win32 API currently does not let developers
//      //         interfere with the DHCP addressing process programmatically
//#else
//#if defined (DHCLIENT_SUPPORT)
//      // sanity check(s)
//      ACE_ASSERT (Common_Tools::getProcessId (ACE_TEXT_ALWAYS_CHAR ("dhclient")));
//      // *NOTE*: relinquish the current DHCP lease on the configured interface
//      ACE_ASSERT (false); // *TODO*
//#endif // DHCLIENT_SUPPORT
//#endif // ACE_WIN32 || ACE_WIN64

//      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_DISASSOCIATE);
//      break;
//    }
//    //////////////////////////////////////
//    case NET_WLAN_MONITOR_STATE_INITIALIZED:
//      break;
//    case NET_WLAN_MONITOR_STATE_SCANNED:
//    {
//      // *NOTE*: set state early to support atomic state transition
//      //         notifications
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_SCANNED;
//      } // end lock scope

//      // sanity check(s)
//      ACE_ASSERT (configuration_);

//      std::string SSID_string = SSID ();
//      if (unlikely (!isConnectionNotified_ &&
//                    ((configuration_->SSID.empty () &&
//                      !SSID_string.empty ()) ||                 // not configured, associated
//                     (!configuration_->SSID.empty () &&
//                      !SSID_string.empty ()          &&
//                      !ACE_OS::strcmp (configuration_->SSID.c_str (),
//                                      SSID_string.c_str ()))))) // already associated to configured ESSID
//      {
//        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
//        break;
//      } // end IF

//      bool essid_is_cached = false;
//      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
//        // check cache whether the configured ESSID (if any) is known
//        Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t iterator =
//            SSIDCache_.find (configuration_->SSID);
//        essid_is_cached = (iterator != SSIDCache_.end ());
//      } // end lock scope

//      if (unlikely (SSID_string.empty ()           && // not associated
//                    !configuration_->SSID.empty () && // configured
//                    essid_is_cached                && // configured SSID known (i.e. cached)
//                    configuration_->autoAssociate))   // auto-associate enabled
//      {
//        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);
//        break;
//      } // end IF

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//      // switch back to 'idle', but wait in-between scans
//      ACE_Time_Value scan_interval (NET_WLAN_MONITOR_UNIX_SCAN_INTERVAL, 0);
//      int result = ACE_OS::sleep (scan_interval);
//      if (unlikely (result == -1))
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
//                    &scan_interval));
//#endif // ACE_WIN32 || ACE_WIN64

//      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);

//      break;
//    }
//    case NET_WLAN_MONITOR_STATE_ASSOCIATED:
//    {
//      // *NOTE*: set state early to support atomic state transition
//      //         notifications
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATED;
//      } // end lock scope

//      // sanity check(s)
//      ACE_ASSERT (configuration_);

//      std::string SSID_string = SSID ();
//      ACE_ASSERT (!SSID_string.empty ());
//      if (unlikely (ACE_OS::strcmp (configuration_->SSID.c_str (),
//                                    SSID_string.c_str ()))) // associated to a different SSID
//      {
//        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_DISASSOCIATE);
//        break;
//      } // end IF

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientinherited::handle_,
//                                                           configuration_->interfaceIdentifier,
//                                                           wlan_intf_opcode_autoconf_enabled,
//                                                           configuration_->enableAutoConf)))
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
//                    clientinherited::handle_,
//                    wlan_intf_opcode_autoconf_enabled,
//                    (configuration_->enableAutoConf ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
//      if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientinherited::handle_,
//                                                           configuration_->interfaceIdentifier,
//                                                           wlan_intf_opcode_background_scan_enabled,
//                                                           configuration_->enableBackgroundScans)))
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
//                    clientinherited::handle_,
//                    wlan_intf_opcode_background_scan_enabled,
//                    (configuration_->enableBackgroundScans ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
//      if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientinherited::handle_,
//                                                           configuration_->interfaceIdentifier,
//                                                           wlan_intf_opcode_media_streaming_mode,
//                                                           configuration_->enableMediaStreamingMode)))
//        ACE_DEBUG ((LM_ERROR,NET_WLAN_MONITOR_STATE_DISASSOCIATE
//                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
//                    clientinherited::handle_,
//                    wlan_intf_opcode_media_streaming_mode,
//                    (configuration_->enableMediaStreamingMode ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
//#else
//      // *TODO*: optimize tx-power
//#endif // ACE_WIN32 || ACE_WIN64

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//      if (likely (isConnectionNotified_))
//        goto continue_2;
//      try {
//        onAssociate (configuration_->interfaceIdentifier,
//                     SSID (),
//                     true);
//      } catch (...) {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(\"%s\",%s,true), continuing\n"),
//                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
//                    ACE_TEXT (SSID ().c_str ())));
//      }
//continue_2:
//#endif // ACE_WIN32 || ACE_WIN64

//      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_CONNECT);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
////reset_state:
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATED;
//      } // end lock scope
//#endif
//      break;
//    }
//    case NET_WLAN_MONITOR_STATE_CONNECTED:
//    {
//      // *NOTE*: set state early to support atomic state transition
//      //         notifications
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#else
//      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
//#endif
//        inherited::state_ = NET_WLAN_MONITOR_STATE_CONNECTED;
//      } // end lock scope

//      if (likely (isConnectionNotified_))
//        goto continue_3;
//      isConnectionNotified_ = true;
//      try {
//        onConnect (configuration_->interfaceIdentifier,
//                   SSID (),
//                   true);
//      } catch (...) {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(\"%s\",%s,true), continuing\n"),
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
//                    ACE_TEXT (SSID ().c_str ())));
//#else
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(\"%s\",%s,true), continuing\n"),
//                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
//                    ACE_TEXT (SSID ().c_str ())));
//#endif
//      }

//continue_3:
//      if (unlikely (isFirstConnect_))
//        isFirstConnect_ = false;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (unlikely (scanTimerId_ == -1))
//        startScanTimer ();
//#else
//      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
//#endif
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("unknown/invalid state switch: \"%s\" --> \"%s\" --> check implementation !, returning\n"),
//                  ACE_TEXT (stateToString (inherited::state_).c_str ()),
//                  ACE_TEXT (stateToString (newState_in).c_str ())));
//      break;
//    }
//  } // end SWITCH
//}
