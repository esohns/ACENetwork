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
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#elif defined (ACE_LINUX)
#include "sys/capability.h"
#include "linux/capability.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (SSL_SUPPORT)
#include "openssl/prov_ssl.h"
#endif // SSL_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#include "libavutil/pixfmt.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include <iostream>
#include <limits>
#include <regex>
#include <string>

#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_os_tools.h"

#if defined (SSL_SUPPORT)
#include "common_error_tools.h"
#endif // SSL_SUPPORT

#include "common_event_tools.h"

#include "common_logger_queue.h"
#include "common_log_tools.h"

#include "common_timer_tools.h"

#include "common_ui_tools.h"
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#include "stream_misc_defines.h"

#include "stream_file_sink.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

#include "net_client_defines.h"

#include "http_defines.h"

#include "test_i_common.h"
#include "test_i_common_modules.h"
#include "test_i_connection_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_connection_stream.h"
#include "test_i_defines.h"
#include "test_i_eventhandler.h"
#include "test_i_message.h"
#include "test_i_session_message.h"
#include "test_i_signalhandler.h"
#include "test_i_stream.h"

#include "test_i_web_tv_common.h"
#include "test_i_web_tv_defines.h"

#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("WebTVStream");
const char stream_name_string_3[] = ACE_TEXT_ALWAYS_CHAR ("WebTVStream_3");
const char stream_name_string_4a[] = ACE_TEXT_ALWAYS_CHAR ("WebTVAudioInputStream");
const char stream_name_string_4b[] = ACE_TEXT_ALWAYS_CHAR ("WebTVAVInputStream");

void
do_print_usage (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_print_usage"));

  // enable verbatim boolean output
  std::cout.setf (std::ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_TEST_I_SUBDIRECTORY),
                                                      true); // configuration-

  std::cout << ACE_TEXT_ALWAYS_CHAR ("usage: ")
            << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("currently available options:")
            << std::endl;
  std::string ini_file = configuration_path;
  ini_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ini_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_INI_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [PATH] : configuration file [\"")
            << ini_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d        : debug parser [")
            << COMMON_PARSER_DEFAULT_YACC_TRACE
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#if defined (GTK_SUPPORT)
  std::string gtk_rc_file = configuration_path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e [PATH] : Gtk .rc file [\"")
            << gtk_rc_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
#endif // GTK_SUPPORT
  std::string output_file = Common_File_Tools::getTempDirectory ();
  output_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_DEFAULT_TARGET_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f[[PATH]]: (output) file [")
            << output_file
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string UI_file = configuration_path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[PATH]]: UI definition file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" : no GUI}")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l        : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r        : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]: statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t        : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u [id]   : (default) channel# [")
            << 0
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v        : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-z        : use hardware decoder [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_process_arguments (int argc_in,
                      ACE_TCHAR** argv_in, // cannot be const...
                      std::string& configurationFile_out,
                      bool& debugParser_out,
#if defined (GTK_SUPPORT)
                      std::string& GtkRcFileName_out,
#endif // GTK_SUPPORT
                      std::string& outputFileName_out,
                      std::string& UIDefinitonFileName_out,
                      bool& logToFile_out,
                      bool& useReactor_out,
                      ACE_Time_Value& statisticReportingInterval_out,
                      bool& traceInformation_out,
                      unsigned int& channel_out,
                      bool& printVersionAndExit_out,
                      bool& useHardwareDecoder_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_process_arguments"));

  std::string working_directory =
    Common_File_Tools::getWorkingDirectory ();
  std::string temp_directory = Common_File_Tools::getTempDirectory ();
  std::string configuration_directory =
      Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                        ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_TEST_I_SUBDIRECTORY),
                                                        true); // configuration-

  // initialize results
  configurationFile_out = configuration_directory;
  configurationFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configurationFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_INI_FILE);
  debugParser_out = COMMON_PARSER_DEFAULT_YACC_TRACE;
#if defined (GTK_SUPPORT)
  GtkRcFileName_out = configuration_directory;
  GtkRcFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  GtkRcFileName_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_RC_FILE);
