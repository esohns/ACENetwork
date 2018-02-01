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
#endif
#include "ace/Log_Msg.h"
#include "ace/Synch.h"
#include "ace/Proactor.h"
#include "ace/Profile_Timer.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/POSIX_Proactor.h"
#endif
#include "ace/Reactor.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#include "common_file_tools.h"
#include "common_logger.h"
#include "common_signal_tools.h"
#include "common_tools.h"

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#include "common_ui_defines.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

#include "test_u_defines.h"

#include "test_u_eventhandler.h"
#include "test_u_signalhandler.h"
#include "test_u_ui_callbacks.h"

#include "wlan_monitor_common.h"
#include "wlan_monitor_defines.h"

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
    ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_CONFIGURATION_DIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("available configuration options:") << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-a           : auto-associate with the AP [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c           : console mode [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif
  std::string UI_file = configuration_path;
  UI_file += ACE_DIRECTORY_SEPARATOR_STR;
  UI_file += ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_UI_DEFINITION_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]] : UI definition file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i [GUID]    : network interface [\"")
            << ACE_TEXT_ALWAYS_CHAR (Net_Common_Tools::interfaceToString (Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_11)).c_str ())
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i [STRING]  : network interface [\"")
            << ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_WLAN)
#endif
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l           : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r [VALUE]   : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL
            << ACE_TEXT_ALWAYS_CHAR ("] {0: off})")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [SSID]    : SSID")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t           : trace information")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v           : print version information and exit")
            << std::endl;
}

bool
do_processArguments (const int& argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     bool& autoAssociate_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#endif
                     std::string& UIDefinitionFilePath_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     struct _GUID& interfaceIdentifier_out,
#else
                     std::string& interfaceIdentifier_out,
#endif
                     bool& logToFile_out,
                     unsigned int& statisticReportingInterval_out,
                     std::string& SSID_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path = Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_CONFIGURATION_DIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  autoAssociate_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#endif
  UIDefinitionFilePath_out = configuration_path;
  UIDefinitionFilePath_out += ACE_DIRECTORY_SEPARATOR_STR;
  UIDefinitionFilePath_out +=
      ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_UI_DEFINITION_FILE);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  interfaceIdentifier_out =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_11);
#else
  interfaceIdentifier_out =
    ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_WLAN);
#endif
  logToFile_out = false;
  statisticReportingInterval_out =
      STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  SSID_out.clear ();
  traceInformation_out = false;
  printVersionAndExit_out = false;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("acg::i:lr:s:tv"));
#else
                              ACE_TEXT ("ag::i:lr:s:tv"));
