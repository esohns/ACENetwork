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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
              0,                                                   // # thread(s)
#else
              1,                                                   // # thread(s)
#endif
              false,                                               // auto-start ?
              ////////////////////////////
              //NULL)                                             // queue handle
              // *TODO*: this looks dodgy, but seems to work nonetheless...
              &queue_)                                            // queue handle
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , clientHandle_ (ACE_INVALID_HANDLE)
 , scanTimerId_ (-1)
 , timerHandler_ (this,
                  false)
 , timerInterface_ (NULL)
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
 , isConnectionNotified_ (false)
 , isFirstConnect_ (true)
 , isInitialized_ (false)
 , localSAP_ ()
 , peerSAP_ ()
 , retries_ (0)
 , SSIDsToInterfaceIdentifier_ ()
 , subscribersLock_ ()
 , subscribers_ ()
 , userData_ (NULL)
 /////////////////////////////////////////
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
 , SSIDSeenBefore_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

//  inherited::reactor (ACE_Reactor::instance ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_OS::memset (&range_, 0, sizeof (struct iw_range));

#if defined (DHCLIENT_SUPPORT)
  isc_result_t status_i = dhcpctl_initialize ();
  if (status_i != ISC_R_SUCCESS)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_initialize(): \"%s\", continuing\n"),
                ACE_TEXT (isc_result_totext (status_i))));
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
  if (scanTimerId_ != -1)
    cancelScanTimer ();
  if (unlikely (clientHandle_ != ACE_INVALID_HANDLE))
    Net_WLAN_Tools::finalize (clientHandle_);
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
#endif // ACE_WIN32 || ACE_WIN64
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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

  if (unlikely (!Net_WLAN_Tools::initialize (clientHandle_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), returning\n")));
    return;
  } // end IF
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
#endif // ACE_WIN32 || ACE_WIN64

  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    interfaceIdentifiers_ = Net_WLAN_Tools::getInterfaces (clientHandle_);
#else
    interfaceIdentifiers_ = Net_WLAN_Tools::getInterfaces ();
#endif // ACE_WIN32 || ACE_WIN64
  } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _WLAN_INTERFACE_INFO_LIST* interface_list_p = NULL;
  DWORD notification_mask = WLAN_NOTIFICATION_SOURCE_ALL;
  DWORD previous_notification_mask = 0;
  DWORD result = WlanRegisterNotification (clientHandle_,
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
    Net_WLAN_Tools::initialize (clientHandle_);
  clientHandle_ = ACE_INVALID_HANDLE;

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

  isActive_ = true;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inherited2::change (NET_WLAN_MONITOR_STATE_IDLE);
#endif // ACE_WIN32 || ACE_WIN64
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
  if (scanTimerId_ != -1)
    cancelScanTimer ();
  Net_WLAN_Tools::finalize (clientHandle_);
  clientHandle_ = ACE_INVALID_HANDLE;
#else
  isActive_ = false;
  if (waitForCompletion_in)
    inherited::wait (true);

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
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  isActive_ = false;

  inherited2::change (NET_WLAN_MONITOR_STATE_INITIALIZED);
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
      stop (true,  // wait ?
            true); // N/A
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_ASSERT (clientHandle_ == ACE_INVALID_HANDLE);
    ACE_ASSERT (scanTimerId_ == -1);
    timerInterface_ = NULL;
#else
    if (buffer_)
    {
      ACE_OS::free (buffer_);
      buffer_ = NULL;
    } // end IF
    bufferSize_ = 0;
    ACE_ASSERT (handle_ == ACE_INVALID_HANDLE);
    ACE_OS::memset (&range_, 0, sizeof (struct iw_range));
#endif // ACE_WIN32 || ACE_WIN64

    configuration_ = NULL;
    ACE_ASSERT (!isActive_);
    isConnectionNotified_ = false;
    isFirstConnect_ = true;
    localSAP_.reset ();
    peerSAP_.reset ();

    { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
      interfaceIdentifiers_.clear ();
      SSIDsToInterfaceIdentifier_.clear ();
      subscribers_.clear ();
    } // end lock scope

    userData_ = NULL;
    unsigned int result = queue_.flush (false);
    ACE_UNUSED_ARG (result);
    SSIDSeenBefore_ = false;

    isInitialized_ = false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (configuration_in.timerInterface);
  //if (configuration_in.timerInterface)
    timerInterface_ = configuration_in.timerInterface;
  //else
  //{
  //  ACE_DEBUG ((LM_WARNING,
  //              ACE_TEXT ("timer manager interface handle not specified, using default implementation\n")));
  //  timerInterface_ = COMMON_TIMERMANAGER_SINGLETON::instance ();
  //} // end ELSE
#endif
  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool subscribe_this = false;
  if (!configuration_->notificationCB)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("WLAN API notification callback not specified, using default implementation\n")));
    configuration_->notificationCB = network_wlan_default_notification_cb;
    configuration_->notificationCBData =
        static_cast<Net_WLAN_IMonitorCB*> (this);
  } // end IF
  else
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("WLAN API notification callback specified; disabled default event subscription, subscribing 'this'\n")));
    subscribe_this = true;
  } // end ELSE
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (configuration_->SSID.size () > DOT11_SSID_MAX_LENGTH))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid configuration, aborting\n")));
    return false;
  } // end IF
  if (subscribe_this)
    subscribe (this);
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (configuration_->subscriber))
    subscribe (configuration_->subscriber);
  // *TODO*: remove type inference
  userData_ = configuration_->userData;

  isInitialized_ = true;

  inherited2::initialize ();

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
const WLAN_SIGNAL_QUALITY
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::get_2 () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::get_2"));

  // sanity check(s)
  if (unlikely (!isActive_))
    return 0;
  std::string SSID_string = SSID ();
  if (unlikely (SSID_string.empty ()))
    return 0;

  SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator;
  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
    iterator = SSIDsToInterfaceIdentifier_.find (SSID_string);
    if (likely (iterator != SSIDsToInterfaceIdentifier_.end ()))
      return ((*iterator).second.second.signalQuality);
  } // end lock scope

  return 0;
}
#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
                                             const struct ether_addr& accessPointLinkLayerAddress_in,
