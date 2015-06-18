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
//#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "ace/streams.h"
#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif
#include "ace/Log_Msg.h"
#include "ace/Proactor.h"
#include "ace/Profile_Timer.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/POSIX_Proactor.h"
#endif
#include "ace/Reactor.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#ifdef LIBACENETWORK_ENABLE_VALGRIND_SUPPORT
#include "valgrind/valgrind.h"
#endif

#include "common_file_tools.h"
#include "common_logger.h"
#include "common_tools.h"

#include "common_ui_defines.h"
//#include "common_ui_glade_definition.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_allocatorheap.h"

#include "net_common_tools.h"
#include "net_configuration.h"
#include "net_connection_manager_common.h"
#include "net_macros.h"

#include "net_server_common.h"
#include "net_server_common_tools.h"
#include "net_server_listener_common.h"

#ifdef HAVE_CONFIG_H
#include "libacenetwork_config.h"
#endif

#include "net_callbacks.h"
#include "net_common.h"
#include "net_defines.h"
#include "net_eventhandler.h"
#include "net_module_eventhandler.h"

#include "net_server_defines.h"
#include "net_server_signalhandler.h"

// globals
unsigned int random_seed;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
struct random_data random_data;
char random_state_buffer[BUFSIZ];
#endif

void
do_printUsage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("src");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT ("usage: ")
            << programName_in
            << ACE_TEXT (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT ("currently available options:") << std::endl;
  std::cout << ACE_TEXT ("-c [VALUE]   : max #connections ([")
            << NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS
            << ACE_TEXT ("])")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (NET_SERVER_UI_FILE);
  std::cout << ACE_TEXT ("-g[[STRING]] : UI file [\"")
            << path
            << ACE_TEXT ("\"] {\"\" --> no GUI}")
            << std::endl;
  std::cout << ACE_TEXT ("-h           : use thread-pool [")
            << NET_EVENT_USE_THREAD_POOL
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-i [VALUE]   : client ping interval (ms) [")
            << NET_SERVER_DEFAULT_CLIENT_PING_INTERVAL
            << ACE_TEXT ("] {0 --> OFF})")
            << std::endl;
//  std::cout << ACE_TEXT("-k [VALUE]  : client keep-alive timeout ([")
//            << NET_SERVER_DEF_CLIENT_KEEPALIVE
//            << ACE_TEXT("] second(s) {0 --> no timeout})")
//            << std::endl;
  std::cout << ACE_TEXT ("-l           : log to a file [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-m           : receive uni/multi/broadcast UDP [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-n [STRING]  : network interface [\"")
            << ACE_TEXT (NET_INTERFACE_DEFAULT)
            << ACE_TEXT ("\"]")
            << std::endl;
  // *TODO*: this doesn't really make sense (yet)
  std::cout << ACE_TEXT ("-o           : use loopback [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-p [VALUE]   : listening port [")
            << NET_SERVER_DEFAULT_LISTENING_PORT
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-r           : use reactor [")
            << NET_EVENT_USE_REACTOR
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-s [VALUE]   : statistics reporting interval (second(s)) [")
            << NET_SERVER_DEFAULT_STATISTICS_REPORTING_INTERVAL
            << ACE_TEXT ("] {0 --> OFF})")
            << std::endl;
  std::cout << ACE_TEXT ("-t           : trace information") << std::endl;
  std::cout << ACE_TEXT ("-v           : print version information and exit")
            << std::endl;
  std::cout << ACE_TEXT ("-x [VALUE]   : #dispatch threads [")
            << NET_SERVER_DEFAULT_NUMBER_OF_DISPATCHING_THREADS
            << ACE_TEXT ("]")
            << std::endl;
}

bool
do_processArguments (const int& argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& maxNumConnections_out,
                     std::string& UIFile_out,
                     bool& useThreadPool_out,
                     unsigned int& clientPingInterval_out,
                     //unsigned int& keepAliveTimeout_out,
                     bool& logToFile_out,
                     bool& useUDP_out,
                     std::string& networkInterface_out,
                     bool& useLoopback_out,
                     unsigned short& listeningPortNumber_out,
                     bool& useReactor_out,
                     unsigned int& statisticsReportingInterval_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numDispatchThreads_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("src");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  maxNumConnections_out =
    NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (NET_SERVER_UI_FILE);
  UIFile_out = path;
  useThreadPool_out = NET_EVENT_USE_THREAD_POOL;
  clientPingInterval_out =
    NET_SERVER_DEFAULT_CLIENT_PING_INTERVAL;
//  keepAliveTimeout_out = NET_SERVER_DEF_CLIENT_KEEPALIVE;
  logToFile_out = false;
  useUDP_out = false;
  networkInterface_out =
    ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT);
  useLoopback_out = false;
  listeningPortNumber_out = NET_SERVER_DEFAULT_LISTENING_PORT;
  useReactor_out = NET_EVENT_USE_REACTOR;
  statisticsReportingInterval_out =
      NET_SERVER_DEFAULT_STATISTICS_REPORTING_INTERVAL;
  traceInformation_out = false;
  printVersionAndExit_out = false;
  numDispatchThreads_out =
    NET_SERVER_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("c:g::hi:k:lmn:op:rs:tvx:"));

  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'c':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> maxNumConnections_out;
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
      case 'h':
      {
        useThreadPool_out = true;
        break;
      }
      case 'i':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> clientPingInterval_out;
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
      case 'm':
      {
        useUDP_out = true;
        break;
      }
      case 'n':
      {
        networkInterface_out =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'o':
      {
        useLoopback_out = true;
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
        converter >> statisticsReportingInterval_out;
        break;
      }
      case 't':
      {
        traceInformation_out = true;
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
do_work (unsigned int maxNumConnections_in,
         const std::string& UIDefinitionFile_in,
         bool useThreadPool_in,
         unsigned int pingInterval_in,
         //unsigned int keepAliveTimeout_in,
         bool useUDP_in,
         const std::string& networkInterface_in,
         bool useLoopback_in,
         unsigned short listeningPortNumber_in,
         bool useReactor_in,
         unsigned int statisticsReportingInterval_in,
         unsigned int numDispatchThreads_in,
         Net_GTK_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Net_Server_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

  // step0a: initialize stream configuration object
  Stream_ModuleConfiguration module_configuration;
  ACE_OS::memset (&module_configuration, 0, sizeof (module_configuration));

  Net_EventHandler ui_event_handler (&CBData_in);
  Net_Module_EventHandler_Module event_handler (ACE_TEXT_ALWAYS_CHAR ("EventHandler"),
                                                NULL);
  Net_Module_EventHandler* event_handler_p =
    dynamic_cast<Net_Module_EventHandler*> (event_handler.writer ());
  if (!event_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_EventHandler> failed, returning\n")));
    return;
  } // end IF
  event_handler_p->initialize (&CBData_in.subscribers,
                               &CBData_in.subscribersLock);
  event_handler_p->subscribe (&ui_event_handler);

  Stream_AllocatorHeap heap_allocator;
  Net_StreamMessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES, // maximum #buffers
                                                  &heap_allocator,         // heap allocator handle
                                                  true);                   // block ?

  Net_Configuration configuration;
  // ********************** socket configuration data **************************
  configuration.socketConfiguration.bufferSize =
    NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE;
  configuration.socketConfiguration.linger =
      NET_SERVER_SOCKET_DEFAULT_LINGER;

  // ********************** stream configuration data **************************
  configuration.streamConfiguration.protocolConfiguration =
    &configuration.protocolConfiguration;
  configuration.streamConfiguration.streamConfiguration.bufferSize =
    NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
  configuration.streamConfiguration.streamConfiguration.deleteModule = false;
  configuration.streamConfiguration.streamConfiguration.messageAllocator =
    &message_allocator;
  configuration.streamConfiguration.streamConfiguration.module =
    (!UIDefinitionFile_in.empty () ? &event_handler
                                   : NULL);
  configuration.streamConfiguration.streamConfiguration.moduleConfiguration =
    &module_configuration;
  configuration.streamConfiguration.streamConfiguration.printFinalReport =
    false;
  configuration.streamConfiguration.streamConfiguration.statisticReportingInterval =
    statisticsReportingInterval_in;
  configuration.streamConfiguration.streamConfiguration.useThreadPerConnection =
    false;
  configuration.streamConfiguration.userData = &configuration.streamSessionData;

  // ******************** protocol configuration data **************************
  configuration.protocolConfiguration.bufferSize =
    NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
  configuration.protocolConfiguration.peerPingInterval = pingInterval_in;
  configuration.protocolConfiguration.pingAutoAnswer = true;
  configuration.protocolConfiguration.printPongMessages = true;

  //  config.delete_module = false;
  // *WARNING*: set at runtime, by the appropriate connection handler
  //  config.sessionID = 0; // (== socket handle !)
  //  config.statisticsReportingInterval = 0; // == off
  //	config.printFinalReport = false;
  // ************ runtime data ************
  //	config.currentStatistics = {};
  //	config.lastCollectionTimestamp = ACE_Time_Value::zero;

  // step0b: initialize event dispatch
  if (!Common_Tools::initializeEventDispatch (useReactor_in,
                                              useThreadPool_in,
                                              configuration.streamConfiguration.streamConfiguration.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF

  // step1: initialize regular (global) statistics reporting
  Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start ();
  Stream_StatisticHandler_Reactor_t statistics_handler (ACTION_REPORT,
                                                        NET_CONNECTIONMANAGER_SINGLETON::instance (),
                                                        false);
  long timer_id = -1;
  if (statisticsReportingInterval_in)
  {
    ACE_Event_Handler* handler_p = &statistics_handler;
    ACE_Time_Value interval (statisticsReportingInterval_in,
                             0);
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

      return;
    } // end IF
  } // end IF

  // step2: signal handling
  if (useReactor_in)
    CBData_in.listenerHandle = NET_SERVER_LISTENER_SINGLETON::instance ();
  else
    CBData_in.listenerHandle =
      NET_SERVER_ASYNCHLISTENER_SINGLETON::instance ();
  Net_Server_SignalHandlerConfiguration_t signal_handler_configuration;
  ACE_OS::memset (&signal_handler_configuration,
                  0,
                  sizeof (signal_handler_configuration));
  signal_handler_configuration.listener = CBData_in.listenerHandle;
  signal_handler_configuration.statisticReportingHandler =
      NET_CONNECTIONMANAGER_SINGLETON::instance ();
  signal_handler_configuration.statisticReportingTimerID = timer_id;
  signalHandler_in.initialize (signal_handler_configuration);
  if (!Common_Tools::initializeSignals (signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeSignals(), returning\n")));

    // clean up
    timer_manager_p->stop ();

    return;
  } // end IF

  // step3: initialize connection manager
  NET_CONNECTIONMANAGER_SINGLETON::instance ()->initialize (maxNumConnections_in);
  Net_StreamUserData session_data;
  NET_CONNECTIONMANAGER_SINGLETON::instance ()->set (configuration,
                                                     &session_data);

  // step4: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

  // step4a: start GTK event loop ?
  if (!UIDefinitionFile_in.empty ())
  {
    CBData_in.GTKState.finalizationHook = idle_finalize_UI_cb;
    CBData_in.GTKState.initializationHook = idle_initialize_server_UI_cb;
    //CBData_in.GTKState.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    CBData_in.GTKState.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
    CBData_in.GTKState.userData = &CBData_in;

    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->start ();
    if (!COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));

      // clean up
      timer_manager_p->stop ();

      return;
    } // end IF
  } // end IF

  // step4b: initialize worker(s)
  int group_id = -1;
  // *NOTE*: this variable needs to stay on the working stack, it's passed to
  //         the worker(s) (if any)
  bool use_reactor = useReactor_in;
  if (useThreadPool_in &&
      (numDispatchThreads_in > 1))
  {
    if (!Common_Tools::startEventDispatch (use_reactor,
                                           numDispatchThreads_in,
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
      if (!UIDefinitionFile_in.empty ())
        COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();
      timer_manager_p->stop ();

      return;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("started event dispatch...\n")));
  } // end IF

  // step4c: start listening
  Net_SocketHandlerConfiguration socket_handler_configuration;
  //ACE_OS::memset (&socket_handler_configuration,
  //                0,
  //                sizeof (socket_handler_configuration));
  socket_handler_configuration.bufferSize =
    NET_STREAM_MESSAGE_DATA_BUFFER_SIZE;
  socket_handler_configuration.messageAllocator = &message_allocator;
  socket_handler_configuration.socketConfiguration =
      configuration.socketConfiguration;
  Net_Server_ListenerConfiguration_t listener_configuration;
  //ACE_OS::memset (&listener_configuration,
  //                0,
  //                sizeof (listener_configuration));
  listener_configuration.addressFamily = ACE_ADDRESS_FAMILY_INET;
  listener_configuration.allocator = &message_allocator;
  listener_configuration.connectionManager =
    NET_CONNECTIONMANAGER_SINGLETON::instance ();
  listener_configuration.portNumber = listeningPortNumber_in;
  listener_configuration.socketHandlerConfiguration =
    &socket_handler_configuration;
  listener_configuration.statisticCollectionInterval =
    statisticsReportingInterval_in;
  listener_configuration.useLoopbackDevice = useLoopback_in;
  if (!CBData_in.listenerHandle->initialize (listener_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize listener, returning\n")));

    // clean up
    if (useThreadPool_in &&
        (numDispatchThreads_in > 1))
      Common_Tools::finalizeEventDispatch (useReactor_in,
                                           !useReactor_in,
                                           group_id);
    //		{ // synch access
    //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

    //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
    //					 iterator != CBData_in.event_source_ids.end();
    //					 iterator++)
    //				g_source_remove(*iterator);
    //		} // end lock scope
    if (!UIDefinitionFile_in.empty ())
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();
    timer_manager_p->stop ();

    return;
  } // end IF
  CBData_in.listenerHandle->start ();
  if (!CBData_in.listenerHandle->isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start listener (port: %u), returning\n"),
                listeningPortNumber_in));

    // clean up
    if (useThreadPool_in &&
        (numDispatchThreads_in > 1))
      Common_Tools::finalizeEventDispatch (useReactor_in,
                                           !useReactor_in,
                                           group_id);
    //		{ // synch access
    //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

    //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
    //					 iterator != CBData_in.event_source_ids.end();
    //					 iterator++)
    //				g_source_remove(*iterator);
    //		} // end lock scope
    if (!UIDefinitionFile_in.empty ())
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();
    timer_manager_p->stop ();

    return;
  } // end IF

  // *NOTE*: from this point on, clean up any remote connections !

  // *NOTE*: when using a thread pool, handle things differently...
  if (useThreadPool_in &&
      (numDispatchThreads_in > 1))
  {
    result = ACE_Thread_Manager::instance ()->wait_grp (group_id);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", continuing\n"),
                  group_id));
  } // end IF
  else
  {
    if (useReactor_in)
    {
      ACE_Reactor* reactor_p = ACE_Reactor::instance ();
      ACE_ASSERT (reactor_p);
      result = reactor_p->run_reactor_event_loop (0);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to handle events: \"%m\", aborting\n")));
    } // end IF
    else
    {
      ACE_Proactor* proactor_p = ACE_Proactor::instance ();
      ACE_ASSERT (proactor_p);
//      // *NOTE*: unblock [SIGRTMIN,SIGRTMAX] IFF on POSIX AND using the
//      // ACE_POSIX_SIG_Proactor (the default)
//#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
//      ACE_POSIX_Proactor* proactor_impl_p =
//          dynamic_cast<ACE_POSIX_Proactor*> (proactor_p->implementation ());
//      ACE_ASSERT (proactor_impl_p);
//      ACE_POSIX_Proactor::Proactor_Type proactor_type =
//          proactor_impl_p->get_impl_type ();
//      sigset_t original_mask;
//      if (!useReactor_in &&
//          (proactor_type == ACE_POSIX_Proactor::PROACTOR_SIG))
//        Common_Tools::unblockRealtimeSignals (original_mask);
//#endif
      result = proactor_p->proactor_run_event_loop (0);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to handle events: \"%m\", aborting\n")));
