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

#include <map>
#include <string>
#include <vector>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <cstdint>
#else
#if defined (NL80211_SUPPORT)
#include <set>
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
#include <wlanapi.h>
#endif // WLANAPI_SUPPORT
#else
#include "net/ethernet.h"

#if defined (NL80211_SUPPORT)
#include "linux/nl80211.h"
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Containers_T.h"
#include "ace/INET_Addr.h"
#include "ace/OS.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#endif // GUI_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_xml_common.h"
#include "common_xml_parser.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "net_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "net_packet_headers.h"

#include "net_wlan_profile_xml_handler.h"
#endif // ACE_WIN32 || ACE_WIN64

enum Net_WLAN_MonitorState
{
  NET_WLAN_MONITOR_STATE_INVALID = -1,
  // -------------------------------------
  // 'transitional' states (task-/timeout-oriented, i.e. may return to previous
  // state upon completion)
  NET_WLAN_MONITOR_STATE_IDLE = 0,       // idle (i.e. waiting between scans/connection attempts/...)
  NET_WLAN_MONITOR_STATE_SCAN,           // scanning
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  NET_WLAN_MONITOR_STATE_AUTHENTICATE,   // authenticating to access point
  NET_WLAN_MONITOR_STATE_ASSOCIATE,      // associating to access point
#endif // ACE_WIN32 || ACE_WIN64
  NET_WLAN_MONITOR_STATE_CONNECT,        // requesting IP address (e.g. DHCP)
  NET_WLAN_MONITOR_STATE_DISCONNECT,     // releasing IP address (e.g. DHCP)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  NET_WLAN_MONITOR_STATE_DISASSOCIATE,   // disassociating from access point
  NET_WLAN_MONITOR_STATE_DEAUTHENTICATE, // deauthenticating from access point
#endif // ACE_WIN32 || ACE_WIN64
  ////////////////////////////////////////
  // 'static' states (discrete/persisting, i.e. long(er)-term)
  NET_WLAN_MONITOR_STATE_INITIALIZED,    // initialized
  NET_WLAN_MONITOR_STATE_SCANNED,        // scanning complete
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  NET_WLAN_MONITOR_STATE_AUTHENTICATED,  // authenticated to access point
  NET_WLAN_MONITOR_STATE_ASSOCIATED,     // associated to access point
#endif // ACE_WIN32 || ACE_WIN64
  NET_WLAN_MONITOR_STATE_CONNECTED,      // has a valid IP address (i.e. DHCP handshake completed, lease obtained)
  ////////////////////////////////////////
  NET_WLAN_MONITOR_STATE_MAX
};

enum Net_WLAN_MonitorAPIType
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *TODO*: available since WinXP
  NET_WLAN_MONITOR_API_WLANAPI = 0, // --> default
#else
  NET_WLAN_MONITOR_API_WEXT = 0, // ioctl only (mostly deprecated)
  NET_WLAN_MONITOR_API_NL80211,  // --> default
  NET_WLAN_MONITOR_API_DBUS,     // listens/talks to the (systemd-) 'NetworkManager' service
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
  uint8_t  data[];
#if defined (__GNUC__)
} __attribute__ ((__packed__));
#else
};
#endif // __GNUC__
#if defined (_MSC_VER)
#pragma pack (pop)
#endif // _MSC_VER

