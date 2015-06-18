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
#include "stdafx.h"

#ifndef ACE_IOSTREAM_ALT_T_CPP
#define ACE_IOSTREAM_ALT_T_CPP

#include "IOStream_alt_T.h"

#include "ace/Log_Msg.h"
#include "ace/OS_Memory.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#if !defined (__ACE_INLINE__)
//#include "ace/IOStream_T.inl"
//#include "IOStream_alt_T.inl"
#endif /* !__ACE_INLINE__ */

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

// The typical constructor.  This will initialize your STREAM and then
// setup the iostream baseclass to use a custom streambuf based on
// STREAM.
template <class STREAM>
ACE_IOStream_alt_T<STREAM>::ACE_IOStream_alt_T (STREAM &stream,
                                                u_int streambuf_size)
 : iostream (0)
 , STREAM (stream)
 , streambuf_ (NULL)
{
  ACE_NEW_NORETURN (streambuf_,
                    STREAMBUF_T (this,
                                 streambuf_size));
  if (!streambuf_)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, continuing\n")));
  else
    iostream::init (streambuf_);
}

template <class STREAM>
ACE_IOStream_alt_T<STREAM>::ACE_IOStream_alt_T (u_int streambuf_size)
 : iostream (0)
 , STREAM ()
 , streambuf_ (NULL)
{
  ACE_NEW_NORETURN (streambuf_,
                    STREAMBUF_T (this,
                                 streambuf_size));
  if (!streambuf_)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, continuing\n")));
  else
    iostream::init (streambuf_);
}

// We have to get rid of the streambuf_ ourselves since we gave it to
// iostream ()
template <class STREAM>
ACE_IOStream_alt_T<STREAM>::~ACE_IOStream_alt_T (void)
{
  delete this->streambuf_;
}

// The only ambituity in the multiple inheritance is the close ()
// function.
template <class STREAM>
int
ACE_IOStream_alt_T<STREAM>::close (void)
{
  return STREAM::close ();
}

template <class STREAM>
ACE_IOStream_alt_T<STREAM>&
ACE_IOStream_alt_T<STREAM>::operator>> (ACE_Time_Value*& tv)
{
  ACE_Time_Value *old_tv = this->streambuf_->recv_timeout (tv);
  tv = old_tv;
  return *this;
}

#if defined (ACE_HAS_STRING_CLASS)

// A simple string operator.  The base iostream has 'em for char* but
// that isn't always the best thing for a String.  If we don't provide
// our own here, we may not get what we want.

template <class STREAM> ACE_IOStream_alt_T<STREAM> &
ACE_IOStream_alt_T<STREAM>::operator>> (ACE_IOStream_String &v)
{
  if (ipfx0 ())
    {
      char c;
      this->get (c);

      for (v = c;
           this->get (c) && !isspace (c);
           v += c)
        continue;
    }

  isfx ();

  return *this;
}

template <class STREAM> ACE_IOStream_alt_T<STREAM> &
ACE_IOStream_alt_T<STREAM>::operator<< (ACE_IOStream_String &v)
{
  if (opfx ())
    {
#if defined (ACE_WIN32) && defined (_MSC_VER)
      for (int i = 0; i < v.GetLength (); ++i)
#else
      for (u_int i = 0; i < (u_int) v.length (); ++i)
#endif /* ACE_WIN32 && defined (_MSC_VER) */
        this->put (v[i]);
    }

  osfx ();

  return *this;
}

//// A more clever put operator for strings that knows how to deal with
//// quoted strings containing back-quoted quotes.

//template <class STREAM> STREAM &
//operator>> (STREAM &stream,
//            ACE_Quoted_String &str)
//{
//  char c;

//  if (!(stream >> c)) // eat space up to the first char
//    // stream.set (ios::eofbit|ios::failbit);
//    return stream;

//  str = "";     // Initialize the string

//  // if we don't have a quote, append until we see space
//  if (c != '"')
//    for (str = c; stream.get (c) && !isspace (c); str += c)
//      continue;
//  else
//    for (; stream.get (c) && c != '"'; str += c)
//      if (c == '\\')
//        {
//          stream.get (c);
//          if (c != '"')
//            str += '\\';
//        }

//  return stream;
//}

//template <class STREAM> STREAM &
//operator<< (STREAM &stream,
//            ACE_Quoted_String &str)
//{
//  stream.put ('"');

//  for (u_int i = 0; i < str.length (); ++i)
//    {
//      if (str[i] == '"')
//        stream.put ('\\');
//      stream.put (str[i]);
//    }

//  stream.put ('"');

//  return stream;
//}

#endif /* ACE_HAS_STRING_CLASS */

ACE_END_VERSIONED_NAMESPACE_DECL

#endif /* ACE_IOSTREAM_ALT_T_CPP */
