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

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net/ethernet.h"

#if defined (NL80211_SUPPORT)
#include <linux/netlink.h>
#include <linux/nl80211.h>

#include "netlink/handlers.h"
#include "netlink/netlink.h"
#include "netlink/genl/ctrl.h"
#include "netlink/genl/genl.h"
#endif // NL80211_SUPPORT

#if defined (DHCLIENT_SUPPORT)
extern "C"
{
#include "dhcpctl/dhcpctl.h"
}
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (_DEBUG)
#include "ace/High_Res_Timer.h"
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"
#include "ace/Time_Value.h"

#include "common_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_timer_manager_common.h"
#else
#if defined (NL80211_SUPPORT)
#include "common_math_tools.h"
#endif // NL80211_SUPPORT
#if defined (_DEBUG)
#include "common_timer_tools.h"
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_defines.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"
#include "net_packet_headers.h"

#include "net_wlan_defines.h"

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
//      struct ether_addr ap_mac_address, ether_addr_s;
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
//        ap_mac_address = (*iterator).second.second.accessPointLinkLayerAddress;
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
//                                                ap_mac_address,
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
//                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
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
//                             &ap_mac_address.ether_addr_octet,
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
//                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
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
//                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
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

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_WLANAPI,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited ()
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_WLANAPI,
                   UserDataType>::~Net_WLAN_Monitor_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::~Net_WLAN_Monitor_T"));

}

template <typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_WLANAPI,
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
  ACE_ASSERT (inherited::clientinherited::handle_ == ACE_INVALID_HANDLE);

  if (unlikely (!Net_WLAN_Tools::initialize (inherited::clientinherited::handle_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), returning\n")));
    return;
  } // end IF
  ACE_ASSERT (inherited::clientinherited::handle_ != ACE_INVALID_HANDLE);

  Net_InterfaceIdentifiers_t interface_identifiers_a =
      Net_WLAN_Tools::getInterfaces (inherited::clientinherited::handle_);

  struct _WLAN_INTERFACE_INFO_LIST* interface_list_p = NULL;
  DWORD notification_mask = WLAN_NOTIFICATION_SOURCE_ALL;
  DWORD previous_notification_mask = 0;
  DWORD result =
      WlanRegisterNotification (inherited::clientinherited::handle_,
                                notification_mask,
                                FALSE,
                                inherited::configuration_->notificationCB,
                                inherited::configuration_->notificationCBData,
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
  if (inherited::clientinherited::handle_ != ACE_INVALID_HANDLE)
    Net_WLAN_Tools::initialize (inherited::clientinherited::handle_);
  inherited::clientinherited::handle_ = ACE_INVALID_HANDLE;

  return;

continue_:
  inherited::isActive_ = true;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
}

template <typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_WLANAPI,
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  if (unlikely (!inherited::isActive_))
    return;
  ACE_ASSERT (inherited::clientinherited::handle_ != ACE_INVALID_HANDLE);

  if (likely (inherited::scanTimerId_ != -1))
    cancelScanTimer ();
  Net_WLAN_Tools::finalize (inherited::clientinherited::handle_);
  inherited::clientinherited::handle_ = ACE_INVALID_HANDLE;

  inherited::isActive_ = false;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_INITIALIZED);
}

template <typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_WLANAPI,
                   UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    userData_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  userData_ = configuration_in.userData;

  return inherited::initialize (configuration_in);
}
#else
#if defined (WEXT_SUPPORT)
template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_IOCTL,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited ()
 , range_ ()
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

  //  inherited::reactor (ACE_Reactor::instance ());
  ACE_OS::memset (&range_, 0, sizeof (struct iw_range));
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
                   NET_WLAN_MONITOR_API_IOCTL,
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
#if defined (ACE_LINUX)
  if (Net_Common_Tools::isNetworkManagerRunning ())
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("systemd NetworkManager service is running; this may interfere with the monitoring activities, continuing\n")));
#endif
    ACE_ASSERT (inherited::handle_ == ACE_INVALID_HANDLE);

  inherited::handle_ = ACE_OS::socket (AF_INET,
                                       SOCK_DGRAM,
                                       0);
  if (unlikely (inherited::handle_ == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(AF_INET,SOCK_DGRAM,0): \"%m\", returning\n")));
    return;
  } // end IF
//  inherited::set_handle (inherited::handle_);

  int result =
      iw_get_range_info (inherited::handle_,
                         inherited::configuration_->interfaceIdentifier.c_str (),
                         &range_);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_get_range_info(%d,\"%s\"): \"%m\", returning\n"),
                inherited::handle_,
                ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ())));
    return;
  } // end IFs
  // verify that the interface supports scanning
  if (range_.we_version_compiled < 14)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("interface (was: \"%s\") does not support scanning, returning\n"),
                ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ())));
    return;
  } // end IF

  // monitor the interface in a dedicated thread
  ACE_Time_Value deadline =
      (COMMON_TIME_NOW +
       ACE_Time_Value (0, COMMON_TIMEOUT_DEFAULT_THREAD_SPAWN * 1000));
  result = inherited::open (NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", returning\n")));
    return;
  } // end IF
  do
  { // *NOTE*: the livelock here
    if (COMMON_TIME_NOW > deadline)
    {
      ACE_OS::last_error (ETIMEDOUT);
      break;
    } // end IF
  } while (!inherited::TASK_T::isRunning ());
  if (unlikely (!inherited::TASK_T::isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", returning\n")));
    return;
  } // end IF

  inherited::isActive_ = true;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
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
                   NET_WLAN_MONITOR_API_IOCTL,
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  if (!inherited::isActive_)
    return;

  inherited::isActive_ = false;
  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);

  int result = -1;
//  if (likely (inherited::isRegistered_))
//  {
//    ACE_Reactor* reactor_p = inherited::reactor ();
//    ACE_ASSERT (reactor_p);
//    result = reactor_p->remove_handler (this,
//                                        ACE_Event_Handler::READ_MASK);
//    if (unlikely (result == -1))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(0x%@,ACE_Event_Handler::READ_MASK): \"%m\", continuing\n"),
//                  this));
//    inherited::isRegistered_ = false;
//  } // end IF

  if (likely (inherited::handle_ != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::close (inherited::handle_);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  inherited::handle_));
    inherited::handle_ = ACE_INVALID_HANDLE;
  } // end IF

  if (likely (inherited::buffer_))
  {
    ACE_OS::free (inherited::buffer_);
    inherited::buffer_ = NULL;
  } // end IF
  inherited::bufferSize_ = 0;
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
                   NET_WLAN_MONITOR_API_IOCTL,
                   UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    ACE_OS::memset (&range_, 0, sizeof (struct iw_range));

    userData_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  userData_ = configuration_in.userData;

  return inherited::initialize (configuration_in);
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
                   NET_WLAN_MONITOR_API_IOCTL,
                   UserDataType>::inherited::handle_input (ACE_HANDLE inherited::handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::inherited::handle_input"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::handle_in == inherited::handle_);

  int result = -1;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   inherited::configuration_->interfaceIdentifier.c_str (),
                   IFNAMSIZ);
  struct stream_descr stream_descr_s;
  ACE_OS::memset (&stream_descr_s, 0, sizeof (struct stream_descr));
  struct iw_event iw_event_s;
  ACE_OS::memset (&iw_event_s, 0, sizeof (struct iw_event));
  int error = 0;
  Net_WLAN_SSIDToInterfaceIdentifierIterator_t iterator;
  struct ether_addr ap_mac_address;
  ACE_OS::memset (&ap_mac_address, 0, sizeof (struct ether_addr));
  std::string essid_string;
  ACE_TCHAR buffer_a[BUFSIZ];
#if defined (_DEBUG)
  std::set<std::string> known_ssids, current_ssids;
