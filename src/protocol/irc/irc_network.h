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

#include <map>
#include <string>

#include "ace/INET_Addr.h"

#include "common_parser_common.h"

#include "net_common.h"
#include "net_iconnection.h"

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
   , protocolConfiguration (NULL)
   , statistic ()
  {}

  // *TODO*: remove these two ASAP
  IRC_IControl*                     controller;
  struct IRC_ProtocolConfiguration* protocolConfiguration;

  IRC_Statistic_t                   statistic;
};

typedef string_list_t channels_t;
typedef channels_t::iterator channels_iterator_t;
typedef channels_t::const_iterator channels_const_iterator_t;
typedef std::map<std::string, IRC_ChannelModes_t> channel_modes_t;
typedef channel_modes_t::iterator channel_modes_iterator_t;
typedef std::map<std::string, ACE_UINT16> channel_to_number_of_users_t;
typedef channel_to_number_of_users_t::iterator channel_to_number_of_users_iterator_t;
typedef std::map<std::string, string_list_t> channel_to_names_t;
typedef channel_to_names_t::iterator channel_to_names_iterator_t;
struct IRC_SessionState
 : IRC_ConnectionState
{
  IRC_SessionState ()
   : IRC_ConnectionState ()
   , channel ()
   , away (false)
   , channels ()
   , channelModes ()
   , isFirstMessage (false)
   , nickName ()
   , userModes ()
   , serverExtensions ()
  {}

  std::string                  channel;
  bool                         away;
  channels_t                   channels;
  channel_modes_t              channelModes;
  bool                         isFirstMessage;
  std::string                  nickName;
  IRC_UserModes_t              userModes;

  channel_to_names_t           channelToNames;
  channel_to_number_of_users_t channelToNumberOfUsers;

  server_extensions_t          serverExtensions;
};

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct IRC_SessionState,
                          IRC_Statistic_t> IRC_IConnection_t;

#endif