#endif
                                             const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::associate"));

  bool result = false;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_UNUSED_ARG (accessPointLinkLayerAddress_in);
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
  bool restart = isActive_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID interface_identifier = interfaceIdentifier_in;
#else
  std::string interface_identifier = interfaceIdentifier_in;
#endif // ACE_WIN32 || ACE_WIN64
  std::string current_ssid_s = SSID ();
  if (likely (!SSID_in.empty ()))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_ASSERT (SSID_in.size () <= DOT11_SSID_MAX_LENGTH);
#else
    ACE_ASSERT (SSID_in.size () <= IW_ESSID_MAX_SIZE);
#endif // ACE_WIN32 || ACE_WIN64

    if (unlikely (!ACE_OS::strcmp (SSID_in.c_str (),
                                   current_ssid_s.c_str ())))
      goto configure;
  } // end IF
  else
  {
    if (unlikely (current_ssid_s.empty ()))
      goto configure;
  } // end ELSE

  // check cache ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
#else
  if (unlikely (interface_identifier.empty ()))
#endif // ACE_WIN32 || ACE_WIN64
  {
    // check cache
    SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator_2;
    { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
      iterator_2 = SSIDsToInterfaceIdentifier_.find (SSID_in);
      if (iterator_2 != SSIDsToInterfaceIdentifier_.end ())
      {
#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("SSID (was: %s) detected on interface \"%s\", continuing\n"),
                    ACE_TEXT (SSID_in.c_str ()),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("SSID (was: %s) detected on interface \"%s\", continuing\n"),
                    ACE_TEXT (SSID_in.c_str ()),
                    ACE_TEXT (interface_identifier.c_str ())));
#endif
#endif
        interface_identifier = (*iterator_2).second.first;
      } // end IF
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("SSID (was: %s) not detected (yet); cannot auto-select interface, continuing\n"),
                    ACE_TEXT (SSID_in.c_str ())));
    } // end lock scope
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (!InlineIsEqualGUID (interface_identifier, GUID_NULL));
#else
  ACE_ASSERT (!interface_identifier.empty ());
#endif // ACE_WIN32 || ACE_WIN64

configure:
  if (likely (restart))
    stop (true,  // wait ?
          true); // N/A

  // reconfigure ?
  if (likely (configuration_))
  {
    configuration_->autoAssociate = !SSID_in.empty ();
    configuration_->interfaceIdentifier = interface_identifier;
    configuration_->SSID = SSID_in;
    isConnectionNotified_ = false;
  } // end IF

  if (unlikely (!restart))
  { // *NOTE*: not active
    struct ether_addr ether_addr_s;
    ACE_OS::memset (&ether_addr_s, 0, sizeof (struct ether_addr));
    if (likely (!SSID_in.empty ()))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = Net_WLAN_Tools::associate (clientHandle_,
                                          interface_identifier,
                                          SSID_in);
#else
      result = Net_WLAN_Tools::associate (interface_identifier,
                                          ether_addr_s,
                                          SSID_in,
                                          handle_);
#endif // ACE_WIN32 || ACE_WIN64
    else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = Net_WLAN_Tools::disassociate (clientHandle_,
                                             interface_identifier);
#else
      result = Net_WLAN_Tools::disassociate (interface_identifier,
                                             handle_);
#endif // ACE_WIN32 || ACE_WIN64
    goto end;
  } // end ELSE

  if (likely (restart))
    start ();

  result = true;

end:
  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::scan (REFGUID interfaceIdentifier_in)
#else
                   UserDataType>::scan (const std::string& interfaceIdentifier_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::scan"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  if (unlikely (!isActive_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not running, returning\n")));
    return;
  } // end IF
  std::string current_ssid_s = SSID ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL) &&
                InlineIsEqualGUID (interfaceIdentifier_in,
                                   configuration_->interfaceIdentifier)))
#else
  if (unlikely (!interfaceIdentifier_in.empty () &&
                !ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
                                 configuration_->interfaceIdentifier.c_str ())))
#endif // ACE_WIN32 || ACE_WIN64
    if (!configuration_->autoAssociate &&
        current_ssid_s.empty ())
      return; // --> nothing to do

  stop (true,  // wait ?
        true); // N/A

  // reconfigure
  // *TODO*: support monitoring multiple/all interfaces at the same time
  Net_InterfaceIdentifiers_t interface_identifiers;
  SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator_2;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID interface_identifier = interfaceIdentifier_in;
#else
  std::string interface_identifier = interfaceIdentifier_in;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (InlineIsEqualGUID (interface_identifier, GUID_NULL))
#else
  if (interface_identifier.empty ())
#endif // ACE_WIN32 || ACE_WIN64
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    interface_identifiers = interfaceIdentifiers_;
  } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (InlineIsEqualGUID (interface_identifier, GUID_NULL))
#else
  if (interface_identifier.empty ())
