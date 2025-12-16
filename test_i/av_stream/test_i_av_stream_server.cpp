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
#define INITGUID // *NOTE*: this exports DEFINE_GUIDs (see stream_misc_common.h)
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include <iostream>
#include <sstream>
#include <string>

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

#include "common_file_tools.h"
#include "common_os_tools.h"

#include "common_event_tools.h"

#include "common_logger_queue.h"
#include "common_log_tools.h"

#include "common_signal_tools.h"

#include "common_timer_tools.h"

#include "common_ui_defines.h"
#if defined (GTK_SUPPORT)
//#include "common_ui_glade_definition.h"
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H
#include "stream_allocatorheap.h"
#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_common.h"
#include "stream_misc_defines.h"

#if defined (HAVE_CONFIG_H)
#include "ACENetwork_config.h"
#endif // HAVE_CONFIG_H
#include "test_i_common.h"
#include "test_i_defines.h"

#if defined (GTK_SUPPORT)
#include "test_i_gtk_callbacks.h"
#endif // GTK_SUPPORT
#include "test_i_common_modules.h"
#include "test_i_module_eventhandler.h"

#include "test_i_av_stream_server_common.h"
#include "test_i_av_stream_server_eventhandler.h"
#include "test_i_av_stream_server_listener_common.h"
#include "test_i_av_stream_server_signalhandler.h"
#include "test_i_av_stream_server_stream.h"

const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("AVStream");
// *TODO*: unneeded (from client), remove ASAP
const char stream_name_string_2[] = ACE_TEXT_ALWAYS_CHAR ("AVStream_Video");
const char stream_name_string_3[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_NET_IO_DEFAULT_NAME_STRING);

void
do_printUsage (const std::string& programName_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_printUsage"));

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
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-c [VALUE]  : maximum number of connections [")
            << TEST_I_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::string path = configuration_path;
  std::string gtk_rc_file = path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-e          : Gtk .rc file [\"")
            << gtk_rc_file
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-f[[STRING]]: (target) file name [\"")
            << ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE)
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> do not save}")
            << std::endl;
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_GLADE_FILE);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-g[[STRING]]: UI file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
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
#endif
  std::string default_network_interface_string;
  default_network_interface_string =
    Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-n [STRING] : network interface [\"")
            << default_network_interface_string
            << ACE_TEXT_ALWAYS_CHAR ("\"]")
            << std::endl;
  // *TODO*: this doesn't really make sense (yet)
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-o          : use loopback device [")
            << false
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
  std::cout << ACE_TEXT_ALWAYS_CHAR ("-p [VALUE]  : listening port [")
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
            << TEST_I_DEFAULT_NUMBER_OF_SERVER_DISPATCH_THREADS
            << ACE_TEXT_ALWAYS_CHAR ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     unsigned int& maximumNumberOfConnections_out,
                     std::string& gtkRcFile_out,
                     std::string& outputFile_out,
                     std::string& gtkGladeFile_out,
                     bool& logToFile_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     enum Stream_MediaFramework_Type& mediaFramework_out,
#endif // ACE_WIN32 || ACE_WIN64
                     std::string& netWorkInterface_out,
                     bool& useLoopBack_out,
                     unsigned short& listeningPortNumber_out,
                     bool& useReactor_out,
                     unsigned int& statisticReportingInterval_out,
                     bool& traceInformation_out,
                     bool& useUDP_out,
                     bool& printVersionAndExit_out,
                     unsigned int& numberOfDispatchThreads_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_TEST_I_SUBDIRECTORY),
                                                      true); // configuration-

  // initialize results
  std::string path = configuration_path;
  gtkRcFile_out = path;
  gtkRcFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtkRcFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  maximumNumberOfConnections_out =
    TEST_I_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
  outputFile_out = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  gtkGladeFile_out = path;
  gtkGladeFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtkGladeFile_out += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_GLADE_FILE);
  logToFile_out = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  mediaFramework_out = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  netWorkInterface_out =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
  useLoopBack_out = false;
  listeningPortNumber_out = TEST_I_DEFAULT_PORT;
  useReactor_out =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  statisticReportingInterval_out =
    STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  traceInformation_out = false;
  useUDP_out = false;
  printVersionAndExit_out = false;
  numberOfDispatchThreads_out =
    TEST_I_DEFAULT_NUMBER_OF_SERVER_DISPATCH_THREADS;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("c:e:f::g::lmn:op:rs:tuvx:"),
#else
                              ACE_TEXT ("c:e:f::g::ln:op:rs:tuvx:"),
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
      case 'c':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> maximumNumberOfConnections_out;
        break;
      }
      case 'e':
      {
        gtkRcFile_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'f':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          outputFile_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        else
          outputFile_out.clear ();
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
      case 'n':
      {
        netWorkInterface_out = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        break;
      }
      case 'o':
      {
        useLoopBack_out = true;
        break;
      }
      case 'p':
      {
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << argumentParser.opt_arg ();
        converter >> listeningPortNumber_out;
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
        converter >> numberOfDispatchThreads_out;
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

#if defined (VALGRIND_USE)
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_out.sig_del (SIGRTMAX);        // 64
#endif // VALGRIND_USE
#endif // ACE_WIN32 || ACE_WIN64
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
do_initialize_directshow (struct _AMMediaType& sourceAudioMediaType_inout,
                          struct _AMMediaType& sourceVideoMediaType_inout,
                          struct _AMMediaType& outputVideoMediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_directshow"));

  HRESULT result = E_FAIL;
  struct _AMMediaType* media_type_p = NULL;

  Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::free (sourceAudioMediaType_inout);
  Stream_MediaFramework_DirectShow_Tools::free (sourceVideoMediaType_inout);
  Stream_MediaFramework_DirectShow_Tools::free (outputVideoMediaType_inout);

  // sanity check(s)
  ACE_ASSERT (!sourceVideoMediaType_inout.pbFormat);
  sourceVideoMediaType_inout.pbFormat =
    static_cast<BYTE*> (CoTaskMemAlloc (sizeof (struct tagVIDEOINFOHEADER)));
  if (!sourceVideoMediaType_inout.pbFormat)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to CoTaskMemAlloc(%u): \"%m\", aborting\n"),
                sizeof (struct tagVIDEOINFOHEADER)));
    goto error;
  } // end IF
  ACE_OS::memset (sourceVideoMediaType_inout.pbFormat, 0, sizeof (struct tagVIDEOINFOHEADER));
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (sourceVideoMediaType_inout.pbFormat);
  ACE_ASSERT (video_info_header_p);

  // *NOTE*: empty --> use entire video
  BOOL result_2 = SetRectEmpty (&video_info_header_p->rcSource);
  ACE_ASSERT (result_2);
  // *NOTE*: empty --> fill entire buffer
  result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
  ACE_ASSERT (result_2);

  // *NOTE*: 640x480 * 4 * 30 * 8
  // *TODO*: this formula applies to RGB format(s) only
  video_info_header_p->dwBitRate =
    (STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH *
     STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT * 4 * 30 * 8);
  //video_info_header_p->dwBitErrorRate = 0;
  video_info_header_p->AvgTimePerFrame =
    MILLISECONDS_TO_100NS_UNITS (1000 / 30); // --> 30 fps

  // *TODO*: make this configurable (and part of a protocol)
  video_info_header_p->bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
  video_info_header_p->bmiHeader.biWidth = STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH;
  video_info_header_p->bmiHeader.biHeight = STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT;
  video_info_header_p->bmiHeader.biPlanes = 1;
  video_info_header_p->bmiHeader.biBitCount = 32;
  video_info_header_p->bmiHeader.biCompression = BI_RGB;
  video_info_header_p->bmiHeader.biSizeImage =
    GetBitmapSize (&video_info_header_p->bmiHeader);
  //video_info_header_p->bmiHeader.biXPelsPerMeter;
  //video_info_header_p->bmiHeader.biYPelsPerMeter;
  //video_info_header_p->bmiHeader.biClrUsed;
  //video_info_header_p->bmiHeader.biClrImportant;

  sourceVideoMediaType_inout.majortype = MEDIATYPE_Video;
  // work out the GUID for the subtype from the header info
  // *TODO*: cannot use GetBitmapSubtype(), as it returns MEDIASUBTYPE_RGB32
  //         for uncompressed RGB (the Color Space Converter expects
  //         MEDIASUBTYPE_ARGB32)
  //struct _GUID SubTypeGUID = MEDIASUBTYPE_RGB24;
  //struct _GUID SubTypeGUID = GetBitmapSubtype (&video_info_header_p->bmiHeader);
  //if (SubTypeGUID == GUID_NULL)
  //{
  //  ACE_DEBUG ((LM_WARNING,
  //              ACE_TEXT ("failed to GetBitmapSubtype(), falling back\n")));
  //  //SubTypeGUID = MEDIASUBTYPE_Avi; // fallback
  //  SubTypeGUID = MEDIASUBTYPE_RGB24; // fallback
  //} // end IF
  sourceVideoMediaType_inout.subtype = MEDIASUBTYPE_RGB32;
  sourceVideoMediaType_inout.bFixedSizeSamples = TRUE;
  sourceVideoMediaType_inout.bTemporalCompression = FALSE;
  sourceVideoMediaType_inout.lSampleSize =
    video_info_header_p->bmiHeader.biSizeImage;
  sourceVideoMediaType_inout.formattype = FORMAT_VideoInfo;
  sourceVideoMediaType_inout.cbFormat = sizeof (struct tagVIDEOINFOHEADER);

  media_type_p =
    Stream_MediaFramework_DirectShow_Tools::copy (sourceVideoMediaType_inout);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
  outputVideoMediaType_inout = *media_type_p;
  delete (media_type_p); media_type_p = NULL;

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
                                 &sourceAudioMediaType_inout,
                                 TRUE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CreateAudioMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  return true;

