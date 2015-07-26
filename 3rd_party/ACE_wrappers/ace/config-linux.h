// *NOTE*: uncomment the line corresponding to your platform !
#include "ace/config-linux.h"

//// *NOTE*: needed for ACE_IOStream
//#define ACE_HAS_STANDARD_CPP_LIBRARY 1

// *NOTE*: don't use the regular pipe-based mechanism,
// it has several drawbacks (see relevant documentation)
#define ACE_HAS_REACTOR_NOTIFICATION_QUEUE

// *NOTE*: libpthread now supports thread names
//         (see /usr/include/pthread.h::453: pthread_setname_np())
//#define ACE_HAS_PTHREAD_ATTR_SETNAME

// *NOTE*: set FD_SETSIZE so select-based reactors can dispatch more than the
//         default (1024, see below) handles
//#include <bits/typesizes.h>
#include <sys/types.h>
#undef __FD_SETSIZE
#define __FD_SETSIZE 65536
#include <sys/select.h>
//#include <linux/posix_types.h>

// *NOTE*: ACE_IOStream support requires these definitions
////#undef ACE_LACKS_IOSTREAM_TOTALLY
//#define ACE_USES_OLD_IOSTREAMS
//#undef ACE_LACKS_ACE_IOSTREAM
// *NOTE*: iostream(.h) [from compat-gcc-34-c++ (3.4.6)] has no support for
//         iostream::ipfx/ipsx
#define ACE_LACKS_IOSTREAM_FX

#define ACE_LACKS_LINEBUFFERED_STREAMBUF
#define ACE_LACKS_UNBUFFERED_STREAMBUF
// *NOTE*: recent gcc iostream::operator>> implementations do not support
//         pointer-type arguments any more; this disables the corresponding
//         (macro) code in IOStream.h
// *TODO*: remove this define and add platform/compiler-specific #ifdefs to
//         IOStream.h
#define ACE_LACKS_SIGNED_CHAR
