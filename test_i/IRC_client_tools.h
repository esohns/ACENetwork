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

#ifndef IRC_CLIENT_TOOLS_H
#define IRC_CLIENT_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"

#include "IRC_client_common.h"
#include "IRC_client_IRCmessage.h"
#include "IRC_client_IRC_codes.h"
#include "IRC_client_network.h"

// forward declaration(s)
struct IRC_Client_IRCLoginOptions;
struct IRC_Client_ModuleHandlerConfiguration;

class IRC_Client_Tools
{
 public:
  // debug info
  static std::string dump (const IRC_Client_IRCMessage&);
  static std::string dump (const IRC_Client_UserModes_t&);
  static std::string dump (const IRC_Client_ChannelModes_t&);

  static std::string IRCCode2String (const IRC_Client_IRCNumeric_t&);
  static std::string IRCChannelMode2String (const IRC_Client_ChannelMode&);
  static std::string IRCUserMode2String (const IRC_Client_UserMode&);
  static std::string IRCMessage2String (const IRC_Client_IRCMessage&);

  static void parseConfigurationFile (const std::string&,
                                      IRC_Client_IRCLoginOptions&,
                                      IRC_Client_Connections_t&);

  static IRC_Client_CommandType_t IRCCommandString2Type (const std::string&);
  static unsigned int merge (const std::string&,       // mode string (e.g. "+i")
                             IRC_Client_UserModes_t&); // input/return value: (merged) user modes
  static unsigned int merge (const std::string&,          // mode string (e.g. "+i")
                             IRC_Client_ChannelModes_t&); // input/return value: (merged) channel modes
  static char IRCChannelMode2Char (const IRC_Client_ChannelMode&);
  static char IRCUserMode2Char (const IRC_Client_UserMode&);

  static bool isValidIRCChannelName (const std::string&); // string

  // *NOTE*: iff (delete module == true), fire-and-forget the last argument !
  static ACE_HANDLE connect (IRC_Client_IConnector_t&,          // connector handle
                             const ACE_INET_Addr&,              // peer address
                             const IRC_Client_IRCLoginOptions&, // login options
                             bool,                              // clone module ?
                             bool,                              // delete module ?
                             Stream_Module_t*&);                // (final) module handle

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Tools ())
  ACE_UNIMPLEMENTED_FUNC (~IRC_Client_Tools ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Tools (const IRC_Client_Tools&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Tools& operator= (const IRC_Client_Tools&))

  // helper methods
  static IRC_Client_UserMode IRCUserModeChar2UserMode (char);
  static IRC_Client_ChannelMode IRCChannelModeChar2ChannelMode (char);
  static std::string stringify (const IRC_Client_Parameters_t&, // parameters
                                int = 0);                       // starting index (-1: LAST parameter)
};

#endif
