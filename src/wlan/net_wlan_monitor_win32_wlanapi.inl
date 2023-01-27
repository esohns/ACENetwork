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

#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Proactor.h"
#include "ace/Reactor.h"
#include "ace/Time_Value.h"

#include "common_tools.h"

#include "common_timer_manager_common.h"

#include "stream_defines.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"
#include "net_packet_headers.h"

#include "net_wlan_defines.h"

template <//typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
Net_WLAN_Monitor_T<//AddressType,
                   ACE_INET_Addr,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_WLANAPI,
                   UserDataType>::Net_WLAN_Monitor_T ()
 : inherited ()
 , userData_ (NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::Net_WLAN_Monitor_T"));

}

template <//typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<//AddressType,
                   ACE_INET_Addr,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_WLANAPI,
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
  ACE_ASSERT (inherited::configuration_);

  // sanity check(s)
  ACE_ASSERT (inherited::clientHandle_ == ACE_INVALID_HANDLE);

  if (unlikely (!Net_WLAN_Tools::initialize (inherited::clientHandle_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::initialize(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::clientHandle_ != ACE_INVALID_HANDLE);

  Net_InterfaceIdentifiers_t interface_identifiers_a =
      Net_WLAN_Tools::getInterfaces (inherited::clientHandle_);

  struct _WLAN_INTERFACE_INFO_LIST* interface_list_p = NULL;
  DWORD notification_mask = WLAN_NOTIFICATION_SOURCE_ALL;
  DWORD previous_notification_mask = 0;
  DWORD result =
      WlanRegisterNotification (inherited::clientHandle_,
                                notification_mask,
                                FALSE,
                                inherited::configuration_->notificationCB,
                                inherited::configuration_->notificationCBData,
                                NULL,
                                &previous_notification_mask);
  if (unlikely (result != ERROR_SUCCESS))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::WlanRegisterNotification(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (inherited::clientHandle_ != ACE_INVALID_HANDLE)
    Net_WLAN_Tools::initialize (inherited::clientHandle_);
  inherited::clientHandle_ = ACE_INVALID_HANDLE;

  return false;

continue_:
  inherited::isActive_ = true;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_IDLE);

  return true;
}

template <//typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
void
Net_WLAN_Monitor_T<//AddressType,
                   ACE_INET_Addr,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_WLANAPI,
                   UserDataType>::stop (bool,
                                        bool)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::stop"));

  // sanity check(s)
  if (unlikely (!inherited::isActive_))
    return;
  ACE_ASSERT (inherited::clientHandle_ != ACE_INVALID_HANDLE);

  if (likely (inherited::scanTimerId_ != -1))
    inherited::cancelScanTimer ();
  Net_WLAN_Tools::finalize (inherited::clientHandle_);
  inherited::clientHandle_ = ACE_INVALID_HANDLE;

  inherited::isActive_ = false;

  inherited::STATEMACHINE_T::change (NET_WLAN_MONITOR_STATE_INITIALIZED);
}

template <//typename AddressType,
          typename ConfigurationType,
          typename UserDataType>
bool
Net_WLAN_Monitor_T<//AddressType,
                   ACE_INET_Addr,
                   ConfigurationType,
                   NET_WLAN_MONITOR_API_WLANAPI,
                   UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_WLAN_Monitor_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    userData_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  userData_ = configuration_in.userData;

  return inherited::initialize (configuration_in);
}
