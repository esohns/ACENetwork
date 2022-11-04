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

#include <iostream>
#include <limits>
#include <regex>
#include <string>

#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
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

#include "common.h"

#include "common_event_tools.h"

#include "common_log_tools.h"
#include "common_logger.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_dec_common.h"

#include "stream_net_http_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"
#include "net_defines.h"

#include "http_defines.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_module_spreadsheetwriter.h"

#include "test_i_connection_manager_common.h"
#include "test_i_signalhandler.h"
#include "test_i_stream.h"
#include "test_i_trending_common.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("HTTPGetStream");

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

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
  std::string configuration_file = path;
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_LIBREOFFICE_BOOTSTRAP_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [FILE]   : LibreOffice bootstrap .ini [")
            << configuration_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : debug parser [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  configuration_file = path;
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_INPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [FILE]   : LibreOffice calc template [")
            << configuration_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-h [HOST]   : LibreOffice application hostname [")
            << ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i [ISIN]   : stock index identifier [")
            << ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_ISIN)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o [FILE]   : (output) LibreOffice spreadsheet .ods [")
            << output_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : LibreOffice application port number [")
            << TEST_I_DEFAULT_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [STRING] : stock index value URL [")
            << TEST_I_DEFAULT_URL
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]  : #dispatch threads [")
            << TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     std::string& bootstrapFileName_out,
                     bool& debug_out,
                     std::string& templateFileName_out,
                     std::string& hostName_out,
                     std::string& ISIN_out,
                     bool& logToFile_out,
                     std::string& outputFileName_out,
                     unsigned short& port_out,
                     bool& useReactor_out,
                     bool& traceInformation_out,
                     std::string& URI_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out,
                     ACE_INET_Addr& remoteHost_out,
                     bool& useSSL_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string path = Common_File_Tools::getWorkingDirectory ();

  // initialize results
  bootstrapFileName_out = path;
  bootstrapFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrapFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  bootstrapFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrapFileName_out +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_LIBREOFFICE_BOOTSTRAP_FILE);
  debug_out = COMMON_PARSER_DEFAULT_YACC_TRACE;
  templateFileName_out = path;
  templateFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  templateFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  templateFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  templateFileName_out +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_INPUT_FILE);
  hostName_out = ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST);
  ISIN_out = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_ISIN);
  logToFile_out = false;
  outputFileName_out = path;
  outputFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  outputFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  port_out = TEST_I_DEFAULT_PORT;
  useReactor_out =
          (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  traceInformation_out = false;
  URI_out = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_URL);
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS;
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

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("c:df:h:i:lo:p:rtu:vx:"),
                              1,                         // skip command name
                              1,                         // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS, // ordering
                              0);                        // for now, don't use long options

  int option = 0;
  std::istringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'c':
      {
        bootstrapFileName_out =
            ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'd':
      {
        debug_out = true;
        break;
      }
      case 'f':
      {
        templateFileName_out =
            ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'h':
      {
        hostName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'i':
      {
        ISIN_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'o':
      {
        outputFileName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'p':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.str (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
        converter >> port_out;
        break;
      }
      case 'r':
      {
        useReactor_out = true;
        break;
      }
      case 't':
      {
        traceInformation_out = true;
        break;
      }
      case 'u':
      {
        URI_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());

        break;
      }
      case 'v':
      {
        printVersionAndExit_out = true;
        break;
      }
      case 'x':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.str (argumentParser.opt_arg ());
        converter >> numberOfDispatchThreads_out;
        break;
      }
      // error handling
      case ':':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("option \"%c\" requires an argument, aborting\n"),
                    argumentParser.opt_opt ()));
        return false;
      }
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    ACE_TEXT (argumentParser.last_option ())));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    ACE_TEXT (argumentParser.long_option ())));
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

  // step2: parse URL
  ACE_INET_Addr host_address;
  std::string hostname_string;
  std::string URI_s;
  if (!HTTP_Tools::parseURL (URI_out,
                             remoteHost_out,
                             hostname_string,
                             URI_s,
                             useSSL_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                ACE_TEXT (URI_out.c_str ())));
    return false;
  } // end IF

  // step2: validate address/verify host name exists
  //        --> resolve
  ACE_TCHAR buffer[HOST_NAME_MAX];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result = remoteHost_out.get_host_name (buffer,
                                         sizeof (buffer));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::get_host_name(): \"%m\", aborting\n")));
    return false;
  } // end IF
  std::string hostname = ACE_TEXT_ALWAYS_CHAR (buffer);
  std::string dotted_decimal_string;
  if (!Net_Common_Tools::getAddress (hostname,
                                     dotted_decimal_string))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::getAddress(), aborting\n")));
    return false;
  } // end IF

  //// step3: validate URI
  //std::string regex_string =
  //    ACE_TEXT_ALWAYS_CHAR ("^(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
  ////regex_string =
  ////    ACE_TEXT_ALWAYS_CHAR ("^(?:http(?:s)?://)?((.+\\.)+([^\\/]+))(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
  //std::regex regex;
  //regex.assign (regex_string,
  //              (std::regex_constants::ECMAScript |
  //               std::regex_constants::icase));
  //std::smatch match_results_3;
  //if (!std::regex_match (URI_out,
  //                       match_results_3,
  //                       regex,
  //                       std::regex_constants::match_default))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("invalid URI (was: \"%s\"), aborting\n"),
  //              ACE_TEXT (URI_out.c_str ())));
  //  return false;
  //} // end IF
  //ACE_ASSERT (match_results_3.ready () && !match_results_3.empty ());

  //if (!match_results_3[2].matched)
  //  URI_out += ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_NET_SOURCE_HTTP_GET_DEFAULT_URL);
  ////else if (!match_results_3[3].matched)
  ////  URI_out += ACE_TEXT_ALWAYS_CHAR (HTML_DEFAULT_SUFFIX);

  return true;
}

