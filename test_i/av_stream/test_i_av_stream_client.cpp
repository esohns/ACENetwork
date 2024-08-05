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
#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define INITGUID // *NOTE*: this exports DEFINE_GUIDs (see stream_misc_common.h)
#include "mfapi.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Log_Msg.h"
#include "ace/Profile_Timer.h"
#include "ace/Sig_Handler.h"
#include "ace/Signal.h"
#include "ace/Synch.h"
#include "ace/Version.h"

#if defined (HAVE_CONFIG_H)
#include "Common_config.h"
#endif // HAVE_CONFIG_H

#include "common_file_tools.h"
#include "common_os_tools.h"

#include "common_event_tools.h"

#include "common_log_tools.h"
#if defined (GUI_SUPPORT)
#include "common_logger_queue.h"
#endif // GUI_SUPPORT

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_defines.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

#include "stream_allocatorheap.h"
#include "stream_control_message.h"
#include "stream_macros.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#include "stream_lib_tools.h"

#include "stream_misc_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H

#include "test_i_common.h"
#include "test_i_defines.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
#include "test_i_common_modules.h"

#include "test_i_av_stream_common.h"

#include "test_i_av_stream_client_common.h"
#include "test_i_av_stream_client_eventhandler.h"
#include "test_i_av_stream_client_message.h"
#include "test_i_av_stream_client_session_message.h"
#include "test_i_av_stream_client_signalhandler.h"
#include "test_i_av_stream_client_stream.h"
#include "test_i_av_stream_stream_common.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("AVStream_Audio");
const char stream_name_string_2[] = ACE_TEXT_ALWAYS_CHAR ("AVStream_Video");
const char stream_name_string_3[] =
    ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING);

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c          : show console [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
#else
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-a [STRING] : audio device [\"")
            << ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_DEVICE_NAME)
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::string video_device_file = ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
  video_device_file += ACE_DIRECTORY_SEPARATOR_CHAR;
  video_device_file += ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-d [STRING] : video device [\"")
            << video_device_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e          : Gtk .rc file [\"\"] {\"\": no GUI}")
            << std::endl;
  // *TODO*: implement logic to query the hardware for potential formats and use
  //         the most applicable one.
  //         This implementation merely selects the 'first' of the available
  //         RGB-/Chroma-Luminance family of formats, which is non-portable
  //         behaviour: note how the receiver needs to know (or dynamically
  //         deduce) the correct transformation
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f          : send uncompressed video [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("])")
            << std::endl;
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_SOURCE_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\": no GUI}")
            << std::endl;
  std::string default_interface_string =
    Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
  ACE_ASSERT (!default_interface_string.empty ());
  ACE_INET_Addr interface_address, gateway_address;
  Net_Common_Tools::interfaceToIPAddress (default_interface_string,
                                          interface_address,
                                          gateway_address);
  std::string interface_address_string =
    Net_Common_Tools::IPAddressToString (interface_address, true, false);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-h [STRING] : target host [\"")
            << ACE_TEXT_ALWAYS_CHAR (interface_address_string.c_str ())
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-l          : log to a file [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-m          : use media foundation [")
            << (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
#endif // ACE_WIN32 || ACE_WIN64
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : port number [")
            << TEST_I_DEFAULT_PORT
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-r          : use reactor [")
            << (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR)
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-s [VALUE]  : statistic reporting interval (second(s)) [")
            << STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S
            << ACE_TEXT_ALWAYS_CHAR ("] [0: off]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-t          : trace information [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-u          : use UDP [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-v          : print version information and exit [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-x [VALUE]  : #dispatch threads [")
            << NET_CLIENT_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     bool& showConsole_out,
#else
                     struct Stream_Device_Identifier& audioDeviceIdentifier_out,
                     struct Stream_Device_Identifier& videoDeviceIdentifier_out,
#endif // ACE_WIN32 || ACE_WIN64
                     std::string& gtkRcFile_out,
                     bool& useUncompressedFormat_out,
                     std::string& gtkGladeFile_out,
                     std::string& hostName_out,
                     bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     enum Stream_MediaFramework_Type& mediaFramework_out,
#endif // ACE_WIN32 || ACE_WIN64
                     unsigned short& port_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& useUDP_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfProactorDispatchThreads_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();

  // initialize results
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  showConsole_out = false;
#else
  audioDeviceIdentifier_out.identifier =
    ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_DEVICE_NAME);
  videoDeviceIdentifier_out.identifier =
      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
  videoDeviceIdentifier_out.identifier += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  videoDeviceIdentifier_out.identifier +=
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  //gtkRcFile_out = path;
  //gtkRcFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //gtkRcFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  useUncompressedFormat_out = false;
  std::string default_interface_string =
    Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
  ACE_ASSERT (!default_interface_string.empty ());
  ACE_INET_Addr interface_address, gateway_address;
  Net_Common_Tools::interfaceToIPAddress (default_interface_string,
                                          interface_address, gateway_address);
  std::string interface_address_string =
    Net_Common_Tools::IPAddressToString (interface_address, true, false);
  hostName_out = interface_address_string;
  gtkGladeFile_out = path;
  gtkGladeFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtkGladeFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_SOURCE_GLADE_FILE);
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework_out = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  port_out = TEST_I_DEFAULT_PORT;
  useReactor_out =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  useUDP_out = false;
  printVersionAndExit_out = false;
  numberOfProactorDispatchThreads_out =
    NET_CLIENT_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("ce:fg::h:lmp:rs:tuvx:"),
#else
                              ACE_TEXT ("a:d:e:fg::h:lp:rs:tuvx:"),
#endif // ACE_WIN32 || ACE_WIN64
                              1,                          // skip command name
                              1,                          // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS,  // ordering
                              0);                         // for now, don't use long options

  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'c':
      {
        showConsole_out = true;
        break;
      }
#else
      case 'a':
      {
        audioDeviceIdentifier_out.identifier =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'd':
      {
        videoDeviceIdentifier_out.identifier =
          ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
      case 'e':
      {
        gtkRcFile_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'f':
      {
        useUncompressedFormat_out = true;
        break;
      }
      case 'g':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          gtkGladeFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          gtkGladeFile_out.clear ();
        break;
      }
      case 'h':
      {
        hostName_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'l':
      {
        logToFile_out = true;
        break;
      }
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      case 'm':
      {
        mediaFramework_out = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
        break;
      }
#endif // ACE_WIN32 || ACE_WIN64
      case 'p':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> port_out;
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
        useUDP_out = true;
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
        converter >> numberOfProactorDispatchThreads_out;
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

#ifdef ENABLE_VALGRIND_SUPPORT
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif
#endif
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
do_initialize_directshow (const struct Stream_Device_Identifier& deviceIdentifier_in,
                          bool hasUI_in,
                          IGraphBuilder*& IGraphBuilder_out,
                          IAMStreamConfig*& IAMStreamConfig_out,
                          struct _AMMediaType& captureAudioFormat_inout,
                          struct _AMMediaType& captureVideoFormat_inout,
                          struct _AMMediaType& outputVideoFormat_inout) // directshow sample grabber-
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  HRESULT result = E_FAIL;
  IAMBufferNegotiation* buffer_negotiation_p = NULL;
  Stream_MediaFramework_DirectShow_Graph_t graph_layout;
  Stream_MediaFramework_DirectShow_GraphConfiguration_t graph_configuration;
  BOOL result_2 = false;
  IMediaFilter* media_filter_p = NULL;
  struct _AMMediaType* media_type_p = NULL;

  // sanity check(s)
  ACE_ASSERT (!IGraphBuilder_out);
  ACE_ASSERT (!IAMStreamConfig_out);

  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

  if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (deviceIdentifier_in,
                                                        CLSID_VideoInputDeviceCategory,
                                                        IGraphBuilder_out,
                                                        buffer_negotiation_p,
                                                        IAMStreamConfig_out,
                                                        graph_layout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), aborting\n"),
                ACE_TEXT (deviceIdentifier_in.identifier._string)));
    goto error;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);
  ACE_ASSERT (buffer_negotiation_p);
  ACE_ASSERT (IAMStreamConfig_out);
  buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

  if (!Stream_Device_DirectShow_Tools::getCaptureFormat (IGraphBuilder_out,
                                                         CLSID_VideoInputDeviceCategory,
                                                         captureVideoFormat_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::getCaptureFormat(CLSID_VideoInputDeviceCategory), aborting\n")));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("\"%s\": default capture format: %s\n"),
              ACE_TEXT (Stream_Device_DirectShow_Tools::devicePathToString (deviceIdentifier_in.identifier._string).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (captureVideoFormat_inout, true).c_str ())));
  media_type_p =
    Stream_MediaFramework_DirectShow_Tools::copy (captureVideoFormat_inout);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  outputVideoFormat_inout = *media_type_p;
  delete (media_type_p); media_type_p = NULL;

  // *NOTE*: the default save format is RGB32
  ACE_ASSERT (InlineIsEqualGUID (outputVideoFormat_inout.majortype, MEDIATYPE_Video));
  outputVideoFormat_inout.subtype = MEDIASUBTYPE_RGB32;
  outputVideoFormat_inout.bFixedSizeSamples = TRUE;
  outputVideoFormat_inout.bTemporalCompression = FALSE;
  if (InlineIsEqualGUID (outputVideoFormat_inout.formattype, FORMAT_VideoInfo))
  { ACE_ASSERT (outputVideoFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
    struct tagVIDEOINFOHEADER* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (outputVideoFormat_inout.pbFormat);
    // *NOTE*: empty --> use entire video
    result_2 = SetRectEmpty (&video_info_header_p->rcSource);
    ACE_ASSERT (SUCCEEDED (result_2));
    result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
    // *NOTE*: empty --> fill entire buffer
    ACE_ASSERT (SUCCEEDED (result_2));
    ACE_ASSERT (video_info_header_p->dwBitErrorRate == 0);
    ACE_ASSERT (video_info_header_p->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
    ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
    video_info_header_p->bmiHeader.biBitCount = 32;
    video_info_header_p->bmiHeader.biCompression = BI_RGB;
    video_info_header_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header_p->bmiHeader);
    //GetBitmapSize (&video_info_header_p->bmiHeader);
    ////video_info_header_p->bmiHeader.biXPelsPerMeter;
    ////video_info_header_p->bmiHeader.biYPelsPerMeter;
    ////video_info_header_p->bmiHeader.biClrUsed;
    ////video_info_header_p->bmiHeader.biClrImportant;
    ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * 8) *                   // bits / frame
      (UNITS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps

    outputVideoFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else if (InlineIsEqualGUID (outputVideoFormat_inout.formattype, FORMAT_VideoInfo2))
  {
    ACE_ASSERT (outputVideoFormat_inout.cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
    struct tagVIDEOINFOHEADER2* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (outputVideoFormat_inout.pbFormat);
    ACE_ASSERT (video_info_header_p->bmiHeader.biSize == sizeof (struct tagBITMAPINFOHEADER));
    ACE_ASSERT (video_info_header_p->bmiHeader.biPlanes == 1);
    video_info_header_p->bmiHeader.biBitCount = 32;
    video_info_header_p->bmiHeader.biCompression = BI_RGB;
    video_info_header_p->bmiHeader.biSizeImage =
      DIBSIZE (video_info_header_p->bmiHeader);
    ////video_info_header_p->bmiHeader.biXPelsPerMeter;
    ////video_info_header_p->bmiHeader.biYPelsPerMeter;
    ////video_info_header_p->bmiHeader.biClrUsed;
    ////video_info_header_p->bmiHeader.biClrImportant;
    ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
    video_info_header_p->dwBitRate =
      (video_info_header_p->bmiHeader.biSizeImage * 8) *                   // bits / frame
      (UNITS / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)); // fps

    outputVideoFormat_inout.lSampleSize = video_info_header_p->bmiHeader.biSizeImage;
  } // end IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (outputVideoFormat_inout.formattype).c_str ())));
    goto error;
  } // end ELSE

  struct tWAVEFORMATEX waveformatex_s;
  ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
  waveformatex_s.wFormatTag = STREAM_DEV_MIC_DEFAULT_FORMAT;
  waveformatex_s.nChannels = STREAM_DEV_MIC_DEFAULT_CHANNELS;
  waveformatex_s.nSamplesPerSec = STREAM_DEV_MIC_DEFAULT_SAMPLE_RATE;
  waveformatex_s.wBitsPerSample = STREAM_DEV_MIC_DEFAULT_BITS_PER_SAMPLE;
  waveformatex_s.nBlockAlign =
    (waveformatex_s.nChannels * (waveformatex_s.wBitsPerSample / 8));
  waveformatex_s.nAvgBytesPerSec =
    (waveformatex_s.nSamplesPerSec * waveformatex_s.nBlockAlign);
  //waveformatex_s.cbSize = 0;
  result = CreateAudioMediaType (&waveformatex_s,
                                 &captureAudioFormat_inout,
                                 TRUE); // set format ?
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CreateAudioMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  if (hasUI_in)
  {
    IAMStreamConfig_out->Release (); IAMStreamConfig_out = NULL;
    IGraphBuilder_out->Release (); IGraphBuilder_out = NULL;
    return true;
  } // end IF

  if (!Stream_Module_Decoder_Tools::loadVideoRendererGraph (CLSID_VideoInputDeviceCategory,
                                                            captureVideoFormat_inout,
                                                            outputVideoFormat_inout,
                                                            NULL,
                                                            IGraphBuilder_out,
                                                            graph_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Decoder_Tools::loadVideoRendererGraph(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (IGraphBuilder_out);
  result = IGraphBuilder_out->QueryInterface (IID_PPV_ARGS (&media_filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_filter_p);
  result = media_filter_p->SetSyncSource (NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  media_filter_p->Release (); media_filter_p = NULL;

  return true;

error:
  if (media_type_p)
  {
    CoTaskMemFree (media_type_p); media_type_p = NULL;
  } // end IF
  if (media_filter_p)
    media_filter_p->Release ();
  if (buffer_negotiation_p)
    buffer_negotiation_p->Release ();
  Stream_MediaFramework_DirectShow_Tools::free (outputVideoFormat_inout);
  Stream_MediaFramework_DirectShow_Tools::free (captureVideoFormat_inout);
  Stream_MediaFramework_DirectShow_Tools::free (captureAudioFormat_inout);
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release (); IAMStreamConfig_out = NULL;
  } // end IF
  if (IGraphBuilder_out)
  {
    IGraphBuilder_out->Release (); IGraphBuilder_out = NULL;
  } // end IF

  return false;
}

bool
do_initialize_mediafoundation (bool useUncompressedFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_mediafoundation"));

  HRESULT result = E_FAIL;

  result = MFStartup (MF_VERSION,
                      MFSTARTUP_LITE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFStartup(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  return true;

error:
  result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  return false;
}

void
do_finalize_directshow (struct Test_I_AVStream_Client_DirectShow_UI_CBData& CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

  if (CBData_in.streamConfiguration)
  {
    CBData_in.streamConfiguration->Release (); CBData_in.streamConfiguration = NULL;
  } // end IF

  Test_I_AVStream_Client_DirectShow_StreamConfigurationsIterator_t iterator =
    CBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != CBData_in.configuration->streamConfigurations.end ());
  Test_I_AVStream_Client_DirectShow_StreamConfiguration_t::ITERATOR_T iterator_2 =
    (*iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator_2 != (*iterator).second.end ());
  if ((*iterator_2).second.second->builder)
  {
    (*iterator_2).second.second->builder->Release (); (*iterator_2).second.second->builder = NULL;
  } // end IF
}

void
do_finalize_mediafoundation ()
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_mediafoundation"));

  HRESULT result = E_FAIL;

  result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
}
#endif // ACE_WIN32 || ACE_WIN64

void
do_work (const struct Stream_Device_Identifier& audioDeviceIdentifier_in,
         const struct Stream_Device_Identifier& videoDeviceIdentifier_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         bool showConsole_in,
#endif // ACE_WIN32 || ACE_WIN64
         bool useUncompressedFormat_in,
         const std::string& UIDefinitionFilename_in,
         const std::string& hostName_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         enum Stream_MediaFramework_Type mediaFramework_in,
#endif // ACE_WIN32 || ACE_WIN64
         unsigned short port_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         bool useUDP_in,
         unsigned int numberOfProactorDispatchThreads_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Test_I_AVStream_Client_MediaFoundation_UI_CBData& mediaFoundationCBData_in,
         struct Test_I_AVStream_Client_DirectShow_UI_CBData& directShowCBData_in,
#else
         struct Test_I_AVStream_Client_ALSA_V4L_UI_CBData& CBData_in,
#endif // ACE_WIN32 || ACE_WIN64
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         const ACE_Sig_Set& previousSignalMask_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  ACE_UNUSED_ARG (useUncompressedFormat_in);

  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool is_running = false;
#endif

  // step0a: initialize event dispatch
  struct Test_I_AVStream_Configuration* camstream_configuration_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      camstream_configuration_p = directShowCBData_in.configuration;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      camstream_configuration_p = mediaFoundationCBData_in.configuration;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  camstream_configuration_p = CBData_in.configuration;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (camstream_configuration_p);
  if (useReactor_in)
    camstream_configuration_p->dispatchConfiguration.numberOfReactorThreads =
      TEST_I_DEFAULT_NUMBER_OF_CLIENT_DISPATCH_THREADS;
  else
    camstream_configuration_p->dispatchConfiguration.numberOfProactorThreads =
      numberOfProactorDispatchThreads_in;
  if (!Common_Event_Tools::initializeEventDispatch (camstream_configuration_p->dispatchConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
      &camstream_configuration_p->dispatchConfiguration;

  // step0b: initialize configuration and stream
  struct Common_AllocatorConfiguration* allocator_configuration_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      allocator_configuration_p =
        &directShowCBData_in.configuration->allocatorConfiguration;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      allocator_configuration_p =
        &mediaFoundationCBData_in.configuration->allocatorConfiguration;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  allocator_configuration_p = &CBData_in.configuration->allocatorConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (allocator_configuration_p);
  allocator_configuration_p->defaultBufferSize = 1 << 20; // 1Mb
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (*allocator_configuration_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize heap allocator, returning\n")));
    return;
  } // end IF
  Stream_IAllocator* allocator_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Client_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                                                     &heap_allocator,     // heap allocator handle
                                                                                     true);               // block ?
  Test_I_AVStream_Client_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                                                               &heap_allocator,     // heap allocator handle
                                                                                               true);               // block ?
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      allocator_p = &directshow_message_allocator;
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      allocator_p = &mediafoundation_message_allocator;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  Test_I_AVStream_Client_ALSA_V4L_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                                        &heap_allocator,     // heap allocator handle
                                                                        true);               // block ?
  allocator_p = &message_allocator;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (allocator_p);

  struct Stream_ModuleConfiguration module_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration;
  struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_2; // visualization
  struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_3; // network io
  struct Test_I_AVStream_Client_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration_4; // audio capture
  Test_I_AVStream_Client_DirectShow_StreamConfiguration_t directshow_stream_configuration;
  struct Test_I_AVStream_Client_DirectShow_StreamConfiguration directshow_stream_configuration_2;
  Test_I_AVStream_Client_DirectShow_StreamConfiguration_t directshow_stream_configuration_3;
  struct Test_I_AVStream_Client_DirectShow_StreamConfiguration directshow_stream_configuration_4;
  Test_I_AVStream_Client_DirectShow_StreamConfigurationsIterator_t directshow_stream_iterator;
  Test_I_AVStream_Client_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;

  struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration;
  struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_2; // visualization
  struct Test_I_AVStream_Client_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration_3; // network io
  Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t mediafoundation_stream_configuration;
  struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration mediafoundation_stream_configuration_2;
  Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t mediafoundation_stream_configuration_3;
  struct Test_I_AVStream_Client_MediaFoundation_StreamConfiguration mediafoundation_stream_configuration_4;

  Test_I_AVStream_Client_MediaFoundation_StreamConfigurationsIterator_t mediafoundation_stream_iterator;
  Test_I_AVStream_Client_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;

  Test_I_AVStream_Client_DirectShow_Streamer_Module directshow_streamer (directShowCBData_in.videoStream,
                                                                         ACE_TEXT_ALWAYS_CHAR ("Streamer"));
  Test_I_AVStream_Client_DirectShow_EventHandler_Module directshow_event_handler (directShowCBData_in.videoStream,
                                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_AVStream_Client_MediaFoundation_Streamer_Module mediafoundation_streamer ((useUDP_in ? mediaFoundationCBData_in.UDPStream : mediaFoundationCBData_in.videoStream),
                                                                                   ACE_TEXT_ALWAYS_CHAR ("Streamer"));
  Test_I_AVStream_Client_MediaFoundation_EventHandler_Module mediafoundation_event_handler ((useUDP_in ? mediaFoundationCBData_in.UDPStream : mediaFoundationCBData_in.videoStream),
                                                                                            ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_modulehandler_configuration.allocatorConfiguration =
        &directShowCBData_in.configuration->allocatorConfiguration;
      directshow_modulehandler_configuration.concurrency =
        STREAM_HEADMODULECONCURRENCY_ACTIVE;
      directshow_modulehandler_configuration.deviceIdentifier =
        videoDeviceIdentifier_in;

      directshow_modulehandler_configuration_2 =
        directshow_modulehandler_configuration;
      directshow_modulehandler_configuration_2.display =
        Common_UI_Tools::getDefaultDisplay ();

      directshow_modulehandler_configuration_4 =
        directshow_modulehandler_configuration;
      directshow_modulehandler_configuration_4.deviceIdentifier =
        audioDeviceIdentifier_in;

      directshow_stream_configuration_2.allocatorConfiguration = allocator_configuration_p;
      directshow_stream_configuration_2.messageAllocator = allocator_p;
      if (!UIDefinitionFilename_in.empty ())
        directshow_stream_configuration_2.module = &directshow_event_handler;
      directshow_stream_configuration_2.module_2 = &directshow_streamer;
      directshow_stream_configuration_2.printFinalReport = true;
      directshow_stream_configuration.initialize (module_configuration,
                                                  directshow_modulehandler_configuration,
                                                  directshow_stream_configuration_2);

      directshow_stream_configuration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DIRECTSHOW_DEFAULT_NAME_STRING),
                                                              std::make_pair (&module_configuration,
                                                                              &directshow_modulehandler_configuration_2)));
      directshow_stream_configuration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WASAPI_CAPTURE_DEFAULT_NAME_STRING),
                                                              std::make_pair (&module_configuration,
                                                                              &directshow_modulehandler_configuration_4)));

      directShowCBData_in.configuration->streamConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                                      directshow_stream_configuration));
      directshow_stream_iterator =
        directShowCBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_stream_iterator != directShowCBData_in.configuration->streamConfigurations.end ());

      directshow_modulehandler_configuration_3 =
        directshow_modulehandler_configuration;
      directshow_modulehandler_configuration_3.concurrency =
        STREAM_HEADMODULECONCURRENCY_CONCURRENT;
      directshow_modulehandler_configuration_3.inbound = false;

      directshow_stream_configuration_4 = directshow_stream_configuration_2;
      directshow_stream_configuration_4.module = NULL;
      directshow_stream_configuration_3.initialize (module_configuration,
                                                    directshow_modulehandler_configuration_3,
                                                    directshow_stream_configuration_4);
      directShowCBData_in.configuration->streamConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING),
                                                                                      directshow_stream_configuration_3));

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_modulehandler_configuration.allocatorConfiguration =
        allocator_configuration_p;
      mediafoundation_modulehandler_configuration.configuration =
        mediaFoundationCBData_in.configuration;
      mediafoundation_modulehandler_configuration.connectionConfigurations =
        &mediaFoundationCBData_in.configuration->connectionConfigurations;
      mediafoundation_modulehandler_configuration.deviceIdentifier =
        videoDeviceIdentifier_in;
      mediafoundation_modulehandler_configuration.statisticReportingInterval =
        ACE_Time_Value (statisticReportingInterval_in, 0);
      //(*mediafoundation_modulehandler_iterator).second.second->stream =
      //  ((mediafoundation_configuration.protocol == NET_TRANSPORTLAYER_TCP) ? mediaFoundationCBData_in.stream
      //                                                                      : mediaFoundationCBData_in.UDPStream);

      mediafoundation_modulehandler_configuration_2.display =
        Common_UI_Tools::getDefaultDisplay ();

      mediafoundation_stream_configuration_2.allocatorConfiguration = allocator_configuration_p;
      mediafoundation_stream_configuration_2.mediaFoundationConfiguration =
        &mediaFoundationCBData_in.configuration->mediaFoundationConfiguration;
      mediafoundation_stream_configuration_2.messageAllocator = allocator_p;
      if (!UIDefinitionFilename_in.empty ())
      {
        mediafoundation_stream_configuration_2.cloneModule = true;
        mediafoundation_stream_configuration_2.module = &mediafoundation_event_handler;
      } // end IF
      mediafoundation_stream_configuration_2.printFinalReport = true;
      mediafoundation_stream_configuration.initialize (module_configuration,
                                                       mediafoundation_modulehandler_configuration,
                                                       mediafoundation_stream_configuration_2);
      mediafoundation_modulehandler_configuration.deviceIdentifier.clear ();
      mediafoundation_modulehandler_configuration.direct3DConfiguration =
        &mediaFoundationCBData_in.configuration->direct3DConfiguration;

      mediafoundation_stream_configuration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_MEDIAFOUNDATION_DEFAULT_NAME_STRING),
                                                                   std::make_pair (&module_configuration,
                                                                                   &mediafoundation_modulehandler_configuration_2)));

      mediaFoundationCBData_in.configuration->streamConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                                           mediafoundation_stream_configuration));
      mediafoundation_stream_iterator =
        mediaFoundationCBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediaFoundationCBData_in.configuration->streamConfigurations.end ());

      mediafoundation_modulehandler_configuration_3 =
        mediafoundation_modulehandler_configuration;
      mediafoundation_modulehandler_configuration_3.concurrency =
        STREAM_HEADMODULECONCURRENCY_CONCURRENT;
      mediafoundation_modulehandler_configuration_3.inbound = false;

      mediafoundation_stream_configuration_4 = mediafoundation_stream_configuration_2;
      mediafoundation_stream_configuration_4.module = NULL;
      mediafoundation_stream_configuration_3.initialize (module_configuration,
                                                         mediafoundation_modulehandler_configuration_3,
                                                         mediafoundation_stream_configuration_4);
      mediaFoundationCBData_in.configuration->streamConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING),
                                                                                           mediafoundation_stream_configuration_3));
      //mediafoundation_stream_iterator =
      //  mediaFoundationCBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING));
      //ACE_ASSERT (mediafoundation_stream_iterator != mediaFoundationCBData_in.configuration->streamConfigurations.end ());

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  Test_I_AVStream_Client_ALSA_V4L_StreamConfigurationsIterator_t stream_iterator;
  Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t::ITERATOR_T modulehandler_iterator;

  Test_I_AVStream_Client_ALSA_V4L_Streamer_Module streamer_module (CBData_in.videoStream,
                                                                   ACE_TEXT_ALWAYS_CHAR ("Streamer"));

  Test_I_AVStream_Client_ALSA_V4L_EventHandler_t event_handler (&CBData_in);
  Test_I_AVStream_Client_ALSA_V4L_Module_EventHandler_Module event_handler_module (CBData_in.videoStream,
                                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));

  struct Stream_MediaFramework_ALSA_Configuration ALSA_configuration; // capture
  ALSA_configuration.asynch = false;
