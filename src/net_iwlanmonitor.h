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

#ifndef NET_IWLANMONITOR_T_H
#define NET_IWLANMONITOR_T_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#else
#include "dbus/dbus.h"
#endif

#include "ace/INET_Addr.h"

#include "common_iget.h"
#include "common_iinitialize.h"
#include "common_isubscribe.h"

class Net_IWLANCB
{
 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onAssociate (REFGUID,            // device identifier
#else
  virtual void onAssociate (const std::string&, // device identifier
#endif
                            const std::string&, // SSID
                            bool) = 0;          // success ?
  // *IMPORTANT NOTE*: Net_IWLANMonitor_T::addresses() may not return
  //                   significant data before this, as the link layer
  //                   configuration (e.g. DHCP handshake, ...) most likely has
  //                   not been established
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onConnect (REFGUID,            // device identifier
#else
  virtual void onConnect (const std::string&, // device identifier
#endif
                          const std::string&, // SSID
                          bool) = 0;          // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onHotPlug (REFGUID,            // device identifier
#else
  virtual void onHotPlug (const std::string&, // device identifier
#endif
                          bool) = 0;          // enabled ? : disabled/removed
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onScanComplete (REFGUID) = 0;            // device identifier
#else
  virtual void onScanComplete (const std::string&) = 0; // device identifier
#endif
};

//////////////////////////////////////////

class Net_IWLANMonitorBase
 : public Net_IWLANCB
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , public Common_IGet_T<HANDLE>
#else
 , public Common_IGetP_T<struct DBusConnection>
#endif
{
 public:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool associate (REFGUID,                 // device identifier {GUID_NULL: any}
#else
  virtual bool associate (const std::string&,      // device identifier {"": any}
#endif
                          const std::string&) = 0; // SSID

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  virtual std::string deviceIdentifier () const = 0;
#endif
  virtual std::string SSID () const = 0;
};

template <typename AddressType,
          typename ConfigurationType>
class Net_IWLANMonitor_T
 : public Net_IWLANMonitorBase
 , public Common_IGetR_T<ConfigurationType>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , public Common_IGet1R_T<std::string> // cache access
 , public Common_ISet2R_T<std::string> // cache access
#endif
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_ISubscribe_T<Net_IWLANCB>
{
 public:
  virtual bool addresses (AddressType&,            // return value: local SAP
                          AddressType&) const = 0; // return value: peer SAP
};

//////////////////////////////////////////

//typedef Net_IWLANMonitor_T<ACE_INET_Addr,
//                           struct Net_WLANMonitorConfiguration> Net_IInetWLANMonitor_t;

#endif
