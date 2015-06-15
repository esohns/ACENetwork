/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

//=============================================================================
/**
 *  @file    IOStream_alt_T.h
 *
 *  @author Erik Sohns <eriksohns@123mail.org>
 *
 */
//=============================================================================
#ifndef ACE_IOSTREAM_ALT_T_H
#define ACE_IOSTREAM_ALT_T_H

#include /**/ "ace/pre.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#  include "ace/Global_Macros.h"
#  include "ace/iosfwd.h"
//#  include "ace/IOStream.h"
//#  include "ace/Time_Value.h"

#  if defined (ACE_LACKS_IOSTREAM_FX)
#   include "ace/os_include/os_ctype.h"
#  endif /**/

#include "IOStream_alt_macros.h"
#include "Streambuf_alt.h"

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

// forward declarations
class ACE_Time_Value;

//#  if defined (ACE_HAS_STRING_CLASS)
//template <class STREAM> STREAM & operator>> (STREAM &stream, ACE_Quoted_String &str);
//template <class STREAM> STREAM & operator<< (STREAM &stream, ACE_Quoted_String &str);
//#  endif /* defined (ACE_HAS_STRING_CLASS) */

/**
 * @class ACE_IOStream_alt_T
 *
 * @brief A template adapter for creating an iostream-like object using
 * an ACE IPC Stream for the actual I/O.  Iostreams use an
 * underlying streambuf object for the IO interface.  The
 * iostream class and derivatives provide you with a host of
 * convenient operators that access the streambuf.
 *
 * We inherit all characteristics of iostream and your <STREAM>
 * class.  When you create a new class from this template, you
 * can use it anywhere you would have used your original
 * <STREAM> class.
 * To create an iostream for your favorite ACE IPC class (e.g.,
 * ACE_SOCK_Stream), feed that class to this template's
 * <STREAM> parameter, e.g.,
 * typedef ACE_Svc_Handler<ACE_SOCK_iostream,
 * ACE_INET_Addr, ACE_NULL_SYNCH>
 * Service_Handler;
 * Because the operators in the iostream class are not virtual,
 * you cannot easily provide overloads in your custom
 * ACE_IOStream classes.  To make these things work correctly,
 * you need to overload ALL operators of the ACE_IOStream you
 * create. I've attempted to do that here to make things easier
 * for you but there are no guarantees.
 * In the iostream.cpp file is an example of why it is necessary
 * to overload all of the get/put operators when you want to
 * customize only one or two.
 */
template <typename STREAM>
class ACE_IOStream_alt_T : public iostream, public STREAM
{
public:
  // = Initialization and termination methods.
  ACE_IOStream_alt_T (STREAM &stream,
                      u_int streambuf_size = ACE_STREAMBUF_SIZE);

  /**
   * The default constructor.  This will initialize your STREAM and
   * then setup the iostream baseclass to use a custom streambuf based
   * on STREAM.
   */
  ACE_IOStream_alt_T (u_int streambuf_size = ACE_STREAMBUF_SIZE);

  /// We have to get rid of the <streambuf_> ourselves since we gave it
  /// to the <iostream> base class;
  virtual ~ACE_IOStream_alt_T (void);

  /// The only ambiguity in the multiple inheritance is the <close>
  /// function.
  virtual int close (void);

  /**
   * Returns 1 if we're at the end of the <STREAM>, i.e., if the
   * connection has closed down or an error has occurred, else 0.
   * Under the covers, <eof> calls the streambuf's @a timeout function
   * which will reset the timeout flag.  As as result, you should save
   * the return of <eof> and check it instead of calling <eof>
   * successively.
   */
  int eof (void) const;

#  if defined (ACE_HAS_STRING_CLASS)
  /**
   * A simple string operator.  The base <iostream> has them for char*
   * but that isn't always the best thing for a <String>.  If we don't
   * provide our own here, we may not get what we want.
   */
  virtual ACE_IOStream_alt_T<STREAM> &operator>> (ACE_IOStream_String &v);