//  ALSA_configuration.asynch = STREAM_LIB_ALSA_CAPTURE_DEFAULT_ASYNCH;
  ALSA_configuration.bufferSize = STREAM_LIB_ALSA_CAPTURE_DEFAULT_BUFFER_SIZE;
  ALSA_configuration.bufferTime = STREAM_LIB_ALSA_CAPTURE_DEFAULT_BUFFER_TIME;
  ALSA_configuration.periods = STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIODS;
  ALSA_configuration.periodSize = STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIOD_SIZE;
  ALSA_configuration.periodTime = STREAM_LIB_ALSA_CAPTURE_DEFAULT_PERIOD_TIME;

  struct Stream_AllocatorConfiguration allocator_configuration;
  struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration modulehandler_configuration_2; // alsa source
  struct Test_I_AVStream_Client_ALSA_V4L_ModuleHandlerConfiguration modulehandler_configuration_3; // net io
  modulehandler_configuration.ALSAConfiguration = &ALSA_configuration;
  modulehandler_configuration.allocatorConfiguration = &allocator_configuration;
  modulehandler_configuration.concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  modulehandler_configuration.connectionConfigurations =
      &CBData_in.configuration->connectionConfigurations;
  modulehandler_configuration.deviceIdentifier = videoDeviceIdentifier_in;
  modulehandler_configuration.messageAllocator = &message_allocator;
  modulehandler_configuration.subscriber = &event_handler;

  struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration stream_configuration;
  struct Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration stream_configuration_2;
  Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t stream_configuration_3;
  Test_I_AVStream_Client_ALSA_V4L_StreamConfiguration_t stream_configuration_4;
  stream_configuration.allocatorConfiguration = &allocator_configuration;

  stream_configuration.format.audio.format = STREAM_LIB_ALSA_DEFAULT_FORMAT;
  stream_configuration.format.audio.rate = STREAM_LIB_ALSA_DEFAULT_SAMPLE_RATE;
  stream_configuration.format.audio.channels = STREAM_LIB_ALSA_DEFAULT_CHANNELS;

