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

#include "net_os_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DBUS_SUPPORT)
#include "dbus/dbus.h"
#endif // DBUS_SUPPORT
#include "linux/capability.h"
#endif // ACE_WIN32 || ACE_WIN64

#include <regex>
#include <string>

#include "ace/Configuration.h"
#include "ace/Configuration_Import_Export.h"
#include "ace/SStringfwd.h"

#include "common_file_tools.h"
#include "common_string_tools.h"
#include "common_tools.h"
#include "common_process_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DBUS_SUPPORT)
#include "common_dbus_defines.h"
#include "common_dbus_tools.h"
#endif // DBUS_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "net_common_tools.h"
#include "net_defines.h"
#include "net_macros.h"

#include "net_wlan_defines.h"
#include "net_wlan_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//bool
//Net_OS_Tools::ifUpDownManageInterface (const std::string& interfaceIdentifier_in,
//                                           bool toggle_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("Net_OS_Tools::ifUpDownManageInterface"));

//  // sanity check(s)
//  ACE_ASSERT (!interfaceIdentifier_in.empty ());
//  std::string configuration_file_path =
//      ACE_TEXT_ALWAYS_CHAR ("/etc/NetworkManager/NetworkManager.conf");
//  if (unlikely (!Common_File_Tools::isReadable (configuration_file_path)))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("invalid NetworkManager configuration file (was: \"%s\"), aborting\n"),
//                ACE_TEXT (configuration_file_path.c_str ())));
//    return false;
//  } // end IF

//  ACE_Configuration_Heap configuration;
//  int result_2 = configuration.open (ACE_DEFAULT_CONFIG_SECTION_SIZE);
//  if (unlikely (result_2 == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Configuration_Heap::open(%d): \"%m\", aborting\n"),
//                ACE_DEFAULT_CONFIG_SECTION_SIZE));
//    return false;
//  } // end IF
//  ACE_Ini_ImpExp ini_importer (configuration);
//  result_2 =
//      ini_importer.import_config (ACE_TEXT (configuration_file_path.c_str ()));
//  if (unlikely (result_2 == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Ini_ImpExp::import_config(\"%s\"): \"%m\", aborting\n"),
//                ACE_TEXT (configuration_file_path.c_str ())));
//    return false;
//  } // end IF
//  ACE_TString sub_section_string, value_name_string, value_string;
//  int index = 0, index_2 = 0;
//  const ACE_Configuration_Section_Key& root_section =
//      configuration.root_section ();
//  ACE_Configuration_Section_Key sub_section_key;
//  enum ACE_Configuration::VALUETYPE value_type_e;
//  std::string regex_string = ACE_TEXT_ALWAYS_CHAR ("^(?:(.+),)*(.*)$");
//  std::string regex_string_2 =
//      ACE_TEXT_ALWAYS_CHAR ("^(?:(?:mac:((?:[01234567890abcdef]{2}:){5}[01234567890abcdef]{2})|interface-name:(.+));)*(?:mac:((?:[01234567890abcdef]{2}:){5}[01234567890abcdef]{2})|interface-name:(.+))?$");
//  std::regex regex;
//  std::smatch smatch;
//  std::string value_string_2, value_string_3;
//  std::string::size_type position = std::string::npos;
//  std::smatch::iterator iterator;
//  bool section_found_b = false, value_found_b = false, modified_b = false;

//  // step1: make sure the 'keyfile' plugin is loaded
//  try {
//    regex.assign (regex_string,
//                  std::regex::ECMAScript);
//  } catch (std::regex_error& error) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to std::regex::assign(\"%s\"): \"%s\", aborting\n"),
//                ACE_TEXT (regex_string.c_str ()),
//                ACE_TEXT (error.what ())));
//    return false;
//  }
//  do
//  {
//    result_2 = configuration.enumerate_sections (root_section,
//                                                 index,
//                                                 sub_section_string);
//    if (result_2)
//      break;
//    if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (sub_section_string.c_str ()),
//                        ACE_TEXT_ALWAYS_CHAR ("main")))
//      goto next;

//    section_found_b = true;
//    result_2 = configuration.open_section (root_section,
//                                           sub_section_string.c_str (),
//                                           0,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    do
//    {
//      result_2 = configuration.enumerate_values (sub_section_key,
//                                                 index_2,
//                                                 value_name_string,
//                                                 value_type_e);
//      if (result_2)
//        break;
//      if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (value_name_string.c_str ()),
//                          ACE_TEXT_ALWAYS_CHAR ("plugins")))
//        goto next_2;

