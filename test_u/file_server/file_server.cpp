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

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

//#include "ace/streams.h"
#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN32
#include "ace/Log_Msg.h"
//#include "ace/Synch.h"
#include "ace/Proactor.h"
#include "ace/Profile_Timer.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/POSIX_Proactor.h"
#endif // ACE_WIN32 || ACE_WIN32
#include "ace/Reactor.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_file_tools.h"
#include "common_tools.h"

#include "common_log_tools.h"
#include "common_logger.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"

#include "stream_misc_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"
#include "net_macros.h"

#include "net_client_defines.h"

#include "net_server_common_tools.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#include "test_u_common.h"
#include "test_u_defines.h"
#include "test_u_connection_manager_common.h"
#include "test_u_stream.h"
#include "test_u_eventhandler.h"
#include "test_u_module_eventhandler.h"

#include "file_server_common.h"
#include "file_server_defines.h"
#include "file_server_listener_common.h"
#include "file_server_signalhandler.h"

// globals
unsigned int random_seed;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
struct random_data random_data;
char random_state_buffer[BUFSIZ];
#endif

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("FileServerStream");

void
do_printUsage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:") << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c           : console mode [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif
  std::string path = configuration_path;
  std::string source_file = Common_File_Tools::getTempDirectory ();
  source_file += ACE_DIRECTORY_SEPARATOR_STR;
  source_file += ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_DEFAULT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [FILE]    : source file [")
            << source_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string UI_file_path = path;
  UI_file_path += ACE_DIRECTORY_SEPARATOR_STR;
  UI_file_path += ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_UI_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]] : UI definition file [\"")
            << UI_file_path
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i [STRING]  : network interface [\"")
            << ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
//  std::cout << ACE_TEXT_ALWAYS_CHAR("-k [VALUE]  : client keep-alive timeout ([")
//            << NET_SERVER_DEF_CLIENT_KEEPALIVE
//            << ACE_TEXT_ALWAYS_CHAR("] second(s) {0 --> no timeout})")
//            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l           : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n [VALUE]   : maximum number of (concurrent) connections [")
            << NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  // *TODO*: this doesn't really make sense (see '-i' option)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o           : use loopback [")
            << NET_INTERFACE_DEFAULT_USE_LOOPBACK
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]   : listening port [")
            << NET_SERVER_DEFAULT_LISTENING_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r           : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]   : statistic reporting interval (second(s)) [")
            << NET_SERVER_DEFAULT_STATISTIC_REPORTING_INTERVAL
            << ACE_TEXT_ALWAYS_CHAR ("] {0: off})")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t           : trace information")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u           : use UDP [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v           : print version information and exit")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]   : #dispatch threads [")
            << NET_SERVER_DEFAULT_NUMBER_OF_DISPATCH_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (const int& argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#endif
                     std::string& fileName_out,
                     std::string& UIFile_out,
                     std::string& networkInterface_out,
                     //unsigned int& keepAliveTimeout_out,
                     bool& logToFile_out,
                     unsigned int& maximumNumberOfConnections_out,
                     bool& useLoopBack_out,
                     unsigned short& listeningPortNumber_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& useUDP_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#endif
  std::string path = configuration_path;
  fileName_out = Common_File_Tools::getTempDirectory ();
  fileName_out += ACE_DIRECTORY_SEPARATOR_STR;
  fileName_out += ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_DEFAULT_FILE);
  UIFile_out = path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_STR;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_UI_FILE);
  networkInterface_out =
    ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET);
//  keepAliveTimeout_out = NET_SERVER_DEF_CLIENT_KEEPALIVE;
  logToFile_out = false;
  maximumNumberOfConnections_out =
    NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
  useLoopBack_out = NET_INTERFACE_DEFAULT_USE_LOOPBACK;
  listeningPortNumber_out = NET_SERVER_DEFAULT_LISTENING_PORT;
  useReactor_out =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
      NET_SERVER_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out = false;
  useUDP_out = false;
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    NET_SERVER_DEFAULT_NUMBER_OF_DISPATCH_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("cf:g::i:k:ln:op:rs:tuvx:"));
#else
                              ACE_TEXT ("f:g::i:k:ln:op:rs:tuvx:"));
