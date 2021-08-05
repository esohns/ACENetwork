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

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN32
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
//#include "ace/Synch.h"
#include "ace/Version.h"

#if defined (VALGRIND_SUPPORT)
#include "valgrind/valgrind.h"
#endif // VALGRIND_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common.h"
//#include "common_file_tools.h"
#include "common_tools.h"

#include "common_logger.h"
#include "common_log_tools.h"

#include "common_parser_common.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "ace/Synch.h"
#include "stream_file_sink.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"

#include "http_defines.h"
#include "http_scanner.h"

#include "test_u_common.h"
#include "test_u_defines.h"

#include "test_u_common_modules.h"
#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_HTTP_decoder_common.h"
#include "test_u_HTTP_decoder_defines.h"
#include "test_u_HTTP_decoder_signalhandler.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("HTTPDecoderStream");

void
do_printUsage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_printUsage"));

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b [VALUE]  : buffer size (byte(s)) [")
            << TEST_U_DEFAULT_BUFFER_SIZE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : debug parser [")
            << COMMON_PARSER_DEFAULT_YACC_TRACE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::string output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : (output) file name [")
            << output_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : port number [")
            << HTTP_DEFAULT_SERVER_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [STRING] : URL")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]  : #dispatch threads [")
            << TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& bufferSize_out,
                     bool& debugParser_out,
                     std::string& outputFileName_out,
                     std::string& hostName_out,
                     bool& logToFile_out,
                     unsigned short& port_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& useSSL_out,
                     std::string& URL_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  int result = -1;
  std::string path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  bufferSize_out = TEST_U_DEFAULT_BUFFER_SIZE;
  debugParser_out = COMMON_PARSER_DEFAULT_YACC_TRACE;
  outputFileName_out = path;
  outputFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  outputFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_OUTPUT_FILE);
  hostName_out.clear ();
  logToFile_out = false;
  port_out = HTTP_DEFAULT_SERVER_PORT;
  useReactor_out =
          (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out = STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out = false;
  useSSL_out = false;
  URL_out.clear ();
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out = TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
                               ACE_TEXT ("b:df:lrs:tu:vx:"),
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
      case 'b':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argument_parser.opt_arg ();
        converter >> bufferSize_out;
        break;
      }
      case 'd':
      {
        debugParser_out = true;
        break;
      }
      case 'f':
      {
        outputFileName_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
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
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argument_parser.opt_arg ();
        converter >> statisticReportingInterval_out;
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
        std::string URI_string;
        if (!HTTP_Tools::parseURL (URL_out,
                                   hostName_out,
                                   URI_string,
                                   useSSL_out))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), aborting\n"),
                      ACE_TEXT (URL_out.c_str ())));
          return false;
        } // end IF

        std::string hostname_string = hostName_out;
        ACE_INET_Addr peer_address;
        size_t position =
          hostName_out.find_last_of (':', std::string::npos);
        if (position == std::string::npos)
        {
          port_out = (useSSL_out ? HTTPS_DEFAULT_SERVER_PORT
                                 : HTTP_DEFAULT_SERVER_PORT);
          hostname_string += ':';
          std::ostringstream converter;
          converter << port_out;
          hostname_string += converter.str ();
        } // end IF
        else
        {
          std::istringstream converter (hostName_out.substr (position + 1,
                                                             std::string::npos));
          converter >> port_out;
        } // end ELSE
        result = peer_address.set (hostname_string.c_str (),
                                   AF_INET);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                      ACE_TEXT (hostname_string.c_str ())));
          return false;
        } // end IF

        // step2: validate address/verify host name exists
        //        --> resolve
        ACE_TCHAR buffer[HOST_NAME_MAX];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result = peer_address.get_host_name (buffer,
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
                      ACE_TEXT ("failed to Net_Common_Tools::getAddress(\"%s\"), aborting\n"),
                      ACE_TEXT (hostname.c_str ())));
          return false;
        } // end IF

        // step3: validate URI
        std::string regex_string =
            ACE_TEXT_ALWAYS_CHAR ("^(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
        std::regex regex (regex_string,
                          (std::regex_constants::ECMAScript |
                           std::regex_constants::icase));
        std::smatch match_results_3;
        if (!std::regex_match (URI_string,
                               match_results_3,
                               regex,
                               std::regex_constants::match_default))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid URI (was: \"%s\"), aborting\n"),
                      ACE_TEXT (URI_string.c_str ())));
          return false;
        } // end IF
