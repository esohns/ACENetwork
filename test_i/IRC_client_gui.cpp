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
#endif
#include "ace/POSIX_Proactor.h"
#include "ace/Proactor.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#include "gtk/gtk.h"

#include "common_file_tools.h"
#include "common_tools.h"

#include "common_ui_defines.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_cachedallocatorheap.h"

#ifdef HAVE_CONFIG_H
#include "libacenetwork_config.h"
#endif
#include "net_defines.h"

#include "IRC_common.h"

#include "IRC_client_defines.h"
#include "IRC_client_gui_callbacks.h"
#include "IRC_client_gui_common.h"
#include "IRC_client_gui_defines.h"
#include "IRC_client_messageallocator.h"
#include "IRC_client_module_IRChandler.h"
#include "IRC_client_network.h"
#include "IRC_client_signalhandler.h"

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

  std::cout << ACE_TEXT ("usage: ")
            << programName_in
            << ACE_TEXT (" [OPTIONS]")
            << std::endl << std::endl;
  std::cout << ACE_TEXT ("currently available options:") << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (IRC_CLIENT_CNF_DEF_INI_FILE);
  std::cout << ACE_TEXT ("-c [FILE] : configuration file")
            << ACE_TEXT (" [\"")
            << path
            << ACE_TEXT ("\"]")
            << std::endl;
  std::cout << ACE_TEXT ("-d        : debug")
            << ACE_TEXT (" [")
            << (IRC_CLIENT_DEF_LEX_TRACE || IRC_CLIENT_DEF_YACC_TRACE)
            << ACE_TEXT ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (IRC_CLIENT_GUI_GTK_UI_RC_FILE);
  std::cout << ACE_TEXT ("-g [FILE] : GTK rc file")
            << ACE_TEXT (" [\"")
            << path
            << ACE_TEXT ("\"]")
            << std::endl;
  std::cout << ACE_TEXT ("-h        : use thread-pool [")
            << NET_EVENT_USE_THREAD_POOL
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-l        : log to a file")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (IRC_CLIENT_GUI_DEF_FILE_PHONEBOOK);
  std::cout << ACE_TEXT ("-p [FILE] : phonebook file")
            << ACE_TEXT (" [\"")
            << path
            << ACE_TEXT ("\"]")
            << std::endl;
  std::cout << ACE_TEXT ("-r        : use reactor [")
            << NET_EVENT_USE_REACTOR
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-s [VALUE]: reporting interval (seconds: 0 --> OFF)")
            << ACE_TEXT (" [")
            << IRC_CLIENT_DEF_STATSINTERVAL
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-t        : trace information")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  std::cout << ACE_TEXT ("-u [DIR]  : UI file directory")
            << ACE_TEXT (" [\"")
            << path
            << ACE_TEXT ("\"]")
            << std::endl;
  std::cout << ACE_TEXT ("-v        : print version information and exit")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-x [VALUE]: #thread pool threads ([")
            << IRC_CLIENT_DEF_NUM_TP_THREADS
            << ACE_TEXT ("]")
            << std::endl;
} // end print_usage

