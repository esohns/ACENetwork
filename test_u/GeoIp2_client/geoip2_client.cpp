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
#include <sstream>
#include <string>

#if defined (GEOIP2_SUPPORT)
#include "maxminddb.h"
#endif // GEOIP2_SUPPORT

#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
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
#include "common_tools.h"

#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"
#include "net_macros.h"

#include "test_u_common_tools.h"
#include "test_u_defines.h"

#include "geoip2_defines.h"

void
do_print_usage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_print_usage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR(COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (GEOIP2_CLIENT_DATABASE_FILE_NAME);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [PATH]   : maxmind .mmdb database file [")
            << path
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-i [ADDRESS]: ip address to query [")
            << ACE_TEXT_ALWAYS_CHAR ("")
            << ACE_TEXT_ALWAYS_CHAR ("] {\"\": use default network interface}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_process_arguments (int argc_in,
                      ACE_TCHAR** argv_in, // cannot be const...
                      std::string& databaseFilePath_out,
                      ACE_INET_Addr& IPAddress_out,
                      bool& logToFile_out,
                      bool& traceInformation_out,
                      bool& printVersionAndExit_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_process_arguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (GEOIP2_CLIENT_DATABASE_FILE_NAME);
  databaseFilePath_out = path;
  //IPAddress_out.set ((u_short)0, (ACE_UINT32)INADDR_ANY, 1, 0);
  logToFile_out = false;
  traceInformation_out = false;
  printVersionAndExit_out = false;

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("f:i:ltv");
  ACE_Get_Opt argument_parser (argc_in,
                               argv_in,
                               ACE_TEXT (options_string.c_str ()),
                               1,                         // skip command name
                               1,                         // report parsing errors
                               ACE_Get_Opt::PERMUTE_ARGS, // ordering
                               0);                        // for now, don't use long options

  int option = 0;
  std::stringstream converter;
  while ((option = argument_parser ()) != EOF)
  {
    switch (option)
    {
      case 'f':
      {
        databaseFilePath_out = argument_parser.opt_arg ();
        break;
      }
      case 'i':
      {
        std::string address_string =
          ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        IPAddress_out.set ((u_short)0,
                           address_string.c_str (),
                           1,
                           Net_Common_Tools::matchIPv4Address (address_string) ? AF_INET : AF_INET6);
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
      // error handling
      case ':':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("option \"%c\" requires an argument, aborting\n"),
                    argument_parser.opt_opt ()));
        return false;
      }
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    ACE_TEXT (argument_parser.last_option ())));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    ACE_TEXT (argument_parser.long_option ())));
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
do_work (const std::string& databaseFilePath_in,
         const ACE_INET_Addr& IPAddress_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

  // step0a: initialize random number generator
  Test_U_Common_Tools::initialize ();

  MMDB_s mmdb_s;
  int gai_error, mmdb_error;
  std::string ip_address_string;
  //MMDB_entry_data_list_s* entry_data_list_p = NULL;
  MMDB_lookup_result_s result_2;
  MMDB_entry_data_s entry_data_s;
  result = MMDB_open (databaseFilePath_in.c_str (),
                      MMDB_MODE_MMAP,
                      &mmdb_s);
  if (MMDB_SUCCESS != result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::MMDB_open(\"%s\"): \"%s\", returning\n"),
                ACE_TEXT (databaseFilePath_in.c_str ()),
                ACE_TEXT (MMDB_strerror (result))));
    goto clean_up;
  } // end IF

  ip_address_string =
    Net_Common_Tools::IPAddressToString (IPAddress_in, true, false);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("looking up location of address \"%s\"\n"),
              ACE_TEXT (ip_address_string.c_str ())));
  result_2 = MMDB_lookup_string (&mmdb_s,
                                 ip_address_string.c_str (),
                                 &gai_error,
                                 &mmdb_error);
  if (0 != gai_error)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::MMDB_lookup_string(): \"%s\", returning\n"),
                ACE_TEXT (gai_strerror (gai_error))));
    goto clean_up;
  } // end IF
  if (MMDB_SUCCESS != mmdb_error)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::MMDB_lookup_string(): \"%s\", returning\n"),
                ACE_TEXT (MMDB_strerror (mmdb_error))));
    goto clean_up;
  } // end IF
  if (!result_2.found_entry)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("database has no entry for IP address (was: \"%s\"), returning\n"),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (IPAddress_in, true, false).c_str ())));
    goto clean_up;
  } // end IF

  //result = MMDB_get_entry_data_list (&result_2.entry,
  //                                   &entry_data_list_p);
  //if (MMDB_SUCCESS != result)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ::MMDB_get_entry_data_list(): \"%s\", returning\n"),
  //              ACE_TEXT (MMDB_strerror (result))));
  //  goto clean_up;
  //} // end IF
  //ACE_ASSERT (entry_data_list_p);
  //MMDB_dump_entry_data_list (stdout,
  //                           entry_data_list_p,
  //                           2);

  result =
    MMDB_get_value (&result_2.entry,
                    &entry_data_s,
                    ACE_TEXT_ALWAYS_CHAR ("location"), ACE_TEXT_ALWAYS_CHAR ("latitude"), NULL);
  if (MMDB_SUCCESS != result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::MMDB_get_value(): \"%s\", returning\n"),
                ACE_TEXT (MMDB_strerror (result))));
    goto clean_up;
  } // end IF
  if (!entry_data_s.has_data)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("IP address entry has not data for path (was: \"%s:%s\"), returning\n"),
                ACE_TEXT ("location"), ACE_TEXT ("latitude")));
    goto clean_up;
  } // end IF
  ACE_ASSERT (entry_data_s.type == MMDB_DATA_TYPE_DOUBLE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("latitude: ") << std::fixed << entry_data_s.double_value;
  result =
    MMDB_get_value (&result_2.entry,
                    &entry_data_s,
                    ACE_TEXT_ALWAYS_CHAR ("location"), ACE_TEXT_ALWAYS_CHAR ("longitude"), NULL);
  if (MMDB_SUCCESS != result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::MMDB_get_value(): \"%s\", returning\n"),
                ACE_TEXT (MMDB_strerror (result))));
    goto clean_up;
  } // end IF
  if (!entry_data_s.has_data)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("IP address entry has not data for path (was: \"%s:%s\"), returning\n"),
                ACE_TEXT ("location"), ACE_TEXT ("longitude")));
    goto clean_up;
  } // end IF
  ACE_ASSERT (entry_data_s.type == MMDB_DATA_TYPE_DOUBLE);
  std::cout << ACE_TEXT_ALWAYS_CHAR (", longitude: ") << std::fixed << entry_data_s.double_value;
  std::cout << std::endl;

