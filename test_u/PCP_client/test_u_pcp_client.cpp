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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#include "sys/capability.h"
#include "linux/capability.h"
#endif // ACE_WIN32 || ACE_WIN32

#include <iostream>
#include <limits>
#include <regex>
#include <string>

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN32
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_file_tools.h"
#include "common_os_tools.h"

#include "common_event_tools.h"

#include "common_log_tools.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_logger_queue.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_defines.h"
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H
#include "stream_allocatorheap.h"

#include "stream_file_sink.h"

#include "stream_misc_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H
#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

#include "net_client_common_tools.h"
#include "net_client_defines.h"

#include "http_defines.h"

#include "pcp_defines.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_callbacks.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "test_u_common.h"
#include "test_u_defines.h"

#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_connection_stream.h"
#include "test_u_eventhandler.h"
#include "test_u_message.h"
#include "test_u_module_eventhandler.h"
#include "test_u_session_message.h"
#include "test_u_signalhandler.h"

#include "test_u_pcp_client_common.h"
#include "test_u_pcp_client_defines.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("PCPClientStream");

void
do_printUsage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string path =
    Common_File_Tools::getWorkingDirectory ();

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : debug parser [")
            << COMMON_PARSER_DEFAULT_YACC_TRACE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::string configuration_path = path;
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  std::string gtk_rc_file = configuration_path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GTK_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e [PATH]   : Gtk .rc file [\"")
            << gtk_rc_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
#endif // GTK_USE
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
  std::string UI_file = configuration_path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[PATH]]  : UI definition file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
#endif // GUI_SUPPORT
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n[[STRING]]: interface identifier [\"")
            << ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]  : #dispatch threads [")
            << TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     //bool& requestBroadcastReplies_out,
                     bool& debugParser_out,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                     std::string& GtkRcFileName_out,
#endif // GTK_USE
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
                     std::string& UIDefinitonFileName_out,
#endif // GUI_SUPPORT
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
                     //bool& useLoopback_out,
                     //bool& sendRequestOnOffer_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  //requestBroadcastReplies_out = PCP_DEFAULT_FLAGS_BROADCAST;
  debugParser_out = COMMON_PARSER_DEFAULT_YACC_TRACE;
  std::string configuration_path = path;
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GtkRcFileName_out = configuration_path;
  GtkRcFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  GtkRcFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GTK_RC_FILE);
#endif // GTK_USE
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
  UIDefinitonFileName_out = configuration_path;
  UIDefinitonFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitonFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GLADE_FILE);
#endif // GUI_SUPPORT
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  interfaceIdentifier_out =
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    Net_Common_Tools::getDefaultInterface_2 (NET_LINKLAYER_802_3);
#else
    Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
#endif // _WIN32_WINNT_VISTA
#else
  interfaceIdentifier_out =
    ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT_ETHERNET);
#endif // ACE_WIN32 || ACE_WIN64
  //useLoopback_out = NET_INTERFACE_DEFAULT_USE_LOOPBACK;
  //sendRequestOnOffer_out = TEST_U_DEFAULT_PCP_SEND_REQUEST_ON_OFFER;
  useReactor_out =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                              ACE_TEXT ("de:g::lrs:tvx:"),
#else
                              ACE_TEXT ("dg::lrs:tvx:"),
#endif // GTK_USE
#else
                              ACE_TEXT ("dlrs:tvx:"),
#endif // GUI_SUPPORT
#else
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                              ACE_TEXT ("de:g::ln::rs:tvx:"),
#else
                              ACE_TEXT ("dg::ln::rs:tvx:"),
#endif // GTK_USE
#else
                              ACE_TEXT ("dln::rs:tvx:"),
#endif // GUI_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
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
      //case 'b':
      //{
      //  requestBroadcastReplies_out = true;
      //  break;
      //}
      case 'd':
      {
        debugParser_out = true;
        break;
      }
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
      case 'e':
      {
        GtkRcFileName_out = argumentParser.opt_arg ();
        break;
      }
#endif // GTK_USE
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          UIDefinitonFileName_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIDefinitonFileName_out.clear ();
        break;