bool
do_processArguments (int argc_in,
                     ACE_TCHAR* argv_in[], // cannot be const...
                     std::string& configurationFile_out,
                     bool& debug_out,
                     std::string& UIRCFile_out,
                     bool& useThreadpool_out,
                     bool& logToFile_out,
                     std::string& phonebookFile_out,
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
#if defined (DEBUG_DEBUGGER)
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  configurationFile_out          = configuration_path;
  configurationFile_out         += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out         +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  configurationFile_out         += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out         +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEF_INI_FILE);

  debug_out                      =
    (IRC_CLIENT_DEF_LEX_TRACE || IRC_CLIENT_DEF_YACC_TRACE);

  UIRCFile_out                   = configuration_path;
  UIRCFile_out                  += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIRCFile_out                  +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  UIRCFile_out                  += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIRCFile_out                  +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_RC_FILE);

  useThreadpool_out              = NET_EVENT_USE_THREAD_POOL;

  logToFile_out                  = false;

  phonebookFile_out              = configuration_path;
  phonebookFile_out             += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  phonebookFile_out             +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  phonebookFile_out             += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  phonebookFile_out             +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DEF_FILE_PHONEBOOK);

  useReactor_out                 = NET_EVENT_USE_REACTOR;

  statisticReportingInterval_out = IRC_CLIENT_DEF_STATSINTERVAL;

  traceInformation_out           = false;

  UIDefinitionFileDirectory_out  = configuration_path;
  UIDefinitionFileDirectory_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitionFileDirectory_out +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);

  printVersionAndExit_out        = false;
  numThreadPoolThreads_out       = IRC_CLIENT_DEF_NUM_TP_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("c:dg:hlp:rs:tu:vx:"),
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
      case 'h':
      {
        useThreadpool_out = true;
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
do_work (bool useThreadPool_in,
         bool useReactor_in,
         unsigned int numDispatchThreads_in,
         IRC_Client_GTK_CBData& userData_in,
         const std::string& UIDefinitionFile_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         IRC_Client_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  // sanity check(s)
  ACE_ASSERT (userData_in.configuration);

  // step1: initialize IRC handler module
  IRC_Client_StreamModuleConfiguration module_configuration;
  userData_in.configuration->streamConfiguration.streamConfiguration.moduleConfiguration =
      &module_configuration.moduleConfiguration;

  IRC_Client_Module_IRCHandler_Module IRC_handler (ACE_TEXT_ALWAYS_CHAR (IRC_HANDLER_MODULE_NAME),
                                                   NULL);
  IRC_Client_Module_IRCHandler* IRCHandler_impl_p = NULL;
  IRCHandler_impl_p =
    dynamic_cast<IRC_Client_Module_IRCHandler*> (IRC_handler.writer ());
  if (!IRCHandler_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_IRCHandler> failed, returning\n")));
    return;
  } // end IF
  if (!IRCHandler_impl_p->initialize (NULL,
                                      userData_in.configuration->streamConfiguration.streamConfiguration.messageAllocator,
                                      userData_in.configuration->streamConfiguration.streamConfiguration.bufferSize,
                                      userData_in.configuration->protocolConfiguration.automaticPong,
                                      userData_in.configuration->protocolConfiguration.printPingDot))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler::initialize(), returning\n")));
    return;
  } // end IF
  userData_in.configuration->streamConfiguration.streamConfiguration.module =
    &IRC_handler;
  userData_in.configuration->streamConfiguration.streamConfiguration.deleteModule =
    false;

  // step2: initialize event dispatch
  if (!Common_Tools::initializeEventDispatch (useReactor_in,
                                              numDispatchThreads_in,
                                              userData_in.configuration->streamConfiguration.streamConfiguration.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize event dispatch, returning\n")));
    return;
  } // end IF

  // step3a: initialize connection manager
  IRC_Client_Connection_Manager_t* connection_manager_p =
      IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  IRC_Client_SessionData session_data;
  connection_manager_p->set (*userData_in.configuration,
                             &session_data);

  // step3b: initialize timer manager
  Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  Common_TimerConfiguration timer_configuration;
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start ();

  // step4: initialize signal handling
  IRC_Client_SignalHandlerConfiguration signal_handler_configuration;
  if (!signalHandler_in.initialize (signal_handler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_SignalHandler::initialize(): \"%m\", returning\n")));

    // clean up
    timer_manager_p->stop ();

    return;
  } // end IF
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

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step5: start GTK event loop
  userData_in.GTKState.initializationHook = idle_initialize_UI_cb;
  userData_in.GTKState.finalizationHook = idle_finalize_UI_cb;
  userData_in.GTKState.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN)] =
    std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
  userData_in.GTKState.userData = &userData_in;

  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->start ();
  if (!COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start GTK event dispatch, returning\n")));

    // clean up
    timer_manager_p->stop ();

    return;
  } // end IF

  // step6: initialize worker(s)
  int group_id = -1;
  // *NOTE*: this variable needs to stay on the working stack, it's passed to
  //         the worker(s) (if any)
  bool use_reactor = useReactor_in;
  if (!Common_Tools::startEventDispatch (use_reactor,
                                         numDispatchThreads_in,
                                         group_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::startEventDispatch(), returning\n")));

    // clean up
    timer_manager_p->stop ();
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();

    return;
  } // end IF

  // *NOTE*: from this point on, clean up any remote connections !

  // step7: dispatch events
  Common_Tools::dispatchEvents (useReactor_in,
                                !useReactor_in,
                                group_id);

  // step8: clean up
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->wait ();
  timer_manager_p->stop ();

  // wait for connection processing to complete
  IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
  IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->wait ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

void
do_parsePhonebookFile (const std::string& serverConfigFile_in,
                       IRC_Client_PhoneBook& phoneBook_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_parsePhonebookFile"));

  // initialize return value(s)
  phoneBook_out.timestamp.update (ACE_Time_Value::zero);
  phoneBook_out.networks.clear ();
  phoneBook_out.servers.clear ();

  ACE_Configuration_Heap config_heap;
  if (config_heap.open ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Configuration_Heap::open() failed, returning\n")));
    return;
  } // end IF

  ACE_Ini_ImpExp import (config_heap);
  if (import.import_config (serverConfigFile_in.c_str ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Ini_ImpExp::import_config(\"%s\") failed, returning\n"),
                ACE_TEXT (serverConfigFile_in.c_str ())));
    return;
  } // end IF

  // step1: find/open "timestamp" section...
  ACE_Configuration_Section_Key section_key;
  if (config_heap.open_section (config_heap.root_section (),
                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_TIMESTAMP_SECTION_HEADER),
                                0, // MUST exist !
                                section_key) != 0)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(\"%s\"), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_TIMESTAMP_SECTION_HEADER)));
    return;
  } // end IF

  // import value...
  int val_index = 0;
  ACE_TString val_name, val_value;
  ACE_Configuration::VALUETYPE val_type;
  std::stringstream converter;
  while (config_heap.enumerate_values (section_key,
                                       val_index,
                                       val_name,
                                       val_type) == 0)
  {
    if (config_heap.get_string_value (section_key,
                                      val_name.c_str (),
                                      val_value))
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"), returning\n"),
                  ACE_TEXT (val_name.c_str ())));
      return;
    } // end IF

