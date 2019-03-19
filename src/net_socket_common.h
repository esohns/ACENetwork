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

#ifndef NET_SOCKET_COMMON_H
#define NET_SOCKET_COMMON_H

#include "ace/Global_Macros.h"
#include "ace/SOCK_Stream.h"
#include "ace/Synch_Traits.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_sock_dgram.h"
#include "net_tcpsockethandler.h"
#include "net_udpsockethandler.h"

typedef Net_TCPSocketHandler_T<ACE_NULL_SYNCH,
                               ACE_SOCK_STREAM,
                               Net_TCPSocketConfiguration_t> Test_U_TCPSocketHandler_t;
typedef Net_AsynchTCPSocketHandler_T<Net_TCPSocketConfiguration_t> Test_U_AsynchTCPSocketHandler_t;
typedef Net_UDPSocketHandler_T<ACE_NULL_SYNCH,
                               Net_SOCK_Dgram,
                               Net_UDPSocketConfiguration_t> Test_U_UDPSocketHandler_t;
typedef Net_AsynchUDPSocketHandler_T<Net_SOCK_Dgram,
                                     Net_UDPSocketConfiguration_t> Test_U_AsynchUDPSocketHandler_t;

#endif
