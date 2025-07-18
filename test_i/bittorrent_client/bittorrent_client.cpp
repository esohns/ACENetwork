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
#include <list>
#include <sstream>
#include <string>

#if defined (CURSES_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN32)
#include "curses.h"
#else
#include "ncurses.h"
// *NOTE*: the ncurses "timeout" macros conflicts with
//         ACE_Synch_Options::timeout. Since not currently being used, it's safe
//         to undefine...
#undef timeout
#endif // ACE_WIN32 || ACE_WIN32
#endif // CURSES_SUPPORT

//// *WORKAROUND*
//using namespace std;
//// *IMPORTANT NOTE*: several ACE headers include ace/iosfwd.h, which introduces
////                   a problem in conjunction with the standard include headers
////                   when ACE_USES_OLD_IOSTREAMS is defined
////                   --> include the necessary headers manually (see above), and
////                       prevent ace/iosfwd.h from causing any harm
//#define ACE_IOSFWD_H

#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#include "ace/iosfwd.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN32
#include "ace/POSIX_Proactor.h"
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

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_cachedallocatorheap.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_defines.h"

#include "bittorrent_control.h"
#include "bittorrent_defines.h"

#include "test_i_defines.h"

#include "bittorrent_client_configuration.h"
#include "bittorrent_client_defines.h"
// *TODO*: to be removed ASAP
#include "bittorrent_client_network.h"
#include "bittorrent_client_session_common.h"
#include "bittorrent_client_signalhandler.h"
#include "bittorrent_client_stream_common.h"

#include "bittorrent_client_gui_common.h"
#if defined (CURSES_SUPPORT)
#include "bittorrent_client_curses.h"
#endif // CURSES_SUPPORT

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("BitTorrentStream");

void
do_printUsage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  std::cout << ACE_TEXT ("usage: ")
            << programName_in
            << ACE_TEXT (" [OPTIONS]")
            << std::endl << std::endl;
  std::cout << ACE_TEXT ("currently available options:") << std::endl;
  std::cout << ACE_TEXT ("-b        : do NOT send bitfield after peer handshake")
            << ACE_TEXT (" [")
            << !BITTORRENT_DEFAULT_SEND_BITFIELD_AFTER_PEER_HANDSHAKE
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-c        : do NOT request 'compact' peer addresses")
            << ACE_TEXT (" [")
            << !BITTORRENT_DEFAULT_REQUEST_COMPACT_PEER_ADDRESSES
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-d        : debug parser")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_DEFAULT_TORRENT_FILE);
  std::cout << ACE_TEXT ("-f [FILE] : meta-info file")
            << ACE_TEXT (" [\"")
            << path
            << ACE_TEXT ("\"]")
            << std::endl;
  std::cout << ACE_TEXT ("-l        : log to a file")
            << ACE_TEXT (" [")
            << TEST_I_DEFAULT_SESSION_LOG
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-m        : allow multiple connections per peer")
            << ACE_TEXT (" [")
            << BITTORRENT_DEFAULT_ALLOW_MULTIPLE_CONNECTIONS_PER_PEER
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-r        : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-s [VALUE]: reporting interval (seconds: 0 --> OFF)")
            << ACE_TEXT (" [")
            << TEST_I_DEFAULT_STATISTIC_REPORTING_INTERVAL
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-t        : trace information")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-v        : print version information and exit")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
} // end print_usage

