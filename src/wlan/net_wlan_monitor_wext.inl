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

#include <algorithm>
#include <functional>

#include "net/ethernet.h"

#if defined (DHCLIENT_SUPPORT)
extern "C"
{
#include "dhcpctl/dhcpctl.h"
}
#endif // DHCLIENT_SUPPORT

#if defined (_DEBUG)
#include "ace/High_Res_Timer.h"
#endif // _DEBUG
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"
#include "ace/Time_Value.h"

#include "common_tools.h"

#if defined (DBUS_SUPPORT)
#include "common_dbus_tools.h"
#endif // DBUS_SUPPORT

#if defined (_DEBUG)
#include "common_timer_tools.h"
#endif // _DEBUG

#include "stream_defines.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"
#include "net_packet_headers.h"

#include "net_wlan_defines.h"

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_WEXT,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited ()
 , range_ ()
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

  //  inherited::reactor (ACE_Reactor::instance ());
  ACE_OS::memset (&range_, 0, sizeof (struct iw_range));
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_WEXT,
                   UserDataType>::start (ACE_Time_Value* timeout_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::start"));

  ACE_UNUSED_ARG (timeout_in);

  // sanity check(s)
  if (unlikely (!inherited::isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, aborting\n")));
    return false;
  } // end IF
  if (unlikely (inherited::isActive_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("already started, aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::handle_ == ACE_INVALID_HANDLE);
#if defined (DBUS_SUPPORT) && defined (SD_BUS_SUPPORT)
  if (Common_DBus_Tools::isUnitRunning (NULL,
                                        COMMON_SYSTEMD_UNIT_NETWORKMANAGER))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("systemd unit \"%s\" is running; this may interfere with the monitoring activities, continuing\n"),
                ACE_TEXT (COMMON_SYSTEMD_UNIT_NETWORKMANAGER)));
  } // end IF
  else if (Common_DBus_Tools::isUnitRunning (NULL,
                                             COMMON_SYSTEMD_UNIT_WPASUPPLICANT))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("systemd unit \"%s\" is running; this may interfere with the monitoring activities, continuing\n"),
                ACE_TEXT (COMMON_SYSTEMD_UNIT_WPASUPPLICANT)));
  } // end ELSE IF
#endif // DBUS_SUPPORT && SD_BUS_SUPPORT

  inherited::handle_ = ACE_OS::socket (AF_INET,
                                       SOCK_DGRAM,
                                       0);
  if (unlikely (inherited::handle_ == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::socket(AF_INET,SOCK_DGRAM,0): \"%m\", aborting\n")));
    return false;
  } // end IF
//  inherited::set_handle (inherited::handle_);

  int result =
      iw_get_range_info (inherited::handle_,
                         inherited::configuration_->interfaceIdentifier.c_str (),
                         &range_);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to iw_get_range_info(%d,\"%s\"): \"%m\", aborting\n"),
                inherited::handle_,
                ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ())));
    return false;
  } // end IFs
  // verify that the interface supports scanning
  if (range_.we_version_compiled < 14)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("interface (was: \"%s\") does not support scanning, aborting\n"),
                ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ())));
    return false;
  } // end IF

  // monitor the interface in a dedicated thread
  ACE_Time_Value deadline =
      (COMMON_TIME_NOW +
       ACE_Time_Value (0, COMMON_TIMEOUT_DEFAULT_THREAD_SPAWN * 1000));
  result = inherited::open (NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", aborting\n")));
    return false;
  } // end IF
  do
  { // *NOTE*: the livelock here
    if (COMMON_TIME_NOW > deadline)
    {
      ACE_OS::last_error (ETIMEDOUT);
      break;
    } // end IF
  } while (!inherited::TASK_T::isRunning ());
  if (unlikely (!inherited::TASK_T::isRunning ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", aborting\n")));
    return false;
  } // end IF

  inherited::isActive_ = true;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);

  return true;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_WEXT,
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool highPriority_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);

  // sanity check(s)
  if (!inherited::isActive_)
    return;

  inherited::isActive_ = false;
  inherited::stop (waitForCompletion_in,
                   highPriority_in);

  int result = -1;
