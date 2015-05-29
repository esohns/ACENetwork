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

#include "stream_common.h"

#include "IRC_client_common.h"
#include "IRC_client_IRCmessage.h"

// forward declaration(s)
class Stream_IAllocator;

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

  static IRC_Client_CommandType_t IRCCommandString2Type (const std::string&);
  static void merge (const std::string&,       // mode string (e.g. "+i")
                     IRC_Client_UserModes_t&); // input/return value: (merged) user modes
  static void merge (const std::string&,          // mode string (e.g. "+i")
                     IRC_Client_ChannelModes_t&); // input/return value: (merged) channel modes
  static char IRCChannelMode2Char (const IRC_Client_ChannelMode&);
  static char IRCUserMode2Char (const IRC_Client_UserMode&);

  static bool isValidIRCChannelName (const std::string&); // string

  static bool connect (Stream_IAllocator*,                // message allocator
                       const IRC_Client_IRCLoginOptions&, // login options
                       bool,                              // debug scanner ?
                       bool,                              // debug parser ?
                       unsigned int,                      // statistics reporting interval (0: OFF)
                       const std::string&,                // hostname
                       unsigned short,                    // port
                       Stream_Module_t*);                 // final module

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Tools ());
  ACE_UNIMPLEMENTED_FUNC (~IRC_Client_Tools ());
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Tools (const IRC_Client_Tools&));
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Tools& operator= (const IRC_Client_Tools&));

  // helper methods
  static IRC_Client_UserMode IRCUserModeChar2UserMode (char);
  static IRC_Client_ChannelMode IRCChannelModeChar2ChannelMode (char);
  static std::string concatParams (const IRC_Client_Parameters_t&, // parameters
                                   int = 0);                       // starting index (-1: LAST parameter)
};

#endif
