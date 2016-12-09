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

// *WORKAROUND*
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include <ace/Get_Opt.h>
#include <ace/High_Res_Timer.h>
#include <ace/iosfwd.h>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <ace/Init_ACE.h>
#endif
#include <ace/Synch.h>
#include <ace/POSIX_Proactor.h>
#include <ace/Proactor.h>
#include <ace/Profile_Timer.h>
#include <ace/Sig_Handler.h>
#include <ace/Signal.h>
#include <ace/Version.h>

#if defined (ACE_WIN32) || defined (ACE_WIN32)
#include <curses.h>
#else
#include <ncurses.h>
// *NOTE*: the ncurses "timeout" macros conflicts with
//         ACE_Synch_Options::timeout. Since not currently being used, it's safe
//         to undefine...
#undef timeout
#endif

#include "common_file_tools.h"
#include "common_tools.h"

#include "stream_cachedallocatorheap.h"

#include "stream_misc_messagehandler.h"

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

#include "net_defines.h"

#include "bittorrent_bencoding_scanner.h"
#include "bittorrent_common.h"
#include "bittorrent_control.h"
#include "bittorrent_defines.h"
#include "bittorrent_tools.h"

#include "bittorrent_client_configuration.h"
#include "bittorrent_client_curses.h"
#include "bittorrent_client_defines.h"
// *TODO*: to be removed ASAP
#include "bittorrent_client_gui_common.h"
#include "bittorrent_client_network.h"
#include "bittorrent_client_session_common.h"
#include "bittorrent_client_signalhandler.h"
#include "bittorrent_client_stream_common.h"
#include "bittorrent_client_tools.h"

#include "test_i_defines.h"

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
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_CONFIGURATION_DIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT ("usage: ")
            << programName_in
            << ACE_TEXT (" [OPTIONS]")
            << std::endl << std::endl;
  std::cout << ACE_TEXT ("currently available options:") << std::endl;
  std::cout << ACE_TEXT ("-d        : debug parser")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::string path = configuration_path;
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
  std::cout << ACE_TEXT ("-n        : use (PD|n)curses library [")
            << BITTORRENT_CLIENT_DEFAULT_USE_CURSES
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-r        : use reactor [")
            << NET_EVENT_USE_REACTOR
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
  std::cout << ACE_TEXT ("-x [VALUE]: #thread pool threads ([")
            << TEST_I_DEFAULT_NUMBER_OF_TP_THREADS
            << ACE_TEXT ("]")
            << std::endl;
} // end print_usage

bool
do_processArguments (int argc_in,
                     ACE_TCHAR* argv_in[], // cannot be const...
                     bool& debugParser_out,
                     std::string& metaInfoFileName_out,
                     bool& logToFile_out,
                     bool& useCursesLibrary_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numThreadPoolThreads_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_CONFIGURATION_DIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  debugParser_out                = false;

  metaInfoFileName_out           = configuration_path;
  metaInfoFileName_out          += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  metaInfoFileName_out          +=
      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_DEFAULT_TORRENT_FILE);

  logToFile_out                  = TEST_I_DEFAULT_SESSION_LOG;
  useCursesLibrary_out           = BITTORRENT_CLIENT_DEFAULT_USE_CURSES;
  useReactor_out                 = NET_EVENT_USE_REACTOR;
  statisticReportingInterval_out =
      TEST_I_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out           = false;
  printVersionAndExit_out        = false;
  numThreadPoolThreads_out       = TEST_I_DEFAULT_NUMBER_OF_TP_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("df:lnrs:tvx:"),
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
      case 'n':
      {
        useCursesLibrary_out = true;
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
                      bool useCursesLibrary,
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
#endif

  // *NOTE*: let (n)curses install it's own signal handler and process events in
  //         (w)getch()
  if (useCursesLibrary)
  {
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
//    signals_out.sig_del (SIGINT);
    signals_out.sig_del (SIGWINCH);
#endif
  } // end IF

// *NOTE*: gdb sends some signals (when running in an IDE ?)
//         --> remove signals (and let IDE handle them)
#if defined (__GNUC__) && defined (DEBUG_DEBUGGER)
//  signals_out.sig_del (SIGINT);
  signals_out.sig_del (SIGCONT);
  signals_out.sig_del (SIGHUP);
#endif
}

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
#endif

  BitTorrent_Client_ThreadData* data_p =
    static_cast<BitTorrent_Client_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->controller);

  // step1: start a download session
  try {
    data_p->controller->download (data_p->filename);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in BitTorrent_IControl_T::download(\"%s\"), aborting\n"),
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
      &const_cast<BitTorrent_Client_SessionState&> (session_p->state ());

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
#endif

  return return_value;

error:
  // *NOTE*: signal main thread (resumes dispatching)
  Common_Tools::finalizeEventDispatch (data_p->useReactor,
                                       !data_p->useReactor,
                                       -1);

  return return_value;
}

