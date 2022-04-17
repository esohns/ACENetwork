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
#include <list>
#include <sstream>
#include <string>

#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN32
#include "ace/Log_Msg.h"
#include "ace/Synch.h"
#include "ace/Proactor.h"
#include "ace/Profile_Timer.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
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

#include "common_logger.h"
#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_second_publisher.h"
#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_defines.h"
//#include "common_ui_gtk_glade_definition.h"
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

#include "net_defines.h"
#include "net_iconnector.h"
#include "net_macros.h"

#include "test_u_common_tools.h"
#include "test_u_defines.h"

#include "net_client_server_defines.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#include "test_u_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_stream.h"
#include "test_u_eventhandler.h"
#include "test_u_message.h"
#include "test_u_module_eventhandler.h"
#include "test_u_sessionmessage.h"

#include "net_client_common.h"
#include "net_client_connector_common.h"
#include "net_client_defines.h"
#include "net_client_signalhandler.h"
#include "net_client_timeouthandler.h"

#include "net_server_defines.h"

// globals
unsigned int random_seed;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct random_data random_data;
char random_state_buffer[BUFSIZ];
#endif // ACE_WIN32 || ACE_WIN64

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("NetClientStream");

void
do_printUsage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-a           : alternating mode [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [VALUE]   : maximum #connections [")
            << NET_CLIENT_DEFAULT_MAX_NUM_OPEN_CONNECTIONS
            << ACE_TEXT_ALWAYS_CHAR ("] {0 --> unlimited}")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
  std::string UI_file_path = path;
  UI_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file_path += ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]] : UI file [\"")
            << UI_file_path
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
#endif // GUI_SUPPORT
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i [VALUE]   : connection interval (s) [")
            << NET_CLIENT_DEFAULT_SERVER_CONNECT_INTERVAL
            << ACE_TEXT_ALWAYS_CHAR ("] {0 --> OFF}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l           : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n [STRING]  : server hostname [\"")
            << ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_DEFAULT_SERVER_HOSTNAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]   : server port [")
            << NET_SERVER_DEFAULT_LISTENING_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r           : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s           : ping interval (ms) [")
            << NET_CLIENT_DEFAULT_SERVER_PING_INTERVAL
            << ACE_TEXT_ALWAYS_CHAR ("] {0: OFF}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t           : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u           : use UDP [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v           : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  // *IMPORTANT NOTE*: iff -r is set and this is <= 1, use the 'select' reactor
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]   : #dispatch threads [")
            << NET_CLIENT_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-y           : run stress-test [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     bool& alternatingMode_out,
                     unsigned int& maximumNumberOfConnections_out,
#if defined (SSL_SUPPORT)
                     std::string& certificateFile_out,
                     std::string& privateKeyFile_out,
#endif // SSL_SUPPORT
#if defined (GUI_SUPPORT)
                     std::string& UIFile_out,
#endif // GUI_SUPPORT
                     unsigned int& connectionInterval_out,
                     bool& logToFile_out,
                     std::string& serverHostname_out,
                     unsigned short& serverPortNumber_out,
                     bool& useReactor_out,
                     ACE_Time_Value& pingInterval_out,
                     bool& traceInformation_out,
                     enum Net_TransportLayerType& protocol_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numDispatchThreads_out,
                     bool& runStressTest_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  alternatingMode_out = false;
  maximumNumberOfConnections_out = NET_CLIENT_DEFAULT_MAX_NUM_OPEN_CONNECTIONS;
  std::string path;
#if defined (SSL_SUPPORT)
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  certificateFile_out = path;
  certificateFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  certificateFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_PEM_CERTIFICATE_FILE);
  privateKeyFile_out = path;
  privateKeyFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  privateKeyFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_PEM_PRIVATE_KEY_FILE);
#endif // SSL_SUPPORT
#if defined (GUI_SUPPORT)
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  UIFile_out = path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_FILE);
#endif // GUI_SUPPORT
  connectionInterval_out = NET_CLIENT_DEFAULT_SERVER_CONNECT_INTERVAL;
  logToFile_out = false;
  serverHostname_out =
      ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_DEFAULT_SERVER_HOSTNAME);
  serverPortNumber_out = NET_SERVER_DEFAULT_LISTENING_PORT;
  useReactor_out =
          (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  pingInterval_out.set (NET_CLIENT_DEFAULT_SERVER_PING_INTERVAL / 1000,
                        (NET_CLIENT_DEFAULT_SERVER_PING_INTERVAL % 1000) * 1000);
  traceInformation_out = false;
  protocol_out = NET_TRANSPORTLAYER_TCP;
  printVersionAndExit_out = false;
  numDispatchThreads_out = NET_CLIENT_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS;
  runStressTest_out = false;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (GUI_SUPPORT)
                              ACE_TEXT ("ac:e:f:g::i:ln:p:rSs:tuvx:y"),
#else
                              ACE_TEXT ("ac:e:f:i:ln:p:rSs:tuvx:y"),
#endif // GUI_SUPPORT
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
      case 'a':
      {
        alternatingMode_out = true;
        break;
      }
      case 'c':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> maximumNumberOfConnections_out;
        break;
      }