#endif // GTK_SUPPORT
  outputFileName_out = temp_directory;
  outputFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  outputFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_DEFAULT_TARGET_FILE);
  UIDefinitonFileName_out = configuration_directory;
  UIDefinitonFileName_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UIDefinitonFileName_out +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_GLADE_FILE);
  logToFile_out = false;
  useReactor_out = (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
    (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S ? ACE_Time_Value (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S, 0)
                                                   : ACE_Time_Value::zero);
  traceInformation_out = false;
  channel_out = 0;
  printVersionAndExit_out = false;
  useHardwareDecoder_out = false;

  std::string options_string = ACE_TEXT_ALWAYS_CHAR ("c:df:lrs:tu:vz");
#if defined (GTK_SUPPORT)
  options_string += ACE_TEXT_ALWAYS_CHAR ("e:");
#endif // GTK_SUPPORT
  options_string += ACE_TEXT_ALWAYS_CHAR ("g:");
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
      case 'c':
      {
        configurationFile_out =
            ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'd':
      {
        debugParser_out = true;
        break;
      }
#if defined (GTK_SUPPORT)
      case 'e':
      {
        GtkRcFileName_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
#endif // GTK_SUPPORT
      case 'f':
      {
        outputFileName_out = ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        break;
      }
      case 'g':
      {
        UIDefinitonFileName_out =
            ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
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
        unsigned int value = 0;
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        converter >> value;
        statisticReportingInterval_out =
          (value ? ACE_Time_Value (value, 0) : ACE_Time_Value::zero);
        break;
      }
      case 't':
      {
        traceInformation_out = true;
        break;
      }
      case 'u':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << ACE_TEXT_ALWAYS_CHAR (argument_parser.opt_arg ());
        converter >> channel_out;
        break;
      }
      case 'v':
      {
        printVersionAndExit_out = true;
        break;
      }
      case 'z':
      {
        useHardwareDecoder_out = true;
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
do_initialize_signals (ACE_Sig_Set& signals_out,
                       ACE_Sig_Set& ignoredSignals_out)
{
  NETWORK_TRACE (ACE_TEXT ("::do_initialize_signals"));

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
  // --> add valid signals manually (see <signal.h>)...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signals_out.sig_add (SIGINT);            // 2       /* interrupt */
  signals_out.sig_add (SIGILL);            // 4       /* illegal instruction - invalid function image */
  signals_out.sig_add (SIGFPE);            // 8       /* floating point exception */
//  signals_out.sig_add (SIGSEGV);           // 11      /* segment violation */
  signals_out.sig_add (SIGTERM);           // 15      /* Software termination signal from kill */
  signals_out.sig_add (SIGBREAK);          // 21      /* Ctrl-Break sequence */
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
#endif // ACE_WIN32 || ACE_WIN64
}

bool
do_parse_configuration_file (const std::string& fileName_in,
                            Test_I_WebTV_ChannelConfigurations_t& configuration_inout)
{
  NETWORK_TRACE (ACE_TEXT ("::do_parse_configuration_file"));

     // initialize return value(s)
  configuration_inout.clear ();

  ACE_Configuration_Heap configuration_heap;
  int result = configuration_heap.open ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("ACE_Configuration_Heap::open() failed: \"%m\", aborting\n")));
    return false;
  } // end IF

  ACE_Ini_ImpExp ini_import_export (configuration_heap);
  result = ini_import_export.import_config (fileName_in.c_str ());
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("ACE_Ini_ImpExp::import_config(\"%s\") failed, aborting\n"),
               ACE_TEXT (fileName_in.c_str ())));
    return false;
  } // end IF

  // find/open "channels" section...
  ACE_Configuration_Section_Key root_section_key;
  result =
      configuration_heap.open_section (configuration_heap.root_section (),
                                       ACE_TEXT (TEST_I_WEBTV_CNF_CHANNELS_SECTION_HEADER),
                                       0, // MUST exist !
                                       root_section_key);
  if (unlikely (result == -1))
  {
    ACE_ERROR ((LM_ERROR,
               ACE_TEXT ("failed to ACE_Configuration_Heap::open_section(%s), returning\n"),
               ACE_TEXT (TEST_I_WEBTV_CNF_CHANNELS_SECTION_HEADER)));
    return false;
  } // end IF

  // import channels...
  int index = 0;
  ACE_TString section_name;
  struct Test_I_WebTV_ChannelConfiguration channel_configuration_s;
  unsigned int channel_number = 0;
  while (configuration_heap.enumerate_sections (root_section_key,
                                                index,
                                                section_name) == 0)
  {
    // open section
    ACE_Configuration_Section_Key section_key;
    result =
        configuration_heap.open_section (root_section_key,
                                         section_name.c_str (),
                                         0, // MUST exist !
                                         section_key);
    ACE_ASSERT (result == 0);

    int index_2 = 0;
    ACE_TString item_name, item_value;
    ACE_Configuration::VALUETYPE item_type;
    u_int item_value_2;
    while (configuration_heap.enumerate_values (section_key,
                                                index_2,
                                                item_name,
                                                item_type) == 0)
    {
      switch (item_type)
      {
        case ACE_Configuration::INTEGER:
        {
          result =
              configuration_heap.get_integer_value (section_key,
                                                    ACE_TEXT (item_name.c_str ()),
                                                    item_value_2);
          if (unlikely (result == -1))
          {
            ACE_ERROR ((LM_ERROR,
                       ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"): \"%m\", aborting\n"),
                       ACE_TEXT (item_name.c_str ())));
            return false;
          } // end IF
          if (!ACE_OS::strcmp (item_name.c_str (),
                              ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_CHANNEL_NUMBER_KEY)))
            channel_number = item_value_2;
          else if (!ACE_OS::strcmp (item_name.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_CHANNEL_INDEX_POSITIONS_KEY)))
            channel_configuration_s.indexPositions = item_value_2;
          else if (!ACE_OS::strcmp (item_name.c_str (),
                                   ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_CHANNEL_MAX_INDEX_KEY)))
            channel_configuration_s.maxIndex = item_value_2;
          break;
        }
        case ACE_Configuration::STRING:
        {
          result =
              configuration_heap.get_string_value (section_key,
                                                   ACE_TEXT (item_name.c_str ()),
                                                   item_value);
          if (unlikely (result == -1))
          {
            ACE_ERROR ((LM_ERROR,
                       ACE_TEXT ("failed to ACE_Configuration_Heap::get_string_value(\"%s\"): \"%m\", aborting\n"),
                       ACE_TEXT (item_name.c_str ())));
            return false;
          } // end IF
          if (!ACE_OS::strcmp (item_name.c_str (),
                               ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_CHANNEL_NUMBER_KEY)))
            channel_number = ACE_OS::atoi (item_value.c_str ());
          else if (!ACE_OS::strcmp (item_name.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_CHANNEL_NAME_KEY)))
            channel_configuration_s.name = item_value.c_str ();
          else if (!ACE_OS::strcmp (item_name.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_CHANNEL_INDEX_POSITIONS_KEY)))
            channel_configuration_s.indexPositions =
                ACE_OS::atoi (item_value.c_str ());
          else if (!ACE_OS::strcmp (item_name.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_CHANNEL_MAX_INDEX_KEY)))
            channel_configuration_s.maxIndex =
                ACE_OS::atoi (item_value.c_str ());
          else if (!ACE_OS::strcmp (item_name.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_CHANNEL_URL_KEY)))
            channel_configuration_s.mainURL = item_value.c_str ();
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown item type (was: %d), continuing\n"),
                      item_type));
          break;
        }
      } // end SWITCH
      ++index_2;
    } // end WHILE
    configuration_inout.insert (std::make_pair (channel_number, channel_configuration_s));
    channel_configuration_s.clear ();
    ++index;
  } // end WHILE
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("imported \"%s\": %u channel(s)\n"),
              ACE_TEXT (fileName_in.c_str ()),
              configuration_inout.size ()));

  return true;
}

