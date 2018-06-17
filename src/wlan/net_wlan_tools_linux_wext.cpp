/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
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
#include "stdafx.h"

#include "net_wlan_tools.h"

#include <sys/capability.h>
#include <linux/capability.h>
//#include <linux/ieee80211.h>
#include <net/if_arp.h>
#include "ifaddrs.h"
#include "iwlib.h"

#include "ace/Handle_Set.h"
#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch.h"

#include "common_tools.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "net_wlan_defines.h"

std::string
Net_WLAN_Tools::decodeSSID (void* data_in,
                            size_t length_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::decodeSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  ACE_ASSERT (data_in);
  ACE_ASSERT (length_in && length_in <= IW_ESSID_MAX_SIZE);

  uint8_t* data_p = static_cast<uint8_t*> (data_in);
  char buffer_a[4 + 1]; // '\\xAB\0'
  int result_2 = -1;
  for (size_t i = 0;
       i < length_in;
       ++i)
  {
    if (::isprint (data_p[i]) &&
        data_p[i] != ' '      &&
        data_p[i] != '\\')
      result += static_cast<char> (data_p[i]);
    else if (data_p[i] == ' ')
      result += ' ';
    else
    { ACE_ASSERT (data_p[i] == '\\'); // --> unicode character
      ACE_OS::memset (buffer_a, 0, sizeof (char[4 + 1]));
      result_2 =
          ACE_OS::sprintf (buffer_a,
                           ACE_TEXT_ALWAYS_CHAR ("\\x%.2x"),
                           data_p[i + 1]);
      ACE_ASSERT (result_2 == 4);
      result += buffer_a;
    } // end ELSE
  } // end FOR

  return result;
}

Net_InterfaceIdentifiers_t
Net_WLAN_Tools::getInterfaces (int addressFamily_in,
                               int flags_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getInterfaces"));

  // initialize return value(s)
  Net_InterfaceIdentifiers_t result;

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
    if (!Net_WLAN_Tools::isInterface (ifaddrs_2->ifa_name))
      continue;
    if (unlikely (addressFamily_in != AF_UNSPEC))
    { ACE_ASSERT (ifaddrs_2->ifa_addr);
//      if (!ifaddrs_2->ifa_addr)
//        continue;
      if (addressFamily_in == AF_MAX)
      {
        if ((ifaddrs_2->ifa_addr->sa_family != AF_INET) &&
            (ifaddrs_2->ifa_addr->sa_family != AF_INET6))
          continue;
      } // end IF
      else if (ifaddrs_2->ifa_addr->sa_family != addressFamily_in)
        continue;
    } // end IF
//    else
//    { ACE_ASSERT (ifaddrs_2->ifa_addr);
//      if (unlikely (ifaddrs_2->ifa_addr->sa_family == AF_PACKET))
//      continue;
//    } // end IF
    if (unlikely (flags_in &&
                  !(ifaddrs_2->ifa_flags & flags_in)))
      continue;

    result.push_back (ifaddrs_2->ifa_name);
  } // end FOR

  // clean up
  ::freeifaddrs (ifaddrs_p);

  // *NOTE*: there may be duplicate entries (e.g. one for each address family)
  //         --> remove duplicates
  std::sort (result.begin (), result.end ());
  Net_InterfacesIdentifiersIterator_t iterator =
      std::unique (result.begin (), result.end ());
  result.erase (iterator, result.end ());
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#endif /* ACE_HAS_GETIFADDRS */

  return result;
}

bool
Net_WLAN_Tools::isInterface (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::isInterface"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);

  bool result = false;
  int socket_handle = -1;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  int result_2 = -1;

  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  socket_handle = ACE_OS::socket (AF_INET,
                                  SOCK_STREAM,
                                  0);
  if (unlikely (socket_handle == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
    return false;
  } // end IF
  // *TODO*: verify the presence of Wireless Extensions
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCGIWNAME,
                            &iwreq_s);
  if (!result_2)
    result = true;

  result_2 = ACE_OS::close (socket_handle);
  if (unlikely (result_2 == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));

  return result;
}

