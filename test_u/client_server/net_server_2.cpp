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

#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN32
#include "ace/Log_Msg.h"
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
#include "common_os_tools.h"

#include "common_event_tools.h"

#include "common_logger_queue.h"
#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_second_publisher.h"
#include "common_timer_tools.h"

#include "common_ui_defines.h"
#if defined (GTK_SUPPORT)
//#include "common_ui_glade_definition.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

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

#include "test_u_defines.h"

#include "net_client_defines.h"
#include "net_client_server_defines.h"
#if defined (GTK_SUPPORT)
#include "test_u_callbacks_2.h"
#endif // GTK_SUPPORT
#include "test_u_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_stream.h"
#include "test_u_eventhandler.h"
#include "test_u_eventhandler_2.h"

#include "net_server_common.h"
#include "net_server_common_tools.h"
#include "net_server_defines.h"
#include "net_server_listener_common.h"
#include "net_server_signalhandler.h"

// globals
unsigned int random_seed;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct random_data random_data;
char random_state_buffer[BUFSIZ];
#endif // ACE_WIN32 || ACE_WIN64

const char stream_name_io_string_[] = ACE_TEXT_ALWAYS_CHAR ("NetIOStream");
const char stream_name_server_string_[] = ACE_TEXT_ALWAYS_CHAR ("NetServerStream");

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:") << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [VALUE]   : maximum number of (concurrent) connections [")
            << NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string UI_file_path = path;
  UI_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file_path += ACE_TEXT_ALWAYS_CHAR (NET_SERVER_2_UI_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]] : UI definition file [\"")
            << UI_file_path
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i [VALUE]   : client ping interval (ms) [")
            << NET_SERVER_DEFAULT_CLIENT_PING_INTERVAL_MS
            << ACE_TEXT_ALWAYS_CHAR ("] {0: off})")
            << std::endl;
//  std::cout << ACE_TEXT_ALWAYS_CHAR("-k [VALUE]  : client keep-alive timeout ([")
//            << NET_SERVER_DEF_CLIENT_KEEPALIVE
//            << ACE_TEXT_ALWAYS_CHAR("] second(s) {0 --> no timeout})")
//            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l           : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-m           : receive uni/multi/broadcast UDP [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n [STRING]  : network interface identifier [\"")
            << ACE_TEXT_ALWAYS_CHAR (Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3).c_str ())
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  // *TODO*: this doesn't really make sense (see '-n' option)
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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-S           : use SSL [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]   : statistic reporting interval (second(s)) [")
            << NET_SERVER_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
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
            << NET_SERVER_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (const int& argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& maximumNumberOfConnections_out,
#if defined (SSL_SUPPORT)
                     std::string& certificateFile_out,
                     std::string& privateKeyFile_out,
#endif // SSL_SUPPORT
                     std::string& UIFile_out,
                     ACE_Time_Value& pingInterval_out,
                     //unsigned int& keepAliveTimeout_out,
                     bool& logToFile_out,
                     std::string& networkInterface_out,
                     bool& useLoopBack_out,
                     unsigned short& listeningPortNumber_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     enum Net_TransportLayerType& protocol_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  maximumNumberOfConnections_out =
    NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
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
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  UIFile_out = path;
  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIFile_out += ACE_TEXT_ALWAYS_CHAR (NET_SERVER_2_UI_FILE);
  pingInterval_out.set (NET_SERVER_DEFAULT_CLIENT_PING_INTERVAL_MS / 1000,
                        (NET_SERVER_DEFAULT_CLIENT_PING_INTERVAL_MS % 1000) * 1000);
//  keepAliveTimeout_out = NET_SERVER_DEF_CLIENT_KEEPALIVE;
  logToFile_out = false;
  networkInterface_out =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
  useLoopBack_out = NET_INTERFACE_DEFAULT_USE_LOOPBACK;
  listeningPortNumber_out = NET_SERVER_DEFAULT_LISTENING_PORT;
  useReactor_out =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
      NET_SERVER_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  protocol_out = NET_TRANSPORTLAYER_TCP;
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    NET_SERVER_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS;

  ACE_Get_Opt argument_parser (argc_in,                                   // argc
                               argv_in,                                   // argv
                               ACE_TEXT ("c:e:f:g::i:k:ln:op:rSs:tuvx:"), // optstring
                               1,                                         // skip_args
                               1,                                         // report_errors
                               ACE_Get_Opt::PERMUTE_ARGS,                 // ordering
                               0);                                        // long_only
  int option = 0;
  std::stringstream converter;
  while ((option = argument_parser ()) != EOF)
  {
    switch (option)
    {
      case 'c':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argument_parser.opt_arg ();
        converter >> maximumNumberOfConnections_out;
        break;
      }
      case 'g':
      {
        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
        if (opt_arg)
          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIFile_out.clear ();
        break;
      }
      case 'i':
      {
        unsigned int ping_interval = 0;
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argument_parser.opt_arg ();
        converter >> ping_interval;
        pingInterval_out.set (ping_interval / 1000,
                              (ping_interval % 1000) * 1000);
        break;
      }
//      case 'k':
//      {
//        converter.clear();
//        converter.str(ACE_TEXT_ALWAYS_CHAR(""));
//        converter << argument_parser.opt_arg();
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
        networkInterface_out =
          ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
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
        converter << argument_parser.opt_arg ();
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
        converter << argument_parser.opt_arg ();
        converter >> statisticReportingInterval_out;
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
        converter << argument_parser.opt_arg ();
        converter >> numberOfDispatchThreads_out;
        break;
      }
      // error handling
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    ACE_TEXT (argument_parser.last_option ())));
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

