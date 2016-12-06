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
#include <ace/Log_Msg.h>

#include "net_defines.h"
#include "net_macros.h"

#include "net_client_defines.h"

#include "http_tools.h"

#include "bittorrent_defines.h"
#include "bittorrent_tools.h"

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::BitTorrent_Session_T ()
 : inherited ()
 , trackerConnectionManager_ (NULL)
 , trackerHandlerConfiguration_ (NULL)
 , logToFile_ (BITTORRENT_DEFAULT_SESSION_LOG)
 , metaInfoFileName_ ()
 , peerHandlerModule_ (NULL)
 , peerStreamHandler_ (this)
 , trackerHandlerModule_ (NULL)
 , trackerStreamHandler_ (this)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::BitTorrent_Session_T"));

  ACE_NEW_NORETURN (peerHandlerModule_,
                    PEER_MESSAGEHANDLER_MODULE_T (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_HANDLER_MODULE_NAME),
                                                  NULL,
                                                  false));
  if (!peerHandlerModule_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  ACE_NEW_NORETURN (trackerHandlerModule_,
                    TRACKER_MESSAGEHANDLER_MODULE_T (ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_HANDLER_MODULE_NAME),
                                                     NULL,
                                                     false));
  if (!trackerHandlerModule_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::~BitTorrent_Session_T ()
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::~BitTorrent_Session_T"));

  if (peerHandlerModule_)
    delete peerHandlerModule_;
  if (trackerHandlerModule_)
    delete trackerHandlerModule_;
}

