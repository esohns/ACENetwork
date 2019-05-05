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

#if defined (NL80211_SUPPORT)
#include "netlink/handlers.h"
#endif // NL80211_SUPPORT

#if defined (DBUS_SUPPORT)
#include <map>

#include "dbus/dbus.h"
//#include "dbus/dbus-glib.h"
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_wlan_common.h"
#include "net_wlan_monitor_base.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#if defined (NL80211_SUPPORT)
struct sockaddr_nl;
struct nlmsgerr;
struct nl_msg;
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
          typename ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
          ////////////////////////////////
          ACE_SYNCH_DECL,
          typename TimePolicyType,
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
{
  typedef Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  > inherited;
#else
                                  ,ACE_SYNCH_USE,
                                  TimePolicyType> inherited;
#endif // ACE_WIN32 || ACE_WIN64

  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_Monitor_T<AddressType,
                                                ConfigurationType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                                                ACE_SYNCH_USE,
                                                TimePolicyType,
#endif // ACE_WIN32 || ACE_WIN64
                                                MonitorAPI_e,
                                                UserDataType>,
                             ACE_SYNCH_MUTEX>;

 public:
  inline virtual ~Net_WLAN_Monitor_T () {}

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
#if defined (WLANAPI_SUPPORT)
template <//typename AddressType,
          typename ConfigurationType,
          ////////////////////////////////
          typename UserDataType>
class Net_WLAN_Monitor_T<//AddressType,
                         ACE_INET_Addr,
                         ConfigurationType,
                         NET_WLAN_MONITOR_API_WLANAPI,
                         UserDataType>
 : public Net_WLAN_Monitor_Base_T<//AddressType,
                                  ACE_INET_Addr,
                                  ConfigurationType>
{
  typedef Net_WLAN_Monitor_Base_T<//AddressType,
                                  ACE_INET_Addr,
                                  ConfigurationType> inherited;

  // singleton has access to the ctor/dtors
  friend class ACE_Singleton<Net_WLAN_Monitor_T<//AddressType,
                                                ACE_INET_Addr,
                                                ConfigurationType,
                                                NET_WLAN_MONITOR_API_WLANAPI,
                                                UserDataType>,
                             ACE_SYNCH_MUTEX>;

 public:
  inline virtual ~Net_WLAN_Monitor_T () {}

  // override (part of) Common_ITaskControl_T
  virtual void start (ACE_thread_t&); // return value: thread handle (if any)
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
//  inline bool isRunning () const { return isActive_; }

  // override (part of) Net_IWLANMonitor_T
  virtual bool initialize (const ConfigurationType&); // configuration handle
  inline virtual const HANDLE get_2 () const { return inherited::clientHandle_; }
  //inline virtual const WLAN_SIGNAL_QUALITY get_3 () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (0); ACE_NOTREACHED (return 0;) }
  inline virtual std::string SSID () const { return Net_WLAN_Tools::associatedSSID (inherited::clientHandle_, (inherited::configuration_ ? inherited::configuration_->interfaceIdentifier : GUID_NULL)); }

 protected:
  Net_WLAN_Monitor_T ();

  // implement Net_WLAN_IManager
  inline virtual bool do_associate (REFGUID interfaceIdentifier_in, const std::string& SSID_in) { return (SSID_in.empty () ? Net_WLAN_Tools::disassociate (inherited::clientHandle_, interfaceIdentifier_in) : Net_WLAN_Tools::associate (inherited::clientHandle_, interfaceIdentifier_in, SSID_in)); }
  inline virtual bool do_authenticate (REFGUID interfaceIdentifier_in, const struct ether_addr& accessPointMACAddress_in, const std::string& SSID_in) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false); }
  inline virtual void do_scan (REFGUID interfaceIdentifier_in, const struct ether_addr& accessPointMACAddress_in, const std::string& SSID_in) { ACE_UNUSED_ARG (accessPointMACAddress_in); Net_WLAN_Tools::scan (inherited::clientHandle_, interfaceIdentifier_in, SSID_in); }

  UserDataType* userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  // implement (part of) Common_IStateMachine_T
