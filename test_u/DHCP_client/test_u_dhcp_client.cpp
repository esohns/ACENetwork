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
#include <regex>
#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#elif defined (ACE_LINUX)
#include <sys/capability.h>
#include <linux/capability.h>
#endif

#include <ace/Get_Opt.h>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <ace/Init_ACE.h>
#endif
#include <ace/Log_Msg.h>
#include <ace/Profile_Timer.h>
#include <ace/Sig_Handler.h>
#include <ace/Signal.h>
#include <ace/Synch.h>
#include <ace/Version.h>

#ifdef LIBACENETWORK_ENABLE_VALGRIND_SUPPORT
#include <valgrind/valgrind.h>
#endif

#include "common.h"
#include "common_file_tools.h"
#include "common_logger.h"
#include "common_tools.h"

#include "common_ui_defines.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_allocatorheap.h"
#include "stream_macros.h"

#include "stream_file_sink.h"

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

#include "net_common_tools.h"

#include "dhcp_defines.h"

#include "test_u_callbacks.h"
#include "test_u_common.h"
#include "test_u_connection_common.h"
#include "test_u_connection_manager_common.h"
#include "test_u_connection_stream.h"
#include "test_u_defines.h"
#include "test_u_eventhandler.h"
#include "test_u_message.h"
#include "test_u_module_eventhandler.h"
#include "test_u_session_message.h"
#include "test_u_signalhandler.h"

#include "test_u_dhcp_client_common.h"

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("DHCP_client");
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-b          : reuqest broadcast replies [")
            << DHCP_DEFAULT_FLAGS_BROADCAST
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d          : debug parser [")
            << DHCP_DEFAULT_YACC_TRACE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::string configuration_path = path;
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  std::string gtk_rc_file = configuration_path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GTK_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e [PATH]   : Gtk .rc file [\"")
            << gtk_rc_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::string output_file = path;
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_OUTPUT_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f[[PATH]]  : (output) file name [")
            << output_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string UI_file = configuration_path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[PATH]]  : UI definition file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n[[STRING]]: network interface [\"")
            << ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  // *TODO*: this doesn't really make sense (see '-n' option)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o          : use loopback [")
            << NET_INTERFACE_DEFAULT_USE_LOOPBACK
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p          : use thread pool [")
            << NET_EVENT_USE_THREAD_POOL
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-q          : send request on offer [")
            << TEST_U_DEFAULT_DHCP_SEND_REQUEST_ON_OFFER
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << NET_EVENT_USE_REACTOR
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL
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
                     bool& requestBroadcastReplies_out,
                     bool& debugParser_out,
                     std::string& GtkRcFileName_out,
                     std::string& fileName_out,
                     std::string& UIDefinitonFileName_out,
                     bool& logToFile_out,
                     std::string& interface_out,
                     bool& useLoopback_out,
                     bool& useThreadPool_out,
                     bool& sendRequestOnOffer_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("DHCP_client");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  requestBroadcastReplies_out = DHCP_DEFAULT_FLAGS_BROADCAST;
  debugParser_out = DHCP_DEFAULT_YACC_TRACE;
  std::string configuration_path = path;
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  GtkRcFileName_out = configuration_path;
  GtkRcFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  GtkRcFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GTK_RC_FILE);
  fileName_out = path;
  fileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  fileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_OUTPUT_FILE);
  UIDefinitonFileName_out = configuration_path;
  UIDefinitonFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitonFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GLADE_FILE);
  logToFile_out = false;
  interface_out = ACE_TEXT_ALWAYS_CHAR (NET_INTERFACE_DEFAULT);
  useLoopback_out = NET_INTERFACE_DEFAULT_USE_LOOPBACK;
  useThreadPool_out = NET_EVENT_USE_THREAD_POOL;
  sendRequestOnOffer_out = TEST_U_DEFAULT_DHCP_SEND_REQUEST_ON_OFFER;
  useReactor_out = NET_EVENT_USE_REACTOR;
  statisticReportingInterval_out = STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out = false;
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("bde:f::g::ln::opqrs:tvx:"),
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
      case 'b':
      {
        requestBroadcastReplies_out = true;
        break;
      }
      case 'd':
      {
        debugParser_out = true;
        break;
      }
      case 'e':
      {
        GtkRcFileName_out = argumentParser.opt_arg ();
        break;
      }
      case 'f':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          fileName_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          fileName_out.clear ();
        break;
      }
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          UIDefinitonFileName_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          UIDefinitonFileName_out.clear ();
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'n':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          interface_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          interface_out.clear ();
        break;
      }
      case 'o':
      {
        useLoopback_out = true;
        break;
      }
      case 'p':
      {
        useThreadPool_out = true;
        break;
      }
      case 'q':
      {
        sendRequestOnOffer_out = true;
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
  STREAM_TRACE (ACE_TEXT ("::do_initializeSignals"));

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

#if defined (LIBACENETWORK_ENABLE_VALGRIND_SUPPORT)
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif
#endif
}

