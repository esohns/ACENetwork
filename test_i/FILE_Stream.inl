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

//#include "ace/ACE.h"
//#include "ace/Log_Msg.h"
//#include "ace/OS_NS_sys_file.h"

ACE_BEGIN_VERSIONED_NAMESPACE_DECL

ACE_INLINE
ACE_FILE_Stream::ACE_FILE_Stream (void)
{
  // ACE_TRACE ("ACE_FILE_Stream::ACE_FILE_Stream");
}

ACE_INLINE
ACE_FILE_Stream::ACE_FILE_Stream (ACE_HANDLE h)
{
  // ACE_TRACE ("ACE_FILE_Stream::ACE_FILE_Stream");
  this->set_handle (h);
}

ACE_INLINE
ACE_FILE_Stream::~ACE_FILE_Stream (void)
{
  // ACE_TRACE ("ACE_FILE_Stream::~ACE_FILE_Stream");
}

//ACE_INLINE int
//ACE_FILE_Stream::close_reader (void)
//{
//  ACE_TRACE ("ACE_FILE_Stream::close_reader");
//  if (this->get_handle () != ACE_INVALID_HANDLE)
//    return ACE_OS::shutdown (this->get_handle (), ACE_SHUTDOWN_READ);
//  else
//    return 0;
//}

//// Shut down just the writing end of a ACE_FILE.

//ACE_INLINE int
//ACE_FILE_Stream::close_writer (void)
//{
//  ACE_TRACE ("ACE_FILE_Stream::close_writer");
//  if (this->get_handle () != ACE_INVALID_HANDLE)
//    return ACE_OS::shutdown (this->get_handle (), ACE_SHUTDOWN_WRITE);
//  else
//    return 0;
//}

ACE_INLINE ssize_t
ACE_FILE_Stream::recv (void* buf,
                       size_t n,
                       int flags,
                       const ACE_Time_Value *timeout) const
{
  //ACE_TRACE ("ACE_FILE_Stream::recv");
  ACE_UNUSED_ARG (flags);
  ACE_UNUSED_ARG (timeout);
  return ACE_FILE_IO::recv (buf, n);
}

ACE_INLINE ssize_t
ACE_FILE_Stream::recv (void *buf,
                       size_t n,
                       const ACE_Time_Value *timeout) const
{
  //ACE_TRACE ("ACE_FILE_Stream::recv");
  ACE_UNUSED_ARG (timeout);
  return ACE_FILE_IO::recv (buf, n);
}

ACE_INLINE ssize_t
ACE_FILE_Stream::recvv (iovec iov[],
                        int n,
                        const ACE_Time_Value *timeout) const
{
  //ACE_TRACE ("ACE_FILE_Stream::recvv");
  ACE_UNUSED_ARG (timeout);
  return ACE_FILE_IO::recv (iov, n);
}

ACE_INLINE ssize_t
ACE_FILE_Stream::recvv (iovec *io_vec,
                        const ACE_Time_Value *timeout) const
{
  //ACE_TRACE ("ACE_FILE_Stream::recvv");
  ACE_UNUSED_ARG (timeout);
  return const_cast<ACE_FILE_Stream*> (this)->ACE_FILE_IO::recvv (io_vec);
}

//ACE_INLINE ssize_t
//ACE_FILE_Stream::recv (size_t n, ...) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::recv");
//  return ACE_FILE_IO::recv (n);
//}

//ACE_INLINE ssize_t
//ACE_FILE_Stream::recv (void *buf,
//                       size_t n,
//                       ACE_OVERLAPPED *overlapped) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::recv");
//  return ACE_FILE_IO::recv (buf, n, overlapped);
//}

ACE_INLINE ssize_t
ACE_FILE_Stream::send (const void *buf,
                       size_t n,
                       int flags,
                       const ACE_Time_Value *timeout) const
{
  //ACE_TRACE ("ACE_FILE_Stream::send");
  ACE_UNUSED_ARG (flags);
  ACE_UNUSED_ARG (timeout);
  return ACE_FILE_IO::send (buf, n);
}

ACE_INLINE ssize_t
ACE_FILE_Stream::send (const void *buf,
                       size_t n,
                       const ACE_Time_Value *timeout) const
{
  //ACE_TRACE ("ACE_FILE_Stream::send");
  ACE_UNUSED_ARG (timeout);
  return ACE_FILE_IO::send (buf, n);
}

ACE_INLINE ssize_t
ACE_FILE_Stream::sendv (const iovec iov[],
                        int n,
                        const ACE_Time_Value *timeout) const
{
  //ACE_TRACE ("ACE_FILE_Stream::sendv");
  ACE_UNUSED_ARG (timeout);
  return ACE_FILE_IO::sendv (iov, n);
}

