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

#if defined (CURSES_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN32)
#include "curses.h"
#else
#include "ncurses.h"
// *NOTE*: the ncurses "timeout" macros conflicts with
//         ACE_Synch_Options::timeout. Since not currently being used, it's safe
//         to undefine...
#undef timeout
#endif // ACE_WIN32 || ACE_WIN32
#endif // CURSES_SUPPORT

// *WORKAROUND*
using namespace std;
// *IMPORTANT NOTE*: several ACE headers include ace/iosfwd.h, which introduces
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
#endif // ACE_WIN32 || ACE_WIN32
#include "ace/POSIX_Proactor.h"
#include "ace/Proactor.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_defines.h"
#include "common_os_tools.h"

#include "common_event_tools.h"

#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_cachedallocatorheap.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_defines.h"

#include "net_client_common_tools.h"

#include "irc_common.h"
#include "irc_defines.h"

#include "test_i_defines.h"

#include "IRC_client_sessionmessage.h"
#include "IRC_client_common_modules.h"
#include "IRC_client_configuration.h"
#include "IRC_client_defines.h"
#include "IRC_client_messageallocator.h"
#include "IRC_client_module_IRChandler.h"
#include "IRC_client_session_common.h"
#include "IRC_client_signalhandler.h"
#include "IRC_client_tools.h"

#if defined (CURSES_SUPPORT)
#include "IRC_client_curses.h"
#endif // CURSES_SUPPORT

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:") << std::endl;
  // *NOTE*: the default behaviour is to use the first 'connection' entry in the
  //         configuration file (if any)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-a [HOST]  : IRC server address {\"")
            << ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEFAULT_SERVER_HOSTNAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"}")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEFAULT_INI_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [FILE]  : configuration file {\"")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("\"}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d         : debug parser {")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l         : log to a file {")
            << IRC_CLIENT_SESSION_DEFAULT_LOG
            << ACE_TEXT_ALWAYS_CHAR ("}")
            << std::endl;
#if defined (CURSES_SUPPORT)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n         : use (PD|n)curses library {")
#if defined (CURSES_USE)
            << true
#else
            << false
#endif // CURSES_USE
            << ACE_TEXT_ALWAYS_CHAR ("}")
            << std::endl;
#endif // CURSES_SUPPORT
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-N [STRING]: nickname {\"")
            << ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_NICKNAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r         : use reactor {")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE] : reporting interval (seconds: 0 --> OFF) {")
            << NET_STATISTIC_DEFAULT_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t         : trace information {")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u         : register connection {")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v         : print version information and exit {")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("}")
            << std::endl;
} // end print_usage

bool
do_processArguments (int argc_in,
                     ACE_TCHAR* argv_in[], // cannot be const...
                     ACE_INET_Addr& serverAddress_out,
                     std::string& configurationFile_out,
                     bool& debugParser_out,
                     bool& logToFile_out,
#if defined (CURSES_SUPPORT)
                     bool& useCursesLibrary_out,
#endif // CURSES_SUPPORT
                     std::string& nickName_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& registerConnection_out,
                     bool& printVersionAndExit_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize configuration
  std::string address_string =
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEFAULT_SERVER_HOSTNAME);
  address_string += ':';
  std::ostringstream converter;
  converter << IRC_DEFAULT_SERVER_PORT;
  address_string += converter.str ();
  int result =
    serverAddress_out.string_to_addr (address_string.c_str (),
                                      AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::string_to_addr(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (address_string.c_str ())));
    return false;
  } // end IF
  configurationFile_out          = configuration_path;
  configurationFile_out         += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out         +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  configurationFile_out         += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out         +=
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEFAULT_INI_FILE);
  debugParser_out                = false;
  logToFile_out                  = IRC_CLIENT_SESSION_DEFAULT_LOG;
#if defined (CURSES_SUPPORT)
#if defined (CURSES_USE)
  useCursesLibrary_out           = true;
#else
  useCursesLibrary_out           = false;
#endif // CURSES_USE
#endif // CURSES_SUPPORT
  nickName_out                   = ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_NICKNAME);
  useReactor_out                 =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out = NET_STATISTIC_DEFAULT_REPORTING_INTERVAL_S;
  traceInformation_out           = false;
  registerConnection_out         = false;
  printVersionAndExit_out        = false;

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("a:c:dlN:rs:tuv");
#if defined (CURSES_SUPPORT)
  options_string += ACE_TEXT_ALWAYS_CHAR ("n");
