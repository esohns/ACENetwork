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

#if defined (NL80211_SUPPORT)
#include "linux/netlink.h"
#include "linux/nl80211.h"

#include "netlink/handlers.h"
#include "netlink/netlink.h"
#include "netlink/genl/ctrl.h"
#include "netlink/genl/genl.h"
#endif // NL80211_SUPPORT
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
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_defines.h"

#include "net_common.h"
#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

#if defined (ACE_LINUX)
#if defined (DHCLIENT_SUPPORT)
#include "dhcp_tools.h"
#endif // DHCLIENT_SUPPORT
#endif // ACE_LINUX

#include "net_wlan_defines.h"

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::Net_WLAN_Monitor_Base_T ()
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::Net_WLAN_Monitor_Base_T ()
#endif // ACE_WIN32 || ACE_WIN64
 : inherited ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
 , clientHandle_ (ACE_INVALID_HANDLE)
#endif // WLANAPI_SUPPORT
 , scanTimerId_ (-1)
 , timerHandler_ (this,
                  true) // one-shot ?
 , timerInterface_ (NULL)
#elif defined (ACE_LINUX)
#if defined (WEXT_SUPPORT)
 , buffer_ (NULL)
 , bufferSize_ (0)
 , handle_ (ACE_INVALID_HANDLE)
// , isRegistered_ (false)
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
 , nl80211CBData_ ()
 , familyId_ (0)
 , socketHandle_ (NULL)
#endif // NL80211_SUPPORT
#if defined (DHCLIENT_SUPPORT)
 , dhclientCBData_ ()
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
 , configuration_ (NULL)
 , isActive_ (false)
 , isConnectionNotified_ (false)
 , isFirstConnect_ (true)
 , isInitialized_ (false)
 , localSAP_ ()
 , peerSAP_ ()
 , retries_ (0)
 , SSIDCache_ ()
 , subscribersLock_ ()
 , subscribers_ ()
 /////////////////////////////////////////
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
// , queue_ (STREAM_QUEUE_MAX_SLOTS)
//#endif // ACE_WIN32 || ACE_WIN64
 , SSIDSeenBefore_ (false)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::Net_WLAN_Monitor_Base_T"));

//  inherited::reactor (ACE_Reactor::instance ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#if defined (DHCLIENT_SUPPORT)
  dhclientCBData_.monitor = this;

  isc_result_t status_i = dhcpctl_initialize ();
  if (unlikely (status_i != ISC_R_SUCCESS))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::dhcpctl_initialize(): \"%s\", continuing\n"),
                ACE_TEXT (isc_result_totext (status_i))));
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::~Net_WLAN_Monitor_Base_T ()
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::~Net_WLAN_Monitor_Base_T ()
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::~Net_WLAN_Monitor_Base_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (scanTimerId_ != -1)
    cancelScanTimer ();
#if defined (WLANAPI_SUPPORT)
  if (unlikely (clientHandle_ != ACE_INVALID_HANDLE))
    Net_WLAN_Tools::finalize (clientHandle_);
#endif // WLANAPI_SUPPORT
#else
  if (unlikely (isActive_))
    inherited::stop (true,
                     true);

#if defined (WEXT_SUPPORT)
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
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
  if (unlikely (socketHandle_))
    nl_socket_free (socketHandle_);
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
const Net_WLAN_AccessPointCacheValue_t&
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::get1RR (const std::string& SSID_in) const
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::get1RR (const std::string& SSID_in) const
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::get1R"));

  // *NOTE*: this must be static so the returned reference does not fall off the
  //         stack early when no match has been found
  static Net_WLAN_AccessPointCacheValue_t return_value_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return_value_s.first = GUID_NULL;
#else
  return_value_s.first.clear ();
#endif // ACE_WIN32 || ACE_WIN64
  return_value_s.second.IPAddress.reset ();

  Net_WLAN_AccessPointCacheConstIterator_t iterator;
  { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_, return_value_s);
    iterator = SSIDCache_.find (SSID_in);
    if (iterator != SSIDCache_.end ())
      return (*iterator).second;
  } // end lock scope

  return return_value_s;
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::set3R (const std::string& SSID_in,
                                  REFGUID interfaceIdentifier_in,
                                  const struct Net_WLAN_AccessPointState& state_in)
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::set3R (const std::string& SSID_in,
                                                const std::string& interfaceIdentifier_in,
                                                const struct Net_WLAN_AccessPointState& state_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::set3R"));

  // sanity check(s)
  if (SSID_in.empty ())
    ACE_ASSERT (!SSID_in.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (!InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL));
#else
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
#endif // ACE_WIN32 || ACE_WIN64

  Net_WLAN_AccessPointCacheValue_t cache_value;
  cache_value.first = interfaceIdentifier_in;
  cache_value.second = state_in;
  Net_WLAN_AccessPointCacheIterator_t iterator;
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    iterator = SSIDCache_.find (SSID_in);
    if (unlikely (iterator != SSIDCache_.end ()))
    {
      (*iterator).second = cache_value;
      return;
    } // end IF
    SSIDCache_.insert (std::make_pair (SSID_in, cache_value));
  } // end lock scope
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
bool
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::initialize (const ConfigurationType& configuration_in)
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::initialize (const ConfigurationType& configuration_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::initialize"));

  if (unlikely (isInitialized_))
  {
    if (isActive_)
      stop (true,  // wait ?
            true); // N/A

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
    ACE_ASSERT (clientHandle_ == ACE_INVALID_HANDLE);
#endif // WLANAPI_SUPPORT
    ACE_ASSERT (scanTimerId_ == -1);
    timerInterface_ = NULL;
#elif defined (ACE_LINUX)
#if defined (WEXT_SUPPORT)
    if (buffer_)
    {
      ACE_OS::free (buffer_); buffer_ = NULL;
    } // end IF
    bufferSize_ = 0;
    ACE_ASSERT (handle_ == ACE_INVALID_HANDLE);
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
    familyId_ = 0;

    if (socketHandle_)
    {
      nl_close (socketHandle_); nl_socket_free (socketHandle_); socketHandle_ = NULL;
    } // end IF
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

    configuration_ = NULL;
    ACE_ASSERT (!isActive_);
    isConnectionNotified_ = false;
    isFirstConnect_ = true;
    localSAP_.reset ();
    peerSAP_.reset ();

    { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
      SSIDCache_.clear ();
      subscribers_.clear ();
    } // end lock scope

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//    unsigned int result = queue_.flush (false);
//    ACE_UNUSED_ARG (result);
//#endif
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
#else
//#if defined (WEXT_SUPPORT)
//#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
  ACE_ASSERT (!socketHandle_);

  socketHandle_ = nl_socket_alloc ();
  if (unlikely (!socketHandle_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nl_socket_alloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
//  nl_socket_set_nonblocking (socketHandle_);
//  nl_socket_set_buffer_size (socketHandle_, int rx, int tx);

  int result = nl_connect (socketHandle_, NETLINK_GENERIC);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_connect(0x%@): \"%s\", aborting\n"),
                socketHandle_,
                ACE_TEXT (nl_geterror (result))));
    return false;
  } // end IF
  nl_socket_disable_auto_ack (socketHandle_);
  nl_socket_disable_msg_peek (socketHandle_);

  familyId_ =
      genl_ctrl_resolve (socketHandle_,
                         ACE_TEXT_ALWAYS_CHAR (NL80211_GENL_NAME));
  if (unlikely (familyId_ < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genl_ctrl_resolve(%@,%s): \"%s\", aborting\n"),
                socketHandle_,
                ACE_TEXT (NL80211_GENL_NAME),
                ACE_TEXT (nl_geterror (familyId_))));
    return false;
  } // end IF

  // *TODO*: disable sequence checks selectively for multicast messages
  nl_socket_disable_seq_check (socketHandle_);
