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

#ifndef NET_WLAN_MONITOR_BASE_H
#define NET_WLAN_MONITOR_BASE_H

#include <list>
#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#if defined (WLANAPI_SUPPORT)
#include <wlanapi.h>
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
#if defined (DHCLIENT_SUPPORT)
extern "C"
{
#include "dhcpctl/dhcpctl.h"
}
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_ilock.h"
#include "common_istatistic.h"
#include "common_task_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_timer_common.h"
#include "common_timer_handler.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "net_common.h"

#include "net_wlan_common.h"
#include "net_wlan_imanager.h"
#include "net_wlan_imonitor.h"
#include "net_wlan_monitor_statemachine.h"
#include "net_wlan_tools.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
struct _L2_NOTIFICATION_DATA;
void WINAPI
network_wlan_default_notification_cb (struct _L2_NOTIFICATION_DATA*,
                                      PVOID);
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
struct nl_sock;
#endif // NL80211_SUPPORT

#if defined (DHCLIENT_SUPPORT)
void
net_wlan_dhclient_connection_event_cb (dhcpctl_handle, // omapi object handle
                                       dhcpctl_status, // result status
                                       void*);         // user data
void
net_wlan_dhclient_connect_cb (dhcpctl_handle, // omapi object handle
                              dhcpctl_status, // result status
                              void*);         // user data
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ////////////////////////////////
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif // ACE_WIN32 || ACE_WIN64
class Net_WLAN_Monitor_Base_T
 : public Net_WLAN_MonitorStateMachine
 , public Net_WLAN_IManager
 , public Net_WLAN_IMonitor_T<AddressType,
                              ConfigurationType>
 , public Common_IStatistic_T<Net_Statistic_t>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , public Common_ITimerHandler
#endif // ACE_WIN32 || ACE_WIN64
{
  typedef Net_WLAN_MonitorStateMachine inherited;

 public:
  // convenient types
  typedef Net_WLAN_MonitorStateMachine STATEMACHINE_T;
  typedef Net_WLAN_IMonitor_T<AddressType,
                              ConfigurationType> INTERFACE_T;
  typedef std::list<Net_WLAN_IMonitorCB*> SUBSCRIBERS_T;

  virtual ~Net_WLAN_Monitor_Base_T ();

  // override (part of) Common_ITaskControl_T
  inline bool isRunning () const { return isActive_; }

  // implement Net_IWLANMonitor_T
  inline virtual const ConfigurationType& getR_4 () const { ACE_ASSERT (configuration_); return *configuration_; }
  virtual const Net_WLAN_AccessPointCacheValue_t& get1RR (const std::string&) const;
  virtual void set3R (const std::string&,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      REFGUID,
#else
                      const std::string&,
#endif // ACE_WIN32 || ACE_WIN64
                      const struct Net_WLAN_AccessPointState&);
  virtual bool initialize (const ConfigurationType&); // configuration handle
  virtual void subscribe (Net_WLAN_IMonitorCB*); // new subscriber
  virtual void unsubscribe (Net_WLAN_IMonitorCB*); // existing subscriber
  inline virtual bool addresses (AddressType& localSAP_out, AddressType& peerSAP_out) const { localSAP_out = localSAP_; peerSAP_out = peerSAP_; return true; }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool associate (REFGUID,                  // interface identifier
#else
  virtual bool associate (const std::string&,       // interface identifier
                          const struct ether_addr&, // BSSID
#endif // ACE_WIN32 || ACE_WIN64
                          const std::string&);      // (E)SSID
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void scan (REFGUID); // interface identifier
#else
  virtual void scan (const std::string&); // interface identifier
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual struct _GUID interfaceIdentifier () const { ACE_ASSERT (configuration_); return configuration_->interfaceIdentifier; }
  inline virtual const WLAN_SIGNAL_QUALITY get_3 () const;
#else
  inline virtual std::string interfaceIdentifier () const { ACE_ASSERT (configuration_); return configuration_->interfaceIdentifier; }
  inline virtual const unsigned int get_3 () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (0); ACE_NOTREACHED (return 0;) }
#endif // ACE_WIN32 || ACE_WIN64

  virtual Net_WLAN_SSIDs_t SSIDs () const;

 protected:
  // convenient types
  typedef typename SUBSCRIBERS_T::iterator SUBSCRIBERS_ITERATOR_T;

  Net_WLAN_Monitor_Base_T ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool startScanTimer (const ACE_Time_Value& = ACE_Time_Value::zero); // delay {ACE_Time_Value::zero: expire immediately}
  bool cancelScanTimer ();

  // implement Common_ITimerHandler
  // *NOTE*: the scan timer is one-shot and restarted during the state-change
  inline virtual const long get () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (0); ACE_NOTREACHED (return 0;) }
  virtual void handle (const void*);
