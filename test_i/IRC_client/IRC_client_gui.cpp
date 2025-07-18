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
#endif // ACE_WIN32 || ACE_WIN32
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

#include "common_file_tools.h"
#include "common_os_tools.h"

#include "common_event_tools.h"

#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_cachedallocatorheap.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_defines.h"

#include "irc_common.h"
#include "irc_defines.h"

#include "test_i_defines.h"

#include "IRC_client_defines.h"
#if defined (GTK_SUPPORT)
#include "IRC_client_gui_callbacks.h"
#endif // GTK_SUPPORT
#include "IRC_client_gui_common.h"
#include "IRC_client_gui_defines.h"
#include "IRC_client_messageallocator.h"
#include "IRC_client_sessionmessage.h"
#include "IRC_client_module_IRChandler.h"
#include "IRC_client_network.h"
#include "IRC_client_signalhandler.h"
#include "IRC_client_stream.h"
#include "IRC_client_tools.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("IRCClientStream");

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
            << std::endl << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEFAULT_INI_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [FILENAME]   : configuration file [\"")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d              : debug [")
            << (COMMON_PARSER_DEFAULT_LEX_TRACE ||
                COMMON_PARSER_DEFAULT_YACC_TRACE)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g [FILENAME]   : GTK rc file [\"")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l              : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DEF_FILE_PHONEBOOK);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [FILENAME]   : phonebook file [\"")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r              : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]      : reporting interval (seconds) [0: off] [")
            << NET_STATISTIC_DEFAULT_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t              : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  //path = configuration_path;
  //path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  //std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [DIRECTORY]  : UI file directory [\"")
  //          << path
  //          << ACE_TEXT_ALWAYS_CHAR ("\"]")
  //          << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v              : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]      : #thread pool threads [")
            << COMMON_EVENT_REACTOR_DEFAULT_THREADPOOL_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
} // end print_usage

bool
do_processArguments (int argc_in,
                     ACE_TCHAR* argv_in[], // cannot be const
                     std::string& configurationFile_out,
                     bool& debug_out,
                     std::string& UIRCFile_out,
                     bool& logToFile_out,
                     std::string& phonebookFile_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numThreadPoolThreads_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  configurationFile_out          = configuration_path;
  configurationFile_out         += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out         +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  configurationFile_out         += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out         +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEFAULT_INI_FILE);

  debug_out                      =
    (COMMON_PARSER_DEFAULT_LEX_TRACE ||
     COMMON_PARSER_DEFAULT_YACC_TRACE);

  UIRCFile_out                   = configuration_path;
  UIRCFile_out                  += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIRCFile_out                  +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  UIRCFile_out                  += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIRCFile_out                  +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_RC_FILE);

  logToFile_out                  = false;

  phonebookFile_out              = configuration_path;
  phonebookFile_out             += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  phonebookFile_out             +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  phonebookFile_out             += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  phonebookFile_out             +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DEF_FILE_PHONEBOOK);

  useReactor_out                 =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);

  statisticReportingInterval_out = NET_STATISTIC_DEFAULT_REPORTING_INTERVAL_S;

  traceInformation_out           = false;

  //UIDefinitionFileDirectory_out  = configuration_path;
  //UIDefinitionFileDirectory_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //UIDefinitionFileDirectory_out +=
  //  ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  printVersionAndExit_out        = false;
  numThreadPoolThreads_out       = COMMON_EVENT_REACTOR_DEFAULT_THREADPOOL_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("c:dg:lp:rs:tvx:"),
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
      case 'c':
      {
        configurationFile_out = argumentParser.opt_arg ();
        break;
      }
      case 'd':
      {
        debug_out = true;
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
      case 'p':
      {
        phonebookFile_out = argumentParser.opt_arg ();
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
#endif // ACE_WIN32 || ACE_WIN64
}

void
do_work (unsigned int numberOfDispatchThreads_in,
         struct IRC_Client_UI_CBData& CBData_in,
         const std::string& UIDefinitionFile_in,
         bool debug_in,
         const ACE_Time_Value& statisticReportingInterval_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         IRC_Client_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  // sanity check(s)
  ACE_ASSERT (CBData_in.configuration);

  // step1: initialize configuration

  // initialize protocol configuration
  struct IRC_AllocatorConfiguration allocator_configuration;
  Stream_CachedAllocatorHeap_T<struct Common_AllocatorConfiguration> heap_allocator (NET_STREAM_MAX_MESSAGES,
                                                                                     IRC_MAXIMUM_FRAME_SIZE + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize allocator: \"%m\", aborting\n")));
    return;
  } // end IF
  IRC_Client_MessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES,
                                                   &heap_allocator);

  //user_data.connectionConfiguration = &configuration.connectionConfiguration;