//        ACE_ASSERT (match_results_3.ready () && !match_results_3.empty ());
        ACE_ASSERT (!match_results_3.empty ());

        if (!match_results_3[2].matched)
          URL_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_URL);
//        else if (!match_results_3[3].matched)
//          URL_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_SUFFIX);

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
        converter << argument_parser.opt_arg ();
        converter >> numberOfDispatchThreads_out;
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
do_initializeSignals (bool allowUserRuntimeConnect_in,
                      ACE_Sig_Set& signals_out,
                      ACE_Sig_Set& ignoredSignals_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_initializeSignals"));

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

#ifdef ENABLE_VALGRIND_SUPPORT
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif
#endif
}

void
do_work (unsigned int bufferSize_in,
         bool debugParser_in,
         const std::string& fileName_in,
         const std::string& hostName_in,
         unsigned short port_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         bool useSSL_in,
         const std::string& URL_in,
         unsigned int numberOfDispatchThreads_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_U_Protocol_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  // step0a: initialize configuration and stream
  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;
  struct Test_U_HTTPDecoder_Configuration configuration;
  configuration.dispatch = (useReactor_in ? COMMON_EVENT_DISPATCH_REACTOR
                                          : COMMON_EVENT_DISPATCH_PROACTOR);

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  Test_U_HTTPDecoder_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                           &heap_allocator,     // heap allocator handle
                                                           true);               // block ?

  Test_U_ConnectionManager_t* connection_manager_p =
    TEST_U_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);

  // *********************** parser configuration data *************************
  configuration.parserConfiguration.debugParser = debugParser_in;
  if (debugParser_in)
    configuration.parserConfiguration.debugScanner = true;
  // *********************** socket configuration data *************************
  Test_U_ConnectionConfiguration connection_configuration;
  int result =
    connection_configuration.address.set (port_in,
                                          hostName_in.c_str (),
                                          1,
                                          ACE_ADDRESS_FAMILY_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                ACE_TEXT (hostName_in.c_str ()),
                port_in));
    return;
  } // end IF
  connection_configuration.allocatorConfiguration = &allocator_configuration;
  connection_configuration.allocatorConfiguration->defaultBufferSize = bufferSize_in;
  connection_configuration.useLoopBackDevice =
    connection_configuration.address.is_loopback ();
