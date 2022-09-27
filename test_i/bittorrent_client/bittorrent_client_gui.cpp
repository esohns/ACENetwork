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
#include <map>
#include <sstream>
#include <string>

#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#include "ace/iosfwd.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/POSIX_Proactor.h"
#include "ace/Synch.h"
#include "ace/Proactor.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_event_tools.h"

#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (GTK_USE)
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_cachedallocatorheap.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_defines.h"

//#include "http_scanner.h"

#include "bittorrent_common.h"
#include "bittorrent_defines.h"

#include "test_i_defines.h"

#include "ace/Synch.h"
#include "bittorrent_client_configuration.h"
#include "bittorrent_client_defines.h"
#if defined (GTK_USE)
#include "bittorrent_client_gui_callbacks.h"
#endif // GTK_USE
#include "bittorrent_client_gui_common.h"
#include "bittorrent_client_gui_defines.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_session_common.h"
#include "bittorrent_client_signalhandler.h"
#include "bittorrent_client_stream_common.h"
#include "bittorrent_client_tools.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("BitTorrentStream");

void
do_printUsage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d              : debug [")
            << (COMMON_PARSER_DEFAULT_LEX_TRACE ||
                COMMON_PARSER_DEFAULT_YACC_TRACE)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (BITTORRENT_CLIENT_DEFAULT_TORRENT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [FILENAME]   : .torrent file [\"")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (BITTORRENT_CLIENT_GUI_GTK_UI_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g [FILENAME]   : GTK rc file [\"")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l              : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r              : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]      : reporting interval (seconds) [0: off] [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t              : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [DIRECTORY]  : UI file directory [\"")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v              : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]      : #thread pool threads [")
            << TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
} // end print_usage

bool
do_processArguments (int argc_in,
                     ACE_TCHAR* argv_in[], // cannot be const
                     bool& debug_out,
                     std::string& torrentFile_out,
                     std::string& UIRCFile_out,
                     bool& logToFile_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     std::string& UIDefinitionFileDirectory_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numThreadPoolThreads_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  // initialize results
  debug_out =(COMMON_PARSER_DEFAULT_LEX_TRACE ||
              COMMON_PARSER_DEFAULT_YACC_TRACE);

  torrentFile_out = configuration_path;
  torrentFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  torrentFile_out +=
    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_DEFAULT_TORRENT_FILE);

  UIRCFile_out                   = configuration_path;
  UIRCFile_out                  += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIRCFile_out                  +=
    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_UI_RC_FILE);

  logToFile_out                  = false;

  useReactor_out                 =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);

  statisticReportingInterval_out =
    BITTORRENT_DEFAULT_STATISTIC_REPORTING_INTERVAL;

  traceInformation_out           = false;

  UIDefinitionFileDirectory_out  = configuration_path;

  printVersionAndExit_out        = false;
  numThreadPoolThreads_out       =
      TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("df:g:lrs:tu:vx:"),
                              1, // skip command name
                              1, // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS, // ordering
                              0); // for now, don't use long options

  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'd':
      {
        debug_out = true;
        break;
      }
      case 'f':
      {
        torrentFile_out = argumentParser.opt_arg ();
        break;
      }
      case 'g':
      {
        UIRCFile_out = argumentParser.opt_arg ();
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
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
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
        UIDefinitionFileDirectory_out = argumentParser.opt_arg ();
        break;
      }
      case 'v':
      {
        printVersionAndExit_out = true;
        break;
      }
      case 'x':
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << argumentParser.opt_arg ();
        converter >> numThreadPoolThreads_out;
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
do_work (bool debug_in,
         const std::string& torrentFile_in,
         unsigned int numberOfDispatchThreads_in,
         bool useReactor_in,
         struct BitTorrent_Client_UI_CBData& CBData_in,
         const std::string& UIDefinitionFile_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         BitTorrent_Client_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  // sanity check(s)
  ACE_ASSERT (CBData_in.configuration);

  // initialize configuration objects

  // initialize protocol configuration
  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;
  allocator_configuration.defaultBufferSize =
      BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX;

  Stream_CachedAllocatorHeap_T<struct Common_AllocatorConfiguration> heap_allocator (NET_STREAM_MAX_MESSAGES,
                                                                                     BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize allocator, aborting\n")));
    return;
  } // end IF
  BitTorrent_Client_PeerMessageAllocator_t peer_message_allocator (NET_STREAM_MAX_MESSAGES,
                                                                   &heap_allocator);
  BitTorrent_Client_TrackerMessageAllocator_t tracker_message_allocator (NET_STREAM_MAX_MESSAGES,
                                                                         &heap_allocator);
  BitTorrent_Client_Control_t bittorrent_control (&CBData_in.configuration->sessionConfiguration);

  CBData_in.controller = &bittorrent_control;

  ////////////////////////// parser configuration //////////////////////////////
  CBData_in.configuration->parserConfiguration.debugScanner = debug_in;
  CBData_in.configuration->parserConfiguration.debugParser = debug_in;

  ////////////////////////// stream configuration //////////////////////////////
  struct BitTorrent_PeerStreamConfiguration peer_stream_configuration;
  struct BitTorrent_TrackerStreamConfiguration tracker_stream_configuration;

  peer_stream_configuration.messageAllocator = &peer_message_allocator;
  tracker_stream_configuration.messageAllocator =
    &tracker_message_allocator;

  CBData_in.configuration->sessionConfiguration.parserConfiguration =
      &CBData_in.configuration->parserConfiguration;

  ////////////////////// socket handler configuration //////////////////////////
  BitTorrent_Client_PeerConnectionConfiguration peer_connection_configuration;
  peer_connection_configuration.allocatorConfiguration =
      &allocator_configuration;
  peer_connection_configuration.messageAllocator = &peer_message_allocator;
//  peer_connection_configuration.statisticReportingInterval =
//    ACE_Time_Value (reporting_interval, 0);
  peer_connection_configuration.streamConfiguration =
    &CBData_in.configuration->peerStreamConfiguration;

  CBData_in.configuration->peerConnectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                                &peer_connection_configuration));

  BitTorrent_Client_TrackerConnectionConfiguration tracker_connection_configuration;
  tracker_connection_configuration.allocatorConfiguration =
      &allocator_configuration;
  tracker_connection_configuration.messageAllocator =
    &tracker_message_allocator;
