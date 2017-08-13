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

#ifndef IRC_TOOLS_H
#define IRC_TOOLS_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"

#include "irc_common.h"
#include "irc_exports.h"
#include "irc_record.h"
#include "irc_codes.h"

// forward declaration(s)
struct IRC_LoginOptions;
struct IRC_ModuleHandlerConfiguration;

class IRC_Export IRC_Tools
{
 public:
  inline virtual ~IRC_Tools () {};

  // debug info
  static std::string dump (const IRC_Record&);
  static std::string dump (const IRC_UserModes_t&);
  static std::string dump (const IRC_ChannelModes_t&);

  static std::string CommandToString (const IRC_NumericCommand_t&);
  static std::string ChannelModeToString (const IRC_ChannelMode&);
  static std::string UserModeToString (const IRC_UserMode&);
  static std::string RecordToString (const IRC_Record&);

  static IRC_CommandType_t CommandToType (const std::string&);
  static unsigned int merge (const std::string&, // mode string (e.g. "+i")
                             IRC_UserModes_t&);  // input/return value: (merged) user modes
  static unsigned int merge (const std::string&,   // mode string (e.g. "+i")
                             IRC_ChannelModes_t&); // input/return value: (merged) channel modes
  static char ChannelModeToChar (const IRC_ChannelMode&);
  static char UserModeToChar (const IRC_UserMode&);

  static bool isValidChannelName (const std::string&); // string

  //// *NOTE*: iff (delete module == true), fire-and-forget the last argument !
  //static ACE_HANDLE connect (IRC_IConnector_t&,       // connector handle
  //                           const ACE_INET_Addr&,    // peer address
  //                           const IRC_LoginOptions&, // login options
  //                           bool,                    // clone module ?
  //                           bool,                    // delete module ?
  //                           Stream_Module_t*&);      // (final) module handle

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Tools ())
  //ACE_UNIMPLEMENTED_FUNC (~IRC_Tools ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Tools (const IRC_Tools&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Tools& operator= (const IRC_Tools&))

  // helper methods
  static IRC_UserMode CharToUserMode (char);
  static IRC_ChannelMode CharToChannelMode (char);
  static std::string stringify (const IRC_Parameters_t&, // parameters
                                int = 0);                // starting index (-1: LAST parameter)
};

#endif
