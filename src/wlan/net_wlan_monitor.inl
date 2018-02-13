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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net/ethernet.h"

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

#include "common_timer_tools.h"
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Time_Value.h"

#include "common_tools.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_timer_manager_common.h"
#endif

#include "stream_defines.h"

#include "net_common_tools.h"
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , range_ ()
#endif // ACE_WIN32 || ACE_WIN64
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

//  inherited::reactor (ACE_Reactor::instance ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_OS::memset (&range_, 0, sizeof (struct iw_range));

//#if defined (DHCLIENT_SUPPORT)
//  isc_result_t status_i = dhcpctl_initialize ();
//  if (status_i != ISC_R_SUCCESS)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ::dhcpctl_initialize(): \"%s\", continuing\n"),
//                ACE_TEXT (isc_result_totext (status_i))));
//#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
}

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
                   UserDataType>::~Net_WLAN_Monitor_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::~Net_WLAN_Monitor_T"));

}

template <typename AddressType,
          typename ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
          ACE_SYNCH_DECL,
          typename TimePolicyType,
#endif
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                   ACE_SYNCH_USE,
                   TimePolicyType,
#endif
                   MonitorAPI_e,
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (inherited::clientHandle_ == ACE_INVALID_HANDLE);

  if (unlikely (!Net_WLAN_Tools::initialize (inherited::clientHandle_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), returning\n")));
    return;
  } // end IF
  ACE_ASSERT (inherited::clientHandle_ != ACE_INVALID_HANDLE);
#endif // ACE_WIN32 || ACE_WIN64

  Net_InterfaceIdentifiers_t interface_identifiers_a =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Net_WLAN_Tools::getInterfaces (inherited::clientHandle_);
#else
      Net_WLAN_Tools::getInterfaces ();
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _WLAN_INTERFACE_INFO_LIST* interface_list_p = NULL;
  DWORD notification_mask = WLAN_NOTIFICATION_SOURCE_ALL;
  DWORD previous_notification_mask = 0;
  DWORD result = WlanRegisterNotification (inherited::clientHandle_,
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
  if (inherited::clientHandle_ != ACE_INVALID_HANDLE)
    Net_WLAN_Tools::initialize (inherited::clientHandle_);
  inherited::clientHandle_ = ACE_INVALID_HANDLE;

  return;

continue_:
#else
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

  int result = iw_get_range_info (inherited::handle_,
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
      break;
  } while (!inherited::isRunning ());
  if (unlikely (!inherited::isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", returning\n")));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  inherited::isActive_ = true;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
}

template <typename AddressType,
          typename ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
          ACE_SYNCH_DECL,
          typename TimePolicyType,
#endif
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                   ACE_SYNCH_USE,
                   TimePolicyType,
#endif
                   MonitorAPI_e,
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  if (!inherited::isActive_)
    return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (inherited::clientHandle_ != ACE_INVALID_HANDLE);
  if (inherited::scanTimerId_ != -1)
    cancelScanTimer ();
  Net_WLAN_Tools::finalize (inherited::clientHandle_);
  inherited::clientHandle_ = ACE_INVALID_HANDLE;
#else
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
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inherited::isActive_ = false;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_INITIALIZED);
#endif
}

template <typename AddressType,
          typename ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
          ACE_SYNCH_DECL,
          typename TimePolicyType,
#endif
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                   ACE_SYNCH_USE,
                   TimePolicyType,
#endif
                   MonitorAPI_e,
                   UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    ACE_OS::memset (&range_, 0, sizeof (struct iw_range));
#endif // ACE_WIN32 || ACE_WIN64

    userData_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  userData_ = configuration_in.userData;

  return inherited::initialize (configuration_in);
}

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
//      Net_WLAN_Tools::scan (clientHandle_,
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
//                            handle_,
//                            false); // don't wait