#if defined (GUI_SUPPORT)
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
#endif // GUI_SUPPORT
      case 'i':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> connectionInterval_out;
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'n':
      {
        serverHostname_out = argumentParser.opt_arg ();
        break;
      }
      case 'p':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> serverPortNumber_out;
        break;
      }
      case 'r':
      {
        useReactor_out = true;
        break;
      }
      case 's':
      {
        unsigned int ping_interval = 0;
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> ping_interval;
        pingInterval_out.set (ping_interval / 1000,
                              (ping_interval % 1000) * 1000);
        break;
      }
      case 'S':
      {
        protocol_out = NET_TRANSPORTLAYER_SSL;
        break;
      }
      case 't':
      {
        traceInformation_out = true;
        break;
      }
      case 'u':
      {
        protocol_out = NET_TRANSPORTLAYER_UDP;
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
        converter >> numDispatchThreads_out;
        break;
      }
      case 'y':
      {
        runStressTest_out = true;
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
                ACE_TEXT("failed to ACE_Sig_Set::fill_set(): \"%m\", returning\n")));
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

#ifdef RPG_ENABLE_VALGRIND_SUPPORT
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif
#endif
}

void
do_work (enum Client_TimeoutHandler::ActionModeType actionMode_in,
         unsigned int maxNumConnections_in,
#if defined (SSL_SUPPORT)
         const std::string& certificateFile_in,
         const std::string& privateKeyFile_in,
#endif // SSL_SUPPORT
#if defined (GUI_SUPPORT)
         const std::string& UIDefinitionFile_in,
#endif // GUI_SUPPORT
         unsigned int connectionInterval_in,
         const std::string& serverHostname_in,
         unsigned short serverPortNumber_in,
         bool useReactor_in,
         const ACE_Time_Value& pingInterval_in,
         const ACE_Time_Value& statisticReportingInterval_in,
         unsigned int numberOfDispatchThreads_in,
         enum Net_TransportLayerType protocol_in,
         struct Client_Configuration& configuration_in,
#if defined (GUI_SUPPORT)
         struct Client_UI_CBData& CBData_in,
#endif // GUI_SUPPORT
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Client_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

  // step0a: initialize random number generator
  Test_U_Common_Tools::initialize ();

  // step0a: initialize configuration
#if defined (GUI_SUPPORT)
  CBData_in.configuration = &configuration_in;
  CBData_in.progressData.configuration = &configuration_in;

  Test_U_EventHandler_t ui_event_handler (&CBData_in);
#else
  Test_U_EventHandler_t ui_event_handler ();
#endif // GUI_SUPPORT
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
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (configuration_in.allocatorConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  Test_U_MessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,         // heap allocator handle
                                               true);                   // block ?
  // ********************* protocol configuration data *************************
  configuration_in.protocolConfiguration.pingInterval =
    ((actionMode_in == Client_TimeoutHandler::ACTION_STRESS) ? ACE_Time_Value::zero
                                                             : pingInterval_in);
  configuration_in.protocolConfiguration.printPongMessages =
    UIDefinitionFile_in.empty ();
  configuration_in.protocolConfiguration.transportLayer = protocol_in;
  // ********************** stream configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
  struct ClientServer_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_U_StreamConfiguration stream_configuration;
  modulehandler_configuration.computeThroughput = true;
  modulehandler_configuration.printFinalReport = true;
  modulehandler_configuration.protocolConfiguration =
    &configuration_in.protocolConfiguration;
  modulehandler_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;
#if defined (GUI_SUPPORT)
  modulehandler_configuration.subscriber = &ui_event_handler;
  modulehandler_configuration.subscribers = &CBData_in.subscribers;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  modulehandler_configuration.lock = &CBData_in.UIState->subscribersLock;
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT

  stream_configuration.allocatorConfiguration =
      &configuration_in.allocatorConfiguration;
  stream_configuration.cloneModule = true;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module =
#if defined (GUI_SUPPORT)
      (!UIDefinitionFile_in.empty () ? &event_handler
                                     : NULL);
#else
      NULL;
#endif // GUI_SUPPORT
  stream_configuration.printFinalReport = true;
  // *TODO*: is this correct ?
  stream_configuration.serializeOutput =
    (useReactor_in && (numberOfDispatchThreads_in > 1)); // <-- using 'threadpool' reactor
  //configuration_in.streamConfiguration.configuration_.userData =
  //  &configuration_in.userData;
  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);

  // ********************** connection configuration data **********************
  Test_U_TCPConnectionConfiguration tcp_connection_configuration;
  Test_U_UDPConnectionConfiguration udp_connection_configuration;
  tcp_connection_configuration.allocatorConfiguration =
      &configuration_in.allocatorConfiguration;
  tcp_connection_configuration.messageAllocator = &message_allocator;
  tcp_connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  tcp_connection_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;

  udp_connection_configuration.allocatorConfiguration =
      &configuration_in.allocatorConfiguration;
  udp_connection_configuration.messageAllocator = &message_allocator;
  udp_connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  //udp_connection_configuration.writeOnly = true;
  udp_connection_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;
  Net_ConnectionConfigurationsIterator_t iterator, iterator_2;

  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("TCP"),
                                                                    &tcp_connection_configuration));
  iterator =
    configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("TCP"));
  ACE_ASSERT (iterator != configuration_in.connectionConfigurations.end ());
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("UDP"),
                                                                    &udp_connection_configuration));
  iterator_2 =
    configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("UDP"));
  ACE_ASSERT (iterator_2 != configuration_in.connectionConfigurations.end ());

  //  config.useThreadPerConnection = false;
  //  config.serializeOutput = false;

  //  config.notificationStrategy = NULL;
