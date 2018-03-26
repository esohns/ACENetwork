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

#include <linux/netlink.h>
#include <linux/nl80211.h>

#include "netlink/handlers.h"
#include "netlink/netlink.h"
#include "netlink/genl/ctrl.h"
#include "netlink/genl/genl.h"

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

#include "common_dbus_defines.h"
#include "common_dbus_tools.h"

#include "common_math_tools.h"

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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited ()
 , inherited2 ()
 , userData_ (NULL)
 /////////////////////////////////////////
 , buffer_ (NULL)
 , callbacks_ (NULL)
 , controlId_ (0)
 , error_ (0)
 , headerReceived_ (false)
 , inputStream_ ()
 , isRegistered_ (false)
 , isSubscribedToMulticastGroups_ (false)
 , message_ (NULL)
 , multipartDoneCBData_ ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

  inherited::TASK_T::threadCount_ = 1;

//  inherited::TASK_T::reactor (ACE_Reactor::instance ());
//  inherited2::proactor (ACE_Proactor::instance ());

  multipartDoneCBData_.error = &error_;
  multipartDoneCBData_.monitor = this;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::~Net_WLAN_Monitor_T ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::~Net_WLAN_Monitor_T"));

  int result = -1;

  if (buffer_)
    buffer_->release ();

  if (isRegistered_)
  { ACE_ASSERT (inherited::configuration_);
    switch (inherited::configuration_->dispatch)
    {
      case COMMON_EVENT_DISPATCH_PROACTOR:
      {
        result = inputStream_.cancel ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\", continuing\n")));
        break;
      }
      case COMMON_EVENT_DISPATCH_REACTOR:
      {
        ACE_Reactor* reactor_p = inherited::TASK_T::reactor ();
        ACE_ASSERT (reactor_p);
        result = reactor_p->remove_handler (this,
                                            ACE_Event_Handler::ALL_EVENTS_MASK);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::remove_handler(%@): \"%m\", continuing\n"),
                      this));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unkown dispatch type (was: %d), continuing\n"),
                    inherited::configuration_->dispatch));
        break;
      }
    } // end SWITCH
  } // end IF

  if (callbacks_)
    nl_cb_put (callbacks_);

  if (message_)
    nlmsg_free (message_);
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::start ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::start"));

  // sanity check(s)
  if (unlikely (!inherited::isInitialized_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not initialized, returning\n")));
    return;
  } // end IF
  if (unlikely (inherited::isActive_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("already started, returning\n")));
    return;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::socketHandle_);
  ACE_ASSERT (callbacks_);
//  ACE_ASSERT (!controlId_);
//  ACE_ASSERT (!error_);
//  ACE_ASSERT (!isRegistered_);

  int result = -1;
  struct nl_msg* message_p = NULL;
  Net_WLAN_nl80211_MulticastGroupIds_t multicast_group_ids_m;
  struct Net_WLAN_nl80211_MulticastGroupIdQueryCBData cb_data_s;
  cb_data_s.map = &multicast_group_ids_m;
  Net_WLAN_IMonitorBase* imonitor_base_p = this;
  int socket_handle_i = nl_socket_get_fd (inherited::socketHandle_);
  ACE_Time_Value deadline;

  if (controlId_)
    goto continue_3;

  controlId_ =
      genl_ctrl_resolve (inherited::socketHandle_,
                         ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_NL80211_CONTROL_NAME_STRING));
  if (unlikely (controlId_ < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genl_ctrl_resolve(%@,\"%s\"): \"%s\", returning\n"),
                inherited::socketHandle_,
                ACE_TEXT (NET_WLAN_MONITOR_NL80211_CONTROL_NAME_STRING),
                ACE_TEXT (nl_geterror (controlId_))));
    goto error;
  } // end IF

continue_3:
  // subscribe to all defined nl80211 multicast groups (i.e. WLAN events)
  if (isSubscribedToMulticastGroups_)
    goto continue_4;

  // step1: resolve multicast group ids
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_CONFIG),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_SCAN),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_REG),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_MLME),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_VENDOR),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_NAN),
                                                -ENOENT));
  multicast_group_ids_m.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (NL80211_MULTICAST_GROUP_TESTMODE),
                                                -ENOENT));
  message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    goto error;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,       // port #
                              NL_AUTO_SEQ,        // sequence #
                              controlId_,         // family id
                              0,                  // (user-) hdrlen
                              0,                  // flags
                              CTRL_CMD_GETFAMILY, // command id
                              0)))                // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
    nlmsg_free (message_p);
    goto error;
  } // end IF
  NLA_PUT_STRING (message_p,
                  CTRL_ATTR_FAMILY_NAME,
                  ACE_TEXT_ALWAYS_CHAR (NL80211_GENL_NAME));
  result = nl_send_auto_complete (inherited::socketHandle_, message_p);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto_complete(%@): \"%s\", returning\n"),
                inherited::socketHandle_,
                ACE_TEXT (nl_geterror (result))));
    nlmsg_free (message_p);
    goto error;
  } // end IF
  nlmsg_free (message_p);
  message_p = NULL;

  result = nl_cb_err (callbacks_,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_error_cb,
                      &error_);
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_ACK,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_ack_cb,
                      &error_);
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_FINISH,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_finish_cb,
                      &multipartDoneCBData_);
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_VALID,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_multicast_groups_cb,
                      &cb_data_s);
  ACE_ASSERT (result >= 0);
  error_ = 1;
  while (//(result > 0) &&
         error_ > 0)
    result = nl_recvmsgs (inherited::socketHandle_, callbacks_);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", returning\n"),
                inherited::socketHandle_,
                ACE_TEXT (nl_geterror ((result < 0) ? result : error_))));
    goto error;
  } // end IF

  // step2: subscribe to all available multicast groups
  for (Net_WLAN_nl80211_MulticastGroupIdsIterator_t iterator = multicast_group_ids_m.begin ();
       iterator != multicast_group_ids_m.end ();
       ++iterator)
  {
    if (unlikely ((*iterator).second == -ENOENT))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("nl80211 multicast group (was: \"%s\") not available, continuing\n"),
                  ACE_TEXT ((*iterator).first.c_str ())));
      continue;
    } // end IF
    result = nl_socket_add_memberships (inherited::socketHandle_,
                                        (*iterator).second,
                                        NFNLGRP_NONE);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_socket_add_memberships(%@,%d): \"%s\", returning\n"),
                  inherited::socketHandle_,
                  (*iterator).second,
                  ACE_TEXT (nl_geterror (result))));
      goto error;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("subscribed to nl80211 multicast group \"%s\" (id: %u)\n"),
                ACE_TEXT ((*iterator).first.c_str ()),
                static_cast<ACE_UINT32> ((*iterator).second)));