#endif

  if (!inherited::buffer_)
  { ACE_ASSERT (!inherited::bufferSize_);
    inherited::bufferSize_ = IW_SCAN_MAX_DATA;
    inherited::buffer_ = ACE_OS::malloc (inherited::bufferSize_);
    if (unlikely (!inherited::buffer_))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory (%u byte(s)): \"%m\", returning\n"),
                  inherited::bufferSize_));
      goto continue_;
    } // end IF
  } // end IF
  iwreq_s.u.data.pointer = inherited::buffer_;
  iwreq_s.u.data.length = inherited::bufferSize_;

fetch_scan_result_data:
  result = ACE_OS::ioctl (inherited::handle_in,
                          SIOCGIWSCAN,
                          &iwreq_s);
  if (result < 0)
  {
    error = ACE_OS::last_error ();
    if ((error == E2BIG) && // 7: buffer too small
        (range_.we_version_compiled > 16))
    {
      if (iwreq_s.u.data.length > inherited::bufferSize_)
        inherited::bufferSize_ = iwreq_s.u.data.length;
      else
        inherited::bufferSize_ *= 2; // grow dynamically
      goto retry;
    } // end IF

    if (unlikely (error != EAGAIN)) // 11: result(s) not available yet
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCGIWSCAN): \"%m\", returning\n"),
                  inherited::handle_in));
//    else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("scan results not yet available, returning\n")));
    goto continue_;
  } // end IF

  // the driver may have reported the required buffer size
  if (unlikely (iwreq_s.u.data.length > inherited::bufferSize_))
  { // --> grow the buffer and retry
    inherited::bufferSize_ = iwreq_s.u.data.length;
retry:
    inherited::buffer_ = ACE_OS::realloc (inherited::buffer_,
                                          inherited::bufferSize_);
    if (!inherited::buffer_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to reallocate memory (%u byte(s)): \"%m\", returning\n"),
                  inherited::bufferSize_));
      goto continue_;
    } // end IF
    iwreq_s.u.data.pointer = inherited::buffer_;
    iwreq_s.u.data.length = inherited::bufferSize_;
    goto fetch_scan_result_data;
  } // end IF
  ACE_ASSERT (iwreq_s.u.data.length && (iwreq_s.u.data.length <= inherited::bufferSize_));

  // received scan results

  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, 0);
    // clear cache
    do
    {
      iterator =
          std::find_if (inherited::SSIDCache_.begin (), inherited::SSIDCache_.end (),
                        std::bind2nd (Net_WLAN_SSIDToInterfaceIdentifierFindPredicate (),
                                      inherited::configuration_->interfaceIdentifier));
      if (iterator == inherited::SSIDCache_.end ())
        break;

#if defined (_DEBUG)
      known_ssids.insert ((*iterator).first);
#endif
      inherited::SSIDCache_.erase ((*iterator).first);
    } while (true);
  } // end lock scope

  // process the result data
  iw_init_event_stream (&stream_descr_s,
                        static_cast<char*> (inherited::buffer_),
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
        struct Net_WLAN_AssociationConfiguration association_configuration_s;
        association_configuration_s.accessPointLinkLayerAddress =
            ap_mac_address;
        { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, 0);
          inherited::SSIDCache_.insert (std::make_pair (essid_string,
                                                        std::make_pair (inherited::configuration_->interfaceIdentifier,
                                                                        association_configuration_s)));
        } // end lock scope
#if defined (_DEBUG)
        if (known_ssids.find (essid_string) == known_ssids.end ())
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("\"%s\": detected wireless access point (MAC address: %s, ESSID: %s)...\n"),
                      ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ()),
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
                    ACE_TEXT ("invalid/unknown WE event (was: %d), returning\n"),
                    iw_event_s.cmd));
        goto continue_;
      }
    } // end SWITCH
  } while (true);
#if defined (_DEBUG)
  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, 0);
    for (Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t iterator_2 = inherited::SSIDCache_.begin ();
         iterator_2 != inherited::SSIDCache_.end ();
         ++iterator_2)
      if (!ACE_OS::strcmp ((*iterator_2).second.first.c_str (),
                           inherited::configuration_->interfaceIdentifier.c_str ()))
        current_ssids.insert ((*iterator_2).first);
  } // end lock scope
  for (std::set<std::string>::const_iterator iterator_2 = known_ssids.begin ();
       iterator_2 != known_ssids.end ();
       ++iterator_2)
    if (current_ssids.find (*iterator_2) == current_ssids.end ())
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": lost contact to ESSID (was: %s)...\n"),
                  ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ()),
                  ACE_TEXT ((*iterator_2).c_str ())));
#endif

  try {
    inherited::onScanComplete (inherited::configuration_->interfaceIdentifier);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n")));
  }

continue_:
  // *NOTE*: do not deregister from the callback
  return 0;
}
#endif // WEXT_SUPPORT

//////////////////////////////////////////

#if defined (NL80211_SUPPORT)
template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited ()
 , inherited2 ()
 , userData_ (NULL)
 /////////////////////////////////////////
 , buffer_ (NULL)
 , callbacks_ (NULL)
 , controlId_ (0)
 , error_ (0)
 , headerReceived_ (false)
 , inputStream_ ()
 , isRegistered_ (false)
 , isSubscribedToMulticastGroups_ (false)
 , message_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

  inherited::TASK_T::threadCount_ = 1;

//  inherited::TASK_T::reactor (ACE_Reactor::instance ());
//  inherited2::proactor (ACE_Proactor::instance ());
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::~Net_WLAN_Monitor_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::~Net_WLAN_Monitor_T"));

  int result = -1;

  if (buffer_)
    buffer_->release ();

  if (isRegistered_)
  { ACE_ASSERT (inherited::configuration_);
    switch (inherited::configuration_->dispatch)
    {
      case COMMON_EVENT_DISPATCH_PROACTOR:
      {
        result = inputStream_.cancel ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\", continuing\n")));
        break;
      }
      case COMMON_EVENT_DISPATCH_REACTOR:
      {
        ACE_Reactor* reactor_p = inherited::TASK_T::reactor ();
        ACE_ASSERT (reactor_p);
        result = reactor_p->remove_handler (this,
                                            ACE_Event_Handler::ALL_EVENTS_MASK);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::remove_handler(%@): \"%m\", continuing\n"),
                      this));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unkown dispatch type (was: %d), continuing\n"),
                    inherited::configuration_->dispatch));
        break;
      }
    } // end SWITCH
  } // end IF

  if (callbacks_)
    nl_cb_put (callbacks_);

  if (message_)
    nlmsg_free (message_);
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
                   NET_WLAN_MONITOR_API_NL80211,
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

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (callbacks_);
//  ACE_ASSERT (!controlId_);
  ACE_ASSERT (!error_);
  ACE_ASSERT (inherited::handle_);
//  ACE_ASSERT (!isRegistered_);
#if defined (ACE_LINUX)
  if (Net_Common_Tools::isNetworkManagerRunning ())
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the (systemd) NetworkManager service is running; this may interfere with the monitoring activities, continuing\n")));
#endif // ACE_LINUX

  int result = -1;
  struct nl_msg* message_p = NULL;
  Net_WLAN_nl80211_MulticastGroupIds_t multicast_group_ids_m;
  struct Net_WLAN_nl80211_MulticastGroupIdQueryCBData cb_data_s;
  cb_data_s.map = &multicast_group_ids_m;
  Net_WLAN_IMonitorBase* imonitor_base_p = this;
  int socket_handle_i = nl_socket_get_fd (inherited::handle_);
  ACE_Time_Value deadline;

  if (controlId_)
    goto continue_;

  controlId_ =
      genl_ctrl_resolve (inherited::handle_,
                         ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_NL80211_CONTROL_NAME_STRING));
  if (unlikely (controlId_ < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genl_ctrl_resolve(0x%@,\"%s\"): \"%s\", returning\n"),
                inherited::handle_,
                ACE_TEXT (NET_WLAN_MONITOR_NL80211_CONTROL_NAME_STRING),
                ACE_TEXT (nl_geterror (controlId_))));
    goto error;
  } // end IF