void
do_work (const std::string& configurationFile_in,
         bool debugParser_in,
         const std::string& outputFileName_in,
         const std::string& UIDefinitionFileName_in,
         bool useReactor_in,
         const ACE_Time_Value& statisticReportingInterval_in,
         unsigned int channel_in,
         struct Test_I_WebTV_Configuration& configuration_in,
         struct Test_I_WebTV_UI_CBData& CBData_in,
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         Test_I_SignalHandler& signalHandler_in,
         bool useHardwareDecoder_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_work"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_MediaFramework_DirectSound_Tools::initialize ();
#endif // ACE_WIN32 || ACE_WIN64

  Net_Common_Tools::initialize ();

  // step0a: initialize configuration and stream
  Test_I_WebTV_ChannelConfigurations_t channels;
  if (unlikely (!do_parse_configuration_file (configurationFile_in,
                                              channels)))
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to do_parse_configuration_file(\"%s\"), returning\n"),
               ACE_TEXT (configurationFile_in.c_str ())));
    return;
  } // end IF
  Test_I_WebTV_ChannelConfigurationsIterator_t channel_iterator;
  ACE_INET_Addr host_address;
  std::string hostname_string, URI_string;
  bool use_SSL = false;
  if (channel_in)
  {
    channel_iterator = channels.find (channel_in);
    if (unlikely (channel_iterator == channels.end ()))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("invalid channel# (was: %u), returning\n"),
                 channel_in));
      return;
    } // end IF
    if (!HTTP_Tools::parseURL ((*channel_iterator).second.mainURL,
                               host_address,
                               hostname_string,
                               URI_string,
                               use_SSL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HTTP_Tools::parseURL(\"%s\"), returning\n"),
                  ACE_TEXT ((*channel_iterator).second.mainURL.c_str ())));
      return;
    } // end IF
  } // end IF

#if defined (SSL_SUPPORT)
  std::string filename_string =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                      (Common_Error_Tools::inDebugSession () ? ACE_TEXT_ALWAYS_CHAR ("..") : ACE_TEXT_ALWAYS_CHAR ("..")),
                                                      false); // data
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (NET_PROTOCOL_DEFAULT_SSL_TRUSTED_CAS_FILENAME_STRING);
  if (unlikely (!Net_Common_Tools::initializeSSLContext (ACE_TEXT_ALWAYS_CHAR (""),
                                                         ACE_TEXT_ALWAYS_CHAR (""),
                                                         filename_string, // trusted CAs
                                                         true,            // client
                                                         NULL)))          // default context
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to Net_Common_Tools::initializeSSLContext(), returning\n")));
    return;
  } // end IF
#endif // SSL_SUPPORT

  Test_I_MessageHandler_Module event_handler_module (NULL,
                                                     ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_MessageHandler_Module event_handler_module_2 (NULL,
                                                       ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_MessageHandler_3_Module event_handler_module_3 (NULL,
                                                         ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_AllocatorConfiguration allocator_configuration_2;
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration video_codec_configuration;
  video_codec_configuration.codecId = AV_CODEC_ID_H264;
  // video_codec_configuration.profile = FF_PROFILE_H264_HIGH;
  video_codec_configuration.profile = FF_PROFILE_H264_BASELINE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  video_codec_configuration.deviceType = AV_HWDEVICE_TYPE_DXVA2;
  //video_codec_configuration.deviceType = AV_HWDEVICE_TYPE_D3D11VA;
  //video_codec_configuration.deviceType = AV_HWDEVICE_TYPE_D3D12VA;
  video_codec_configuration.format = AV_PIX_FMT_DXVA2_VLD;
#else
  video_codec_configuration.deviceType = AV_HWDEVICE_TYPE_VAAPI;
  video_codec_configuration.format = AV_PIX_FMT_VAAPI;
  // video_codec_configuration.deviceType = AV_HWDEVICE_TYPE_VDPAU;
  // video_codec_configuration.format = AV_PIX_FMT_VDPAU;
#endif // ACE_WIN32 || ACE_WIN64
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration audio_codec_configuration;
  audio_codec_configuration.codecId = AV_CODEC_ID_AAC;
#else
  struct Stream_AllocatorConfiguration allocator_configuration_2;
#endif // FFMPEG_SUPPORT
  allocator_configuration.defaultBufferSize = TEST_I_DEFAULT_BUFFER_SIZE * 10;
  allocator_configuration_2.defaultBufferSize = TEST_I_DEFAULT_BUFFER_SIZE * 10;

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (allocator_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  Test_I_MessageAllocator_t message_allocator (NET_STREAM_MAX_MESSAGES, // maximum #buffers
                                               &heap_allocator,         // heap allocator handle
                                               true);                   // block ?
  Test_I_MessageAllocator_3_t message_allocator_3 (NET_STREAM_MAX_MESSAGES, // maximum #buffers
                                                   &heap_allocator,         // heap allocator handle
                                                   true);                   // block ?
  Test_I_WebTV_MessageQueue_t av_input_queue (STREAM_QUEUE_MAX_SLOTS,
                                              NULL);
  Test_I_WebTV_MessageQueue_t audio_input_queue (STREAM_QUEUE_MAX_SLOTS,
                                                 NULL);
  Test_I_WebTV_MessageQueue_t audio_output_queue (STREAM_QUEUE_MAX_SLOTS,
                                                  NULL);
  Test_I_WebTV_MessageQueue_t save_queue (STREAM_QUEUE_MAX_SLOTS,
                                          NULL);
  // Test_I_WebTV_MessageQueue_t save_audio_queue (STREAM_QUEUE_MAX_SLOTS,
  //                                               NULL); // inject (decoded) audio messages
  Test_I_AVStream av_input_stream;
  Test_I_AudioStream audio_input_stream;
  ACE_Thread_Mutex timeout_handler_lock;
  Test_I_TimeoutHandler timeout_handler, timeout_handler_2;
  timeout_handler.lock_ = &timeout_handler_lock;
  timeout_handler_2.lock_ = &timeout_handler_lock;
  Test_I_EventHandler message_handler (&CBData_in); // master.m3u(8)
  Test_I_EventHandler_2 message_handler_2 (&CBData_in); // audio/video playlists
  Test_I_EventHandler_3 message_handler_3 (&CBData_in); // audio/video content
  CBData_in.channels = &channels;
  CBData_in.currentChannel = channel_in;
  CBData_in.AVStream = &av_input_stream;
  CBData_in.AudioStream = &audio_input_stream;
  CBData_in.audioTimeoutHandler = &timeout_handler;
  CBData_in.videoTimeoutHandler = &timeout_handler_2;

  // *********************** socket configuration data ************************
  Test_I_WebTV_ConnectionConfiguration_t connection_configuration;
  connection_configuration.socketConfiguration.method = TLS_client_method ();
  connection_configuration.socketConfiguration.minimalVersion = TLS1_3_VERSION;
  connection_configuration.socketConfiguration.maximalVersion = TLS1_3_VERSION;
  connection_configuration.allocatorConfiguration = &allocator_configuration;
  connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  connection_configuration.messageAllocator = &message_allocator;
  connection_configuration.streamConfiguration =
    &configuration_in.streamConfiguration;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                    &connection_configuration));

  Test_I_WebTV_ConnectionConfiguration_t connection_configuration_2a =
    connection_configuration;
  connection_configuration_2a.streamConfiguration =
    &configuration_in.streamConfiguration_2a;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("2a"),
                                                                    &connection_configuration_2a));
  Test_I_WebTV_ConnectionConfiguration_t connection_configuration_2b =
    connection_configuration;
  connection_configuration_2b.streamConfiguration =
    &configuration_in.streamConfiguration_2b;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("2b"),
                                                                    &connection_configuration_2b));

  Test_I_WebTV_ConnectionConfiguration_3_t connection_configuration_3a;
  connection_configuration_3a.socketConfiguration.method = TLS_client_method ();
  connection_configuration_3a.socketConfiguration.minimalVersion = TLS1_3_VERSION;
  connection_configuration_3a.socketConfiguration.maximalVersion = TLS1_3_VERSION;
  connection_configuration_3a.allocatorConfiguration = &allocator_configuration;
  connection_configuration_3a.messageAllocator = &message_allocator_3;
  connection_configuration_3a.streamConfiguration =
    &configuration_in.streamConfiguration_3a;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("3a"),
                                                                    &connection_configuration_3a));
  Test_I_WebTV_ConnectionConfiguration_3_t connection_configuration_3b =
    connection_configuration_3a;
  connection_configuration_3b.streamConfiguration =
    &configuration_in.streamConfiguration_3b;
  configuration_in.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("3b"),
                                                                    &connection_configuration_3b));

  // ********************** stream configuration data **************************
  // ********************** parser configuration data **************************
