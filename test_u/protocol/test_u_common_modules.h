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

#ifndef TEST_U_COMMON_MODULES_H
#define TEST_U_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_module_filewriter.h"

#include "net_module_runtimestatistic.h"

#include "http_codes.h"
#include "http_common.h"
#include "http_configuration.h"
//#include "http_module_bisector.h"
#include "http_module_parser.h"
#include "http_module_streamer.h"
//#include "http_stream_common.h"

#include "test_u_common.h"

// forward declarations
class Test_U_SessionMessage;
class Test_U_Message;

// declare module(s)
typedef Stream_Module_FileWriterH_T<ACE_SYNCH_MUTEX,
                                    ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    Test_U_SessionMessage,
                                    Test_U_Message,
                                    Test_U_ModuleHandlerConfiguration,
                                    Test_U_StreamState,
                                    Test_U_StreamSessionData,
                                    Test_U_StreamSessionData_t,
                                    HTTP_RuntimeStatistic_t> Test_U_Module_FileWriterH;

//typedef HTTP_Module_Bisector_T<ACE_SYNCH_MUTEX,
//                              ACE_MT_SYNCH,
//                              Common_TimePolicy_t,
//                              Test_U_SessionMessage,
//                              Test_U_Message,
//                              HTTP_ModuleHandlerConfiguration,
//                              HTTP_StreamState,
//                              HTTP_Stream_SessionData,
//                              HTTP_Stream_SessionData_t,
//                              HTTP_RuntimeStatistic_t> HTTP_Module_Bisector_t;

//typedef HTTP_Module_Parser_T<ACE_MT_SYNCH,
//                             Common_TimePolicy_t,
//                             Test_U_SessionMessage,
//                             Test_U_Message> Test_U_Module_Parser;
typedef HTTP_Module_ParserH_T<ACE_SYNCH_MUTEX,
                              ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              Test_U_SessionMessage,
                              Test_U_Message,
                              Test_U_ModuleHandlerConfiguration,
                              Test_U_StreamState,
                              Test_U_StreamSessionData,
                              Test_U_StreamSessionData_t,
                              HTTP_RuntimeStatistic_t> Test_U_Module_Parser;
typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              Test_U_SessionMessage,
                              Test_U_Message> Test_U_Module_Streamer;

typedef Net_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          Test_U_SessionMessage,
                                          Test_U_Message,
                                          HTTP_Method_t,
                                          HTTP_RuntimeStatistic_t> Test_U_Module_Statistic_ReaderTask_t;
typedef Net_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                          Common_TimePolicy_t,
                                          Test_U_SessionMessage,
                                          Test_U_Message,
                                          HTTP_Method_t,
                                          HTTP_RuntimeStatistic_t> Test_U_Module_Statistic_WriterTask_t;

typedef Stream_Module_FileWriter_T<Test_U_SessionMessage,
                                   Test_U_Message,
                                   Test_U_ModuleHandlerConfiguration,
                                   Test_U_StreamSessionData> Test_U_Module_FileWriter;

// declare module(s)
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                      // task synch type
                              Common_TimePolicy_t,               // time policy
                              Stream_ModuleConfiguration,        // module configuration type
                              Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              Test_U_Module_FileWriterH);        // writer type

//DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                    // task synch type
//                              Common_TimePolicy_t,             // time policy
//                              Stream_ModuleConfiguration,      // module configuration type
//                              HTTP_ModuleHandlerConfiguration, // module handler configuration type
//                              Test_U_Module_Parser);           // writer type
//DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                   // task synch type
//                          Common_TimePolicy_t,            // time policy
//                          Stream_ModuleConfiguration,     // module configuration type
//                          HTTP_ModuleHandlerConfiguration, // module handler configuration type
//                          HTTP_Module_Streamer,            // reader type
//                          HTTP_Module_Bisector_t,          // writer type
//                          HTTP_Module_Marshal);            // name
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                      // task synch type
                          Common_TimePolicy_t,               // time policy
                          Stream_ModuleConfiguration,        // module configuration type
                          Test_U_ModuleHandlerConfiguration, // module handler configuration type
                          Test_U_Module_Streamer,            // reader type
                          Test_U_Module_Parser,              // writer type
                          Test_U_Module_Marshal);            // name

DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                         // task synch type
                          Common_TimePolicy_t,                  // time policy type
                          Stream_ModuleConfiguration,           // module configuration type
                          Test_U_ModuleHandlerConfiguration,    // module handler configuration type
                          Test_U_Module_Statistic_ReaderTask_t, // reader type
                          Test_U_Module_Statistic_WriterTask_t, // writer type
                          Test_U_Module_RuntimeStatistic);      // name

DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                      // task synch type
                              Common_TimePolicy_t,               // time policy
                              Stream_ModuleConfiguration,        // module configuration type
                              Test_U_ModuleHandlerConfiguration, // module handler configuration type
                              Test_U_Module_FileWriter);         // writer type

#endif
