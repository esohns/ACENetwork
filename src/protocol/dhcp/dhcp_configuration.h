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

#ifndef DHCP_CONFIGURATION_H
#define DHCP_CONFIGURATION_H

#include <string>

#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_inotify.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "dhcp_common.h"
#include "dhcp_defines.h"
#include "dhcp_stream_common.h"

// forward declarations
struct DHCP_ConnectionState;
struct DHCP_Record;
struct DHCP_ModuleHandlerConfiguration;
class DHCP_SessionMessage;
struct DHCP_Stream_SessionData;
struct DHCP_Stream_UserData;

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          DHCP_Configuration,
//                          DHCP_ConnectionState,
//                          DHCP_RuntimeStatistic_t,
//                          DHCP_Stream> DHCP_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 DHCP_Configuration,
                                 DHCP_ConnectionState,
                                 DHCP_RuntimeStatistic_t,
                                 ////////
                                 DHCP_Stream_UserData> DHCP_IConnection_Manager_t;

typedef Common_INotify_T<DHCP_Stream_SessionData,
                         DHCP_Record,
                         DHCP_SessionMessage> DHCP_IStreamNotify_t;

struct DHCP_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline DHCP_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   //////////////////////////////////////
   , userData (NULL)
  {
    PDUSize = DHCP_BUFFER_SIZE;
  };

  DHCP_Stream_UserData* userData;
};

//struct DHCP_ConnectorConfiguration
//{
//  inline DHCP_ConnectorConfiguration ()
//   : /*configuration (NULL)
//   ,*/ connectionManager (NULL)
//   , socketHandlerConfiguration (NULL)
//   //, statisticCollectionInterval (0)
//  {};
//
//  //DHCP_Configuration*              configuration;
//  DHCP_IConnection_Manager_t*      connectionManager;
//  DHCP_SocketHandlerConfiguration* socketHandlerConfiguration;
//  unsigned int                    statisticCollectionInterval; // statistic collecting interval (second(s)) [0: off]
//};

struct DHCP_ProtocolConfiguration
{
  inline DHCP_ProtocolConfiguration ()
   : requestBroadcastReplies (DHCP_DEFAULT_FLAGS_BROADCAST)
   , sendRequestOnOffer (false)
  {};

  bool requestBroadcastReplies;
  bool sendRequestOnOffer;
};

struct DHCP_StreamConfiguration;
struct DHCP_ModuleHandlerConfiguration
 : public Stream_ModuleHandlerConfiguration
{
  inline DHCP_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   //////////////////////////////////////
   , printProgressDot (DHCP_DEFAULT_PRINT_PROGRESSDOT)
   , protocolConfiguration (NULL)
   , streamConfiguration (NULL)
  {
    crunchMessages = DHCP_DEFAULT_CRUNCH_MESSAGES; // http parser module

    traceParsing = DHCP_DEFAULT_YACC_TRACE; // http parser module
    traceScanning = DHCP_DEFAULT_LEX_TRACE; // http parser module
  };

  bool                        printProgressDot;  // print dot '.' (stdlog) for received messages
  DHCP_ProtocolConfiguration* protocolConfiguration;

  DHCP_StreamConfiguration*   streamConfiguration;
};

struct DHCP_StreamConfiguration
 : Stream_Configuration
{
  inline DHCP_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , protocolConfiguration (NULL)
   //, userData (NULL)
  {
    bufferSize = DHCP_BUFFER_SIZE;
  };

  Stream_ModuleConfiguration*      moduleConfiguration;        // stream module configuration
  DHCP_ModuleHandlerConfiguration* moduleHandlerConfiguration; // module handler configuration
  DHCP_ProtocolConfiguration*      protocolConfiguration;      // protocol configuration

  //DHCP_Stream_UserData*           userData;
};

#endif