bool
Net_WLAN_Tools::associate (const std::string& interfaceIdentifier_in,
                           const struct ether_addr& APMACAddress_in,
                           const std::string& SSID_in,
                           ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associate"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);
  ACE_ASSERT (SSID_in.size () <= IW_ESSID_MAX_SIZE);
  ACE_ASSERT (!SSID_in.empty ());

  bool result = false;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  struct iw_mlme iw_mlme_s;
  ACE_OS::memset (&iw_mlme_s, 0, sizeof (struct iw_mlme));
  ACE_HANDLE socket_handle = handle_in;
  bool close_handle = false;
  int result_2 = -1;
  int error = 0;
  unsigned int retries_i = 0;

  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = ACE_OS::socket (AF_INET,
                                    SOCK_STREAM,
                                    0);
    if (unlikely (socket_handle == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
      return false;
    } // end IF
    close_handle = true;
  } // end IF

  // step0: set mode ?
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCGIWMODE,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    if (error != EALREADY) // 114
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCGIWMODE): \"%m\", aborting\n"),
                  socket_handle));
      goto clean;
    } // end IF
  } // end IF
  if (unlikely (iwreq_s.u.mode != IW_MODE_INFRA))
  {
    iwreq_s.u.mode = IW_MODE_INFRA;
    result_2 = ACE_OS::ioctl (socket_handle,
                              SIOCSIWMODE,
                              &iwreq_s);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EALREADY) // 114
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWMODE): \"%m\", aborting\n"),
                    socket_handle));
        goto clean;
      } // end IF
    } // end IF
  } // end IF

  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_ifrn.ifrn_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);

  // step1: set BSSID (AP MAC)
  iwreq_s.u.ap_addr.sa_family = ARPHRD_ETHER;
  ACE_OS::memcpy (iwreq_s.u.ap_addr.sa_data,
                  &APMACAddress_in.ether_addr_octet,
                  ETH_ALEN);
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCSIWAP,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    if (error != EALREADY) // 114
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWAP): \"%m\", aborting\n"),
                  socket_handle));
      goto clean;
    } // end IF
  } // end IF

  // step2: set ESSID
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_ifrn.ifrn_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  iwreq_s.u.essid.flags = 1;
  iwreq_s.u.essid.length = SSID_in.size ();
//  if (drv->we_version_compiled < 21) {
//		/* For historic reasons, set SSID length to include one extra
//		 * character, C string nul termination, even though SSID is
//		 * really an octet string that should not be presented as a C
//		 * string. Some Linux drivers decrement the length by one and
//		 * can thus end up missing the last octet of the SSID if the
//		 * length is not incremented here. WE-21 changes this to
//		 * explicitly require the length _not_ to include nul
//		 * termination. */
//		if (ssid_len)
//			ssid_len++;
//}
  iwreq_s.u.essid.pointer = const_cast<char*> (SSID_in.c_str ());
  retries_i = 0;
set_essid:
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCSIWESSID,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    if (error == EBUSY) // 16: driver is busy (scanning, (re-)setting bssid, ...)
    {
      if (retries_i < NET_WLAN_MONITOR_AP_ASSOCIATION_RETRIES)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWESSID): \"%m\", retrying...\n"),
                    socket_handle));
        ++retries_i;
        goto set_essid;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWESSID): \"%m\", giving up\n"),
                  socket_handle));
    } // end IF
    if (error != EALREADY) // 114
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWESSID): \"%m\", aborting\n"),
                  socket_handle));
      goto clean;
    } // end IF
  } // end IF

  // step3: MLME associate
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_ifrn.ifrn_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
//  ACE_OS::memset (&iw_mlme_s, 0, sizeof (struct iw_mlme));
  iw_mlme_s.addr.sa_family = ARPHRD_ETHER;
  ACE_OS::memcpy (iw_mlme_s.addr.sa_data,
                  &APMACAddress_in.ether_addr_octet,
                  ETH_ALEN);
  iw_mlme_s.cmd = IW_MLME_ASSOC;
