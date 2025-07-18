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

#ifndef TEST_U_EVENTHANDLER_H
#define TEST_U_EVENTHANDLER_H

#include "ace/Global_Macros.h"

#include "net_wlan_imonitor.h"

// forward declarations
struct WLANMonitor_UI_CBData;

class Test_U_EventHandler
 : public Net_WLAN_IMonitorCB
{
 public:
  Test_U_EventHandler (struct WLANMonitor_UI_CBData*); // UI callback data
  inline virtual ~Test_U_EventHandler () {}

  // implement Net_WLAN_IMonitorCB
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onSignalQualityChange (REFGUID,              // interface identifier
                                      WLAN_SIGNAL_QUALITY); // signal quality (of current association)
#else
  virtual void onSignalQualityChange (const std::string&, // interface identifier
                                      unsigned int);      // signal quality (of current association)
#endif
  // *IMPORTANT NOTE*: Net_IWLANMonitor_T::addresses() may not return
  //                   significant data before this, as the link layer
  //                   configuration (e.g. DHCP handshake, ...) most likely has
  //                   not been established
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onConnect (REFGUID,            // interface identifier
#else
  virtual void onConnect (const std::string&, // interface identifier
#endif
                          const std::string&, // SSID
                          bool);              // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onDisconnect (REFGUID,            // interface identifier
#else
  virtual void onDisconnect (const std::string&, // interface identifier
#endif
                             const std::string&, // SSID
                             bool);              // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onAssociate (REFGUID,            // interface identifier
#else
  virtual void onAssociate (const std::string&, // interface identifier
#endif
                            const std::string&, // SSID
                            bool);              // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onDisassociate (REFGUID,            // interface identifier
#else
  virtual void onDisassociate (const std::string&, // interface identifier
#endif
                               const std::string&, // SSID
                               bool);              // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onScanComplete (REFGUID);            // interface identifier
#else
  virtual void onScanComplete (const std::string&); // interface identifier
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onHotPlug (REFGUID,            // interface identifier
#else
  virtual void onHotPlug (const std::string&, // interface identifier
#endif
                          bool);              // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onRemove (REFGUID,            // interface identifier
#else
  virtual void onRemove (const std::string&, // interface identifier
#endif
                         bool);              // success ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler ())
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler (const Test_U_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Test_U_EventHandler& operator=(const Test_U_EventHandler&))

  struct WLANMonitor_UI_CBData* CBData_;
};

#endif
