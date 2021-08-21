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

#include "ace/Log_Msg.h"

#include "common_string_tools.h"

#include "stream_dec_tools.h"

#include "net_defines.h"
#include "net_macros.h"

#include "http_common.h"
#include "http_defines.h"
#include "http_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                     SessionMessageType>::HTTP_Module_Parser_T (ISTREAM_T* stream_in)
#else
                     SessionMessageType>::HTTP_Module_Parser_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , inherited2 (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_LEXER_DFA_TABLES_FILENAME), // scanner tables file (if any)
               COMMON_PARSER_DEFAULT_LEX_TRACE,                           // trace scanning ?
               COMMON_PARSER_DEFAULT_YACC_TRACE)                          // trace parsing ?
 , headFragment_ (NULL)
 , crunch_ (HTTP_DEFAULT_CRUNCH_MESSAGES) // strip protocol data ?
 , chunks_ ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::HTTP_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::~HTTP_Module_Parser_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::~HTTP_Module_Parser_T"));

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
bool
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::initialize"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    ACE_ASSERT (inherited::msg_queue_);
    result = inherited::msg_queue_->activate ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::activate(): \"%m\", continuing\n")));

    if (headFragment_)
    {
      headFragment_->release (); headFragment_ = NULL;
    } // end IF
    crunch_ = HTTP_DEFAULT_CRUNCH_MESSAGES;
    chunks_.clear ();
  } // end IF

  crunch_ = configuration_in.crunchMessages;

  ACE_ASSERT (!configuration_in.parserConfiguration->messageQueue);
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
      inherited::msg_queue_;
  if (!inherited2::initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to HTTP_ParserDriver_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
        NULL;
    return false;
  } // end IF
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
      NULL;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleDataMessage"));

  DataMessageType* message_p = NULL;
  int result = -1;
  bool release_inbound_message = true; // message_inout
  bool release_message = false; // message_p
  typename SessionMessageType::DATA_T* session_data_container_p =
    inherited::sessionData_;

  // initialize return value(s)
  passMessageDownstream_out = false;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () - message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    if (!headFragment_)
      headFragment_ = message_inout;
    else
    {
      for (message_p = headFragment_;
           message_p->cont ();
           message_p = dynamic_cast<DataMessageType*> (message_p->cont ()));
      message_p->cont (message_inout);
    } // end ELSE
    message_p = headFragment_;
  } // end lock scope
  ACE_ASSERT (message_p);
  message_inout = NULL;
  release_inbound_message = false;

  { // *NOTE*: protect scanner/parser state
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
    //              message_p->id (),
    //              message_p->length ()));

    if (!this->parse (message_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to HTTP_IParser::parse() (message ID was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->id ()));
      goto error;
    } // end IF
    // the message fragment has been parsed successfully

    if (!this->hasFinished ())
      goto continue_; // --> wait for more data to arrive
  } // end lock scope

  // *NOTE*: the message has been parsed successfully
  //         --> pass the data (chain) downstream
  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    //// *NOTE*: new data fragments may have arrived by now
    ////         --> set the next head fragment ?
    //message_2 = dynamic_cast<DataMessageType*> (message_p->cont ());
    //if (message_2)
    //  message_p->cont (NULL);

    result = inherited::put_next (headFragment_, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      headFragment_->release ();
      goto error;
    } // end IF
    headFragment_ = NULL;
  } // end lock scope

  // *IMPORTANT NOTE*: send 'step' session message so downstream modules know
  //                   that the complete document data has arrived
  if (likely (session_data_container_p))
    session_data_container_p->increase ();
  if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STEP,
                                               session_data_container_p,
                                               NULL)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                inherited::name (),
                STREAM_SESSION_MESSAGE_STEP));