//  connection_configuration.writeOnly = true;
  connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  connection_configuration.messageAllocator = &message_allocator;
  connection_configuration.initialize (configuration.streamConfiguration);

  configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                 &connection_configuration));
  Net_ConnectionConfigurationsIterator_t iterator =
    configuration.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration.connectionConfigurations.end ());

  // ********************** stream configuration data **************************
  // ********************** module configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
  struct Test_U_HTTPDecoder_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_U_HTTPDecoder_StreamConfiguration stream_configuration;
  //configuration.moduleHandlerConfiguration.allocatorConfiguration =
  //  &configuration.allocatorConfiguration;
  // *NOTE*: yyparse() does not currently return until the whole entity has been
  //         processed --> use (a) dedicated thread(s) so the event dispatch
  //         does not deadlock in single-threaded reactor/proactor scenarios
  modulehandler_configuration.concurrency =
    STREAM_HEADMODULECONCURRENCY_ACTIVE;
  modulehandler_configuration.configuration = &configuration;
  modulehandler_configuration.connectionConfigurations =
    &configuration.connectionConfigurations;
  modulehandler_configuration.connectionManager = connection_manager_p;
  modulehandler_configuration.dumpFileName =
    Common_Log_Tools::getLogDirectory (ACENetwork_PACKAGE_NAME);
  modulehandler_configuration.dumpFileName += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  modulehandler_configuration.dumpFileName += ACE_TEXT_ALWAYS_CHAR ("dump.txt");
  modulehandler_configuration.hostName = hostName_in;
  modulehandler_configuration.parserConfiguration =
    &configuration.parserConfiguration;
  modulehandler_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  modulehandler_configuration.streamConfiguration =
    &configuration.streamConfiguration;
  modulehandler_configuration.targetFileName = fileName_in;
  modulehandler_configuration.URL = URL_in;

  // ******************** (sub-)stream configuration data **********************
  //if (bufferSize_in)
  //  configuration.streamConfiguration.bufferSize = bufferSize_in;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.printFinalReport = true;
  configuration.streamConfiguration.initialize (module_configuration,
                                                modulehandler_configuration,
                                                stream_configuration);

  // step0b: initialize event dispatch
  if (useReactor_in)
    configuration.dispatchConfiguration.numberOfReactorThreads =
      numberOfDispatchThreads_in;
  else
    configuration.dispatchConfiguration.numberOfProactorThreads =
      numberOfDispatchThreads_in;
  if (!Common_Tools::initializeEventDispatch (configuration.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
      &configuration.dispatchConfiguration;

  // step0c: initialize connection manager
  struct Net_UserData user_data_s;
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (*dynamic_cast<Test_U_ConnectionConfiguration*> ((*iterator).second),
                             &user_data_s);

  // step0d: initialize regular (global) statistic reporting
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  ACE_thread_t thread_id = 0;
  timer_manager_p->start (thread_id);
  ACE_UNUSED_ARG (thread_id);
  Net_StreamStatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                                  connection_manager_p,
                                                  false);
  long timer_id = -1;
  if (statisticReportingInterval_in)
  {
    ACE_Time_Value interval (statisticReportingInterval_in, 0);
    timer_id =
      timer_manager_p->schedule_timer (&statistic_handler,         // event handler handle
                                       NULL,                       // asynchronous completion token
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (timer_id == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));

      // clean up
      timer_manager_p->stop ();

      return;
    } // end IF
  } // end IF

  // step0c: initialize signal handling
  configuration.signalHandlerConfiguration.dispatchState =
    &event_dispatch_state_s;
  //configuration.signalHandlerConfiguration.statisticReportingHandler =
  //  connection_manager_p;
  //configuration.signalHandlerConfiguration.statisticReportingTimerId = timer_id;
  if (!signalHandler_in.initialize (configuration.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));

    // clean up
    timer_manager_p->stop ();

    return;
  } // end IF
  if (!Common_Signal_Tools::initialize ((useReactor_in ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                       : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));

    // clean up
    timer_manager_p->stop ();

    return;
  } // end IF

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting

  // step1a: initialize worker(s)
  if (!Common_Tools::startEventDispatch (event_dispatch_state_s))
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
    timer_manager_p->stop ();

    return;
  } // end IF

  // step1: connect
  Test_U_IConnector_t* iconnector_p = NULL;
  Test_U_IConnection_t* connection_p = NULL;
  Test_U_IStreamConnection_t* istream_connection_p = NULL;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  ACE_Time_Value initialization_timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT_S,
                                         0);
  ACE_Time_Value deadline;
  enum Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
//  Test_U_MessageData* message_data_p = NULL;
  struct HTTP_Record* record_p = NULL;
  Test_U_MessageData_t* message_data_container_p = NULL;
  Test_U_Message* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  size_t pdu_size_i = 0;

  if (useReactor_in)
  {
#if defined (SSL_SUPPORT)
    if (useSSL_in)
      ACE_NEW_NORETURN (iconnector_p,
                        Test_U_SSLConnector_t (true));
    else
#endif // SSL_SUPPORT
      ACE_NEW_NORETURN (iconnector_p,
                        Test_U_TCPConnector_t (true));
  } // end IF
  else
  {
#if defined (SSL_SUPPORT)
    // *TODO*: add SSL support to the proactor framework
    ACE_ASSERT (port_in != HTTPS_DEFAULT_SERVER_PORT);
#endif // SSL_SUPPORT
    ACE_NEW_NORETURN (iconnector_p,
                      Test_U_TCPAsynchConnector_t (true));
  } // end ELSE
  if (!iconnector_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    goto clean_up;
  } // end IF
  if (!iconnector_p->initialize (*dynamic_cast<Test_U_ConnectionConfiguration*> ((*iterator).second)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector, returning\n")));
    goto clean_up;
  } // end IF
  handle =
    iconnector_p->connect (NET_SOCKET_CONFIGURATION_TCP_CAST ((*iterator).second)->address);
  if (handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s, returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_SOCKET_CONFIGURATION_TCP_CAST ((*iterator).second)->address).c_str ())));
    goto clean_up;
  } // end IF
  if (iconnector_p->useReactor ())
    connection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle));
