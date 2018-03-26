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
#include <net/ethernet.h>

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
                  false)
 , timerInterface_ (NULL)
#else
#if defined (WEXT_SUPPORT)
 , buffer_ (NULL)
 , bufferSize_ (0)
 , handle_ (ACE_INVALID_HANDLE)
// , isRegistered_ (false)
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
 , familyId_ (0)
 , socketHandle_ (NULL)
#endif // NL80211_SUPPORT
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
#else
#if defined (DHCLIENT_SUPPORT)
  isc_result_t status_i = dhcpctl_initialize ();
  if (status_i != ISC_R_SUCCESS)
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
#elif defined (NL80211_SUPPORT)
  if (unlikely (handle_))
    nl_socket_free (handle_);
#elif defined (DBUS_SUPPORT)
#endif // WEXT_SUPPORT
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
                        >::~Net_WLAN_Monitor_Base_T ()
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::get1RR (const std::string& SSID_in) const
#endif // ACE_WIN32 || ACE_WIN64
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::get1R"));

  Net_WLAN_AccessPointCacheValue_t return_value_s;

  Net_WLAN_AccessPointCacheConstIterator_t iterator;
  { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_, return_value_s);
    iterator = SSIDCache_.find (SSID_in);
    if (iterator != SSIDCache_.end ())
      return_value_s = (*iterator).second;
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
#else
#if defined (WEXT_SUPPORT)
    if (buffer_)
    {
      ACE_OS::free (buffer_);
      buffer_ = NULL;
    } // end IF
    bufferSize_ = 0;
    ACE_ASSERT (handle_ == ACE_INVALID_HANDLE);
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
    familyId_ = 0;

    if (socketHandle_)
    {
      nl_socket_free (socketHandle_);
      socketHandle_ = NULL;
    } // end  IF
#endif // NL80211_SUPPORT
#if defined (DBUS_SUPPORT)
#endif // DBUS_SUPPORT
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
//  nl_socket_set_nonblocking (handle_);
// nl_socket_disable_msg_peek (handle_);
//  nl_socket_set_buffer_size (handle_, int rx, int tx);
//  nl_socket_disable_auto_ack (handle_);

  int result = nl_connect (socketHandle_, NETLINK_GENERIC);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_connect(0x%@): \"%s\", aborting\n"),
                socketHandle_,
                ACE_TEXT (nl_geterror (result))));
    return false;
  } // end IF

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename AddressType,
          typename ConfigurationType>
const WLAN_SIGNAL_QUALITY
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType>::get_2 () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::get_2"));

  // sanity check(s)
  if (unlikely (!isActive_))
    return 0;
  std::string SSID_string = SSID ();
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
#endif // ACE_WIN32 || ACE_WIN64

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
                  ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceIdentifierToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&const_cast<struct ether_addr&> (accessPointLinkLayerAddress_in).ether_addr_octet)).c_str ()),
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
    start ();

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
#endif // WLANAPI_USE
#else
  if (interfaceIdentifier_in.empty ())
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces (
// *TODO*: this compiles only one implementation; move out of the base class
#if defined (WEXT_USE)
#elif defined (NL80211_USE)
                                                             socketHandle_,
                                                             familyId_,
#elif defined (DBUS_USE)
#endif // DBUS_USE
                                                             AF_UNSPEC,
                                                             0);
#endif // ACE_WIN32 || ACE_WIN64
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);
  if (interface_identifiers_a.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no WLAN interfaces found; cannot auto-select, returning\n")));
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
                ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s), returning\n")));
                ACE_TEXT (Net_Common_Tools::interfaceIdentifierToString (interfaceIdentifier_in).c_str ()),
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
    configuration_->interfaceIdentifier = interface_identifiers_a.front ();
    configuration_->SSID.clear ();
//    isConnectionNotified_ = false;
  } // end IF

  if (restart_b)
    start ();
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
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_IDLE;
      } // end lock scope

      if (!isActive_ ||              // stop()ped
          inherited::hasShutDown ())
      {
        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_INITIALIZED);
        break;
      } // end IF
      std::string SSID_string = this->SSID ();
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
        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
        break;