bool
do_processArguments (int argc_in,
                     ACE_TCHAR* argv_in[], // cannot be const...
                     bool& requestCompactPeerAddresses_out,
                     bool& sendBitfieldAfterHandshake_out,
                     bool& debugParser_out,
                     std::string& metaInfoFileName_out,
                     bool& logToFile_out,
                     bool& allowMultipleConnectionsPerPeer_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  requestCompactPeerAddresses_out =
    BITTORRENT_DEFAULT_REQUEST_COMPACT_PEER_ADDRESSES;
  sendBitfieldAfterHandshake_out =
    BITTORRENT_DEFAULT_SEND_BITFIELD_AFTER_PEER_HANDSHAKE;
  debugParser_out                = false;

  metaInfoFileName_out           = configuration_path;
  metaInfoFileName_out          += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  metaInfoFileName_out          +=
      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_DEFAULT_TORRENT_FILE);

  logToFile_out                  = TEST_I_DEFAULT_SESSION_LOG;
  allowMultipleConnectionsPerPeer_out =
    BITTORRENT_DEFAULT_ALLOW_MULTIPLE_CONNECTIONS_PER_PEER;
  useReactor_out                 =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
      TEST_I_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out           = false;
  printVersionAndExit_out        = false;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("bcdf:lmrs:tv"),
                              1,                         // skip command name
                              1,                         // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS, // ordering
                              0);                        // for now, don't support long options
  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'b':
      {
        sendBitfieldAfterHandshake_out = false;
        break;
      }
      case 'c':
      {
        requestCompactPeerAddresses_out = false;
        break;
      }
      case 'd':
      {
        debugParser_out = true;
        break;
      }
      case 'f':
      {
        metaInfoFileName_out = argumentParser.opt_arg ();
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'm':
      {
        allowMultipleConnectionsPerPeer_out = true;
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
                    argumentParser.opt_opt ()));
        return false;
      }
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    argumentParser.last_option ()));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    argumentParser.long_option ()));
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

  // *PORTABILITY*: on Microsoft Windows (TM) most signals are not defined,
  //                and ACE_Sig_Set::fill_set() doesn't really work as specified
  //                --> add valid signals (see <signal.h>)...
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

#if defined (VALGRIND_USE)
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif // VALGRIND_USE
#endif // ACE_WIN32 || ACE_WIN64

#if defined (CURSES_USE)
  // *NOTE*: let (n)curses install it's own signal handler and process events in
  //         (w)getch()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//    signals_out.sig_del (SIGINT);
  signals_out.sig_del (SIGWINCH);
#endif // ACE_WIN32 || ACE_WIN64
#endif // CURSES_USE

//// *NOTE*: gdb sends some signals (when running in an IDE ?)
////         --> remove signals (and let IDE handle them)
//#if defined (__GNUC__) && defined (DEBUG_DEBUGGER)
////  signals_out.sig_del (SIGINT);
//  signals_out.sig_del (SIGCONT);
//  signals_out.sig_del (SIGHUP);
//#endif // __GNUC__ && DEBUG_DEBUGGER
}

#if defined (CURSES_USE)
ACE_THR_FUNC_RETURN
session_setup_curses_function (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::session_setup_curses_function"));

  ACE_UNUSED_ARG (arg_in);

  ACE_THR_FUNC_RETURN return_value;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return_value = -1;
#else
  return_value = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  struct BitTorrent_Client_ThreadData* data_p =
    static_cast<struct BitTorrent_Client_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);
  ACE_ASSERT (data_p->dispatchState);

  // step1: start a download session
  try {
    data_p->controller->request (data_p->filename);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_IControl_T::request(\"%s\"), aborting\n"),
                ACE_TEXT (data_p->filename.c_str ())));
    goto error;
  }

  // step2: run curses event dispatch ?
  if (data_p->cursesState)
  {
    BitTorrent_Client_ISession_t* session_p =
        data_p->controller->get (data_p->filename);
    if (!session_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to retrieve session handle (metainfo file was: \"%s\"), aborting\n"),
                  ACE_TEXT (data_p->filename.c_str ())));
      goto error;
    } // end IF
    data_p->cursesState->sessionState =
      &const_cast<struct BitTorrent_Client_SessionState&> (session_p->state ());

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("running curses dispatch loop...\n")));

    bool result_2 = curses_main (*data_p->cursesState,
                                 data_p->controller);
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::curses_main(), aborting\n")));
      goto error;
    } // end IF
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("running curses dispatch loop...DONE\n")));
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return_value = 0;
#else
  return_value = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  return return_value;

error:
  // *NOTE*: signal main thread (resumes dispatching)
  Common_Tools::finalizeEventDispatch (data_p->dispatchState->proactorGroupId,
                                       data_p->dispatchState->reactorGroupId,
                                       false);                                 // don't block

  return return_value;
}
#endif // CURSES_USE