//  config.delete_module = false;
  // *WARNING*: set at runtime, by the appropriate connection handler
//  config.sessionID = 0; // (== socket handle !)
//  config.statisticsReportingInterval = 0; // == off
//	config.printFinalReport = false;
  // ************ runtime data ************
//	config.currentStatistics = {};
//	config.lastCollectionTimestamp = ACE_Time_Value::zero;

  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Common_Timer_Tools::configuration_.dispatch = COMMON_TIMER_DISPATCH_QUEUE;
  Common_Timer_Tools::configuration_.publishSeconds = true;
  timer_manager_p->initialize (Common_Timer_Tools::configuration_);
  timer_manager_p->start (NULL);
  //    (useReactor_in ? COMMON_TIMER_DISPATCH_REACTOR : COMMON_TIMER_DISPATCH_PROACTOR);
  Common_Timer_Tools::initialize ();

  // step0c: initialize connector
  Test_U_TCPConnectionManager_t* connection_manager_p =
    TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  typename Test_U_TCPConnectionManager_t::INTERFACE_T* iconnection_manager_p =
    connection_manager_p;
  Test_U_UDPConnectionManager_t* connection_manager_2 =
    TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_2);
  typename Test_U_UDPConnectionManager_t::INTERFACE_T* iconnection_manager_2 =
    connection_manager_2;

  Client_TCP_AsynchConnector_t asynch_tcp_connector (true);
  Client_TCP_Connector_t tcp_connector (true);