#else
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (handle));
#endif
  else
  {
    // step1: wait for the connection to register with the manager
    // *TODO*: avoid tight loop here
    deadline = (COMMON_TIME_NOW +
                ACE_Time_Value (NET_CONNECTION_ASYNCH_DEFAULT_TIMEOUT_S,
                                0));
    //result = ACE_OS::sleep (timeout);
    //if (result == -1)
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
    //              &timeout));
    do
    {
      connection_p =
        connection_manager_p->get (NET_SOCKET_CONFIGURATION_TCP_CAST ((*iterator).second)->address);
      if (connection_p)
        break;
    } while (COMMON_TIME_NOW < deadline);
  } // end IF
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\", returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_SOCKET_CONFIGURATION_TCP_CAST ((*iterator).second)->address).c_str ())));
    goto clean_up;
  } // end IF
  // step1b: wait for the connection to finish initializing
  // *TODO*: avoid tight loop here
  deadline = COMMON_TIME_NOW + initialization_timeout;
  do
  {
    status = connection_p->status ();
    if (status == NET_CONNECTION_STATUS_OK)
      break;
  } while (COMMON_TIME_NOW < deadline);
  if (status != NET_CONNECTION_STATUS_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connection to %s (status was: %d), returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_SOCKET_CONFIGURATION_TCP_CAST ((*iterator).second)->address).c_str ()),
                status));

    // clean up
    //connection_p->close ();
    //connection_p->decrease ();

    //goto clean_up;
  } // end IF
  // step1c: wait for the connection stream to finish initializing
  istream_connection_p =
    dynamic_cast<Test_U_IStreamConnection_t*> (connection_p);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Test_U_IStreamConnection_t>(0x%@), returning\n"),
                connection_p));

    // clean up
    connection_p->close ();
    connection_p->decrease (); connection_p = NULL;

    goto clean_up;
  } // end IF
  //istream_connection_p->wait (STREAM_STATE_RUNNING,
  //                            NULL); // <-- block
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connected to %s\n"),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_SOCKET_CONFIGURATION_TCP_CAST ((*iterator).second)->address).c_str ())));
#endif // _DEBUG

  // step2: send HTTP request
//  ACE_NEW_NORETURN (message_data_p,
//                    Test_U_MessageData ());
//  if (!message_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory, returning\n")));

//    // clean up
//    connection_p->close ();
//    connection_p->decrease ();

//    goto clean_up;
//  } // end IF
//  ACE_NEW_NORETURN (message_data_p->HTTPRecord,
  ACE_NEW_NORETURN (record_p,
                    struct HTTP_Record ());
//  if (!message_data_p->HTTPRecord)
  if (!record_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));

    // clean up
    connection_p->close ();
    connection_p->decrease (); connection_p = NULL;
//    delete message_data_p;
    delete record_p;

    goto clean_up;
  } // end IF
  record_p->method = HTTP_Codes::HTTP_METHOD_GET;
  record_p->URI = URL_in;
  record_p->version = HTTP_Codes::HTTP_VERSION_1_1;
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
  ACE_NEW_NORETURN (message_data_container_p,
//                    Test_U_MessageData_t (message_data_p,
                    Test_U_MessageData_t (record_p,
                                          true));
  if (!message_data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));

    // clean up
    connection_p->close ();
    connection_p->decrease (); connection_p = NULL;
//    delete message_data_p;
    delete record_p;

    goto clean_up;
  } // end IF

  ACE_ASSERT ((*iterator).second->allocatorConfiguration);
  pdu_size_i =
    (*iterator).second->allocatorConfiguration->defaultBufferSize;// +
//    (*iterator).second->allocatorConfiguration->paddingBytes;

