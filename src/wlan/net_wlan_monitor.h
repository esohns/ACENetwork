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

#ifndef NET_WLAN_MONITOR_H
#define NET_WLAN_MONITOR_H

#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (WEXT_SUPPORT)
#include "iwlib.h"
#endif // WEXT_SUPPORT

#if defined (DBUS_SUPPORT)
#include <map>

#if defined (NL80211_SUPPORT)
#include "ace/Asynch_IO.h"
#endif // NL80211_SUPPORT

#include "dbus/dbus.h"
//#include "dbus/dbus-glib.h"
#endif // DBUS_SUPPORT

//#include "ace/Event_Handler.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

//#include "common_event_handler.h"

//#include "net_common.h"

#include "net_wlan_common.h"
#include "net_wlan_monitor_base.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (NL80211_SUPPORT)
struct sockaddr_nl;
struct nlmsgerr;
struct nl_msg;

static int
network_wlan_nl80211_error_cb (struct sockaddr_nl*,
                               struct nlmsgerr*,
                               void*);

static int
network_wlan_nl80211_finish_cb (struct nl_msg*,
                                void*);
static int
network_wlan_nl80211_ack_cb (struct nl_msg*,
                             void*);
static int
network_wlan_nl80211_no_seq_check_cb (struct nl_msg*,
                                      void*);
static int
network_wlan_nl80211_family_cb (struct nl_msg*,
                                void*);

static int
network_wlan_nl80211_scan_result_cb (struct nl_msg*,
                                     void*);
static int
network_wlan_nl80211_scan_data_cb (struct nl_msg*,
                                   void*);
#endif // NL80211_SUPPORT

#if defined (DBUS_SUPPORT)
void
network_wlan_dbus_main_wakeup_cb (void*);
DBusHandlerResult
network_wlan_dbus_default_filter_cb (struct DBusConnection*,
                                     struct DBusMessage*,
                                     void*);
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

template <typename AddressType,
          typename ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          >
#else
          ////////////////////////////////
          ,ACE_SYNCH_DECL,
          typename TimePolicyType,
#endif
          ////////////////////////////////
          enum Net_WLAN_MonitorAPIType MonitorAPI_e,
          ////////////////////////////////
          typename UserDataType>
class Net_WLAN_Monitor_T
 : public Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  >
#else
                                  ,ACE_SYNCH_USE,
                                  TimePolicyType>
#endif
{
  typedef Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  > inherited;
#else
                                  ,ACE_SYNCH_USE,
                                  TimePolicyType> inherited;
#endif

  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_Monitor_T<AddressType,
                                                ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                                                ACE_SYNCH_USE,
                                                TimePolicyType,
#endif
                                                MonitorAPI_e,
                                                UserDataType>,
                             ACE_SYNCH_MUTEX>;

 public:
  inline virtual ~Net_WLAN_Monitor_T () {}

//  // override (part of) Net_IWLANMonitor_T
//  // *TODO*: remove ASAP
//#if defined (DBUS_SUPPORT)
//  inline virtual const struct DBusConnection* const getP () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
//#endif

 protected:
  Net_WLAN_Monitor_T ();

  UserDataType* userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  // implement (part of) Common_IStateMachine_T
//  virtual void onChange (enum Net_WLAN_MonitorState); // new state
};

//////////////////////////////////////////
// (partial) specializations

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename AddressType,
          typename ConfigurationType,
          ////////////////////////////////
          typename UserDataType>