//////////////////////////////////////////

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
bool
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::initialize (const ConfigurationType& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::initialize"));

  // *TODO*: remove type inferences
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, false);

    inherited::state_.controller = configuration_in.controller;
  } // end lock scope
  trackerConnectionManager_ = configuration_in.trackerConnectionManager;
  trackerHandlerConfiguration_ =
      const_cast<ConfigurationType&> (configuration_in).trackerSocketHandlerConfiguration;
  metaInfoFileName_ = configuration_in.metaInfoFileName;

  return inherited::initialize (configuration_in);
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::connect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::connect"));

  // step0: subscribe to notifications
  PeerConnectionConfigurationType* configuration_p = NULL;
  PeerUserDataType* user_data_p = NULL;
  typename PeerStreamType::IDATA_NOTIFY_T* subscriber_p = NULL;
  bool clone_module = false;
  bool delete_module = false;
  Stream_Module_t* module_p = NULL;
  if (inherited::connectionManager_)
  {
    inherited::connectionManager_->get (configuration_p,
                                        user_data_p);
    ACE_ASSERT (configuration_p);
    ACE_ASSERT (configuration_p->socketHandlerConfiguration);
    ACE_ASSERT (configuration_p->socketHandlerConfiguration->socketConfiguration);
    ACE_ASSERT (configuration_p->streamConfiguration);
    ACE_ASSERT (configuration_p->streamConfiguration->moduleHandlerConfiguration);

    // *TODO*: remove type inferences

    // step1: set up configuration
    configuration_p->socketHandlerConfiguration->socketConfiguration->address =
        address_in;

    subscriber_p =
        configuration_p->streamConfiguration->moduleHandlerConfiguration->subscriber;
    ACE_ASSERT (!subscriber_p);
    configuration_p->streamConfiguration->moduleHandlerConfiguration->subscriber =
        &peerStreamHandler_;

    clone_module = configuration_p->streamConfiguration->cloneModule;
    delete_module = configuration_p->streamConfiguration->deleteModule;
    module_p = configuration_p->streamConfiguration->module;
    ACE_ASSERT (!module_p);

    configuration_p->streamConfiguration->cloneModule = false;
    configuration_p->streamConfiguration->deleteModule = false;
    configuration_p->streamConfiguration->module = peerHandlerModule_;
  } // end IF

  // step1: (try to) connect
  inherited::connect (address_in);

  // step2: reset configuration
  if (inherited::connectionManager_)
  {
    configuration_p->streamConfiguration->moduleHandlerConfiguration->subscriber =
        subscriber_p;

    configuration_p->streamConfiguration->cloneModule = clone_module;
    configuration_p->streamConfiguration->deleteModule = delete_module;
    configuration_p->streamConfiguration->module = module_p;
  } // end IF
};
template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::disconnect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::disconnect"));

  inherited::disconnect (id_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

    if (inherited::state_.connections.empty () &&
        inherited::state_.controller)
    {
      try {
        inherited::state_.controller->notify (metaInfoFileName_,
                                              BITTORRENT_EVENT_CANCELLED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
      }
    } // end IF
  } // end lock scope
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::trackerConnect (const ACE_INET_Addr& address_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerConnect"));

  TrackerConnectorType connector (trackerConnectionManager_,
                                  ACE_Time_Value::zero);
  ACE_HANDLE handle = ACE_INVALID_HANDLE;

  // debug info
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result = address_in.addr_to_string (buffer,
                                          sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));

  // step0: subscribe to notifications
  TrackerConnectionConfigurationType* configuration_p = NULL;
  TrackerUserDataType* user_data_p = NULL;
  typename TrackerStreamType::IDATA_NOTIFY_T* subscriber_p = NULL;
  bool clone_module = false;
  bool delete_module = false;
  Stream_Module_t* module_p = NULL;
  if (trackerConnectionManager_)
  {
    trackerConnectionManager_->get (configuration_p,
                                    user_data_p);
    ACE_ASSERT (configuration_p);
    ACE_ASSERT (configuration_p->socketHandlerConfiguration);
    ACE_ASSERT (configuration_p->socketHandlerConfiguration->socketConfiguration);
    ACE_ASSERT (configuration_p->streamConfiguration);
    ACE_ASSERT (configuration_p->streamConfiguration->moduleHandlerConfiguration);

    // *TODO*: remove type inferences

    // step1: set up configuration
    configuration_p->socketHandlerConfiguration->socketConfiguration->address =
        address_in;

    subscriber_p =
        configuration_p->streamConfiguration->moduleHandlerConfiguration->subscriber;
    ACE_ASSERT (!subscriber_p);
    configuration_p->streamConfiguration->moduleHandlerConfiguration->subscriber =
        &trackerStreamHandler_;

    clone_module = configuration_p->streamConfiguration->cloneModule;
    delete_module = configuration_p->streamConfiguration->deleteModule;
    module_p = configuration_p->streamConfiguration->module;
    ACE_ASSERT (!module_p);

    configuration_p->streamConfiguration->cloneModule = false;
    configuration_p->streamConfiguration->deleteModule = false;
    configuration_p->streamConfiguration->module = trackerHandlerModule_;
  } // end IF

  ACE_Time_Value deadline;
  ACE_Time_Value initialization_timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT,
                                         0);
  Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
  typename TrackerConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
      NULL;

  // step1: initialize connector
  typename TrackerConnectorType::ICONNECTOR_T* iconnector_p = &connector;
  typename TrackerConnectorType::ICONNECTION_T* iconnection_p = NULL;
  ACE_ASSERT (trackerHandlerConfiguration_);
  if (!iconnector_p->initialize (*trackerHandlerConfiguration_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
    goto error;
  } // end IF

  // step2: try to connect
  handle = iconnector_p->connect (address_in);
  if (handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\": \"%m\", returning\n"),
                buffer));
    goto error;
  } // end IF
  if (inherited::isAsynch_)
  {
    deadline =
        (COMMON_TIME_NOW +
         ACE_Time_Value (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0));
    ACE_Time_Value delay (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT_INTERVAL,
                          0);
    do
    {
      // *TODO*: this does not work...
      iconnection_p = trackerConnectionManager_->get (address_in);
      if (iconnection_p)
        break; // done

      result = ACE_OS::sleep (delay);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    &delay));
    } while (COMMON_TIME_NOW < deadline);
  } // end ELSE
  else
    iconnection_p = trackerConnectionManager_->get (handle);
  if (!iconnection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\": \"%m\", returning\n"),
                buffer));
    goto error;
  } // end IF

  // step3a: wait for the connection to finish initializing
  // *TODO*: avoid tight loop here
  deadline = COMMON_TIME_NOW + initialization_timeout;
  do
  {
    status = iconnection_p->status ();
    if (status == NET_CONNECTION_STATUS_OK) break;
  } while (COMMON_TIME_NOW < deadline);
  if (status != NET_CONNECTION_STATUS_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("connection (to: \"%s\") failed to initialize (status was: %d), returning\n"),
                buffer,
                status));
    goto error;
  } // end IF
  // step3b: wait for the connection stream to finish initializing
  istream_connection_p =
    dynamic_cast<typename TrackerConnectorType::ISTREAM_CONNECTION_T*> (iconnection_p);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), returning\n"),
                iconnection_p));
    goto error;
  } // end IF
  istream_connection_p->wait (STREAM_STATE_RUNNING,
                              NULL); // <-- block

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connected to \"%s\": %d...\n"),
              buffer,
              iconnection_p->id ()));

  iconnection_p->decrease ();