void
do_work (bool requestBroadcastReplies_in,
         bool debugParser_in,
         const std::string& fileName_in,
         const std::string& UIDefinitionFileName_in,
         const std::string& networkInterface_in,
         bool useLoopback_in,
         bool useThreadPool_in,
         bool sendRequestOnOffer_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         unsigned int numberOfDispatchThreads_in,
         Test_U_DHCPClient_GTK_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_U_SignalHandler& signalHandler_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DEBUG)
//  Common_Tools::printCapabilities ();
  if (!Common_Tools::hasCapability (CAP_NET_BIND_SERVICE))
    if (!Common_Tools::setCapability (CAP_NET_BIND_SERVICE))
    {
      char* capability_name_string_p = ::cap_to_name (CAP_NET_BIND_SERVICE);
      if (!capability_name_string_p)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::cap_to_name(%d): \"%m\", continuing\n"),
                    CAP_NET_BIND_SERVICE));

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_Tools::setCapability(\"%s\"): \"%m\", returning\n"),
                  ACE_TEXT (capability_name_string_p)));

      // clean up
      result = ::cap_free (capability_name_string_p);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ::cap_free(): \"%m\", continuing\n")));

      return;
    } // end IF
  ACE_ASSERT (Common_Tools::hasCapability (CAP_NET_BIND_SERVICE));
//  Common_Tools::printPriviledges ();
#endif
#endif

  // step0b: initialize random number generator
  Common_Tools::initialize ();

  // step0c: initialize configuration and stream
  Test_U_DHCPClient_Configuration configuration;
  configuration.userData.configuration = &configuration;
  configuration.userData.streamConfiguration =
      &configuration.streamConfiguration;
  configuration.useReactor = useReactor_in;

