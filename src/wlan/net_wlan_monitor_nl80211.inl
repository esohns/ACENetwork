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

#include "linux/netlink.h"
#include "linux/nl80211.h"

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

#if defined (ACE_LINUX) && defined (DBUS_NM_SUPPORT) && defined (SD_BUS_SUPPORT)
#include "common_dbus_defines.h"
#include "common_dbus_tools.h"

#include "net_os_tools.h"
#endif // ACE_LINUX && DBUS_NM_SUPPORT && SD_BUS_SUPPORT

#include "common_math_tools.h"

#if defined (_DEBUG)
#include "common_timer_tools.h"
#endif // _DEBUG

#include "stream_defines.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"
#include "net_packet_headers.h"

#include "net_wlan_common.h"
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
 , features_ ()
 , headerReceived_ (false)
 , inputStream_ ()
 , isRegistered_ (false)
 , isSubscribedToMulticastGroups_ (false)
 , message_ (NULL)
 , protocolFeatures_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

  inherited::TASK_T::threadCount_ = 2;

//  inherited::TASK_T::reactor (ACE_Reactor::instance ());
//  inherited2::proactor (ACE_Proactor::instance ());

  inherited::nl80211CBData_.error = &error_;
  inherited::nl80211CBData_.features = &features_;
  inherited::nl80211CBData_.monitor = this;
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
bool
Net_WLAN_Monitor_T<AddressType,
                   ConfigurationType,
                   ACE_SYNCH_USE,
                   TimePolicyType,
                   NET_WLAN_MONITOR_API_NL80211,
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
                ACE_TEXT ("already started, returning\n")));
    return true;
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
  inherited::nl80211CBData_.map = &multicast_group_ids_m;
  ACE_HANDLE socket_handle_h = nl_socket_get_fd (inherited::socketHandle_);
  ACE_Time_Value deadline;

  if (controlId_)
    goto continue_;

  controlId_ =
      genl_ctrl_resolve (inherited::socketHandle_,
                         ACE_TEXT_ALWAYS_CHAR (NET_WLAN_MONITOR_NL80211_CONTROL_NAME_STRING));
  if (unlikely (controlId_ < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to genl_ctrl_resolve(%@,\"%s\"): \"%s\", aborting\n"),
                inherited::socketHandle_,
                ACE_TEXT (NET_WLAN_MONITOR_NL80211_CONTROL_NAME_STRING),
                ACE_TEXT (nl_geterror (controlId_))));
    goto error;
  } // end IF

continue_:
  if (unlikely (!Net_WLAN_Tools::getWiPhyFeatures (inherited::configuration_->interfaceIdentifier,
                                                   inherited::configuration_->wiPhyIdentifier,
                                                   inherited::socketHandle_,
                                                   inherited::familyId_,
                                                   features_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::getWiPhyFeatures(\"%s\",\"%s\",%@,%d), aborting\n"),
                ACE_TEXT (inherited::configuration_->interfaceIdentifier.c_str ()),
                ACE_TEXT (inherited::configuration_->wiPhyIdentifier.c_str ()),
                inherited::socketHandle_,
                inherited::familyId_));
    goto error;
  } // end IF