continue_:
        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCAN);
#endif // ACE_WIN32 || ACE_WIN64
        break;
      } // end IF

      // not connected

      // sanity check(s)
      ACE_ASSERT (configuration_);

      bool essid_is_cached = false;
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
            SSIDCache_.find (configuration_->SSID);
        essid_is_cached = (iterator != SSIDCache_.end ());
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
        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCAN);
        goto reset_state;
      } // end IF

      ACE_ASSERT (SSID_string.empty () && !configuration_->SSID.empty () && essid_is_cached && configuration_->autoAssociate);

      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);

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
#else
#if defined (_DEBUG)
#if defined (WEXT_SUPPORT)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": scanning...\n"),
                  ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
#else
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": [%T] scanning...\n"),
                  ACE_TEXT (configuration_->interfaceIdentifier).c_str ()));
#endif // _DEBUG
#endif // WEXT_SUPPORT || NL80211_SUPPORT || DBUS_SUPPORT
#if defined (WEXT_SUPPORT)
#if defined (_DEBUG)
      ACE_Time_Value scan_time;
      std::string scan_time_string;
      ACE_High_Res_Timer timer;
      timer.start ();
#endif // _DEBUG
#endif // WEXT_SUPPORT
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

#if defined (WEXT_SUPPORT)
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

      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCANNED);
#endif // WEXT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATE:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATE;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);
      ACE_ASSERT (!configuration_->SSID.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _GUID interface_identifier = GUID_NULL;
#else
      std::string interface_identifier;
      struct ether_addr ap_mac_address_s;
#if defined (WEXT_SUPPORT)
      struct ether_addr ether_addr_s;
      bool result = false;
      int result_2 = -1;
      ACE_Time_Value result_poll_interval (0,
                                           NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL * 1000);
      ACE_Time_Value result_timeout (NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT,
                                     0);
      ACE_Time_Value deadline;
      bool shutdown = false;
#endif // WEXT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
          SSIDCache_.find (configuration_->SSID);
        ACE_ASSERT (iterator != SSIDCache_.end ());
        interface_identifier = (*iterator).second.first;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
        ap_mac_address_s = (*iterator).second.second.linkLayerAddress;
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
      if (unlikely (ACE_OS::strcmp (interface_identifier.c_str (),
                                    configuration_->interfaceIdentifier.c_str ())))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("found SSID (was: %s) on interface \"%s\" (configured interface was: \"%s\"), continuing\n"),
                    ACE_TEXT (configuration_->SSID.c_str ()),
                    ACE_TEXT (interface_identifier.c_str ()),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

      std::string SSID_string = this->SSID ();
      if (!SSID_string.empty () &&
          ACE_OS::strcmp (SSID_string.c_str (),
                          configuration_->SSID.c_str ()))
      {
        try {
          do_associate (interface_identifier,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                        ap_mac_address_s,
#endif // ACE_WIN32 || ACE_WIN64
                        ACE_TEXT_ALWAYS_CHAR ("")); // <-- disassociate
        } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                      ACE_TEXT (Net_Common_Tools::interfaceIdentifierToString (interface_identifier).c_str ()),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                      ACE_TEXT ("")));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                      ACE_TEXT (interface_identifier.c_str ()),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                      ACE_TEXT ("")));
