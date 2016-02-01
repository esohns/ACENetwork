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

#ifndef TEST_U_DHCPCLIENT_COMMON_H
#define TEST_U_DHCPCLIENT_COMMON_H

//#include "gtk/gtk.h"

#include "test_u_common.h"
#include "test_u_connection_common.h"
#include "test_u_stream.h"

typedef Test_U_Stream_T<Test_U_OutboundConnector_t> Test_U_Stream_t;
typedef Test_U_Stream_T<Test_U_OutboundAsynchConnector_t> Test_U_AsynchStream_t;

struct Test_U_DHCPClient_Configuration
 : Test_U_Configuration
{
  inline Test_U_DHCPClient_Configuration ()
   : Test_U_Configuration ()
   , handle (ACE_INVALID_HANDLE)
  {};

  ACE_HANDLE handle;
};

struct Test_U_DHCPClient_GTK_CBData
 : Test_U_GTK_CBData
{
  inline Test_U_DHCPClient_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
  {};

  Test_U_DHCPClient_Configuration* configuration;
};

//struct Test_U_DHCPClient_ThreadData
//{
//  inline Test_U_DHCPClient_ThreadData ()
//   : CBData (NULL)
//   , eventSourceID (0)
//  {};

//  Test_U_DHCPClient_GTK_CBData* CBData;
//  guint                         eventSourceID;
//};

#endif
