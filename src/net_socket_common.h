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

#ifndef Net_SOCKET_COMMON_H
#define Net_SOCKET_COMMON_H

#include "ace/Global_Macros.h"
#include "ace/SOCK_Dgram.h"
//#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Dgram_Bcast.h"
#include "ace/SOCK_Dgram_Mcast.h"

#include "stream_common.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_configuration.h"
#include "net_itransportlayer.h"
#include "net_stream.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_asynch_udpsocket_base.h"
#include "net_stream_common.h"
#include "net_stream_tcpsocket_base.h"
#include "net_stream_udpsocket_base.h"
#include "net_tcpsockethandler.h"
#include "net_udpsockethandler.h"

typedef Net_StreamAsynchTCPSocketBase_T<Net_Configuration_t,
                                        Net_StreamSessionData_t,
                                        Net_IInetTransportLayer_t,
                                        Stream_Statistic_t,
                                        Net_Stream,
                                        Net_AsynchTCPSocketHandler> Net_AsynchTCPHandler_t;
//typedef Net_StreamAsynchTCPSocketBase_T<Net_Configuration_t,
//                                        Net_StreamSessionData_t,
//                                        Net_INetlinkTransportLayer_t,
//                                        Stream_Statistic_t,
//                                        Net_Stream,
//                                        Net_AsynchTCPSocketHandler> Net_AsynchNetlinkHandler_t;

typedef Net_StreamAsynchUDPSocketBase_T<Net_Configuration_t,
                                        Net_StreamSessionData_t,
                                        Net_IInetTransportLayer_t,
                                        Stream_Statistic_t,
                                        Net_Stream,
                                        ACE_SOCK_DGRAM,
                                        Net_AsynchUDPSocketHandler> Net_AsynchUDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_Configuration_t,
                                        Net_StreamSessionData_t,
                                        Net_IInetTransportLayer_t,
                                        Stream_Statistic_t,
                                        Net_Stream,
                                        ACE_SOCK_DGRAM_MCAST,
                                        Net_AsynchUDPSocketHandler> Net_AsynchIPMulticastHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_Configuration_t,
                                        Net_StreamSessionData_t,
                                        Net_IInetTransportLayer_t,
                                        Stream_Statistic_t,
                                        Net_Stream,
                                        ACE_SOCK_DGRAM_BCAST,
                                        Net_AsynchUDPSocketHandler> Net_AsynchIPBroadcastHandler_t;

typedef Net_StreamTCPSocketBase_T<Net_Configuration_t,
                                  Net_StreamSessionData_t,
                                  Net_IInetTransportLayer_t,
                                  Stream_Statistic_t,
                                  Net_Stream,
                                  Net_TCPSocketHandler> Net_TCPHandler_t;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
typedef Net_StreamTCPSocketBase_T<Net_Configuration_t,
                                  Net_StreamSessionData_t,
                                  Net_INetlinkTransportLayer_t,
                                  Stream_Statistic_t,
                                  Net_Stream,
                                  Net_TCPSocketHandler> Net_NetlinkHandler_t;
#endif
typedef Net_StreamUDPSocketBase_T<Net_Configuration_t,
                                  Net_StreamSessionData_t,
                                  Net_IInetTransportLayer_t,
                                  Stream_Statistic_t,
                                  Net_Stream,
                                  Net_UDPSocketHandler_T<ACE_SOCK_DGRAM> > Net_UDPHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_Configuration_t,
                                  Net_StreamSessionData_t,
                                  Net_IInetTransportLayer_t,
                                  Stream_Statistic_t,
                                  Net_Stream,
                                  Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_MCAST> > Net_IPMulticastHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_Configuration_t,
                                  Net_StreamSessionData_t,
                                  Net_IInetTransportLayer_t,
                                  Stream_Statistic_t,
                                  Net_Stream,
                                  Net_UDPSocketHandler_T<ACE_SOCK_DGRAM_BCAST> > Net_IPBroadcastHandler_t;

#endif
