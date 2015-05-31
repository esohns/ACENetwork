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

#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#include "common_file_tools.h"
#include "common_tools.h"

#include "common_ui_gtk_manager.h"

#include "stream_allocatorheap.h"

#ifdef HAVE_CONFIG_H
#include "libacenetwork_config.h"
#endif

#include "net_client_connector_common.h"
//#include "net_common.h"
#include "net_defines.h"

#include "IRC_client_configuration.h"
#include "IRC_client_defines.h"
#include "IRC_client_messageallocator.h"
#include "IRC_client_module_IRChandler.h"
#include "IRC_client_network.h"
#include "IRC_client_signalhandler.h"

void
do_printUsage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (ios::boolalpha);

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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT ("usage: ")
            << programName_in
            << ACE_TEXT (" [OPTIONS]")
            << std::endl << std::endl;
  std::cout << ACE_TEXT ("currently available options:") << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
#if defined (DEBUG_DEBUGGER)
  path += ACE_TEXT_ALWAYS_CHAR ("protocol");
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
#endif
  path += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEF_INI_FILE);
  std::cout << ACE_TEXT ("-c [FILE]   : config file")
            << ACE_TEXT (" [\"")
            << path
            << ACE_TEXT ("\"]")
            << std::endl;
  std::cout << ACE_TEXT ("-d          : debug parser")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-l          : log to a file")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-t          : trace information")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-v          : print version information and exit")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-x [VALUE]  : #thread pool threads ([")
            << IRC_CLIENT_DEF_NUM_TP_THREADS
            << ACE_TEXT ("]")
            << std::endl;
} // end print_usage