#endif
  } // end FOR

  /* disable sequence checking for multicast messages */
  result = nl_cb_set (callbacks_,
                      NL_CB_SEQ_CHECK,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_no_seq_check_cb,
                      &error_);
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_VALID,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_default_handler_cb,
                      imonitor_base_p);
  ACE_ASSERT (result >= 0);

  if (!Net_WLAN_Tools::getFeatures (inherited::configuration_->interfaceIdentifier,
                                    inherited::socketHandle_,
                                    inherited::familyId_,
                                    features_,
                                    extendedFeatures_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::getFeatures(\"%s\",%@,%d), returning\n"),
                ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ()),
                inherited::socketHandle_,
                inherited::familyId_));
    goto error;
  } // end IF

  isSubscribedToMulticastGroups_ = true;

continue_4:
  ACE_ASSERT (socket_handle_i);
  switch (inherited::configuration_->dispatch)
  {
    case COMMON_EVENT_DISPATCH_PROACTOR:
    { ACE_ASSERT (inherited::configuration_);
      inherited2::handle (socket_handle_i);

      ACE_Proactor* proactor_p =
//          inherited2::proactor ();
          ACE_Proactor::instance ();
      ACE_ASSERT (proactor_p);

      inherited2::proactor (proactor_p);

      result = inputStream_.open (*this,
                                  socket_handle_i,
                                  NULL,
                                  proactor_p);
      if (unlikely (result == -1))
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Asynch_Read_Stream::open(%d): \"%m\", returning\n"),
                    socket_handle_i));
        goto error;
      } // end IF

      if (unlikely (!initiate_read_stream (inherited::configuration_->defaultBufferSize)))
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Monitor_T::initiate_read_stream(%u), returning\n"),
                    inherited::configuration_->defaultBufferSize));
        goto error;
      } // end IF

      break;
    }
    case COMMON_EVENT_DISPATCH_REACTOR:
    {
      ACE_HANDLE handle_h = nl_socket_get_fd (inherited::socketHandle_);
      ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);
//      inherited2::set_handle ();
      inherited::TASK_T::set_handle (handle_h);

      ACE_Reactor* reactor_p =
//          inherited::TASK_T::reactor ();
          ACE_Reactor::instance ();
      ACE_ASSERT (reactor_p);

      inherited::TASK_T::reactor (reactor_p);

      result = reactor_p->register_handler (handle_h,
                                            this,
                                            ACE_Event_Handler::READ_MASK);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Reactor::register_handler(%d,0x%@,ACE_Event_Handler::READ_MASK): \"%m\", returning\n"),
                    handle_h,
                    this));
        goto error;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unkown dispatch type (was: %d), returning\n"),
                  inherited::configuration_->dispatch));
      goto error;
    }
  } // end SWITCH
  isRegistered_ = true;

  // monitor the interface in a dedicated thread
  deadline =
      (COMMON_TIME_NOW +
       ACE_Time_Value (0, COMMON_TIMEOUT_DEFAULT_THREAD_SPAWN * 1000));
  result = inherited::open (NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", returning\n")));
    goto error;
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
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", returning\n")));
    goto error;
  } // end IF

  inherited::isActive_ = true;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);

  return;

error:
nla_put_failure:
  if (isRegistered_)
  {
    switch (inherited::configuration_->dispatch)
    {
      case COMMON_EVENT_DISPATCH_PROACTOR:
      {
        result = inputStream_.cancel ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\", continuing\n")));
        break;
      }
      case COMMON_EVENT_DISPATCH_REACTOR:
      {
        ACE_HANDLE handle_h = nl_socket_get_fd (inherited::socketHandle_);
        ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);
  //      inherited2::set_handle ();
        inherited::TASK_T::set_handle (handle_h);

        ACE_Reactor* reactor_p =
  //          inherited::TASK_T::reactor ();
            ACE_Reactor::instance ();
        ACE_ASSERT (reactor_p);

        result = reactor_p->remove_handler (handle_h,
                                            ACE_Event_Handler::ALL_EVENTS_MASK);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d): \"%m\", continuing\n"),
                      handle_h));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unkown dispatch type (was: %d), continuing\n"),
                    inherited::configuration_->dispatch));
        break;
      }
    } // end SWITCH
    isRegistered_ = false;
  } // end IF
  if (message_p)
    nlmsg_free (message_p);
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool lockedAccess_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (unlikely (!inherited::isActive_))
    return;

  int result = -1;

  if (likely (isRegistered_))
  {
    switch (inherited::configuration_->dispatch)
    {
      case COMMON_EVENT_DISPATCH_PROACTOR:
      {
        result = inputStream_.cancel ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\", continuing\n")));
        break;
      }
      case COMMON_EVENT_DISPATCH_REACTOR:
      {
        ACE_HANDLE handle_h = nl_socket_get_fd (inherited::socketHandle_);
        ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);
  //      inherited2::set_handle ();
        inherited::TASK_T::set_handle (handle_h);

        ACE_Reactor* reactor_p =
  //          inherited::TASK_T::reactor ();
            ACE_Reactor::instance ();
        ACE_ASSERT (reactor_p);

        result = reactor_p->remove_handler (handle_h,
                                            ACE_Event_Handler::ALL_EVENTS_MASK);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d): \"%m\", continuing\n"),
                      handle_h));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unkown dispatch type (was: %d), continuing\n"),
                    inherited::configuration_->dispatch));
        break;
      }
    } // end SWITCH
    isRegistered_ = false;
  } // end IF

  inherited::isActive_ = false;
  inherited::stop (waitForCompletion_in,
                   lockedAccess_in);
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    if (buffer_)
    {
      buffer_->release ();
      buffer_ = NULL;
    } // end IF

    if (callbacks_)
    {
      nl_cb_put (callbacks_);
      callbacks_ = NULL;
    } // end IF

    controlId_ = 0;
    error_ = 0;

    extendedFeatures_.clear ();
    features_ = 0;

    headerReceived_ = false;

    int result = -1;
    if (isRegistered_)
    { ACE_ASSERT (inherited::configuration_);
      switch (inherited::configuration_->dispatch)
      {
        case COMMON_EVENT_DISPATCH_PROACTOR:
        {
          result = inputStream_.cancel ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Asynch_Read_Stream::cancel(): \"%m\", continuing\n")));
          break;
        }
        case COMMON_EVENT_DISPATCH_REACTOR:
        {
          ACE_HANDLE handle_h = nl_socket_get_fd (inherited::socketHandle_);
          ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);

          ACE_Reactor* reactor_p = inherited::TASK_T::reactor ();
          ACE_ASSERT (reactor_p);
          result = reactor_p->remove_handler (handle_h,
                                              ACE_Event_Handler::ALL_EVENTS_MASK);
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d): \"%m\", continuing\n"),
                        handle_h));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unkown dispatch type (was: %d), continuing\n"),
                      inherited::configuration_->dispatch));
          break;
        }
      } // end SWITCH
    } // end IF
    isRegistered_ = false;

    isSubscribedToMulticastGroups_ = false;

    if (message_)
    {
      nlmsg_free (message_);
      message_ = NULL;
    } // end IF

    userData_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  userData_ = configuration_in.userData;

  if (unlikely (!inherited::initialize (configuration_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Monitor_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  // sanity check(s)
  ACE_ASSERT (inherited::socketHandle_);
  ACE_ASSERT (!callbacks_);

  callbacks_ =
#if defined (_DEBUG)
      nl_cb_alloc (configuration_in.debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
#else
      nl_cb_alloc (NL_CB_DEFAULT);
#endif // _DEBUG
  if (unlikely (!callbacks_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nl_cb_alloc (%d): \"%m\", aborting\n"),
#if defined (_DEBUG)
                (configuration_in.debug ? NL_CB_DEBUG : NL_CB_DEFAULT)));
#else
                NL_CB_DEFAULT));
#endif // _DEBUG
    return false;
  } // end IF
  nl_socket_set_cb (inherited::socketHandle_, callbacks_);

  // sanity check(s)
