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

#ifndef NET_CLIENT_COMMON_H
#define NET_CLIENT_COMMON_H

#include "net_connection_manager_common.h"

// forward declarations
struct Net_SocketHandlerConfiguration;

struct Net_Client_ConnectorConfiguration
{
 inline Net_Client_ConnectorConfiguration ()
  : connectionManager (NULL)
  , socketHandlerConfiguration (NULL)
  //, statisticCollectionInterval (0)
  {};

  Net_IInetConnectionManager_t*   connectionManager;
  Net_SocketHandlerConfiguration* socketHandlerConfiguration;
  //unsigned int                    statisticCollectionInterval; // statistics collecting interval (second(s))
  //                                                             // 0 --> DON'T collect statistics
};

#endif
