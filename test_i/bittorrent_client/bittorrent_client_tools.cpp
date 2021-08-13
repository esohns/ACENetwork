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
#include "stdafx.h"

//#include "ace/Synch.h"
#include "bittorrent_client_tools.h"

#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Log_Msg.h"

#include "stream_iallocator.h"

#include "net_defines.h"
#include "net_macros.h"

#include "bittorrent_client_configuration.h"
#include "bittorrent_client_defines.h"

ACE_HANDLE
BitTorrent_Client_Tools::connect (BitTorrent_Client_IPeerConnector_t& peerConnector_in,
                                  BitTorrent_Client_ITrackerConnector_t& trackerConnector_in,
                                  const ACE_INET_Addr& address_in,
                                  bool cloneModule_in, // ? ==> delete module
                                  Stream_Module_t*& finalModule_inout,
                                  bool isPeer_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_Tools::connect"));

  ACE_HANDLE return_value = ACE_INVALID_HANDLE;

//  int result = -1;
  BitTorrent_Client_PeerConnectionConfiguration* peer_configuration_p =
      NULL;
  struct Net_UserData* peer_user_data_p = NULL;
  BitTorrent_Client_TrackerConnectionConfiguration* tracker_configuration_p =
      NULL;
  struct Net_UserData* tracker_user_data_p = NULL;

  // step0: retrive default configuration
  BitTorrent_Client_IPeerConnection_Manager_t* peer_connection_manager_p =
    BITTORRENT_CLIENT_PEERCONNECTION_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (peer_connection_manager_p);
  BitTorrent_Client_ITrackerConnection_Manager_t* tracker_connection_manager_p =
    BITTORRENT_CLIENT_TRACKERCONNECTION_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (tracker_connection_manager_p);
  peer_connection_manager_p->get (peer_configuration_p,
                                  peer_user_data_p);
  tracker_connection_manager_p->get (tracker_configuration_p,
                                     tracker_user_data_p);
  ACE_ASSERT (peer_user_data_p);
  ACE_ASSERT (tracker_user_data_p);
  // *TODO*: remove type inferences
  ACE_ASSERT (peer_configuration_p);
  ACE_ASSERT (peer_configuration_p->streamConfiguration);
  ACE_ASSERT (tracker_configuration_p);
  ACE_ASSERT (tracker_configuration_p->streamConfiguration);

  // step1: set up configuration
  if (isPeer_in)
    peer_configuration_p->socketConfiguration.address = address_in;
  else
    tracker_configuration_p->socketConfiguration.address = address_in;
  bool clone_module_b = cloneModule_in;
  Stream_Module_t* final_module_p = finalModule_inout;
  if (finalModule_inout)
  {
    if (isPeer_in)
    {
      clone_module_b =
        peer_configuration_p->streamConfiguration->configuration_->cloneModule;
      peer_configuration_p->streamConfiguration->configuration_->cloneModule =
        cloneModule_in;
      final_module_p =
        peer_configuration_p->streamConfiguration->configuration_->module;
      peer_configuration_p->streamConfiguration->configuration_->module =
        finalModule_inout;
    } // end IF
    else
    {
      clone_module_b =
        tracker_configuration_p->streamConfiguration->configuration_->cloneModule;
      tracker_configuration_p->streamConfiguration->configuration_->cloneModule =
        cloneModule_in;
      final_module_p =
        tracker_configuration_p->streamConfiguration->configuration_->module;
      tracker_configuration_p->streamConfiguration->configuration_->module =
        finalModule_inout;
    } // end ELSE
  } // end IF

  // step2: initialize connector
  bool result_2 =
      (isPeer_in ? peerConnector_in.initialize (*peer_configuration_p)
                 : trackerConnector_in.initialize (*tracker_configuration_p));
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", aborting\n")));
    goto error;
  } // end IF

  //connection_manager_p->lock ();
  //user_data_p->configuration = &configuration;
  //connection_manager_p->set (configuration,
  //                           user_data_p);

  // step3: (try to) connect to the server
  return_value = (isPeer_in ? peerConnector_in.connect (address_in)
                            : trackerConnector_in.connect (address_in));
  if (return_value == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to Net_IConnector_T::connect(%s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ())));
    goto error;
  } // end IF
  if (!((isPeer_in && peerConnector_in.useReactor ()) ||
        (!isPeer_in && trackerConnector_in.useReactor ())))
  {
    // step0a: wait for the connection attempt to complete
    BitTorrent_Client_IPeerConnector_t::IASYNCH_CONNECTOR_T* iasynch_peer_connector_p =
      dynamic_cast<BitTorrent_Client_IPeerConnector_t::IASYNCH_CONNECTOR_T*> (&peerConnector_in);
    ACE_ASSERT (iasynch_peer_connector_p);
    BitTorrent_Client_ITrackerConnector_t::IASYNCH_CONNECTOR_T* iasynch_tracker_connector_p =
      dynamic_cast<BitTorrent_Client_ITrackerConnector_t::IASYNCH_CONNECTOR_T*> (&trackerConnector_in);
    ACE_ASSERT (iasynch_tracker_connector_p);
    typename BitTorrent_Client_AsynchPeerConnector_t::ICONNECTION_T* ipeer_connection_p =
      NULL;
    typename BitTorrent_Client_AsynchPeerConnector_t::ISTREAM_CONNECTION_T* ipeer_stream_connection_p =
      NULL;
    typename BitTorrent_Client_AsynchTrackerConnector_t::ICONNECTION_T* itracker_connection_p =
      NULL;
    typename BitTorrent_Client_AsynchTrackerConnector_t::ISTREAM_CONNECTION_T* itracker_stream_connection_p =
      NULL;
    ACE_Time_Value timeout (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_S,
                            0);
    ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
    ACE_Time_Value delay (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_INTERVAL_S,
                          0);
    //ACE_Time_Value initialization_timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT_S,
    //                                       0);
    Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
    int result = (isPeer_in ? iasynch_peer_connector_p->wait (return_value,
                                                              timeout)
                            : iasynch_tracker_connector_p->wait (return_value,
                                                                 timeout));
    if (unlikely (result))
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IConnector::connect(%s): \"%s\" aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
                  ACE::sock_error (result)));