error:
  //if (media_filter_p)
  //  media_filter_p->Release ();
  if (media_type_p)
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
  Stream_MediaFramework_DirectShow_Tools::free (sourceAudioMediaType_inout);
  Stream_MediaFramework_DirectShow_Tools::free (sourceVideoMediaType_inout);
  Stream_MediaFramework_DirectShow_Tools::free (outputVideoMediaType_inout);

  return false;
}

bool
do_initialize_mediafoundation (IMFMediaType*& sourceMediaType_out,
                               IMFMediaType*& outputMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("::do_initialize_mediafoundation"));

  HRESULT result = E_FAIL;

  struct _GUID subTypeGUID = GUID_NULL;

  // sanity check(s)
  if (sourceMediaType_out)
  {
    sourceMediaType_out->Release (); sourceMediaType_out = NULL;
  } // end IF
  if (outputMediaType_out)
  {
    outputMediaType_out->Release (); outputMediaType_out = NULL;
  } // end IF

  result = MFStartup (MF_VERSION,
                      MFSTARTUP_LITE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFStartup(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  Stream_MediaFramework_MediaFoundation_Tools::initialize ();

  //// work out the GUID for the subtype from the header info
  //// *TODO*: cannot use GetBitmapSubtype(), as it returns MEDIASUBTYPE_RGB32
  ////         for uncompressed RGB (the Color Space Converter expects
  ////         MEDIASUBTYPE_ARGB32)
  subTypeGUID = MFVideoFormat_ARGB32;
  if (InlineIsEqualGUID (subTypeGUID, GUID_NULL))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to GetBitmapSubtype(), falling back\n")));
    //subTypeGUID = MEDIASUBTYPE_Avi; // fallback
    subTypeGUID = MFVideoFormat_RGB24; // fallback
  } // end IF

  HRESULT result_2 = MFCreateMediaType (&sourceMediaType_out);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (sourceMediaType_out);

  result_2 = sourceMediaType_out->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = sourceMediaType_out->SetGUID (MF_MT_SUBTYPE, subTypeGUID);
  ACE_ASSERT (SUCCEEDED (result_2));
  //result_2 = sourceMediaType_out->SetUINT32 (MF_MT_DEFAULT_STRIDE,
  //                                           STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH * 4);
  //ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = MFSetAttributeSize (sourceMediaType_out,
                                 MF_MT_FRAME_RATE,
                                 STREAM_DEV_CAM_DEFAULT_CAPTURE_FRAME_RATE, 1);
  ACE_ASSERT (SUCCEEDED (result_2));
  //result_2 = sourceMediaType_out->SetUINT32 (MF_MT_AVG_BITRATE, UNITS);
  //ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = MFSetAttributeSize (sourceMediaType_out,
                                 MF_MT_FRAME_SIZE,
                                 STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH, STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT);
  ACE_ASSERT (SUCCEEDED (result_2));
  //result_2 = MFSetAttributeSize (sourceMediaType_out,
  //                               MF_MT_FRAME_RATE_RANGE_MAX,
  //                               30, 1);
  //ACE_ASSERT (SUCCEEDED (result_2));
  //result_2 = MFSetAttributeSize (sourceMediaType_out,
  //                               MF_MT_FRAME_RATE_RANGE_MIN,
  //                               15, 1);
  //ACE_ASSERT (SUCCEEDED (result_2));

  result_2 = sourceMediaType_out->SetUINT32 (MF_MT_INTERLACE_MODE,
                                             MFVideoInterlace_Progressive);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = sourceMediaType_out->SetUINT32 (MF_MT_ALL_SAMPLES_INDEPENDENT,
                                             1);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = MFSetAttributeRatio (sourceMediaType_out,
                                  MF_MT_PIXEL_ASPECT_RATIO,
                                  1, 1);
  ACE_ASSERT (SUCCEEDED (result_2));
  //result_2 = sourceMediaType_out->SetUINT32 (MF_MT_FIXED_SIZE_SAMPLES,
  //                                           1);
  //ACE_ASSERT (SUCCEEDED (result_2));
  UINT32 frame_size = 0;
  result_2 = MFCalculateImageSize (subTypeGUID,
                                   STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH, STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT,
                                   &frame_size);
  ACE_ASSERT (SUCCEEDED (result_2));
  result_2 = sourceMediaType_out->SetUINT32 (MF_MT_SAMPLE_SIZE,
                                             frame_size);
  ACE_ASSERT (SUCCEEDED (result_2));
  //result_2 = sourceMediaType_out->SetUINT32 (MF_MT_MPEG2_PROFILE,
  //                                           eAVEncH264VProfile_Main);
  //ACE_ASSERT (SUCCEEDED (result_2));
  //result_2 = sourceMediaType_out->SetUINT32 (CODECAPI_AVEncCommonRateControlMode,
  //                                           eAVEncCommonRateControlMode_Quality);
  //ACE_ASSERT (SUCCEEDED (result_2));
  //result_2 = sourceMediaType_out->SetUINT32 (CODECAPI_AVEncCommonQuality,
  //                                       80);

  outputMediaType_out =
    Stream_MediaFramework_MediaFoundation_Tools::copy (sourceMediaType_out);
  ACE_ASSERT (outputMediaType_out);

  return true;

error:
  if (sourceMediaType_out)
  {
    sourceMediaType_out->Release (); sourceMediaType_out = NULL;
  } // end IF
  if (outputMediaType_out)
  {
    outputMediaType_out->Release (); outputMediaType_out = NULL;
  } // end IF

  result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  return false;
}

void
do_finalize_directshow (struct Test_I_AVStream_Server_DirectShow_UI_CBData& CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_directshow"));

  Test_I_AVStream_Server_DirectShow_StreamConfiguration_t::ITERATOR_T iterator =
    CBData_in.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != CBData_in.configuration->streamConfiguration.end ());

  if ((*iterator).second.second->builder)
  {
    (*iterator).second.second->builder->Release (); (*iterator).second.second->builder = NULL;
  } // end IF
}

void
do_finalize_mediafoundation (struct Test_I_AVStream_Server_MediaFoundation_UI_CBData& CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("::do_finalize_mediafoundation"));

  Test_I_AVStream_Server_MediaFoundation_StreamConfiguration_t::ITERATOR_T iterator =
    CBData_in.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != CBData_in.configuration->streamConfiguration.end ());

  if ((*iterator).second.second->mediaSource)
  {
    (*iterator).second.second->mediaSource->Release (); (*iterator).second.second->mediaSource = NULL;
  } // end IF
  //if ((*iterator).second.sourceReader)
  //{
  //  (*iterator).second.sourceReader->Release (); (*iterator).second.sourceReader = NULL;
  //} // end IF
  if ((*iterator).second.second->session)
  {
    (*iterator).second.second->session->Release (); (*iterator).second.second->session = NULL;
  } // end IF

  HRESULT result = MFShutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFShutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
}
#endif // ACE_WIN32 || ACE_WIN64

void
do_work (unsigned int maximumNumberOfConnections_in,
         const std::string& fileName_in,
         const std::string& UIDefinitionFilename_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         enum Stream_MediaFramework_Type mediaFramework_in,
#endif // ACE_WIN32 || ACE_WIN64
         const std::string& networkInterface_in,
         bool useLoopBack_in,
         unsigned short listeningPortNumber_in,
         bool useReactor_in,
         unsigned int statisticReportingInterval_in,
         bool useUDP_in,
         unsigned int numberOfDispatchThreads_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         struct Test_I_AVStream_Server_MediaFoundation_UI_CBData& mediaFoundationCBData_in,
         struct Test_I_AVStream_Server_DirectShow_UI_CBData& directShowCBData_in,
#else
         struct Test_I_AVStream_Server_UI_CBData& CBData_in,
#endif // ACE_WIN32 || ACE_WIN64
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout,
         const ACE_Sig_Set& previousSignalMask_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         Test_I_AVStream_Server_DirectShow_SignalHandler_t& directShowSignalHandler_in,
         Test_I_AVStream_Server_MediaFoundation_SignalHandler_t& mediaFoundationSignalHandler_in)