//  Stream_Module_t* module_p = NULL;
//  Test_U_Module_FileWriter_Module file_writer (ACE_TEXT_ALWAYS_CHAR ("FileWriter"),
//                                               NULL,
//                                               true);
//  module_p = &file_writer;
//  Test_U_IModuleHandler_t* module_handler_p =
//    dynamic_cast<Test_U_IModuleHandler_t*> (module_p->writer ());
//  if (!module_handler_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: dynamic_cast<Test_U_IModuleHandler_t>(0x%@) failed, returning\n"),
//                module_p->name (),
//                module_p->writer ()));
//    return;
//  } // end IF

  Stream_AllocatorHeap_T<Test_U_AllocatorConfiguration> heap_allocator;
  Test_U_MessageAllocator_t message_allocator (TEST_U_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,     // heap allocator handle
                                               true);               // block ?

  Test_U_EventHandler ui_event_handler (&CBData_in);
  Test_U_Module_EventHandler_Module event_handler (ACE_TEXT_ALWAYS_CHAR ("EventHandler"),
                                                   NULL,
                                                   true);
  //Test_U_Stream_t stream;
  //Test_U_AsynchStream_t asynch_stream;
  //Test_U_StreamBase_t* stream_p = &stream;
  //if (!useReactor_in) stream_p = &asynch_stream;
  Test_U_Module_EventHandler* event_handler_p =
      dynamic_cast<Test_U_Module_EventHandler*> (event_handler.writer ());
  if (!event_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Test_U_Module_EventHandler> failed, returning\n")));
    return;
  } // end IF
  event_handler_p->initialize (&CBData_in.subscribers,
                               &CBData_in.subscribersLock);
  event_handler_p->subscribe (&ui_event_handler);

  Test_U_ConnectionManager_t* connection_manager_p =
    TEST_U_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);

  // *********************** socket configuration data ************************
  // *IMPORTANT NOTE*: (global) UDP broadcast appears to be broken on
  //                   Windows 7 and above (see e.g.:
  //                   - http://serverfault.com/questions/72112/how-to-alter-the-global-broadcast-address-255-255-255-255-behavior-on-windows
  // *WORKAROUND*: make sure the 'metric' value of the 'target' interface
  //               (i.e. the one that shares the subnet with the target DHCP
  //               server) is the lowest (run 'route print' and inspect the
  //               (global) broadcast route entries) to ensure that broadcast
  //               packets are (at least) sent out on the correct subnet
  result =
    configuration.socketConfiguration.address.set (static_cast<u_short> (DHCP_DEFAULT_SERVER_PORT),
                                                   static_cast<ACE_UINT32> (INADDR_BROADCAST),
                                                   1,
                                                   0);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
    return;
  } // end IF
  configuration.socketConfiguration.networkInterface = networkInterface_in;
  configuration.socketConfiguration.useLoopBackDevice = useLoopback_in;
  configuration.socketConfiguration.writeOnly = true;
  // ******************** socket handler configuration data *******************
  configuration.socketHandlerConfiguration.listenerConfiguration =
    &configuration.listenerConfiguration;
  configuration.socketHandlerConfiguration.messageAllocator =
    &message_allocator;
  configuration.socketHandlerConfiguration.socketConfiguration =
    &configuration.socketConfiguration;
  configuration.socketHandlerConfiguration.statisticReportingInterval =
    statisticReportingInterval_in;
  configuration.socketHandlerConfiguration.userData =
    &configuration.userData;

  // ********************** stream configuration data **************************
  configuration.streamConfiguration.cloneModule = true;
  configuration.streamConfiguration.messageAllocator = &message_allocator;
  configuration.streamConfiguration.module =
    (!UIDefinitionFileName_in.empty () ? &event_handler
                                       : NULL);
  configuration.streamConfiguration.moduleConfiguration =
    &configuration.moduleConfiguration;
  configuration.streamConfiguration.moduleHandlerConfiguration =
    &configuration.moduleHandlerConfiguration;
  configuration.streamConfiguration.printFinalReport = true;
  configuration.streamConfiguration.statisticReportingInterval =
      statisticReportingInterval_in;

  // ********************** module configuration data **************************
  configuration.moduleConfiguration.streamConfiguration =
    &configuration.streamConfiguration;

  configuration.moduleHandlerConfiguration.protocolConfiguration =
    &configuration.protocolConfiguration;
  configuration.moduleHandlerConfiguration.streamConfiguration =
    &configuration.streamConfiguration;
  configuration.moduleHandlerConfiguration.configuration = &configuration;
  configuration.moduleHandlerConfiguration.traceParsing = debugParser_in;
  if (debugParser_in)
    configuration.moduleHandlerConfiguration.traceScanning = true;
  configuration.moduleHandlerConfiguration.targetFileName = fileName_in;
  configuration.moduleHandlerConfiguration.socketConfiguration =
    &configuration.socketConfiguration;
  configuration.moduleHandlerConfiguration.socketHandlerConfiguration =
    &configuration.socketHandlerConfiguration;

  // ********************** protocol configuration data ************************
  configuration.protocolConfiguration.requestBroadcastReplies =
      requestBroadcastReplies_in;
  configuration.protocolConfiguration.sendRequestOnOffer =
      sendRequestOnOffer_in;

  // ********************* listener configuration data ************************
  if (useLoopback_in)
    result =
        configuration.listenerConfiguration.address.set (DHCP_DEFAULT_CLIENT_PORT,
                                                         ACE_LOCALHOST,
                                                         1,
                                                         ACE_ADDRESS_FAMILY_INET);
  else if (!networkInterface_in.empty ())
  {
    if (!Net_Common_Tools::interface2IPAddress (networkInterface_in,
                                                configuration.listenerConfiguration.address))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::interface2IPAddress(), continuing\n")));
      result = -1;
    } // end IF
    configuration.listenerConfiguration.address.set_port_number (DHCP_DEFAULT_CLIENT_PORT,
                                                                 1);
    result = 0;
  } // end ELSE IF
  else
    result =
        configuration.listenerConfiguration.address.set (static_cast<u_short> (DHCP_DEFAULT_CLIENT_PORT),
                                                         static_cast<ACE_UINT32> (INADDR_ANY));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to set listening address: \"%m\", returning\n")));
    return;
  } // end IF

  if (requestBroadcastReplies_in)
  {
    result =
      configuration.listenerConfiguration.address.set (static_cast<u_short> (DHCP_DEFAULT_CLIENT_PORT),
                                                       static_cast<ACE_UINT32> (INADDR_NONE));
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to set listening address: \"%m\", returning\n")));
      return;
    } // end IF
  } // end IF

  //configuration.listenerConfiguration.connectionManager =
  //  connection_manager_p;
  configuration.listenerConfiguration.messageAllocator = &message_allocator;
  configuration.listenerConfiguration.socketHandlerConfiguration =
    &configuration.socketHandlerConfiguration;
  configuration.listenerConfiguration.statisticReportingInterval =
    statisticReportingInterval_in;
  configuration.listenerConfiguration.useLoopBackDevice = useLoopback_in;

  // step0b: initialize event dispatch
  struct Common_DispatchThreadData thread_data;
  thread_data.numberOfDispatchThreads = numberOfDispatchThreads_in;
  thread_data.useReactor = useReactor_in;
  if (!Common_Tools::initializeEventDispatch (thread_data.useReactor,
                                              useThreadPool_in,
                                              thread_data.numberOfDispatchThreads,
                                              thread_data.proactorType,
                                              thread_data.reactorType,
                                              configuration.streamConfiguration.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF

  ACE_Time_Value deadline = ACE_Time_Value::zero;
  Test_U_IConnector_t* iconnector_p = NULL;
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  Test_U_ConnectionManager_t::INTERFACE_T* iconnection_manager_p =
      connection_manager_p;
  ACE_ASSERT (iconnection_manager_p);
  int group_id = -1;
  ACE_Time_Value timeout (NET_CLIENT_DEFAULT_INITIALIZATION_TIMEOUT, 0);

  Test_U_IConnection_t* iconnection_p = NULL;
  Test_U_ISocketConnection_t* isocket_connection_p = NULL;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
//  Test_U_MessageData* message_data_p = NULL;
//  Test_U_MessageData_t* message_data_container_p = NULL;
  Test_U_Message* message_p = NULL;

  // step0c: initialize connection manager
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  connection_manager_p->set (configuration,
                             &configuration.userData);

  // step0d: initialize regular (global) statistic reporting
  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start ();
  Stream_StatisticHandler_Reactor_t statistic_handler (ACTION_REPORT,
                                                       connection_manager_p,
                                                       false);
  //Stream_StatisticHandler_Proactor_t statistic_handler_proactor (ACTION_REPORT,
  //                                                               connection_manager_p,
  //                                                               false);
  long timer_id = -1;
  if (statisticReportingInterval_in)
  {
    ACE_Event_Handler* handler_p = &statistic_handler;
    ACE_Time_Value interval (statisticReportingInterval_in, 0);
    timer_id =
      timer_manager_p->schedule_timer (handler_p,                  // event handler
                                       NULL,                       // ACT
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
  CBData_in.configuration = &configuration;
  //if (useReactor_in)
  //  CBData_in.configuration->signalHandlerConfiguration.listener =
  //    TEST_U_LISTENER_SINGLETON::instance ();
  //else
  //  CBData_in.configuration->signalHandlerConfiguration.listener =
  //    TEST_U_ASYNCHLISTENER_SINGLETON::instance ();
  CBData_in.configuration->signalHandlerConfiguration.statisticReportingHandler =
    connection_manager_p;
  CBData_in.configuration->signalHandlerConfiguration.statisticReportingTimerID =
    timer_id;
  CBData_in.configuration->signalHandlerConfiguration.useReactor =
    useReactor_in;
  if (!signalHandler_in.initialize (configuration.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    return;
  } // end IF
  if (!Common_Tools::initializeSignals (signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeSignals(), returning\n")));
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
  if (!Common_Tools::startEventDispatch (thread_data,
                                         group_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));
    return;
  } // end IF

  // step1b: connect (broadcast)
  // *NOTE*: the DHCP server address may not be known at this stage, so
  //         connection to the unicast address is handled by the discovery
  //         module
  configuration.streamConfiguration.module = NULL;
  connection_manager_p->set (configuration,
                             &configuration.userData);

  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result =
      configuration.socketConfiguration.address.addr_to_string (buffer,
                                                                sizeof (buffer),
                                                                1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

  //Test_U_OutboundConnector_t connector (connection_manager_p,
  //                                      configuration.socketHandlerConfiguration.statisticReportingInterval);
  //Test_U_OutboundAsynchConnector_t asynch_connector (connection_manager_p,
  //                                                   configuration.socketHandlerConfiguration.statisticReportingInterval);
  Test_U_OutboundConnectorBcast_t connector (connection_manager_p,
                                             configuration.socketHandlerConfiguration.statisticReportingInterval);
  Test_U_OutboundAsynchConnectorBcast_t asynch_connector (connection_manager_p,
                                                          configuration.socketHandlerConfiguration.statisticReportingInterval);
  if (useReactor_in)
    iconnector_p = &connector;
  else
    iconnector_p = &asynch_connector;
  if (!iconnector_p->initialize (configuration.socketHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector, returning\n")));
    return;
  } // end IF
  handle =
    iconnector_p->connect (configuration.socketConfiguration.address);
  if (handle == ACE_INVALID_HANDLE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\", returning\n"),
                buffer));
    return;
  } // end IF
  if (iconnector_p->useReactor ())
    configuration.moduleHandlerConfiguration.broadcastConnection =
        connection_manager_p->get (handle);
  else
  {
    // step1: wait for the connection to register with the manager
    // *TODO*: avoid tight loop here
    deadline = (COMMON_TIME_NOW +
                ACE_Time_Value (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT,
                                0));
    //result = ACE_OS::sleep (timeout);
    //if (result == -1)
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
    //              &timeout));
    do
    {
      configuration.moduleHandlerConfiguration.broadcastConnection =
          connection_manager_p->get (configuration.socketConfiguration.address);
      if (configuration.moduleHandlerConfiguration.broadcastConnection) break;
    } while (COMMON_TIME_NOW < deadline);
  } // end IF
  if (!configuration.moduleHandlerConfiguration.broadcastConnection)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\", returning\n"),
                buffer));
    return;
  } // end IF
  // step1b: wait for the connection to finish initializing
  // *TODO*: avoid tight loop here
  deadline = COMMON_TIME_NOW + timeout;
  do
  {
    status =
        configuration.moduleHandlerConfiguration.broadcastConnection->status ();
    if (status == NET_CONNECTION_STATUS_OK) break;
  } while (COMMON_TIME_NOW < deadline);
  if (status != NET_CONNECTION_STATUS_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connection to \"%s\" (status was: %d), returning\n"),
                buffer,
                status));
    return;
  } // end IF
  // step1c: wait for the connection stream to finish initializing
  isocket_connection_p =
    dynamic_cast<Test_U_ISocketConnection_t*> (configuration.moduleHandlerConfiguration.broadcastConnection);
  if (!isocket_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Test_U_ISocketConnection_t>(0x%@), returning\n"),
                configuration.moduleHandlerConfiguration.broadcastConnection));
    return;
  } // end IF
  isocket_connection_p->wait (STREAM_STATE_RUNNING,
                              NULL); // <-- block
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%d: connected to \"%s\"...\n"),
              configuration.moduleHandlerConfiguration.broadcastConnection->id (),
              buffer));

  // step1ca: reinitialize connection manager
  configuration.socketConfiguration.writeOnly = false;
  configuration.streamConfiguration.module =
    (!UIDefinitionFileName_in.empty () ? &event_handler
                                       : NULL);
  //connection_manager_p->set (configuration,
  //                           &configuration.userData);

  // step1cb: start listening ?
  if (UIDefinitionFileName_in.empty ())
  {
    Test_U_Connector_t connector_2 (iconnection_manager_p,
                                    configuration.streamConfiguration.statisticReportingInterval);
    Test_U_AsynchConnector_t asynch_connector_2 (iconnection_manager_p,
                                                 configuration.streamConfiguration.statisticReportingInterval);
    if (useReactor_in)
      iconnector_p = &connector_2;
    else
      iconnector_p = &asynch_connector_2;
    if (!iconnector_p->initialize (configuration.socketHandlerConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));

      // clean up
      connection_manager_p->abort ();

      return;
    } // end IF

    // connect
    // *********************** socket configuration data ***********************
    result =
      configuration.listenerConfiguration.address.addr_to_string (buffer,
                                                                  sizeof (buffer),
                                                                  1);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    // *TODO*: support one-thread operation by scheduling a signal and manually
    //         running the dispatch loop for a limited time...
    configuration.handle =
        iconnector_p->connect (configuration.listenerConfiguration.address);
    if (configuration.handle == ACE_INVALID_HANDLE)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to \"%s\", returning\n"),
                  buffer));

      // clean up
      iconnector_p->abort ();
      connection_manager_p->abort ();

      return;
    } // end IF
    if (useReactor_in)
      iconnection_p = connection_manager_p->get (configuration.handle);
    else
    {
      configuration.handle = ACE_INVALID_HANDLE;

      // *TODO*: avoid tight loop here
      ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
      //result = ACE_OS::sleep (timeout);
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
      //              &timeout));
      deadline = COMMON_TIME_NOW + timeout;
      //CBData_in.connection = NULL;
      do
      {
        iconnection_p =
            connection_manager_p->get (configuration.listenerConfiguration.address);
        if (iconnection_p)
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          configuration.handle =
              reinterpret_cast<ACE_HANDLE> (iconnection_p->id ());
#else
          configuration.handle =
              static_cast<ACE_HANDLE> (iconnection_p->id ());
#endif
          iconnection_p->decrease ();
          break;
        } // end IF
      } while (COMMON_TIME_NOW < deadline);
    } // end IF
    if (!iconnection_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to \"%s\", returning\n"),
                  buffer));

      // clean up
      iconnector_p->abort ();
      connection_manager_p->abort ();

      return;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%d: listening to (UDP) \"%s\"...\n"),
                iconnection_p->id (),
                buffer));

    // step2: send DHCP request