//ACE_INLINE ssize_t
//ACE_FILE_Stream::send (size_t n, ...) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::send");
//  return ACE_FILE_IO::send (n);
//}

//ACE_INLINE ssize_t
//ACE_FILE_Stream::send (const void *buf,
//                       size_t n,
//                       ACE_OVERLAPPED *overlapped) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::send");
//  return ACE_FILE_IO::send (buf, n, overlapped);
//}

/////////////////////////////////////////

//ACE_INLINE ssize_t
//ACE_FILE_Stream::recv (char* buf,
//                       ssize_t len,
//                       int flags,
//                       ACE_Time_Value* timeout)
//{
//  //ACE_TRACE ("ACE_FILE_Stream::recv");
//  size_t bytes_transferred = 0;
//  return ACE::recv_n (this->get_handle (),
//                      buf,
//                      len,
//                      flags,
//                      timeout,
//                      &bytes_transferred);
//}
//
//ACE_INLINE ssize_t
//ACE_FILE_Stream::recv_n (void *buf,
//                         size_t len,
//                         int flags,
//                         const ACE_Time_Value *timeout,
//                         size_t *bytes_transferred) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::recv_n");
//  return ACE::recv_n (this->get_handle (),
//                      buf,
//                      len,
//                      flags,
//                      timeout,
//                      bytes_transferred);
//}
//
//ACE_INLINE ssize_t
//ACE_FILE_Stream::recv_n (void *buf,
//                         size_t len,
//                         const ACE_Time_Value *timeout,
//                         size_t *bytes_transferred) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::recv_n");
//  return ACE::recv_n (this->get_handle (),
//                      buf,
//                      len,
//                      timeout,
//                      bytes_transferred);
//}
//
//ACE_INLINE ssize_t
//ACE_FILE_Stream::recvv_n (iovec iov[],
//                          int n,
//                          const ACE_Time_Value *timeout,
//                          size_t *bytes_transferred) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::recvv_n");
//  return ACE::recvv_n (this->get_handle (),
//                       iov,
//                       n,
//                       timeout,
//                       bytes_transferred);
//}
//
//ACE_INLINE ssize_t
//ACE_FILE_Stream::send_n (const void *buf,
//                         size_t len,
//                         int flags,
//                         const ACE_Time_Value *timeout,
//                         size_t *bytes_transferred) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::send_n");
//  return ACE::send_n (this->get_handle (),
//                      buf,
//                      len,
//                      flags,
//                      timeout,
//                      bytes_transferred);
//}
//
//ACE_INLINE ssize_t
//ACE_FILE_Stream::send_n (const void *buf,
//                         size_t len,
//                         const ACE_Time_Value *timeout,
//                         size_t *bytes_transferred) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::send_n");
//  return ACE::send_n (this->get_handle (),
//                      buf,
//                      len,
//                      timeout,
//                      bytes_transferred);
//}
//
//ACE_INLINE ssize_t
//ACE_FILE_Stream::sendv_n (const iovec iov[],
//                          int n,
//                          const ACE_Time_Value *timeout,
//                          size_t *bytes_transferred) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::sendv_n");
//  return ACE::sendv_n (this->get_handle (),
//                       iov,
//                       n,
//                       timeout,
//                       bytes_transferred);
//}
//
//ACE_INLINE ssize_t
//ACE_FILE_Stream::send_n (const ACE_Message_Block *message_block,
//                         const ACE_Time_Value *timeout,
//                         size_t *bytes_transferred) const
//{
//  //ACE_TRACE ("ACE_FILE_Stream::send_n");
//  return ACE::send_n (this->get_handle (),
//                      message_block,
//                      timeout,
//                      bytes_transferred);
//}

//ACE_INLINE ssize_t
//ACE_FILE_Stream::send_urg (const void *ptr,
//                           size_t len,
//                           const ACE_Time_Value *timeout) const
//{
//  ACE_TRACE ("ACE_FILE_Stream::send_urg");
//  return ACE::send (this->get_handle (),
//                    ptr,
//                    len,
//                    MSG_OOB,
//                    timeout);
//}

//ACE_INLINE ssize_t
//ACE_FILE_Stream::recv_urg (void *ptr,
//                           size_t len,
//                           const ACE_Time_Value *timeout) const
//{
//  ACE_TRACE ("ACE_FILE_Stream::recv_urg");
//  return ACE::recv (this->get_handle (),
//                    ptr,
//                    len,
//                    MSG_OOB,
//                    timeout);
//}

ACE_END_VERSIONED_NAMESPACE_DECL