//     ACE_DEBUG((LM_DEBUG,
//                ACE_TEXT("enumerated %s, type %d\n"),
//                val_name.c_str(),
//                val_type));

    // *TODO*: move these strings...
    if (val_name == ACE_TEXT_ALWAYS_CHAR ("date"))
    {
      std::string timestamp = ACE_TEXT_ALWAYS_CHAR (val_value.c_str ());
      // parse timestamp
      std::string::size_type current_fwd_slash = 0;
      std::string::size_type last_fwd_slash = 0;
      current_fwd_slash = timestamp.find ('/', 0);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined(_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
      if (current_fwd_slash == -1)
#else
      if (current_fwd_slash == std::string::npos)
#endif
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to parse timestamp (was: \"%s\"), returning\n"),
                    ACE_TEXT (serverConfigFile_in.c_str ()),
                    ACE_TEXT (val_value.c_str ())));
        return;
      } // end IF
      converter.str (ACE_TEXT_ALWAYS_CHAR(""));
      converter.clear ();
      converter << timestamp.substr (0,
                                     current_fwd_slash);
      long day = 0;
      converter >> day;
      phoneBook_out.timestamp.day (day);
      last_fwd_slash = current_fwd_slash;
      current_fwd_slash = timestamp.find ('/', current_fwd_slash + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined(_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
      if (current_fwd_slash == -1)
#else
      if (current_fwd_slash == std::string::npos)
#endif
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to parse timestamp (was: \"%s\"), returning\n"),
                    ACE_TEXT (serverConfigFile_in.c_str ()),
                    ACE_TEXT (val_value.c_str ())));
        return;
      } // end IF
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << timestamp.substr (last_fwd_slash + 1,
                                     last_fwd_slash - current_fwd_slash - 1);
      long month = 0;
      converter >> month;
      phoneBook_out.timestamp.month (month);
      last_fwd_slash = current_fwd_slash;
      current_fwd_slash = timestamp.find ('/', current_fwd_slash + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined(_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
      if (current_fwd_slash != -1)
#else
      if (current_fwd_slash != std::string::npos)
#endif
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to parse timestamp (was: \"%s\"), returning\n"),
                    ACE_TEXT(serverConfigFile_in.c_str ()),
                    ACE_TEXT(val_value.c_str ())));
        return;
      } // end IF
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << timestamp.substr (last_fwd_slash + 1,
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
                                    -1);