#if defined (_DEBUG)
  CBData_in.configuration->parserConfiguration.debugParser = debug_in;
  CBData_in.configuration->parserConfiguration.debugScanner = debug_in;
#endif // _DEBUG
  ////////////////////// socket handler configuration //////////////////////////
  IRC_Client_ConnectionConfiguration connection_configuration;
  connection_configuration.allocatorConfiguration = &allocator_configuration;
//  connection_configuration.statisticReportingInterval =
//    ACE_Time_Value (reporting_interval, 0);
  connection_configuration.messageAllocator = &message_allocator;

  connection_configuration.streamConfiguration =
    &CBData_in.configuration->streamConfiguration;

  CBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                            &connection_configuration));
  Net_ConnectionConfigurationsIterator_t iterator =
    CBData_in.configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != CBData_in.configuration->connectionConfigurations.end ());

  ////////////////////////// stream configuration //////////////////////////////
    struct Stream_ModuleConfiguration module_configuration;
  struct IRC_Client_ModuleHandlerConfiguration modulehandler_configuration;
  struct IRC_Client_StreamConfiguration stream_configuration;
  modulehandler_configuration.connectionConfigurations =
    &CBData_in.configuration->connectionConfigurations;
  modulehandler_configuration.parserConfiguration =
    &CBData_in.configuration->parserConfiguration;
  modulehandler_configuration.protocolConfiguration =
    &CBData_in.configuration->protocolConfiguration;
  modulehandler_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  modulehandler_configuration.streamConfiguration =
    &CBData_in.configuration->streamConfiguration;

  stream_configuration.allocatorConfiguration = &allocator_configuration;
  stream_configuration.messageAllocator = &message_allocator;

  CBData_in.configuration->streamConfiguration.initialize (module_configuration,
                                                           modulehandler_configuration,
                                                           stream_configuration);

  //IRC_Client_Module_IRCHandler_Module IRC_handler (ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_HANDLER_MODULE_NAME),
  //                                                 NULL,
  //                                                 true);
  //IRC_Client_Module_IRCHandler* IRCHandler_impl_p = NULL;
  //IRCHandler_impl_p =
  //  dynamic_cast<IRC_Client_Module_IRCHandler*> (IRC_handler.writer ());
  //if (!IRCHandler_impl_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("dynamic_cast<IRC_Client_Module_IRCHandler> failed, returning\n")));
  //  return;
  //} // end IF
  //if (!IRCHandler_impl_p->initialize (CBData_in.configuration->moduleHandlerConfiguration))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::initialize(), returning\n")));
  //  return;
  //} // end IF
  //CBData_in.configuration->streamConfiguration.module = &IRC_handler;
  //CBData_in.configuration->streamConfiguration.cloneModule = true;

  // step2: initialize event dispatch
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
    &CBData_in.configuration->dispatchConfiguration;
  if (CBData_in.configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
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
  IRC_Client_Connection_Manager_t* connection_manager_p =
      IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (*static_cast<IRC_Client_ConnectionConfiguration*> ((*iterator).second),
                             NULL);

  // step3b: initialize timer manager
  Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  struct Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);

  // step4: initialize signal handling
  struct IRC_Client_SignalHandlerConfiguration signal_handler_configuration;
  signal_handler_configuration.dispatchState = &event_dispatch_state_s;
  if (!signalHandler_in.initialize (signal_handler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_SignalHandler::initialize(): \"%m\", returning\n")));
    timer_manager_p->stop ();
    return;
  } // end IF
  if (!Common_Signal_Tools::initialize (((CBData_in.configuration->dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR) ? COMMON_SIGNAL_DISPATCH_REACTOR
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

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step5: start UI event loop
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (!CBData_in.UIState);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  CBData_in.UIState = &const_cast<Common_UI_GTK_State_t&> (state_r);
  CBData_in.UIState->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));

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
    timer_manager_p->stop ();
    return;
  } // end IF
#endif // GTK_USE

  // step6: initialize worker(s)
//  int group_id = -1;
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::startEventDispatch(), returning\n")));

    // clean up
    timer_manager_p->stop ();