#endif // GUI_SUPPORT
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      case 'n':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          interfaceIdentifier_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          interfaceIdentifier_out.clear ();
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
      //case 'o':
      //{
      //  useLoopback_out = true;
      //  break;
      //}
      //case 'q':
      //{
      //  sendRequestOnOffer_out = true;
      //  break;
      //}
      case 'r':
      {
        useReactor_out = true;
        break;
      }
      case 's':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
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
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> numberOfDispatchThreads_out;
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

#if defined (VALGRIND_USE)
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif // VALGRIND_USE
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (//bool requestBroadcastReplies_in,
         bool debugParser_in,
#if defined (GUI_SUPPORT)
         const std::string& UIDefinitionFileName_in,
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
         REFGUID interfaceIdentifier_in,
#else
         const std::string& interfaceIdentifier_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
         const std::string& interfaceIdentifier_in,
#endif // ACE_WIN32 || ACE_WIN64
         //bool useLoopback_in,
         //bool sendRequestOnOffer_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         unsigned int numberOfDispatchThreads_in,
         struct PCPClient_Configuration& configuration_in,
#if defined (GUI_SUPPORT)
         struct PCPClient_UI_CBData& CBData_in,
#endif // GUI_SUPPORT
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_U_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

  // step0c: initialize configuration and stream
  configuration_in.dispatch =
      (useReactor_in ? COMMON_EVENT_DISPATCH_REACTOR
                     : COMMON_EVENT_DISPATCH_PROACTOR);

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct PCP_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (configuration_in.allocatorConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  PCPClient_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                                  &heap_allocator,     // heap allocator handle
                                                  true);               // block ?

  Test_U_EventHandler ui_event_handler (
#if defined (GUI_SUPPORT)
                                        &CBData_in);
#else
                                        NULL);
#endif // GUI_SUPPORT
  Test_U_Module_EventHandler_Module event_handler (NULL,
                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  PCPClient_ConnectionManager_t* connection_manager_p =
    PCPCLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  CBData_in.progressData.state = &state_r;
#endif // GTK_USE
#endif // GUI_SUPPORT

  // *********************** parser configuration data *************************
#if defined (_DEBUG)
  configuration_in.parserConfiguration.debugParser = debugParser_in;
  if (debugParser_in)
    configuration_in.parserConfiguration.debugScanner = true;
#endif // _DEBUG
  // *********************** socket configuration data *************************
  ACE_INET_Addr interface_address, gateway_address;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier = interfaceIdentifier_in;
#else
  std::string interface_identifier = interfaceIdentifier_in;
#endif // _WIN32_WINNT_VISTA
#else
  std::string interface_identifier = interfaceIdentifier_in;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (InlineIsEqualGUID (interface_identifier, GUID_NULL))
#else
  if (interface_identifier.empty ())
#endif // _WIN32_WINNT_VISTA
#else
  if (interface_identifier.empty ())
#endif // ACE_WIN32 || ACE_WIN64
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    interface_identifier =
      Net_Common_Tools::getDefaultInterface_2 (NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP);
#else
    interface_identifier =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP);
#endif // _WIN32_WINNT_VISTA
#else
    interface_identifier =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3 | NET_LINKLAYER_802_11 | NET_LINKLAYER_PPP);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (InlineIsEqualGUID (interface_identifier, GUID_NULL))
#else
  if (interface_identifier.empty ())
#endif // _WIN32_WINNT_VISTA
#else
  if (interface_identifier.empty ())
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::getDefaultInterface(), aborting\n")));
    return;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (!Net_Common_Tools::interfaceToIPAddress_2 (interface_identifier,
                                                 interface_address,
                                                 gateway_address))
#else
  if (!Net_Common_Tools::interfaceToIPAddress (interface_identifier,
                                               interface_address,
                                               gateway_address))
#endif // _WIN32_WINNT_VISTA
#else
  if (!Net_Common_Tools::interfaceToIPAddress (interface_identifier,
                                               interface_address,
                                               gateway_address))
#endif // ACE_WIN32 || ACE_WIN64
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress_2(\"%s\"), aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
    return;
  } // end IF
  interface_address.set_port_number (PCP_DEFAULT_CLIENT_PORT,
                                     1);
  ACE_INET_Addr external_address;
  if (!Net_Common_Tools::interfaceToExternalIPAddress (interface_identifier,
                                                       external_address))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_identifier).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), returning\n"),
                ACE_TEXT (interface_identifier.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
    return;
  } // end IF
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  CBData_in.externalAddress = external_address;
  CBData_in.gatewayAddress = gateway_address;
  CBData_in.interfaceAddress = interface_address;
