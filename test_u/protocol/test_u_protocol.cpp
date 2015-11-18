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
#endif
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#ifdef LIBACESTREAM_ENABLE_VALGRIND_SUPPORT
#include "valgrind/valgrind.h"
#endif

#include "common.h"
#include "common_file_tools.h"
#include "common_logger.h"
#include "common_tools.h"

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#ifdef HAVE_CONFIG_H
#include "libACEStream_config.h"
#endif

#include "stream_module_filewriter.h"

#include "net_common_tools.h"

#include "test_i_common.h"
#include "test_i_defines.h"

#include "test_i_module_databasewriter.h"

#include "test_i_http_get_common.h"
#include "test_i_http_get_signalhandler.h"
#include "test_i_http_get_stream.h"

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b [VALUE]  : buffer size (byte(s)) [")
            << TEST_I_DEFAULT_BUFFER_SIZE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::string database_options_file = path;
#if defined (DEBUG_DEBUGGER)
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file += ACE_TEXT_ALWAYS_CHAR ("http_get");
#endif
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_OPTIONS_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [PATH]   : database options file [")
            << database_options_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [STRING] : write to database [")
            << ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e [STRING] : write to database table [")
            << ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_TABLE)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [STRING] : (output) file name [")
            << output_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o          : use thread pool [")
            << NET_EVENT_USE_THREAD_POOL
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : port number [")
            << TEST_I_DEFAULT_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << NET_EVENT_USE_REACTOR
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING
            << ACE_TEXT_ALWAYS_CHAR ("] {0: off}")
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
            << TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-z [VALUE]  : loop [")
            << 0
            << ACE_TEXT_ALWAYS_CHAR ("] {-1: forever}")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& bufferSize_out,
                     std::string& dataBaseOptionsFileName_out,
                     std::string& outputDataBase_out,
                     std::string& outputDataBaseTable_out,
                     std::string& outputFileName_out,
                     std::string& hostName_out,
                     bool& logToFile_out,
                     bool& useThreadPool_out,
                     unsigned short& port_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     std::string& URI_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out,
                     size_t& loop_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  bufferSize_out = TEST_I_DEFAULT_BUFFER_SIZE;
  dataBaseOptionsFileName_out = path;
#if defined (DEBUG_DEBUGGER)
  dataBaseOptionsFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  dataBaseOptionsFileName_out += ACE_TEXT_ALWAYS_CHAR ("http_get");
#endif
  dataBaseOptionsFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  dataBaseOptionsFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  dataBaseOptionsFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  dataBaseOptionsFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_OPTIONS_FILE);
  outputDataBase_out = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB);
  outputDataBaseTable_out =
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_TABLE);
  outputFileName_out = path;
  outputFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  outputFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  hostName_out.clear ();
  logToFile_out = false;
  useThreadPool_out = NET_EVENT_USE_THREAD_POOL;
  port_out = TEST_I_DEFAULT_PORT;
  useReactor_out = NET_EVENT_USE_REACTOR;
  statisticReportingInterval_out = STREAM_DEFAULT_STATISTIC_REPORTING;
  traceInformation_out = false;
  URI_out.clear ();
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;
  loop_out = 0;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("b:c:d:e:f:lors:tu:vx:z:"),
                              1,                         // skip command name
                              1,                         // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS, // ordering
                              0);                        // for now, don't use long options

  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'b':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> bufferSize_out;
        break;
      }
      case 'c':
      {
        dataBaseOptionsFileName_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'd':
      {
        outputDataBase_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'e':
      {
        outputDataBaseTable_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'f':
      {
        outputFileName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'o':
      {
        useThreadPool_out = true;
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
        converter << argumentParser.opt_arg ();
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
        // step1: parse URL
        URI_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        std::string regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^(?:http://)?([[:alnum:]-.]+)(?:\\:([[:digit:]]{1,5}))?(.+)?$");
        std::regex regex (regex_string);
        std::smatch match_results;
        if (!std::regex_match (URI_out,
                               match_results,
                               regex,
                               std::regex_constants::match_default))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid URL string (was: \"%s\"), aborting\n"),
                      ACE_TEXT (URI_out.c_str ())));
          return false;
        } // end IF
        ACE_ASSERT (match_results.ready () && !match_results.empty ());

        ACE_ASSERT (match_results[1].matched);
        hostName_out = match_results[1];
        if (match_results[2].matched)
        {
          converter.clear ();
          converter.str (ACE_TEXT_ALWAYS_CHAR (""));
          converter << match_results[2].str ();
          converter >> port_out;
        } // end IF
        ACE_ASSERT (match_results[3].matched);
//        URI_out = match_results[3];

        // step2: validate address/verify host name exists
        //        --> resolve
        std::string dotted_decimal_string;
        // *TODO*: support IPv6 as well
        regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]{1,3}\\.){4}$");
        regex = regex_string;
        std::smatch match_results_2;
        if (std::regex_match (hostName_out,
                              match_results_2,
                              regex,
                              std::regex_constants::match_default))
        {
          ACE_ASSERT (match_results_2.ready ()  &&
                      !match_results_2.empty () &&
                      match_results_2[1].matched);
          dotted_decimal_string = hostName_out;
        } // end IF
        if (!Net_Common_Tools::getAddress (hostName_out,
                                           dotted_decimal_string))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_Common_Tools::getAddress(), aborting\n")));
          return false;
        } // end IF

        // step3: validate URI
//        regex_string =
//            ACE_TEXT_ALWAYS_CHAR ("^(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
        regex_string =
            ACE_TEXT_ALWAYS_CHAR ("^(?:http://)?((.+\\.)+([^\\/]+))(\\/.+(?=\\/))*\\/(.+?)(\\.(html|htm))?$");
        regex.assign (regex_string,
                      (std::regex_constants::ECMAScript |
                       std::regex_constants::icase));
        std::smatch match_results_3;
        if (!std::regex_match (URI_out,
                               match_results_3,
                               regex,
                               std::regex_constants::match_default))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid URI (was: \"%s\"), aborting\n"),
                      ACE_TEXT (URI_out.c_str ())));
          return false;
        } // end IF
        ACE_ASSERT (match_results_3.ready () && !match_results_3.empty ());

        if (!match_results_3[2].matched)
          URI_out += ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_NET_SOURCE_HTTP_GET_DEFAULT_URL);
        else if (!match_results_3[3].matched)
          URI_out += ACE_TEXT_ALWAYS_CHAR (HTML_DEFAULT_SUFFIX);

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
        converter << argumentParser.opt_arg ();
        converter >> numberOfDispatchThreads_out;
        break;
      }
      case 'z':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> loop_out;
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

  return true;
}