struct Net_WLAN_AccessPointState
{
  Net_WLAN_AccessPointState ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   : frequency (0)
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
   : authenticationType (NL80211_AUTHTYPE_AUTOMATIC)
   , frequency (0)
#else
   : frequency (0)
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
   , IPAddress ()
   , lastSeen (0)
   , linkLayerAddress ()
   , signalQuality (0)
  {
    ACE_OS::memset (&linkLayerAddress, 0, sizeof (struct ether_addr));
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
  enum nl80211_auth_type authenticationType;
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  ACE_UINT32             frequency;
  ACE_INET_Addr          IPAddress;
  unsigned int           lastSeen; // ms
  struct ether_addr      linkLayerAddress;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
  WLAN_SIGNAL_QUALITY    signalQuality;
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
  unsigned int           signalQuality; // mBm
#endif // ACE_WIN32 || ACE_WIN64
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef std::pair <struct _GUID,
#else
typedef std::pair <std::string,
#endif // ACE_WIN32 || ACE_WIN64
                   struct Net_WLAN_AccessPointState> Net_WLAN_AccessPointCacheValue_t;
typedef std::multimap<std::string,
                      Net_WLAN_AccessPointCacheValue_t> Net_WLAN_AccessPointCache_t;
typedef Net_WLAN_AccessPointCache_t::const_iterator Net_WLAN_AccessPointCacheConstIterator_t;
typedef Net_WLAN_AccessPointCache_t::iterator Net_WLAN_AccessPointCacheIterator_t;
typedef std::pair<Net_WLAN_AccessPointCacheIterator_t, bool> Net_WLAN_AccessPointCacheFindResult_t;
typedef std::pair<std::string,
                  Net_WLAN_AccessPointCacheValue_t> Net_WLAN_AccessPointCacheEntry_t;
struct Net_WLAN_AccessPointCacheFindPredicate
 : public std::binary_function<Net_WLAN_AccessPointCacheEntry_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               struct _GUID,
#else
                               std::string,
#endif // ACE_WIN32 || ACE_WIN64
                               bool>
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline bool operator() (const Net_WLAN_AccessPointCacheEntry_t& entry_in, struct _GUID value_in) const { return InlineIsEqualGUID (entry_in.second.first, value_in); }
#else
  inline bool operator() (const Net_WLAN_AccessPointCacheEntry_t& entry_in, std::string value_in) const { return entry_in.second.first == value_in; }
#endif // ACE_WIN32 || ACE_WIN64
};

typedef std::vector<std::string> Net_WLAN_SSIDs_t;
typedef Net_WLAN_SSIDs_t::iterator Net_WLAN_SSIDsIterator_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef std::vector<std::string> Net_WLAN_Profiles_t;
typedef Net_WLAN_Profiles_t::const_iterator Net_WLAN_ProfilesIterator_t;

struct Net_WLAN_Profile_ParserContext
{
  Net_WLAN_SSIDs_t SSIDs;
};
//typedef Common_XML_Parser_T<ACE_MT_SYNCH,
//                            struct Common_XML_ParserConfiguration,
//                            struct Net_WLAN_Profile_ParserContext,
//                            Net_WLAN_Profile_XML_Handler> Net_WLAN_Profile_Parser_t;
typedef Common_XML_Parser_T<ACE_MT_SYNCH,
                            struct Common_XML_ParserConfiguration,
                            struct Net_WLAN_Profile_ParserContext,
                            Net_WLAN_Profile_XML_ListHandler> Net_WLAN_Profile_ListParser_t;
#elif defined (ACE_LINUX)
typedef std::map<std::string, unsigned int> Net_WLAN_WiPhyIdentifiers_t;
typedef Net_WLAN_WiPhyIdentifiers_t::iterator Net_WLAN_WiPhyIdentifiersIterator_t;
typedef std::pair<std::string, unsigned int> Net_WLAN_WiPhyIdentifiersEntry_t;
struct Net_WLAN_WiPhyIdentifiersFindPredicate
 : public std::binary_function<Net_WLAN_WiPhyIdentifiersEntry_t,
                               unsigned int,
                               bool>
{
  inline bool operator() (const Net_WLAN_WiPhyIdentifiersEntry_t& entry_in, unsigned int value_in) const { return entry_in.second == value_in; }
};

#if defined (NL80211_SUPPORT)
typedef std::map<std::string, int> Net_WLAN_nl80211_MulticastGroupIds_t;
typedef Net_WLAN_nl80211_MulticastGroupIds_t::iterator Net_WLAN_nl80211_MulticastGroupIdsIterator_t;

typedef std::set<enum nl80211_ext_feature_index> Net_WLAN_nl80211_ExtendedFeatures_t;
typedef Net_WLAN_nl80211_ExtendedFeatures_t::iterator Net_WLAN_nl80211_ExtendedFeaturesIterator_t;
struct Net_WLAN_nl80211_Features
{
  Net_WLAN_nl80211_Features ()
   : extendedFeatures ()
   , features (0)
  {}