#endif // NL80211_SUPPORT
//#if defined (DBUS_SUPPORT)
//#endif // DBUS_SUPPORT

#endif // ACE_WIN32 || ACE_WIN64
  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool subscribe_this = false;
  if (!configuration_->notificationCB ||
      (configuration_->notificationCB == network_wlan_default_notification_cb)) // re-initialized ?
  {
    if (!configuration_->notificationCB)
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("WLAN API notification callback not specified, using default implementation\n")));
    configuration_->notificationCB = network_wlan_default_notification_cb;
    configuration_->notificationCBData =
        static_cast<Net_WLAN_IMonitorCB*> (this);
  } // end IF
  else
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("WLAN API notification callback specified; disabled default event subscription, subscribing 'this' instead\n")));
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

  isInitialized_ = true;

  inherited::initialize ();

  return true;
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::subscribe (Net_WLAN_IMonitorCB* interfaceHandle_in)
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::subscribe (Net_WLAN_IMonitorCB* interfaceHandle_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::subscribe"));

  // sanity check(s)
  if (unlikely (!interfaceHandle_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument (was: 0x%@), returning\n"),
                interfaceHandle_in));
    return;
  } // end IF

  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    subscribers_.push_back (interfaceHandle_in);
  } // end lock scope
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::unsubscribe (Net_WLAN_IMonitorCB* interfaceHandle_in)
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::unsubscribe (Net_WLAN_IMonitorCB* interfaceHandle_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::unsubscribe"));

  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
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

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
const WLAN_SIGNAL_QUALITY
#else
const unsigned int
#endif // ACE_WIN32 || ACE_WIN64
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       >::get_4 () const
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::get_4 () const
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::get_4"));

  // sanity check(s)
  if (unlikely (!isActive_))
    return 0;
  std::string SSID_string = this->SSID ();
  if (unlikely (SSID_string.empty ()))
    return 0;

  Net_WLAN_AccessPointCacheConstIterator_t iterator;
  { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
    iterator = SSIDCache_.find (SSID_string);
    if (likely (iterator != SSIDCache_.end ()))
      return ((*iterator).second.second.signalQuality);
  } // end lock scope

  return 0;
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
bool
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::associate (REFGUID interfaceIdentifier_in,
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::associate (const std::string& interfaceIdentifier_in,
                                                    const struct ether_addr& accessPointLinkLayerAddress_in,
#endif // ACE_WIN32 || ACE_WIN64
                                                    const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::associate"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_UNUSED_ARG (accessPointLinkLayerAddress_in);
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
  bool restart_b = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID interface_identifier = interfaceIdentifier_in;
#else
  std::string interface_identifier = interfaceIdentifier_in;
#endif // ACE_WIN32 || ACE_WIN64
  std::string current_ssid_s = this->SSID ();
  if (likely (!SSID_in.empty ()))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_ASSERT (SSID_in.size () <= DOT11_SSID_MAX_LENGTH);
#else
#if defined (WEXT_SUPPORT)
    ACE_ASSERT (SSID_in.size () <= IW_ESSID_MAX_SIZE);
#elif defined (NL80211_SUPPORT) || defined (DBUS_SUPPORT)
    ACE_ASSERT (SSID_in.size () <= 32);
#endif // NL80211_SUPPORT || DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

    if (unlikely (!ACE_OS::strcmp (SSID_in.c_str (),
                                   current_ssid_s.c_str ())))
      return true; // nothing to do
  } // end IF
  else // --> (disassociate and-) reset configuration
    goto continue_;

  // check cache ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (InlineIsEqualGUID (interface_identifier, GUID_NULL)))
#else
  if (unlikely (interface_identifier.empty ()))
#endif // ACE_WIN32 || ACE_WIN64
  {
    // check cache
    Net_WLAN_AccessPointCacheConstIterator_t iterator_2;
    { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
      iterator_2 = SSIDCache_.find (SSID_in);
      if (iterator_2 != SSIDCache_.end ())
      {
#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("SSID (was: %s) detected from interface \"%s\", continuing\n"),
                    ACE_TEXT (SSID_in.c_str ()),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("SSID (was: %s) detected from interface \"%s\", continuing\n"),
                    ACE_TEXT (SSID_in.c_str ()),
                    ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
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

continue_:
  if (unlikely (!current_ssid_s.empty () &&
                ACE_OS::strcmp (SSID_in.c_str (),
                                current_ssid_s.c_str ())))
  {
    try {
      do_associate (interfaceIdentifier_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                    accessPointLinkLayerAddress_in,
#endif // ACE_WIN32 || ACE_WIN64
                    ACE_TEXT_ALWAYS_CHAR ("")); // <-- disassociate
    } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s), continuing\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ()),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&const_cast<struct ether_addr&> (accessPointLinkLayerAddress_in).ether_addr_octet)).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
    }
  } // end IF

  if (likely (isActive_))
  {
    restart_b = true;
    stop (true,  // wait ?
          true); // N/A
  } // end IF
  ACE_ASSERT (!isActive_);

  // reconfigure ?
  if (likely (configuration_))
  { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_, false);
    configuration_->autoAssociate = !SSID_in.empty ();
    configuration_->interfaceIdentifier = interface_identifier;
    configuration_->SSID = SSID_in;
    isConnectionNotified_ = false;
  } // end IF

  if (likely (restart_b))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    initialize (*configuration_);