#if defined (_DEBUG)
  configuration_in.parserConfiguration.debugParser = debugParser_in;
  if (debugParser_in)
    configuration_in.parserConfiguration.debugScanner = true;
#endif // _DEBUG
  // ********************** module configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
  struct Test_I_WebTV_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_I_WebTV_ModuleHandlerConfiguration modulehandler_configuration_marshal; // marshal
  struct Test_I_WebTV_ModuleHandlerConfiguration modulehandler_configuration_2a;
  struct Test_I_WebTV_ModuleHandlerConfiguration modulehandler_configuration_marshal_2a; // marshal
  struct Test_I_WebTV_ModuleHandlerConfiguration modulehandler_configuration_2b;
  struct Test_I_WebTV_ModuleHandlerConfiguration modulehandler_configuration_marshal_2b; // marshal

  struct Test_I_WebTV_StreamConfiguration stream_configuration;
  struct Test_I_WebTV_StreamConfiguration stream_configuration_2a;
  struct Test_I_WebTV_StreamConfiguration stream_configuration_2b;

  modulehandler_configuration.allocatorConfiguration =
    &allocator_configuration;
  modulehandler_configuration.defragmentMode = STREAM_DEFRAGMENT_CONDENSE;
  modulehandler_configuration.closeAfterReception = true;
  modulehandler_configuration.concurrency =
      STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration.connectionConfigurations =
    &configuration_in.connectionConfigurations;
  modulehandler_configuration.messageAllocator = &message_allocator;
  struct HTTP_ParserConfiguration parser_configuration;
  modulehandler_configuration.parserConfiguration =
    &parser_configuration;
