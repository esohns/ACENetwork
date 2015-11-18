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

#ifndef TEST_I_COMMON_H
#define TEST_I_COMMON_H

#include <algorithm>
#include <deque>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <string>

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_inotify.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data_base.h"
#include "stream_statemachine_control.h"

#include "stream_module_db_common.h"

#include "stream_module_htmlparser.h"

#include "stream_module_net_common.h"

#include "net_defines.h"

#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"
//#include "test_i_message.h"
//#include "test_i_session_message.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Stream_Message;
class Test_I_Stream_SessionMessage;
struct Test_I_ConnectionState;

typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

typedef Stream_Statistic Test_I_RuntimeStatistic_t;

typedef Common_IStatistic_T<Test_I_RuntimeStatistic_t> Test_I_StatisticReportingHandler_t;

struct Test_I_DataItem
{
 inline Test_I_DataItem ()
  : description ()
  , URI ()
 {};
 inline bool operator== (Test_I_DataItem rhs_in)
 {
   return URI == rhs_in.URI;
 };

 std::string description;
 std::string URI;
};
typedef std::list<Test_I_DataItem> Test_I_DataItems_t;
typedef Test_I_DataItems_t::const_iterator Test_I_DataItemsIterator_t;
typedef std::map<ACE_Time_Value, Test_I_DataItems_t> Test_I_PageData_t;
typedef Test_I_PageData_t::const_reverse_iterator Test_I_PageDataReverseConstIterator_t;
typedef Test_I_PageData_t::iterator Test_I_PageDataIterator_t;
struct Test_I_DataSet
{
  inline Test_I_DataSet ()
   : pageData ()
   , title ()
  {};

  Test_I_PageData_t pageData;
  std::string       title;
};
typedef std::list<Test_I_DataSet> Test_I_DataSets_t;
typedef Test_I_DataSets_t::const_iterator Test_I_DataSetsIterator_t;

enum Test_I_SAXParserState
{
  SAXPARSER_STATE_INVALID = -1,
  /////////////////////////////////////
  SAXPARSER_STATE_IN_HEAD = 0,
  SAXPARSER_STATE_IN_HTML,
  SAXPARSER_STATE_IN_BODY,
  /////////////////////////////////////
  SAXPARSER_STATE_READ_DATE,
  SAXPARSER_STATE_READ_DESCRIPTION,
  SAXPARSER_STATE_READ_TITLE,
  /////////////////////////////////////
  SAXPARSER_STATE_READ_ITEM,
  SAXPARSER_STATE_READ_ITEMS
};
struct Test_I_SAXParserContext
 : Stream_Module_HTMLParser_SAXParserContextBase
{
  inline Test_I_SAXParserContext ()
   : Stream_Module_HTMLParser_SAXParserContextBase ()
   , dataItem ()
   , sessionData (NULL)
   , state (SAXPARSER_STATE_INVALID)
   , timeStamp ()
  {};

  Test_I_DataItem            dataItem;
  Test_I_Stream_SessionData* sessionData;
  Test_I_SAXParserState      state;
  ACE_Time_Value             timeStamp;
};

struct Test_I_Configuration;
struct Test_I_Stream_Configuration;
struct Test_I_UserData
 : Stream_UserData
{
  inline Test_I_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_I_Configuration*        configuration;
  Test_I_Stream_Configuration* streamConfiguration;
};

struct Test_I_Stream_SessionData
 : Stream_SessionData
{
  inline Test_I_Stream_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , data ()
   , parserContext (NULL)
   , targetFileName ()
   , userData (NULL)
  {};
  inline Test_I_Stream_SessionData& operator= (Test_I_Stream_SessionData& rhs_in)
  {
    Stream_SessionData::operator= (rhs_in);

    connectionState = (connectionState ? connectionState : rhs_in.connectionState);
    if (data.pageData.empty ()) data = rhs_in.data;
    else
    {
      Test_I_PageDataIterator_t iterator;
      Test_I_DataItemsIterator_t iterator_2;
      for (Test_I_PageDataIterator_t iterator_3 = rhs_in.data.pageData.begin ();
           iterator_3 != rhs_in.data.pageData.end ();
           ++iterator_3)
      {
        iterator = data.pageData.find ((*iterator_3).first);
        if (iterator == data.pageData.end ())
        {
          data.pageData.insert (*iterator_3);
          continue;
        } // end IF

        for (Test_I_DataItemsIterator_t iterator_4 = (*iterator_3).second.begin ();
             iterator_4 != (*iterator_3).second.end ();
             ++iterator_4)
        {
          iterator_2 = std::find ((*iterator).second.begin (),
                                  (*iterator).second.end (),
                                  *iterator_4);
          if (iterator_2 == (*iterator_3).second.end ())
            (*iterator).second.push_back (*iterator_4);
        } // end FOR
      } // end FOR
    } // end ELSE
    parserContext = (parserContext ? parserContext : rhs_in.parserContext);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Test_I_ConnectionState*  connectionState;
  Test_I_DataSet           data; // html handler module
  Test_I_SAXParserContext* parserContext; // html parser/handler module
  std::string              targetFileName; // file writer module
  Test_I_UserData*         userData;
};
typedef Stream_SessionDataBase_T<Test_I_Stream_SessionData> Test_I_Stream_SessionData_t;

struct Test_I_Stream_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Stream_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ////////////////////////////////////
   , userData (NULL)
  {};

  Test_I_UserData* userData;
};