//  stream_configuration.format.format.pixelformat = V4L2_PIX_FMT_RGB24;
  stream_configuration.format.video.format.pixelformat = V4L2_PIX_FMT_YUYV;
  stream_configuration.format.video.format.width =
    STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH;
  stream_configuration.format.video.format.height =
    STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT;
  stream_configuration.format.video.frameRate.numerator =
    STREAM_DEV_CAM_DEFAULT_CAPTURE_FRAME_RATE;

  stream_configuration.messageAllocator = allocator_p;
  stream_configuration.module = &event_handler_module;
  stream_configuration.module_2 = &streamer_module;
  modulehandler_configuration.outputFormat.format =
    Stream_MediaFramework_Tools::v4lFormatToffmpegFormat (V4L2_PIX_FMT_RGB24);
  modulehandler_configuration.outputFormat.resolution.height =
    stream_configuration.format.video.format.height;
  modulehandler_configuration.outputFormat.resolution.width =
    stream_configuration.format.video.format.width;

  stream_configuration_3.initialize (module_configuration,
                                     modulehandler_configuration,
                                     stream_configuration);
  modulehandler_configuration_2 = modulehandler_configuration;
  modulehandler_configuration_2.deviceIdentifier = audioDeviceIdentifier_in;
  stream_configuration_3.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING),
                                                 std::make_pair (&module_configuration,
                                                                 &modulehandler_configuration_2)));
  CBData_in.configuration->streamConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                        stream_configuration_3));
  stream_iterator =
    CBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (stream_iterator != CBData_in.configuration->streamConfigurations.end ());
  modulehandler_iterator = (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());
  allocator_configuration_p = &allocator_configuration;

  modulehandler_configuration_3 = modulehandler_configuration;
  modulehandler_configuration_3.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration_3.inbound = false;

  stream_configuration_2.allocatorConfiguration = &allocator_configuration;
  stream_configuration_2.messageAllocator = allocator_p;
  stream_configuration_4.initialize (module_configuration,
                                     modulehandler_configuration_3,
                                     stream_configuration_2);
  CBData_in.configuration->streamConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING),
                                                            stream_configuration_4));