bool
do_processArguments (int argc_in,
                     ACE_TCHAR* argv_in[], // cannot be const...
                     std::string& configurationFile_out,
                     bool& debugParser_out,
                     bool& logToFile_out,
                     bool& traceInformation_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numThreadPoolThreads_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize configuration
  configurationFile_out = configuration_path;
  configurationFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEF_INI_FILE);

  debugParser_out          = false;
  logToFile_out            = false;
  traceInformation_out     = false;
  printVersionAndExit_out  = false;
  numThreadPoolThreads_out = IRC_CLIENT_DEF_NUM_TP_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("c:dltvx:"),
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
      case 'c':
      {
        configurationFile_out = argumentParser.opt_arg ();
        break;
      }
      case 'd':
      {
        debugParser_out = true;
        break;
      }
      case 'l':
      {
        logToFile_out = true;
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
do_work (IRC_Client_Configuration& configuration_in,
         const std::string& serverHostname_in,
         unsigned short serverPortNumber_in,
         IRC_Client_SignalHandler& signalHandler_in,
         unsigned int numDispatchThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

  // step1: initialize event dispatch
  if (!Common_Tools::initializeEventDispatch (NET_EVENT_USE_REACTOR,
                                              numDispatchThreads_in,
                                              configuration_in.streamConfiguration.streamConfiguration.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize event dispatch, returning\n")));
    return;
  } // end IF

  // step2: initialize client connector
  Net_SocketHandlerConfiguration_t socket_handler_configuration;
  socket_handler_configuration.bufferSize = IRC_CLIENT_BUFFER_SIZE;
  socket_handler_configuration.messageAllocator =
    configuration_in.streamConfiguration.streamConfiguration.messageAllocator;
  socket_handler_configuration.socketConfiguration =
    configuration_in.socketConfiguration;
  Net_Client_IConnector_t* connector_p = NULL;
  if (NET_EVENT_USE_REACTOR)
    ACE_NEW_NORETURN (connector_p,
                      IRC_Client_Connector_t (&socket_handler_configuration,
                                              IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance (),
                                              0));
  else
    ACE_NEW_NORETURN (connector_p,
                      IRC_Client_AsynchConnector_t (&socket_handler_configuration,
                                                    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance (),
                                                    0));
  if (!connector_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    return;
  } // end IF

  // step3: initialize signal handling
  IRC_Client_SignalHandlerConfiguration_t signal_handler_configuration;
  //  ACE_OS::memset (&signal_handler_configuration,
  //                  0,
  //                  sizeof (signal_handler_configuration));
//  signal_handler_configuration.actionTimerId = -1;
  signal_handler_configuration.connector = connector_p;
  result =
      signal_handler_configuration.peerAddress.set (serverPortNumber_in,
                                                    serverHostname_in.c_str (),
                                                    1,
                                                    AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", aborting\n")));

    // clean up
    delete connector_p;

    return;
  } // end IF
  if (!signalHandler_in.initialize (signal_handler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, aborting\n")));

    // clean up
    delete connector_p;

    return;
  } // end IF

  // step4a: initialize connection manager
  IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->initialize (std::numeric_limits<unsigned int>::max ());
  IRC_Client_SessionData session_data;
  IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->set (configuration_in,
                                                            &session_data);

  //// step5a: start GTK event loop
  //COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->start ();
  //if (!COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->isRunning ())
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to start GTK event dispatch, aborting\n")));

  //  // clean up
  //  delete connector_p;
  //  Common_Tools::finalizeSignals (signal_set,
  //                                 NET_EVENT_USE_REACTOR,
  //                                 previous_signal_actions);

  //  return;
  //} // end IF

  // step5b: initialize worker(s)
  int group_id = -1;
  if (!Common_Tools::startEventDispatch (NET_EVENT_USE_REACTOR,
                                         numDispatchThreads_in,
                                         group_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, aborting\n")));

    // clean up
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();
    delete connector_p;

    return;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("started event dispatch...\n")));

  // step6: (try to) connect to the server
  bool result_2 =
      connector_p->connect (signal_handler_configuration.peerAddress);
  if (!NET_EVENT_USE_REACTOR)
  {
    ACE_Time_Value delay (1, 0);
    ACE_OS::sleep (delay);
    if (IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->numConnections () != 1)
      result_2 = false;
  } // end IF
  if (!result_2)
  {
    // debug info
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    result =
        signal_handler_configuration.peerAddress.addr_to_string (buffer,
                                                                 sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect(\"%s\"): \"%m\", returning\n"),
                buffer));

    // clean up
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();
    delete connector_p;

    return;
  } // end IF

  // *NOTE*: from this point on, we need to clean up any remote connections !

  // step7: dispatch events
  // *NOTE*: when using a thread pool, handle things differently...
  if (numDispatchThreads_in > 1)
  {
    if (ACE_Thread_Manager::instance ()->wait_grp (group_id) == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", continuing\n"),
                  group_id));
  } // end IF
  else
  {
    if (NET_EVENT_USE_REACTOR)
    {
      /*      // *WARNING*: restart system calls (after e.g. SIGINT) for the reactor
      ACE_Reactor::instance()->restart(1);
      */
      if (ACE_Reactor::instance ()->run_reactor_event_loop (0) == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to handle events: \"%m\", aborting\n")));
    } // end IF
    else
      if (ACE_Proactor::instance ()->proactor_run_event_loop (0) == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to handle events: \"%m\", aborting\n")));
  } // end ELSE

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished event dispatch...\n")));

  // step8: clean up
  IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->abort ();
  IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ()->wait ();
  delete connector_p;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