#endif // ACE_WIN32 || ACE_WIN64
  {
    if (interface_identifiers.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no interfaces found; cannot auto-select, returning\n")));
      return;
    } // end IF
    interface_identifier = interface_identifiers.front ();
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (!InlineIsEqualGUID (interface_identifier, GUID_NULL));
#else
  ACE_ASSERT (!interface_identifier.empty ());
#endif // ACE_WIN32 || ACE_WIN64

  if (!current_ssid_s.empty ())
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Net_WLAN_Tools::disassociate (clientHandle_,
                                  configuration_->interfaceIdentifier);
#else
    Net_WLAN_Tools::disassociate (configuration_->interfaceIdentifier,
                                  handle_);
#endif // ACE_WIN32 || ACE_WIN64

  configuration_->autoAssociate = false;
  configuration_->interfaceIdentifier = interface_identifier;
  configuration_->SSID.clear ();

  start ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return Net_WLAN_Tools::associatedSSID (clientHandle_,
                                         (configuration_ ? configuration_->interfaceIdentifier : GUID_NULL));
#else
  return Net_WLAN_Tools::associatedSSID ((configuration_ ? configuration_->interfaceIdentifier : ACE_TEXT_ALWAYS_CHAR ("")),
                                         handle_);
#endif // ACE_WIN32 || ACE_WIN64
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
#endif // ACE_WIN32 || ACE_WIN64
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
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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

  switch (newState_in)
  {
    case NET_WLAN_MONITOR_STATE_INVALID:
    {
      ACE_ASSERT (false);
      ACE_NOTSUP;
      ACE_NOTREACHED (break;)
    }
    case NET_WLAN_MONITOR_STATE_IDLE:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_IDLE;
      } // end lock scope

      if (!isActive_ ||              // stop()ped
          inherited::hasShutDown ())
      {
        inherited2::change (NET_WLAN_MONITOR_STATE_INITIALIZED);
        break;
      } // end IF
      std::string SSID_string = SSID ();
      if (!SSID_string.empty ()) // connected
      {
        if (!isFirstConnect_)
          goto continue_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        // sanity check(s)
        ACE_ASSERT (configuration_);

        try {
          onAssociate (configuration_->interfaceIdentifier,
                       SSID_string,
                       true);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(\"%s\",%s,true), continuing\n"),
                      ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                      ACE_TEXT (SSID_string.c_str ())));
        }
#else
        inherited2::change (NET_WLAN_MONITOR_STATE_CONNECTED);
        break;

continue_:
        inherited2::change (NET_WLAN_MONITOR_STATE_SCAN);
#endif // ACE_WIN32 || ACE_WIN64
        break;
      } // end IF

      // not connected

      // sanity check(s)
      ACE_ASSERT (configuration_);

      bool essid_is_cached = false;
      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
            SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
        essid_is_cached = (iterator != SSIDsToInterfaceIdentifier_.end ());
        if (unlikely (!SSIDSeenBefore_ && essid_is_cached))
          SSIDSeenBefore_ = true;
      } // end lock scope

      // *NOTE*: there is no need to reset the state when returning from a
      //         transitional state, as the current state is not set until this
      //         method returns. Note that this holds only iff the state hasn't
      //         already changed; see: common_state_machine_base.inl:214
      //         --> reset the state just in case

      if (configuration_->SSID.empty () || // not configured
          !essid_is_cached              || // configured SSID unknown (i.e. not cached yet)
          !configuration_->autoAssociate)  // auto-associate disabled
      {
        inherited2::change (NET_WLAN_MONITOR_STATE_SCAN);
        goto reset_state;
      } // end IF

      ACE_ASSERT (SSID_string.empty () && !configuration_->SSID.empty () && essid_is_cached && configuration_->autoAssociate);

      inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);

reset_state:
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_IDLE;
      } // end lock scope

      break;
    }
    case NET_WLAN_MONITOR_STATE_SCAN:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_SCAN;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);

      // scan (and wait/poll for results)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": [%T] scanning...\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ())));
#endif // _DEBUG
      Net_WLAN_Tools::scan (clientHandle_,
                            configuration_->interfaceIdentifier,
                            (SSIDSeenBefore_ ? configuration_->SSID
                                             : ACE_TEXT_ALWAYS_CHAR ("")));
#else
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": [%T] scanning...\n"),
                  ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
#endif // _DEBUG
      Net_WLAN_Tools::scan (configuration_->interfaceIdentifier,
                            (SSIDSeenBefore_ ? configuration_->SSID
                                             : ACE_TEXT_ALWAYS_CHAR ("")),
                            handle_,
                            false); // don't wait

      int result = -1;
      int error = 0;
      ACE_Time_Value result_poll_interval (0,
                                           NET_WLAN_MONITOR_SCAN_DEFAULT_RESULT_POLL_INTERVAL * 1000);
