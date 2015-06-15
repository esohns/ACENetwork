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

#include "ace/Time_Value.h"

#include "stream_common.h"

#include "net_configuration.h"

#include "IRC_client_common.h"
#include "IRC_client_defines.h"

// forward declarations
class IRC_Client_IIRCControl;

typedef Stream_Statistic IRC_Client_RuntimeStatistic_t;
struct IRC_Client_SessionData
{
  inline IRC_Client_SessionData ()
   : currentStatistics ()
   , lastCollectionTimestamp (ACE_Time_Value::zero)
  {};

  IRC_Client_RuntimeStatistic_t currentStatistics;
  ACE_Time_Value                lastCollectionTimestamp;
};

struct IRC_Client_ProtocolConfiguration
{
  inline IRC_Client_ProtocolConfiguration ()
   : automaticPong (IRC_CLIENT_STREAM_DEF_AUTOPONG)
   , loginOptions ()
   , printPingDot (IRC_CLIENT_DEF_PRINT_PINGDOT)
  {};

  bool                       automaticPong;
  IRC_Client_IRCLoginOptions loginOptions;
  bool                       printPingDot;
};

struct IRC_Client_StreamConfiguration
{
  inline IRC_Client_StreamConfiguration ()
   : crunchMessageBuffers (IRC_CLIENT_DEF_CRUNCH_MESSAGES)
   , debugScanner (IRC_CLIENT_DEF_LEX_TRACE)
   , debugParser (IRC_CLIENT_DEF_YACC_TRACE)
   , sessionID (0)
   , streamConfiguration ()
   , sessionData (NULL)
   , protocolConfiguration (NULL)
  {};

  bool                              crunchMessageBuffers;  // crunch message buffers ?
  bool                              debugScanner;          // debug yacc ?
  bool                              debugParser;           // debug lex ?
  unsigned int                      sessionID;             // session ID
  Stream_Configuration              streamConfiguration;   // stream configuration
  IRC_Client_SessionData*           sessionData;           // session data
  IRC_Client_ProtocolConfiguration* protocolConfiguration; // protocol configuration
};

struct IRC_Client_Configuration
{
  inline IRC_Client_Configuration ()
   : socketConfiguration ()
   , streamConfiguration ()
   //, streamSessionData ()
   , protocolConfiguration ()
  {};

  // **************************** socket data **********************************
  Net_SocketConfiguration          socketConfiguration;
  // **************************** stream data **********************************
  IRC_Client_StreamConfiguration   streamConfiguration;
  //IRC_Client_SessionData*          streamSessionData;
  // *************************** protocol data *********************************
  IRC_Client_ProtocolConfiguration protocolConfiguration;
  // ***************************************************************************
  IRC_Client_IIRCControl*          controller;
};

#endif
