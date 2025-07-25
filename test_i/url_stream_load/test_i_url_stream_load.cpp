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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#elif defined (ACE_LINUX)
#include "sys/capability.h"
#include "linux/capability.h"
#endif // ACE_WIN32 || ACE_WIN64

#include <iostream>
#include <limits>
#include <regex>
#include <string>

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_os_tools.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "common_event_tools.h"

#include "common_logger_queue.h"
#include "common_log_tools.h"

#include "common_timer_tools.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"

#include "stream_misc_defines.h"

#include "stream_file_sink.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

#include "net_client_defines.h"

#include "http_defines.h"

#if defined (GTK_SUPPORT)
#include "test_i_callbacks.h"
#endif // GTK_SUPPORT
#include "test_i_common.h"
#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_connection_stream.h"
#include "test_i_defines.h"
#include "test_i_eventhandler.h"
#include "test_i_message.h"
#include "test_i_module_eventhandler.h"
#include "test_i_session_message.h"
#include "test_i_signalhandler.h"

#include "test_i_url_stream_load_common.h"
#include "test_i_url_stream_load_defines.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("URLStreamLoadStream");

void
do_print_usage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_print_usage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string path =
    Common_File_Tools::getWorkingDirectory ();

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d        : debug parser [")
            << COMMON_PARSER_DEFAULT_YACC_TRACE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::string configuration_path = path;
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string gtk_rc_file = configuration_path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e [PATH] : Gtk .rc file [\"")
            << gtk_rc_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::string output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f[[PATH]]: (output) file name [")
            << output_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string UI_file = configuration_path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[PATH]]: UI definition file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l        : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r        : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]: statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t        : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [URL]  : stream URL [")
            << ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_DEFAULT_URL)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v        : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_process_arguments (int argc_in,
                      ACE_TCHAR** argv_in, // cannot be const...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      bool& showConsole_out,
#endif
                      bool& debugParser_out,
#if defined (GTK_USE)
                      std::string& GtkRcFileName_out,
#endif // GTK_USE
                      std::string& fileName_out,
                      std::string& UIDefinitonFileName_out,
                      bool& logToFile_out,
                      unsigned short& port_out,
                      bool& useReactor_out,
                      ACE_Time_Value& statisticReportingInterval_out,
                      bool& traceInformation_out,
                      std::string& URL_out,
                      bool& printVersionAndExit_out,
                      std::string& hostName_out,
                      ACE_INET_Addr& remoteHost_out,
                      bool& useSSL_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_process_arguments"));

  std::string working_directory =
    Common_File_Tools::getWorkingDirectory ();
  std::string temp_directory = Common_File_Tools::getTempDirectory ();
  std::string configuration_directory = working_directory;
  configuration_directory += ACE_DIRECTORY_SEPARATOR_STR_A;
  configuration_directory +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  // initialize results
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#endif
  debugParser_out = COMMON_PARSER_DEFAULT_YACC_TRACE;
#if defined (GTK_USE)
  GtkRcFileName_out = configuration_directory;
  GtkRcFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  GtkRcFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_RC_FILE);
#endif // GTK_USE
  fileName_out = temp_directory;
  fileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  fileName_out +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_DEFAULT_OUTPUT_FILE);
  UIDefinitonFileName_out = configuration_directory;
  UIDefinitonFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitonFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_GLADE_FILE);
  logToFile_out = false;
  port_out = 0;
  useReactor_out = (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
    (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S ? ACE_Time_Value (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S, 0)
                                                   : ACE_Time_Value::zero);
  traceInformation_out = false;
  URL_out = ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_DEFAULT_URL);
  printVersionAndExit_out = false;
  hostName_out.clear ();
  int result =
    remoteHost_out.set (static_cast<u_short> (HTTP_DEFAULT_SERVER_PORT),
                        static_cast<ACE_UINT32> (INADDR_LOOPBACK),
                        1,
                        0);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set (): \"%m\", aborting\n")));
    return false;
  } // end IF
  useSSL_out = false;

  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               ACE_TEXT ("cde:f:g:lrs:tu:v"),
#else
                               ACE_TEXT ("de:f:g:lrs:tu:v"),