#if defined (SSL_SUPPORT)
  Client_SSL_Connector_t ssl_connector (true);

  std::string filename_string =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                      ACE_TEXT_ALWAYS_CHAR (""),
                                                      false); // data
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (NET_PROTOCOL_DEFAULT_SSL_TRUSTED_CAS_FILENAME_STRING);

  if (unlikely (!Net_Common_Tools::initializeSSLContext (certificateFile_in,
                                                         privateKeyFile_in,
                                                         filename_string,    // trusted CAs
                                                         true,               // client mode
                                                         NULL)))             // default context
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::initializeSSLContext(\"%s\",\"%s\",NULL), returning\n"),
                ACE_TEXT (certificateFile_in.c_str ()),
                ACE_TEXT (privateKeyFile_in.c_str ())));
    return;
  } // end IF
#endif // SSL_SUPPORT
  Client_UDP_AsynchConnector_t asynch_udp_connector (true);
  Client_UDP_Connector_t udp_connector (true);
  Test_U_ITCPConnector_t* connector_p = NULL;
  Test_U_IUDPConnector_t* connector_2 = NULL;
  switch (protocol_in)
  {
    case NET_TRANSPORTLAYER_TCP:
    {
      if (useReactor_in)
        connector_p = &tcp_connector;
      else
        connector_p = &asynch_tcp_connector;
      break;
    }
#if defined (SSL_SUPPORT)
    case NET_TRANSPORTLAYER_SSL:
    {
      if (useReactor_in)
        connector_p = &ssl_connector;
//      else
//        connector_p = &asynch_ssl_connector;
      break;
    }
#endif // SSL_SUPPORT
    case NET_TRANSPORTLAYER_UDP:
    {
      if (useReactor_in)
        connector_2 = &udp_connector;
      else
        connector_2 = &asynch_udp_connector;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown protocol (was: %d), returning\n"),
                  protocol_in));
      return;
    }
  } // end SWITCH
  switch (protocol_in)
  {
    case NET_TRANSPORTLAYER_TCP:
    case NET_TRANSPORTLAYER_SSL:
    { ACE_ASSERT (connector_p);
      if (!connector_p->initialize (tcp_connection_configuration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
        return;
      } // end IF
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    { ACE_ASSERT (connector_2);
      if (!connector_2->initialize (udp_connection_configuration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
        return;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown protocol (was: %d), returning\n"),
                  protocol_in));
      return;
    }
  } // end SWITCH

  // step0d: initialize connection manager
  struct Net_UserData user_data;
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (tcp_connection_configuration,
                             NULL);
  connection_manager_p->start (NULL);
  connection_manager_2->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_2->set (udp_connection_configuration,
                             NULL);
  connection_manager_2->start (NULL);

  // step0e: initialize action timer
  ACE_INET_Addr peer_address (serverPortNumber_in,
                              serverHostname_in.c_str (),
                              ACE_ADDRESS_FAMILY_INET);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("set peer address: %s\n"),
              ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
  tcp_connection_configuration.socketConfiguration.address = peer_address;
  if (!useReactor_in)
    udp_connection_configuration.socketConfiguration.connect = true;
  udp_connection_configuration.socketConfiguration.peerAddress = peer_address;
  ACE_INET_Addr listen_address;
  result =
    udp_connection_configuration.socketConfiguration.listenAddress.set (static_cast<u_short> (serverPortNumber_in + 1),
                                                                        static_cast<ACE_UINT32> (INADDR_ANY),
                                                                        1,
                                                                        0);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set() (was: %s), continuing\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("set listen address: %s\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (udp_connection_configuration.socketConfiguration.listenAddress).c_str ())));

  configuration_in.signalHandler = &signalHandler_in;
  Client_TimeoutHandler timeout_handler (actionMode_in,
                                         maxNumConnections_in,
                                         configuration_in.protocolConfiguration,
                                         tcp_connection_configuration,
                                         udp_connection_configuration,
                                         (useReactor_in ? COMMON_EVENT_DISPATCH_REACTOR : COMMON_EVENT_DISPATCH_PROACTOR));
  configuration_in.timeoutHandler = &timeout_handler;
  if (
#if defined (GUI_SUPPORT)
      UIDefinitionFile_in.empty () &&
#endif // GUI_SUPPORT
      (connectionInterval_in > 0))
  {
    // schedule action interval timer
    ACE_Time_Value interval (((actionMode_in == Client_TimeoutHandler::ACTION_STRESS) ? (NET_CLIENT_DEFAULT_SERVER_STRESS_INTERVAL / 1000)
                                                                                      : connectionInterval_in),
                             ((actionMode_in == Client_TimeoutHandler::ACTION_STRESS) ? ((NET_CLIENT_DEFAULT_SERVER_STRESS_INTERVAL % 1000) * 1000)
                                                                                      : 0));
    configuration_in.signalHandlerConfiguration.actionTimerId =
        timer_manager_p->schedule_timer (&timeout_handler,           // event handler handle
                                         NULL,                       // asynchronous completion token
                                         COMMON_TIME_NOW + interval, // first wakeup time
                                         interval);                  // interval
    if (configuration_in.signalHandlerConfiguration.actionTimerId == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule action timer: \"%m\", returning\n")));
      Common_Timer_Tools::finalize ();
      timer_manager_p->stop ();
      return;
    } // end IF
  } // end IF

  // step0e: initialize signal handling
  // *WORKAROUND*: ACE_INET_Addr::operator= is broken
  configuration_in.signalHandlerConfiguration.address = peer_address;
  configuration_in.signalHandlerConfiguration.TCPConnectionConfiguration =
    &tcp_connection_configuration;
  configuration_in.signalHandlerConfiguration.UDPConnectionConfiguration =
    &udp_connection_configuration;
  configuration_in.signalHandlerConfiguration.protocolConfiguration =
    &configuration_in.protocolConfiguration;
  configuration_in.signalHandlerConfiguration.messageAllocator =
    &message_allocator;
  configuration_in.signalHandlerConfiguration.stopEventDispatchOnShutdown =
    UIDefinitionFile_in.empty ();
  if (!signalHandler_in.initialize (configuration_in.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    Common_Timer_Tools::finalize ();
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
    Common_Timer_Tools::finalize ();
    timer_manager_p->stop ();
    return;
  } // end IF

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  // step1a: start GTK event loop ?
  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
#endif // GTK_USE
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (GTK_USE)
    gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);

    ACE_ASSERT (CBData_in.UIState);
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    CBData_in.UIState->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));

    gtk_manager_p->start (NULL);
    ACE_Time_Value timeout (0,
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
    result = ACE_OS::sleep (timeout);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &timeout));
    if (!gtk_manager_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
      Common_Timer_Tools::finalize ();
      timer_manager_p->stop ();
      return;
    } // end IF
