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

#ifndef IRC_CLIENT_CONFIGURATION_H
#define IRC_CLIENT_CONFIGURATION_H

#include <string>

#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_inotify.h"

#include "stream_common.h"

#include "net_defines.h"
#include "net_iconnectionmanager.h"

//#include "net_configuration.h"

#include "IRC_client_common.h"
#include "IRC_client_defines.h"

// forward declarations
struct IRC_Client_Configuration;
struct IRC_Client_CursesState;
struct IRC_Client_ConnectionState;
class IRC_Client_IIRCControl;
class IRC_Client_IRCMessage;
struct IRC_Client_ModuleHandlerConfiguration;
class IRC_Client_SessionMessage;
//class IRC_Client_Stream;
struct IRC_Client_StreamSessionData;
struct IRC_Client_StreamUserData;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          IRC_Client_Configuration,
//                          IRC_Client_ConnectionState,
//                          IRC_Client_RuntimeStatistic_t,
//                          IRC_Client_Stream> IRC_Client_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 IRC_Client_Configuration,
                                 IRC_Client_ConnectionState,
                                 IRC_Client_RuntimeStatistic_t,
                                 ////////
                                 IRC_Client_StreamUserData> IRC_Client_IConnection_Manager_t;
typedef Common_INotify_T<IRC_Client_StreamSessionData,
                         IRC_Client_IRCMessage,
                         IRC_Client_SessionMessage> IRC_Client_IStreamNotify_t;

struct IRC_Client_SocketHandlerConfiguration
 : public Net_SocketHandlerConfiguration
{
  inline IRC_Client_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   //////////////////////////////////////
   , userData (NULL)
  {
    PDUSize = IRC_CLIENT_BUFFER_SIZE;
  };

  IRC_Client_StreamUserData* userData;
};

//struct IRC_Client_ConnectorConfiguration
//{
//  inline IRC_Client_ConnectorConfiguration ()
//   : /*configuration (NULL)
//   ,*/ connectionManager (NULL)
//   , socketHandlerConfiguration (NULL)
//   //, statisticCollectionInterval (0)
//  {};
//
//  //IRC_Client_Configuration*              configuration;
//  IRC_Client_IConnection_Manager_t*      connectionManager;
//  IRC_Client_SocketHandlerConfiguration* socketHandlerConfiguration;
//  unsigned int                           statisticCollectionInterval; // statistics collecting interval (second(s)) [0: off]
//};

struct IRC_Client_IRCLoginOptions
{
  inline IRC_Client_IRCLoginOptions ()
   : passWord ()
   , nickName (IRC_CLIENT_DEF_IRC_NICKNAME)
   , user ()
   , channel ()
  {};

  std::string passWord;
  std::string nickName;
  struct User
  {
    inline User ()
     : userName ()
     , hostName ()
     , serverName ()
     , realName ()
    {};

    std::string userName;
    struct Hostname
    {
      inline Hostname ()
       : discriminator (INVALID)
       , string (NULL)
      {};

      enum
      {
        INVALID = -1,
        MODE = 0,
        STRING,
      } discriminator;
      union
      {
        // *NOTE*: "modern" connects (see RFC2812 Section 3.1.3)
        unsigned char mode;
        // *NOTE*: "traditional" connects (see RFC1459 Section 4.1.3)
        std::string*  string;
      };
    } hostName;
    std::string serverName;
    std::string realName;
  } user;
  std::string channel;
};

struct IRC_Client_ProtocolConfiguration
{
  inline IRC_Client_ProtocolConfiguration ()
   : automaticPong (IRC_CLIENT_STREAM_DEF_AUTOPONG)
   , loginOptions ()
   , printPingDot (IRC_CLIENT_DEF_PRINT_PINGDOT)
  {};

  bool                       automaticPong; // automatically answer "ping" messages
  IRC_Client_IRCLoginOptions loginOptions;
  bool                       printPingDot;  // print dot ('.') for every answered PING to stdlog
};