//      value_found_b = true;
//      result_2 = configuration.get_string_value (sub_section_key,
//                                                 value_name_string.c_str (),
//                                                 value_string);
//      ACE_ASSERT (result_2 == 0);
//      value_string_2 = value_string.c_str ();
//      if (!std::regex_match (value_string_2,
//                             smatch,
//                             regex,
//                             std::regex_constants::match_default))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to std::regex_match(\"%s\"), aborting\n"),
//                    ACE_TEXT (configuration_file_path.c_str ())));
//        return false;
//      } // end IF
//      for (iterator = ++smatch.begin ();
//           iterator != smatch.end ();
//           ++iterator)
//      { ACE_ASSERT ((*iterator).matched);
//        value_string_2 = (*iterator).str ();
//        if (!ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR ("keyfile"),
//                             value_string_2.c_str ()))
//          break;
//      } // end FOR
//      if ((iterator == smatch.end ()) &&
//          toggle_in)
//      {
//        if (!value_string.empty ())
//          value_string += ACE_TEXT_ALWAYS_CHAR (",");
//        value_string += ACE_TEXT_ALWAYS_CHAR ("keyfile");
//        result_2 = configuration.set_string_value (sub_section_key,
//                                                   value_name_string.c_str (),
//                                                   value_string);
//        ACE_ASSERT (result_2 == 0);
//        modified_b = true;
//      } // end IF

//next_2:
//      ++index_2;
//    } while (true);
//    if (unlikely (result_2 == -1))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Configuration::enumerate_values(\"%s\"): \"%m\", aborting\n"),
//                  sub_section_string.c_str ()));
//      return false;
//    } // end IF

//    break;

//next:
//    ++index;
//  } while (true);
//  if (unlikely (result_2 == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Configuration::enumerate_sections(): \"%m\", aborting\n")));
//    return false;
//  } // end IF
//  if (unlikely (!section_found_b))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s: section \"%s\" not found, creating\n"),
//                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
//                ACE_TEXT ("main")));
//    result_2 = configuration.open_section (root_section,
//                                           ACE_TEXT_ALWAYS_CHAR ("main"),
//                                           1,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    result_2 =
//        configuration.set_string_value (sub_section_key,
//                                        ACE_TEXT_ALWAYS_CHAR ("plugins"),
//                                        ACE_TEXT_ALWAYS_CHAR ("keyfile"));
//    ACE_ASSERT (result_2 == 0);
//    modified_b = true;
//  } // end IF
//  else if (unlikely (!value_found_b))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s[%s]: value \"%s\" not found, creating\n"),
//                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
//                ACE_TEXT ("main"),
//                ACE_TEXT ("plugins")));
//    result_2 = configuration.open_section (root_section,
//                                           ACE_TEXT_ALWAYS_CHAR ("main"),
//                                           0,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    result_2 =
//        configuration.set_string_value (sub_section_key,
//                                        ACE_TEXT_ALWAYS_CHAR ("plugins"),
//                                        ACE_TEXT_ALWAYS_CHAR ("keyfile"));
//    ACE_ASSERT (result_2 == 0);
//    modified_b = true;
//  } // end ELSE IF

//  // step2: add/remove 'unmanaged-devices' entry to/from the 'keyfile' section
//  index = 0, index_2 = 0;
//  section_found_b = false, value_found_b = false;
//  try {
//    regex.assign (regex_string_2,
//                  std::regex::ECMAScript);
//  } catch (std::regex_error& error) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to std::regex::assign(\"%s\"): \"%s\", aborting\n"),
//                ACE_TEXT (regex_string_2.c_str ()),
//                ACE_TEXT (error.what ())));
//    return false;
//  }
//  do
//  {
//    result_2 = configuration.enumerate_sections (root_section,
//                                                 index,
//                                                 sub_section_string);
//    if (result_2)
//      break;
//    if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (sub_section_string.c_str ()),
//                        ACE_TEXT_ALWAYS_CHAR ("keyfile")))
//      goto next_3;

//    section_found_b = true;
//    result_2 = configuration.open_section (root_section,
//                                           sub_section_string.c_str (),
//                                           0,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    do
//    {
//      result_2 = configuration.enumerate_values (sub_section_key,
//                                                 index_2,
//                                                 value_name_string,
//                                                 value_type_e);
//      if (result_2)
//        break;
//      if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (value_name_string.c_str ()),
//                          ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices")))
//        goto next_4;