//continue_2:
  // subscribe to all defined nl80211 multicast groups (i.e. WLAN events)
  if (isSubscribedToMulticastGroups_)
    goto continue_3;

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
                ACE_TEXT ("failed to nlmsg_alloc (): \"%m\", aborting\n")));
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
                ACE_TEXT ("failed to genlmsg_put (): \"%m\", aborting\n")));
    goto error;
  } // end IF
  NLA_PUT_STRING (message_p,
                  CTRL_ATTR_FAMILY_NAME,
                  ACE_TEXT_ALWAYS_CHAR (NL80211_GENL_NAME));
  result = nl_send_auto (inherited::socketHandle_, message_p);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_send_auto(%@): \"%s\", aborting\n"),
                inherited::socketHandle_,
                ACE_TEXT (nl_geterror (result))));
    goto error;
  } // end IF
  nlmsg_free (message_p);
  message_p = NULL;

  result = nl_cb_err (callbacks_,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_error_cb,
                      &(inherited::nl80211CBData_));
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_ACK,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_ack_cb,
                      &(inherited::nl80211CBData_));
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_FINISH,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_finish_cb,
                      &(inherited::nl80211CBData_));
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_VALID,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_multicast_groups_cb,
                      &(inherited::nl80211CBData_));
  ACE_ASSERT (result >= 0);
  result = 1;
  while ((result > 0) &&
         !error_)
    result = nl_recvmsgs (inherited::socketHandle_, callbacks_);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", aborting\n"),
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
                  ACE_TEXT ("failed to nl_socket_add_memberships(%@,%d): \"%s\", aborting\n"),
                  inherited::socketHandle_,
                  (*iterator).second,
                  ACE_TEXT (nl_geterror (result))));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("subscribed to nl80211 multicast group \"%s\" (id: %u)\n"),
                ACE_TEXT ((*iterator).first.c_str ()),
                static_cast<ACE_UINT32> ((*iterator).second)));
  } // end FOR

  /* disable sequence checking for multicast messages */
  result = nl_cb_set (callbacks_,
                      NL_CB_SEQ_CHECK,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_no_seq_check_cb,
                      &(inherited::nl80211CBData_));
  ACE_ASSERT (result >= 0);
  result = nl_cb_set (callbacks_,
                      NL_CB_VALID,
                      NL_CB_CUSTOM,
                      network_wlan_nl80211_default_handler_cb,
                      &(inherited::nl80211CBData_));
  ACE_ASSERT (result >= 0);

  isSubscribedToMulticastGroups_ = true;

continue_3:
  ACE_ASSERT (socket_handle_h != ACE_INVALID_HANDLE);
  switch (inherited::configuration_->dispatch)
  {
    case COMMON_EVENT_DISPATCH_PROACTOR:
    { ACE_ASSERT (inherited::configuration_);
      inherited2::handle (socket_handle_h);

      ACE_Proactor* proactor_p =
//          inherited2::proactor ();
          ACE_Proactor::instance ();
      ACE_ASSERT (proactor_p);

      inherited2::proactor (proactor_p);

      result = inputStream_.open (*this,
                                  socket_handle_h,
                                  NULL,
                                  proactor_p);
      if (unlikely (result == -1))
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Asynch_Read_Stream::open(%d): \"%m\", aborting\n"),
                    socket_handle_h));
        goto error;
      } // end IF

      if (unlikely (!initiate_read_stream (inherited::configuration_->defaultBufferSize)))
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Monitor_T::initiate_read_stream(%u), aborting\n"),
                    inherited::configuration_->defaultBufferSize));
        goto error;
      } // end IF

      break;
    }
    case COMMON_EVENT_DISPATCH_REACTOR:
    {
//      inherited2::set_handle ();
      inherited::TASK_T::set_handle (socket_handle_h);

      ACE_Reactor* reactor_p =
//          inherited::TASK_T::reactor ();
          ACE_Reactor::instance ();
      ACE_ASSERT (reactor_p);

      inherited::TASK_T::reactor (reactor_p);

      result = reactor_p->register_handler (socket_handle_h,
                                            this,
                                            ACE_Event_Handler::READ_MASK);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Reactor::register_handler(%d,0x%@,ACE_Event_Handler::READ_MASK): \"%m\", aborting\n"),
                    socket_handle_h,
                    this));
        goto error;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unkown dispatch type (was: %d), aborting\n"),
                  inherited::configuration_->dispatch));
      goto error;
    }
  } // end SWITCH
  isRegistered_ = true;

  // monitor the interface in a dedicated thread
  result = nl_socket_set_nonblocking (inherited::socketHandle_);
  ACE_ASSERT (result == 0);

  deadline =
      (COMMON_TIME_NOW +
       ACE_Time_Value (0, COMMON_TIMEOUT_DEFAULT_THREAD_SPAWN * 1000));
  result = inherited::open (NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", aborting\n")));
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
                ACE_TEXT ("failed to Common_TaskBase_T::open(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  inherited::isActive_ = true;

  inherited::change (NET_WLAN_MONITOR_STATE_INITIALIZED);
  inherited::change (NET_WLAN_MONITOR_STATE_IDLE);

  return true;

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
      { ACE_ASSERT (socket_handle_h != ACE_INVALID_HANDLE);
  //      inherited2::set_handle ();
        inherited::TASK_T::set_handle (socket_handle_h);

        ACE_Reactor* reactor_p =
  //          inherited::TASK_T::reactor ();
            ACE_Reactor::instance ();
        ACE_ASSERT (reactor_p);

        result = reactor_p->remove_handler (socket_handle_h,
                                            ACE_Event_Handler::ALL_EVENTS_MASK);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Reactor::remove_handler(%d): \"%m\", continuing\n"),
                      socket_handle_h));
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
                   UserDataType>::stop (bool waitForCompletion_in,
                                        bool highPriority_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  // sanity check(s)
  if (unlikely (!inherited::isActive_))
    return;

  int result = -1;

  if (likely (isRegistered_))
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

  // *NOTE*: makes nl receiver thread leave (see svc())
  inherited::isActive_ = false;
  // *NOTE*: stops state machine thread
  inherited::STATEMACHINE_T::stop (waitForCompletion_in,
                                   highPriority_in);
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
      buffer_->release (); buffer_ = NULL;
    } // end IF

    if (callbacks_)
    {
      nl_cb_put (callbacks_);
      callbacks_ = NULL;
    } // end IF

    controlId_ = 0;
    error_ = 0;
    features_.extendedFeatures.clear ();
    features_.features = 0;
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

    protocolFeatures_ = 0;

    userData_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  userData_ = configuration_in.userData;