void
do_work (struct BitTorrent_Client_Configuration& configuration_in,
         bool debugParser_in,
         const std::string& metaInfoFileName_in,
         bool useReactor_in,
         bool requestCompactPeerAddresses_in,
         bool sendBitfieldAfterHandshake_in,
         bool allowMultiplerConnectionsPerPeer_in,
         const ACE_Time_Value& statisticReportingInterval_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         BitTorrent_Client_SignalHandler& signalHandler_in,
         unsigned int numberOfDispatchThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  // step1: initialize timers
  Common_Timer_Tools::configuration_.dispatch =
      (useReactor_in ? COMMON_TIMER_DISPATCH_REACTOR
                     : COMMON_TIMER_DISPATCH_PROACTOR);
  //Common_Timer_Tools::configuration_.publishSeconds = false;
  Common_Timer_Tools::initialize ();
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  long timer_id = -1;

#if defined (CURSES_USE)
  int result = -1;
  // *TODO*: clean this up
  struct BitTorrent_Client_CursesState curses_state;
  struct BitTorrent_Client_ThreadData curses_thread_data;
  ACE_thread_t thread_id = -1;
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  char thread_name[BUFSIZ];
  ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  char* thread_name_p = NULL;
  const char* thread_name_2 = NULL;
  int group_id_2 = (COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1); // *TODO*
  ACE_Thread_Manager* thread_manager_p = NULL;
#endif // CURSES_USE
#if defined (SSL_SUPPORT)
  BitTorrent_Client_SSLControl_t bittorrent_control (&configuration_in.sessionConfiguration);
#else
  BitTorrent_Client_Control_t bittorrent_control (&configuration_in.sessionConfiguration);
#endif // SSL_SUPPORT
  BitTorrent_Client_PeerConnection_Manager_t* peer_connection_manager_p =
      BITTORRENT_CLIENT_PEERCONNECTION_MANAGER_SINGLETON::instance ();
  BitTorrent_Client_TrackerConnection_Manager_t* tracker_connection_manager_p =
      BITTORRENT_CLIENT_TRACKERCONNECTION_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (peer_connection_manager_p);
  ACE_ASSERT (tracker_connection_manager_p);
  Net_StreamStatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                                  peer_connection_manager_p,
                                                  false);
  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;
  allocator_configuration.defaultBufferSize =
    BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX * 2;
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  //Stream_CachedAllocatorHeap_T<struct Common_AllocatorConfiguration> heap_allocator (NET_STREAM_MAX_MESSAGES,
  //                                                                                   BITTORRENT_PEER_REQUEST_BLOCK_LENGTH_MAX + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF

  BitTorrent_Client_PeerMessageAllocator_t peer_message_allocator (NET_STREAM_MAX_MESSAGES,
                                                                   &heap_allocator,
                                                                   true);
  BitTorrent_Client_TrackerMessageAllocator_t tracker_message_allocator (NET_STREAM_MAX_MESSAGES,
                                                                         &heap_allocator,
                                                                         true);

  // step1: initialize configuration
#if defined (CURSES_USE)
  configuration_in.cursesState = &curses_state;
#endif // CURSES_USE

#if defined (_DEBUG)
  configuration_in.parserConfiguration.debugParser = debugParser_in;
  if (debugParser_in)
    configuration_in.parserConfiguration.debugScanner = true;