#endif
  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
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
      case 'f':
      {
        fileName_out =
          Common_File_Tools::realPath (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
        break;
      }
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
      case 'i':
      {
        networkInterface_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
//      case 'k':
//      {
//        converter.clear();
//        converter.str(ACE_TEXT_ALWAYS_CHAR(""));
//        converter << argumentParser.opt_arg();
//        converter >> keepAliveTimeout_out;
//        break;
//      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'n':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> maximumNumberOfConnections_out;
        break;
      }
      case 'o':
      {
        useLoopBack_out = true;
        break;
      }
      case 'p':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> listeningPortNumber_out;
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
        useUDP_out = true;
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
      // error handling
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    ACE_TEXT (argumentParser.last_option ())));
        return false;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%c\", continuing\n"),
                    option));
        break;
      }
    } // end SWITCH
  } // end WHILE

  return true;
}

void
do_initializeSignals (bool useReactor_in,
                      bool allowUserRuntimeStats_in,
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

  // *PORTABILITY*: on Windows most signals are not defined,
  // and ACE_Sig_Set::fill_set() doesn't really work as specified
  // --> add valid signals (see <signal.h>)...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signals_out.sig_add (SIGINT);            // 2       /* interrupt */
  signals_out.sig_add (SIGILL);            // 4       /* illegal instruction - invalid function image */
  signals_out.sig_add (SIGFPE);            // 8       /* floating point exception */
  //  signals_out.sig_add(SIGSEGV);          // 11      /* segment violation */
  signals_out.sig_add (SIGTERM);           // 15      /* Software termination signal from kill */
  if (allowUserRuntimeStats_in)
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
  // ---------------------------------------------------------------------------
  if (!allowUserRuntimeStats_in)
  {
    signals_out.sig_del (SIGUSR1);         // 10      /* User-defined signal 1 */
    ignoredSignals_out.sig_add (SIGUSR1);  // 10      /* User-defined signal 1 */
  } // end IF
  // *NOTE* core dump on SIGSEGV
  signals_out.sig_del (SIGSEGV);           // 11      /* Segmentation fault: Invalid memory reference */
  // *NOTE* don't care about SIGPIPE
  signals_out.sig_del (SIGPIPE);           // 12      /* Broken pipe: write to pipe with no readers */
  signals_out.sig_del (SIGSTOP);           // 19      /* Stop process */

  // *IMPORTANT NOTE*: "...NPTL makes internal use of the first two real-time
  //                   signals (see also signal(7)); these signals cannot be
  //                   used in applications. ..." (see 'man 7 pthreads')
  // --> on POSIX platforms, make sure that ACE_SIGRTMIN == 34
//  for (int i = ACE_SIGRTMIN;
//       i <= ACE_SIGRTMAX;
//       i++)
//    signals_out.sig_del (i);

  if (!useReactor_in)
  {
    ACE_Proactor* proactor_p = ACE_Proactor::instance ();
    ACE_ASSERT (proactor_p);
    ACE_POSIX_Proactor* proactor_impl_p =
        dynamic_cast<ACE_POSIX_Proactor*> (proactor_p->implementation ());
    ACE_ASSERT (proactor_impl_p);
    if (proactor_impl_p->get_impl_type () == ACE_POSIX_Proactor::PROACTOR_SIG)
      signals_out.sig_del (COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL);
  } // end IF
#endif
}

