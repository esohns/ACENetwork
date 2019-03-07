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

// We will be given a STREAM by the iostream object which creates us.
// See the ACE_IOStream template for how that works.  Like other
// streambuf objects, we can be input-only, output-only or both.

template <class STREAM>
ACE_Streambuf_alt_T<STREAM>::ACE_Streambuf_alt_T (STREAM *peer,
                                                  u_int streambuf_size,
                                                  int io_mode)
 : ACE_Streambuf_alt (streambuf_size, io_mode)
 , peer_ (peer)
{
  // A streambuf allows for unbuffered IO where every character is
  // read as requested and written as provided.  To me, this seems
  // terribly inefficient for socket-type operations, so I've disabled
  // it.  All of the work would be done by the underflow/overflow
  // functions anyway and I haven't implemented anything there to
  // support unbuffered IO.

#if !defined (ACE_LACKS_UNBUFFERED_STREAMBUF)
  this->unbuffered (0);
#endif /* ! ACE_LACKS_UNBUFFERED_STREAMBUF */

  // Linebuffered is similar to unbuffered.  Again, I don't have any
  // need for this and I don't see the advantage.  I believe this
  // would have to be supported by underflow/overflow to be effective.
#if !defined (ACE_LACKS_LINEBUFFERED_STREAMBUF)
  this->linebuffered (0);
#endif /* ! ACE_LACKS_LINEBUFFERED_STREAMBUF */
}

template <class STREAM>
ssize_t
ACE_Streambuf_alt_T<STREAM>::send (char *buf,
                                   ssize_t len)
{
  return peer_->send_n (buf, len);
}

template <class STREAM>
ssize_t
ACE_Streambuf_alt_T<STREAM>::recv (char *buf,
                                   ssize_t len,
                                   ACE_Time_Value *tv)
{
  return this->recv (buf, len, 0, tv);
}

template <class STREAM>
ssize_t
ACE_Streambuf_alt_T<STREAM>::recv (char *buf,
                                   ssize_t len,
                                   int flags,
                                   ACE_Time_Value * tv)
{
  this->timeout_ = 0;
  errno = ESUCCESS;
  ssize_t rval = peer_->recv (buf, len, flags, tv);
  if (errno == ETIME)
    this->timeout_ = 1;
  return rval;
}

template <class STREAM>
ssize_t
ACE_Streambuf_alt_T<STREAM>::recv_n (char *buf,
                                     ssize_t len,
                                     int flags,
                                     ACE_Time_Value *tv)
{
  return recv (buf, len, flags, tv);
}

template <class STREAM>
ACE_HANDLE
ACE_Streambuf_alt_T<STREAM>::get_handle (void)
{
  return peer_ ? peer_->get_handle () : 0;
}
