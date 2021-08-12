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

#ifndef DHCP_CONFIGURATION_H
#define DHCP_CONFIGURATION_H

#include <deque>
#include <map>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_inotify.h"

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "dhcp_common.h"
#include "dhcp_defines.h"
#include "dhcp_stream_common.h"

// forward declarations
struct DHCP_ConnectionState;
struct DHCP_Record;
struct DHCP_ModuleHandlerConfiguration;
class DHCP_SessionMessage;
struct DHCP_Stream_SessionData;

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct DHCP_Configuration,
//                          struct DHCP_ConnectionState,
//                          DHCP_Statistic_t,
//                          DHCP_Stream> DHCP_IConnection_t;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct DHCP_ConnectionConfiguration,
                                 struct DHCP_ConnectionState,
                                 DHCP_Statistic_t,
                                 struct Net_UserData> DHCP_IConnection_Manager_t;

struct DHCP_AllocatorConfiguration
 : Common_Parser_FlexAllocatorConfiguration
{
  DHCP_AllocatorConfiguration ()
   : Common_Parser_FlexAllocatorConfiguration ()
  {
    defaultBufferSize = DHCP_MESSAGE_SIZE;
  }
};

//typedef Common_INotify_T<unsigned int,
//                         struct DHCP_Stream_SessionData,
//                         DHCP_Record,
//                         DHCP_SessionMessage> DHCP_IStreamNotify_t;

struct DHCP_ProtocolConfiguration
{
  DHCP_ProtocolConfiguration ()
   : requestBroadcastReplies (DHCP_DEFAULT_FLAGS_BROADCAST)
   , sendRequestOnOffer (false)
  {}

  bool requestBroadcastReplies;
  bool sendRequestOnOffer;
};

struct DHCP_StreamConfiguration;
struct DHCP_ModuleHandlerConfiguration
 : public Stream_ModuleHandlerConfiguration
{
  DHCP_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   ///////////////////////////////////////
//   , connection (NULL)
//   , connectionConfigurations (NULL)
   , printProgressDot (DHCP_DEFAULT_PRINT_PROGRESSDOT)
   , protocolConfiguration (NULL)
  {
    crunchMessages = DHCP_DEFAULT_CRUNCH_MESSAGES; // dhcp parser module
    printFinalReport = true;
  }

//  Test_U_IConnection_t*       connection; // UDP target/net IO module
//  DHCP_ConnectionConfigurations_t*   connectionConfigurations;
  bool                               printProgressDot; // file writer module
  struct DHCP_ProtocolConfiguration* protocolConfiguration;
};
typedef std::map<std::string,
                 struct DHCP_ModuleHandlerConfiguration*> DHCP_ModuleHandlerConfigurations_t;
typedef DHCP_ModuleHandlerConfigurations_t::const_iterator DHCP_ModuleHandlerConfigurationsConstIterator_t;

struct DHCP_StreamConfiguration
 : Stream_Configuration
{
  DHCP_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration ()
   , moduleHandlerConfigurations ()
   , protocolConfiguration (NULL)
  {}

  struct Stream_ModuleConfiguration* moduleConfiguration;        // stream module configuration
  DHCP_ModuleHandlerConfigurations_t moduleHandlerConfigurations; // module handler configuration
  struct DHCP_ProtocolConfiguration* protocolConfiguration;      // protocol configuration
};

#endif
