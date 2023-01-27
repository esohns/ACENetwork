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

#ifndef HTTP_NETWORK_H
#define HTTP_NETWORK_H

#include "net_common.h"
#include "net_iconnection.h"

#include "http_common.h"

// forward declarations
//struct HTTP_StreamConfiguration;

//class HTTP_ConnectionConfiguration
// : public Net_ConnectionConfiguration_T<struct Common_Parser_FlexAllocatorConfiguration,
//                                        struct HTTP_StreamConfiguration,
//                                        NET_TRANSPORTLAYER_TCP>
//{
// public:
//  HTTP_ConnectionConfiguration ()
//   : Net_ConnectionConfiguration_T ()
//  {}
//};

struct HTTP_ConnectionState
 : Net_StreamConnectionState
{
  HTTP_ConnectionState ()
   : Net_StreamConnectionState ()
   //, configuration (NULL)
  {}

  //HTTP_ConnectionConfiguration* configuration;
};

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> HTTP_IConnection_t;

#endif