//      value_found_b = true;
//      result_2 = configuration.get_string_value (sub_section_key,
//                                                 value_name_string.c_str (),
//                                                 value_string);
//      ACE_ASSERT (result_2 == 0);
//      value_string_2 = value_string.c_str ();
//      if (!std::regex_match (value_string_2,
//                             smatch,
//                             regex,
//                             std::regex_constants::match_default))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to std::regex_match(\"%s\"), aborting\n"),
//                    ACE_TEXT (configuration_file_path.c_str ())));
//        return false;
//      } // end IF
//      for (iterator = ++smatch.begin ();
//           iterator != smatch.end ();
//           ++iterator)
//      { ACE_ASSERT ((*iterator).matched);
//        value_string_2 = (*iterator).str ();
//        position = value_string_2.find (':', 0);
//        if (position == 2)
//        {
//          struct ether_addr ether_addr_s =
//              Net_Common_Tools::stringToLinkLayerAddress (value_string_2);
//          value_string_2 =
//              Net_Common_Tools::linkLayerAddressToInterfaceIdentifier (ether_addr_s);
//          ACE_ASSERT (!value_string_2.empty ());
//        } // end IF
//        if (!ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
//                             value_string_2.c_str ()))
//          break;
//      } // end FOR
//      if (toggle_in)
//      {
//        if (iterator != smatch.end ())
//        {
//          value_string.clear ();
//          for (std::smatch::iterator iterator_2  = ++smatch.begin ();
//               iterator_2 != smatch.end ();
//               ++iterator_2)
//          {
//            if (iterator_2 == iterator)
//              continue;
//            if (iterator != ++smatch.begin ())
//              value_string += ACE_TEXT_ALWAYS_CHAR (";");
//            value_string_3 = (*iterator_2).str ();
//            position = value_string_3.find (':', 0);
//            if (position == 2)
//              value_string += ACE_TEXT_ALWAYS_CHAR ("mac:");
//            else
//              value_string += ACE_TEXT_ALWAYS_CHAR ("interface-name:");
//            value_string += value_string_3.c_str ();
//          } // end FOR

//          result_2 =
//              configuration.set_string_value (sub_section_key,
//                                              ACE_TEXT ("unmanaged-devices"),
//                                              value_string);
//          ACE_ASSERT (result_2 == 0);
//          modified_b = true;
//        } // end IF
//      } // end IF
//      else
//      {
//        if (iterator == smatch.end ())
//        {
//          if (!value_string.empty ())
//            value_string += ACE_TEXT_ALWAYS_CHAR (";");
//          value_string += ACE_TEXT_ALWAYS_CHAR ("interface-name:");
//          value_string += interfaceIdentifier_in.c_str ();
//          result_2 =
//              configuration.set_string_value (sub_section_key,
//                                              ACE_TEXT ("unmanaged-devices"),
//                                              value_string);
//          ACE_ASSERT (result_2 == 0);
//          modified_b = true;
//        } // end IF
//      } // end ELSE

//next_4:
//      ++index_2;
//    } while (true);
//    if (unlikely (result_2 == -1))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Configuration::enumerate_values(\"%s\"): \"%m\", aborting\n"),
//                  sub_section_string.c_str ()));
//      return false;
//    } // end IF

//next_3:
//    ++index;
//  } while (true);
//  if (unlikely (result_2 == -1))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Configuration::enumerate_sections(): \"%m\", aborting\n")));
//    return false;
//  } // end IF
//  if (unlikely (!section_found_b))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s: section \"%s\" not found, creating\n"),
//                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
//                ACE_TEXT ("keyfile")));
//    result_2 = configuration.open_section (root_section,
//                                           ACE_TEXT_ALWAYS_CHAR ("keyfile"),
//                                           1,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    if (toggle_in)
//      value_string.clear ();
//    else
//    {
//      value_string = ACE_TEXT_ALWAYS_CHAR ("interface-name:");
//      value_string += interfaceIdentifier_in.c_str ();
//    } // end ELSE
//    result_2 =
//        configuration.set_string_value (sub_section_key,
//                                        ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices"),
//                                        value_string);
//    ACE_ASSERT (result_2 == 0);
//    modified_b = true;
//  } // end IF
//  else if (unlikely (!value_found_b))
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s[%s]: value \"%s\" not found, creating\n"),
//                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
//                ACE_TEXT ("keyfile"),
//                ACE_TEXT ("unmanaged-devices")));
//    result_2 = configuration.open_section (root_section,
//                                           ACE_TEXT_ALWAYS_CHAR ("keyfile"),
//                                           0,
//                                           sub_section_key);
//    ACE_ASSERT (result_2 == 0);
//    if (toggle_in)
//      value_string.clear ();
//    else
//    {
//      value_string = ACE_TEXT_ALWAYS_CHAR ("interface-name:");
//      value_string += interfaceIdentifier_in.c_str ();
//    } // end ELSE
//    result_2 =
//        configuration.set_string_value (sub_section_key,
//                                        ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices"),
//                                        value_string);
//    ACE_ASSERT (result_2 == 0);
//    modified_b = true;
//  } // end ELSE IF

//  if (modified_b)
//  {
//    bool drop_privileges = false;
//    if (!Common_File_Tools::canWrite (configuration_file_path, static_cast<uid_t> (-1)))
//      drop_privileges = Common_Tools::switchUser (0);

//    if (unlikely (!Common_File_Tools::backup (configuration_file_path)))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Common_File_Tools::backup(\"\"), continuing\n"),
//                  ACE_TEXT (configuration_file_path.c_str ())));

