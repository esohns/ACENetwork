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

#ifndef HTTP_CONFIGURATION_H
#define HTTP_CONFIGURATION_H

#include <string>

#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_inotify.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_stream_common.h"

// forward declarations
struct HTTP_ConnectionState;
struct HTTP_Record;
class HTTP_SessionMessage;
struct HTTP_Stream_SessionData;
struct HTTP_Stream_UserData;

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct Net_ConnectionConfiguration,
//                          struct HTTP_ConnectionState,
//                          HTTP_Statistic_t,
//                          HTTP_Stream> HTTP_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct Net_ConnectionConfiguration,
                                 struct HTTP_ConnectionState,
                                 HTTP_Statistic_t,
                                 struct HTTP_Stream_UserData> HTTP_IConnection_Manager_t;

//typedef Common_INotify_T<unsigned int,
//                         HTTP_Stream_SessionData,
//                         HTTP_Record,
//                         HTTP_SessionMessage> HTTP_IStreamNotify_t;

//struct HTTP_ConnectionConfiguration
// : Net_ConnectionConfiguration
//{
//  inline HTTP_ConnectionConfiguration ()
//   : Net_ConnectionConfiguration ()
//   //////////////////////////////////////
//  {};
//};

//struct HTTP_ConnectorConfiguration
//{
//  inline HTTP_ConnectorConfiguration ()
//   : /*configuration (NULL)
//   ,*/ connectionManager (NULL)
//   , socketHandlerConfiguration (NULL)
//   //, statisticCollectionInterval (0)
//  {};
//
//  //HTTP_Configuration*                   configuration;
//  HTTP_IConnection_Manager_t*             connectionManager;
//  struct HTTP_SocketHandlerConfiguration* socketHandlerConfiguration;
//  unsigned int                            statisticCollectionInterval; // statistic collecting interval (second(s)) [0: off]
//};

#endif
