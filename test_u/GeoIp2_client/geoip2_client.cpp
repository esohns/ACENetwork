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
#include "common_tools.h"

#include "common_logger.h"
#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//#include "common_ui_gtk_builder_definition.h"
//#include "common_ui_gtk_defines.h"
////#include "common_ui_gtk_glade_definition.h"
//#include "common_ui_gtk_manager_common.h"
//#endif // GTK_USE
//#endif // GUI_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"
#include "net_macros.h"

#include "test_u_common_tools.h"
#include "test_u_defines.h"

#include "geoip2_defines.h"
//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//#include "test_u_callbacks.h"
//#endif // GTK_USE
//#endif // GUI_SUPPORT

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f [PATH]   : maxmind .mmdb database file [")
            << ACE_TEXT_ALWAYS_CHAR (GEOIP2_CLIENT_DATABASE_FILE_NAME)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
//#if defined (GUI_SUPPORT)
//  std::string UI_file_path = path;
//  UI_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
//  UI_file_path += ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_FILE);
//  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
//            << UI_file_path
//            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
//            << std::endl;
//#endif // GUI_SUPPORT
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
//#if defined (GUI_SUPPORT)
//                      std::string& UIFile_out,
//#endif // GUI_SUPPORT
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
  path += ACE_TEXT_ALWAYS_CHAR (GEOIP2_CLIENT_DATABASE_FILE_NAME);
  databaseFilePath_out = path;
//#if defined (GUI_SUPPORT)
//  path = configuration_path;
//  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
//  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
//  UIFile_out = path;
//  UIFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
//  UIFile_out += ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_FILE);
//#endif // GUI_SUPPORT
  //IPAddress_out.set ((u_short)0, 0U);
  logToFile_out = false;
  traceInformation_out = false;
  printVersionAndExit_out = false;

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("f:i:ltv");
#if defined (GUI_SUPPORT)
  options_string += ACE_TEXT_ALWAYS_CHAR ("g::");
#endif // GUI_SUPPORT
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
//#if defined (GUI_SUPPORT)
//      case 'g':
//      {
//        ACE_TCHAR* opt_arg = argument_parser.opt_arg ();
//        if (opt_arg)
//          UIFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
//        else
//          UIFile_out.clear ();
//        break;
//      }
//#endif // GUI_SUPPORT
      case 'i':
      {
        IPAddress_out.set ((u_short)0,
                           ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ()),
                           1,
                           AF_INET);
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

//void
//do_initialize_signals (ACE_Sig_Set& signals_out,
//                       ACE_Sig_Set& ignoredSignals_out)
//{
//  NETWORK_TRACE (ACE_TEXT ("::do_initialize_signals"));
//
//  int result = -1;
//
//  // initialize return value(s)
//  result = signals_out.empty_set ();
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
//    return;
//  } // end IF
//  result = ignoredSignals_out.empty_set ();
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
//    return;
//  } // end IF
//
//  // *PORTABILITY*: on Windows(TM) platforms most signals are not defined, and
//  //                ACE_Sig_Set::fill_set() doesn't really work as specified
//  // --> add valid signals (see <signal.h>)...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  signals_out.sig_add (SIGINT);            // 2       /* interrupt */
//  signals_out.sig_add (SIGILL);            // 4       /* illegal instruction - invalid function image */
//  signals_out.sig_add (SIGFPE);            // 8       /* floating point exception */
////  signals_out.sig_add (SIGSEGV);           // 11      /* segment violation */
//  signals_out.sig_add (SIGTERM);           // 15      /* Software termination signal from kill */
//  signals_out.sig_add (SIGBREAK);        // 21      /* Ctrl-Break sequence */
//  signals_out.sig_add (SIGABRT);           // 22      /* abnormal termination triggered by abort call */
//  signals_out.sig_add (SIGABRT_COMPAT);    // 6       /* SIGABRT compatible with other platforms, same as SIGABRT */
//#else
//  result = signals_out.fill_set ();
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT("failed to ACE_Sig_Set::fill_set(): \"%m\", returning\n")));
//    return;
//  } // end IF
//  // *NOTE*: cannot handle some signals --> registration fails for these...
//  signals_out.sig_del (SIGKILL);           // 9       /* Kill signal */
//  signals_out.sig_del (SIGSTOP);           // 19      /* Stop process */
//  // ---------------------------------------------------------------------------
//  // *NOTE* core dump on SIGSEGV
//  signals_out.sig_del (SIGSEGV);           // 11      /* Segmentation fault: Invalid memory reference */
//  // *NOTE* don't care about SIGPIPE
//  signals_out.sig_del (SIGPIPE);           // 12      /* Broken pipe: write to pipe with no readers */
//
//#if defined (VALGRIND_SUPPORT)
//  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
//  // if the application installs its own handler (see documentation)
//  if (RUNNING_ON_VALGRIND)
//    signals_out.sig_del (SIGRTMAX);        // 64
//#endif // VALGRIND_SUPPORT
//#endif // ACE_WIN32 || ACE_WIN64
//}