//  stream_iterator =
//    CBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING));
//  ACE_ASSERT (stream_iterator != CBData_in.configuration->streamConfigurations.end ());
#endif // ACE_WIN32 || ACE_WIN64
  camstream_configuration_p->protocol = (useUDP_in ? NET_TRANSPORTLAYER_UDP
                                                   : NET_TRANSPORTLAYER_TCP);

  result = false;
  if (useReactor_in)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (mediaFramework_in)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        ACE_NEW_NORETURN (directShowCBData_in.audioStream,
                          Test_I_AVStream_Client_DirectShow_Audio_Stream ());
        if (useUDP_in)
          ACE_NEW_NORETURN (directShowCBData_in.videoStream,
                            Test_I_AVStream_Client_DirectShow_UDPStream_t ());
        else
          ACE_NEW_NORETURN (directShowCBData_in.videoStream,
                            Test_I_AVStream_Client_DirectShow_TCPStream_t ());
        result =
          (directShowCBData_in.audioStream && directShowCBData_in.videoStream);
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        ACE_NEW_NORETURN (mediaFoundationCBData_in.videoStream,
                          Test_I_AVStream_Client_MediaFoundation_TCPStream_t ());
        ACE_NEW_NORETURN (mediaFoundationCBData_in.UDPStream,
                          Test_I_AVStream_Client_MediaFoundation_UDPStream_t ());
        result =
          (mediaFoundationCBData_in.audioStream && mediaFoundationCBData_in.videoStream &&
           mediaFoundationCBData_in.UDPStream);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    mediaFramework_in));
        return;
      }
    } // end SWITCH
#else
    ACE_NEW_NORETURN (CBData_in.audioStream,
                      Test_I_AVStream_Client_ALSA_Stream ());
    ACE_NEW_NORETURN (CBData_in.videoStream,
                      Test_I_AVStream_Client_V4L_TCPStream_t ());
    ACE_NEW_NORETURN (CBData_in.UDPStream,
                      Test_I_AVStream_Client_V4L_UDPStream_t ());
    result = (CBData_in.audioStream && CBData_in.videoStream &&
              CBData_in.UDPStream);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  else
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (mediaFramework_in)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        ACE_NEW_NORETURN (directShowCBData_in.audioStream,
                          Test_I_AVStream_Client_DirectShow_Audio_Stream ());
        if (useUDP_in)
          ACE_NEW_NORETURN (directShowCBData_in.videoStream,
                            Test_I_AVStream_Client_DirectShow_AsynchUDPStream_t ());
        else
          ACE_NEW_NORETURN (directShowCBData_in.videoStream,
                            Test_I_AVStream_Client_DirectShow_AsynchTCPStream_t ());
        result =
          (directShowCBData_in.audioStream && directShowCBData_in.videoStream);
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        ACE_NEW_NORETURN (mediaFoundationCBData_in.videoStream,
                          Test_I_AVStream_Client_MediaFoundation_AsynchTCPStream_t ());
        ACE_NEW_NORETURN (mediaFoundationCBData_in.UDPStream,
                          Test_I_AVStream_Client_MediaFoundation_AsynchUDPStream_t ());
        result =
          (mediaFoundationCBData_in.audioStream && mediaFoundationCBData_in.videoStream &&
           mediaFoundationCBData_in.UDPStream);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    mediaFramework_in));
        return;
      }
    } // end SWITCH
#else
    ACE_NEW_NORETURN (CBData_in.audioStream,
                      Test_I_AVStream_Client_ALSA_Stream ());
    ACE_NEW_NORETURN (CBData_in.videoStream,
                      Test_I_AVStream_Client_V4L_AsynchTCPStream_t ());
    ACE_NEW_NORETURN (CBData_in.UDPStream,
                      Test_I_AVStream_Client_V4L_AsynchUDPStream_t ());
    result =
        (CBData_in.audioStream && CBData_in.videoStream && CBData_in.UDPStream);