  /// The converse of the <String::put> operator.
  virtual ACE_IOStream_alt_T<STREAM> &operator<< (ACE_IOStream_String &v);

#  endif /* ACE_HAS_STRING_CLASS */
  // = Using the macros to provide get/set operators.
  // *NOTE*: due to limitations of the GET/PUT macros,
  //         only output is supported at this time
  //         (see above)
  GETPUT_FUNC_SET (ACE_IOStream_alt_T<STREAM>)
  //PUT_FUNC_SET (ACE_IOStream_alt_T<STREAM>)

#  if defined (ACE_LACKS_IOSTREAM_FX)
  virtual int ipfx (int noskip = 0)
    {
      if (good ())
        {
          if (tie () != 0)
             tie ()->flush ();
          if (!noskip && flags () & skipws)
            {
              int ch;
              while (isspace (ch = rdbuf ()->sbumpc ()))
                continue;
              if (ch != EOF)
                  rdbuf ()->sputbackc (ch);
            }
          if (good ())
              return 1;
        }
#    if !defined (ACE_WIN32)
      // MS VC++ 5.0 doesn't declare setstate.
      setstate (failbit);
#    endif /* !ACE_WIN32 */
      return (0);
    }
  virtual int ipfx0 (void)         {  return ipfx (0); }  // Optimized ipfx(0)
  virtual int ipfx1 (void)                                // Optimized ipfx(1)
    {
      if (good ())
        {
          if (tie () != 0)
             tie ()->flush ();
          if (good ())
              return 1;
        }
#    if !defined (ACE_WIN32)
      // MS VC++ 5.0 doesn't declare setstate.
      setstate (failbit);
#    endif /* !ACE_WIN32 */
      return (0);
    }
  virtual void isfx (void) {  return; }
  virtual int opfx (void)
    {
      if (good () && tie () != 0)
        tie ()->flush ();
      return good ();
    }
  virtual void osfx (void) {  if (flags () & unitbuf) flush (); }
#  else
#    if defined (__GNUC__)
  virtual int ipfx0 (void) { return iostream::ipfx0 (); }  // Optimized ipfx(0)
  virtual int ipfx1 (void) { return iostream::ipfx1 (); }  // Optimized ipfx(1)
#    else
  inline virtual int ipfx0 (void) { return iostream::ipfx (0); }
  inline virtual int ipfx1 (void) { return iostream::ipfx (1); }
#    endif /* __GNUC__ */
  virtual int ipfx (int need = 0) {  return iostream::ipfx (need); }
  virtual void isfx (void)        {  iostream::isfx (); }
  virtual int opfx (void)         {  return iostream::opfx (); }
  virtual void osfx (void)        {  iostream::osfx (); }
#  endif /* ACE_LACKS_IOSTREAM_FX */

  /// Allow the programmer to provide a timeout for read operations.
  /// Give it a pointer to NULL to block forever.
  ACE_IOStream_alt_T<STREAM> & operator>> (ACE_Time_Value *&tv);

protected:
  /// This is where all of the action takes place.  The streambuf_ is
  /// the interface to the underlying STREAM.
  typedef ACE_Streambuf_alt_T<STREAM> STREAMBUF_T;
  STREAMBUF_T *streambuf_;

private:
  // = Private methods.

  // We move these into the private section so that they cannot be
  // used by the application programmer.  This is necessary because
  // streambuf_ will be buffering IO on the STREAM object.  If these
  // functions were used in your program, there is a danger of getting
  // the datastream out of sync.
  ACE_UNIMPLEMENTED_FUNC (ssize_t send (...))
  ACE_UNIMPLEMENTED_FUNC (ssize_t recv (...))
  ACE_UNIMPLEMENTED_FUNC (ssize_t send_n (...))
  ACE_UNIMPLEMENTED_FUNC (ssize_t recv_n (...))
};

ACE_END_VERSIONED_NAMESPACE_DECL

#  if defined (__ACE_INLINE__)
//#    include "ace/IOStream_T.inl"
#    include "IOStream_alt_T.inl"
#  endif /* __ACE_INLINE__ */

#  if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
//#    include "ace/IOStream_T.cpp"
#    include "IOStream_alt_T.cpp"
#  endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#  if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
//#    pragma implementation ("IOStream_T.cpp")
#    pragma implementation ("IOStream_alt_T.cpp")
#  endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#include /**/ "ace/post.h"
#endif /* ACE_IOSTREAM_ALT_T_H */
