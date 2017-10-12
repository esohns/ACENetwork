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

#include "net_server_common_tools.h"

#include <sstream>

#include "ace/ACE.h"
#include "ace/Dirent_Selector.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_defines.h"
#include "common_file_tools.h"

#include "net_macros.h"

#include "net_server_defines.h"

// initialize statics
unsigned int
Net_Server_Common_Tools::maximumNumberOfLogFiles_ =
  NET_SERVER_LOG_MAXIMUM_NUMBER_OF_FILES;

int
Net_Server_Common_Tools::selector (const dirent* dirEntry_in)
{
  //NETWORK_TRACE (ACE_TEXT ("Net_Server_Common_Tools::selector"));

  // *IMPORTANT NOTE*: select only files following the naming schema for
  //                   log files: "<PREFIX>[_<NUMBER>]<SUFFIX>"

  // sanity check --> prefix ok ?
  if (likely (ACE_OS::strncmp (dirEntry_in->d_name,
                               ACE_TEXT_ALWAYS_CHAR (NET_SERVER_LOG_FILENAME_PREFIX),
                               ACE_OS::strlen (ACE_TEXT_ALWAYS_CHAR (NET_SERVER_LOG_FILENAME_PREFIX)))))
    return 0;

  return 1;
}

int
Net_Server_Common_Tools::comparator (const dirent** d1,
                                     const dirent** d2)
{
  //NETWORK_TRACE (ACE_TEXT ("Net_Server_Common_Tools::comparator"));

  return ACE_OS::strcmp ((*d1)->d_name,
                         (*d2)->d_name);
}