#endif
                               1,                         // skip command name
                               1,                         // report parsing errors
                               ACE_Get_Opt::PERMUTE_ARGS, // ordering
                               0);                        // for now, don't use long options

  int option = 0;
  std::stringstream converter;
  while ((option = argument_parser ()) != EOF)
  {
    switch (option)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'c':
      {
        showConsole_out = true;
        break;
      }
#endif
      case 'd':
      {
        debugParser_out = true;
        break;
      }
      case 'e':
      {
#if defined (GTK_USE)
        GtkRcFileName_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
#endif // GTK_USE
        break;
      }
      case 'f':
      {
        fileName_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'g':
      {
        UIDefinitonFileName_out =
          ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'r':
      {
        useReactor_out = true;
        break;
      }
      case 's':
      {
        unsigned int value = 0;
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        converter >> value;
        statisticReportingInterval_out =
          (value ? ACE_Time_Value (value, 0) : ACE_Time_Value::zero);
        break;
      }
      case 't':
      {
        traceInformation_out = true;
        break;
      }
      case 'u':
      {
        URL_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());

        // step1: parse URL
        std::string URI_s;
        if (!HTTP_Tools::parseURL (URL_out,
                                   remoteHost_out,
                                   hostName_out,
                                   URI_s,
                                   useSSL_out))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                      ACE_TEXT (URL_out.c_str ())));
          return false;
        } // end IF

//        std::string hostname_string = hostName_out;
//        size_t position =
//          hostname_string.find_last_of (':', std::string::npos);
//        if (position == std::string::npos)
//        {
//          port_out = (useSSL_out ? HTTPS_DEFAULT_SERVER_PORT
//                                 : HTTP_DEFAULT_SERVER_PORT);
//          hostname_string += ':';
//          std::ostringstream converter;
//          converter << port_out;
//          hostname_string += converter.str ();
//        } // end IF
//        else
//        {
//          std::istringstream converter (hostname_string.substr (position + 1,
//                                                                std::string::npos));
//          converter >> port_out;
//        } // end ELSE
//        result = remoteHost_out.set (hostname_string.c_str (),
//                                     AF_INET);
//        if (result == -1)
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
//                      ACE_TEXT (hostname_string.c_str ())));
//          return false;
//        } // end IF

        // step2: validate address/verify host name exists
        //        --> resolve
        ACE_TCHAR buffer_a[HOST_NAME_MAX];
        ACE_OS::memset (buffer_a, 0, sizeof (ACE_TCHAR[HOST_NAME_MAX]));
        result = remoteHost_out.get_host_name (buffer_a,
                                               sizeof (ACE_TCHAR[HOST_NAME_MAX]));
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::get_host_name(): \"%m\", aborting\n")));
          return false;
        } // end IF
        std::string hostname = ACE_TEXT_ALWAYS_CHAR (buffer_a);
        std::string dotted_decimal_string;
        if (!Net_Common_Tools::getAddress (hostname,
                                           dotted_decimal_string))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_Common_Tools::getAddress(\"%s\"), aborting\n"),
                      ACE_TEXT (hostname.c_str ())));
          return false;
        } // end IF

        break;
      }
      case 'v':
      {
        printVersionAndExit_out = true;
        break;
      }
      // error handling
      case ':':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("option \"%c\" requires an argument, aborting\n"),
                    argument_parser.opt_opt ()));
        return false;
      }
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    ACE_TEXT (argument_parser.last_option ())));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    ACE_TEXT (argument_parser.long_option ())));
        return false;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("parse error, aborting\n")));
        return false;
      }
    } // end SWITCH
  } // end WHILE

  return true;
}

