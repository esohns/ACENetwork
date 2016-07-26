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

#if defined (ACE_WIN32) || defined (ACE_WIN32)
#include "curses.h"
#else
#include "ncurses.h"
// *NOTE*: the ncurses "timeout" macros conflicts with
//         ACE_Synch_Options::timeout. Since not currently being used, it's safe
//         to undefine...
#undef timeout
#endif

#include "common_file_tools.h"
#include "common_tools.h"

#include "stream_cachedallocatorheap.h"

#ifdef HAVE_CONFIG_H
#include "libACENetwork_config.h"
#endif

#include "net_defines.h"

#include "irc_common.h"
#include "irc_defines.h"

#include "IRC_client_configuration.h"
#include "IRC_client_curses.h"
#include "IRC_client_defines.h"
#include "IRC_client_messageallocator.h"
#include "IRC_client_module_IRChandler.h"
#include "IRC_client_session_common.h"
#include "IRC_client_signalhandler.h"
#include "IRC_client_tools.h"

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
  path += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEFAULT_INI_FILE);
  std::cout << ACE_TEXT ("-c [FILE] : configuration file")
            << ACE_TEXT (" [\"")
            << path
            << ACE_TEXT ("\"]")
            << std::endl;
  std::cout << ACE_TEXT ("-d        : debug parser")
            << ACE_TEXT (" [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-l        : log to a file")
            << ACE_TEXT (" [")
            << IRC_CLIENT_SESSION_DEFAULT_LOG
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-n        : use (PD|n)curses library [")
            << IRC_CLIENT_SESSION_USE_CURSES
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-r        : use reactor [")
            << IRC_CLIENT_DEFAULT_USE_REACTOR
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-s [VALUE]: reporting interval (seconds: 0 --> OFF)")
            << ACE_TEXT (" [")
            << IRC_CLIENT_DEFAULT_STATISTIC_REPORTING_INTERVAL
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
            << IRC_CLIENT_DEFAULT_NUMBER_OF_TP_THREADS
            << ACE_TEXT ("]")
            << std::endl;
} // end print_usage

bool
do_processArguments (int argc_in,
                     ACE_TCHAR* argv_in[], // cannot be const...
                     std::string& configurationFile_out,
                     bool& debugParser_out,
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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize configuration
  configurationFile_out          = configuration_path;
  configurationFile_out         += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out         += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEFAULT_INI_FILE);

  debugParser_out                = false;
  logToFile_out                  = IRC_CLIENT_SESSION_DEFAULT_LOG;
  useCursesLibrary_out           = IRC_CLIENT_SESSION_USE_CURSES;
  useReactor_out                 = NET_EVENT_USE_REACTOR;
  statisticReportingInterval_out =
      IRC_CLIENT_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  traceInformation_out           = false;
  printVersionAndExit_out        = false;
  numThreadPoolThreads_out       = IRC_CLIENT_DEFAULT_NUMBER_OF_TP_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT ("c:dlnrs:tvx:"),
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
connection_setup_curses_function (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::connection_setup_curses_function"));

  ACE_THR_FUNC_RETURN return_value;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return_value = -1;
#else
  return_value = arg_in;
#endif

  IRC_Client_InputThreadData* thread_data_p =
    static_cast<IRC_Client_InputThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (thread_data_p);

  int result = -1;
  ACE_Time_Value delay (IRC_CLIENT_CONNECTION_ASYNCH_TIMEOUT_INTERVAL, 0);

  IRC_Client_IConnection_t* connection_p = NULL;
  IRC_Client_ISocketConnection_t* socket_connection_p = NULL;
  bool result_2 = false;
  const Stream_Module_t* module_p = NULL;
  const IRC_Client_Stream* stream_p = NULL;
  IRC_IControl* icontrol_p = NULL;
  IRC_IRegistrationStateMachine_t* iregistration_p = NULL;
  ACE_Time_Value deadline = ACE_Time_Value::zero;
  std::string channel_string;
  string_list_t channels, keys;
  IRC_Client_IConnection_Manager_t* connection_manager_p =
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);

  // step1: wait for connection ?
  if (!thread_data_p->useReactor)
  {
    ACE_Time_Value deadline =
      (COMMON_TIME_NOW +
       ACE_Time_Value (IRC_CLIENT_CONNECTION_ASYNCH_TIMEOUT, 0));

    do
    {
      result = ACE_OS::sleep (delay);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    &delay));

      if (connection_manager_p->count () >= 1)
        break; // done
    } while (COMMON_TIME_NOW < deadline);
  } // end IF
  // *NOTE*: signal main thread (resumes dispatching)
  Common_Tools::finalizeEventDispatch (thread_data_p->useReactor,
                                       !thread_data_p->useReactor,
                                       -1);

  connection_p = connection_manager_p->operator[] (0);
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_IConnection_Manager_t::operator[0]: \"%m\", aborting\n")));
    goto clean_up;
  } // end IF

  // step2: register connection with the server
  // *NOTE*: this entails a little delay (waiting for the welcome notice...)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registering IRC connection...\n")));

  socket_connection_p =
    dynamic_cast<IRC_Client_ISocketConnection_t*> (connection_p);
  if (!socket_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<IRC_Client_ISocketConnection_t>(%@): \"%m\", aborting\n"),
                connection_p));
    goto clean_up;
  } // end IF
  stream_p = &socket_connection_p->stream ();
  for (Stream_Iterator_t iterator (*stream_p);
       (iterator.next (module_p) != 0);
       iterator.advance ())
    if (ACE_OS::strcmp (module_p->name (),
                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_HANDLER_MODULE_NAME)) == 0)
      break;
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("module \"%s\" not found, aborting\n"),
                ACE_TEXT (IRC_CLIENT_HANDLER_MODULE_NAME)));
    goto clean_up;
  } // end IF
  icontrol_p =
    dynamic_cast<IRC_IControl*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!icontrol_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<IRC_IControl*>(0x%@), aborting\n"),
                connection_p));
    goto clean_up;
  } // end IF
  try
  {
    result_2 =
      icontrol_p->registerc (thread_data_p->configuration->protocolConfiguration.loginOptions);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_IControl::registerc(), continuing\n")));
  }
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_IControl::registerc(), aborting\n")));
    goto clean_up;
  } // end IF

  // step3: wait for registration to complete
  iregistration_p =
    dynamic_cast<IRC_IRegistrationStateMachine_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!iregistration_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<IRC_IRegistrationStateMachine_t*>: \"%m\", aborting\n")));
    goto clean_up;
  } // end ELSE
  // *NOTE*: this entails a little delay (waiting for connection registration...)
  delay.set (IRC_MAXIMUM_WELCOME_DELAY, 0);
  // *NOTE*: cannot use COMMON_TIME_NOW, as this is a high precision monotonous
  //         clock... --> use standard getimeofday
  deadline = ACE_OS::gettimeofday () + delay;
  try
  {
    result_2 = iregistration_p->wait (REGISTRATION_STATE_FINISHED,
                                      &deadline);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IRegistration_t::wait(%#T), continuing\n"),
                &delay));
    result_2 = false;
  }
  if (!result_2 ||
      (iregistration_p->current () != REGISTRATION_STATE_FINISHED))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_IRegistration_t::wait(%#T), aborting\n"),
                &delay));
    goto clean_up;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registering IRC connection...DONE\n")));

  // step4: join a channel
  channel_string =
    thread_data_p->configuration->protocolConfiguration.loginOptions.channel;
  // sanity check(s): has '#' prefix ?
  if (channel_string.find ('#', 0) != 0)
    channel_string.insert (channel_string.begin (), '#');
  // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH characters ?
  // *TODO*: support the CHANNELLEN=xxx "feature" of the server...
  if (channel_string.size () > IRC_PRT_MAXIMUM_CHANNEL_LENGTH)
    channel_string.resize (IRC_PRT_MAXIMUM_CHANNEL_LENGTH);
  channels.push_back (channel_string);
  try
  {
    icontrol_p->join (channels, keys);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::join(\"%s\"), aborting\n"),
                ACE_TEXT (channel_string.c_str ())));
    goto clean_up;
  }

  // step5: run curses event dispatch ?
  if (thread_data_p->cursesState)
  {
    //IRC_Client_ISession_t* session_p =
    //  dynamic_cast<IRC_Client_ISession_t*> (connection_p);
    //if (!session_p)
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to dynamic_cast<IRC_Client_ISession_t*>(0x%@), aborting\n"),
    //              connection_p));

    //  // clean up
    //  connection_p->decrease ();
    //  Common_Tools::finalizeEventDispatch (thread_data_p->useReactor,
    //                                       !thread_data_p->useReactor,
    //                                       -1);

    //  goto clean_up;
    //} // end IF
    //const IRC_Client_ConnectionState& connection_state_r = session_p->state ();
    const IRC_Client_SessionState& session_state_r =
      connection_p->state ();
    thread_data_p->cursesState->sessionState =
      &const_cast<IRC_Client_SessionState&> (session_state_r);

    // step6: clean up
    connection_p->decrease ();
    connection_p = NULL;

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("running curses dispatch loop...\n")));

    bool result_2 = curses_main (*thread_data_p->cursesState,
                                 icontrol_p);
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::curses_main(), aborting\n")));
      goto clean_up;
    } // end IF
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("running curses dispatch loop...DONE\n")));
  } // end IF
  else
  {
    // step6: clean up
    connection_p->decrease ();
  } // end ELSE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return_value = 0;