clean_up:
  //if (entry_data_list_p)
  //  MMDB_free_entry_data_list (entry_data_list_p);
  MMDB_close (&mmdb_s);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

void
do_print_version (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_print_version"));

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
  if (unlikely (result == -1))
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
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (GEOIP2_CLIENT_DATABASE_FILE_NAME);
  std::string database_filepath_string = path;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID interface_s = Net_Common_Tools::getDefaultInterface_2 ();
#else
  std::string interface_s = Net_Common_Tools::getDefaultInterface ();
#endif // ACE_WIN32 || ACE_WIN64
  ACE_INET_Addr ip_address;
  if (unlikely (!Net_Common_Tools::interfaceToExternalIPAddress (interface_s,
                                                                 ip_address)))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), aborting\n"),
                ACE_TEXT (Net_Common_Tools::interfaceToString (interface_s).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to Net_Common_Tools::interfaceToExternalIPAddress(\"%s\"), aborting\n"),
               ACE_TEXT (interface_s.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF
  bool log_to_file = false;
  bool trace_information = false;
  bool print_version_and_exit = false;

  // step1b: parse/process/validate configuration
  if (unlikely (!do_process_arguments (argc_in,
                                       argv_in,
                                       database_filepath_string,
                                       ip_address,
                                       log_to_file,
                                       trace_information,
                                       print_version_and_exit)))
  {
    // make 'em learn
    do_print_usage (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (unlikely (result == -1))
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
  if (unlikely (!Common_File_Tools::isReadable (database_filepath_string) ||
                false))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));
    do_print_usage (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

//#if defined (GTK_USE)
//  Common_Logger_t logger (logstack_p,
//                          lock_p);
//#endif // GTK_USE
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                        ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));
  if (unlikely (!Common_Log_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])), // program name
                                               log_file_name,                                     // log file name
                                               false,                                             // log to syslog ?
                                               false,                                             // trace messages ?
                                               trace_information,                                 // debug messages ?
                                               NULL)))                                            // (ui) logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initializeLogging(), aborting\n")));

    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_print_version (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));

    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_SUCCESS;
  } // end IF

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (false); // fill ?
  ACE_Sig_Set ignored_signal_set (false); // fill ?
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  // *TODO*: the reasoning here is incomplete
  bool use_fd_based_reactor = true;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  use_fd_based_reactor =
    (/*use_reactor &&*/ !(COMMON_EVENT_REACTOR_TYPE == COMMON_REACTOR_WFMO));
  bool use_signal_based_proactor = false;
#else
  bool use_signal_based_proactor = true;
#endif // ACE_WIN32 || ACE_WIN64
  bool stack_traces = true;
  if (unlikely (!Common_OS_Tools::setResourceLimits (use_fd_based_reactor,        // file descriptors
                                                     stack_traces,                // stack traces
                                                     use_signal_based_proactor))) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (database_filepath_string,
           ip_address,
           signal_set,
           ignored_signal_set,
           previous_signal_actions);
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
  if (unlikely (process_profile.elapsed_time (elapsed_time) == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
    // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (unlikely (result == -1))
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

  Common_Log_Tools::finalize ();
  Common_Tools::finalize ();

  // *PORTABILITY*: on Windows, finalize ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