#endif // ACE_WIN32 || ACE_WIN64

    ACE_thread_t thread_id = 0;
    start (thread_id);
    ACE_UNUSED_ARG (thread_id);
  } // end IF

  return true;
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::scan (REFGUID interfaceIdentifier_in)
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::scan (const std::string& interfaceIdentifier_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::scan"));

  Net_InterfaceIdentifiers_t interface_identifiers_a;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (InlineIsEqualGUID (interfaceIdentifier_in, GUID_NULL))
#if defined (WLANAPI_USE)
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (clientHandle_);
#else
    ;
#endif // WLANAPI_USE
#elif defined (ACE_LINUX)
  if (interfaceIdentifier_in.empty ())
// *TODO*: compile only one implementation; move this out of the base class
#if defined (WEXT_USE)
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (AF_UNSPEC,
                                                             0);
#elif defined (NL80211_USE)
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (socketHandle_,
                                                             familyId_);
#elif defined (DBUS_USE)
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (connection_,
                                                             AF_UNSPEC,
                                                             0);
#else
    ;
#endif
#endif // ACE_WIN32 || ACE_WIN64
  else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    interface_identifiers_a.push_back (interfaceIdentifier_in);
#else
    interface_identifiers_a.push_back (Net_Common_Tools::interfaceToString (interfaceIdentifier_in));
#endif // _WIN32_WINNT_VISTA
#else
    interface_identifiers_a.push_back (interfaceIdentifier_in);
#endif // ACE_WIN32 || ACE_WIN64
  if (interface_identifiers_a.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no wireless LAN interfaces found; cannot auto-select, returning\n")));
    return;
  } // end IF

  bool restart_b = isActive_;
  if (likely (isActive_))
  {
    restart_b = true;
    stop (true,  // wait ?
          true); // N/A
  } // end IF
  ACE_ASSERT (!isActive_);

  // *TODO*: support monitoring multiple/all interfaces at the same time
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct ether_addr ap_mac_address_s;
#endif // ACE_WIN32 || ACE_WIN64
  try {
    do_associate (interfaceIdentifier_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                  ap_mac_address_s,
#endif // ACE_WIN32 || ACE_WIN64
                  ACE_TEXT_ALWAYS_CHAR ("")); // <-- disassociate
  } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s), returning\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT ("")));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), returning\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT ("")));
#endif // ACE_WIN32 || ACE_WIN64
    return;
  }

  // reconfigure ?
  if (likely (configuration_))
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    configuration_->autoAssociate = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    configuration_->interfaceIdentifier = interface_identifiers_a.front ();
#else
    configuration_->interfaceIdentifier =
      Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (interface_identifiers_a.front ()));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    configuration_->interfaceIdentifier = interface_identifiers_a.front ();
#endif // ACE_WIN32 || ACE_WIN64
    configuration_->SSID.clear ();
//    isConnectionNotified_ = false;
  } // end IF

  if (restart_b)
  {
    ACE_thread_t thread_id = 0;
    start (thread_id);
    ACE_UNUSED_ARG (thread_id);
  } // end IF
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
Net_WLAN_SSIDs_t
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::SSIDs () const
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::SSIDs () const
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::SSIDs"));

  Net_WLAN_SSIDs_t result;

  // sanity check(s)
  ACE_ASSERT (configuration_);

  { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_, result);
    for (Net_WLAN_AccessPointCacheConstIterator_t iterator = SSIDCache_.begin ();
         iterator != SSIDCache_.end ();
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

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::onChange (enum Net_WLAN_MonitorState newState_in)
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onChange (enum Net_WLAN_MonitorState newState_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onChange"));

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
      bool essid_is_cached = false;

      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_IDLE;
      } // end lock scope

      if (!isActive_               // stop()ped
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         )
#else
          || inherited::hasShutDown ())
#endif // ACE_WIN32 || ACE_WIN64
      {
        inherited::change (NET_WLAN_MONITOR_STATE_INITIALIZED);
        break;
      } // end IF
      std::string SSID_string = this->SSID ();
      if (!SSID_string.empty ()) // connected
      {
        if (!isFirstConnect_)
          goto continue_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        inherited::change (NET_WLAN_MONITOR_STATE_CONNECTED);
#else
        inherited::change (NET_WLAN_MONITOR_STATE_AUTHENTICATED);
#endif // ACE_WIN32 || ACE_WIN64
        break;

continue_:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_USE)
        // *NOTE*: the timer handler might trigger before scanTimerId_ has been
        //         set --> grab the lock
        { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
          if (unlikely (scanTimerId_ == -1))
          {
            ACE_Reverse_Lock<ACE_MT_SYNCH::RECURSIVE_MUTEX> reverse_lock (subscribersLock_,
                                                                          ACE_Acquire_Method::ACE_REGULAR);
            { ACE_GUARD (ACE_Reverse_Lock<ACE_MT_SYNCH::RECURSIVE_MUTEX>, aGuard_2, reverse_lock);
              if (!startScanTimer (ACE_Time_Value::zero))
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to start scan timer, continuing\n")));
            } // end lock scope
          } // end IF
        } // end lock scope
#endif // WLANAPI_USE
#endif // ACE_WIN32 || ACE_WIN64
        inherited::change (NET_WLAN_MONITOR_STATE_SCAN);
        // *NOTE*: there is no need to reset the state when returning from a
        //         transitional state, as the current state is not set until this
        //         method returns. Note that this holds only iff the state has not
        //         already changed; see: common_state_machine_base.inl:214
        //         --> reset the state just in case
        goto reset_state;
      } // end IF

      // not connected

      // sanity check(s)
      ACE_ASSERT (configuration_);

      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
            SSIDCache_.find (configuration_->SSID);
        essid_is_cached = (iterator != SSIDCache_.end ());
        if (unlikely (!SSIDSeenBefore_ && essid_is_cached))
          SSIDSeenBefore_ = true;
      } // end lock scope

      if (configuration_->SSID.empty () || // not configured
          !essid_is_cached              || // configured SSID unknown (i.e. not cached yet)
          !configuration_->autoAssociate)  // auto-associate disabled
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_USE)
        // *NOTE*: the timer handler might trigger before scanTimerId_ has been
        //         set --> grab the lock
        { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
          if (unlikely (scanTimerId_ == -1))
          {
            ACE_Reverse_Lock<ACE_MT_SYNCH::RECURSIVE_MUTEX> reverse_lock (subscribersLock_,
                                                                          ACE_Acquire_Method::ACE_REGULAR);
            { ACE_GUARD (ACE_Reverse_Lock<ACE_MT_SYNCH::RECURSIVE_MUTEX>, aGuard_2, reverse_lock);
              if (!startScanTimer (ACE_Time_Value::zero))
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to start scan timer, continuing\n")));
            } // end lock scope
          } // end IF
        } // end lock scope
#endif // WLANAPI_USE
#endif // ACE_WIN32 || ACE_WIN64
        inherited::change (NET_WLAN_MONITOR_STATE_SCAN);
        // *NOTE*: there is no need to reset the state when returning from a
        //         transitional state, as the current state is not set until this
        //         method returns. Note that this holds only iff the state has not
        //         already changed; see: common_state_machine_base.inl:214
        //         --> reset the state just in case
        goto reset_state;
      } // end IF

      ACE_ASSERT (SSID_string.empty () && !configuration_->SSID.empty () && essid_is_cached && configuration_->autoAssociate);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited::change (NET_WLAN_MONITOR_STATE_CONNECT);
#else
      inherited::change (NET_WLAN_MONITOR_STATE_AUTHENTICATE);
#endif // ACE_WIN32 || ACE_WIN64

reset_state:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_IDLE;
      } // end lock scope

      break;
    }
    case NET_WLAN_MONITOR_STATE_SCAN:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_SCAN;
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
#elif defined (ACE_LINUX)
#if defined (_DEBUG)
#if defined (WEXT_USE)
      ACE_Time_Value scan_time;
      std::string scan_time_string;
      ACE_High_Res_Timer timer;
#elif defined (NL80211_USE)
      nl80211CBData_.timestamp = COMMON_TIME_NOW;
#endif // WEXT_USE || NL80211_USE
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": scanning...\n"),
                  ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));

#if defined (WEXT_USE)
      timer.start ();