#endif // GTK_USE
#endif // GUI_SUPPORT

  PCPClient_ConnectionConfiguration connection_configuration_io_unicast;
  PCPClient_ConnectionConfiguration connection_configuration_inbound_multicast;
  connection_configuration_io_unicast.allocatorConfiguration =
    &configuration_in.allocatorConfiguration;
  if (useReactor_in)
    ;
  else
    connection_configuration_io_unicast.socketConfiguration.connect =
      true;
  connection_configuration_io_unicast.socketConfiguration.interfaceIdentifier =
    interface_identifier;
  connection_configuration_io_unicast.socketConfiguration.peerAddress =
    gateway_address;
  connection_configuration_io_unicast.socketConfiguration.peerAddress.set_port_number (static_cast<u_short> (PCP_DEFAULT_SERVER_PORT),
                                                                                       1); // encode
  connection_configuration_io_unicast.socketConfiguration.reuseAddress =
    true;
  connection_configuration_io_unicast.statisticReportingInterval =
    statisticReportingInterval_in;
  connection_configuration_io_unicast.messageAllocator = &message_allocator;
  connection_configuration_io_unicast.streamConfiguration =
    &configuration_in.streamConfiguration;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("In/Out"),
                                                                    &connection_configuration_io_unicast));
  Net_ConnectionConfigurationsIterator_t iterator =
    configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("In/Out"));
  ACE_ASSERT (iterator != configuration_in.connectionConfigurations.end ());

  connection_configuration_io_unicast.socketConfiguration.listenAddress =
    interface_address;
  connection_configuration_io_unicast.socketConfiguration.listenAddress.set_port_number (static_cast<u_short> (PCP_DEFAULT_CLIENT_PORT),
                                                                                         1); // encode

  // ********************** stream configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
  struct PCPClient_ModuleHandlerConfiguration modulehandler_configuration;
  struct PCP_StreamConfiguration stream_configuration;
  stream_configuration.cloneModule = true;
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module =
    (!UIDefinitionFileName_in.empty () ? &event_handler
                                       : NULL);
  stream_configuration.printFinalReport = true;

  // ********************** module configuration data **************************
//  struct PCPClient_StreamConfiguration stream_configuration;
  modulehandler_configuration.allocatorConfiguration =
      &configuration_in.allocatorConfiguration;
  modulehandler_configuration.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration.connectionConfigurations =
    &CBData_in.configuration->connectionConfigurations;
  modulehandler_configuration.parserConfiguration =
    &configuration_in.parserConfiguration;
  modulehandler_configuration.printFinalReport = true;
  modulehandler_configuration.protocolConfiguration =
    &configuration_in.protocolConfiguration;
  modulehandler_configuration.statisticReportingInterval =
    (statisticReportingInterval_in ? ACE_Time_Value (statisticReportingInterval_in, 0)
                                   : ACE_Time_Value::zero);
  modulehandler_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;
#if defined (GUI_SUPPORT)
  modulehandler_configuration.subscriber = &ui_event_handler;
  modulehandler_configuration.subscribers = &CBData_in.subscribers;
#if defined (GTK_USE)
  modulehandler_configuration.lock = &state_r.subscribersLock;
#endif // GTK_USE
#endif // GUI_SUPPORT

  stream_configuration.allocatorConfiguration =
      &configuration_in.allocatorConfiguration;
  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);
  PCPClient_StreamConfiguration_t::ITERATOR_T iterator_2 =
    configuration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != configuration_in.streamConfiguration.end ());

  // ********************** protocol configuration data ************************
  //configuration_in.protocolConfiguration.requestBroadcastReplies =
  //    requestBroadcastReplies_in;
  //configuration_in.protocolConfiguration.sendRequestOnOffer =
  //    sendRequestOnOffer_in;

  // ********************* listener configuration data *************************
  connection_configuration_inbound_multicast =
    connection_configuration_io_unicast;
  connection_configuration_inbound_multicast.socketConfiguration.connect = false;
  connection_configuration_inbound_multicast.socketConfiguration.reuseAddress =
    false;
  result =
    connection_configuration_inbound_multicast.socketConfiguration.listenAddress.set (static_cast<u_short> (PCP_DEFAULT_CLIENT_PORT),
                                                                                      ACE_TEXT_ALWAYS_CHAR (NET_ADDRESS_IPV4_MULTICAST),
                                                                                      1,  // encode
                                                                                      AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to set listening address: \"%m\", returning\n")));
    return;
  } // end IF
  connection_configuration_inbound_multicast.socketConfiguration.peerAddress =
    connection_configuration_inbound_multicast.socketConfiguration.listenAddress;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("In_2"),
                                                                    &connection_configuration_inbound_multicast));

  // step0b: initialize event dispatch
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
    return;
  } // end IF
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration = &configuration_in.dispatchConfiguration;

  ACE_Time_Value deadline = ACE_Time_Value::zero;