void
do_initializeSignals (ACE_Sig_Set& signals_out,
                      ACE_Sig_Set& ignoredSignals_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_initializeSignals"));

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
  signals_out.sig_add (SIGBREAK);        // 21      /* Ctrl-Break sequence */
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
do_work (const std::string& bootstrapFileName_in,
         bool debug_in,
         const std::string& templateFileName_in,
         const std::string& hostName_in,
         const std::string& ISIN_in,
         const std::string& fileName_in,
         unsigned short port_in,
         bool useReactor_in,
         const ACE_INET_Addr& remoteHost_in,
         bool useSSL_in,
         const std::string& URL_in,
         unsigned int numberOfDispatchThreads_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_I_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

//#if defined (SSL_SUPPORT)
//  std::string data_directory_string =
//    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
//                                                      ACE_TEXT_ALWAYS_CHAR (""),
//                                                      false); // data
//  std::string filename_string = data_directory_string;
//  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR;
//  filename_string +=
//    ACE_TEXT_ALWAYS_CHAR (NET_PROTOCOL_DEFAULT_SSL_TRUSTED_CAS_FILENAME_STRING);
//  data_directory_string =
//    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
//                                                      ACE_TEXT_ALWAYS_CHAR (""),
//                                                      true); // config
//  std::string certificate_string = data_directory_string;
//  certificate_string += ACE_DIRECTORY_SEPARATOR_CHAR;
//  certificate_string +=
//    ACE_TEXT_ALWAYS_CHAR (NET_PROTOCOL_DEFAULT_SSL_CERTIFICATE_FILENAME_STRING);
//  std::string key_string = data_directory_string;
//  key_string += ACE_DIRECTORY_SEPARATOR_CHAR;
//  key_string +=
//    ACE_TEXT_ALWAYS_CHAR (NET_PROTOCOL_DEFAULT_SSL_KEY_FILENAME_STRING);
//  if (unlikely (!Net_Common_Tools::initializeSSLContext (certificate_string,
//                                                         key_string,
//                                                         filename_string, // trusted CAs
//                                                         true,            // client
//                                                         NULL)))          // default context
//  {
//    ACE_DEBUG ((LM_ERROR,
//               ACE_TEXT ("failed to Net_Common_Tools::initializeSSLContext(), returning\n")));
//    return;
//  } // end IF
//#endif // SSL_SUPPORT

  bool finalize_event_dispatch = false;
  bool stop_timers = false;
//  int group_id = -1;
  int result = -1;
  Common_IInitialize_T<Test_I_Trending_StreamConfiguration_t>* iinitialize_p =
    NULL;
  Stream_IStreamControlBase* istream_base_p = NULL;
//  Stream_IStream_t* istream_p = NULL;
  Stream_Base* stream_base_p = NULL;
  Test_I_StockItem stock_item;
  Test_I_Trending_Configuration configuration;
  Test_I_Trending_InetConnectionManager_t* connection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  Net_StreamStatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                                  connection_manager_p,
                                                  false);
  Common_Timer_Manager_t* timer_manager_p = NULL;
  struct Common_TimerConfiguration timer_configuration;
  struct Common_EventDispatchConfiguration event_dispatch_configuration_s;
//  ACE_thread_t thread_id = 0;
  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_AllocatorHeap_T::initialize, returning\n")));
  //  goto error;
  //} // end IF
  Test_I_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?
  Test_I_Trending_ConnectionConfiguration_t connection_configuration;
  struct Stream_ModuleConfiguration module_configuration;
  struct Test_I_Trending_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_I_Trending_ModuleHandlerConfiguration modulehandler_configuration_2; // connection-
  struct Test_I_Trending_StreamConfiguration stream_configuration;
  struct Test_I_Trending_StreamConfiguration stream_configuration_2; // connection-
  Net_ConnectionConfigurationsIterator_t iterator;
  struct Common_EventDispatchState event_dispatch_state_s;
  struct Net_UserData user_data_s;

  // step0a: initialize configuration and stream
  if (useReactor_in)
  {
    if (useSSL_in)
#if defined (SSL_SUPPORT)
      ACE_NEW_NORETURN (stream_base_p,
                        Test_I_Trending_SSL_Stream_t ());
#else
      ;
#endif // SSL_SUPPORT
    else
      ACE_NEW_NORETURN (stream_base_p,
                        Test_I_Trending_Stream_t ());
  } // end IF
  else
    ACE_NEW_NORETURN (stream_base_p,
                      Test_I_Trending_AsynchStream_t ());
  if (!stream_base_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto error;
  } // end IF
  //configuration.userData.connectionConfiguration =
  //    &configuration.connectionConfiguration;
  //configuration.userData.streamConfiguration =
  //    &configuration.streamConfiguration;
  if (useReactor_in)
    configuration.dispatchConfiguration.numberOfReactorThreads =
      numberOfDispatchThreads_in;
  else
    configuration.dispatchConfiguration.numberOfProactorThreads =
      numberOfDispatchThreads_in;

  // *********************** socket configuration data ************************
  connection_configuration.socketConfiguration.address = remoteHost_in;
  connection_configuration.socketConfiguration.hostname =
    ACE_TEXT_ALWAYS_CHAR ("www.tagesschau.de");
  connection_configuration.socketConfiguration.useLoopBackDevice =
    connection_configuration.socketConfiguration.address.is_loopback ();
  //connection_configuration.socketConfiguration.version = TLS1_VERSION;

  //connection_configuration.writeOnly = true;
  connection_configuration.messageAllocator = &message_allocator;
  connection_configuration.allocatorConfiguration = &allocator_configuration;
  connection_configuration.allocatorConfiguration->defaultBufferSize =
    TEST_I_DEFAULT_BUFFER_SIZE;
  connection_configuration.streamConfiguration =
    &configuration.streamConfiguration_2;

  configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                 &connection_configuration));
  iterator =
    configuration.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration.connectionConfigurations.end ());

  // ********************** stream configuration data **************************
  // ********************** parser configuration data **************************