#endif
  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'a':
      {
        autoAssociate_out = true;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'c':
      {
        showConsole_out = true;
        break;
      }
#endif
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          UIDefinitionFilePath_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIDefinitionFilePath_out.clear ();
        break;
      }
      case 'i':
      {
        interfaceIdentifier_out =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
#else
            ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
#endif
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'r':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> statisticReportingInterval_out;
        break;
      }
      case 's':
      {
        SSID_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
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
do_work (bool autoAssociate_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#endif
         const std::string& UIDefinitionFile_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         REFGUID interfaceIdentifier_in,
#else
         const std::string& interfaceIdentifier_in,
#endif
         unsigned int statisticReportingInterval_in,
         const std::string& SSID_in,
         struct WLANMonitor_GTK_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_U_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;
  struct Test_U_SignalHandlerConfiguration signal_handler_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
  Net_WLAN_IInetMonitor_t* iwlanmonitor_p = NULL;
  WLANMonitor_GTK_Manager_t* gtk_manager_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND window_p = NULL;
  BOOL was_visible_b = true;
#endif
//  long timer_id = -1;
//  int group_id = -1;
//  bool stop_event_dispatch = false;

  // step0a: initialize configuration
  struct WLANMonitor_Configuration configuration;
  CBData_in.configuration = &configuration;

  timer_manager_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  configuration.WLANMonitorConfiguration.timerInterface =
    timer_manager_p;
#endif
  iwlanmonitor_p = NET_WLAN_INETMONITOR_SINGLETON::instance ();
  ACE_ASSERT (iwlanmonitor_p);

  Test_U_StatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                               NET_WLAN_INETMONITOR_SINGLETON::instance (),
                                               false);
  Test_U_EventHandler ui_event_handler (&CBData_in);

  // ********************** monitor configuration data *************************
  configuration.signalHandlerConfiguration.hasUI =
      !UIDefinitionFile_in.empty ();
  configuration.signalHandlerConfiguration.statisticReportingHandler =
      NET_WLAN_INETMONITOR_SINGLETON::instance ();
  configuration.signalHandlerConfiguration.useReactor =
      NET_EVENT_USE_REACTOR;
  configuration.WLANMonitorConfiguration.autoAssociate =
      autoAssociate_in;
  configuration.WLANMonitorConfiguration.interfaceIdentifier =
      interfaceIdentifier_in;
  configuration.WLANMonitorConfiguration.SSID =
      SSID_in;
  configuration.WLANMonitorConfiguration.subscriber = &ui_event_handler;

  // step1: initialize regular (global) statistic reporting ?
  timer_manager_p->initialize (configuration.timerConfiguration);
  timer_manager_p->start ();
  ACE_ASSERT (timer_manager_p->isRunning ());

  if (statisticReportingInterval_in)
  {
    ACE_Time_Value interval (statisticReportingInterval_in,
                             0);
    configuration.signalHandlerConfiguration.statisticReportingTimerId =
      timer_manager_p->schedule_timer (&statistic_handler,         // event handler handle
                                       NULL,                       // asynchronous completion token
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (configuration.signalHandlerConfiguration.statisticReportingTimerId == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", aborting\n")));
      goto error;
    } // end IF
  } // end IF

  // step2: signal handling
  if (!signalHandler_in.initialize (configuration.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, aborting\n")));
    goto error;
  } // end IF
  if (!Common_Signal_Tools::initialize (//COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                        COMMON_SIGNAL_DISPATCH_REACTOR,
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), aborting\n")));
    goto error;
  } // end IF

  // step5: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

  // step5a: start GTK event loop ?
  if (!UIDefinitionFile_in.empty ())
  {
    gtk_manager_p =
      WLANMONITOR_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);

    CBData_in.finalizationHook = idle_finalize_ui_cb;
    CBData_in.initializationHook = idle_initialize_ui_cb;
    CBData_in.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
    CBData_in.userData = &CBData_in;

    gtk_manager_p->start ();
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

//  // step4b: initialize worker(s)
//  if (!Common_Tools::startEventDispatch (thread_data,
//                                         group_id))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to start event dispatch, aborting\n")));
//    goto error;
//  } // end IF
//  stop_event_dispatch = true;

  // step4c: start monitoring ?
  if (UIDefinitionFile_in.empty ())
  {
    if (!iwlanmonitor_p->initialize (configuration.WLANMonitorConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize WLAN monitor, aborting\n")));
      goto error;
    } // end IF

    iwlanmonitor_p->start ();
    if (!iwlanmonitor_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start WLAN monitor, aborting\n")));
      goto error;
    } // end IF
  } // end IF
  else
    gtk_manager_p->wait ();

  iwlanmonitor_p->wait (false); // wait for message queue ?

//  if (!useUDP_in)
//    Common_Tools::dispatchEvents (useReactor_in,
//                                  group_id);
//  stop_event_dispatch = false;

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
  if (!UIDefinitionFile_in.empty ())
    gtk_manager_p->stop (true,
                         true);
//  if (stop_event_dispatch)
//    Common_Tools::finalizeEventDispatch (useReactor_in,
//                                          !useReactor_in,
//                                          group_id);
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

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path = Common_File_Tools::getWorkingDirectory ();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
    ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_CONFIGURATION_DIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  bool auto_associate = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#endif
  std::string UI_definition_file_path = configuration_path;
  UI_definition_file_path += ACE_DIRECTORY_SEPARATOR_STR;
  UI_definition_file_path +=
      ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_UI_DEFINITION_FILE);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID interface_identifier =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_11);
#else
  std::string interface_identifier =
    ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_WLAN);
#endif
  bool log_to_file = false;