//  virtual void onChange (enum Net_WLAN_MonitorState); // new state
};
#endif // WLANAPI_SUPPORT
#elif defined (ACE_LINUX)
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
  inline virtual const ACE_HANDLE get_2 () const { return inherited::handle_; }

  inline virtual std::string SSID () const { return Net_WLAN_Tools::associatedSSID ((inherited::configuration_ ? inherited::configuration_->interfaceIdentifier : ACE_TEXT_ALWAYS_CHAR ("")), inherited::handle_); }

 protected:
  Net_WLAN_Monitor_T ();

  // implement Net_WLAN_IManager
  inline virtual bool do_associate (const std::string& interfaceIdentifier_in, const struct ether_addr& APMACAddress_in, const std::string& SSID_in) { return Net_WLAN_Tools::associate (interfaceIdentifier_in, APMACAddress_in, SSID_in, inherited::handle_); }
  inline virtual bool do_authenticate (const std::string&, const struct ether_addr&, const std::string&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  inline virtual void do_scan (const std::string& interfaceIdentifier_in, const struct ether_addr& APMACAddress_in, const std::string& SSID_in) { ACE_UNUSED_ARG (APMACAddress_in); Net_WLAN_Tools::scan (interfaceIdentifier_in, SSID_in, inherited::handle_, false); }
  // *TODO*: remove these ASAP
#if defined (NL80211_SUPPORT)
  inline virtual const struct nl_sock* const getP () const { ACE_ASSERT (inherited::socketHandle_); return inherited::socketHandle_; }
  inline virtual const int get_3 () const { ACE_ASSERT (inherited::familyId_ > 0); return inherited::familyId_; }
#endif // NL80211_SUPPORT
#if defined (DBUS_SUPPORT)
  inline virtual const struct DBusConnection* const getP_2 () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
  inline virtual const std::string& get1RR_2 (const std::string&) const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (ACE_TEXT_ALWAYS_CHAR ("")); ACE_NOTREACHED (return ACE_TEXT_ALWAYS_CHAR ("");) }
  inline virtual void set2R (const std::string&, const std::string&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif // DBUS_SUPPORT

  struct iw_range range_;

  UserDataType*   userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  ////////////////////////////////////////

  // override some ACE_Event_Handler methods
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
};
#endif // WEXT_SUPPORT

//////////////////////////////////////////

#if defined (NL80211_SUPPORT)
// *NOTE*: ripped from nelink-private/types.h
#define NL_NO_AUTO_ACK		(1<<4)

struct nl_cb
{
  nl_recvmsg_msg_cb_t	cb_set[NL_CB_TYPE_MAX+1];
  void *			cb_args[NL_CB_TYPE_MAX+1];

  nl_recvmsg_err_cb_t	cb_err;
  void *			cb_err_arg;

  /** May be used to replace nl_recvmsgs with your own implementation
   * in all internal calls to nl_recvmsgs. */
  int			(*cb_recvmsgs_ow)(struct nl_sock *,
              struct nl_cb *);

  /** Overwrite internal calls to nl_recv, must return the number of
   * octets read and allocate a buffer for the received data. */
  int			(*cb_recv_ow)(struct nl_sock *,
                struct sockaddr_nl *,
                unsigned char **,
                struct ucred **);

  /** Overwrites internal calls to nl_send, must send the netlink
   * message. */
  int			(*cb_send_ow)(struct nl_sock *,
                struct nl_msg *);

  int			cb_refcnt;
  /** indicates the callback that is currently active */
  enum nl_cb_type		cb_active;
};

struct nl_sock
{
  struct sockaddr_nl	s_local;
  struct sockaddr_nl	s_peer;
  int			s_fd;
  int			s_proto;
  unsigned int		s_seq_next;
  unsigned int		s_seq_expect;
  int			s_flags;
  struct nl_cb *		s_cb;
  size_t			s_bufsize;
};

// *NOTE*: ripped from nelink-private/netlink.h
static inline int nl_cb_call (struct nl_cb* cb, enum nl_cb_type type, struct nl_msg* msg)
{
  int ret;

  cb->cb_active = type;
  ret = cb->cb_set[type](msg, cb->cb_args[type]);
  cb->cb_active = __NL_CB_TYPE_MAX;
  return ret;
}

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
 , public ACE_Handler
{
  typedef Net_WLAN_Monitor_Base_T<AddressType,
                                  ConfigurationType,
                                  ACE_SYNCH_USE,
                                  TimePolicyType> inherited;
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
  inline virtual const struct nl_sock* const getP () const { ACE_ASSERT (inherited::socketHandle_); return inherited::socketHandle_; }
  inline virtual const int get_3 () const { ACE_ASSERT (inherited::familyId_ > 0); return inherited::familyId_; }
  // *TODO*: remove these ASAP
#if defined (WEXT_SUPPORT)
  inline virtual const ACE_HANDLE get_2 () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (ACE_INVALID_HANDLE); ACE_NOTREACHED (return ACE_INVALID_HANDLE;) }
