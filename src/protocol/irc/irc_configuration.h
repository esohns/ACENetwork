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

#ifndef IRC_CONFIGURATION_H
#define IRC_CONFIGURATION_H

#include <string>

#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_inotify.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "irc_common.h"
#include "irc_defines.h"
#include "irc_icontrol.h"
#include "irc_stream_common.h"

// forward declarations
struct IRC_ConnectionState;
class IRC_Record;
struct IRC_ModuleHandlerConfiguration;
class IRC_SessionMessage;
struct IRC_Stream_SessionData;
struct IRC_Stream_UserData;

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          IRC_Configuration,
//                          IRC_ConnectionState,
//                          IRC_RuntimeStatistic_t,
//                          IRC_Stream> IRC_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 IRC_Configuration,
                                 IRC_ConnectionState,
                                 IRC_RuntimeStatistic_t,
                                 ////////
                                 IRC_Stream_UserData> IRC_IConnection_Manager_t;

typedef Common_INotify_T<unsigned int,
                         IRC_Stream_SessionData,
                         IRC_Record,
                         IRC_SessionMessage> IRC_IStreamNotify_t;
//typedef IRC_IControl_T<IRC_IStreamNotify_t> IRC_IControl_t;

struct IRC_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline IRC_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   //////////////////////////////////////
   , userData (NULL)
  {
    PDUSize = IRC_BUFFER_SIZE;
  };

  IRC_Stream_UserData* userData;
};

//struct IRC_ConnectorConfiguration
//{
//  inline IRC_ConnectorConfiguration ()
//   : /*configuration (NULL)
//   ,*/ connectionManager (NULL)
//   , socketHandlerConfiguration (NULL)
//   //, statisticCollectionInterval (0)
//  {};
//
//  //IRC_Configuration*              configuration;
//  IRC_IConnection_Manager_t*      connectionManager;
//  IRC_SocketHandlerConfiguration* socketHandlerConfiguration;
//  unsigned int                    statisticCollectionInterval; // statistic collecting interval (second(s)) [0: off]
//};

struct IRC_ProtocolConfiguration
{
  inline IRC_ProtocolConfiguration ()
   : automaticPong (IRC_STREAM_DEFAULT_AUTOPONG)
   , loginOptions ()
   , printPingDot (IRC_CLIENT_DEFAULT_PRINT_PINGDOT)
  {};

  bool             automaticPong; // automatically answer "ping" messages
  IRC_LoginOptions loginOptions;
  bool             printPingDot;  // print dot '.' (stdlog) for answered PINGs
};

struct IRC_ModuleHandlerConfiguration
 : public Stream_ModuleHandlerConfiguration
{
  inline IRC_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   //////////////////////////////////////
   , active (false)
   //////////////////////////////////////
   , crunchMessages (IRC_DEFAULT_CRUNCH_MESSAGES)
   , traceParsing (IRC_DEFAULT_YACC_TRACE)
   , traceScanning (IRC_DEFAULT_LEX_TRACE)
   //////////////////////////////////////
   , printFinalReport (true)
   , printProgressDot (false)
   , pushStatisticMessages (true)
   , protocolConfiguration (NULL)
   , streamConfiguration (NULL)
  {};

  /* socket handler */
  bool                       active;

  /* splitter */
  // *NOTE*: this option may be useful for (downstream) parsers that only work
  //         on CONTIGUOUS buffers (i.e. cannot parse chained message blocks)
  // *WARNING*: currently, this does NOT work with multithreaded streams
  //            --> USE WITH CAUTION !
  bool                       crunchMessages;
  bool                       traceParsing;       // debug yacc (bison) ?
  bool                       traceScanning;      // debug (f)lex ?

  bool                       printFinalReport;
  bool                       printProgressDot; // file writer module
  bool                       pushStatisticMessages;
  IRC_ProtocolConfiguration* protocolConfiguration;

  Stream_Configuration*      streamConfiguration;
};

struct IRC_StreamConfiguration
 : Stream_Configuration
{
  inline IRC_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration_2 ()
   , moduleHandlerConfiguration_2 ()
   , protocolConfiguration (NULL)
   //, userData (NULL)
  {
    bufferSize = IRC_BUFFER_SIZE;
  };

  Stream_ModuleConfiguration     moduleConfiguration_2;        // stream module configuration
  IRC_ModuleHandlerConfiguration moduleHandlerConfiguration_2; // module handler configuration
  IRC_ProtocolConfiguration*     protocolConfiguration;        // protocol configuration

  //IRC_Stream_UserData*           userData;
};

#endif
