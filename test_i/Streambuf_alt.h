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
*  @file    Streambuf_alt.h
*
*  @author Erik Sohns <eriksohns@123mail.org>
*
*/
//=============================================================================
#ifndef ACE_STREAMBUF_ALT_H
#define ACE_STREAMBUF_ALT_H

#include /**/ "ace/pre.h"
//#include /**/ "ace/ACE_export.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

// Needed on Windows for streambuf
// FUZZ: disable check_for_streams_include
#include "ace/streams.h"
//#include "ace/iosfwd.h"

#include "ace/Time_Value.h"

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

/**
 * @class ACE_Streambuf_alt
 *
 * @brief Create your custom streambuf by providing and ACE_*_Stream
 * object to this template.  I have tested it with
 * ACE_SOCK_Stream and it should work fine for others as well.
 *
 * For any iostream object, the real work is done by the
 * underlying streambuf class.  That is what we create here.
 * A streambuf has an internal buffer area into which data is
 * read and written as the iostream requests and provides data.
 * At some point during the read process, the iostream will
 * realize that the streambuf has no more data.  The underflow
 * function of the streambuf is then called.
 * Likewise, during the write process, the iostream will
 * eventually notice that the streabuf's buffer has become full
 * and will invoke the overflow function.
 * The empty/full state of the read/write "buffers" are
 * controled by two sets pointers.  One set is dedicated to
 * read, the other to write.  These pointers, in turn, reference
 * a common buffer that is to be shared by both read and write
 * operations.  It is this common buffer to which data is
 * written and from which it is read.
 * The common buffer is used by functions of the streambuf as
 * well as the iostream.  Because of this and the fact that it
 * is "shared" by both read and write operators, there is a
 * danger of data corruption if read and write operations are
 * allowed to take place "at the same time".
 * To prevent data corruption, we manipulate the read and write
 * pointer sets so that the streambuf is in either a read-mode
 * or write-mode at all times and can never be in both modes at
 * the same time.
 * In the constructor: set the read and write sets to NULL This
 * causes the underflow or overflow operators to be invoked at
 * the first IO activity of the iostream.
 * In the underflow function we arrange for the common buffer to
 * reference our read buffer and for the write pointer set to be
 * disabled.  If a write operation is performed by the iostream
 * this will cause the overflow function to be invoked.
 * In the overflow function we arrange for the common buffer to
 * reference our write buffer and for the read pointer set to be
 * disabled.  This causes the underflow function to be invoked
 * when the iostream "changes our mode".
 * The overflow function will also invoke the send_n function to
 * flush the buffered data to our peer.  Similarly, the sync and
 * syncout functions will cause send_n to be invoked to send the
 * data.
 * Since socket's and the like do not support seeking, there can
 * be no method for "syncing" the input.  However, since we
 * maintain separate read/write buffers, no data is lost by
 * "syncing" the input.  It simply remains buffered.
 */
//class ACE_Export ACE_Streambuf_alt : public streambuf
class ACE_Streambuf_alt : public streambuf
{
public:
  /**
   * If the default allocation strategy were used the common buffer
   * would be deleted when the object destructs.  Since we are
   * providing separate read/write buffers, it is up to us to manage
   * their memory.
   */
  virtual ~ACE_Streambuf_alt (void);

  /// Get the current Time_Value pointer and provide a new one.
  ACE_Time_Value *recv_timeout (ACE_Time_Value *tv = 0);

  /**
   * Use this to allocate a new/different buffer for put operations.
   * If you do not provide a buffer pointer, one will be allocated.
   * That is the preferred method.  If you do provide a buffer, the
   * size must match that being used by the get buffer.  If
   * successful, you will receive a pointer to the current put buffer.
   * It is your responsibility to delete this memory when you are done
   * with it.
   */
  char *reset_put_buffer (char *newBuffer = 0,
                          u_int _streambuf_size = 0,
                          u_int _pptr = 0 );

  /// Return the number of bytes to be 'put' onto the stream media.
  ///    pbase + put_avail = pptr
  u_int put_avail (void);

  /**
   * Use this to allocate a new/different buffer for get operations.
   * If you do not provide a buffer pointer, one will be allocated.
   * That is the preferred method.  If you do provide a buffer, the
   * size must match that being used by the put buffer.  If
   * successful, you will receive a pointer to the current get buffer.
   * It is your responsibility to delete this memory when you are done
   * with it.
   */
  char *reset_get_buffer (char *newBuffer = 0,
                          u_int _streambuf_size = 0,
                          u_int _gptr = 0,
                          u_int _egptr = 0);

  /// Return the number of bytes not yet gotten.  eback + get_waiting =
  /// gptr
  u_int get_waiting (void);

  /// Return the number of bytes in the get area (includes some already
  /// gotten); eback + get_avail = egptr
  u_int get_avail (void);

  /// Query the streambuf for the size of its buffers.
  u_int streambuf_size (void);

  /// Did we take an error because of an IO operation timeout?
  /// @note Invoking this resets the flag.
  u_char timeout (void);

protected:
  ACE_Streambuf_alt (u_int streambuf_size,
                     int io_mode);

  /// Sync both input and output. See syncin/syncout below for
  /// descriptions.
  virtual int sync (void);

  // = Signatures for the underflow/overflow discussed above.
  virtual int underflow (void);

  /// The overflow function receives the character which caused the
  /// overflow.
  virtual int overflow (int c = EOF);

