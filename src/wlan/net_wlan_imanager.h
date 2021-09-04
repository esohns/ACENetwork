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

#ifndef NET_WLAN_IMANAGER_H
#define NET_WLAN_IMANAGER_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#else
#include "net/ethernet.h"
#endif // ACE_WIN32 || ACE_WIN64

class Net_WLAN_IManager
{
 public:
  // *IMPORTANT NOTE*: these must (!) never block

  // *TODO*: move all of MLME here eventually

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool do_associate (REFGUID,                  // interface identifier {GUID_NULL: any}
#elif defined (ACE_LINUX)
  virtual bool do_associate (const std::string&,       // interface identifier {"": any}
                             const struct ether_addr&, // AP BSSID (i.e. access point MAC address)
#endif // ACE_WIN32 || ACE_WIN64
                             const std::string&) = 0;  // (E)SSID {"": disassociate}

  // *TODO*: conceive of an ADT for authentication credentials
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  virtual bool do_authenticate (const std::string&,       // interface identifier {"": any}
                                const struct ether_addr&, // access point BSSID (i.e. access point MAC address)
                                const std::string&) = 0;  // (E)SSID {"": deauthenticate}
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void do_scan (REFGUID,                  // interface identifier {GUID_NULL: all}
#elif defined (ACE_LINUX)
  virtual void do_scan (const std::string&,       // interface identifier {"": all}
#endif // ACE_WIN32 || ACE_WIN64
                        const struct ether_addr&, // access point BSSID (i.e. access point MAC address) {0: all}
                        const std::string&) = 0;  // (E)SSID {"": all}
};

#endif
