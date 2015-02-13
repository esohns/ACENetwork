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
#include "stdafx.h"

#include "net_client_asynchconnector.h"

#include "ace/Log_Msg.h"

//#include "common_defines.h"

//#include "net_connection_manager_common.h"
#include "net_macros.h"

Net_Client_AsynchConnector::Net_Client_AsynchConnector ()
// : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector::Net_Client_AsynchConnector"));

  // init base class
  if (inherited::open (false,       // pass addresses ?
                       NULL,        // default proactor
                       true) == -1) // validate new connections ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::open(): \"%m\", continuing\n")));
}

Net_Client_AsynchConnector::~Net_Client_AsynchConnector ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector::~Net_Client_AsynchConnector"));

}

Net_AsynchTCPConnection*
Net_Client_AsynchConnector::make_handler (void)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector::make_handler"));

  // init return value(s)
  Net_AsynchTCPConnection* result = NULL;

  // default behavior
//  ACE_NEW_NORETURN(result,
//                   Net_AsynchTCPConnection (NET_CONNECTIONMANAGER_SINGLETON::instance ()));
  ACE_NEW_NORETURN (result,
                    Net_AsynchTCPConnection ());
  if (!result)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return result;
}

void
Net_Client_AsynchConnector::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector::abort"));

  if (inherited::cancel () == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Asynch_Connector::cancel(): \"%m\", continuing\n")));
}

void
Net_Client_AsynchConnector::connect (const ACE_INET_Addr& peer_address)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector::connect"));

  int result =
    inherited::connect (peer_address,                            // remote SAP
                        ACE_sap_any_cast (const ACE_INET_Addr&), // local SAP
                        1,                                       // re-use address (SO_REUSEADDR) ?
                        NULL);                                   // ACT
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    if (peer_address.addr_to_string (buffer,
                                     sizeof (buffer)) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT("failed to ACE_Asynch_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));
  } // end IF
}

int
Net_Client_AsynchConnector::validate_connection (const ACE_Asynch_Connect::Result& result_in,
                                                 const ACE_INET_Addr& remoteSAP_in,
                                                 const ACE_INET_Addr& localSAP_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_AsynchConnector::validate_connection"));

  // success ?
  if (result_in.success () == 0)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    if (remoteSAP_in.addr_to_string (buffer, sizeof (buffer)) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Client_AsynchConnector::connect(\"%s\"): \"%s\", aborting\n"),
                buffer,
                ACE_OS::strerror (result_in.error ())));
  } // end IF

  return ((result_in.success () == 1) ? 0 : -1);
}
