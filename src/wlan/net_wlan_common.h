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

#ifndef NET_WLAN_COMMON_H
#define NET_WLAN_COMMON_H

#include <string>
#include <vector>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Synch_Traits.h"

#include "common_xml_common.h"
#include "common_xml_parser.h"

#include "net_wlan_profile_xml_handler.h"
#endif

enum Net_WLAN_MonitorAPIType : int
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *TODO*: available since WinXP
  NET_WLAN_MONITOR_API_WLANAPI = 0,
#else
  NET_WLAN_MONITOR_API_IOCTL = 0,
  NET_WLAN_MONITOR_API_DBUS,
#endif
  ////////////////////////////////////////
  NET_WLAN_MONITOR_API_MAX,
  NET_WLAN_MONITOR_API_INVALID
};

// *NOTE*: see: 802.11 RFC
#if defined (_MSC_VER)
#pragma pack (push, 1)
#endif // _MSC_VER
struct Net_WLAN_IEEE802_11_InformationElement
{
  uint8_t  id;
  uint8_t  length;
  uint8_t* data;
#ifdef __GNUC__
} __attribute__ ((__packed__));
#else
};
#endif
#if defined (_MSC_VER)
#pragma pack (pop)
#endif

typedef std::vector<std::string> Net_WLAN_SSIDs_t;
typedef Net_WLAN_SSIDs_t::iterator Net_WLAN_SSIDsIterator_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef std::vector<std::string> Net_WLAN_Profiles_t;
typedef Net_WLAN_Profiles_t::iterator Net_WLAN_ProfilesIterator_t;

struct Net_WLAN_Profile_ParserContext
{
  Net_WLAN_SSIDs_t SSIDs;
};
typedef Common_XML_Parser_T<ACE_MT_SYNCH,
                            struct Common_XML_ParserConfiguration,
                            struct Net_WLAN_Profile_ParserContext,
                            Net_WLAN_Profile_XML_Handler> Net_WLAN_Profile_Parser_t;
#endif

#endif
