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

#ifndef TEST_U_CONNECTION_COMMON_H
#define TEST_U_CONNECTION_COMMON_H

#include "ace/INET_Addr.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_iconnection.h"

#include "test_u_common.h"

//struct Test_U_StreamConfiguration;
//struct Test_U_ConnectionConfiguration
// : Net_ConnectionConfiguration
//{
//  inline Test_U_ConnectionConfiguration ()
//   : Net_ConnectionConfiguration ()
//   ///////////////////////////////////////
//   , socketHandlerConfiguration (NULL)
//   , streamConfiguration (NULL)
//   , userData (NULL)
//  {};

//  struct Test_U_SocketHandlerConfiguration* socketHandlerConfiguration;
//  struct Test_U_StreamConfiguration*        streamConfiguration;

//  struct Test_U_UserData*                   userData;
//};

//struct Test_U_ConnectionState
// : Net_ConnectionState
//{
//  inline Test_U_ConnectionState ()
//   : Net_ConnectionState ()
//   , userData (NULL)
//  {};

//  struct Net_UserData* userData;
//};

//////////////////////////////////////////

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct Test_U_ConnectionConfiguration,
//                          struct Test_U_ConnectionState,
//                          Net_Statistic_t> Test_U_IConnection_t;

//////////////////////////////////////////

#endif