void
do_initialize_signals (bool allowUserRuntimeConnect_in,
                       ACE_Sig_Set& signals_out,
                       ACE_Sig_Set& ignoredSignals_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_initialize_signals"));

  int result = -1;

  // initialize return value(s)
  result = signals_out.empty_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
    return;
  } // end IF
  result = ignoredSignals_out.empty_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
    return;
  } // end IF

  // *PORTABILITY*: on Windows(TM) platforms most signals are not defined, and
  //                ACE_Sig_Set::fill_set() doesn't really work as specified
  // --> add valid signals (see <signal.h>)...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signals_out.sig_add (SIGINT);            // 2       /* interrupt */
  signals_out.sig_add (SIGILL);            // 4       /* illegal instruction - invalid function image */
  signals_out.sig_add (SIGFPE);            // 8       /* floating point exception */
  //  signals_out.sig_add (SIGSEGV);           // 11      /* segment violation */
  signals_out.sig_add (SIGTERM);           // 15      /* Software termination signal from kill */
  if (allowUserRuntimeConnect_in)
  {
    signals_out.sig_add (SIGBREAK);        // 21      /* Ctrl-Break sequence */
    ignoredSignals_out.sig_add (SIGBREAK); // 21      /* Ctrl-Break sequence */
  } // end IF
  signals_out.sig_add (SIGABRT);           // 22      /* abnormal termination triggered by abort call */
  signals_out.sig_add (SIGABRT_COMPAT);    // 6       /* SIGABRT compatible with other platforms, same as SIGABRT */
#else
  result = signals_out.fill_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::fill_set(): \"%m\", returning\n")));
    return;
  } // end IF
  // *NOTE*: cannot handle some signals --> registration fails for these...
  signals_out.sig_del (SIGKILL);           // 9       /* Kill signal */
  signals_out.sig_del (SIGSTOP);           // 19      /* Stop process */
  // ---------------------------------------------------------------------------
  if (!allowUserRuntimeConnect_in)
  {
    signals_out.sig_del (SIGUSR1);         // 10      /* User-defined signal 1 */
    ignoredSignals_out.sig_add (SIGUSR1);  // 10      /* User-defined signal 1 */
  } // end IF
  // *NOTE* core dump on SIGSEGV
  signals_out.sig_del (SIGSEGV);           // 11      /* Segmentation fault: Invalid memory reference */
  // *NOTE* don't care about SIGPIPE
  signals_out.sig_del (SIGPIPE);           // 12      /* Broken pipe: write to pipe with no readers */

#if defined (VALGRIND_USE)
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif // VALGRIND_USE
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (bool debugParser_in,
         const std::string& fileName_in,
         const std::string& UIDefinitionFileName_in,
         bool useReactor_in,
         const ACE_Time_Value& statisticReportingInterval_in,
         const std::string& URL_in,
         const ACE_INET_Addr& remoteHost_in,
         struct Test_I_URLStreamLoad_Configuration& configuration_in,
         struct Test_I_URLStreamLoad_UI_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#endif // ACE_WIN32 || ACE_WIN64
         Test_I_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  // step0a: initialize configuration and stream
#if defined (SSL_SUPPORT)
  std::string filename_string =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                      (Common_Error_Tools::inDebugSession () ? ACE_TEXT_ALWAYS_CHAR ("..") : ACE_TEXT_ALWAYS_CHAR ("")),
                                                      false); // data
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (NET_PROTOCOL_DEFAULT_SSL_TRUSTED_CAS_FILENAME_STRING);
  if (unlikely (!Net_Common_Tools::initializeSSLContext (ACE_TEXT_ALWAYS_CHAR (""),
                                                         ACE_TEXT_ALWAYS_CHAR (""),
                                                         filename_string, // trusted CAs
                                                         true,            // client
                                                         NULL)))          // default context
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to Net_Common_Tools::initializeSSLContext(), returning\n")));
    return;
  } // end IF
