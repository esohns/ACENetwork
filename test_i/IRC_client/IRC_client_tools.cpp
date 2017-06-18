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
#include "IRC_client_tools.h"

#include <ace/Configuration.h>
#include <ace/Configuration_Import_Export.h>
#include <ace/Log_Msg.h>

#include "stream_iallocator.h"

#include "net_defines.h"
#include "net_macros.h"

#include "IRC_client_configuration.h"
#include "IRC_client_defines.h"

void
IRC_Client_Tools::parseConfigurationFile (const std::string& fileName_in,
                                          IRC_LoginOptions& loginOptions_out,
                                          IRC_Client_Connections_t& connections_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Tools::parseConfigurationFile"));

  // initialize return value(s)
  connections_out.clear ();

  ACE_Configuration_Heap configuration_heap;
  int result = configuration_heap.open ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Configuration_Heap::open() failed: \"%m\", returning\n")));
    return;
  } // end IF

  ACE_Ini_ImpExp ini_import_export (configuration_heap);
  result = ini_import_export.import_config (fileName_in.c_str ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Ini_ImpExp::import_config(\"%s\") failed, returning\n"),
                ACE_TEXT (fileName_in.c_str ())));
    return;
  } // end IF

  // find/open "login" section...
  ACE_Configuration_Section_Key section_key;
  result =
    configuration_heap.open_section (configuration_heap.root_section (),
                                     ACE_TEXT (IRC_CLIENT_CNF_LOGIN_SECTION_HEADER),
                                     0, // MUST exist !
                                     section_key);
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(%s), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_LOGIN_SECTION_HEADER)));
    return;
  } // end IF

  // import values...
  int index = 0;
  ACE_TString item_name, item_value;
  ACE_Configuration::VALUETYPE item_type;
  while (configuration_heap.enumerate_values (section_key,
                                              index,
                                              item_name,
                                              item_type) == 0)
  {
    switch (item_type)
    {
      case ACE_Configuration::STRING:
      {
        result =
            configuration_heap.get_string_value (section_key,
                                                 ACE_TEXT (item_name.c_str ()),
                                                 item_value);
        if (result == -1)
        {
          ACE_ERROR ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"): \"%m\", returning\n"),
                      ACE_TEXT (item_name.c_str ())));
          return;
        } // end IF
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown item type (was: %d)\n"),
                    item_type));
        break;
      }
    } // end SWITCH

//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("enumerated %s, type %d\n"),
//                 ACE_TEXT (item_name.c_str ()),
//                 item_type));

    if (item_name == ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_PASSWORD_LABEL))
      loginOptions_out.password = ACE_TEXT_ALWAYS_CHAR (item_value.c_str ());
    else if (item_name == ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_NICKNAME_LABEL))
      loginOptions_out.nickname = ACE_TEXT_ALWAYS_CHAR (item_value.c_str ());
    else if (item_name == ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_USER_LABEL))
      loginOptions_out.user.userName =
          ACE_TEXT_ALWAYS_CHAR (item_value.c_str ());
    else if (item_name == ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_REALNAME_LABEL))
      loginOptions_out.user.realName =
          ACE_TEXT_ALWAYS_CHAR (item_value.c_str ());
    else if (item_name == ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_CHANNEL_LABEL))
      loginOptions_out.channel = ACE_TEXT_ALWAYS_CHAR (item_value.c_str ());
    else
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("unexpected key (was: \"%s\"), continuing\n"),
                  ACE_TEXT (item_name.c_str ())));

    ++index;
  } // end WHILE

  // find/open "connection" section...
  result =
    configuration_heap.open_section (configuration_heap.root_section (),
                                     ACE_TEXT (IRC_CLIENT_CNF_CONNECTION_SECTION_HEADER),
                                     0, // MUST exist !
                                     section_key);
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(%s), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_CONNECTION_SECTION_HEADER)));
    return;
  } // end IF

  // import values...
  index = 0;
  IRC_Client_ConnectionEntry entry;