//    ACE_NEW_NORETURN (message_data_p,
//                      Test_U_MessageData ());
//    if (!message_data_p)
//    {
//      ACE_DEBUG ((LM_CRITICAL,
//                  ACE_TEXT ("failed to allocate memory, returning\n")));
//      goto clean_up;
//    } // end IF
//    ACE_NEW_NORETURN (message_data_p->DHCPRecord,
//                      DHCP_Record ());
//    if (!message_data_p->DHCPRecord)
//    {
//      ACE_DEBUG ((LM_CRITICAL,
//                  ACE_TEXT ("failed to allocate memory, returning\n")));

//      // clean up
//      delete message_data_p;

//      goto clean_up;
//    } // end IF
//    // *IMPORTANT NOTE*: fire-and-forget API (message_data_p)
//    ACE_NEW_NORETURN (message_data_container_p,
//                      Test_U_MessageData_t (message_data_p,
//                                            true));
//    if (!message_data_container_p)
//    {
//      ACE_DEBUG ((LM_CRITICAL,
//                  ACE_TEXT ("failed to allocate memory, returning\n")));

//      // clean up
//      delete message_data_p;

//      goto clean_up;
//    } // end IF
allocate:
    message_p =
        static_cast<Test_U_Message*> (message_allocator.malloc (configuration.socketHandlerConfiguration.PDUSize));
    // keep retrying ?
    if (!message_p && !message_allocator.block ())
      goto allocate;
    if (!message_p)
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Test_U_Message: \"%m\", returning\n")));

      // clean up
