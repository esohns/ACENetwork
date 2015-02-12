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

#include "net_asynch_udpsockethandler.h"

#include "net_macros.h"

Net_AsynchUDPSocketHandler::Net_AsynchUDPSocketHandler()
 //: inherited(Net_CONNECTIONMANAGER_SINGLETON::instance())
{
  NETWORK_TRACE(ACE_TEXT("Net_AsynchUDPSocketHandler::Net_AsynchUDPSocketHandler"));

}

Net_AsynchUDPSocketHandler::~Net_AsynchUDPSocketHandler()
{
  NETWORK_TRACE(ACE_TEXT("Net_AsynchUDPSocketHandler::~Net_AsynchUDPSocketHandler"));

}

//void
//Net_AsynchUDPSocketHandler::ping()
//{
//  NETWORK_TRACE(ACE_TEXT("Net_AsynchUDPSocketHandler::ping"));
//
//  inherited::myStream.ping();
//}

void
Net_AsynchUDPSocketHandler::open(ACE_HANDLE handle_in,
                                     ACE_Message_Block& messageBlock_in)
{
  NETWORK_TRACE(ACE_TEXT("Net_AsynchUDPSocketHandler::open"));

  //// step1: prepare data processing stream
  //inherited::myUserData.sessionID = inherited::myStream.getSessionID(); // (== socket handle)

  //// step2: init/start stream, register reading data with reactor...
  //inherited::open(handle_in,
  //                messageBlock_in);
}