#endif // ACE_WIN32 || ACE_WIN64
  } // end ELSE
  if (!result)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));
    return;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_modulehandler_iterator =
        (*directshow_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != (*directshow_stream_iterator).second.end ());

      result =
        do_initialize_directshow (videoDeviceIdentifier_in,
                                  !UIDefinitionFilename_in.empty (), // has UI ?
                                  (*directshow_modulehandler_iterator).second.second->builder,
                                  directShowCBData_in.streamConfiguration,
                                  (*directshow_stream_iterator).second.configuration_->format.audio,
                                  (*directshow_stream_iterator).second.configuration_->format.video,
                                  (*directshow_modulehandler_iterator).second.second->outputFormat);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      result = do_initialize_mediafoundation (useUncompressedFormat_in);
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize media framework, returning\n")));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  int result_2 = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Client_DirectShow_EventHandler_t directshow_ui_event_handler (&directShowCBData_in);
  Test_I_AVStream_Client_MediaFoundation_EventHandler_t mediafoundation_ui_event_handler (&mediaFoundationCBData_in);
#else
  Test_I_AVStream_Client_ALSA_V4L_EventHandler_t ui_event_handler (&CBData_in);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Client_DirectShow_EventHandler* directshow_event_handler_p =
    NULL;
  Test_I_AVStream_Client_MediaFoundation_EventHandler* mediafoundation_event_handler_p =
    NULL;
#else
//  Test_I_AVStream_Client_ALSA_V4L_Module_EventHandler* module_event_handler_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_TimerConfiguration timer_configuration;
  timer_configuration.dispatch =
    (useReactor_in ? COMMON_TIMER_DISPATCH_REACTOR
                   : COMMON_TIMER_DISPATCH_PROACTOR);
  Common_Timer_Manager_t* timer_manager_p =
      COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  long timer_id = -1;
//  int group_id = -1;
  Net_IConnectionManagerBase* iconnection_manager_p = NULL;
//  Net_IStreamStatisticHandler_t* report_handler_p = NULL;
  //Test_I_AVStream_Client_Stream_IStatistic_t stream_report_handler;
  Stream_IStreamControlBase* stream_p = NULL, *stream_2 = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
  Common_UI_GTK_State_t* ui_state_p = NULL;
#endif // GTK_USE
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Client_MediaFoundation_TCPConnectionConfiguration_t mediafoundation_tcp_connection_configuration;
  Test_I_AVStream_Client_MediaFoundation_UDPConnectionConfiguration_t mediafoundation_udp_connection_configuration;
  Test_I_AVStream_Client_DirectShow_TCPConnectionConfiguration_t directshow_tcp_connection_configuration;
  Test_I_AVStream_Client_DirectShow_UDPConnectionConfiguration_t directshow_udp_connection_configuration;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_stream_iterator =
        directShowCBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING));
      ACE_ASSERT (directshow_stream_iterator != directShowCBData_in.configuration->streamConfigurations.end ());

      directshow_tcp_connection_configuration.allocatorConfiguration = allocator_configuration_p;
      directshow_tcp_connection_configuration.streamConfiguration =
        &(*directshow_stream_iterator).second;
      directShowCBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("TCP"),
                                                                                          &directshow_tcp_connection_configuration));
      directshow_udp_connection_configuration.allocatorConfiguration = allocator_configuration_p;
      directshow_udp_connection_configuration.streamConfiguration =
        &(*directshow_stream_iterator).second;
      directshow_udp_connection_configuration.delayRead = true;
      directshow_udp_connection_configuration.socketConfiguration.writeOnly =
        true;
      directShowCBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("UDP"),
                                                                                          &directshow_udp_connection_configuration));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_stream_iterator =
        mediaFoundationCBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING));
      ACE_ASSERT (mediafoundation_stream_iterator != mediaFoundationCBData_in.configuration->streamConfigurations.end ());

      mediafoundation_tcp_connection_configuration.allocatorConfiguration = allocator_configuration_p;
      mediafoundation_tcp_connection_configuration.streamConfiguration =
        &(*mediafoundation_stream_iterator).second;
      mediaFoundationCBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("TCP"),
                                                                                               &mediafoundation_tcp_connection_configuration));
      mediaFoundationCBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("UDP"),
                                                                                               &mediafoundation_udp_connection_configuration));

      mediafoundation_stream_iterator =
        mediaFoundationCBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_stream_iterator != mediaFoundationCBData_in.configuration->streamConfigurations.end ());

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t connection_configuration;
  connection_configuration.allocatorConfiguration = &allocator_configuration;
//  connection_configuration.allocatorConfiguration->defaultBufferSize = bufferSize_in;

//  stream_iterator =
//    CBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING));
//  ACE_ASSERT (stream_iterator != CBData_in.configuration->streamConfigurations.end ());
//  stream_iterator =
//    CBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
//  ACE_ASSERT (stream_iterator != CBData_in.configuration->streamConfigurations.end ());

//  result =
//    connection_configuration.initialize ((*stream_iterator).second);
  ACE_ASSERT (result);
  CBData_in.configuration->connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (""),
                                                                &connection_configuration));
#endif // ACE_WIN32 || ACE_WIN64

  struct Net_UserData user_data_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Client_MediaFoundation_TCPConnectionManager_t* mediafoundation_tcp_connection_manager_p =
    NULL;
  Test_I_AVStream_Client_MediaFoundation_UDPConnectionManager_t* mediafoundation_udp_connection_manager_p =
    NULL;
  Test_I_AVStream_Client_DirectShow_TCPConnectionManager_t* directshow_tcp_connection_manager_p =
    NULL;
  Test_I_AVStream_Client_DirectShow_UDPConnectionManager_t* directshow_udp_connection_manager_p =
    NULL;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_tcp_connection_manager_p =
        TEST_I_AVSTREAM_CLIENT_DIRECTSHOW_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
      ACE_ASSERT (directshow_tcp_connection_manager_p);
      directshow_tcp_connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                                       ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
      directshow_tcp_connection_manager_p->set (directshow_tcp_connection_configuration,
                                                &user_data_s);
      directshow_udp_connection_manager_p =
        TEST_I_AVSTREAM_CLIENT_DIRECTSHOW_UDP_CONNECTIONMANAGER_SINGLETON::instance ();
      ACE_ASSERT (directshow_udp_connection_manager_p);
      directshow_udp_connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                                       ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
      directshow_udp_connection_manager_p->set (directshow_udp_connection_configuration,
                                                &user_data_s);

      if (useUDP_in)
      {
        iconnection_manager_p = directshow_udp_connection_manager_p;
        directshow_modulehandler_configuration.connectionConfigurationName =
          ACE_TEXT_ALWAYS_CHAR ("UDP");
      } // end IF
      else
      {
        iconnection_manager_p = directshow_tcp_connection_manager_p;
        directshow_modulehandler_configuration.connectionConfigurationName =
          ACE_TEXT_ALWAYS_CHAR ("TCP");
      } // end ELSE
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_tcp_connection_manager_p =
        TEST_I_AVSTREAM_CLIENT_MEDIAFOUNDATION_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
      ACE_ASSERT (mediafoundation_tcp_connection_manager_p);
      mediafoundation_tcp_connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                                            ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
      mediafoundation_tcp_connection_manager_p->set (mediafoundation_tcp_connection_configuration,
                                                     &user_data_s);

      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());

      (*mediafoundation_modulehandler_iterator).second.second->connectionManager =
        mediafoundation_tcp_connection_manager_p;
      iconnection_manager_p = mediafoundation_tcp_connection_manager_p;
      //report_handler_p = mediafoundation_tcp_connection_manager_p;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  Net_ConnectionConfigurationsIterator_t connection_iterator =
    CBData_in.configuration->connectionConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (connection_iterator != CBData_in.configuration->connectionConfigurations.end ());
  Test_I_AVStream_Client_ALSA_V4L_TCPConnectionManager_t* connection_manager_p =
    TEST_I_AVSTREAM_CLIENT_ALSA_V4L_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  connection_manager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                    ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  connection_manager_p->set (*static_cast<Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t*> ((*connection_iterator).second),
                             &user_data_s);
  (*modulehandler_iterator).second.second->connectionManager =
    connection_manager_p;
  iconnection_manager_p = connection_manager_p;