//    iwreq_s.u.data.flags = 0;Stream_IStream_t
//  iw_mlme_s.reason_code = 0; // *TODO*: include <linux/ieee80211.h, see above>
  iwreq_s.u.data.length = sizeof (struct iw_mlme);
  iwreq_s.u.data.pointer = &iw_mlme_s;
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCSIWMLME,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    if ((error != ENOTSUP) && // 95 *TODO*
        (error != EALREADY))  // 114
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWMLME): \"%m\", aborting\n"),
                  socket_handle));
      goto clean;
    } // end IF
  } // end IF

  // step4: commit pending changes
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_ifrn.ifrn_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCSIWCOMMIT,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    if (error != ENOTSUP) // 95
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWCOMMIT): \"%m\", aborting\n"),
                  socket_handle));
      goto clean;
    } // end IF
  } // end IF

  result = true;

clean:
  if (unlikely (close_handle))
  {
    result_2 = ACE_OS::close (socket_handle);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
  } // end IF

  return result;
}

bool
Net_WLAN_Tools::disassociate (const std::string& interfaceIdentifier_in,
                              ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::disassociate"));

  bool result = false;

  // sanity check(s)
  bool release_handle = false;
  struct iwreq iwreq_s;
  std::string essid_string;
  struct iw_mlme iw_mlme_s;
  struct ether_addr interface_mac_address_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_HANDLE socket_handle = handle_in;
  int result_2 = -1;
  int error = 0;

  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = ACE_OS::socket (AF_INET,
                                    SOCK_STREAM,
                                    0);
    if (unlikely (socket_handle == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
      return false;
    } // end IF
    release_handle = true;
  } // end IF
  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (unlikely (interfaceIdentifier_in.empty ()))
  {
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces ();
    if (interface_identifiers_a.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument, aborting\n")));
      goto clean;
    } // end IF
  } // end IF
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);

  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    ACE_ASSERT ((*iterator).size () <= IFNAMSIZ);
    ACE_OS::strncpy (iwreq_s.ifr_ifrn.ifrn_name,
                     (*iterator).c_str (),
                     IFNAMSIZ);

    // step1: reset ESSID
//    iwreq_s.u.essid.flags = 0; // --> promiscuous
    iwreq_s.u.essid.pointer = const_cast<char*> (essid_string.c_str ());
    result_2 = ACE_OS::ioctl (socket_handle,
                              SIOCSIWESSID,
                              &iwreq_s);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EALREADY) // 114
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWESSID): \"%m\", aborting\n"),
                    socket_handle));
        goto clean;
      } // end IF
    } // end IF

    // step2: reset BSSID (AP MAC)
    ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
    ACE_OS::strncpy (iwreq_s.ifr_ifrn.ifrn_name,
                     (*iterator).c_str (),
                     IFNAMSIZ);
    iwreq_s.u.ap_addr.sa_family = ARPHRD_ETHER;
//    ACE_OS::memset (iwreq_s.u.ap_addr.sa_data, 0, ETH_ALEN);
    result_2 = ACE_OS::ioctl (socket_handle,
                              SIOCSIWAP,
                              &iwreq_s);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EALREADY) // 114
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWAP): \"%m\", aborting\n"),
                    socket_handle));
        goto clean;
      } // end IF
    } // end IF

    // step3: MLME deauthenticate
    ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
    ACE_OS::strncpy (iwreq_s.ifr_ifrn.ifrn_name,
                     (*iterator).c_str (),
                     IFNAMSIZ);
    ACE_OS::memset (&iw_mlme_s, 0, sizeof (struct iw_mlme));
    interface_mac_address_s =
        Net_Common_Tools::interfaceToLinkLayerAddress (*iterator);
    iw_mlme_s.addr.sa_family = ARPHRD_ETHER;
    ACE_OS::memcpy (iw_mlme_s.addr.sa_data,
                    &interface_mac_address_s.ether_addr_octet,
                    sizeof (struct ether_addr));
    iw_mlme_s.cmd = IW_MLME_DEAUTH;