#else
         Test_I_AVStream_Server_SignalHandler_t& signalHandler_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("::do_work"));

  int result = -1;

  // step0a: initialize event dispatch
  struct Common_EventDispatchConfiguration* event_dispatch_configuration_p =
    NULL;
  struct Common_Parser_FlexAllocatorConfiguration allocator_configuration;
  bool serialize_output = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_AVStream_Server_DirectShow_Configuration directshow_configuration;
  struct Test_I_AVStream_Server_DirectShow_StreamConfiguration directshow_stream_configuration;
  struct Test_I_AVStream_Server_MediaFoundation_Configuration mediafoundation_configuration;
  struct Test_I_AVStream_Server_MediaFoundation_StreamConfiguration mediafoundation_stream_configuration;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (useReactor_in)
        directshow_configuration.dispatchConfiguration.numberOfReactorThreads =
          numberOfDispatchThreads_in;
      else
        directshow_configuration.dispatchConfiguration.numberOfProactorThreads =
          numberOfDispatchThreads_in;
      event_dispatch_configuration_p =
        &directshow_configuration.dispatchConfiguration;

      directShowCBData_in.configuration = &directshow_configuration;
      serialize_output =
        directshow_stream_configuration.serializeOutput;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (useReactor_in)
        mediafoundation_configuration.dispatchConfiguration.numberOfReactorThreads =
          numberOfDispatchThreads_in;
      else
        mediafoundation_configuration.dispatchConfiguration.numberOfProactorThreads =
          numberOfDispatchThreads_in;
      event_dispatch_configuration_p =
        &mediafoundation_configuration.dispatchConfiguration;

      mediaFoundationCBData_in.configuration = &mediafoundation_configuration;
      serialize_output =
        mediafoundation_stream_configuration.serializeOutput;
      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  struct Test_I_AVStream_Server_Configuration configuration;
  struct Test_I_AVStream_Server_StreamConfiguration stream_configuration;
  if (useReactor_in)
    configuration.dispatchConfiguration.numberOfReactorThreads =
      numberOfDispatchThreads_in;
  else
    configuration.dispatchConfiguration.numberOfProactorThreads =
      numberOfDispatchThreads_in;
  event_dispatch_configuration_p = &configuration.dispatchConfiguration;
  CBData_in.configuration = &configuration;
  serialize_output = stream_configuration.serializeOutput;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (event_dispatch_configuration_p);
  ACE_UNUSED_ARG (serialize_output);
  if (!Common_Event_Tools::initializeEventDispatch (*event_dispatch_configuration_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Event_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF
  struct Common_EventDispatchState event_dispatch_state_s;
  event_dispatch_state_s.configuration =
      event_dispatch_configuration_p;

  // step0b: initialize configuration and stream
  struct Test_I_AVStream_Configuration* camstream_configuration_p = NULL;
  struct Common_AllocatorConfiguration* allocator_configuration_p =
    &allocator_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      camstream_configuration_p = &directshow_configuration;
      directShowCBData_in.configuration = &directshow_configuration;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      camstream_configuration_p = &mediafoundation_configuration;
      mediaFoundationCBData_in.configuration = &mediafoundation_configuration;
      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  camstream_configuration_p = &configuration;
  CBData_in.configuration = &configuration;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (camstream_configuration_p);
  camstream_configuration_p->protocol = (useUDP_in ? NET_TRANSPORTLAYER_UDP
                                                   : NET_TRANSPORTLAYER_TCP);

  // ********************** module configuration data **************************
  struct Stream_ModuleConfiguration module_configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AllocatorProperties allocator_properties;
  ACE_OS::memset (&allocator_properties, 0, sizeof (struct _AllocatorProperties));
  allocator_properties.cBuffers =
      STREAM_LIB_DIRECTSHOW_VIDEO_DEFAULT_SOURCE_BUFFERS;
  allocator_properties.cbBuffer = -1; // <-- use default
  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  //allocatorProperties_.cbAlign = -1;  // <-- use default
  allocator_properties.cbAlign = 1;
  // *TODO*: IMemAllocator::SetProperties returns E_INVALIDARG (0x80070057)
  //         if this is -1/0 (why ?)
  //allocatorProperties.cbPrefix = -1; // <-- use default
  allocator_properties.cbPrefix = 0;

  struct Test_I_AVStream_Server_DirectShow_ModuleHandlerConfiguration directshow_modulehandler_configuration;
  Test_I_AVStream_Server_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_iterator;
  struct Test_I_AVStream_Server_MediaFoundation_ModuleHandlerConfiguration mediafoundation_modulehandler_configuration;
  Test_I_AVStream_Server_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_iterator;

  Test_I_AVStream_Server_DirectShow_EventHandler_t directshow_ui_event_handler (&directShowCBData_in);
  Test_I_AVStream_Server_MediaFoundation_EventHandler_t mediafoundation_ui_event_handler (&mediaFoundationCBData_in);

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      //directshow_configuration.parserConfiguration.debugScanner = true;
      //directshow_configuration.parserConfiguration.debugParser = true;

      directshow_modulehandler_configuration.concurrency =
        STREAM_HEADMODULECONCURRENCY_CONCURRENT;
      directshow_modulehandler_configuration.configuration =
        &directshow_configuration;
      directshow_modulehandler_configuration.connectionConfigurations =
        &directshow_configuration.connectionConfigurations;
      directshow_modulehandler_configuration.defragmentMode =
        STREAM_DEFRAGMENT_CLONE;
      directshow_modulehandler_configuration.deviceIdentifier.identifier._guid =
        Stream_MediaFramework_DirectSound_Tools::getDefaultDevice (false);
      directshow_modulehandler_configuration.deviceIdentifier.identifierDiscriminator =
        Stream_Device_Identifier::GUID;
      directshow_modulehandler_configuration.display =
        Common_UI_Tools::getDefaultDisplay ();
      directshow_modulehandler_configuration.fileIdentifier.identifier =
        fileName_in;
      directshow_modulehandler_configuration.filterConfiguration =
        &directshow_configuration.filterConfiguration;
      directshow_modulehandler_configuration.inbound = true;
      directshow_modulehandler_configuration.parserConfiguration =
        &directshow_configuration.parserConfiguration;
      directshow_modulehandler_configuration.printProgressDot =
        UIDefinitionFilename_in.empty ();
      directshow_modulehandler_configuration.statisticReportingInterval =
          ACE_Time_Value (statisticReportingInterval_in, 0);
      directshow_modulehandler_configuration.streamConfiguration =
        &directshow_configuration.streamConfiguration;
      directshow_modulehandler_configuration.subscriber =
        &directshow_ui_event_handler;
      directshow_modulehandler_configuration.allocatorConfiguration =
        &allocator_configuration;

      directshow_stream_configuration.allocatorConfiguration = &allocator_configuration;

      directshow_configuration.streamConfiguration.initialize (module_configuration,
                                                               directshow_modulehandler_configuration,
                                                               directshow_stream_configuration);
      directshow_modulehandler_iterator =
        directshow_configuration.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != directshow_configuration.streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_modulehandler_configuration.configuration = &mediafoundation_configuration;
      mediafoundation_modulehandler_configuration.connectionConfigurations =
        &mediafoundation_configuration.connectionConfigurations;

      //modulehandler_configuration.connectionManager = connection_manager_p;
      //result =
      //  modulehandler_configuration.format->SetUINT32 (MF_MT_SAMPLE_SIZE,
      //                                                 frameSize_in);
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_SAMPLE_SIZE,%u): \"%s\", returning\n"),
      //              frameSize_in,
      //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      //  goto clean;
      //} // end IF
      mediafoundation_modulehandler_configuration.inbound = true;
      mediafoundation_modulehandler_configuration.printProgressDot =
        UIDefinitionFilename_in.empty ();
      mediafoundation_modulehandler_configuration.statisticReportingInterval =
          ACE_Time_Value (statisticReportingInterval_in, 0);
      mediafoundation_modulehandler_configuration.streamConfiguration =
        &mediafoundation_configuration.streamConfiguration;
      mediafoundation_modulehandler_configuration.subscriber =
        &mediafoundation_ui_event_handler;
      mediafoundation_modulehandler_configuration.fileIdentifier.identifier =
        fileName_in;

      mediafoundation_stream_configuration.allocatorConfiguration = &allocator_configuration;

      mediafoundation_configuration.streamConfiguration.initialize (module_configuration,
                                                                    mediafoundation_modulehandler_configuration,
                                                                    mediafoundation_stream_configuration);

      mediafoundation_modulehandler_iterator =
        mediafoundation_configuration.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != mediafoundation_configuration.streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  struct Test_I_AVStream_Server_ModuleHandlerConfiguration modulehandler_configuration;
  struct Test_I_AVStream_Server_ModuleHandlerConfiguration modulehandler_configuration_2; // splitter

  struct Stream_MediaFramework_ALSA_Configuration ALSA_configuration; // playback
  ALSA_configuration.asynch = false;

  modulehandler_configuration.ALSAConfiguration = &ALSA_configuration;
  modulehandler_configuration.allocatorConfiguration = &allocator_configuration;
  modulehandler_configuration.concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  modulehandler_configuration.configuration = &configuration;
  modulehandler_configuration.connectionConfigurations =
    &configuration.connectionConfigurations;
  modulehandler_configuration.defragmentMode = STREAM_DEFRAGMENT_CLONE;
  modulehandler_configuration.deviceIdentifier.identifier =
    Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                     SND_PCM_STREAM_PLAYBACK);
  if (modulehandler_configuration.deviceIdentifier.identifier.empty ())
    modulehandler_configuration.deviceIdentifier.identifier =
      ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_DEFAULT_DEVICE_PREFIX);
  modulehandler_configuration.inbound = true;

  //modulehandler_configuration.connectionManager = connection_manager_p;
  //modulehandler_configuration.format.type =
  //  V4L2_BUF_TYPE_VIDEO_CAPTURE;
  //modulehandler_configuration.format.fmt.pix.bytesperline = 960;
  ////modulehandler_configuration.format.fmt.pix.field =
  ////      V4L2_FIELD_NONE;
  //modulehandler_configuration.format.fmt.pix.height = 240;
  //modulehandler_configuration.format.fmt.pix.pixelformat =
  //  V4L2_PIX_FMT_BGR24;
  //modulehandler_configuration.format.fmt.pix.sizeimage = 230400;
  //modulehandler_configuration.format.fmt.pix.width = 320;
  modulehandler_configuration.outputFormat.format =
      AV_PIX_FMT_RGB32;
  modulehandler_configuration.outputFormat.resolution.height =
      STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT;
  modulehandler_configuration.outputFormat.resolution.width =
      STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH ;

  modulehandler_configuration.parserConfiguration =
      &configuration.parserConfiguration;
  modulehandler_configuration.printProgressDot =
    UIDefinitionFilename_in.empty ();
  modulehandler_configuration.statisticReportingInterval =
    ACE_Time_Value (statisticReportingInterval_in, 0);
  modulehandler_configuration.streamConfiguration =
    &configuration.streamConfiguration;
  modulehandler_configuration.targetFileName = fileName_in;

  stream_configuration.allocatorConfiguration = &allocator_configuration;

  stream_configuration.format.audio.format = STREAM_LIB_ALSA_DEFAULT_FORMAT;
  stream_configuration.format.audio.rate = STREAM_LIB_ALSA_DEFAULT_SAMPLE_RATE;
  stream_configuration.format.audio.channels = STREAM_LIB_ALSA_DEFAULT_CHANNELS;

  stream_configuration.format.video.format.pixelformat = V4L2_PIX_FMT_RGB32;
  stream_configuration.format.video.format.height =
      STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT;
  stream_configuration.format.video.format.width =
      STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH ;
  stream_configuration.format.video.frameRate.numerator =
      STREAM_DEV_CAM_DEFAULT_CAPTURE_FRAME_RATE;

  configuration.streamConfiguration.initialize (module_configuration,
                                                modulehandler_configuration,
                                                stream_configuration);
  Test_I_AVStream_Server_StreamConfiguration_t::ITERATOR_T iterator =
      configuration.streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (iterator != configuration.streamConfiguration.end ());

  modulehandler_configuration_2 = modulehandler_configuration;
  configuration.streamConfiguration.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_SPLITTER_DEFAULT_NAME_STRING),
                                                            std::make_pair (&module_configuration,
                                                                            &modulehandler_configuration_2)));
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: in UI mode, COM has already been initialized for this thread
  // *TODO*: where has that happened ?
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      //directshow_modulehandler_iterator =
      //  directShowCBData_in.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_iterator != directShowCBData_in.configuration->streamConfiguration.end ());
      result =
        do_initialize_directshow (directshow_stream_configuration.format.audio,
                                  directshow_stream_configuration.format.video,
                                  (*directshow_modulehandler_iterator).second.second->outputFormat);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_modulehandler_iterator =
        mediaFoundationCBData_in.configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_iterator != mediaFoundationCBData_in.configuration->streamConfiguration.end ());
      ACE_ASSERT (!(*mediafoundation_modulehandler_iterator).second.second->sourceFormat);
      //ACE_ASSERT (!(*mediafoundation_modulehandler_iterator).second.second->outputFormat);
      result =
        do_initialize_mediafoundation ((*mediafoundation_modulehandler_iterator).second.second->sourceFormat,
                                       (*mediafoundation_modulehandler_iterator).second.second->outputFormat);
      ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second->sourceFormat);
      ACE_ASSERT ((*mediafoundation_modulehandler_iterator).second.second->outputFormat);
      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to intialize media framework, returning\n")));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  ACE_ASSERT (allocator_configuration_p);
  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Common_AllocatorConfiguration> heap_allocator;
  if (!heap_allocator.initialize (*allocator_configuration_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to intialize heap allocator, returning\n")));
    return;
  } // end IF
  Stream_IAllocator* allocator_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Server_DirectShow_MessageAllocator_t directshow_message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                                                     &heap_allocator,     // heap allocator handle
                                                                                     true);               // block ?
  Test_I_AVStream_Server_MediaFoundation_MessageAllocator_t mediafoundation_message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                                                               &heap_allocator,     // heap allocator handle
                                                                                               true);               // block ?
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      allocator_p = &directshow_message_allocator;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      allocator_p = &mediafoundation_message_allocator;
      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  Test_I_AVStream_Server_MessageAllocator_t message_allocator (TEST_I_MAX_MESSAGES, // maximum #buffers
                                                               &heap_allocator,     // heap allocator handle
                                                               true);               // block ?
  allocator_p = &message_allocator;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (allocator_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Server_DirectShow_EventHandler_Module directshow_event_handler (NULL,
                                                                                  ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  Test_I_AVStream_Server_MediaFoundation_EventHandler_Module mediafoundation_event_handler (NULL,
                                                                                            ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
#else
  Test_I_AVStream_Server_EventHandler_t ui_event_handler (&CBData_in);
  Test_I_AVStream_Server_Module_EventHandler_Module event_handler (NULL,
                                                                   ACE_TEXT_ALWAYS_CHAR (STREAM_MISC_MESSAGEHANDLER_DEFAULT_NAME_STRING));
  modulehandler_configuration.subscriber = &ui_event_handler;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  directshow_modulehandler_configuration.messageAllocator = allocator_p;

  Test_I_AVStream_Server_DirectShow_EventHandler* directshow_event_handler_p =
    NULL;
  Test_I_AVStream_Server_MediaFoundation_EventHandler* mediafoundation_event_handler_p =
    NULL;
#else
  Test_I_AVStream_Server_Module_EventHandler* event_handler_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_TimerConfiguration timer_configuration;
  timer_configuration.dispatch = COMMON_TIMER_DISPATCH_PROACTOR;
  Common_Timer_Manager_t* timer_manager_p =
        COMMON_TIMERMANAGER_SINGLETON::instance ();
  ACE_ASSERT (timer_manager_p);
  long timer_id = -1;
//  int group_id = -1;
  Net_IStreamStatisticHandler_t* report_handler_p = NULL;
  bool result_2 = false;
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p = NULL;
#endif // GTK_USE

  ACE_INET_Addr inet_address, gateway_address;
  if (!Net_Common_Tools::interfaceToIPAddress (networkInterface_in,
                                               inet_address,
                                               gateway_address))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_Common_Tools::interfaceToIPAddress(), returning\n")));
    return;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Server_MediaFoundation_TCPConnectionConfiguration_t mediafoundation_tcp_connection_configuration;
  Test_I_AVStream_Server_DirectShow_TCPConnectionConfiguration_t directshow_tcp_connection_configuration;
  Test_I_AVStream_Server_MediaFoundation_UDPConnectionConfiguration_t mediafoundation_udp_connection_configuration;
  Test_I_AVStream_Server_DirectShow_UDPConnectionConfiguration_t directshow_udp_connection_configuration;

  Test_I_AVStream_Server_MediaFoundation_TCPConnectionManager_t* mediafoundation_tcp_connection_manager_p =
    NULL;
  Test_I_AVStream_Server_DirectShow_TCPConnectionManager_t* directshow_tcp_connection_manager_p =
    NULL;
  Test_I_AVStream_Server_MediaFoundation_UDPConnectionManager_t* mediafoundation_udp_connection_manager_p =
    NULL;
  Test_I_AVStream_Server_DirectShow_UDPConnectionManager_t* directshow_udp_connection_manager_p =
    NULL;
  //Net_ConnectionConfigurationsIterator_t connection_configuration_iterator;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_tcp_connection_configuration.allocatorConfiguration = &allocator_configuration;
      directshow_udp_connection_configuration.allocatorConfiguration = &allocator_configuration;
      directshow_configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("TCP"),
                                                                                &directshow_tcp_connection_configuration));
      directshow_configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("UDP"),
                                                                                &directshow_udp_connection_configuration));

      directshow_tcp_connection_manager_p =
        TEST_I_AVSTREAM_SERVER_DIRECTSHOW_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
      directshow_tcp_connection_manager_p->initialize (maximumNumberOfConnections_in ? maximumNumberOfConnections_in
                                                                                     : std::numeric_limits<unsigned int>::max (),
                                                       ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
      directshow_udp_connection_manager_p =
        TEST_I_AVSTREAM_SERVER_DIRECTSHOW_UDP_CONNECTIONMANAGER_SINGLETON::instance ();
      directshow_udp_connection_manager_p->initialize (maximumNumberOfConnections_in ? maximumNumberOfConnections_in
                                                                                     : std::numeric_limits<unsigned int>::max (),
                                                       ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
      (*directshow_modulehandler_iterator).second.second->connectionManager =
        directshow_tcp_connection_manager_p;
      report_handler_p = directshow_tcp_connection_manager_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("TCP"),
                                                                                     &mediafoundation_tcp_connection_configuration));

      mediafoundation_tcp_connection_manager_p =
        TEST_I_AVSTREAM_SERVER_MEDIAFOUNDATION_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
      mediafoundation_tcp_connection_manager_p->initialize (maximumNumberOfConnections_in ? maximumNumberOfConnections_in
                                                                                          : std::numeric_limits<unsigned int>::max (),
                                                            ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
      mediafoundation_udp_connection_manager_p =
        TEST_I_AVSTREAM_SERVER_MEDIAFOUNDATION_UDP_CONNECTIONMANAGER_SINGLETON::instance ();
      mediafoundation_udp_connection_manager_p->initialize (maximumNumberOfConnections_in ? maximumNumberOfConnections_in
                                                                                          : std::numeric_limits<unsigned int>::max (),
                                                            ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
      (*mediafoundation_modulehandler_iterator).second.second->connectionManager =
        mediafoundation_tcp_connection_manager_p;
      report_handler_p = mediafoundation_tcp_connection_manager_p;
      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  Test_I_AVStream_Server_TCPConnectionConfiguration_t tcp_connection_configuration;
  Test_I_AVStream_Server_UDPConnectionConfiguration_t udp_connection_configuration;
  configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("TCP"),
                                                                 &tcp_connection_configuration));
  configuration.connectionConfigurations.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("UDP"),
                                                                 &udp_connection_configuration));
  Test_I_AVStream_Server_TCPConnectionManager_t* tcp_connection_manager_p =
    TEST_I_AVSTREAM_SERVER_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
  Test_I_AVStream_Server_UDPConnectionManager_t* udp_connection_manager_p =
    TEST_I_AVSTREAM_SERVER_UDP_CONNECTIONMANAGER_SINGLETON::instance ();
  tcp_connection_manager_p->initialize ((maximumNumberOfConnections_in ? maximumNumberOfConnections_in
                                                                       : std::numeric_limits<unsigned int>::max ()),
                                        ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  udp_connection_manager_p->initialize ((maximumNumberOfConnections_in ? maximumNumberOfConnections_in
                                                                       : std::numeric_limits<unsigned int>::max ()),
                                        ACE_Time_Value (0, NET_STATISTIC_DEFAULT_VISIT_INTERVAL_MS * 1000));
  report_handler_p = tcp_connection_manager_p;
  (*iterator).second.second->connectionManager = tcp_connection_manager_p;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (report_handler_p);
  Net_StreamStatisticHandler_t statistic_handler (COMMON_STATISTIC_ACTION_REPORT,
                                                  report_handler_p,
                                                  false);
  ACE_Event_Handler* event_handler_2 = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_event_handler_p =
        dynamic_cast<Test_I_AVStream_Server_DirectShow_EventHandler*> (directshow_event_handler.writer ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_event_handler_p =
        dynamic_cast<Test_I_AVStream_Server_MediaFoundation_EventHandler*> (mediafoundation_event_handler.writer ());
      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
  ACE_ASSERT (mediafoundation_event_handler_p || directshow_event_handler_p);
#else
//  Net_ConnectionConfigurationsIterator_t iterator_2;
  event_handler_p =
    dynamic_cast<Test_I_AVStream_Server_Module_EventHandler*> (event_handler.writer ());
  if (!event_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Test_I_AVStream_Server_Module_EventHandler>, returning\n")));
    return;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  struct Net_UserData user_data_s;
  // ********************** socket configuration data **************************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_tcp_connection_configuration.socketConfiguration.address.set_port_number (listeningPortNumber_in,
                                                                                           1);
      directshow_tcp_connection_configuration.socketConfiguration.useLoopBackDevice =
        useLoopBack_in;
      if (directshow_tcp_connection_configuration.socketConfiguration.useLoopBackDevice)
      {
        result = directshow_tcp_connection_configuration.socketConfiguration.address.set (listeningPortNumber_in,
                                                                                          INADDR_LOOPBACK,
                                                                                          1,
                                                                                          0);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
      } // end IF
      directshow_tcp_connection_configuration.statisticReportingInterval =
        statisticReportingInterval_in;
      directshow_tcp_connection_configuration.messageAllocator =
        &directshow_message_allocator;
      directshow_tcp_connection_configuration.streamConfiguration =
        &directshow_configuration.streamConfiguration;

      directshow_tcp_connection_manager_p->set (directshow_tcp_connection_configuration,
                                                &user_data_s);

      result =
        directshow_udp_connection_configuration.socketConfiguration.peerAddress.set (listeningPortNumber_in + 1,
                                                                                     INADDR_LOOPBACK,
                                                                                     1,
                                                                                     0);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
      directshow_udp_connection_configuration.socketConfiguration.listenAddress.set_port_number (listeningPortNumber_in,
                                                                                                 1);
      directshow_udp_connection_configuration.socketConfiguration.useLoopBackDevice =
        useLoopBack_in;
      if (directshow_udp_connection_configuration.socketConfiguration.useLoopBackDevice)
      {
        result =
          directshow_udp_connection_configuration.socketConfiguration.listenAddress.set (listeningPortNumber_in,
                                                                                         INADDR_LOOPBACK,
                                                                                         1,
                                                                                         0);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
      } // end IF
      directshow_udp_connection_configuration.statisticReportingInterval =
        statisticReportingInterval_in;
      directshow_udp_connection_configuration.messageAllocator =
        &directshow_message_allocator;
      directshow_udp_connection_configuration.streamConfiguration =
        &directshow_configuration.streamConfiguration;

      directshow_udp_connection_manager_p->set (directshow_udp_connection_configuration,
                                                &user_data_s);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_tcp_connection_configuration.socketConfiguration.address.set_port_number (listeningPortNumber_in,
                                                                                                1);
      mediafoundation_tcp_connection_configuration.socketConfiguration.useLoopBackDevice =
        useLoopBack_in;
      if (mediafoundation_tcp_connection_configuration.socketConfiguration.useLoopBackDevice)
      {
        result =
          mediafoundation_tcp_connection_configuration.socketConfiguration.address.set (listeningPortNumber_in,
                                                                                        INADDR_LOOPBACK,
                                                                                        1,
                                                                                        0);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
      } // end IF
      //if (bufferSize_in)
      //  (*connection_configuration_iterator).second->PDUSize =
      //    bufferSize_in;
      mediafoundation_tcp_connection_configuration.statisticReportingInterval =
        statisticReportingInterval_in;

      mediafoundation_tcp_connection_configuration.messageAllocator =
        &mediafoundation_message_allocator;
      mediafoundation_tcp_connection_configuration.streamConfiguration =
        &mediafoundation_configuration.streamConfiguration;

      mediafoundation_tcp_connection_manager_p->set (mediafoundation_tcp_connection_configuration,
                                                     &user_data_s);
      break;
    } // end IF
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  tcp_connection_configuration.socketConfiguration.address = inet_address;
  tcp_connection_configuration.socketConfiguration.address.set_port_number (listeningPortNumber_in,
                                                                            1);
  tcp_connection_configuration.allocatorConfiguration =
    &allocator_configuration;
  tcp_connection_configuration.socketConfiguration.useLoopBackDevice =
    useLoopBack_in;
  if (tcp_connection_configuration.socketConfiguration.useLoopBackDevice)
  {
    result =
      tcp_connection_configuration.socketConfiguration.address.set (listeningPortNumber_in,
                                                                    INADDR_LOOPBACK,
                                                                    1,
                                                                    0);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
  } // end IF
  tcp_connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  tcp_connection_configuration.messageAllocator = &message_allocator;
  tcp_connection_configuration.streamConfiguration =
    &configuration.streamConfiguration;

  tcp_connection_manager_p->set (tcp_connection_configuration,
                                 &user_data_s);

  result =
    udp_connection_configuration.socketConfiguration.peerAddress.set (listeningPortNumber_in + 1,
                                                                      INADDR_LOOPBACK,
                                                                      1,
                                                                      0);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
  udp_connection_configuration.socketConfiguration.listenAddress.set_port_number (listeningPortNumber_in,
                                                                                  1);
  udp_connection_configuration.allocatorConfiguration =
    &allocator_configuration;
  udp_connection_configuration.socketConfiguration.useLoopBackDevice =
    useLoopBack_in;
  if (udp_connection_configuration.socketConfiguration.useLoopBackDevice)
  {
    result =
      udp_connection_configuration.socketConfiguration.listenAddress.set (listeningPortNumber_in,
                                                                          INADDR_LOOPBACK,
                                                                          1,
                                                                          0);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::set(): \"%m\", continuing\n")));
  } // end IF
  udp_connection_configuration.statisticReportingInterval =
    statisticReportingInterval_in;
  udp_connection_configuration.messageAllocator = &message_allocator;
  udp_connection_configuration.streamConfiguration =
    &configuration.streamConfiguration;

  udp_connection_manager_p->set (udp_connection_configuration,
                                 &user_data_s);