//  ACE_ASSERT (!inherited::socketHandle_);
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
#if defined (ACE_LINUX) && defined (DBUS_NM_SUPPORT) && defined (SD_BUS_SUPPORT)
  if (Common_DBus_Tools::isUnitRunning (NULL,
                                        COMMON_SYSTEMD_UNIT_NETWORKMANAGER))
    if (unlikely (!Net_OS_Tools::networkManagerManageInterface (configuration_in.interfaceIdentifier,
                                                                false)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_OS_Tools::networkManagerManageInterface(\"%s\",false), continuing\n"),
                  ACE_TEXT (configuration_in.interfaceIdentifier.c_str ())));
  if (Common_DBus_Tools::isUnitRunning (NULL,
                                        COMMON_SYSTEMD_UNIT_WPASUPPLICANT))
    if (unlikely (!Net_WLAN_Tools::WPASupplicantManageInterface (NULL,
                                                                 configuration_in.interfaceIdentifier,
                                                                 false)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_WLAN_Tools::WPASupplicantManageInterface(\"%s\",false), aborting\n"),
                  ACE_TEXT (configuration_in.interfaceIdentifier.c_str ())));
      return false;
    } // end IF
//  if (Common_DBus_Tools::isUnitRunning (NULL,
//                                        COMMON_SYSTEMD_UNIT_IFUPDOWN)                     &&
//  Net_Common_Tools::isIfUpDownManagingInterface (configuration_in.interfaceIdentifier))
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("systemd unit \"%s\" is running and managing interface \"%s\"; this may interfere with the WLAN monitoring activity: please reinstall, continuing\n"),
//                ACE_TEXT (COMMON_SYSTEMD_UNIT_IFUPDOWN)));
#endif // ACE_LINUX && DBUS_NM_SUPPORT && SD_BUS_SUPPORT
  std::string interface_identifier_string =
    (configuration_in.interfaceIdentifier.empty () ? Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_11)
                                                   : configuration_in.interfaceIdentifier);
  if (!Net_WLAN_Tools::getProtocolFeatures (interface_identifier_string,
                                            inherited::socketHandle_,
                                            inherited::familyId_,
                                            protocolFeatures_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::getProtocolFeatures(\"%s\",%@,%d), aborting\n"),
                ACE_TEXT (configuration_in.interfaceIdentifier.c_str ()),
                inherited::socketHandle_,
                inherited::familyId_));
    return false;
  } // end IF

  ACE_ASSERT (inherited::configuration_);
  if (unlikely (inherited::configuration_->wiPhyIdentifier.empty ()))
  {
//    Net_WLAN_WiPhyIdentifiers_t wiphys_a =
//        Net_WLAN_Tools::getWiPhys (configuration_in.interfaceIdentifier,
//                                   inherited::socketHandle_,
//                                   inherited::familyId_,
//                                   false);
////                                   (NET_WLAN_MONITOR_NL80211_DEFAULT_SPLITWIPHYDUMPS && (protocolFeatures_ & NL80211_PROTOCOL_FEATURE_SPLIT_WIPHY_DUMP)));
//    Net_WLAN_WiPhyIdentifiersIterator_t iterator =
//        std::find_if (wiphys_a.begin (), wiphys_a.end (),
//                      std::bind2nd (Net_WLAN_WiPhyIdentifiersFindPredicate (),
//                                    0));
//    if (iterator == wiphys_a.end ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("\"%s\": failed to retrieve wiphy (index was: %u), aborting\n"),
//                  ACE_TEXT (configuration_in.interfaceIdentifier.c_str ()),
//                  0));
//      return false;
//    } // end IF
//    inherited::configuration_->wiPhyIdentifier = (*iterator).first;
    inherited::configuration_->wiPhyIdentifier =
        Net_WLAN_Tools::wiPhyIndexToWiPhyNameString (configuration_in.interfaceIdentifier,
                                                     inherited::socketHandle_,
                                                     inherited::familyId_,
                                                     0);
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("\"%s\": wiphy unspecified, setting default: \"%s\" (index: %u), continuing\n"),
                ACE_TEXT (configuration_in.interfaceIdentifier.c_str ()),
                ACE_TEXT (inherited::configuration_->wiPhyIdentifier.c_str ()),
                0));
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
                   UserDataType>::do_associate (const std::string& interfaceIdentifier_in,
                                                const struct ether_addr& accessPointMACAddress_in,
                                                const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::do_associate"));

  bool result = true;
  enum nl80211_auth_type authenticationType_e = NL80211_AUTHTYPE_AUTOMATIC;
  ACE_UINT32 frequency_i = 0;
  bool result_2 = false;

  // sanity check(s)
  ACE_ASSERT (inherited::familyId_);
  ACE_ASSERT (inherited::socketHandle_);
  if (!SSID_in.empty ())
  {
    ACE_ASSERT (!Net_Common_Tools::isAny (accessPointMACAddress_in));
  } // end IF

  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (interfaceIdentifier_in.empty ())
    interface_identifiers_a =
        Net_WLAN_Tools::getInterfaces (inherited::socketHandle_,
                                       inherited::familyId_);
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);

  // check cache ?
  if (SSID_in.empty ())
    goto continue_;
  { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, false);
    Net_WLAN_AccessPointCacheConstIterator_t iterator =
        inherited::SSIDCache_.find (SSID_in);
    ACE_ASSERT (iterator != inherited::SSIDCache_.end ());