//  PCPClient_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
//      connection_manager_p;
//  ACE_ASSERT (iconnection_manager_p);
//  int group_id = -1;
  ACE_Time_Value timeout (NET_CONNECTION_DEFAULT_INITIALIZATION_TIMEOUT_S, 0);
  ACE_HANDLE handle = ACE_INVALID_HANDLE;

  // step0c: initialize connection manager
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));

  // step0d: initialize regular (global) statistic reporting
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  struct Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);
  PCP_StatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                            connection_manager_p,
                                            false);
  long timer_id = -1;
  if (statisticReportingInterval_in)
  {
    ACE_Time_Value interval (statisticReportingInterval_in, 0);
    timer_id =
      timer_manager_p->schedule_timer (&statistic_handler,         // event handler handle
                                       NULL,                       // asynchronous completion token
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (timer_id == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));
      return;
    } // end IF
  } // end IF

  // step0c: initialize signal handling
  configuration_in.signalHandlerConfiguration.dispatchState =
    &event_dispatch_state_s;
  //if (useReactor_in)
  //  configuration_in.signalHandlerConfiguration.listener =
  //    TEST_U_LISTENER_SINGLETON::instance ();
  //else
  //  configuration_in.signalHandlerConfiguration.listener =
  //    TEST_U_ASYNCHLISTENER_SINGLETON::instance ();
  configuration_in.signalHandlerConfiguration.statisticReportingHandler =
    connection_manager_p;
  configuration_in.signalHandlerConfiguration.statisticReportingTimerId =
    timer_id;
  configuration_in.signalHandlerConfiguration.stopEventDispatchOnShutdown =
    UIDefinitionFileName_in.empty ();
  if (!signalHandler_in.initialize (configuration_in.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
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
    return;
  } // end IF

  // step1: handle events (signals, incoming connections/data[, timers], ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // [timer events:]
  // - perform (connection) statistic collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

  // step1a: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));
    return;
  } // end IF

  // step1cb: start listening ?
  PCP_ISession_t* isession_p = NULL;
  if (UIDefinitionFileName_in.empty ())
  {
    PCP_IConnection_t* iconnection_p = NULL;

    // listen on unicast port
    iterator =
      configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("In/Out"));
    ACE_ASSERT (iterator != configuration_in.connectionConfigurations.end ());
    PCPClient_UnicastConnector_t connector_2 (true);
    PCPClient_UnicastAsynchConnector_t asynch_connector_2 (true);
    if (useReactor_in)
      handle =
        Net_Client_Common_Tools::connect<PCPClient_UnicastConnector_t> (connector_2,
                                                                        *static_cast<PCPClient_ConnectionConfiguration*> ((*iterator).second),
                                                                        configuration_in.userData,
                                                                        NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress,
                                                                        true,
                                                                        false);
    else
      handle =
        Net_Client_Common_Tools::connect<PCPClient_UnicastAsynchConnector_t> (asynch_connector_2,
                                                                              *static_cast<PCPClient_ConnectionConfiguration*> ((*iterator).second),
                                                                              configuration_in.userData,
                                                                              NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress,
                                                                              true,
                                                                              false);
    if (handle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress).c_str ())));
      Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                 true); // wait ?
      return;
    } // end IF
    iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle));