//  modulehandler_configuration.statisticReportingInterval =
//    statisticReportingInterval_in;
  modulehandler_configuration.subscriber = &message_handler;
  modulehandler_configuration.waitForConnect = false;
  // ******************** (sub-)stream configuration data *********************
  stream_configuration.messageAllocator = &message_allocator;
  stream_configuration.module = &event_handler_module;
  stream_configuration.printFinalReport = true;
  configuration_in.streamConfiguration.initialize (module_configuration,
                                                   modulehandler_configuration,
                                                   stream_configuration);

  modulehandler_configuration_marshal = modulehandler_configuration;
  struct HTTP_ParserConfiguration parserConfiguration_2;
  modulehandler_configuration_marshal.parserConfiguration =
    &parserConfiguration_2;
  configuration_in.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
                                                               std::make_pair (&module_configuration,
                                                                               &modulehandler_configuration_marshal)));

  modulehandler_configuration_2a = modulehandler_configuration;
  struct HTTP_ParserConfiguration parserConfiguration_2a;
  modulehandler_configuration_2a.parserConfiguration = &parserConfiguration_2a;
  modulehandler_configuration_2a.subscriber = &message_handler_2;
  // ******************** (sub-)stream configuration data *********************
  stream_configuration_2a = stream_configuration;
  stream_configuration_2a.cloneModule = true;
  stream_configuration_2a.module = &event_handler_module_2;
  configuration_in.streamConfiguration_2a.initialize (module_configuration,
                                                      modulehandler_configuration_2a,
                                                      stream_configuration_2a);
  modulehandler_configuration_marshal_2a = modulehandler_configuration_2a;
  struct HTTP_ParserConfiguration parserConfiguration_marshal_2a;
  modulehandler_configuration_marshal_2a.parserConfiguration =
    &parserConfiguration_marshal_2a;
  configuration_in.streamConfiguration_2a.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_marshal_2a)));
  modulehandler_configuration_2b = modulehandler_configuration;
  struct HTTP_ParserConfiguration parserConfiguration_2b;
  modulehandler_configuration_2b.parserConfiguration = &parserConfiguration_2b;
  modulehandler_configuration_2b.subscriber = &message_handler_2;
  // ******************** (sub-)stream configuration data *********************
  stream_configuration_2b = stream_configuration;
  stream_configuration_2b.cloneModule = true;
  stream_configuration_2b.module = &event_handler_module_2;
  configuration_in.streamConfiguration_2b.initialize (module_configuration,
                                                      modulehandler_configuration_2b,
                                                      stream_configuration_2b);
  modulehandler_configuration_marshal_2b = modulehandler_configuration_2b;
  struct HTTP_ParserConfiguration parserConfiguration_marshal_2b;
  modulehandler_configuration_marshal_2b.parserConfiguration =
    &parserConfiguration_marshal_2b;
  configuration_in.streamConfiguration_2b.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_marshal_2b)));

  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_3a;
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_marshal_3a;
  modulehandler_configuration_3a.allocatorConfiguration =
    &allocator_configuration;
  modulehandler_configuration_3a.defragmentMode = STREAM_DEFRAGMENT_CONDENSE;
  modulehandler_configuration_3a.closeAfterReception = true;
  modulehandler_configuration_3a.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration_3a.connectionConfigurations =
    &configuration_in.connectionConfigurations;
  modulehandler_configuration_3a.messageAllocator = &message_allocator;
  struct HTTP_ParserConfiguration parserConfiguration_3a;
  modulehandler_configuration_3a.parserConfiguration =
    &parserConfiguration_3a;
  modulehandler_configuration_3a.queue = &audio_input_queue;
  //  modulehandler_configuration_3a.statisticReportingInterval =
  //    statisticReportingInterval_in;
  modulehandler_configuration_3a.subscriber = &message_handler_3;
  modulehandler_configuration_3a.waitForConnect = false;
  struct Test_I_WebTV_StreamConfiguration_3 stream_configuration_3a;
  stream_configuration_3a.messageAllocator = &message_allocator_3;
  stream_configuration_3a.cloneModule = true;
  stream_configuration_3a.module = &event_handler_module_3;
  configuration_in.streamConfiguration_3a.initialize (module_configuration,
                                                      modulehandler_configuration_3a,
                                                      stream_configuration_3a);
  modulehandler_configuration_marshal_3a = modulehandler_configuration_3a;
  struct HTTP_ParserConfiguration parserConfiguration_marshal_3a;
  modulehandler_configuration_marshal_3a.parserConfiguration =
    &parserConfiguration_marshal_3a;
  configuration_in.streamConfiguration_3a.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_marshal_3a)));

  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_3b;
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_marshal_3b;
  modulehandler_configuration_3b.allocatorConfiguration =
    &allocator_configuration;
  modulehandler_configuration_3b.defragmentMode = STREAM_DEFRAGMENT_CONDENSE;
  modulehandler_configuration_3b.closeAfterReception = true;
  modulehandler_configuration_3b.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration_3b.connectionConfigurations =
    &configuration_in.connectionConfigurations;
  modulehandler_configuration_3b.messageAllocator = &message_allocator;
  modulehandler_configuration_3b.queue = &av_input_queue;
  //  modulehandler_configuration_3b.statisticReportingInterval =
  //    statisticReportingInterval_in;
  modulehandler_configuration_3b.subscriber = &message_handler_3;
  modulehandler_configuration_3b.waitForConnect = false;
  struct HTTP_ParserConfiguration parserConfiguration_3b;
  modulehandler_configuration_3b.parserConfiguration =
      &parserConfiguration_3b;
  struct Test_I_WebTV_StreamConfiguration_3 stream_configuration_3b;
  stream_configuration_3b.messageAllocator = &message_allocator_3;
  stream_configuration_3b.cloneModule = true;
  stream_configuration_3b.module = &event_handler_module_3;
  configuration_in.streamConfiguration_3b.initialize (module_configuration,
                                                      modulehandler_configuration_3b,
                                                      stream_configuration_3b);
  modulehandler_configuration_marshal_3b = modulehandler_configuration_3b;
  struct HTTP_ParserConfiguration parserConfiguration_marshal_3b;
  modulehandler_configuration_marshal_3b.parserConfiguration =
    &parserConfiguration_marshal_3b;
  configuration_in.streamConfiguration_3b.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("Marshal"),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_marshal_3b)));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Stream_MediaFramework_ALSA_Configuration ALSA_configuration;
  ALSA_configuration.asynch = false;
#endif // ACE_WIN32 || ACE_WIN64
  struct Stream_Miscellaneous_DelayConfiguration delay_configuration;
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_4a;
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_queue_sink_4a;
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_4b;
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_audio_injector_4b; // audio injector
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_audio_decoder_4b; // audio decoder
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_audio_queue_sink_4b; // audio queue
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_save_queue_source_4b; // save queue
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_save_converter_4b; // save converter
  struct Test_I_WebTV_ModuleHandlerConfiguration_3 modulehandler_configuration_save_audio_injector_4b; // save audio injector
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  modulehandler_configuration_4b.ALSAConfiguration = &ALSA_configuration;
#endif // ACE_WIN32 || ACE_WIN64
  modulehandler_configuration_4b.allocatorConfiguration =
    &allocator_configuration_2;
#if defined (FFMPEG_SUPPORT)
  modulehandler_configuration_4b.codecConfiguration = &video_codec_configuration;
#endif // FFMPEG_SUPPORT
  modulehandler_configuration_4b.concurrency =
      STREAM_HEADMODULECONCURRENCY_ACTIVE;
  modulehandler_configuration_4b.defragmentMode = STREAM_DEFRAGMENT_CLONE;
//  delay_configuration.mode = STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES;
  delay_configuration.mode = STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER;
  delay_configuration.interval = ACE_Time_Value (1, 0); // frames per second
  modulehandler_configuration_4b.delayConfiguration = &delay_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration_4b.deviceIdentifier.identifierDiscriminator =
    Stream_Device_Identifier::GUID;
  modulehandler_configuration_4b.deviceIdentifier.identifier._guid =
    Stream_MediaFramework_DirectSound_Tools::getDefaultDevice (false); // playback
  struct tWAVEFORMATEX* waveformatex_p =
    Stream_MediaFramework_DirectSound_Tools::getAudioEngineMixFormat (modulehandler_configuration_4b.deviceIdentifier.identifier._guid);
#else
  modulehandler_configuration_4b.deviceIdentifier.identifier =
    Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                     SND_PCM_STREAM_PLAYBACK);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration_4b.outputFormat.audio.format =
      Stream_Module_Decoder_Tools::to (*waveformatex_p);
  modulehandler_configuration_4b.outputFormat.audio.channels =
      waveformatex_p->nChannels;
  modulehandler_configuration_4b.outputFormat.audio.sampleRate =
      waveformatex_p->nSamplesPerSec;
  CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