//      int result = -1;
//      int error = 0;
//      ACE_Time_Value result_poll_interval (0,
//                                           NET_WLAN_MONITOR_SCAN_DEFAULT_RESULT_POLL_INTERVAL * 1000);
//fetch_scan_result_data:
//      ACE_OS::last_error (0);
//      // *TODO*: implement nl80211 support, rather than polling
//      result = handle_input (handle_);
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
//        if (unlikely (!Net_WLAN_Tools::disassociate (clientHandle_,
//                                                     interface_identifier)))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to Net_WLAN_Tools::disassociate(0x%@,\"%s\"), continuing\n"),
//                      clientHandle_,
//                      ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
//#else
//        if (unlikely (!Net_WLAN_Tools::disassociate (interface_identifier_string,
//                                                     handle_)))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to Net_WLAN_Tools::disassociate(\"%s\",%d), continuing\n"),
//                      ACE_TEXT (interface_identifier_string.c_str ()),
//                      handle_));
//#endif // ACE_WIN32 || ACE_WIN64
//      } // end IF

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (unlikely (!Net_WLAN_Tools::associate (clientHandle_,
//                                                interface_identifier,
//                                                configuration_->SSID)))
//#else
//associate:
//      if (unlikely (!Net_WLAN_Tools::associate (interface_identifier_string,
//                                                ap_mac_address,
//                                                configuration_->SSID,
//                                                handle_)))
//#endif // ACE_WIN32 || ACE_WIN64
//      {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Net_WLAN_Tools::associate(0x%@,\"%s\",%s), returning\n"),
//                    clientHandle_,
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
//                    ACE_TEXT (configuration_->SSID.c_str ())));
//#else
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Net_WLAN_Tools::associate(\"%s\",%s,%s,%d), returning\n"),
//                    ACE_TEXT (interface_identifier_string.c_str ()),
//                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
//                    ACE_TEXT (configuration_->SSID.c_str ()),
//                    handle_));
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
//                                           handle_);
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
//      ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
//      Net_WLAN_Tools::disassociate (clientHandle_,
//                                    configuration_->interfaceIdentifier);
//#else
//      ACE_ASSERT (handle_ != ACE_INVALID_HANDLE);
//      Net_WLAN_Tools::disassociate (configuration_->interfaceIdentifier,
//                                    handle_);
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
//      if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
//                                                           configuration_->interfaceIdentifier,
//                                                           wlan_intf_opcode_autoconf_enabled,
//                                                           configuration_->enableAutoConf)))
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
//                    clientHandle_,
//                    wlan_intf_opcode_autoconf_enabled,
//                    (configuration_->enableAutoConf ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
//      if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
//                                                           configuration_->interfaceIdentifier,
//                                                           wlan_intf_opcode_background_scan_enabled,
//                                                           configuration_->enableBackgroundScans)))
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
//                    clientHandle_,
//                    wlan_intf_opcode_background_scan_enabled,
//                    (configuration_->enableBackgroundScans ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
//      if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
//                                                           configuration_->interfaceIdentifier,
//                                                           wlan_intf_opcode_media_streaming_mode,
//                                                           configuration_->enableMediaStreamingMode)))
//        ACE_DEBUG ((LM_ERROR,NET_WLAN_MONITOR_STATE_DISASSOCIATE
//                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
//                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
//                    clientHandle_,
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <typename AddressType,
          typename ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
          ACE_SYNCH_DECL,
          typename TimePolicyType,
#endif
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
int
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                   ACE_SYNCH_USE,
                   TimePolicyType,
#endif
                   MonitorAPI_e,
                   UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::handle_input"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (handle_in == inherited::handle_);

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
  result = ACE_OS::ioctl (handle_in,
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
                  handle_in));
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
#endif

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
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
 /////////////////////////////////////////
 , DBusDispatchStarted_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

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

    DBusDispatchStarted_ = false;
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
                   UserDataType>::do_scan (const std::string& interfaceIdentifier_in)
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
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (!inherited::configuration_->SSID.empty ());

  int result = -1;
  ACE_Time_Value interval (1, 0);
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value now = COMMON_TIME_NOW;
  do
  {
    if (inherited::configuration_->SSID != Net_WLAN_Tools::associatedSSID (inherited::configuration_->interfaceIdentifier))
    {
      // SSID not found ? --> initiate scan
      if (unlikely (!Net_WLAN_Tools::hasSSID (inherited::configuration_->interfaceIdentifier,
                                              inherited::configuration_->SSID)))
      {
        Net_WLAN_Tools::scan (inherited::configuration_->interfaceIdentifier);
        goto sleep;
      } // end IF

      if (unlikely (!associate (inherited::configuration_->interfaceIdentifier,
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