continue_:
  // subscribe to all defined nl80211 multicast groups (i.e. WLAN events)
  if (isSubscribedToMulticastGroups_)
    goto continue_2;

  // step1: resolve multicast group ids
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_CONFIG),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_SCAN),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_REG),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_MLME),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_VENDOR),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_NAN),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_TESTMODE),
                                                -ENOENT));
  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    goto error;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,       // port #
                              NL_AUTO_SEQ,        // sequence #
                              controlId_,         // family id
                              0,                  // (user-) hdrlen
                              0,                  // flags
                              CTRL_CMD_GETFAMILY, // command id
                              0)))                // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
    nlmsg_free (message_p);
    goto error;
  } // end IF
  NLA_PUT_STRING (message_p,
                  CTRL_ATTR_FAMILY_NAME,
                  ACE_TEXT_ALWAYS_CHAR (NL80211_GENL_NAME));
  result = nl_send_auto_complete (inherited::handle_, message_p);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto_complete(%@): \"%s\", returning\n"),
                inherited::handle_,
                ACE_TEXT (nl_geterror (result))));
    nlmsg_free (message_p);
    goto error;
  } // end IF
  nlmsg_free (message_p);
  message_p = NULL;

  result = nl_cb_err (callbacks_,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_error_cb,
                      &error_);
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_ACK,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_ack_cb,
                      &error_);
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_FINISH,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_finish_cb,
                      &error_);
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_VALID,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_multicast_groups_cb,
                      &cb_data_s);
  ACE_ASSERT (result >= 0);
  error_ = 1;
  while (//(result > 0) &&
         error_ > 0)
    result = nl_recvmsgs (inherited::handle_, callbacks_);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", returning\n"),
                inherited::handle_,
                ACE_TEXT (nl_geterror ((result < 0) ? result : error_))));
    goto error;
  } // end IF

  // step2: subscribe to all available multicast groups
  for (Net_WLAN_nl80211_MulticastGroupIdsIterator_t iterator = multicast_group_ids_m.begin ();
       iterator != multicast_group_ids_m.end ();
       ++iterator)
  {
    if (unlikely ((*iterator).second == -ENOENT))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("nl80211 multicast group (was: \"%s\") not available, continuing\n"),
                  ACE_TEXT ((*iterator).first.c_str ())));
      continue;
    } // end IF
    result = nl_socket_add_memberships (inherited::handle_,
                                        (*iterator).second,
                                        NFNLGRP_NONE);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_socket_add_memberships(%@,%d): \"%s\", returning\n"),
                  inherited::handle_,
                  (*iterator).second,
                  ACE_TEXT (nl_geterror (result))));
      goto error;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("subscribed to nl80211 multicast group \"%s\" (id: %u)\n"),
                ACE_TEXT ((*iterator).first.c_str ()),
                static_cast<ACE_UINT32> ((*iterator).second)));
#endif
  } // end FOR

  /* disable sequence checking for multicast messages */
  result = nl_cb_set (callbacks_,
                      NL_CB_SEQ_CHECK,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_no_seq_check_cb,
                      &error_);
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_VALID,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_default_handler_cb,
                      imonitor_base_p);
  ACE_ASSERT (result >= 0);

  if (!Net_WLAN_Tools::getFeatures (inherited::configuration_->interfaceIdentifier,
                                    inherited::handle_,
                                    inherited::familyId_,
                                    features_,
                                    extendedFeatures_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::getFeatures(\"%s\",%@,%d), returning\n"),
                ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ()),
                inherited::handle_,
                inherited::familyId_));
    goto error;
  } // end IF

  isSubscribedToMulticastGroups_ = true;

continue_2:
  ACE_ASSERT (socket_handle_i);
  switch (inherited::configuration_->dispatch)
  {
    case COMMON_EVENT_DISPATCH_PROACTOR:
    { ACE_ASSERT (inherited::configuration_);
      inherited2::handle (socket_handle_i);

      ACE_Proactor* proactor_p =
//          inherited2::proactor ();
          ACE_Proactor::instance ();
      ACE_ASSERT (proactor_p);

      inherited2::proactor (proactor_p);

      result = inputStream_.open (*this,
                                  socket_handle_i,
                                  NULL,
                                  proactor_p);
      if (unlikely (result == -1))
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Asynch_Read_Stream::open(%d): \"%m\", returning\n"),
                    socket_handle_i));
        goto error;
      } // end IF

      if (unlikely (!initiate_read_stream (inherited::configuration_->defaultBufferSize)))
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Monitor_T::initiate_read_stream(%u), returning\n"),
                    inherited::configuration_->defaultBufferSize));
        goto error;
      } // end IF

      break;
    }
    case COMMON_EVENT_DISPATCH_REACTOR:
    {
      ACE_HANDLE handle_h = nl_socket_get_fd (inherited::handle_);
      ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);
//      inherited2::set_handle ();
      inherited::TASK_T::set_handle (handle_h);

      ACE_Reactor* reactor_p =
//          inherited::TASK_T::reactor ();
          ACE_Reactor::instance ();
      ACE_ASSERT (reactor_p);

      inherited::TASK_T::reactor (reactor_p);

      result = reactor_p->register_handler (handle_h,
                                            this,
                                            ACE_Event_Handler::READ_MASK);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Reactor::register_handler(%d,0x%@,ACE_Event_Handler::READ_MASK): \"%m\", returning\n"),
                    handle_h,
                    this));
        goto error;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unkown dispatch type (was: %d), returning\n"),
                  inherited::configuration_->dispatch));
      goto error;
    }
  } // end SWITCH
  isRegistered_ = true;

  // monitor the interface in a dedicated thread
  deadline =
      (COMMON_TIME_NOW +
       ACE_Time_Value (0, COMMON_TIMEOUT_DEFAULT_THREAD_SPAWN * 1000));
  result = inherited::open (NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", returning\n")));
    goto error;
  } // end IF
  do
  { // *NOTE*: the livelock here
    if (COMMON_TIME_NOW > deadline)
    {
      ACE_OS::last_error (ETIMEDOUT);
      break;
    } // end IF
  } while (!inherited::TASK_T::isRunning ());
  if (unlikely (!inherited::TASK_T::isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", returning\n")));
    goto error;
  } // end IF

  inherited::isActive_ = true;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);

  return;

