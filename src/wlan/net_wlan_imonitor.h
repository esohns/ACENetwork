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

#ifndef NET_WLAN_IMONITOR_H
#define NET_WLAN_IMONITOR_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#include <wlanapi.h>
#else
#include <net/ethernet.h>

#if defined (DBUS_SUPPORT)
#include "dbus/dbus.h"
#endif
#endif

//#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"
#include "common_iinitialize.h"
#include "common_ilock.h"
#include "common_isubscribe.h"
#include "common_itaskcontrol.h"

#include "net_wlan_common.h"

class Net_WLAN_IMonitorCB
{
 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onSignalQualityChange (REFGUID,                  // interface identifier
                                      WLAN_SIGNAL_QUALITY) = 0; // signal quality (of current association)
#else
  virtual void onSignalQualityChange (const std::string&, // interface identifier
                                      unsigned int) = 0;  // signal quality (of current association)
#endif // ACE_WIN32 || ACE_WIN64
  // *IMPORTANT NOTE*: Net_IWLANMonitor_T::addresses() may not return
  //                   significant data before this, as the link layer
  //                   configuration (e.g. DHCP handshake, ...) most likely has
  //                   not been established
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onConnect (REFGUID,            // interface identifier
#else
  virtual void onConnect (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                          const std::string&, // SSID
                          bool) = 0;          // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onDisconnect (REFGUID,            // interface identifier
#else
  virtual void onDisconnect (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                             const std::string&, // SSID
                             bool) = 0;          // success ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onAssociate (REFGUID,            // interface identifier
#else
  virtual void onAssociate (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                            const std::string&, // SSID
                            bool) = 0;          // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onDisassociate (REFGUID,            // interface identifier
#else
  virtual void onDisassociate (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                               const std::string&, // SSID
                               bool) = 0;          // success ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onScanComplete (REFGUID) = 0;            // interface identifier
#else
  virtual void onScanComplete (const std::string&) = 0; // interface identifier
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onHotPlug (REFGUID,            // interface identifier
#else
  virtual void onHotPlug (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                          bool) = 0;          // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onRemove (REFGUID,            // interface identifier
#else
  virtual void onRemove (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                         bool) = 0;          // success ?
};

//////////////////////////////////////////

class Net_WLAN_IMonitorBase
 : public Net_WLAN_IMonitorCB
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
 , public Common_IGet_T<HANDLE>
#endif // WLANAPI_SUPPORT
 , public Common_IGet_2_T<WLAN_SIGNAL_QUALITY>
#else
#if defined (WEXT_SUPPORT)
 , public Common_IGet_T<ACE_HANDLE>
#elif defined (NL80211_SUPPORT)
 , public Common_IGetP_T<struct nl_sock>
 , public Common_IGet_T<int>
#elif defined (DBUS_SUPPORT)
 , public Common_IGetP_T<struct DBusConnection>
#endif // WEXT_SUPPORT
 , public Common_IGet_2_T<unsigned int>
#endif // ACE_WIN32 || ACE_WIN64
{
 public:
  // *TODO*: support monitoring multiple interfaces at the same time

  // *IMPORTANT NOTE*: does not block; results are reported by callback (see:
  //                   subscribe())
  // *NOTE*: effectively does the following:
  //         - disconnect from any AP (if associated and SSID is ""/different)
  //         - reconfigure the interface and SSID (if any/different)
  //         - set the 'auto-associate' flag
  //         - resume scanning
  //         --> associate as soon as the given AP/SSID combination is detected
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool associate (REFGUID,                  // interface identifier {GUID_NULL: any}
#else
  virtual bool associate (const std::string&,       // interface identifier {"": any}
                          const struct ether_addr&, // AP BSSID (i.e. AP MAC address)
#endif // ACE_WIN32 || ACE_WIN64
                          const std::string&) = 0;  // (E)SSID {"": disassociate}
  // *IMPORTANT NOTE*: does not block; results are reported by callback (see:
  //                   subscribe())
  // *NOTE*: effectively does the following:
  //         - disconnect from any AP (if associated)
  //         - reconfigure the interface and reset the SSID
  //         - reset the 'auto-associate' flag
  //         - resume scanning
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void scan (REFGUID) = 0; // interface identifier {GUID_NULL: all}
#else
  virtual void scan (const std::string&) = 0; // interface identifier {"": all}
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual struct _GUID interfaceIdentifier () const = 0; // returns currently monitored interface (if any)
#else
  virtual std::string interfaceIdentifier () const = 0; // returns currently monitored interface (if any)
#endif // ACE_WIN32 || ACE_WIN64
  virtual std::string SSID () const = 0; // returns currently associated (E)SSID (if any)

  virtual Net_WLAN_SSIDs_t SSIDs () const = 0; // returns (E)SSID(s) published by the AP(s) within range
};

template <typename AddressType,
          typename ConfigurationType>
class Net_WLAN_IMonitor_T
 : public Net_WLAN_IMonitorBase
 , virtual public Common_ITaskControl_T<ACE_MT_SYNCH,
                                        Common_ILock_T<ACE_MT_SYNCH> >
 , public Common_IGetR_2_T<ConfigurationType>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , public Common_IGet1R_T<std::string> // cache access
 , public Common_ISet2R_T<std::string> // cache access
#endif // ACE_WIN32 || ACE_WIN64
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_ISubscribe_T<Net_WLAN_IMonitorCB>
{
 public:
  virtual bool addresses (AddressType&,            // return value: local SAP
                          AddressType&) const = 0; // return value: peer SAP
};

//////////////////////////////////////////

//typedef Net_WLAN_IMonitor_T<ACE_INET_Addr,
//                            struct Net_WLANMonitorConfiguration> Net_IInetWLANMonitor_t;

#endif