#else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_IConnector::connect(%s): \"%s\" aborting\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
                  ACE_TEXT (ACE_OS::strerror (result))));
#endif // ACE_WIN32 || ACE_WIN64
      goto error;
    } // end IF

    // step0b: wait for the connection to register with the manager
    // *TODO*: this may not be accurate/applicable for/to all protocols
    do
    {
      // *TODO*: avoid this tight loop
      if (isPeer_in)
        ipeer_connection_p = peer_connection_manager_p->get (address_in,
                                                             true);
      else
        itracker_connection_p = tracker_connection_manager_p->get (address_in,
                                                                   true);
      if (ipeer_connection_p || itracker_connection_p)
        break; // done
      result = ACE_OS::sleep (delay);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    &delay));
    } while (COMMON_TIME_NOW < deadline);
    if (!(ipeer_connection_p || itracker_connection_p))
      goto error;

    // step3a: wait for the connection to finish initializing
    //deadline = COMMON_TIME_NOW + initialization_timeout;
    // *TODO*: avoid tight loop here
    do
    {
      status = (isPeer_in ? ipeer_connection_p->status ()
                          : itracker_connection_p->status ());
      // *TODO*: break early upon failure too
      if (status == NET_CONNECTION_STATUS_OK)
        break;
    } while (COMMON_TIME_NOW < deadline);
    if (status != NET_CONNECTION_STATUS_OK)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("connection failed to initialize (status was: %d), returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_in).c_str ()),
                  status));
      if (isPeer_in)
      {
        ipeer_connection_p->decrease (); ipeer_connection_p = NULL;
      } // end IF
      else
      {
        itracker_connection_p->decrease (); itracker_connection_p = NULL;
      } // end ELSE
      goto error;
    } // end IF
    // step3b: wait for the connection stream to finish initializing
    if (isPeer_in)
      ipeer_stream_connection_p =
        dynamic_cast<typename BitTorrent_Client_AsynchPeerConnector_t::ISTREAM_CONNECTION_T*> (ipeer_connection_p);
    else
      itracker_stream_connection_p =
        dynamic_cast<typename BitTorrent_Client_AsynchTrackerConnector_t::ISTREAM_CONNECTION_T*> (itracker_connection_p);
    if (!(ipeer_stream_connection_p || itracker_stream_connection_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>, returning\n")));//,
                  //(isPeer_in ? ipeer_connection_p : itracker_connection_p)));
      if (isPeer_in)
      {
        ipeer_connection_p->decrease (); ipeer_connection_p = NULL;
      } // end IF
      else
      {
        itracker_connection_p->decrease (); itracker_connection_p = NULL;
      } // end ELSE
      goto error;
    } // end IF
    if (isPeer_in)
    {
      ipeer_stream_connection_p->wait (STREAM_STATE_RUNNING,
                                       NULL); // <-- block
      ipeer_connection_p->decrease (); ipeer_connection_p = NULL;
    } // end IF
    else
    {
      itracker_stream_connection_p->wait (STREAM_STATE_RUNNING,
                                          NULL); // <-- block
      itracker_connection_p->decrease (); itracker_connection_p = NULL;
    } // end ELSE
  } // end IF

  // clean up
  if (finalModule_inout)
  {
    if (isPeer_in)
    {
      peer_configuration_p->streamConfiguration->configuration_->cloneModule =
        clone_module_b;
      peer_configuration_p->streamConfiguration->configuration_->module =
        final_module_p;
    } // end IF
    else
    {
      tracker_configuration_p->streamConfiguration->configuration_->cloneModule =
        clone_module_b;
      tracker_configuration_p->streamConfiguration->configuration_->module =
        final_module_p;
    } // end ELSE
  } // end IF

  //connection_manager_p->unlock ();

  // *NOTE*: handlers automagically register with the connection manager and
  //         will also de-register and self-destruct on disconnects !

  return return_value;

error:
  if (finalModule_inout)
  {
    if (isPeer_in)
    {
      peer_configuration_p->streamConfiguration->configuration_->cloneModule =
        clone_module_b;
      peer_configuration_p->streamConfiguration->configuration_->module =
        final_module_p;
    } // end IF
    else
    {
      tracker_configuration_p->streamConfiguration->configuration_->cloneModule =
        clone_module_b;
      tracker_configuration_p->streamConfiguration->configuration_->module =
        final_module_p;
    } // end ELSE
  } // end IF

  return ACE_INVALID_HANDLE;
}
