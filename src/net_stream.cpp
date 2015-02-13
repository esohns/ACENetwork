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

#include "net_stream.h"

#include <string>

#include "net_defines.h"
#include "net_macros.h"

Net_Stream::Net_Stream ()
 : inherited ()
 , socketHandler_ (std::string("SocketHandler"),
                   NULL)
 , headerParser_ (std::string("HeaderParser"),
                  NULL)
 , runtimeStatistic_ (std::string("RuntimeStatistic"),
                      NULL)
 , protocolHandler_ (std::string("ProtocolHandler"),
                     NULL)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::Net_Stream"));

  // remember the "owned" ones...
  // *TODO*: clean this up
  // *NOTE*: one problem is that we need to explicitly close() all
  // modules which we have NOT enqueued onto the stream (e.g. because init()
  // failed...)
  inherited::availableModules_.insert_tail (&socketHandler_);
  inherited::availableModules_.insert_tail (&headerParser_);
  inherited::availableModules_.insert_tail (&runtimeStatistic_);
  inherited::availableModules_.insert_tail (&protocolHandler_);

  // *CHECK* fix ACE bug: modules should initialize their "next" member to NULL !
  inherited::Stream_Module_t* module = NULL;
  for (ACE_DLList_Iterator<inherited::Stream_Module_t> iterator (inherited::availableModules_);
       iterator.next (module);
       iterator.advance ())
    module->next (NULL);
}

Net_Stream::~Net_Stream ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::~Net_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
Net_Stream::init (const Net_StreamProtocolConfigurationState_t& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::init"));

  // sanity check(s)
  ACE_ASSERT (!isInitialized_);

  // things to be done here:
  // - init notification strategy (if any)
  // -------------------------------------------------------------
  // - push the final module onto the stream (if any)
  // -------------------------------------------------------------
  // - init modules (done for the ones "owned" by the stream)
  // - push them onto the stream (tail-first) !
  // -------------------------------------------------------------

  if (configuration_in.configuration.notificationStrategy)
  {
    Stream_Module_t* module = inherited::head ();
    if (!module)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no head module found, aborting\n")));

      return false;
    } // end IF
    Stream_Task_t* task = module->reader ();
    if (!task)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("no head module reader task found, aborting\n")));

      return false;
    } // end IF
    task->msg_queue ()->notification_strategy (configuration_in.configuration.notificationStrategy);
  } // end IF

  // ---------------------------------------------------------------------------

  if (configuration_in.configuration.module)
    if (inherited::push (configuration_in.configuration.module) == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                  configuration_in.configuration.module->name ()));

      return false;
    } // end IF

  // ---------------------------------------------------------------------------

  // ******************* Protocol Handler ************************
  Net_Module_ProtocolHandler* protocolHandler_impl = NULL;
  protocolHandler_impl = dynamic_cast<Net_Module_ProtocolHandler*> (protocolHandler_.writer ());
  if (!protocolHandler_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_ProtocolHandler> failed, aborting\n")));

    return false;
  } // end IF
  if (!protocolHandler_impl->init (configuration_in.configuration.messageAllocator,
                                   configuration_in.sessionID,
                                   configuration_in.peerPingInterval,
                                   configuration_in.pingAutoAnswer,
                                   configuration_in.printPongMessages)) // print ('.') for received "pong"s...
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (protocolHandler_.name ())));

    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push (&protocolHandler_) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (protocolHandler_.name ())));

    return false;
  } // end IF

  // ******************* Runtime Statistics ************************
  Net_Module_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
  runtimeStatistic_impl = dynamic_cast<Net_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("dynamic_cast<RPG_Net_Module_RuntimeStatistic> failed, aborting\n")));

    return false;
  } // end IF
  if (!runtimeStatistic_impl->init (configuration_in.configuration.statisticsReportingInterval, // reporting interval (seconds)
                                    configuration_in.configuration.printFinalReport,            // print final report ?
                                    configuration_in.configuration.messageAllocator))           // message allocator handle
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (runtimeStatistic_.name ())));

    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push (&runtimeStatistic_) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (runtimeStatistic_.name ())));

    return false;
  } // end IF

  // ******************* Header Parser ************************
  Net_Module_HeaderParser* headerParser_impl = NULL;
  headerParser_impl = dynamic_cast<Net_Module_HeaderParser*> (headerParser_.writer ());
  if (!headerParser_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_HeaderParser> failed, aborting\n")));

    return false;
  } // end IF
  if (!headerParser_impl->init ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (headerParser_.name ())));

    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push (&headerParser_) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (headerParser_.name ())));

    return false;
  } // end IF

  // ******************* Socket Handler ************************
  Net_Module_SocketHandler* socketHandler_impl = NULL;
  socketHandler_impl = dynamic_cast<Net_Module_SocketHandler*> (socketHandler_.writer ());
  if (!socketHandler_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<RPG_Net_Module_SocketHandler> failed, aborting\n")));

    return false;
  } // end IF
  if (!socketHandler_impl->init (configuration_in.configuration.messageAllocator,
                                 configuration_in.sessionID,
                                 configuration_in.configuration.useThreadPerConnection,
                                 NET_STATISTICS_COLLECTION_INTERVAL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (socketHandler_.name ())));

    return false;
  } // end IF

  // enqueue the module...
  // *NOTE*: push()ing the module will open() it
  // --> set the argument that is passed along
  socketHandler_.arg (&const_cast<Net_StreamProtocolConfigurationState_t&> (configuration_in));
  if (inherited::push (&socketHandler_) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT(socketHandler_.name ())));

    return false;
  } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  inherited::allocator_ = configuration_in.configuration.messageAllocator;

  // OK: all went well
  inherited::isInitialized_ = true;