//   u_int port = 0;
  std::stringstream converter;
  while (configuration_heap.enumerate_values (section_key,
                                              index,
                                              item_name,
                                              item_type) == 0)
  {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("enumerated %s, type %d\n"),
//                 ACE_TEXT (val_name.c_str ()),
//                 val_type));

    ACE_ASSERT (item_type == ACE_Configuration::STRING);
    result = configuration_heap.get_string_value (section_key,
                                                  ACE_TEXT (item_name.c_str ()),
                                                  item_value);
    if (result == -1)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(%s), returning\n"),
                  ACE_TEXT (item_name.c_str ())));
      return;
    } // end IF

    if (item_name == ACE_TEXT (IRC_CLIENT_CNF_SERVER_LABEL))
    {
      entry.hostName = ACE_TEXT_ALWAYS_CHAR (item_value.c_str ());

      if (!entry.ports.empty ()) connections_out.push_back (entry);
    } // end IF
    else if (item_name == ACE_TEXT (IRC_CLIENT_CNF_PORT_LABEL))
    {
//       ACE_ASSERT (val_type == ACE_Configuration::INTEGER);
//       if (config_heap.get_integer_value (section_key,
//                                          val_name.c_str (),
//                                          port))
//       {
//         ACE_ERROR ((LM_ERROR,
//                     ACE_TEXT ("failed to ACE_Configuration_Heap::get_integer_value(%s), returning\n"),
//                     ACE_TEXT (val_name.c_str ())));
//         return;
//       } // end IF
      IRC_Client_PortRange_t port_range;
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << item_value.c_str ();
      converter >> port_range.first;
//       port_range.first = static_cast<unsigned short> (port);
      port_range.second = port_range.first;
      entry.ports.push_back (port_range);

      if (!entry.hostName.empty ()) connections_out.push_back (entry);
    } // end IF
    else
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("unexpected key (was: \"%s\"), continuing\n"),
                  ACE_TEXT (item_name.c_str ())));

    ++index;
  } // end WHILE

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("imported \"%s\"...\n"),
//               ACE_TEXT (configurationFilename_in.c_str ())));
}

ACE_HANDLE
IRC_Client_Tools::connect (IRC_Client_IConnector_t& connector_in,
                           const ACE_INET_Addr& peerAddress_in,
                           const struct IRC_LoginOptions& loginOptions_in,
                           bool cloneModule_in,
                           bool deleteModule_in,
                           Stream_Module_t*& finalModule_inout)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Tools::connect"));

  ACE_HANDLE return_value = ACE_INVALID_HANDLE;

  int result = -1;
  struct IRC_Client_ConnectionConfiguration* configuration_p = NULL;
  struct IRC_Client_UserData* user_data_p = NULL;

  // step0: retrive default configuration
  IRC_Client_IConnection_Manager_t* connection_manager_p =
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->get (configuration_p,
                             user_data_p);
  ACE_ASSERT (user_data_p);
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_p);
  ACE_ASSERT (configuration_p->protocolConfiguration);
  ACE_ASSERT (configuration_p->streamConfiguration);

  // step1: set up configuration
  configuration_p->protocolConfiguration->loginOptions =
      loginOptions_in;
  configuration_p->socketHandlerConfiguration.socketConfiguration.address =
      peerAddress_in;
  if (finalModule_inout)
  {
    configuration_p->streamConfiguration->configuration_.cloneModule =
      cloneModule_in;
    configuration_p->streamConfiguration->configuration_.deleteModule =
      deleteModule_in;
    configuration_p->streamConfiguration->configuration_.module =
      finalModule_inout;
    if (deleteModule_in)
      finalModule_inout = NULL;
  } // end IF

  // step2: initialize connector
  if (!connector_in.initialize (*configuration_p))
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
  return_value = connector_in.connect (peerAddress_in);
  if (return_value == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to connect(%s): \"%m\", aborting\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (peerAddress_in).c_str ())));
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