#if defined (_DEBUG)
  configuration.parserConfiguration.debugParser = debug_in;
  if (debug_in)
    configuration.parserConfiguration.debugScanner = true;
#endif // _DEBUG
  // ********************** module configuration data **************************
  modulehandler_configuration.allocatorConfiguration =
    &allocator_configuration;
  modulehandler_configuration.configuration = &configuration;
  modulehandler_configuration.connectionConfigurations =
    &configuration.connectionConfigurations;
  modulehandler_configuration.connectionManager =
    connection_manager_p;
  modulehandler_configuration.fileName = templateFileName_in;
  result =
    modulehandler_configuration.libreOfficeHost.set (port_in,
                                                     hostName_in.c_str (),
                                                     1,
                                                     AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                ACE_TEXT (hostName_in.c_str ()),
                port_in));
    goto error;
  } // end IF
  modulehandler_configuration.libreOfficeRc = bootstrapFileName_in;
  modulehandler_configuration.parserConfiguration =
      &configuration.parserConfiguration;
  modulehandler_configuration.passive = false;
  stock_item.ISIN = ISIN_in;
  modulehandler_configuration.stockItems.insert (stock_item);
  modulehandler_configuration.streamConfiguration = &configuration.streamConfiguration;
  modulehandler_configuration.fileIdentifier.identifier = fileName_in;
  //modulehandler_configuration.hostName = hostName_in;

  //modulehandler_configuration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_ACCEPT_HEADER_STRING),
  //                                                                ACE_TEXT_ALWAYS_CHAR ("text/html, application/xhtml+xml, */*")));
  //modulehandler_configuration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_ACCEPT_REFERER_STRING),
  //                                                                ACE_TEXT_ALWAYS_CHAR ("http://kurse.boerse.ard.de/ard/kurse_einzelkurs_uebersicht.htn?i=118700&suchbegriff=US0079031078&exitPoint=")));
  //modulehandler_configuration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_LANGUAGE_HEADER_STRING),
  //                                                                ACE_TEXT_ALWAYS_CHAR ("de-DE,de;q=0.8,en-US;q=0.5,en;q=0.3")));
  //modulehandler_configuration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_ENCODING_HEADER_STRING),
  //                                                                ACE_TEXT_ALWAYS_CHAR ("gzip, deflate")));
  //modulehandler_configuration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_COOKIE_HEADER_STRING),
  //                                                                ACE_TEXT_ALWAYS_CHAR ("xtvrn=$452061$; backlink=http://boerse.ard.de/index.html; usf_mobil=1; USF-C-usf_mobil=1")));
  modulehandler_configuration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_HOST_STRING),
                                                                  ACE_TEXT_ALWAYS_CHAR ("www.tagesschau.de")));
  //modulehandler_configuration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_AGENT_HEADER_STRING),
  //                                                                ACE_TEXT_ALWAYS_CHAR ("Mozilla/5.0 (Windows NT 6.3; WOW64; Trident/7.0; rv:11.0) like Gecko")));
  //modulehandler_configuration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_TRACKING_HEADER_STRING),
  //                                                                ACE_TEXT_ALWAYS_CHAR ("1")));
  modulehandler_configuration.HTTPHeaders.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONNECTION_STRING),
                                                                  ACE_TEXT_ALWAYS_CHAR ("Keep-Alive")));

  modulehandler_configuration.URL = URL_in;
  modulehandler_configuration_2 = modulehandler_configuration;
  modulehandler_configuration_2.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;

  // ******************** (sub-)stream configuration data **********************
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.printFinalReport = true;
  stream_configuration_2 = stream_configuration;
  stream_configuration_2.printFinalReport = false;
  configuration.streamConfiguration.initialize (module_configuration,
                                                modulehandler_configuration,
                                                stream_configuration);
  configuration.streamConfiguration_2.initialize (module_configuration,
                                                  modulehandler_configuration_2,
                                                  stream_configuration_2);

  // step0b: initialize event dispatch
  event_dispatch_configuration_s.numberOfProactorThreads =
          (!useReactor_in ? numberOfDispatchThreads_in : 0);
  event_dispatch_configuration_s.numberOfReactorThreads =
          (useReactor_in ? numberOfDispatchThreads_in : 0);
  if (!Common_Event_Tools::initializeEventDispatch (event_dispatch_configuration_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    goto error;
  } // end IF

  // step0c: (re-)configure connection manager
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (NET_STATISTIC_DEFAULT_COLLECTION_INTERVAL_MS, 0));
  connection_manager_p->set (*static_cast<Test_I_Trending_ConnectionConfiguration_t*> ((*iterator).second),
                             &user_data_s);

  // step0d: initialize regular (global) statistic reporting
  timer_manager_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);
  stop_timers = true;
  //Stream_StatisticHandler_Proactor_t statistics_handler_proactor (ACTION_REPORT,
  //                                                                connection_manager_p,
  //                                                                false);
  //long timer_id = -1;
  //if (statisticReportingInterval_in)
  //{
  //  ACE_Event_Handler* handler_p = &statistics_handler;
  //  ACE_Time_Value interval (statisticReportingInterval_in, 0);
  //  timer_id =
  //    timer_manager_p->schedule_timer (handler_p,                  // event handler
  //                                     NULL,                       // ACT
  //                                     COMMON_TIME_NOW + interval, // first wakeup time
  //                                     interval);                  // interval
  //  if (timer_id == -1)
  //  {
  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));

  //    // clean up
  //    timer_manager_p->stop ();
  //    delete stream_p;

  //    return;
  //  } // end IF
  //} // end IF

  // step0c: initialize signal handling
  configuration.signalHandlerConfiguration.dispatchState =
      &event_dispatch_state_s;
  //configuration.signalHandlerConfiguration.statisticReportingHandler =
  //  connection_manager_p;
  //configuration.signalHandlerConfiguration.statisticReportingTimerID = timer_id;
  signalHandler_in.initialize (configuration.signalHandlerConfiguration);
  if (!Common_Signal_Tools::initialize ((useReactor_in ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                       : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), aborting\n")));
    goto error;
  } // end IF

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting

  // step1a: initialize worker(s)
  event_dispatch_state_s.configuration =
      &event_dispatch_configuration_s;
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

    goto error;
  } // end IF
  finalize_event_dispatch = true;

  iinitialize_p =
    dynamic_cast<Common_IInitialize_T<Test_I_Trending_StreamConfiguration_t>*> (stream_base_p);
  ACE_ASSERT (iinitialize_p);
  if (!iinitialize_p->initialize (configuration.streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, aborting\n")));
    goto error;
  } // end IF

  // *NOTE*: this call blocks until the file has been sent (or an error
  //         occurs)
  istream_base_p = dynamic_cast<Stream_IStreamControlBase*> (stream_base_p);
  ACE_ASSERT (istream_base_p);
  istream_base_p->start ();
  //    if (!stream_p->isRunning ())
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to start stream, aborting\n")));

  //      // clean up
  //      //timer_manager_p->stop ();

  //      return;
  //    } // end IF
  istream_base_p->wait (true, false, false);

  // step3: clean up
  connection_manager_p->stop (false, true);
  connection_manager_p->abort ();
  connection_manager_p->wait ();
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

  //result = event_handler.close (ACE_Module_Base::M_DELETE_NONE);
  //if (result == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
  //              event_handler.name ()));
  delete stream_base_p;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  return;

error:
  if (finalize_event_dispatch)
    Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                               true); // wait ?
  if (stop_timers)
    timer_manager_p->stop ();
  if (stream_base_p)
    delete stream_base_p;
}