#endif // WEXT_USE
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64

      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      try {
        do_scan (configuration_->interfaceIdentifier,
                 ap_mac_address_s,
                 (SSIDSeenBefore_ ? configuration_->SSID
                                  : ACE_TEXT_ALWAYS_CHAR ("")));
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_scan(), continuing\n")));
      }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#if defined (WEXT_USE)
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
#if defined (_DEBUG)
      timer.stop ();
      timer.elapsed_time (scan_time);
      Common_Timer_Tools::periodToString (scan_time,
                                          scan_time_string);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": scanning (h:m:s.us) %s...DONE\n"),
                  ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                  ACE_TEXT (scan_time_string.c_str ())));
#endif // _DEBUG

      inherited::change (NET_WLAN_MONITOR_STATE_SCANNED);
#endif // WEXT_USE
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case NET_WLAN_MONITOR_STATE_AUTHENTICATE:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
        inherited::state_ = NET_WLAN_MONITOR_STATE_AUTHENTICATE;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);
      ACE_ASSERT (!configuration_->SSID.empty ());
      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
#if defined (ACE_LINUX)
      std::string interface_identifier;
#if defined (WEXT_USE)
      struct ether_addr ether_addr_s;
      bool result = false;
      int result_2 = -1;
      ACE_Time_Value result_poll_interval (0,
                                           NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL * 1000);
      ACE_Time_Value result_timeout (NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT,
                                     0);
      ACE_Time_Value deadline;
      bool shutdown = false;
#endif // WEXT_USE
#endif // ACE_LINUX
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
          SSIDCache_.find (configuration_->SSID);
        ACE_ASSERT (iterator != SSIDCache_.end ());
        interface_identifier = (*iterator).second.first;
        ap_mac_address_s = (*iterator).second.second.linkLayerAddress;
      } // end lock scope
      if (unlikely (ACE_OS::strcmp (interface_identifier.c_str (),
                                    configuration_->interfaceIdentifier.c_str ())))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("found SSID (was: %s) on interface \"%s\" (configured interface was: \"%s\"), continuing\n"),
                    ACE_TEXT (configuration_->SSID.c_str ()),
                    ACE_TEXT (interface_identifier.c_str ()),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));

      std::string SSID_string = this->SSID ();
      if ((!configuration_->SSID.empty ()              &&
           !SSID_string.empty ()                       &&
           ACE_OS::strcmp (configuration_->SSID.c_str (),
                           SSID_string.c_str ())) &&
          configuration_->autoAssociate)
      {
        inherited::change (NET_WLAN_MONITOR_STATE_DISCONNECT);
        break;
      } // end IF

#if defined (ACE_LINUX)
#if defined (WEXT_USE)
authenticate:
#endif // WEXT_USE
#endif // ACE_LINUX
      try {
        do_authenticate (interface_identifier,
                         ap_mac_address_s,
                         configuration_->SSID);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_authenticate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (interface_identifier.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
      }

#if defined (ACE_LINUX)
#if defined (WEXT_USE)
      deadline = COMMON_TIME_NOW + result_timeout;
      do
      {
        ether_addr_s =
          Net_WLAN_Tools::associatedBSSID (configuration_->interfaceIdentifier,
                                           handle_);
        if (!ACE_OS::memcmp (&ether_addr_s.ether_addr_octet,
                             &ap_mac_address_s.ether_addr_octet,
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
                      ACE_TEXT ("\"%s\": failed to associate with access point (MAC was: %s; SSID was: %s): timed out (was: %#T), giving up\n"),
                      ACE_TEXT (interface_identifier.c_str ()),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                      ACE_TEXT (configuration_->SSID.c_str ()),
                      &result_poll_interval));
          retries_ = 0;
          inherited::change (NET_WLAN_MONITOR_STATE_IDLE);
          break;
        } // end IF
        else
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("\"%s\": failed to associate with access point (MAC was: %s; SSID was: %s): timed out (was: %#T), retrying...\n"),
                      ACE_TEXT (interface_identifier.c_str ()),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                      ACE_TEXT (configuration_->SSID.c_str ()),
                      &result_poll_interval));
          ++retries_;
          goto authenticate;
        } // end ELSE
      } // end IF

      inherited::change (NET_WLAN_MONITOR_STATE_AUTHENTICATED);
#endif // WEXT_USE
#endif // ACE_LINUX
      break;
    }
    case NET_WLAN_MONITOR_STATE_DEAUTHENTICATE:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
        inherited::state_ = NET_WLAN_MONITOR_STATE_DEAUTHENTICATE;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (!this->SSID ().empty ()); // associated
      ACE_ASSERT (configuration_);

      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      try {
        do_authenticate (configuration_->interfaceIdentifier,
                         ap_mac_address_s,
                         ACE_TEXT_ALWAYS_CHAR ("")); // <-- deauthenticate
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_authenticate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT ("")));
      }

#if defined (ACE_LINUX)
#if defined (WEXT_USE)
      bool essid_is_cached = false;
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
            SSIDCache_.find (configuration_->SSID);
        essid_is_cached = (iterator != SSIDCache_.end ());
      } // end lock scope

      if (configuration_->SSID.empty () || // not configured
          !essid_is_cached)                // configured SSID unknown (i.e. not cached)
      {
        inherited::change (NET_WLAN_MONITOR_STATE_IDLE);
        break;
      } // end IF
      // --> configured && configured SSID known (i.e. cached)
      inherited::change (NET_WLAN_MONITOR_STATE_SCANNED);
#endif // WEXT_USE
#endif // ACE_LINUX

      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATE:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
        inherited::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATE;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);
      ACE_ASSERT (!configuration_->SSID.empty ());

      std::string interface_identifier;
      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
#if defined (ACE_LINUX)
#if defined (WEXT_USE)
      struct ether_addr ether_addr_s;
      bool result = false;
      int result_2 = -1;
      ACE_Time_Value result_poll_interval (0,
                                           NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL * 1000);
      ACE_Time_Value result_timeout (NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT,
                                     0);
      ACE_Time_Value deadline;
      bool shutdown = false;
#endif // WEXT_USE
#endif // ACE_LINUX

      // check cache whether the configured ESSID (if any) is known
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
          SSIDCache_.find (configuration_->SSID);
        ACE_ASSERT (iterator != SSIDCache_.end ());
        interface_identifier = (*iterator).second.first;
        ap_mac_address_s = (*iterator).second.second.linkLayerAddress;
      } // end lock scope
      if (unlikely (ACE_OS::strcmp (interface_identifier.c_str (),
                                    configuration_->interfaceIdentifier.c_str ())))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("found SSID (was: %s) on interface \"%s\" (configured interface was: \"%s\"), continuing\n"),
                    ACE_TEXT (configuration_->SSID.c_str ()),
                    ACE_TEXT (interface_identifier.c_str ()),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));

      std::string SSID_string = this->SSID ();
      if (!SSID_string.empty ()                       &&
          ACE_OS::strcmp (configuration_->SSID.c_str (),
                          SSID_string.c_str ()))
      {
        inherited::change (NET_WLAN_MONITOR_STATE_DISCONNECT);
        break;
      } // end IF