#endif // GTK_USE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
      Common_Timer_Tools::finalize ();
      timer_manager_p->stop ();
#if defined (GTK_USE)
      gtk_manager_p->stop (true, true);
#endif // GTK_USE
      return;
    } // end IF
    BOOL was_visible_b = ::ShowWindow (window_p, SW_HIDE);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
#endif // GUI_SUPPORT

  // step1b: initialize worker(s)
  if (!Common_Tools::startEventDispatch (*configuration_in.signalHandlerConfiguration.dispatchState))
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
    Common_Timer_Tools::finalize ();
    timer_manager_p->stop ();
#if defined (GUI_SUPPORT)
    if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
      gtk_manager_p->stop (true, true);
#else
      ;
#endif // GTK_USE
#endif // GUI_SUPPORT
    return;
  } // end IF

  // step1c: connect immediately ?
  if (
#if defined (GUI_SUPPORT)
      UIDefinitionFile_in.empty () &&
#endif // GUI_SUPPORT
      !connectionInterval_in)
  {
    ACE_HANDLE handle_h = connector_p->connect (peer_address);
    if (handle_h == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));

      // clean up
      if (numberOfDispatchThreads_in >= 1)
        Common_Tools::finalizeEventDispatch (*configuration_in.signalHandlerConfiguration.dispatchState,
                                             false); // don't block
      //		{ // synch access
      //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

      //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
      //					 iterator != CBData_in.event_source_ids.end();
      //					 iterator++)
      //				g_source_remove(*iterator);
      //		} // end lock scope
      Common_Timer_Tools::finalize ();
      timer_manager_p->stop ();
