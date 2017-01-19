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

#ifndef FILE_SERVER_STREAM_COMMON_H
#define FILE_SERVER_STREAM_COMMON_H

#include "stream_session_data.h"

#include "test_u_connection_common.h"
#include "test_u_stream_common.h"

struct Test_U_FileServer_SessionData
 : Test_U_StreamSessionData
{
  inline Test_U_FileServer_SessionData ()
   : Test_U_StreamSessionData ()
   , connection (NULL)
  {};

  Test_U_IConnection_t* connection;
};
typedef Stream_SessionData_T<struct Test_U_FileServer_SessionData> Test_U_FileServer_SessionData_t;

struct Test_U_FileServer_StreamState
 : Test_U_StreamState
{
  inline Test_U_FileServer_StreamState ()
   : Test_U_StreamState ()
   , currentSessionData (NULL)
  {};

  struct Test_U_FileServer_SessionData* currentSessionData;
};

#endif
