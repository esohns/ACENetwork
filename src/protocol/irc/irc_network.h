/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns                                      *
 *   erik.sohns@web.de                                                     *
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

#ifndef IRC_NETWORK_H
#define IRC_NETWORK_H

//#include <map>
//#include <string>

#include "net_common.h"
#include "net_connection_configuration.h"

#include "irc_common.h"
#include "irc_defines.h"

// forward declarations
class IRC_IControl;

struct IRC_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  IRC_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    defaultBufferSize = IRC_MAXIMUM_FRAME_SIZE;
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    paddingBytes = COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE;
  }
};

//struct IRC_StreamConfiguration;
//class IRC_ConnectionConfiguration
// : public Net_ConnectionConfiguration_T<struct IRC_AllocatorConfiguration,
//                                        struct IRC_StreamConfiguration,
//                                        NET_TRANSPORTLAYER_TCP>
//{
//  IRC_ConnectionConfiguration ()
//   : Net_ConnectionConfiguration_T ()
//   ///////////////////////////////////////
//   , protocolConfiguration (NULL)
//  {
//    PDUSize = IRC_MAXIMUM_FRAME_SIZE;
//  }

//  struct IRC_ProtocolConfiguration* protocolConfiguration;
//};
//typedef std::map<std::string,
//                 struct IRC_ConnectionConfiguration> IRC_ConnectionConfigurations_t;
//typedef IRC_ConnectionConfigurations_t::iterator IRC_ConnectionConfigurationIterator_t;

struct IRC_ConnectionState
 : Net_ConnectionState
{
  IRC_ConnectionState ()
   : Net_ConnectionState ()
   , controller (NULL)
   , statistic ()
  {}

  IRC_IControl*   controller;
  IRC_Statistic_t statistic;
};

struct IRC_SessionState
 : IRC_ConnectionState
{
  IRC_SessionState ()
   : IRC_ConnectionState ()
   , away (false)
   , channel ()
   , channelModes ()
   , isFirstMessage (false)
   , nickName ()
   , userModes ()
  {}

  // *TODO*: remove this
  bool               away;
  std::string        channel;
  IRC_ChannelModes_t channelModes;
  bool               isFirstMessage;
  std::string        nickName;
  IRC_UserModes_t    userModes;
};

#endif
