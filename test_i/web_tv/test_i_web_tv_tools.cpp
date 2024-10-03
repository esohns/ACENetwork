/***************************************************************************
 *   Copyright (C) 2024 by Erik Sohns                                      *
 *   erik.sohns@posteo.de                                                  *
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

#include "test_i_web_tv_tools.h"

#include <iomanip>
#include <regex>

#include "ace/Basic_Types.h"

#include "test_i_web_tv_defines.h"

void
Test_I_WebTV_Tools::addSegmentURIs (unsigned int program_in,
                                    Test_I_WebTV_ChannelSegmentURLs_t& URIs_out,
                                    unsigned int maxIndex_in,
                                    unsigned int indexPositions_in)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_WebTV_Tools::addSegmentURIs"));

  std::string URI_string_head, URI_string_head_2, URI_string_tail;
  std::string regex_string;
  std::regex regex;
  std::smatch match_results;
  std::stringstream converter;
  bool URI_has_path = false;
  std::string::size_type position_i;
  ACE_ASSERT (!URIs_out.empty ());
  std::string last_URI_string = URIs_out.back ();

  switch (program_in)
  {
    case 1:  // Das Erste
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = last_URI_string.rfind ('/', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);
      URI_has_path = true;
      URI_string_tail =
          last_URI_string.substr (position_i + 1, std::string::npos);
      URI_string_head = last_URI_string;
      position_i = last_URI_string.find ('/', 0);
      URI_string_head.erase (position_i, std::string::npos);

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (last_URI_string.substr (position_i + 1, indexPositions_in));
      converter >> index_2;

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^-]+)(-)([[:alnum:]]+)(-)([[:digit:]]+)(T)([[:digit:]]+)(_)([[:digit:]]+)(.ts|.aac)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (URI_string_tail,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[7].str ());
      converter >> date_i;

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[9].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str () +
                           match_results[5].str () +
                           match_results[6].str () +
                           match_results[7].str () + // date
                           match_results[8].str ());
      URI_string_tail = match_results[10].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        if (URI_has_path)
        {
          URI_string += '/';
          converter.str (ACE_TEXT_ALWAYS_CHAR (""));
          converter.clear ();
          converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
          URI_string += converter.str ();
          URI_string += '/';
          URI_string += URI_string_head_2;
        } // end IF

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 5:  // ARTE // *TODO*
    case 8:  // MDR
    case 28: // NASA TV
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = last_URI_string.rfind ('/', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);
      URI_string_tail =
          last_URI_string.substr (position_i + 1, std::string::npos);

      position_i = last_URI_string.find ('/', 0);
      ACE_ASSERT (position_i != std::string::npos);
      position_i = last_URI_string.find ('/', position_i + 1);
      ACE_ASSERT (position_i != std::string::npos);

      URI_string_head = last_URI_string;
      URI_string_head.erase (position_i + 1, std::string::npos);

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (last_URI_string.substr (position_i + 1, indexPositions_in));
      converter >> index_2;

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^(master)(_)([[:alnum:]]+)(_)([[:digit:]]+)(.ts)$");
      regex.assign (regex_string);
      if (unlikely (!std::regex_match (URI_string_tail,
                                       match_results,
                                       regex,
                                       std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[5].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str ());
      URI_string_tail = match_results[6].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
        URI_string += converter.str ();
        URI_string += '/';
        URI_string += URI_string_head_2;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 7: // HR
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = last_URI_string.rfind ('/', std::string::npos);
      URI_string_tail =
          last_URI_string.substr (position_i + 1, std::string::npos);
      URI_string_head = last_URI_string;
      position_i = last_URI_string.find ('/', 0);
      position_i = last_URI_string.find ('/', position_i + 1);
      URI_string_head.erase (position_i, std::string::npos);

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (last_URI_string.substr (position_i + 1, indexPositions_in));
      converter >> index_2;

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^-]+)(-)([^-]+)(-)([[:digit:]]+)(_)([[:digit:]]+)(.ts|.aac)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (URI_string_tail,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[5].str ());
      converter >> date_i;

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[7].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str () +
                           match_results[5].str () + // date
                           match_results[6].str ());
      URI_string_tail = match_results[8].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        URI_string += '/';
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
        URI_string += converter.str ();
        URI_string += '/';
        URI_string += URI_string_head_2;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 10: // RBB
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = last_URI_string.rfind ('/', std::string::npos);
      URI_string_tail =
          last_URI_string.substr (position_i + 1, std::string::npos);
      URI_string_head = last_URI_string;
      position_i = last_URI_string.find ('/', 0);
      position_i = last_URI_string.find ('/', position_i + 1);
      URI_string_head.erase (position_i, std::string::npos);

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (last_URI_string.substr (position_i + 1, indexPositions_in));
      converter >> index_2;

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^-]+)(-)([^-]+)(-)([^-]+)(-)([^-]+)(-)([[:digit:]]+)(_)([[:digit:]]+)(.ts)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (URI_string_tail,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[9].str ());
      converter >> date_i;

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[11].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str () +
                           match_results[5].str () +
                           match_results[6].str () +
                           match_results[7].str () +
                           match_results[8].str () +
                           match_results[9].str () + // date
                           match_results[10].str ());
      URI_string_tail = match_results[12].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        URI_string += '/';
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
        URI_string += converter.str ();
        URI_string += '/';
        URI_string += URI_string_head_2;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 13: // WDR
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = last_URI_string.rfind ('/', std::string::npos);
      URI_string_tail =
          last_URI_string.substr (position_i + 1, std::string::npos);
      URI_string_head = last_URI_string;
      position_i = last_URI_string.find ('/', 0);
      URI_string_head.erase (position_i, std::string::npos);

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (last_URI_string.substr (position_i + 1, indexPositions_in));
      converter >> index_2;

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^_]+)(_)([[:digit:]]+)(_)([[:digit:]]+)(.ts|.aac)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (URI_string_tail,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[3].str ());
      converter >> date_i;

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[5].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () + // date
                           match_results[4].str ());
      URI_string_tail = match_results[6].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        URI_string += '/';
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
        URI_string += converter.str ();
        URI_string += '/';
        URI_string += URI_string_head_2;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 14: // Parlamentsfernsehen
    {
      ACE_UINT32 index_i = 0, date_i = 0;

      position_i = last_URI_string.rfind ('_', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);

      URI_string_head = last_URI_string;
      URI_string_head.erase (position_i + 1, std::string::npos);

      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^(media)(-)([[:alnum:]]+)(_)([[:alnum:]]+)(_)([[:alpha:]]+)(_)([[:digit:]]+)(.ts)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (last_URI_string,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[9].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str () +
                           match_results[5].str () +
                           match_results[6].str () +
                           match_results[7].str () +
                           match_results[8].str ());
      URI_string_tail = match_results[10].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << ++index_i;
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 15: // ANIXE
    {
      ACE_UINT32 index_i = 0, date_i = 0;

      position_i = last_URI_string.rfind ('_', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);

      URI_string_head = last_URI_string;
      URI_string_head.erase (position_i + 1, std::string::npos);

      regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^(media)(_)([[:digit:]]+)(.ts)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (last_URI_string,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[3].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str ());
      URI_string_tail = match_results[4].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << ++index_i;
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 18: // QVC
    {
      ACE_UINT32 index_i = 0, date_i = 0;

      position_i = last_URI_string.rfind ('_', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);

      URI_string_head = last_URI_string;
      URI_string_head.erase (position_i + 1, std::string::npos);

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]+)(_)([^_]+)(_)([^_]+)(_)([[:digit:]]+)(.ts)$");
      regex.assign (regex_string);
      if (unlikely(!std::regex_match (last_URI_string,
                                      match_results,
                                      regex,
                                      std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[7].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str () +
                           match_results[5].str () +
                           match_results[6].str ());
      URI_string_tail = match_results[8].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << ++index_i;
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 25: // Channel NewsAsia
    {
      ACE_UINT32 index_i = 0, date_i = 0;

      position_i = last_URI_string.rfind ('_', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);

      URI_string_head = last_URI_string;
      URI_string_head.erase (position_i + 1, std::string::npos);

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^_]+)(_)([^_]+)(_)([[:digit:]]+)(.*)$");
      regex.assign (regex_string);
      if (unlikely (!std::regex_match (last_URI_string,
                                       match_results,
                                       regex,
                                       std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[5].str ());
      converter >> index_i;
      URI_string_head_2 = (match_results[1].str () +
                           match_results[2].str () +
                           match_results[3].str () +
                           match_results[4].str ());
      URI_string_tail = match_results[6].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << ++index_i;
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    case 35: // ALEX
    {
      ACE_UINT32 index_i = 0, date_i = 0;

      position_i = last_URI_string.rfind ('_', std::string::npos);
      ACE_ASSERT (position_i != std::string::npos);

      URI_string_head = last_URI_string;
      URI_string_head.erase (position_i + 1, std::string::npos);

      regex_string =
          ACE_TEXT_ALWAYS_CHAR ("^([^-]+)(-)([^_]+)(_)([^_]+)(_)([^_]+)(_)([[:digit:]]+)(.*)$");
      regex.assign (regex_string);
      if (unlikely (!std::regex_match (last_URI_string,
                                       match_results,
                                       regex,
                                       std::regex_constants::match_default)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                    ACE_TEXT (last_URI_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT (match_results.ready () && !match_results.empty ());

      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter.str (match_results[9].str ());
      converter >> index_i;
      //URI_string_head_2 = (match_results[1].str () +
      //                     match_results[2].str () +
      //                     match_results[3].str () +
      //                     match_results[4].str () +
      //                     match_results[5].str () +
      //                     match_results[6].str () +
      //                     match_results[7].str () +
      //                     match_results[8].str ());
      URI_string_tail = match_results[10].str ();

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter << ++index_i;
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
    default:
    {
      ACE_UINT32 index_i = 0, index_2 = 0, date_i = 0;

      position_i = last_URI_string.rfind ('/', std::string::npos);
      if (position_i != std::string::npos)
      {
        URI_has_path = true;
        URI_string_tail =
            last_URI_string.substr (position_i + 1, std::string::npos);
        URI_string_head = last_URI_string;
        position_i = last_URI_string.find ('/', 0);
        URI_string_head.erase (position_i, std::string::npos);

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (last_URI_string.substr (position_i + 1, indexPositions_in));
        converter >> index_2;

        regex_string =
            ACE_TEXT_ALWAYS_CHAR ("^([^_]+)(_)([[:alnum:]]+)(_)([[:digit:]]+)(_)([[:digit:]]+)(_)([[:digit:]]+)(.ts|.aac)$");
        regex.assign (regex_string);
        if (unlikely(!std::regex_match (URI_string_tail,
                                        match_results,
                                        regex,
                                        std::regex_constants::match_default)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                      ACE_TEXT (last_URI_string.c_str ())));
          return;
        } // end IF
        ACE_ASSERT (match_results.ready () && !match_results.empty ());

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (match_results[7].str ());
        converter >> date_i;

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (match_results[9].str ());
        converter >> index_i;
        URI_string_head_2 = (match_results[1].str () +
                             match_results[2].str () +
                             match_results[3].str () +
                             match_results[4].str () +
                             match_results[5].str () +
                             match_results[6].str () +
                             match_results[7].str () + // date
                             match_results[8].str ());
        URI_string_tail = match_results[10].str ();
      } // end IF
      else
      {
        std::string regex_string =
            ACE_TEXT_ALWAYS_CHAR ("^([[:digit:]]+)(.ts|.aac)$");
        regex.assign (regex_string);
        if (unlikely(!std::regex_match (last_URI_string,
                                        match_results,
                                        regex,
                                        std::regex_constants::match_default)))
        {
          ACE_DEBUG ((LM_ERROR,
                     ACE_TEXT("failed to parse segment URI \"%s\", returning\n"),
                     ACE_TEXT (last_URI_string.c_str ())));
          return;
        } // end IF
        ACE_ASSERT (match_results.ready () && !match_results.empty ());
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        converter.str (match_results[1].str ());
        converter >> index_i;
        URI_string_tail = match_results[2].str ();
      } // end ELSE

      std::string URI_string;
      for (unsigned int i = 0;
           i < TEST_I_WEBTV_DEFAULT_NUMBER_OF_QUEUED_SEGMENTS;
           ++i)
      {
        URI_string = URI_string_head;
        if (URI_has_path)
        {
          URI_string += '/';
          converter.str (ACE_TEXT_ALWAYS_CHAR (""));
          converter.clear ();
          converter << std::setw (indexPositions_in) << std::setfill ('0') << index_2;
          URI_string += converter.str ();
          URI_string += '/';
          URI_string += URI_string_head_2;
        } // end IF

        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter.clear ();
        if (indexPositions_in)
          converter << std::setw (indexPositions_in) << std::setfill ('0') << ++index_i;
        else
          converter << ++index_i;
        if (maxIndex_in &&
            (index_i == maxIndex_in))
        {
          index_i = 1; // *TODO*: or 0 ?
          ++index_2;
        } // end IF
        URI_string += converter.str ();
        URI_string += URI_string_tail;
        URIs_out.push_back (URI_string);
      } // end FOR

      break;
    }
  } // end SWITCH
}