error:
nla_put_failure:
  if (isRegistered_)
  {
    switch (inherited::configuration_->dispatch)
    {
      case COMMON_EVENT_DISPATCH_PROACTOR:
      {
        result = inputStream_.cancel ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\", continuing\n")));
        break;
      }
      case COMMON_EVENT_DISPATCH_REACTOR:
      {
        ACE_Reactor* reactor_p = inherited::TASK_T::reactor ();
        ACE_ASSERT (reactor_p);
        result = reactor_p->remove_handler (this,
                                            ACE_Event_Handler::ALL_EVENTS_MASK);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::remove_handler(%@): \"%m\", continuing\n"),
                      this));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unkown dispatch type (was: %d), continuing\n"),
                    inherited::configuration_->dispatch));
        break;
      }
    } // end SWITCH
    isRegistered_ = false;
  } // end IF
  if (message_p)
    nlmsg_free (message_p);
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (unlikely (!inherited::isActive_))
    return;

  int result = -1;

  if (likely (isRegistered_))
  {
    switch (inherited::configuration_->dispatch)
    {
      case COMMON_EVENT_DISPATCH_PROACTOR:
      {
        result = inputStream_.cancel ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\", continuing\n")));
        break;
      }
      case COMMON_EVENT_DISPATCH_REACTOR:
      {
        ACE_Reactor* reactor_p = inherited::TASK_T::reactor ();
        ACE_ASSERT (reactor_p);
        result = reactor_p->remove_handler (this,
                                            ACE_Event_Handler::ALL_EVENTS_MASK);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::remove_handler(%@): \"%m\", continuing\n"),
                      this));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unkown dispatch type (was: %d), continuing\n"),
                    inherited::configuration_->dispatch));
        break;
      }
    } // end SWITCH
    isRegistered_ = false;
  } // end IF

  inherited::isActive_ = false;
  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    if (buffer_)
    {
      buffer_->release ();
      buffer_ = NULL;
    } // end IF

    if (callbacks_)
    {
      nl_cb_put (callbacks_);
      callbacks_ = NULL;
    } // end IF

    controlId_ = 0;
    error_ = 0;

    extendedFeatures_.clear ();
    features_ = 0;

    headerReceived_ = false;

    int result = -1;
    if (isRegistered_)
    { ACE_ASSERT (inherited::configuration_);
      switch (inherited::configuration_->dispatch)
      {
        case COMMON_EVENT_DISPATCH_PROACTOR:
        {
          result = inputStream_.cancel ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\", continuing\n")));
          break;
        }
        case COMMON_EVENT_DISPATCH_REACTOR:
        {
          ACE_Reactor* reactor_p = inherited::TASK_T::reactor ();
          ACE_ASSERT (reactor_p);
          result = reactor_p->remove_handler (this,
                                              ACE_Event_Handler::ALL_EVENTS_MASK);
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Reactor::remove_handler(%@): \"%m\", continuing\n"),
                        this));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unkown dispatch type (was: %d), continuing\n"),
                      inherited::configuration_->dispatch));
          break;
        }
      } // end SWITCH
    } // end IF
    isRegistered_ = false;

    isSubscribedToMulticastGroups_ = false;

    if (message_)
    {
      nlmsg_free (message_);
      message_ = NULL;
    } // end IF

    userData_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  userData_ = configuration_in.userData;

  if (unlikely (!inherited::initialize (configuration_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Monitor_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  // sanity check(s)
  ACE_ASSERT (inherited::handle_);
  ACE_ASSERT (!callbacks_);

  callbacks_ =
#if defined (_DEBUG)
      nl_cb_alloc (configuration_in.debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
#else
      nl_cb_alloc (NL_CB_DEFAULT);
#endif // _DEBUG
  if (unlikely (!callbacks_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nl_cb_alloc (%d): \"%m\", aborting\n"),
#if defined (_DEBUG)
                (configuration_in.debug ? NL_CB_DEBUG : NL_CB_DEFAULT)));
#else
                NL_CB_DEFAULT));
#endif // _DEBUG
    return false;
  } // end IF

  nl_socket_set_cb (inherited::handle_, callbacks_);

  return true;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::do_associate (const std::string& interfaceIdentifier_in,
                                                const struct ether_addr& APMACAddress_in,
                                                const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::do_associate"));

  // sanity check(s)
  ACE_ASSERT (inherited::handle_);
  ACE_ASSERT (inherited::familyId_);
  if (!SSID_in.empty ())
  {
    ACE_ASSERT (!Net_Common_Tools::isAny (APMACAddress_in));
  } // end IF

  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (interfaceIdentifier_in.empty ())
    interface_identifiers_a =
        Net_WLAN_Tools::getInterfaces (inherited::handle_,
                                       inherited::familyId_,
                                       AF_UNSPEC,
                                       0);
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);

  struct nl_msg* message_p = NULL;
  // *TODO*: implement propert disassociate/deauthenticate procedure
//  int command_i = (SSID_in.empty () ? NL80211_CMD_DISASSOCIATE
//                                    : NL80211_CMD_ASSOCIATE);
  int command_i = (SSID_in.empty () ? NL80211_CMD_DISCONNECT
                                    : NL80211_CMD_CONNECT);
  int result = -1;
  unsigned int if_index_i = 0;
  struct ether_addr ap_mac_address_s = APMACAddress_in;
  std::string ssid_string = SSID_in;
  ACE_UINT32 frequency_i = 0, authentication_type_i = 0;
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    message_p = nlmsg_alloc ();
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
      return false;
    } // end IF
    if (unlikely (!genlmsg_put (message_p,
                                NL_AUTO_PORT,              // port #
                                NL_AUTO_SEQ,               // sequence #
                                inherited::familyId_,      // family id
                                0,                         // (user-) hdrlen
//                                NLM_F_REQUEST | NLM_F_ACK, // flags
                                0,
                                command_i,                 // command id
                                0)))                       // interface version
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
      goto error;
    } // end IF
    if_index_i = ::if_nametoindex ((*iterator).c_str ());
    if (unlikely (!if_index_i))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT ((*iterator).c_str ())));
      goto error;
    } // end IF
    NLA_PUT_U32 (message_p,
                 NL80211_ATTR_IFINDEX,
                 static_cast<ACE_UINT32> (if_index_i));
//    if (command_i == NL80211_CMD_DISASSOCIATE)
    if (command_i == NL80211_CMD_DISCONNECT)
    {
      if (Net_Common_Tools::isAny (ap_mac_address_s))
        ap_mac_address_s = Net_WLAN_Tools::associatedBSSID (*iterator,
                                                            NULL,
                                                            inherited::familyId_);
//      ssid_string = Net_WLAN_Tools::associatedSSID (*iterator,
//                                                    NULL,
//                                                    inherited::familyId_);
      // *TODO*: callers specify the most appropriate reason code
      NLA_PUT_U16 (message_p,
                   NL80211_ATTR_REASON_CODE,
                   NET_WLAN_MONITOR_NL80211_REASON_CODE_LEAVING);
//      nla_put_flag (message_p,
//                    NL80211_ATTR_LOCAL_STATE_CHANGE);
    } // end IF
    ACE_ASSERT (!Net_Common_Tools::isAny (ap_mac_address_s));
    NLA_PUT (message_p,
             NL80211_ATTR_MAC,
             ETH_ALEN,
             ap_mac_address_s.ether_addr_octet);
//    if (command_i == NL80211_CMD_ASSOCIATE)
    if (command_i == NL80211_CMD_CONNECT)
    {
      NLA_PUT (message_p,
               NL80211_ATTR_SSID,
               ssid_string.size (),
               ssid_string.c_str ());
      { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, 0);
        Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t iterator =
            inherited::SSIDCache_.begin ();
        if (likely (iterator != inherited::SSIDCache_.end ()))
        {
          frequency_i = (*iterator).second.second.frequency;
          authentication_type_i = (*iterator).second.second.authenticationType;
        } // end IF
      } // end lock scope
      NLA_PUT_U32 (message_p,
                   NL80211_ATTR_AUTH_TYPE,
                   authentication_type_i);
      // NL80211_ATTR_USE_MFP
      // *NOTE*: the next two attributes, if included, "...are restrictions on
      //         BSS selection, i.e., they effectively prevent roaming within
      //         the ESS. ..."
//      NLA_PUT (message_p,
//               NL80211_ATTR_MAC,
//               ETH_ALEN,
//               APMACAddress_in.ether_addr_octet);
//      NLA_PUT_U32 (message_p,
//                   NL80211_ATTR_WIPHY_FREQ,
//                   frequency_i);
      // NL80211_ATTR_CONTROL_PORT
      // NL80211_ATTR_CONTROL_PORT_ETHERTYPE
      // NL80211_ATTR_CONTROL_PORT_NO_ENCRYPT
      // NL80211_ATTR_MAC_HINT
      // NL80211_ATTR_WIPHY_FREQ_HINT
      // NL80211_ATTR_PREV_BSSID
    } // end IF
    NLA_PUT (message_p,
             NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
             //  and FT IEs..."
             0,
             NULL);

    result = nl_send_auto (inherited::handle_, message_p);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_send_auto(0x%@): \"%s\", returning\n"),
                  inherited::handle_,
                  ACE_TEXT (nl_geterror (result))));
      goto error;
    } // end IF

    // clean up
    nlmsg_free (message_p);
    message_p = NULL;
  } // end FOR

  return true;

error:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return false;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::do_scan (const std::string& interfaceIdentifier_in,
                                           const struct ether_addr& APMACAddress_in,
                                           const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::do_scan"));

  // sanity check(s)
  ACE_ASSERT (inherited::familyId_);
  ACE_ASSERT (inherited::handle_);

  int result = -1;
  unsigned int if_index_i = 0;
  struct nl_msg* nested_attribute_p = NULL;
  ACE_UINT32 scan_flags_i = 0;
  struct nl_msg* message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    return;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,              // port #
                              NL_AUTO_SEQ,               // sequence #
                              inherited::familyId_,      // family id
                              0,                         // (user-) hdrlen
//                              NLM_F_REQUEST | NLM_F_ACK, // flags
                              0,
                              NL80211_CMD_TRIGGER_SCAN,  // command id
                              0)))                       // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
    goto error;
  } // end IF
  if_index_i = ::if_nametoindex (interfaceIdentifier_in.c_str ());
  if (unlikely (!if_index_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto error;
  } // end IF
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_IFINDEX,
               static_cast<ACE_UINT32> (if_index_i));
  // *NOTE*: "...NL80211_ATTR_BSSID can be used to specify a BSSID to scan for;
  //         if not included, the wildcard BSSID will be used. ..."
  if (unlikely (!Net_Common_Tools::isAny (APMACAddress_in)))
    NLA_PUT (message_p,
             NL80211_ATTR_BSSID,
             ETH_ALEN,
             APMACAddress_in.ether_addr_octet);
  // *NOTE: "...If no SSID is passed, no probe requests are sent and a passive
  //         scan is performed. ..."
  nested_attribute_p = nlmsg_alloc ();
  if (unlikely (!nested_attribute_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    goto error;
  } // end IF
  NLA_PUT (nested_attribute_p,
           1,
           (SSID_in.empty () ? 0 : SSID_in.size ()),
           (SSID_in.empty () ? ACE_TEXT_ALWAYS_CHAR ("") : SSID_in.c_str ()));
  nla_put_nested (message_p,
                  NL80211_ATTR_SCAN_SSIDS,
                  nested_attribute_p);
  nlmsg_free (nested_attribute_p);
  nested_attribute_p = NULL;
//  NLA_PUT (message_p,
//           NL80211_ATTR_SSID,
//           SSID_in.size (),
//           SSID_in.c_str ());
  if (inherited::configuration_->lowPriorityScans &&
      hasFeature (NL80211_FEATURE_LOW_PRIORITY_SCAN,
                  MAX_NL80211_EXT_FEATURES))
    scan_flags_i |= NL80211_SCAN_FLAG_LOW_PRIORITY;
  if (inherited::configuration_->flushCacheBeforeScans &&
      hasFeature (NL80211_FEATURE_SCAN_FLUSH,
                  MAX_NL80211_EXT_FEATURES))
    scan_flags_i |= NL80211_SCAN_FLAG_FLUSH;
//  NL80211_SCAN_FLAG_AP
//  // *NOTE*: check features first; set NL80211_ATTR_MAC[, NL80211_ATTR_MAC_MASK]
  if (inherited::configuration_->randomizeMACAddressForScans &&
      (hasFeature (NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR,
                   MAX_NL80211_EXT_FEATURES) ||
       hasFeature (NL80211_FEATURE_ND_RANDOM_MAC_ADDR,
                   MAX_NL80211_EXT_FEATURES)))
    scan_flags_i |= NL80211_SCAN_FLAG_RANDOM_ADDR;
  // *TODO*: find out what these are
// NL80211_SCAN_FLAG_FILS_MAX_CHANNEL_TIME
// NL80211_SCAN_FLAG_ACCEPT_BCAST_PROBE_RESP;
// NL80211_SCAN_FLAG_OCE_PROBE_REQ_HIGH_TX_RATE;
// NL80211_SCAN_FLAG_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION;
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_SCAN_FLAGS,
               scan_flags_i);
  if (scan_flags_i & NL80211_SCAN_FLAG_RANDOM_ADDR)
  {
    // *NOTE*: "...the NL80211_ATTR_MAC and NL80211_ATTR_MAC_MASK attributes may
    //         also be given in which case only the masked bits will be
    //         preserved from the MAC address and the remainder randomised. If
    //         the attributes are not given full randomisation (46 bits, locally
    //         administered 1, multicast 0) is assumed. ..."
//    struct ether_addr interface_mac_address_s;
//    ACE_OS::memset (&interface_mac_address_s, 0, sizeof (struct ether_addr));
//    NLA_PUT (message_p,
//             NL80211_ATTR_MAC,
//             ETH_ALEN,
//             interface_mac_address_s.ether_addr_octet);
//    NLA_PUT (message_p,
//             NL80211_ATTR_MAC_MASK,
//             ETH_ALEN,
//             interface_mac_address_s.ether_addr_octet);
  } // end IF
  // *NOTE*: "...if passed, define which channels should be scanned; if not
  //          passed, all channels allowed for the current regulatory domain are
  //          used. ..."
//  NLA_PUT_U32 (message_p,
//               NL80211_ATTR_SCAN_FREQUENCIES,
//               authentication_type_i);
//  // *NOTE*: do not send probe requests at 'Complementary Code Keying' rate
//  nla_put_flag (message_p,
//                NL80211_ATTR_TX_NO_CCK_RATE);
//  NLA_PUT_U32 (message_p,
//               NL80211_ATTR_MEASUREMENT_DURATION,
//               0);
// NL80211_ATTR_MEASUREMENT_DURATION_MANDATORY
//  NLA_PUT_FLAG (message_p,
//                NL80211_ATTR_SCHED_SCAN_MULTI);
  NLA_PUT (message_p,
           NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
                            //  and FT IEs..."
           0,
           NULL);

  result = nl_send_auto (inherited::handle_, message_p);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto(0x%@): \"%s\", returning\n"),
                inherited::handle_,
                ACE_TEXT (nl_geterror (result))));
    goto error;
  } // end IF

  // clean up
  nlmsg_free (message_p);
  message_p = NULL;

  return;

error:
nla_put_failure:
  if (nested_attribute_p)
    nlmsg_free (nested_attribute_p);
  if (message_p)
    nlmsg_free (message_p);
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::handle_input"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::handle_);
  ACE_ASSERT (handle_in == nl_socket_get_fd (inherited::handle_));

  int result = -1;
  ssize_t bytes_received = -1;
  ACE_Message_Block* message_block_p =
      allocateMessage (inherited::configuration_->defaultBufferSize);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                inherited::configuration_->defaultBufferSize));
    return -1; // <-- remove 'this' from dispatch
  } // end IF
  struct sockaddr_nl address_s;
  ACE_OS::memset (&address_s, 0, sizeof (struct sockaddr_nl));
  int address_length_i = sizeof (struct sockaddr_nl);

  // read some data from the socket
