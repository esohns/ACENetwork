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

#ifndef HTTP_MODULE_BISECTOR_H
#define HTTP_MODULE_BISECTOR_H

#include <ace/Global_Macros.h>

#include "stream_headmoduletask_base.h"

//// define/declare the lexer's prototype (see HTTP_bisector.h)
typedef void* yyscan_t;
extern int HTTP_Bisector_lex (yyscan_t);
//// *TODO*: this should be part of HTTP_bisector.h
//#define YY_DECL extern int HTTP_Bisector_lex (yyscan_t)
//YY_DECL;

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;
typedef struct yy_buffer_state* YY_BUFFER_STATE;

template <typename LockType,
          ////////////////////////////////
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType>
class HTTP_Module_Bisector_T
 : public Stream_HeadModuleTaskBase_T<LockType,
                                      ACE_SYNCH_USE,
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
                                      StatisticContainerType>
{
 public:
  HTTP_Module_Bisector_T (LockType* = NULL, // lock handle (state machine)
                          ////////////////
                          bool = false);    // auto-start ?
  virtual ~HTTP_Module_Bisector_T ();

//#if defined (__GNUG__) || defined (_MSC_VER)
#if defined (_MSC_VER)
  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T::initialize;
#endif

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?

  // catch the session ID...
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: this reuses the interface to implement timer-based data collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ACE_SYNCH_USE,
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
                                      StatisticContainerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_Bisector_T ())
  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_Bisector_T (const HTTP_Module_Bisector_T&))
  ACE_UNIMPLEMENTED_FUNC (HTTP_Module_Bisector_T& operator= (const HTTP_Module_Bisector_T&))

  // convenience types
  //typedef HTTP_Client_SessionData SESSIONDATA_T;

  //// helper methods
  //bool putStatisticMessage (const StatisticContainerType&) const;

  // helper methods (to drive the scanner)
  bool scan_begin (char*,   // base address
                   size_t); // length of data block
  void scan_end ();

  // scanner
  YY_BUFFER_STATE    bufferState_;
  yyscan_t           context_;
  unsigned int       numberOfFrames_;

  // message buffer(s)
  ACE_Message_Block* buffer_; // <-- continuation chain
  unsigned int       messageLength_;
};

// include template definition
#include "bittorrent_module_bisector.inl"

#endif