//  report_handler_p = connection_manager_p;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (iconnection_manager_p);
//  ACE_ASSERT (report_handler_p);
  Net_StatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                            NULL,
                                            false);
  ACE_Event_Handler* event_handler_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Client_DirectShow_SignalHandler_t directshow_signal_handler;
  Test_I_AVStream_Client_MediaFoundation_SignalHandler_t mediafoundation_signal_handler;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      directshow_event_handler_p =
        dynamic_cast<Test_I_AVStream_Client_DirectShow_EventHandler*> (directshow_event_handler.writer ());
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      mediafoundation_event_handler_p =
        dynamic_cast<Test_I_AVStream_Client_MediaFoundation_EventHandler*> (mediafoundation_event_handler.writer ());
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  Test_I_AVStream_Client_ALSA_V4L_SignalHandler_t signal_handler;
  event_handler_p =
    dynamic_cast<Test_I_AVStream_Client_ALSA_V4L_Module_EventHandler*> (event_handler_module.writer ());
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result_2 =
        directshow_tcp_connection_configuration.socketConfiguration.address.set (port_in,
                                                                                 hostName_in.c_str (),
                                                                                 1,
                                                                                 AF_INET);
      if (result_2 == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                    ACE_TEXT (hostName_in.c_str ()),
                    port_in));
        goto clean;
      } // end IF
      directshow_tcp_connection_configuration.socketConfiguration.useLoopBackDevice =
        directshow_tcp_connection_configuration.socketConfiguration.address.is_loopback ();
      directshow_tcp_connection_configuration.statisticReportingInterval =
        statisticReportingInterval_in;
      directshow_tcp_connection_configuration.messageAllocator =
        &directshow_message_allocator;

      result_2 =
        directshow_udp_connection_configuration.socketConfiguration.peerAddress.set (port_in,
                                                                                     hostName_in.c_str (),
                                                                                     1,
                                                                                     AF_INET);
      if (result_2 == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                    ACE_TEXT (hostName_in.c_str ()),
                    port_in));
        goto clean;
      } // end IF
      directshow_udp_connection_configuration.socketConfiguration.useLoopBackDevice =
        directshow_udp_connection_configuration.socketConfiguration.peerAddress.is_loopback ();
      directshow_udp_connection_configuration.statisticReportingInterval =
        statisticReportingInterval_in;
      directshow_udp_connection_configuration.messageAllocator =
        &directshow_message_allocator;

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // *************************** media foundation ****************************
      mediaFoundationCBData_in.configuration->mediaFoundationConfiguration.controller =
        ((mediaFoundationCBData_in.configuration->protocol == NET_TRANSPORTLAYER_TCP) ? mediaFoundationCBData_in.videoStream
                                                                                      : mediaFoundationCBData_in.UDPStream);

      result_2 =
        mediafoundation_tcp_connection_configuration.socketConfiguration.address.set (port_in,
                                                                                      hostName_in.c_str (),
                                                                                      1,
                                                                                      AF_INET);
      if (result_2 == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                    ACE_TEXT (hostName_in.c_str ()),
                    port_in));
        goto clean;
      } // end IF
      mediafoundation_tcp_connection_configuration.socketConfiguration.useLoopBackDevice =
        mediafoundation_tcp_connection_configuration.socketConfiguration.address.is_loopback ();
      mediafoundation_tcp_connection_configuration.statisticReportingInterval =
        statisticReportingInterval_in;
      mediafoundation_tcp_connection_configuration.messageAllocator =
        &mediafoundation_message_allocator;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  result_2 =
    NET_CONFIGURATION_TCP_CAST ((*connection_iterator).second)->socketConfiguration.address.set (port_in,
                                                                                                 hostName_in.c_str (),
                                                                                                 1,
                                                                                                 AF_INET);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s:%u\"): \"%m\", returning\n"),
                ACE_TEXT (hostName_in.c_str ()),
                port_in));
    goto clean;
  } // end IF
//  (*connection_iterator).second->bufferSize = static_cast<int> (bufferSize_in);
  NET_CONFIGURATION_TCP_CAST ((*connection_iterator).second)->socketConfiguration.useLoopBackDevice =
      NET_CONFIGURATION_TCP_CAST ((*connection_iterator).second)->socketConfiguration.address.is_loopback ();
//  (*connection_iterator).second.writeOnly = true;
  (*connection_iterator).second->statisticReportingInterval =
    statisticReportingInterval_in;
  static_cast<Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t*> ((*connection_iterator).second)->messageAllocator =
      &message_allocator;
  static_cast<Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t*> ((*connection_iterator).second)->streamConfiguration =
      &stream_configuration_4;

  connection_manager_p->set (*static_cast<Test_I_AVStream_Client_ALSA_V4L_TCPConnectionConfiguration_t*> ((*connection_iterator).second),
                             &user_data_s);
#endif // ACE_WIN32 || ACE_WIN64
  // ********************** module configuration data **************************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_iterator).second.second->allocatorConfiguration =
        allocator_configuration_p;
      (*directshow_modulehandler_iterator).second.second->configuration =
        directShowCBData_in.configuration;
      (*directshow_modulehandler_iterator).second.second->connectionConfigurations =
        &directShowCBData_in.configuration->connectionConfigurations;
      (*directshow_modulehandler_iterator).second.second->direct3DConfiguration =
        &directShowCBData_in.configuration->direct3DConfiguration;
      (*directshow_modulehandler_iterator).second.second->statisticReportingInterval =
        ACE_Time_Value (statisticReportingInterval_in, 0);
      //(*directshow_modulehandler_iterator).second.second->stream =
      //  ((directshow_configuration.protocol == NET_TRANSPORTLAYER_TCP) ? directShowCBData_in.stream
      //                                                                 : directShowCBData_in.UDPStream);
      (*directshow_modulehandler_iterator).second.second->subscriber =
        &directshow_ui_event_handler;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_modulehandler_iterator =
        (*mediafoundation_stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != (*mediafoundation_stream_iterator).second.end ());

      //(*mediafoundation_modulehandler_iterator).second.second->direct3DConfiguration =
      //  &mediaFoundationCBData_in.configuration->direct3DConfiguration;
      (*mediafoundation_modulehandler_iterator).second.second->subscriber =
        &mediafoundation_ui_event_handler;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  modulehandler_iterator =
    (*stream_iterator).second.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_iterator != (*stream_iterator).second.end ());

  (*modulehandler_iterator).second.second->connectionManager =
    connection_manager_p;
  (*modulehandler_iterator).second.second->configuration = CBData_in.configuration;
  (*modulehandler_iterator).second.second->connectionConfigurations =
      &CBData_in.configuration->connectionConfigurations;
  (*modulehandler_iterator).second.second->statisticReportingInterval =
    ACE_Time_Value (statisticReportingInterval_in, 0);
  //(*modulehandler_iterator).second.second->stream =
  //    ((V4L_configuration.protocol == NET_TRANSPORTLAYER_TCP) ? CBData_in.stream
  //                                                             : CBData_in.UDPStream);
//  (*modulehandler_iterator).second.subscriber = &ui_event_handler;

//  (*modulehandler_iterator).second.second->deviceIdentifier.identifier =
//      interfaceIdentifier_in;
  // *TODO*: turn these into an option
  (*modulehandler_iterator).second.second->buffers =
      STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  ui_state_p = &const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  (*modulehandler_iterator).second.second->pixelBufferLock =
      &ui_state_p->lock;
  CBData_in.UIState = ui_state_p;
#endif // GTK_USE
#endif // GUI_SUPPORT

  (*modulehandler_iterator).second.second->method =
      V4L2_MEMORY_USERPTR;
  (*modulehandler_iterator).second.second->streamConfiguration =
      &(*stream_iterator).second;
#endif // ACE_WIN32 || ACE_WIN64

  // step0d: initialize regular (global) statistic reporting
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);
  if (statisticReportingInterval_in)
  {
    ACE_Time_Value interval (statisticReportingInterval_in, 0);
    timer_id =
      timer_manager_p->schedule_timer (&statistic_handler,         // event handler handle
                                       NULL,                       // asynchronous completion token
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (timer_id == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));
      goto clean;
    } // end IF
  } // end IF

  // step0e: initialize signal handling
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directShowCBData_in.configuration->signalHandlerConfiguration.connectionManager =
        TEST_I_AVSTREAM_CLIENT_DIRECTSHOW_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
      directShowCBData_in.configuration->signalHandlerConfiguration.dispatchState =
        &event_dispatch_state_s;
      directShowCBData_in.configuration->signalHandlerConfiguration.audioStream =
        directShowCBData_in.audioStream;
      directShowCBData_in.configuration->signalHandlerConfiguration.videoStream =
        directShowCBData_in.videoStream;
      result =
        directshow_signal_handler.initialize (directShowCBData_in.configuration->signalHandlerConfiguration);
      event_handler_p = &directshow_signal_handler;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediaFoundationCBData_in.configuration->signalHandlerConfiguration.connectionManager =
        TEST_I_AVSTREAM_CLIENT_MEDIAFOUNDATION_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
      mediaFoundationCBData_in.configuration->signalHandlerConfiguration.dispatchState =
        &event_dispatch_state_s;
      mediaFoundationCBData_in.configuration->signalHandlerConfiguration.audioStream =
        mediaFoundationCBData_in.audioStream;
      mediaFoundationCBData_in.configuration->signalHandlerConfiguration.videoStream =
        mediaFoundationCBData_in.videoStream;
      result =
        mediafoundation_signal_handler.initialize (mediaFoundationCBData_in.configuration->signalHandlerConfiguration);
      event_handler_p = &mediafoundation_signal_handler;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  CBData_in.configuration->signalHandlerConfiguration.connectionManager =
    TEST_I_AVSTREAM_CLIENT_ALSA_V4L_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
  CBData_in.configuration->signalHandlerConfiguration.dispatchState =
      &event_dispatch_state_s;
  CBData_in.configuration->signalHandlerConfiguration.audioStream =
      CBData_in.audioStream;
  CBData_in.configuration->signalHandlerConfiguration.videoStream =
      CBData_in.videoStream;
  result =
    signal_handler.initialize (CBData_in.configuration->signalHandlerConfiguration);
  event_handler_p = &signal_handler;