void
do_work (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#endif // ACE_WIN32 || ACE_WIN64
         const std::string& fileName_in,
         const std::string& UIDefinitionFile_in,
         const std::string& networkInterface_in,
         //unsigned int keepAliveTimeout_in,
         unsigned int maximumNumberOfConnections_in,
         bool useLoopBack_in,
         unsigned short listeningPortNumber_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         bool useUDP_in,
         unsigned int numberOfDispatchThreads_in,
#if defined (GUI_SUPPORT)
         struct FileServer_UI_CBData& CBData_in,
#endif // GUI_SUPPORT
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         FileServer_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;
  ACE_thread_t thread_id = 0;
  struct Common_EventDispatchConfiguration event_dispatch_configuration_s;
  struct Common_EventDispatchState event_dispatch_state_s;
  struct Common_TimerConfiguration timer_configuration;
  struct FileServer_SignalHandlerConfiguration signal_handler_configuration;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND window_p = NULL;
  BOOL was_visible_b = true;
#endif
  long timer_id = -1;
  int group_id = -1;
  bool stop_event_dispatch = false;

  // step0a: initialize configuration
  struct FileServer_Configuration configuration;
#if defined (GUI_SUPPORT)
  CBData_in.configuration = &configuration;
#endif // GUI_SUPPORT

  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);

  FileServer_TCPConnectionManager_t* connection_manager_p =
    FILESERVER_TCPCONNECTIONMANAGER_SINGLETON::instance ();
  FileServer_TCPIConnectionManager_t* iconnection_manager_p =
    connection_manager_p;
  ACE_ASSERT (iconnection_manager_p);

  Net_StreamStatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                                  connection_manager_p,
                                                  false);
  Test_U_EventHandler ui_event_handler (
#if defined (GUI_SUPPORT)
                                        &CBData_in
#endif // GUI_SUPPORT
                                       );
  Test_U_Module_EventHandler_Module event_handler (NULL,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_U_Module_EventHandler* event_handler_p =
    dynamic_cast<Test_U_Module_EventHandler*> (event_handler.writer ());
  if (!event_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Module_EventHandler> failed, returning\n")));
    return;
  } // end IF

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_Parser_FlexAllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (configuration.allocatorConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize message allocator, returning\n")));
    return;
  } // end IF
  Test_U_MessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,         // heap allocator handle
                                               true);                   // block ?

  // ******************** protocol configuration data **************************
  // ********************** stream configuration data **************************
