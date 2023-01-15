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

#ifndef TEST_I_CONNECTION_MANAGER_COMMON_H
#define TEST_I_CONNECTION_MANAGER_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"

#include "ftp_common.h"

#include "test_i_connection_common.h"

// forward declarations
class FTP_Client_ConnectionConfiguration;

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 FTP_Client_ConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 FTP_Statistic_t,
                                 struct Net_UserData> FTP_Client_IConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 FTP_Client_ConnectionConfiguration,
                                 struct Net_StreamConnectionState,
                                 FTP_Statistic_t,
                                 struct Net_UserData> FTP_Client_ConnectionManager_t;

typedef ACE_Singleton<FTP_Client_ConnectionManager_t,
                      ACE_SYNCH_MUTEX> FTP_CLIENT_CONNECTIONMANAGER_SINGLETON;

#endif
