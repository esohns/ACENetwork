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
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Log_Msg.h"
//#include "ace/Synch.h"
#include "ace/Proactor.h"
#include "ace/Profile_Timer.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/POSIX_Proactor.h"
#endif // ACE_WIN32 || ACE_WIN64
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

#include "common_timer_manager_common.h"
#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_defines.h"
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

#include "test_u_defines.h"

#include "test_u_eventhandler.h"
#include "test_u_signalhandler.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_ui_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

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
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("available configuration options:") << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-a           : auto-associate with the wireless access point [")
            << true
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c           : console mode [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#else
#if defined (DBUS_SUPPORT)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d           : use DBus/NetworkManager implementation [")
            << (NET_WLAN_MONITOR_DEFAULT_API == NET_WLAN_MONITOR_API_DBUS)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  std::string UI_file = configuration_path;
  UI_file += ACE_DIRECTORY_SEPARATOR_STR;
  UI_file += ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_UI_DEFINITION_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]] : UI definition file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (WEXT_SUPPORT)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i           : use ioctl implementation [")
            << (NET_WLAN_MONITOR_DEFAULT_API == NET_WLAN_MONITOR_API_WEXT)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-k           : use nl80211 implementation [")
            << (NET_WLAN_MONITOR_DEFAULT_API == NET_WLAN_MONITOR_API_NL80211)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l           : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n [GUID]    : network interface [\"")
            << Net_Common_Tools::interfaceToString (Net_Common_Tools::getDefaultInterface_2 (NET_LINKLAYER_802_11)).c_str ()
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n [STRING]  : network interface [\"")
            << Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_11).c_str ()
#endif // _WIN32_WINNT_VISTA
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n [STRING]  : network interface [\"")
            << ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_WLAN)
#endif // ACE_WIN32 || ACE_WIN64
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
//  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r [VALUE]   : statistic reporting interval (second(s)) [")
//            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL
//            << ACE_TEXT_ALWAYS_CHAR ("] {0: off})")
//            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
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
                     enum Net_WLAN_MonitorAPIType& API_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#endif // ACE_WIN32 || ACE_WIN64
                     std::string& UIDefinitionFilePath_out,
                     bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                     struct _GUID& interfaceIdentifier_out,
#else
                     std::string& interfaceIdentifier_out,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
                     std::string& interfaceIdentifier_out,
#endif // ACE_WIN32 || ACE_WIN64
//                     unsigned int& statisticReportingInterval_out,
                     bool& useReactor_out,
                     std::string& SSID_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path = Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#endif // DEBUG_DEBUGGER

  // initialize results
  autoAssociate_out = true;
  API_out = NET_WLAN_MONITOR_DEFAULT_API;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#endif // ACE_WIN32 || ACE_WIN64
  UIDefinitionFilePath_out = configuration_path;
  UIDefinitionFilePath_out += ACE_DIRECTORY_SEPARATOR_STR;
  UIDefinitionFilePath_out +=
      ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_UI_DEFINITION_FILE);
  interfaceIdentifier_out =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    Net_Common_Tools::getDefaultInterface_2 (NET_LINKLAYER_802_11);
#else
    Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_11);
#endif // _WIN32_WINNT_VISTA
#else
    ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_WLAN);
#endif // ACE_WIN32 || ACE_WIN64
  logToFile_out = false;
//  statisticReportingInterval_out =
//      STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  useReactor_out =
          (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  SSID_out.clear ();
  traceInformation_out = false;
  printVersionAndExit_out = false;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("acg::ln:r:s:tv"));
#else
                              ACE_TEXT ("adg::ikln:r:s:tv"));
#endif // ACE_WIN32 || ACE_WIN64
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
#else
#if defined (DBUS_SUPPORT)
      case 'd':
      {
        API_out = NET_WLAN_MONITOR_API_DBUS;
        break;
      }
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          UIDefinitionFilePath_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIDefinitionFilePath_out.clear ();
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (WEXT_SUPPORT)
      case 'i':
      {
        API_out = NET_WLAN_MONITOR_API_WEXT;
        break;
      }
#endif // WEXT_SUPPORT
#if defined (NL80211_SUPPORT)
      case 'k':
      {
        API_out = NET_WLAN_MONITOR_API_NL80211;
        break;
      }
