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

#include "stream_common.h"

#include "IRC_client_common.h"
#include "IRC_client_network.h"

// forward declaration(s)
struct IRC_LoginOptions;

class IRC_Client_Tools
{
 public:
  static void parseConfigurationFile (const std::string&,
                                      IRC_LoginOptions&,
                                      IRC_Client_Connections_t&);

  // *NOTE*: iff (delete module == true), fire-and-forget the last argument !
  static ACE_HANDLE connect (IRC_Client_IConnector_t&, // connector handle
                             const ACE_INET_Addr&,     // peer address
                             const IRC_LoginOptions&,  // login options
                             bool,                     // clone module ?
                             bool,                     // delete module ?
                             Stream_Module_t*&);       // (final) module handle

 private:
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Tools ())
  ACE_UNIMPLEMENTED_FUNC (~IRC_Client_Tools ())
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Tools (const IRC_Client_Tools&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Client_Tools& operator= (const IRC_Client_Tools&))
};

#endif
