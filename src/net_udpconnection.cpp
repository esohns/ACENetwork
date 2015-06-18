/***************************************************************************
*   Copyright (C) 2010 by Erik Sohns   *
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

#include "net_udpconnection.h"

#include "net_configuration.h"
#include "net_exports.h"
#include "net_socket_common.h"
#include "net_stream_common.h"

// export (common) template instances
#if defined (_MSC_VER)
template Net_Export class Net_UDPConnection_T<Net_StreamUserData,
                                              Net_StreamSessionData_t,
                                              Net_UDPHandler_t,
                                              Net_SocketHandlerConfiguration>;
template Net_Export class Net_AsynchUDPConnection_T<Net_StreamUserData,
                                                    Net_StreamSessionData_t,
                                                    Net_AsynchUDPHandler_t,
                                                    Net_SocketHandlerConfiguration>;
#else
template class Net_UDPConnection_T<Net_StreamUserData,
                                   Net_StreamSessionData_t,
                                   Net_UDPHandler_t,
                                   Net_SocketHandlerConfiguration>;
template class Net_AsynchUDPConnection_T<Net_StreamUserData,
                                         Net_StreamSessionData_t,
                                         Net_AsynchUDPHandler_t,
                                         Net_SocketHandlerConfiguration>;
#endif