#else
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (handle));
#endif // ACE_WIN32 || ACE_WIN64
    ACE_ASSERT (iconnection_p);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: listening to (UDP) %s\n"),
                iconnection_p->id (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress).c_str ())));
    isession_p->initialize (*static_cast<PCPClient_ConnectionConfiguration*> ((*iterator).second),
                            iconnection_p);
    iconnection_p->decrease (); iconnection_p = NULL;

    // listen on multicast port
    iterator =
      configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR ("In_2"));
    ACE_ASSERT (iterator != configuration_in.connectionConfigurations.end ());
    PCPClient_McastConnector_t connector_3 (true);
    PCPClient_McastAsynchConnector_t asynch_connector_3 (true);
    if (useReactor_in)
      handle =
        Net_Client_Common_Tools::connect<PCPClient_McastConnector_t> (connector_3,
                                                                      *static_cast<PCPClient_ConnectionConfiguration*> ((*iterator).second),
                                                                      configuration_in.userData,
                                                                      NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress,
                                                                      true,
                                                                      false);
    else
      handle =
        Net_Client_Common_Tools::connect<PCPClient_McastAsynchConnector_t> (asynch_connector_3,
                                                                            *static_cast<PCPClient_ConnectionConfiguration*> ((*iterator).second),
                                                                            configuration_in.userData,
                                                                            NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress,
                                                                            true,
                                                                            false);
    if (handle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to %s, returning\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress).c_str ())));
      connection_manager_p->abort ();
      connection_manager_p->wait ();
      Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                 true); // wait ?
      return;
    } // end IF
    iconnection_p =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      connection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle));
#else
      connection_manager_p->get (static_cast<Net_ConnectionId_t> (handle));
#endif // ACE_WIN32 || ACE_WIN64
    ACE_ASSERT (iconnection_p);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: listening to (UDP) %s\n"),
                iconnection_p->id (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (NET_CONFIGURATION_UDP_CAST ((*iterator).second)->socketConfiguration.listenAddress).c_str ())));
    iconnection_p->decrease (); iconnection_p = NULL;

    // step2: send PCP request
    ACE_INET_Addr internal_address;
    internal_address = interface_address;
    internal_address.set_port_number (HTTP_DEFAULT_SERVER_PORT,
                                      1); // encode
    external_address.set_port_number (HTTP_DEFAULT_SERVER_PORT,
                                      1); // encode

    struct PCP_SessionState& state_r =
        const_cast<struct PCP_SessionState&> (isession_p->getR ());
    state_r.timeStamp = COMMON_TIME_NOW;

    isession_p->map (external_address,
                     internal_address);
  } // end IF

  // *NOTE*: from this point on, clean up any remote connections !

#if defined (GUI_SUPPORT)
  // step1a: start UI event loop ?
  if (!UIDefinitionFileName_in.empty ())
  {
#if defined (GTK_USE)
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    CBData_in.UIState = &state_r;
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    CBData_in.UIState->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
        std::make_pair (UIDefinitionFileName_in, static_cast<GtkBuilder*> (NULL));

    gtk_manager_p->start (NULL);
    ACE_Time_Value one_second (1, 0);
    result = ACE_OS::sleep (one_second);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
    if (!COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
      (*iterator_2).second.second->connection->decrease ();
      connection_manager_p->abort ();
      connection_manager_p->wait ();
      return;
    } // end IF
#endif // GTK_USE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    HWND window_p = ::GetConsoleWindow ();
//    if (!window_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
//      return;
//    } // end IF
    //BOOL was_visible_b = ::ShowWindow (window_p, SW_HIDE);
    //ACE_UNUSED_ARG (was_visible_b);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_USE)
    gtk_manager_p->wait (false);
#endif // GTK_USE
  } // end IF
  else
#endif // GUI_SUPPORT
    Common_Event_Tools::dispatchEvents (event_dispatch_state_s);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  timer_manager_p->stop ();
  if (UIDefinitionFileName_in.empty ())
    (*iterator_2).second.second->connection->decrease ();
  connection_manager_p->wait ();
  if (!UIDefinitionFileName_in.empty ())
    Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                               true); // wait ?

//  if (!UIDefinitionFileName_in.empty ())
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (true);
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

//  result = file_writer.close (ACE_Module_Base::M_DELETE_NONE);
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
//                file_writer.name ()));
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (false,  // COM ?
                            true);  // RNG ?
#else
  Common_Tools::initialize (true); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64

  std::string path =
    Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  //bool request_broadcast_replies = PCP_DEFAULT_FLAGS_BROADCAST;
  bool debug_parser = COMMON_PARSER_DEFAULT_YACC_TRACE;
  std::string configuration_path = path;
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  std::string gtk_rc_file = configuration_path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GTK_RC_FILE);
#endif // GTK_USE
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
  std::string ui_definition_file = configuration_path;
  ui_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GLADE_FILE);
