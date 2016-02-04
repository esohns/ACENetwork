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

#include "test_i_module_htmlwriter.h"

#include <sstream>

#include "ace/Date_Time.h"

#include "stream_macros.h"

#include "net_common_tools.h"

Test_I_Stream_Module_HTMLWriter::Test_I_Stream_Module_HTMLWriter ()
 : inherited ()
 , fileWritten_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLWriter::Test_I_Stream_Module_HTMLWriter"));

}

Test_I_Stream_Module_HTMLWriter::~Test_I_Stream_Module_HTMLWriter ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLWriter::~Test_I_Stream_Module_HTMLWriter"));

  // clean up
  if (inherited::document_)
  {
    xmlFreeDoc (inherited::document_);
    inherited::document_ = NULL;
  } // end IF
}

void
Test_I_Stream_Module_HTMLWriter::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLWriter::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  // *TODO*: remove type inferences
  const Test_I_Stream_SessionData_t& session_data_container_r =
    message_inout->get ();
  Test_I_Stream_SessionData& session_data_r =
    const_cast<Test_I_Stream_SessionData&> (session_data_container_r.get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      ACE_ASSERT (!inherited::document_);
      inherited::document_ = htmlNewDocNoDtD (NULL,  // URI
                                              NULL); // external ID
      if (!inherited::document_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to htmlNewDocNoDtD(), aborting\n"),
                    inherited::mod_->name ()));

        // *TODO*: remove type inference
        session_data_r.aborted = true;

        return;
      } // end IF
      fileWritten_ = false;

      break;
    }
    case STREAM_SESSION_END:
    {
      // sanity check(s)
      if (!inherited::document_) break; // nothing to do

      xmlNodePtr root_node_p =
//          xmlDocGetRootElement (inherited::document_);
          xmlNewNode (NULL,
                      BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html")));
      ACE_ASSERT (root_node_p);
      xmlDocSetRootElement (inherited::document_,
                            root_node_p);

      // ------------------------------- head ----------------------------------
      xmlNodePtr head_node_p =
          xmlNewChild (root_node_p,                              // parent
                       NULL,                                     // namespace
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head")), // name
                       NULL);                                    // content
      ACE_ASSERT (head_node_p);
      xmlNodePtr node_p =
          xmlNewChild (head_node_p,                                    // parent
                       NULL,                                           // namespace
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("title")),      // name
                       BAD_CAST (session_data_r.data.title.c_str ())); // content
      ACE_ASSERT (node_p);

      // ------------------------------- body ----------------------------------
      xmlNodePtr body_node_p =
          xmlNewChild (root_node_p,                              // parent
                       NULL,                                     // namespace
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body")), // name
                       NULL);                                    // content
      ACE_ASSERT (body_node_p);

      ACE_Date_Time date;
      date.update ();
      std::ostringstream converter;
      xmlNodePtr list_node_p = NULL;
      xmlAttrPtr attribute_p = NULL;
      std::string URL_base =
          Net_Common_Tools::URL2HostName (inherited::configuration_.URL,
                                          true,
                                          true);
      std::string URL;
      for (Test_I_PageDataReverseConstIterator_t iterator = session_data_r.data.pageData.rbegin ();
           iterator != session_data_r.data.pageData.rend ();
           ++iterator)
      {
        date.update ((*iterator).first);
        converter.clear ();
        converter.str (ACE_TEXT_ALWAYS_CHAR (""));
        converter << date.day () << '.' << date.month () << '.' << date.year ();
        node_p =
            xmlNewChild (body_node_p,                            // parent
                         NULL,                                   // namespace
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("h4")), // name
                         BAD_CAST (converter.str ().c_str ()));  // content
        ACE_ASSERT (node_p);

        list_node_p =
            xmlNewChild (body_node_p,                            // parent
                         NULL,                                   // namespace
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("ul")), // name
                         NULL);                                  // content
        ACE_ASSERT (list_node_p);
        for (Test_I_DataItemsIterator_t iterator2 = (*iterator).second.begin ();
             iterator2 != (*iterator).second.end ();
             ++iterator2)
        {
          node_p =
              xmlNewChild (list_node_p,                            // parent
                           NULL,                                   // namespace
                           BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("li")), // name
                           NULL);                                  // content
          ACE_ASSERT (node_p);
          node_p =
              xmlNewChild (node_p,                                        // parent
                           NULL,                                          // namespace
                           BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("a")),         // name
                           BAD_CAST ((*iterator2).description.c_str ())); // content
          ACE_ASSERT (node_p);
          URL = URL_base;
          URL += (*iterator2).URI;
          attribute_p =
              xmlSetProp (node_p,                                   // node
                          BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("href")), // name
                          BAD_CAST (URL.c_str ()));    // value
          ACE_ASSERT (attribute_p);
        } // end FOR
      } // end FOR

      result = htmlSaveFile (inherited::configuration_.targetFileName.c_str (),
                             inherited::document_);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to htmlSaveFile(\"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_.targetFileName.c_str ())));

        // *TODO*: remove type inference
        session_data_r.aborted = true;

        return;
      } // end IF
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: wrote \"%s\" (%d byte(s))\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_.targetFileName.c_str ()),
                    result));
      fileWritten_ = true;

      // clean up
      if (inherited::document_)
      {
        xmlFreeDoc (inherited::document_);
        inherited::document_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