retry:
  bytes_received =
    ACE_OS::recvfrom (handle_in,                                       // socket handle
                      message_block_p->wr_ptr (),                      // buffer
                      message_block_p->capacity (),                    // #bytes to read
                      0,                                               // flags
                      reinterpret_cast<struct sockaddr*> (&address_s), // address
                      &address_length_i);                              // address size
  switch (bytes_received)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if (error == EWOULDBLOCK)      // 11: SSL_read() failed (buffer is empty)
        goto retry;
      if ((error != EPIPE)        && // 32: connection reset by peer (write)
          // -------------------------------------------------------------------
          (error != EBADF)        && // 9
          (error != ENOTSOCK)     && // 88
          (error != ECONNABORTED) && // 103: connection abort()ed locally
          (error != ECONNRESET))     // 104: connection reset by peer (read)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::recvfrom(%d): \"%m\", aborting\n"),
                    handle_in));

      // clean up
      message_block_p->release ();

      return -1; // <-- remove 'this' from dispatch
    }
    // *** GOOD CASES ***
    case 0:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%d: socket was closed by the peer\n"),
                  handle_in));

      // clean up
      message_block_p->release ();

      return -1; // <-- remove 'this' from dispatch
    }
    default:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("[%d]: received %d bytes\n"),
//                  handle_in,
//                  bytes_received));

      // adjust write pointer
      message_block_p->wr_ptr (static_cast<size_t> (bytes_received));

      break;
    }
  } // end SWITCH

  // *NOTE*: fire-and-forget message_block_p
  processMessage (address_s,
                  message_block_p);

  // *NOTE*: do not deregister from the reactor
  return 0;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::handle_read_stream (const ACE_Asynch_Read_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::handle_read_stream"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ACE_Message_Block* message_block_p = NULL;
  struct sockaddr_nl address_s;
  ACE_OS::memset (&address_s, 0, sizeof (struct sockaddr_nl));

  // *IMPORTANT NOTE*: there is currently no way to retrieve the source address
  //                   when using the proactor implementation based on aio(7)
  // *TODO*: implement ACE_Asynch_Read_Dgram/Result specializations that support
  //         recvfrom(2) semantics

  if (unlikely (!result_in.success ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::read(%d,%u): \"%s\", returning\n"),
                result_in.handle (),
                result_in.bytes_to_read (),
                ACE_TEXT (ACE_OS::strerror (result_in.error ()))));
    return;
  } // end IF

  // make a shallow copy of the message buffer
  message_block_p = result_in.message_block ().duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", returning\n")));
    return;
  } // end IF

  // *NOTE*: fire-and-forget message_block_p
  processMessage (address_s,
                  message_block_p);

  if (unlikely (!initiate_read_stream (inherited::configuration_->defaultBufferSize)))
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Monitor_T::initiate_read_stream(%u), returning\n"),
                inherited::configuration_->defaultBufferSize));
    return;
  } // end IF
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::svc (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::svc"));

  int result = -1;
  int error = 0;

  { ACE_GUARD_RETURN (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_, -1);
    if (!inherited::dispatchStarted_)
    {
      inherited::dispatchStarted_ = true;
      goto state_machine;
    } // end IF
  } // end lock scope

//netlink_dispatch:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): netlink dispatch (thread id: %t, group: %d) starting\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  // sanity check(s)
  ACE_ASSERT (callbacks_);
  ACE_ASSERT (inherited::handle_);

  do
  { // *IMPORTANT NOTE*: there is no known way to cleanly unblock from a socket
    //                   read. Possible solutions:
    //                   - close()/shutdown() do not work
    //                   - SIGINT might work, ugly
    //                   - flag-setting and sending a 'dummy request', ugly
    //                   --> use ACE event dispatch and circumvent nl_revmsgs()
    result = nl_recvmsgs (inherited::handle_, callbacks_);
    if (unlikely (result < 0))
    {
      if (-result == NLE_BAD_SOCK)  // 3: shutdown (see below))
        break;
      else if (-result != NLE_BUSY) // 25
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\" (%d), returning\n"),
                    inherited::handle_,
                    ACE_TEXT (nl_geterror (result)), result));
        break;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", continuing\n"),
                  inherited::handle_,
                  ACE_TEXT (nl_geterror (result))));
    } // end IF
    error = ACE_OS::last_error ();
    if (unlikely (error &&
                  (error != EAGAIN))) // 11: socket is non-blocking, no data
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_recvmsgs(%@): \"%m\", returning\n"),
                  inherited::handle_));
      break;
    } // end IF
  } while (true);

  { ACE_GUARD_RETURN (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_, -1);
    inherited::dispatchStarted_ = false;
  } // end lock scope

  return result;

state_machine:
  result = inherited::svc ();

  // *NOTE*: the base class is not aware of the fact that not all task threads
  //         are dispatching the message queue
  //         --> shutdown manually:
  //         - reset the socket
  //         - flush the MB_STOP message
  // *NOTE*: nl_close calls close(), which, on Linux systems, does not (!)
  //         signal the thread blocked in recvmsg() (see above)
  // *NOTE*: apparently, shutdown(2) does not work on netlink sockets; it
  //         returns ENOTSUP
  //         --> reconnect using a fresh socket
//  int result_2 = ACE_OS::shutdown (nl_socket_get_fd (inherited::handle_),
//                                   SHUT_RDWR);
//  if (unlikely (result_2 == -1))
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_OS::shutdown(%d,%d): \"%m\", continuing\n"),
//                nl_socket_get_fd (inherited::handle_), SHUT_RDWR));
//  nl_close (inherited::handle_);
// nl_socket_disable_msg_peek (inherited::handle_);
//  nl_socket_set_buffer_size (inherited::handle_, int rx, int tx);
//  nl_socket_disable_auto_ack (inherited::handle_);

//  int result_2 = nl_connect (inherited::handle_, NETLINK_GENERIC);
//  if (unlikely (result_2 < 0))
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to nl_connect(%@): \"%s\", continuing\n"),
//                inherited::handle_,
//                ACE_TEXT (nl_geterror (result_2))));

  int result_2 = inherited::flush (ACE_Task_Flags::ACE_FLUSHALL);
//  if (unlikely (result_2 != 1))
  if (unlikely (result_2 != 0))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task_T::flush(): \"%m\", continuing\n")));
  result_2 = inherited::msg_queue_->activate ();
  if (unlikely (result_2 == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_MessageQueue_T::activate(): \"%m\", continuing\n")));
  inherited::dispatchStarted_ = false;

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
ACE_Message_Block*
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::allocateMessage (unsigned int bufferSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* return_value = NULL;

  ACE_NEW_NORETURN (return_value,
                    ACE_Message_Block (bufferSize_in,                      // size
                                       ACE_Message_Block::MB_DATA,         // type
                                       NULL,                               // cont
                                       NULL,                               // data
                                       NULL,                               // allocator_strategy
                                       NULL,                               // locking_strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution_time
                                       ACE_Time_Value::max_time,           // deadline_time
                                       NULL,                               // data_block_allocator
                                       NULL));                             // message_block_allocator
  if (unlikely (!return_value))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (was: %u byte(s)): \"%m\", aborting\n"),
                bufferSize_in));
    return NULL;
  } // end IF

  return return_value;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::processMessage (const struct sockaddr_nl& sourceAddress_in,
                                                  ACE_Message_Block*& messageBlock_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::processMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (messageBlock_inout);

  int result = -1;
  ACE_Message_Block* message_block_p, *message_block_2;
  int buffer_size;
  int err = 0, multipart = 0, interrupted = 0, nrecv = 0;
  struct nlmsghdr* nlmsghdr_p = NULL;
  //  struct ucred* ucred_p = NULL;
  unsigned int missing_bytes, bytes_to_copy, offset;

  // form a chain of buffers
  message_block_2 = buffer_;
  if (message_block_2)
  {
    while (message_block_2->cont ())
      message_block_2 = message_block_2->cont ();
    message_block_2->cont (messageBlock_inout);
  } // end IF
  else
    buffer_ = messageBlock_inout;
  ACE_ASSERT (buffer_);
  messageBlock_inout = NULL;

  // dissect nl80211 messages
next:
  // sanity check(s)
  buffer_size = buffer_->total_length ();
  if (buffer_size < NLMSG_HDRLEN)
    return; // need more data

  if (!message_)
  {
    message_ = nlmsg_alloc ();
    if (unlikely (!message_))
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", returning\n")));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (message_);
  nlmsghdr_p =  nlmsg_hdr (message_);
  ACE_ASSERT (nlmsghdr_p);

  // step1: assemble the message header ?
  if (headerReceived_)
    goto continue_;

  message_block_p = buffer_;
  missing_bytes = NLMSG_HDRLEN;
  bytes_to_copy = 0;
  offset = 0;
  do
  {
    bytes_to_copy = message_block_p->length ();
    if (likely (bytes_to_copy > missing_bytes))
      bytes_to_copy = missing_bytes;

    ACE_OS::memcpy (nlmsghdr_p + offset,
                    message_block_p->rd_ptr (),
                    bytes_to_copy);
    offset += bytes_to_copy;

    message_block_p->rd_ptr (bytes_to_copy);
    if (!message_block_p->length ())
      message_block_p = message_block_p->cont ();
    ACE_ASSERT (message_block_p);

    missing_bytes -= bytes_to_copy;
  } while (missing_bytes);
  buffer_size -= NLMSG_HDRLEN;

  headerReceived_ = true;

  // step2: assemble the message body