#if defined (ACE_LINUX)
  Common_DBus_PolicyKit_Details_t action_details_a;
  if (Common_DBus_Tools::isUnitRunning (NULL,
                                        COMMON_SYSTEMD_UNIT_NETWORKMANAGER)                     &&
      Net_Common_Tools::isNetworkManagerManagingInterface (configuration_in.interfaceIdentifier))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("systemd unit \"%s\" is running and managing interface %s; this may interfere with the monitoring activity: please reinstall, aborting\n"),
                ACE_TEXT (COMMON_SYSTEMD_UNIT_NETWORKMANAGER),
                ACE_TEXT (configuration_in.interfaceIdentifier.c_str ())));
    return false;
  } // end IF
  if (Common_DBus_Tools::isUnitRunning (NULL,
                                        COMMON_SYSTEMD_UNIT_WPASUPPLICANT)                     &&
      Net_WLAN_Tools::isWPASupplicantManagingInterface (NULL,
                                                        configuration_in.interfaceIdentifier))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("systemd unit \"%s\" is running and managing interface %s; this may interfere with the monitoring activity: please reinstall, aborting\n"),
                ACE_TEXT (COMMON_SYSTEMD_UNIT_WPASUPPLICANT)));
    return false;
  } // end IF
#endif // ACE_LINUX

  return true;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::do_associate (const std::string& interfaceIdentifier_in,
                                                const struct ether_addr& APMACAddress_in,
                                                const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::do_associate"));

  // sanity check(s)
  ACE_ASSERT (inherited::familyId_);
  ACE_ASSERT (inherited::socketHandle_);
  if (!SSID_in.empty ())
  {
    ACE_ASSERT (!Net_Common_Tools::isAny (APMACAddress_in));
  } // end IF

  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (interfaceIdentifier_in.empty ())
    interface_identifiers_a =
        Net_WLAN_Tools::getInterfaces (inherited::socketHandle_,
                                       inherited::familyId_,
                                       AF_UNSPEC,
                                       0);
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);

  struct nl_msg* message_p = NULL;
  // *TODO*: implement propert disassociate/deauthenticate procedure
//  int command_i = (SSID_in.empty () ? NL80211_CMD_DISASSOCIATE
//                                    : NL80211_CMD_ASSOCIATE);
  int command_i = (SSID_in.empty () ? NL80211_CMD_DISCONNECT
                                    : NL80211_CMD_CONNECT);
  int result = -1;
  unsigned int if_index_i = 0;
  struct ether_addr ap_mac_address_s_s = APMACAddress_in;
  std::string ssid_string = SSID_in;
  ACE_UINT32 frequency_i = 0, authentication_type_i = 0;
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    message_p = nlmsg_alloc ();
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
      return false;
    } // end IF
    if (unlikely (!genlmsg_put (message_p,
                                NL_AUTO_PORT,              // port #
                                NL_AUTO_SEQ,               // sequence #
                                inherited::familyId_,      // family id
                                0,                         // (user-) hdrlen
//                                NLM_F_REQUEST | NLM_F_ACK, // flags
                                0,
                                command_i,                 // command id
                                0)))                       // interface version
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
      goto error;
    } // end IF
    if_index_i = ::if_nametoindex ((*iterator).c_str ());
    if (unlikely (!if_index_i))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT ((*iterator).c_str ())));
      goto error;
    } // end IF
    NLA_PUT_U32 (message_p,
                 NL80211_ATTR_IFINDEX,
                 static_cast<ACE_UINT32> (if_index_i));