//    iwreq_s.u.data.flags = 0;
    iw_mlme_s.reason_code = 3; // *TODO*: include <linux/ieee80211.h, see above>
    iwreq_s.u.data.length = sizeof (struct iw_mlme);
    iwreq_s.u.data.pointer = &iw_mlme_s;
    result_2 = ACE_OS::ioctl (socket_handle,
                              SIOCSIWMLME,
                              &iwreq_s);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EALREADY) // 114
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWMLME): \"%m\", aborting\n"),
                    socket_handle));
        goto clean;
      } // end IF
    } // end IF

    // step4: MLME disassociate
    ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
    ACE_OS::strncpy (iwreq_s.ifr_ifrn.ifrn_name,
                     (*iterator).c_str (),
                     IFNAMSIZ);
    ACE_OS::memset (&iw_mlme_s, 0, sizeof (struct iw_mlme));
//    interface_mac_address_s =
//        Net_Common_Tools::interfaceToLinkLayerAddress (*iterator);
    iw_mlme_s.addr.sa_family = ARPHRD_ETHER;
    ACE_OS::memcpy (iw_mlme_s.addr.sa_data,
                    &interface_mac_address_s.ether_addr_octet,
                    sizeof (struct ether_addr));
    iw_mlme_s.cmd = IW_MLME_DISASSOC;
//    iwreq_s.u.data.flags = 0;
    iw_mlme_s.reason_code = 3; // *TODO*: include <linux/ieee80211.h, see above>
    iwreq_s.u.data.length = sizeof (struct iw_mlme);
    iwreq_s.u.data.pointer = &iw_mlme_s;
    result_2 = ACE_OS::ioctl (socket_handle,
                              SIOCSIWMLME,
                              &iwreq_s);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EALREADY) // 114
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWMLME): \"%m\", aborting\n"),
                    socket_handle));
        goto clean;
      } // end IF
    } // end IF

    // step5: commit pending changes
    ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
    ACE_OS::strncpy (iwreq_s.ifr_ifrn.ifrn_name,
                     (*iterator).c_str (),
                     IFNAMSIZ);
    result_2 = ACE_OS::ioctl (socket_handle,
                              SIOCSIWCOMMIT,
                              &iwreq_s);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != ENOTSUP) // 95
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWCOMMIT): \"%m\", aborting\n"),
                    socket_handle));
        goto clean;
      } // end IF
    } // end IF
  } // end FOR

  result = true;

clean:
  if (unlikely (release_handle))
  {
    result_2 = ACE_OS::close (socket_handle);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
  } // end IF

  return result;
}

void
Net_WLAN_Tools::scan (const std::string& interfaceIdentifier_in,
                      const std::string& ESSID_in,
                      ACE_HANDLE handle_in,
                      bool wait_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::scan"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);
  ACE_ASSERT (ESSID_in.size () <= IW_ESSID_MAX_SIZE);

  ACE_HANDLE socket_handle = handle_in;
  bool close_handle = false;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  struct iw_scan_req iw_scan_req_s;
  ACE_OS::memset (&iw_scan_req_s, 0, sizeof (struct iw_scan_req));
  //  iwreq_s.u.data.flags = IW_SCAN_DEFAULT;
  iwreq_s.u.data.flags = (IW_SCAN_ALL_ESSID |
                          IW_SCAN_ALL_FREQ  |
                          IW_SCAN_ALL_MODE  |
                          IW_SCAN_ALL_RATE);
  int result = -1;
#if defined (ACE_LINUX)
  bool handle_capabilities = false;
#endif // ACE_LINUX

  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  if (likely (!ESSID_in.empty ()))
  { ACE_ASSERT (ESSID_in.size () <= IW_ESSID_MAX_SIZE);
    iw_scan_req_s.bssid.sa_family = ARPHRD_ETHER;
    ACE_OS::memset (iw_scan_req_s.bssid.sa_data, 0xff, ETH_ALEN);
    iw_scan_req_s.essid_len = ESSID_in.size ();
    ACE_OS::memcpy (iw_scan_req_s.essid,
                    ESSID_in.c_str (),
                    ESSID_in.size ());
    iwreq_s.u.data.flags = IW_SCAN_THIS_ESSID;
    iwreq_s.u.data.length = sizeof (struct iw_scan_req);
    iwreq_s.u.data.pointer = &iw_scan_req_s;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("scanning for ESSID %s...\n"),
                ACE_TEXT (ESSID_in.c_str ())));
  } // end IF
  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = ACE_OS::socket (AF_INET,
                                    SOCK_STREAM,
                                    0);
    if (unlikely (socket_handle == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", returning\n")));
      return;
    } // end IF
    close_handle = true;
  } // end IF