#else
                                    std::string::npos);
#endif
      long year = 0;
      converter >> year;
      phoneBook_out.timestamp.year (year);
    } // end IF

    ++val_index;
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
  if (config_heap.open_section (config_heap.root_section (),
                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_NETWORKS_SECTION_HEADER),
                                0, // MUST exist !
                                section_key) != 0)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(\"%s\"), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_NETWORKS_SECTION_HEADER)));
    return;
  } // end IF

  // import values...
  val_index = 0;
  while (config_heap.enumerate_values (section_key,
                                       val_index,
                                       val_name,
                                       val_type) == 0)
  {
    if (config_heap.get_string_value (section_key,
                                      val_name.c_str (),
                                      val_value))
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"), returning\n"),
                  ACE_TEXT (val_name.c_str ())));
      return;
    } // end IF

//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("enumerated %s, type %d\n"),
//                 ACE_TEXT (val_name.c_str ()),
//                 val_type));

    phoneBook_out.networks.insert (ACE_TEXT_ALWAYS_CHAR (val_value.c_str ()));

    ++val_index;
  } // end WHILE

  // step3: find/open "servers" section...
  if (config_heap.open_section (config_heap.root_section (),
                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_SERVERS_SECTION_HEADER),
                                0, // MUST exist !
                                section_key) != 0)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(\"%s\"), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_SERVERS_SECTION_HEADER)));
    return;
  } // end IF

  // import values...
  val_index = 0;
  IRC_Client_ConnectionEntry entry;
  std::string entry_name;
  IRC_Client_PortRange_t port_range;
  bool no_range = false;
  while (config_heap.enumerate_values (section_key,
                                       val_index,
                                       val_name,
                                       val_type) == 0)
  {
    entry.listeningPorts.clear ();

    if (config_heap.get_string_value (section_key,
                                      val_name.c_str (),
                                      val_value))
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"), returning\n"),
                  ACE_TEXT (val_name.c_str ())));
      return;
    } // end IF

//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("enumerated %s, type %d\n"),
//                 ACE_TEXT (val_name.c_str ()),
//                 val_type));

    std::string server_line_string = ACE_TEXT_ALWAYS_CHAR (val_value.c_str ());

    // parse connection name
    std::string::size_type current_position = 0;
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
    std::string::size_type last_position = -1;
#else
    std::string::size_type last_position = std::string::npos;
#endif
    current_position =
        server_line_string.find (ACE_TEXT_ALWAYS_CHAR ("SERVER:"), 0);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
    if (current_position == -1)
#else
    if (current_position == std::string::npos)
