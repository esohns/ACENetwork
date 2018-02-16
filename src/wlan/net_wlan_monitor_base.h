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
#include <map>
#include <string>
#include <vector>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#include <wlanapi.h>
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

//#include "stream_messagequeue.h"

#include "net_common.h"

#include "net_wlan_common.h"
#include "net_wlan_imonitor.h"
#include "net_wlan_monitor_statemachine.h"
#include "net_wlan_tools.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
void WINAPI
network_wlan_default_notification_cb (PWLAN_NOTIFICATION_DATA,
                                      PVOID);
#endif // ACE_WIN32 || ACE_WIN64

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ////////////////////////////////
          ,ACE_SYNCH_DECL,
          typename TimePolicyType>
#endif
class Net_WLAN_Monitor_Base_T
 : public Net_WLAN_MonitorStateMachine
 , public Net_WLAN_IMonitor_T<AddressType,
                              ConfigurationType>
 , public Common_IStatistic_T<Net_Statistic_t>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , public Common_ITimerHandler
#endif
{
  typedef Net_WLAN_MonitorStateMachine inherited;

 public:
  // convenient types
  typedef Net_WLAN_IMonitor_T<AddressType,
                              ConfigurationType> INTERFACE_T;
  typedef std::list<Net_WLAN_IMonitorCB*> SUBSCRIBERS_T;

  virtual ~Net_WLAN_Monitor_Base_T ();

  // override (part of) Common_ITaskControl_T
  inline bool isRunning () const { return isActive_; }

  // implement Net_IWLANMonitor_T
  inline virtual const ConfigurationType& getR_2 () const { ACE_ASSERT (configuration_); return *configuration_; }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  inline virtual const std::string& get1R (const std::string&) const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (std::string ()); ACE_NOTREACHED (return std::string ();) }
  inline virtual void set2R (const std::string&, const std::string&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif
  virtual bool initialize (const ConfigurationType&); // configuration handle
  virtual void subscribe (Net_WLAN_IMonitorCB*); // new subscriber
  virtual void unsubscribe (Net_WLAN_IMonitorCB*); // existing subscriber
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual const HANDLE get () const { return clientHandle_; }
  inline virtual const WLAN_SIGNAL_QUALITY get_2 () const;
#else
  inline virtual const ACE_HANDLE get () const { return handle_; }
  inline virtual const unsigned int get_2 () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (0); ACE_NOTREACHED (return 0;) }
#endif
  inline virtual bool addresses (AddressType& localSAP_out, AddressType& peerSAP_out) const { localSAP_out = localSAP_; peerSAP_out = peerSAP_; return true; }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool associate (REFGUID,                  // interface identifier
#else
  virtual bool associate (const std::string&,       // interface identifier
                          const struct ether_addr&, // BSSID
#endif
                          const std::string&);      // (E)SSID
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void scan (REFGUID); // interface identifier
#else
  virtual void scan (const std::string&); // interface identifier
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual struct _GUID interfaceIdentifier () const { ACE_ASSERT (configuration_); return configuration_->interfaceIdentifier; }
#else
  inline virtual std::string interfaceIdentifier () const { ACE_ASSERT (configuration_); return configuration_->interfaceIdentifier; }
#endif
  virtual std::string SSID () const;

  virtual Net_WLAN_SSIDs_t SSIDs () const;

 protected:
  // convenient types
  typedef typename SUBSCRIBERS_T::iterator SUBSCRIBERS_ITERATOR_T;

  Net_WLAN_Monitor_Base_T ();

  // *TODO*: remove any implementation-specific members
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HANDLE                                clientHandle_; // API-
  long                                  scanTimerId_;
  Common_TimerHandler                   timerHandler_;
  Common_ITimer_t*                      timerInterface_;
#else
  void*                                 buffer_; // scan results
  size_t                                bufferSize_;
  ACE_HANDLE                            handle_;
#endif
  ConfigurationType*                    configuration_;
  bool                                  isActive_;
  bool                                  isConnectionNotified_;
  bool                                  isFirstConnect_;
  bool                                  isInitialized_;
  AddressType                           localSAP_;
  AddressType                           peerSAP_;
  unsigned int                          retries_; // AP association-/connection-
  Net_WLAN_SSIDToInterfaceIdentifier_t  SSIDCache_;

  // *IMPORTANT NOTE*: this must be 'recursive', so that callees may unsubscribe
  //                   from within the notification callbacks
  mutable ACE_MT_SYNCH::RECURSIVE_MUTEX subscribersLock_;
  SUBSCRIBERS_T                         subscribers_;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool do_associate (REFGUID,                 // interface identifier
#else
  virtual bool do_associate (const std::string&,       // interface identifier
#endif
                             const struct ether_addr&, // BSSID
                             const std::string&) = 0;  // (E)SSID
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void do_scan (REFGUID) = 0;            // interface identifier
#else
  virtual void do_scan (const std::string&) = 0; // interface identifier
#endif

  ////////////////////////////////////////

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

  ////////////////////////////////////////

 private:
  // comvenient types
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//  typedef Stream_MessageQueue_T<ACE_SYNCH_USE,
//                                TimePolicyType,
//                                ACE_Message_Block> MESSAGEQUEUE_T;
//#endif

  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_Base_T (const Net_WLAN_Monitor_Base_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_Base_T& operator= (const Net_WLAN_Monitor_Base_T&))

  // implement (part of) Common_IStateMachine_T
  virtual void onChange (enum Net_WLAN_MonitorState); // new state

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // override some ACE_Event_Handler methods
  inline virtual ACE_HANDLE get_handle (void) const { return handle_; }
  inline virtual void set_handle (ACE_HANDLE handle_in) { ACE_ASSERT (handle_ == ACE_INVALID_HANDLE); ACE_ASSERT (handle_in != ACE_INVALID_HANDLE); handle_ = handle_in; }
#endif

  // hide/override (part of) Common_(Asynch)TaskBase_T
  using inherited::lock;
  using inherited::unlock;
  using inherited::getR;
  inline void finished () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline void wait () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif

  // implement Common_IStatistic_T
  inline virtual bool collect (Net_Statistic_t& statistic_inout) { ACE_UNUSED_ARG (statistic_inout); ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  // *TODO*: report (current) interface statistic(s)
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  void startScanTimer ();
  void cancelScanTimer ();

  // implement Common_ITimerHandler
  inline virtual void handle (const void*) { inherited2::change (NET_WLAN_MONITOR_STATE_SCAN); }
#endif

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//  MESSAGEQUEUE_T                                  queue_;
//#endif
  bool                                  SSIDSeenBefore_;
};

// include template definition
#include "net_wlan_monitor_base.inl"

#endif