#endif // WEXT_SUPPORT
#if defined (DBUS_SUPPORT)
  inline virtual const struct DBusConnection* const getP_2 () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
  inline virtual const std::string& get1RR_2 (const std::string&) const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (ACE_TEXT_ALWAYS_CHAR ("")); ACE_NOTREACHED (return ACE_TEXT_ALWAYS_CHAR ("");) }
  inline virtual void set2R (const std::string&, const std::string&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif // DBUS_SUPPORT

  inline virtual std::string SSID () const { return Net_WLAN_Tools::associatedSSID ((inherited::configuration_ ? inherited::configuration_->interfaceIdentifier : ACE_TEXT_ALWAYS_CHAR ("")), NULL, inherited::familyId_); }

 protected:
  Net_WLAN_Monitor_T ();

  // implement Net_WLAN_IManager
  virtual bool do_associate (const std::string&,       // interface identifier {"": any}
                             const struct ether_addr&, // AP BSSID (i.e. AP MAC address)
                             const std::string&);      // (E)SSID {"": disassociate}
  virtual bool do_authenticate (const std::string&,       // interface identifier {"": any}
                                const struct ether_addr&, // AP BSSID (i.e. AP MAC address)
                                const std::string&);      // (E)SSID {"": deauthenticate}
  virtual void do_scan (const std::string&,       // interface identifier {"": all}
                        const struct ether_addr&, // AP BSSID (i.e. AP MAC address) {0: all}
                        const std::string&);      // (E)SSID {"": all}

  UserDataType*                    userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  ////////////////////////////////////////

  // override some ACE_Event_Handler/ACE_Handler methods
  virtual int handle_input (ACE_HANDLE = ACE_INVALID_HANDLE);
  virtual void handle_read_stream (const ACE_Asynch_Read_Stream::Result&);

  // override some ACE_Task_Base methods
  virtual int svc (void);

  ACE_Message_Block* allocateMessage (unsigned int); // buffer size
  // *NOTE*: fire-and-forget API (second argument)
  void processMessage (const struct sockaddr_nl&, // source address
                       ACE_Message_Block*&);      // i/o: message buffer
  bool initiate_read_stream (unsigned int); // buffer size

  ACE_Message_Block*               buffer_;
  struct nl_cb*                    callbacks_;
  int                              controlId_;
  int                              error_;
  struct Net_WLAN_nl80211_Features features_;
  bool                             headerReceived_;
  ACE_Asynch_Read_Stream           inputStream_;
  bool                             isRegistered_;
  bool                             isSubscribedToMulticastGroups_;
  struct nl_msg*                   message_;
  ACE_UINT32                       protocolFeatures_;
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
  inline virtual const struct DBusConnection* const getP_2 () const { return connection_; }
  virtual const std::string& get1RR_2 (const std::string&) const;
  virtual void set2R (const std::string&,
                      const std::string&);
  // *TODO*: remove these ASAP
//#if defined (WEXT_SUPPORT)
//  inline virtual const ACE_HANDLE get_2 () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (ACE_INVALID_HANDLE); ACE_NOTREACHED (return ACE_INVALID_HANDLE;) }
//#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
  inline virtual const struct nl_sock* const getP () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
  inline virtual const int get_3 () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
#endif // NL80211_SUPPORT

  inline virtual std::string SSID () const { return Net_WLAN_Tools::associatedSSID (connection_, (inherited::configuration_ ? inherited::configuration_->interfaceIdentifier : ACE_TEXT_ALWAYS_CHAR (""))); }

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

  // implement Net_WLAN_IManager
  virtual bool do_associate (const std::string&,       // interface identifier {"": any}
                             const struct ether_addr&, // AP BSSID (i.e. AP MAC address)
                             const std::string&);      // (E)SSID {"": disassociate}
  inline virtual bool do_authenticate (const std::string&, const struct ether_addr&, const std::string&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  virtual void do_scan (const std::string&,       // interface identifier {"": all}
                        const struct ether_addr&, // AP BSSID (i.e. AP MAC address) {0: all}
                        const std::string&);      // (E)SSID {"": all}

  struct DBusConnection*              connection_;
//  struct DBusGProxy*                              proxy_;
  INTERFACEIDENTIFIER_TO_OBJECTPATH_T objectPathCache_;

  UserDataType*                       userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T (const Net_WLAN_Monitor_T&))
  ACE_UNIMPLEMENTED_FUNC (Net_WLAN_Monitor_T& operator= (const Net_WLAN_Monitor_T&))

  // override some ACE_Task_Base methods
  virtual int svc (void);
};
#endif /* DBUS_SUPPORT */
#endif /* ACE_WIN32 || ACE_WIN64 || ACE_LINUX */

//////////////////////////////////////////

// include template definition(s)
#include "net_wlan_monitor.inl"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
#include "net_wlan_monitor_win32_wlanapi.inl"
#endif /* WLANAPI_SUPPORT */
#elif defined (ACE_LINUX)
#if defined (WEXT_SUPPORT)
#include "net_wlan_monitor_wext.inl"
#endif /* WEXT_SUPPORT */
#if defined (NL80211_SUPPORT)
#include "net_wlan_monitor_nl80211.inl"
#endif /* NL80211_SUPPORT */
#if defined (DBUS_SUPPORT)
#include "net_wlan_monitor_dbus.inl"
#endif /* DBUS_SUPPORT */
#endif /* ACE_WIN32 || ACE_WIN64 || ACE_LINUX */

#endif