#endif
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to parse server (was: \"%s\"), returning\n"),
                  ACE_TEXT (serverConfigFile_in.c_str ()),
                  ACE_TEXT (val_value.c_str ())));
      return;
    } // end IF
    // *TODO*: needs further parsing...
    entry_name = server_line_string.substr (0,
                                            current_position);
    last_position = current_position + 6;

    // parse hostname
    current_position = server_line_string.find (':', last_position + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
    if (current_position == -1)
#else
    if (current_position == std::string::npos)
#endif
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to parse server (was: \"%s\"), returning\n"),
                  ACE_TEXT (serverConfigFile_in.c_str ()),
                  ACE_TEXT (val_value.c_str ())));
      return;
    } // end IF
    entry.hostName = server_line_string.substr (last_position + 1,
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
        server_line_string.find (ACE_TEXT_ALWAYS_CHAR("GROUP:"), current_position + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
    if (group == -1)
#else
    if (group == std::string::npos)
#endif
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("\"%s\": failed to parse server (was: \"%s\"), returning\n"),
                  ACE_TEXT (serverConfigFile_in.c_str ()),
                  ACE_TEXT (val_value.c_str ())));
      return;
    } // end IF
    do
    {
      no_range = false;

      next_comma = server_line_string.find (',', current_position + 1);
      if (next_comma > group)
        next_comma = group;

      // port range ?
      current_position = server_line_string.find ('-', current_position + 1);
// *TODO*: there is a linking problem using std::string::npos in MSVC 2010...
#if defined (_MSC_VER) && (_MSC_VER >= 1600) /* VS2010 or newer */
      if ((current_position == -1) ||
#else
      if ((current_position == std::string::npos) ||
#endif
          (current_position > next_comma))
        no_range = true;
      else
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << server_line_string.substr (last_position + 1,
                                                current_position - last_position - 1);
        converter >> port_range.first;

        last_position = current_position;
      } // end ELSE
      current_position = next_comma;
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << server_line_string.substr (last_position + 1,
                                              current_position - last_position - 1);
      converter >> port_range.second;
      if (no_range)
        port_range.first = port_range.second;
      entry.listeningPorts.push_back(port_range);

      // skip to next port(range)
      if (next_comma == group)
      {
        // this means we've reached the end of the list...
        // --> skip over "GROUP:"
        last_position = next_comma + 5;

        // proceed
        break;
      } // end IF

      last_position = current_position;
    } while (true);

    // parse "group" (== network)
    entry.network = server_line_string.substr (last_position + 1);

    phoneBook_out.networks.insert (entry.network);
    phoneBook_out.servers.insert (std::make_pair (entry_name, entry));

    ++val_index;
  } // end WHILE

//   for (IRC_Client_NetworksIterator_t iterator = phoneBook_out.networks.begin ();
//        iterator != phoneBook_out.networks.end ();
//        iterator++)
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("network: \"%s\"\n"),
//                 ACE_TEXT ((*iterator).c_str ())));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("parsed %u phonebook (timestamp: %u/%u/%u) entries (%u network(s))...\n"),
              phoneBook_out.servers.size (),
              phoneBook_out.timestamp.month (),
              phoneBook_out.timestamp.day (),
              phoneBook_out.timestamp.year (),
              phoneBook_out.networks.size ()));
}

void
do_parseConfigurationFile (const std::string& configFilename_in,
                           IRC_Client_IRCLoginOptions& loginOptions_out,
                           IRC_Client_PhoneBook& phoneBook_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_parseConfigurationFile"));

  ACE_Configuration_Heap config_heap;
  if (config_heap.open ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Configuration_Heap::open failed, returning\n")));
    return;
  } // end IF

  ACE_Ini_ImpExp import (config_heap);
  if (import.import_config (configFilename_in.c_str ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("ACE_Ini_ImpExp::import_config(\"%s\") failed, returning\n"),
                ACE_TEXT (configFilename_in.c_str ())));
    return;
  } // end IF

  // find/open "login" section...
  ACE_Configuration_Section_Key section_key;
  if (config_heap.open_section (config_heap.root_section (),
                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_LOGIN_SECTION_HEADER),
                                0, // MUST exist !
                                section_key) != 0)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(%s), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_LOGIN_SECTION_HEADER)));
    return;
  } // end IF

  // import values...
  int val_index = 0;
  ACE_TString val_name, val_string_value;
  ACE_Configuration::VALUETYPE val_type;
  while (config_heap.enumerate_values (section_key,
                                       val_index,
                                       val_name,
                                       val_type) == 0)
  {
    if (val_type == ACE_Configuration::STRING)
      if (config_heap.get_string_value (section_key,
                                        val_name.c_str (),
                                        val_string_value))
      {
        ACE_ERROR ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(%s), returning\n"),
                    ACE_TEXT (val_name.c_str ())));
        return;
      } // end IF