void
do_work (const std::string& databaseFilePath_in,
//#if defined (GUI_SUPPORT)
//         const std::string& UIDefinitionFile_in,
//#endif // GUI_SUPPORT
         const ACE_INET_Addr& IPAddress_in,
//#if defined (GUI_SUPPORT)
//         struct Client_UI_CBData& CBData_in,
//#endif // GUI_SUPPORT
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

  // step0a: initialize random number generator
  Test_U_Common_Tools::initialize ();

  // step0a: initialize configuration
//#if defined (GUI_SUPPORT)
//  CBData_in.configuration = &configuration_in;
//  CBData_in.progressData.configuration = &configuration_in;
//
//  Test_U_EventHandler_t ui_event_handler (&CBData_in);
//#else
//  Test_U_EventHandler_t ui_event_handler ();
//#endif // GUI_SUPPORT

  // step0e: initialize signal handling
  // *WORKAROUND*: ACE_INET_Addr::operator= is broken
  //configuration_in.signalHandlerConfiguration.address = peer_address;
  //configuration_in.signalHandlerConfiguration.TCPConnectionConfiguration =
  //  &tcp_connection_configuration;
  //configuration_in.signalHandlerConfiguration.UDPConnectionConfiguration =
  //  &udp_connection_configuration;
  //configuration_in.signalHandlerConfiguration.protocolConfiguration =
  //  &configuration_in.protocolConfiguration;
  //configuration_in.signalHandlerConfiguration.messageAllocator =
  //  &message_allocator;
  //configuration_in.signalHandlerConfiguration.stopEventDispatchOnShutdown =
  //  UIDefinitionFile_in.empty ();
  //if (!signalHandler_in.initialize (configuration_in.signalHandlerConfiguration))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to initialize signal handler, returning\n")));
  //  Common_Timer_Tools::finalize ();
  //  timer_manager_p->stop ();
  //  return;
  //} // end IF
  //if (!Common_Signal_Tools::initialize ((useReactor_in ? COMMON_SIGNAL_DISPATCH_REACTOR
  //                                                     : COMMON_SIGNAL_DISPATCH_PROACTOR),
  //                                      signalSet_in,
  //                                      ignoredSignalSet_in,
  //                                      &signalHandler_in,
  //                                      previousSignalActions_inout))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
  //  Common_Timer_Tools::finalize ();
  //  timer_manager_p->stop ();
  //  return;
  //} // end IF

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  // step1a: start GTK event loop ?
//  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
//#endif // GTK_USE
//  if (!UIDefinitionFile_in.empty ())
//  {
//#if defined (GTK_USE)
//    gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
//    ACE_ASSERT (gtk_manager_p);
//
//    ACE_ASSERT (CBData_in.UIState);
//    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
//    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
//    CBData_in.UIState->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
//      std::make_pair (UIDefinitionFile_in, static_cast<GtkBuilder*> (NULL));
//
//    gtk_manager_p->start (NULL);
//    ACE_Time_Value timeout (0,
//                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
//    result = ACE_OS::sleep (timeout);
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
//                  &timeout));
//    if (!gtk_manager_p->isRunning ())
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
//      Common_Timer_Tools::finalize ();
//      timer_manager_p->stop ();
//      return;
//    } // end IF
//#endif // GTK_USE
//
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    HWND window_p = GetConsoleWindow ();
//    if (!window_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
//      Common_Timer_Tools::finalize ();
//      timer_manager_p->stop ();
//#if defined (GTK_USE)
//      gtk_manager_p->stop (true, true);
//#endif // GTK_USE
//      return;
//    } // end IF
//    BOOL was_visible_b = ::ShowWindow (window_p, SW_HIDE);
//#endif // ACE_WIN32 || ACE_WIN64
//  } // end IF
//#endif // GUI_SUPPORT

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//    gtk_manager_p->wait ();
//#endif // GTK_USE
//#endif // GUI_SUPPORT

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("latitude: ") << entry_data_s.double_value;
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
  std::cout << ACE_TEXT_ALWAYS_CHAR (", longitude: ") << entry_data_s.double_value;
  std::cout << std::endl;

clean_up:
  //if (entry_data_list_p)
  //  MMDB_free_entry_data_list (entry_data_list_p);
  MMDB_close (&mmdb_s);

  //timer_manager_p->stop ();
  //timer_manager_p->wait ();
//  Common_Timer_Tools::finalize ();

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
  path += ACE_TEXT_ALWAYS_CHAR(GEOIP2_CLIENT_DATABASE_FILE_NAME);
  std::string database_filepath_string = path;
//#if defined (GUI_SUPPORT)
//  path = configuration_path;
//  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
//  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
//  std::string UI_file_path = path;
//  UI_file_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
//  UI_file_path += ACE_TEXT_ALWAYS_CHAR (NET_CLIENT_UI_FILE);
//#endif // GUI_SUPPORT
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
//#if defined (GUI_SUPPORT)
//                            UI_file_path,
//#endif // GUI_SUPPORT
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
//#if defined (GUI_SUPPORT)
//      (!UI_file_path.empty () && !Common_File_Tools::isReadable (UI_file_path)) ||
//#endif // GUI_SUPPORT
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

//#if defined (GUI_SUPPORT)
//  struct Client_UI_CBData ui_cb_data;
//#endif // GUI_SUPPORT

  // step1d: initialize logging and/or tracing
//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  Common_MessageStack_t* logstack_p = NULL;
//  ACE_SYNCH_MUTEX* lock_p = NULL;
//  Common_UI_GTK_Manager_t* gtk_manager_p =
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
//  ACE_ASSERT (gtk_manager_p);
//  Common_UI_GTK_State_t& state_r =
//    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
//  logstack_p = &state_r.logStack;
//  lock_p = &state_r.logStackLock;
//  ui_cb_data.UIState = &state_r;
//  ui_cb_data.progressData.state = &state_r;
//#endif // GTK_USE
//#endif // GUI_SUPPORT

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  Common_Logger_t logger (logstack_p,
//                          lock_p);
//#endif // GTK_USE
//#endif // GUI_SUPPORT
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                        ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])));
  if (unlikely (!Common_Log_Tools::initializeLogging (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])), // program name
                                                      log_file_name,                                     // log file name
                                                      false,                                             // log to syslog ?
                                                      false,                                             // trace messages ?
                                                      trace_information,                                 // debug messages ?