//    result_2 =
//        ini_importer.export_config (ACE_TEXT (configuration_file_path.c_str ()));
//    if (unlikely (result_2 == -1))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Ini_ImpExp::export_config(): \"%m\", aborting\n")));

//      if (drop_privileges)
//        Common_Tools::switchUser (static_cast<uid_t> (-1));

//      return false;
//    } // end IF
//    if (drop_privileges)
//      Common_Tools::switchUser (static_cast<uid_t> (-1));
//  } // end IF

//  return true;
//}

bool
Net_OS_Tools::networkManagerManageInterface (const std::string& interfaceIdentifier_in,
                                             bool toggle_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_OS_Tools::networkManagerManageInterface"));

  std::string configuration_file_path =
      ACE_TEXT_ALWAYS_CHAR ("/etc/NetworkManager/NetworkManager.conf");
  ACE_Configuration_Heap configuration;
  ACE_Ini_ImpExp ini_importer (configuration);
  int result_2 = -1;
  ACE_TString sub_section_string, value_name_string, value_string;
  int index = 0, index_2 = 0;
  const ACE_Configuration_Section_Key& root_section =
      configuration.root_section ();
  ACE_Configuration_Section_Key sub_section_key;
  enum ACE_Configuration::VALUETYPE value_type_e;
  std::string regex_string = ACE_TEXT_ALWAYS_CHAR ("^(?:(.+),)*(.*)$");
  std::string regex_string_2 =
      ACE_TEXT_ALWAYS_CHAR ("^(mac:(?:([01234567890abcdef]{2}):){5}[01234567890abcdef]{2}|interface-name:[^;]+;)*(mac:(?:([01234567890abcdef]{2}):){5}[01234567890abcdef]{2}|interface-name:.+)?$");
  std::regex regex;
  std::smatch smatch;
  std::string value_string_2, value_string_3;
  std::string::size_type position = std::string::npos;
  std::smatch::iterator iterator;
  bool section_found_b = false, value_found_b = false, modified_b = false;
  bool is_managing_interface_b =
      Net_OS_Tools::isNetworkManagerManagingInterface (interfaceIdentifier_in);

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  if ((toggle_in && is_managing_interface_b))// ||
//      (!toggle_in && !is_managing_interface_b))
    return true; // nothing to do
  if (unlikely (!Common_File_Tools::canRead (configuration_file_path)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::canRead(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF

  result_2 = configuration.open (ACE_DEFAULT_CONFIG_SECTION_SIZE);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration_Heap::open(%d): \"%m\", aborting\n"),
                ACE_DEFAULT_CONFIG_SECTION_SIZE));
    return false;
  } // end IF
  result_2 =
      ini_importer.import_config (ACE_TEXT (configuration_file_path.c_str ()));
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Ini_ImpExp::import_config(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF

  // step1: make sure the 'keyfile' plugin is loaded
  try {
    regex.assign (regex_string,
                  std::regex::ECMAScript);
  } catch (std::regex_error& error) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to std::regex::assign(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (regex_string.c_str ()),
                ACE_TEXT (error.what ())));
    return false;
  }
  do
  {
    result_2 = configuration.enumerate_sections (root_section,
                                                 index,
                                                 sub_section_string);
    if (result_2)
      break;
    if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (sub_section_string.c_str ()),
                        ACE_TEXT_ALWAYS_CHAR ("main")))
      goto next;

    section_found_b = true;
    result_2 = configuration.open_section (root_section,
                                           sub_section_string.c_str (),
                                           0,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    do
    {
      result_2 = configuration.enumerate_values (sub_section_key,
                                                 index_2,
                                                 value_name_string,
                                                 value_type_e);
      if (result_2)
        break;
      if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (value_name_string.c_str ()),
                          ACE_TEXT_ALWAYS_CHAR ("plugins")))
        goto next_2;

      value_found_b = true;
      result_2 = configuration.get_string_value (sub_section_key,
                                                 value_name_string.c_str (),
                                                 value_string);
      ACE_ASSERT (result_2 == 0);
      value_string_2 = value_string.c_str ();
      if (!std::regex_match (value_string_2,
                             smatch,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to std::regex_match(\"%s\"), aborting\n"),
                    ACE_TEXT (configuration_file_path.c_str ())));
        return false;
      } // end IF
      for (iterator = ++smatch.begin ();
           iterator != smatch.end ();
           ++iterator)
      { ACE_ASSERT ((*iterator).matched);
        value_string_2 = (*iterator).str ();
        if (!ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR ("keyfile"),
                             value_string_2.c_str ()))
          break;
      } // end FOR
      if ((iterator == smatch.end ()) &&
          toggle_in)
      {
        if (!value_string.empty ())
          value_string += ACE_TEXT_ALWAYS_CHAR (",");
        value_string += ACE_TEXT_ALWAYS_CHAR ("keyfile");
        result_2 = configuration.set_string_value (sub_section_key,
                                                   value_name_string.c_str (),
                                                   value_string);
        ACE_ASSERT (result_2 == 0);
        modified_b = true;
      } // end IF