fetch_scan_result_data:
      ACE_OS::last_error (0);
      // *TODO*: implement nl80211 support, rather than polling
      result = handle_input (handle_);
      ACE_UNUSED_ARG (result);
      error = ACE_OS::last_error ();
      if (error == EAGAIN) // 11: result data not available yet
      {
        result = ACE_OS::sleep (result_poll_interval);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                      &result_poll_interval));
        if (inherited::hasShutDown ())
          break;
        goto fetch_scan_result_data;
      } // end IF

      inherited2::change (NET_WLAN_MONITOR_STATE_SCANNED);
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATE:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATE;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);
      ACE_ASSERT (!configuration_->SSID.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _GUID interface_identifier = GUID_NULL;
#else
      std::string interface_identifier_string;
      struct ether_addr ap_mac_address, ether_addr_s;
      bool result = false;
      int result_2 = -1;
      ACE_Time_Value result_poll_interval (0,
                                           NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL * 1000);
      ACE_Time_Value result_timeout (NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT,
                                     0);
      ACE_Time_Value deadline;
      bool shutdown = false;
#endif // ACE_WIN32 || ACE_WIN64
      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
          SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
        ACE_ASSERT (iterator != SSIDsToInterfaceIdentifier_.end ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        interface_identifier = (*iterator).second.first;
#else
        interface_identifier_string = (*iterator).second.first;
        ap_mac_address = (*iterator).second.second.accessPointLinkLayerAddress;
#endif // ACE_WIN32 || ACE_WIN64
      } // end lock scope
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (!InlineIsEqualGUID (interface_identifier,
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
#endif // ACE_WIN32 || ACE_WIN64

      std::string SSID_string = SSID ();
      if (!SSID_string.empty () &&
          ACE_OS::strcmp (SSID_string.c_str (),
                          configuration_->SSID.c_str ()))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if (unlikely (!Net_WLAN_Tools::disassociate (clientHandle_,
                                                     interface_identifier)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_WLAN_Tools::disassociate(0x%@,\"%s\"), continuing\n"),
                      clientHandle_,
                      ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
        if (unlikely (!Net_WLAN_Tools::disassociate (interface_identifier_string,
                                                     handle_)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_WLAN_Tools::disassociate(\"%s\",%d), continuing\n"),
                      ACE_TEXT (interface_identifier_string.c_str ()),
                      handle_));
#endif // ACE_WIN32 || ACE_WIN64
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (!Net_WLAN_Tools::associate (clientHandle_,
                                                interface_identifier,
                                                configuration_->SSID)))
#else
associate:
      if (unlikely (!Net_WLAN_Tools::associate (interface_identifier_string,
                                                ap_mac_address,
                                                configuration_->SSID,
                                                handle_)))
#endif // ACE_WIN32 || ACE_WIN64
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::associate(0x%@,\"%s\",%s), returning\n"),
                    clientHandle_,
                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::associate(\"%s\",%s,%s,%d), returning\n"),
                    ACE_TEXT (interface_identifier_string.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ()),
                    handle_));
#endif // ACE_WIN32 || ACE_WIN64
        break;
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      break;
#else
      // *TODO*: implement nl80211 support, rather than polling
      deadline = COMMON_TIME_NOW + result_timeout;
      do
      {
        ether_addr_s =
          Net_WLAN_Tools::associatedBSSID (configuration_->interfaceIdentifier,
                                           handle_);
        if (!ACE_OS::memcmp (&ether_addr_s.ether_addr_octet,
                             &ap_mac_address.ether_addr_octet,
                             ETH_ALEN))
        {
          result = true;
          break;
        } // end IF

        result_2 = ACE_OS::sleep (result_poll_interval);
        if (unlikely (result_2 == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                      &result_poll_interval));
        shutdown = inherited::hasShutDown ();
      } while (!shutdown && (COMMON_TIME_NOW <= deadline));
      if (unlikely (shutdown))
        break;
      if (unlikely (!result))
      {
        if (retries_ == NET_WLAN_MONITOR_AP_ASSOCIATION_RETRIES)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to associate with %s (interface was: \"%s\", SSID was: %s): timed out (was: %#T), giving up\n"),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
                      ACE_TEXT (interface_identifier_string.c_str ()),
                      ACE_TEXT (configuration_->SSID.c_str ()),
                      &result_poll_interval));
          retries_ = 0;
          inherited2::change (NET_WLAN_MONITOR_STATE_IDLE);
          break;
        } // end IF
        else
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to associate with %s (interface was: \"%s\", SSID was: %s): timed out (was: %#T), retrying...\n"),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address)).c_str ()),
                      ACE_TEXT (interface_identifier_string.c_str ()),
                      ACE_TEXT (configuration_->SSID.c_str ()),
                      &result_poll_interval));
          ++retries_;
          goto associate;
        } // end ELSE
      } // end IF

      inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
      break;
#endif // ACE_WIN32 || ACE_WIN64
    }
    case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_DISASSOCIATE;
      } // end lock scope

      std::string SSID_string = SSID ();
      ACE_ASSERT (!SSID_string.empty ()); // connected

      // sanity check(s)
      ACE_ASSERT (configuration_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
      Net_WLAN_Tools::disassociate (clientHandle_,
                                    configuration_->interfaceIdentifier);
#else
      ACE_ASSERT (handle_ != ACE_INVALID_HANDLE);
      Net_WLAN_Tools::disassociate (configuration_->interfaceIdentifier,
                                    handle_);
#endif

      bool essid_is_cached = false;
      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
            SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
        essid_is_cached = (iterator != SSIDsToInterfaceIdentifier_.end ());
      } // end lock scope

      if (configuration_->SSID.empty () || // not configured
          !essid_is_cached)                // configured SSID unknown (i.e. not cached)
      {
        inherited2::change (NET_WLAN_MONITOR_STATE_IDLE);
        goto reset_state_2;
      } // end IF
      else
        goto scanned;

reset_state_2:
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_DISASSOCIATE;
      } // end lock scope

      break;

      // configured && configured SSID known (i.e. cached)