#else
  return_value = NULL;
#endif

  goto done;

clean_up:
  if (connection_p)
    connection_p->decrease ();

  Common_Tools::finalizeEventDispatch (thread_data_p->useReactor,
                                       !thread_data_p->useReactor,
                                       -1);

done:
  return return_value;
}

void
do_work (IRC_Client_Configuration& configuration_in,
         bool useCursesLibrary_in,
         const std::string& serverHostname_in,
         unsigned short serverPortNumber_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         IRC_Client_SignalHandler& signalHandler_in,
         unsigned int numberOfDispatchThreads_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;
  // *TODO*: clean this up
  IRC_Client_CursesState curses_state;
  if (useCursesLibrary_in)
    configuration_in.cursesState = &curses_state;

  // step1: initialize IRC handler module
  configuration_in.moduleHandlerConfiguration.streamConfiguration =
      &configuration_in.streamConfiguration;
  configuration_in.moduleHandlerConfiguration.protocolConfiguration =
      &configuration_in.protocolConfiguration;
  configuration_in.streamConfiguration.moduleHandlerConfiguration =
      &configuration_in.moduleHandlerConfiguration;
  configuration_in.streamConfiguration.moduleConfiguration =
      &configuration_in.moduleConfiguration;
  IRC_Client_Module_IRCHandler_Module IRC_handler (ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_HANDLER_MODULE_NAME),
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
  configuration_in.streamConfiguration.cloneModule = true;
  configuration_in.streamConfiguration.deleteModule = false;
  configuration_in.streamConfiguration.module = &IRC_handler;
  if (!IRCHandler_impl_p->initialize (configuration_in.moduleHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_Module_IRCHandler_Module::initialize(), returning\n")));
    return;
  } // end IF

  // step2: initialize event dispatch
  struct Common_DispatchThreadData thread_data;
  thread_data.numberOfDispatchThreads = numberOfDispatchThreads_in;
  thread_data.useReactor = configuration_in.useReactor;
  if (!Common_Tools::initializeEventDispatch (configuration_in.useReactor,
                                              (thread_data.numberOfDispatchThreads > 1),
                                              thread_data.numberOfDispatchThreads,
                                              thread_data.proactorType,
                                              thread_data.reactorType,
                                              configuration_in.streamConfiguration.serializeOutput))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF

  // step3: initialize client connector
  //IRC_Client_SocketHandlerConfiguration* socket_handler_configuration_p = NULL;
  //ACE_NEW_NORETURN (socket_handler_configuration_p,
  //                  IRC_Client_SocketHandlerConfiguration ());
  //if (!socket_handler_configuration_p)
  //{
  //  ACE_DEBUG ((LM_CRITICAL,
  //              ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
  //  return;
  //} // end IF
  //configuration_in.socketHandlerConfiguration.bufferSize =
  //  IRC_CLIENT_BUFFER_SIZE;
  configuration_in.socketHandlerConfiguration.messageAllocator =
    configuration_in.streamConfiguration.messageAllocator;
  configuration_in.socketHandlerConfiguration.socketConfiguration =
    &configuration_in.socketConfiguration;
  configuration_in.socketHandlerConfiguration.statisticReportingInterval =
    configuration_in.streamConfiguration.statisticReportingInterval;
  //IRC_Client_SessionState session_state;
  //session_state.configuration = configuration_in;
  //IRC_Client_ConnectorConfiguration connector_configuration;
  IRC_Client_Connection_Manager_t* connection_manager_p =
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  //connector_configuration.connectionManager = connection_manager_p;
  //connector_configuration.userData = &configuration_in.streamUserData;
  //connector_configuration.socketHandlerConfiguration =
  //  &configuration_in.socketHandlerConfiguration;
  IRC_Client_SessionConnector_t connector (connection_manager_p,
                                           configuration_in.streamConfiguration.statisticReportingInterval);
  IRC_Client_AsynchSessionConnector_t asynch_connector (connection_manager_p,
                                                        configuration_in.streamConfiguration.statisticReportingInterval);
  IRC_Client_IConnector_t* connector_p = NULL;
  if (configuration_in.useReactor)
    connector_p = &connector;
  else
    connector_p = &asynch_connector;
  //if (!connector_p->initialize (connector_configuration))
  if (!connector_p->initialize (configuration_in.socketHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
    return;
  } // end IF

  // step3: initialize signal handling
  IRC_Client_SignalHandlerConfiguration signal_handler_configuration;
  signal_handler_configuration.connector = connector_p;
  signal_handler_configuration.cursesState = &curses_state;
  result =
      signal_handler_configuration.peerAddress.set (serverPortNumber_in,
                                                    serverHostname_in.c_str (),
                                                    1,
                                                    AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", returning\n")));
    return;
  } // end IF
  if (!signalHandler_in.initialize (signal_handler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_SignalHandler::initialize(), returning\n")));
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

  // step4: initialize connection manager
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max ());
  connection_manager_p->set (configuration_in,
                             //configuration_in.streamUserData);
                             NULL);

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step5a: initialize worker(s)
  if (!Common_Tools::startEventDispatch (thread_data,
                                         configuration_in.groupID))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::startEventDispatch(), returning\n")));
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
    Common_Tools::finalizeEventDispatch (configuration_in.useReactor,
                                         !configuration_in.useReactor,
                                         configuration_in.groupID);

    return;
  } // end IF

  // *NOTE*: from this point, clean up any remote connections

  // step6a: wait for connection / setup

  IRC_Client_InputThreadData input_thread_data;
  input_thread_data.configuration = &configuration_in;
  input_thread_data.groupID = configuration_in.groupID;
  input_thread_data.moduleHandlerConfiguration =
      &configuration_in.moduleHandlerConfiguration;
  if (useCursesLibrary_in)
    input_thread_data.cursesState = &curses_state;
  input_thread_data.useReactor = configuration_in.useReactor;
  ACE_thread_t thread_id = -1;
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  //char thread_name[BUFSIZ];
  //ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  char* thread_name_p = NULL;
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

    return;
  } // end IF
  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
  const char* thread_name_2 = thread_name_p;
  int group_id_2 = (COMMON_EVENT_THREAD_GROUP_ID + 1); // *TODO*
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  result =
    thread_manager_p->spawn (::connection_setup_curses_function, // function
                             &input_thread_data,                 // argument
                             (THR_NEW_LWP      |
                              THR_JOINABLE     |
                              THR_INHERIT_SCHED),                // flags
                             &thread_id,                         // thread id
                             &thread_handle,                     // thread handle
                             ACE_DEFAULT_THREAD_PRIORITY,        // priority
                             group_id_2,                         // group id
                             NULL,                               // stack
                             0,                                  // stack size
                             &thread_name_2);                    // name
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));

    // clean up
    Common_Tools::finalizeEventDispatch (configuration_in.useReactor,
                                         !configuration_in.useReactor,
                                         configuration_in.groupID);

    return;
  } // end IF
  Common_Tools::dispatchEvents (configuration_in.useReactor,
                                configuration_in.groupID);
  // *NOTE*: awoken by the worker thread (see above)...
  if (connection_manager_p->count () < 1)
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
    result = thread_manager_p->wait_grp (group_id_2);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", returning\n"),
                  group_id_2));

    return;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connected...\n")));

  // step6b: dispatch events
  if (!Common_Tools::startEventDispatch (thread_data,
                                         configuration_in.groupID))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::startEventDispatch(), returning\n")));

    // clean up
    result = thread_manager_p->wait_grp (group_id_2);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", returning\n"),
                  group_id_2));
    connection_manager_p->abort ();
    connection_manager_p->wait ();

    return;
  } // end IF
  Common_Tools::dispatchEvents (configuration_in.useReactor,
                                configuration_in.groupID);

  // step7: clean up
  result = thread_manager_p->wait_grp (group_id_2);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", returning\n"),
                group_id_2));
  connection_manager_p->abort ();
  connection_manager_p->wait ();

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
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
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_i");
#endif // #ifdef DEBUG_DEBUGGER

  std::string configuration_file_name        = configuration_path;
  configuration_file_name                   += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file_name                   +=
      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEFAULT_INI_FILE);

  bool debug_parser                          = false;
  bool log_to_file                           = IRC_CLIENT_SESSION_DEFAULT_LOG;
  bool use_curses_library                    = IRC_CLIENT_SESSION_USE_CURSES;
  bool use_reactor                           = IRC_CLIENT_DEFAULT_USE_REACTOR;
  unsigned int statistic_reporting_interval  =
      IRC_CLIENT_DEFAULT_STATISTIC_REPORTING_INTERVAL;
  bool trace_information                     = false;
  bool print_version_and_exit                = false;
  unsigned int number_of_thread_pool_threads =
      IRC_CLIENT_DEFAULT_NUMBER_OF_TP_THREADS;
  if (!do_processArguments (argc_in,
                            argv_in,
                            configuration_file_name,
                            debug_parser,
                            log_to_file,
                            use_curses_library,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            print_version_and_exit,
                            number_of_thread_pool_threads))
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
  if (!Common_File_Tools::isReadable (configuration_file_name))
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
  Stream_CachedAllocatorHeap_T<Stream_AllocatorConfiguration> heap_allocator (NET_STREAM_MAX_MESSAGES,
                                                                              IRC_BUFFER_SIZE);
  IRC_Client_MessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES,
                                                   &heap_allocator);

  IRC_Client_Configuration configuration;
  ////////////////////////////////////////
  configuration.streamConfiguration.messageAllocator = &message_allocator;
  configuration.streamConfiguration.statisticReportingInterval =
    statistic_reporting_interval;
  ////////////////////////////////////////
  configuration.protocolConfiguration.loginOptions.nickName =
    ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_NICKNAME);
  //   userData.loginOptions.user.username = ;
  std::string host_name;
  if (!Net_Common_Tools::getHostname (host_name))
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
  Common_Tools::getCurrentUserName (configuration.protocolConfiguration.loginOptions.user.userName,
                                    configuration.protocolConfiguration.loginOptions.user.realName);

  // step7: parse configuration file(s) (if any)
  host_name = ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEFAULT_SERVER_HOSTNAME);
  unsigned short port_number = IRC_DEFAULT_SERVER_PORT;
  if (!configuration_file_name.empty ())
  {
    IRC_Client_Connections_t connections;
    IRC_Client_Tools::parseConfigurationFile (configuration_file_name,
                                              configuration.protocolConfiguration.loginOptions,
                                              connections);
    if (!connections.empty ())
    {
      IRC_Client_ConnectionsIterator_t iterator = connections.begin ();
      host_name = (*iterator).hostName;
      IRC_Client_PortRangesIterator_t iterator_2 = (*iterator).ports.begin ();
      port_number = (*iterator_2).first;
    } // end IF
  } // end IF
  ///////////////////////////////////////
  configuration.useReactor = use_reactor;

  // step8: do work
  ACE_High_Res_Timer timer;
  timer.start ();
  do_work (configuration,
           use_curses_library,
           host_name,
           port_number,
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