  Net_WLAN_nl80211_ExtendedFeatures_t extendedFeatures;
  ACE_UINT32                          features;
};

class Net_WLAN_IMonitorBase;
struct Net_WLAN_nl80211_CBData
{
  Net_WLAN_nl80211_CBData ()
   : address ()
   , done (NULL)
   , dumping (false)
   , error (NULL)
   , features (NULL)
   , index (0)
   , interfaces (NULL)
   , name ()
   , protocolFeatures (NULL)
   , scanning (false)
   , SSIDs ()
#if defined (_DEBUG)
   , timestamp (ACE_Time_Value::zero)
#endif // _DEBUG
   , type (NL80211_IFTYPE_UNSPECIFIED)
   , map (NULL)
   , monitor (NULL)
   , wiPhys (NULL)
  {
    ACE_OS::memset (&address, 0, sizeof (struct ether_addr));
  }

  struct ether_addr                     address;
  int*                                  done;
  bool                                  dumping;
  int*                                  error;
  struct Net_WLAN_nl80211_Features*     features;
  unsigned int                          index; // interface-/wiphy-
  Net_InterfaceIdentifiers_t*           interfaces;
  std::string                           name; // wiphy-
  ACE_UINT32*                           protocolFeatures;
  bool                                  scanning;
  Net_WLAN_SSIDs_t*                     SSIDs;
#if defined (_DEBUG)
  ACE_Time_Value                        timestamp;
#endif // _DEBUG
  enum nl80211_iftype                   type;
  Net_WLAN_nl80211_MulticastGroupIds_t* map;
  Net_WLAN_IMonitorBase*                monitor;
  Net_WLAN_WiPhyIdentifiers_t*          wiPhys;
};

// error handling
int
network_wlan_nl80211_error_cb (struct sockaddr_nl*,
                               struct nlmsgerr*,
                               void*);

// protocol handling
int
network_wlan_nl80211_no_seq_check_cb (struct nl_msg*,
                                      void*);
int
network_wlan_nl80211_ack_cb (struct nl_msg*,
                             void*);
int
network_wlan_nl80211_finish_cb (struct nl_msg*,
                                void*);
int
network_wlan_nl80211_multicast_groups_cb (struct nl_msg*,
                                          void*);

// data handling
int
network_wlan_nl80211_default_handler_cb (struct nl_msg*,
                                         void*);
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

struct Net_WLAN_AssociationConfiguration
{
  Net_WLAN_AssociationConfiguration ()
   : accessPointLinkLayerAddress ()
   , authenticationType (0)
  {
    ACE_OS::memset (&accessPointLinkLayerAddress,
                    0,
                    sizeof (struct ether_addr));
  }

  struct ether_addr accessPointLinkLayerAddress;
  unsigned int      authenticationType;
};

//////////////////////////////////////////

enum Net_WLAN_EventType
{
#if defined (GUI_SUPPORT)
  NET_WLAN_EVENT_INTERFACE_HOTPLUG = COMMON_UI_EVENT_OTHER_USER_BASE,
#else
  NET_WLAN_EVENT_INTERFACE_HOTPLUG = 0,
#endif // GUI_SUPPORT
  NET_WLAN_EVENT_INTERFACE_REMOVE,
  NET_WLAN_EVENT_SCAN_COMPLETE,
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
  NET_WLAN_EVENT_ASSOCIATE,
  NET_WLAN_EVENT_DISASSOCIATE,
//#endif // ACE_WIN32 || ACE_WIN64
  NET_WLAN_EVENT_CONNECT,
  NET_WLAN_EVENT_DISCONNECT,
  NET_WLAN_EVENT_SIGNAL_QUALITY_CHANGED,
  ////////////////////////////////////////
  NET_WLAN_EVENT_MAX,
  NET_WLAN_EVENT_INVALID
};
typedef ACE_Unbounded_Stack<enum Net_WLAN_EventType> Net_WLAN_Events_t;
typedef ACE_Unbounded_Stack<enum Net_WLAN_EventType>::ITERATOR Net_WLAN_EventsIterator_t;

#endif