#else
  modulehandler_configuration_4b.outputFormat.audio.format = AV_SAMPLE_FMT_FLT;
  modulehandler_configuration_4b.outputFormat.audio.channels = 2;
  modulehandler_configuration_4b.outputFormat.audio.sampleRate = 48000;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  modulehandler_configuration_4b.outputFormat.video.format = AV_PIX_FMT_RGB24;
#else
  modulehandler_configuration_4b.outputFormat.video.format = AV_PIX_FMT_RGB32;
#endif // ACE_WIN32 || ACE_WIN64
#endif // FFMPEG_SUPPORT
  modulehandler_configuration_4b.queue = &av_input_queue;
  modulehandler_configuration_4b.display = Common_UI_Tools::getDefaultDisplay ();
  // struct HTTP_ParserConfiguration parserConfiguration_4;
  modulehandler_configuration_4b.statisticReportingInterval =
    statisticReportingInterval_in;
  modulehandler_configuration_4b.subscriber = &message_handler_3;
  modulehandler_configuration_4b.targetFileName =
    outputFileName_in;

  modulehandler_configuration_4a = modulehandler_configuration_4b;
  modulehandler_configuration_4a.queue = &audio_input_queue;
  struct Test_I_WebTV_StreamConfiguration_3 stream_configuration_4a; // audio input-
  stream_configuration_4a.messageAllocator = &message_allocator_3;
  configuration_in.streamConfiguration_4a.initialize (module_configuration,
                                                      modulehandler_configuration_4a,
                                                      stream_configuration_4a);
  modulehandler_configuration_queue_sink_4a = modulehandler_configuration_4a;
  modulehandler_configuration_queue_sink_4a.queue = &audio_output_queue;
  configuration_in.streamConfiguration_4a.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SINK_DEFAULT_NAME_STRING),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_queue_sink_4a)));

  struct Test_I_WebTV_StreamConfiguration_3 stream_configuration_4b; // AV input-
#if defined (FFMPEG_SUPPORT)
  stream_configuration_4b.mediaType.audio.channels = 2;
  stream_configuration_4b.mediaType.audio.format = AV_SAMPLE_FMT_FLT;
  stream_configuration_4b.mediaType.audio.sampleRate = 48000;
  //stream_configuration_4b.mediaType.video.frameRate.num = 30;
#endif // FFMPEG_SUPPORT
  stream_configuration_4b.messageAllocator = &message_allocator_3;
  //stream_configuration_4b.cloneModule = false;
  stream_configuration_4b.module = &event_handler_module_3;
  stream_configuration_4b.printFinalReport = true;
  stream_configuration_4b.useHardwareDecoder = useHardwareDecoder_in;
  configuration_in.streamConfiguration_4b.initialize (module_configuration,
                                                      modulehandler_configuration_4b,
                                                      stream_configuration_4b);
  modulehandler_configuration_audio_injector_4b = modulehandler_configuration_4b;
  modulehandler_configuration_audio_injector_4b.queue = &audio_output_queue;
  configuration_in.streamConfiguration_4b.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_INJECTOR_DEFAULT_NAME_STRING),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_audio_injector_4b)));
  modulehandler_configuration_audio_decoder_4b = modulehandler_configuration_4b;
#if defined (FFMPEG_SUPPORT)
  modulehandler_configuration_audio_decoder_4b.codecConfiguration = &audio_codec_configuration;
#endif // FFMPEG_SUPPORT
  configuration_in.streamConfiguration_4b.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_AUDIO_DECODER_DEFAULT_NAME_STRING),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_audio_decoder_4b)));
  modulehandler_configuration_audio_queue_sink_4b = modulehandler_configuration_4b;
  modulehandler_configuration_audio_queue_sink_4b.queue = &save_queue;
  configuration_in.streamConfiguration_4b.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_QUEUE_SINK_DEFAULT_NAME_STRING),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_audio_queue_sink_4b)));
  modulehandler_configuration_save_queue_source_4b = modulehandler_configuration_4b;
  modulehandler_configuration_save_queue_source_4b.autoStart = true;
  modulehandler_configuration_save_queue_source_4b.concurrency =
    STREAM_HEADMODULECONCURRENCY_ACTIVE;
  modulehandler_configuration_save_queue_source_4b.queue = &save_queue;
  configuration_in.streamConfiguration_4b.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("QueueSource_2"),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_save_queue_source_4b)));
  modulehandler_configuration_save_converter_4b = modulehandler_configuration_4b;
  modulehandler_configuration_save_converter_4b.flipImage = true;
  modulehandler_configuration_save_converter_4b.handleResize = false;
  modulehandler_configuration_save_converter_4b.outputFormat.video.format =
    AV_PIX_FMT_BGR24;
  configuration_in.streamConfiguration_4b.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_CONVERTER_DEFAULT_NAME_STRING),
                                                                  std::make_pair (&module_configuration,
                                                                                  &modulehandler_configuration_save_converter_4b)));

  // modulehandler_configuration_save_audio_injector_4b = modulehandler_configuration_4b;
  // modulehandler_configuration_save_audio_injector_4b.queue = &save_audio_queue;
  // configuration_in.streamConfiguration_4b.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("Injector_2"),
  //                                                                 std::make_pair (&module_configuration,
  //                                                                                 &modulehandler_configuration_save_audio_injector_4b)));

  // step0c: initialize connection manager
  Test_I_ConnectionManager_t* connection_manager_p =
    TEST_I_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (connection_configuration,
                             NULL);
  Test_I_ConnectionManager_3_t* connection_manager_3 =
    TEST_I_CONNECTIONMANAGER_SINGLETON_3::instance ();
  ACE_ASSERT (connection_manager_3);
  connection_manager_3->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_3->set (connection_configuration_3a,
                             NULL);

  Common_Timer_Manager_t* timer_manager_p =
    COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  struct Common_TimerConfiguration timer_configuration;
  timer_configuration.dispatch = COMMON_TIMER_DISPATCH_REACTOR;
#if defined (GTK_SUPPORT)
  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
