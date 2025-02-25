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

#ifndef FTP_CONFIGURATION_H
#define FTP_CONFIGURATION_H

#include "ace/INET_Addr.h"

#include "common_parser_common.h"

#include "stream_common.h"

#include "net_common.h"
#include "net_iconnectionmanager.h"

#include "ftp_common.h"

// forward declarations
class FTP_ConnectionConfiguration;
struct FTP_ConnectionState;
struct FTP_Record;
struct FTP_ModuleHandlerConfiguration;
struct FTP_Stream_SessionData;

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          struct FTP_Configuration,
//                          struct FTP_ConnectionState,
//                          struct Net_StreamStatistic,
//                          FTP_Stream> FTP_IConnection_t;
//typedef Net_IConnectionManager_T<ACE_INET_Addr,
//                                 FTP_ConnectionConfiguration,
//                                 struct FTP_ConnectionState,
//                                 struct Net_StreamStatistic,
//                                 struct Net_UserData> FTP_IConnection_Manager_t;

struct FTP_AllocatorConfiguration
 : Common_Parser_FlexAllocatorConfiguration
{
  FTP_AllocatorConfiguration ()
   : Common_Parser_FlexAllocatorConfiguration ()
  {}
};

//typedef Common_INotify_T<unsigned int,
//                         struct FTP_Stream_SessionData,
//                         FTP_Record,
//                         FTP_SessionMessage> FTP_IStreamNotify_t;

struct FTP_ProtocolConfiguration
{
  FTP_ProtocolConfiguration ()
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

struct FTP_ModuleHandlerConfiguration
 : virtual Stream_ModuleHandlerConfiguration
{
  FTP_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   ///////////////////////////////////////
   , request (NULL)
   , parserConfiguration (NULL)
   , protocolConfiguration (NULL)
  {}

  struct FTP_Request*                        request;
  struct Common_FlexBisonParserConfiguration* parserConfiguration; // parser module(s)
  struct FTP_ProtocolConfiguration*          protocolConfiguration;
};

struct FTP_StreamConfiguration
 : virtual Stream_Configuration
{
  FTP_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleConfiguration (NULL)
   //, moduleHandlerConfigurations ()
   , protocolConfiguration (NULL)
  {}

  struct Stream_ModuleConfiguration* moduleConfiguration;   // stream module configuration
  struct FTP_ProtocolConfiguration* protocolConfiguration; // protocol configuration
};

typedef Stream_Configuration_T<//stream_name_string_,
                               struct FTP_StreamConfiguration,
                               struct FTP_ModuleHandlerConfiguration> FTP_StreamConfiguration_t;

#endif