//  configuration.allocatorConfiguration.defaultBufferSize =
//    bufferSize_in;
  struct Stream_ModuleConfiguration module_configuration;
  struct Test_U_ModuleHandlerConfiguration modulehandler_configuration;
  modulehandler_configuration.allocatorConfiguration =
    &configuration.allocatorConfiguration;
  modulehandler_configuration.connectionConfigurations =
    &configuration.connectionConfigurations;
  modulehandler_configuration.fileIdentifier.identifier = fileName_in;
  if (useUDP_in)
    modulehandler_configuration.inbound = false;
  modulehandler_configuration.printFinalReport = true;
  //modulehandler_configuration.program =
  //  FILE_SERVER_DEFAULT_MPEG_TS_PROGRAM_NUMBER;
  modulehandler_configuration.statisticReportingInterval =
    ACE_Time_Value (statisticReportingInterval_in, 0);
  modulehandler_configuration.streamConfiguration =
    &configuration.streamConfiguration;
  //modulehandler_configuration.streamType =
  //  FILE_SERVER_DEFAULT_MPEG_TS_STREAM_TYPE;
  if (!UIDefinitionFile_in.empty ())
  {
    modulehandler_configuration.subscriber = &ui_event_handler;
#if defined (GUI_SUPPORT)
    modulehandler_configuration.subscribers = &CBData_in.subscribers;
    modulehandler_configuration.lock =
      &CBData_in.UIState->subscribersLock;
#endif // GUI_SUPPORT
  } // end IF

  configuration.streamConfiguration.configuration_.cloneModule =
    !(UIDefinitionFile_in.empty ());
  configuration.streamConfiguration.configuration_.messageAllocator =
    &message_allocator;
  configuration.streamConfiguration.configuration_.module = &event_handler;
  configuration.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                            std::make_pair (module_configuration,
                                                                            modulehandler_configuration)));

  //configuration.streamConfiguration.protocolConfiguration =
  //  &configuration.protocolConfiguration;
  // *TODO*: is this correct ?
  configuration.streamConfiguration.configuration_.dispatch =
    (useReactor_in ? COMMON_EVENT_DISPATCH_REACTOR
                   : COMMON_EVENT_DISPATCH_PROACTOR);
  configuration.streamConfiguration.configuration_.serializeOutput =
    (useReactor_in && (numberOfDispatchThreads_in > 1));

  // ********************** socket configuration data **************************
  FileServer_TCPConnectionConfiguration tcp_connection_configuration;
  FileServer_UDPConnectionConfiguration udp_connection_configuration;
  Net_ConnectionConfigurationsIterator_t iterator;
  Net_ConnectionConfigurationsIterator_t iterator_2;
  if (useUDP_in)
  {
    result =
      udp_connection_configuration.listenAddress.set (listeningPortNumber_in,
                                                      static_cast<ACE_UINT32> (INADDR_ANY),
                                                      1,
                                                      0);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    udp_connection_configuration.writeOnly = true;
  } // end IF
  // ****************** connection configuration data **************************
  //configuration.userData.connectionConfiguration =
  //    &configuration.connectionConfiguration;

  if (useUDP_in)
  {
    udp_connection_configuration.messageAllocator = &message_allocator;
    udp_connection_configuration.initialize (configuration.allocatorConfiguration,
                                             configuration.streamConfiguration);
    configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                   &udp_connection_configuration));
  } // end IF
  else
  {
    //configuration.listenerConfiguration.messageAllocator =
    //    &message_allocator;
    //configuration.listenerConfiguration.connectionConfiguration =
    //    &tcp_connection_configuration;

    tcp_connection_configuration.messageAllocator = &message_allocator;
    tcp_connection_configuration.initialize (configuration.allocatorConfiguration,
                                             configuration.streamConfiguration);
    configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                   &tcp_connection_configuration));
  } // end ELSE
  iterator =
    configuration.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration.connectionConfigurations.end ());

  //  config.delete_module = false;
  // *WARNING*: set at runtime, by the appropriate connection handler
  //  config.sessionID = 0; // (== socket handle !)
  //  config.statisticsReportingInterval = 0; // == off
  //	config.printFinalReport = false;
  // ************ runtime data ************
  //	config.currentStatistics = {};
  //	config.lastCollectionTimestamp = ACE_Time_Value::zero;

  // step0b: initialize event dispatch
  event_dispatch_state_s.configuration =
    &configuration.dispatchConfiguration;
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
    goto error;
  } // end IF

  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (thread_id);
  ACE_UNUSED_ARG (thread_id);

  // step1: initialize regular (global) statistic reporting
  if (statisticReportingInterval_in)
  {
    ACE_Time_Value interval (statisticReportingInterval_in,
                             0);
    timer_id =
      timer_manager_p->schedule_timer (&statistic_handler,         // event handler handle
                                       NULL,                       // asynchronous completion token
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (timer_id == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", aborting\n")));
      goto error;
    } // end IF
  } // end IF

  // step2: signal handling
  if (useReactor_in)
    configuration.listener = FILESERVER_LISTENER_SINGLETON::instance ();
  else
    configuration.listener = FILESERVER_ASYNCHLISTENER_SINGLETON::instance ();

  signal_handler_configuration.dispatchState =
    &event_dispatch_state_s;
  signal_handler_configuration.listener = configuration.listener;
  signal_handler_configuration.statisticReportingHandler = connection_manager_p;
  signal_handler_configuration.statisticReportingTimerId = timer_id;
  if (!signalHandler_in.initialize (signal_handler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, aborting\n")));
    goto error;
  } // end IF
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

  // step3: initialize connection manager
  connection_manager_p->initialize (maximumNumberOfConnections_in,
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  iconnection_manager_p->set (*dynamic_cast<FileServer_TCPConnectionConfiguration*> ((*iterator).second),
                              NULL);

  // step4: initialize listener
  //if (networkInterface_in.empty ()); // *TODO*
  if (useLoopBack_in)
  {
    result =
      NET_SOCKET_CONFIGURATION_TCP_CAST((*iterator).second)->address.set (listeningPortNumber_in,
                                                                          INADDR_LOOPBACK,
                                                                          1,
                                                                          0);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
      goto error;
    } // end IF
//    result =
//      configuration.UDPListenerConfiguration.listenAddress.set (listeningPortNumber_in,
//                                                                INADDR_LOOPBACK,
//                                                                1,
//                                                                0);
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));
//      goto error;
//    } // end IF
  } // end IF
  else
  {
    NET_SOCKET_CONFIGURATION_TCP_CAST((*iterator).second)->address.set_port_number (listeningPortNumber_in,
                                                                                    1);
//    configuration.UDPListenerConfiguration.listenAddress.set_port_number (listeningPortNumber_in,
//                                                                          1);
  } // end ELSE
  //configuration.listenerConfiguration.useLoopBackDevice = useLoopBack_in;

  // step5: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

#if defined (GUI_SUPPORT)
  // step1a: start UI event loop ?
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (GTK_USE)
    gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());


    //Common_UI_GladeDefinition ui_definition (argc_in,
