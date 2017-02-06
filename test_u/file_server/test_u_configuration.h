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

#ifndef TEST_U_CONFIGURATION_H
#define TEST_U_CONFIGURATION_H

#include <list>

#include <ace/INET_Addr.h>
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include <ace/Netlink_Addr.h>
#endif
#include <ace/Time_Value.h>

#include "stream_common.h"
#include "stream_iallocator.h"
//#include "stream_imessagequeue.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "net_server_defines.h"

#include "test_u_common.h"
#include "test_u_stream_common.h"

#include "test_u_connection_common.h"

#include "file_server_defines.h"
#include "file_server_stream_common.h"

// forward declarations
class Stream_IMessageQueue;
struct Test_U_ConnectionState;
class Test_U_Message;
class Test_U_SessionMessage;

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct Test_U_ConnectionConfiguration,
                                 struct Test_U_ConnectionState,
                                 Net_RuntimeStatistic_t,
                                 struct Test_U_UserData> Test_U_IInetConnectionManager_t;

//struct Test_U_ProtocolConfiguration
//{
//  inline Test_U_ProtocolConfiguration ()
//   : PDUSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
//   , pingInterval (ACE_Time_Value::zero)
//   , pingAutoAnswer (true)
//   , printPongMessages (true)
//  {};
//
//  unsigned int   PDUSize; // pdu size (if fixed)
//  ACE_Time_Value pingInterval; // {0: off}
//  bool           pingAutoAnswer;
//  bool           printPongMessages;
//};

//struct Test_U_ConnectionConfiguration;
struct Test_U_Configuration
{
  inline Test_U_Configuration ()
   : socketConfiguration ()
   , socketHandlerConfiguration ()
   , connectionConfiguration ()
   , allocatorConfiguration ()
   , streamConfiguration ()
   //, protocolConfiguration ()
   , userData ()
  {};

  // **************************** socket data **********************************
  struct Net_SocketConfiguration           socketConfiguration;
  struct Test_U_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Test_U_ConnectionConfiguration    connectionConfiguration;
  // **************************** stream data **********************************
  struct Stream_AllocatorConfiguration     allocatorConfiguration;
  struct Test_U_StreamConfiguration        streamConfiguration;
  // *************************** protocol data *********************************
  //struct Test_U_ProtocolConfiguration      protocolConfiguration;

  struct Test_U_UserData                   userData;
};

#endif