//  tracker_connection_configuration.statisticReportingInterval =
//    ACE_Time_Value (reporting_interval, 0);
  tracker_connection_configuration.streamConfiguration =
    &CBData_in.configuration->trackerStreamConfiguration;

  CBData_in.configuration->trackerConnectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                                   &tracker_connection_configuration));

  // step1: initialize configuration
  Net_ConnectionConfigurationsIterator_t iterator =
    CBData_in.configuration->peerConnectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != CBData_in.configuration->peerConnectionConfigurations.end ());
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    CBData_in.configuration->trackerConnectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != CBData_in.configuration->trackerConnectionConfigurations.end ());

  struct Stream_ModuleConfiguration module_configuration;
  struct BitTorrent_Client_PeerModuleHandlerConfiguration peer_modulehandler_configuration;
  peer_modulehandler_configuration.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  peer_modulehandler_configuration.statisticReportingInterval =
    (*iterator).second->statisticReportingInterval;
  peer_modulehandler_configuration.streamConfiguration =
      &CBData_in.configuration->peerStreamConfiguration;
  peer_modulehandler_configuration.parserConfiguration =
    &CBData_in.configuration->peerParserConfiguration;
//  peer_modulehandler_configuration.protocolConfiguration =
//      &CBData_in.configuration->protocolConfiguration;
  CBData_in.configuration->peerStreamConfiguration.initialize (module_configuration,
                                                               peer_modulehandler_configuration,
                                                               peer_stream_configuration);

  struct BitTorrent_Client_TrackerModuleHandlerConfiguration tracker_modulehandler_configuration;
  tracker_modulehandler_configuration.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  tracker_modulehandler_configuration.statisticReportingInterval =
    (*iterator_2).second->statisticReportingInterval;
  tracker_modulehandler_configuration.streamConfiguration =
      &CBData_in.configuration->trackerStreamConfiguration;
  tracker_modulehandler_configuration.parserConfiguration =
    &CBData_in.configuration->trackerParserConfiguration;
  CBData_in.configuration->trackerStreamConfiguration.initialize (module_configuration,
                                                                  tracker_modulehandler_configuration,
                                                                  tracker_stream_configuration);

  CBData_in.configuration->sessionConfiguration.controller =
      &bittorrent_control;
  CBData_in.configuration->sessionConfiguration.metaInfoFileName =
      torrentFile_in;
  CBData_in.configuration->sessionConfiguration.connectionConfiguration =
      &peer_connection_configuration;
  CBData_in.configuration->sessionConfiguration.trackerConnectionConfiguration =
      &tracker_connection_configuration;
  CBData_in.configuration->sessionConfiguration.parserConfiguration =
      &CBData_in.configuration->parserConfiguration;
  CBData_in.configuration->sessionConfiguration.dispatch =
      (useReactor_in ? COMMON_EVENT_DISPATCH_REACTOR
                     : COMMON_EVENT_DISPATCH_PROACTOR);

  // step2: initialize event dispatch
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
    &CBData_in.configuration->dispatchConfiguration;
  if (useReactor_in)
    CBData_in.configuration->dispatchConfiguration.numberOfReactorThreads =
      numberOfDispatchThreads_in;
  else
    CBData_in.configuration->dispatchConfiguration.numberOfProactorThreads =
      numberOfDispatchThreads_in;
  if (!Common_Event_Tools::initializeEventDispatch (CBData_in.configuration->dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize event dispatch, returning\n")));
    return;
  } // end IF

  // step3a: initialize connection manager
  BitTorrent_Client_PeerConnection_Manager_t* peer_connection_manager_p =
      BITTORRENT_CLIENT_PEERCONNECTION_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (peer_connection_manager_p);
  peer_connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                         ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  peer_connection_manager_p->set (*static_cast<BitTorrent_Client_PeerConnectionConfiguration*> ((*iterator).second),
                                  NULL);
  BitTorrent_Client_TrackerConnection_Manager_t* tracker_connection_manager_p =
      BITTORRENT_CLIENT_TRACKERCONNECTION_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (tracker_connection_manager_p);
  tracker_connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                            ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  tracker_connection_manager_p->set (*static_cast<BitTorrent_Client_TrackerConnectionConfiguration*> ((*iterator_2).second),
                                     NULL);

  // step3b: initialize timer manager
  Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);

  // step4: initialize signal handling
  BitTorrent_Client_SignalHandlerConfiguration signal_handler_configuration;
  signal_handler_configuration.control = &bittorrent_control;
  if (!signalHandler_in.initialize (signal_handler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BITTORRENT_CLIENT_SignalHandler::initialize(): \"%m\", returning\n")));
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
    timer_manager_p->stop ();
    return;
  } // end IF

  // step6b: start session controller
  bittorrent_control.start (NULL);

  // step5: start GTK event loop
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  ACE_ASSERT (CBData_in.UIState);
//  Common_UI_GTK_State_t& state_r =
//    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  CBData_in.UIState->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
#endif // GTK_USE
#if defined (GTK_USE)
  gtk_manager_p->start (NULL);
  ACE_Time_Value one_second (1, 0);
  int result = ACE_OS::sleep (one_second);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
  if (!gtk_manager_p->isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start GTK event dispatch, returning\n")));

    // clean up
    timer_manager_p->stop ();

    return;
  } // end IF