next_2:
      ++index_2;
    } while (true);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration::enumerate_values(\"%s\"): \"%m\", aborting\n"),
                  sub_section_string.c_str ()));
      return false;
    } // end IF

    break;

next:
    ++index;
  } while (true);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration::enumerate_sections(): \"%m\", aborting\n")));
    return false;
  } // end IF
  if (unlikely (!section_found_b))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: section \"%s\" not found, creating\n"),
                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT ("main")));
    result_2 = configuration.open_section (root_section,
                                           ACE_TEXT_ALWAYS_CHAR ("main"),
                                           1,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    result_2 =
        configuration.set_string_value (sub_section_key,
                                        ACE_TEXT_ALWAYS_CHAR ("plugins"),
                                        ACE_TEXT_ALWAYS_CHAR ("keyfile"));
    ACE_ASSERT (result_2 == 0);
    modified_b = true;
  } // end IF
  else if (unlikely (!value_found_b))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s[%s]: value \"%s\" not found, creating\n"),
                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT ("main"),
                ACE_TEXT ("plugins")));
    result_2 = configuration.open_section (root_section,
                                           ACE_TEXT_ALWAYS_CHAR ("main"),
                                           0,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    result_2 =
        configuration.set_string_value (sub_section_key,
                                        ACE_TEXT_ALWAYS_CHAR ("plugins"),
                                        ACE_TEXT_ALWAYS_CHAR ("keyfile"));
    ACE_ASSERT (result_2 == 0);
    modified_b = true;
  } // end ELSE IF

  // step2: add/remove 'unmanaged-devices' entry to/from the 'keyfile' section
  index = 0, index_2 = 0;
  section_found_b = false, value_found_b = false;
  try {
    regex.assign (regex_string_2,
                  std::regex::ECMAScript);
  } catch (std::regex_error& error) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to std::regex::assign(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (regex_string_2.c_str ()),
                ACE_TEXT (error.what ())));
    return false;
  }
  do
  {
    result_2 = configuration.enumerate_sections (root_section,
                                                 index,
                                                 sub_section_string);
    if (result_2)
      break;
    if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (sub_section_string.c_str ()),
                        ACE_TEXT_ALWAYS_CHAR ("keyfile")))
      goto next_3;

    section_found_b = true;
    result_2 = configuration.open_section (root_section,
                                           sub_section_string.c_str (),
                                           0,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    do
    {
      result_2 = configuration.enumerate_values (sub_section_key,
                                                 index_2,
                                                 value_name_string,
                                                 value_type_e);
      if (result_2)
        break;
      if (ACE_OS::strcmp (ACE_TEXT_ALWAYS_CHAR (value_name_string.c_str ()),
                          ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices")))
        goto next_4;

      value_found_b = true;
      result_2 = configuration.get_string_value (sub_section_key,
                                                 value_name_string.c_str (),
                                                 value_string);
      ACE_ASSERT (result_2 == 0);
      value_string_2 = value_string.c_str ();
      if (!std::regex_match (value_string_2,
                             smatch,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to std::regex_match(\"%s\"), aborting\n"),
                    ACE_TEXT (configuration_file_path.c_str ())));
        return false;
      } // end IF
      for (iterator = ++smatch.begin ();
           iterator != smatch.end ();
           ++iterator)
      {
        if (!(*iterator).matched)
          continue;
        value_string_2 = (*iterator).str ();
        position = value_string_2.find (':', 0);
        ACE_ASSERT (position != std::string::npos);
        if (position == 3) // mac:
        {
          struct ether_addr ether_addr_s =
              Net_Common_Tools::stringToLinkLayerAddress (value_string_2);
          value_string_2 =
              Net_Common_Tools::linkLayerAddressToInterface (ether_addr_s);
          ACE_ASSERT (!value_string_2.empty ());
        } // end IF
        else
        { ACE_ASSERT (position == 14); // interface-name:
          value_string_2.erase (0, position + 1);
        } // end ELSE
        if (!ACE_OS::strcmp (interfaceIdentifier_in.c_str (),
                             value_string_2.c_str ()))
          break;
      } // end FOR
      if (toggle_in)
      {
        if (iterator != smatch.end ())
        {
          value_string.clear ();
          for (std::smatch::iterator iterator_2  = ++smatch.begin ();
               iterator_2 != smatch.end ();
               ++iterator_2)
          {
            if (!(*iterator_2).matched ||
                (iterator_2 == iterator))
              continue;
            if (iterator != ++smatch.begin ())
              value_string += ACE_TEXT_ALWAYS_CHAR (";");
            value_string_3 = (*iterator_2).str ();
            value_string += value_string_3.c_str ();
          } // end FOR

          result_2 =
              configuration.set_string_value (sub_section_key,
                                              ACE_TEXT ("unmanaged-devices"),
                                              value_string);
          ACE_ASSERT (result_2 == 0);
          modified_b = true;
        } // end IF
      } // end IF
      else
      {
        if (iterator == smatch.end ())
        {
          if (!value_string.empty ())
            value_string += ACE_TEXT_ALWAYS_CHAR (";");
          value_string += ACE_TEXT_ALWAYS_CHAR ("interface-name:");
          value_string += interfaceIdentifier_in.c_str ();
          result_2 =
              configuration.set_string_value (sub_section_key,
                                              ACE_TEXT ("unmanaged-devices"),
                                              value_string);
          ACE_ASSERT (result_2 == 0);
          modified_b = true;
        } // end IF
      } // end ELSE

next_4:
      ++index_2;
    } while (true);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Configuration::enumerate_values(\"%s\"): \"%m\", aborting\n"),
                  sub_section_string.c_str ()));
      return false;
    } // end IF