//#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
//      // reset signal mask
//      result = ACE_OS::thr_sigsetmask (SIG_SETMASK,
//                                       &original_mask,
//                                       NULL);
//#endif
    } // end ELSE
  } // end ELSE

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished event dispatch...\n")));

  // clean up
  // *NOTE*: listener has stopped, interval timer has been cancelled,
  // and connections have been aborted...
  //		{ // synch access
  //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //					 iterator != CBData_in.event_source_ids.end();
  //					 iterator++)
  //				g_source_remove(*iterator);
  //		} // end lock scope
  if (!UIDefinitionFile_in.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();
  timer_manager_p->stop ();

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
            << ACE_TEXT (LIBACENETWORK_PACKAGE)
            << ACE_TEXT (": ")
            << ACE_TEXT (LIBACENETWORK_PACKAGE_VERSION)
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
//// *PORTABILITY*: on Windows, initialize ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  result = ACE::init ();
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
//    return EXIT_FAILURE;
//  } // end IF
//#endif

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
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  ACE_OS::memset (random_state_buffer, 0, sizeof (random_state_buffer));
  result = ::initstate_r (random_seed,
                          random_state_buffer, sizeof (random_state_buffer),
                          &random_data);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initstate_r(): \"%s\", aborting\n")));

    //    // *PORTABILITY*: on Windows, fini ACE...
    //#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //    if (ACE::fini () == -1)
    //      ACE_DEBUG ((LM_ERROR,
    //                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
    //#endif

    return EXIT_FAILURE;
  } // end IF
  result = ::srandom_r (random_seed, &random_data);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize random seed: \"%s\", aborting\n")));

    //    // *PORTABILITY*: on Windows, fini ACE...
    //#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //    if (ACE::fini () == -1)
    //      ACE_DEBUG ((LM_ERROR,
    //                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
    //#endif

    return EXIT_FAILURE;
  } // end IF
