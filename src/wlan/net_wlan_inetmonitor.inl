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

#include "common_macros.h"

#include "net_common_tools.h"
#include "net_macros.h"

template <typename ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
          ACE_SYNCH_DECL,
          typename TimePolicyType,
#endif // ACE_WIN32 || ACE_WIN64
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
Net_WLAN_InetMonitor_T<ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                       ACE_SYNCH_USE,
                       TimePolicyType,
#endif // ACE_WIN32 || ACE_WIN64
                       MonitorAPI_e,
                       UserDataType>::Net_WLAN_InetMonitor_T ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_InetMonitor_T::Net_WLAN_InetMonitor_T"));

}

//////////////////////////////////////////

template <typename ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
          ACE_SYNCH_DECL,
          typename TimePolicyType,
#endif // ACE_WIN32 || ACE_WIN64
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_InetMonitor_T<ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                       ACE_SYNCH_USE,
                       TimePolicyType,
#endif // ACE_WIN32 || ACE_WIN64
                       MonitorAPI_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       UserDataType>::onConnect (REFGUID interfaceIdentifier_in,
#else
                       UserDataType>::onConnect (const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
                                                 const std::string& SSID_in,
                                                 bool success_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_InetMonitor_T::onConnect"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (clientHandle_ != ACE_INVALID_HANDLE);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (!configuration_->SSID.empty ());

  struct ether_addr ether_addr_s =
    Net_WLAN_Tools::getAccessPointAddress (inherited::clientHandle_,
                                           interfaceIdentifier_in,
                                           SSID_in);

  if (!success_in)
  {
    if (unlikely (retries_ == NET_WLAN_MONITOR_AP_CONNECTION_RETRIES))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to connect to %s (SSID was: %s), giving up\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ether_addr_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      retries_ = 0;
      inherited::change (NET_WLAN_MONITOR_STATE_SCANNED);
    } // end IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to connect to %s (SSID was: %s), retrying...\n"),
                  ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
                  ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ether_addr_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                  ACE_TEXT (SSID_in.c_str ())));
      ++retries_;
      inherited::change (NET_WLAN_MONITOR_STATE_CONNECT);
    } // end ELSE
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  // synch access
  { ACE_GUARD (typename ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_);
    // *NOTE*: this works because the lock is recursive
    // *WARNING* if callees unsubscribe() within the callback bad things
    //           happen, as the current iterator is invalidated
    //           --> use a slightly modified for-loop (advance first before
    //               invoking the callback (works for MOST containers...)
    for (typename inherited::SUBSCRIBERS_ITERATOR_T iterator = inherited::subscribers_.begin ();
         iterator != inherited::subscribers_.end ();
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

#if defined (_DEBUG)
  // sanity check(s)
  if (!success_in)
    return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!Net_Common_Tools::interfaceToIPAddress_2 (interfaceIdentifier_in,
                                                           inherited::localSAP_,
                                                           inherited::peerSAP_)))
#else
  if (unlikely (!Net_Common_Tools::interfaceToIPAddress (interfaceIdentifier_in,
                                                         inherited::localSAP_,
                                                         inherited::peerSAP_)))
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": (MAC: %s) connected to access point (MAC: %s; SSID: %s): %s <---> %s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&Net_Common_Tools::interfaceToLinkLayerAddress_2 (interfaceIdentifier_in)), NET_LINKLAYER_802_11).c_str ()),
              ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_s.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
              ACE_TEXT (SSID_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::localSAP_).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::peerSAP_).c_str ())));
#else
  Net_WLAN_AccessPointCacheConstIterator_t iterator;
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_);
    iterator =
      inherited::SSIDCache_.find (inherited::configuration_->SSID);
    if (unlikely (iterator == inherited::SSIDCache_.end ()))
      goto continue_; // *NOTE*: nost probable reason: not scanned yet (connected on start ?)
    struct ether_addr ether_addr_s =
        Net_Common_Tools::interfaceToLinkLayerAddress (interfaceIdentifier_in);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("\"%s\": (MAC: %s) connected to access point (MAC: %s; SSID: %s): %s <---> %s\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&ether_addr_s), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&(*iterator).second.second.linkLayerAddress.ether_addr_octet), NET_LINKLAYER_802_11).c_str ()),
                ACE_TEXT (SSID_in.c_str ()),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::localSAP_).c_str ()),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::peerSAP_).c_str ())));
  } // end lock scope
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
continue_:
  ;
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG
}
