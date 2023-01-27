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

#include "test_i_m3u_module_parser.h"

#include "ace/Log_Msg.h"

#include "common_string_tools.h"

#include "stream_macros.h"

Test_I_M3U_Module_Parser::Test_I_M3U_Module_Parser (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , contentLength_ (0)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_M3U_Module_Parser::Test_I_M3U_Module_Parser"));

}

ACE_Task<ACE_MT_SYNCH,
         Common_TimePolicy_t>*
Test_I_M3U_Module_Parser:: clone ()
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_M3U_Module_Parser::clone"));

  // initialize return value(s)
  Stream_Task_t* task_p = NULL;

  Stream_Module_t* module_p = NULL;
  ACE_NEW_NORETURN (module_p,
                    Test_I_M3U_Module_Parser_Module (const_cast<ISTREAM_T*> (inherited::getP ()),
                                                     ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ())));
  if (!module_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory(%u): %m, aborting\n"),
                inherited::mod_->name (),
                sizeof (Test_I_M3U_Module_Parser_Module)));
  else
  {
    task_p = module_p->writer ();
    ACE_ASSERT (task_p);

    Test_I_M3U_Module_Parser* eventHandler_impl =
        dynamic_cast<Test_I_M3U_Module_Parser*> (task_p);
    if (!eventHandler_impl)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<Test_I_M3U_Module_Parser> failed, aborting\n"),
                  inherited::mod_->name ()));

      // clean up
      delete module_p;

      return NULL;
    } // end IF
    eventHandler_impl->initialize (*inherited::STREAM_TASK_BASE_T::configuration_,
                                   inherited::allocator_);
  } // end ELSE

  return task_p;
}

void
Test_I_M3U_Module_Parser::handleDataMessage (Test_I_Message*& message_inout,
                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_M3U_Module_Parser::handleDataMessage"));

  int result = -1;

  // append \n to final fragment, if not present
  std::istringstream converter;
  ACE_UINT32 missing_bytes = 0;
  const Test_I_MessageDataContainer& data_container_r = message_inout->getR ();
  const Test_I_URLStreamLoad_MessageData& data_r = data_container_r.getR ();
  HTTP_HeadersConstIterator_t iterator =
    data_r.headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
  if (unlikely (iterator == data_r.headers.end ()))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: missing \"%s\" HTTP header, continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
    // assume the message contains all of the content
    contentLength_ =
      (inherited::headFragment_ ? inherited::headFragment_->total_length () + message_inout->total_length ()
                                : message_inout->total_length ());
  } // end IF
  else
  {
    converter.str ((*iterator).second);
    converter >> contentLength_;
    missing_bytes =
      contentLength_ - (inherited::headFragment_ ? inherited::headFragment_->total_length () : 0)
                     - message_inout->total_length ();
  } // end ELSE
  if (!missing_bytes)
  {
    int result = -1;
    size_t size_i = 0, capacity_i = 0;
    ACE_Message_Block* message_block_p = message_inout;
    while (message_block_p->cont ())
      message_block_p = message_block_p->cont ();

    if (*(message_block_p->rd_ptr () + (message_block_p->length () - 1)) != '\n')
    {
      if (!message_block_p->space ())
      {
        capacity_i = message_block_p->capacity () + 1 + 1; // allocate space for final \n as well (see below)
        size_i = message_block_p->size () + 1 + 1;
        result = message_block_p->size (capacity_i);
        ACE_ASSERT (result != -1);
        result = message_block_p->size (size_i);
        ACE_ASSERT (result != -1);
        ACE_ASSERT (message_block_p->space () == 2);
      } // end IF

      result = message_block_p->copy (ACE_TEXT_ALWAYS_CHAR ("\n"), 1);
      ACE_ASSERT (result != -1);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: appended missing line break to data\n"),
                  inherited::mod_->name ()));
    } // end IF

    if (!message_block_p->space ())
    {
      capacity_i = message_block_p->capacity () + 1;
      size_i = message_block_p->size () + 1;
      result = message_block_p->size (capacity_i);
      ACE_ASSERT (result != -1);
      result = message_block_p->size (size_i);
      ACE_ASSERT (result != -1);
      ACE_ASSERT (message_block_p->space () == 1);
    } // end IF

    result = message_block_p->copy (ACE_TEXT_ALWAYS_CHAR ("\n"), 1);
    ACE_ASSERT (result != -1);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: appended line break to data\n"),
                inherited::mod_->name ()));
  } // end IF

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  // append the "\0\0"-sequence, as required by flex
  //ACE_ASSERT (message_inout->capacity () - message_inout->length () >= STREAM_MISC_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  //*(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  //*(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  //message_inout->finalize (); // reset any data it might already have
  result = inherited::parserQueue_.enqueue_tail (message_inout,
                                                 NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF
  message_inout = NULL;

  if (!missing_bytes)
    inherited::PARSER_BASE_T::finished_ = true;
}

bool
Test_I_M3U_Module_Parser::hasFinished () const
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_M3U_Module_Parser::hasFinished"));

  return (contentLength_ &&
          (contentLength_ == inherited::scannerState_.offset));
}

void
Test_I_M3U_Module_Parser::record (struct M3U_Playlist*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("Test_I_M3U_Module_Parser::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT("%s\n"),
  //            ACE_TEXT (Test_I_M3U_Tools::PlaylistToString (*record_inout).c_str ())));

  // make sure the whole fragment chain references the same data record
  ACE_ASSERT (inherited::headFragment_);
  Test_I_Message* message_p = static_cast<Test_I_Message*> (inherited::headFragment_);
  Test_I_MessageDataContainer& data_container_r =
    const_cast<Test_I_MessageDataContainer&> (message_p->getR ());
  struct Test_I_URLStreamLoad_MessageData& data_r =
    const_cast<struct Test_I_URLStreamLoad_MessageData&> (data_container_r.getR ());
  data_r.M3UPlaylist = record_inout;
  Test_I_MessageDataContainer* data_container_2 = NULL;
  Test_I_Message* message_2 = static_cast<Test_I_Message*> (inherited::headFragment_->cont ());
  while (message_2)
  {
    data_container_r.increase ();
    data_container_2 = &data_container_r;
    message_2->initialize (data_container_2,
                           message_p->sessionId (),
                           NULL);
    message_2 = static_cast<Test_I_Message*> (message_2->cont ());
  } // end WHILE

  int result = inherited::put_next (inherited::headFragment_, NULL);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT("%s: failed to ACE_Task::put_next(): \"%m\", continuing\n"),
                inherited::mod_->name ()));

  inherited::headFragment_ = NULL;
  inherited::fragment_ = NULL;
  inherited::PARSER_BASE_T::finished_ = true;
}
