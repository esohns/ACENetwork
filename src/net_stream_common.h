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

#ifndef NET_STREAM_COMMON_H
#define NET_STREAM_COMMON_H

#include <list>

#include "ace/Synch_Traits.h"

#include "common.h"
#include "common_inotify.h"
#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_session_data_base.h"

#include "net_configuration.h"
#include "net_message.h"

typedef Stream_SessionDataBase_T<Net_UserData_t> Net_StreamSessionData_t;

typedef Stream_IModule<ACE_MT_SYNCH,
                       Common_TimePolicy_t,
                       Stream_ModuleConfiguration_t> Net_IModule_t;

typedef Common_INotify_T<Stream_ModuleConfiguration_t,
                         Net_Message> Net_INotify_t;
typedef std::list<Net_INotify_t*> Net_Subscribers_t;
typedef Net_Subscribers_t::iterator Net_SubscribersIterator_t;

typedef Common_ISubscribe_T<Net_INotify_t> Net_ISubscribe_t;

#endif