#endif // GTK_USE

  // step6: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::startEventDispatch(), returning\n")));

    // clean up
    timer_manager_p->stop ();
#if defined (GTK_USE)
    gtk_manager_p->stop (true, true);
#endif // GTK_USE

    return;
  } // end IF

  // *NOTE*: from this point on, clean up any remote connections !

#if defined (GTK_USE)
  gtk_manager_p->wait (false);
#endif // GTK_USE

  // step8: clean up
  bittorrent_control.stop (true,  // wait ?
                           true); // high priority ?

  Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                             true); // wait ?

  // wait for connection processing to complete
  peer_connection_manager_p->abort ();
  tracker_connection_manager_p->abort ();
  peer_connection_manager_p->wait ();
  tracker_connection_manager_p->wait ();

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
  //         version number... Need this, as the library soname is compared to
  //         this string
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

  // step1: initialize libraries
  // *PORTABILITY*: on Windows, initialize ACE
  // *IMPORTANT NOTE*: when linking with SDL, things go wrong:
  //                   ACE_TMAIN --> main --> SDL_main --> ...
  // --> ACE::init is not called automatically (see OS_main.cpp:74),
  //     which would call WSAStartup(); do it manually (doesn't hurt anyway)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND window_p = GetConsoleWindow ();
  if (!window_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::GetConsoleWindow(), aborting\n")));
    return EXIT_FAILURE;
  } // end IF
  BOOL was_visible_b = ShowWindow (window_p, SW_HIDE);

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

  // step2 initialize/validate configuration

  // step2a: process commandline arguments
  std::string configuration_path             =
    Common_File_Tools::getWorkingDirectory ();
  configuration_path                   += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path                   +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  bool debug                                 =
      (COMMON_PARSER_DEFAULT_LEX_TRACE ||
       COMMON_PARSER_DEFAULT_YACC_TRACE);

  std::string torrent_file_name        = configuration_path;
  torrent_file_name                   += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  torrent_file_name                   +=
    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_DEFAULT_TORRENT_FILE);

  std::string rc_file_name                 = configuration_path;
  rc_file_name                            += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  rc_file_name                            +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  rc_file_name                            += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  rc_file_name                            +=
    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_UI_RC_FILE);

  bool log_to_file                           = false;

  bool use_reactor                           =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);

  unsigned int reporting_interval            =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;

  bool trace_information                     = false;

  std::string UIDefinitionFile_directory     = configuration_path;
  UIDefinitionFile_directory                += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitionFile_directory                +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  bool print_version_and_exit                = false;
  unsigned int number_of_thread_pool_threads =
    TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS;
  if (!do_processArguments (argc_in,
                            argv_in,
                            debug,
                            torrent_file_name,
                            rc_file_name,
                            log_to_file,
                            use_reactor,
                            reporting_interval,
                            trace_information,
                            UIDefinitionFile_directory,
                            print_version_and_exit,
                            number_of_thread_pool_threads))
  {
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_FAILURE;
  } // end IF

  // assemble FQ filename (Glade-UI XML)
  std::string ui_definition_file_name = UIDefinitionFile_directory;
  ui_definition_file_name += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_file_name +=
    ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_GUI_GTK_UI_MAIN_FILE);

  // step2b: validate argument(s)
  if ((!torrent_file_name.empty () &&
       !Common_File_Tools::isReadable (torrent_file_name)) ||
      !Common_File_Tools::isReadable (ui_definition_file_name))
  {
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_FAILURE;
  } // end IF

  // step3: initialize logging and/or tracing
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                        ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]), // program name
                                            log_file_name,              // log file name
                                            false,                      // log to syslog ?
                                            false,                      // trace messages ?
                                            trace_information,          // debug messages ?
                                            NULL))                      // logger
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_FAILURE;
  } // end IF

  // step4: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (use_reactor,
                        false,
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

    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_FAILURE;
  } // end IF