void
do_work (BitTorrent_Client_Configuration& configuration_in,
         bool useCursesLibrary_in,
         const std::string& metaInfoFileName_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         BitTorrent_Client_SignalHandler& signalHandler_in,
         unsigned int numberOfDispatchThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;
  // *TODO*: clean this up
  BitTorrent_Client_CursesState curses_state;
  BitTorrent_Client_ThreadData curses_thread_data;
  ACE_thread_t thread_id = -1;
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  //char thread_name[BUFSIZ];
  //ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  char* thread_name_p = NULL;
  const char* thread_name_2 = NULL;
  int group_id_2 = (COMMON_EVENT_THREAD_GROUP_ID + 1); // *TODO*
  ACE_Thread_Manager* thread_manager_p = NULL;
  BitTorrent_Client_Control_t bittorrent_control (&configuration_in.sessionConfiguration);
  BitTorrent_Client_PeerConnection_Manager_t* peer_connection_manager_p =
      BITTORRENT_CLIENT_PEERCONNECTION_MANAGER_SINGLETON::instance ();
  BitTorrent_Client_TrackerConnection_Manager_t* tracker_connection_manager_p =
      BITTORRENT_CLIENT_TRACKERCONNECTION_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (peer_connection_manager_p);
  ACE_ASSERT (tracker_connection_manager_p);

  // step1: initialize configuration
  if (useCursesLibrary_in)
    configuration_in.cursesState = &curses_state;

  configuration_in.peerModuleHandlerConfiguration.streamConfiguration =
      &configuration_in.peerStreamConfiguration;
//  configuration_in.moduleHandlerConfiguration.protocolConfiguration =
  configuration_in.peerStreamConfiguration.moduleHandlerConfiguration =
      &configuration_in.peerModuleHandlerConfiguration;
  configuration_in.peerStreamConfiguration.moduleConfiguration =
      &configuration_in.moduleConfiguration;
  configuration_in.trackerModuleHandlerConfiguration.streamConfiguration =
      &configuration_in.trackerStreamConfiguration;
  configuration_in.trackerStreamConfiguration.moduleHandlerConfiguration =
      &configuration_in.trackerModuleHandlerConfiguration;
  configuration_in.trackerStreamConfiguration.moduleConfiguration =
      &configuration_in.moduleConfiguration;

  // step2: initialize event dispatch
  struct Common_DispatchThreadData dispatch_thread_data;
  dispatch_thread_data.numberOfDispatchThreads = numberOfDispatchThreads_in;
  dispatch_thread_data.useReactor = configuration_in.useReactor;
  if (!Common_Tools::initializeEventDispatch (configuration_in.useReactor,
                                              (dispatch_thread_data.numberOfDispatchThreads > 1),
                                              dispatch_thread_data.numberOfDispatchThreads,
                                              dispatch_thread_data.proactorType,
                                              dispatch_thread_data.reactorType,
                                              configuration_in.peerStreamConfiguration.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    goto clean;
  } // end IF

  // step3: initialize configuration (part 2)
  //configuration_in.socketHandlerConfiguration.bufferSize =
  //  BITTORRENT_CLIENT_BUFFER_SIZE;
  configuration_in.peerSocketHandlerConfiguration.messageAllocator =
    configuration_in.peerStreamConfiguration.messageAllocator;
  configuration_in.peerSocketHandlerConfiguration.socketConfiguration =
    &configuration_in.socketConfiguration;
  configuration_in.peerSocketHandlerConfiguration.statisticReportingInterval =
    configuration_in.peerStreamConfiguration.statisticReportingInterval;
  configuration_in.peerSocketHandlerConfiguration.userData =
    &configuration_in.peerUserData;

  configuration_in.trackerSocketHandlerConfiguration.messageAllocator =
    configuration_in.trackerStreamConfiguration.messageAllocator;
  configuration_in.trackerSocketHandlerConfiguration.socketConfiguration =
    &configuration_in.socketConfiguration;
  configuration_in.trackerSocketHandlerConfiguration.statisticReportingInterval =
    configuration_in.trackerStreamConfiguration.statisticReportingInterval;
  configuration_in.trackerSocketHandlerConfiguration.userData =
    &configuration_in.trackerUserData;

  configuration_in.peerConnectionConfiguration.socketHandlerConfiguration =
    &configuration_in.peerSocketHandlerConfiguration;
  configuration_in.peerConnectionConfiguration.streamConfiguration =
    &configuration_in.peerStreamConfiguration;
  configuration_in.peerConnectionConfiguration.userData =
    &configuration_in.peerUserData;

  configuration_in.trackerConnectionConfiguration.socketHandlerConfiguration =
    &configuration_in.trackerSocketHandlerConfiguration;
  configuration_in.trackerConnectionConfiguration.streamConfiguration =
    &configuration_in.trackerStreamConfiguration;
  configuration_in.trackerConnectionConfiguration.userData =
    &configuration_in.trackerUserData;

  configuration_in.sessionConfiguration.controller =
      &bittorrent_control;
  configuration_in.sessionConfiguration.connectionManager =
      peer_connection_manager_p;
  configuration_in.sessionConfiguration.trackerConnectionManager =
      tracker_connection_manager_p;
  configuration_in.sessionConfiguration.metaInfoFileName =
      metaInfoFileName_in;
  configuration_in.sessionConfiguration.socketHandlerConfiguration =
      &configuration_in.peerSocketHandlerConfiguration;
  configuration_in.sessionConfiguration.traceScanning =
      configuration_in.peerModuleHandlerConfiguration.traceScanning;
  configuration_in.sessionConfiguration.traceParsing =
      configuration_in.peerModuleHandlerConfiguration.traceParsing;
  configuration_in.sessionConfiguration.trackerSocketHandlerConfiguration =
      &configuration_in.trackerSocketHandlerConfiguration;
  configuration_in.sessionConfiguration.useReactor =
      configuration_in.useReactor;

  if (useCursesLibrary_in)
    configuration_in.signalHandlerConfiguration.cursesState = &curses_state;

  configuration_in.peerUserData.configuration =
      &configuration_in.peerConnectionConfiguration;
  configuration_in.peerUserData.moduleConfiguration =
      &configuration_in.moduleConfiguration;
  configuration_in.peerUserData.moduleHandlerConfiguration =
      &configuration_in.peerModuleHandlerConfiguration;
  configuration_in.trackerUserData.configuration =
      &configuration_in.trackerConnectionConfiguration;
  configuration_in.trackerUserData.moduleConfiguration =
      &configuration_in.moduleConfiguration;
  configuration_in.trackerUserData.moduleHandlerConfiguration =
      &configuration_in.trackerModuleHandlerConfiguration;

  // step4: initialize signal handling
  if (!signalHandler_in.initialize (configuration_in.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to BitTorrent_Client_SignalHandler::initialize(), returning\n")));
    goto clean;
  } // end IF
  if (!Common_Tools::initializeSignals (signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeSignals(), returning\n")));
    goto clean;
  } // end IF

  // step5: initialize connection manager
  peer_connection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  peer_connection_manager_p->set (configuration_in.peerConnectionConfiguration,
                                  //configuration_in.streamUserData);
                                  NULL);
  tracker_connection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  tracker_connection_manager_p->set (configuration_in.trackerConnectionConfiguration,
                                     //configuration_in.streamUserData);
                                     NULL);

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly
  //   shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step6a: initialize dispatch thread(s)
  if (!Common_Tools::startEventDispatch (dispatch_thread_data,
                                         configuration_in.groupID))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::startEventDispatch(), returning\n")));
    goto clean;
  } // end IF

  // step6b: (try to) connect to the torrent tracker

  curses_thread_data.configuration = &configuration_in;
  curses_thread_data.controller = &bittorrent_control;
  if (useCursesLibrary_in)
    curses_thread_data.cursesState = &curses_state;
  curses_thread_data.filename = metaInfoFileName_in;
//  curses_thread_data.groupID = group_id_2;
//  curses_thread_data.moduleHandlerConfiguration =
//      &configuration_in.moduleHandlerConfiguration;
  curses_thread_data.useReactor = configuration_in.useReactor;

  ACE_NEW_NORETURN (thread_name_p,
                    char[BUFSIZ]);
  if (!thread_name_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
    Common_Tools::finalizeEventDispatch (configuration_in.useReactor,
                                         !configuration_in.useReactor,
                                         configuration_in.groupID);

    goto clean;
  } // end IF
  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
  thread_name_2 = thread_name_p;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
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
    Common_Tools::finalizeEventDispatch (configuration_in.useReactor,
                                         !configuration_in.useReactor,
                                         configuration_in.groupID);

    goto clean;
  } // end IF

  // step6c: dispatch connection attempt, wait for the session to finish
  Common_Tools::dispatchEvents (configuration_in.useReactor,
                                configuration_in.groupID);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  // step7: clean up
clean:
  peer_connection_manager_p->wait ();
  tracker_connection_manager_p->wait ();
  result = thread_manager_p->wait_grp (group_id_2);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", continuing\n"),
                group_id_2));

  return;

