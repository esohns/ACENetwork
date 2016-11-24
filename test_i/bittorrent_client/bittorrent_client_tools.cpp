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

#include <ace/Synch.h>
#include "bittorrent_client_tools.h"

#include <ace/Configuration.h>
#include <ace/Configuration_Import_Export.h>
#include <ace/Log_Msg.h>

#include "stream_iallocator.h"

#include "net_defines.h"
#include "net_macros.h"

#include "bittorrent_client_configuration.h"
#include "bittorrent_client_defines.h"

ACE_HANDLE
BitTorrent_Client_Tools::connect (BitTorrent_Client_IConnector_t& connector_in,
                                  const ACE_INET_Addr& address_in,
                                  bool cloneModule_in,
                                  bool deleteModule_in,
                                  Stream_Module_t*& finalModule_inout,
                                  bool isPeer_in)
{
  NETWORK_TRACE (ACE_TEXT ("BitTorrent_Client_Tools::connect"));

  ACE_HANDLE return_value = ACE_INVALID_HANDLE;

  int result = -1;
  BitTorrent_Client_Configuration* configuration_p = NULL;
  BitTorrent_Client_UserData* user_data_p = NULL;

  // step0: retrive default configuration
  BitTorrent_Client_IConnection_Manager_t* connection_manager_p =
    BITTORRENT_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->get (configuration_p,
                             user_data_p);
  ACE_ASSERT (user_data_p);
  // *TODO*: remove type inferences
  ACE_ASSERT (user_data_p->configuration);

  // step1: set up configuration
  user_data_p->configuration->socketConfiguration.address = address_in;
  if (finalModule_inout)
  {
    user_data_p->configuration->streamConfiguration.cloneModule =
      cloneModule_in;
    user_data_p->configuration->streamConfiguration.deleteModule =
      deleteModule_in;
    user_data_p->configuration->streamConfiguration.module =
      finalModule_inout;
    if (deleteModule_in) finalModule_inout = NULL;
  } // end IF

  // step2: initialize connector
  if (!connector_in.initialize ((isPeer_in ? user_data_p->configuration->socketHandlerConfiguration
                                           : user_data_p->configuration->trackerSocketHandlerConfiguration)))
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
  return_value = connector_in.connect (address_in);
  if (return_value == ACE_INVALID_HANDLE)
  {
    // debug info
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result = address_in.addr_to_string (buffer,
                                        sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to connect(\"%s\"): \"%m\", aborting\n"),
                buffer));
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