//  Common_MessageStack_t* logstack_p = NULL;
//  ACE_SYNCH_MUTEX* lock_p = NULL;
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
//  logstack_p = &state_r.logStack;
//  lock_p = &state_r.logStackLock;
#endif // GTK_USE

  struct BitTorrent_Client_Configuration configuration;
  struct BitTorrent_Client_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;
  ui_cb_data.UIState = &state_r;
//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
  BitTorrent_Client_SignalHandler signal_handler;

  // step5: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_SUCCESS;
  } // end IF

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ui_cb_data.UIFileDirectory = UIDefinitionFile_directory;
#endif // GTK_USE
#endif // GUI_SUPPORT
//   userData.phoneBook;
//   userData.loginOptions.password = ;
//  ui_cb_data.configuration->protocolConfiguration.loginOptions.nickName =
//      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_DEFAULT_NICKNAME);
//   userData.loginOptions.user.username = ;

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ui_cb_data.progressData.state = &state_r;

  // step8: initialize GTK UI
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.RCFiles.push_back (rc_file_name);
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (ui_cb_data.configuration->GTKConfiguration);
#endif // GTK_USE
#endif // GUI_SUPPORT

  // step9: do work
  ACE_High_Res_Timer timer;
  timer.start ();
  do_work (debug,
           torrent_file_name,
           number_of_thread_pool_threads,
           use_reactor,
           ui_cb_data,
           ui_definition_file_name,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);

  // debug info
  timer.stop ();
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  // debug info
  process_profile.stop ();
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  result = process_profile.elapsed_time (elapsed_time);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, fini ACE...
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
              user_time_string.c_str (),
              system_time_string.c_str (),
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

  // step10: clean up
  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalizeLogging ();

  // step11: finalize libraries
  // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