#endif // NL80211_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'n':
      {
        interfaceIdentifier_out =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
            Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
#else
            ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
            ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
#endif // ACE_WIN32 || ACE_WIN64
        break;
      }
      case 'r':
      {
//        converter.clear ();
//        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
//        converter << argumentParser.opt_arg ();
//        converter >> statisticReportingInterval_out;
        useReactor_out = true;
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
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (bool autoAssociate_in,
         enum Net_WLAN_MonitorAPIType API_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#endif // ACE_WIN32 || ACE_WIN64
         const std::string& UIDefinitionFile_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
         REFGUID interfaceIdentifier_in,
#else
         const std::string& interfaceIdentifier_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
         const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
//         unsigned int statisticReportingInterval_in,
         bool useReactor_in,
         const std::string& SSID_in,
#if defined (GUI_SUPPORT)
         struct WLANMonitor_UI_CBData& CBData_in,
#endif // GUI_SUPPORT
         struct WLANMonitor_Configuration& configuration_in,
         unsigned int numberOfDispatchThreads_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_U_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  struct Test_U_SignalHandlerConfiguration signal_handler_configuration;
  Common_Timer_Manager_t* timer_manager_p = NULL;
  Net_WLAN_IMonitor_t* iwlanmonitor_p = NULL;
  Net_IStatisticHandler_t* istatistic_handler_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND window_p = NULL;
  BOOL was_visible_b = true;
#endif // ACE_WIN32 || ACE_WIN64
//  long timer_id = -1;
//  int group_id = -1;
//  bool stop_event_dispatch = false;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  int result = -1;
#endif // GTK_USE
#endif // GUI_SUPPORT
  struct Common_EventDispatchState event_dispatch_state_s;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (_DEBUG)
  Common_Tools::printCapabilities ();
#endif // _DEBUG
#endif // ACE_WIN32 || ACE_WIN64

  timer_manager_p = COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  configuration_in.WLANMonitorConfiguration.timerInterface =
    timer_manager_p;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (WLANAPI_SUPPORT)
  iwlanmonitor_p = NET_WLAN_WLANAPIMONITOR_SINGLETON::instance ();
  istatistic_handler_p = NET_WLAN_WLANAPIMONITOR_SINGLETON::instance ();
#else
      ACE_ASSERT (false);
      ACE_NOTSUP;
      ACE_NOTREACHED (break;)
#endif // WLANAPI_SUPPORT
#else
  switch (API_in)
  {
    case NET_WLAN_MONITOR_API_WEXT:
#if defined (WEXT_SUPPORT)
      iwlanmonitor_p = NET_WLAN_WEXTMONITOR_SINGLETON::instance ();
      istatistic_handler_p = NET_WLAN_WEXTMONITOR_SINGLETON::instance ();
#endif // WEXT_SUPPORT
      break;
    case NET_WLAN_MONITOR_API_NL80211:
      iwlanmonitor_p = NET_WLAN_NL80211MONITOR_SINGLETON::instance ();
      istatistic_handler_p = NET_WLAN_NL80211MONITOR_SINGLETON::instance ();
      break;
    case NET_WLAN_MONITOR_API_DBUS:
#if defined (DBUS_SUPPORT)
      iwlanmonitor_p = NET_WLAN_DBUSMONITOR_SINGLETON::instance ();
      istatistic_handler_p = NET_WLAN_DBUSMONITOR_SINGLETON::instance ();
#endif // DBUS_SUPPORT
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown WLAN monitor API (was: %d), aborting\n"),
                  API_in));
      return;
    }
  } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (iwlanmonitor_p && istatistic_handler_p);
  iwlanmonitor_p->initialize (configuration_in.WLANMonitorConfiguration);

  Net_StatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                            dynamic_cast<Net_IStatisticHandler_t*> (iwlanmonitor_p),
                                            false);
  Test_U_EventHandler ui_event_handler (&CBData_in);
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  WLANMonitor_UI_GTK_Manager_t* gtk_manager_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // ********************** monitor configuration data *************************
  configuration_in.signalHandlerConfiguration.monitor = iwlanmonitor_p;
  configuration_in.signalHandlerConfiguration.statisticReportingHandler =
      istatistic_handler_p;
  configuration_in.WLANMonitorConfiguration.autoAssociate =
      autoAssociate_in;
  configuration_in.WLANMonitorConfiguration.interfaceIdentifier =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      interfaceIdentifier_in;