void
do_parseConfigurationFile (const std::string& configFilename_in,
                           IRC_Client_IRCLoginOptions& loginOptions_out,
                           std::string& serverHostname_out,
                           unsigned short& serverPortNumber_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_parseConfigurationFile"));

  // init return value(s)
  serverHostname_out   = ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_SERVER_HOSTNAME);
  serverPortNumber_out = IRC_CLIENT_DEF_SERVER_PORT;

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
  ACE_TString val_name, val_value;
  ACE_Configuration::VALUETYPE val_type;
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
      loginOptions_out.password = ACE_TEXT_ALWAYS_CHAR (val_value.c_str ());
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("nick"))
      loginOptions_out.nick = ACE_TEXT_ALWAYS_CHAR (val_value.c_str());
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("user"))
      loginOptions_out.user.username = ACE_TEXT_ALWAYS_CHAR (val_value.c_str ());
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("realname"))
      loginOptions_out.user.realname = ACE_TEXT_ALWAYS_CHAR (val_value.c_str ());
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("channel"))
      loginOptions_out.channel = ACE_TEXT_ALWAYS_CHAR (val_value.c_str ());
    else
      ACE_ERROR ((LM_ERROR,
                  ACE_TEXT ("unexpected key \"%s\", continuing\n"),
                  ACE_TEXT (val_name.c_str ())));

    ++val_index;
  } // end WHILE

  // find/open "connection" section...
  if (config_heap.open_section (config_heap.root_section(),
                                ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_CONNECTION_SECTION_HEADER),
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
                  ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(%s), returning\n"),
                  ACE_TEXT (val_name.c_str ())));
      return;
    } // end IF