#endif // GUI_SUPPORT
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct _GUID interface_identifier = GUID_NULL;
#else
  std::string interface_identifier;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
  std::string interface_identifier;
#endif // ACE_WIN32 || ACE_WIN64
  //bool use_loopback = NET_INTERFACE_DEFAULT_USE_LOOPBACK;
  //bool send_request_on_offer =
  //    TEST_U_DEFAULT_PCP_SEND_REQUEST_ON_OFFER;
  bool use_reactor =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            //request_broadcast_replies,
                            debug_parser,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                            gtk_rc_file,
#endif // GTK_USE
#endif // GUI_SUPPORT
#if defined (GUI_SUPPORT)
                            ui_definition_file,
#endif // GUI_SUPPORT
                            log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                            interface_identifier,
#else
                            interface_identifier,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
                            interface_identifier,
#endif // ACE_WIN32 || ACE_WIN64
                            //use_loopback,
                            //send_request_on_offer,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            print_version_and_exit,
                            number_of_dispatch_threads))
  {
    // make 'em learn
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

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_U_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  if (
#if defined (GUI_SUPPORT)
      (ui_definition_file.empty () ||
       !Common_File_Tools::isReadable (ui_definition_file)) ||
#if defined (GTK_USE)
      (!gtk_rc_file.empty () &&
       !Common_File_Tools::isReadable (gtk_rc_file))        ||
#endif // GTK_USE
#else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      InlineIsEqualGUID (interface_identifier, GUID_NULL)   ||
#else
      interface_identifier.empty ()                         ||
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
      interface_identifier.empty ()                         ||
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT
      //(use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool)
     false)
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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_Logger_Queue_t logger;
  logger.initialize (&state_r.logQueue,
                     &state_r.logQueueLock);
#endif // GTK_USE
#endif // GUI_SUPPORT
  std::string log_file_name;
  if (log_to_file)
  {
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                        ACE::basename (argv_in[0]));
    if (log_file_name.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Log_Tools::getLogFilename(), aborting\n")));

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
  } // end IF
  if (!Common_Log_Tools::initialize (ACE::basename (argv_in[0]),               // program name
                                     log_file_name,                            // log file name
                                     false,                                    // log to syslog ?
                                     false,                                    // trace messages ?
                                     trace_information,                        // debug messages ?
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                     NULL))                                    // (ui) logger ?
//                                            (ui_definition_file.empty () ? NULL
//                                                                         : &logger))) // (ui) logger ?
#elif defined (WXWIDGETS_USE)
                                     NULL))                                    // (ui) logger ?
#else
                                     NULL))                                    // (ui) logger ?
#endif // XXX_USE
#else
                                     NULL))                                    // (ui) logger ?
#endif // GUI_SUPPORT
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

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (true, // allow SIGUSR1/SIGBREAK
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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
#endif // GUI_SUPPORT
  Test_U_SignalHandler signal_handler;

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_OS_Tools::setResourceLimits (false, // file descriptors
                                           true,  // stack traces
                                           true)) // pending signals
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

  struct PCPClient_Configuration configuration_s;
#if defined (GUI_SUPPORT)
  // step1h: initialize GLIB / G(D|T)K[+] / GNOME
  struct PCPClient_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration_s;
#if defined (GTK_USE)
  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
  if (!gtk_rc_file.empty ())
    ui_cb_data.configuration->GTKConfiguration.RCFiles.push_back (gtk_rc_file);
  if (!ui_definition_file.empty ())
    if (!COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (ui_cb_data.configuration->GTKConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager::initialize(), aborting\n")));

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
#endif // GTK_USE
#endif // GUI_SUPPORT

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (//request_broadcast_replies,
           debug_parser,
#if defined (GUI_SUPPORT)
           ui_definition_file,
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
           interface_identifier,
#else
           interface_identifier,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
#else
           interface_identifier,
#endif // ACE_WIN32 || ACE_WIN64
           //use_loopback,
           //send_request_on_offer,
           use_reactor,
           statistic_reporting_interval,
           number_of_dispatch_threads,
           configuration_s,
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
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