//  if (likely (inherited::isRegistered_))
//  {
//    ACE_Reactor* reactor_p = inherited::reactor ();
//    ACE_ASSERT (reactor_p);
//    result = reactor_p->remove_handler (this,
//                                        ACE_Event_Handler::READ_MASK);
//    if (unlikely (result == -1))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Reactor::remove_handler(0x%@,ACE_Event_Handler::READ_MASK): \"%m\", continuing\n"),
//                  this));
//    inherited::isRegistered_ = false;
//  } // end IF

  if (likely (inherited::handle_ != ACE_INVALID_HANDLE))
  {
    result = ACE_OS::close (inherited::handle_);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::close(%d): \"%m\", continuing\n"),
                  inherited::handle_));
    inherited::handle_ = ACE_INVALID_HANDLE;
  } // end IF

  if (likely (inherited::buffer_))
  {
    ACE_OS::free (inherited::buffer_);
    inherited::buffer_ = NULL;
  } // end IF
  inherited::bufferSize_ = 0;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_WEXT,
                   UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    ACE_OS::memset (&range_, 0, sizeof (struct iw_range));

    userData_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  userData_ = configuration_in.userData;

  return inherited::initialize (configuration_in);
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
int
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_WEXT,
                   UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::handle_input"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (handle_in == inherited::handle_);

  int result = -1;
  struct iwreq iwreq_s;
  ACE_OS::memset (&iwreq_s, 0, sizeof (struct iwreq));
  ACE_OS::strncpy (iwreq_s.ifr_name,
                   inherited::configuration_->interfaceIdentifier.c_str (),
                   IFNAMSIZ);
  struct stream_descr stream_descr_s;
  ACE_OS::memset (&stream_descr_s, 0, sizeof (struct stream_descr));
  struct iw_event iw_event_s;
  ACE_OS::memset (&iw_event_s, 0, sizeof (struct iw_event));
  int error = 0;
  Net_WLAN_AccessPointCacheIterator_t iterator;
  struct ether_addr ap_mac_address_s;
  ACE_OS::memset (&ap_mac_address_s, 0, sizeof (struct ether_addr));
  std::string essid_string;
  ACE_TCHAR buffer_a[BUFSIZ];
#if defined (_DEBUG)
  std::set<std::string> known_ssids, current_ssids;
#endif // _DEBUG

  if (!inherited::buffer_)
  { ACE_ASSERT (!inherited::bufferSize_);
    inherited::bufferSize_ = IW_SCAN_MAX_DATA;
    inherited::buffer_ = ACE_OS::malloc (inherited::bufferSize_);
    if (unlikely (!inherited::buffer_))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory (%u byte(s)): \"%m\", returning\n"),
                  inherited::bufferSize_));
      goto continue_;
    } // end IF
  } // end IF
  iwreq_s.u.data.pointer = inherited::buffer_;
  iwreq_s.u.data.length = inherited::bufferSize_;

fetch_scan_result_data:
  result = ACE_OS::ioctl (handle_in,
                          SIOCGIWSCAN,
                          &iwreq_s);
  if (result < 0)
  {
    error = ACE_OS::last_error ();
    if ((error == E2BIG) && // 7: buffer too small
        (range_.we_version_compiled > 16))
    {
      if (iwreq_s.u.data.length > inherited::bufferSize_)
        inherited::bufferSize_ = iwreq_s.u.data.length;
      else
        inherited::bufferSize_ *= 2; // grow dynamically
      goto retry;
    } // end IF

    if (unlikely (error != EAGAIN)) // 11: result(s) not available yet
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::ioctl(%d,SIOCGIWSCAN): \"%m\", returning\n"),
                  handle_in));
