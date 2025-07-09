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

#ifndef NET_CONFIGURATION_H
#define NET_CONFIGURATION_H

#include <map>

#include "common_event_common.h"
#include "common_event_defines.h"

#include "common_ui_common.h"

#include "net_common.h"

// forward declarations
struct Net_ConnectionConfigurationBase;

typedef std::map<Net_ConnectionId_t,
                 struct Net_ConnectionConfigurationBase*> Net_SessionConnectionConfigurations_t;
typedef Net_SessionConnectionConfigurations_t::iterator Net_SessionConnectionConfigurationsIterator_t;

struct Net_ConnectionConfigurationBase;
struct Common_ParserConfiguration;
struct Net_SessionConfiguration
{
  Net_SessionConfiguration ()
   : connectionConfiguration (NULL)
   , connectionConfigurations ()
   , dispatch (COMMON_EVENT_DEFAULT_DISPATCH)
   //, parserConfiguration (NULL)
   , CBData (NULL)
  {}

  struct Net_ConnectionConfigurationBase* connectionConfiguration;
  Net_SessionConnectionConfigurations_t   connectionConfigurations; // *TODO*: this is part of the state; move it there
  enum Common_EventDispatchType           dispatch;

  struct Common_UI_CBData*                CBData;
};

//template <typename ConnectionConfigurationType,
//          enum Net_TransportLayerType TransportLayerType_e>
//class Net_ListenerConfiguration_T
// //: public Net_ConnectionConfigurationBase_T<TransportLayerType_e>
//{
// public:
//  Net_ListenerConfiguration_T ()
////   : Net_ConnectionConfigurationBase_T ()
//   : addressFamily (AF_INET)
//   , connectionConfiguration (NULL)
//  {}
//
//  int                          addressFamily;
//  ConnectionConfigurationType* connectionConfiguration;
//};

#endif