#if defined (ACE_LINUX)
#if defined (WEXT_USE)
associate:
#endif // WEXT_USE
#endif // ACE_LINUX
      try {
        do_associate (interface_identifier,
                      ap_mac_address_s,
                      configuration_->SSID);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (interface_identifier.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
      }

#if defined (ACE_LINUX)
#if defined (WEXT_USE)
      deadline = COMMON_TIME_NOW + result_timeout;
      do
      {
        ether_addr_s =
          Net_WLAN_Tools::associatedBSSID (configuration_->interfaceIdentifier,
                                           handle_);
        if (!ACE_OS::memcmp (&ether_addr_s.ether_addr_octet,
                             &ap_mac_address_s.ether_addr_octet,
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
                      ACE_TEXT ("\"%s\": failed to associate with access point (MAC was: %s; SSID was: %s): timed out (was: %#T), giving up\n"),
                      ACE_TEXT (interface_identifier.c_str ()),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                      ACE_TEXT (configuration_->SSID.c_str ()),
                      &result_poll_interval));
          retries_ = 0;
          inherited::change (NET_WLAN_MONITOR_STATE_IDLE);
          break;
        } // end IF
        else
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("\"%s\": failed to associate with access point (MAC was: %s; SSID was: %s): timed out (was: %#T), retrying...\n"),
                      ACE_TEXT (interface_identifier.c_str ()),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(ap_mac_address_s.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                      ACE_TEXT (configuration_->SSID.c_str ()),
                      &result_poll_interval));
          ++retries_;
          goto associate;
        } // end ELSE
      } // end IF

      inherited::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
#endif // WEXT_USE
#endif // ACE_LINUX
      break;
    }
    case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
        inherited::state_ = NET_WLAN_MONITOR_STATE_DISASSOCIATE;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (!this->SSID ().empty ()); // associated
      ACE_ASSERT (configuration_);

      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      try {
        do_associate (configuration_->interfaceIdentifier,
                      ap_mac_address_s,
                      ACE_TEXT_ALWAYS_CHAR ("")); // <-- disassociate
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT ("")));
      }

#if defined (ACE_LINUX)
#if defined (WEXT_USE)
      bool essid_is_cached = false;
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
            SSIDCache_.find (configuration_->SSID);
        essid_is_cached = (iterator != SSIDCache_.end ());
      } // end lock scope

      if (configuration_->SSID.empty () || // not configured
          !essid_is_cached)                // configured SSID unknown (i.e. not cached)
      {
        inherited::change (NET_WLAN_MONITOR_STATE_IDLE);
        break;
      } // end IF
      // --> configured && configured SSID known (i.e. cached)
      inherited::change (NET_WLAN_MONITOR_STATE_SCANNED);
#endif // WEXT_USE
#endif // ACE_LINUX

      break;
    }
#endif // ACE_WIN32 || ACE_WIN64
    case NET_WLAN_MONITOR_STATE_CONNECT:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_CONNECT;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);
      ACE_ASSERT (!configuration_->SSID.empty ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _GUID interface_identifier =
#else
      std::string interface_identifier =
#endif // ACE_WIN32 || ACE_WIN64
        configuration_->interfaceIdentifier;
      //struct ether_addr ap_mac_address_s;
      //ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      // check cache whether the configured ESSID (if any) is known
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
          SSIDCache_.find (configuration_->SSID);
        ACE_ASSERT (iterator != SSIDCache_.end ());
        interface_identifier = (*iterator).second.first;
        //ap_mac_address_s = (*iterator).second.second.linkLayerAddress;
      } // end lock scope
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (!InlineIsEqualGUID (interface_identifier, configuration_->interfaceIdentifier)))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("found SSID (was: %s) on interface \"%s\" (configured interface was: \"%s\"), continuing\n"),
                    ACE_TEXT (configuration_->SSID.c_str ()),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ())));
#else
      if (unlikely (ACE_OS::strcmp (interface_identifier.c_str (),
                                    configuration_->interfaceIdentifier.c_str ())))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("found SSID (was: %s) on interface \"%s\" (configured interface was: \"%s\"), continuing\n"),
                    ACE_TEXT (configuration_->SSID.c_str ()),
                    ACE_TEXT (interface_identifier.c_str ()),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

      std::string SSID_string = this->SSID ();
      if (unlikely (!SSID_string.empty ()                       &&
                    ACE_OS::strcmp (configuration_->SSID.c_str (),
                                    SSID_string.c_str ())))
      {
        inherited::change (NET_WLAN_MONITOR_STATE_DISCONNECT);
        break;
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      try {
        do_associate (interface_identifier,
                      configuration_->SSID);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
      }
#elif defined (ACE_LINUX)
      // *NOTE*: query the interface-local DHCP server on the current IP address
      //         lease (if any), and/or request an address for the configured
      //         interface
//      bool result = false;
//      ACE_Time_Value result_poll_interval (0,
//                                           NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL * 1000);
//      ACE_Time_Value result_timeout (NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT,
//                                     0);
//      ACE_Time_Value deadline;
//      bool shutdown = false;

#if defined (DHCLIENT_USE)
      ACE_ASSERT (dhclientCBData_.connection == dhcpctl_null_handle);
      dhcpctl_handle authenticator_h = dhcpctl_null_handle;
      ACE_INET_Addr inet_address (static_cast<u_short> (DHCP_DHCLIENT_OMAPI_PORT),
                                  ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_LOCALHOST_IP_STRING),
                                  AF_INET);
      if (unlikely (!DHCP_Tools::connectDHClient (inet_address,
                                                  authenticator_h,
                                                  net_wlan_dhclient_connection_event_cb,
                                                  &dhclientCBData_,
                                                  dhclientCBData_.connection)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to DHCP_Tools::connectDHClient(%s), returning\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inet_address, false).c_str ())));
        break;
      } // end IF
      ACE_ASSERT (dhclientCBData_.connection != dhcpctl_null_handle);
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("connected to dhclient process (address was: %s): %d/%d\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (inet_address, false).c_str ()),
                  omapi_connection_readfd (dhclientCBData_.connection),
                  omapi_connection_writefd (dhclientCBData_.connection)));