class Net_WLAN_Monitor_T<AddressType,
                         ConfigurationType,
                         NET_WLAN_MONITOR_API_WLANAPI,
                         UserDataType>
 : public Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType>
{
  typedef Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType> inherited;

  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_Monitor_T<AddressType,
                                                ConfigurationType,
                                                NET_WLAN_MONITOR_API_WLANAPI,
                                                UserDataType>,
                             ACE_SYNCH_MUTEX>;

 public:
  virtual ~Net_WLAN_Monitor_T ();

  // override (part of) Common_ITaskControl_T
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
//  inline bool isRunning () const { return isActive_; }

  // override (part of) Net_IWLANMonitor_T
  virtual bool initialize (const ConfigurationType&); // configuration handle
  // *TODO*: remove ASAP
#if defined (DBUS_SUPPORT)
  inline virtual const struct DBusConnection* const getP () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
#endif

 protected:
  Net_WLAN_Monitor_T ();

  inline virtual bool do_associate (REFGUID interfaceIdentifier_in, const struct ether_addr&, const std::string& SSID_in) { return Net_WLAN_Tools::associate (inherited::clientHandle_, interfaceIdentifier_in, SSID_in); }
  inline virtual void do_scan (REFGUID interfaceIdentifier_in) { ACE_ASSERT (inherited::configuration_); Net_WLAN_Tools::scan (inherited::clientHandle_, interfaceIdentifier_in, inherited::configuration_->SSID); }

  UserDataType* userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  // implement (part of) Common_IStateMachine_T
//  virtual void onChange (enum Net_WLAN_MonitorState); // new state
};
#else
#if defined (WEXT_SUPPORT)
template <typename AddressType,
          typename ConfigurationType,
          ////////////////////////////////
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename UserDataType>
class Net_WLAN_Monitor_T<AddressType,
                         ConfigurationType,
                         ACE_SYNCH_USE,
                         TimePolicyType,
                         NET_WLAN_MONITOR_API_IOCTL,
                         UserDataType>
 : public Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType,
                                  ACE_SYNCH_USE,
                                  TimePolicyType>
{
  typedef Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType,
                                  ACE_SYNCH_USE,
                                  TimePolicyType> inherited;

  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_Monitor_T<AddressType,
                                                ConfigurationType,
                                                ACE_SYNCH_USE,
                                                TimePolicyType,
                                                NET_WLAN_MONITOR_API_IOCTL,
                                                UserDataType>,
                             ACE_SYNCH_MUTEX>;

 public:
  inline virtual ~Net_WLAN_Monitor_T () {}

  // override (part of) Common_ITaskControl_T
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
//  inline bool isRunning () const { return isActive_; }

  // override (part of) Net_IWLANMonitor_T
  virtual bool initialize (const ConfigurationType&); // configuration handle
  // *TODO*: remove ASAP
#if defined (DBUS_SUPPORT)
  inline virtual const struct DBusConnection* const getP () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
#endif

 protected:
  Net_WLAN_Monitor_T ();

  inline virtual bool do_associate (const std::string& interfaceIdentifier_in, const struct ether_addr& APMACAddress_in, const std::string& SSID_in) { return Net_WLAN_Tools::associate (interfaceIdentifier_in, APMACAddress_in, SSID_in, inherited::handle_); }
  inline virtual void do_scan (const std::string& interfaceIdentifier_in) { ACE_ASSERT (inherited::configuration_); Net_WLAN_Tools::scan (interfaceIdentifier_in, inherited::configuration_->SSID, inherited::handle_, false); }

  struct iw_range range_;

  UserDataType*   userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  // implement (part of) Common_IStateMachine_T
//  virtual void onChange (enum Net_WLAN_MonitorState); // new state

  ////////////////////////////////////////

  // override some ACE_Event_Handler methods
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
};
#endif // WEXT_SUPPORT

//////////////////////////////////////////

#if defined (NL80211_SUPPORT)
template <typename AddressType,
          typename ConfigurationType,
          ////////////////////////////////
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename UserDataType>
class Net_WLAN_Monitor_T<AddressType,
                         ConfigurationType,
                         ACE_SYNCH_USE,
                         TimePolicyType,
                         NET_WLAN_MONITOR_API_NL80211,
                         UserDataType>
 : public Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType,
                                  ACE_SYNCH_USE,
                                  TimePolicyType>
