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
#include "stdafx.h"

#include "net_server_common_tools.h"

#include "net_macros.h"

Net_Server_IListener_t*
Net_Server_Common_Tools::getListenerSingleton ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Common_Tools::getListenerSingleton"));

  return NET_SERVER_LISTENER_SINGLETON::instance ();
}

Net_Server_IListener_t*
Net_Server_Common_Tools::getAsynchListenerSingleton ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Common_Tools::getAsynchListenerSingleton"));

  return NET_SERVER_ASYNCHLISTENER_SINGLETON::instance ();
}