#else
  ACE_OS::srand (static_cast<u_int> (random_seed));
#endif
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("initializing random seed...DONE\n")));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("src");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  unsigned int max_num_connections =
    NET_SERVER_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (NET_SERVER_UI_FILE);
  std::string UI_file = path;
  bool use_threadpool = NET_EVENT_USE_THREAD_POOL;
  unsigned int ping_interval = NET_SERVER_DEFAULT_CLIENT_PING_INTERVAL;
  //  unsigned int keep_alive_timeout = NET_SERVER_DEFAULT_TCP_KEEPALIVE;
  bool log_to_file = false;
  bool use_udp = false;
  std::string network_interface =
    ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT);
  bool use_loopback = false;
  unsigned short listening_port_number = NET_SERVER_DEFAULT_LISTENING_PORT;
  bool use_reactor = NET_EVENT_USE_REACTOR;
  unsigned int statistics_reporting_interval =
    NET_SERVER_DEFAULT_STATISTICS_REPORTING_INTERVAL;
  bool trace_information = false;
  bool print_version_and_exit = false;
  unsigned int num_dispatch_threads =
    NET_SERVER_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            max_num_connections,
                            UI_file,
                            use_threadpool,
                            ping_interval,
                            //keep_alive_timeout,
                            log_to_file,
                            use_udp,
                            network_interface,
                            use_loopback,
                            listening_port_number,
                            use_reactor,
                            statistics_reporting_interval,
                            trace_information,
                            print_version_and_exit,
                            num_dispatch_threads))
  {
    // make 'em learn...
    do_printUsage (ACE::basename (argv_in[0]));

//    // *PORTABILITY*: on Windows, need to fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

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
  if ((!UI_file.empty () && !Common_File_Tools::isReadable (UI_file)) ||
      (use_threadpool && !use_reactor))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid argument, aborting\n")));

    // make 'em learn...
    do_printUsage (ACE::basename (argv_in[0]));

//    // *PORTABILITY*: on Windows, need to fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF
  if (num_dispatch_threads == 0)
    num_dispatch_threads = 1;

  Net_GTK_CBData gtk_cb_user_data;
  gtk_cb_user_data.allowUserRuntimeStatistic =
    (statistics_reporting_interval == 0); // handle SIGUSR1/SIGBREAK
                                          // iff regular reporting
                                          // is off

  // step1d: initialize logging and/or tracing
  Common_Logger logger (&gtk_cb_user_data.logStack,
                        &gtk_cb_user_data.stackLock);
  std::string log_file;
  if (log_to_file &&
      !Net_Server_Common_Tools::getNextLogFilename (Common_File_Tools::getDumpDirectory (),
                                                    log_file))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::getNextLogFilename(), aborting\n")));

    //    // *PORTABILITY*: on Windows, need to fini ACE...
    //#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //    result = ACE::fini ();
    //    if (result == -1)
    //      ACE_DEBUG ((LM_ERROR,
    //                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
    //#endif

    return EXIT_FAILURE;
  } // end IF
  if (!Common_Tools::initializeLogging (ACE::basename (argv_in[0]),     // program name
                                        log_file,                      // logfile
                                        true,                          // log to syslog ?
                                        false,                         // trace messages ?
                                        trace_information,             // debug messages ?
                                        (UI_file.empty () ? NULL
                                                          : &logger))) // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeLogging(), aborting\n")));

    //    // *PORTABILITY*: on Windows, need to fini ACE...
    //#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //    result = ACE::fini ();
    //    if (result == -1)
    //      ACE_DEBUG ((LM_ERROR,
    //                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
    //#endif

    return EXIT_FAILURE;
  } // end IF

  // step1e: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (use_reactor,
                        gtk_cb_user_data.allowUserRuntimeStatistic, // handle SIGUSR1/SIGBREAK
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

    Common_Tools::finalizeLogging ();
    //    // *PORTABILITY*: on Windows, need to fini ACE...
    //#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //    result = ACE::fini ();
    //    if (result == -1)
    //      ACE_DEBUG ((LM_ERROR,
    //                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
    //#endif

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
//    // *PORTABILITY*: on Windows, need to fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF
  Net_Server_SignalHandler signal_handler (use_reactor);

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, need to fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_SUCCESS;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  // *TODO*: the reasoning here is incomplete
  bool use_fd_based_reactor = use_reactor;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  use_fd_based_reactor = (use_reactor && !COMMON_EVENT_WINXX_USE_WFMO_REACTOR);
