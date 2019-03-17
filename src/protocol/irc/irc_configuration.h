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

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "irc_common.h"
#include "irc_defines.h"
#include "irc_icontrol.h"
#include "irc_network.h"
#include "irc_stream_common.h"

// forward declarations
class IRC_Record;
struct IRC_ModuleHandlerConfiguration;
//class IRC_SessionMessage;
struct IRC_Stream_SessionData;

//struct IRC_AllocatorConfiguration
// : Stream_AllocatorConfiguration
//{
//  IRC_AllocatorConfiguration ()
//   : Stream_AllocatorConfiguration ()
//  {
//    defaultBufferSize = IRC_MAXIMUM_FRAME_SIZE;
//    // *NOTE*: this facilitates (message block) data buffers to be scanned with
//    //         'flex's yy_scan_buffer() method
//    paddingBytes = COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE;
//  }
//};

struct IRC_ProtocolConfiguration
{
  IRC_ProtocolConfiguration ()
   : automaticPong (IRC_STREAM_DEFAULT_AUTOPONG)
   , loginOptions ()
   , printPingDot (IRC_CLIENT_DEFAULT_PRINT_PINGDOT)
  {}

  bool                    automaticPong; // automatically answer "ping" messages
  struct IRC_LoginOptions loginOptions;
  bool                    printPingDot;  // print dot '.' (stdlog) for answered PINGs
};

struct IRC_StreamConfiguration;
struct IRC_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  IRC_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   //, connectionConfigurations (NULL)
   , protocolConfiguration (NULL)
   , printProgressDot (false)
   , streamConfiguration (NULL)
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_CONCURRENT;

    // *NOTE*: this option may be useful for (downstream) parsers that only work
    //         on CONTIGUOUS buffers (i.e. cannot parse chained message blocks)
    // *WARNING*: currently, this does NOT work with multithreaded streams
    //            --> USE WITH CAUTION !
    crunchMessages = IRC_DEFAULT_CRUNCH_MESSAGES;
    inbound = true;
    passive = false;
  }

  //IRC_ConnectionConfigurations_t*   connectionConfigurations;
  struct IRC_ProtocolConfiguration* protocolConfiguration;
  bool                              printProgressDot; // file writer module
  struct IRC_StreamConfiguration*   streamConfiguration;
};
//typedef std::map<std::string,
//                 struct IRC_ModuleHandlerConfiguration> IRC_ModuleHandlerConfigurations_t;
//typedef IRC_ModuleHandlerConfigurations_t::const_iterator IRC_ModuleHandlerConfigurationsConstIterator_t;

struct IRC_StreamConfiguration
 : Stream_Configuration
{
  IRC_StreamConfiguration ()
   : Stream_Configuration ()
   //, moduleConfiguration_2 ()
   //, moduleHandlerConfigurations ()
   , protocolConfiguration (NULL)
   , userData (NULL)
  {}

  //struct Stream_ModuleConfiguration moduleConfiguration_2;       // stream module configuration
  //IRC_ModuleHandlerConfigurations_t moduleHandlerConfigurations; // module handler configuration
  struct IRC_ProtocolConfiguration* protocolConfiguration;       // protocol configuration

  struct Net_UserData*              userData;
};

#endif