//    if (command_i == NL80211_CMD_DISASSOCIATE)
    if (command_i == NL80211_CMD_DISCONNECT)
    {
      if (Net_Common_Tools::isAny (ap_mac_address_s_s))
        ap_mac_address_s_s =
            Net_WLAN_Tools::associatedBSSID (*iterator,
                                             NULL,
                                             inherited::familyId_);
//      ssid_string = Net_WLAN_Tools::associatedSSID (*iterator,
//                                                    NULL,
//                                                    inherited::familyId_);
      // *TODO*: callers specify the most appropriate reason code
      NLA_PUT_U16 (message_p,
                   NL80211_ATTR_REASON_CODE,
                   NET_WLAN_MONITOR_NL80211_REASON_CODE_LEAVING);
//      nla_put_flag (message_p,
//                    NL80211_ATTR_LOCAL_STATE_CHANGE);
    } // end IF
    ACE_ASSERT (!Net_Common_Tools::isAny (ap_mac_address_s_s));
    NLA_PUT (message_p,
             NL80211_ATTR_MAC,
             ETH_ALEN,
             ap_mac_address_s_s.ether_addr_octet);
//    if (command_i == NL80211_CMD_ASSOCIATE)
    if (command_i == NL80211_CMD_CONNECT)
    {
      NLA_PUT (message_p,
               NL80211_ATTR_SSID,
               ssid_string.size (),
               ssid_string.c_str ());
      { ACE_GUARD_RETURN (typename ACE_SYNCH_USE::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, 0);
        Net_WLAN_AccessPointCacheConstIterator_t iterator =
            inherited::SSIDCache_.begin ();
        if (likely (iterator != inherited::SSIDCache_.end ()))
        {
          frequency_i = (*iterator).second.second.frequency;
          authentication_type_i = (*iterator).second.second.authenticationType;
        } // end IF
      } // end lock scope
      NLA_PUT_U32 (message_p,
                   NL80211_ATTR_AUTH_TYPE,
                   authentication_type_i);
      // NL80211_ATTR_USE_MFP
      // *NOTE*: the next two attributes, if included, "...are restrictions on
      //         BSS selection, i.e., they effectively prevent roaming within
      //         the ESS. ..."
//      NLA_PUT (message_p,
//               NL80211_ATTR_MAC,
//               ETH_ALEN,
//               APMACAddress_in.ether_addr_octet);
//      NLA_PUT_U32 (message_p,
//                   NL80211_ATTR_WIPHY_FREQ,
//                   frequency_i);
      // NL80211_ATTR_CONTROL_PORT
      // NL80211_ATTR_CONTROL_PORT_ETHERTYPE
      // NL80211_ATTR_CONTROL_PORT_NO_ENCRYPT
      // NL80211_ATTR_MAC_HINT
      // NL80211_ATTR_WIPHY_FREQ_HINT
      // NL80211_ATTR_PREV_BSSID
    } // end IF
    NLA_PUT (message_p,
             NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
             //  and FT IEs..."
             0,
             NULL);

    result = nl_send_auto (inherited::socketHandle_, message_p);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_send_auto(0x%@): \"%s\", returning\n"),
                  inherited::socketHandle_,
                  ACE_TEXT (nl_geterror (result))));
      goto error;
    } // end IF

    // clean up
    nlmsg_free (message_p);
    message_p = NULL;
  } // end FOR

  return true;

error:
nla_put_failure:
  if (message_p)
    nlmsg_free (message_p);

  return false;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::do_scan (const std::string& interfaceIdentifier_in,
                                           const struct ether_addr& APMACAddress_in,
                                           const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::do_scan"));

  // sanity check(s)
  ACE_ASSERT (inherited::familyId_);
  ACE_ASSERT (inherited::socketHandle_);

  int result = -1;
  unsigned int if_index_i = 0;
  struct nl_msg* nested_attribute_p = NULL;
  ACE_UINT32 scan_flags_i = 0;
  struct nl_msg* message_p = nlmsg_alloc ();
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    return;
  } // end IF
  if (unlikely (!genlmsg_put (message_p,
                              NL_AUTO_PORT,              // port #
                              NL_AUTO_SEQ,               // sequence #
                              inherited::familyId_,      // family id
                              0,                         // (user-) hdrlen
//                              NLM_F_REQUEST | NLM_F_ACK, // flags
                              0,
                              NL80211_CMD_TRIGGER_SCAN,  // command id
                              0)))                       // interface version
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", returning\n")));
    goto error;
  } // end IF
  if_index_i = ::if_nametoindex (interfaceIdentifier_in.c_str ());
  if (unlikely (!if_index_i))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to if_nametoindex(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto error;
  } // end IF
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_IFINDEX,
               static_cast<ACE_UINT32> (if_index_i));
  // *NOTE*: "...NL80211_ATTR_BSSID can be used to specify a BSSID to scan for;
  //         if not included, the wildcard BSSID will be used. ..."
  if (unlikely (!Net_Common_Tools::isAny (APMACAddress_in)))
    NLA_PUT (message_p,
             NL80211_ATTR_BSSID,
             ETH_ALEN,
             APMACAddress_in.ether_addr_octet);
  // *NOTE: "...If no SSID is passed, no probe requests are sent and a passive
  //         scan is performed. ..."
  nested_attribute_p = nlmsg_alloc ();
  if (unlikely (!nested_attribute_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", returning\n")));
    goto error;
  } // end IF
  NLA_PUT (nested_attribute_p,
           1,
           (SSID_in.empty () ? 0 : SSID_in.size ()),
           (SSID_in.empty () ? ACE_TEXT_ALWAYS_CHAR ("") : SSID_in.c_str ()));
  nla_put_nested (message_p,
                  NL80211_ATTR_SCAN_SSIDS,
                  nested_attribute_p);
  nlmsg_free (nested_attribute_p);
  nested_attribute_p = NULL;
//  NLA_PUT (message_p,
//           NL80211_ATTR_SSID,
//           SSID_in.size (),
//           SSID_in.c_str ());
  if (inherited::configuration_->lowPriorityScans &&
      hasFeature (NL80211_FEATURE_LOW_PRIORITY_SCAN,
                  MAX_NL80211_EXT_FEATURES))
    scan_flags_i |= NL80211_SCAN_FLAG_LOW_PRIORITY;
  if (inherited::configuration_->flushCacheBeforeScans &&
      hasFeature (NL80211_FEATURE_SCAN_FLUSH,
                  MAX_NL80211_EXT_FEATURES))
    scan_flags_i |= NL80211_SCAN_FLAG_FLUSH;