#endif // _DEBUG

      // verify interface state, address lease
      if (unlikely (!DHCP_Tools::getInterfaceState (dhclientCBData_.connection,
                                                    configuration_->interfaceIdentifier,
                                                    net_wlan_dhclient_connect_cb,
                                                    &dhclientCBData_)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to DHCP_Tools::getInterfaceState(%@,\"%s\"), returning\n"),
                    dhclientCBData_.connection,
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
        DHCP_Tools::disconnectDHClient (dhclientCBData_.connection);
        dhclientCBData_.connection = NULL;
        break;
      } // end IF
#endif // DHCLIENT_USE
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
    case NET_WLAN_MONITOR_STATE_DISCONNECT:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_DISCONNECT;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);
      ACE_ASSERT (!configuration_->SSID.empty ());
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
          SSIDCache_.find (configuration_->SSID);
        ACE_ASSERT (iterator != SSIDCache_.end ());
      } // end lock scope
      ACE_ASSERT (!this->SSID ().empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // *NOTE*: the Win32 API does not let developers interfere with the DHCP
      //         handshake/addressing process programmatically at this time
#elif defined (ACE_LINUX)
#if defined (DHCLIENT_SUPPORT)
      ACE_ASSERT (dhclientCBData_.connection == dhcpctl_null_handle);
      dhcpctl_handle authenticator_h = dhcpctl_null_handle;
      ACE_INET_Addr inet_address (static_cast<u_short> (DHCP_DHCLIENT_OMAPI_PORT),
                                  ACE_TEXT_ALWAYS_CHAR (DHCP_DHCLIENT_LOCALHOST_IP_STRING),
                                  AF_INET);
      if (unlikely (!DHCP_Tools::connectDHClient (inet_address,
                                                  authenticator_h,
                                                  net_wlan_dhclient_connection_event_cb,
                                                  &dhclientCBData_,
                                                  dhclientCBData_.connection)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to DHCP_Tools::connectDHClient(%s), returning\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inet_address, false).c_str ())));
        break;
      } // end IF
      ACE_ASSERT (dhclientCBData_.connection != dhcpctl_null_handle);
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("connected to dhclient process (address was: %s): %d/%d\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (inet_address, false).c_str ()),
                  omapi_connection_readfd (dhclientCBData_.connection),
                  omapi_connection_writefd (dhclientCBData_.connection)));
#endif // _DEBUG

      if (unlikely (!DHCP_Tools::relinquishLease (dhclientCBData_.connection,
                                                  configuration_->interfaceIdentifier)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to DHCP_Tools::relinquishLease(%@,\"%s\"), returning\n"),
                    dhclientCBData_.connection,
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (inet_address, false).c_str ())));
        DHCP_Tools::disconnectDHClient (dhclientCBData_.connection);
        dhclientCBData_.connection = dhcpctl_null_handle;
        break;
      } // end IF
      DHCP_Tools::disconnectDHClient (dhclientCBData_.connection);
      dhclientCBData_.connection = dhcpctl_null_handle;
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (configuration_);

      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
      try {
        do_associate (configuration_->interfaceIdentifier,
                      ACE_TEXT_ALWAYS_CHAR ("")); // <-- disassociate
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    ACE_TEXT ("")));
      }
#else
      inherited::change (NET_WLAN_MONITOR_STATE_DISASSOCIATE);
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
    //////////////////////////////////////
    case NET_WLAN_MONITOR_STATE_INITIALIZED:
      break;
    case NET_WLAN_MONITOR_STATE_SCANNED:
    {
      std::string SSID_string = this->SSID ();
      enum Net_WLAN_MonitorState next_state_e =
          NET_WLAN_MONITOR_STATE_INVALID;
      // *NOTE*: scans complete asynchronously; change state only when actually
      //         in 'scan mode'
      enum Net_WLAN_MonitorState previous_state_e =
          NET_WLAN_MONITOR_STATE_INVALID;
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        previous_state_e =
          (SSID_string.empty () ? inherited::state_
                                : NET_WLAN_MONITOR_STATE_CONNECTED);
        inherited::state_ = NET_WLAN_MONITOR_STATE_SCANNED;
      } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      // sanity check(s)
      ACE_ASSERT (configuration_);

      try {
        onScanComplete (configuration_->interfaceIdentifier);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onScanComplete(\"%s\"), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
      }
#endif // ACE_WIN32 || ACE_WIN64

      bool done = true;
      switch (previous_state_e)
      {
        case NET_WLAN_MONITOR_STATE_IDLE:
        case NET_WLAN_MONITOR_STATE_SCAN:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        case NET_WLAN_MONITOR_STATE_SCANNED: // *TODO*: remove ASAP
#endif // ACE_WIN32 || ACE_WIN64
        {
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
          inherited::change (NET_WLAN_MONITOR_STATE_IDLE);
          break;
        }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
        case NET_WLAN_MONITOR_STATE_AUTHENTICATE:
        case NET_WLAN_MONITOR_STATE_ASSOCIATE:
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_CONNECT:
        {
          // reset state
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
          { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
            inherited::state_ = previous_state_e;
          } // end lock scope
          break;
        }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
        case NET_WLAN_MONITOR_STATE_AUTHENTICATED:
          next_state_e = NET_WLAN_MONITOR_STATE_DEAUTHENTICATE;
          // *WARNING*: falls through
        case NET_WLAN_MONITOR_STATE_ASSOCIATED:
          next_state_e =
            ((previous_state_e == NET_WLAN_MONITOR_STATE_AUTHENTICATED) ? NET_WLAN_MONITOR_STATE_DEAUTHENTICATE
                                                                        : NET_WLAN_MONITOR_STATE_DISASSOCIATE);
          // *WARNING*: falls through
#endif // ACE_WIN32 || ACE_WIN64
        case NET_WLAN_MONITOR_STATE_CONNECTED:
        {
          next_state_e =
            ((previous_state_e == NET_WLAN_MONITOR_STATE_CONNECTED) ? NET_WLAN_MONITOR_STATE_DISCONNECT
                                                                    : next_state_e);

          // reset state
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
          { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
            inherited::state_ = previous_state_e;
          } // end lock scope

//          ACE_ASSERT (!SSID_string.empty ());
          if (unlikely (!configuration_->SSID.empty () &&
                        ACE_OS::strcmp (configuration_->SSID.c_str (),
                                        SSID_string.c_str ()))) // configured, associated to different SSID
          { ACE_ASSERT (isConnectionNotified_);
            isConnectionNotified_ = false;
            done = false;
          } // end IF
          else
            inherited::change (NET_WLAN_MONITOR_STATE_IDLE);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("unknown/invalid state switch: \"%s\" --> \"%s\" --> check implementation !, continuing\n"),
                      ACE_TEXT (stateToString (previous_state_e).c_str ()),
                      ACE_TEXT (stateToString (NET_WLAN_MONITOR_STATE_SCANNED).c_str ())));
          break;
        }
      } // end SWITCH
      if (done)
        break;

      // configured, associated to different SSID
      ACE_ASSERT (!configuration_->SSID.empty ());
      // --> check cache whether the configured ESSID is within reach
      bool essid_is_cached = false;
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
            SSIDCache_.find (configuration_->SSID);
        essid_is_cached = (iterator != SSIDCache_.end ());
      } // end lock scope

      // (try to) auto-associate ?
      if (unlikely (essid_is_cached              && // configured SSID within reach
                    configuration_->autoAssociate)) // auto-associate enabled
        inherited::change (next_state_e);
      else
        inherited::change (NET_WLAN_MONITOR_STATE_IDLE);
      break;
    }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    case NET_WLAN_MONITOR_STATE_AUTHENTICATED:
    {
      // *NOTE*: set state early to support atomic state transition
      //         notifications
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
        inherited::state_ = NET_WLAN_MONITOR_STATE_AUTHENTICATED;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);

      std::string SSID_string = this->SSID ();
      if ((!configuration_->SSID.empty () &&
           !SSID_string.empty ()          &&
           ACE_OS::strcmp (configuration_->SSID.c_str (),
                           SSID_string.c_str ())) && // authenticated by another access point
          configuration_->autoAssociate)
      {
        inherited::change (NET_WLAN_MONITOR_STATE_DEAUTHENTICATE);
        break;
      } // end IF

      enum Net_WLAN_MonitorState next_state_e =
              ((configuration_->SSID.empty () || (SSID_string == configuration_->SSID)) ? NET_WLAN_MONITOR_STATE_ASSOCIATED
                                                                                        : NET_WLAN_MONITOR_STATE_ASSOCIATE);
      inherited::change (next_state_e);

      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATED:
    {
      // *NOTE*: set state early to support atomic state transition
      //         notifications
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
        inherited::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATED;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);

      std::string SSID_string = this->SSID ();
      if ((!configuration_->SSID.empty () &&
           !SSID_string.empty ()          &&
           ACE_OS::strcmp (configuration_->SSID.c_str (),
                           SSID_string.c_str ())) && // associated to a different SSID
          configuration_->autoAssociate)
      {
        inherited::change (NET_WLAN_MONITOR_STATE_DISASSOCIATE);
        break;
      } // end IF