#endif // ACE_WIN32 || ACE_WIN64
        }
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (WEXT_SUPPORT)
associate:
#endif // WEXT_SUPPORT && defined (WEXT_USE)
#endif // ACE_WIN32 || ACE_WIN64
      try {
        do_associate (interface_identifier,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                      ap_mac_address_s,
#endif // ACE_WIN32 || ACE_WIN64
                      configuration_->SSID);
      } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceIdentifierToString (interface_identifier).c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (interface_identifier.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT (configuration_->SSID.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
      }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (WEXT_SUPPORT)
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
          inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
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

      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
#endif // WEXT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
    case NET_WLAN_MONITOR_STATE_DISASSOCIATE:
    {
      // *NOTE*: actually 'set' intermediate states to support atomic state
      //         transition notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_DISASSOCIATE;
      } // end lock scope

      // sanity check(s)
//      std::string SSID_string = SSID ();
//      ACE_ASSERT (!SSID_string.empty ()); // associated
      ACE_ASSERT (configuration_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      struct ether_addr ap_mac_address_s;
      ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
#endif // ACE_WIN32 || ACE_WIN64
      try {
        do_associate (configuration_->interfaceIdentifier,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                      ap_mac_address_s,
#endif // ACE_WIN32 || ACE_WIN64
                      ACE_TEXT_ALWAYS_CHAR ("")); // <-- disassociate
      } catch (...) {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceIdentifierToString (configuration_->interfaceIdentifier).c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT ("")));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IManager::do_associate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT ("")));
#endif // ACE_WIN32 || ACE_WIN64
      }

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
        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
        break;
      } // end IF

      // --> configured && configured SSID known (i.e. cached)
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCANNED);
      break;
    }
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
      //{ ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
      //  // check cache whether the configured ESSID (if any) is known
      //  Net_WLAN_AccessPointCacheConstIterator_t iterator =
      //    SSIDCache_.find (configuration_->SSID);
      //  ACE_ASSERT (iterator != SSIDCache_.end ());
      //} // end lock scope
      std::string SSID_string = this->SSID ();
      ACE_ASSERT (!SSID_string.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // *NOTE*: apparently, the Win32 API currently does not let developers
      //         interfere with the WLAN DHCP addressing process
      //         programmatically
      //ACE_ASSERT (!ACE_OS::strcmp (configuration_->SSID.c_str (),
      //                             SSID_string.c_str ()));
#else
      // *NOTE*: query the interface-local DHCP server on the current IP address
      //         lease (if any), and/or request an address for the configured
      //         interface
//      bool result = false;
      ACE_Time_Value result_poll_interval (0,
                                           NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_RESULT_POLL_INTERVAL * 1000);
      ACE_Time_Value result_timeout (NET_WLAN_MONITOR_ASSOCIATION_DEFAULT_TIMEOUT,
                                     0);
      ACE_Time_Value deadline;
//      bool shutdown = false;
#if defined (DHCLIENT_SUPPORT)
      // *IMPORTANT NOTE*: for this to work:
      //                   - a local ISC 'dhclient' process must be running
      //                   - it must have been configured to listen for OMAPI
      //                     (see also: man omapi(3), dhclient.conf(5)) traffic
      // *NOTE*: the last precondition requires an entry 'omapi port [port]' in
      //         /etc/dhclient.conf; this feature is not documented in current
      //         versions, however
      // sanity check(s)
      if (unlikely (!Common_Tools::getProcessId (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DHCLIENT_EXECUTABLE_STRING))))
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("DHCP client (executable was: \"%s\") process not found, continuing\n"),
                    ACE_TEXT (NET_WLAN_MONITOR_DHCLIENT_EXECUTABLE_STRING)));

      dhcpctl_handle connection_h = dhcpctl_null_handle;
      dhcpctl_handle authenticator_h = dhcpctl_null_handle;
      dhcpctl_handle interface_h = dhcpctl_null_handle;
      dhcpctl_status wait_status = -1;
      dhcpctl_data_string result_string = NULL;
      dhcpctl_status status_i =
          dhcpctl_connect (&connection_h,
                           ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DHCLIENT_LOCALHOST_IP_STRING),
                           NET_WLAN_MONITOR_DHCLIENT_OMAPI_PORT,
                           authenticator_h);
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        if (status_i == ISC_R_NOMORE)
          ; // 29: client not listening on OMAPI port
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_connect(%s:%u): \"%s\", returning\n"),
                    ACE_TEXT (NET_WLAN_MONITOR_DHCLIENT_LOCALHOST_IP_STRING),
                    NET_WLAN_MONITOR_DHCLIENT_OMAPI_PORT,
                    ACE_TEXT (isc_result_totext (status_i))));
        break;
      } // end IF
      ACE_ASSERT (connection_h != dhcpctl_null_handle);
      status_i =
          dhcpctl_new_object (&interface_h,
                              connection_h,
                              ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DHCLIENT_OBJECT_INTERFACE_STRING));
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_new_object(0x%@,%s): \"%s\", returning\n"),
                    connection_h,
                    ACE_TEXT (NET_WLAN_MONITOR_DHCLIENT_OBJECT_INTERFACE_STRING),
                    ACE_TEXT (isc_result_totext (status_i))));
        goto clean;
      } // end IF
      ACE_ASSERT (interface_h != dhcpctl_null_handle);
      status_i =
          dhcpctl_set_string_value (interface_h,
                                    ACE_TEXT_ALWAYS_CHAR (configuration_->interfaceIdentifier.c_str ()),
                                    ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DHCLIENT_OBJECT_VALUE_NAME_STRING));
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_set_string_value(0x%@,%s,\"%s\"): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (NET_WLAN_MONITOR_DHCLIENT_OBJECT_VALUE_NAME_STRING),
                    ACE_TEXT (isc_result_totext (status_i))));
        goto clean;
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
        goto clean;
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
        goto clean;
      } // end IF
      if (unlikely (wait_status != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_open_object(0x%@,create|excl): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (isc_result_totext (wait_status))));
        goto clean;
      } // end IF
      status_i =
          dhcpctl_get_value (&result_string,
                             interface_h,
                             ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_DHCLIENT_OBJECT_VALUE_STATE_STRING));
      if (unlikely (status_i != ISC_R_SUCCESS))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::dhcpctl_get_value(0x%@,\"%s\"): \"%s\", returning\n"),
                    interface_h,
                    ACE_TEXT (NET_WLAN_MONITOR_DHCLIENT_OBJECT_VALUE_STATE_STRING),
                    ACE_TEXT (isc_result_totext (wait_status))));
        goto clean;
      } // end IF
      ACE_ASSERT (result_string);
      dhcpctl_data_string_dereference (&result_string, MDL);