#else
      Net_Common_Tools::indexToInterface_2 (Net_Common_Tools::interfaceToIndex (interfaceIdentifier_in));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
      interfaceIdentifier_in;
#endif // ACE_WIN32 || ACE_WIN64
  configuration_in.WLANMonitorConfiguration.SSID = SSID_in;
  configuration_in.WLANMonitorConfiguration.subscriber = &ui_event_handler;

  // step1: initialize regular (global) statistic reporting ?
  timer_manager_p->initialize (configuration_in.timerConfiguration);
  ACE_thread_t thread_id = 0;
  timer_manager_p->start (thread_id);
  ACE_UNUSED_ARG (thread_id);
  ACE_ASSERT (timer_manager_p->isRunning ());

//  if (statisticReportingInterval_in)
//  {
//    ACE_Time_Value interval (statisticReportingInterval_in,
//                             0);
//    configuration_in.signalHandlerConfiguration.statisticReportingTimerId =
//      timer_manager_p->schedule_timer (&statistic_handler,         // event handler handle
//                                       NULL,                       // asynchronous completion token
//                                       COMMON_TIME_NOW + interval, // first wakeup time
//                                       interval);                  // interval
//    if (configuration_in.signalHandlerConfiguration.statisticReportingTimerId == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to schedule timer: \"%m\", aborting\n")));
//      goto error;
//    } // end IF
//  } // end IF

  // step2: signal handling
  if (!signalHandler_in.initialize (configuration_in.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, aborting\n")));
    goto error;
  } // end IF
  if (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                        //COMMON_SIGNAL_DISPATCH_REACTOR,
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
  if (useReactor_in)
    configuration_in.dispatchConfiguration.numberOfReactorThreads =
      numberOfDispatchThreads_in;
  else
    configuration_in.dispatchConfiguration.numberOfProactorThreads =
      numberOfDispatchThreads_in;
  if (!Common_Tools::initializeEventDispatch (configuration_in.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF
  event_dispatch_state_s.configuration =
      &configuration_in.dispatchConfiguration;
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

  // step5a: start GTK event loop ?
#if defined (GUI_SUPPORT)
  if (!UIDefinitionFile_in.empty ())
  {
#if defined (GTK_USE)
    gtk_manager_p = WLANMONITOR_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);

    CBData_in.UIState->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
#endif // GTK_USE
    CBData_in.monitor = iwlanmonitor_p;
    //CBData_in.userData = &CBData_in;

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
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_USE)
    ACE_ASSERT (gtk_manager_p);
    thread_id = 0;
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
    gtk_manager_p->wait (false); // no message queue
#endif // GTK_USE
  } // end IF
  else
  {
#endif // GUI_SUPPORT
    if (!iwlanmonitor_p->initialize (configuration_in.WLANMonitorConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize WLAN monitor, aborting\n")));
      goto error;
    } // end IF

    thread_id = 0;
    iwlanmonitor_p->start (thread_id);
    ACE_UNUSED_ARG (thread_id);
    if (!iwlanmonitor_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start WLAN monitor, aborting\n")));
      goto error;
    } // end IF
#if defined (GUI_SUPPORT)
  } // end ELSE
#endif // GUI_SUPPORT

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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    gtk_manager_p->stop (true,
                         true);
#else
    ;
#endif // GTK_USE
#else
    ;
#endif // GUI_SUPPORT
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
#endif // ACE_WIN32 || ACE_WIN64

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
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  bool auto_associate = true;
  enum Net_WLAN_MonitorAPIType API_e = NET_WLAN_MONITOR_DEFAULT_API;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#endif // ACE_WIN32 || ACE_WIN64
  std::string UI_definition_file_path = configuration_path;
  UI_definition_file_path += ACE_DIRECTORY_SEPARATOR_STR;
  UI_definition_file_path +=
      ACE_TEXT_ALWAYS_CHAR (WLAN_MONITOR_UI_DEFINITION_FILE);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier =
      Net_Common_Tools::getDefaultInterface_2 (NET_LINKLAYER_802_11);
#else
  std::string interface_identifier =
    Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_11);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  std::string interface_identifier =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_11);
//    ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_WLAN);
#endif // ACE_WIN32 || ACE_WIN64
  bool log_to_file = false;
//  unsigned int statistic_reporting_interval =
//    NET_STATISTIC_DEFAULT_REPORTING_INTERVAL_S;
  bool use_reactor =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  std::string SSID_string;
  bool trace_information = false;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
      TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            auto_associate,
                            API_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#endif // ACE_WIN32 || ACE_WIN64
                            UI_definition_file_path,
                            log_to_file,
                            interface_identifier,
                            use_reactor,
//                            statistic_reporting_interval,
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
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1d: initialize logging and/or tracing
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_MessageStack_t* logstack_p = NULL;
  ACE_SYNCH_MUTEX* lock_p = NULL;
  WLANMonitor_UI_GTK_Manager_t* gtk_manager_p =
    WLANMONITOR_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  struct WLANMonitor_UI_GTK_State& state_r =
    const_cast<struct WLANMonitor_UI_GTK_State&> (gtk_manager_p->getR_2 ());
  logstack_p = &state_r.logStack;
  lock_p = &state_r.logStackLock;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // step1d: initialize logging and/or tracing
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
                                        ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                                             ACE_DIRECTORY_SEPARATOR_CHAR)));
  if (!Common_Log_Tools::initializeLogging (ACE::basename (argv_in[0]),    // program name
                                            log_file_name,                 // log file name
                                            true,                          // log to syslog ?
                                            false,                         // trace messages ?
                                            trace_information,             // debug messages ?
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                            (UI_definition_file_path.empty () ? NULL
                                                                              : &logger))) // (ui) logger ?