#if defined (ACE_LINUX)
      // *TODO*: optimize tx-power
#endif // ACE_LINUX

      if (likely (isConnectionNotified_))
        goto continue_2;
      try {
        onAssociate (configuration_->interfaceIdentifier,
                     SSID_string,
                     true);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (SSID_string.c_str ())));
      }

continue_2:
      enum Net_WLAN_MonitorState next_state_e =
              ((configuration_->SSID.empty () || (SSID_string == configuration_->SSID)) ? NET_WLAN_MONITOR_STATE_CONNECTED
                                                                                        : NET_WLAN_MONITOR_STATE_CONNECT);
      inherited::change (next_state_e);

      break;
    }
#endif // ACE_WIN32 || ACE_WIN64
    case NET_WLAN_MONITOR_STATE_CONNECTED:
    {
      // *NOTE*: set state early to support atomic state transition
      //         notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_CONNECTED;
      } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_USE)
      ACE_ASSERT (configuration_);

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
#endif // WLANAPI_USE
#endif // ACE_WIN32 || ACE_WIN64

      if (likely (isConnectionNotified_))
        goto continue_3;
      isConnectionNotified_ = true;

      try {
        onConnect (configuration_->interfaceIdentifier,
                   this->SSID (),
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
                    ACE_TEXT (this->SSID ().c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
      }

continue_3:
      if (unlikely (isFirstConnect_))
        isFirstConnect_ = false;

      // start scanning ?
      ACE_ASSERT (configuration_);

      if (unlikely (configuration_->enableBackgroundScans))
        inherited::change (NET_WLAN_MONITOR_STATE_SCAN);

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid state switch: \"%s\" --> \"%s\" --> check implementation !, returning\n"),
                  ACE_TEXT (stateToString (inherited::state_).c_str ()),
                  ACE_TEXT (stateToString (newState_in).c_str ())));
      break;
    }
  } // end SWITCH
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename AddressType,
          typename ConfigurationType>
bool
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType>::startScanTimer (const ACE_Time_Value& delay_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::startScanTimer"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (scanTimerId_ == -1);
  ACE_ASSERT (timerInterface_);

  // *NOTE*: the timer is restarted automatically in the callback handler
  //         --> do not delay
  ACE_Time_Value elapsed = COMMON_TIME_NOW;
  // *NOTE*: the timer handler might trigger before scanTimerId_ has been set
  //         --> grab the lock
  { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_, result);
    try {
      scanTimerId_ =
        timerInterface_->schedule_timer (&timerHandler_,
                                         NULL, // no ACT
                                         delay_in,
                                         ACE_Time_Value::zero);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_ITimer_T::schedule_timer(%T#), continuing\n"),
                  &delay_in));
    }
    if (unlikely (scanTimerId_ == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_ITimer_T::schedule_timer(%T#): \"%m\", continuing\n"),
                  &delay_in));
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("started WLAN scan timer (id: %d)\n"),
                scanTimerId_));
#endif // _DEBUG
  } // end lock scope

  return true;
}
template <typename AddressType,
          typename ConfigurationType>
bool
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType>::cancelScanTimer ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::cancelScanTimer"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (scanTimerId_ != -1);

  int result_2 = -1;
  const void* ACT_p = NULL;
  try {
    result_2 = timerInterface_->cancel_timer (scanTimerId_,
                                              &ACT_p,
                                              1);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_ITimer_T::cancel_timer(%d), aborting\n"),
                scanTimerId_));
  }
  if (unlikely (result_2 == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_ITimer_T::cancel_timer(%d): \"%m\", aborting\n"),
                scanTimerId_));
  else
    result = true;

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("cancelled WLAN scan timer (id was: %d)\n"),
              scanTimerId_));
#endif // _DEBUG
  scanTimerId_ = -1;
  ACE_UNUSED_ARG (ACT_p);

  return result;
}

template <typename AddressType,
          typename ConfigurationType>
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType>::handle (const void* ACT_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::handle"));

  // *NOTE*: the timer handler might trigger before scanTimerId_ has been set
  //         --> grab the lock
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    if (likely (scanTimerId_ != -1))
      scanTimerId_ = -1;
  } // end lock scope

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
  // *NOTE*: the WinXP SP2/SP3 WLAN API does not report completed scans (see
  //         also: https://docs.microsoft.com/en-us/windows/desktop/api/wlanapi/nf-wlanapi-wlanscan)
  //         so the scan timer also serves as timeout after which results are
  //         retrieved 'manually'
  ACE_ASSERT (configuration_);

  try {
    this->onScanComplete (configuration_->interfaceIdentifier);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("\"%s\": caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ())));
  }
#else
  inherited::change (NET_WLAN_MONITOR_STATE_SCAN);
#endif // _WIN32_WINNT_WINXP
}

#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::onSignalQualityChange (REFGUID interfaceIdentifier_in,
                                                  WLAN_SIGNAL_QUALITY signalQuality_in)
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onSignalQualityChange (const std::string& interfaceIdentifier_in,
                                                                unsigned int signalQuality_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onSignalQualityChange"));

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
              ACE_TEXT (this->SSID ().c_str ()),
              signalQuality_in));
#endif // ACE_WIN32 || ACE_WIN64

  std::string SSID_string = this->SSID ();
  ACE_ASSERT (!SSID_string.empty ());
  Net_WLAN_AccessPointCacheIterator_t iterator;
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // update cache
    iterator = SSIDCache_.find (SSID_string);
    if (likely (iterator != SSIDCache_.end ()))
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType>
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType,
                        ACE_SYNCH_USE,
                        TimePolicyType>::onAssociate (const std::string& interfaceIdentifier_in,
                                                      const std::string& SSID_in,
                                                      bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onAssociate"));

  struct ether_addr ether_addr_s, ether_addr_2;
  ACE_OS::memset (&ether_addr_2, 0, sizeof (struct ether_addr));

  // synch access
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
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
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to associate with access point (SSID was: %s)\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
#endif // _DEBUG
    return;
  } // end IF