#endif // SSL_SUPPORT

  Test_I_EventHandler message_handler (&CBData_in);
  Test_I_Module_EventHandler_Module event_handler_module (NULL,
                                                          ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_Module_EventHandler_2_Module event_handler_module_2 (NULL,
                                                              ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;
  allocator_configuration.defaultBufferSize = 16384;
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  Test_I_MessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,         // heap allocator handle
                                               true);                   // block ?
  Test_I_MessageAllocator_2_t message_allocator_2 (NET_STREAM_MAX_MESSAGES, // maximum #buffers
                                                   &heap_allocator,         // heap allocator handle
                                                   true);                   // block ?

  // *********************** socket configuration data ************************
  Test_I_URLStreamLoad_ConnectionConfiguration_t connection_configuration;
  connection_configuration.socketConfiguration.address = remoteHost_in;
  connection_configuration.allocatorConfiguration = &allocator_configuration;
  connection_configuration.socketConfiguration.useLoopBackDevice =
    connection_configuration.socketConfiguration.address.is_loopback ();
//  connection_configuration.statisticReportingInterval =
//    statisticReportingInterval_in;
  connection_configuration.messageAllocator = &message_allocator;
  //connection_configuration.PDUSize = bufferSize_in;
  //connection_configuration.userData = &CBData_in.configuration->userData;
  connection_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;

  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                    &connection_configuration));
  Net_ConnectionConfigurationsIterator_t iterator =
    configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.connectionConfigurations.end ());

  Test_I_URLStreamLoad_ConnectionConfiguration_2_t connection_configuration_2;
  connection_configuration_2.allocatorConfiguration = &allocator_configuration;
  connection_configuration_2.socketConfiguration.useLoopBackDevice = false;
//  connection_configuration_2.statisticReportingInterval =
//    statisticReportingInterval_in;
  connection_configuration_2.messageAllocator = &message_allocator_2;
  connection_configuration_2.streamConfiguration =
    &configuration_in.streamConfiguration_2;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("2"),
                                                                    &connection_configuration_2));
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("2"));
  ACE_ASSERT (iterator_2 != configuration_in.connectionConfigurations.end ());

  // ********************** stream configuration data **************************
  // ********************** parser configuration data **************************
#if defined (_DEBUG)
  configuration_in.parserConfiguration.debugParser = debugParser_in;
  if (debugParser_in)
    configuration_in.parserConfiguration.debugScanner = true;
#endif // _DEBUG
  // ********************** module configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
  struct Test_I_URLStreamLoad_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_I_URLStreamLoad_StreamConfiguration stream_configuration;
  modulehandler_configuration.allocatorConfiguration =
    &allocator_configuration;
  modulehandler_configuration.closeAfterReception = true;
  modulehandler_configuration.concurrency =
      STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration.connectionConfigurations =
    &configuration_in.connectionConfigurations;
  modulehandler_configuration.defragmentMode = STREAM_DEFRAGMENT_CONDENSE;
  //modulehandler_configuration.connectionManager = connection_manager_p;
//  configuration_in.parserConfiguration.debugParser = true;
//  configuration_in.parserConfiguration.debugScanner = true;
  modulehandler_configuration.messageAllocator = &message_allocator;
  modulehandler_configuration.parserConfiguration =
    &configuration_in.parserConfiguration;
//  modulehandler_configuration.statisticReportingInterval =
//    statisticReportingInterval_in;
  modulehandler_configuration.subscriber = &message_handler;
  modulehandler_configuration.targetFileName = fileName_in;
  modulehandler_configuration.URL = URL_in;
  modulehandler_configuration.waitForConnect = false;
  // ******************** (sub-)stream configuration data *********************
  //if (bufferSize_in)
  //  CBData_in.configuration->allocatorConfiguration.defaultBufferSize =
  //    bufferSize_in;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = &event_handler_module;
  stream_configuration.printFinalReport = true;
  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);
  //configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
  //                                                             std::make_pair (module_configuration,
  //                                                                             modulehandler_configuration)));

  //module_handler_p->initialize (configuration.moduleHandlerConfiguration);

  struct Test_I_URLStreamLoad_ModuleHandlerConfiguration_2 modulehandler_configuration_2;
  modulehandler_configuration_2.allocatorConfiguration =
    &allocator_configuration;
  modulehandler_configuration_2.closeAfterReception = true;
#if defined (FFMPEG_SUPPORT)
  modulehandler_configuration_2.codecId = AV_CODEC_ID_H263;
#endif // FFMPEG_SUPPORT
  modulehandler_configuration_2.concurrency =
      STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration_2.connectionConfigurations =
    &configuration_in.connectionConfigurations;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration_2.outputFormat.subtype = MEDIASUBTYPE_RGB24;
#else
#if defined (FFMPEG_SUPPORT)
  modulehandler_configuration_2.outputFormat.format = AV_PIX_FMT_RGB24;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  modulehandler_configuration_2.parserConfiguration =
    &configuration_in.parserConfiguration;