#endif // CURSES_SUPPORT
  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
                              ACE_TEXT (options_string.c_str ()),
                              1,                         // skip command name
                              1,                         // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS, // ordering
                              0);                        // for now, don't support long options

  int option = 0;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'a':
      {
        address_string = argumentParser.opt_arg ();
        address_string += ':';
        std::ostringstream converter;
        converter << IRC_DEFAULT_SERVER_PORT;
        address_string += converter.str ();
        result =
          serverAddress_out.string_to_addr (address_string.c_str (),
                                            AF_INET);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::string_to_addr(\"%s\"): \"%m\", aborting\n"),
                      ACE_TEXT (address_string.c_str ())));
          return false;
        } // end IF
        break;
      }
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
#if defined (CURSES_SUPPORT)
      case 'n':
      {
        useCursesLibrary_out = true;
        break;
      }
#endif // CURSES_SUPPORT
      case 'N':
      {
        nickName_out = argumentParser.opt_arg ();
        break;
      }
      case 'r':
      {
        useReactor_out = true;
        break;
      }
      case 's':
      {
        std::istringstream converter_2;
        converter_2.str (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
        converter_2 >> statisticReportingInterval_out;
        break;
      }
      case 't':
      {
        traceInformation_out = true;
        break;
      }
      case 'u':
      {
        registerConnection_out = true;
        break;
      }
      case 'v':
      {
        printVersionAndExit_out = true;
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
#if defined (CURSES_SUPPORT)
                      bool useCursesLibrary,
#endif // CURSES_SUPPORT
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
#endif // ACE_WIN32 || ACE_WIN64

  // *NOTE*: let (n)curses install its' own signal handler and process events in
  //         (w)getch()
#if defined (CURSES_SUPPORT)
  if (useCursesLibrary)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    //    signals_out.sig_del (SIGINT);
    signals_out.sig_del (SIGWINCH);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
#endif // CURSES_SUPPORT
}

ACE_THR_FUNC_RETURN
connection_setup_function (void* arg_in)
{
  NETWORK_TRACE (ACE_TEXT ("::connection_setup_function"));

  ACE_THR_FUNC_RETURN return_value;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return_value = -1;
#else
  return_value = arg_in;
#endif // ACE_WIN32 || ACE_WIN64

  struct IRC_Client_InputThreadData* thread_data_p =
    static_cast<struct IRC_Client_InputThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (thread_data_p);
  ACE_ASSERT (thread_data_p->configuration);

  int result = -1;
  ACE_Time_Value delay (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_INTERVAL_S, 0);

  IRC_Client_IConnection_t* connection_p = NULL;
  IRC_Client_IStreamConnection_t* istream_connection_p = NULL;
  bool result_2 = false;
  const Stream_Module_t* module_p = NULL;
  Stream_IStream_t* stream_p = NULL;
  IRC_IControl* icontrol_p = NULL;
  IRC_IRegistrationStateMachine_t* iregistration_p = NULL;
  ACE_Time_Value deadline = ACE_Time_Value::zero;
  std::string channel_string;
  string_list_t channels, keys;
  IRC_Client_Connection_Manager_t* connection_manager_p =
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
#if defined (CURSES_SUPPORT)
  struct IRC_SessionState* session_state_p = NULL;
  struct IRC_Client_CursesState* curses_state_p = NULL;
#endif // CURSES_SUPPORT

  // step1: wait for connection ?
  if (thread_data_p->configuration->dispatchConfiguration.numberOfProactorThreads > 0)
  {
    ACE_Time_Value deadline =
      (COMMON_TIME_NOW +
       ACE_Time_Value (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_S, 0));

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

  connection_p = connection_manager_p->operator[] (0);
  if (!connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_IConnection_Manager_t::operator[0]: \"%m\", aborting\n")));
    goto clean_up;
  } // end IF

  istream_connection_p =
    dynamic_cast<IRC_Client_IStreamConnection_t*> (connection_p);
  if (!istream_connection_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<IRC_Client_IStreamConnection_t>(%@): \"%m\", aborting\n"),
                connection_p));
    goto clean_up;
  } // end IF
  stream_p = &const_cast<IRC_Client_Stream_t&> (istream_connection_p->stream ());
  module_p =
    stream_p->find (ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_HANDLER_MODULE_NAME));
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