//  NL80211_SCAN_FLAG_AP
//  // *NOTE*: check features first; set NL80211_ATTR_MAC[, NL80211_ATTR_MAC_MASK]
  if (inherited::configuration_->randomizeMACAddressForScans &&
      (hasFeature (NL80211_FEATURE_SCAN_RANDOM_MAC_ADDR,
                   MAX_NL80211_EXT_FEATURES) ||
       hasFeature (NL80211_FEATURE_ND_RANDOM_MAC_ADDR,
                   MAX_NL80211_EXT_FEATURES)))
    scan_flags_i |= NL80211_SCAN_FLAG_RANDOM_ADDR;
  // *TODO*: find out what these are
// NL80211_SCAN_FLAG_FILS_MAX_CHANNEL_TIME
// NL80211_SCAN_FLAG_ACCEPT_BCAST_PROBE_RESP;
// NL80211_SCAN_FLAG_OCE_PROBE_REQ_HIGH_TX_RATE;
// NL80211_SCAN_FLAG_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION;
  NLA_PUT_U32 (message_p,
               NL80211_ATTR_SCAN_FLAGS,
               scan_flags_i);
  if (scan_flags_i & NL80211_SCAN_FLAG_RANDOM_ADDR)
  {
    // *NOTE*: "...the NL80211_ATTR_MAC and NL80211_ATTR_MAC_MASK attributes may
    //         also be given in which case only the masked bits will be
    //         preserved from the MAC address and the remainder randomised. If
    //         the attributes are not given full randomisation (46 bits, locally
    //         administered 1, multicast 0) is assumed. ..."
//    struct ether_addr interface_mac_address_s;
//    ACE_OS::memset (&interface_mac_address_s, 0, sizeof (struct ether_addr));
//    NLA_PUT (message_p,
//             NL80211_ATTR_MAC,
//             ETH_ALEN,
//             interface_mac_address_s.ether_addr_octet);
//    NLA_PUT (message_p,
//             NL80211_ATTR_MAC_MASK,
//             ETH_ALEN,
//             interface_mac_address_s.ether_addr_octet);
  } // end IF
  // *NOTE*: "...if passed, define which channels should be scanned; if not
  //          passed, all channels allowed for the current regulatory domain are
  //          used. ..."
//  NLA_PUT_U32 (message_p,
//               NL80211_ATTR_SCAN_FREQUENCIES,
//               authentication_type_i);
//  // *NOTE*: do not send probe requests at 'Complementary Code Keying' rate
//  nla_put_flag (message_p,
//                NL80211_ATTR_TX_NO_CCK_RATE);
//  NLA_PUT_U32 (message_p,
//               NL80211_ATTR_MEASUREMENT_DURATION,
//               0);
// NL80211_ATTR_MEASUREMENT_DURATION_MANDATORY
//  NLA_PUT_FLAG (message_p,
//                NL80211_ATTR_SCHED_SCAN_MULTI);
  NLA_PUT (message_p,
           NL80211_ATTR_IE, // "...VendorSpecificInfo, but also including RSN IE
                            //  and FT IEs..."
           0,
           NULL);

  multipartDoneCBData_.scanning = true;

  result = nl_send_auto (inherited::socketHandle_, message_p);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto(0x%@): \"%s\", returning\n"),
                inherited::socketHandle_,
                ACE_TEXT (nl_geterror (result))));
    goto error;
  } // end IF

  // clean up
  nlmsg_free (message_p);
  message_p = NULL;

  return;

error:
nla_put_failure:
  if (nested_attribute_p)
    nlmsg_free (nested_attribute_p);
  if (message_p)
    nlmsg_free (message_p);
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::handle_input"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::socketHandle_);
  ACE_ASSERT (handle_in == nl_socket_get_fd (inherited::socketHandle_));

  int result = -1;
  ssize_t bytes_received = -1;
  ACE_Message_Block* message_block_p =
      allocateMessage (inherited::configuration_->defaultBufferSize);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocateMessage(%u), aborting\n"),
                inherited::configuration_->defaultBufferSize));
    return -1; // <-- remove 'this' from dispatch
  } // end IF
  struct sockaddr_nl address_s;
  ACE_OS::memset (&address_s, 0, sizeof (struct sockaddr_nl));
  int address_length_i = sizeof (struct sockaddr_nl);

  // read some data from the socket
retry:
  bytes_received =
    ACE_OS::recvfrom (handle_in,                                       // socket handle
                      message_block_p->wr_ptr (),                      // buffer
                      message_block_p->capacity (),                    // #bytes to read
                      0,                                               // flags
                      reinterpret_cast<struct sockaddr*> (&address_s), // address
                      &address_length_i);                              // address size
  switch (bytes_received)
  {
    case -1:
    {
      // *IMPORTANT NOTE*: a number of issues can occur here:
      // - connection reset by peer
      // - connection abort()ed locally
      int error = ACE_OS::last_error ();
      if (error == EWOULDBLOCK)      // 11: SSL_read() failed (buffer is empty)
        goto retry;
      if ((error != EPIPE)        && // 32: connection reset by peer (write)
          // -------------------------------------------------------------------
          (error != EBADF)        && // 9
          (error != ENOTSOCK)     && // 88
          (error != ECONNABORTED) && // 103: connection abort()ed locally
          (error != ECONNRESET))     // 104: connection reset by peer (read)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::recvfrom(%d): \"%m\", aborting\n"),
                    handle_in));

      // clean up
      message_block_p->release ();

      return -1; // <-- remove 'this' from dispatch
    }
    // *** GOOD CASES ***
    case 0:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%d: socket was closed by the peer\n"),
                  handle_in));

      // clean up
      message_block_p->release ();

      return -1; // <-- remove 'this' from dispatch
    }
    default:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("[%d]: received %d bytes\n"),