//  if (!useReactor_in)
//  {
//    ACE_Proactor* proactor_p = ACE_Proactor::instance ();
//    ACE_ASSERT (proactor_p);
//    ACE_POSIX_Proactor* proactor_impl_p =
//        dynamic_cast<ACE_POSIX_Proactor*> (proactor_p->implementation ());
//    ACE_ASSERT (proactor_impl_p);
//    if (proactor_impl_p->get_impl_type () == ACE_POSIX_Proactor::PROACTOR_SIG)
//      signals_out.sig_del (COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL);
//  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (unsigned int maximumNumberOfConnections_in,
#if defined (SSL_SUPPORT)
         const std::string& certificateFile_in,
         const std::string& privateKeyFile_in,
#endif // SSL_SUPPORT
         const std::string& UIDefinitionFile_in,
         const ACE_Time_Value& pingInterval_in,
         //unsigned int keepAliveTimeout_in,
         const std::string& networkInterface_in,
         bool useLoopBack_in,
         unsigned short listeningPortNumber_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         unsigned int numberOfDispatchThreads_in,
         enum Net_TransportLayerType protocol_in,
         struct Server_Configuration_2& configuration_in,
         struct Server_UI_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Server_SignalHandler_2& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  CBData_in.UIState =
      &const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  ACE_ASSERT (CBData_in.UIState);
  CBData_in.progressData.state = CBData_in.UIState;
#endif // GTK_USE
  CBData_in.configuration = &configuration_in;
  CBData_in.progressData.configuration = &configuration_in;

  int result = -1;
  Test_U_TCPConnectionManager_t* connection_manager_p =
    TEST_U_TCPCONNECTIONMANAGER_SINGLETON::instance ();
  Test_U_UDPConnectionManager_t* connection_manager_2 =
    TEST_U_UDPCONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  ACE_ASSERT (connection_manager_2);

  // step0a: initialize configuration
  Test_U_Server_Asynch_TCP_Stream_2 asynch_tcp_stream;
  Test_U_Server_TCP_Stream_2 tcp_stream;
#if defined (SSL_SUPPORT)
  Test_U_Server_SSL_Stream_2 ssl_stream;