struct IRC_Client_ModuleHandlerConfiguration
 : public Stream_ModuleHandlerConfiguration
{
  inline IRC_Client_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   //////////////////////////////////////
   , active (false)
   //////////////////////////////////////
   , crunchMessages (IRC_CLIENT_DEF_CRUNCH_MESSAGES)
   , traceParsing (IRC_CLIENT_DEF_YACC_TRACE)
   , traceScanning (IRC_CLIENT_DEF_LEX_TRACE)
   //////////////////////////////////////
   , protocolConfiguration (NULL)
   , streamConfiguration (NULL)
   //////////////////////////////////////
   , subscriber (NULL)
   , userData (NULL)
  {};

  /* socket handler */
  bool                              active;

  /* splitter */
  // *NOTE*: this option may be useful for (downstream) parsers that only work
  //         on CONTIGUOUS buffers (i.e. cannot parse chained message blocks)
  // *WARNING*: currently, this does NOT work with multithreaded streams
  //            --> USE WITH CAUTION !
  bool                              crunchMessages;
  bool                              traceParsing;       // debug yacc (bison) ?
  bool                              traceScanning;      // debug (f)lex ?

  IRC_Client_ProtocolConfiguration* protocolConfiguration;

  Stream_Configuration*             streamConfiguration;

  /* handler */
  IRC_Client_IStreamNotify_t*       subscriber; // (initial) subscriber
  IRC_Client_StreamUserData*        userData;
};

struct IRC_Client_StreamConfiguration
 : Stream_Configuration
{
  inline IRC_Client_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration_2 ()
   , moduleHandlerConfiguration_2 ()
   , protocolConfiguration (NULL)
   , userData (NULL)
  {
    bufferSize = IRC_CLIENT_BUFFER_SIZE;
  };

  Stream_ModuleConfiguration            moduleConfiguration_2;        // stream module configuration
  IRC_Client_ModuleHandlerConfiguration moduleHandlerConfiguration_2; // module handler configuration
  IRC_Client_ProtocolConfiguration*     protocolConfiguration;        // protocol configuration

  IRC_Client_StreamUserData*            userData;
};

struct IRC_Client_InputHandlerConfiguration
{
  inline IRC_Client_InputHandlerConfiguration ()
   : controller (NULL)
   , streamConfiguration (NULL)
  {};

  IRC_Client_IIRCControl* controller;
  Stream_Configuration*   streamConfiguration;
};

struct IRC_Client_Configuration
{
  inline IRC_Client_Configuration ()
   : socketConfiguration ()
   , socketHandlerConfiguration ()
   //////////////////////////////////////
   , streamConfiguration ()
   , streamUserData ()
   //////////////////////////////////////
   , protocolConfiguration ()
   //////////////////////////////////////
   , cursesState (NULL)
   , encoding (IRC_CLIENT_DEF_ENCODING)
   , groupID (COMMON_EVENT_THREAD_GROUP_ID)
   , logToFile (IRC_CLIENT_SESSION_DEF_LOG)
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // ****************************** socket *************************************
  Net_SocketConfiguration               socketConfiguration;
  IRC_Client_SocketHandlerConfiguration socketHandlerConfiguration;
  // ****************************** stream *************************************
  IRC_Client_StreamConfiguration        streamConfiguration;
  IRC_Client_StreamUserData*            streamUserData;
  // ***************************** protocol ************************************
  IRC_Client_ProtocolConfiguration      protocolConfiguration;
  // ***************************************************************************
  // *TODO*: move this somewhere else
  IRC_Client_CursesState*               cursesState;
  // *NOTE*: see also https://en.wikipedia.org/wiki/Internet_Relay_Chat#Character_encoding
  // *TODO*: implement support for 7-bit ASCII (as it is the most compatible
  //         encoding)
  IRC_Client_CharacterEncoding          encoding;
  int                                   groupID;
  bool                                  logToFile;
  bool                                  useReactor;
};

#endif