#elif defined (WXWIDGETS_USE)
                                            NULL))                                         // (ui) logger ?
#else
                                            NULL))                                         // (ui) logger ?
#endif // XXX_USE
#else
                                            NULL))                                         // (ui) logger ?
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
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_SUCCESS;
  } // end IF

  // step1h: initialize UI framework
  struct WLANMonitor_Configuration configuration;
#if defined (GUI_SUPPORT)
  struct WLANMonitor_UI_CBData ui_cb_data;
  ui_cb_data.allowUserRuntimeStatistic = true;
//    (statistic_reporting_interval == 0); // handle SIGUSR1/SIGBREAK
//                                         // iff regular reporting
//                                         // is off
  ui_cb_data.configuration = &configuration;
#if defined (GTK_USE)
  ui_cb_data.UIState = &state_r;

  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  WLANMonitor_GtkBuilderDefinition_t gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_ui_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_ui_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
  if (!UI_definition_file_path.empty ())
    WLANMONITOR_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (ui_cb_data.configuration->GTKConfiguration);
#endif // GTK_USE
#endif // GUI_SUPPORT

  // step1e: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (use_reactor,
//                        (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR),
                        true,
//                        (statistic_reporting_interval == 0), // handle SIGUSR1/SIGBREAK
//                                                             // iff regular reporting
//                                                             // is off
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
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           //use_reactor,
                                           (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR),
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
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
#endif // GUI_SUPPORT
  Test_U_SignalHandler signal_handler (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                       lock_2);

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  // *TODO*: the reasoning here is incomplete
//  bool use_fd_based_reactor = use_reactor;
  bool use_fd_based_reactor = use_reactor;
//      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  use_fd_based_reactor =
    (/*(COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)*/ use_reactor &&
     !(COMMON_EVENT_REACTOR_TYPE == COMMON_REACTOR_WFMO));
#endif // ACE_WIN32 || ACE_WIN64
  bool stack_traces = true;
  bool use_signal_based_proactor = !use_reactor;
//  bool use_signal_based_proactor =
//      (COMMON_EVENT_DEFAULT_DISPATCH != COMMON_EVENT_DISPATCH_REACTOR);
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
    Common_Log_Tools::finalizeLogging ();
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
  do_work (auto_associate,
           API_e,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#endif // ACE_WIN32 || ACE_WIN64
           UI_definition_file_path,
           interface_identifier,
           use_reactor,
//           statistic_reporting_interval,
           SSID_string,
#if defined (GUI_SUPPORT)
           ui_cb_data,
#endif // GUI_SUPPORT
           configuration,
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

  // stop profile timer
  process_profile.stop ();

  // only process profile left to do
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
    Common_Log_Tools::finalizeLogging ();
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

  Common_Signal_Tools::finalize (//COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                 COMMON_SIGNAL_DISPATCH_REACTOR,
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
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