//  modulehandler_configuration_2.statisticReportingInterval =
//    statisticReportingInterval_in;
  modulehandler_configuration_2.subscriber = &message_handler;
  modulehandler_configuration_2.targetFileName = fileName_in;
  modulehandler_configuration_2.URL = URL_in;
  modulehandler_configuration_2.waitForConnect = false;
  struct Test_I_URLStreamLoad_StreamConfiguration_2 stream_configuration_2;
  stream_configuration_2.messageAllocator = &message_allocator_2;
  stream_configuration_2.module = &event_handler_module_2;
  stream_configuration_2.printFinalReport = true;
  configuration_in.streamConfiguration_2.initialize (module_configuration,
                                                     modulehandler_configuration_2,
                                                     stream_configuration_2);

  // step0c: initialize connection manager
  Test_I_ConnectionManager_t* connection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (*static_cast<Test_I_URLStreamLoad_ConnectionConfiguration_t*> ((*iterator).second),
                             NULL);
  Test_I_ConnectionManager_2_t* connection_manager_2 =
    TEST_I_CONNECTIONMANAGER_SINGLETON_2::instance ();
  ACE_ASSERT (connection_manager_2);
  connection_manager_2->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_2->set (*static_cast<Test_I_URLStreamLoad_ConnectionConfiguration_2_t*> ((*iterator_2).second),
                             NULL);

  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  struct Common_TimerConfiguration timer_configuration;
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
#endif // GTK_USE
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
    &configuration_in.dispatchConfiguration;

  // step0b: initialize event dispatch
  configuration_in.dispatchConfiguration.numberOfReactorThreads =
    ((configuration_in.dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR) ? TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS
                                                                                        : 0);
  configuration_in.dispatchConfiguration.numberOfProactorThreads =
    ((configuration_in.dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_PROACTOR) ? TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS 
                                                                                         : 0);
  if (!Common_Event_Tools::initializeEventDispatch (configuration_in.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    goto clean;
  } // end IF

  //// step0d: initialize regular (global) statistic reporting
  //Stream_StatisticHandler_Reactor_t statistic_handler (ACTION_REPORT,
  //                                                     connection_manager_p,
  //                                                     false);
  ////Stream_StatisticHandler_Proactor_t statistic_handler_proactor (ACTION_REPORT,
  ////                                                               connection_manager_p,
  ////                                                               false);
  //long timer_id = -1;
  //if (statisticReportingInterval_in)
  //{
  //  ACE_Event_Handler* handler_p = &statistic_handler;
  //  ACE_Time_Value interval (statisticReportingInterval_in, 0);
  //  timer_id =
  //    timer_manager_p->schedule_timer (handler_p,                  // event handler
  //                                     NULL,                       // ACT
  //                                     COMMON_TIME_NOW + interval, // first wakeup time
  //                                     interval);                  // interval
  //  if (timer_id == -1)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));

  //    // clean up
  //    timer_manager_p->stop ();
  //    delete stream_p;

  //    return;
  //  } // end IF
  //} // end IF

  // step0c: initialize signal handling
  //CBData_in.configuration->signalHandlerConfiguration.hasUI =
  //  !interfaceDefinitionFile_in.empty ();
  configuration_in.signalHandlerConfiguration.dispatchState =
    &event_dispatch_state_s;
  //configuration.signalHandlerConfiguration.statisticReportingHandler =
  //  connection_manager_p;
  //configuration.signalHandlerConfiguration.statisticReportingTimerID = timer_id;
  if (!signalHandler_in.initialize (configuration_in.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    goto clean;
  } // end IF
  if (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
    goto clean;
  } // end IF

  // intialize timers
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);

  // step1a: start GTK event loop ?
  if (!UIDefinitionFileName_in.empty ())
  {
#if defined (GTK_USE)
    gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    gtk_manager_p->start (NULL);
    ACE_Time_Value timeout (0,
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
    int result = ACE_OS::sleep (timeout);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &timeout));
    if (!gtk_manager_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
      goto clean;
    } // end IF
#endif // GTK_USE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
      goto clean;
    } // end IF
    BOOL was_visible_b = false;
    if (!showConsole_in)
      was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif
  } // end IF

  // *WARNING*: from this point on, clean up any remote connections !

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting

  // step1a: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));

    // clean up
    //		{ // synch access
    //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

    //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
    //					 iterator != CBData_in.event_source_ids.end();
    //					 iterator++)
    //				g_source_remove(*iterator);
    //		} // end lock scope

    goto clean;
  } // end IF