next_3:
    ++index;
  } while (true);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Configuration::enumerate_sections(): \"%m\", aborting\n")));
    return false;
  } // end IF
  if (unlikely (!section_found_b))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: section \"%s\" not found, creating\n"),
                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()),
                               ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT ("keyfile")));
    result_2 = configuration.open_section (root_section,
                                           ACE_TEXT_ALWAYS_CHAR ("keyfile"),
                                           1,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    if (toggle_in)
      value_string.clear ();
    else
    {
      value_string = ACE_TEXT_ALWAYS_CHAR ("interface-name:");
      value_string += interfaceIdentifier_in.c_str ();
    } // end ELSE
    result_2 =
        configuration.set_string_value (sub_section_key,
                                        ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices"),
                                        value_string);
    ACE_ASSERT (result_2 == 0);
    modified_b = true;
  } // end IF
  else if (unlikely (!value_found_b))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s[%s]: value \"%s\" not found, creating\n"),
                ACE::basename (ACE_TEXT (configuration_file_path.c_str ()), ACE_DIRECTORY_SEPARATOR_CHAR),
                ACE_TEXT ("keyfile"),
                ACE_TEXT ("unmanaged-devices")));
    result_2 = configuration.open_section (root_section,
                                           ACE_TEXT_ALWAYS_CHAR ("keyfile"),
                                           0,
                                           sub_section_key);
    ACE_ASSERT (result_2 == 0);
    if (toggle_in)
      value_string.clear ();
    else
    {
      value_string = ACE_TEXT_ALWAYS_CHAR ("interface-name:");
      value_string += interfaceIdentifier_in.c_str ();
    } // end ELSE
    result_2 =
        configuration.set_string_value (sub_section_key,
                                        ACE_TEXT_ALWAYS_CHAR ("unmanaged-devices"),
                                        value_string);
    ACE_ASSERT (result_2 == 0);
    modified_b = true;
  } // end ELSE IF

  if (modified_b)
  {
    bool drop_capabilities = false;
    bool drop_privileges = false;
    if (!Common_File_Tools::canWrite (configuration_file_path, static_cast<uid_t> (-1)))
    {
      if (Common_Tools::canCapability (CAP_DAC_OVERRIDE))
        drop_capabilities = Common_Tools::setCapability (CAP_DAC_OVERRIDE,
                                                         CAP_EFFECTIVE);
      else if (Common_Tools::switchUser (0)) // try seteuid()
        drop_privileges = true;
    } // end IF

    if (unlikely (!Common_File_Tools::backup (configuration_file_path)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::backup(\"\"), continuing\n"),
                  ACE_TEXT (configuration_file_path.c_str ())));

    result_2 =
        ini_importer.export_config (ACE_TEXT (configuration_file_path.c_str ()));
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Ini_ImpExp::export_config(\"%s\"): \"%m\", aborting\n"),
                  ACE_TEXT (configuration_file_path.c_str ())));
      if (drop_capabilities)
        Common_Tools::dropCapability (CAP_DAC_OVERRIDE);
      else if (drop_privileges)
        Common_Tools::switchUser (static_cast<uid_t> (-1));
      return false;
    } // end IF
    if (drop_capabilities)
      Common_Tools::dropCapability (CAP_DAC_OVERRIDE);
    else if (drop_privileges)
      Common_Tools::switchUser (static_cast<uid_t> (-1));
  } // end IF

  return true;
}