clean:
      if (likely (interface_h != dhcpctl_null_handle))
      {
        status_i = omapi_object_dereference (&interface_h, MDL);
        if (unlikely (status_i != ISC_R_SUCCESS))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ::omapi_object_dereference(0x%@): \"%s\", continuing\n"),
                      interface_h,
                      ACE_TEXT (isc_result_totext (wait_status))));
      } // end IF
      if (likely (connection_h != dhcpctl_null_handle))
      {
        status_i = omapi_disconnect (connection_h, 0);
        if (unlikely (status_i != ISC_R_SUCCESS))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ::omapi_disconnect(0x%@,%d): \"%s\", continuing\n"),
                      connection_h, 0,
                      ACE_TEXT (isc_result_totext (wait_status))));
      } // end IF
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_CONNECTED);
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
      std::string SSID_string = this->SSID ();
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

      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_DISASSOCIATE);
      break;
    }
    //////////////////////////////////////
    case NET_WLAN_MONITOR_STATE_INITIALIZED:
      break;
    case NET_WLAN_MONITOR_STATE_SCANNED:
    {
      // *NOTE*: set state early to support atomic state transition
      //         notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_SCANNED;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);

      std::string SSID_string = this->SSID ();
      if (unlikely (!isConnectionNotified_ &&
                    ((configuration_->SSID.empty () &&
                      !SSID_string.empty ()) ||                 // not configured, associated
                     (!configuration_->SSID.empty () &&
                      !SSID_string.empty ()          &&
                      !ACE_OS::strcmp (configuration_->SSID.c_str (),
                                      SSID_string.c_str ()))))) // already associated to configured ESSID
      {
        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
        break;
      } // end IF

      bool essid_is_cached = false;
      { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
        // check cache whether the configured ESSID (if any) is known
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
            SSIDCache_.find (configuration_->SSID);
        essid_is_cached = (iterator != SSIDCache_.end ());
      } // end lock scope

      if (unlikely (SSID_string.empty ()           && // not associated
                    !configuration_->SSID.empty () && // configured
                    essid_is_cached                && // configured SSID known (i.e. cached)
                    configuration_->autoAssociate))   // auto-associate enabled
      {
        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);
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

      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);

      break;
    }
    case NET_WLAN_MONITOR_STATE_ASSOCIATED:
    {
      // *NOTE*: set state early to support atomic state transition
      //         notifications
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      { ACE_GUARD (ACE_NULL_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#else
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATED;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (configuration_);

      std::string SSID_string = this->SSID ();
      ACE_ASSERT (!SSID_string.empty ());
      if (unlikely (ACE_OS::strcmp (configuration_->SSID.c_str (),
                                    SSID_string.c_str ()))) // associated to a different SSID
      {
        inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_DISASSOCIATE);
        break;
      } // end IF

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
        ACE_DEBUG ((LM_ERROR,NET_WLAN_MONITOR_STATE_DISASSOCIATE
                    ACE_TEXT ("\"%s\": failed to Net_WLAN_Tools::setDeviceSettingBool(0x%@,%d,%s), continuing\n"),
                    ACE_TEXT (Net_Common_Tools::interfaceToString (configuration_->interfaceIdentifier).c_str ()),
                    clientHandle_,
                    wlan_intf_opcode_media_streaming_mode,
                    (configuration_->enableMediaStreamingMode ? ACE_TEXT ("true") : ACE_TEXT ("false"))));
#else
      // *TODO*: optimize tx-power
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      if (likely (isConnectionNotified_))
        goto continue_2;
      try {
        onAssociate (configuration_->interfaceIdentifier,
                     this->SSID (),
                     true);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onAssociate(\"%s\",%s,true), continuing\n"),
                    ACE_TEXT (configuration_->interfaceIdentifier.c_str ()),
                    ACE_TEXT (this->SSID ().c_str ())));
      }