scanned:
      inherited2::change (NET_WLAN_MONITOR_STATE_SCANNED);
      break;
    }
    case NET_WLAN_MONITOR_STATE_CONNECT:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_CONNECT;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);
      ACE_ASSERT (!configuration_->SSID.empty ());
      //{ ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
      //  // check cache whether the configured ESSID (if any) is known
      //  SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
      //    SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
      //  ACE_ASSERT (iterator != SSIDsToInterfaceIdentifier_.end ());
      //} // end lock scope
      std::string SSID_string = SSID ();
      ACE_ASSERT (!SSID_string.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // *NOTE*: apparently, the Win32 API currently does not let developers
      //         interfere with the WLAN DHCP addressing process
      //         programmatically
      //ACE_ASSERT (!ACE_OS::strcmp (configuration_->SSID.c_str (),
      //                             SSID_string.c_str ()));
#else
//      bool result = false;
      ACE_Time_Value result_poll_interval (0,
                                           NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL * 1000);
      ACE_Time_Value result_timeout (NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT,
                                     0);
      ACE_Time_Value deadline;
//      bool shutdown = false;
#if defined (DHCLIENT_SUPPORT)
      // sanity check(s)
      ACE_ASSERT (Common_Tools::getProcessId (ACE_TEXT_ALWAYS_CHAR ("dhclient")));
      // *NOTE*: query the local dhclient for the current DHCP lease on the
      //         configured interface
dhcpctl:
      dhcpctl_handle connection_h = dhcpctl_null_handle;
      dhcpctl_handle authenticator_h = dhcpctl_null_handle;
      dhcpctl_handle interface_h = dhcpctl_null_handle;
      dhcpctl_status wait_status = -1;
      dhcpctl_data_string result_string;
      dhcpctl_status status_i =
          dhcpctl_connect (&connection_h,
                           ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DHCLIENT_LOCALHOST_IP_STRING),
                           NET_WLAN_MONITOR_DHCLIENT_OMAPI_PORT,
                           authenticator_h);
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_connect(%s:%u): \"%s\", returning\n"),
                    ACE_TEXT (NET_WLAN_MONITOR_DHCLIENT_LOCALHOST_IP_STRING), NET_WLAN_MONITOR_DHCLIENT_OMAPI_PORT,
                    ACE_TEXT (isc_result_totext (status_i))));
        break;
      } // end IF
      ACE_ASSERT (connection_h != dhcpctl_null_handle);

      status_i = dhcpctl_new_object (&interface_h,
                                     connection_h,
                                     ACE_TEXT_ALWAYS_CHAR ("interface"));
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_new_object(0x%@,%s): \"%s\", returning\n"),
                    connection_h,
                    ACE_TEXT ("interface"),
                    ACE_TEXT (isc_result_totext (status_i))));
        break;
      } // end IF
      ACE_ASSERT (interface_h != dhcpctl_null_handle);
      status_i =
          dhcpctl_set_string_value (interface_h,
                                    ACE_TEXT_ALWAYS_CHAR (configuration_->interfaceIdentifier.c_str ()),
                                    ACE_TEXT_ALWAYS_CHAR ("name"));
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_set_string_value(0x%@,%s,\"%s\"): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT ("name"),
                    ACE_TEXT (isc_result_totext (status_i))));
        break;
      } // end IF
      status_i =
          dhcpctl_open_object (interface_h,
                               connection_h,
                               DHCPCTL_CREATE | DHCPCTL_EXCL);
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_open_object(0x%@,0x%@,%d): \"%s\", returning\n"),
                    interface_h,
                    connection_h,
                    DHCPCTL_CREATE | DHCPCTL_EXCL,
                    ACE_TEXT (isc_result_totext (status_i))));
        break;
      } // end IF
      // *TODO*: add a timeout here
      status_i = dhcpctl_wait_for_completion (interface_h,
                                              &wait_status);
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_wait_for_completion(0x%@): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (isc_result_totext (status_i))));
        break;
      } // end IF
      if (unlikely (wait_status != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_open_object(0x%@,create|excl): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (isc_result_totext (wait_status))));
        break;
      } // end IF
      ACE_OS::memset (&result_string, 0, sizeof (dhcpctl_data_string));
      status_i = dhcpctl_get_value (&result_string,
                                    interface_h,
                                    ACE_TEXT_ALWAYS_CHAR ("state"));
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_get_value(0x%@,state): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (isc_result_totext (wait_status))));
        break;
      } // end IF

      dhcpctl_data_string_dereference (&result_string, MDL);
      ACE_ASSERT (false); // *TODO*
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::change (NET_WLAN_MONITOR_STATE_CONNECTED);
      break;
    }
    case NET_WLAN_MONITOR_STATE_DISCONNECT:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_DISCONNECT;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);
      ACE_ASSERT (!configuration_->SSID.empty ());
      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
          SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
        ACE_ASSERT (iterator != SSIDsToInterfaceIdentifier_.end ());
      } // end lock scope
      std::string SSID_string = SSID ();
      ACE_ASSERT (!SSID_string.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // *NOTE*: apparently, the Win32 API currently does not let developers
      //         interfere with the DHCP addressing process programmatically
#else
#if defined (DHCLIENT_SUPPORT)
      // sanity check(s)
      ACE_ASSERT (Common_Tools::getProcessId (ACE_TEXT_ALWAYS_CHAR ("dhclient")));
      // *NOTE*: relinquish the current DHCP lease on the configured interface
      ACE_ASSERT (false); // *TODO*
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

      inherited2::change (NET_WLAN_MONITOR_STATE_DISASSOCIATE);
      break;
    }
    //////////////////////////////////////
    case NET_WLAN_MONITOR_STATE_INITIALIZED:
    {
      // *NOTE*: set state early to support atomic state transition
      //         notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_INITIALIZED;
      } // end lock scope

      if (!isActive_)
        break; // nothing to do

      std::string SSID_string = SSID ();
      if (SSID_string.empty ())
      {
        inherited2::change (NET_WLAN_MONITOR_STATE_SCAN);
        goto reset_state_3;
      } // end IF
      else
        goto connected;

reset_state_3:
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_DISASSOCIATE;
      } // end lock scope

      break;

connected:
      inherited2::change (NET_WLAN_MONITOR_STATE_CONNECTED);
      break;
    }
    case NET_WLAN_MONITOR_STATE_SCANNED:
    {
      // *NOTE*: set state early to support atomic state transition
      //         notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_SCANNED;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);

      std::string SSID_string = SSID ();
      if ((configuration_->SSID.empty () && // not configured
           !SSID_string.empty ()) ||        // associated
          (!configuration_->SSID.empty () &&        // configured
           !SSID_string.empty ()          &&        // associated
           !ACE_OS::strcmp (configuration_->SSID.c_str (),
                            SSID_string.c_str ()))) // already associated to configured ESSID
      {
        inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
        break;
      } // end IF

      bool essid_is_cached = false;
      { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
            SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
        essid_is_cached = (iterator != SSIDsToInterfaceIdentifier_.end ());
      } // end lock scope

      if (!configuration_->SSID.empty () && // configured
          essid_is_cached                && // configured SSID known (i.e. cached)
          configuration_->autoAssociate)    // auto-associate enabled
      {
        inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);
        break;
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      // switch back to 'idle', but wait in-between scans
      ACE_Time_Value scan_interval (NET_WLAN_MONITOR_UNIX_SCAN_INTERVAL, 0);
      int result = ACE_OS::sleep (scan_interval);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    &scan_interval));