#if defined (CURSES_SUPPORT)
  curses_state_p =
    &const_cast<struct IRC_Client_CursesState&> (COMMON_UI_CURSES_MANAGER_SINGLETON::instance ()->getR ());
  ACE_ASSERT (!curses_state_p->controller);
  curses_state_p->controller = icontrol_p;
  ACE_ASSERT (!curses_state_p->sessionState);
  session_state_p =
    &const_cast<struct IRC_SessionState&> (connection_p->state ());
  curses_state_p->sessionState = session_state_p;
#endif // CURSES_SUPPORT

  // step2: register connection with the server ?
  if (!thread_data_p->configuration->protocolConfiguration.registerConnection)
    goto continue_;

  // *NOTE*: this entails a little delay (waiting for the welcome notice...)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registering IRC connection...\n")));

  try {
    result_2 =
      icontrol_p->registerc (thread_data_p->configuration->protocolConfiguration.loginOptions);
  } catch (...) {
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
  delay.set (IRC_MAXIMUM_WELCOME_DELAY_S, 0);
  // *NOTE*: cannot use COMMON_TIME_NOW, as this is a high precision monotonous
  //         clock... --> use standard getimeofday
  deadline = ACE_OS::gettimeofday () + delay;
  try {
    result_2 = iregistration_p->wait (IRC_REGISTRATION_STATE_FINISHED,
                                      &deadline);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IRegistration_t::wait(%#T), continuing\n"),
                &delay));
    result_2 = false;
  }
  if (!result_2 ||
      (iregistration_p->current () != IRC_REGISTRATION_STATE_FINISHED))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_IRegistration_t::wait(%#T), aborting\n"),
                &delay));
    goto clean_up;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("registering IRC connection...DONE\n")));

continue_:
  // step4: join a channel
  channel_string =
    thread_data_p->configuration->protocolConfiguration.loginOptions.channel;
  // sanity check(s): has '#' prefix ?
  if (channel_string.find ('#', 0) != 0)
    channel_string.insert (channel_string.begin (), '#');
  // sanity check(s): larger than IRC_CLIENT_CNF_IRC_MAX_CHANNEL_LENGTH characters ?
  // *TODO*: support the CHANNELLEN=xxx "feature" of the server
  if (channel_string.size () > IRC_PRT_MAXIMUM_CHANNEL_LENGTH)
    channel_string.resize (IRC_PRT_MAXIMUM_CHANNEL_LENGTH);
  channels.push_back (channel_string);
  try {
    icontrol_p->join (channels, keys);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in IRC_Client_IIRCControl::join(\"%s\"), aborting\n"),
                ACE_TEXT (channel_string.c_str ())));
    goto clean_up;
  }

  // step6: clean up
  connection_p->decrease (); connection_p = NULL;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return_value = 0;
#else
  return_value = NULL;
#endif // ACE_WIN32 || ACE_WIN64

clean_up:
  if (connection_p)
    connection_p->decrease ();

  Common_Event_Tools::finalizeEventDispatch (*thread_data_p->dispatchState,
                                             false,  // don't block
                                             false); // don't delete singletons

  return return_value;
}