continue_:
  // sanity check(s)
  ACE_ASSERT (nlmsghdr_p);
  missing_bytes = nlmsg_datalen (nlmsghdr_p);
  if (missing_bytes > buffer_size)
    return; // need more data

  message_block_p = buffer_;
  while (!message_block_p->length ())
    message_block_p = message_block_p->cont ();
  bytes_to_copy = 0;
  offset = 0;
  do
  {
    bytes_to_copy = message_block_p->length ();
    if (likely (bytes_to_copy > missing_bytes))
      bytes_to_copy = missing_bytes;

    ACE_OS::memcpy (nlmsg_data (nlmsghdr_p) + offset,
                    message_block_p->rd_ptr (),
                    bytes_to_copy);
    offset += bytes_to_copy;

    message_block_p->rd_ptr (bytes_to_copy);
    if (!message_block_p->length ())
      message_block_p = message_block_p->cont ();

    missing_bytes -= bytes_to_copy;
  } while (missing_bytes);
  buffer_size -= nlmsg_datalen (nlmsghdr_p);

  // unlink fully processed head message buffer(s)
  if (message_block_p &&
      (message_block_p != buffer_))
  { // --> there is trailing data
    ACE_ASSERT (buffer_size);
    message_block_2 = buffer_;
    while (message_block_2->cont () != message_block_p)
      message_block_2 = message_block_2->cont ();
    ACE_ASSERT (message_block_2->cont ());
    message_block_2->cont (NULL);

    buffer_->release ();
    buffer_ = message_block_p;
  } // end IF
  else if (!message_block_p)
  { // --> there is no more data
    ACE_ASSERT (!buffer_size);
    buffer_->release ();
    buffer_ = NULL;
  } // end ELSE IF

  headerReceived_ = false;

  // step3: dispatch message
  // *NOTE*: this code is adapted from libnl nl.c
#define NL_CB_CALL(cb,type,msg) \
do { \
  err = nl_cb_call (cb, type, msg); \
  switch (err) { \
    case NL_OK: \
    case NL_STOP: \
        break; \
    case NL_SKIP: \
        goto continue_2; \
    default: \
        goto error; \
  } \
} while (0)

  nlmsg_set_proto (message_,
                   inherited::handle_->s_proto);
  nlmsg_set_src (message_,
                 &const_cast<struct sockaddr_nl&> (sourceAddress_in));
//    if (ucred_p)
//      nlmsg_set_creds (message_, ucred_p);

#if defined (_DEBUG)
  if (inherited::configuration_->debug)
    nl_msg_dump (message_, stderr);
#endif // _DEBUG

  nrecv++;

  /* Raw callback is the first, it gives the most control
     * to the user and he can do his very own parsing. */
  if (callbacks_->cb_set[NL_CB_MSG_IN])
    NL_CB_CALL (callbacks_, NL_CB_MSG_IN, message_);

  /* Sequence number checking. The check may be done by
     * the user, otherwise a very simple check is applied
     * enforcing strict ordering */
  if (callbacks_->cb_set[NL_CB_SEQ_CHECK]) {
    NL_CB_CALL (callbacks_, NL_CB_SEQ_CHECK, message_);

    /* Only do sequence checking if auto-ack mode is enabled */
  } else if (!(inherited::handle_->s_flags & NL_NO_AUTO_ACK)) {
    if (nlmsghdr_p->nlmsg_seq != inherited::handle_->s_seq_expect) {
      if (callbacks_->cb_set[NL_CB_INVALID])
        NL_CB_CALL (callbacks_, NL_CB_INVALID, message_);
      else {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid message sequence # (was: %d, expected: %d), returning\n"),
                    nlmsghdr_p->nlmsg_seq,
                    inherited::handle_->s_seq_expect));
        err = -NLE_SEQ_MISMATCH;
        goto error;
      } // end ELSE
    } // end IF
  }

  if (nlmsghdr_p->nlmsg_type == NLMSG_DONE     ||
      nlmsghdr_p->nlmsg_type == NLMSG_ERROR    ||
      nlmsghdr_p->nlmsg_type == NLMSG_NOOP     ||
      nlmsghdr_p->nlmsg_type == NLMSG_OVERRUN) {
    /* We can't check for !NLM_F_MULTI since some netlink
       * users in the kernel are broken. */
    inherited::handle_->s_seq_expect++;
  } // end IF

  if (nlmsghdr_p->nlmsg_flags & NLM_F_MULTI)
    multipart = 1;

  if (nlmsghdr_p->nlmsg_flags & NLM_F_DUMP_INTR) {
    if (callbacks_->cb_set[NL_CB_DUMP_INTR])
      NL_CB_CALL (callbacks_, NL_CB_DUMP_INTR, message_);
    else {
      /*
         * We have to continue reading to clear
         * all messages until a NLMSG_DONE is
         * received and report the inconsistency.
         */
      interrupted = 1;
    } // end ELSE
  } // end IF

  /* Other side wishes to see an ack for this message */
  if (nlmsghdr_p->nlmsg_flags & NLM_F_ACK) {
    if (callbacks_->cb_set[NL_CB_SEND_ACK])
      NL_CB_CALL (callbacks_, NL_CB_SEND_ACK, message_);
    else {
      /* FIXME: implement */
    }
  } // end IF

  /* messages terminates a multipart message, this is
     * usually the end of a message and therefore we slip
     * out of the loop by default. the user may overrule
     * this action by skipping this packet. */
  if (nlmsghdr_p->nlmsg_type == NLMSG_DONE) {
    multipart = 0;
    if (callbacks_->cb_set[NL_CB_FINISH])
      NL_CB_CALL (callbacks_, NL_CB_FINISH, message_);
  } // end IF

  /* Message to be ignored, the default action is to
     * skip this message if no callback is specified. The
     * user may overrule this action by returning
     * NL_PROCEED. */
  else if (nlmsghdr_p->nlmsg_type == NLMSG_NOOP) {
    if (callbacks_->cb_set[NL_CB_SKIPPED])
      NL_CB_CALL (callbacks_, NL_CB_SKIPPED, message_);
    else
      goto continue_2;
  }

  /* Data got lost, report back to user. The default action is to
     * quit parsing. The user may overrule this action by retuning
     * NL_SKIP or NL_PROCEED (dangerous) */
  else if (nlmsghdr_p->nlmsg_type == NLMSG_OVERRUN) {
    if (callbacks_->cb_set[NL_CB_OVERRUN])
      NL_CB_CALL (callbacks_, NL_CB_OVERRUN, message_);
    else {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("buffer overrun, returning\n")));
      err = -NLE_MSG_OVERFLOW;
      goto error;
    } // end ELSE
  }

  /* Message carries a nlmsgerr */
  else if (nlmsghdr_p->nlmsg_type == NLMSG_ERROR) {
    struct nlmsgerr* e =
        reinterpret_cast<struct nlmsgerr*> (nlmsg_data (nlmsghdr_p));

    if (nlmsghdr_p->nlmsg_len < nlmsg_size (sizeof (*e))) {
      /* Truncated error message, the default action
         * is to stop parsing. The user may overrule
         * this action by returning NL_SKIP or
         * NL_PROCEED (dangerous) */
      if (callbacks_->cb_set[NL_CB_INVALID])
        NL_CB_CALL (callbacks_, NL_CB_INVALID, message_);
      else {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("truncated message, returning\n")));
        err = -NLE_MSG_TRUNC;
        goto error;
      } // end ELSE
    } else if (e->error) {
      /* Error message reported back from kernel. */
      if (callbacks_->cb_err) {
        err =
            callbacks_->cb_err (&const_cast<struct sockaddr_nl&> (sourceAddress_in),
                                e,
                                callbacks_->cb_err_arg);
        if (err < 0)
          goto error;
        else if (err == NL_SKIP)
          goto continue_2;
        else if (err == NL_STOP) {
          err = -nl_syserr2nlerr (e->error);
          goto error;
        }
      } else {
        err = -nl_syserr2nlerr (e->error);
        goto error;
      } // end ELSE
    } else if (callbacks_->cb_set[NL_CB_ACK])
      NL_CB_CALL (callbacks_, NL_CB_ACK, message_);
  } else {
    /* Valid message (not checking for MULTIPART bit to
       * get along with broken kernels. NL_SKIP has no
       * effect on this.  */
    if (callbacks_->cb_set[NL_CB_VALID])
      NL_CB_CALL (callbacks_, NL_CB_VALID, message_);
  } // end ELSE

