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

#include "ace/Synch.h"
#include "test_u_stream.h"
#include "test_u_tcpconnection.h"

#include "ace/Log_Msg.h"

#include "test_u_sessionmessage.h"

#include "net_defines.h"
#include "net_macros.h"

Test_U_TCPConnection::Test_U_TCPConnection (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                            const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_TCPConnection::Test_U_TCPConnection"));

}

Test_U_TCPConnection::Test_U_TCPConnection ()
 : inherited (NULL,
              ACE_Time_Value (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0))
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_TCPConnection::Test_U_TCPConnection"));

}

Test_U_TCPConnection::~Test_U_TCPConnection ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_TCPConnection::~Test_U_TCPConnection"));

}

void
Test_U_TCPConnection::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_TCPConnection::ping"));

  inherited::stream_.ping ();
}

//////////////////////////////////////////

Test_U_AsynchTCPConnection::Test_U_AsynchTCPConnection (ICONNECTION_MANAGER_T* interfaceHandle_in,
                                                        const ACE_Time_Value& statisticCollectionInterval_in)
 : inherited (interfaceHandle_in,
              statisticCollectionInterval_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_AsynchTCPConnection::Test_U_AsynchTCPConnection"));

}

Test_U_AsynchTCPConnection::Test_U_AsynchTCPConnection ()
 : inherited (NULL,
              ACE_Time_Value (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0))
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_AsynchTCPConnection::Test_U_AsynchTCPConnection"));

}

Test_U_AsynchTCPConnection::~Test_U_AsynchTCPConnection ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_AsynchTCPConnection::~Test_U_AsynchTCPConnection"));

}

void
Test_U_AsynchTCPConnection::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_U_AsynchTCPConnection::ping"));

  inherited::stream_.ping ();
}