#endif // _DEBUG

  struct Stream_ModuleConfiguration module_configuration;
  struct BitTorrent_Client_PeerModuleHandlerConfiguration peer_modulehandler_configuration;
  peer_modulehandler_configuration.concurrency =
    STREAM_HEADMODULECONCURRENCY_ACTIVE;
  peer_modulehandler_configuration.parserConfiguration =
      &configuration_in.peerParserConfiguration;
  //peer_modulehandler_configuration.statisticReportingInterval =
  //  statisticReportingInterval_in;
  peer_modulehandler_configuration.streamConfiguration =
      &configuration_in.peerStreamConfiguration;

  struct BitTorrent_PeerStreamConfiguration peer_stream_configuration;
  peer_stream_configuration.messageAllocator = &peer_message_allocator;
  configuration_in.peerStreamConfiguration.initialize (module_configuration,
                                                       peer_modulehandler_configuration,
                                                       peer_stream_configuration);

  struct BitTorrent_Client_TrackerModuleHandlerConfiguration tracker_modulehandler_configuration;
  tracker_modulehandler_configuration.concurrency =
    STREAM_HEADMODULECONCURRENCY_ACTIVE;
  tracker_modulehandler_configuration.parserConfiguration =
      &configuration_in.trackerParserConfiguration;
  //tracker_modulehandler_configuration.statisticReportingInterval =
  //  statisticReportingInterval_in;
  tracker_modulehandler_configuration.streamConfiguration =
      &configuration_in.trackerStreamConfiguration;

  struct BitTorrent_TrackerStreamConfiguration tracker_stream_configuration;
  tracker_stream_configuration.messageAllocator = &tracker_message_allocator;
  configuration_in.trackerStreamConfiguration.initialize (module_configuration,
                                                          tracker_modulehandler_configuration,
                                                          tracker_stream_configuration);

  BitTorrent_Client_PeerConnectionConfiguration peer_connection_configuration;
  BitTorrent_Client_TrackerConnectionConfiguration tracker_connection_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier;
#else
  std::string interface_identifier;
#endif // _WIN32_WINNT_VISTA
#else
  std::string interface_identifier;
#endif // ACE_WIN32 || ACE_WIN64

  // step2: initialize event dispatch
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
    &configuration_in.dispatchConfiguration;
  if (useReactor_in)
    configuration_in.dispatchConfiguration.numberOfReactorThreads =
      numberOfDispatchThreads_in;
  else
    configuration_in.dispatchConfiguration.numberOfProactorThreads =
      numberOfDispatchThreads_in;
  if (!Common_Event_Tools::initializeEventDispatch (configuration_in.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    goto clean;
  } // end IF

  // step3: initialize configuration (part 2)
  //peer_connection_configuration.bufferSize = BITTORRENT_CLIENT_BUFFER_SIZE;
  peer_connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;

  peer_connection_configuration.allocatorConfiguration = &allocator_configuration;
  peer_connection_configuration.messageAllocator =
    &peer_message_allocator;
  peer_connection_configuration.streamConfiguration =
    &configuration_in.peerStreamConfiguration;

  configuration_in.peerConnectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                        &peer_connection_configuration));

  tracker_connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;

  tracker_connection_configuration.allocatorConfiguration =
    &allocator_configuration;
  tracker_connection_configuration.messageAllocator =
    &tracker_message_allocator;
  tracker_connection_configuration.streamConfiguration =
    &configuration_in.trackerStreamConfiguration;

  configuration_in.trackerConnectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                           &tracker_connection_configuration));

  configuration_in.sessionConfiguration.controller =
    &bittorrent_control;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  interface_identifier = Net_Common_Tools::getDefaultInterface_2 ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
#else
  interface_identifier = Net_Common_Tools::getDefaultInterface ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
#endif // _WIN32_WINNT_VISTA
#else
  interface_identifier = Net_Common_Tools::getDefaultInterface ((NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP));
#endif // ACE_WIN32 || ACE_WIN64
  if (!Net_Common_Tools::interfaceToExternalIPAddress (interface_identifier,
                                                       configuration_in.sessionConfiguration.externalIPAddress))
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), continuing\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
  configuration_in.sessionConfiguration.metaInfoFileName =
    metaInfoFileName_in;
  configuration_in.sessionConfiguration.connectionConfiguration =
    &peer_connection_configuration;
  configuration_in.sessionConfiguration.trackerConnectionConfiguration =
    &tracker_connection_configuration;
  configuration_in.sessionConfiguration.parserConfiguration =
      &configuration_in.parserConfiguration;
  configuration_in.sessionConfiguration.dispatch =
      ((configuration_in.dispatchConfiguration.numberOfReactorThreads > 0) ? COMMON_EVENT_DISPATCH_REACTOR
                                                                           : COMMON_EVENT_DISPATCH_PROACTOR);
  configuration_in.sessionConfiguration.allowMultipleConnectionsPerPeer =
    allowMultiplerConnectionsPerPeer_in;
  configuration_in.sessionConfiguration.requestCompactPeerAddresses =
    requestCompactPeerAddresses_in;
  configuration_in.sessionConfiguration.sendBitfieldAfterHandshake =
    sendBitfieldAfterHandshake_in;

  configuration_in.signalHandlerConfiguration.controller = &bittorrent_control;