//      message_data_container_p->decrease ();
      connection_manager_p->abort ();

      return;
    } // end IF
    DHCP_Record DHCP_record;
    DHCP_record.op = DHCP_Codes::DHCP_OP_REQUEST;
    DHCP_record.htype = DHCP_FRAME_HTYPE;
    DHCP_record.hlen = DHCP_FRAME_HLEN;
    DHCP_record.xid = DHCP_Tools::generateXID ();
    if (configuration.protocolConfiguration.requestBroadcastReplies)
      DHCP_record.flags = DHCP_FLAGS_BROADCAST;
    if (!Net_Common_Tools::interface2MACAddress (configuration.socketConfiguration.networkInterface,
                                                 DHCP_record.chaddr))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Net_Common_Tools::interface2MACAddress(), returning\n")));

      // clean up
      //    message_data_container_p->decrease ();
      connection_manager_p->abort ();

      return;
    } // end IF
    // *TODO*: support optional options:
    //         - 'requested IP address'    (50)
    //         - 'IP address lease time'   (51)
    //         - 'overload'                (52)
    char message_type = DHCP_Codes::DHCP_MESSAGE_DISCOVER;
    DHCP_record.options.insert (std::make_pair (DHCP_Codes::DHCP_OPTION_DHCP_MESSAGETYPE,
                                                std::string (1, message_type)));
    //         - 'parameter request list'  (55) [include in all subsequent messages]
    //         - 'message'                 (56)
    //         - 'maximum message size'    (57)
    //         - 'vendor class identifier' (60)
    //         - 'client identifier'       (61)
    // *IMPORTANT NOTE*: fire-and-forget API (message_data_container_p)
    //  message_p->initialize (message_data_container_p,
    message_p->initialize (DHCP_record,
                           NULL);

    Test_U_ISocketConnection_t* isocket_connection_p =
      dynamic_cast<Test_U_ISocketConnection_t*> (configuration.moduleHandlerConfiguration.broadcastConnection);
    ACE_ASSERT (isocket_connection_p);

    Test_U_ConnectionState& state_r =
        const_cast<Test_U_ConnectionState&> (isocket_connection_p->state ());
    state_r.timeStamp = COMMON_TIME_NOW;
    state_r.xid = DHCP_record.xid;

    Test_U_OutboundConnectionStream& stream_r =
        const_cast<Test_U_OutboundConnectionStream&> (isocket_connection_p->stream ());
    const Test_U_StreamSessionData_t* session_data_container_p =
        stream_r.get ();
    ACE_ASSERT (session_data_container_p);
    Test_U_StreamSessionData& session_data_r =
        const_cast<Test_U_StreamSessionData&> (session_data_container_p->get ());
    session_data_r.timeStamp = state_r.timeStamp;
    session_data_r.xid = DHCP_record.xid;

    ACE_Message_Block* message_block_p = message_p;
    isocket_connection_p->send (message_block_p);
  } // end IF

  // *NOTE*: from this point on, clean up any remote connections !

  // step1a: start GTK event loop ?
  if (!UIDefinitionFileName_in.empty ())
  {
    CBData_in.finalizationHook = idle_finalize_UI_cb;
    CBData_in.initializationHook = idle_initialize_UI_cb;
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    CBData_in.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
        std::make_pair (UIDefinitionFileName_in, static_cast<GtkBuilder*> (NULL));
    CBData_in.userData = &CBData_in;

    //CBData_in.stream = stream_p;

    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->start ();
    ACE_Time_Value one_second (1, 0);
    result = ACE_OS::sleep (one_second);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(): \"%m\", continuing\n")));
    if (!COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));

      // clean up
      connection_manager_p->abort ();

      return;
    } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = ::GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
      return;
    } // end IF
    //BOOL was_visible_b = ::ShowWindow (window_p, SW_HIDE);
    //ACE_UNUSED_ARG (was_visible_b);
