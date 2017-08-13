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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_netlinksockethandler.h"
#endif

#include "test_u_connection_common.h"
#include "file_server_connection_common.h"

// forward declarations
struct FileServer_UserData;

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct FileServer_ConnectionConfiguration,
                                 struct FileServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct FileServer_UserData> FileServer_IInetConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct FileServer_ConnectionConfiguration,
                                 struct FileServer_ConnectionState,
                                 Net_Statistic_t,
                                 struct FileServer_UserData> FileServer_InetConnectionManager_t;

typedef ACE_Singleton<FileServer_InetConnectionManager_t,
                      ACE_SYNCH_MUTEX> FILESERVER_CONNECTIONMANAGER_SINGLETON;

#endif