#if defined (ACE_LINUX)
  // (temporarily) elevate privileges to start scan
  if (!Common_Tools::hasCapability (CAP_NET_ADMIN,
                                    CAP_EFFECTIVE))
  {
    if (unlikely (!Common_Tools::setCapability (CAP_NET_ADMIN,
                                                CAP_EFFECTIVE)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::setCapability(%s): \"%m\", aborting\n"),
                  ACE_TEXT (Common_Tools::capabilityToString (CAP_NET_ADMIN).c_str ())));
      goto error;
    } // end IF
    handle_capabilities = true;
  } // end IF
#endif // ACE_LINUX

  result = ACE_OS::ioctl (socket_handle,
                          SIOCSIWSCAN,
                          &iwreq_s);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
//    if (error == EPERM) // 1
    if ((error != EAGAIN) && // 11: scan not complete
        (error != EBUSY))    // 16: another scan is in progress
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWSCAN): \"%m\", returning\n"),
                  socket_handle));
      goto error;
    } // end IF
  } // end IF

  if (unlikely (wait_in))
  {
    // *IMPORTANT NOTE*: the kernel apparently does not signal the ioctl socket
    //                   file descriptor on the arrival of result data
    //                   --> poll
    // *TODO*: why ?
    ACE_ASSERT (false);
    ACE_NOTSUP;

    ACE_NOTREACHED (return;)
//    ACE_Handle_Set handle_set;
//    handle_set.set_bit (socket_handle);
//    // wait (maximum) 250ms between set and (first) result data
//    ACE_Time_Value timeout (0, 250000);
//    result = ACE_OS::select (static_cast<int> (socket_handle) + 1,
//                             static_cast<fd_set*> (handle_set),    // read
//                             NULL,                                 // write
//                             NULL,                                 // exception
////                             &timeout);
//                             NULL);
//    switch (result)
//    {
//      case 0:
//        ACE_OS::last_error (ETIME);
//        // *WARNING*: control falls through here
//      case -1:
//      {
//        if (unlikely (result == -1))
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_OS::select(%d): \"%m\", returning\n"),
//                      socket_handle));
//          goto error;
//        } // end IF
//        break;
//      }
//      default:
//        break;
//    } // end SWITCH
  } // end IF

error:
  if (unlikely (close_handle))
  {
    result = ACE_OS::close (socket_handle);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
  } // end IF

#if defined (ACE_LINUX)
  if (handle_capabilities)
    if (!Common_Tools::dropCapability (CAP_NET_ADMIN))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::dropCapability(CAP_NET_ADMIN): \"%m\", continuing\n")));
#endif // ACE_LINUX
}

