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
#include "net_configuration.h"
#include "net_iconnectionmanager.h"

#include "http_common.h"
#include "http_network.h"

#include "test_i_common.h"

#include "test_i_connection_common.h"

// forward declarations
struct Test_I_UserData;

typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_URLStreamLoad_ConnectionConfiguration_t,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> Test_I_IConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_URLStreamLoad_ConnectionConfiguration_t,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct Net_UserData> Test_I_ConnectionManager_t;

typedef ACE_Singleton<Test_I_ConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_CONNECTIONMANAGER_SINGLETON;

//////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                ACE_INET_Addr,
                                Test_I_URLStreamLoad_ConnectionConfiguration_2_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                struct Net_UserData> Test_I_IConnectionManager_2_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                ACE_INET_Addr,
                                Test_I_URLStreamLoad_ConnectionConfiguration_2_t,
                                struct HTTP_ConnectionState,
                                HTTP_Statistic_t,
                                struct Net_UserData> Test_I_ConnectionManager_2_t;

typedef ACE_Singleton<Test_I_ConnectionManager_2_t,
                      ACE_SYNCH_MUTEX> TEST_I_CONNECTIONMANAGER_SINGLETON_2;

#endif