#endif // ACE_WIN32 || ACE_WIN64
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    goto clean;
  } // end IF
  ACE_ASSERT (event_handler_p);
  if (!Common_Signal_Tools::initialize ((useReactor_in ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                       : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        event_handler_p,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), aborting\n")));
    goto clean;
  } // end IF

  // step1: handle events (signals, incoming connections/data, timers, ...)
  // reactor/proactor event loop:
  // - dispatch connection attempts to acceptor
  // - dispatch socket events
  // timer events:
  // - perform statistic collecting/reporting
  // [GTK events:]
  // - dispatch UI events (if any)

  // step1a: start GTK event loop ?
  if (!UIDefinitionFilename_in.empty ())
  {
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
    gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    ui_state_p = &const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    ACE_ASSERT (ui_state_p);
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFile_in, static_cast<GladeXML*> (NULL));
    ui_state_p->builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFilename_in, static_cast<GtkBuilder*> (NULL));

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
#endif // GUI_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
      goto clean;
    } // end IF
    BOOL was_visible_b = false;
    if (!showConsole_in)
      was_visible_b = ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  // step1b: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));
    goto clean;
  } // end IF

  if (UIDefinitionFilename_in.empty ())
  {
    result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (mediaFramework_in)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        directshow_stream_iterator =
          directShowCBData_in.configuration->streamConfigurations.find (ACE_TEXT_ALWAYS_CHAR (""));
        ACE_ASSERT (directshow_stream_iterator != directShowCBData_in.configuration->streamConfigurations.end ());

        stream_p = directShowCBData_in.audioStream;
        stream_2 = directShowCBData_in.videoStream;

        result =
          directShowCBData_in.audioStream->initialize ((*directshow_stream_iterator).second);
        result &=
          directShowCBData_in.videoStream->initialize ((*directshow_stream_iterator).second);
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        if (mediaFoundationCBData_in.configuration->protocol == NET_TRANSPORTLAYER_TCP)
        {
          stream_p = mediaFoundationCBData_in.audioStream;
          stream_2 = mediaFoundationCBData_in.videoStream;

          result =
            mediaFoundationCBData_in.audioStream->initialize ((*mediafoundation_stream_iterator).second);
          result =
            mediaFoundationCBData_in.videoStream->initialize ((*mediafoundation_stream_iterator).second);
        } // end IF
        else
        {
          stream_p = mediaFoundationCBData_in.UDPStream;
          result =
            mediaFoundationCBData_in.UDPStream->initialize ((*mediafoundation_stream_iterator).second);
        } // end ELSE
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    mediaFramework_in));
        return;
      }
    } // end SWITCH
#else
    if (CBData_in.configuration->protocol == NET_TRANSPORTLAYER_TCP)
    {
      stream_p = CBData_in.audioStream;
      stream_2 = CBData_in.videoStream;

      result = CBData_in.audioStream->initialize ((*stream_iterator).second);
      result &= CBData_in.videoStream->initialize ((*stream_iterator).second);
    } // end IF
    else
    {
      stream_p = CBData_in.UDPStream;
      result = CBData_in.UDPStream->initialize ((*stream_iterator).second);
    } // end ELSE
#endif // ACE_WIN32 || ACE_WIN64
    if (!result)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize stream, aborting\n")));

      // clean up
      Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                 true); // wait ?

      goto clean;
    } // end IF
    ACE_ASSERT (stream_p && stream_2);

    stream_p->start ();
    stream_2->start ();

    stream_p->wait (true, false, false);
    stream_2->wait (true, false, false);

    // clean up
  //    connection_manager_p->stop ();
  } // end IF
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  else
    gtk_manager_p->wait (false);
#endif // GTK_USE
#endif // GUI_SUPPORT

//    connection_manager_p->abort ();
  iconnection_manager_p->wait ();
  Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                             true); // wait ?

  // step3: clean up
clean:
//  if (!UIDefinitionFilename_in.empty ())
//    Test_I_AVStream_Client_GTK_MANAGER_SINGLETON::instance ()->stop ();
  timer_manager_p->stop (true, true);

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

  //result = event_handler.close (ACE_Module_Base::M_DELETE_NONE);
  //if (result == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
  //              event_handler.name ()));

  Common_Signal_Tools::finalize ((useReactor_in ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previousSignalActions_inout,
                                 previousSignalMask_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directShowCBData_in.audioStream->remove (&directshow_streamer, true, true);
      directShowCBData_in.videoStream->remove (&directshow_streamer, true, true);
      delete directShowCBData_in.audioStream; directShowCBData_in.audioStream = NULL;
      delete directShowCBData_in.videoStream; directShowCBData_in.videoStream = NULL;
      do_finalize_directshow (directShowCBData_in);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediaFoundationCBData_in.audioStream->remove (&mediafoundation_streamer, true, true);
      mediaFoundationCBData_in.videoStream->remove (&mediafoundation_streamer, true, true);
      delete mediaFoundationCBData_in.audioStream; mediaFoundationCBData_in.audioStream = NULL;
      delete mediaFoundationCBData_in.videoStream; mediaFoundationCBData_in.videoStream = NULL;
      delete mediaFoundationCBData_in.UDPStream; mediaFoundationCBData_in.UDPStream = NULL;
      do_finalize_mediafoundation ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    }
  } // end SWITCH
#else
  CBData_in.audioStream->remove (&streamer_module, true, true);
  CBData_in.videoStream->remove (&streamer_module, true, true);
  delete CBData_in.audioStream; CBData_in.audioStream = NULL;
  delete CBData_in.videoStream; CBData_in.videoStream = NULL;
  delete CBData_in.UDPStream; CBData_in.UDPStream = NULL;
#endif // ACE_WIN32 || ACE_WIN64

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

void
do_printVersion (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printVersion"));

  std::ostringstream converter;

  // compiler version string
  converter << ACE::compiler_major_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::compiler_minor_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::compiler_beta_version ();

  std::cout << programName_in
            << ACE_TEXT_ALWAYS_CHAR (" compiled on ")
            << ACE::compiler_name ()
            << ACE_TEXT_ALWAYS_CHAR (" ")
            << converter.str ()
            << std::endl << std::endl;

  std::cout << ACE_TEXT_ALWAYS_CHAR ("libraries: ")
            << std::endl
#if defined (HAVE_CONFIG_H)
            << ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_NAME)
            << ACE_TEXT_ALWAYS_CHAR (": ")
            << ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION)
            << std::endl
#endif // HAVE_CONFIG_H
            ;

  converter.str ("");
  // ACE version string
  converter << ACE::major_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::minor_version ();
  converter << ACE_TEXT_ALWAYS_CHAR (".");
  converter << ACE::beta_version ();
  // *NOTE*: cannot use ACE_VERSION, as it doesn't contain the (potential) beta
  // version number (the library soname is compared to this string)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("ACE: ")
//             << ACE_VERSION
            << converter.str ()
            << std::endl;
}

#undef main
int
ACE_TMAIN (int argc_in,
           ACE_TCHAR* argv_in[])
{
  STREAM_TRACE (ACE_TEXT ("::main"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result = -1;
#endif // ACE_WIN32 || ACE_WIN64

  // step0: initialize
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *PORTABILITY*: on Windows, initialize ACE...
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (true,   // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64

  std::string configuration_path = Common_File_Tools::getWorkingDirectory ();

  // step1a set defaults
  struct Stream_Device_Identifier audio_device_identifier;
  struct Stream_Device_Identifier video_device_identifier;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool show_console = false;
#else
  audio_device_identifier.identifier =
      ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_DEVICE_NAME);
  video_device_identifier.identifier =
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
  video_device_identifier.identifier += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  video_device_identifier.identifier +=
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);
#endif // ACE_WIN32 || ACE_WIN64
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string gtk_rc_filename;
  //std::string gtk_rc_filename = path;
  //gtk_rc_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  //gtk_rc_filename += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  bool use_uncompressed_format = false;
  std::string gtk_glade_filename = path;
  gtk_glade_filename += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_glade_filename +=
    ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_SOURCE_GLADE_FILE);
  std::string default_interface_string =
    Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
  ACE_ASSERT (!default_interface_string.empty ());
  ACE_INET_Addr interface_address, gateway_address;
  Net_Common_Tools::interfaceToIPAddress (default_interface_string,
                                          interface_address, gateway_address);
  std::string interface_address_string =
    Net_Common_Tools::IPAddressToString (interface_address, true, false);
  std::string host_name = interface_address_string;
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type media_framework_e =
    STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  unsigned short port = TEST_I_DEFAULT_PORT;
  bool use_reactor =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  bool use_UDP = false;
  bool print_version_and_exit = false;
  unsigned int number_of_proactor_dispatch_threads =
    NET_CLIENT_DEFAULT_NUMBER_OF_PROACTOR_DISPATCH_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            show_console,
#else
                            audio_device_identifier,
                            video_device_identifier,
#endif // ACE_WIN32 || ACE_WIN64
                            gtk_rc_filename,
                            use_uncompressed_format,
                            gtk_glade_filename,
                            host_name,
                            log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
                            port,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            use_UDP,
                            print_version_and_exit,
                            number_of_proactor_dispatch_threads))
  {
    do_printUsage (ACE::basename (argv_in[0]));
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
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
  if (TEST_I_MAX_MESSAGES)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("limiting the number of message buffers could (!) lead to deadlocks --> make sure you know what you are doing...\n")));