struct ether_addr
Net_WLAN_Tools::associatedBSSID (const std::string& interfaceIdentifier_in,
                                 ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedBSSID"));

  // initialize return value(s)
  struct ether_addr return_value;
  ACE_OS::memset (&return_value, 0, sizeof (struct ether_addr));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  ACE_ASSERT (interfaceIdentifier_in.size () <= IFNAMSIZ);

  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_HANDLE socket_handle = handle_in;
  bool close_handle = false;
  int result_2 = -1;
  int error = 0;

  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = ACE_OS::socket (AF_INET,
                                    SOCK_STREAM,
                                    0);
    if (unlikely (socket_handle == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
      return return_value;
    } // end IF
    close_handle = true;
  } // end IF

  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCGIWAP,
                            &iwreq_s);
  if (unlikely (result_2 == -1))
  {
    error = ACE_OS::last_error ();
    ACE_UNUSED_ARG (error);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCSIWAP): \"%m\", aborting\n"),
                socket_handle));
    goto clean;
  } // end IF
  ACE_OS::memcpy (&return_value,
                  iwreq_s.u.ap_addr.sa_data,
                  sizeof (struct ether_addr));

clean:
  if (unlikely (close_handle))
  {
    result_2 = ACE_OS::close (socket_handle);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
  } // end IF

  return return_value;
}

std::string
Net_WLAN_Tools::associatedSSID (const std::string& interfaceIdentifier_in,
                                ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedSSID"));

  // initialize return value(s)
  std::string result;

  // sanity check(s)
  std::string interface_identifier = interfaceIdentifier_in;
  if (unlikely (interfaceIdentifier_in.empty ()))
  {
    Net_InterfaceIdentifiers_t interface_identifiers_a =
      Net_WLAN_Tools::getInterfaces ();
    if (interface_identifiers_a.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument, aborting\n")));
      return result;
    } // end IF
    interface_identifier = interface_identifiers_a.front ();
  } // end IF

  ACE_HANDLE socket_handle = handle_in;
  int result_2 = -1;
  struct iwreq iwreq_s;
  char essid[IW_ESSID_MAX_SIZE + 1];
  bool close_socket = false;

  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = iw_sockets_open ();
    if (socket_handle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to iw_sockets_open(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    close_socket = true;
  } // end IF

  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::memset (essid, 0, sizeof (IW_ESSID_MAX_SIZE + 1));
  iwreq_s.u.essid.pointer = essid;
  iwreq_s.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  result_2 = iw_get_ext (socket_handle,
                         interface_identifier.c_str (),
                         SIOCGIWESSID,
                         &iwreq_s);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_get_ext(\"%s\",SIOCGIWESSID): \"%m\", aborting\n"),
                ACE_TEXT (interface_identifier.c_str ())));
    goto error;
  } // end IF
  // *NOTE*: the length iwreq_s.u.essid.length is wrong
//  ACE_ASSERT (iwreq_s.u.essid.length && (iwreq_s.u.essid.length <= IW_ESSID_MAX_SIZE));
//  result.assign (essid, iwreq_s.u.essid.length);
  result = essid;

error:
  if (unlikely (close_socket))
    iw_sockets_close (socket_handle);

//continue_:
  return result;
}

