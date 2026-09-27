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

#ifndef TEST_U_WS_CLIENT_COMMON_H
#define TEST_U_WS_CLIENT_COMMON_H

#include <list>
#include <string>

//#include "libxml/tree.h"
//#include "libxml/xpath.h"

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/Singleton.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#endif // GTK_SUPPORT

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "stream_html_common.h"

#include "stream_module_xmlparser.h"

#include "net_defines.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_ilistener.h"

#include "ws_common.h"
#include "ws_network.h"
#include "ws_stream_common.h"

#include "test_u_common.h"
#include "test_u_stream_common.h"
#if defined (GTK_SUPPORT)
#include "test_u_gtk_common.h"
#endif // GTK_SUPPORT

#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_defines.h"

struct WebSocket_Client_AllocatorConfiguration
 : Common_Parser_FlexAllocatorConfiguration
{
  WebSocket_Client_AllocatorConfiguration ()
   : Common_Parser_FlexAllocatorConfiguration ()
  {
    defaultBufferSize = STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE;
  }
};

struct WebSocket_Client_MessageData
 : WebSocket_Record
{
  WebSocket_Client_MessageData ()
   : WebSocket_Record ()
   //, document (NULL)
   //, xPathObject (NULL)
  {}

  virtual ~WebSocket_Client_MessageData ()
  {
    //if (document)
    //  xmlFreeDoc (document);
    //if (xPathObject)
    //  xmlXPathFreeObject (xPathObject);
  }
  inline struct WebSocket_Client_MessageData& operator= (struct HTTP_Record& rhs_in) { HTTP_Record::operator= (rhs_in); return *this; }
  inline struct WebSocket_Client_MessageData& operator= (struct WebSocket_Record& rhs_in) { WebSocket_Record::operator= (rhs_in); return *this; }
  inline void operator+= (struct WebSocket_Client_MessageData rhs_in) { ACE_UNUSED_ARG (rhs_in); ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  //xmlDocPtr         document;
  //xmlXPathObjectPtr xPathObject;
};
typedef Stream_DataBase_T<struct WebSocket_Client_MessageData> WebSocket_Client_MessageData_t;

class Test_U_Message;
class Test_U_SessionMessage;
typedef Stream_ISessionDataNotify_T<struct WebSocket_Client_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> WebSocket_Client_ISessionNotify_t;
typedef std::list<WebSocket_Client_ISessionNotify_t*> WebSocket_Client_Subscribers_t;
typedef WebSocket_Client_Subscribers_t::const_iterator WebSocket_Client_SubscribersIterator_t;

//typedef Net_IConnectionManager_T<ACE_INET_Addr,
//                                 WebSocket_Client_ConnectionConfiguration_t,
//                                 struct WebSocket_ConnectionState,
//                                 UPnP_Statistic_t,
//                                 Test_U_UserData> WebSocket_Client_IConnectionManager_t;

//typedef Stream_ControlMessage_T<enum Stream_ControlType,
//                                enum Stream_ControlMessageType,
//                                struct Common_Parser_FlexAllocatorConfiguration> WebSocket_Client_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_Parser_FlexAllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> WebSocket_Client_MessageAllocator_t;

//extern const char stream_name_string_[];
struct WebSocket_Client_StreamConfiguration;
struct WebSocket_Client_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct WebSocket_Client_StreamConfiguration,
                               struct WebSocket_Client_ModuleHandlerConfiguration> WebSocket_Client_StreamConfiguration_t;
struct WebSocket_Client_ModuleHandlerConfiguration
 : WebSocket_ModuleHandlerConfiguration
{
  WebSocket_Client_ModuleHandlerConfiguration ()
   : WebSocket_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   //, mode (STREAM_MODULE_XML_PARSER_MODE_DOM)
   , subscriber (NULL)
   //, xPathQueryString ()
   //, xPathNameSpaces ()
  {
    passive = false;
  }

  WebSocket_Client_IConnection_t*    connection; // UDP target/net IO module
  Net_ConnectionConfigurations_t*    connectionConfigurations;
  //enum Stream_Module_XML_Parser_Mode mode;
  WebSocket_Client_ISessionNotify_t* subscriber;
  //std::string                        xPathQueryString;
  //Stream_HTML_XPathNameSpaces_t      xPathNameSpaces;
};

struct WebSocket_Client_StreamConfiguration
 : WebSocket_StreamConfiguration
{
  WebSocket_Client_StreamConfiguration ()
   : WebSocket_StreamConfiguration ()
   //, parserContext (NULL)
  {}

  //struct Stream_Module_XMLParser_SAXParserContextBase* parserContext; // XML-
};

struct WebSocket_Client_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  WebSocket_Client_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

  WebSocket_IStatisticReportingHandler_t* statisticReportingHandler;
  long                                    statisticReportingTimerId;
};

//////////////////////////////////////////

struct WebSocket_Client_Configuration
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
{
  WebSocket_Client_Configuration ()
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
   , allocatorConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , parserConfiguration ()
   //, parserContext()
   , streamConfiguration ()
   , handle (ACE_INVALID_HANDLE)
  {
    //parserConfiguration.headerOnly = true; // WebSocket is header-only HTTP
  }

  struct WebSocket_Client_AllocatorConfiguration     allocatorConfiguration;
  // **************************** signal data **********************************
  struct WebSocket_Client_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                     connectionConfigurations;
  // **************************** parser data **********************************
  struct HTTP_ParserConfiguration                    parserConfiguration;
  //struct Stream_Module_XMLParser_SAXParserContextBase parserContext;
  // **************************** stream data **********************************
  WebSocket_Client_StreamConfiguration_t             streamConfiguration;
  // *************************** listener data *********************************

  ACE_HANDLE                                         handle;
};

//////////////////////////////////////////

struct WebSocket_Client_UI_ProgressData
#if defined (GTK_USE)
 : Test_U_GTK_ProgressData
#endif // GTK_USE
{
  WebSocket_Client_UI_ProgressData ()
#if defined (GTK_USE)
   : Test_U_GTK_ProgressData ()
#endif // GTK_USE
  {}
};

class Test_U_EventHandler;
struct WebSocket_Client_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#endif // GTK_USE
{
  WebSocket_Client_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
   , configuration (NULL)
#else
   : configuration (NULL)
#endif // GTK_USE
   , eventHandler (NULL)
   , progressData ()
  {}

  struct WebSocket_Client_Configuration*  configuration;
  Test_U_EventHandler*                    eventHandler;
  struct WebSocket_Client_UI_ProgressData progressData;
};

struct WebSocket_Client_ThreadData
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#endif // GTK_USE
{
  WebSocket_Client_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
   , CBData (NULL)
#else
   : CBData (NULL)
#endif // GTK_USE
  {}

  struct WebSocket_Client_UI_CBData* CBData;
};

#endif
