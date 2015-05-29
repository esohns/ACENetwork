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

typedef Stream_Statistic_t IRC_Client_RuntimeStatistic;
struct IRC_Client_SessionData
{
  inline IRC_Client_SessionData ()
   : currentStatistics ()
   , lastCollectionTimestamp (ACE_Time_Value::zero)
  {};

  IRC_Client_RuntimeStatistic currentStatistics;
  ACE_Time_Value              lastCollectionTimestamp;
};

struct IRC_Client_StreamConfiguration
{
  inline IRC_Client_StreamConfiguration ()
   : crunchMessageBuffers (IRC_CLIENT_DEF_CRUNCH_MESSAGES)
   , debugScanner (IRC_CLIENT_DEF_TRACE_SCANNING)
   , debugParser (IRC_CLIENT_DEF_TRACE_PARSING)
  {};

  bool crunchMessageBuffers;
  bool debugScanner;
  bool debugParser;
};

struct IRC_Client_ProtocolConfiguration
{
  inline IRC_Client_ProtocolConfiguration ()
   : loginOptions ()
   , streamConfiguration ()
  {};

  IRC_Client_IRCLoginOptions     loginOptions;
  IRC_Client_StreamConfiguration streamConfiguration;
};

struct IRC_Client_Configuration
{
  inline IRC_Client_Configuration ()
   : socketConfiguration ()
   , streamConfiguration ()
   , streamSessionData ()
   , protocolConfiguration ()
  {};

  // **************************** socket data **********************************
  Net_SocketConfiguration_t        socketConfiguration;
  // **************************** stream data **********************************
  Stream_Configuration_t           streamConfiguration;
  IRC_Client_SessionData           streamSessionData;
  // *************************** protocol data *********************************
  IRC_Client_ProtocolConfiguration protocolConfiguration;
};

#endif