//    else
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("scan results not yet available, returning\n")));
    goto continue_;
  } // end IF

  // the driver may have reported the required buffer size
  if (unlikely (iwreq_s.u.data.length > inherited::bufferSize_))
  { // --> grow the buffer and retry
    inherited::bufferSize_ = iwreq_s.u.data.length;
retry:
    inherited::buffer_ = ACE_OS::realloc (inherited::buffer_,
                                          inherited::bufferSize_);
    if (!inherited::buffer_)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to reallocate memory (%u byte(s)): \"%m\", returning\n"),
                  inherited::bufferSize_));
      goto continue_;
    } // end IF
    iwreq_s.u.data.pointer = inherited::buffer_;
    iwreq_s.u.data.length = inherited::bufferSize_;
    goto fetch_scan_result_data;
  } // end IF
  ACE_ASSERT (iwreq_s.u.data.length && (iwreq_s.u.data.length <= inherited::bufferSize_));

  // received scan results

  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, 0);
    // clear cache
    do
    {
      iterator =
          std::find_if (inherited::SSIDCache_.begin (), inherited::SSIDCache_.end (),
                        std::bind2nd (Net_WLAN_AccessPointCacheFindPredicate (),
                                      inherited::configuration_->interfaceIdentifier));
      if (iterator == inherited::SSIDCache_.end ())
        break;

#if defined (_DEBUG)
      known_ssids.insert ((*iterator).first);
#endif // _DEBUG
      inherited::SSIDCache_.erase ((*iterator).first);
    } while (true);
  } // end lock scope

  // process the result data
  iw_init_event_stream (&stream_descr_s,
                        static_cast<char*> (inherited::buffer_),
                        static_cast<int> (iwreq_s.u.data.length));
  do
  {
    result = iw_extract_event_stream (&stream_descr_s,
                                      &iw_event_s,
                                      range_.we_version_compiled);
    if (result <= 0)
      break; // done

    switch (iw_event_s.cmd)
    {
      case SIOCGIWAP:
      {
        ACE_OS::memcpy (&ap_mac_address_s,
                        reinterpret_cast<void*> (iw_event_s.u.ap_addr.sa_data),
                        sizeof (struct ether_addr));
        break;
      }
      case SIOCGIWNWID:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        if (!iw_event_s.u.nwid.disabled)
          ACE_OS::sprintf (buffer_a,
                           ACE_TEXT_ALWAYS_CHAR ("NWID: %X"),
                           iw_event_s.u.nwid.value);
#endif // _DEBUG
        break;
      }
      case SIOCGIWFREQ:
      {
        double frequency_d = iw_freq2float (&iw_event_s.u.freq);
        int channel =iw_freq_to_channel (frequency_d, &range_);
        ACE_UNUSED_ARG (channel);
        break;
      }
      case SIOCGIWMODE:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        ACE_OS::sprintf (buffer_a,
                         ACE_TEXT_ALWAYS_CHAR ("Mode: %s"),
                         iw_operation_mode[iw_event_s.u.mode]);
#endif // _DEBUG
        break;
      }
      case SIOCGIWNAME:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        ACE_OS::sprintf (buffer_a,
                         ACE_TEXT_ALWAYS_CHAR ("Protocol: %-1.16s"),
                         iw_event_s.u.name);
#endif // _DEBUG
        break;
      }
      case SIOCGIWESSID:
      {
        ACE_ASSERT (iw_event_s.u.essid.length && (iw_event_s.u.essid.length <= IW_ESSID_MAX_SIZE));
        ACE_ASSERT (iw_event_s.u.essid.pointer);
        essid_string.assign (reinterpret_cast<char*> (iw_event_s.u.essid.pointer),
                             iw_event_s.u.essid.length);
        if (iw_event_s.u.essid.flags)
        {
          /* Does it have an ESSID index ? */
          if ((iw_event_s.u.essid.flags & IW_ENCODE_INDEX) > 1)
            ;
//            printf("                    ESSID:\"%s\" [%d]\n", essid,
//                   (iw_event_s.u.essid.flags & IW_ENCODE_INDEX));
//          else
//            printf("                    ESSID:\"%s\"\n", essid);
        } // end IF
//        else
//          printf("                    ESSID:off/any/hidden\n");
        struct Net_WLAN_AccessPointState access_point_state_s;
        access_point_state_s.linkLayerAddress = ap_mac_address_s;
        { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, 0);
          inherited::SSIDCache_.insert (std::make_pair (essid_string,
                                                        std::make_pair (inherited::configuration_->interfaceIdentifier,
                                                                        access_point_state_s)));
        } // end lock scope
#if defined (_DEBUG)
        if (known_ssids.find (essid_string) == known_ssids.end ())
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("\"%s\": detected wireless access point (MAC address: %s, ESSID: %s)...\n"),
                      ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ()),
                      ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&ap_mac_address_s),
                                                                            NET_LINKLAYER_802_11).c_str ()),
                      ACE_TEXT (essid_string.c_str ())));
