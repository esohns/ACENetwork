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

#include "IRC_client_stream.h"

#include <string>

IRC_Client_Stream::IRC_Client_Stream ()
 : inherited ()
 , IRCMarshal_ (ACE_TEXT_ALWAYS_CHAR ("IRCMarshal"),
                NULL)
 , IRCParser_ (ACE_TEXT_ALWAYS_CHAR ("IRCParser"),
               NULL)
 //, IRCHandler_ (ACE_TEXT_ALWAYS_CHAR ("IRCHandler"),
 //               NULL)
 , runtimeStatistic_ (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic"),
                      NULL)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::IRC_Client_Stream"));

  // remember the ones we "own"...
  // *TODO*: clean this up
  // *NOTE*: one problem is that we need to explicitly close() all
  // modules which we have NOT enqueued onto the stream (e.g. because init()
  // failed...)
  inherited::availableModules_.insert_tail (&IRCMarshal_);
  inherited::availableModules_.insert_tail (&IRCParser_);
  inherited::availableModules_.insert_tail (&runtimeStatistic_);

  // fix ACE bug: modules should initialize their "next" member to NULL !
//   for (MODULE_CONTAINERITERATOR_TYPE iter = myAvailableModules.begin();
  inherited::MODULE_T* module_p = NULL;
  for (ACE_DLList_Iterator<inherited::MODULE_T> iterator (inherited::availableModules_);
       iterator.next (module_p);
       iterator.advance ())
    module_p->next (NULL);
}

IRC_Client_Stream::~IRC_Client_Stream ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::~IRC_Client_Stream"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

bool
IRC_Client_Stream::initialize (const IRC_Client_StreamConfiguration& configuration_in)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::initialize"));

  // sanity check(s)
  ACE_ASSERT (!inherited::isInitialized_);

  // things to be done here:
  // - create modules (done for the ones "owned" by the stream itself)
  // - initialize modules
  // - push them onto the stream (tail-first) !

  //  ACE_OS::memset (&inherited::state_, 0, sizeof (inherited::state_));
  inherited::state_.sessionID = configuration_in.sessionID;

  // ******************* Runtime Statistics ************************
  IRC_Client_Module_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
  runtimeStatistic_impl =
    dynamic_cast<IRC_Client_Module_Statistic_WriterTask_t*> (runtimeStatistic_.writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF
  if (!runtimeStatistic_impl->initialize (configuration_in.streamConfiguration.statisticReportingInterval,
                                          configuration_in.streamConfiguration.messageAllocator))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (runtimeStatistic_.name ())));
    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push (&runtimeStatistic_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
                ACE_TEXT (runtimeStatistic_.name ())));
    return false;
  } // end IF