#endif // SSL_SUPPORT
  Test_U_Server_Asynch_UDP_Stream_2 asynch_udp_stream;
  Test_U_Server_UDP_Stream_2 udp_stream;

  Test_U_EventHandler_t ui_event_handler (&CBData_in);
  Test_U_EventHandler_2 ui_event_handler_2 (&CBData_in);
  Test_U_Module_EventHandler_Module event_handler (NULL,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_U_Module_EventHandler_Module event_handler_2 (NULL,
                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_Parser_FlexAllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (configuration_in.allocatorConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize allocator, returning\n")));
    return;
  } // end IF
  Test_U_MessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,         // heap allocator handle
                                               true);                   // block ?

  // ******************** protocol configuration data **************************
  configuration_in.protocolConfiguration.pingInterval = pingInterval_in;
  configuration_in.protocolConfiguration.printPongMessages =
    UIDefinitionFile_in.empty ();
  configuration_in.protocolConfiguration.transportLayer = protocol_in;
  // ********************** stream configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
  struct ClientServer_ModuleHandlerConfiguration modulehandler_configuration; // connection-
  struct ClientServer_ModuleHandlerConfiguration modulehandler_configuration_2; // server tcp|ssl;
  struct ClientServer_ModuleHandlerConfiguration modulehandler_configuration_3; // server udp;
  struct Test_U_StreamConfiguration stream_configuration; // connection io-
  struct Test_U_StreamConfiguration stream_configuration_2; // server tcp|ssl
  struct Test_U_StreamConfiguration stream_configuration_3; // server udp

  Test_U_TCPConnectionConfiguration connection_configuration;
  Test_U_UDPConnectionConfiguration connection_configuration_2;

  stream_configuration.allocatorConfiguration =
      &configuration_in.allocatorConfiguration;
  stream_configuration.cloneModule = !(UIDefinitionFile_in.empty ());
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module =
      (!UIDefinitionFile_in.empty () ? &event_handler
                                     : NULL);
  // *TODO*: is this correct ?
  stream_configuration.serializeOutput =
    (useReactor_in && (numberOfDispatchThreads_in > 1));

  modulehandler_configuration.computeThroughput = true;
  modulehandler_configuration.printFinalReport = true;
  modulehandler_configuration.protocolConfiguration =
    &configuration_in.protocolConfiguration;
  modulehandler_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  modulehandler_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;
  modulehandler_configuration.subscriber = &ui_event_handler;
  modulehandler_configuration.subscribers = &CBData_in.subscribers;
#if defined (GTK_USE) || defined (WXWIDGETS_USE)
  ACE_ASSERT (CBData_in.UIState);
  modulehandler_configuration.lock = &(CBData_in.UIState->subscribersLock);
#endif // GTK_USE || WXWIDGETS_USE
  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);

  modulehandler_configuration_2 = modulehandler_configuration;
  modulehandler_configuration_2.concurrency =
    STREAM_HEADMODULECONCURRENCY_ACTIVE;
  modulehandler_configuration_2.listenerConfiguration =
    &connection_configuration;
  modulehandler_configuration_2.subscriber = &ui_event_handler_2;
  stream_configuration_2 = stream_configuration;
  stream_configuration_2.module = &event_handler_2;
  configuration_in.streamConfiguration_2.initialize (module_configuration,
                                                     modulehandler_configuration_2,
                                                     stream_configuration_2);

  modulehandler_configuration_3 = modulehandler_configuration_2;
  modulehandler_configuration_3.listenerConfiguration =
      &connection_configuration_2;
  stream_configuration_3 = stream_configuration;
  stream_configuration_3.module = &event_handler_2;
  configuration_in.streamConfiguration_3.initialize (module_configuration,
                                                     modulehandler_configuration_3,
                                                     stream_configuration_3);

  // ********************** connection configuration data **********************
  Net_ConnectionConfigurationsIterator_t iterator, iterator_2;
  Test_U_TCPConnectionConfiguration* connection_configuration_p = NULL;
  Test_U_UDPConnectionConfiguration* connection_configuration_p_2 = NULL;
  //configuration_in.listenerConfiguration.messageAllocator =
  //  &message_allocator;
  //configuration_in.listenerConfiguration.connectionConfiguration =
  //  &connection_configuration;

  connection_configuration.allocatorConfiguration =
      &configuration_in.allocatorConfiguration;
  connection_configuration.messageAllocator = &message_allocator;
  connection_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("TCP"),
                                                                    &connection_configuration));
  iterator =
    configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("TCP"));
  ACE_ASSERT (iterator != configuration_in.connectionConfigurations.end ());
  connection_configuration_p =
    static_cast<Test_U_TCPConnectionConfiguration*> ((*iterator).second);
  ACE_ASSERT (connection_configuration_p);
  connection_configuration_2.allocatorConfiguration =
      &configuration_in.allocatorConfiguration;
  connection_configuration_2.messageAllocator = &message_allocator;
  connection_configuration_2.streamConfiguration =
    &configuration_in.streamConfiguration;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("UDP"),
                                                                    &connection_configuration_2));
  iterator_2 =
    configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("UDP"));
  ACE_ASSERT (iterator_2 != configuration_in.connectionConfigurations.end ());
  connection_configuration_p_2 =
    static_cast<Test_U_UDPConnectionConfiguration*> ((*iterator_2).second);
  ACE_ASSERT (connection_configuration_p_2);

  //  config.delete_module = false;
  // *WARNING*: set at runtime, by the appropriate connection handler
  //  config.sessionID = 0; // (== socket handle !)
  //  config.statisticsReportingInterval = 0; // == off
  //	config.printFinalReport = false;
  // ************ runtime data ************
  //	config.currentStatistics = {};
  //	config.lastCollectionTimestamp = ACE_Time_Value::zero;

  // step0b: initialize event dispatch
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
    &configuration_in.dispatchConfiguration;
  if (useReactor_in)
  {
    configuration_in.dispatchConfiguration.dispatch =
        COMMON_EVENT_DISPATCH_REACTOR;
    configuration_in.dispatchConfiguration.numberOfReactorThreads =
      numberOfDispatchThreads_in;
    configuration_in.dispatchConfiguration.reactorType =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ((numberOfDispatchThreads_in > 1) ? COMMON_REACTOR_THREAD_POOL
                                          : COMMON_REACTOR_ACE_DEFAULT);
#else
        ((numberOfDispatchThreads_in > 1) ? COMMON_REACTOR_THREAD_POOL//COMMON_REACTOR_DEV_POLL
                                          : COMMON_REACTOR_ACE_DEFAULT);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  else
  {
    configuration_in.dispatchConfiguration.numberOfReactorThreads =
        1;
    configuration_in.dispatchConfiguration.numberOfProactorThreads =
      numberOfDispatchThreads_in;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    configuration_in.dispatchConfiguration.proactorType =
      COMMON_PROACTOR_POSIX_SIG;
#endif // ACE_WIN32 || ACE_WIN64
  } // end ELSE
  if (!Common_Event_Tools::initializeEventDispatch (configuration_in.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF

  // step1: initialize regular (global) statistics reporting
  Common_Timer_Tools::configuration_.dispatch =
    (useReactor_in ? COMMON_TIMER_DISPATCH_REACTOR : COMMON_TIMER_DISPATCH_PROACTOR);
  Common_Timer_Tools::configuration_.publishSeconds = true;
  Common_Timer_Tools::initialize ();
  Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Net_StreamStatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                                  connection_manager_p,
                                                  false);
  long timer_id = -1;
  if (statisticReportingInterval_in)
  {
    ACE_Time_Value interval (statisticReportingInterval_in, 0);
    timer_id =
      timer_manager_p->schedule_timer (&statistic_handler,   // event handler handle
                                       NULL,                 // asynchronous completion token
                                       ACE_Time_Value::zero, // initial delay
                                       interval);            // interval
    if (unlikely (timer_id == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));
      Common_Timer_Tools::finalize ();
      return;
    } // end IF
  } // end IF

  // step2: signal handling
  configuration_in.asynchTCPStream = &asynch_tcp_stream;
  configuration_in.TCPStream = &tcp_stream;
#if defined (SSL_SUPPORT)
  configuration_in.SSLStream = &ssl_stream;
#endif // SSL_SUPPORT
  configuration_in.asynchUDPStream = &asynch_udp_stream;
  configuration_in.UDPStream = &udp_stream;

  struct Server_SignalHandlerConfiguration_2 signal_handler_configuration;
  signal_handler_configuration.asynchTCPStream = &asynch_tcp_stream;
  signal_handler_configuration.TCPStream = &tcp_stream;
#if defined (SSL_SUPPORT)
  signal_handler_configuration.SSLStream = &ssl_stream;
#endif // SSL_SUPPORT
  signal_handler_configuration.asynchUDPStream = &asynch_udp_stream;
  signal_handler_configuration.UDPStream = &udp_stream;

  signal_handler_configuration.statisticReportingHandler =
    connection_manager_p;
  signal_handler_configuration.statisticReportingTimerId = timer_id;
  signal_handler_configuration.dispatchState =
    &event_dispatch_state_s;
  if (!signalHandler_in.initialize (signal_handler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    Common_Timer_Tools::finalize ();
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
    return;
  } // end IF

  // step3: initialize connection manager
  struct Net_UserData user_data;
  connection_manager_p->initialize (maximumNumberOfConnections_in,
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (*connection_configuration_p,
                             &user_data);
  connection_manager_p->start (NULL);
  connection_manager_2->initialize (maximumNumberOfConnections_in,
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_2->set (*connection_configuration_p_2,
                             &user_data);
  connection_manager_2->start (NULL);

  // step4: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

  // step1a: start UI event loop ?
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (GTK_USE)
    ACE_ASSERT (CBData_in.UIState);
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    CBData_in.UIState->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
#endif // GTK_USE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));

      Common_Timer_Tools::finalize ();
      return;
    } // end IF
    BOOL was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_USE)
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
      return;
    } // end IF
#endif // GTK_USE
  } // end IF

  // step4b: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));

    if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
      gtk_manager_p->stop (true, true);