//                                         argv_in);
    Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
    //CBData_in.configuration->GTKConfiguration.argc = argc_in;
    //CBData_in.configuration->GTKConfiguration.argv = argv_in;
    CBData_in.configuration->GTKConfiguration.CBData = &CBData_in;
    CBData_in.configuration->GTKConfiguration.definition = &gtk_ui_definition;
    CBData_in.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_ui_cb;
    CBData_in.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_ui_cb;
    CBData_in.UIState = &state_r;
    CBData_in.UIState->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
    ACE_ASSERT (gtk_manager_p);
    gtk_manager_p->initialize (CBData_in.configuration->GTKConfiguration);
#endif // GTK_USE
    //CBData_in.userData = &CBData_in;

#if defined (GTK_USE)
    ACE_ASSERT (gtk_manager_p);
    ACE_thread_t thread_id = 0;
    gtk_manager_p->start (thread_id);
    ACE_UNUSED_ARG (thread_id);
    ACE_Time_Value timeout (0,
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION * 1000);
    result = ACE_OS::sleep (timeout);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &timeout));
    if (!gtk_manager_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, aborting\n")));
      goto error;
    } // end IF
#endif // GTK_USE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), aborting\n")));
      goto error;
    } // end IF
    if (!showConsole_in)
      was_visible_b = ShowWindow (window_p, SW_HIDE);
#endif
  } // end IF
#endif // GUI_SUPPORT

  // step4b: initialize worker(s)
  if (!Common_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, aborting\n")));
    goto error;
  } // end IF
  stop_event_dispatch = true;

  // step4c: start listening ?
#if defined (GUI_SUPPORT)
  if (UIDefinitionFile_in.empty ())
  {
#endif // GUI_SUPPORT
    if (!configuration.listener->initialize (*dynamic_cast<FileServer_TCPConnectionConfiguration*> ((*iterator).second)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize listener, aborting\n")));
      goto error;
    } // end IF

    if (useUDP_in)
    {
      //Test_U_IInetConnector_t* iconnector_p = NULL;

      //if (useReactor_in)
      //  ACE_NEW_NORETURN (iconnector_p,
      //                    Test_U_UDPConnector_t (connection_manager_p,
      //                                           configuration.streamConfiguration.statisticReportingInterval));
      //else
      //  ACE_NEW_NORETURN (iconnector_p,
      //                    Test_U_UDPAsynchConnector_t (connection_manager_p,
      //                                                 configuration.streamConfiguration.statisticReportingInterval));
      //bool result_2 =
      //  iconnector_p->initialize (configuration);
      //if (!iconnector_p)
      //{
      //  ACE_DEBUG ((LM_CRITICAL,
      //              ACE_TEXT ("failed to allocate memory, returning\n")));
      //  goto error;
      //} // end IF
      ////  Stream_IInetConnector_t* iconnector_p = &connector;
      //if (!result_2)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
      //  goto error;
      //} // end IF

      //// connect
      //// *TODO*: support one-thread operation by scheduling a signal and manually
      ////         running the dispatch loop for a limited time...
      //configuration.handle =
      //  iconnector_p->connect (configuration.socketConfiguration.address);
      //if (!useReactor_in)
      //{
      //  // *TODO*: avoid tight loop here
      //  ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
      //  //result = ACE_OS::sleep (timeout);
      //  //if (result == -1)
      //  //  ACE_DEBUG ((LM_ERROR,
      //  //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
      //  //              &timeout));
      //  ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
      //  Test_U_UDPAsynchConnector_t::ICONNECTION_T* connection_p = NULL;
      //  do
      //  {
      //    connection_p =
      //      connection_manager_p->get (configuration.socketConfiguration.address);
      //    if (connection_p)
      //    {
      //      configuration.handle =
      //          reinterpret_cast<ACE_HANDLE> (connection_p->id ());
      //      connection_p->decrease ();
      //      break;
      //    } // end IF
      //  } while (COMMON_TIME_NOW < deadline);
      //} // end IF
      //result_2 = !(configuration.handle == ACE_INVALID_HANDLE);
      //if (!result_2)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to connect to \"%s\", returning\n"),
      //              ACE_TEXT (Net_Common_Tools::IPAddress2String (configuration.socketConfiguration.address).c_str ())));
      //  goto error;
      //} // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("listening to UDP \"%s\"...\n"),
      //            ACE_TEXT (Net_Common_Tools::IPAddress2String (configuration.socketConfiguration.address).c_str ())));

      //// clean up
      //delete iconnector_p;

      Test_U_UDPStream stream;
      if (!stream.initialize (configuration.streamConfiguration,
                              ACE_INVALID_HANDLE))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize stream, aborting\n")));
        goto error;
      } // end IF

      stream.start ();
      if (!stream.isRunning ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to start stream, aborting\n")));
        goto error;
      } // end IF
      stream.wait (true,  // wait for thread(s)
                   false, // wait for upstream ?
                   true); // wait for downstream ?
    } // end IF
    else
    {
      ACE_thread_t thread_id = 0;
      configuration.listener->start (thread_id);
      ACE_UNUSED_ARG (thread_id);
      if (!configuration.listener->isRunning ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to start listener (port: %u), aborting\n"),
                    listeningPortNumber_in));
        goto error;
      } // end IF
    } // end ELSE