#endif // ACE_WIN32 || ACE_WIN64

  // **************************** stream data **********************************
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // ******************** DirectShow configuration data ************************
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      //directshow_configuration.pinConfiguration.bufferSize = bufferSize_in;
      ACE_ASSERT (!directshow_configuration.pinConfiguration.format);
      directshow_configuration.pinConfiguration.format =
        Stream_MediaFramework_DirectShow_Tools::copy (directshow_stream_configuration.format.video);
      ACE_ASSERT (directshow_configuration.pinConfiguration.format);

      directshow_configuration.pinConfiguration.allocatorProperties =
        &allocator_properties;
      directshow_configuration.filterConfiguration.allocatorProperties =
        &allocator_properties;
      directshow_configuration.filterConfiguration.pinConfiguration =
        &directshow_configuration.pinConfiguration;

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64

  // ******************** (sub-)stream configuration data **********************
  //if (bufferSize_in)
  //  allocator_configuration.defaultBufferSize = bufferSize_in;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      allocator_configuration.defaultBufferSize =
        (directshow_stream_configuration.format.video.lSampleSize +
         sizeof (struct acestream_av_stream_header));
      directshow_stream_configuration.cloneModule = true;
      directshow_stream_configuration.messageAllocator = allocator_p;
      if (!UIDefinitionFilename_in.empty ())
        directshow_stream_configuration.module =
          &directshow_event_handler;
      directshow_stream_configuration.printFinalReport = true;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_stream_configuration.cloneModule = true;
      mediafoundation_stream_configuration.messageAllocator = allocator_p;
      if (!UIDefinitionFilename_in.empty ())
        mediafoundation_stream_configuration.module =
          &mediafoundation_event_handler;
      //mediafoundation_configuration.streamConfiguration.mediaFoundationConfiguration =
      //  &mediafoundation_configuration.mediaFoundationConfiguration;
      mediafoundation_stream_configuration.printFinalReport = true;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  allocator_configuration.defaultBufferSize =
    (STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT * STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH * 4) +
    sizeof (struct acestream_av_stream_header);

  stream_configuration.cloneModule = true;
  stream_configuration.messageAllocator = allocator_p;
  if (!UIDefinitionFilename_in.empty ())
    stream_configuration.module = &event_handler;
  stream_configuration.printFinalReport = true;
