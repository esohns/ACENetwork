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

#include "ace/INET_Addr.h"

#include "net_iconnector.h"
#include "net_connection_configuration.h"

typedef Net_IConnector_T<ACE_INET_Addr,
                         Net_TCPSocketConfiguration_t> Net_TCPIConnector_t;
typedef Net_IAsynchConnector_T<ACE_INET_Addr,
                               Net_TCPSocketConfiguration_t> Net_TCPIAsynchConnector_t;

typedef Net_IConnector_T<ACE_INET_Addr,
                         Net_UDPSocketConfiguration_t> Net_UDPIConnector_t;
typedef Net_IAsynchConnector_T<ACE_INET_Addr,
                               Net_UDPSocketConfiguration_t> Net_UDPIAsynchConnector_t;

#endif