//                  handle_in,
//                  bytes_received));

      // adjust write pointer
      message_block_p->wr_ptr (static_cast<size_t> (bytes_received));

      break;
    }
  } // end SWITCH

  // *NOTE*: fire-and-forget message_block_p
  processMessage (address_s,
                  message_block_p);

  // *NOTE*: do not deregister from the reactor
  return 0;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::handle_read_stream (const ACE_Asynch_Read_Stream::Result& result_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::handle_read_stream"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ACE_Message_Block* message_block_p = NULL;
  struct sockaddr_nl address_s;
  ACE_OS::memset (&address_s, 0, sizeof (struct sockaddr_nl));

  // *IMPORTANT NOTE*: there is currently no way to retrieve the source address
  //                   when using the proactor implementation based on aio(7)
  // *TODO*: implement ACE_Asynch_Read_Dgram/Result specializations that support
  //         recvfrom(2) semantics

  if (unlikely (!result_in.success ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::read(%d,%u): \"%s\", returning\n"),
                result_in.handle (),
                result_in.bytes_to_read (),
                ACE_TEXT (ACE_OS::strerror (result_in.error ()))));
    return;
  } // end IF

  // make a shallow copy of the message buffer
  message_block_p = result_in.message_block ().duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", returning\n")));
    return;
  } // end IF

  // *NOTE*: fire-and-forget message_block_p
  processMessage (address_s,
                  message_block_p);

  if (unlikely (!initiate_read_stream (inherited::configuration_->defaultBufferSize)))
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Monitor_T::initiate_read_stream(%u), returning\n"),
                inherited::configuration_->defaultBufferSize));
    return;
  } // end IF
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::svc (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::svc"));

  int result = -1;
  int error = 0;

  { ACE_GUARD_RETURN (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_, -1);
    if (!inherited::dispatchStarted_)
    {
      inherited::dispatchStarted_ = true;
      goto state_machine;
    } // end IF
  } // end lock scope

//netlink_dispatch:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%s): netlink dispatch (thread id: %t, group: %d) starting\n"),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  // sanity check(s)
  ACE_ASSERT (inherited::socketHandle_);
  ACE_ASSERT (callbacks_);

  do
  { // *IMPORTANT NOTE*: there is no known way to cleanly unblock from a socket
    //                   read. Possible solutions:
    //                   - close()/shutdown() do not work
    //                   - SIGINT might work, ugly
    //                   - flag-setting and sending a 'dummy request', ugly
    //                   --> use ACE event dispatch and circumvent nl_revmsgs()
    result = nl_recvmsgs (inherited::socketHandle_, callbacks_);
    if (unlikely (result < 0))
    {
      if (-result == NLE_BAD_SOCK)  // 3: shutdown (see below))
        break;
      else if (-result != NLE_BUSY) // 25
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\" (%d), returning\n"),
                    inherited::socketHandle_,
                    ACE_TEXT (nl_geterror (result)), result));
        break;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", continuing\n"),
                  inherited::socketHandle_,
                  ACE_TEXT (nl_geterror (result))));
    } // end IF
    error = ACE_OS::last_error ();
    if (unlikely (error &&
                  (error != EAGAIN))) // 11: socket is non-blocking, no data
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_recvmsgs(%@): \"%m\", returning\n"),
                  inherited::socketHandle_));
      break;
    } // end IF
  } while (true);

  { ACE_GUARD_RETURN (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_, -1);
    inherited::dispatchStarted_ = false;
  } // end lock scope

  return result;

state_machine:
  result = inherited::svc ();

  // *NOTE*: the base class is not aware of the fact that not all task threads
  //         are dispatching the message queue
  //         --> shutdown manually:
  //         - reset the socket
  //         - flush the MB_STOP message
  // *NOTE*: nl_close calls close(), which, on Linux systems, does not (!)
  //         signal the thread blocked in recvmsg() (see above)
  // *NOTE*: apparently, shutdown(2) does not work on netlink sockets; it
  //         returns ENOTSUP
  //         --> reconnect using a fresh socket
//  int result_2 = ACE_OS::shutdown (nl_socket_get_fd (inherited::handle_),
//                                   SHUT_RDWR);
//  if (unlikely (result_2 == -1))
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_OS::shutdown(%d,%d): \"%m\", continuing\n"),
//                nl_socket_get_fd (inherited::handle_), SHUT_RDWR));
//  nl_close (inherited::handle_);
// nl_socket_disable_msg_peek (inherited::handle_);
//  nl_socket_set_buffer_size (inherited::handle_, int rx, int tx);
//  nl_socket_disable_auto_ack (inherited::handle_);

//  int result_2 = nl_connect (inherited::handle_, NETLINK_GENERIC);
//  if (unlikely (result_2 < 0))
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to nl_connect(%@): \"%s\", continuing\n"),
//                inherited::handle_,
//                ACE_TEXT (nl_geterror (result_2))));

  int result_2 = inherited::flush (ACE_Task_Flags::ACE_FLUSHALL);
//  if (unlikely (result_2 != 1))
  if (unlikely (result_2 != 0))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task_T::flush(): \"%m\", continuing\n")));
  result_2 = inherited::msg_queue_->activate ();
  if (unlikely (result_2 == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_MessageQueue_T::activate(): \"%m\", continuing\n")));
  inherited::dispatchStarted_ = false;

  return result;
}

template <typename AddressType,
          typename ConfigurationType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename UserDataType>
ACE_Message_Block*
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::allocateMessage (unsigned int bufferSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::allocateMessage"));

  // initialize return value(s)
  ACE_Message_Block* return_value = NULL;

  ACE_NEW_NORETURN (return_value,
                    ACE_Message_Block (bufferSize_in,                      // size
                                       ACE_Message_Block::MB_DATA,         // type
                                       NULL,                               // cont
                                       NULL,                               // data
                                       NULL,                               // allocator_strategy
                                       NULL,                               // locking_strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution_time
                                       ACE_Time_Value::max_time,           // deadline_time
                                       NULL,                               // data_block_allocator
                                       NULL));                             // message_block_allocator
  if (unlikely (!return_value))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory (was: %u byte(s)): \"%m\", aborting\n"),
                bufferSize_in));
    return NULL;
  } // end IF

  return return_value;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::processMessage (const struct sockaddr_nl& sourceAddress_in,
                                                  ACE_Message_Block*& messageBlock_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::processMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::socketHandle_);
  ACE_ASSERT (messageBlock_inout);

  ACE_Message_Block* message_block_p, *message_block_2;
  unsigned int buffer_size;
  int err = 0;