#if defined (CURSES_USE)
  configuration_in.signalHandlerConfiguration.cursesState = &curses_state;
#endif // CURSES_USE

  // step4: initialize signal handling
  if (!signalHandler_in.initialize (configuration_in.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_Client_SignalHandler::initialize(), returning\n")));
    goto clean;
  } // end IF
  if (!Common_Signal_Tools::initialize (((configuration_in.dispatchConfiguration.numberOfReactorThreads > 0) ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                                                                             : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
    goto clean;
  } // end IF

  // step5: initialize connection manager
  peer_connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                         ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  peer_connection_manager_p->set (peer_connection_configuration,
                                  NULL);
  peer_connection_manager_p->start (NULL);
  tracker_connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                            ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  tracker_connection_manager_p->set (tracker_connection_configuration,
                                     NULL);
  tracker_connection_manager_p->start (NULL);

  if (statisticReportingInterval_in != ACE_Time_Value::zero)
  {
    timer_id =
      timer_manager_p->schedule_timer (&statistic_handler,             // event handler handle
                                       NULL,                           // asynchronous completion token
                                       ACE_Time_Value::zero,           // initial delay
                                       statisticReportingInterval_in); // interval
    if (unlikely (timer_id == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));
      goto clean;
    } // end IF
  } // end IF

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly
  //   shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step6a: initialize dispatch thread(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::startEventDispatch(), returning\n")));
    goto clean;
  } // end IF

  // step6b: (try to) connect to the torrent tracker
  bittorrent_control.start (NULL);

#if defined (CURSES_USE)
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  curses_thread_data.configuration = &configuration_in;
  curses_thread_data.controller = &bittorrent_control;
  curses_thread_data.cursesState = &curses_state;
  curses_thread_data.dispatchState = &event_dispatch_state_s;
  curses_thread_data.filename = metaInfoFileName_in;
//  curses_thread_data.groupId = group_id_2;
//  curses_thread_data.moduleHandlerConfiguration =
//      &configuration_in.moduleHandlerConfiguration;

  ACE_NEW_NORETURN (thread_name_p,
                    char[BUFSIZ]);
  if (!thread_name_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
    Common_Tools::finalizeEventDispatch (event_dispatch_state_s.proactorGroupId,
                                         event_dispatch_state_s.reactorGroupId,
                                         false);

    goto clean;
  } // end IF
  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
  thread_name_2 = thread_name_p;
  result =
    thread_manager_p->spawn (::session_setup_curses_function, // function
                             &curses_thread_data,             // argument
                             (THR_NEW_LWP      |
                              THR_JOINABLE     |
                              THR_INHERIT_SCHED),             // flags
                             &thread_id,                      // thread id
                             &thread_handle,                  // thread handle
                             ACE_DEFAULT_THREAD_PRIORITY,     // priority
                             group_id_2,                      // group id
                             NULL,                            // stack
                             0,                               // stack size
                             &thread_name_2);                 // name
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));

    // clean up
    Common_Tools::finalizeEventDispatch (event_dispatch_state_s.proactorGroupId,
                                         event_dispatch_state_s.reactorGroupId,
                                         false);

    goto clean;
  } // end IF
#endif // CURSES_USE

  // step6c: dispatch connection attempt, wait for the session to finish
  bittorrent_control.request (metaInfoFileName_in);
  // wait for the download to complete
  try {
    bittorrent_control.wait (true);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_IControl_T::wait(), returning\n")));
    goto clean;
  }
  bittorrent_control.stop (true,   // wait ?
                           false); // high priority ?

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("session complete...\n")));

  Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                             true,   // wait ?
                                             false); // do not remove the event dispatch singletons here

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  // step7: clean up
clean:
  peer_connection_manager_p->stop (true, false);
  peer_connection_manager_p->abort (true);
  tracker_connection_manager_p->stop (true, false);
  tracker_connection_manager_p->abort (true);