#endif // ACE_WIN32 || ACE_WIN64

      inherited2::change (NET_WLAN_MONITOR_STATE_IDLE);

      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATED:
    {
      // *NOTE*: set state early to support atomic state transition
      //         notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATED;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
                                                           configuration_->interfaceIdentifier,
                                                           wlan_intf_opcode_autoconf_enabled,
                                                           configuration_->enableAutoConf)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    clientHandle_,
                    wlan_intf_opcode_autoconf_enabled,
                    (configuration_->enableAutoConf ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
      if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
                                                           configuration_->interfaceIdentifier,
                                                           wlan_intf_opcode_background_scan_enabled,
                                                           configuration_->enableBackgroundScans)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    clientHandle_,
                    wlan_intf_opcode_background_scan_enabled,
                    (configuration_->enableBackgroundScans ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
      if (unlikely (!Net_WLAN_Tools::setDeviceSettingBool (clientHandle_,
                                                           configuration_->interfaceIdentifier,
                                                           wlan_intf_opcode_media_streaming_mode,
                                                           configuration_->enableMediaStreamingMode)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    clientHandle_,
                    wlan_intf_opcode_media_streaming_mode,
                    (configuration_->enableMediaStreamingMode ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      try {
        onAssociate (configuration_->interfaceIdentifier,
                     SSID (),
                     true);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (SSID ().c_str ())));
      }
#endif // ACE_WIN32 || ACE_WIN64

      inherited2::change (NET_WLAN_MONITOR_STATE_CONNECT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//reset_state:
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATED;
      } // end lock scope
#endif
      break;
    }
    case NET_WLAN_MONITOR_STATE_CONNECTED:
    {
      // *NOTE*: set state early to support atomic state transition
      //         notifications
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited2::stateLock_));
        inherited2::state_ = NET_WLAN_MONITOR_STATE_CONNECTED;
      } // end lock scope

      if (likely (isConnectionNotified_))
        goto continue_2;
      isConnectionNotified_ = true;
      try {
        onConnect (configuration_->interfaceIdentifier,
                   SSID (),
                   true);
      } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    ACE_TEXT (SSID ().c_str ())));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onConnect(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (SSID ().c_str ())));
#endif
      }

continue_2:
      if (unlikely (isFirstConnect_))
        isFirstConnect_ = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (scanTimerId_ == -1))
        startScanTimer ();
#else
      inherited2::change (NET_WLAN_MONITOR_STATE_IDLE);
#endif
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
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::onSignalQualityChange (REFGUID interfaceIdentifier_in,
                                                         WLAN_SIGNAL_QUALITY signalQuality_in)
#else
                   UserDataType>::onSignalQualityChange (const std::string& interfaceIdentifier_in,
                                                         unsigned int signalQuality_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onSignalQualityChange"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\"/%s: signal quality changed: %u%%\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (SSID ().c_str ()),
              signalQuality_in));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\"/%s: signal quality changed: %u\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (SSID ().c_str ()),
              signalQuality_in));
#endif

  std::string SSID_string = SSID ();
  ACE_ASSERT (!SSID_string.empty ());
  SSIDS_TO_INTERFACEIDENTIFIER_MAP_ITERATOR_T iterator;
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // update cache
    iterator = SSIDsToInterfaceIdentifier_.find (SSID_string);
    if (likely (iterator != SSIDsToInterfaceIdentifier_.end ()))
      (*iterator).second.second.signalQuality = signalQuality_in;

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
        (*(iterator++))->onSignalQualityChange (interfaceIdentifier_in,
                                                signalQuality_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onSignalQualityChange(), continuing\n")));
      }
    } // end FOR
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
#endif // ACE_WIN32 || ACE_WIN64
                                               const std::string& SSID_in,
                                               bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onAssociate"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (configuration_);

  struct ether_addr ether_addr_s, ether_addr_2;
  ether_addr_s =
    Net_WLAN_Tools::associatedBSSID (clientHandle_,
                                     interfaceIdentifier_in);
  ether_addr_2 =
    Net_WLAN_Tools::getAccessPointAddress (clientHandle_,
                                           interfaceIdentifier_in,
                                           SSID_in);
  if (success_in)
  {
    if (!ACE_OS::memcmp (&ether_addr_s.ether_addr_octet,
                         &ether_addr_2.ether_addr_octet,
                         ETH_ALEN))
      inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
  } // end IF
  else
  {
    if (unlikely (retries_ == NET_WLAN_MONITOR_AP_ASSOCIATION_RETRIES))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to associate with %s (interface was: \"%s\", SSID was: %s), giving up\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ether_addr_2.ether_addr_octet)).c_str ()),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      retries_ = 0;
      inherited2::change (NET_WLAN_MONITOR_STATE_IDLE);
    } // end IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to associate with %s (interface was: \"%s\", SSID was: %s), retrying...\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ether_addr_2.ether_addr_octet)).c_str ()),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      ++retries_;
      inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);
    } // end ELSE
  } // end ELSE