#endif // ACE_WIN32 || ACE_WIN64

  // *TODO*: remove any implementation-specific members
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
  HANDLE                                clientHandle_; // API-
#endif // WLANAPI_SUPPORT
  long                                  scanTimerId_;
  Common_TimerHandler                   timerHandler_;
  Common_ITimer_t*                      timerInterface_;
#elif defined (ACE_LINUX)
#if defined (WEXT_SUPPORT)
  void*                                 buffer_; // scan results
  size_t                                bufferSize_;
  ACE_HANDLE                            handle_;
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
  struct Net_WLAN_nl80211_CBData        nl80211CBData_;
  int                                   familyId_;
  struct nl_sock*                       socketHandle_;
#endif // NL80211_SUPPORT
#if defined (DBUS_SUPPORT)
#endif // DBUS_SUPPORT

#if defined (DHCLIENT_SUPPORT)
  struct Net_WLAN_dhclient_CBData       dhclientCBData_;
#endif // DHCLIENT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  ConfigurationType*                    configuration_;
  bool                                  isActive_;
  bool                                  isConnectionNotified_;
  bool                                  isFirstConnect_;
  bool                                  isInitialized_;
  AddressType                           localSAP_;
  AddressType                           peerSAP_;
  unsigned int                          retries_; // AP association-/connection-
  Net_WLAN_AccessPointCache_t           SSIDCache_;

  // *IMPORTANT NOTE*: this must be 'recursive', so that callees may unsubscribe
  //                   from within the notification callbacks
  // *NOTE*: also secures scanTimerId_
  mutable ACE_MT_SYNCH::RECURSIVE_MUTEX subscribersLock_;
  SUBSCRIBERS_T                         subscribers_;

  ////////////////////////////////////////

  // implement Net_WLAN_IMonitorCB
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onSignalQualityChange (REFGUID,              // interface identifier
                                      WLAN_SIGNAL_QUALITY); // signal quality (of current association)
#else
  virtual void onSignalQualityChange (const std::string&, // interface identifier
                                      unsigned int);      // signal quality (of current association)
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
                          bool);              // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onDisconnect (REFGUID,            // interface identifier
#else
  virtual void onDisconnect (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                             const std::string&, // SSID
                             bool);              // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  virtual void onAssociate (const std::string&, // interface identifier
                            const std::string&, // SSID
                            bool);              // success ?
  virtual void onDisassociate (const std::string&, // interface identifier
                               const std::string&, // SSID
                               bool);              // success ?
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onScanComplete (REFGUID);            // interface identifier
#else
  virtual void onScanComplete (const std::string&); // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onHotPlug (REFGUID,            // interface identifier
#else
  virtual void onHotPlug (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                          bool);              // success ?
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onRemove (REFGUID,            // interface identifier
#else
  virtual void onRemove (const std::string&, // interface identifier
#endif // ACE_WIN32 || ACE_WIN64
                         bool);              // success ?

  ////////////////////////////////////////

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_Base_T (const Net_WLAN_Monitor_Base_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_Base_T& operator= (const Net_WLAN_Monitor_Base_T&))

  // implement (part of) Common_IStateMachine_T
  virtual void onChange (enum Net_WLAN_MonitorState); // new state

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual bool lock (bool = true) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  inline virtual int unlock (bool = false) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual const ACE_MT_SYNCH::MUTEX& getR () const { ACE_MT_SYNCH::MUTEX lock; return lock; }
  inline virtual void idle () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void finished () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void wait (bool = true) const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#elif defined (ACE_LINUX)
  // override some ACE_Event_Handler methods
#if defined (WEXT_SUPPORT)
  inline virtual ACE_HANDLE get_handle (void) const { return handle_; }
  inline virtual void set_handle (ACE_HANDLE handle_in) { ACE_ASSERT (handle_ == ACE_INVALID_HANDLE); ACE_ASSERT (handle_in != ACE_INVALID_HANDLE); handle_ = handle_in; }
#endif // WEXT_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

  // hide/override (part of) Common_(Asynch)TaskBase_T
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
  using inherited::lock;
  using inherited::unlock;
  using inherited::getR;
  inline virtual void finished () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual void wait () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // implement Common_IStatistic_T
  inline virtual bool collect (Net_Statistic_t& statistic_inout) { ACE_UNUSED_ARG (statistic_inout); ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  // *TODO*: report (current) interface statistic(s)
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  bool                                  SSIDSeenBefore_;
};

// include template definition
#include "net_wlan_monitor_base.inl"

#endif