#endif
  bool stack_traces = true;
  bool use_signal_based_proactor = !use_reactor;
  if (!Common_Tools::setResourceLimits (use_fd_based_reactor,       // file descriptors
                                        stack_traces,               // stack traces
                                        use_signal_based_proactor)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
//    // *PORTABILITY*: on Windows, need to fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF

  // step1h: init GLIB / G(D|T)K[+] / GNOME ?
  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  Common_UI_GtkBuilderDefinition ui_definition (argc_in,
                                                argv_in);
  if (!UI_file.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (argc_in,
                                                              argv_in,
                                                              &gtk_cb_user_data.GTKState,
                                                              &ui_definition);

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (max_num_connections,
           UI_file,
           use_threadpool,
           ping_interval,
           //keep_alive_timeout,
           use_udp,
           network_interface,
           use_loopback,
           listening_port_number,
           use_reactor,
           statistics_reporting_interval,
           num_dispatch_threads,
           gtk_cb_user_data,
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
//    // *PORTABILITY*: on Windows, need to fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (ACE_Profile_Timer::Rusage));
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

  Common_Tools::finalizeSignals (signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Tools::finalizeLogging ();
  //// *PORTABILITY*: on Windows, must fini ACE...
  //#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //  result = ACE::fini ();
  //  if (result == -1)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to ACE::fini(): \"%m\", aborting\n")));
  //    return EXIT_FAILURE;
  //  } // end IF
  //#endif

  return EXIT_SUCCESS;
} // end main