//  bool use_reactor = NET_EVENT_USE_REACTOR;
  unsigned int statistic_reporting_interval =
    NET_STATISTIC_DEFAULT_REPORTING_INTERVAL;
  std::string SSID_string;
  bool trace_information = false;
  bool print_version_and_exit = false;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            auto_associate,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#endif
                            UI_definition_file_path,
                            interface_identifier,
                            log_to_file,
//                            use_reactor,
                            statistic_reporting_interval,
                            SSID_string,
                            trace_information,
                            print_version_and_exit))
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
//  if (use_reactor && (number_of_dispatch_threads > 1))
//    use_thread_pool = true;

  if ((!UI_definition_file_path.empty () && !Common_File_Tools::isReadable (UI_definition_file_path)) ||
      (UI_definition_file_path.empty () && (auto_associate && SSID_string.empty ())))
//      (use_thread_pool && !use_reactor)                                    ||
//      (use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool))
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

  struct WLANMonitor_GTK_CBData gtk_cb_data;
  gtk_cb_data.allowUserRuntimeStatistic =
    (statistic_reporting_interval == 0); // handle SIGUSR1/SIGBREAK
                                         // iff regular reporting
                                         // is off
  gtk_cb_data.progressData.state = &gtk_cb_data;

  // step1d: initialize logging and/or tracing
  Common_Logger_t logger (&gtk_cb_data.logStack,
                          &gtk_cb_data.logStackLock);
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
        Common_File_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (LIBACENETWORK_PACKAGE_NAME),
                                           ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                                                ACE_DIRECTORY_SEPARATOR_CHAR)));
  if (!Common_Tools::initializeLogging (ACE::basename (argv_in[0]),    // program name
                                        log_file_name,                 // log file name
                                        true,                          // log to syslog ?
                                        false,                         // trace messages ?
                                        trace_information,             // debug messages ?
                                        (UI_definition_file_path.empty () ? NULL
                                                                          : &logger))) // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeLogging(), aborting\n")));

    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step1e: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (//use_reactor,
                        NET_EVENT_USE_REACTOR,
                        gtk_cb_data.allowUserRuntimeStatistic, // handle SIGUSR1/SIGBREAK
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
                                           //use_reactor,
                                           NET_EVENT_USE_REACTOR,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

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
  Test_U_SignalHandler signal_handler ((UI_definition_file_path.empty () ? NULL
                                                                         : &gtk_cb_data));

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, finalize ACE
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
  // *TODO*: the reasoning here is incomplete
//  bool use_fd_based_reactor = use_reactor;
  bool use_fd_based_reactor = NET_EVENT_USE_REACTOR;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  use_fd_based_reactor =
    (NET_EVENT_USE_REACTOR && !(COMMON_EVENT_REACTOR_TYPE == COMMON_REACTOR_WFMO));
#endif
  bool stack_traces = true;
//  bool use_signal_based_proactor = !use_reactor;
  bool use_signal_based_proactor = !NET_EVENT_USE_REACTOR;
  if (!Common_Tools::setResourceLimits (use_fd_based_reactor,       // file descriptors
                                        stack_traces,               // stack traces
                                        use_signal_based_proactor)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   signal_set,
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

  // step1h: init GLIB / G(D|T)K[+] / GNOME ?
  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  WLANMonitor_GtkBuilderDefinition_t ui_definition (argc_in,
                                                    argv_in);
  if (!UI_definition_file_path.empty ())
    WLANMONITOR_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (argc_in,
                                                                   argv_in,
                                                                   &gtk_cb_data,
                                                                   &ui_definition);

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (auto_associate,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#endif
           UI_definition_file_path,
           interface_identifier,
//           use_reactor,
           statistic_reporting_interval,
           SSID_string,
           gtk_cb_data,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);
  timer.stop ();

  // debug info
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  Common_Timer_Tools::periodToString (working_time,
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

    Common_Signal_Tools::finalize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                   signal_set,
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
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (ACE_Profile_Timer::Rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);
  std::string user_time_string;
  std::string system_time_string;
  Common_Timer_Tools::periodToString (user_time,
                                      user_time_string);
  Common_Timer_Tools::periodToString (system_time,
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

  Common_Signal_Tools::finalize (//COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                 COMMON_SIGNAL_DISPATCH_REACTOR,
                                 signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Tools::finalizeLogging ();

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