void
do_initializeSignals (bool allowUserRuntimeConnect_in,
                      ACE_Sig_Set& signals_out,
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
         const std::string& dataBaseOptionsFileName_in,
         const std::string& dataBase_in,
         const std::string& dataBaseTable_in,
         const std::string& fileName_in,
         const std::string& hostName_in,
         bool useThreadPool_in,
         unsigned short port_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         const std::string& URL_in,
         unsigned int numberOfDispatchThreads_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Stream_Source_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  // step0a: initialize configuration and stream
  Test_I_Configuration configuration;
  Test_I_StreamBase_t* stream_p = NULL;
  if (useReactor_in)
    ACE_NEW_NORETURN (stream_p,
                      Test_I_HTTPGet_Stream_t ());
  else
    ACE_NEW_NORETURN (stream_p,
                      Test_I_HTTPGet_AsynchStream_t ());
  if (!stream_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    return;
  } // end IF
  configuration.userData.configuration = &configuration;
  configuration.userData.streamConfiguration =
      &configuration.streamConfiguration;
  configuration.useReactor = useReactor_in;

  Stream_Module_t* module_p = NULL;
  Test_I_Stream_Module_DataBaseWriter_Module database_writer (ACE_TEXT_ALWAYS_CHAR ("DataBaseWriter"),
                                                              NULL,
                                                              true);
  Test_I_Stream_Module_FileWriter_Module file_writer (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
                                                      NULL,
                                                      true);
  module_p = &file_writer;
  if (!dataBase_in.empty ())
    module_p = &database_writer;
  Test_I_IModuleHandler_t* module_handler_p =
    dynamic_cast<Test_I_IModuleHandler_t*> (module_p->writer ());
  if (!module_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Test_I_IModuleHandler_t>(0x%@) failed, returning\n"),
                module_p->name (),
                module_p->writer ()));

    // clean up
    delete stream_p;

    return;
  } // end IF

  Stream_AllocatorHeap heap_allocator;
  Test_I_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?

  Test_I_Stream_InetConnectionManager_t* connection_manager_p =
    TEST_I_STREAM_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);

  // *********************** socket configuration data ************************
  int result =
    configuration.socketConfiguration.peerAddress.set (port_in,
                                                       hostName_in.c_str (),
                                                       1,
                                                       ACE_ADDRESS_FAMILY_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                ACE_TEXT (hostName_in.c_str ()),
                port_in));

    // clean up
    delete stream_p;

    return;
  } // end IF
  configuration.socketConfiguration.useLoopBackDevice =
    configuration.socketConfiguration.peerAddress.is_loopback ();
  configuration.socketConfiguration.writeOnly = true;
  // ******************** socket handler configuration data *******************
  configuration.socketHandlerConfiguration.messageAllocator =
    &message_allocator;
  configuration.socketHandlerConfiguration.PDUSize = bufferSize_in;
  configuration.socketHandlerConfiguration.socketConfiguration =
    &configuration.socketConfiguration;
  configuration.socketHandlerConfiguration.statisticReportingInterval =
    statisticReportingInterval_in;
  configuration.socketHandlerConfiguration.userData =
    &configuration.userData;

  // ********************** stream configuration data **************************
  // ********************** module configuration data **************************
  configuration.moduleConfiguration.streamConfiguration =
    &configuration.streamConfiguration;

  configuration.moduleHandlerConfiguration.streamConfiguration =
    &configuration.streamConfiguration;

  configuration.moduleHandlerConfiguration.configuration = &configuration;
  configuration.moduleHandlerConfiguration.connectionManager =
    connection_manager_p;
  configuration.moduleHandlerConfiguration.dataBaseOptionsFileName =
    dataBaseOptionsFileName_in;
  configuration.moduleHandlerConfiguration.dataBaseTable = dataBaseTable_in;
  configuration.moduleHandlerConfiguration.targetFileName = fileName_in;
  configuration.moduleHandlerConfiguration.hostName = hostName_in;
  configuration.moduleHandlerConfiguration.loginOptions.database = dataBase_in;
  //configuration.moduleHandlerConfiguration.loginOptions.password =;
  //configuration.moduleHandlerConfiguration.loginOptions.user = ;
  configuration.moduleHandlerConfiguration.URL = URL_in;
  configuration.moduleHandlerConfiguration.socketConfiguration =
    &configuration.socketConfiguration;
  configuration.moduleHandlerConfiguration.socketHandlerConfiguration =
    &configuration.socketHandlerConfiguration;
  configuration.moduleHandlerConfiguration.stream = stream_p;
  // ******************** (sub-)stream configuration data *********************
  if (bufferSize_in)
    configuration.streamConfiguration.bufferSize = bufferSize_in;
  configuration.streamConfiguration.messageAllocator = &message_allocator;
  configuration.streamConfiguration.module = module_p;
  configuration.streamConfiguration.moduleConfiguration =
    &configuration.moduleConfiguration;
  configuration.streamConfiguration.moduleHandlerConfiguration =
    &configuration.moduleHandlerConfiguration;
  configuration.streamConfiguration.printFinalReport = true;
  configuration.streamConfiguration.statisticReportingInterval =
    statisticReportingInterval_in;

  module_handler_p->initialize (configuration.moduleHandlerConfiguration);

  // step0b: initialize event dispatch
  if (!Common_Tools::initializeEventDispatch (useReactor_in,
                                              useThreadPool_in,
                                              numberOfDispatchThreads_in,
                                              configuration.streamConfiguration.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));

    // clean up
    delete stream_p;

    return;
  } // end IF

  // step0c: initialize connection manager
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  connection_manager_p->set (configuration,
                             &configuration.userData);

  // step0d: initialize regular (global) statistic reporting
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start ();
  Stream_StatisticHandler_Reactor_t statistics_handler (ACTION_REPORT,
                                                        connection_manager_p,
                                                        false);
  //Stream_StatisticHandler_Proactor_t statistics_handler_proactor (ACTION_REPORT,
  //                                                                connection_manager_p,
  //                                                                false);
  long timer_id = -1;
  if (statisticReportingInterval_in)
  {
    ACE_Event_Handler* handler_p = &statistics_handler;
    ACE_Time_Value interval (statisticReportingInterval_in, 0);
    timer_id =
      timer_manager_p->schedule_timer (handler_p,                  // event handler
                                       NULL,                       // ACT
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (timer_id == -1)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));

      // clean up
      timer_manager_p->stop ();
      delete stream_p;

      return;
    } // end IF
  } // end IF

  // step0c: initialize signal handling
  //configuration.signalHandlerConfiguration.statisticReportingHandler =
  //  connection_manager_p;
  //configuration.signalHandlerConfiguration.statisticReportingTimerID = timer_id;
  signalHandler_in.initialize (configuration.signalHandlerConfiguration);
  if (!Common_Tools::initializeSignals (signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeSignals(), aborting\n")));

    // clean up
    timer_manager_p->stop ();
    delete stream_p;

    return;
  } // end IF

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting

  // step1a: initialize worker(s)
  int group_id = -1;
  if (!Common_Tools::startEventDispatch (&useReactor_in,
                                         numberOfDispatchThreads_in,
                                         group_id))
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
    delete stream_p;

    return;
  } // end IF

  unsigned int counter = 0;