#if defined (GTK_USE)
    gtk_manager_p->stop (true,  // wait ?
                         true); // high priority ?
#endif // GTK_USE

    return;
  } // end IF

  // *NOTE*: from this point on, clean up any remote connections !

  // step7: dispatch events
  Common_Event_Tools::dispatchEvents (event_dispatch_state_s);

  // step8: clean up
  timer_manager_p->stop ();
#if defined (GTK_USE)
  gtk_manager_p->wait (false); // don't wait for message queue
#endif // GTK_USE

  // wait for connection processing to complete
  IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
  IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->wait ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

void
do_parsePhonebookFile (const std::string& phonebookFilename_in,
                       IRC_Client_PhoneBook& phoneBook_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_parsePhonebookFile"));

  // initialize return value(s)
  phoneBook_out.timeStamp.update (ACE_Time_Value::zero);
  phoneBook_out.networks.clear ();
  phoneBook_out.servers.clear ();

  int result = -1;
  ACE_Configuration_Heap configuration_heap;
  result = configuration_heap.open ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Configuration_Heap::open() failed, returning\n")));
    return;
  } // end IF

  ACE_Ini_ImpExp ini_import_export (configuration_heap);
  result = ini_import_export.import_config (phonebookFilename_in.c_str ());
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Ini_ImpExp::import_config(\"%s\") failed, returning\n"),
                ACE_TEXT (phonebookFilename_in.c_str ())));
    return;
  } // end IF

  // step1: find/open "timestamp" section...
  ACE_Configuration_Section_Key section_key;
  result =
    configuration_heap.open_section (configuration_heap.root_section (),
                                     ACE_TEXT (IRC_CLIENT_CNF_TIMESTAMP_SECTION_HEADER),
                                     0, // MUST exist !
                                     section_key);
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(\"%s\"), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_TIMESTAMP_SECTION_HEADER)));
    return;
  } // end IF

  // import value...
  int index = 0;
  ACE_TString item_name, item_value;
  ACE_Configuration::VALUETYPE item_type;
  std::stringstream converter;
  while (configuration_heap.enumerate_values (section_key,
                                              index,
                                              item_name,
                                              item_type) == 0)
  {
    result =
      configuration_heap.get_string_value (section_key,
                                           ACE_TEXT (item_name.c_str ()),
                                           item_value);
    if (result == -1)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"), returning\n"),
                  ACE_TEXT (item_name.c_str ())));
      return;
    } // end IF

//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("enumerated %s, type %d\n"),
//                val_name.c_str(),
//                val_type));

    if (item_name == ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DATE_SECTION_HEADER))
    {
      std::string time_stamp = ACE_TEXT_ALWAYS_CHAR (item_value.c_str ());
      // parse timestamp
      std::string::size_type current_fwd_slash = 0;
      std::string::size_type last_fwd_slash = 0;
      current_fwd_slash = time_stamp.find ('/', 0);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
      if (current_fwd_slash == -1)
#else
      if (current_fwd_slash == std::string::npos)
#endif // _MSC_VER >= 1600
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to parse timestamp (was: \"%s\"), returning\n"),
                    ACE_TEXT (phonebookFilename_in.c_str ()),
                    ACE_TEXT (item_value.c_str ())));
        return;
      } // end IF
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << time_stamp.substr (0, current_fwd_slash);
      long day = 0;
      converter >> day;
      phoneBook_out.timeStamp.day (day);
      last_fwd_slash = current_fwd_slash;
      current_fwd_slash = time_stamp.find ('/', current_fwd_slash + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
      if (current_fwd_slash == -1)
#else
      if (current_fwd_slash == std::string::npos)
#endif
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to parse timestamp (was: \"%s\"), returning\n"),
                    ACE_TEXT (phonebookFilename_in.c_str ()),
                    ACE_TEXT (item_value.c_str ())));
        return;
      } // end IF
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << time_stamp.substr (last_fwd_slash + 1,
                                      last_fwd_slash - current_fwd_slash - 1);
      long month = 0;
      converter >> month;
      phoneBook_out.timeStamp.month (month);
      last_fwd_slash = current_fwd_slash;
      current_fwd_slash = time_stamp.find ('/', current_fwd_slash + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined(_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
      if (current_fwd_slash != -1)
#else
      if (current_fwd_slash != std::string::npos)
#endif
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to parse timestamp (was: \"%s\"), returning\n"),
                    ACE_TEXT (phonebookFilename_in.c_str ()),
                    ACE_TEXT (item_value.c_str ())));
        return;
      } // end IF
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << time_stamp.substr (last_fwd_slash + 1,
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
                                    -1);