#endif // _DEBUG
        break;
      }
      case SIOCGIWENCODE:
      {
        ACE_TCHAR buffer_2[IW_ENCODING_TOKEN_MAX];
        ACE_OS::memset (buffer_2, 0, sizeof (ACE_TCHAR[IW_ENCODING_TOKEN_MAX]));
        if (iw_event_s.u.data.pointer)
        { ACE_ASSERT (iw_event_s.u.data.length <= IW_ENCODING_TOKEN_MAX);
          ACE_OS::memcpy (buffer_2,
                          iw_event_s.u.data.pointer,
                          iw_event_s.u.data.length);
//        else
//          iw_event_s.u.data.flags |= IW_ENCODE_NOKEY;
        } // end IF
        if (iw_event_s.u.data.flags & IW_ENCODE_DISABLED);
//          printf ("off\n");
        else
        {
#if defined (_DEBUG)
          iw_print_key (buffer_a,
                        sizeof (ACE_TCHAR[BUFSIZ]),
                        reinterpret_cast<unsigned char*> (iw_event_s.u.data.pointer),
                        iw_event_s.u.data.length,
                        iw_event_s.u.data.flags);
#endif // _DEBUG
          /* Other info... */
          if ((iw_event_s.u.data.flags & IW_ENCODE_INDEX) > 1)
            ;
//            printf (" [%d]", iw_event_s.u.data.flags & IW_ENCODE_INDEX);
          if (iw_event_s.u.data.flags & IW_ENCODE_RESTRICTED)
            ;
//            printf ("   Security mode:restricted");
          if (iw_event_s.u.data.flags & IW_ENCODE_OPEN)
            ;
//            printf ("   Security mode:open");
        } // end ELSE
        break;
      }
      case SIOCGIWRATE:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        iw_print_bitrate (buffer_a,
                          sizeof (ACE_TCHAR[BUFSIZ]),
                          iw_event_s.u.bitrate.value);
#endif // _DEBUG
        break;
      }
      case SIOCGIWMODUL:
      {
        for (int i = 0;
             i < IW_SIZE_MODUL_LIST;
             ++i)
        {
          if ((iw_event_s.u.param.value & iw_modul_list[i].mask) == iw_modul_list[i].mask)
            ;
#if defined (_DEBUG)
          ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
          ACE_OS::sprintf (buffer_a,
                           ACE_TEXT_ALWAYS_CHAR ("Modulation: %s"),
                           iw_modul_list[i].cmd);
#endif // _DEBUG
        } // end FOR
        break;
      }
      case IWEVQUAL:
      {
#if defined (_DEBUG)
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[BUFSIZ]));
        iw_print_stats (buffer_a, sizeof (ACE_TCHAR[BUFSIZ]),
                        &iw_event_s.u.qual,
                        &range_, 1);
#endif
        break;
      }
#ifndef WE_ESSENTIAL
      case IWEVGENIE:
      {
#if defined (_DEBUG)
//        iw_print_gen_ie (iw_event_s.u.data.pointer,
//                         iw_event_s.u.data.length);
#endif // _DEBUG
        break;
      }
#endif /* WE_ESSENTIAL */
      case IWEVCUSTOM:
      {
#if defined (_DEBUG)
        ACE_TCHAR buffer_2[IW_CUSTOM_MAX + 1];
        ACE_OS::memset (buffer_2, 0, sizeof (ACE_TCHAR[IW_CUSTOM_MAX + 1]));
        ACE_ASSERT (iw_event_s.u.data.length <= IW_CUSTOM_MAX);
        if (iw_event_s.u.data.pointer && iw_event_s.u.data.length)
          ACE_OS::memcpy (buffer_2,
                          iw_event_s.u.data.pointer,
                          iw_event_s.u.data.length);
//        buffer_2[iw_event_s.u.data.length] = '\0';
#endif // _DEBUG
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown WE event (was: %d), returning\n"),
                    iw_event_s.cmd));
        goto continue_;
      }
    } // end SWITCH
  } while (true);
#if defined (_DEBUG)
  { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, 0);
    for (Net_WLAN_AccessPointCacheConstIterator_t iterator_2 = inherited::SSIDCache_.begin ();
         iterator_2 != inherited::SSIDCache_.end ();
         ++iterator_2)
      if (!ACE_OS::strcmp ((*iterator_2).second.first.c_str (),
                           inherited::configuration_->interfaceIdentifier.c_str ()))
        current_ssids.insert ((*iterator_2).first);
  } // end lock scope
  for (std::set<std::string>::const_iterator iterator_2 = known_ssids.begin ();
       iterator_2 != known_ssids.end ();
       ++iterator_2)
    if (current_ssids.find (*iterator_2) == current_ssids.end ())
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("\"%s\": lost contact to ESSID (was: %s)...\n"),
                  ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ()),
                  ACE_TEXT ((*iterator_2).c_str ())));
#endif // _DEBUG

  try {
    inherited::onScanComplete (inherited::configuration_->interfaceIdentifier);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Net_WLAN_IMonitorCB::onScanComplete(), continuing\n")));
  }

continue_:
  // *NOTE*: do not deregister from the callback
  return 0;
}