error:
  if (trackerConnectionManager_)
  {
    configuration_p->streamConfiguration->moduleHandlerConfiguration->subscriber =
        subscriber_p;

    configuration_p->streamConfiguration->cloneModule = clone_module;
    configuration_p->streamConfiguration->deleteModule = delete_module;
    configuration_p->streamConfiguration->module = module_p;

  } // end IF
};
template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::trackerDisconnect (Net_ConnectionId_t id_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::trackerDisconnect"));

  inherited::disconnect (id_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

    if (inherited::state_.connections.empty () &&
        inherited::state_.controller)
    {
      try {
        inherited::state_.controller->notify (metaInfoFileName_,
                                              BITTORRENT_EVENT_CANCELLED);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in BitTorrent_IControl_T::notify(), continuing\n")));
      }
    } // end IF
  } // end lock scope
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::notify (const struct HTTP_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (HTTP_Tools::dump (record_in).c_str ())));
#endif
}
template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::notify (const struct BitTorrent_Record& record_in,
                                          ACE_Message_Block* messageBlock_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::notify"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (BitTorrent_Tools::Record2String (record_in).c_str ())));
#endif

  switch (record_in.type)
  {
    case BITTORRENT_MESSAGETYPE_PIECE:
    {
      // sanity check(s)
      ACE_ASSERT (messageBlock_in);

      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), returning\n"),
                  record_in.type));
      return;
    }
  } // end SWITCH
}

//////////////////////////////////////////

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::error (const struct BitTorrent_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::error"));

  std::string message_text = BitTorrent_Tools::Record2String (record_in);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("received error message: \"%s\"\n"),
              ACE_TEXT (message_text.c_str ())));
}

template <typename PeerHandlerConfigurationType,
          typename TrackerHandlerConfigurationType,
          typename PeerConnectionConfigurationType,
          typename TrackerConnectionConfigurationType,
          typename PeerConnectionStateType,
          typename PeerStreamType,
          typename TrackerStreamType,
          typename StreamStatusType,
          typename PeerHandlerModuleType,
          typename TrackerHandlerModuleType,
          typename PeerConnectionType,
          typename TrackerConnectionType,
          typename PeerConnectionManagerType,
          typename TrackerConnectionManagerType,
          typename PeerConnectorType,
          typename TrackerConnectorType,
          typename ConfigurationType,
          typename StateType,
          typename PeerUserDataType,
          typename TrackerUserDataType,
          typename ControllerInterfaceType,
          typename CBDataType>
void
BitTorrent_Session_T<PeerHandlerConfigurationType,
                     TrackerHandlerConfigurationType,
                     PeerConnectionConfigurationType,
                     TrackerConnectionConfigurationType,
                     PeerConnectionStateType,
                     PeerStreamType,
                     TrackerStreamType,
                     StreamStatusType,
                     PeerHandlerModuleType,
                     TrackerHandlerModuleType,
                     PeerConnectionType,
                     TrackerConnectionType,
                     PeerConnectionManagerType,
                     TrackerConnectionManagerType,
                     PeerConnectorType,
                     TrackerConnectorType,
                     ConfigurationType,
                     StateType,
                     PeerUserDataType,
                     TrackerUserDataType,
                     ControllerInterfaceType,
                     CBDataType>::log (const struct BitTorrent_Record& record_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Session_T::log"));

  std::string message_text = BitTorrent_Tools::Record2String (record_in);
  log (std::string (), // --> server log
       message_text);
}