bool
Net_OS_Tools::isIfUpDownManagingInterface (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_OS_Tools::isIfUpDownManagingInterface"));

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  std::string configuration_file_path =
      ACE_TEXT_ALWAYS_CHAR ("/etc/network/interfaces");
  if (unlikely (!Common_File_Tools::canRead (configuration_file_path)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::canRead(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF

  std::map<std::string, std::string> logical_to_physical_interfaces_m;
  std::map<std::string, std::string>::const_iterator map_iterator;
  Net_InterfaceIdentifiers_t managed_interface_identifiers_a,
      unmanaged_interface_identifiers_a;
  unsigned char* data_p = NULL;
  unsigned int file_size_i = 0;
  if (unlikely (!Common_File_Tools::load (configuration_file_path,
                                          data_p,
                                          file_size_i)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                ACE_TEXT (configuration_file_path.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (data_p);

  std::istringstream converter (reinterpret_cast<char*> (data_p),
                                std::ios_base::in);
  char buffer_a [BUFSIZ];
  std::string regex_string_stanza_1st =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)(?:[[:space:]]*)(?:(iface)|(mapping)|(auto)|(no-auto-down)|(no-scripts)|(allow-)|(source)|(source-directory) )(.+)$");
  std::string regex_string_iface =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)(?:[[:space:]]*)(?:(address)|(gateway)|(up) )(.+)$");
  std::string regex_string_mapping =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)(?:[[:space:]]*)(?:(map)|(script) )(.+)$");
  std::string regex_string_mapping_2 =
    ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]*)(#)(?:[[:space:]]*)(.+)(?: (.+))*$");
  std::regex regex (regex_string_stanza_1st);
  std::smatch match_results;
  std::string buffer_string;
  int index_i = 0;
  bool is_comment_b = false;
  bool is_iface_b = false;
  std::string mapped_interface_identifier_string;
  bool is_mapping_b = false, is_mapping_2 = false;
  do
  {
    converter.getline (buffer_a, sizeof (char[BUFSIZ]));
    buffer_string = buffer_a;
    if (Common_String_Tools::isspace (buffer_string))
    {
      if (is_mapping_b)
        mapped_interface_identifier_string.clear ();

      is_comment_b = false;
      is_iface_b = false;
      is_mapping_b = false;
      is_mapping_2 = false;
      regex.assign (regex_string_stanza_1st);
      continue;
    } // end IF
    if (!std::regex_match (buffer_string,
                           match_results,
                           regex,
                           std::regex_constants::match_default))
      continue;
    ACE_ASSERT (match_results.ready () && !match_results.empty ());
    index_i = 1;
    for (std::smatch::iterator iterator = ++match_results.begin ();
         iterator != match_results.end ();
         ++iterator, ++index_i)
    {
      if (index_i == 1)
      {
        is_comment_b =
            !ACE_OS::strcmp (match_results[1].str ().c_str (),
                             ACE_TEXT_ALWAYS_CHAR ("#"));
        continue;
      } // end IF
      if (is_iface_b)
      {
        if (is_comment_b)
          continue;
        goto iface;
      } // end IF
      else if (is_mapping_b || is_mapping_2)
      {
        if (is_comment_b)
          continue;
        goto map;
      } // end IF

      if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                           ACE_TEXT_ALWAYS_CHAR ("auto")))
      { ACE_ASSERT (match_results[index_i + 1].matched);
        if (is_comment_b)
          unmanaged_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        else
          managed_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
      } // end IF
      else if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                                ACE_TEXT_ALWAYS_CHAR ("iface")))
      { ACE_ASSERT (match_results[index_i + 1].matched);
        if (is_comment_b)
          unmanaged_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        else
          managed_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        is_iface_b = true;
        regex.assign (regex_string_iface);
      } // end IF
      else if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                                ACE_TEXT_ALWAYS_CHAR ("mapping")))
      { ACE_ASSERT (match_results[index_i + 1].matched);
        if (is_comment_b)
          unmanaged_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        else
        {
          mapped_interface_identifier_string =
              match_results[index_i + 1].str ();
          managed_interface_identifiers_a.push_back (match_results[index_i + 1].str ());
        } // end ELSE
        is_mapping_b = true;
        regex.assign (regex_string_mapping);
      } // end IF
      ++iterator;
      continue;

iface:
      continue;

map:
      if (!ACE_OS::strcmp ((*iterator).str ().c_str (),
                           ACE_TEXT_ALWAYS_CHAR ("map")))
      { ACE_ASSERT (is_mapping_b);
        is_mapping_2 = true;
        regex.assign (regex_string_mapping_2);
        continue;
      } // end IF

      if (is_mapping_2)
      { ACE_ASSERT (!mapped_interface_identifier_string.empty ());
        for (std::smatch::iterator iterator_2 = iterator;
             iterator_2 != match_results.end ();
             ++iterator_2)
          logical_to_physical_interfaces_m.insert (std::make_pair ((*iterator_2).str (),
                                                                   mapped_interface_identifier_string));
        is_mapping_2 = false;
      } // end IF
    } // end FOR
  } while (!converter.fail ());