#else
      ;
#endif // GTK_USE
    Common_Timer_Tools::finalize ();
    return;
  } // end IF

  // step4c: start listening
  if (!useReactor_in)
    connection_configuration_p_2->socketConfiguration.connect = true;
  if (useLoopBack_in)
  {
    result =
      connection_configuration_p_2->socketConfiguration.listenAddress.set (listeningPortNumber_in,
                                                                           INADDR_LOOPBACK,
                                                                           1,
                                                                           0);
    ACE_ASSERT (!result);
    result =
      connection_configuration_p_2->socketConfiguration.peerAddress.set (listeningPortNumber_in,
                                                                         INADDR_LOOPBACK,
                                                                         1,
                                                                         0);
    ACE_ASSERT (!result);
    result =
      connection_configuration_p->socketConfiguration.address.set (listeningPortNumber_in,
                                                                   INADDR_LOOPBACK,
                                                                   1,
                                                                   0);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));

      if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
        gtk_manager_p->stop (true, true);
#else
        ;
#endif // GTK_USE
      Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                 true); // wait ?
      Common_Timer_Tools::finalize ();
      return;
    } // end IF
  } // end IF
  else
  {
    connection_configuration_p_2->socketConfiguration.listenAddress.set_port_number (listeningPortNumber_in,
                                                                                     1);

    ACE_INET_Addr nic_address, gateway_address;
    Net_Common_Tools::interfaceToIPAddress (networkInterface_in,
                                            nic_address,
                                            gateway_address);
    result =
      connection_configuration_p_2->socketConfiguration.peerAddress.set (static_cast<u_short> (listeningPortNumber_in + 1),
                                                                         nic_address.get_ip_address (),
                                                                         1,
                                                                         0);
    ACE_ASSERT (result != -1);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("set peer address to %s\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (connection_configuration_p_2->socketConfiguration.peerAddress, false, false).c_str ())));

    connection_configuration_p->socketConfiguration.address.set_port_number (listeningPortNumber_in,
                                                                             1);
  } // end ELSE

//   Server_Asynch_TCP_Listener_t* asynch_tcp_listener_p =
//     SERVER_ASYNCH_TCP_LISTENER_SINGLETON::instance ();
//   Server_TCP_Listener_t* tcp_listener_p =
//     SERVER_TCP_LISTENER_SINGLETON::instance ();
// #if defined (SSL_SUPPORT)
//   Server_SSL_Listener_t* ssl_listener_p =
//     SERVER_SSL_LISTENER_SINGLETON::instance ();
// #endif // SSL_SUPPORT
//   ACE_ASSERT (asynch_tcp_listener_p);
//   if (!asynch_tcp_listener_p->initialize (*connection_configuration_p))
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to initialize asynchronous TCP listener, returning\n")));

//     Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
//                                                true); // wait ?

//     if (!UIDefinitionFile_in.empty ())
// #if defined (GTK_USE)
//       gtk_manager_p->stop (true, true);
// #else
//       ;
// #endif // GTK_USE
//     Common_Timer_Tools::finalize ();
//     return;
//   } // end IF
//   ACE_ASSERT (tcp_listener_p);
//   if (!tcp_listener_p->initialize (*connection_configuration_p))
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to initialize TCP listener, returning\n")));

//     Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
//                                                true); // wait ?

//     if (!UIDefinitionFile_in.empty ())
// #if defined (GTK_USE)
//       gtk_manager_p->stop (true, true);
// #else
//       ;
// #endif // GTK_USE
//     Common_Timer_Tools::finalize ();
//     return;
//   } // end IF
// #if defined (SSL_SUPPORT)
//   ACE_ASSERT (ssl_listener_p);
//   if (!ssl_listener_p->initialize (*connection_configuration_p))
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to initialize SSL listener, returning\n")));

//     Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
//                                                true); // wait ?
//     if (!UIDefinitionFile_in.empty ())
// #if defined (GTK_USE)
//       gtk_manager_p->stop (true, true);
// #else
//       ;
// #endif // GTK_USE
//     Common_Timer_Tools::finalize ();
//     return;
//   } // end IF
// #endif // SSL_SUPPORT

#if defined (SSL_SUPPORT)
  std::string module_name =
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_PARENT_SUBDIRECTORY);
  std::string filename_string =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                      module_name,
                                                      false); // data
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (NET_PROTOCOL_DEFAULT_SSL_TRUSTED_CAS_FILENAME_STRING);

  if (unlikely (!Net_Common_Tools::initializeSSLContext (certificateFile_in,
                                                         privateKeyFile_in,
                                                         filename_string,    // trusted CAs
                                                         false,              // server mode
                                                         NULL)))             // default context
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::initializeSSLContext(\"%s\",\"%s\",NULL), returning\n"),
                ACE_TEXT (certificateFile_in.c_str ()),
                ACE_TEXT (privateKeyFile_in.c_str ())));

    Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                               true); // wait ?
    if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
      gtk_manager_p->stop (true, true);