//  if ((event_dispatch_type_e == COMMON_EVENT_DISPATCH_REACTOR) &&
//      (number_of_dispatch_threads > 1) && !use_thread_pool)
//  { // *NOTE*: see also: man (2) select
//    // *TODO*: verify this for MS Windows based systems
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("the select()-based reactor is not reentrant, using the thread-pool reactor instead...\n")));
//    use_thread_pool = true;
//  } // end IF
  if ((!gtk_glade_filename.empty () &&
       !Common_File_Tools::isReadable (gtk_glade_filename))                         ||
      (!gtk_rc_filename.empty () &&
       !Common_File_Tools::isReadable (gtk_rc_filename))
//      ((event_dispatch_type_e == COMMON_EVENT_DISPATCH_REACTOR) && (number_of_dispatch_threads > 1) && !use_thread_pool)
      )
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid arguments, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

#if defined (GUI_SUPPORT)
//  struct Common_UI_CBData* ui_cb_data_p = NULL;
#if defined (GTK_USE)
  Common_UI_GTK_Configuration_t* gtk_configuration_p = NULL;
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  bool result_2 = false;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_AVStream_Client_DirectShow_UI_CBData directshow_ui_cb_data;
  struct Test_I_AVStream_Client_MediaFoundation_UI_CBData mediafoundation_ui_cb_data;
  struct Test_I_AVStream_Client_DirectShow_Configuration directshow_configuration;
  struct Test_I_AVStream_Client_MediaFoundation_Configuration mediafoundation_configuration;
  Stream_MediaFramework_Tools::initialize (media_framework_e);
  switch (media_framework_e)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      audio_device_identifier =
        Stream_Device_DirectShow_Tools::getDefaultCaptureDevice (CLSID_AudioInputDeviceCategory);
      video_device_identifier =
        Stream_Device_DirectShow_Tools::getDefaultCaptureDevice (CLSID_VideoInputDeviceCategory);
      directshow_ui_cb_data.mediaFramework = media_framework_e;
      //directshow_ui_cb_data.progressData.state =
      //  &directshow_ui_cb_data;
      //ui_cb_data_p = &directshow_ui_cb_data;
      directshow_ui_cb_data.configuration = &directshow_configuration;
#if defined (GTK_USE)
      directshow_ui_cb_data.UIState = &state_r;
      directshow_configuration.GTKConfiguration.argc = argc_in;
      directshow_configuration.GTKConfiguration.argv = argv_in;
      directshow_configuration.GTKConfiguration.CBData = &directshow_ui_cb_data;
      directshow_configuration.GTKConfiguration.eventHooks.finiHook =
        idle_finalize_source_UI_cb;
      directshow_configuration.GTKConfiguration.eventHooks.initHook =
        idle_initialize_source_UI_cb;
      directshow_configuration.GTKConfiguration.definition =
        &gtk_ui_definition;
      if (!gtk_rc_filename.empty ())
        directshow_configuration.GTKConfiguration.RCFiles.push_back (gtk_rc_filename);
      gtk_configuration_p = &directshow_configuration.GTKConfiguration;
#endif // GTK_USE
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      audio_device_identifier =
        Stream_Device_MediaFoundation_Tools::getDefaultCaptureDevice (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
      video_device_identifier =
        Stream_Device_MediaFoundation_Tools::getDefaultCaptureDevice (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
      mediafoundation_ui_cb_data.mediaFramework = media_framework_e;
      //mediafoundation_ui_cb_data.progressData.state =
      //  &mediafoundation_ui_cb_data;
      //ui_cb_data_p = &mediafoundation_ui_cb_data;
      mediafoundation_ui_cb_data.configuration = &mediafoundation_configuration;
#if defined (GTK_USE)
      mediafoundation_ui_cb_data.UIState = &state_r;
      mediafoundation_configuration.GTKConfiguration.argc = argc_in;
      mediafoundation_configuration.GTKConfiguration.argv = argv_in;
      mediafoundation_configuration.GTKConfiguration.CBData = &mediafoundation_ui_cb_data;
      mediafoundation_configuration.GTKConfiguration.eventHooks.finiHook =
        idle_finalize_source_UI_cb;
      mediafoundation_configuration.GTKConfiguration.eventHooks.initHook =
        idle_initialize_source_UI_cb;
      mediafoundation_configuration.GTKConfiguration.definition =
        &gtk_ui_definition;
      if (!gtk_rc_filename.empty ())
        mediafoundation_configuration.GTKConfiguration.RCFiles.push_back (gtk_rc_filename);
      gtk_configuration_p = &mediafoundation_configuration.GTKConfiguration;
#endif // GTK_USE
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  media_framework_e));

      Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // *PORTABILITY*: on Windows, finalize ACE...
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
      return EXIT_FAILURE;
    }
  } // end SWITCH
#else
  struct Test_I_AVStream_Client_ALSA_V4L_UI_CBData ui_cb_data;
  struct Test_I_AVStream_Client_ALSA_V4L_Configuration V4L_configuration;
  ui_cb_data.configuration = &V4L_configuration;
#if defined (GTK_USE)
  ui_cb_data.UIState = &state_r;
  V4L_configuration.GTKConfiguration.argc = argc_in;
  V4L_configuration.GTKConfiguration.argv = argv_in;
  V4L_configuration.GTKConfiguration.CBData = &ui_cb_data;
  V4L_configuration.GTKConfiguration.eventHooks.finiHook =
      idle_finalize_source_UI_cb;
  V4L_configuration.GTKConfiguration.eventHooks.initHook =
      idle_initialize_source_UI_cb;
  V4L_configuration.GTKConfiguration.definition = &gtk_ui_definition;
  if (!gtk_rc_filename.empty ())
    V4L_configuration.GTKConfiguration.RCFiles.push_back (gtk_rc_filename);
  gtk_configuration_p = &V4L_configuration.GTKConfiguration;
#endif // GTK_USE
//  ui_cb_data_p = &ui_cb_data;
#endif // ACE_WIN32 || ACE_WIN64
//  ACE_ASSERT (ui_cb_data_p);
#endif // GUI_SUPPORT

  // step1d: initialize logging and/or tracing
  Common_Logger_Queue_t logger;
  logger.initialize (&state_r.logQueue,
                     &state_r.logQueueLock);
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                        ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR));
  if (!Common_Log_Tools::initialize (ACE::basename (argv_in[0], ACE_DIRECTORY_SEPARATOR_CHAR), // program name
                                     log_file_name,                            // log file name
                                     false,                                    // log to syslog ?
                                     false,                                    // trace messages ?
                                     trace_information,                        // debug messages ?
                                     (gtk_glade_filename.empty () ? NULL
                                                                  : &logger))) // logger ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Log_Tools::initialize(), aborting\n")));

    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1e: pre-initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (true, // allow SIGUSR1/SIGBREAK
                        signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false); // fill ?
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           (use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                           true,  // using networking ?
                                           false, // using asynch timers ?
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

  // step1f: handle specific program modes
  if (print_version_and_exit)
  {
    do_printVersion (ACE::basename (argv_in[0]));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
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

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
    return EXIT_FAILURE;
  } // end IF

#if defined (GUI_SUPPORT)
  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
#if defined (GTK_USE)
  if (!gtk_glade_filename.empty ())
  { ACE_ASSERT (gtk_configuration_p);
    gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    result_2 = gtk_manager_p->initialize (*gtk_configuration_p);
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_UI_GTK_Manager_T::initialize(), aborting\n")));

      Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                  : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                     previous_signal_actions,
                                     previous_signal_mask);
      Common_Log_Tools::finalize ();
      Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      // *PORTABILITY*: on Windows, finalize ACE...
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64
      return EXIT_FAILURE;
    } // end IF
  } // end IF
#endif // GTK_USE
#endif // GUI_SUPPORT

  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (audio_device_identifier,
           video_device_identifier,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           show_console,
#endif // ACE_WIN32 || ACE_WIN64
           use_uncompressed_format,
           gtk_glade_filename,
           host_name,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
           port,
           use_reactor,
           statistic_reporting_interval,
           use_UDP,
           number_of_proactor_dispatch_threads,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           mediafoundation_ui_cb_data,
           directshow_ui_cb_data,
#else
           ui_cb_data,
#endif // ACE_WIN32 || ACE_WIN64
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           previous_signal_mask);
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
  if (process_profile.elapsed_time (elapsed_time) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                                : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    // *PORTABILITY*: on Windows, finalize ACE...
    result = ACE::fini ();
    if (result == -1)
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

  Common_Signal_Tools::finalize ((use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR
                                              : COMMON_SIGNAL_DISPATCH_PROACTOR),
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalize ();
  Common_Tools::finalize ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *PORTABILITY*: on Windows, finalize ACE...
  result = ACE::fini ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif // ACE_WIN32 || ACE_WIN64

  return EXIT_SUCCESS;
} // end main