//clean:
  if (data_p)
    delete [] data_p;

  // replace all logical interfaces with their physical interfaces
  for (Net_InterfacesIdentifiersIterator_t iterator = managed_interface_identifiers_a.begin ();
       iterator != managed_interface_identifiers_a.end ();
       ++iterator)
  {
    map_iterator = logical_to_physical_interfaces_m.find (*iterator);
    if (map_iterator == logical_to_physical_interfaces_m.end ())
      continue;

    managed_interface_identifiers_a.push_back ((*map_iterator).second);
    managed_interface_identifiers_a.erase (iterator);
    iterator = managed_interface_identifiers_a.begin ();
  } // end FOR
  for (Net_InterfacesIdentifiersIterator_t iterator = managed_interface_identifiers_a.begin ();
       iterator != managed_interface_identifiers_a.end ();
       ++iterator)
    if (!ACE_OS::strcmp ((*iterator).c_str (),
                         interfaceIdentifier_in.c_str ()))
      return true;

  return false;
}

bool
Net_OS_Tools::isNetworkManagerManagingInterface (const std::string& interfaceIdentifier_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_OS_Tools::isNetworkManagerManagingInterface"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (!interfaceIdentifier_in.empty ());
  if (!Common_Process_Tools::id (ACE_TEXT_ALWAYS_CHAR (NET_EXE_NETWORKMANAGER_STRING)))
    return false; // *TODO*: avoid false negatives

#if defined (DBUS_SUPPORT)
//  // sanity check(s)
//  if (!Common_DBus_Tools::isUnitRunning (NULL,
//                                         COMMON_SYSTEMD_UNIT_NETWORKMANAGER))
//    return false; // *TODO*: avoid false negatives

  struct DBusError error_s;
  dbus_error_init (&error_s);
  struct DBusConnection* connection_p = NULL;
  struct DBusMessage* reply_p = NULL;
  struct DBusMessage* message_p = NULL;
  struct DBusMessageIter iterator, iterator_2;
  const char* argument_string_p = NULL;
  dbus_bool_t value_b = 0;

  connection_p = dbus_bus_get_private (DBUS_BUS_SYSTEM,
                                       &error_s);
  if (unlikely (!connection_p ||
                dbus_error_is_set (&error_s)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_bus_get_private(DBUS_BUS_SYSTEM): \"%s\", aborting\n"),
                ACE_TEXT (error_s.message)));

    dbus_error_free (&error_s);

    return false; // *TODO* avoid false negatives
  } // end IF
  dbus_connection_set_exit_on_disconnect (connection_p,
                                          false);

  std::string device_object_path_string =
      Net_WLAN_Tools::deviceToDBusObjectPath (connection_p,
                                              interfaceIdentifier_in);
  if (unlikely (device_object_path_string.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_WLAN_Tools::deviceToDBusObjectPath(\"%s\"), aborting\n"),
                ACE_TEXT (interfaceIdentifier_in.c_str ())));
    goto clean; // *TODO* avoid false negatives
  } // end IF

  message_p =
      dbus_message_new_method_call (ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_SERVICE),
                                    device_object_path_string.c_str (),
                                    ACE_TEXT_ALWAYS_CHAR (COMMON_DBUS_INTERFACE_PROPERTIES_STRING),
                                    ACE_TEXT_ALWAYS_CHAR ("Get"));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_new_method_call(Get): \"%m\", aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  argument_string_p =
      ACE_TEXT_ALWAYS_CHAR (NET_WLAN_DBUS_NETWORKMANAGER_DEVICE_INTERFACE);
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  argument_string_p = ACE_TEXT_ALWAYS_CHAR ("Managed");
  dbus_message_iter_init_append (message_p, &iterator);
  if (unlikely (!dbus_message_iter_append_basic (&iterator,
                                                 DBUS_TYPE_STRING,
                                                 &argument_string_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_append_basic(): \"%m\", aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  reply_p = Common_DBus_Tools::exchange (connection_p,
                                         message_p,
                                         -1); // timeout (ms)
  ACE_ASSERT (!message_p);
  if (unlikely (!reply_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_DBus_Tools::exchange(-1): \"%m\", aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  if (unlikely (!dbus_message_iter_init (reply_p, &iterator)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dbus_message_iter_init(), aborting\n")));
    goto clean; // *TODO* avoid false negatives
  } // end IF
  if (unlikely (!Common_DBus_Tools::validateType (iterator,
                                                  DBUS_TYPE_VARIANT)))
    goto clean; // *TODO* avoid false negatives
  dbus_message_iter_recurse (&iterator, &iterator_2);
  ACE_ASSERT (dbus_message_iter_get_arg_type (&iterator_2) == DBUS_TYPE_BOOLEAN);
  dbus_message_iter_get_basic (&iterator_2, &value_b);
  dbus_message_unref (reply_p); reply_p = NULL;

  result = value_b;

clean:
  if (connection_p)
  {
    dbus_connection_close (connection_p);
    dbus_connection_unref (connection_p);
  } // end IF
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
#endif // DBUS_SUPPORT

  return result;
}
#endif // ACE_WIN32 || ACE_WIN64