continue_:
error:
  if (release_inbound_message)
  { ACE_ASSERT (message_inout);
    message_inout->release (); message_inout = NULL;
  } // end IF
  if (release_message)
  { ACE_ASSERT (message_p);
    message_p->release (); message_p = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_END:
    {
      //// *NOTE*: (in a 'passive' scenario,) a parser thread may be waiting for
      ////         additional (entity) fragments to arrive
      ////         --> tell it to return
      //ACE_ASSERT (inherited::msg_queue_);
      //result = inherited::msg_queue_->pulse ();
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to ACE_Message_Queue_Base::pulse(): \"%m\", continuing\n")));

      if (headFragment_)
      {
        headFragment_->release (); headFragment_ = NULL;
      } // end IF
      chunks_.clear ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::record (struct HTTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (record_inout == inherited2::record_);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (!headFragment_->isInitialized ());

  // debug info
  if (inherited2::trace_)
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s"),
                ACE_TEXT (HTTP_Tools::dump (*record_inout).c_str ())));

  // set session data format
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  HTTP_HeadersIterator_t iterator =
      record_inout->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING));
  if (iterator != record_inout->headers.end ())
  {
    session_data_r.format =
        HTTP_Tools::EncodingToCompressionFormat ((*iterator).second);
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: set compression format: \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::compressionFormatToString (session_data_r.format).c_str ())));
#endif // _DEBUG
  } // end IF

  DATA_CONTAINER_T* data_container_p, *data_container_2 = NULL;
  DataMessageType* message_p = NULL;
  DATA_T* data_p = NULL;
  ACE_Message_Block* message_block_p = headFragment_;
  unsigned int bytes_to_skip = 0;

  ACE_NEW_NORETURN (data_container_p,
                    DATA_CONTAINER_T ());
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    delete record_inout; record_inout = NULL;
    goto error;
  } // end IF
  data_container_p->setPR (record_inout);
  ACE_ASSERT (!record_inout);
  data_container_2 = data_container_p;
  headFragment_->initialize (data_container_2,
                             headFragment_->sessionId (),
                             NULL);

  // make sure the whole fragment chain references the same data record
  // sanity check(s)
  message_p = dynamic_cast<DataMessageType*> (headFragment_->cont ());
  while (message_p)
  {
    data_container_p->increase ();
    data_container_2 = data_container_p;
    message_p->initialize (data_container_2,
                           message_p->sessionId (),
                           NULL);
    message_p = dynamic_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  // frame the content
  data_p = &const_cast<DATA_T&> (data_container_p->getR ());
  if (chunks_.empty ())
  {
    // *IMPORTANT NOTE*: the parsers' offset points to the begining of the body
    bytes_to_skip = inherited2::offset ();
    do
    { ACE_ASSERT (message_block_p);
      if (bytes_to_skip <= message_block_p->length ())
        break;
      bytes_to_skip -= message_block_p->length ();
      message_block_p->rd_ptr (message_block_p->length ());
      message_block_p = message_block_p->cont ();
    } while (true);
    message_block_p->rd_ptr (bytes_to_skip);
    iterator =
        data_p->headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
    ACE_ASSERT (iterator != data_p->headers.end ());
    std::istringstream converter;
    converter.str ((*iterator).second);
    converter >> bytes_to_skip;
    ACE_ASSERT (headFragment_->total_length () == bytes_to_skip);
  } // end IF
  else
  { ACE_ASSERT (!chunks_.empty ());
    ACE_Message_Block* message_block_2 = NULL;
    CHUNKS_ITERATOR_T iterator_2 = chunks_.begin ();
    CHUNKS_ITERATOR_T iterator_3;
    unsigned int total_data = 0;
    bytes_to_skip = (*iterator_2).first; // skip HTTP header + first chunk line
    do
    { ACE_ASSERT (message_block_p);
      if (bytes_to_skip <= message_block_p->length ())
        break;
      bytes_to_skip -= message_block_p->length ();
      message_block_p->rd_ptr (message_block_p->length ());
      message_block_p = message_block_p->cont ();
    } while (true);
    message_block_p->rd_ptr (bytes_to_skip);
    if (!message_block_p->length ())
      message_block_p = message_block_p->cont ();
    for (;
         iterator_2 != chunks_.end ();
         ++iterator_2)
    {
      total_data += (*iterator_2).second;
      bytes_to_skip = (*iterator_2).second; // skip chunk
      if (!bytes_to_skip) // no (more) data
      {
        // skip over any trailing header(s) and end delimiter
        bytes_to_skip = inherited2::offset () - (*iterator_2).first;
        do
        { ACE_ASSERT (message_block_p);
          if (bytes_to_skip <= message_block_p->length ())
            break;
          bytes_to_skip -= message_block_p->length ();
          message_block_p->rd_ptr (message_block_p->length ());
          message_block_p = message_block_p->cont ();
        } while (true);
        message_block_p->rd_ptr (bytes_to_skip);
      } // end IF
      else
      {
        do
        { ACE_ASSERT (message_block_p);
          if (bytes_to_skip <= message_block_p->length ())
            break;
          bytes_to_skip -= message_block_p->length ();
          message_block_p = message_block_p->cont ();
        } while (true);
        if (bytes_to_skip < message_block_p->length ())
        {
          message_block_2 = message_block_p->duplicate ();
          message_block_2->cont (message_block_p->cont ());
          message_block_p->cont (message_block_2);
          message_block_p->length (bytes_to_skip);
          message_block_2->rd_ptr (bytes_to_skip);
        } // end IF
        message_block_p = message_block_p->cont ();
        // skip over chunk delimiter
        while (!message_block_p->length ())
          message_block_p = message_block_p->cont ();
        ACE_ASSERT (*message_block_p->rd_ptr () == '\r');
        message_block_p->rd_ptr (1);
        while (!message_block_p->length ())
          message_block_p = message_block_p->cont ();
        ACE_ASSERT (*message_block_p->rd_ptr () == '\n');
        message_block_p->rd_ptr (1);
        // skip over next chunk line
        iterator_3 = iterator_2;
        std::advance (iterator_3, 1);
        ACE_ASSERT (iterator_3 != chunks_.end ());
        bytes_to_skip = (*iterator_3).first -
                        ((*iterator_2).first + (*iterator_2).second + 2);
        do
        { ACE_ASSERT (message_block_p);
          if (bytes_to_skip <= message_block_p->length ())
            break;
          bytes_to_skip -= message_block_p->length ();
          message_block_p->rd_ptr (message_block_p->length ());
          message_block_p = message_block_p->cont ();
        } while (true);
        message_block_p->rd_ptr (bytes_to_skip);
      } // end ELSE
    } // end FOR
    ACE_ASSERT (headFragment_->total_length () == total_data);
    chunks_.clear ();
  } // end ELSE

  inherited2::finished_ = true;
error:
  inherited2::record_ = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
HTTP_Module_Parser_T<ACE_SYNCH_USE,
                     TimePolicyType,
                     ConfigurationType,
                     ControlMessageType,
                     DataMessageType,
                     SessionMessageType>::chunk (unsigned int size_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_Parser_T::chunk"));

  chunks_.push_back (std::make_pair (inherited2::offset (), size_in));
}

////////////////////////////////////////////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      TimerManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      UserDataType>::HTTP_Module_ParserH_T (ISTREAM_T* stream_in)
#else
                      UserDataType>::HTTP_Module_ParserH_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
  : inherited (stream_in,                               // stream handle
               false,                                   // auto-start ? (active mode only)
               STREAM_HEADMODULECONCURRENCY_CONCURRENT, // concurrency mode
               true)                                    // generate sesssion messages ?
 , inherited2 (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_LEXER_DFA_TABLES_FILENAME), // scanner tables file (if any)
               COMMON_PARSER_DEFAULT_LEX_TRACE,                           // trace scanning ?
               COMMON_PARSER_DEFAULT_YACC_TRACE)                          // trace parsing ?
 , headFragment_ (NULL)
 , crunch_ (HTTP_DEFAULT_CRUNCH_MESSAGES) // strip protocol data ?
 , chunks_ ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::HTTP_Module_ParserH_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      TimerManagerType,
                      UserDataType>::~HTTP_Module_ParserH_T ()
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::~HTTP_Module_ParserH_T"));

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
bool
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      TimerManagerType,
                      UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                 Stream_IAllocator* allocator_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    if (headFragment_)
    {
      headFragment_->release (); headFragment_ = NULL;
    } // end IF
    crunch_ = HTTP_DEFAULT_CRUNCH_MESSAGES;
    chunks_.clear ();
  } // end IF

  crunch_ = configuration_in.crunchMessages;
  ACE_ASSERT (!configuration_in.parserConfiguration->messageQueue);
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
      inherited::msg_queue_;
  if (!inherited2::initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize parser driver: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
        NULL;
    return false;
  } // end IF
  const_cast<const ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
      NULL;

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      TimerManagerType,
                      UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleDataMessage"));

  DataMessageType* message_p = NULL;
  int result = -1;
  bool release_inbound_message = true; // message_inout
  bool release_message = false; // message_p

  // initialize return value(s)
  passMessageDownstream_out = false;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () - message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    if (!headFragment_)
      headFragment_ = message_inout;
    else
    {
      for (message_p = headFragment_;
           message_p->cont ();
           message_p = dynamic_cast<DataMessageType*> (message_p->cont ()));
      message_p->cont (message_inout);

      //// just signal the parser (see below for an explanation)
      //result = condition_.broadcast ();
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%s\", continuing\n"),
      //              inherited::mod_->name ()));
    } // end ELSE

    message_p = headFragment_;
  } // end lock scope
  ACE_ASSERT (message_p);
  message_inout = NULL;
  release_inbound_message = false;

  { // *NOTE*: protect scanner/parser state
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
    //              message_p->id (),
    //              message_p->length ()));

    if (!this->parse (message_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to HTTP_ParserDriver::parse() (message ID was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->id ()));
      goto error;
    } // end IF
    // the message fragment has been parsed successfully

    if (!this->hasFinished ())
      goto continue_; // --> wait for more data to arrive
  } // end lock scope

  // *NOTE*: the message has been parsed successfully
  //         --> pass the data (chain) downstream
  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    //// *NOTE*: new data fragments may have arrived by now
    ////         --> set the next head fragment ?
    //message_2 = dynamic_cast<DataMessageType*> (message_p->cont ());
    //if (message_2)
    //  message_p->cont (NULL);

    result = inherited::put_next (headFragment_, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      headFragment_->release ();
      goto error;
    } // end IF
    headFragment_ = NULL;
  } // end lock scope