#if defined (GUI_SUPPORT)
  } // end IF
  else
#if defined (GTK_USE)
    gtk_manager_p->wait ();
#else
    ;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // *NOTE*: from this point on, clean up any remote connections !

  if (!useUDP_in)
    Common_Tools::dispatchEvents (useReactor_in,
                                  group_id);
  stop_event_dispatch = false;

  // clean up
  // *NOTE*: listener has stopped, interval timer has been cancelled,
  // and connections have been aborted...

  //// wait for connection processing to complete
  //NET_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
  //NET_CONNECTIONMANAGER_SINGLETON::instance ()->wait ();

  result = event_handler.close (ACE_Module_Base::M_DELETE_NONE);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
                event_handler.name ()));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

error:
  timer_manager_p->stop ();
//		{ // synch access
//			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

//			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
//					 iterator != CBData_in.event_source_ids.end();
//					 iterator++)
//				g_source_remove(*iterator);
//		} // end lock scope
#if defined (GUI_SUPPORT)
  if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
    gtk_manager_p->stop (true);
#else
    ;
#endif // GTK_USE
#endif // GUI_SUPPORT
  if (stop_event_dispatch)
    Common_Tools::finalizeEventDispatch (useReactor_in,
                                         !useReactor_in,
                                         group_id);
  if (configuration.listener &&
      !useUDP_in)
    configuration.listener->stop (true,
                                  true);
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
           ACE_TCHAR** argv_in)
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

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  // initialize randomness
  // *TODO*: use STL functionality here
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("initializing random seed (RAND_MAX = %d)...\n"),
              RAND_MAX));
  ACE_Time_Value now = COMMON_TIME_NOW;
  random_seed = static_cast<unsigned int> (now.sec ());
  // *PORTABILITY*: outside glibc, this is not very portable...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::srand (static_cast<u_int> (random_seed));
#else
  ACE_OS::memset (random_state_buffer, 0, sizeof (random_state_buffer));
  result = ::initstate_r (random_seed,
                          random_state_buffer, sizeof (random_state_buffer),
                          &random_data);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initstate_r(): \"%s\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
  result = ::srandom_r (random_seed, &random_data);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize random seed: \"%s\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("initializing random seed...DONE\n")));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#endif
  std::string path = configuration_path;
  std::string source_file = Common_File_Tools::getTempDirectory ();
  source_file += ACE_DIRECTORY_SEPARATOR_STR;
  source_file += ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_DEFAULT_FILE);
  std::string UI_file_path = path;
  UI_file_path += ACE_DIRECTORY_SEPARATOR_STR;
  UI_file_path += ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_UI_FILE);
  std::string network_interface =
    ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET);
  //  unsigned int keep_alive_timeout = NET_SERVER_DEFAULT_TCP_KEEPALIVE;
  bool log_to_file = false;
  unsigned int maximum_number_of_connections =
    NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
  bool use_loopback = NET_INTERFACE_DEFAULT_USE_LOOPBACK;
  unsigned short listening_port_number = NET_SERVER_DEFAULT_LISTENING_PORT;
  bool use_reactor =
          (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval =
    NET_SERVER_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  bool trace_information = false;
  bool use_udp = false;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    NET_SERVER_DEFAULT_NUMBER_OF_DISPATCH_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#endif
                            source_file,
                            UI_file_path,
                            network_interface,
                            //keep_alive_timeout,
                            log_to_file,
                            maximum_number_of_connections,
                            use_loopback,
                            listening_port_number,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            use_udp,
                            print_version_and_exit,
                            number_of_dispatch_threads))
  {
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif
    return EXIT_FAILURE;
  } // end IF

  // step1c: validate arguments
  // *NOTE*: probably requires CAP_NET_BIND_SERVICE
  if (listening_port_number <= 1023)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("using (privileged) port #: %d...\n"),
                listening_port_number));

  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (NET_STREAM_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could lead to deadlocks...\n")));
  if ((UI_file_path.empty () && !Common_File_Tools::isReadable (source_file))   ||
      (!UI_file_path.empty () && !Common_File_Tools::isReadable (UI_file_path)) ||
      //(use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool)
      false)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, finalize ACE
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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_MessageStack_t* logstack_p = NULL;
  ACE_SYNCH_MUTEX* lock_p = NULL;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
  logstack_p = &state_r.logStack;
  lock_p = &state_r.logStackLock;
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_Logger_t logger (logstack_p,
                          lock_p);
#endif // GTK_USE
#endif // GUI_SUPPORT
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
        Net_Server_Common_Tools::getNextLogFileName (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                     ACE_TEXT_ALWAYS_CHAR (FILE_SERVER_LOG_FILENAME_PREFIX));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]),    // program name
                                            log_file_name,                 // log file name
                                            true,                          // log to syslog ?
                                            false,                         // trace messages ?
                                            trace_information,             // debug messages ?
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                            (UI_file_path.empty () ? NULL
                                                                   : &logger))) // (ui) logger ?