//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//                                                      (UI_file_path.empty () ? NULL
//                                                                             : &logger)))               // (ui) logger ?
//#elif defined (WXWIDGETS_USE)
//                                            NULL))                              // (ui) logger ?
//#else
//                                            NULL))                              // (ui) logger ?
//#endif // XXX_USE
//#else
                                                      NULL)))                                            // (ui) logger ?
//#endif // GUI_SUPPORT
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

    Common_Log_Tools::finalizeLogging ();
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
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  //do_initialize_signals (signal_set,
  //                       ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
//  if (unlikely (!Common_Signal_Tools::preInitialize (signal_set,
//                                                     (use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
//                                                                  : COMMON_SIGNAL_DISPATCH_PROACTOR),
//                                                     true, // use networking
//                                                     previous_signal_actions,
//                                                     previous_signal_mask)))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));
//
//    Common_Log_Tools::finalizeLogging ();
//    Common_Tools::finalize ();
//    // *PORTABILITY*: on Windows, finalize ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    result = ACE::fini ();
//    if (unlikely (result == -1))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif // ACE_WIN32 || ACE_WIN64
//    return EXIT_FAILURE;
//  } // end IF
  //configuration.signalHandlerConfiguration.dispatchState =
  //  &event_dispatch_state_s;
  //Client_SignalHandler signal_handler;

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
  if (unlikely (!Common_Tools::setResourceLimits (use_fd_based_reactor,        // file descriptors
                                                  stack_traces,                // stack traces
                                                  use_signal_based_proactor))) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::setResourceLimits(), aborting\n")));

    //Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
    //                                            : COMMON_SIGNAL_DISPATCH_PROACTOR),
    //                               previous_signal_actions,
    //                               previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
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

//  // step1h: initialize UI framework
//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
//  configuration.GTKConfiguration.argc = argc_in;
//  configuration.GTKConfiguration.argv = argv_in;
//  configuration.GTKConfiguration.CBData = &ui_cb_data;
//  configuration.GTKConfiguration.eventHooks.finiHook =
//      idle_finalize_UI_cb;
//  configuration.GTKConfiguration.eventHooks.initHook =
//      idle_initialize_client_UI_cb;
//  configuration.GTKConfiguration.definition = &gtk_ui_definition;
//  if (!UI_file_path.empty ())
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (configuration.GTKConfiguration);
//#endif // GTK_USE
//#endif // GUI_SUPPORT

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (database_filepath_string,
//#if defined (GUI_SUPPORT)
//           UI_file_path,
//#endif // GUI_SUPPORT
           ip_address,
           //configuration,
//#if defined (GUI_SUPPORT)
//           ui_cb_data,
//#endif // GUI_SUPPORT
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

    //Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
    //                                            : COMMON_SIGNAL_DISPATCH_PROACTOR),
    //                               previous_signal_actions,
    //                               previous_signal_mask);
    Common_Log_Tools::finalizeLogging ();
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

  //Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
  //                                            : COMMON_SIGNAL_DISPATCH_PROACTOR),
  //                               previous_signal_actions,
  //                               previous_signal_mask);
  Common_Log_Tools::finalizeLogging ();
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