std::string
Net_Server_Common_Tools::getNextLogFileName (const std::string& packageName_in,
                                             const std::string& programName_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Server_Common_Tools::getNextLogFileName"));

  // sanity check(s)
  ACE_ASSERT (!programName_in.empty ());

  std::string result;
  int result_2 = -1;
  std::string directory;
  unsigned int fallback_level = 0;

fallback:
  directory = Common_File_Tools::getLogDirectory (packageName_in,
                                                  fallback_level);
  if (directory.empty ())
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to Common_File_Tools::getLogDirectory(\"%s\",%d), falling back\n"),
                ACE_TEXT (packageName_in.c_str ()),
                fallback_level));

    directory = Common_File_Tools::getWorkingDirectory ();
    if (directory.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::getWorkingDirectory(), aborting\n")));
      return std::string ();
    } // end IF
    std::string filename = Common_File_Tools::getTempFilename (programName_in);
    if (filename.empty ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::getTempFilename(\"%s\"), aborting\n"),
                  ACE_TEXT (programName_in.c_str ())));
      return result;
    } // end IF
    result = directory;
    result += ACE_DIRECTORY_SEPARATOR_STR;
    result +=
        ACE_TEXT_ALWAYS_CHAR (ACE::basename (ACE_TEXT (filename.c_str ()),
                                             ACE_DIRECTORY_SEPARATOR_CHAR));
    result += COMMON_LOG_FILENAME_SUFFIX;

    return result;
  } // end IF

  // sanity check(s): log directory exists ?
  // --> (try to) create it
  if (!Common_File_Tools::isDirectory (directory))
  {
    if (!Common_File_Tools::createDirectory (directory))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::createDirectory(\"%s\"), aborting\n"),
                  ACE_TEXT (directory.c_str ())));
      return std::string ();
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("created directory: \"%s\"...\n"),
                ACE_TEXT (directory.c_str ())));
  } // end IF

  // assemble correct log file name
  result = programName_in;
  result += COMMON_LOG_FILENAME_SUFFIX;

  // retrieve all existing logs and sort them alphabetically
  ACE_Dirent_Selector entries;
  result_2 = entries.open (ACE_TEXT (directory.c_str ()),
                           &Net_Server_Common_Tools::selector,
                           &Net_Server_Common_Tools::comparator);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Dirent_Selector::open(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (directory.c_str ())));
    return std::string ();
  } // end IF

  //   ACE_DEBUG ((LM_DEBUG,
  //               ACE_TEXT ("found %d logfiles...\n"),
  //               entries.length ()));

  // OK: iterate over the entries and perform some magic...
  // *NOTE*: entries have been sorted alphabetically:
  //         1 current 2 4 3 --> current 1 2 3 4
  // *TODO*: someone/thing could generate "fake" log files which would "confuse"
  //         this algorithm
  // skip handling of "<PREFIX><SUFFIX>" (if found)...
  // *NOTE*: <PREFIX><SUFFIX> will become <PREFIX>_1<SUFFIX>...
  bool found_current = false;
  // sscanf settings
  unsigned int number = 0;
  std::string format_string ("%u");
  format_string += ACE_TEXT_ALWAYS_CHAR (COMMON_LOG_FILENAME_SUFFIX);
  std::stringstream converter;
  std::string old_FQ_file_name, new_FQ_file_name;
  unsigned int index = Net_Server_Common_Tools::maximumNumberOfLogFiles_ - 1;
  for (int i = entries.length () - 1;
       i >= 0;
       i--)
  {
    // perform "special treatment" if "<PREFIX><SUFFIX>" found
    result_2 = ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (entries[i]->d_name),
                               result.c_str ());
    if (result_2 == 0)
    {
      found_current = true;

      // skip this one for now (see below)
      continue;
    } // end IF

    // extract number...
    try
    {
      // *TODO*: do this in C++...
      result_2 =
        ::sscanf (ACE_TEXT_ALWAYS_CHAR (entries[i]->d_name) + programName_in.size (),
                  format_string.c_str (),
                  &number);
      if (result_2 != 1)
      {
        if (result_2)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("::sscanf() failed for \"%s\": \"%m\", continuing\n"),
                      ACE_TEXT (entries[i]->d_name)));
        continue;
      } // end IF
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in ::sscanf() for \"%s\": \"%m\", aborting\n"),
                  ACE_TEXT (entries[i]->d_name)));
      return std::string ();
    }

    // adjust the index if the number is lower than the maximum
    if (number < index) index = number + 1;

    // if the number is higher than the maximum AND there are more than enough
    // log files, delete it
    if ((static_cast<unsigned int> (number) >=
         (Net_Server_Common_Tools::maximumNumberOfLogFiles_ - 1)) &&
        (static_cast<unsigned int> (entries.length ()) >=
         Net_Server_Common_Tools::maximumNumberOfLogFiles_))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("removing obsolete file \"%s\"...\n"),
                  ACE_TEXT (entries[i]->d_name)));

      // clean up
      std::string FQFileName = directory;
      FQFileName += ACE_DIRECTORY_SEPARATOR_CHAR_A;
      FQFileName += entries[i]->d_name;
      Common_File_Tools::deleteFile (FQFileName);

      continue;
    } // end IF

    // logrotate file...
    old_FQ_file_name = directory;
    old_FQ_file_name += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    old_FQ_file_name += entries[i]->d_name;

    new_FQ_file_name = directory;
    new_FQ_file_name += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    new_FQ_file_name += ACE_TEXT_ALWAYS_CHAR (NET_SERVER_LOG_FILENAME_PREFIX);
    new_FQ_file_name += ACE_TEXT_ALWAYS_CHAR ("_");

    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << index;

    new_FQ_file_name += converter.str ();
    new_FQ_file_name += ACE_TEXT_ALWAYS_CHAR (COMMON_LOG_FILENAME_SUFFIX);
    // *NOTE*: last parameter affects Win32 behaviour only,
    //         see ace/OS_NS_stdio.inl
    result_2 = ACE_OS::rename (old_FQ_file_name.c_str (),
                               new_FQ_file_name.c_str (),
                               -1);
    if (result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::rename() \"%s\" to \"%s\": \"%m\", aborting\n"),
                  ACE_TEXT (old_FQ_file_name.c_str ()),
                  ACE_TEXT (new_FQ_file_name.c_str ())));
      return std::string ();
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("renamed file \"%s\" to \"%s\"...\n"),
                ACE_TEXT (old_FQ_file_name.c_str ()),
                ACE_TEXT (new_FQ_file_name.c_str ())));

    index--;
  } // end FOR

  directory += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  result = directory + result;

  if (found_current)
  {
    new_FQ_file_name = directory;
    new_FQ_file_name += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    new_FQ_file_name += ACE_TEXT_ALWAYS_CHAR (NET_SERVER_LOG_FILENAME_PREFIX);
    new_FQ_file_name += ACE_TEXT_ALWAYS_CHAR ("_1");
    new_FQ_file_name += ACE_TEXT_ALWAYS_CHAR (COMMON_LOG_FILENAME_SUFFIX);

    // *NOTE*: last parameter affects Win32 behaviour only
    //         see ace/OS_NS_stdio.inl
    result_2 = ACE_OS::rename (result.c_str (),
                               new_FQ_file_name.c_str (),
                               -1);
    if (result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::rename() \"%s\" to \"%s\": \"%m\", aborting\n"),
                  ACE_TEXT (result.c_str ()),
                  ACE_TEXT (new_FQ_file_name.c_str ())));
      return std::string ();
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("renamed file \"%s\" to \"%s\"...\n"),
                ACE_TEXT (result.c_str ()),
                ACE_TEXT (new_FQ_file_name.c_str ())));
  } // end IF

  // sanity check(s)
  if (!Common_File_Tools::create (result))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to Common_File_Tools::create(\"%s\"), falling back\n"),
                ACE_TEXT (result.c_str ())));

    ++fallback_level;

    goto fallback;
  } // end IF
  if (!Common_File_Tools::deleteFile (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::deleteFile(\"%s\"), aborting\n"),
                ACE_TEXT (result.c_str ())));
    return std::string ();
  } // end IF

  return result;
}

//Net_IListener_t*
//Net_Server_Common_Tools::getListenerSingleton ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Server_Common_Tools::getListenerSingleton"));

//  return NET_SERVER_LISTENER_SINGLETON::instance ();
//}

//Net_IListener_t*
//Net_Server_Common_Tools::getAsynchListenerSingleton ()
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_Server_Common_Tools::getAsynchListenerSingleton"));

//  return NET_SERVER_ASYNCHLISTENER_SINGLETON::instance ();
//}
