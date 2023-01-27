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

#include "common_parser_common.h"

#include "net_common.h"
#include "net_connection_configuration.h"

#include "irc_common.h"
#include "irc_defines.h"

// forward declarations
class IRC_IControl;

struct IRC_AllocatorConfiguration
 : Common_Parser_FlexAllocatorConfiguration
{
  IRC_AllocatorConfiguration ()
   : Common_Parser_FlexAllocatorConfiguration ()
  {
    defaultBufferSize = IRC_MAXIMUM_FRAME_SIZE;
  }
};

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

typedef string_list_t channels_t;
typedef channels_t::iterator channels_iterator_t;
typedef channels_t::const_iterator channels_const_iterator_t;
typedef std::map<std::string, IRC_ChannelModes_t> channel_modes_t;
typedef channel_modes_t::iterator channel_modes_iterator_t;
struct IRC_SessionState
 : IRC_ConnectionState
{
  IRC_SessionState ()
   : IRC_ConnectionState ()
   , activeChannel ()
   , away (false)
   , channels ()
   , channelModes ()
   , isFirstMessage (false)
   , nickName ()
   , userModes ()
  {}

  // *TODO*: remove this
  std::string     activeChannel;
  bool            away;
  channels_t      channels;
  channel_modes_t channelModes;
  bool            isFirstMessage;
  std::string     nickName;
  IRC_UserModes_t userModes;
};

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct IRC_SessionState,
                          IRC_Statistic_t> IRC_IConnection_t;

#endif