COMMON_DEFINE_PRINTVERSION_FUNCTION(do_printVersion,STREAM_MAKE_VERSION_STRING_VARIABLE(programName_in,ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  STREAM_TRACE (ACE_TEXT ("::main"));

  int result = EXIT_FAILURE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool finalize_ACE = false;
#endif // ACE_WIN32 || ACE_WIN64
  bool finalize_logging = false;
  bool finalize_signals = false;
  ACE_Profile_Timer process_profile;

  std::string path, bootstrap_file, template_file;
  std::string ISIN_string, host_name, output_file;
  bool debug, log_to_file, use_thread_pool, use_reactor, trace_information;
  unsigned short port;
  std::string URL;
  bool print_version_and_exit;
  unsigned int number_of_dispatch_threads;
  ACE_INET_Addr remote_host;
  bool use_SSL;

  std::string log_file_name;

  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
  Test_I_SignalHandler signal_handler;

  ACE_High_Res_Timer timer;
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_Time_Value user_time, system_time, working_time;
  std::string user_time_string, system_time_string, working_time_string;

  // step0: initialize
  // *PORTABILITY*: on Windows, initialize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result_2 = ACE::init ();
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  finalize_ACE = true;
#endif // ACE_WIN32 || ACE_WIN64

  // start profile timer
  process_profile.start ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (false,  // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64

  path = Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  bootstrap_file = path;
  bootstrap_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrap_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  bootstrap_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  bootstrap_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_LIBREOFFICE_BOOTSTRAP_FILE);
  debug = COMMON_PARSER_DEFAULT_YACC_TRACE;
  template_file = path;
  template_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  template_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  template_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  template_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_INPUT_FILE);
  host_name.clear ();
  ISIN_string = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_ISIN);
  log_to_file = false;
  output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  port = TEST_I_DEFAULT_PORT;
  use_reactor =
          (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  trace_information = false;
  use_thread_pool = false;
  URL = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_URL);
  print_version_and_exit = false;
  number_of_dispatch_threads =
    TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS;
  result = remote_host.set (static_cast<u_short> (HTTP_DEFAULT_SERVER_PORT),
                            static_cast<ACE_UINT32> (INADDR_LOOPBACK),
                            1,
                            0);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  use_SSL = false;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            bootstrap_file,
                            debug,
                            template_file,
                            host_name,
                            ISIN_string,
                            log_to_file,
                            output_file,
                            port,
                            use_reactor,
                            trace_information,
                            URL,
                            print_version_and_exit,
                            number_of_dispatch_threads,
                            remote_host,
                            use_SSL))
  {
    do_printUsage (ACE::basename (argv_in[0]));
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
  if (use_reactor                      &&
      (number_of_dispatch_threads > 1) &&
      !use_thread_pool)
  { // *NOTE*: see also: man (2) select
    // *TODO*: verify this for MS Windows based systems
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the select()-based reactor is not reentrant, using the thread-pool reactor instead...\n")));
    use_thread_pool = true;
  } // end IF
  if ((bootstrap_file.empty () ||
       !Common_File_Tools::isReadable (bootstrap_file)) ||
      ISIN_string.empty ()                              ||
      output_file.empty ()                              ||
      host_name.empty ()                                ||
      //(use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool) ||
      URL.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));
    do_printUsage (ACE::basename (argv_in[0]));
    goto error;
  } // end IF
  if (number_of_dispatch_threads == 0) number_of_dispatch_threads = 1;

  // step1d: initialize logging and/or tracing
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                        ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR)));
  if (!Common_Log_Tools::initializeLogging (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR)), // program name
                                            log_file_name,                        // log file name
                                            false,                                // log to syslog ?
                                            false,                                // trace messages ?
                                            trace_information,                    // debug messages ?
                                            NULL))                                // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));
    goto error;
  } // end IF
  finalize_logging = true;

  // step1e: pre-initialize signal handling
  do_initializeSignals (signal_set,
                        ignored_signal_set);
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           (use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                        : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                           true,  // using networking ?
                                           false, // using asynch timers ?
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));
    goto error;
  } // end IF
  finalize_signals = true;

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    result = EXIT_SUCCESS;

    goto continue_;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false, // file descriptors
                                        true,  // stack traces
                                        true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));
    goto error;
  } // end IF

  timer.start ();
  // step2: do actual work
  do_work (bootstrap_file,
           debug,
           template_file,
           host_name,
           ISIN_string,
           output_file,
           port,
           use_reactor,
           remote_host,
           use_SSL,
           URL,
           number_of_dispatch_threads,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
  timer.stop ();

  // debug info
  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  // stop profile timer
  process_profile.stop ();
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
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  user_time.set (elapsed_rusage.ru_utime);
  system_time.set (elapsed_rusage.ru_stime);
  user_time_string = Common_Timer_Tools::periodToString (user_time);
  system_time_string = Common_Timer_Tools::periodToString (system_time);

  // debug info
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

  result = EXIT_SUCCESS;

continue_:
error:
  if (finalize_signals)
    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
  if (finalize_logging)
    Common_Log_Tools::finalizeLogging ();
  Common_Tools::finalize ();
  // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (finalize_ACE)
  {
    int result_2 = ACE::fini ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  return result;
} // end main