allocate:
  message_p =
    static_cast<Test_U_Message*> (message_allocator.malloc (pdu_size_i));
  // keep retrying ?
  if (!message_p && !message_allocator.block ())
    goto allocate;
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));

    // clean up
    connection_p->close ();
    connection_p->decrease (); connection_p = NULL;
    message_data_container_p->decrease (); message_data_container_p = NULL;

    goto clean_up;
  } // end IF
  // *IMPORTANT NOTE*: fire-and-forget API (message_data_container_p)
  message_p->initialize (message_data_container_p,
                         message_p->sessionId (),
                         NULL);
  // *IMPORTANT NOTE*: fire-and-forget API (message_p)
  message_block_p = message_p;
  istream_connection_p->send (message_block_p);

  connection_manager_p->wait ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

clean_up:
  timer_manager_p->stop ();
  connection_manager_p->stop (true,  // wait ?
                              true,  // high priority ?
                              true); // locked access ?
  Common_Tools::finalizeEventDispatch (event_dispatch_state_s.proactorGroupId,
                                       event_dispatch_state_s.reactorGroupId,
                                       true);

  delete iconnector_p;

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

//  result = file_writer.close (ACE_Module_Base::M_DELETE_NONE);
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
//                file_writer.name ()));
}

void
do_printVersion (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_printVersion"));

  std::ostringstream converter;

  // compiler version string...
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
#ifdef HAVE_CONFIG_H
            << ACE_TEXT (ACENetwork_PACKAGE_NAME)
            << ACE_TEXT (": ")
            << ACE_TEXT (ACENetwork_PACKAGE_VERSION)
            << std::endl
#endif
            ;

  converter.str ("");
  // ACE version string...
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

  // step0: initialize
  // *PORTABILITY*: on Windows, initialize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::init ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif
  Common_Tools::initialize ();

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("protocol");
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  unsigned int buffer_size = TEST_U_DEFAULT_BUFFER_SIZE;
  bool debug_parser = COMMON_PARSER_DEFAULT_YACC_TRACE;
  std::string output_file = ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_OUTPUT_FILE);
  std::string host_name;
  bool log_to_file = false;
  unsigned short port = HTTP_DEFAULT_SERVER_PORT;
  bool use_reactor =
            (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  bool trace_information = false;
  bool use_SSL = false;
  std::string URL;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
                            debug_parser,
                            output_file,
                            host_name,
                            log_to_file,
                            port,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            use_SSL,
                            URL,
                            print_version_and_exit,
                            number_of_dispatch_threads))
  {
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_U_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  if (host_name.empty ()                                                    ||
      //(use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool) ||
      URL.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  if (number_of_dispatch_threads == 0)
    number_of_dispatch_threads = 1;

  // step1d: initialize logging and/or tracing
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                        ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]),           // program name
                                            log_file_name,                        // log file name
                                            false,                                // log to syslog ?
                                            false,                                // trace messages ?
                                            trace_information,                    // debug messages ?
                                            NULL))                                // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (true, // allow SIGUSR1/SIGBREAK
                        signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  sigset_t previous_signal_mask;
  result = ACE_OS::sigemptyset (&previous_signal_mask);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sigemptyset(): \"%m\", aborting\n")));

    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           use_reactor,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  ACE_SYNCH_RECURSIVE_MUTEX signal_lock;
  Test_U_Protocol_SignalHandler signal_handler ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                             : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                                &signal_lock);

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_SUCCESS;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false, // file descriptors
                                        true,  // stack traces
                                        true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (buffer_size,
           debug_parser,
           output_file,
           host_name,
           port,
           use_reactor,
           statistic_reporting_interval,
           use_SSL,
           URL,
           number_of_dispatch_threads,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
  timer.stop ();

  // debug info
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  // stop profile timer...
  process_profile.stop ();

  // only process profile left to do...
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  if (process_profile.elapsed_time (elapsed_time) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);
  std::string user_time_string;
  std::string system_time_string;
  user_time_string = Common_Timer_Tools::periodToString (user_time);
  system_time_string = Common_Timer_Tools::periodToString (system_time);

  // debug info
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
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
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ())));
#endif

  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalizeLogging ();

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

  return EXIT_SUCCESS;
} // end main
