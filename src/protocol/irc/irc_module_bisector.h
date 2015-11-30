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

#ifndef IRC_MODULE_BISECTOR_H
#define IRC_MODULE_BISECTOR_H

#include "ace/Global_Macros.h"

#include "common_istatistic.h"

#include "stream_headmoduletask_base.h"
#include "stream_streammodule_base.h"

//// define/declare the lexer's prototype (see irc_bisector.h)
typedef void* yyscan_t;
extern int IRC_Bisector_lex (yyscan_t);
//// *TODO*: this should be part of irc_bisector.h
//#define YY_DECL extern int IRC_Bisector_lex (yyscan_t)
//YY_DECL;

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;
typedef struct yy_buffer_state* YY_BUFFER_STATE;

template <typename LockType,
          ///////////////////////////////
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          ///////////////////////////////
          typename ConfigurationType,
          ///////////////////////////////
          typename StreamStateType,
          ///////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ///////////////////////////////
          typename StatisticContainerType>
class IRC_Module_Bisector_T
 : public Stream_HeadModuleTaskBase_T<LockType,
                                      ///
                                      TaskSynchType,
                                      TimePolicyType,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StreamStateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType>
   // implement this to have a generic (timed) event handler to trigger
   // periodic statistic collection
 , public Common_IStatistic_T<StatisticContainerType>
{
 public:
  IRC_Module_Bisector_T ();
  virtual ~IRC_Module_Bisector_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<LockType,
                                    /////
                                    TaskSynchType,
                                    TimePolicyType,
                                    SessionMessageType,
                                    ProtocolMessageType,
                                    /////
                                    ConfigurationType,
                                    /////
                                    StreamStateType,
                                    /////
                                    SessionDataType,
                                    SessionDataContainerType>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?

  // catch the session ID...
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: this reuses the interface to implement timer-based data collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ///
                                      TaskSynchType,
                                      TimePolicyType,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StreamStateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (IRC_Module_Bisector_T (const IRC_Module_Bisector_T&))
  ACE_UNIMPLEMENTED_FUNC (IRC_Module_Bisector_T& operator= (const IRC_Module_Bisector_T&))

  // convenience types
  typedef Stream_StatisticHandler_Reactor_T<StatisticContainerType> STATISTICHANDLER_T;
  //typedef IRC_Client_SessionData SESSIONDATA_T;

  // helper methods
  bool putStatisticMessage (const StatisticContainerType&) const;

  // helper methods (to drive the scanner)
  bool scan_begin (char*,   // base address
                   size_t); // length of data block
  void scan_end ();

  // timer
  STATISTICHANDLER_T statisticCollectHandler_;
  long               statisticCollectHandlerID_;

  // scanner
  YY_BUFFER_STATE    bufferState_;
  yyscan_t           context_;
  unsigned int       numberOfFrames_;

  // message buffer(s)
  ACE_Message_Block* buffer_; // <-- continuation chain
  unsigned int       messageLength_;

  bool               isInitialized_;
};

// include template implementation
#include "irc_module_bisector.inl"

#endif
