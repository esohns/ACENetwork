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

Net_Stream::Net_Stream()
 : //inherited(),
   mySocketHandler(std::string("SocketHandler"),
                   NULL),
   myHeaderParser(std::string("HeaderParser"),
                  NULL),
   myRuntimeStatistic(std::string("RuntimeStatistic"),
                      NULL),
   myProtocolHandler(std::string("ProtocolHandler"),
                     NULL)
{
  NETWORK_TRACE(ACE_TEXT("Net_Stream::Net_Stream"));

  // remember the "owned" ones...
  // *TODO*: clean this up
  // *NOTE*: one problem is that we need to explicitly close() all
  // modules which we have NOT enqueued onto the stream (e.g. because init()
  // failed...)
  myAvailableModules.insert_tail(&mySocketHandler);
  myAvailableModules.insert_tail(&myHeaderParser);
  myAvailableModules.insert_tail(&myRuntimeStatistic);
  myAvailableModules.insert_tail(&myProtocolHandler);

  // *CHECK* fix ACE bug: modules should initialize their "next" member to NULL !
  inherited::MODULE_TYPE* module = NULL;
  for (ACE_DLList_Iterator<inherited::MODULE_TYPE> iterator(myAvailableModules);
       iterator.next(module);
       iterator.advance())
    module->next(NULL);
}

Net_Stream::~Net_Stream()
{
  NETWORK_TRACE(ACE_TEXT("Net_Stream::~Net_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown();
}

bool
Net_Stream::init(const RPG_Net_ConfigPOD& configuration_in)
{
  NETWORK_TRACE(ACE_TEXT("Net_Stream::init"));

  // sanity check(s)
  ACE_ASSERT(!myIsInitialized);

  // things to be done here:
  // - init notification strategy (if any)
  // -------------------------------------------------------------
  // - push the final module onto the stream (if any)
  // -------------------------------------------------------------
  // - init modules (done for the ones "owned" by the stream)
  // - push them onto the stream (tail-first) !
  // -------------------------------------------------------------

  if (configuration_in.streamSocketConfiguration.notificationStrategy)
  {
    MODULE_TYPE* module = head();
    if (!module)
    {
      ACE_DEBUG((LM_ERROR,
                 ACE_TEXT("no head module found, aborting\n")));

      return false;
    } // end IF
    TASK_TYPE* task = module->reader();
    if (!task)
    {
      ACE_DEBUG((LM_ERROR,
                 ACE_TEXT("no head module reader task found, aborting\n")));

      return false;
    } // end IF
    task->msg_queue ()->notification_strategy (configuration_in.streamSocketConfiguration.notificationStrategy);
  } // end IF

  // ---------------------------------------------------------------------------

  if (configuration_in.streamSocketConfiguration.module)
    if (inherited::push (configuration_in.streamSocketConfiguration.module) == -1)
    {
      ACE_DEBUG((LM_ERROR,
                 ACE_TEXT("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
                 configuration_in.streamSocketConfiguration.module->name ()));

      return false;
    } // end IF

  // ---------------------------------------------------------------------------

  // ******************* Protocol Handler ************************
  RPG_Net_Module_ProtocolHandler* protocolHandler_impl = NULL;
  protocolHandler_impl = dynamic_cast<RPG_Net_Module_ProtocolHandler*>(myProtocolHandler.writer());
  if (!protocolHandler_impl)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("dynamic_cast<RPG_Net_Module_ProtocolHandler> failed, aborting\n")));

    return false;
  } // end IF
  if (!protocolHandler_impl->init (configuration_in.streamSocketConfiguration.messageAllocator,
                                   configuration_in.streamSocketConfiguration.sessionID,
                                   configuration_in.peerPingInterval,
                                   configuration_in.pingAutoAnswer,
                                   configuration_in.printPongMessages)) // print ('.') for received "pong"s...
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to initialize module: \"%s\", aborting\n"),
               myProtocolHandler.name()));

    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push(&myProtocolHandler) == -1)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
               myProtocolHandler.name()));

    return false;
  } // end IF

  // ******************* Runtime Statistics ************************
  RPG_NET_MODULE_RUNTIMESTATISTICS_T* runtimeStatistic_impl = NULL;
  runtimeStatistic_impl = dynamic_cast<RPG_NET_MODULE_RUNTIMESTATISTICS_T*>(myRuntimeStatistic.writer());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("dynamic_cast<RPG_Net_Module_RuntimeStatistic> failed, aborting\n")));

    return false;
  } // end IF
  if (!runtimeStatistic_impl->init (configuration_in.streamSocketConfiguration.statisticsReportingInterval, // reporting interval (seconds)
                                    configuration_in.streamSocketConfiguration.printFinalReport,            // print final report ?
                                    configuration_in.streamSocketConfiguration.messageAllocator))           // message allocator handle
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to initialize module: \"%s\", aborting\n"),
               myRuntimeStatistic.name()));

    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push(&myRuntimeStatistic) == -1)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
               myRuntimeStatistic.name()));

    return false;
  } // end IF

  // ******************* Header Parser ************************
  RPG_Net_Module_HeaderParser* headerParser_impl = NULL;
  headerParser_impl = dynamic_cast<RPG_Net_Module_HeaderParser*>(myHeaderParser.writer());
  if (!headerParser_impl)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("dynamic_cast<RPG_Net_Module_HeaderParser> failed, aborting\n")));

    return false;
  } // end IF
  if (!headerParser_impl->init())
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to initialize module: \"%s\", aborting\n"),
               myHeaderParser.name()));

    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push(&myHeaderParser) == -1)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
               myHeaderParser.name()));

    return false;
  } // end IF

  // ******************* Socket Handler ************************
  RPG_Net_Module_SocketHandler* socketHandler_impl = NULL;
  socketHandler_impl = dynamic_cast<RPG_Net_Module_SocketHandler*>(mySocketHandler.writer());
  if (!socketHandler_impl)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("dynamic_cast<RPG_Net_Module_SocketHandler> failed, aborting\n")));

    return false;
  } // end IF
  if (!socketHandler_impl->init (configuration_in.streamSocketConfiguration.messageAllocator,
                                 configuration_in.streamSocketConfiguration.sessionID,
                                 configuration_in.streamSocketConfiguration.useThreadPerConnection,
                                 RPG_NET_STATISTICS_COLLECTION_INTERVAL))
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to initialize module: \"%s\", aborting\n"),
               mySocketHandler.name()));

    return false;
  } // end IF

  // enqueue the module...
  // *NOTE*: push()ing the module will open() it
  // --> set the argument that is passed along
  mySocketHandler.arg (&const_cast<RPG_Net_ConfigPOD&>(configuration_in));
  if (inherited::push(&mySocketHandler) == -1)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("failed to ACE_Stream::push(\"%s\"): \"%m\", aborting\n"),
               mySocketHandler.name()));

    return false;
  } // end IF

  // -------------------------------------------------------------

  // set (session) message allocator
  inherited::myAllocator = configuration_in.streamSocketConfiguration.messageAllocator;

  // OK: all went well
  inherited::myIsInitialized = true;