//  int multipart = 0, interrupted = 0, nrecv = 0;
  struct nlmsghdr* nlmsghdr_p = NULL;
  //  struct ucred* ucred_p = NULL;
  unsigned int missing_bytes, bytes_to_copy, offset;

  // form a chain of buffers
  message_block_2 = buffer_;
  if (message_block_2)
  {
    while (message_block_2->cont ())
      message_block_2 = message_block_2->cont ();
    message_block_2->cont (messageBlock_inout);
  } // end IF
  else
    buffer_ = messageBlock_inout;
  ACE_ASSERT (buffer_);
  messageBlock_inout = NULL;

  // dissect nl80211 messages
next:
  // sanity check(s)
  buffer_size = buffer_->total_length ();
  if (buffer_size < NLMSG_HDRLEN)
    return; // need more data

  if (!message_)
  {
    message_ = nlmsg_alloc ();
    if (unlikely (!message_))
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to nlmsg_alloc(): \"%m\", returning\n")));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (message_);
  nlmsghdr_p =  nlmsg_hdr (message_);
  ACE_ASSERT (nlmsghdr_p);

  // step1: assemble the message header ?
  if (headerReceived_)
    goto continue_;

  message_block_p = buffer_;
  missing_bytes = NLMSG_HDRLEN;
  bytes_to_copy = 0;
  offset = 0;
  do
  {
    bytes_to_copy = message_block_p->length ();
    if (likely (bytes_to_copy > missing_bytes))
      bytes_to_copy = missing_bytes;

    ACE_OS::memcpy (nlmsghdr_p + offset,
                    message_block_p->rd_ptr (),
                    bytes_to_copy);
    offset += bytes_to_copy;

    message_block_p->rd_ptr (bytes_to_copy);
    if (!message_block_p->length ())
      message_block_p = message_block_p->cont ();
    ACE_ASSERT (message_block_p);

    missing_bytes -= bytes_to_copy;
  } while (missing_bytes);
  buffer_size -= NLMSG_HDRLEN;

  headerReceived_ = true;

  // step2: assemble the message body
continue_:
  // sanity check(s)
  ACE_ASSERT (nlmsghdr_p);
  missing_bytes = nlmsg_datalen (nlmsghdr_p);
  if (missing_bytes > buffer_size)
    return; // need more data

  message_block_p = buffer_;
  while (!message_block_p->length ())
    message_block_p = message_block_p->cont ();
  bytes_to_copy = 0;
  offset = 0;
  do
  {
    bytes_to_copy = message_block_p->length ();
    if (likely (bytes_to_copy > missing_bytes))
      bytes_to_copy = missing_bytes;

    ACE_OS::memcpy (static_cast<char*> (nlmsg_data (nlmsghdr_p)) + offset,
                    message_block_p->rd_ptr (),
                    bytes_to_copy);
    offset += bytes_to_copy;

    message_block_p->rd_ptr (bytes_to_copy);
    if (!message_block_p->length ())
      message_block_p = message_block_p->cont ();

    missing_bytes -= bytes_to_copy;
  } while (missing_bytes);
  buffer_size -= nlmsg_datalen (nlmsghdr_p);

  // unlink fully processed head message buffer(s)
  if (message_block_p &&
      (message_block_p != buffer_))
  { // --> there is trailing data
    ACE_ASSERT (buffer_size);
    message_block_2 = buffer_;
    while (message_block_2->cont () != message_block_p)
      message_block_2 = message_block_2->cont ();
    ACE_ASSERT (message_block_2->cont ());
    message_block_2->cont (NULL);

    buffer_->release ();
    buffer_ = message_block_p;
  } // end IF
  else if (!message_block_p)
  { // --> there is no more data
    ACE_ASSERT (!buffer_size);
    buffer_->release ();
    buffer_ = NULL;
  } // end ELSE IF

  headerReceived_ = false;

  // step3: dispatch message
  // *NOTE*: this code is adapted from libnl nl.c
#define NL_CB_CALL(cb,type,msg) \
do { \
  err = nl_cb_call (cb, type, msg); \
  switch (err) { \
    case NL_OK: \
    case NL_STOP: \
        break; \
    case NL_SKIP: \
        goto continue_2; \
    default: \
        goto error; \
  } \
} while (0)

  nlmsg_set_proto (message_,
                   inherited::socketHandle_->s_proto);
  nlmsg_set_src (message_,
                 &const_cast<struct sockaddr_nl&> (sourceAddress_in));
//    if (ucred_p)
//      nlmsg_set_creds (message_, ucred_p);

#if defined (_DEBUG)
  if (inherited::configuration_->debug)
    nl_msg_dump (message_, stderr);
#endif // _DEBUG

//  nrecv++;

  /* Raw callback is the first, it gives the most control
     * to the user and he can do his very own parsing. */
  if (callbacks_->cb_set[NL_CB_MSG_IN])
    NL_CB_CALL (callbacks_, NL_CB_MSG_IN, message_);

  /* Sequence number checking. The check may be done by
     * the user, otherwise a very simple check is applied
     * enforcing strict ordering */
  if (callbacks_->cb_set[NL_CB_SEQ_CHECK]) {
    NL_CB_CALL (callbacks_, NL_CB_SEQ_CHECK, message_);

    /* Only do sequence checking if auto-ack mode is enabled */
  } else if (!(inherited::socketHandle_->s_flags & NL_NO_AUTO_ACK)) {
    if (nlmsghdr_p->nlmsg_seq != inherited::socketHandle_->s_seq_expect) {
      if (callbacks_->cb_set[NL_CB_INVALID])
        NL_CB_CALL (callbacks_, NL_CB_INVALID, message_);
      else {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid message sequence # (was: %d, expected: %d), returning\n"),
                    nlmsghdr_p->nlmsg_seq,
                    inherited::socketHandle_->s_seq_expect));
        err = -NLE_SEQ_MISMATCH;
        goto error;
      } // end ELSE
    } // end IF
  }

  if (nlmsghdr_p->nlmsg_type == NLMSG_DONE     ||
      nlmsghdr_p->nlmsg_type == NLMSG_ERROR    ||
      nlmsghdr_p->nlmsg_type == NLMSG_NOOP     ||
      nlmsghdr_p->nlmsg_type == NLMSG_OVERRUN) {
    /* We can't check for !NLM_F_MULTI since some netlink
       * users in the kernel are broken. */
    inherited::socketHandle_->s_seq_expect++;
  } // end IF