#endif // ACE_WIN32 || ACE_WIN64

  // ********************* listener configuration data *************************

  // step0d: initialize regular (global) statistic reporting
  timer_manager_p->initialize (timer_configuration);
  timer_manager_p->start (NULL);
  if (statisticReportingInterval_in)
  {
    ACE_Time_Value interval (statisticReportingInterval_in, 0);
    timer_id =
      timer_manager_p->schedule_timer (&statistic_handler,         // event handler handle
                                       NULL,                       // asynchrnous completion token
                                       COMMON_TIME_NOW + interval, // first wakeup time
                                       interval);                  // interval
    if (timer_id == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to schedule timer: \"%m\", returning\n")));
      timer_manager_p->stop ();
      goto clean;
    } // end IF
  } // end IF

  // step0e: initialize signal handling
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_configuration.signalHandlerConfiguration.connectionManager =
        TEST_I_AVSTREAM_SERVER_DIRECTSHOW_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
      directshow_configuration.signalHandlerConfiguration.dispatchState =
        &event_dispatch_state_s;
      //if (useReactor_in)
      //  directshow_configuration.signalHandlerConfiguration.listener =
      //    Test_I_AVStream_Server_DIRECTSHOW_LISTENER_SINGLETON::instance ();
      //else
        directshow_configuration.signalHandlerConfiguration.listener =
          TEST_I_AVSTREAM_SERVER_DIRECTSHOW_ASYNCHLISTENER_SINGLETON::instance ();
      directshow_configuration.signalHandlerConfiguration.statisticReportingHandler =
        report_handler_p;
      directshow_configuration.signalHandlerConfiguration.statisticReportingTimerId =
        timer_id;
      result =
        directShowSignalHandler_in.initialize (directshow_configuration.signalHandlerConfiguration);
      event_handler_2 = &directShowSignalHandler_in;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_configuration.signalHandlerConfiguration.connectionManager =
        TEST_I_AVSTREAM_SERVER_MEDIAFOUNDATION_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
      mediafoundation_configuration.signalHandlerConfiguration.dispatchState =
        &event_dispatch_state_s;
      //if (useReactor_in)
      //  mediafoundation_configuration.signalHandlerConfiguration.listener =
      //    Test_I_AVStream_Server_MEDIAFOUNDATION_LISTENER_SINGLETON::instance ();
      //else
        mediafoundation_configuration.signalHandlerConfiguration.listener =
          TEST_I_AVSTREAM_SERVER_MEDIAFOUNDATION_ASYNCHLISTENER_SINGLETON::instance ();
      mediafoundation_configuration.signalHandlerConfiguration.statisticReportingHandler =
        report_handler_p;
      mediafoundation_configuration.signalHandlerConfiguration.statisticReportingTimerId =
        timer_id;
      result =
        mediaFoundationSignalHandler_in.initialize (mediafoundation_configuration.signalHandlerConfiguration);
      event_handler_2 = &mediaFoundationSignalHandler_in;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  configuration.signalHandlerConfiguration.connectionManager =
      tcp_connection_manager_p;
  configuration.signalHandlerConfiguration.dispatchState =
      &event_dispatch_state_s;