#if defined (GTK_USE)
  gtk_manager_p->wait (false);
#endif // GTK_USE

  // step3: clean up
  connection_manager_p->stop (false, // wait ?
                              true); // high priority ?
  connection_manager_p->abort (false);
  connection_manager_p->wait ();
  connection_manager_2->stop (false, // wait ?
                              true); // high priority ?
  connection_manager_2->abort ();
  connection_manager_2->wait ();

  Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                             true); // wait ?

  timer_manager_p->stop ();

  //		{ // synch access
  //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //					 iterator != CBData_in.event_source_ids.end();
  //					 iterator++)
  //				g_source_remove(*iterator);
  //		} // end lock scope
  //timer_manager_p->stop ();

  //  { // synch access
  //    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //		for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //				 iterator != CBData_in.event_source_ids.end();
  //				 iterator++)
  //			g_source_remove(*iterator);
  //	} // end lock scope

  //if (!interfaceDefinitionFile_in.empty ())
  //{
  //  int result = event_handler_module.close (ACE_Module_Base::M_DELETE_NONE);
  //  if (result == -1)
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
  //                event_handler_module.name ()));
  //} // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  return;

clean:
  Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                             true); // wait ?
  timer_manager_p->stop ();
  if (!UIDefinitionFileName_in.empty ())
#if defined (GTK_USE)
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (true,  // wait ?
                                                        true); // high priority ?
#else
    ;
#endif // GTK_USE
}

void
do_print_version (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_print_version"));

  std::ostringstream converter;

  // compiler version string
  converter << ACE::compiler_major_version ();
  converter << ACE_TEXT (".");
  converter << ACE::compiler_minor_version ();
  converter << ACE_TEXT (".");
  converter << ACE::compiler_beta_version ();

  std::cout << programName_in
            << ACE_TEXT (" compiled on ")
            << ACE::compiler_name ()
            << ACE_TEXT (" ")
            << converter.str ()
            << std::endl << std::endl;

  std::cout << ACE_TEXT ("libraries: ")
            << std::endl
#if defined (HAVE_CONFIG_H)
            << ACE_TEXT (ACENetwork_PACKAGE_NAME)
            << ACE_TEXT (": ")
            << ACE_TEXT (ACENetwork_PACKAGE_VERSION)
            << std::endl
#endif // HAVE_CONFIG_H
            ;

  converter.str ("");
  // ACE version string
  converter << ACE::major_version ();
  converter << ACE_TEXT (".");
  converter << ACE::minor_version ();
  converter << ACE_TEXT (".");
  converter << ACE::beta_version ();

  // *NOTE*: cannot use ACE_VERSION, as it doesn't contain the (potential) beta
  // version number... Need this, as the library soname is compared to this
  // string
  std::cout << ACE_TEXT ("ACE: ")
//             << ACE_VERSION
            << converter.str ()
            << std::endl;
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  NETWORK_TRACE (ACE_TEXT ("::main"));

  int result = -1;
  ACE_Profile_Timer process_profile;
  std::string path;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console;
#endif // ACE_WIN32 || ACE_WIN64
  bool debug_parser;
  std::string configuration_path;
  std::string output_file;
  std::string hostname;
  std::string ui_definition_file;
  bool log_to_file;
  unsigned short port;
  bool use_reactor;
  ACE_Time_Value statistic_reporting_interval;
  bool trace_information;
  std::string url;
  bool print_version_and_exit;
  ACE_INET_Addr address;
  bool use_ssl;
  struct Test_I_URLStreamLoad_Configuration configuration;
  struct Test_I_URLStreamLoad_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;
#if defined (GTK_USE)
  std::string gtk_rc_file;
  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
#endif // GTK_USE
  std::string log_file_name;
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
  Test_I_SignalHandler signal_handler;
#if defined (GTK_USE)
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  configuration.GTKConfiguration.argc = argc_in;
  configuration.GTKConfiguration.argv = argv_in;
  configuration.GTKConfiguration.CBData = &ui_cb_data;
  configuration.GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  configuration.GTKConfiguration.eventHooks.initHook =
      idle_initialize_UI_cb;
  configuration.GTKConfiguration.definition = &gtk_ui_definition;
#endif // GTK_USE
  ACE_High_Res_Timer timer;
  std::string working_time_string;
  ACE_Time_Value working_time;
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  ACE_Profile_Timer::Rusage elapsed_rusage;
  std::string user_time_string, system_time_string;
  ACE_Time_Value user_time, system_time;

  // step0: initialize
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::init ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  process_profile.start ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (false,  // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64
  Common_File_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (argv_in[0]));

  // step1a set defaults
#if defined (GTK_USE)
  gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ui_cb_data.UIState = &state_r;
#endif // GTK_USE
  Common_Logger_Queue_t logger;
  logger.initialize (&state_r.logQueue,
                     &state_r.logQueueLock);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  show_console = false;
#endif // ACE_WIN32 || ACE_WIN64
  debug_parser = COMMON_PARSER_DEFAULT_YACC_TRACE;
  path =
    Common_File_Tools::getWorkingDirectory ();
  configuration_path = path;
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GTK_USE)
  gtk_rc_file = configuration_path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_RC_FILE);