void
do_work (struct IRC_Client_Configuration& configuration_in,
         const ACE_INET_Addr& serverAddress_in,
#if defined (CURSES_SUPPORT)
         bool useCursesLibrary_in,
#endif // CURSES_SUPPORT
         const ACE_Time_Value& statisticReportingInterval_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         IRC_Client_SignalHandler& signalHandler_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

  // step1: initialize IRC handler module

  // initialize protocol configuration
  struct IRC_AllocatorConfiguration allocator_configuration;
  Stream_CachedAllocatorHeap_T<struct Common_AllocatorConfiguration> heap_allocator (NET_STREAM_MAX_MESSAGES,
                                                                                     IRC_MAXIMUM_FRAME_SIZE + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator: \"%m\", aborting\n")));
    return;
  } // end IF
  IRC_Client_MessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES,
                                                   &heap_allocator);

  ////////////////////////////////////////

  struct Stream_ModuleConfiguration module_configuration;
  struct IRC_Client_ModuleHandlerConfiguration modulehandler_configuration;
  struct IRC_Client_StreamConfiguration stream_configuration;
  modulehandler_configuration.parserConfiguration =
    &configuration_in.parserConfiguration;
  modulehandler_configuration.protocolConfiguration =
    &configuration_in.protocolConfiguration;
  modulehandler_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  modulehandler_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;

  stream_configuration.allocatorConfiguration = &allocator_configuration;
  stream_configuration.messageAllocator = &message_allocator;

  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);
  IRC_Client_StreamConfiguration_t::ITERATOR_T iterator =
    configuration_in.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration_in.streamConfiguration.end ());

  // step2: initialize event dispatch
  configuration_in.dispatchConfiguration.numberOfReactorThreads =
      ((configuration_in.dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR) ? TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS
                                                                                          : 0);
  configuration_in.dispatchConfiguration.numberOfProactorThreads =
      ((configuration_in.dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_PROACTOR) ? TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS
                                                                                           : 0);
  if (!Common_Event_Tools::initializeEventDispatch (configuration_in.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
    &configuration_in.dispatchConfiguration;

#if defined (CURSES_SUPPORT)
  struct IRC_Client_CursesState& state_r =
    const_cast<struct IRC_Client_CursesState&> (COMMON_UI_CURSES_MANAGER_SINGLETON::instance ()->getR ());
#endif // CURSES_SUPPORT
  struct Common_UI_State ui_state_s;

  // step3: initialize client connector
  IRC_Client_ConnectionConfiguration connection_configuration;
  connection_configuration.allocatorConfiguration = &allocator_configuration;
  connection_configuration.socketConfiguration.address = serverAddress_in;

  //connection_configuration.bufferSize = IRC_CLIENT_BUFFER_SIZE;
  connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;

  connection_configuration.messageAllocator = &message_allocator;
  connection_configuration.protocolConfiguration =
    &configuration_in.protocolConfiguration;
  connection_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;
#if defined (CURSES_SUPPORT)
  if (useCursesLibrary_in)
    connection_configuration.UIState = &state_r;
  else
    connection_configuration.UIState = &ui_state_s;
#else
  connection_configuration.UIState = &ui_state_s;
#endif // CURSES_SUPPORT

  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                    &connection_configuration));
  Net_ConnectionConfigurationsIterator_t iterator_2 =
    configuration_in.connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != configuration_in.connectionConfigurations.end ());

  //IRC_Client_SessionState session_state;
  //session_state.configuration = configuration_in;
  //IRC_Client_ConnectorConfiguration connector_configuration;
  IRC_Client_Connection_Manager_t* connection_manager_p =
    IRC_CLIENT_CONNECTIONMANAGER_SINGLETON::instance ();
  IRC_Client_SessionConnector_t connector (true);
  IRC_Client_AsynchSessionConnector_t asynch_connector (true);
  IRC_Client_IConnector_t* connector_p = NULL;
  if (configuration_in.dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
    connector_p = &connector;
  else
    connector_p = &asynch_connector;

  // step3: initialize signal handling
  struct IRC_Client_SignalHandlerConfiguration signal_handler_configuration;
  signal_handler_configuration.connector = connector_p;
  signal_handler_configuration.dispatchState = &event_dispatch_state_s;
  signal_handler_configuration.peerAddress = serverAddress_in;
  if (!signalHandler_in.initialize (signal_handler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IRC_Client_SignalHandler::initialize(), returning\n")));
    return;
  } // end IF
  if (!Common_Signal_Tools::initialize (((configuration_in.dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR) ? COMMON_SIGNAL_DISPATCH_REACTOR
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

  // step4: initialize connection manager
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (*static_cast<IRC_Client_ConnectionConfiguration*> ((*iterator_2).second),
                             NULL);

  // event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step5a: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::startEventDispatch(), returning\n")));
    return;
  } // end IF

  // step5b: (try to) connect to the server
  struct Net_UserData user_data_s;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  if (configuration_in.dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR)
    handle =
      Net_Client_Common_Tools::connect (connector,
                                        *static_cast<IRC_Client_ConnectionConfiguration*> ((*iterator_2).second),
                                        user_data_s,
                                        serverAddress_in,
                                        true,
                                        true);
  else
    handle =
      Net_Client_Common_Tools::connect (asynch_connector,
                                        *static_cast<IRC_Client_ConnectionConfiguration*> ((*iterator_2).second),
                                        user_data_s,
                                        serverAddress_in,
                                        true,
                                        true);
  if (unlikely (handle == ACE_INVALID_HANDLE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to %s: \"%m\", returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (serverAddress_in).c_str ())));
    // *IMPORTANT NOTE*: cannot close event dispatch singletons here; still
    //                   referenced by the connectors
    Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                               true,   // wait ?
                                               false); // close singletons ?
    return;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connected...\n")));

  // *NOTE*: from this point, clean up any remote connections

  // step6a: wait for connection / setup

  struct IRC_Client_InputThreadData input_thread_data_s;
  input_thread_data_s.configuration = &configuration_in;
  input_thread_data_s.dispatchState = &event_dispatch_state_s;
  input_thread_data_s.moduleHandlerConfiguration =
    const_cast<struct IRC_Client_ModuleHandlerConfiguration*> ((*iterator).second.second);
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
    connection_manager_p->abort ();
    connection_manager_p->wait ();
    Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                               true,
                                               false);
    return;
  } // end IF
  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
  const char* thread_name_2 = thread_name_p;
  int group_id_2 = (COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1); // *TODO*
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  result =
    thread_manager_p->spawn (::connection_setup_function, // function
                             &input_thread_data_s,        // argument
                             (THR_NEW_LWP      |
                              THR_JOINABLE     |
                              THR_INHERIT_SCHED),         // flags
                             &thread_id,                  // thread id
                             &thread_handle,              // thread handle
                             ACE_DEFAULT_THREAD_PRIORITY, // priority
                             group_id_2,                  // group id
                             NULL,                        // stack
                             0,                           // stack size
                             &thread_name_2);             // name
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));
    connection_manager_p->abort ();
    connection_manager_p->wait ();
    Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                               true,
                                               false);
    return;
  } // end IF
  Common_Event_Tools::dispatchEvents (event_dispatch_state_s);
  // *NOTE*: awoken by the worker thread (see above)...
  if (connection_manager_p->count () < 1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\": \"%m\", returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (signal_handler_configuration.peerAddress, false, false).c_str ())));
    result = thread_manager_p->wait_grp (group_id_2);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", returning\n"),
                  group_id_2));
    connection_manager_p->abort ();
    connection_manager_p->wait ();
    return;
  } // end IF
  result = thread_manager_p->wait_grp (group_id_2);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Manager::wait_grp(%d): \"%m\", returning\n"),
                group_id_2));

  // step6b: setup GUI