// , public Common_EventHandlerBase
 , public ACE_Handler
{
  typedef Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType,
                                  ACE_SYNCH_USE,
                                  TimePolicyType> inherited;
//  typedef Common_EventHandlerBase inherited2;
  typedef ACE_Handler inherited2;

  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_Monitor_T<AddressType,
                                                ConfigurationType,
                                                ACE_SYNCH_USE,
                                                TimePolicyType,
                                                NET_WLAN_MONITOR_API_NL80211,
                                                UserDataType>,
                             ACE_SYNCH_MUTEX>;

 public:
  virtual ~Net_WLAN_Monitor_T ();

  // override (part of) Common_ITaskControl_T
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
//  inline bool isRunning () const { return isActive_; }

  // override (part of) Net_IWLANMonitor_T
  virtual bool initialize (const ConfigurationType&); // configuration handle
  // *TODO*: remove ASAP
#if defined (DBUS_SUPPORT)
  inline virtual const struct DBusConnection* const getP () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
#endif

 protected:
  Net_WLAN_Monitor_T ();

  virtual bool do_associate (const std::string&,
                             const struct ether_addr&,
                             const std::string&);
  virtual void do_scan (const std::string&);

  UserDataType*          userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  // implement (part of) Common_IStateMachine_T
//  virtual void onChange (enum Net_WLAN_MonitorState); // new state

  ////////////////////////////////////////

  // override some ACE_Event_Handler methods
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);

  bool initiate_read_stream (unsigned int); // buffer size

  ACE_Asynch_Read_Stream inputStream_;
  bool                   isRegistered_;
};
#endif // NL80211_SUPPORT

//////////////////////////////////////////

#if defined (DBUS_SUPPORT)
template <typename AddressType,
          typename ConfigurationType,
          ////////////////////////////////
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename UserDataType>
class Net_WLAN_Monitor_T<AddressType,
                         ConfigurationType,
                         ACE_SYNCH_USE,
                         TimePolicyType,
                         NET_WLAN_MONITOR_API_DBUS,
                         UserDataType>
 : public Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType,
                                  ACE_SYNCH_USE,
                                  TimePolicyType>
{
  typedef Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType,
                                  ACE_SYNCH_USE,
                                  TimePolicyType> inherited;

  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_Monitor_T<AddressType,
                                                ConfigurationType,
                                                ACE_SYNCH_USE,
                                                TimePolicyType,
                                                NET_WLAN_MONITOR_API_DBUS,
                                                UserDataType>,
                             ACE_SYNCH_MUTEX>;

 public:
  virtual ~Net_WLAN_Monitor_T ();

  // override (part of) Common_ITaskControl_T
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
//  inline bool isRunning () const { return isActive_; }

  // override (part of) Net_IWLANMonitor_T
  virtual bool initialize (const ConfigurationType&); // configuration handle
  virtual const std::string& get1R (const std::string&) const;
  inline virtual const struct DBusConnection* const getP () const { return connection_; }

 protected:
  // convenient types
  // key: identifier, value: DBus object path
  typedef std::map<std::string, std::string> INTERFACEIDENTIFIER_TO_OBJECTPATH_T;
  typedef INTERFACEIDENTIFIER_TO_OBJECTPATH_T::const_iterator INTERFACEIDENTIFIER_TO_OBJECTPATH_CONSTITERATOR_T;
  typedef INTERFACEIDENTIFIER_TO_OBJECTPATH_T::iterator INTERFACEIDENTIFIER_TO_OBJECTPATH_ITERATOR_T;
  typedef std::pair<std::string, std::string> INTERFACEIDENTIFIER_TO_OBJECTPATH_PAIR_T;
  struct INTERFACEIDENTIFIER_TO_OBJECTPATH_FIND_PREDICATE
   : public std::binary_function<INTERFACEIDENTIFIER_TO_OBJECTPATH_PAIR_T,
                                 std::string,
                                 bool>
  {
    inline bool operator() (const INTERFACEIDENTIFIER_TO_OBJECTPATH_PAIR_T& entry_in, std::string value_in) const { return entry_in.second == value_in; }
  };

  Net_WLAN_Monitor_T ();

  virtual bool do_associate (const std::string&,       // interface identifier
                             const struct ether_addr&, // BSSID
                             const std::string&);      // (E)SSID
  virtual void do_scan (const std::string&); // interface identifier

  struct DBusConnection*              connection_;
//  struct DBusGProxy*                              proxy_;
  INTERFACEIDENTIFIER_TO_OBJECTPATH_T objectPathCache_;

  UserDataType*                       userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  // override some ACE_Task_Base methods
  virtual int svc (void);

  bool                                DBusDispatchStarted_;
};
#endif /* DBUS_SUPPORT */
#endif /* ACE_WIN32 || ACE_WIN64 */

//////////////////////////////////////////

// include template definition
#include "net_wlan_monitor.inl"

#endif