continue_2:
  err = 0;
  nlmsg_free (message_);
  message_ = NULL;

  if (buffer_)
    goto next;

  return;

error:
  if (message_)
    nlmsg_free (message_);
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::initiate_read_stream (unsigned int bufferSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initiate_read_stream"));

  // step1: allocate a message buffer
  ACE_Message_Block* message_block_p = allocateMessage (bufferSize_in);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate message (was: %u byte(s)): \"%m\", aborting\n"),
                bufferSize_in));
    return false;
  } // end IF

  // step2: start (asynchronous) read
//  size_t bytes_to_read = message_block_p->size ();
  int result =
      inputStream_.read (*message_block_p,                     // buffer
                         message_block_p->size (),             // buffer size
                         NULL,                                 // ACT
                         0,                                    // priority
                         COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
//      inputStream_.recv (message_block_p,                      // buffer
//                         message_block_p->size (),             // buffer size
//                         0,                                    // flags
//                         ACE_PROTOCOL_FAMILY_NETLINK,          // protocol family
//                         NULL,                                 // ACT
//                         0,                                    // priority
//                         COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::read(%u): \"%m\", aborting\n"),
//                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::recv(%u): \"%m\", aborting\n"),
                bufferSize_in));

    // clean up
    message_block_p->release ();

    return false;
  } // end IF

  return true;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::hasFeature (enum nl80211_feature_flags feature_in,
                                              enum nl80211_ext_feature_index extendedFeature_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::hasFeature"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (feature_in || (extendedFeature_in < MAX_NL80211_EXT_FEATURES));

  if (likely (feature_in))
    return (features_ & static_cast<ACE_UINT32> (feature_in));

  return (extendedFeatures_.find (extendedFeature_in) != extendedFeatures_.end ());
}
#endif // NL80211_SUPPORT

//////////////////////////////////////////

#if defined (DBUS_SUPPORT)
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
    inherited::stop (true,
                     true);
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
  ACE_ASSERT (objectPathCache_.empty ());
  Net_InterfaceIdentifiers_t interface_identifiers_a;
  std::string device_object_path;
  // convert device identifier to object path ?
  if (!inherited::configuration_->interfaceIdentifier.empty ())
    interface_identifiers_a.push_back (inherited::configuration_->interfaceIdentifier);
  else
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces ();
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    device_object_path =
        Net_WLAN_Tools::deviceToDBusPath (connection_,
                                          *iterator);
    if (unlikely (device_object_path.empty ()))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::deviceToDBusPath(\"%s\"), continuing\n"),
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
                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  if (!inherited::isActive_)
    return;

  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);
//  deviceDBusPath_.resize (0);

  inherited::isActive_ = false;
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
                   UserDataType>::get1R (const std::string& value_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::get1R"));

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
  Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t iterator =
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
          Net_WLAN_Tools::deviceToDBusPath (connection_,
                                            (*iterator).second.first);
      if (unlikely (device_object_path_string.empty ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::deviceToDBusPath(0x%@,\"%s\"), aborting\n"),
                    connection_,
                    ACE_TEXT ((*iterator).second.first.c_str ())));
        return false;
      } // end IF
    } // end ELSE
  } // end IF
  else
  {
    device_object_path_string =
        Net_WLAN_Tools::SSIDToDeviceDBusPath (connection_,
                                              SSID_in);
    if (unlikely (device_object_path_string.empty ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToDeviceDBusPath(0x%@,%s), aborting\n"),
                  connection_,
                  ACE_TEXT (SSID_in.c_str ())));
      return false;
    } // end IF
  } // end ELSE
  ACE_ASSERT (!device_object_path_string.empty ());

  // step2: retrieve corresponding access point object path
  std::string access_point_object_path_string =
      Net_WLAN_Tools::SSIDToAccessPointDBusPath (connection_,
                                                 device_object_path_string,
                                                 SSID_in);
  if (unlikely (access_point_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToAccessPointDBusPath(0x%@,\"%s\",%s), aborting\n"),
                connection_,
                ACE_TEXT (device_object_path_string.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
    return false;
  } // end IF

  // step3: retrieve available and matching connection profile(s)
  std::string connection_object_path_string =
      Net_WLAN_Tools::SSIDToConnectionDBusPath (connection_,
                                                device_object_path_string,
                                                SSID_in);
  if (unlikely (connection_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::SSIDToConnectionDBusPath(0x%@,\"%s\",%s), aborting\n"),
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
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces ();
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
    inherited::dispatchStarted_ = false;
  } // end lock scope

  return 0;

monitor_thread:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): monitor (group: %d, thread id: %t) starting...\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (!inherited::configuration_->SSID.empty ());

  int result = -1;
  ACE_Time_Value interval (1, 0);
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value now = COMMON_TIME_NOW;
  do
  {
    if (inherited::configuration_->SSID != Net_WLAN_Tools::associatedSSID (inherited::configuration_->interfaceIdentifier,
                                                                           inherited::handle_))
    {
      // SSID not found ? --> initiate scan
      if (unlikely (!Net_WLAN_Tools::hasSSID (inherited::configuration_->interfaceIdentifier,
                                              inherited::configuration_->SSID)))
      {
        Net_WLAN_Tools::scan (inherited::configuration_->interfaceIdentifier,
                              inherited::configuration_->SSID,
                              inherited::handle_,
                              false);
        goto sleep;
      } // end IF

      struct ether_addr ap_mac_address_s;
      if (unlikely (!inherited::associate (inherited::configuration_->interfaceIdentifier,
                                           ap_mac_address_s,
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
      message_block_p->release ();

      break; // done
    } // end IF
    message_block_p->release ();
    message_block_p = NULL;
  } while (true);

  return 0;
}
#endif /* DBUS_SUPPORT */
#endif /* ACE_WIN32 || ACE_WIN64 */