//    ACE_ASSERT ((*iterator).second.second.authenticationType == inherited::configuration_->authenticationType);
    authenticationType_e = inherited::configuration_->authenticationType;
//    ACE_ASSERT ((*iterator).second.second.frequency == inherited::configuration_->frequency);
    if (unlikely (inherited::configuration_->frequency &&
                  ((*iterator).second.second.frequency != inherited::configuration_->frequency)))
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("cached access point frequency (is: %u MHz) differs from configured frequency (expected: %u MHz), continuing\n"),
                  (*iterator).second.second.frequency,
                  inherited::configuration_->frequency));
    frequency_i = (*iterator).second.second.frequency;
  } // end lock scope

continue_:
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    inherited::nl80211CBData_.index = ::if_nametoindex ((*iterator).c_str ());
    if (unlikely (!inherited::nl80211CBData_.index))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::if_nametoindex(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT ((*iterator).c_str ())));
      result = false;
      continue;
    } // end IF

    result_2 =
        (SSID_in.empty () ? Net_WLAN_Tools::disassociate (interfaceIdentifier_in,
                                                          inherited::socketHandle_,
                                                          inherited::familyId_)
                          : Net_WLAN_Tools::associate (interfaceIdentifier_in,
                                                       accessPointMACAddress_in,
                                                       SSID_in,
                                                       authenticationType_e,
                                                       frequency_i,
                                                       inherited::socketHandle_,
                                                       inherited::familyId_));
    if (unlikely (!result_2))
    {
      if (SSID_in.empty ())
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::disassociate(\"%s\"), continuing\n"),
                    ACE_TEXT (interfaceIdentifier_in.c_str ())));
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::associate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (interfaceIdentifier_in.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<const unsigned char*> (&(accessPointMACAddress_in.ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT (SSID_in.c_str ())));
      result = false;
    } // end IF
  } // end FOR

  return result;
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
                   UserDataType>::do_authenticate (const std::string& interfaceIdentifier_in,
                                                   const struct ether_addr& accessPointMACAddress_in,
                                                   const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::do_authenticate"));

  bool result = true;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::familyId_);
  ACE_ASSERT (inherited::socketHandle_);
  if (!SSID_in.empty ())
  {
    ACE_ASSERT (!Net_Common_Tools::isAny (accessPointMACAddress_in));
  } // end IF

  Net_InterfaceIdentifiers_t interface_identifiers_a;
  if (interfaceIdentifier_in.empty ())
    interface_identifiers_a =
        Net_WLAN_Tools::getInterfaces (inherited::socketHandle_,
                                       inherited::familyId_);
  else
    interface_identifiers_a.push_back (interfaceIdentifier_in);

  // check cache
  enum nl80211_auth_type authenticationType_e = NL80211_AUTHTYPE_AUTOMATIC;
  ACE_UINT32 frequency_i = 0;
  { ACE_GUARD_RETURN (ACE_MT_SYNCH::RECURSIVE_MUTEX, aGuard, inherited::subscribersLock_, false);
    Net_WLAN_AccessPointCacheConstIterator_t iterator =
        inherited::SSIDCache_.find (SSID_in);
    ACE_ASSERT (iterator != inherited::SSIDCache_.end ());
//    ACE_ASSERT ((*iterator).second.second.authenticationType == inherited::configuration_->authenticationType);
    authenticationType_e = inherited::configuration_->authenticationType;
//    ACE_ASSERT ((*iterator).second.second.frequency == inherited::configuration_->frequency);
    if (unlikely (inherited::configuration_->frequency &&
                  ((*iterator).second.second.frequency != inherited::configuration_->frequency)))
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("cached access point frequency (is: %u MHz) differs from configured frequency (expected: %u MHz), continuing\n"),
                  (*iterator).second.second.frequency,
                  inherited::configuration_->frequency));
    frequency_i = (*iterator).second.second.frequency;
  } // end lock scope

  bool result_2 = false;
  for (Net_InterfacesIdentifiersIterator_t iterator = interface_identifiers_a.begin ();
       iterator != interface_identifiers_a.end ();
       ++iterator)
  {
    inherited::nl80211CBData_.index = ::if_nametoindex ((*iterator).c_str ());
    if (unlikely (!inherited::nl80211CBData_.index))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::if_nametoindex(\"%s\"): \"%m\", continuing\n"),
                  ACE_TEXT ((*iterator).c_str ())));
      result = false;
      continue;
    } // end IF

    result_2 =
        (SSID_in.empty () ? Net_WLAN_Tools::deauthenticate (interfaceIdentifier_in,
                                                            inherited::socketHandle_,
                                                            inherited::familyId_)
                          : Net_WLAN_Tools::authenticate (interfaceIdentifier_in,
                                                          accessPointMACAddress_in,
                                                          SSID_in,
                                                          authenticationType_e,
                                                          frequency_i,
                                                          inherited::socketHandle_,
                                                          inherited::familyId_));
    if (unlikely (!result_2))
    {
      if (SSID_in.empty ())
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::deauthenticate(\"%s\"), continuing\n"),
                    ACE_TEXT (interfaceIdentifier_in.c_str ())));
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Net_WLAN_Tools::authenticate(\"%s\",%s,%s), continuing\n"),
                    ACE_TEXT (interfaceIdentifier_in.c_str ()),
                    ACE_TEXT (Net_Common_Tools::LinkLayerAddressToString (reinterpret_cast<unsigned char*> (&(const_cast<struct ether_addr&> (accessPointMACAddress_in).ether_addr_octet)), NET_LINKLAYER_802_11).c_str ()),
                    ACE_TEXT (SSID_in.c_str ())));
      result = false;
    } // end IF
  } // end FOR

  return result;
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
                                           const struct ether_addr& accessPointMACAddress_in,
                                           const std::string& SSID_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::do_scan"));

  inherited::nl80211CBData_.index = ::if_nametoindex (interfaceIdentifier_in.c_str ());
  if (unlikely (!inherited::nl80211CBData_.index))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::if_nametoindex(\"%s\"): \"%m\", returning\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    return;
  } // end IF
  inherited::nl80211CBData_.scanning = true;

  if (unlikely (!Net_WLAN_Tools::scan (interfaceIdentifier_in,
                                       inherited::socketHandle_,
                                       inherited::familyId_,
                                       features_,
                                       accessPointMACAddress_in,
                                       SSID_in,
                                       NET_WLAN_MONITOR_NL80211_DEFAULT_LOWPRIORITYSCANS,
                                       NET_WLAN_MONITOR_NL80211_DEFAULT_FLUSHCACHEBEFORESCANS,
                                       NET_WLAN_MONITOR_NL80211_DEFAULT_RANDOMIZEMACADDRESSFORSCANS)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::scan(\"%s\"), returning\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));

    inherited::nl80211CBData_.scanning = false;

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
                   UserDataType>::handle_input (ACE_HANDLE handle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::handle_input"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::socketHandle_);
  ACE_ASSERT (handle_in == nl_socket_get_fd (inherited::socketHandle_));

//  int result = -1;
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
      if ((error == EWOULDBLOCK) ||     // 11 : SSL_read() failed (buffer is empty)
          (error == ENOBUFS))           // 105: no buffer space (--> retry)
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

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
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
    // *TODO*: use non-blocking sockets
    result = nl_recvmsgs (inherited::socketHandle_, callbacks_);
    if (unlikely (result < 0))
    {
      if (-result == NLE_BAD_SOCK)           // 3: shutdown (see below))
        break;
      else if (-result == NLE_AGAIN)         // 4: non-blocking socket
      {
        ACE_Time_Value delay (0,
                              NET_WLAN_MONITOR_DEFAULT_DATA_POLL_INTERVAL_MS * 1000);
        ACE_OS::sleep (delay);
      } // end ELSE IF
      else if ((-result != NLE_BUSY)      && // 25
               (-result != -NLE_DUMP_INTR))  // 33: state has changed, dump (see: NLM_F_DUMP) is inconsistent
                                             //     most probable reason: driver received new scan results
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\" (%d), returning\n"),
                    inherited::socketHandle_,
                    ACE_TEXT (nl_geterror (result)), result));
        inherited::STATEMACHINE_T::stop (false,  // stop the state machine ASAP
                                         false);
        break;
      } // end IF
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("failed to nl_recvmsgs(%@): \"%s\", continuing\n"),
//                  inherited::socketHandle_,
//                  ACE_TEXT (nl_geterror (result))));
    } // end IF
    error = ACE_OS::last_error ();
    if (unlikely (error &&
                  (error != EAGAIN))) // 11: socket is non-blocking, no data
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to nl_recvmsgs(%@): \"%m\", returning\n"),
                  inherited::socketHandle_));
      inherited::STATEMACHINE_T::stop (false,  // stop the state machine ASAP
                                       false);
      break;
    } // end IF

    if (!inherited::isActive_)
      break;
  } while (true);

  nl_close (inherited::socketHandle_); //nl_socket_free (inherited::socketHandle_); inherited::socketHandle_ = NULL;

  return result;

state_machine:
  result = inherited::STATEMACHINE_T::svc ();

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
  if (unlikely (result_2 < 0))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task_T::flush(): \"%m\", continuing\n")));
  result_2 = inherited::msg_queue_->activate ();
  if (unlikely (result_2 == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_MessageQueue_T::activate(): \"%m\", continuing\n")));

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
    inherited::isActive_ = false; // make the nl dispatch thread leave ASAP
    inherited::dispatchStarted_ = false;
  } // end lock scope

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