#endif

    result = COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_Base::wait (): \"%m\", continuing\n")));
  } // end IF

  Common_Tools::dispatchEvents (useReactor_in,
                                group_id);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  timer_manager_p->stop ();
  connection_manager_p->stop ();
  connection_manager_p->wait ();
  Common_Tools::finalizeEventDispatch (useReactor_in,
                                       !useReactor_in,
                                       group_id);

  if (configuration.moduleHandlerConfiguration.broadcastConnection)
  {
//    configuration.moduleHandlerConfiguration.broadcastConnection->close ();
    configuration.moduleHandlerConfiguration.broadcastConnection->decrease ();
  } // end IF
  if (iconnection_p)
  {
//    iconnection_p->close ();
    iconnection_p->decrease ();
  } // end IF

  if (!UIDefinitionFileName_in.empty ())
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (true);
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
  STREAM_TRACE (ACE_TEXT ("::do_printVersion"));

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
           ACE_TCHAR* argv_in[])
{
  STREAM_TRACE (ACE_TEXT ("::main"));

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
  Common_Tools::initialize ();

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  std::string path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("..");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("test_u");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR ("DHCP_client");
#endif // #ifdef DEBUG_DEBUGGER

  // step1a set defaults
  bool request_broadcast_replies = DHCP_DEFAULT_FLAGS_BROADCAST;
  bool debug_parser = DHCP_DEFAULT_YACC_TRACE;
  std::string configuration_path = path;
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path +=
      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
  std::string gtk_rc_file = configuration_path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GTK_RC_FILE);
  std::string output_file = path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_OUTPUT_FILE);
  std::string ui_definition_file = configuration_path;
  ui_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_file += ACE_TEXT_ALWAYS_CHAR (TEST_U_DEFAULT_GLADE_FILE);
  bool log_to_file = false;
  std::string interface;
  bool use_loopback = NET_INTERFACE_DEFAULT_USE_LOOPBACK;
  bool use_thread_pool = NET_EVENT_USE_THREAD_POOL;
  bool send_request_on_offer =
      TEST_U_DEFAULT_DHCP_SEND_REQUEST_ON_OFFER;
  bool use_reactor = NET_EVENT_USE_REACTOR;
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  bool trace_information = false;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    TEST_U_DEFAULT_NUMBER_OF_DISPATCHING_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            request_broadcast_replies,
                            debug_parser,
                            gtk_rc_file,
                            output_file,
                            ui_definition_file,
                            log_to_file,
                            interface,
                            use_loopback,
                            use_thread_pool,
                            send_request_on_offer,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            print_version_and_exit,
                            number_of_dispatch_threads))
  {
    // make 'em learn...
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, finalize ACE...
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
  if (TEST_U_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  if (use_reactor                      &&
      (number_of_dispatch_threads > 1) &&
      !use_thread_pool)
  { // *NOTE*: see also: man (2) select
    // *TODO*: verify this for MS Windows based systems
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("the select()-based reactor is not reentrant, using the thread-pool reactor instead...\n")));
    use_thread_pool = true;
  } // end IF
  if ((ui_definition_file.empty () && interface.empty ())                  ||
      (!ui_definition_file.empty () &&
       !Common_File_Tools::isReadable (ui_definition_file))                ||
      (!gtk_rc_file.empty () &&
       !Common_File_Tools::isReadable (gtk_rc_file))                       ||
      (use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  if (number_of_dispatch_threads == 0) number_of_dispatch_threads = 1;

  Test_U_DHCPClient_GTK_CBData gtk_cb_user_data;
  gtk_cb_user_data.progressData.GTKState = &gtk_cb_user_data;
  // step1d: initialize logging and/or tracing
  Common_Logger_t logger (&gtk_cb_user_data.logStack,
                          &gtk_cb_user_data.lock);
  std::string log_file_name;
  if (log_to_file)
  {
    log_file_name =
      Common_File_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (LIBACENETWORK_PACKAGE_NAME),
                                         ACE::basename (argv_in[0]));
    if (log_file_name.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::getLogFilename(), aborting\n")));

      // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

      return EXIT_FAILURE;
    } // end IF
  } // end IF
  if (!Common_Tools::initializeLogging (ACE::basename (argv_in[0]),               // program name
                                        log_file_name,                            // log file name
                                        false,                                    // log to syslog ?
                                        false,                                    // trace messages ?
                                        trace_information,                        // debug messages ?
                                        (ui_definition_file.empty () ? NULL
                                                                     : &logger))) // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeLogging(), aborting\n")));

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
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (true, // allow SIGUSR1/SIGBREAK
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
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

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
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  Test_U_SignalHandler signal_handler;

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Tools::finalizeSignals (signal_set,
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

    return EXIT_SUCCESS;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_Tools::setResourceLimits (false, // file descriptors
                                        true,  // stack traces
                                        true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Tools::finalizeSignals (signal_set,
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

  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
  if (!gtk_rc_file.empty ())
    gtk_cb_user_data.RCFiles.push_back (gtk_rc_file);
  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  Common_UI_GtkBuilderDefinition ui_definition (argc_in,
                                                argv_in);
  if (!ui_definition_file.empty ())
    if (!COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (argc_in,
                                                                   argv_in,
                                                                   &gtk_cb_user_data,
                                                                   &ui_definition))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager::initialize(), aborting\n")));

      Common_Tools::finalizeSignals (signal_set,
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

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (request_broadcast_replies,
           debug_parser,
           output_file,
           ui_definition_file,
           interface,
           use_loopback,
           use_thread_pool,
           send_request_on_offer,
           use_reactor,
           statistic_reporting_interval,
           number_of_dispatch_threads,
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

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

  return EXIT_SUCCESS;
} // end main