#if defined (CURSES_SUPPORT)
  configuration_in.cursesConfiguration.hooks.initHook = curses_init;
  configuration_in.cursesConfiguration.hooks.finiHook = curses_fini;
  configuration_in.cursesConfiguration.hooks.inputHook = curses_input;
  configuration_in.cursesConfiguration.hooks.mainHook = curses_main;
  state_r.dispatchState = &event_dispatch_state_s;

  if (!COMMON_UI_CURSES_MANAGER_SINGLETON::instance ()->initialize (configuration_in.cursesConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_Curses_Manager_T::initialize(), returning\n")));
    connection_manager_p->abort ();
    connection_manager_p->wait ();
    return;
  } // end IF
  if (!COMMON_UI_CURSES_MANAGER_SINGLETON::instance ()->start (NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_UI_Curses_Manager_T::start(), returning\n")));
    connection_manager_p->abort ();
    connection_manager_p->wait ();
    return;
  } // end IF
#endif // CURSES_SUPPORT

  // step6b: dispatch events
  event_dispatch_state_s.proactorGroupId = -1;
  event_dispatch_state_s.reactorGroupId = -1;
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::startEventDispatch(), returning\n")));
#if defined (CURSES_SUPPORT)
    COMMON_UI_CURSES_MANAGER_SINGLETON::instance ()->stop (true,
                                                           true);
#endif // CURSES_SUPPORT
    connection_manager_p->abort ();
    connection_manager_p->wait ();
    return;
  } // end IF
  Common_Event_Tools::dispatchEvents (event_dispatch_state_s);

  // step7: clean up
#if defined (CURSES_SUPPORT)
  COMMON_UI_CURSES_MANAGER_SINGLETON::instance ()->stop (true,
                                                         true);
#endif // CURSES_SUPPORT
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
#if defined (HAVE_CONFIG_H)
            << ACE_TEXT (ACENetwork_PACKAGE_NAME)
            << ACE_TEXT (": ")
            << ACE_TEXT (ACENetwork_PACKAGE_VERSION)
            << std::endl;
#endif // HAVE_CONFIG_H

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

#if defined (CURSES_SUPPORT)
  std::cout << ACE_TEXT ("curses: ")
            << curses_version ()
            << std::endl;
#endif // CURSES_SUPPORT
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  NETWORK_TRACE (ACE_TEXT ("::main"));

  int result = -1;

  // step1: initialize libraries
  // *PORTABILITY*: on Windows, initialize ACE
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

  // step2: initialize/validate configuration

  // step2a: process commandline arguments
  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  ACE_INET_Addr server_address;
  std::string address_string =
    ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_DEFAULT_SERVER_HOSTNAME);
  address_string += ':';
  std::stringstream converter;
  converter << IRC_DEFAULT_SERVER_PORT;
  address_string += converter.str ();
  result =
    server_address.string_to_addr (address_string.c_str (),
                                   AF_INET);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::string_to_addr(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (address_string.c_str ())));

    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_FAILURE;
  } // end IF
  std::string configuration_file_name        = configuration_path;
  configuration_file_name                   += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file_name                   +=
    ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  configuration_file_name                   += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file_name                   +=
      ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_CNF_DEFAULT_INI_FILE);
  bool debug_parser                          = false;
  bool log_to_file                           = IRC_CLIENT_SESSION_DEFAULT_LOG;
