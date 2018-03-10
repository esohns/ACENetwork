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

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <net/ethernet.h>

#if defined (NL80211_SUPPORT)
#include <linux/nl80211.h>
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include <set>
#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Synch_Traits.h"

#include "common_xml_common.h"
#include "common_xml_parser.h"

#include "net_wlan_profile_xml_handler.h"
#endif // ACE_WIN32 || ACE_WIN64

enum Net_WLAN_MonitorAPIType : int
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *TODO*: available since WinXP
  NET_WLAN_MONITOR_API_WLANAPI = 0,
#else
  NET_WLAN_MONITOR_API_IOCTL = 0, // aka 'wext' (mostly deprecated)
  NET_WLAN_MONITOR_API_NL80211,
  NET_WLAN_MONITOR_API_DBUS, // talk to the 'NetworkManager' service
#endif // ACE_WIN32 || ACE_WIN64
  ////////////////////////////////////////
  NET_WLAN_MONITOR_API_MAX,
  NET_WLAN_MONITOR_API_INVALID
};

// *NOTE*: see: 802.11 ieee standard reference specification
#if defined (_MSC_VER)
#pragma pack (push, 1)
#endif // _MSC_VER
struct Net_WLAN_IEEE802_11_InformationElement
{
  uint8_t  id;
  uint8_t  length;
  uint8_t* data;
#if defined (__GNUC__)
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
#if defined (_MSC_VER)
#pragma pack (pop)
#endif // _MSC_VER

struct Net_WLAN_AssociationConfiguration
{
  Net_WLAN_AssociationConfiguration ()
   : accessPointLinkLayerAddress ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , authenticationType (0)
   , frequency (0)
#endif // ACE_WIN32 || ACE_WIN64
   , signalQuality (0)
  {
    ACE_OS::memset (&accessPointLinkLayerAddress,
                    0,
                    sizeof (struct ether_addr));
  };

  struct ether_addr   accessPointLinkLayerAddress;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  WLAN_SIGNAL_QUALITY signalQuality;
#else
  unsigned int        authenticationType;
  unsigned int        frequency;
  unsigned int        signalQuality;
#endif // ACE_WIN32 || ACE_WIN64
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef std::multimap<std::string,
                      std::pair<struct _GUID, struct Net_WLAN_AssociationConfiguration> > Net_WLAN_SSIDToInterfaceIdentifier_t;
#else
typedef std::multimap<std::string,
                      std::pair<std::string, struct Net_WLAN_AssociationConfiguration> > Net_WLAN_SSIDToInterfaceIdentifier_t;
#endif // ACE_WIN32 || ACE_WIN64
typedef Net_WLAN_SSIDToInterfaceIdentifier_t::const_iterator Net_WLAN_SSIDToInterfaceIdentifierConstIterator_t;
typedef Net_WLAN_SSIDToInterfaceIdentifier_t::iterator Net_WLAN_SSIDToInterfaceIdentifierIterator_t;
typedef std::pair<Net_WLAN_SSIDToInterfaceIdentifier_t::iterator, bool> Net_WLAN_SSIDToInterfaceIdentifierResult_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef std::pair<std::network_wlan_nl80211_interface_cbstring,
                  std::pair <struct _GUID, struct Net_WLAN_AssociationConfiguration> > Net_WLAN_SSIDToInterfaceIdentifierEntry_t;
struct Net_WLAN_SSIDToInterfaceIdentifierFindPredicate
 : public std::binary_function<Net_WLAN_SSIDToInterfaceIdentifierEntry_t,
                               struct _GUID,
                               bool>
{
  inline bool operator() (const Net_WLAN_SSIDToInterfaceIdentifierEntry_t& entry_in, struct _GUID value_in) const { return InlineIsEqualGUID (entry_in.second.first, value_in); }
};
#else
typedef std::pair<std::string,
                  std::pair <std::string, struct Net_WLAN_AssociationConfiguration> > Net_WLAN_SSIDToInterfaceIdentifierEntry_t;
struct Net_WLAN_SSIDToInterfaceIdentifierFindPredicate
 : public std::binary_function<Net_WLAN_SSIDToInterfaceIdentifierEntry_t,
                               std::string,
                               bool>
{
  inline bool operator() (const Net_WLAN_SSIDToInterfaceIdentifierEntry_t& entry_in, std::string value_in) const { return entry_in.second.first == value_in; }
};
#endif // ACE_WIN32 || ACE_WIN64

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
#else
#if defined (NL80211_SUPPORT)
typedef std::map<std::string, int> Net_WLAN_nl80211_MulticastGroupIds_t;
typedef Net_WLAN_nl80211_MulticastGroupIds_t::iterator Net_WLAN_nl80211_MulticastGroupIdsIterator_t;
struct Net_WLAN_nl80211_MulticastGroupIdQueryCBData
{
  Net_WLAN_nl80211_MulticastGroupIdQueryCBData ()
   : map (NULL)
  {}

  Net_WLAN_nl80211_MulticastGroupIds_t* map;
};

typedef std::set<enum nl80211_ext_feature_index> Net_WLAN_nl80211_ExtendedFeatures_t;
typedef Net_WLAN_nl80211_ExtendedFeatures_t::iterator Net_WLAN_nl80211_ExtendedFeaturesIterator_t;
struct Net_WLAN_nl80211_FeatureSetCBData
{
  Net_WLAN_nl80211_FeatureSetCBData ()
   : features (0)
   , extendedFeatures ()
  {}

  ACE_UINT32                          features;
  Net_WLAN_nl80211_ExtendedFeatures_t extendedFeatures;
};

struct Net_WLAN_nl80211_InterfaceConfigurationCBData
{
  Net_WLAN_nl80211_InterfaceConfigurationCBData ()
   : index (0)
   , type (NL80211_IFTYPE_UNSPECIFIED)
  {}

  unsigned int        index;
  enum nl80211_iftype type;
};

struct Net_WLAN_nl80211_ScanResult
{
  Net_WLAN_nl80211_ScanResult ()
   : aborted (false)
   , done (false)
  {}

  bool aborted;
  bool done;
};
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#endif