#if defined (GUI_SUPPORT)
      if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
        gtk_manager_p->stop (true,
                             true);
#else
        ;
#endif // GTK_USE
#endif // GUI_SUPPORT
      return;
    } // end IF
    Test_U_TCPConnectionManager_t::ICONNECTION_T* iconnection_p = NULL;
//    Test_U_UDPConnectionManager_t::ICONNECTION_T* iconnection_2 = NULL;
    if (useReactor_in)
    {
      iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle_h));
#else
          iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (handle_h));
#endif
    } // end IF
    else
    {
      // step1: wait for the connection to register with the manager
      ACE_Time_Value timeout (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_S,
                              0);
      ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
      // *TODO*: avoid these tight loops
      // *TODO*: this may not be accurate/applicable for/to all protocols
      do
      {
        iconnection_p = connection_manager_p->get (peer_address,
                                                   true);
        if (iconnection_p)
          break;
      } while (COMMON_TIME_NOW < deadline);
    } // end ELSE
    if (!iconnection_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));

      // clean up
      if (numberOfDispatchThreads_in >= 1)
        Common_Tools::finalizeEventDispatch (*configuration_in.signalHandlerConfiguration.dispatchState,
                                             false);                                                     // don't block
      //		{ // synch access
      //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

      //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
      //					 iterator != CBData_in.event_source_ids.end();
      //					 iterator++)
      //				g_source_remove(*iterator);
      //		} // end lock scope
      Common_Timer_Tools::finalize ();
      timer_manager_p->stop ();
#if defined (GUI_SUPPORT)
      if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
        gtk_manager_p->stop (true, true);
#else
        ;
#endif // GTK_USE
#endif // GUI_SUPPORT
      return;
    } // end IF
    iconnection_p->decrease ();
  } // end IF

  // *NOTE*: from this point on, clean up any remote connections !

  if (UIDefinitionFile_in.empty ())
  {
    // step2: dispatch events
    Common_Tools::dispatchEvents (*configuration_in.signalHandlerConfiguration.dispatchState);
  } // end IF
  else
  {
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    gtk_manager_p->wait ();
#else
    ;
#endif // GTK_USE
#endif // GUI_SUPPORT
  } // end ELSE

  // step3: clean up
  //		{ // synch access
  //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //					 iterator != CBData_in.event_source_ids.end();
  //					 iterator++)
  //				g_source_remove(*iterator);
  //		} // end lock scope
  timer_manager_p->stop ();
  timer_manager_p->wait ();
  Common_Timer_Tools::finalize ();

//  connection_manager_p->abort ();
  //TEST_U_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
  // *IMPORTANT NOTE*: as long as connections are inactive (i.e. events are
  // dispatched by reactor thread(s), there is no real reason to wait here)
//  connection_manager_p->wait ();
  TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ()->wait ();
  TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ()->wait ();

  // step5: stop reactor (&& proactor, if applicable)
  Common_Tools::finalizeEventDispatch (*configuration_in.signalHandlerConfiguration.dispatchState,
                                       false); // don't block

//  { // synch access
//    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

//		for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
//				 iterator != CBData_in.event_source_ids.end();
//				 iterator++)
//			g_source_remove(*iterator);
//	} // end lock scope

  result = event_handler.close (ACE_Module_Base::M_DELETE_NONE);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
                event_handler.name ()));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
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

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (false, // COM ?
                            true); // RNG ?
