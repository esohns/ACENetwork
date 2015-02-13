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

#ifndef Net_CONNECTION_MANAGER_COMMON_H
#define Net_CONNECTION_MANAGER_COMMON_H

#include "ace/Singleton.h"
#include "ace/Synch.h"

#include "net_connection_manager.h"
#include "net_exports.h"
#include "net_stream_common.h"

typedef Net_Connection_Manager_T<Net_StreamProtocolConfigurationState_t,
                                 Net_RuntimeStatistic_t> Net_Connection_Manager_t;
typedef ACE_Singleton<Net_Connection_Manager_t,
                      ACE_Recursive_Thread_Mutex> NET_CONNECTIONMANAGER_SINGLETON;
NET_SINGLETON_DECLARE (ACE_Singleton,
                       Net_Connection_Manager_t,
                       ACE_Recursive_Thread_Mutex)

#endif