//   inherited::dump_state();

  return true;
}

void
Net_Stream::ping()
{
  NETWORK_TRACE(ACE_TEXT("Net_Stream::ping"));

  RPG_Net_Module_ProtocolHandler* protocolHandler_impl = NULL;
  protocolHandler_impl = dynamic_cast<RPG_Net_Module_ProtocolHandler*>(myProtocolHandler.writer());
  if (!protocolHandler_impl)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("dynamic_cast<RPG_Net_Module_ProtocolHandler) failed> (aborting\n")));

    return;
  } // end IF

  // delegate to this module...
  protocolHandler_impl->handleTimeout(NULL);
}

unsigned int
Net_Stream::getSessionID() const
{
  NETWORK_TRACE(ACE_TEXT("Net_Stream::getSessionID"));

  MODULE_TYPE* module = &const_cast<RPG_Net_Module_SocketHandler_Module&>(mySocketHandler);
  RPG_Net_Module_SocketHandler* socketHandler_impl = NULL;
  socketHandler_impl = dynamic_cast<RPG_Net_Module_SocketHandler*>(module->writer());
  if (!socketHandler_impl)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("dynamic_cast<RPG_Net_Module_SocketHandler> failed> (aborting\n")));

    return 0;
  } // end IF

  return socketHandler_impl->getSessionID();
}

bool
Net_Stream::collect(RPG_Net_RuntimeStatistic& data_out) const
{
  NETWORK_TRACE(ACE_TEXT("Net_Stream::collect"));

  RPG_NET_MODULE_RUNTIMESTATISTICS_T* runtimeStatistic_impl = NULL;
  runtimeStatistic_impl = dynamic_cast<RPG_NET_MODULE_RUNTIMESTATISTICS_T*>(const_cast<RPG_Net_Module_RuntimeStatistic_Module&>(myRuntimeStatistic).writer());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG((LM_ERROR,
               ACE_TEXT("dynamic_cast<RPG_Net_Module_RuntimeStatistic) failed> (aborting\n")));

    return false;
  } // end IF

  // delegate to this module...
  return runtimeStatistic_impl->collect(data_out);
}

void
Net_Stream::report() const
{
  NETWORK_TRACE(ACE_TEXT("Net_Stream::report"));

//   RPG_Net_Module_RuntimeStatistic* runtimeStatistic_impl = NULL;
//   runtimeStatistic_impl = dynamic_cast<RPG_Net_Module_RuntimeStatistic*> (//                                            myRuntimeStatistic.writer());
//   if (!runtimeStatistic_impl)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("dynamic_cast<RPG_Net_Module_RuntimeStatistic) failed> (aborting\n")));
//
//     return;
//   } // end IF
//
//   // delegate to this module...
//   return (runtimeStatistic_impl->report());

  // just a dummy
  ACE_ASSERT(false);

  ACE_NOTREACHED(return;)
}