//   inherited::dump_state ();

  return true;
}

void
Net_Stream::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::ping"));

  Net_Module_ProtocolHandler* protocolHandler_impl = NULL;
  protocolHandler_impl = dynamic_cast<Net_Module_ProtocolHandler*> (protocolHandler_.writer ());
  if (!protocolHandler_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_ProtocolHandler> failed, returning\n")));

    return;
  } // end IF

  // delegate to this module...
  protocolHandler_impl->handleTimeout (NULL);
}

unsigned int
Net_Stream::getSessionID () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::getSessionID"));

  Stream_Module_t* module = &const_cast<Net_Module_SocketHandler_Module&> (socketHandler_);
  Net_Module_SocketHandler* socketHandler_impl = NULL;
  socketHandler_impl = dynamic_cast<Net_Module_SocketHandler*> (module->writer ());
  if (!socketHandler_impl)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT ("dynamic_cast<Net_Module_SocketHandler> failed, aborting\n")));

    return 0;
  } // end IF

  return socketHandler_impl->getSessionID ();
}

bool
Net_Stream::collect (Net_RuntimeStatistic_t& data_out) const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::collect"));

  Net_Module_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
  runtimeStatistic_impl = dynamic_cast<Net_Module_Statistic_WriterTask_t*> (const_cast<Net_Module_RuntimeStatistic_Module&> (runtimeStatistic_).writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Net_Module_Statistic_WriterTask_t> failed, aborting\n")));

    return false;
  } // end IF

  // delegate to this module...
  return runtimeStatistic_impl->collect (data_out);
}

void
Net_Stream::report () const
{
  NETWORK_TRACE (ACE_TEXT ("Net_Stream::report"));

//   Net_Module_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<Net_Module_Statistic_WriterTask_t*> (//runtimeStatistic_.writer ());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG ((LM_ERROR,
//                 ACE_TEXT ("dynamic_cast<Net_Module_Statistic_WriterTask_t> failed, returning\n")));
//
//     return;
//   } // end IF
//
//   // delegate to this module...
//   return (runtimeStatistic_impl->report ());

  // just a dummy
  ACE_ASSERT (false);

  ACE_NOTREACHED (return;)
}