//  if (useReactor_in)
//    configuration.signalHandlerConfiguration.listener =
//        Test_I_AVStream_Server_LISTENER_SINGLETON::instance ();
//  else
    configuration.signalHandlerConfiguration.listener =
        TEST_I_AVSTREAM_SERVER_ASYNCHLISTENER_SINGLETON::instance ();
  configuration.signalHandlerConfiguration.statisticReportingHandler =
      report_handler_p;
  configuration.signalHandlerConfiguration.statisticReportingTimerId =
      timer_id;
  result =
      signalHandler_in.initialize (configuration.signalHandlerConfiguration);
  event_handler_2 = &signalHandler_in;
#endif // ACE_WIN32 || ACE_WIN64
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    timer_manager_p->stop ();
    goto clean;
  } // end IF
  if (!Common_Signal_Tools::initialize (COMMON_SIGNAL_DISPATCH_SIGNAL,
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        event_handler_2,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::initialize(), returning\n")));
    timer_manager_p->stop ();
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

  // step1a: start GUI event loop ?
  if (!UIDefinitionFilename_in.empty ())
  {
#if defined (GTK_USE)
    gtk_manager_p = COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
    ACE_ASSERT (gtk_manager_p);
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
    //CBData_in.gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    //  std::make_pair (UIDefinitionFilename_in, static_cast<GladeXML*> (NULL));
    state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
      std::make_pair (UIDefinitionFilename_in, static_cast<GtkBuilder*> (NULL));

    gtk_manager_p->start (NULL);
    ACE_Time_Value timeout (0,
                            COMMON_UI_GTK_TIMEOUT_DEFAULT_MANAGER_INITIALIZATION_MS * 1000);
    result = ACE_OS::sleep (timeout);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &timeout));
    if (!gtk_manager_p->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));
      timer_manager_p->stop ();
      goto clean;
    } // end IF
#endif // GTK_USE

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    HWND window_p = ::GetConsoleWindow ();
    if (!window_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::GetConsoleWindow(), returning\n")));
      timer_manager_p->stop ();
#if defined (GTK_USE)
      gtk_manager_p->stop (true, true);
#endif // GTK_USE
      goto clean;
    } // end IF
    BOOL was_visible_b = ::ShowWindow (window_p, SW_HIDE);
    ACE_UNUSED_ARG (was_visible_b);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  // step1b: initialize worker(s)
  if (!Common_Event_Tools::startEventDispatch (event_dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));

    // clean up
    //		{ // synch access
    //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

    //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
    //					 iterator != CBData_in.event_source_ids.end();
    //					 iterator++)
    //				g_source_remove(*iterator);
    //		} // end lock scope
    if (!UIDefinitionFilename_in.empty ())
#if defined (GTK_USE)
      gtk_manager_p->stop (true, true);
#else
      ;
#endif // GTK_USE
    timer_manager_p->stop ();
    goto clean;
  } // end IF

  // step1c: start listening ?
  if (UIDefinitionFilename_in.empty ())
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Test_I_AVStream_Server_DirectShow_IUDPConnector_t* directshow_iconnector_p = NULL;
    Test_I_AVStream_Server_MediaFoundation_IUDPConnector_t* mediafoundation_iconnector_p =
      NULL;
#else
//    Test_I_AVStream_Server_ITCPConnector_t* i_tcp_connector_p = NULL;
    Test_I_AVStream_Server_IUDPConnector_t* i_udp_connector_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
    if (useUDP_in)
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          if (useReactor_in)
            ACE_NEW_NORETURN (directshow_iconnector_p,
                              Test_I_AVStream_Server_DirectShow_UDPConnector_t (true));
          else
            ACE_NEW_NORETURN (directshow_iconnector_p,
                              Test_I_AVStream_Server_DirectShow_UDPAsynchConnector_t (true));
          result_2 =
            directshow_iconnector_p->initialize (directshow_udp_connection_configuration);
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          if (useReactor_in)
            ACE_NEW_NORETURN (mediafoundation_iconnector_p,
                              Test_I_AVStream_Server_MediaFoundation_UDPConnector_t (true));
          else
            ACE_NEW_NORETURN (mediafoundation_iconnector_p,
                              Test_I_AVStream_Server_MediaFoundation_UDPAsynchConnector_t (true));
          result_2 =
            mediafoundation_iconnector_p->initialize (mediafoundation_udp_connection_configuration);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      mediaFramework_in));
          return;
        } // end ELSE
      } // end SWITCH
#else
      if (useReactor_in)
        ACE_NEW_NORETURN (i_udp_connector_p,
                          Test_I_AVStream_Server_UDPConnector_t (true));
      else
        ACE_NEW_NORETURN (i_udp_connector_p,
                          Test_I_AVStream_Server_UDPAsynchConnector_t (true));

      result_2 = i_udp_connector_p->initialize (udp_connection_configuration);
#endif // ACE_WIN32 || ACE_WIN64
      if (!result_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));

        // clean up
        Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                   true); // wait ?
        //		{ // synch access
        //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

        //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
        //					 iterator != CBData_in.event_source_ids.end();
        //					 iterator++)
        //				g_source_remove(*iterator);
        //		} // end lock scope
        if (!UIDefinitionFilename_in.empty ())
#if defined (GTK_USE)
          gtk_manager_p->stop (true, true);
#else
          ;
#endif // GTK_USE
        timer_manager_p->stop ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (mediaFramework_in)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          {
            delete directshow_iconnector_p;
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          {
            delete mediafoundation_iconnector_p;
            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                        mediaFramework_in));
            return;
          } // end ELSE
        } // end SWITCH
#else
        delete i_udp_connector_p; i_udp_connector_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
        goto clean;
      } // end IF

      // connect
      ACE_INET_Addr listen_address;
      //ACE_thread_t thread_id;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          listen_address =
            directshow_udp_connection_configuration.socketConfiguration.listenAddress;
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          listen_address =
            mediafoundation_udp_connection_configuration.socketConfiguration.listenAddress;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      mediaFramework_in));
          return;
        } // end ELSE
      } // end SWITCH
#else
      listen_address = udp_connection_configuration.socketConfiguration.listenAddress;
#endif // ACE_WIN32 || ACE_WIN64
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string(): \"%m\", continuing\n")));
      // *TODO*: support one-thread operation by scheduling a signal and manually
      //         running the dispatch loop for a limited time...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          directshow_configuration.handle =
            directshow_iconnector_p->connect (listen_address);
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          mediafoundation_configuration.handle =
            mediafoundation_iconnector_p->connect (listen_address);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      mediaFramework_in));
          return;
        } // end ELSE
      } // end SWITCH
#else
      configuration.handle = i_udp_connector_p->connect (listen_address);