#else
      ;
#endif // GTK_USE
    Common_Timer_Tools::finalize ();
    return;
  } // end IF
#endif // SSL_SUPPORT

//   Server_UDP_AsynchConnector_t udp_asynch_connector (true);
//   if (!udp_asynch_connector.initialize (*connection_configuration_p_2))
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to initialize asynchronous UDP connector, returning\n")));

//     Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
//                                                true); // wait ?
//     if (!UIDefinitionFile_in.empty ())
// #if defined (GTK_USE)
//       gtk_manager_p->stop (true, true);
// #else
//       ;
// #endif // GTK_USE
//     Common_Timer_Tools::finalize ();
//     return;
//   } // end IF
//   Server_UDP_Connector_t udp_connector (true);
//   if (!udp_connector.initialize (*connection_configuration_p_2))
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("failed to initialize UDP connector, returning\n")));

//     Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
//                                                true); // wait ?
//     if (!UIDefinitionFile_in.empty ())
// #if defined (GTK_USE)
//       gtk_manager_p->stop (true, true);
// #else
//       ;
// #endif // GTK_USE
//     Common_Timer_Tools::finalize ();
//     return;
//   } // end IF

  // start stream(s) processing
  asynch_tcp_stream.initialize (configuration_in.streamConfiguration_2);
  tcp_stream.initialize (configuration_in.streamConfiguration_2);