#else
  Common_Tools::initialize (true); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  enum Client_TimeoutHandler::ActionModeType action_mode =
    Client_TimeoutHandler::ACTION_NORMAL;
  bool alternating_mode = false;
  unsigned int maximum_number_of_connections =
   NET_CLIENT_DEFAULT_MAX_NUM_OPEN_CONNECTIONS;
  std::string path;
#if defined (SSL_SUPPORT)
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string certificate_file = path;
  certificate_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  certificate_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_PEM_CERTIFICATE_FILE);
  std::string private_key_file = path;
  private_key_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  private_key_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_PEM_PRIVATE_KEY_FILE);
#endif // SSL_SUPPORT
#if defined (GUI_SUPPORT)
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string UI_file_path = path;
  UI_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file_path += ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_FILE);
#endif // GUI_SUPPORT
  unsigned int connection_interval =
   NET_CLIENT_DEFAULT_SERVER_CONNECT_INTERVAL;
  bool log_to_file = false;
  std::string server_hostname =
    ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_DEFAULT_SERVER_HOSTNAME);
  unsigned short server_port_number =
   NET_SERVER_DEFAULT_LISTENING_PORT;
  bool use_reactor =
          (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  ACE_Time_Value ping_interval (NET_CLIENT_DEFAULT_SERVER_PING_INTERVAL / 1000,
                                (NET_CLIENT_DEFAULT_SERVER_PING_INTERVAL % 1000) * 1000);
  ACE_Time_Value statistic_reporting_interval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S,
                                               0);
  bool trace_information = false;
  enum Net_TransportLayerType protocol_e = NET_TRANSPORTLAYER_TCP;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
   NET_CLIENT_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS;
  bool run_stress_test = false;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            alternating_mode,
                            maximum_number_of_connections,
#if defined (SSL_SUPPORT)
                            certificate_file,
                            private_key_file,
#endif // SSL_SUPPORT
#if defined (GUI_SUPPORT)
                            UI_file_path,
#endif // GUI_SUPPORT
                            connection_interval,
                            log_to_file,
                            server_hostname,
                            server_port_number,
                            use_reactor,
                            ping_interval,
                            trace_information,
                            protocol_e,
                            print_version_and_exit,
                            number_of_dispatch_threads,
                            run_stress_test))
  {
    // make 'em learn
    do_printUsage (ACE::basename (argv_in[0]));

    Common_Tools::finalize ();
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
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (NET_STREAM_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could lead to deadlocks...\n")));
  if (
#if defined (GUI_SUPPORT)
      (!UI_file_path.empty () && !Common_File_Tools::isReadable (UI_file_path)) ||
#endif // GUI_SUPPORT
      //(use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool)     ||
      (run_stress_test && ((ping_interval != ACE_Time_Value::zero)              ||
                           (connection_interval != 0)))                         ||
      (alternating_mode && run_stress_test))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));
    do_printUsage (ACE::basename (argv_in[0]));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif
    return EXIT_FAILURE;
  } // end IF
#if defined (GUI_SUPPORT)
  if (!UI_file_path.empty () && !Common_File_Tools::isReadable (UI_file_path))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid UI definition file (was: %s), aborting\n"),
                ACE_TEXT (UI_file_path.c_str ())));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif
    return EXIT_FAILURE;
  } // end IF
#endif // GUI_SUPPORT
  if (number_of_dispatch_threads == 0)
    number_of_dispatch_threads = 1;
  if (alternating_mode)
    action_mode = Client_TimeoutHandler::ACTION_ALTERNATING;
  if (run_stress_test)
    action_mode = Client_TimeoutHandler::ACTION_STRESS;

#if defined (GUI_SUPPORT)
  struct Client_UI_CBData ui_cb_data;
