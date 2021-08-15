/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns                                      *
 *   erik.sohns@web.de                                                     *
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

#ifndef PCP_NETWORK_H
#define PCP_NETWORK_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "net_common.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"

#include "pcp_common.h"

struct PCP_SessionState
{
  PCP_SessionState ()
   : nonce (0)
   , serverAddress (static_cast<u_short> (0),
                    static_cast<ACE_UINT32> (INADDR_ANY))
   , timeStamp (ACE_Time_Value::zero)
  {}

  ACE_UINT64     nonce;         // session nonce
  ACE_INET_Addr  serverAddress;
  ACE_Time_Value timeStamp;     // (mapping-) timeout
};

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct PCP_ConnectionState,
                          PCP_Statistic_t> PCP_IConnection_t;
typedef Net_ISocketConnection_T<ACE_INET_Addr,
                                struct PCP_ConnectionConfiguration,
                                struct PCP_ConnectionState,
                                PCP_Statistic_t,
                                struct PCP_SocketHandlerConfiguration> PCP_ISocketConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct PCP_SocketHandlerConfiguration> PCP_IConnector_t;

/////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct PCP_ConnectionConfiguration,
                                 struct PCP_ConnectionState,
                                 PCP_Statistic_t,
                                 struct Net_UserData> PCP_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct PCP_ConnectionConfiguration,
                                 struct PCP_ConnectionState,
                                 PCP_Statistic_t,
                                 struct Net_UserData> PCP_Connection_Manager_t;

typedef ACE_Singleton<PCP_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> PCP_CONNECTIONMANAGER_SINGLETON;

#endif