#if defined (SSL_SUPPORT)
  ssl_stream.initialize (configuration_in.streamConfiguration_2);
#endif // SSL_SUPPORT
  asynch_udp_stream.initialize (configuration_in.streamConfiguration_3);
  udp_stream.initialize (configuration_in.streamConfiguration_3);
  if (useReactor_in)
  {
    switch (protocol_in)
    {
      case NET_TRANSPORTLAYER_TCP:
        tcp_stream.start ();
        break;
      case NET_TRANSPORTLAYER_UDP:
        udp_stream.start ();
        break;
      case NET_TRANSPORTLAYER_SSL:
#if defined (SSL_SUPPORT)
        ssl_stream.start ();
#endif // SSL_SUPPORT
        break;
      default:
        break;
    } // end SWITCH
  } // end IF
  else
  {
    switch (protocol_in)
    {
      case NET_TRANSPORTLAYER_TCP:
        asynch_tcp_stream.start ();
        break;
      case NET_TRANSPORTLAYER_UDP:
        asynch_udp_stream.start ();
        break;
      case NET_TRANSPORTLAYER_SSL:
#if defined (SSL_SUPPORT)
        //asynch_ssl_stream.start ();
#endif // SSL_SUPPORT
        break;
      default:
        break;
    } // end SWITCH
  } // end ELSE

  // *NOTE*: from this point on, clean up any remote connections !

  Common_Event_Tools::dispatchEvents (event_dispatch_state_s);

  // clean up
  // *NOTE*: listener has stopped, interval timer has been cancelled,
  // and connections have been aborted...
  if (!UIDefinitionFile_in.empty ())
