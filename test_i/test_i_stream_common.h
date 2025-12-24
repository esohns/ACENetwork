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

#ifndef TEST_I_STREAM_COMMON_H
#define TEST_I_STREAM_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mfidl.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "common_file_common.h"

#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_tools.h"
#include "stream_lib_v4l_defines.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_net_common.h"

#include "stream_dev_common.h"

#include "test_i_common.h"

// forward declarations
struct Net_ConnectionState;

struct Test_I_MessageData
{
  Test_I_MessageData ()
  {}

  struct Test_I_MessageData& operator+= (const struct Test_I_MessageData& rhs_in)
  {
    return *this;
  }
};
typedef Stream_DataBase_T<struct Test_I_MessageData> Test_I_MessageData_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_MessageData
 : Test_I_MessageData
{
  Test_I_DirectShow_MessageData ()
   : Test_I_MessageData ()
   , sample (NULL)
   , sampleTime (0)
  {}

  // audio/video
  IMediaSample* sample;
  double        sampleTime;
};
typedef Stream_DataBase_T<struct Test_I_DirectShow_MessageData> Test_I_DirectShow_MessageData_t;

struct Test_I_MediaFoundation_MessageData
 : Test_I_MessageData
{
  Test_I_MediaFoundation_MessageData ()
   : Test_I_MessageData ()
   , sample (NULL)
   , sampleTime (0)
  {}

  // audio/video
  IMFSample* sample;
  LONGLONG   sampleTime;
};
typedef Stream_DataBase_T<struct Test_I_MediaFoundation_MessageData> Test_I_MediaFoundation_MessageData_t;
#else
struct Test_I_ALSA_MessageData
 : Test_I_MessageData
{
  Test_I_ALSA_MessageData ()
   : Test_I_MessageData ()
   , deviceHandle (NULL)
   , release (false)
  {}

  struct _snd_pcm* deviceHandle; // (capture) device handle
  bool             release;
};

struct Test_I_V4L_MessageData
 : Test_I_MessageData
{
  Test_I_V4L_MessageData ()
   : Test_I_MessageData ()
   , fileDescriptor (-1)
   , index (0)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {}

  int         fileDescriptor; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};
