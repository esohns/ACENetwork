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

#include "ace/Log_Msg.h"

#include "common_defines.h"

#include "net_common.h"
#include "net_macros.h"

template <typename ConfigurationType,
          typename SessionDataType>
Net_Client_Connector<ConfigurationType,
                     SessionDataType>::Net_Client_Connector (ICONNECTION_MANAGER_T* interfaceHandle_in)
 : inherited (ACE_Reactor::instance (), // default reactor
              ACE_NONBLOCK)             // flags: non-blocking I/O
              //0)                       // flags
 , interfaceHandle_ (interfaceHandle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector::Net_Client_Connector"));

}

template <typename ConfigurationType,
          typename SessionDataType>
Net_Client_Connector<ConfigurationType,
                     SessionDataType>::~Net_Client_Connector ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector::~Net_Client_Connector"));

}

template <typename ConfigurationType,
          typename SessionDataType>
int
Net_Client_Connector<ConfigurationType,
                     SessionDataType>::make_svc_handler (Net_TCPConnection*& handler_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector::make_svc_handler"));

  // init return value(s)
  handler_inout = NULL;

  // default behavior
  ACE_NEW_NORETURN (handler_inout,
                    Net_TCPConnection ());
                    //Net_TCPConnection (interfaceHandle_));
  if (!handler_inout)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

  return ((handler_inout == NULL) ? -1 : 0);
}

template <typename ConfigurationType,
          typename SessionDataType>
void
Net_Client_Connector<ConfigurationType,
                     SessionDataType>::abort ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector::abort"));

  if (inherited::close () == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::close(): \"%m\", continuing\n")));
}

template <typename ConfigurationType,
          typename SessionDataType>
bool
Net_Client_Connector<ConfigurationType,
                     SessionDataType>::connect (const ACE_INET_Addr& peer_address)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Client_Connector::connect"));

  Net_TCPConnection* handler = NULL;
  int result = inherited::connect (handler,                           // service handler
                                   peer_address,                      // remote SAP
                                   ACE_Synch_Options::defaults,       // synch options
                                   ACE_sap_any_cast (ACE_INET_Addr&), // local SAP
                                   1,                                 // re-use address (SO_REUSEADDR) ?
                                   O_RDWR,                            // flags
                                   0);                                // perms
  if (result == -1)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    if (peer_address.addr_to_string (buffer, sizeof (buffer)) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Connector::connect(\"%s\"): \"%m\", aborting\n"),
                buffer));

    return false;
  } // end IF

  return true;
}