#if defined (GTK_USE)
    gtk_manager_p->stop (true,   // wait for completion ?
                         false); // N/A
#else
    ;
#endif // GTK_USE
  Common_Timer_Tools::finalize ();

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
#if defined (HAVE_CONFIG_H)
            << ACE_TEXT (ACENetwork_PACKAGE_NAME)
            << ACE_TEXT (": ")
            << ACE_TEXT (ACENetwork_PACKAGE_VERSION)
            << std::endl
#endif // HAVE_CONFIG_H
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result = -1;
#endif // ACE_WIN32 || ACE_WIN64

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
#endif // ACE_WIN32 || ACE_WIN64

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  // initialize randomness
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (false,  // COM ?
                            true);  // RNG ?
#else
  Common_Tools::initialize (true); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64
  Common_File_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (argv_in[0]));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  unsigned int maximum_number_of_connections =
    NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
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
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string UI_file_path = path;
  UI_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file_path += ACE_TEXT_ALWAYS_CHAR (NET_SERVER_2_UI_FILE);
  ACE_Time_Value ping_interval (NET_SERVER_DEFAULT_CLIENT_PING_INTERVAL_MS / 1000,
                                (NET_SERVER_DEFAULT_CLIENT_PING_INTERVAL_MS % 1000) * 1000);
  //  unsigned int keep_alive_timeout = NET_SERVER_DEFAULT_TCP_KEEPALIVE;
  bool log_to_file = false;
  std::string network_interface =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
  bool use_loopback = NET_INTERFACE_DEFAULT_USE_LOOPBACK;
  unsigned short listening_port_number = NET_SERVER_DEFAULT_LISTENING_PORT;
  bool use_reactor =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval =
    NET_SERVER_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  enum Net_TransportLayerType protocol_e = NET_TRANSPORTLAYER_TCP;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    NET_SERVER_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            maximum_number_of_connections,
#if defined (SSL_SUPPORT)
                            certificate_file,
                            private_key_file,