#endif // GTK_SUPPORT
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
    &configuration_in.dispatchConfiguration;

  // step0b: initialize event dispatch
  configuration_in.dispatchConfiguration.numberOfReactorThreads =
    ((configuration_in.dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_REACTOR) ? TEST_I_WEBTV_NUMBER_OF_DISPATCH_THREADS_REACTOR : 0);
  configuration_in.dispatchConfiguration.numberOfProactorThreads =
    ((configuration_in.dispatchConfiguration.dispatch == COMMON_EVENT_DISPATCH_PROACTOR) ? TEST_I_WEBTV_NUMBER_OF_DISPATCH_THREADS_PROACTOR : 0);
  if (!Common_Event_Tools::initializeEventDispatch (configuration_in.dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    goto clean;
  } // end IF

  // step0c: initialize signal handling
  //CBData_in.configuration->signalHandlerConfiguration.hasUI =
  //  !interfaceDefinitionFile_in.empty ();
  configuration_in.signalHandlerConfiguration.dispatchState =
    &event_dispatch_state_s;
  //configuration.signalHandlerConfiguration.statisticReportingHandler =
  //  connection_manager_p;
  //configuration.signalHandlerConfiguration.statisticReportingTimerID = timer_id;
  if (!signalHandler_in.initialize (configuration_in.signalHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    goto clean;
  } // end IF
  if (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DEFAULT_DISPATCH_MODE,
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signalHandler_in,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
    goto clean;
  } // end IF

  // intialize timers
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);

  // step1a: start GTK event loop ?
  if (!UIDefinitionFileName_in.empty ())
  {
#if defined (GTK_USE)
    gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    gtk_manager_p->start (NULL);
    ACE_Time_Value timeout (0,
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
    int result = ACE_OS::sleep (timeout);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &timeout));
    if (!gtk_manager_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
      goto clean;
    } // end IF
#endif // GTK_USE

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    HWND window_p = GetConsoleWindow ();
//    if (!window_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
//      goto clean;
//    } // end IF
//    BOOL was_visible_b = false;
//    was_visible_b = ShowWindow (window_p, SW_HIDE);
//    ACE_UNUSED_ARG (was_visible_b);
//#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  else
  {
    if (!av_input_stream.initialize (configuration_in.streamConfiguration_3a))
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to Test_I_AVStream::initialize(), returning\n")));
      goto clean;
    } // end IF
    av_input_stream.start ();
  } // end IF

  // *WARNING*: from this point on, clean up any remote connections !

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistics collecting/reporting

  // step1a: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));
    goto clean;
  } // end IF

#if defined (GTK_USE)
  gtk_manager_p->wait (false);
#endif // GTK_USE

  // step3: clean up
  timer_manager_p->stop ();

  connection_manager_p->stop (false, // wait ?
                              true); // high priority ?
  connection_manager_p->abort (false); // wait ?
  connection_manager_3->stop (false, // wait ?
                              true); // high priority ?
  connection_manager_3->abort (false); // wait ?
  connection_manager_p->wait ();
  connection_manager_3->wait ();

  Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                             true,   // wait ?
                                             false); // close() singletons ?

  audio_input_stream.stop (true,  // wait for completion ?
                           true,  // recurse ?
                           true); // high priority
  audio_input_stream.remove (&event_handler_module_3,
                             true,   // lock ?
                             false); // reset ?
  av_input_stream.stop (true,  // wait for completion ?
                        true,  // recurse ?
                        true); // high priority
  av_input_stream.remove (&event_handler_module_3,
                          true,   // lock ?
                          false); // reset ?

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));

  return;

clean:
  Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                             true); // wait ?
  if (!UIDefinitionFileName_in.empty ())
#if defined (GTK_USE)
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (true,  // wait ?
                                                        true); // high priority ?
#else
    ;
#endif // GTK_USE
  else
    av_input_stream.stop (true,   // wait for completion ?
                          false,  // recurse ?
                          false); // high priority

  audio_input_stream.stop (true,   // wait for completion ?
                           false,  // recurse ?
                           false); // high priority
  av_input_stream.stop (true,   // wait for completion ?
                        false,  // recurse ?
                        false); // high priority

  timer_manager_p->stop ();
}

void
do_print_version (const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("::do_print_version"));

  std::ostringstream converter;

  // compiler version string
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
  // ACE version string
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
  ACE_Profile_Timer process_profile;
  std::string configuration_file;
  bool debug_parser;
  std::string configuration_path;
  std::string output_filename_string;
  std::string ui_definition_file;
  bool log_to_file;
  bool use_reactor;
  ACE_Time_Value statistic_reporting_interval;
  bool trace_information;
  unsigned int channel_i;
  bool print_version_and_exit;
  bool use_hardware_decoder_b;

  struct Test_I_WebTV_Configuration configuration;
  struct Test_I_WebTV_UI_CBData ui_cb_data;
  ui_cb_data.configuration = &configuration;
#if defined (GTK_SUPPORT)
  std::string gtk_rc_file;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
#endif // GTK_SUPPORT
#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
  Common_Logger_Queue_t logger;
  logger.initialize (&state_r.logQueue,
                     &state_r.logQueueLock);
  std::string log_file_name;
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
#if defined (GTK_USE)
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
  Test_I_SignalHandler signal_handler;
#if defined (GTK_USE)
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  configuration.GTKConfiguration.argc = argc_in;
  configuration.GTKConfiguration.argv = argv_in;
  configuration.GTKConfiguration.CBData = &ui_cb_data;
  configuration.GTKConfiguration.eventHooks.finiHook =
      idle_finalize_UI_cb;
  configuration.GTKConfiguration.eventHooks.initHook =
      idle_initialize_UI_cb;
  configuration.GTKConfiguration.definition = &gtk_ui_definition;
#endif // GTK_USE
  ACE_High_Res_Timer timer;
  std::string working_time_string;
  ACE_Time_Value working_time;
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  ACE_Profile_Timer::Rusage elapsed_rusage;
  std::string user_time_string, system_time_string;
  ACE_Time_Value user_time, system_time;

  // step0: initialize
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::init ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
  process_profile.start ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (true,   // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64
  Common_File_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (argv_in[0]));

  // step1a set defaults
  configuration_path =
      Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                        ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_TEST_I_SUBDIRECTORY),
                                                        true); // configuration-
  configuration_file = configuration_path;
  configuration_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_CNF_INI_FILE);
  debug_parser = false;