#endif // GUI_SUPPORT

  // step1d: initialize logging and/or tracing
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_MessageStack_t* logstack_p = NULL;
  ACE_SYNCH_MUTEX* lock_p = NULL;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  logstack_p = &state_r.logStack;
  lock_p = &state_r.logStackLock;
  ui_cb_data.UIState = &state_r;
  ui_cb_data.progressData.state = &state_r;
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
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                        ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]),    // program name
                                            log_file_name,                 // log file name
                                            false,                         // log to syslog ?
                                            false,                         // trace messages ?
                                            trace_information,             // debug messages ?
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                            (UI_file_path.empty () ? NULL
                                                                   : &logger))) // (ui) logger ?
#elif defined (WXWIDGETS_USE)
                                            NULL))                              // (ui) logger ?
#else
                                            NULL))                              // (ui) logger ?
#endif // XXX_USE
#else
                                            NULL))                              // (ui) logger ?
#endif // GUI_SUPPORT
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
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
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif
    return EXIT_SUCCESS;
  } // end IF

  // step1d: initialize event dispatch
  struct Client_Configuration configuration;
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
    &configuration.dispatchConfiguration;
  if (use_reactor)
  {
    configuration.dispatchConfiguration.numberOfReactorThreads =
      number_of_dispatch_threads;
    configuration.dispatchConfiguration.reactorType =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ((number_of_dispatch_threads > 1) ? COMMON_REACTOR_THREAD_POOL
                                          : COMMON_REACTOR_ACE_DEFAULT);
#else
        ((number_of_dispatch_threads > 1) ? COMMON_REACTOR_DEV_POLL
                                          : COMMON_REACTOR_ACE_DEFAULT);
#endif
  } // end IF
  else
  {
    configuration.dispatchConfiguration.numberOfReactorThreads =
      1;
    configuration.dispatchConfiguration.numberOfProactorThreads =
      number_of_dispatch_threads;
  } // end ELSE
  if (!Common_Tools::initializeEventDispatch (configuration.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize event dispatch, aborting\n")));

    Common_Log_Tools::finalizeLogging ();
    Common_Tools::finalize ();
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
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals ((connection_interval == 0), // allow SIGUSR1/SIGBREAK iff
                                                    // regular connections are off
                        signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           (use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                        : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                           true, // use networking
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

    Common_Log_Tools::finalizeLogging ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  configuration.signalHandlerConfiguration.dispatchState =
    &event_dispatch_state_s;
  Client_SignalHandler signal_handler;

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  // *TODO*: the reasoning here is incomplete
  bool use_fd_based_reactor = true;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  use_fd_based_reactor =
    (use_reactor && !(COMMON_EVENT_REACTOR_TYPE == COMMON_REACTOR_WFMO));
#endif // ACE_WIN32 || ACE_WIN64
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
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1h: initialize UI framework
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  configuration.GTKConfiguration.argc = argc_in;
  configuration.GTKConfiguration.argv = argv_in;
  configuration.GTKConfiguration.CBData = &ui_cb_data;
  configuration.GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  configuration.GTKConfiguration.eventHooks.initHook =
      idle_initialize_client_UI_cb;
  configuration.GTKConfiguration.definition = &gtk_ui_definition;
  if (!UI_file_path.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (configuration.GTKConfiguration);
#endif // GTK_USE
#endif // GUI_SUPPORT

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (action_mode,
           maximum_number_of_connections,
#if defined (SSL_SUPPORT)
           certificate_file,
           private_key_file,
#endif // SSL_SUPPORT
#if defined (GUI_SUPPORT)
           UI_file_path,
#endif // GUI_SUPPORT
           connection_interval,
           server_hostname,
           server_port_number,
           use_reactor,
           ping_interval,
           statistic_reporting_interval,
           number_of_dispatch_threads,
           protocol_e,
           configuration,
#if defined (GUI_SUPPORT)
           ui_cb_data,
#endif // GUI_SUPPORT
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
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    Common_Tools::finalize ();
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
#endif

  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalizeLogging ();
  Common_Tools::finalize ();

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

  return EXIT_SUCCESS;
} // end main