loop:
  if (!stream_p->initialize (configuration.streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize stream, aborting\n")));

    // clean up
    Common_Tools::finalizeEventDispatch (useReactor_in,
                                         !useReactor_in,
                                         group_id);
    timer_manager_p->stop ();
    delete stream_p;

    return;
  } // end IF

  // *NOTE*: this call blocks until the file has been sent (or an error
  //         occurs)
  stream_p->start ();
  //    if (!stream_p->isRunning ())
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to start stream, aborting\n")));

  //      // clean up
  //      //timer_manager_p->stop ();

  //      return;
  //    } // end IF
  stream_p->waitForCompletion ();
  if (false)
  {
    ++counter;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("iteration #%u complete...\n"),
                counter));
    if ((static_cast<int> (false) == -1) ||
        (counter != false))
      goto loop;
  } // end IF

  // clean up
  connection_manager_p->stop ();
  Common_Tools::finalizeEventDispatch (useReactor_in,
                                       !useReactor_in,
                                       group_id);
  connection_manager_p->wait ();

  // step3: clean up
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
  delete stream_p;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

void
do_printVersion (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printVersion"));

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
            << ACE_TEXT (LIBACESTREAM_PACKAGE)
            << ACE_TEXT (": ")
            << ACE_TEXT (LIBACESTREAM_PACKAGE_VERSION)
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
  STREAM_TRACE (ACE_TEXT ("::main"));

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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  unsigned int buffer_size = TEST_I_DEFAULT_BUFFER_SIZE;
  std::string database_options_file = configuration_path;
#if defined (DEBUG_DEBUGGER)
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file += ACE_TEXT_ALWAYS_CHAR ("http_get");
#endif
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  database_options_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  database_options_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_OPTIONS_FILE);
  std::string output_database = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB);
  std::string output_database_table =
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_DB_TABLE);
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  std::string output_file = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::string host_name;
  bool log_to_file = false;
  bool use_thread_pool = NET_EVENT_USE_THREAD_POOL;
  unsigned short port = TEST_I_DEFAULT_PORT;
  bool use_reactor = NET_EVENT_USE_REACTOR;
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING;
  bool trace_information = false;
  std::string URL;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;
  size_t loop = 0;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            buffer_size,
                            database_options_file,
                            output_database,
                            output_database_table,
                            output_file,
                            host_name,
                            log_to_file,
                            use_thread_pool,
                            port,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            URL,
                            print_version_and_exit,
                            number_of_dispatch_threads,
                            loop))
  {
    // make 'em learn...
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
  if ((!database_options_file.empty () &&
       !Common_File_Tools::isReadable (database_options_file))              ||
      host_name.empty ()                                                    ||
      (use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool) ||
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
  if (number_of_dispatch_threads == 0) number_of_dispatch_threads = 1;

  // step1d: initialize logging and/or tracing
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_File_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (LIBACESTREAM_PACKAGE_NAME),
                                         ACE::basename (argv_in[0]));
  if (!Common_Tools::initializeLogging (ACE::basename (argv_in[0]),           // program name
                                        log_file_name,                        // log file name
                                        false,                                // log to syslog ?
                                        false,                                // trace messages ?
                                        trace_information,                    // debug messages ?
                                        NULL))                                // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeLogging(), aborting\n")));

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

    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  if (!Common_Tools::preInitializeSignals (signal_set,
                                           use_reactor,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::preInitializeSignals(), aborting\n")));

    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  Stream_Source_SignalHandler signal_handler (use_reactor);

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
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

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
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
           database_options_file,
           output_database,
           output_database_table,
           output_file,
           host_name,
           use_thread_pool,
           port,
           use_reactor,
           statistic_reporting_interval,
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
  Common_Tools::period2String (working_time,
                               working_time_string);

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

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
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
  Common_Tools::period2String (user_time,
                               user_time_string);
  Common_Tools::period2String (system_time,
                               system_time_string);

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

  Common_Tools::finalizeSignals (signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Tools::finalizeLogging ();

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

  return EXIT_SUCCESS;
} // end main