continue_2:
#endif // ACE_WIN32 || ACE_WIN64

      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_CONNECT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//reset_state:
      { ACE_GUARD (ACE_MT_SYNCH::MUTEX, aGuard, *(inherited::stateLock_));
        inherited::state_ = NET_WLAN_MONITOR_STATE_ASSOCIATED;
      } // end lock scope
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (scanTimerId_ == -1))
        startScanTimer ();
#else
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
#endif // ACE_WIN32 || ACE_WIN64
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
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType>::startScanTimer ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::startScanTimer"));

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
#endif // _DEBUG
}
template <typename AddressType,
          typename ConfigurationType>
void
Net_WLAN_Monitor_Base_T<AddressType,
                        ConfigurationType>::cancelScanTimer ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::cancelScanTimer"));

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
                        >::onAssociate (REFGUID interfaceIdentifier_in,
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onAssociate (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                        const std::string& SSID_in,
                                        bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onAssociate"));

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
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
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
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
    } // end IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to associate with %s (interface was: \"%s\", SSID was: %s), retrying...\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ether_addr_2.ether_addr_octet)).c_str ()),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      ++retries_;
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATE);
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
                ACE_TEXT (SSID_in.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
    return;
  } // end IF

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": (MAC: %s) associated with access point (MAC: %s; SSID: %s)\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&Net_Common_Tools::interfaceToLinkLayerAddress (interfaceIdentifier_in)), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_2.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
#else
  Net_WLAN_AccessPointCacheConstIterator_t iterator;
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    iterator =
      SSIDCache_.find (configuration_->SSID);
    if (unlikely (iterator == SSIDCache_.end ()))
      goto continue_; // *NOTE*: nost probable reason: not scanned yet (associated on start ?)
    struct ether_addr ether_addr_s =
        Net_Common_Tools::interfaceToLinkLayerAddress (interfaceIdentifier_in);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": (MAC: %s) associated with access point (MAC: %s; SSID: %s)\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_s), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&(*iterator).second.second), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
  } // end lock scope