#else
                                    std::string::npos);
#endif
      long year = 0;
      converter >> year;
      phoneBook_out.timeStamp.year (year);
    } // end IF

    ++index;
  } // end WHILE

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT("timestamp (d/m/y, h:m:s.u): %d/%d/%d, %d:%d:%d.%d\n"),
//              phoneBook_out.timestamp.day(),
//              phoneBook_out.timestamp.month(),
//              phoneBook_out.timestamp.year(),
//              phoneBook_out.timestamp.hour(),
//              phoneBook_out.timestamp.minute(),
//              phoneBook_out.timestamp.second(),
//              phoneBook_out.timestamp.microsec()));

  // step2: find/open "networks" section...
  result =
    configuration_heap.open_section (configuration_heap.root_section (),
                                     ACE_TEXT (IRC_CLIENT_CNF_NETWORKS_SECTION_HEADER),
                                     0, // MUST exist !
                                     section_key);
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(\"%s\"), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_NETWORKS_SECTION_HEADER)));
    return;
  } // end IF

  // import values...
  index = 0;
  while (configuration_heap.enumerate_values (section_key,
                                              index,
                                              item_name,
                                              item_type) == 0)
  {
    result = configuration_heap.get_string_value (section_key,
                                                  ACE_TEXT (item_name.c_str ()),
                                                  item_value);
    if (result == -1)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"), returning\n"),
                  ACE_TEXT (item_name.c_str ())));
      return;
    } // end IF

//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("enumerated %s, type %d\n"),
//                 ACE_TEXT (val_name.c_str ()),
//                 val_type));

    phoneBook_out.networks.insert (item_value.c_str ());

    ++index;
  } // end WHILE

  // step3: find/open "servers" section...
  result =
    configuration_heap.open_section (configuration_heap.root_section (),
                                     ACE_TEXT (IRC_CLIENT_CNF_SERVERS_SECTION_HEADER),
                                     0, // MUST exist !
                                     section_key);
  if (result == -1)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(\"%s\"), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_SERVERS_SECTION_HEADER)));
    return;
  } // end IF

  // import values...
  index = 0;
  IRC_Client_ConnectionEntry entry;
  std::string entry_name;
  IRC_Client_PortRange_t port_range;
  bool no_range = false;
  while (configuration_heap.enumerate_values (section_key,
                                              index,
                                              item_name,
                                              item_type) == 0)
  {
    entry.ports.clear ();

    result =
      configuration_heap.get_string_value (section_key,
                                           ACE_TEXT (item_name.c_str ()),
                                           item_value);
    if (result == -1)
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"), returning\n"),
                  ACE_TEXT (item_name.c_str ())));
      return;
    } // end IF

//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("enumerated %s, type %d\n"),
//                 ACE_TEXT (item_name.c_str ()),
//                 item_type));

    std::string line_string = item_value.c_str ();

    // parse connection name
    std::string::size_type current_position = 0;
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
    std::string::size_type last_position = -1;
#else
    std::string::size_type last_position = std::string::npos;
#endif
    current_position =
      line_string.find (ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_PHONEBOOK_KEYWORD_SERVER), 0);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
    if (current_position == -1)
#else
    if (current_position == std::string::npos)
#endif
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to parse server (was: \"%s\"), returning\n"),
                  ACE_TEXT (phonebookFilename_in.c_str ()),
                  ACE_TEXT (item_value.c_str ())));
      return;
    } // end IF
    // *TODO*: needs further parsing...
    entry_name = line_string.substr (0, current_position);
    last_position = current_position + 6;

    // parse hostname
    current_position = line_string.find (':', last_position + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
    if (current_position == -1)
#else
    if (current_position == std::string::npos)
#endif
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to parse server (was: \"%s\"), returning\n"),
                  ACE_TEXT (phonebookFilename_in.c_str ()),
                  ACE_TEXT (item_value.c_str ())));
      return;
    } // end IF
    entry.hostName =
      line_string.substr (last_position + 1,
                          current_position - last_position - 1);
    last_position = current_position;

    // parse (list of) port ranges
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
    std::string::size_type next_comma = -1;
#else
    std::string::size_type next_comma = std::string::npos;
#endif
    std::string::size_type group =
      line_string.find (ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_PHONEBOOK_KEYWORD_GROUP),
                        current_position + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
    if (group == -1)