typedef Stream_DataBase_T<struct Test_I_V4L_MessageData> Test_I_V4L_MessageData_t;
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_StreamSessionData
 : Stream_SessionData
{
  Test_I_StreamSessionData ()
   : Stream_SessionData ()
   , connection (NULL)
   , connectionStates ()
   , fileIdentifier ()
  {}

  struct Test_I_StreamSessionData& operator+= (const struct Test_I_StreamSessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    connection = (connection ? connection : rhs_in.connection);
    connectionStates.insert (rhs_in.connectionStates.begin (),
                             rhs_in.connectionStates.end ());
    fileIdentifier =
      (fileIdentifier.empty () ? rhs_in.fileIdentifier : fileIdentifier);

    return *this;
  }

  Net_IINETConnection_t*        connection;
  Stream_Net_ConnectionStates_t connectionStates;
  struct Common_File_Identifier fileIdentifier; // target-
};
typedef Stream_SessionData_T<struct Test_I_StreamSessionData> Test_I_StreamSessionData_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_DirectShow_StreamSessionDataMedia
 : public Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                        struct _AMMediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_DirectShow_StreamSessionDataMedia ()
   : Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                   struct _AMMediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_DirectShow_StreamSessionDataMedia& operator+= (const Test_I_DirectShow_StreamSessionDataMedia& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                  struct _AMMediaType,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_DirectShow_StreamSessionDataMedia> Test_I_DirectShow_StreamSessionDataMedia_t;

class Test_I_MediaFoundation_StreamSessionDataMedia
 : public Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                        IMFMediaType*,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_MediaFoundation_StreamSessionDataMedia ()
   : Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                   IMFMediaType*,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , session (NULL)
  {}

  Test_I_MediaFoundation_StreamSessionDataMedia& operator+= (const Test_I_MediaFoundation_StreamSessionDataMedia& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                  IMFMediaType*,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }

  IMFMediaSession* session;
};
typedef Stream_SessionData_T<Test_I_MediaFoundation_StreamSessionDataMedia> Test_I_MediaFoundation_StreamSessionDataMedia_t;
#else
class Test_I_ALSA_StreamSessionDataMedia
 : public Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                        struct Stream_MediaFramework_ALSA_MediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_ALSA_StreamSessionDataMedia ()
   : Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                   struct Stream_MediaFramework_ALSA_MediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_ALSA_StreamSessionDataMedia& operator+= (const Test_I_ALSA_StreamSessionDataMedia& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                  struct Stream_MediaFramework_ALSA_MediaType,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_ALSA_StreamSessionDataMedia> Test_I_ALSA_StreamSessionDataMedia_t;

class Test_I_V4L_StreamSessionDataMedia
 : public Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                        struct Stream_MediaFramework_V4L_MediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_V4L_StreamSessionDataMedia ()
   : Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                   struct Stream_MediaFramework_V4L_MediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_V4L_StreamSessionDataMedia& operator+= (const Test_I_V4L_StreamSessionDataMedia& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_StreamSessionData,
                                  struct Stream_MediaFramework_V4L_MediaType,
                                  struct Stream_State,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_V4L_StreamSessionDataMedia> Test_I_V4L_StreamSessionDataMedia_t;
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_StreamState
 : Stream_State
{
  Test_I_StreamState ()
   : Stream_State ()
   //, sessionData (NULL)
  {};

  //struct Test_I_StreamSessionData* sessionData;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_StreamState
 : Stream_State
{
  Test_I_DirectShow_StreamState ()
   : Stream_State ()
   //, sessionData (NULL)
  {}

  //Test_I_DirectShow_StreamSessionData* sessionData;
};

struct Test_I_MediaFoundation_StreamState
 : Stream_State
{
  Test_I_MediaFoundation_StreamState ()
   : Stream_State ()
   //, sessionData (NULL)
  {}

  //Test_I_MediaFoundation_StreamSessionData* sessionData;
};
#else
struct Test_I_ALSA_StreamState
 : Stream_State
{
  Test_I_ALSA_StreamState ()
   : Stream_State ()
   //, sessionData (NULL)
  {}

  //Test_I_ALSA_StreamSessionData* sessionData;
};

struct Test_I_V4L_StreamState
 : Stream_State
{
  Test_I_V4L_StreamState ()
   : Stream_State ()
  //, sessionData (NULL)
  {}

  // Test_I_V4L_StreamSessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_ModuleHandlerConfiguration
 : virtual Stream_ModuleHandlerConfiguration
{
  Test_I_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , fileIdentifier ()
   , printProgressDot (false)
  {}

  struct Common_File_Identifier fileIdentifier; // source-/target-
  bool                          printProgressDot;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , builder (NULL)
   , deviceIdentifier ()
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outputFormat ()
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   , sampleIsDataMessage (false)
  {
    ACE_OS::memset (&outputFormat, 0, sizeof (struct _AMMediaType));
  }

  IGraphBuilder*                                               builder;
  struct Stream_Device_Identifier                              deviceIdentifier;
  struct Stream_MediaFramework_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                        filterCLSID;
  struct _AMMediaType                                          outputFormat;
  // *IMPORTANT NOTE*: 'asynchronous' filters implement IAsyncReader (downstream
  //                   filters 'pull' media samples), 'synchronous' filters
  //                   implement IMemInputPin and 'push' media samples to
  //                   downstream filters
  bool                                                         push;
  bool                                                         sampleIsDataMessage;
};

struct Test_I_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , deviceIdentifier ()
   , manageMediaSession (false)
   , mediaFoundationConfiguration (NULL)
   , outputFormat (NULL)
   , session (NULL)
  {}

  struct Stream_Device_Identifier                             deviceIdentifier;
  bool                                                        manageMediaSession;
  struct Stream_MediaFramework_MediaFoundation_Configuration* mediaFoundationConfiguration;
  IMFMediaType*                                               outputFormat;
  IMFMediaSession*                                            session;
};
#else
struct Test_I_ALSA_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_ALSA_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , ALSAConfiguration (NULL)
   , deviceIdentifier ()
   , outputFormat ()
  {
    deviceIdentifier.identifier =
      Stream_MediaFramework_ALSA_Tools::getDeviceName (STREAM_LIB_ALSA_DEVICE_DEFAULT,
                                                       SND_PCM_STREAM_PLAYBACK);
  }

  struct Stream_MediaFramework_ALSA_Configuration* ALSAConfiguration;
  struct Stream_Device_Identifier                  deviceIdentifier;
  struct Stream_MediaFramework_ALSA_MediaType      outputFormat;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_StreamConfiguration
 : Stream_Configuration
{
  Test_I_StreamConfiguration ()
   : Stream_Configuration ()
   , fileIdentifier ()
  {}

  struct Common_File_Identifier fileIdentifier; // target-
};

#endif
