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

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

template <class STREAM>
ACE_INLINE int
ACE_IOStream_alt_T<STREAM>::eof (void) const
{
  // Get the timeout value of the streambuf
  ACE_Time_Value *timeout = this->streambuf_->recv_timeout (0);

  // Reset the timeout value of the streambuf.
  (void) this->streambuf_->recv_timeout (timeout);

  char c;
  int rval = this->streambuf_->recv_n (&c,
                                       sizeof c,
                                       MSG_PEEK,
                                       timeout);

  // Timeout, not an eof
  if (this->streambuf_->timeout())
    return 0;

  // No timeout, got enough data:  not eof
  if (rval == sizeof(char))
    return 0;

  // No timeout, not enough data:  definately eof
  return 1;
}

ACE_END_VERSIONED_NAMESPACE_DECL