#if defined (CURSES_USE)
  result = thread_manager_p->wait_grp (group_id_2);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", continuing\n"),
                group_id_2));
#endif // CURSES_USE

  Common_Timer_Tools::finalize ();
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
            << std::endl;
#endif // HAVE_CONFIG_H

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

#if defined (CURSES_USE)
  std::cout << ACE_TEXT ("curses: ")
            << curses_version ()
            << std::endl;
#endif // CURSES_USE
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  NETWORK_TRACE (ACE_TEXT ("::main"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result = -1;
#endif // ACE_WIN32 || ACE_WIN64

  // step1: initialize libraries
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *PORTABILITY*: on Windows, init ACE...
  result = ACE::init ();
  if (unlikely (result == -1))
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

  // step2: initialize/validate configuration

  // step2a: process commandline arguments
  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  bool allow_multiple_connections_per_peer_b =
    BITTORRENT_DEFAULT_ALLOW_MULTIPLE_CONNECTIONS_PER_PEER;
  bool debug_parser                          = false;
  bool request_compact_peer_addresses_b =
    BITTORRENT_DEFAULT_REQUEST_COMPACT_PEER_ADDRESSES;
  bool send_bitfield_after_handshake_b =
    BITTORRENT_DEFAULT_SEND_BITFIELD_AFTER_PEER_HANDSHAKE;
  std::string meta_info_file_name            = configuration_path;
  meta_info_file_name                       += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  meta_info_file_name                       +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  meta_info_file_name                       += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  meta_info_file_name                       +=
      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_DEFAULT_TORRENT_FILE);

  bool log_to_file                           = TEST_I_DEFAULT_SESSION_LOG;
  bool use_reactor                           =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval  =
      TEST_I_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  bool trace_information                     = false;
  bool print_version_and_exit                = false;
  unsigned int number_of_thread_pool_threads =
    TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS;
  if (!do_processArguments (argc_in,
                            argv_in,
                            request_compact_peer_addresses_b,
                            send_bitfield_after_handshake_b,
                            debug_parser,
                            meta_info_file_name,
                            log_to_file,
                            allow_multiple_connections_per_peer_b,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            print_version_and_exit))
  {
    do_printUsage (ACE::basename (argv_in[0]));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, fini ACE...
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step2b: validate argument(s)
  if (!Common_File_Tools::isReadable (meta_info_file_name))
  {
    do_printUsage (ACE::basename (argv_in[0]));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, fini ACE...
    result = ACE::fini ();
    if (unlikely (result == -1))
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
                                        ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR)));
  if (!Common_Log_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR)), // program name
                                     log_file_name,                                     // log file name
                                     false,                                             // log to syslog ?
                                     false,                                             // trace messages ?
                                     trace_information,                                 // debug messages ?
                                     NULL))                                             // logger
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, fini ACE...
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step4: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (use_reactor,
                        true,
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, fini ACE...
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
//  ACE_SYNCH_RECURSIVE_MUTEX signal_lock;
  BitTorrent_Client_SignalHandler signal_handler;

  // step5: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, fini ACE...
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_SUCCESS;
  } // end IF

  // step6: initialize configuration objects

  // initialize protocol configuration
  struct BitTorrent_Client_Configuration configuration;
  ////////////////////////////////////////

  // step8: do work
  ACE_High_Res_Timer timer;
  timer.start ();
  do_work (configuration,
           debug_parser,
           meta_info_file_name,
           use_reactor,
           request_compact_peer_addresses_b,
           send_bitfield_after_handshake_b,
           allow_multiple_connections_per_peer_b,
           ACE_Time_Value (statistic_reporting_interval, 0),
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler,
           number_of_thread_pool_threads);

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
  if (process_profile.elapsed_time (elapsed_time) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, fini ACE...
    result = ACE::fini ();
    if (unlikely (result == -1))
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

  // step9: clean up
  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalize ();

  // step10: finalize libraries
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, fini ACE...
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