#endif // ACE_WIN32 || ACE_WIN64

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
#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to associate with access point (SSID was: %s)\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
    return;
  } // end IF

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": (MAC: %s) associated with %s (SSID: %s)\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&Net_Common_Tools::interfaceToLinkLayerAddress (interfaceIdentifier_in)), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_2.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
#else
  SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator;
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    iterator =
      SSIDsToInterfaceIdentifier_.find (configuration_->SSID);
    ACE_ASSERT (iterator != SSIDsToInterfaceIdentifier_.end ());
    struct ether_addr ether_addr_s =
        Net_Common_Tools::interfaceToLinkLayerAddress (interfaceIdentifier_in);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": (MAC: %s) associated with %s (SSID: %s)\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_s), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&(*iterator).second.second), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
  } // end lock scope
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::onDisassociate (REFGUID interfaceIdentifier_in,
#else
                   UserDataType>::onDisassociate (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                                  const std::string& SSID_in,
                                                  bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onDisassociate"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (configuration_);

  struct ether_addr ether_addr_s =
    Net_WLAN_Tools::associatedBSSID (clientHandle_,
                                     interfaceIdentifier_in);
  struct Association_Configuration association_configuration;
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // check cache whether the configured ESSID (if any) is known
    SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
      SSIDsToInterfaceIdentifier_.find (SSID_in);
    ACE_ASSERT (iterator != SSIDsToInterfaceIdentifier_.end ());
    association_configuration = (*iterator).second.second;
  } // end lock scope
  if (success_in)
  {
    if (!ACE_OS::memcmp (&ether_addr_s.ether_addr_octet,
                         &association_configuration.accessPointLinkLayerAddress.ether_addr_octet,
                         ETH_ALEN))
      inherited2::change (NET_WLAN_MONITOR_STATE_IDLE);
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

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
        (*(iterator++))->onDisassociate (interfaceIdentifier_in,
                                         SSID_in,
                                         success_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onDisassociate(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  // sanity check(s)
  ACE_ASSERT (configuration_);
  if (!success_in)
  {
#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to disassociate with access point (SSID was: %s)\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to disassociate with access point (SSID was: %s)\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
    return;
  } // end IF

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": disassociated with access point (SSID was: %s)\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (configuration_->SSID.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": disassociated with access point (SSID was: %s)\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (configuration_->SSID.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
#endif // ACE_WIN32 || ACE_WIN64
                                             const std::string& SSID_in,
                                             bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onConnect"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (!configuration_->SSID.empty ());

  struct Association_Configuration association_configuration;
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // check cache whether the configured ESSID (if any) is known
    SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
      SSIDsToInterfaceIdentifier_.find (SSID_in);
    ACE_ASSERT (iterator != SSIDsToInterfaceIdentifier_.end ());
    association_configuration = (*iterator).second.second;
  } // end lock scope

  if (success_in)
  {
    if (!ACE_OS::strcmp (SSID_in.c_str (),
                         configuration_->SSID.c_str ()) &&
        !isFirstConnect_)
      inherited2::change (NET_WLAN_MONITOR_STATE_CONNECTED);
  } // end IF
  else
  {
    if (unlikely (retries_ == NET_WLAN_MONITOR_AP_CONNECTION_RETRIES))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s (interface was: \"%s\", SSID was: %s), giving up\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&association_configuration.accessPointLinkLayerAddress)).c_str ()),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      retries_ = 0;
      inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
    } // end IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s (interface was: \"%s\", SSID was: %s), retrying...\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&association_configuration.accessPointLinkLayerAddress)).c_str ()),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      ++retries_;
      inherited2::change (NET_WLAN_MONITOR_STATE_CONNECT);
    } // end ELSE
  } // end ELSE
#endif // ACE_WIN32 || ACE_WIN64

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
  // *NOTE*: the address type isn't specified at this level of abstraction
  //         --> peer address resolution must be notified by super-class(es)

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("\"%s\": connected to access point (SSID: %s): %s <---> %s\n"),
//              //ACE_TEXT (Net_Common_Tools::WLANInterfaceToString (clientHandle_, interfaceIdentifier_in).c_str ()),
//              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
//              ACE_TEXT (SSID_in.c_str ()),
//              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_).c_str ()),
//              ACE_TEXT (Net_Common_Tools::IPAddressToString (peerSAP_).c_str ())));
//#else
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("\"%s\": connected to access point (SSID: %s): %s <---> %s\n"),
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
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::onDisconnect (REFGUID interfaceIdentifier_in,
#else
                   UserDataType>::onDisconnect (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                                const std::string& SSID_in,
                                                bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onDisconnect"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (!configuration_->SSID.empty ());

  struct Association_Configuration association_configuration;
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // check cache whether the configured ESSID (if any) is known
    SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator =
      SSIDsToInterfaceIdentifier_.find (SSID_in);
    ACE_ASSERT (iterator != SSIDsToInterfaceIdentifier_.end ());
    association_configuration = (*iterator).second.second;
  } // end lock scope

  if (success_in)
  {
    if (!ACE_OS::strcmp (SSID_in.c_str (),
                         configuration_->SSID.c_str ()))
      inherited2::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

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
        (*(iterator++))->onDisconnect (interfaceIdentifier_in,
                                       SSID_in,
                                       success_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onDisconnect(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

  // sanity check(s)
  ACE_ASSERT (configuration_);
  if (!success_in)
  {
#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to disconnect from access point (SSID was: %s)\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to disconnect from access point (SSID was: %s)\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (configuration_->SSID.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
    return;
  } // end IF

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": disconnected from access point (SSID was: %s)\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (configuration_->SSID.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": disconnected from access point (SSID was: %s)\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (configuration_->SSID.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
#endif // ACE_WIN32 || ACE_WIN64
                                             bool success_in)
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
                                    success_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onHotPlug(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": interface enabled: %s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              (success_in ? ACE_TEXT ("yes") : ACE_TEXT ("no"))));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": interface enabled: %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              (success_in ? ACE_TEXT ("yes") : ACE_TEXT ("no"))));
#endif // ACE_WIN32 || ACE_WIN64
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                   UserDataType>::onRemove (REFGUID interfaceIdentifier_in,
#else
                   UserDataType>::onRemove (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                            bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onRemove"));

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
        (*(iterator++))->onRemove (interfaceIdentifier_in,
                                   success_in);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onRemove(), continuing\n")));
      }
    } // end FOR
  } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": interface removed: %s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              (success_in ? ACE_TEXT ("yes") : ACE_TEXT ("no"))));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": interface removed: %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              (success_in ? ACE_TEXT ("yes") : ACE_TEXT ("no"))));
#endif // ACE_WIN32 || ACE_WIN64
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::onScanComplete"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // step1: update the cache
  SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator;
#if defined (_DEBUG)
  std::set<std::string> known_ssids, current_ssids;
#endif // _DEBUG
  Net_WLAN_SSIDs_t ssids = Net_WLAN_Tools::getSSIDs (clientHandle_,
                                                     interfaceIdentifier_in);
  struct Association_Configuration association_configuration;
  association_configuration.accessPointLinkLayerAddress =
    Net_WLAN_Tools::getAccessPointAddress (clientHandle_,
                                           interfaceIdentifier_in,
                                           configuration_->SSID);
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // clear cache
    do
    {
      iterator =
          std::find_if (SSIDsToInterfaceIdentifier_.begin (), SSIDsToInterfaceIdentifier_.end (),
                        std::bind2nd (SSIDS_TO_INTERFACEIDENTIFIER_FIND_S (),
                                      interfaceIdentifier_in));
      if (iterator == SSIDsToInterfaceIdentifier_.end ())
        break;

#if defined (_DEBUG)
      known_ssids.insert ((*iterator).first);
#endif // _DEBUG
      SSIDsToInterfaceIdentifier_.erase ((*iterator).first);
    } while (true);

    for (Net_WLAN_SSIDsIterator_t iterator_2 = ssids.begin ();
         iterator_2 != ssids.end ();
         ++iterator_2)
    {
      SSIDsToInterfaceIdentifier_.insert (std::make_pair (*iterator_2,
                                                          std::make_pair (interfaceIdentifier_in,
                                                                          association_configuration)));
#if defined (_DEBUG)
      current_ssids.insert (*iterator_2);
#endif // _DEBUG
    } // end lock scope
  } // end lock scope
#if defined (_DEBUG)
  for (std::set<std::string>::const_iterator iterator_2 = known_ssids.begin ();
       iterator_2 != known_ssids.end ();
       ++iterator_2)
    if (current_ssids.find (*iterator_2) == current_ssids.end ())
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": lost contact to (E)SSID (was: %s)...\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                  ACE_TEXT ((*iterator_2).c_str ())));
#endif // _DEBUG

  inherited2::change (NET_WLAN_MONITOR_STATE_SCANNED);
#endif // ACE_WIN32 || ACE_WIN64

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
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::startScanTimer ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::startScanTimer"));

  // sanity check(s)
  ACE_ASSERT (scanTimerId_ == -1);
  ACE_ASSERT (timerInterface_);

  ACE_Time_Value scan_interval (NET_WLAN_MONITOR_WIN32_SCAN_INTERVAL,
                                0);
  try {
    scanTimerId_ =
      timerInterface_->schedule_timer (&timerHandler_,
                                       NULL, // no ACT
                                       COMMON_TIME_NOW + scan_interval,
                                       scan_interval);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_ITimer_T::schedule_timer(%T#), continuing\n"),
                &scan_interval));
  }
  if (unlikely (scanTimerId_ == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_ITimer_T::schedule_timer(%T#): \"%m\", continuing\n"),
                &scan_interval));
#if defined (_DEBUG)
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("started WLAN scan interval timer (id: %d, %#T)\n"),
                scanTimerId_,
                &scan_interval));
