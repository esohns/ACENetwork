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

#ifndef FILE_SERVER_LISTENER_COMMON_H
#define FILE_SERVER_LISTENER_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_common.h"

#include "net_server_asynchlistener.h"
#include "net_server_listener.h"

#include "test_u_stream_common.h"
#include "test_u_tcpconnection.h"

#include "file_server_common.h"
#include "file_server_connection_common.h"

// foward declarations
class Test_U_Stream;

typedef Net_Server_AsynchListener_T<Test_U_AsynchTCPConnection,
                                    ACE_INET_Addr,
                                    struct FileServer_ListenerConfiguration,
                                    struct FileServer_ConnectionState,
                                    FileServer_ConnectionConfiguration_t,
                                    Test_U_Stream,
                                    struct FileServer_UserData> FileServer_AsynchListener_t;
typedef Net_Server_Listener_T<Test_U_TCPConnection,
                              ACE_SOCK_ACCEPTOR,
                              ACE_INET_Addr,
                              struct FileServer_ListenerConfiguration,
                              struct FileServer_ConnectionState,
                              FileServer_ConnectionConfiguration_t,
                              Test_U_Stream,
                              struct FileServer_UserData> FileServer_Listener_t;

typedef ACE_Singleton<FileServer_AsynchListener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> FILESERVER_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<FileServer_Listener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> FILESERVER_LISTENER_SINGLETON;

#endif