//error:
//  bittorrent_control.stop (true); // wait ?
//  peer_connection_manager_p->abort ();
//  tracker_connection_manager_p->abort ();
//  peer_connection_manager_p->wait ();
//  tracker_connection_manager_p->wait ();
//  result = thread_manager_p->wait_grp (group_id_2);
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", continuing\n"),
//                group_id_2));
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
            << std::endl;
#endif

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

  std::cout << ACE_TEXT ("curses: ")
            << curses_version ()
            << std::endl;
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  NETWORK_TRACE (ACE_TEXT ("::main"));

  int result = -1;

  // step1: initialize libraries
//  // *PORTABILITY*: on Windows, init ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  if (ACE::init () == -1)
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

  // step2: initialize/validate configuration

  // step2a: process commandline arguments
  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_CONFIGURATION_DIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  bool debug_parser                          = false;

  std::string meta_info_file_name        = configuration_path;
  meta_info_file_name                   += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  meta_info_file_name                   +=
      ACE_TEXT_ALWAYS_CHAR (BITTORRENT_CLIENT_DEFAULT_TORRENT_FILE);

  bool log_to_file                           = TEST_I_DEFAULT_SESSION_LOG;
  bool use_curses_library                    =
      BITTORRENT_CLIENT_DEFAULT_USE_CURSES;
  bool use_reactor                           = NET_EVENT_USE_REACTOR;
  unsigned int statistic_reporting_interval  =
      TEST_I_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  bool trace_information                     = false;
  bool print_version_and_exit                = false;
  unsigned int number_of_thread_pool_threads =
      TEST_I_DEFAULT_NUMBER_OF_TP_THREADS;
  if (!do_processArguments (argc_in,
                            argv_in,
                            debug_parser,
                            meta_info_file_name,
                            log_to_file,
                            use_curses_library,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            print_version_and_exit,
                            number_of_thread_pool_threads))
  {
    do_printUsage (ACE::basename (argv_in[0]));

//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF

  // step2b: validate argument(s)
  if (!Common_File_Tools::isReadable (meta_info_file_name))
  {
    do_printUsage (ACE::basename (argv_in[0]));

//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF

  // step3: initialize logging and/or tracing
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
        Common_File_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (LIBACENETWORK_PACKAGE_NAME),
                                           ACE::basename (argv_in[0]));
  if (!Common_Tools::initializeLogging (ACE::basename (argv_in[0]), // program name
                                        log_file_name,              // log file name
                                        false,                      // log to syslog ?
                                        false,                      // trace messages ?
                                        trace_information,          // debug messages ?
                                        NULL))                      // logger
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeLogging(), aborting\n")));

//    // *PORTABILITY*: on Windows, need to fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF

  // step4: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (use_reactor,
                        use_curses_library,
                        true,
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
//    // *PORTABILITY*: on Windows, fini ACE...
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
//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF
  BitTorrent_Client_SignalHandler signal_handler (use_reactor);

  // step5: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_SUCCESS;
  } // end IF

  // step6: initialize configuration objects

  // initialize protocol configuration
  Stream_CachedAllocatorHeap_T<Stream_AllocatorConfiguration> heap_allocator (NET_STREAM_MAX_MESSAGES,
                                                                              BITTORRENT_BUFFER_SIZE);
  BitTorrent_Client_PeerMessageAllocator_t peer_message_allocator (NET_STREAM_MAX_MESSAGES,
                                                                   &heap_allocator);
  BitTorrent_Client_TrackerMessageAllocator_t tracker_message_allocator (NET_STREAM_MAX_MESSAGES,
                                                                         &heap_allocator);

  BitTorrent_Client_Configuration configuration;
  ////////////////////////////////////////
  configuration.peerStreamConfiguration.messageAllocator =
      &peer_message_allocator;
  configuration.peerStreamConfiguration.statisticReportingInterval =
      statistic_reporting_interval;
  configuration.trackerStreamConfiguration.messageAllocator =
      &tracker_message_allocator;
  configuration.trackerStreamConfiguration.statisticReportingInterval =
      statistic_reporting_interval;
  ////////////////////////////////////////
  configuration.peerModuleHandlerConfiguration.traceParsing = debug_parser;
  configuration.peerModuleHandlerConfiguration.traceScanning = debug_parser;
  configuration.trackerModuleHandlerConfiguration.traceParsing = debug_parser;
  configuration.trackerModuleHandlerConfiguration.traceScanning = debug_parser;
  ///////////////////////////////////////
  configuration.useReactor = use_reactor;

  if (!heap_allocator.initialize (configuration.allocatorConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize allocator, aborting\n")));

    Common_Tools::finalizeLogging ();
//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF

  // step8: do work
  ACE_High_Res_Timer timer;
  timer.start ();
  do_work (configuration,
           use_curses_library,
           meta_info_file_name,
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
  Common_Tools::period2String (working_time,
                               working_time_string);
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

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
//    // *PORTABILITY*: on Windows, fini ACE...
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
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ())));
#endif

  // step9: clean up
  Common_Tools::finalizeSignals (signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Tools::finalizeLogging ();

  // step10: finalize libraries
//  // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  result = ACE::fini ();
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//      ACE_TEXT ("failed to ACE::fini(): \"%m\", aborting\n")));
//    return EXIT_FAILURE;
//  } // end IF
//#endif

  return EXIT_SUCCESS;
} // end main
