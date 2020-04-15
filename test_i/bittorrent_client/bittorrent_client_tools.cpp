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
                                  bool cloneModule_in,
                                  bool deleteModule_in,
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
    peer_configuration_p->address = address_in;
  else
    tracker_configuration_p->address = address_in;
  if (finalModule_inout)
  {
    if (isPeer_in)
    {
      peer_configuration_p->streamConfiguration->configuration->cloneModule =
        cloneModule_in;
      //peer_configuration_p->streamConfiguration->configuration->deleteModule =
      //  deleteModule_in;
      peer_configuration_p->streamConfiguration->configuration->module =
        finalModule_inout;
    } // end IF
    else
    {
      tracker_configuration_p->streamConfiguration->configuration->cloneModule =
        cloneModule_in;
      //tracker_configuration_p->streamConfiguration->configuration->deleteModule =
      //  deleteModule_in;
      tracker_configuration_p->streamConfiguration->configuration->module =
        finalModule_inout;
    } // end ELSE
    if (deleteModule_in)
      finalModule_inout = NULL;
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

  //connection_manager_p->unlock ();

  // *NOTE*: handlers automagically register with the connection manager and
  //         will also de-register and self-destruct on disconnects !

  return return_value;

error:
  if (deleteModule_in)
  {
    delete finalModule_inout;
    finalModule_inout = NULL;
  } // end IF

  return ACE_INVALID_HANDLE;
}
