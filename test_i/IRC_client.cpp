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

//#include <iostream>
#include <list>
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

#include "common_file_tools.h"
#include "common_tools.h"

#include "common_ui_gtk_manager.h"

#include "stream_cachedallocatorheap.h"

#ifdef HAVE_CONFIG_H
#include "libacenetwork_config.h"
#endif

#include "net_client_connector_common.h"
#include "net_defines.h"

#include "IRC_common.h"

#include "IRC_client_configuration.h"
#include "IRC_client_defines.h"
#include "IRC_client_messageallocator.h"
#include "IRC_client_module_IRChandler.h"
#include "IRC_client_session_common.h"
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
  configuration_path = Common_File_Tools::getWorkingDirectory();
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
  path += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEF_INI_FILE);
  std::cout << ACE_TEXT ("-c [FILE]   : configuration file")
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
  std::cout << ACE_TEXT ("-r        : use reactor [")
            << NET_EVENT_USE_REACTOR
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-s [VALUE]: reporting interval (seconds: 0 --> OFF)")
            << ACE_TEXT (" [")
            << IRC_CLIENT_DEF_STATSINTERVAL
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
  configuration_path = Common_File_Tools::getWorkingDirectory();
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize configuration
  configurationFile_out          = configuration_path;
  configurationFile_out         += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out         += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEF_INI_FILE);

  debugParser_out                = false;
  logToFile_out                  = false;
  useReactor_out                 = NET_EVENT_USE_REACTOR;
  statisticReportingInterval_out = IRC_CLIENT_DEF_STATSINTERVAL;
  traceInformation_out           = false;
  printVersionAndExit_out        = false;
  numThreadPoolThreads_out       = IRC_CLIENT_DEF_NUM_TP_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("c:dlrs:tvx:"),
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

ACE_THR_FUNC_RETURN
event_manager_termination_function (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::event_manager_termination_function"));

  IRC_Client_ThreadData* thread_data_p =
    static_cast<IRC_Client_ThreadData*> (arg_in);
  ACE_ASSERT (thread_data_p);
  int result = -1;
  ACE_Time_Value delay (IRC_CLIENT_DEF_CONNECTION_TIMEOUT, 0);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("waiting for connection...(%#T)\n"),
              &delay));

  result = ACE_OS::sleep (delay);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::sleep: \"%m\", continuing\n")));

  Common_Tools::finalizeEventDispatch (thread_data_p->useReactor,
                                       thread_data_p->useProactor,
                                       -1);
                                       //thread_data_p->groupID);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("waiting for connection...DONE\n")));

  return NULL;
}