//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("enumerated %s, type %d\n"),
//                 ACE_TEXT (val_name.c_str ()),
//                 val_type));

    // *TODO*: move these strings...
    if (val_name == ACE_TEXT_ALWAYS_CHAR ("password"))
      loginOptions_out.password = ACE_TEXT_ALWAYS_CHAR (val_string_value.c_str ());
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("nick"))
      loginOptions_out.nickname = ACE_TEXT_ALWAYS_CHAR (val_string_value.c_str ());
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("user"))
      loginOptions_out.user.username = ACE_TEXT_ALWAYS_CHAR (val_string_value.c_str ());
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("realname"))
      loginOptions_out.user.realname = ACE_TEXT_ALWAYS_CHAR (val_string_value.c_str ());
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("channel"))
      loginOptions_out.channel = ACE_TEXT_ALWAYS_CHAR (val_string_value.c_str ());
    else
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("unexpected key \"%s\", continuing\n"),
                  ACE_TEXT (val_name.c_str ())));
    } // end ELSE

    ++val_index;
  } // end WHILE

  // find/open "connection" section...
  if (config_heap.open_section (config_heap.root_section (),
                                ACE_TEXT (IRC_CLIENT_CNF_CONNECTION_SECTION_HEADER),
                                0, // MUST exist !
                                section_key) != 0)
  {
    ACE_ERROR ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(%s), returning\n"),
                ACE_TEXT (IRC_CLIENT_CNF_CONNECTION_SECTION_HEADER)));
    return;
  } // end IF

  // import values...
  val_index = 0;
  IRC_Client_ConnectionEntry entry;
