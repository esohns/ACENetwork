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

#ifndef NET_WLAN_MONITOR_T_H
#define NET_WLAN_MONITOR_T_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#include <wlanapi.h>
#else
#include <net/ethernet.h>

#include "iwlib.h"

#if defined (DBUS_SUPPORT)
#include "dbus/dbus.h"
//#include "dbus/dbus-glib.h"
#endif // DBUS_SUPPORT

#include "ace/Event_Handler.h"
#endif

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_ilock.h"
#include "common_istatistic.h"
#include "common_task_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_timer_common.h"
#include "common_timer_handler.h"
#endif

#include "stream_messagequeue.h"

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
#else
#if defined (DBUS_SUPPORT)
void
network_wlan_dbus_main_wakeup_cb (void*);
DBusHandlerResult
network_wlan_dbus_default_filter_cb (struct DBusConnection*,
                                     struct DBusMessage*,
                                     void*);
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          ////////////////////////////////
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          ////////////////////////////////
          typename UserDataType>
class Net_WLAN_Monitor_T
 : public Common_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            Common_ILock_T<ACE_SYNCH_USE> >
 , public Net_WLAN_MonitorStateMachine
 , public Net_WLAN_IMonitor_T<AddressType,
                              ConfigurationType>
 , public Common_IStatistic_T<Net_Statistic_t>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , public Common_ITimerHandler
#endif
{
  typedef Common_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            Common_ILock_T<ACE_SYNCH_USE> > inherited;
  typedef Net_WLAN_MonitorStateMachine inherited2;

  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                AddressType,
                                                ConfigurationType,
                                                MonitorAPI_e,
                                                UserDataType>,
                             ACE_SYNCH_MUTEX>;

 public:
  // convenient types
  typedef Net_WLAN_IMonitor_T<AddressType,
                              ConfigurationType> INTERFACE_T;
  typedef std::list<Net_WLAN_IMonitorCB*> SUBSCRIBERS_T;

  virtual ~Net_WLAN_Monitor_T ();

  // override (part of) Common_ITaskControl_T
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
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
#if defined (DBUS_SUPPORT)
  inline virtual const struct DBusConnection* const getP () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
#endif
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
  struct Association_Configuration
  {
    Association_Configuration ()
     : accessPointLinkLayerAddress ()
     , signalQuality (0)
    {
      ACE_OS::memset (accessPointLinkLayerAddress.ether_addr_octet,
                      0,
                      sizeof (u_char[ETH_ALEN]));
    };

    struct ether_addr   accessPointLinkLayerAddress;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    WLAN_SIGNAL_QUALITY signalQuality;
#else
    unsigned int        signalQuality;
#endif
  };
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef std::vector<struct _GUID> INTERFACEIDENTIFIERS_T;
  typedef std::multimap<std::string, std::pair<struct _GUID, struct Association_Configuration> > SSIDS_TO_INTERFACEIDENTIFIER_MAP_T;
#else
  typedef std::multimap<std::string, std::pair<std::string, struct Association_Configuration> > SSIDS_TO_INTERFACEIDENTIFIER_MAP_T;
#endif
  typedef typename SSIDS_TO_INTERFACEIDENTIFIER_MAP_T::const_iterator SSIDS_TO_INTERFACEIDENTIFIER_MAP_CONST_ITERATOR_T;
  typedef typename SSIDS_TO_INTERFACEIDENTIFIER_MAP_T::iterator SSIDS_TO_INTERFACEIDENTIFIER_MAP_ITERATOR_T;
  typedef std::pair<typename SSIDS_TO_INTERFACEIDENTIFIER_MAP_T::iterator, bool> SSIDS_TO_INTERFACEIDENTIFIER_MAP_RESULT_T;

  Net_WLAN_Monitor_T ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HANDLE                                          clientHandle_; // API-
  long                                            scanTimerId_;
  Common_TimerHandler                             timerHandler_;
  Common_ITimer_t*                                timerInterface_;
#else
  void*                                           buffer_; // scan results
  size_t                                          bufferSize_;
  ACE_HANDLE                                      handle_;
  struct iw_range                                 range_;
#endif
  ConfigurationType*                              configuration_;
  Net_InterfaceIdentifiers_t                      interfaceIdentifiers_;
  bool                                            isActive_;
  bool                                            isConnectionNotified_;
  bool                                            isFirstConnect_;
  bool                                            isInitialized_;
  AddressType                                     localSAP_;
  AddressType                                     peerSAP_;
  unsigned int                                    retries_; // AP association-/connection-
  SSIDS_TO_INTERFACEIDENTIFIER_MAP_T              SSIDsToInterfaceIdentifier_; // cache

  // *IMPORTANT NOTE*: this must be 'recursive', so that callees may unsubscribe
  //                   from within the notification callbacks
  mutable typename ACE_SYNCH_USE::RECURSIVE_MUTEX subscribersLock_;
  SUBSCRIBERS_T                                   subscribers_;

  UserDataType*                                   userData_;

 private:
  // comvenient types
  typedef Stream_MessageQueue_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                ACE_Message_Block> MESSAGEQUEUE_T;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef std::pair<std::string, std::pair <struct _GUID, struct Association_Configuration> > SSIDS_TO_INTERFACEIDENTIFIER_PAIR_T;
  struct SSIDS_TO_INTERFACEIDENTIFIER_FIND_S
   : public std::binary_function<SSIDS_TO_INTERFACEIDENTIFIER_PAIR_T,
                                 struct _GUID,
                                 bool>
  { inline bool operator() (const SSIDS_TO_INTERFACEIDENTIFIER_PAIR_T& entry_in, struct _GUID value_in) const { return InlineIsEqualGUID (entry_in.second.first, value_in); } };
#else
  typedef std::pair<std::string, std::pair <std::string, struct Association_Configuration> > SSIDS_TO_INTERFACEIDENTIFIER_PAIR_T;
  struct SSIDS_TO_INTERFACEIDENTIFIER_FIND_S
   : public std::binary_function<SSIDS_TO_INTERFACEIDENTIFIER_PAIR_T,
                                 std::string,
                                 bool>
  { inline bool operator() (const SSIDS_TO_INTERFACEIDENTIFIER_PAIR_T& entry_in, std::string value_in) const { return entry_in.second.first == value_in; } };
#endif

  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  // implement (part of) Common_IStateMachine_T
  virtual void onChange (enum Net_WLAN_MonitorState); // new state

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // override some ACE_Task_Base methods
  virtual int svc (void);

  // override some ACE_Event_Handler methods
  inline virtual ACE_HANDLE get_handle (void) const { return handle_; }
  inline virtual void set_handle (ACE_HANDLE handle_in) { ACE_ASSERT (handle_ == ACE_INVALID_HANDLE); ACE_ASSERT (handle_in != ACE_INVALID_HANDLE); handle_ = handle_in; }
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
#endif

  // hide/override (part of) Common_TaskBase_T
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

  MESSAGEQUEUE_T                                  queue_;
  bool                                            SSIDSeenBefore_;
};