#endif // ACE_WIN32 || ACE_WIN64
      ACE_Time_Value timeout (NET_CONNECTION_ASYNCH_DEFAULT_ESTABLISHMENT_TIMEOUT_S,
                              0);
      ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Test_I_AVStream_Server_MediaFoundation_UDPAsynchConnector_t::ICONNECTION_T* mediafoundation_connection_p =
        NULL;
      Test_I_AVStream_Server_DirectShow_UDPAsynchConnector_t::ICONNECTION_T* directshow_connection_p =
        NULL;
      bool done = false;
#else
      typename Test_I_AVStream_Server_UDPAsynchConnector_t::ICONNECTION_T* connection_p =
          NULL;
#endif // ACE_WIN32 || ACE_WIN64
      // *TODO*: avoid tight loop here
      do
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (mediaFramework_in)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          {
            directshow_connection_p =
              directshow_udp_connection_manager_p->get (listen_address,
                                                        false);
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          {
            mediafoundation_connection_p =
              mediafoundation_udp_connection_manager_p->get (listen_address,
                                                             false);
            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                        mediaFramework_in));
            return;
          } // end ELSE
        } // end SWITCH
#else
        connection_p = udp_connection_manager_p->get (listen_address,
                                                      false);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (mediaFramework_in)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          {
            if (directshow_connection_p)
            {
              directshow_configuration.handle =
                reinterpret_cast<ACE_HANDLE> (directshow_connection_p->id ());
              directshow_connection_p->decrease (); directshow_connection_p = NULL;
              done = true;
              break;
            } // end IF
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          {
            if (mediafoundation_connection_p)
            {
              mediafoundation_configuration.handle =
                reinterpret_cast<ACE_HANDLE> (mediafoundation_connection_p->id ());
              mediafoundation_connection_p->decrease (); mediafoundation_connection_p = NULL;
              done = true;
              break;
            } // end IF
            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                        mediaFramework_in));
            return;
          } // end ELSE
        } // end SWITCH
        if (done)
          break;
#else
        if (connection_p)
        {
          configuration.handle = static_cast<ACE_HANDLE> (connection_p->id ());
          connection_p->decrease (); connection_p = NULL;
          break;
        } // end IF
#endif // ACE_WIN32 || ACE_WIN64
      } while (COMMON_TIME_NOW < deadline);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          result_2 = (directshow_configuration.handle == ACE_INVALID_HANDLE);
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          result_2 =
            (mediafoundation_configuration.handle == ACE_INVALID_HANDLE);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      mediaFramework_in));
          return;
        } // end ELSE
      } // end SWITCH
#else
      result_2 = (configuration.handle == ACE_INVALID_HANDLE);
#endif // ACE_WIN32 || ACE_WIN64
      if (!result_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to connect to %s, returning\n"),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (listen_address,
                                                                   false,
                                                                   false).c_str ())));

        // clean up
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        //if (useMediaFoundation_in)
        //  mediafoundation_iconnector_p->abort ();
        //else
        //  directshow_iconnector_p->abort ();
#else
#endif // ACE_WIN32 || ACE_WIN64
        Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                   true); // wait ?
        //		{ // synch access
        //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

        //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
        //					 iterator != CBData_in.event_source_ids.end();
        //					 iterator++)
        //				g_source_remove(*iterator);
        //		} // end lock scope
        if (!UIDefinitionFilename_in.empty ())
#if defined (GTK_USE)
          gtk_manager_p->stop (true, true);
#else
          ;
#endif // GTK_USE
        timer_manager_p->stop ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        switch (mediaFramework_in)
        {
          case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
          {
            delete directshow_iconnector_p;
            break;
          }
          case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
          {
            delete mediafoundation_iconnector_p;
            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                        mediaFramework_in));
            return;
          } // end ELSE
        } // end SWITCH
#else
        delete i_udp_connector_p;
#endif // ACE_WIN32 || ACE_WIN64
        goto clean;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("listening to UDP \"%s\"...\n"),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (listen_address).c_str ())));

      // clean up
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          delete directshow_iconnector_p;
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          delete mediafoundation_iconnector_p;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      mediaFramework_in));
          return;
        } // end ELSE
      } // end SWITCH
#else
      delete i_udp_connector_p;
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
    else
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          result_2 =
            directShowCBData_in.configuration->signalHandlerConfiguration.listener->initialize (directshow_tcp_connection_configuration);
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          result_2 =
            mediaFoundationCBData_in.configuration->signalHandlerConfiguration.listener->initialize (mediafoundation_tcp_connection_configuration);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      mediaFramework_in));
          return;
        } // end ELSE
      } // end SWITCH
#else
      result_2 =
        CBData_in.configuration->signalHandlerConfiguration.listener->initialize (tcp_connection_configuration);
#endif // ACE_WIN32 || ACE_WIN64
      if (!result_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize listener, returning\n")));

        Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                                   true); // wait ?
        //		{ // synch access
        //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

        //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
        //					 iterator != CBData_in.event_source_ids.end();
        //					 iterator++)
        //				g_source_remove(*iterator);
        //		} // end lock scope
        if (!UIDefinitionFilename_in.empty ())
#if defined (GTK_USE)
          gtk_manager_p->stop (true, true);
#else
          ;
#endif // GTK_USE
        timer_manager_p->stop ();
        goto clean;
      } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      switch (mediaFramework_in)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          directShowCBData_in.configuration->signalHandlerConfiguration.listener->start (NULL);
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          mediaFoundationCBData_in.configuration->signalHandlerConfiguration.listener->start (NULL);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      mediaFramework_in));
          return;
        } // end ELSE
      } // end SWITCH
#else
      //ACE_thread_t thread_id = 0;
      CBData_in.configuration->signalHandlerConfiguration.listener->start (NULL);
      //result_2 =
      //  CBData_in.configuration->signalHandlerConfiguration.listener->isRunning ();
#endif // ACE_WIN32 || ACE_WIN64
//      if (!result_2)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to start listener (port: %u), returning\n"),
//                    listeningPortNumber_in));
//
//        Common_Tools::finalizeEventDispatch (event_dispatch_state_s.proactorGroupId,
//                                             event_dispatch_state_s.reactorGroupId,
//                                             true);
//        //		{ // synch access
//        //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);
//
//        //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
//        //					 iterator != CBData_in.event_source_ids.end();
//        //					 iterator++)
//        //				g_source_remove(*iterator);
//        //		} // end lock scope
//        if (!UIDefinitionFilename_in.empty ())
//#if defined (GTK_USE)
//          gtk_manager_p->stop (true, true, true);
//#else
//          ;
//#endif // GTK_USE
//        timer_manager_p->stop ();
//        goto clean;
//      } // end IF
    } // end ELSE
  } // end IF

  // *NOTE*: from this point on, clean up any remote connections !

//  Common_Tools::dispatchEvents (useReactor_in,
//                                group_id);

  // clean up
clean:
  // *NOTE*: listener has stopped, interval timer has been cancelled,
  // and connections have been aborted...
  //		{ // synch access
  //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(CBData_in.lock);

  //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin();
  //					 iterator != CBData_in.event_source_ids.end();
  //					 iterator++)
  //				g_source_remove(*iterator);
  //		} // end lock scope
  if (!UIDefinitionFilename_in.empty ())
  {
#if defined (GTK_USE)
    gtk_manager_p->wait (false);
#endif // GTK_USE
//    connection_manager_p->abort ();
  } // end IF
  else
    Common_Event_Tools::dispatchEvents (event_dispatch_state_s);

  // wait for connection processing to complete
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //  connection_manager_p->stop ();
  //  connection_manager_p->abort ();
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_tcp_connection_manager_p->wait ();
      directshow_udp_connection_manager_p->wait ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_tcp_connection_manager_p->wait ();
      mediafoundation_udp_connection_manager_p->wait ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  if (useUDP_in)
    udp_connection_manager_p->wait ();
  else
    tcp_connection_manager_p->wait ();
#endif // ACE_WIN32 || ACE_WIN64

  timer_manager_p->stop ();

  Common_Event_Tools::finalizeEventDispatch (event_dispatch_state_s,
                                             true); // wait ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result = directshow_event_handler.close (ACE_Module_Base::M_DELETE_NONE);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      result =
        mediafoundation_event_handler.close (ACE_Module_Base::M_DELETE_NONE);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#else
  result = event_handler.close (ACE_Module_Base::M_DELETE_NONE);
#endif // ACE_WIN32 || ACE_WIN64
  if (result == -1)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (mediaFramework_in)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
                    directshow_event_handler.name ()));
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
                    mediafoundation_event_handler.name ()));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    mediaFramework_in));
        return;
      } // end ELSE
    } // end SWITCH
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Module::close (): \"%m\", continuing\n"),
                event_handler.name ()));
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      do_finalize_directshow (directShowCBData_in);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      do_finalize_mediafoundation (mediaFoundationCBData_in);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  mediaFramework_in));
      return;
    } // end ELSE
  } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

COMMON_DEFINE_PRINTVERSION_FUNCTION (do_printVersion,STREAM_MAKE_VERSION_STRING_VARIABLE (programName_in,ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL),version_string),version_string)

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

  // initialize framework(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Tools::initialize (true,   // COM ?
                            false); // RNG ?
#else
  Common_Tools::initialize (false); // RNG ?