#if defined (CURSES_SUPPORT)
#if defined (CURSES_USE)
  bool use_curses_library                    = true;
#else
  bool use_curses_library                    = false;
#endif // CURSES_USE
#endif // CURSES_SUPPORT
  std::string nickname_string                =
    ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_NICKNAME);
  bool use_reactor                           =
      (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval  =
    NET_STATISTIC_DEFAULT_REPORTING_INTERVAL_S;
  bool trace_information                     = false;
  bool register_connection                   = false;
  bool print_version_and_exit                = false;
  if (!do_processArguments (argc_in,
                            argv_in,
                            server_address,
                            configuration_file_name,
                            debug_parser,
                            log_to_file,
#if defined (CURSES_SUPPORT)
                            use_curses_library,
#endif // CURSES_SUPPORT
                            nickname_string,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            register_connection,
                            print_version_and_exit))
  {
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (ACE::fini () == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

    return EXIT_FAILURE;
  } // end IF

  // step2b: validate argument(s)
  if (!Common_File_Tools::isReadable (configuration_file_name))
  {
    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (ACE::fini () == -1)
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
#if defined (CURSES_SUPPORT)
                        use_curses_library,
#endif // CURSES_SUPPORT
                        true,
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
//  ACE_SYNCH_RECURSIVE_MUTEX signal_lock;
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

  // step6: initialize configuration objects
  struct IRC_Client_Configuration configuration;

  ////////////////////////////////////////
#if defined (CURSES_SUPPORT)
  if (use_curses_library)
    configuration.GUIFramework = COMMON_UI_FRAMEWORK_CURSES;
#endif // CURSES_SUPPORT
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
  configuration.protocolConfiguration.registerConnection = register_connection;

  // step7: parse configuration file(s) (if any)
  if (!configuration_file_name.empty ())
  {
    IRC_Client_Connections_t connections;
    IRC_Client_Tools::parseConfigurationFile (configuration_file_name,
                                              configuration.protocolConfiguration.loginOptions,
                                              connections);
    if (server_address.is_any () &&
        !connections.empty ())
    {
      IRC_Client_ConnectionsIterator_t iterator = connections.begin ();
      IRC_Client_PortRangesIterator_t iterator_2 = (*iterator).ports.begin ();
      address_string = (*iterator).hostName;
      address_string += ':';
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << (*iterator_2).first;
      address_string += converter.str ();
      result =
        server_address.string_to_addr (address_string.c_str (),
                                       AF_INET);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::string_to_addr(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (address_string.c_str ())));

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
    } // end IF
  } // end IF
  if (ACE_OS::strcmp (nickname_string.c_str (),
                      ACE_TEXT_ALWAYS_CHAR (IRC_DEFAULT_NICKNAME)))
    configuration.protocolConfiguration.loginOptions.nickname = nickname_string;

  ///////////////////////////////////////
  configuration.dispatchConfiguration.dispatch =
    (use_reactor ? COMMON_EVENT_DISPATCH_REACTOR
                 : COMMON_EVENT_DISPATCH_PROACTOR);

  // step8: do work
  ACE_High_Res_Timer timer;
  timer.start ();
  do_work (configuration,
           server_address,
#if defined (CURSES_SUPPORT)
           use_curses_library,
#endif // CURSES_SUPPORT
           ACE_Time_Value (statistic_reporting_interval, 0),
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler);

  timer.stop ();
  std::string working_time_string;
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  process_profile.stop ();
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

  // step9: clean up
  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalize ();

  // step10: finalize libraries
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
