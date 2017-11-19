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

#include "net_common_tools.h"
#include "net_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
Net_WLAN_InetMonitor_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       MonitorAPI_e,
                       UserDataType>::Net_WLAN_InetMonitor_T ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_InetMonitor_T::Net_WLAN_InetMonitor_T"));

}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          enum Net_WLAN_MonitorAPI MonitorAPI_e,
          typename UserDataType>
void
Net_WLAN_InetMonitor_T<ACE_SYNCH_USE,
                       TimePolicyType,
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
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_InetMonitor_T::onConnect"));

  // synch access
  { ACE_GUARD (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_);
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

  // sanity check(s)
  if (!success_in)
    return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!Net_Common_Tools::interfaceToIPAddress (interfaceIdentifier_in,
#else
  if (!Net_Common_Tools::interfaceToIPAddress (interfaceIdentifier_in,
#endif
                                               inherited::localSAP_,
                                               inherited::peerSAP_))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
#endif
    return;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": connected to SSID %s: %s <---> %s\n"),
              ACE_TEXT (Net_Common_Tools::interfaceToString (interfaceIdentifier_in).c_str ()),
              ACE_TEXT (SSID_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::localSAP_).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::peerSAP_).c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": connected to SSID %s: %s <---> %s\n"),
              ACE_TEXT (interfaceIdentifier_in.c_str ()),
              ACE_TEXT (SSID_in.c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::localSAP_).c_str ()),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (inherited::peerSAP_).c_str ())));
#endif
}