//   // ******************* IRC Handler ************************
//   IRC_Client_Module_IRCHandler* IRCHandler_impl = NULL;
//   IRCHandler_impl = dynamic_cast<IRC_Client_Module_IRCHandler*> (//                                      myIRCHandler.writer());
//   if (!IRCHandler_impl)
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("dynamic_cast<IRC_Client_Module_IRCHandler) failed> (aborting\n")));
//     return false;
//   } // end IF
//   if (!IRCHandler_impl->init(configuration_in.messageAllocator,
//                              (configuration_in.clientPingInterval ? false // servers shouldn't receive "pings" in the first place
//                                                                   : RPG_NET_DEF_CLIENT_PING_PONG), // auto-answer "ping" as a client ?...
//                              (configuration_in.clientPingInterval == 0))) // clients print ('.') dots for received "pings"...
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to initialize module: \"%s\", aborting\n"),
//                myIRCHandler.name()));
//     return false;
//   } // end IF
//
//   // enqueue the module...
//   if (inherited::push(&myIRCHandler))
//   {
//     ACE_DEBUG((LM_ERROR,
//                ACE_TEXT("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
//                myIRCHandler.name()));
//     return false;
//   } // end IF

  // ******************* IRC Parser ************************
  IRC_Client_Module_IRCParser* IRCParser_impl = NULL;
  IRCParser_impl =
    dynamic_cast<IRC_Client_Module_IRCParser*> (IRCParser_.writer ());
  if (!IRCParser_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_IRCParser) failed> (aborting\n")));
    return false;
  } // end IF
  if (!IRCParser_impl->initialize (configuration_in.streamConfiguration.messageAllocator, // message allocator
                                   configuration_in.crunchMessageBuffers,                 // "crunch" messages ?
                                   configuration_in.debugScanner,                         // debug scanner ?
                                   configuration_in.debugParser))                         // debug parser ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (IRCParser_.name ())));
    return false;
  } // end IF

  // enqueue the module...
  if (inherited::push (&IRCParser_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
                ACE_TEXT (IRCParser_.name ())));
    return false;
  } // end IF

  // ******************* IRC Marshal ************************
  IRC_Client_Module_IRCSplitter* IRCSplitter_impl = NULL;
  IRCSplitter_impl =
   dynamic_cast<IRC_Client_Module_IRCSplitter*> (IRCMarshal_.writer ());
  if (!IRCSplitter_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_IRCSplitter> failed, aborting\n")));
    return false;
  } // end IF
  if (!IRCSplitter_impl->initialize (configuration_in.streamConfiguration.messageAllocator, // message allocator
                                     configuration_in.crunchMessageBuffers,                 // "crunch" messages ?
                                     &(inherited::state_),                                  // state handle
                                     0,                                                     // DON'T collect statistics
                                     configuration_in.debugScanner))                        // debug scanning ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize module: \"%s\", aborting\n"),
                ACE_TEXT (IRCMarshal_.name ())));
    return false;
  } // end IF

  // enqueue the module...
  // *NOTE*: push()ing the module will open() it
  // --> set the argument that is passed along (head module needs this)
  IRCMarshal_.arg (const_cast<IRC_Client_StreamConfiguration&> (configuration_in).sessionData);
  if (inherited::push (&IRCMarshal_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::push() module: \"%s\", aborting\n"),
                ACE_TEXT (IRCMarshal_.name ())));
    return false;
  } // end IF

  // set (session) message allocator
  // *TODO*: clean this up ! --> sanity check
  ACE_ASSERT (configuration_in.streamConfiguration.messageAllocator);
  inherited::allocator_ = configuration_in.streamConfiguration.messageAllocator;

  inherited::isInitialized_ = true;
//   inherited::dump_state();

  return true;
}

bool
IRC_Client_Stream::collect (IRC_Client_RuntimeStatistic& data_out)
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::collect"));

  IRC_Client_Module_Statistic_WriterTask_t* runtimeStatistic_impl = NULL;
  runtimeStatistic_impl =
    dynamic_cast<IRC_Client_Module_Statistic_WriterTask_t*> (const_cast<IRC_Client_Module_RuntimeStatistic_Module&> (runtimeStatistic_).writer ());
  if (!runtimeStatistic_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<IRC_Client_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // delegate to this module...
  return (runtimeStatistic_impl->collect (data_out));
}

void
IRC_Client_Stream::report () const
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::report"));

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
  ACE_ASSERT (false);

  ACE_NOTREACHED (return;)
}

void
IRC_Client_Stream::ping ()
{
  NETWORK_TRACE (ACE_TEXT ("IRC_Client_Stream::ping"));

  // delegate to the head module, skip over ACE_Stream_Head...
  MODULE_T* module_p = inherited::head ();
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));
    return;
  } // end IF
  module_p = module_p->next ();
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no head module found, returning\n")));
    return;
  } // end IF

  // sanity check: head == tail ? --> no modules have been push()ed (yet) !
  if (module_p == inherited::tail ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no modules have been enqueued yet --> nothing to do !, returning\n")));
    return;
  } // end IF

  ISTREAM_CONTROL_T* control_impl = NULL;
  control_impl = dynamic_cast<ISTREAM_CONTROL_T*> (module_p->reader ());
  if (!control_impl)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: dynamic_cast<Stream_IStreamControl> failed, returning\n"),
                ACE_TEXT (module_p->name ())));
    return;
  } // end IF

  // *TODO*
  try
  {
//    control_impl->stop ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IStreamControl::stop (module: \"%s\"), returning\n"),
                ACE_TEXT (module_p->name ())));
    return;
  }
}