  /// Resets the <base> pointer and streambuf mode.  This is used
  /// internally when get/put buffers are allocatd.
  void reset_base (void);

protected:
  // = Two pointer sets for manipulating the read/write areas.
  char *eback_saved_;
  char *gptr_saved_;
  char *egptr_saved_;
  char *pbase_saved_;
  char *pptr_saved_;
  char *epptr_saved_;

  // = With cur_mode_ we keep track of our current IO mode.

  // This helps us to optimize the underflow/overflow functions.
  u_char cur_mode_;
  const u_char get_mode_;
  const u_char put_mode_;

  /// mode tells us if we're working for an istream, ostream, or
  /// iostream.
  int mode_;

  /// This defines the size of the input and output buffers.  It can be
  /// set by the object constructor.
  const u_int streambuf_size_;

  /// Did we take an error because of an IO operation timeout?
  u_char timeout_;

  /// We want to allow the user to provide Time_Value pointers to
  /// prevent infinite blocking while waiting to receive data.
  ACE_Time_Value recv_timeout_value_;
  ACE_Time_Value *recv_timeout_;

  /**
   * syncin is called when the input needs to be synced with the
   * source file.  In a filebuf, this results in the <seek> system
   * call being used.  We can't do that on socket-like connections, so
   * this does basically nothing.  That's safe because we have a
   * separate read buffer to maintain the already-read data.  In a
   * filebuf, the single common buffer is used forcing the <seek>
   * call.
   */
  int syncin (void);

  /// syncout() is called when the output needs to be flushed.  This is
  /// easily done by calling the peer's send_n function.
  int syncout (void);

  /// flushbuf() is the worker of syncout.  It is a separate function
  /// because it gets used sometimes in different context.
  int flushbuf (void);

  /**
   * fillbuf is called in a couple of places.  This is the worker of
   * underflow.  It will attempt to fill the read buffer from the
   * peer.
   */
  int fillbuf (void);

  /**
   * Used by fillbuf and others to get exactly one byte from the peer.
   * recv_n is used to be sure we block until something is available.
   * It is virtual because we really need to override it for
   * datagram-derived objects.
   */
  virtual int get_one_byte (void);

  /**
   * Stream connections and "unconnected connections" (ie --
   * datagrams) need to work just a little differently.  We derive
   * custom Streambuf objects for them and provide these functions at
   * that time.
   */
  virtual ssize_t send (char *buf,
                        ssize_t len) = 0;
  virtual ssize_t recv (char *buf,
                        ssize_t len,
                        ACE_Time_Value *tv = 0) = 0;
  virtual ssize_t recv (char *buf,
                        ssize_t len,
                        int flags,
                        ACE_Time_Value *tv = 0) = 0;
  virtual ssize_t recv_n (char *buf,
                          ssize_t len,
                          int flags = 0,
                          ACE_Time_Value *tv = 0) = 0;

  virtual ACE_HANDLE get_handle (void);

//#  if defined (ACE_HAS_STANDARD_CPP_LIBRARY) && (ACE_HAS_STANDARD_CPP_LIBRARY != 0) && !defined (ACE_USES_OLD_IOSTREAMS)
#  if defined (ACE_HAS_STANDARD_CPP_LIBRARY) && (ACE_HAS_STANDARD_CPP_LIBRARY != 0)
  char *base (void) const
    {
      return cur_mode_ == get_mode_ ? eback_saved_
        : cur_mode_ == put_mode_ ? pbase_saved_
        : 0;
    }
  char *ebuf (void) const
    {
      return cur_mode_ == 0 ? 0 : base () + streambuf_size_;
    }

  int blen (void) const
    {
      return streambuf_size_;
    }

  void setb (char* b, char* eb, int /* a */=0)
    {
      setbuf (b, (eb - b));
    }

  int out_waiting (void)
    {
      return pptr () - pbase ();
    }
#  endif /* ACE_HAS_STANDARD_CPP_LIBRARY */
};

/////////////////////////////////////////

template <typename STREAM>
class ACE_Streambuf_alt_T : public ACE_Streambuf_alt
{
public:
  /**
  * We will be given a STREAM by the iostream object which creates
  * us.  See the ACE_IOStream template for how that works.  Like
  * other streambuf objects, we can be input-only, output-only or
  * both.
  */
  ACE_Streambuf_alt_T (STREAM *peer,
                       u_int streambuf_size = ACE_STREAMBUF_SIZE,
                       int io_mode = ios::in | ios::out);

  virtual ssize_t send (char *buf, ssize_t len);

  virtual ssize_t recv (char *buf,
                        ssize_t len,
                        ACE_Time_Value *tv = 0);

  virtual ssize_t recv (char *buf,
                        ssize_t len,
                        int flags,
                        ACE_Time_Value * tv = 0);

  virtual ssize_t recv_n (char *buf,
                          ssize_t len,
                          int flags = 0,
                          ACE_Time_Value *tv = 0);

protected:
  virtual ACE_HANDLE get_handle (void);

  /// This will be our ACE_SOCK_Stream or similar object.
  STREAM *peer_;
};

// *TODO*: do this inline with the ACE coding standard
// include template definition
#include "Streambuf_alt.inl"

#  if defined (__ACE_INLINE__)
//#    include "ace/IOStream_T.inl"
//#    include "Streambuf_alt.inl"
#  endif /* __ACE_INLINE__ */

ACE_END_VERSIONED_NAMESPACE_DECL

#include /**/ "ace/post.h"
#endif /* ACE_STREAMBUF_ALT_H */