#else
    if (group == std::string::npos)
#endif
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to parse server (was: \"%s\"), returning\n"),
                  ACE_TEXT (phonebookFilename_in.c_str ()),
                  ACE_TEXT (item_value.c_str ())));
      return;
    } // end IF
    do
    {
      no_range = false;

      next_comma = line_string.find (',', current_position + 1);
      if (next_comma > group) next_comma = group;

      // port range ?
      current_position = line_string.find ('-', current_position + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
      if ((current_position == -1)                ||
#else
      if ((current_position == std::string::npos) ||
#endif
          (current_position > next_comma))
        no_range = true;
      else
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << line_string.substr (last_position + 1,
                                         current_position - last_position - 1);
        converter >> port_range.first;

        last_position = current_position;
      } // end ELSE
      current_position = next_comma;
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << line_string.substr (last_position + 1,
                                       current_position - last_position - 1);
      converter >> port_range.second;
      if (no_range) port_range.first = port_range.second;
      entry.ports.push_back (port_range);

      // skip to next port(range)
      if (next_comma == group)
      {
        // this means that the end of the list has been reached...
        // --> skip over "GROUP:"
        last_position = next_comma + 5;

        // proceed
        break;
      } // end IF

      last_position = current_position;
    } while (true);

    // parse "group" (== network)
    entry.netWork = line_string.substr (last_position + 1);

    phoneBook_out.networks.insert (entry.netWork);
    phoneBook_out.servers.insert (std::make_pair (entry_name, entry));

    ++index;
  } // end WHILE
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("parsed %u phonebook (timestamp: %u/%u/%u) entries (%u network(s))...\n"),
              phoneBook_out.servers.size (),
              phoneBook_out.timeStamp.month (),
              phoneBook_out.timeStamp.day (),
              phoneBook_out.timeStamp.year (),
              phoneBook_out.networks.size ()));

  // add localhost
  entry.hostName = ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEFAULT_SERVER_HOSTNAME);
  entry.ports.clear ();
  entry.ports.push_back (std::make_pair (IRC_DEFAULT_SERVER_PORT,
                                         IRC_DEFAULT_SERVER_PORT));
  entry.netWork.clear ();
  phoneBook_out.servers.insert (std::make_pair (entry.hostName, entry));

  //   for (IRC_Client_NetworksIterator_t iterator = phoneBook_out.networks.begin ();
  //        iterator != phoneBook_out.networks.end ();
  //        iterator++)
  //     ACE_DEBUG ((LM_DEBUG,
  //                 ACE_TEXT ("network: \"%s\"\n"),
  //                 ACE_TEXT ((*iterator).c_str ())));
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

  std::string configuration_file_name        = configuration_path;
  configuration_file_name                   += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file_name                   +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  configuration_file_name                   += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file_name                   +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEFAULT_INI_FILE);

  bool debug                                 =
    (COMMON_PARSER_DEFAULT_LEX_TRACE ||
     COMMON_PARSER_DEFAULT_YACC_TRACE);

  std::string UIRC_file_name                 = configuration_path;
  UIRC_file_name                            += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIRC_file_name                            +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  UIRC_file_name                            += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIRC_file_name                            +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_RC_FILE);

  bool log_to_file                           = false;

  std::string phonebook_file_name            = configuration_path;
  phonebook_file_name                       += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  phonebook_file_name                       +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  phonebook_file_name                       += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  phonebook_file_name                       +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DEF_FILE_PHONEBOOK);

  bool use_reactor                           =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);

  unsigned int reporting_interval            =
    NET_STATISTIC_DEFAULT_REPORTING_INTERVAL_S;

  bool trace_information                     = false;

  std::string UIDefinitionFile_directory     = configuration_path;
  UIDefinitionFile_directory                += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitionFile_directory                +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);

  bool print_version_and_exit                = false;
  unsigned int number_of_thread_pool_threads =
    COMMON_EVENT_REACTOR_DEFAULT_THREADPOOL_THREADS;
  if (!do_processArguments (argc_in,
                            argv_in,
                            configuration_file_name,
                            debug,
                            UIRC_file_name,
                            log_to_file,
                            phonebook_file_name,
                            use_reactor,
                            reporting_interval,
                            trace_information,
                            //UIDefinitionFile_directory,
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
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_MAIN_FILE);

  // step2b: validate argument(s)
  if (!Common_File_Tools::isReadable (configuration_file_name) ||
      !Common_File_Tools::isReadable (phonebook_file_name)     ||
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
  if (!Common_Log_Tools::initialize (ACE::basename (argv_in[0]), // program name
                                     log_file_name,              // log file name
                                     false,                      // log to syslog ?
                                     false,                      // trace messages ?
                                     trace_information,          // debug messages ?
                                     NULL))                      // logger
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));

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

    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_FAILURE;
  } // end IF