//   u_int port = 0;
  std::stringstream converter;
  while (config_heap.enumerate_values (section_key,
                                       val_index,
                                       val_name,
                                       val_type) == 0)
  {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("enumerated %s, type %d\n"),
//                 ACE_TEXT (val_name.c_str ()),
//                 val_type));

    ACE_ASSERT (val_type == ACE_Configuration::STRING);
    if (config_heap.get_string_value (section_key,
                                      val_name.c_str (),
                                      val_string_value))
    {
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(%s), returning\n"),
                  ACE_TEXT (val_name.c_str ())));
      return;
    } // end IF

    // *TODO*: move these strings...
    if (val_name == ACE_TEXT_ALWAYS_CHAR ("server"))
    {
      entry.hostName = ACE_TEXT_ALWAYS_CHAR (val_string_value.c_str ());

      if (!entry.listeningPorts.empty ())
        phoneBook_out.servers.insert (std::make_pair (entry.hostName, entry));
    }
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("port"))
    {
//       ACE_ASSERT (val_type == ACE_Configuration::INTEGER);
//       if (config_heap.get_integer_value (section_key,
//                                          val_name.c_str (),
//                                          port))
//       {
//         ACE_ERROR ((LM_ERROR,
//                     ACE_TEXT ("failed to ACE_Configuration_Heap::get_integer_value(%s), returning\n"),
//                     ACE_TEXT (val_name.c_str ())));
//         return;
//       } // end IF
      IRC_Client_PortRange_t port_range;
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << val_string_value.c_str ();
      converter >> port_range.first;
//       port_range.first = static_cast<unsigned short> (port);
      port_range.second = port_range.first;
      entry.listeningPorts.push_back (port_range);

      if (!entry.hostName.empty ())
        phoneBook_out.servers.insert (std::make_pair (entry.hostName, entry));
    }
    else
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("unexpected key \"%s\", continuing\n"),
                  ACE_TEXT (val_name.c_str ())));

    ++val_index;
  } // end WHILE

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("imported \"%s\"...\n"),
//               ACE_TEXT (configFilename_in.c_str ())));
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
           ACE_TCHAR* argv_in[])
{
  NETWORK_TRACE (ACE_TEXT ("::main"));

  int result = -1;

  // step1: initialize libraries
  // *PORTABILITY*: on Windows, init ACE...
  // *IMPORTANT NOTE*: when linking with SDL, things go wrong:
  //                   ACE_TMAIN --> main --> SDL_main --> ...
  // --> ACE::init is not called automatically (see OS_main.cpp:74),
  //     which would call WSAStartup(); do it manually (doesn't hurt anyway)
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

  // step2 initialize/validate configuration

  // step2a: process commandline arguments
  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  std::string configuration_file         = configuration_path;
  configuration_file                    += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file                    +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  configuration_file                    += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file                    +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEF_INI_FILE);

  bool debug                             =
    (IRC_CLIENT_DEF_LEX_TRACE || IRC_CLIENT_DEF_YACC_TRACE);

  std::string UIRC_file                  = configuration_path;
  UIRC_file                             += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIRC_file                             +=
    ACE_TEXT (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  UIRC_file                             += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIRC_file                             +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_RC_FILE);

  bool use_thread_pool                   = NET_EVENT_USE_THREAD_POOL;
  bool log_to_file                       = false;

  std::string phonebook_file             = configuration_path;
  phonebook_file                        += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  phonebook_file                        +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);
  phonebook_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  phonebook_file += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_DEF_FILE_PHONEBOOK);

  bool use_reactor                       = NET_EVENT_USE_REACTOR;

  unsigned int reporting_interval        = IRC_CLIENT_DEF_STATSINTERVAL;

  bool trace_information                 = false;

  std::string UIDefinitionFile_directory = configuration_path;
  UIDefinitionFile_directory            += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitionFile_directory            +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_FILE_DIRECTORY);

  bool print_version_and_exit            = false;
  unsigned int num_thread_pool_threads   = IRC_CLIENT_DEF_NUM_TP_THREADS;
  if (!do_processArguments (argc_in,
                            argv_in,
                            configuration_file,
                            debug,
                            UIRC_file,
                            use_thread_pool,
                            log_to_file,
                            phonebook_file,
                            use_reactor,
                            reporting_interval,
                            trace_information,
                            UIDefinitionFile_directory,
                            print_version_and_exit,
                            num_thread_pool_threads))
  {
    // make 'em learn...
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // assemble FQ filename (Glade-UI XML)
  std::string ui_definition_filename = UIDefinitionFile_directory;
  ui_definition_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_filename +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_UI_MAIN_FILE);

  // step2b: validate argument(s)
  if (!Common_File_Tools::isReadable (configuration_file)    ||
      !Common_File_Tools::isReadable (phonebook_file)        ||
      !Common_File_Tools::isReadable (ui_definition_filename))
  {
    // make 'em learn...
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step3: initialize logging and/or tracing
  std::string log_file;
  if (log_to_file)
    log_file = Common_File_Tools::getLogFilename (ACE::basename (argv_in[0]));
  if (!Common_Tools::initializeLogging (ACE::basename (argv_in[0]), // program name
                                        log_file,                   // logfile
                                        false,                      // log to syslog ?
                                        false,                      // trace messages ?
                                        trace_information,          // debug messages ?
                                        NULL))                      // logger
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeLogging(), aborting\n")));

    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step4: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (use_reactor,
                        false,
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
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  if (!Common_Tools::preInitializeSignals (signal_set,
                                           NET_EVENT_USE_REACTOR,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::preInitializeSignals(), aborting\n")));

    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  IRC_Client_SignalHandler signal_handler (use_reactor);

  // step5: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_SUCCESS;
  } // end IF

  // step6: initialize configuration objects

  // initialize protocol configuration
  Stream_CachedAllocatorHeap heap_allocator (NET_STREAM_MAX_MESSAGES,
                                             IRC_CLIENT_BUFFER_SIZE);
  IRC_Client_MessageAllocator message_allocator (NET_STREAM_MAX_MESSAGES,
                                                 &heap_allocator);

  IRC_Client_Configuration configuration;

  configuration.streamConfiguration.streamConfiguration.bufferSize =
      IRC_CLIENT_BUFFER_SIZE;
  configuration.streamConfiguration.streamConfiguration.messageAllocator =
      &message_allocator;
  configuration.streamConfiguration.streamConfiguration.statisticReportingInterval =
      reporting_interval;
  configuration.streamConfiguration.debugScanner = IRC_CLIENT_DEF_LEX_TRACE;
  configuration.streamConfiguration.debugParser = debug;

  IRC_Client_GTK_CBData user_data;
  user_data.configuration = &configuration;
  user_data.UIFileDirectory = UIDefinitionFile_directory;
//   userData.phoneBook;
//   userData.loginOptions.password = ;
  user_data.configuration->protocolConfiguration.loginOptions.nickname =
      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_IRC_NICK);
//   userData.loginOptions.user.username = ;
  std::string hostname;
  if (!Net_Common_Tools::getHostname (hostname))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::getHostname(), aborting\n")));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  if (IRC_CLIENT_CNF_IRC_USERMSG_TRADITIONAL)
  {
    configuration.protocolConfiguration.loginOptions.user.hostname.discriminator =
      IRC_Client_IRCLoginOptions::User::Hostname::STRING;
    configuration.protocolConfiguration.loginOptions.user.hostname.string =
      &hostname;
  } // end IF
  else
  {
    configuration.protocolConfiguration.loginOptions.user.hostname.discriminator =
      IRC_Client_IRCLoginOptions::User::Hostname::BITMASK;
    // *NOTE*: hybrid-7.2.3 seems to have a bug: 4 --> +i
    configuration.protocolConfiguration.loginOptions.user.hostname.mode =
      IRC_CLIENT_DEF_IRC_USERMODE;
  } // end ELSE
  configuration.protocolConfiguration.loginOptions.user.servername =
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_IRC_SERVERNAME);
  configuration.protocolConfiguration.loginOptions.channel =
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_IRC_CHANNEL);
  // populate user/realname
  Common_Tools::getCurrentUserName (configuration.protocolConfiguration.loginOptions.user.username,
                                    configuration.protocolConfiguration.loginOptions.user.realname);

  user_data.GTKState.RCFiles.push_back (UIRC_file);

  // step7: parse configuration file(s) (if any)
  if (!phonebook_file.empty ())
    do_parsePhonebookFile (phonebook_file,
                           user_data.phoneBook);
  if (!configuration_file.empty ())
    do_parseConfigurationFile (configuration_file,
                               configuration.protocolConfiguration.loginOptions,
                               user_data.phoneBook);

  // step8: initialize GTK UI
  Common_UI_GtkBuilderDefinition ui_definition (argc_in,
                                                argv_in);
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (argc_in,
                                                            argv_in,
                                                            &user_data.GTKState,
                                                            &ui_definition);

  // step9: do work
  ACE_High_Res_Timer timer;
  timer.start ();
  do_work (use_thread_pool,
           use_reactor,
           num_thread_pool_threads,
           user_data,
           ui_definition_filename,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);

  // debug info
  timer.stop ();
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time(working_time);
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
    // *PORTABILITY*: on Windows, fini ACE...
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

  // step10: clean up
  Common_Tools::finalizeSignals (signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Tools::finalizeLogging ();

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
#endif

  return EXIT_SUCCESS;
} // end main