Net_WLAN_SSIDs_t
Net_WLAN_Tools::getSSIDs (const std::string& interfaceIdentifier_in,
                          ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::getSSIDs"));

  Net_WLAN_SSIDs_t result;

  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (unlikely (interfaceIdentifier_in.empty ()))
    interface_identifiers_a = Net_WLAN_Tools::getInterfaces ();
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
//  ACE_ASSERT (handle_in != ACE_INVALID_HANDLE);

  int result_2 = -1;
  ACE_HANDLE socket_handle = handle_in;
  bool close_handle = false;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   interfaceIdentifier_in.c_str (),
                   IFNAMSIZ);
  struct iw_range iw_range_s;
  ACE_OS::memset (&iw_range_s, 0, sizeof (struct iw_range));
  struct stream_descr stream_descr_s;
  ACE_OS::memset (&stream_descr_s, 0, sizeof (struct stream_descr));
  struct iw_event iw_event_s;
  ACE_OS::memset (&iw_event_s, 0, sizeof (struct iw_event));
  int error = 0;
  size_t buffer_size = IW_SCAN_MAX_DATA;
  struct ether_addr ap_mac_address;
  ACE_OS::memset (&ap_mac_address, 0, sizeof (struct ether_addr));
  std::string essid_string;

  if (unlikely (socket_handle == ACE_INVALID_HANDLE))
  {
    socket_handle = ACE_OS::socket (AF_INET,
                                    SOCK_STREAM,
                                    0);
    if (unlikely (socket_handle == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::socket(AF_INET): \"%m\", aborting\n")));
      return result;
    } // end IF
    close_handle = true;
  } // end IF

  result_2 = iw_get_range_info (socket_handle,
                                interfaceIdentifier_in.c_str (),
                                &iw_range_s);
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_get_range_info(%d,\"%s\"): \"%m\", aborting\n"),
                socket_handle,
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto clean;
  } // end IF

  iwreq_s.u.data.pointer = ACE_OS::malloc (buffer_size);
  if (unlikely (!iwreq_s.u.data.pointer))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (%u byte(s)): \"%m\", aborting\n"),
                buffer_size));
    goto clean;
  } // end IF
  iwreq_s.u.data.length = buffer_size;

fetch_scan_result_data:
  result_2 = ACE_OS::ioctl (socket_handle,
                            SIOCGIWSCAN,
                            &iwreq_s);
  if (unlikely (result_2 < 0))
  {
    error = ACE_OS::last_error ();
    if ((error == E2BIG) && // 7
        (iw_range_s.we_version_compiled > 16))
    { // buffer too small --> retry
      if (iwreq_s.u.data.length > buffer_size)
        buffer_size = iwreq_s.u.data.length;
      else
        buffer_size *= 2; // grow dynamically
      goto retry;
    } // end IF

    if (unlikely (error == EAGAIN)) // 11
    { // result(s) not available yet
      goto clean;
    } // end IF
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCGIWSCAN): \"%m\", aborting\n"),
                handle_in));
    result.clear ();
    goto clean;
  } // end IF

  // the driver may have reported the required buffer size
  if (unlikely (iwreq_s.u.data.length > buffer_size))
  { // --> grow the buffer and retrys
    buffer_size = iwreq_s.u.data.length;
retry:
    iwreq_s.u.data.pointer = ACE_OS::realloc (iwreq_s.u.data.pointer,
                                              buffer_size);
    if (unlikely (!iwreq_s.u.data.pointer))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to reallocate memory (%u byte(s)): \"%m\", aborting\n"),
                  buffer_size));
      result.clear ();
      goto clean;
    } // end IF
    iwreq_s.u.data.length = buffer_size;
    goto fetch_scan_result_data;
  } // end IF
  ACE_ASSERT (iwreq_s.u.data.length && (iwreq_s.u.data.length <= buffer_size));

  // process the result data
  iw_init_event_stream (&stream_descr_s,
                        static_cast<char*> (iwreq_s.u.data.pointer),
                        static_cast<int> (iwreq_s.u.data.length));
  do
  {
    result_2 = iw_extract_event_stream (&stream_descr_s,
                                        &iw_event_s,
                                        iw_range_s.we_version_compiled);
    if (result_2 <= 0)
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
      case SIOCGIWFREQ:
      case SIOCGIWMODE:
      case SIOCGIWNAME:
      case SIOCGIWENCODE:
      case SIOCGIWRATE:
      case SIOCGIWMODUL:
      case IWEVQUAL:
#ifndef WE_ESSENTIAL
      case IWEVGENIE:
#endif /* WE_ESSENTIAL */
      case IWEVCUSTOM:
        break;
      case SIOCGIWESSID:
      {
        ACE_ASSERT (iw_event_s.u.essid.pointer);
        ACE_ASSERT (iw_event_s.u.essid.length && (iw_event_s.u.essid.length <= IW_ESSID_MAX_SIZE));
        essid_string.assign (reinterpret_cast<char*> (iw_event_s.u.essid.pointer),
                             iw_event_s.u.essid.length);
        //        if (iw_event_s.u.essid.flags)
        //        {
        //          /* Does it have an ESSID index ? */
        //          if((iw_event_s.u.essid.flags & IW_ENCODE_INDEX) > 1)
        //            printf("                    ESSID:\"%s\" [%d]\n", essid,
        //                   (iw_event_s.u.essid.flags & IW_ENCODE_INDEX));
        //          else
        //            printf("                    ESSID:\"%s\"\n", essid);
        //        } // end IF
        //        else
        //          printf("                    ESSID:off/any/hidden\n");
        result.push_back (essid_string);
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("\"%s\": found wireless access point (MAC address: %s, ESSID: %s)...\n"),
                    ACE_TEXT (interfaceIdentifier_in.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address),
                                                                          NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT (essid_string.c_str ())));