#endif // GTK_USE
  output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_DEFAULT_OUTPUT_FILE);
  ui_definition_file = configuration_path;
  ui_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_GLADE_FILE);
  log_to_file = false;
  port = HTTP_DEFAULT_SERVER_PORT;
  use_reactor = (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statistic_reporting_interval =
    (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S ? ACE_Time_Value (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S, 0)
                                                   : ACE_Time_Value::zero);
  trace_information = false;
  url = ACE_TEXT_ALWAYS_CHAR (TEST_I_URLSTREAMLOAD_DEFAULT_URL);
  print_version_and_exit = false;
  use_ssl = false;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));

  // step1b: parse/process/validate configuration
  if (!do_process_arguments (argc_in,
                             argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             show_console,
#endif // ACE_WIN32 || ACE_WIN64
                             debug_parser,
#if defined (GTK_USE)
                             gtk_rc_file,
#endif // GTK_USE
                             output_file,
                             ui_definition_file,
                             log_to_file,
                             port,
                             use_reactor,
                             statistic_reporting_interval,
                             trace_information,
                             url,
                             print_version_and_exit,
                             hostname,
                             address,
                             use_ssl))
  {
    do_print_usage (std::string (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                                      ACE_DIRECTORY_SEPARATOR_CHAR))));
    goto error;
  } // end IF

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_I_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  //if (use_reactor                      &&
  //    (number_of_dispatch_threads > 1) &&
  //    !use_thread_pool)
  //{ // *NOTE*: see also: man (2) select
  //  // *TODO*: verify this for MS Windows based systems
  //  ACE_DEBUG ((LM_WARNING,
  //              ACE_TEXT ("the select()-based reactor is not reentrant, using the thread-pool reactor instead...\n")));
  //  use_thread_pool = true;
  //} // end IF
  if ((!ui_definition_file.empty () &&
       !Common_File_Tools::isReadable (ui_definition_file))
#if defined (GTK_USE)
      || (!gtk_rc_file.empty () &&
          !Common_File_Tools::isReadable (gtk_rc_file)))//                       ||
#else
     )
#endif // GTK_USE
      //(use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));
    do_print_usage (std::string (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                                      ACE_DIRECTORY_SEPARATOR_CHAR))));
    goto error;
  } // end IF

  // step1d: initialize logging and/or tracing
  if (log_to_file)
  {
    log_file_name =
      Common_Log_Tools::getLogFilename (std::string (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME)),
                                        std::string (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                                                          ACE_DIRECTORY_SEPARATOR_CHAR))));
    if (log_file_name.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::getLogFilename(), aborting\n")));
      goto error;
    } // end IF
  } // end IF
  if (!Common_Log_Tools::initialize (std::string (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR))), // program name
                                     log_file_name,                                                                     // log file name
                                     false,                                                                             // log to syslog ?
                                     false,                                                                             // trace messages ?
                                     trace_information,                                                                 // debug messages ?
                                     NULL))                                                                             // (ui-) logger ?
