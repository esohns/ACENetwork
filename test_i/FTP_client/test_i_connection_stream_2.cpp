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
#include "stdafx.h"

#include "test_i_connection_stream_2.h"

#include "ace/Log_Msg.h"

#include "stream_misc_defines.h"

#include "stream_net_defines.h"

#include "stream_stat_defines.h"

#include "net_macros.h"

#include "ftp_defines.h"
#include "ftp_module_parser_data.h"

#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_common.h"
//#include "test_i_common_modules.h"

typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage,
                                     struct FTP_Client_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct FTP_Client_StreamState,
                                     struct FTP_Client_SessionData,
                                     FTP_Client_SessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     FTP_Client_ConnectionManager_t,
                                     struct Stream_UserData> FTP_Client_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_Message,
                                     Test_I_SessionMessage,
                                     struct FTP_Client_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct FTP_Client_StreamState,
                                     struct FTP_Client_SessionData,
                                     FTP_Client_SessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     FTP_Client_ConnectionManager_t,
                                     struct Stream_UserData> FTP_Client_Module_Net_Reader_t;
DATASTREAM_MODULE_DUPLEX (struct FTP_Client_SessionData,                  // session data type
                          enum Stream_SessionMessageType,                 // session event type
                          struct FTP_Client_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                               // stream notification interface type
                          FTP_Client_Module_Net_Reader_t,                 // reader type
                          FTP_Client_Module_Net_Writer_t,                 // writer type
                          FTP_Client_Module_Net_IO);                      // name

typedef FTP_Module_Parser_Data_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct FTP_Client_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 Test_I_Message,
                                 Test_I_SessionMessage> FTP_Client_Module_Parser_Data;
DATASTREAM_MODULE_INPUT_ONLY (struct FTP_Client_SessionData,                // session data type
                              enum Stream_SessionMessageType,               // session event type
                              struct FTP_Client_ModuleHandlerConfiguration, // module handler configuration type
                              libacenetwork_protocol_default_ftp_parser_data_module_name_string,
                              Stream_INotify_t,                             // stream notification interface type
                              FTP_Client_Module_Parser_Data);               // reader type

Test_I_ConnectionStream_2::Test_I_ConnectionStream_2 ()
 : inherited ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_2::Test_I_ConnectionStream_2"));

}

bool
Test_I_ConnectionStream_2::load (Stream_ILayout* layout_inout,
                                 bool& deleteModules_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_ConnectionStream_2::load"));

  Stream_Module_t* module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  FTP_Client_Module_Net_IO_Module (this,
                                                   ACE_TEXT_ALWAYS_CHAR (MODULE_NET_IO_DEFAULT_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  FTP_Client_Module_Parser_Data_Module (this,
                                                        ACE_TEXT_ALWAYS_CHAR (FTP_DEFAULT_MODULE_PARSER_DATA_NAME_STRING)),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;
  //ACE_NEW_RETURN (module_p,
  //                FTP_Client_Module_StatisticReport_Module (this,
  //                                                          ACE_TEXT_ALWAYS_CHAR (MODULE_STAT_REPORT_DEFAULT_NAME_STRING)),
  //                false);
  //layout_inout->append (module_p, NULL, 0);
  //module_p = NULL;

  deleteModules_out = true;

  return true;
}
