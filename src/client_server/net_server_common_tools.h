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

#ifndef NET_SERVER_COMMON_TOOLS_H
#define NET_SERVER_COMMON_TOOLS_H

#include "ace/Global_Macros.h"

#include "net_server_common.h"
#include "net_server_exports.h"

class Net_Server_Export Net_Server_Common_Tools
{
 public:
  static Net_Server_IListener_t* getListenerSingleton ();
  static Net_Server_IListener_t* getAsynchListenerSingleton ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Net_Server_Common_Tools ());
  ACE_UNIMPLEMENTED_FUNC (virtual ~Net_Server_Common_Tools ());
  ACE_UNIMPLEMENTED_FUNC (Net_Server_Common_Tools (const Net_Server_Common_Tools&));
  ACE_UNIMPLEMENTED_FUNC (Net_Server_Common_Tools& operator= (const Net_Server_Common_Tools&));
};

#endif