void
do_work (IRC_Client_Configuration& configuration_in,
         bool useReactor_in,
         const std::string& serverHostname_in,
         unsigned short serverPortNumber_in,
         IRC_Client_SignalHandler& signalHandler_in,
         unsigned int numDispatchThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

  // step1: initialize IRC handler module
  IRC_Client_StreamModuleConfiguration module_configuration;
  configuration_in.streamConfiguration.streamConfiguration.moduleConfiguration =
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
  IRCHandler_impl_p->initialize (NULL,
                                 configuration_in.streamConfiguration.streamConfiguration.messageAllocator,
                                 configuration_in.streamConfiguration.streamConfiguration.bufferSize,
                                 configuration_in.protocolConfiguration.automaticPong,
                                 configuration_in.protocolConfiguration.printPingDot);
  configuration_in.streamConfiguration.streamConfiguration.module =
    &IRC_handler;
  configuration_in.streamConfiguration.streamConfiguration.deleteModule =
    false;

  // step2: initialize event dispatch
  if (!Common_Tools::initializeEventDispatch (useReactor_in,
                                              numDispatchThreads_in,
                                              configuration_in.streamConfiguration.streamConfiguration.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF

  // step2: initialize client connector
  Net_SocketHandlerConfiguration socket_handler_configuration;
  socket_handler_configuration.bufferSize = IRC_CLIENT_BUFFER_SIZE;
  socket_handler_configuration.messageAllocator =
    configuration_in.streamConfiguration.streamConfiguration.messageAllocator;
  socket_handler_configuration.socketConfiguration =
    configuration_in.socketConfiguration;
  Net_Client_IConnector_t* connector_p = NULL;
  IRC_Client_Connection_Manager_t* connection_manager_p =
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  if (useReactor_in)
    ACE_NEW_NORETURN (connector_p,
                      IRC_Client_SessionConnector_t (&socket_handler_configuration,
                                                     connection_manager_p,
                                                     0));
  else
    ACE_NEW_NORETURN (connector_p,
                      IRC_Client_AsynchSessionConnector_t (&socket_handler_configuration,
                                                           connection_manager_p,
                                                           0));
  if (!connector_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
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
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));

    // clean up
    delete connector_p;

    return;
  } // end IF
  if (!signalHandler_in.initialize (signal_handler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_SignalHandler::initialize(), returning\n")));

    // clean up
    delete connector_p;

    return;
  } // end IF

  // step4: initialize connection manager
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  IRC_Client_SessionData session_data;
  connection_manager_p->set (configuration_in,
                             &session_data);

  // step5a: initialize worker(s)
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
    delete connector_p;

    return;
  } // end IF

  // step5b: (try to) connect to the server
  ACE_HANDLE handle =
      connector_p->connect (signal_handler_configuration.peerAddress);
  if (handle == ACE_INVALID_HANDLE)
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
                ACE_TEXT ("failed to connect to \"%s\": \"%m\", returning\n"),
                buffer));

    // clean up
    Common_Tools::finalizeEventDispatch (useReactor_in,
                                         !useReactor_in,
                                         group_id);
    delete connector_p;

    return;
  } // end IF

  // *NOTE*: from this point, clean up any remote connections

  // step6a: wait for connection
  IRC_Client_ThreadData thread_data;
  thread_data.groupID = group_id;
  thread_data.useProactor = !useReactor_in;
  thread_data.useReactor = useReactor_in;
  ACE_thread_t thread_id = -1;
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  //char thread_name[BUFSIZ];
  //ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  char* thread_name_p = NULL;
  ACE_NEW_NORETURN (thread_name_p,
                    char[BUFSIZ]);
  if (!thread_name_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

    // clean up
    Common_Tools::finalizeEventDispatch (useReactor_in,
                                         !useReactor_in,
                                         group_id);
    delete connector_p;

    return;
  } // end IF
  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
  const char* thread_name_2 = thread_name_p;
  int group_id_2 = (COMMON_EVENT_DISPATCH_THREAD_GROUP_ID + 1);
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  result =
    thread_manager_p->spawn (::event_manager_termination_function, // function
                             &thread_data,                         // argument
                             (THR_NEW_LWP      |
                              THR_JOINABLE     |
                              THR_INHERIT_SCHED),                  // flags
                             &thread_id,                           // thread id
                             &thread_handle,                       // thread handle
                             ACE_DEFAULT_THREAD_PRIORITY,          // priority
                             group_id_2,                           // group id
                             NULL,                                 // stack
                             0,                                    // stack size
                             //&thread_name);                        // name
                             &thread_name_2);                      // name
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));

    // clean up
    Common_Tools::finalizeEventDispatch (useReactor_in,
                                         !useReactor_in,
                                         group_id);
    delete connector_p;

    return;
  } // end IF
  Common_Tools::dispatchEvents (useReactor_in,
                                !useReactor_in,
                                group_id);
  if (connection_manager_p->numConnections () < 1)
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
                ACE_TEXT ("failed to connect to \"%s\": \"%m\", returning\n"),
                buffer));

    // clean up
    delete connector_p;

    return;
  } // end IF

  // step6b: dispatch events
  if (!Common_Tools::startEventDispatch (use_reactor,
                                         numDispatchThreads_in,
                                         group_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::startEventDispatch(), returning\n")));

    // clean up
    delete connector_p;

    return;
  } // end IF
  Common_Tools::dispatchEvents (useReactor_in,
                                !useReactor_in,
                                group_id);

  // step7: clean up
  connection_manager_p->abort ();
  connection_manager_p->wait ();
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

  // initialize return value(s)
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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  std::string configuration_file = configuration_path;
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEF_INI_FILE);

  bool debug_parser                         = false;
  bool log_to_file                          = false;
  bool use_reactor                          = NET_EVENT_USE_REACTOR;
  unsigned int statistic_reporting_interval = IRC_CLIENT_DEF_STATSINTERVAL;
  bool trace_information                    = false;
  bool print_version_and_exit               = false;
  unsigned int num_thread_pool_threads      = IRC_CLIENT_DEF_NUM_TP_THREADS;
  if (!do_processArguments (argc_in,
                            argv_in,
                            configuration_file,
                            debug_parser,
                            log_to_file,
                            use_reactor,
                            statistic_reporting_interval,
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

  // step2b: validate argument(s)
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
                                           NET_EVENT_USE_REACTOR,
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
  IRC_Client_SignalHandler signal_handler (use_reactor);

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
    statistic_reporting_interval;
  configuration.streamConfiguration.debugScanner = IRC_CLIENT_DEF_LEX_TRACE;
  configuration.streamConfiguration.debugParser = IRC_CLIENT_DEF_YACC_TRACE;

  configuration.protocolConfiguration.loginOptions.nick =
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
//    // *PORTABILITY*: on Windows, fini ACE...
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

  std::string                server_hostname =
      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEF_SERVER_HOSTNAME);
  unsigned short             server_port_number = IRC_CLIENT_DEF_SERVER_PORT;

  // step7: parse configuration file(s) (if any)
  if (!configuration_file.empty ())
    do_parseConfigurationFile (configuration_file,
                               configuration.protocolConfiguration.loginOptions,
                               server_hostname,
                               server_port_number);

  // step8: do work
  ACE_High_Res_Timer timer;
  timer.start();
  do_work (configuration,
           use_reactor,
           server_hostname,
           server_port_number,
           signal_handler,
           num_thread_pool_threads);

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