#endif // SSL_SUPPORT
                            UI_file_path,
                            ping_interval,
                            //keep_alive_timeout,
                            log_to_file,
                            network_interface,
                            use_loopback,
                            listening_port_number,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            protocol_e,
                            print_version_and_exit,
                            number_of_dispatch_threads))
  {
    do_printUsage (ACE::basename (argv_in[0]));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1c: validate arguments
  // *NOTE*: probably requires CAP_NET_BIND_SERVICE
  if (listening_port_number <= 1023)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("using (privileged) port #: %d\n"),
                listening_port_number));
  //if (use_reactor && (number_of_dispatch_threads > 1))
  //  use_thread_pool = true;

  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (NET_STREAM_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could lead to deadlocks...\n")));
  if ((!UI_file_path.empty () && !Common_File_Tools::isReadable (UI_file_path)) ||
      //(use_thread_pool && !use_reactor)                                         ||
      //(use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool)     ||
      (!use_reactor && (protocol_e == NET_TRANSPORTLAYER_SSL)))
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
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  if (number_of_dispatch_threads == 0)
    number_of_dispatch_threads = 1;

  // step1d: initialize logging and/or tracing
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  Common_Logger_Queue_t logger;
  logger.initialize (&state_r.logQueue,
                     &state_r.logQueueLock);
#endif // GTK_USE
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Net_Server_Common_Tools::getNextLogFileName (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                   Common_File_Tools::basename (ACE_TEXT_ALWAYS_CHAR (argv_in[0]), true));
  if (!Common_Log_Tools::initialize (ACE::basename (argv_in[0]),    // program name
                                     log_file_name,                 // log file name
                                     true,                          // log to syslog ?
                                     false,                         // trace messages ?
                                     trace_information,             // debug messages ?
#if defined (GTK_USE)
                                     (UI_file_path.empty () ? NULL
                                                            : &logger))) // (ui) logger ?
#elif defined (WXWIDGETS_USE)
                                     NULL))                              // (ui) logger ?
#else
                                     NULL))                              // (ui) logger ?
#endif // XXX_USE
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));

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

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_SUCCESS;
  } // end IF

  // step1h: initialize UI framework
  struct Server_Configuration_2 configuration;
  struct Server_UI_CBData ui_cb_data;
#if defined (GTK_USE)
  ui_cb_data.allowUserRuntimeStatistic =
    (statistic_reporting_interval == 0); // handle SIGUSR1/SIGBREAK
                                         // iff regular reporting
                                         // is off
  //ui_cb_data.progressData.state = &ui_cb_data;

    // step1h: init GLIB / G(D|T)K[+] / GNOME ?
  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  configuration.GTKConfiguration.argc = argc_in;
  configuration.GTKConfiguration.argv = argv_in;
  configuration.GTKConfiguration.CBData = &ui_cb_data;
  configuration.GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  configuration.GTKConfiguration.eventHooks.initHook =
      idle_initialize_server_UI_cb;
  configuration.GTKConfiguration.definition = &gtk_ui_definition;
//  if (!gtk_rc_file.empty ())
//    ui_cb_data.configuration->GTKConfiguration.RCFiles.push_back (gtk_rc_file);
  if (!UI_file_path.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (configuration.GTKConfiguration);
#endif // GTK_USE

  // step1e: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (use_reactor,
                        (statistic_reporting_interval == 0), // handle SIGUSR1/SIGBREAK
                                                             // iff regular reporting
                                                             // is off
                        signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
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

    Common_Log_Tools::finalize ();
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
//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
  Server_SignalHandler_2 signal_handler;

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
  if (!Common_OS_Tools::setResourceLimits (use_fd_based_reactor,       // file descriptors
                                           stack_traces,               // stack traces
                                           use_signal_based_proactor)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_OS_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
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

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (maximum_number_of_connections,
#if defined (SSL_SUPPORT)
           certificate_file,
           private_key_file,
#endif // SSL_SUPPORT
           UI_file_path,
           ping_interval,
           //keep_alive_timeout,
           network_interface,
           use_loopback,
           listening_port_number,
           use_reactor,
           statistic_reporting_interval,
           number_of_dispatch_threads,
           protocol_e,
           configuration,
           ui_cb_data,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
  timer.stop ();

  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"\n"),
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
    Common_Log_Tools::finalize ();
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
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (ACE_Profile_Timer::Rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);
  std::string user_time_string;
  std::string system_time_string;
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
#endif // ACE_WIN32 || ACE_WIN64

  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);

  Common_Log_Tools::finalize ();
  Common_Tools::finalize ();

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
    return EXIT_FAILURE;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
