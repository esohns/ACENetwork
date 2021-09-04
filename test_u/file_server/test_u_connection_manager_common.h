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

#ifndef TEST_U_CONNECTION_MANAGER_COMMON_H
#define TEST_U_CONNECTION_MANAGER_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"

#include "test_u_connection_common.h"
#include "file_server_connection_common.h"

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 FileServer_TCPConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> FileServer_TCPIConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FileServer_TCPConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> FileServer_TCPConnectionManager_t;

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 FileServer_UDPConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> FileServer_UDPIConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FileServer_UDPConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> FileServer_UDPConnectionManager_t;

typedef ACE_Singleton<FileServer_TCPConnectionManager_t,
                      ACE_SYNCH_MUTEX> FILESERVER_TCPCONNECTIONMANAGER_SINGLETON;
typedef ACE_Singleton<FileServer_UDPConnectionManager_t,
                      ACE_SYNCH_MUTEX> FILESERVER_UDPCONNECTIONMANAGER_SINGLETON;

#endif
