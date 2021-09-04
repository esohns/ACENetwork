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

#ifndef SMTP_CONFIGURATION_H
#define SMTP_CONFIGURATION_H

//#include <deque>
//#include <map>
//#include <string>

#include "ace/INET_Addr.h"

//#include "common_defines.h"
//#include "common_inotify.h"

#include "stream_common.h"

#include "net_common.h"
//#include "net_configuration.h"
//#include "net_defines.h"
#include "net_iconnectionmanager.h"

#include "smtp_common.h"
//#include "smtp_defines.h"
//#include "smtp_stream_common.h"

// forward declarations
class SMTP_ConnectionConfiguration;
struct SMTP_ConnectionState;
struct SMTP_Record;
struct SMTP_ModuleHandlerConfiguration;
struct SMTP_Stream_SessionData;

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct SMTP_Configuration,
//                          struct SMTP_ConnectionState,
//                          struct Net_StreamStatistic,
//                          SMTP_Stream> SMTP_IConnection_t;
//typedef Net_IConnectionManager_T<ACE_INET_Addr,
//                                 SMTP_ConnectionConfiguration,
//                                 struct SMTP_ConnectionState,
//                                 struct Net_StreamStatistic,
//                                 struct Net_UserData> SMTP_IConnection_Manager_t;

struct SMTP_AllocatorConfiguration
 : Common_Parser_FlexAllocatorConfiguration
{
  SMTP_AllocatorConfiguration ()
   : Common_Parser_FlexAllocatorConfiguration ()
  {}
};

//typedef Common_INotify_T<unsigned int,
//                         struct SMTP_Stream_SessionData,
//                         SMTP_Record,
//                         SMTP_SessionMessage> SMTP_IStreamNotify_t;

struct SMTP_ProtocolConfiguration
{
  SMTP_ProtocolConfiguration ()
   : domain (static_cast<u_short> (0),
             ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
             AF_INET)
   , username ()
   , password ()
  {}

  ACE_INET_Addr domain; // i.e. external address
  std::string   username;
  std::string   password;
};

struct SMTP_ModuleHandlerConfiguration
 : virtual Stream_ModuleHandlerConfiguration
{
  SMTP_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   ///////////////////////////////////////
   , request (NULL)
   , protocolConfiguration (NULL)
  {}

  struct SMTP_Request*               request;
  struct SMTP_ProtocolConfiguration* protocolConfiguration;
};

struct SMTP_StreamConfiguration
 : virtual Stream_Configuration
{
  SMTP_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration (NULL)
   //, moduleHandlerConfigurations ()
   , protocolConfiguration (NULL)
  {}

  struct Stream_ModuleConfiguration* moduleConfiguration;   // stream module configuration
  struct SMTP_ProtocolConfiguration* protocolConfiguration; // protocol configuration
};

typedef Stream_Configuration_T<//stream_name_string_,
                               struct SMTP_StreamConfiguration,
                               struct SMTP_ModuleHandlerConfiguration> SMTP_StreamConfiguration_t;

#endif