#if defined (_DEBUG)
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    Net_WLAN_AccessPointCacheConstIterator_t iterator =
        SSIDCache_.find (configuration_->SSID);
    if (likely (iterator != SSIDCache_.end ())) // *NOTE*: might not have scan results yet
      ether_addr_2 = (*iterator).second.second.linkLayerAddress;
  } // end lock scope
  ether_addr_s =
      Net_Common_Tools::interfaceToLinkLayerAddress (interfaceIdentifier_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": (MAC: %s) associated with access point (MAC: %s; SSID: %s)\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
              ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_2.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#endif // _DEBUG
}
template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType>
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType,
                        ACE_SYNCH_USE,
                        TimePolicyType>::onDisassociate (const std::string& interfaceIdentifier_in,
                                                         const std::string& SSID_in,
                                                         bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onDisassociate"));

  // synch access
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
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
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to disassociate with access point (SSID was: %s)\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
#endif // _DEBUG
    return;
  } // end IF

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": disassociated with access point (SSID was: %s)\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#endif // _DEBUG
}
#endif // ACE_WIN32 || ACE_WIN64

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::onConnect (REFGUID interfaceIdentifier_in,
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onConnect (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                      const std::string& SSID_in,
                                      bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onConnect"));

  struct Net_WLAN_AccessPointState access_point_state_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (!configuration_->SSID.empty ());

  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // check cache whether the configured ESSID (if any) is known
    Net_WLAN_AccessPointCacheConstIterator_t iterator =
      SSIDCache_.find (SSID_in);
    if (iterator != SSIDCache_.end ())
      access_point_state_s = (*iterator).second.second;
    else
      access_point_state_s.linkLayerAddress =
        Net_WLAN_Tools::getAccessPointAddress (clientHandle_,
                                               interfaceIdentifier_in,
                                               SSID_in);
  } // end lock scope

  if (!success_in)
  {
    if (unlikely (retries_ == NET_WLAN_MONITOR_AP_CONNECTION_RETRIES))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to connect to %s (SSID was: %s), giving up\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&access_point_state_s.linkLayerAddress)).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      retries_ = 0;
      inherited::change (NET_WLAN_MONITOR_STATE_SCANNED);
    } // end IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to connect to %s (SSID was: %s), retrying...\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&access_point_state_s.linkLayerAddress)).c_str ()),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      ++retries_;
      inherited::change (NET_WLAN_MONITOR_STATE_CONNECT);
    } // end ELSE
  } // end ELSE
#endif // ACE_WIN32 || ACE_WIN64

  // synch access
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
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

#if defined (_DEBUG)
  // sanity check(s)
  if (!success_in)
    return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!Net_Common_Tools::interfaceToIPAddress_2 (interfaceIdentifier_in,
                                                           localSAP_,
                                                           peerSAP_)))
#else
  if (unlikely (!Net_Common_Tools::interfaceToIPAddress (interfaceIdentifier_in,
                                                         localSAP_,
                                                         peerSAP_)))
#endif // ACE_WIN32 || ACE_WIN64
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
    return;
  } // end IF

  struct ether_addr ether_addr_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ether_addr_s =
      Net_Common_Tools::interfaceToLinkLayerAddress_2 (interfaceIdentifier_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": (MAC: %s) connected to access point (MAC: %s; SSID: %s): %s <---> %s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
              ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&access_point_state_s.linkLayerAddress.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
              ACE_TEXT (SSID_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peerSAP_).c_str ())));
#else
  ether_addr_s =
      Net_Common_Tools::interfaceToLinkLayerAddress (interfaceIdentifier_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": (MAC: %s) connected to access point (MAC: %s; SSID: %s): %s <---> %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
              ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&access_point_state_s.linkLayerAddress.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
              ACE_TEXT (SSID_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (localSAP_).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peerSAP_).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//continue_:
  ;
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
}
template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::onDisconnect (REFGUID interfaceIdentifier_in,
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onDisconnect (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                         const std::string& SSID_in,
                                         bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onDisconnect"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (configuration_);
  //ACE_ASSERT (!configuration_->SSID.empty ());
  ACE_ASSERT (SSID_in.empty ());

  //struct Net_WLAN_AccessPointState access_point_state_s;
  //{ ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
  //  // check cache whether the configured ESSID (if any) is known
  //  Net_WLAN_AccessPointCacheConstIterator_t iterator =
  //    SSIDCache_.find (SSID_in);
  //  if (iterator != SSIDCache_.end ())
  //    access_point_state_s = (*iterator).second.second;
  //} // end lock scope

  if (success_in)
  {
    //if (!ACE_OS::strcmp (SSID_in.c_str (),
    //                     configuration_->SSID.c_str ()))
    if (InlineIsEqualGUID (interfaceIdentifier_in, configuration_->interfaceIdentifier))
      inherited::change (NET_WLAN_MONITOR_STATE_IDLE);
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  // synch access
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
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
                ACE_TEXT (SSID_in.c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to disconnect from access point (SSID was: %s)\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
    return;
  } // end IF

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": disconnected from access point (SSID was: %s)\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": disconnected from access point (SSID was: %s)\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
}

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::onHotPlug (REFGUID interfaceIdentifier_in,
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onHotPlug (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                      bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onHotPlug"));

  // synch access
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
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
template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::onRemove (REFGUID interfaceIdentifier_in,
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onRemove (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                     bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onRemove"));

  // synch access
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
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

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        >::onScanComplete (REFGUID interfaceIdentifier_in)
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onScanComplete (const std::string& interfaceIdentifier_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onScanComplete"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // step1: update the cache
  Net_WLAN_AccessPointCacheConstIterator_t iterator;
#if defined (_DEBUG)
  std::set<std::string> known_ssids, current_ssids;
#endif // _DEBUG
  Net_WLAN_SSIDs_t ssids = Net_WLAN_Tools::getSSIDs (clientHandle_,
                                                     interfaceIdentifier_in);
  struct Net_WLAN_AccessPointState access_point_state_s;
  access_point_state_s.linkLayerAddress =
    Net_WLAN_Tools::getAccessPointAddress (clientHandle_,
                                           interfaceIdentifier_in,
                                           configuration_->SSID);
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // clear cache
    do
    {
      iterator =
          std::find_if (SSIDCache_.begin (), SSIDCache_.end (),
                        std::bind2nd (Net_WLAN_AccessPointCacheFindPredicate (),
                                      interfaceIdentifier_in));
      if (iterator == SSIDCache_.end ())
        break;

#if defined (_DEBUG)
      known_ssids.insert ((*iterator).first);
#endif // _DEBUG
      SSIDCache_.erase ((*iterator).first);
    } while (true);

    for (Net_WLAN_SSIDsIterator_t iterator_2 = ssids.begin ();
         iterator_2 != ssids.end ();
         ++iterator_2)
    {
      SSIDCache_.insert (std::make_pair (*iterator_2,
                                         std::make_pair (interfaceIdentifier_in,
                                                         access_point_state_s)));
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
                  ACE_TEXT ("\"%s\": lost contact to (E)SSID (was: %s)\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                  ACE_TEXT ((*iterator_2).c_str ())));
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64

  // synch access
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inherited::change (NET_WLAN_MONITOR_STATE_SCANNED);
#endif // ACE_WIN32 || ACE_WIN64
}