#endif
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                   TimePolicyType,
                   AddressType,
                   ConfigurationType,
                   MonitorAPI_e,
                   UserDataType>::cancelScanTimer ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::cancelScanTimer"));

  // sanity check(s)
  ACE_ASSERT (scanTimerId_ != -1);

  int result = -1;
  const void* act_p = NULL;
  try {
    result = timerInterface_->cancel_timer (scanTimerId_,
                                            &act_p,
                                            1);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_ITimer_T::cancel_timer(%d), continuing\n"),
                scanTimerId_));
  }
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_ITimer_T::cancel_timer(%d): \"%m\", continuing\n"),
                scanTimerId_));
  scanTimerId_ = -1;
  ACE_UNUSED_ARG (act_p);
}
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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

  change (NET_WLAN_MONITOR_STATE_IDLE);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): monitor (group: %d, thread id: %t) leaving...\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename AddressType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
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
#if defined (_DEBUG)
  std::set<std::string> known_ssids, current_ssids;
#endif

  if (!buffer_)
  { ACE_ASSERT (!bufferSize_);
    bufferSize_ = IW_SCAN_MAX_DATA;
    buffer_ = ACE_OS::malloc (bufferSize_);
    if (unlikely (!buffer_))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory (%u byte(s)): \"%m\", returning\n"),
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
  if (result < 0)
  {
    error = ACE_OS::last_error ();
    if ((error == E2BIG) && // 7: buffer too small
        (range_.we_version_compiled > 16))
    {
      if (iwreq_s.u.data.length > bufferSize_)
        bufferSize_ = iwreq_s.u.data.length;
      else
        bufferSize_ *= 2; // grow dynamically
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
  if (unlikely (iwreq_s.u.data.length > bufferSize_))
  { // --> grow the buffer and retry
    bufferSize_ = iwreq_s.u.data.length;
retry:
    buffer_ = ACE_OS::realloc (buffer_, bufferSize_);
    if (!buffer_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to reallocate memory (%u byte(s)): \"%m\", returning\n"),
                  bufferSize_));
      goto continue_;
    } // end IF
    iwreq_s.u.data.pointer = buffer_;
    iwreq_s.u.data.length = bufferSize_;
    goto fetch_scan_result_data;
  } // end IF
  ACE_ASSERT (iwreq_s.u.data.length && (iwreq_s.u.data.length <= bufferSize_));

  // received scan results

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

#if defined (_DEBUG)
      known_ssids.insert ((*iterator).first);
#endif
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
        struct Association_Configuration association_configuration_s;
        association_configuration_s.accessPointLinkLayerAddress =
            ap_mac_address;
        { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, 0);
          SSIDsToInterfaceIdentifier_.insert (std::make_pair (essid_string,
                                                              std::make_pair (configuration_->interfaceIdentifier,
                                                                              association_configuration_s)));
        } // end lock scope
#if defined (_DEBUG)
        if (known_ssids.find (essid_string) == known_ssids.end ())
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("\"%s\": detected wireless access point (MAC address: %s, ESSID: %s)...\n"),
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
                    ACE_TEXT ("invalid/unknown WE event (was: %d), returning\n"),
                    iw_event_s.cmd));
        goto continue_;
      }
    } // end SWITCH
  } while (true);
#if defined (_DEBUG)
  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, subscribersLock_, 0);
    for (SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T iterator_2 = SSIDsToInterfaceIdentifier_.begin ();
         iterator_2 != SSIDsToInterfaceIdentifier_.end ();
         ++iterator_2)
      if (!ACE_OS::strcmp ((*iterator_2).second.first.c_str (),
                           configuration_->interfaceIdentifier.c_str ()))
        current_ssids.insert ((*iterator_2).first);
  } // end lock scope
  for (std::set<std::string>::const_iterator iterator_2 = known_ssids.begin ();
       iterator_2 != known_ssids.end ();
       ++iterator_2)
    if (current_ssids.find (*iterator_2) == current_ssids.end ())
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": lost contact to ESSID (was: %s)...\n"),
                  ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                  ACE_TEXT ((*iterator_2).c_str ())));
#endif

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