// forward declarations
struct Test_I_Configuration;
typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                      ///////////////////
                      ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      ///////////////////
                      Stream_StateMachine_ControlState,
                      Test_I_Stream_State,
                      ///////////////////
                      Test_I_Stream_Configuration,
                      ///////////////////
                      Test_I_RuntimeStatistic_t,
                      ///////////////////
                      Stream_ModuleConfiguration,
                      Test_I_Stream_ModuleHandlerConfiguration,
                      ///////////////////
                      Test_I_Stream_SessionData,   // session data
                      Test_I_Stream_SessionData_t, // session data container (reference counted)
                      Test_I_Stream_SessionMessage,
                      Test_I_Stream_Message> Test_I_StreamBase_t;
struct Test_I_Stream_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline Test_I_Stream_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , configuration (NULL)
   , connection (NULL)
   , connectionManager (NULL)
   , dataBaseOptionsFileName ()
   , dataBaseTable ()
   , hostName ()
   , inbound (true)
   , loginOptions ()
   , mode (STREAM_MODULE_HTMLPARSER_SAX)
   , passive (false)
   , printProgressDot (false)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , stream (NULL)
   , targetFileName ()
   , URL ()
  {};

  Test_I_Configuration*                     configuration;
  Test_I_IConnection_t*                     connection; // TCP target/IO module
  Test_I_Stream_InetConnectionManager_t*    connectionManager; // TCP IO module
  std::string                               dataBaseOptionsFileName; // db writer module
  std::string                               dataBaseTable; // db writer module
  std::string                               hostName; // net source module
  bool                                      inbound; // net io module
  Stream_Module_DataBase_LoginOptions       loginOptions; // db writer module
  Stream_Module_HTMLParser_Mode             mode; // html parser module
  bool                                      passive; // net source module
  bool                                      printProgressDot; // file writer module
  Net_SocketConfiguration*                  socketConfiguration;
  Test_I_Stream_SocketHandlerConfiguration* socketHandlerConfiguration;
  Test_I_StreamBase_t*                      stream;
  std::string                               targetFileName; // file writer module
  std::string                               URL; // HTTP get module
};

struct Stream_SignalHandlerConfiguration
{
  inline Stream_SignalHandlerConfiguration ()
   : //messageAllocator (NULL)
   /*,*/ statisticReportingInterval (0)
  {};

  //Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistics collecting interval (second(s)) [0: off]
};

struct Test_I_Stream_Configuration
 : Stream_Configuration
{
  inline Test_I_Stream_Configuration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_I_Stream_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct Test_I_Stream_State
 : Stream_State
{
  inline Test_I_Stream_State ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Test_I_Stream_SessionData* currentSessionData;
  Test_I_UserData*           userData;
};

struct Test_I_Configuration
{
  inline Test_I_Configuration ()
   : signalHandlerConfiguration ()
   , socketConfiguration ()
   , socketHandlerConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // **************************** signal data **********************************
  Stream_SignalHandlerConfiguration        signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_SocketConfiguration                  socketConfiguration;
  Test_I_Stream_SocketHandlerConfiguration socketHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_ModuleConfiguration               moduleConfiguration;
  Test_I_Stream_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_I_Stream_Configuration              streamConfiguration;
  // *************************** protocol data *********************************
  Test_I_UserData                          userData;
  bool                                     useReactor;
};

typedef Stream_IModuleHandler_T<Test_I_Stream_ModuleHandlerConfiguration> Test_I_IModuleHandler_t;
typedef Stream_MessageAllocatorHeapBase_T<Test_I_Stream_Message,
                                          Test_I_Stream_SessionMessage> Test_I_MessageAllocator_t;

#endif