#endif // ACE_WIN32 || ACE_WIN64
continue_:
  ;
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
                        >::onDisassociate (REFGUID interfaceIdentifier_in,
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onDisassociate (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                           const std::string& SSID_in,
                                           bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onDisassociate"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (configuration_);

  struct ether_addr ether_addr_s =
    Net_WLAN_Tools::associatedBSSID (clientHandle_,
                                     interfaceIdentifier_in);
  struct Association_Configuration association_configuration;
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // check cache whether the configured ESSID (if any) is known
    Net_WLAN_AccessPointCacheConstIterator_t iterator =
      SSIDCache_.find (SSID_in);
    ACE_ASSERT (iterator != SSIDCache_.end ());
    association_configuration = (*iterator).second.second;
  } // end lock scope
  if (success_in)
  {
    if (!ACE_OS::memcmp (&ether_addr_s.ether_addr_octet,
                         &association_configuration.accessPointLinkLayerAddress.ether_addr_octet,
                         ETH_ALEN))
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);
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
                ACE_TEXT (SSID_in.c_str ())));
#else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": failed to disassociate with access point (SSID was: %s)\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (SSID_in.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
    return;
  } // end IF

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": disassociated with access point (SSID was: %s)\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (SSID_in.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": disassociated with access point (SSID was: %s)\n"),
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
                        >::onConnect (REFGUID interfaceIdentifier_in,
#else
                        ,ACE_SYNCH_USE,
                        TimePolicyType>::onConnect (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                      const std::string& SSID_in,
                                      bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_Base_T::onConnect"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (!configuration_->SSID.empty ());

  struct Association_Configuration association_configuration;
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // check cache whether the configured ESSID (if any) is known
    Net_WLAN_AccessPointCacheConstIterator_t iterator =
      SSIDCache_.find (SSID_in);
    ACE_ASSERT (iterator != SSIDCache_.end ());
    association_configuration = (*iterator).second.second;
  } // end lock scope

  if (success_in)
  {
    if (!ACE_OS::strcmp (SSID_in.c_str (),
                         configuration_->SSID.c_str ()) &&
        !isFirstConnect_)
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_CONNECTED);
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
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
    } // end IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s (interface was: \"%s\", SSID was: %s), retrying...\n"),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&association_configuration.accessPointLinkLayerAddress)).c_str ()),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      ++retries_;
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_CONNECT);
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
//#endif // ACE_WIN32 || ACE_WIN64
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
  ACE_ASSERT (!configuration_->SSID.empty ());

  struct Association_Configuration association_configuration;
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // check cache whether the configured ESSID (if any) is known
    Net_WLAN_AccessPointCacheConstIterator_t iterator =
      SSIDCache_.find (SSID_in);
    ACE_ASSERT (iterator != SSIDCache_.end ());
    association_configuration = (*iterator).second.second;
  } // end lock scope

  if (success_in)
  {
    if (!ACE_OS::strcmp (SSID_in.c_str (),
                         configuration_->SSID.c_str ()))
      inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_ASSOCIATED);
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
  struct Association_Configuration association_configuration;
  association_configuration.accessPointLinkLayerAddress =
    Net_WLAN_Tools::getAccessPointAddress (clientHandle_,
                                           interfaceIdentifier_in,
                                           configuration_->SSID);
  { ACE_GUARD (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, subscribersLock_);
    // clear cache
    do
    {
      iterator =
          std::find_if (SSIDCache_.begin (), SSIDCache_.end (),
                        std::bind2nd (Net_WLAN_SSIDToInterfaceIdentifierFindPredicate (),
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

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCANNED);
#else
#if defined (NL80211_SUPPORT)
  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_SCANNED);
#endif  // NL80211_SUPPORT
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
}