#if defined (GTK_SUPPORT)
  gtk_rc_file = configuration_path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_RC_FILE);
#endif // GTK_SUPPORT
  output_filename_string = Common_File_Tools::getTempDirectory ();
  output_filename_string += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  output_filename_string +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_WEBTV_DEFAULT_TARGET_FILE);
  ui_definition_file = configuration_path;
  ui_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  ui_definition_file +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_UI_GTK_DEFAULT_GLADE_FILE);
  log_to_file = false;
  use_reactor =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statistic_reporting_interval =
    (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S ? ACE_Time_Value (STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S, 0)
                                                   : ACE_Time_Value::zero);
  trace_information = false;
  channel_i = 0;
  print_version_and_exit = false;
  use_hardware_decoder_b = false;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (elapsed_rusage));

  // step1b: parse/process/validate configuration
  if (!do_process_arguments (argc_in,
                             argv_in,
                             configuration_file,
                             debug_parser,
#if defined (GTK_SUPPORT)
                             gtk_rc_file,
#endif // GTK_SUPPORT
                             output_filename_string,
                             ui_definition_file,
                             log_to_file,
                             use_reactor,
                             statistic_reporting_interval,
                             trace_information,
                             channel_i,
                             print_version_and_exit,
                             use_hardware_decoder_b))
  {
    do_print_usage (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                          ACE_DIRECTORY_SEPARATOR_CHAR)));
    goto error;
  } // end IF

  // step1c: validate arguments
  // *IMPORTANT NOTE*: iff the number of message buffers is limited, the
  //                   reactor/proactor thread could (dead)lock on the
  //                   allocator lock, as it cannot dispatch events that would
  //                   free slots
  if (TEST_I_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
  //if (use_reactor                      &&
  //    (number_of_dispatch_threads > 1) &&
  //    !use_thread_pool)
  //{ // *NOTE*: see also: man (2) select
  //  // *TODO*: verify this for MS Windows based systems
  //  ACE_DEBUG ((LM_WARNING,
  //              ACE_TEXT ("the select()-based reactor is not reentrant, using the thread-pool reactor instead...\n")));
  //  use_thread_pool = true;
  //} // end IF
  if (!Common_File_Tools::isReadable (configuration_file)
      || (!ui_definition_file.empty () &&
          !Common_File_Tools::isReadable (ui_definition_file))
#if defined (GTK_SUPPORT)
      || (!gtk_rc_file.empty () &&
          !Common_File_Tools::isReadable (gtk_rc_file))
#endif // GTK_SUPPORT
      //(use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool))
     )
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));
    do_print_usage (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                         ACE_DIRECTORY_SEPARATOR_CHAR)));
    goto error;
  } // end IF

  // step1d: initialize logging and/or tracing
  if (log_to_file)
  {
    log_file_name =
      Common_Log_Tools::getLogFilename (std::string (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME)),
                                        std::string (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                                                          ACE_DIRECTORY_SEPARATOR_CHAR))));
    if (log_file_name.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::getLogFilename(), aborting\n")));
      goto error;
    } // end IF
  } // end IF
  if (!Common_Log_Tools::initialize (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                                          ACE_DIRECTORY_SEPARATOR_CHAR)), // program name
                                     log_file_name,                                                       // log file name
                                     false,                                                               // log to syslog ?
                                     false,                                                               // trace messages ?
                                     trace_information,                                                   // debug messages ?
                                     NULL))                                                               // (ui-) logger ?
//                                            (ui_definition_file.empty () ? NULL
//                                                                         : &logger)))                            // (ui-) logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));
    goto error;
  } // end IF

  // step1e: pre-initialize signal handling
  do_initialize_signals (signal_set,
                         ignored_signal_set);
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
    goto error;
  } // end IF

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_print_version (std::string (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0],
                                                                        ACE_DIRECTORY_SEPARATOR_CHAR))));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_SUCCESS;
  } // end IF

  // step1g: set process resource limits
  // *NOTE*: settings will be inherited by any child processes
  if (!Common_OS_Tools::setResourceLimits (false, // file descriptors
                                           true,  // stack traces
                                           true)) // pending signals
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_OS_Tools::setResourceLimits(), aborting\n")));
    goto error;
  } // end IF

#if defined (GTK_USE)
  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
  if (!gtk_rc_file.empty ())
    ui_cb_data.configuration->GTKConfiguration.RCFiles.push_back (gtk_rc_file);
#endif // GTK_USE
  //Common_UI_GladeDefinition ui_definition (argc_in,
  //                                         argv_in);
  if (!ui_definition_file.empty ())
  {
    ui_cb_data.configuration = &configuration;
#if defined (GTK_USE)
//    ACE_ASSERT (CBData_in.UIState);
    state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (ui_definition_file, static_cast<GtkBuilder*> (NULL));
    ui_cb_data.progressData.state = &state_r;
    //ui_cb_data.userData = &configuration.userData;
    if (!gtk_manager_p->initialize (configuration.GTKConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager::initialize(), aborting\n")));
      goto error;
    } // end IF
#endif // GTK_USE
  } // end IF

  configuration.dispatchConfiguration.dispatch =
    (use_reactor ? COMMON_EVENT_DISPATCH_REACTOR
                 : COMMON_EVENT_DEFAULT_DISPATCH);

  timer.start ();
  // step2: do actual work
  do_work (configuration_file,
           debug_parser,
           output_filename_string,
           ui_definition_file,
           use_reactor,
           statistic_reporting_interval,
           channel_i,
           configuration,
           ui_cb_data,
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           signal_handler,
           use_hardware_decoder_b);
  timer.stop ();

  timer.elapsed_time (working_time);
  working_time_string = Common_Timer_Tools::periodToString (working_time);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));
  process_profile.stop ();

  // only process profile left to do
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  result = process_profile.elapsed_time (elapsed_time);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));
    goto error;
  } // end IF
  process_profile.elapsed_rusage (elapsed_rusage);
  user_time.set (elapsed_rusage.ru_utime);
  system_time.set (elapsed_rusage.ru_stime);
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;

error:
  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalize ();
  Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
  return EXIT_FAILURE;
} // end main