//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("enumerated %s, type %d\n"),
//                 ACE_TEXT (val_name.c_str()),
//                 val_type));

    // *TODO*: move these strings...
    if (val_name == ACE_TEXT_ALWAYS_CHAR ("server"))
      serverHostname_out = ACE_TEXT_ALWAYS_CHAR (val_value.c_str ());
    else if (val_name == ACE_TEXT_ALWAYS_CHAR ("port"))
      serverPortNumber_out = ::atoi (val_value.c_str ());
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
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  NETWORK_TRACE (ACE_TEXT ("::main"));

  int result = -1;

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  // step1: initialize
//  // *PORTABILITY*: on Windows, init ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  if (ACE::init () == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
//    return EXIT_FAILURE;
//  } // end IF
//#endif

  // step1 initialize/validate configuration

  // step1a: process commandline arguments
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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  std::string configuration_file = configuration_path;
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
#if defined (DEBUG_DEBUGGER)
  configuration_file += ACE_TEXT_ALWAYS_CHAR ("protocol");
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
#endif
  configuration_file += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEF_INI_FILE);

  bool debug_parser                    = false;
  bool log_to_file                     = false;
  bool trace_information               = false;
  bool print_version_and_exit          = false;
  unsigned int num_thread_pool_threads = IRC_CLIENT_DEF_NUM_TP_THREADS;
  if (!do_processArguments (argc_in,
                            argv_in,
                            configuration_file,
                            debug_parser,
                            log_to_file,
                            trace_information,
                            print_version_and_exit,
                            num_thread_pool_threads))
  {
    // make 'em learn...
    do_printUsage (ACE::basename (argv_in[0]));

//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF

  // validate argument(s)
  if (!Common_File_Tools::isReadable (configuration_file))
  {
    // make 'em learn...
    do_printUsage (ACE::basename (argv_in[0]));

//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF

  // initialize logging and/or tracing
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

//    // *PORTABILITY*: on Windows, need to fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF

  // parse configuration file (if any)
  IRC_Client_IRCLoginOptions login_options;
  login_options.nick = IRC_CLIENT_DEF_IRC_NICK;
  std::string hostname;
  if (!Net_Common_Tools::getHostname (hostname))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::getHostname(): \"%m\", aborting\n")));

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
  if (IRC_CLIENT_CNF_IRC_USERMSG_TRADITIONAL)
  {
    login_options.user.hostname.discriminator =
        IRC_Client_IRCLoginOptions::User::Hostname::STRING;
    login_options.user.hostname.string = &hostname;
  } // end IF
  else
  {
    login_options.user.hostname.discriminator =
        IRC_Client_IRCLoginOptions::User::Hostname::BITMASK;
    // *NOTE*: hybrid-7.2.3 seems to have a bug: 4 --> +i
    login_options.user.hostname.mode = IRC_CLIENT_DEF_IRC_USERMODE;
  } // end ELSE
  login_options.user.servername =
      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_IRC_SERVERNAME);
  login_options.channel = ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_IRC_CHANNEL);
  // populate user/realname
  Common_Tools::getCurrentUserName (login_options.user.username,
                                    login_options.user.realname);
  std::string                server_hostname =
      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_SERVER_HOSTNAME);
  unsigned short             server_port_number = IRC_CLIENT_DEF_SERVER_PORT;
  if (!configuration_file.empty ())
    do_parseConfigurationFile (configuration_file,
                               login_options,
                               server_hostname,
                               server_port_number);

  // (pre-)initialize signal handling
  ACE_Sig_Set signal_set (0);
  ACE_Sig_Set ignored_signal_set (0);
  do_initializeSignals (NET_EVENT_USE_REACTOR,
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
                                           NET_EVENT_USE_REACTOR,
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
  IRC_Client_SignalHandler signal_handler (NET_EVENT_USE_REACTOR);

  // handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_SUCCESS;
  } // end IF

  // initialize configuration object
  Stream_AllocatorHeap heap_allocator;
  IRC_Client_MessageAllocator message_allocator (NET_STREAM_MAX_MESSAGES,
                                                 &heap_allocator);
  IRC_Client_Module_IRCHandler_Module IRC_handler_module (std::string ("IRCHandler"),
                                                                NULL);
  IRC_Client_Module_IRCHandler* IRC_handler_impl = NULL;
  IRC_handler_impl =
    dynamic_cast<IRC_Client_Module_IRCHandler*> (IRC_handler_module.reader ());
  if (!IRC_handler_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_IRCHandler> failed, aborting\n")));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF
  if (!IRC_handler_impl->initialize (&message_allocator,
                                     IRC_CLIENT_BUFFER_SIZE,
                                     true,                         // auto-answer "ping" as a client ?...
                                     true))                        // clients print ('.') dots for received "pings"...
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (IRC_handler_module.name ())));

    Common_Tools::finalizeSignals (signal_set,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Tools::finalizeLogging ();
//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF

  IRC_Client_Configuration configuration;
  // ************ socket configuration data ************
  configuration.socketConfiguration.bufferSize =
      NET_SOCKET_DEFAULT_RECEIVE_BUFFER_SIZE;

  // ************ stream configuration data ****************
  configuration.streamConfiguration.streamConfiguration.messageAllocator =
      &message_allocator;
  configuration.streamConfiguration.streamConfiguration.bufferSize =
      IRC_CLIENT_BUFFER_SIZE;
  configuration.streamConfiguration.streamConfiguration.module
      = &IRC_handler_module;
  configuration.streamConfiguration.streamConfiguration.statisticReportingInterval =
      0; // == off
  configuration.streamConfiguration.crunchMessageBuffers =
      IRC_CLIENT_DEF_CRUNCH_MESSAGES;
  configuration.streamConfiguration.debugScanner = debug_parser;
  configuration.streamConfiguration.debugParser = debug_parser;

  // ************ protocol configuration data **************
  configuration.protocolConfiguration.loginOptions = login_options;

  ACE_High_Res_Timer timer;
  timer.start();
  // step2: do actual work
  do_work (configuration,
           server_hostname,
           server_port_number,
           signal_handler,
           num_thread_pool_threads);
  // clean up
  IRC_handler_module.close ();
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
//    // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

    return EXIT_FAILURE;
  } // end IF
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (ACE_Profile_Timer::Rusage));
  process_profile.elapsed_rusage(elapsed_rusage);
  ACE_Time_Value user_time(elapsed_rusage.ru_utime);
  ACE_Time_Value system_time(elapsed_rusage.ru_stime);
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
              user_time_string.c_str (),
              system_time_string.c_str ()));
#endif

  Common_Tools::finalizeSignals (signal_set,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Tools::finalizeLogging ();

//  // *PORTABILITY*: on Windows, fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  if (ACE::fini () == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

  return EXIT_SUCCESS;
} // end main