#endif // ACE_WIN32 || ACE_WIN64
  Common_File_Tools::initialize (ACE_TEXT_ALWAYS_CHAR ("av_stream"));

  std::string configuration_path =
    Common_File_Tools::getConfigurationDataDirectory (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                                      ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_TEST_I_SUBDIRECTORY),
                                                      true); // configuration-

  // step1a set defaults
  unsigned int maximum_number_of_connections =
    TEST_I_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS;
  std::string path = configuration_path;
  std::string gtk_rc_file = path;
  gtk_rc_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_rc_file += ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_GTK_RC_FILE);
  std::string output_file = ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_OUTPUT_FILE);
  std::string gtk_glade_file = path;
  gtk_glade_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  gtk_glade_file +=
      ACE_TEXT_ALWAYS_CHAR (TEST_I_DEFAULT_TARGET_GLADE_FILE);
  bool log_to_file = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type media_framework_e =
    STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
#endif // ACE_WIN32 || ACE_WIN64
  std::string network_interface =
      Net_Common_Tools::getDefaultInterface (NET_LINKLAYER_802_3);
  bool use_loopback = false;
  unsigned short listening_port_number = TEST_I_DEFAULT_PORT;
  bool use_reactor =
    (COMMON_EVENT_DEFAULT_DISPATCH == COMMON_EVENT_DISPATCH_REACTOR);
  unsigned int statistic_reporting_interval =
      STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL_S;
  bool trace_information = false;
  bool use_UDP = false;
  bool print_version_and_exit = false;
  unsigned int number_of_dispatch_threads =
    TEST_I_DEFAULT_NUMBER_OF_SERVER_DISPATCH_THREADS;

  // step1b: parse/process/validate configuration
  if (!do_processArguments (argc_in,
                            argv_in,
                            maximum_number_of_connections,
                            gtk_rc_file,
                            output_file,
                            gtk_glade_file,
                            log_to_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
                            network_interface,
                            use_loopback,
                            listening_port_number,
                            use_reactor,
                            statistic_reporting_interval,
                            trace_information,
                            use_UDP,
                            print_version_and_exit,
                            number_of_dispatch_threads))
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
//  if (use_reactor                      &&
//      (number_of_dispatch_threads > 1) &&
//      !use_thread_pool)
//  { // *NOTE*: see also: man (2) select
//    // *TODO*: verify this for MS Windows based systems
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("the select()-based reactor is not reentrant, using the thread-pool reactor instead...\n")));
//    use_thread_pool = true;
//  } // end IF
  if ((gtk_glade_file.empty () &&
       !Common_File_Tools::isValidFilename (output_file))                  ||
      (!gtk_glade_file.empty () &&
       !Common_File_Tools::isReadable (gtk_glade_file))
      //(!gtk_rc_file_name.empty () &&
      // !Common_File_Tools::isReadable (gtk_rc_file_name))                   ||
//      (use_thread_pool && !use_reactor)                                    ||
//      (use_reactor && (number_of_dispatch_threads > 1) && !use_thread_pool)
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
  if (number_of_dispatch_threads == 0)
    number_of_dispatch_threads = 1;

//  ACE_SYNCH_RECURSIVE_MUTEX* lock_2 = NULL;
  struct Test_I_AVStream_UI_CBData* ui_cb_data_p = NULL;
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
//  lock_2 = &state_r.subscribersLock;
#endif // GTK_USE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_I_AVStream_Server_DirectShow_UI_CBData directshow_ui_cb_data;
  struct Test_I_AVStream_Server_MediaFoundation_UI_CBData mediafoundation_ui_cb_data;
  struct Test_I_AVStream_Server_DirectShow_Configuration directshow_configuration;
  struct Test_I_AVStream_Server_MediaFoundation_Configuration mediafoundation_configuration;
#if defined (GTK_USE)
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
#endif // GTK_USE
  switch (media_framework_e)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      //directshow_ui_cb_data.progressData.state = &directshow_ui_cb_data;
      ui_cb_data_p = &directshow_ui_cb_data;
      directshow_ui_cb_data.configuration = &directshow_configuration;
#if defined (GTK_USE)
      directshow_ui_cb_data.UIState = &state_r;
      directshow_configuration.GTKConfiguration.argc = argc_in;
      directshow_configuration.GTKConfiguration.argv = argv_in;
      directshow_configuration.GTKConfiguration.CBData = ui_cb_data_p;
      directshow_configuration.GTKConfiguration.eventHooks.finiHook =
        idle_finalize_target_UI_cb;
      directshow_configuration.GTKConfiguration.eventHooks.initHook =
        idle_initialize_target_UI_cb;
      directshow_configuration.GTKConfiguration.definition = &gtk_ui_definition;
      directshow_configuration.GTKConfiguration.RCFiles.push_back (gtk_rc_file);
#endif // GTK_USE
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //mediafoundation_ui_cb_data.progressData.state =
      //  &mediafoundation_ui_cb_data;
      ui_cb_data_p = &mediafoundation_ui_cb_data;
      mediafoundation_ui_cb_data.configuration = &mediafoundation_configuration;
#if defined (GTK_USE)
      mediafoundation_ui_cb_data.UIState = &state_r;
      mediafoundation_configuration.GTKConfiguration.argc = argc_in;
      mediafoundation_configuration.GTKConfiguration.argv = argv_in;
      mediafoundation_configuration.GTKConfiguration.CBData = ui_cb_data_p;
      mediafoundation_configuration.GTKConfiguration.eventHooks.finiHook =
        idle_finalize_target_UI_cb;
      mediafoundation_configuration.GTKConfiguration.eventHooks.initHook =
        idle_initialize_target_UI_cb;
      mediafoundation_configuration.GTKConfiguration.definition = &gtk_ui_definition;
      mediafoundation_configuration.GTKConfiguration.RCFiles.push_back (gtk_rc_file);
#endif // GTK_USE
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  media_framework_e));
            
      Common_Tools::finalize ();
      // *PORTABILITY*: on Windows, finalize ACE...
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
      return EXIT_FAILURE;
    }
  } // end SWITCH
  ui_cb_data_p->mediaFramework = media_framework_e;
#else
  struct Test_I_AVStream_Server_UI_CBData ui_cb_data;
  struct Test_I_AVStream_Server_Configuration configuration;
  ui_cb_data.configuration = &configuration;
  ui_cb_data_p = &ui_cb_data;
#if defined (GTK_USE)
  ui_cb_data.UIState = &state_r;
  Common_UI_GtkBuilderDefinition_t gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.argc = argc_in;
  ui_cb_data.configuration->GTKConfiguration.argv = argv_in;
  ui_cb_data.configuration->GTKConfiguration.CBData = &ui_cb_data;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.finiHook =
      idle_finalize_target_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.eventHooks.initHook =
      idle_initialize_target_UI_cb;
  ui_cb_data.configuration->GTKConfiguration.definition = &gtk_ui_definition;
  ui_cb_data.configuration->GTKConfiguration.RCFiles.push_back (gtk_rc_file);
#endif // GTK_USE
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (ui_cb_data_p);

  // step1d: initialize logging and/or tracing
  Common_Logger_Queue_t logger;
  logger.initialize (&state_r.logQueue,
                     &state_r.logQueueLock);
  std::string log_file_name;
  if (log_to_file)
    log_file_name =
      Common_Log_Tools::getLogFilename (ACE_TEXT_ALWAYS_CHAR (ACENetwork_PACKAGE_NAME),
                                        ACE::basename (argv_in[0]));
  if (!Common_Log_Tools::initialize (ACE::basename (argv_in[0]),           // program name
                                     log_file_name,                        // log file name
                                     false,                                // log to syslog ?
                                     false,                                // trace messages ?
                                     trace_information,                    // debug messages ?
                                     NULL))                                // (ui-) logger ?
//                                            (gtk_glade_file.empty () ? NULL
//                                                                     : &logger))) // (ui-) logger ?
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_AVStream_Server_DirectShow_SignalHandler_t directshow_signal_handler;
  Test_I_AVStream_Server_MediaFoundation_SignalHandler_t mediafoundation_signal_handler;
#else
  Test_I_AVStream_Server_SignalHandler_t signal_handler;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (true, // allow SIGUSR1/SIGBREAK
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
  if (!Common_OS_Tools::setResourceLimits (true,  // file descriptors
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

#if defined (GTK_USE)
  // step1h: initialize GLIB / G(D|T)K[+] / GNOME ?
  bool result_2 = false;
  if (gtk_glade_file.empty ())
    goto continue_;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (media_framework_e)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result_2 =
        gtk_manager_p->initialize (directshow_configuration.GTKConfiguration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      result_2 =
        gtk_manager_p->initialize (mediafoundation_configuration.GTKConfiguration);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  media_framework_e));
            
      Common_Tools::finalize ();
      // *PORTABILITY*: on Windows, finalize ACE...
      result = ACE::fini ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
      return EXIT_FAILURE;
    }
  } // end SWITCH
#else
  result_2 =
    gtk_manager_p->initialize (configuration.GTKConfiguration);
#endif // ACE_WIN32 || ACE_WIN64
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
#endif // GTK_USE

#if defined (GTK_USE)
continue_:
#endif // GTK_USE
  ACE_High_Res_Timer timer;
  timer.start ();
  // step2: do actual work
  do_work (maximum_number_of_connections,
           output_file,
           gtk_glade_file,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           media_framework_e,
#endif // ACE_WIN32 || ACE_WIN64
           network_interface,
           use_loopback,
           listening_port_number,
           use_reactor,
           statistic_reporting_interval,
           use_UDP,
           number_of_dispatch_threads,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           mediafoundation_ui_cb_data,
           directshow_ui_cb_data,
#else
           ui_cb_data,
#endif // ACE_WIN32 || ACE_WIN64
           signal_set,
           ignored_signal_set,
           previous_signal_actions,
           previous_signal_mask,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
           directshow_signal_handler,
           mediafoundation_signal_handler);
#else
           signal_handler);
#endif // ACE_WIN32 || ACE_WIN64
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
