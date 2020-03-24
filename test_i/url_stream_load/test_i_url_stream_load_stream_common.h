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

#ifndef TEST_I_URLSTREAMLOAD_STREAM_COMMON_H
#define TEST_I_URLSTREAMLOAD_STREAM_COMMON_H

#include <list>
#include <map>
#include <set>
#include <string>

#include "libxml/tree.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "stream_module_htmlparser.h"

#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_network.h"
#include "http_stream_common.h"

#include "test_i_defines.h"
#include "test_i_stream_common.h"

#include "test_i_connection_common.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Message;
class Test_I_SessionMessage;
//struct Test_I_URLStreamLoad_ConnectionConfiguration;
typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_URLStreamLoad_ConnectionConfiguration_t,
                          struct HTTP_ConnectionState,
                          HTTP_Statistic_t> Test_I_IConnection_t;

struct HTTP_Record;
struct Test_I_MessageData
{
  Test_I_MessageData ()
   : HTTPRecord (NULL)
   , HTMLDocument (NULL)
  {};
  ~Test_I_MessageData ()
  {
    if (HTTPRecord)
      delete HTTPRecord;
    if (HTMLDocument)
      xmlFreeDoc (HTMLDocument);
  };
  inline void operator+= (Test_I_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); }
  inline operator struct HTTP_Record&() const { ACE_ASSERT (HTTPRecord); return *HTTPRecord; }

  struct HTTP_Record* HTTPRecord;
  xmlDocPtr           HTMLDocument;
};

struct Test_I_URLStreamLoad_SessionData;
enum Test_I_URLStreamLoad_SAXParserState
{
  TEST_I_SAXPARSER_STATE_INVALID = -1,
  ////////////////////////////////////////
  TEST_I_SAXPARSER_STATE_IN_HEAD = 0,
  TEST_I_SAXPARSER_STATE_IN_HTML,
  TEST_I_SAXPARSER_STATE_IN_BODY
  ////////////////////////////////////////
};
struct Test_I_URLStreamLoad_SAXParserContext
 : Stream_Module_HTMLParser_SAXParserContextBase
{
  Test_I_URLStreamLoad_SAXParserContext ()
   : Stream_Module_HTMLParser_SAXParserContextBase ()
   , sessionData (NULL)
   , state (TEST_I_SAXPARSER_STATE_INVALID)
   , URL ()
  {};

  struct Test_I_URLStreamLoad_SessionData* sessionData;
  enum Test_I_URLStreamLoad_SAXParserState state;
  std::string                              URL;
};

struct Test_I_URLStreamLoad_SessionData
 : Test_I_StreamSessionData
{
  Test_I_URLStreamLoad_SessionData ()
   : Test_I_StreamSessionData ()
   , address (static_cast<u_short> (0),
              static_cast<ACE_UINT32> (INADDR_ANY))
   , connection (NULL)
   , format (STREAM_COMPRESSION_FORMAT_INVALID)
   , parserContext ()
   , targetFileName ()
  {}

  struct Test_I_URLStreamLoad_SessionData& operator= (struct Test_I_URLStreamLoad_SessionData& rhs_in)
  {
    Test_I_StreamSessionData::operator= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

  ACE_INET_Addr                                address;
  Test_I_IConnection_t*                        connection;
  enum Stream_Decoder_CompressionFormatType    format; // HTTP parser module
  struct Test_I_URLStreamLoad_SAXParserContext parserContext; // HTML parser module
  std::string                                  targetFileName; // file writer module
};
typedef Stream_SessionData_T<struct Test_I_URLStreamLoad_SessionData> Test_I_URLStreamLoad_SessionData_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_URLStreamLoad_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_SessionMessage> Test_I_ISessionNotify_t;
typedef std::list<Test_I_ISessionNotify_t*> Test_I_Subscribers_t;
typedef Test_I_Subscribers_t::const_iterator Test_I_SubscribersIterator_t;

struct Test_I_URLStreamLoad_ModuleHandlerConfiguration
 : HTTP_ModuleHandlerConfiguration
{
  Test_I_URLStreamLoad_ModuleHandlerConfiguration ()
   : HTTP_ModuleHandlerConfiguration ()
   , connectionConfigurations (NULL)
   //, contextId (0)
   , mode (STREAM_MODULE_HTMLPARSER_MODE_SAX)
   , subscriber (NULL)
   , subscribers (NULL)
   , targetFileName ()
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
    inbound = true;
  }

  Net_ConnectionConfigurations_t*    connectionConfigurations;
  //guint                                   contextId;
  enum Stream_Module_HTMLParser_Mode mode; // HTML parser module
  Test_I_ISessionNotify_t*           subscriber;
  Test_I_Subscribers_t*              subscribers;
  std::string                        targetFileName; // dump module
};

struct Test_I_URLStreamLoad_StreamConfiguration
 : HTTP_StreamConfiguration
{
  Test_I_URLStreamLoad_StreamConfiguration ()
   : HTTP_StreamConfiguration ()
  {}
};
//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Common_Parser_FlexAllocatorConfiguration,
                               struct Test_I_URLStreamLoad_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_I_URLStreamLoad_ModuleHandlerConfiguration> Test_I_URLStreamLoad_StreamConfiguration_t;

struct Test_I_URLStreamLoad_StreamState
 : Test_I_StreamState
{
  Test_I_URLStreamLoad_StreamState ()
   : Test_I_StreamState ()
   , sessionData (NULL)
  {}

  struct Test_I_URLStreamLoad_SessionData* sessionData;
};

#endif