#endif
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("invalid/unknown WE event type (was: %d), continuing\n"),
                    iw_event_s.cmd));
        break;
      }
    } // end SWITCH
  } while (true);

clean:
  if (iwreq_s.u.data.pointer)
    ACE_OS::free (iwreq_s.u.data.pointer);
  if (unlikely (close_handle))
  {
    result_2 = ACE_OS::close (socket_handle);
    if (unlikely (result_2 == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT (interfaceIdentifier_in.c_str ())));
  } // end IF

  return result;
}

bool
Net_WLAN_Tools::hasSSID (const std::string& interfaceIdentifier_in,
                         const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Tools::associatedSSID"));

  // initialize return value(s)
  bool result = false;

  // sanity check(s)
  if (unlikely (interfaceIdentifier_in.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));
    return result;
  } // end IF

  int result_2 = -1;
  Net_InterfaceIdentifiers_t interface_identifiers_a;
  int socket_handle = -1;
  struct iw_range iw_range_s;
  struct wireless_scan_head wireless_scan_head_s;
  struct wireless_scan* wireless_scan_p = NULL;
  bool done = false;

  // step1: select interface(s)
  if (likely (!interfaceIdentifier_in.empty ()))
  {
    interface_identifiers_a.push_back (interfaceIdentifier_in);
    goto continue_;
  } // end IF
  interface_identifiers_a = Net_WLAN_Tools::getInterfaces ();

continue_:
  socket_handle = iw_sockets_open ();
  if (unlikely (socket_handle == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_sockets_open(): \"%m\", aborting\n")));
    return false;
  } // end IF

  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       (iterator != interface_identifiers_a.end ()) && !done;
       ++iterator)
  {
    ACE_OS::memset (&iw_range_s, 0, sizeof (struct iw_range));
    result_2 = iw_get_range_info (socket_handle,
                                  (*iterator).c_str (),
                                  &iw_range_s);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to iw_get_range_info(): \"%m\", aborting\n")));
      goto error;
    } // end IF

    ACE_OS::memset (&wireless_scan_head_s,
                    0,
                    sizeof (struct wireless_scan_head));
    // *TODO*: do not use iwlib; talk to the driver directly
    result_2 = iw_scan (socket_handle,
                        const_cast<char*> ((*iterator).c_str ()),
                        iw_range_s.we_version_compiled,
                        &wireless_scan_head_s);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to iw_scan(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    for (wireless_scan_p = wireless_scan_head_s.result;
         wireless_scan_p;
         wireless_scan_p = wireless_scan_p->next)
    {
      if (!wireless_scan_p->b.essid_on)
        continue;
      if (!ACE_OS::strncmp (SSID_in.c_str (),
                            wireless_scan_p->b.essid,
                            wireless_scan_p->b.essid_len))
      { // found SSID on current interface
        result = true;
        done = true;
        break;
      } // end IF
    } // end FOR
  } // end FOR

error:
  // clean up
  iw_sockets_close (socket_handle);

  return result;
}