#elif defined (WXWIDGETS_USE)
                                            NULL))                              // (ui) logger ?
#endif // XXX_USE
#else
                                            NULL))                              // (ui) logger ?
#endif // GUI_SUPPORT
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif
    return EXIT_FAILURE;
  } // end IF

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif
    return EXIT_SUCCESS;
  } // end IF

#if defined (GUI_SUPPORT)
  // step1h: initialize UI framework
  struct FileServer_UI_CBData ui_cb_data;
  ui_cb_data.allowUserRuntimeStatistic =
    (statistic_reporting_interval == 0); // handle SIGUSR1/SIGBREAK
                                         // iff regular reporting
                                         // is off
#endif // GUI_SUPPORT

  // step1e: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (use_reactor,
                        (statistic_reporting_interval == 0), // handle SIGUSR1/SIGBREAK
                                                             // iff regular reporting
                                                             // is off
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
    // *PORTABILITY*: on Windows, finalize ACE
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
  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
#endif // GUI_SUPPORT
  FileServer_SignalHandler signal_handler ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                        : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                           lock_2);

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  // *TODO*: the reasoning here is incomplete
  bool use_fd_based_reactor = use_reactor;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  use_fd_based_reactor =
    (use_reactor && !(COMMON_EVENT_REACTOR_TYPE == COMMON_REACTOR_WFMO));
#endif
  bool stack_traces = true;
  bool use_signal_based_proactor = !use_reactor;
  if (!Common_Tools::setResourceLimits (use_fd_based_reactor,       // file descriptors
                                        stack_traces,               // stack traces
                                        use_signal_based_proactor)) // pending signals
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
  do_work (
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#endif
           source_file,
           UI_file_path,
           network_interface,
           //keep_alive_timeout,
           maximum_number_of_connections,
           use_loopback,
           listening_port_number,
           use_reactor,
           statistic_reporting_interval,
           use_udp,
           number_of_dispatch_threads,
           ui_cb_data,
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
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (ACE_Profile_Timer::Rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);
  std::string user_time_string;
  std::string system_time_string;
  user_time_string = Common_Timer_Tools::periodToString (user_time);
  system_time_string = Common_Timer_Tools::periodToString (system_time);

  // debug info
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  ACE_DEBUG((LM_DEBUG,
             ACE_TEXT(" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\nmaximum resident set size = %d\nintegral shared memory size = %d\nintegral unshared data size = %d\nintegral unshared stack size = %d\npage reclaims = %d\npage faults = %d\nswaps = %d\nblock input operations = %d\nblock output operations = %d\nmessages sent = %d\nmessages received = %d\nsignals received = %d\nvoluntary context switches = %d\ninvoluntary context switches = %d\n"),
             elapsed_time.real_time,
             elapsed_time.user_time,
             elapsed_time.system_time,
             user_time_string.c_str(),
             system_time_string.c_str(),
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
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
    return EXIT_FAILURE;
  } // end IF
#endif

  return EXIT_SUCCESS;
} // end main