//  if (nlmsghdr_p->nlmsg_flags & NLM_F_MULTI)
//    multipart = 1;

  if (nlmsghdr_p->nlmsg_flags & NLM_F_DUMP_INTR) {
    if (callbacks_->cb_set[NL_CB_DUMP_INTR])
      NL_CB_CALL (callbacks_, NL_CB_DUMP_INTR, message_);
    else {
      /*
         * We have to continue reading to clear
         * all messages until a NLMSG_DONE is
         * received and report the inconsistency.
         */
//      interrupted = 1;
    } // end ELSE
  } // end IF

  /* Other side wishes to see an ack for this message */
  if (nlmsghdr_p->nlmsg_flags & NLM_F_ACK) {
    if (callbacks_->cb_set[NL_CB_SEND_ACK])
      NL_CB_CALL (callbacks_, NL_CB_SEND_ACK, message_);
    else {
      /* FIXME: implement */
    }
  } // end IF

  /* messages terminates a multipart message, this is
     * usually the end of a message and therefore we slip
     * out of the loop by default. the user may overrule
     * this action by skipping this packet. */
  if (nlmsghdr_p->nlmsg_type == NLMSG_DONE) {
//    multipart = 0;
    if (callbacks_->cb_set[NL_CB_FINISH])
      NL_CB_CALL (callbacks_, NL_CB_FINISH, message_);
  } // end IF

  /* Message to be ignored, the default action is to
     * skip this message if no callback is specified. The
     * user may overrule this action by returning
     * NL_PROCEED. */
  else if (nlmsghdr_p->nlmsg_type == NLMSG_NOOP) {
    if (callbacks_->cb_set[NL_CB_SKIPPED])
      NL_CB_CALL (callbacks_, NL_CB_SKIPPED, message_);
    else
      goto continue_2;
  }

  /* Data got lost, report back to user. The default action is to
     * quit parsing. The user may overrule this action by retuning
     * NL_SKIP or NL_PROCEED (dangerous) */
  else if (nlmsghdr_p->nlmsg_type == NLMSG_OVERRUN) {
    if (callbacks_->cb_set[NL_CB_OVERRUN])
      NL_CB_CALL (callbacks_, NL_CB_OVERRUN, message_);
    else {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("buffer overrun, returning\n")));
      err = -NLE_MSG_OVERFLOW;
      goto error;
    } // end ELSE
  }

  /* Message carries a nlmsgerr */
  else if (nlmsghdr_p->nlmsg_type == NLMSG_ERROR) {
    struct nlmsgerr* e =
        reinterpret_cast<struct nlmsgerr*> (nlmsg_data (nlmsghdr_p));

    if (nlmsghdr_p->nlmsg_len < static_cast<__u32> (nlmsg_size (sizeof (*e)))) {
      /* Truncated error message, the default action
         * is to stop parsing. The user may overrule
         * this action by returning NL_SKIP or
         * NL_PROCEED (dangerous) */
      if (callbacks_->cb_set[NL_CB_INVALID])
        NL_CB_CALL (callbacks_, NL_CB_INVALID, message_);
      else {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("truncated message, returning\n")));
        err = -NLE_MSG_TRUNC;
        goto error;
      } // end ELSE
    } else if (e->error) {
      /* Error message reported back from kernel. */
      if (callbacks_->cb_err) {
        err =
            callbacks_->cb_err (&const_cast<struct sockaddr_nl&> (sourceAddress_in),
                                e,
                                callbacks_->cb_err_arg);
        if (err < 0)
          goto error;
        else if (err == NL_SKIP)
          goto continue_2;
        else if (err == NL_STOP) {
          err = -nl_syserr2nlerr (e->error);
          goto error;
        }
      } else {
        err = -nl_syserr2nlerr (e->error);
        goto error;
      } // end ELSE
    } else if (callbacks_->cb_set[NL_CB_ACK])
      NL_CB_CALL (callbacks_, NL_CB_ACK, message_);
  } else {
    /* Valid message (not checking for MULTIPART bit to
       * get along with broken kernels. NL_SKIP has no
       * effect on this.  */
    if (callbacks_->cb_set[NL_CB_VALID])
      NL_CB_CALL (callbacks_, NL_CB_VALID, message_);
  } // end ELSE

continue_2:
  err = 0;
  nlmsg_free (message_);
  message_ = NULL;

  if (buffer_)
    goto next;

  return;

error:
  if (message_)
  {
    nlmsg_free (message_);
    message_ = NULL;
  } // end IF
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::initiate_read_stream (unsigned int bufferSize_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initiate_read_stream"));

  // step1: allocate a message buffer
  ACE_Message_Block* message_block_p = allocateMessage (bufferSize_in);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate message (was: %u byte(s)): \"%m\", aborting\n"),
                bufferSize_in));
    return false;
  } // end IF

  // step2: start (asynchronous) read
//  size_t bytes_to_read = message_block_p->size ();
  int result =
      inputStream_.read (*message_block_p,                     // buffer
                         message_block_p->size (),             // buffer size
                         NULL,                                 // ACT
                         0,                                    // priority
                         COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
//      inputStream_.recv (message_block_p,                      // buffer
//                         message_block_p->size (),             // buffer size
//                         0,                                    // flags
//                         ACE_PROTOCOL_FAMILY_NETLINK,          // protocol family
//                         NULL,                                 // ACT
//                         0,                                    // priority
//                         COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL); // signal
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::read(%u): \"%m\", aborting\n"),
//                ACE_TEXT ("failed to ACE_Asynch_Read_Stream::recv(%u): \"%m\", aborting\n"),
                bufferSize_in));

    // clean up
    message_block_p->release ();

    return false;
  } // end IF

  return true;
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
                   NET_WLAN_MONITOR_API_NL80211,
                   UserDataType>::hasFeature (enum nl80211_feature_flags feature_in,
                                              enum nl80211_ext_feature_index extendedFeature_in) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::hasFeature"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (feature_in || (extendedFeature_in < MAX_NL80211_EXT_FEATURES));

  if (likely (feature_in))
    return (features_ & static_cast<ACE_UINT32> (feature_in));

  return (extendedFeatures_.find (extendedFeature_in) != extendedFeatures_.end ());
}