#if defined (GTK_USE)
//  Common_MessageStack_t* logstack_p = NULL;
//  ACE_SYNCH_MUTEX* lock_p = NULL;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
//  Common_UI_GTK_State_t& state_r =
//    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
//  logstack_p = &state_r.logStack;
//  lock_p = &state_r.logStackLock;
#endif // GTK_USE

  struct IRC_Client_Configuration configuration;
  struct IRC_Client_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;
//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
  IRC_Client_SignalHandler signal_handler;

  // step5: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_SUCCESS;
  } // end IF

  ui_cb_data.UIFileDirectory = UIDefinitionFile_directory;
//   userData.phoneBook;
//   userData.loginOptions.password = ;
  ui_cb_data.configuration->protocolConfiguration.loginOptions.nickname =
      ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_NICKNAME);
//   userData.loginOptions.user.username = ;
  std::string host_name;
  if (!Net_Common_Tools::getHostname (host_name))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::getHostname(), aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_FAILURE;
  } // end IF
  if (IRC_PRT_USERMSG_TRADITIONAL)
  {
    configuration.protocolConfiguration.loginOptions.user.hostName.discriminator =
      IRC_LoginOptions::User::Hostname::STRING;
    configuration.protocolConfiguration.loginOptions.user.hostName.string =
      &host_name;
  } // end IF
  else
  {
    configuration.protocolConfiguration.loginOptions.user.hostName.discriminator =
      IRC_LoginOptions::User::Hostname::MODE;
    // *NOTE*: hybrid-7.2.3 seems to have a bug: 4 --> +i
    configuration.protocolConfiguration.loginOptions.user.hostName.mode =
      IRC_DEFAULT_USERMODE;
  } // end ELSE
  configuration.protocolConfiguration.loginOptions.user.serverName =
    ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_SERVERNAME);
  configuration.protocolConfiguration.loginOptions.channel =
    ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_CHANNEL);
  // populate user/realname
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_OS_Tools::getUserName (configuration.protocolConfiguration.loginOptions.user.userName,
                                configuration.protocolConfiguration.loginOptions.user.realName);
#else
  Common_OS_Tools::getUserName (static_cast<uid_t> (-1),
                                configuration.protocolConfiguration.loginOptions.user.userName,
                                configuration.protocolConfiguration.loginOptions.user.realName);
#endif // ACE_WIN32 || ACE_WIN64

  // step7: parse configuration file(s) (if any)
  if (Common_File_Tools::isReadable (phonebook_file_name))
    do_parsePhonebookFile (phonebook_file_name,
                           ui_cb_data.phoneBook);
  if (!configuration_file_name.empty ())
  {
    IRC_Client_Connections_t connections;
    IRC_Client_Tools::parseConfigurationFile (configuration_file_name,
                                              configuration.protocolConfiguration.loginOptions,
                                              connections);

    // add connections to phonebook
    for (IRC_Client_ConnectionsIterator_t iterator = connections.begin ();
         iterator != connections.end ();
         ++iterator)
      ui_cb_data.phoneBook.servers.insert (std::make_pair ((*iterator).hostName,
                                                             *iterator));
  } // end IF

  configuration.dispatchConfiguration.dispatch =
      (use_reactor ? COMMON_EVENT_DISPATCH_REACTOR
                   : COMMON_EVENT_DEFAULT_DISPATCH);

#if defined (GTK_USE)
  ui_cb_data.progressData.state =
    &const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE

  // step8: initialize GTK UI
#if defined (GTK_USE)
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.RCFiles.push_back (UIRC_file_name);
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (ui_cb_data.configuration->GTKConfiguration);
#endif // GTK_USE

  // step9: do work
  ACE_High_Res_Timer timer;
  timer.start ();
  do_work (number_of_thread_pool_threads,
           ui_cb_data,
           ui_definition_file_name,
           debug,
           ACE_Time_Value (reporting_interval, 0),
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
    Common_Log_Tools::finalize ();
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
  Common_Log_Tools::finalize ();

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
