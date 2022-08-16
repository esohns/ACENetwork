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

#ifndef PCP_CONFIGURATION_H
#define PCP_CONFIGURATION_H

#include <deque>
#include <map>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_inotify.h"

#include "common_parser_common.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "pcp_common.h"
#include "pcp_defines.h"
#include "pcp_stream_common.h"

// forward declarations
struct PCP_ConnectionState;
struct PCP_Record;
struct PCP_ModuleHandlerConfiguration;
class PCP_SessionMessage;
struct PCP_Stream_SessionData;

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct PCP_Configuration,
//                          struct PCP_ConnectionState,
//                          PCP_Statistic_t,
//                          PCP_Stream> PCP_IConnection_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct PCP_ConnectionConfiguration,
                                 struct PCP_ConnectionState,
                                 PCP_Statistic_t,
                                 struct Net_UserData> PCP_IConnection_Manager_t;

struct PCP_AllocatorConfiguration
 : Common_Parser_FlexAllocatorConfiguration
{
  PCP_AllocatorConfiguration ()
   : Common_Parser_FlexAllocatorConfiguration ()
  {
    defaultBufferSize = PCP_MESSAGE_SIZE;
  }
};

//typedef Common_INotify_T<unsigned int,
//                         struct PCP_Stream_SessionData,
//                         PCP_Record,
//                         PCP_SessionMessage> PCP_IStreamNotify_t;

struct PCP_ProtocolConfiguration
{
  PCP_ProtocolConfiguration ()
  {}
};

struct PCP_StreamConfiguration;
struct PCP_ModuleHandlerConfiguration
 : public Stream_ModuleHandlerConfiguration
{
  PCP_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   ///////////////////////////////////////
   , printProgressDot (PCP_DEFAULT_PRINT_PROGRESSDOT)
   , parserConfiguration (NULL)
   , protocolConfiguration (NULL)
  {
    printFinalReport = true;
  }

  bool                                        printProgressDot; // file writer module
  struct Common_FlexBisonParserConfiguration* parserConfiguration; // parser module(s)
  struct PCP_ProtocolConfiguration*           protocolConfiguration;
};
typedef std::map<std::string,
                 struct PCP_ModuleHandlerConfiguration*> PCP_ModuleHandlerConfigurations_t;
typedef PCP_ModuleHandlerConfigurations_t::const_iterator PCP_ModuleHandlerConfigurationsConstIterator_t;

struct PCP_StreamConfiguration
 : Stream_Configuration
{
  PCP_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration ()
   , moduleHandlerConfigurations ()
   , protocolConfiguration (NULL)
  {}

  struct Stream_ModuleConfiguration* moduleConfiguration;         // stream module configuration
  PCP_ModuleHandlerConfigurations_t  moduleHandlerConfigurations; // module handler configuration
  struct PCP_ProtocolConfiguration*  protocolConfiguration;       // protocol configuration
};

#endif