//////////////////////////////////////////
// partial specialization (for DBus strategy)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DBUS_SUPPORT)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename AddressType,
          typename ConfigurationType,
          ////////////////////////////////
          typename UserDataType>
class Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         AddressType,
                         ConfigurationType,
                         NET_WLAN_MONITOR_API_DBUS,
                         UserDataType>
 : public Common_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            Common_ILock_T<ACE_SYNCH_USE> >
 , public Net_WLAN_IMonitor_T<AddressType,
                              ConfigurationType>
{
  typedef Common_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            Common_ILock_T<ACE_SYNCH_USE> > inherited;

  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_Monitor_T<ACE_SYNCH_USE,
                                                TimePolicyType,
                                                AddressType,
                                                ConfigurationType,
                                                NET_WLAN_MONITOR_API_DBUS,
                                                UserDataType>,
                             ACE_SYNCH_MUTEX>;

 public:
  // convenient types
  typedef Net_WLAN_IMonitor_T<AddressType,
                              ConfigurationType> INTERFACE_T;
  typedef std::list<Net_WLAN_IMonitorCB*> SUBSCRIBERS_T;

  virtual ~Net_WLAN_Monitor_T ();

  // override (part of) Common_ITaskControl_T
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  inline bool isRunning () const { return isActive_; }

  // implement Net_IWLANMonitor_T
  inline virtual const ConfigurationType& getR_2 () const { ACE_ASSERT (configuration_); return *configuration_; }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  virtual const std::string& get1R (const std::string&) const;
  inline virtual void set2R (const std::string& key_in, const std::string& value_in) { identifierToObjectPath_.insert (std::make_pair (key_in, value_in)); }
#endif
  virtual bool initialize (const ConfigurationType&); // configuration handle
  virtual void subscribe (Net_WLAN_IMonitorCB*); // new subscriber
  virtual void unsubscribe (Net_WLAN_IMonitorCB*); // existing subscriber
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual const HANDLE get () const { return clientHandle_; };
#else
  inline virtual const struct DBusConnection* const getP () const { return connection_; }
#endif
  inline virtual bool addresses (AddressType& localSAP_out, AddressType& peerSAP_out) const { localSAP_out = localSAP_; peerSAP_out = peerSAP_; return true; }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool associate (REFGUID,             // interface identifier
#else
  virtual bool associate (const std::string&,  // interface identifier
#endif
                          const std::string&); // SSID
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline virtual void scan (REFGUID interfaceIdentifier_in) { Net_WLAN_Tools::scan (clientHandle_, interfaceIdentifier_in, ACE_TEXT_ALWAYS_CHAR ("")); }
#else
  inline virtual void scan (const std::string& interfaceIdentifier_in) { Net_WLAN_Tools::scan (interfaceIdentifier_in, ACE_TEXT_ALWAYS_CHAR (""), handle_, false); }
#endif
  inline virtual std::string interfaceIdentifier () const { ACE_ASSERT (configuration_); return configuration_->interfaceIdentifier; }
  inline virtual std::string SSID () const { ACE_ASSERT (configuration_); return Net_WLAN_Tools::associatedSSID (configuration_->interfaceIdentifier); }

  virtual Net_WLAN_SSIDs_t SSIDs () const;

 protected:
  // convenient types
  typedef typename SUBSCRIBERS_T::iterator SUBSCRIBERS_ITERATOR_T;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef std::vector<struct _GUID> INTERFACEIDENTIFIERS_T;
#else
  // key: identifier, value: DBus object path
  typedef std::map<std::string, std::string> INTERFACEIDENTIFIERS_T;
  typedef std::pair<std::string, std::string> INTERFACEIDENTIFIERS_PAIR_T;
  struct INTERFACEIDENTIFIERS_FIND_S
   : public std::binary_function<INTERFACEIDENTIFIERS_PAIR_T,
                                 std::string,
                                 bool>
  {
    inline bool operator() (const INTERFACEIDENTIFIERS_PAIR_T& entry_in, std::string value_in) const { return entry_in.second == value_in; }
  };
  typedef INTERFACEIDENTIFIERS_T::const_iterator INTERFACEIDENTIFIERS_CONSTITERATOR_T;
#endif
  typedef INTERFACEIDENTIFIERS_T::iterator INTERFACEIDENTIFIERS_ITERATOR_T;

  Net_WLAN_Monitor_T ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HANDLE                                          clientHandle_;
#else
  struct DBusConnection*                          connection_;
  INTERFACEIDENTIFIERS_T                          identifierToObjectPath_;
//  struct DBusGProxy*                              proxy_;
#endif
  ConfigurationType*                              configuration_;
  bool                                            isActive_;
  bool                                            isInitialized_;
  AddressType                                     localSAP_;
  AddressType                                     peerSAP_;

  // *IMPORTANT NOTE*: this must be 'recursive', so that callees may unsubscribe
  //                   from within the notification callbacks
  mutable typename ACE_SYNCH_USE::RECURSIVE_MUTEX subscribersLock_;
  SUBSCRIBERS_T                                   subscribers_;

  UserDataType*                                   userData_;

 private:
  // comvenient types
  typedef Stream_MessageQueue_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                ACE_Message_Block> MESSAGEQUEUE_T;

  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  // implement Net_IWLANCB
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onAssociate (REFGUID,            // interface identifier
#else
  virtual void onAssociate (const std::string&, // interface identifier
#endif
                            const std::string&, // SSID
                            bool);              // success ?
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
  virtual void onHotPlug (REFGUID,            // interface identifier
#else
  virtual void onHotPlug (const std::string&, // interface identifier
#endif
                          bool);              // arrival ? : removal
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual void onScanComplete (REFGUID);            // interface identifier
#else
  virtual void onScanComplete (const std::string&); // interface identifier
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // override some ACE_Task_Base methods
  virtual int svc (void);
#endif

  // hide/override (part of) Common_TaskBase_T
  using inherited::lock;
  using inherited::unlock;
  using inherited::getR;
  inline void finished () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline void wait () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif

  // helper functions
  INTERFACEIDENTIFIERS_T getDevices () const;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  bool                                            DBusDispatchStarted_;
#endif
  MESSAGEQUEUE_T                                  queue_;
};
#endif /* DBUS_SUPPORT */
#endif /* ACE_WIN32 || ACE_WIN64 */

//////////////////////////////////////////

// include template definition
#include "net_wlan_monitor.inl"

#endif