//                                            (ui_definition_file.empty () ? NULL
//                                                                         : &logger)))                                          // (ui-) logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));
    goto error;
  } // end IF

  // step1e: pre-initialize signal handling
  do_initialize_signals (true, // allow SIGUSR1/SIGBREAK
                         signal_set,
                         ignored_signal_set);
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           (use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                           true,  // using networking ?
                                           false, // using asynch timers ?
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));
    goto error;
  } // end IF

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_print_version (std::string (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR))));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_SUCCESS;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_OS_Tools::setResourceLimits (false, // file descriptors
                                           true,  // stack traces
                                           true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_OS_Tools::setResourceLimits(), aborting\n")));
    goto error;
  } // end IF

#if defined (GTK_USE)
  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
  if (!gtk_rc_file.empty ())
    ui_cb_data.configuration->GTKConfiguration.RCFiles.push_back (gtk_rc_file);
#endif // GTK_USE
  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  if (!ui_definition_file.empty ())
  {
    ui_cb_data.configuration = &configuration;
#if defined (GTK_USE)
//    ACE_ASSERT (CBData_in.UIState);
    state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (ui_definition_file, static_cast<GtkBuilder*> (NULL));
    ui_cb_data.progressData.state = &state_r;
    //ui_cb_data.userData = &configuration.userData;
    if (!gtk_manager_p->initialize (configuration.GTKConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager::initialize(), aborting\n")));
      goto error;
    } // end IF
#endif // GTK_USE
  } // end IF

  configuration.dispatchConfiguration.dispatch =
    (use_reactor ? COMMON_EVENT_DISPATCH_REACTOR
                 : COMMON_EVENT_DEFAULT_DISPATCH);

  timer.start ();
  // step2: do actual work
  do_work (debug_parser,
           output_file,
           ui_definition_file,
           use_reactor,
           statistic_reporting_interval,
           url,
           address,
           configuration,
           ui_cb_data,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#endif // ACE_WIN32 || ACE_WIN64
           signal_handler);
  timer.stop ();

  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));
  process_profile.stop ();

  // only process profile left to do
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  result = process_profile.elapsed_time (elapsed_time);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));
    goto error;
  } // end IF
  process_profile.elapsed_rusage (elapsed_rusage);
  user_time.set (elapsed_rusage.ru_utime);
  system_time.set (elapsed_rusage.ru_stime);
  user_time_string = Common_Timer_Tools::periodToString (user_time);
  system_time_string = Common_Timer_Tools::periodToString (system_time);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\nmaximum resident set size = %d\nintegral shared memory size = %d\nintegral unshared data size = %d\nintegral unshared stack size = %d\npage reclaims = %d\npage faults = %d\nswaps = %d\nblock input operations = %d\nblock output operations = %d\nmessages sent = %d\nmessages received = %d\nsignals received = %d\nvoluntary context switches = %d\ninvoluntary context switches = %d\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ()),
              elapsed_rusage.ru_maxrss,
              elapsed_rusage.ru_ixrss,
              elapsed_rusage.ru_idrss,
              elapsed_rusage.ru_isrss,
              elapsed_rusage.ru_minflt,
              elapsed_rusage.ru_majflt,
              elapsed_rusage.ru_nswap,
              elapsed_rusage.ru_inblock,
              elapsed_rusage.ru_oublock,
              elapsed_rusage.ru_msgsnd,
              elapsed_rusage.ru_msgrcv,
              elapsed_rusage.ru_nsignals,
              elapsed_rusage.ru_nvcsw,
              elapsed_rusage.ru_nivcsw));
#endif // ACE_WIN32 || ACE_WIN64

  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalize ();
  Common_Tools::finalize ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;

error:
  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalize ();
  Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
  return EXIT_FAILURE;
} // end main