continue_:
error:
  if (release_inbound_message)
  { ACE_ASSERT (message_inout);
    message_inout->release (); message_inout = NULL;
  } // end IF
  if (release_message)
  { ACE_ASSERT (message_p);
    message_p->release (); message_p = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      TimerManagerType,
                      UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      const SessionDataType& session_data_r = inherited::sessionData_->getR ();

      // retain session ID for reporting
      ACE_ASSERT (inherited::streamState_);
      ACE_ASSERT (inherited::streamState_->sessionData);
      ACE_ASSERT (inherited::streamState_->sessionData->lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *(inherited::streamState_->sessionData->lock));
        inherited::streamState_->sessionData->sessionId =
          session_data_r.sessionId;
      } // end lock scope

      //// start profile timer
      //profile_.start ();

      break;
    }
    //// *NOTE*: the stream has been link()ed, the message contains the (merged)
    ////         upstream session data --> retain a reference
    //case STREAM_SESSION_MESSAGE_LINK:
    //{
    //  // *NOTE*: relax the concurrency policy in this case
    //  // *TODO*: this isn't very reliable
    //  inherited::concurrent_ = true;

    //  break;
    //}
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message (see above: 2566)
      { ACE_Guard<typename inherited::ITASKCONTROL_T::MUTEX_T> aGuard (inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      if (headFragment_)
      {
        headFragment_->release (); headFragment_ = NULL;
      } // end IF
      chunks_.clear ();

      // *TODO*: remove type inference
      //if (inherited::concurrency_ != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
      this->stop (false, // wait ?
                  false, // high priority ?
                  true); // locked access ?

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
bool
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      TimerManagerType,
                      UserDataType>::collect (StatisticContainerType& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::collect"));

  // step1: initialize info container POD
  data_out.capturedFrames = 0;
  data_out.droppedFrames = 0;
  data_out.bytes = 0.0;
  data_out.dataMessages = 0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *NOTE*: information is collected by the statistic module (if any)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      TimerManagerType,
                      UserDataType>::record (struct HTTP_Record*& record_inout)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::record"));

  // sanity check(s)
  ACE_ASSERT (record_inout);
  ACE_ASSERT (record_inout == inherited2::record_);
  ACE_ASSERT (!headFragment_->isInitialized ());

  // debug info ?
  if (inherited2::trace_)
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s"),
                ACE_TEXT (HTTP_Tools::dump (*record_inout).c_str ())));

  // set session data format
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  HTTP_HeadersIterator_t iterator =
      record_inout->headers.find (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_ENCODING_STRING));
  if (iterator != record_inout->headers.end ())
  {
    session_data_r.format =
        HTTP_Tools::EncodingToCompressionFormat ((*iterator).second);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: set compression format: \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::compressionFormatToString (session_data_r.format).c_str ())));
  } // end IF

  DATA_CONTAINER_T* data_container_p, *data_container_2 = NULL;
  DataMessageType* message_p = NULL;
  DATA_T* data_p = NULL;
  ACE_Message_Block* message_block_p = headFragment_;
  unsigned int bytes_to_skip = 0;

  ACE_NEW_NORETURN (data_container_p,
                    DATA_CONTAINER_T ());
  if (!data_container_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    delete record_inout; record_inout = NULL;
    goto error;
  } // end IF
  data_container_p->setPR (record_inout);
  record_inout = NULL;
  data_container_2 = data_container_p;
  headFragment_->initialize (data_container_2,
                             headFragment_->sessionId (),
                             NULL);

  // make sure the whole fragment chain references the same data record
  // sanity check(s)
  message_p = static_cast<DataMessageType*> (headFragment_->cont ());
  while (message_p)
  {
    data_container_p->increase ();
    data_container_2 = data_container_p;
    message_p->initialize (data_container_2,
                           headFragment_->sessionId (),
                           NULL);
    message_p = static_cast<DataMessageType*> (message_p->cont ());
  } // end WHILE

  // frame the content
  data_p = &const_cast<DATA_T&> (data_container_p->getR ());
  if (chunks_.empty ())
  {
    // *IMPORTANT NOTE*: the parsers' offset points to the begining of the body
    bytes_to_skip = inherited2::offset ();
    do
    { ACE_ASSERT (message_block_p);
      if (bytes_to_skip <= message_block_p->length ())
        break;
      bytes_to_skip -= message_block_p->length ();
      message_block_p->rd_ptr (message_block_p->length ());
      message_block_p = message_block_p->cont ();
    } while (true);
    message_block_p->rd_ptr (bytes_to_skip);
    iterator =
        data_p->headers.find (Common_String_Tools::tolower (ACE_TEXT_ALWAYS_CHAR (HTTP_PRT_HEADER_CONTENT_LENGTH_STRING)));
    ACE_ASSERT (iterator != data_p->headers.end ());
    std::istringstream converter;
    converter.str ((*iterator).second);
    converter >> bytes_to_skip;
    ACE_ASSERT (headFragment_->total_length () == bytes_to_skip);
  } // end IF
  else
  { ACE_ASSERT (!chunks_.empty ());
    ACE_Message_Block* message_block_2 = NULL;
    CHUNKS_ITERATOR_T iterator_2 = chunks_.begin ();
    CHUNKS_ITERATOR_T iterator_3;
    unsigned int total_data = 0;
    bytes_to_skip = (*iterator_2).first; // skip HTTP header + first chunk line
    do
    { ACE_ASSERT (message_block_p);
      if (bytes_to_skip <= message_block_p->length ())
        break;
      bytes_to_skip -= message_block_p->length ();
      message_block_p->rd_ptr (message_block_p->length ());
      message_block_p = message_block_p->cont ();
    } while (true);
    message_block_p->rd_ptr (bytes_to_skip);
    for (;
         iterator_2 != chunks_.end ();
         ++iterator_2)
    {
      total_data += (*iterator_2).second;
      bytes_to_skip = (*iterator_2).second; // skip chunk
      if (!bytes_to_skip) // no (more) data
      {
        // skip over any trailing header(s) and end delimiter
        bytes_to_skip = inherited2::offset () - (*iterator_2).first;
        do
        { ACE_ASSERT (message_block_p);
          if (bytes_to_skip <= message_block_p->length ())
            break;
          bytes_to_skip -= message_block_p->length ();
          message_block_p->rd_ptr (message_block_p->length ());
          message_block_p = message_block_p->cont ();
        } while (true);
        message_block_p->rd_ptr (bytes_to_skip);
      } // end IF
      else
      {
        do
        { ACE_ASSERT (message_block_p);
          if (bytes_to_skip <= message_block_p->length ())
            break;
          bytes_to_skip -= message_block_p->length ();
          message_block_p = message_block_p->cont ();
        } while (true);
        if (bytes_to_skip < message_block_p->length ())
        {
          message_block_2 = message_block_p->duplicate ();
          message_block_2->cont (message_block_p->cont ());
          message_block_p->cont (message_block_2);
          message_block_p->length (bytes_to_skip);
          message_block_2->rd_ptr (bytes_to_skip);
        } // end IF
        message_block_p = message_block_p->cont ();
        // skip over chunk delimiter
        while (!message_block_p->length ())
          message_block_p = message_block_p->cont ();
        ACE_ASSERT (*message_block_p->rd_ptr () == '\r');
        message_block_p->rd_ptr (1);
        while (!message_block_p->length ())
          message_block_p = message_block_p->cont ();
        ACE_ASSERT (*message_block_p->rd_ptr () == '\n');
        message_block_p->rd_ptr (1);
        // skip over next chunk line
        iterator_3 = iterator_2;
        std::advance (iterator_3, 1);
        ACE_ASSERT (iterator_3 != chunks_.end ());
        bytes_to_skip = (*iterator_3).first -
                        ((*iterator_2).first + (*iterator_2).second + 2);
        do
        { ACE_ASSERT (message_block_p);
          if (bytes_to_skip <= message_block_p->length ())
            break;
          bytes_to_skip -= message_block_p->length ();
          message_block_p->rd_ptr (message_block_p->length ());
          message_block_p = message_block_p->cont ();
        } while (true);
        message_block_p->rd_ptr (bytes_to_skip);
      } // end ELSE
    } // end FOR
    ACE_ASSERT (headFragment_->total_length () == total_data);
  } // end ELSE

  inherited2::finished_ = true;
error:
  inherited2::record_ = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
HTTP_Module_ParserH_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      ConfigurationType,
                      StreamControlType,
                      StreamNotificationType,
                      StreamStateType,
                      SessionDataType,
                      SessionDataContainerType,
                      StatisticContainerType,
                      TimerManagerType,
                      UserDataType>::chunk (unsigned int size_in)
{
  NETWORK_TRACE (ACE_TEXT ("HTTP_Module_ParserH_T::chunk"));

  chunks_.push_back (std::make_pair (inherited2::offset (), size_in));
}
