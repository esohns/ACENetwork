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

#ifndef POP_CONFIGURATION_H
#define POP_CONFIGURATION_H

#include "ace/INET_Addr.h"

#include "common_parser_common.h"

#include "stream_common.h"

#include "net_common.h"
#include "net_iconnectionmanager.h"

#include "pop_common.h"

// forward declarations
class POP_ConnectionConfiguration;
struct POP_ConnectionState;
struct POP_Record;
struct POP_ModuleHandlerConfiguration;
struct POP_Stream_SessionData;

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct POP_Configuration,
//                          struct POP_ConnectionState,
//                          struct Net_StreamStatistic,
//                          POP_Stream> POP_IConnection_t;
//typedef Net_IConnectionManager_T<ACE_INET_Addr,
//                                 POP_ConnectionConfiguration,
//                                 struct POP_ConnectionState,
//                                 struct Net_StreamStatistic,
//                                 struct Net_UserData> POP_IConnection_Manager_t;

struct POP_AllocatorConfiguration
 : Common_Parser_FlexAllocatorConfiguration
{
  POP_AllocatorConfiguration ()
   : Common_Parser_FlexAllocatorConfiguration ()
  {}
};

//typedef Common_INotify_T<unsigned int,
//                         struct POP_Stream_SessionData,
//                         POP_Record,
//                         POP_SessionMessage> POP_IStreamNotify_t;

struct POP_ProtocolConfiguration
{
  POP_ProtocolConfiguration ()
   : domain (static_cast<u_short> (0),
             ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
             AF_INET)
   , username ()
   , password ()
  {}

  ACE_INET_Addr domain; // i.e. external address
  std::string   username;
  std::string   password;
};

struct POP_ModuleHandlerConfiguration
 : virtual Stream_ModuleHandlerConfiguration
{
  POP_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   ///////////////////////////////////////
   , request (NULL)
   , protocolConfiguration (NULL)
  {}

  struct POP_Request*               request;
  struct POP_ProtocolConfiguration* protocolConfiguration;
};

struct POP_StreamConfiguration
 : virtual Stream_Configuration
{
  POP_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration (NULL)
   //, moduleHandlerConfigurations ()
   , protocolConfiguration (NULL)
  {}

  struct Stream_ModuleConfiguration* moduleConfiguration;   // stream module configuration
  struct POP_ProtocolConfiguration* protocolConfiguration; // protocol configuration
};

typedef Stream_Configuration_T<//stream_name_string_,
                               struct POP_StreamConfiguration,
                               struct POP_ModuleHandlerConfiguration> POP_StreamConfiguration_t;

#endif
