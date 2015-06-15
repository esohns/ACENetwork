#pragma once

#include "ace/iosfwd.h"

// These typedefs are provided by G++ (on some systems?) without the
// trailing '_'.  Since we can't count on 'em, I've defined them to
// what GNU wants here.
//
typedef ios& (*__manip_) (ios&);
typedef ios_base& (*__bmanip_) (ios_base&);
typedef istream& (*__imanip_) (istream&);
typedef ostream& (*__omanip_) (ostream&);

// Trying to do something like is shown below instead of using the
// __*manip typedefs causes Linux do segfault when "<<endl" is done.
//
//        virtual MT& operator<<(ios& (*func)(ios&))  { (*func)(*this); return *this; }

// This macro defines the get operator for class MT into datatype DT.
// We will use it below to quickly override most (all?)  iostream get
// operators.  Notice how the <ipfx> and <isfx> functions are used.

#define GET_SIG(MT,DT) virtual MT& operator>> (DT v)
#  if (defined (__SUNPRO_CC) && __SUNPRO_CC > 0x510)
#define GET_CODE {                                    \
        if (ipfx (0))                                 \
                {                                     \
                (*((istream*)this)) >> (v);           \
                }                                     \
        isfx ();                                      \
        return *this;                                 \
        }
#  else
#define GET_CODE {                                    \
        if (ipfx (0))                                 \
                {                                     \
                iostream::operator>> (v);             \
                }                                     \
        isfx ();                                      \
        return *this;                                 \
        }
#  endif
#define GET_PROT(MT,DT,CODE)    GET_SIG(MT,DT)  CODE
#define GET_FUNC(MT,DT)         GET_PROT(MT,DT,GET_CODE)

// This macro defines the put operator for class MT into datatype DT.
// We will use it below to quickly override most (all?)  iostream put
// operators.  Notice how the <opfx> and <osfx> functions are used.

#define PUT_SIG(MT,DT) virtual MT& operator<< (DT v)
#  if (defined (__SUNPRO_CC) && __SUNPRO_CC > 0x510)
#define PUT_CODE {                                    \
        if (opfx ())                                  \
                {                                     \
                (*((ostream *) this)) << (v);         \
                }                                     \
        osfx ();                                      \
        return *this;                                 \
        }
#  else
#define PUT_CODE {                                    \
        if (opfx ())                                  \
                {                                     \
                iostream::operator<< (v);             \
                }                                     \
        osfx ();                                      \
        return *this;                                 \
        }
#  endif
#define PUT_PROT(MT,DT,CODE)    PUT_SIG(MT,DT)  CODE
#define PUT_FUNC(MT,DT)         PUT_PROT(MT,DT,PUT_CODE)


// These are necessary in case somebody wants to derive from us and
// override one of these with a custom approach.

#  if defined (ACE_LACKS_CHAR_RIGHT_SHIFTS)
#define GET_FUNC_SET0(MT,CODE,CODE2)                  \
        GET_PROT(MT,short &,CODE)                     \
        GET_PROT(MT,u_short &,CODE)                   \
        GET_PROT(MT,int &,CODE)                       \
        GET_PROT(MT,u_int &,CODE)                     \
        GET_PROT(MT,long &,CODE)                      \
        GET_PROT(MT,u_long &,CODE)                    \
        GET_PROT(MT,float &,CODE)                     \
        GET_PROT(MT,double &,CODE)                    \
        virtual MT& operator>> (__omanip_ func) CODE2 \
        virtual MT& operator>> (__manip_ func)  CODE2
#  else
//#define GET_FUNC_SET0(MT,CODE,CODE2)                  \
//        GET_PROT(MT,short &,CODE)                     \
//        GET_PROT(MT,u_short &,CODE)                   \
//        GET_PROT(MT,int &,CODE)                       \
//        GET_PROT(MT,u_int &,CODE)                     \
//        GET_PROT(MT,long &,CODE)                      \
//        GET_PROT(MT,u_long &,CODE)                    \
//        GET_PROT(MT,float &,CODE)                     \
//        GET_PROT(MT,double &,CODE)                    \
//        GET_PROT(MT,char &,CODE)                      \
//        GET_PROT(MT,u_char &,CODE)                    \
//        GET_PROT(MT,char *,CODE)                      \
//        GET_PROT(MT,u_char *,CODE)                    \
//        virtual MT& operator>> (__omanip_ func) CODE2 \
//        virtual MT& operator>> (__manip_ func)  CODE2
#define GET_FUNC_SET0(MT,CODE,CODE2)                  \
        GET_PROT(MT,bool &,CODE)                      \
        GET_PROT(MT,short &,CODE)                     \
        GET_PROT(MT,u_short &,CODE)                   \
        GET_PROT(MT,int &,CODE)                       \
        GET_PROT(MT,u_int &,CODE)                     \
        GET_PROT(MT,long &,CODE)                      \
        GET_PROT(MT,u_long &,CODE)                    \
        GET_PROT(MT,float &,CODE)                     \
        GET_PROT(MT,double &,CODE)                    \
        GET_PROT(MT,long double &,CODE)               \
        GET_PROT(MT,void* &,CODE)                     \
        GET_PROT(MT,streambuf* ,CODE)                 \
        virtual MT& operator>> (__imanip_ func) CODE2 \
        virtual MT& operator>> (__manip_ func)  CODE2 \
        virtual MT& operator>> (__bmanip_ func) CODE2
#  endif

//#define PUT_FUNC_SET0(MT,CODE,CODE2)                  \
//        PUT_PROT(MT,short,CODE)                       \
//        PUT_PROT(MT,u_short,CODE)                     \
//        PUT_PROT(MT,int,CODE)                         \
//        PUT_PROT(MT,u_int,CODE)                       \
//        PUT_PROT(MT,long,CODE)                        \
//        PUT_PROT(MT,u_long,CODE)                      \
//        PUT_PROT(MT,float,CODE)                       \
//        PUT_PROT(MT,double,CODE)                      \
//        PUT_PROT(MT,char,CODE)                        \
//        PUT_PROT(MT,u_char,CODE)                      \
//        PUT_PROT(MT,const char *,CODE)                \
//        PUT_PROT(MT,u_char *,CODE)                    \
//        PUT_PROT(MT,void *,CODE)                      \
//        virtual MT& operator<< (__omanip_ func) CODE2 \
//        virtual MT& operator<< (__manip_ func)  CODE2
#define PUT_FUNC_SET0(MT,CODE,CODE2)                  \
        PUT_PROT(MT,bool,CODE)                        \
        PUT_PROT(MT,short,CODE)                       \
        PUT_PROT(MT,u_short,CODE)                     \
        PUT_PROT(MT,int,CODE)                         \
        PUT_PROT(MT,u_int,CODE)                       \
        PUT_PROT(MT,long,CODE)                        \
        PUT_PROT(MT,u_long,CODE)                      \
        PUT_PROT(MT,float,CODE)                       \
        PUT_PROT(MT,double,CODE)                      \
        PUT_PROT(MT,long double,CODE)                 \
        PUT_PROT(MT,void*,CODE)                       \
        PUT_PROT(MT,streambuf*,CODE)                  \
        virtual MT& operator<< (__omanip_ func) CODE2 \
        virtual MT& operator<< (__manip_ func)  CODE2 \
        virtual MT& operator<< (__bmanip_ func) CODE2

#  if defined (ACE_LACKS_SIGNED_CHAR)
#define GET_FUNC_SET1(MT,CODE,CODE2) GET_FUNC_SET0(MT,CODE,CODE2)
#define PUT_FUNC_SET1(MT,CODE,CODE2) PUT_FUNC_SET0(MT,CODE,CODE2)
#  else
#define GET_FUNC_SET1(MT,CODE,CODE2) \
          GET_PROT(MT,signed char &,CODE) \
          GET_PROT(MT,signed char *,CODE) \
          GET_FUNC_SET0(MT,CODE,CODE2)

#define PUT_FUNC_SET1(MT,CODE,CODE2) \
          PUT_FUNC(MT,signed char) \
          PUT_FUNC(MT,const signed char *) \
          PUT_FUNC_SET0(MT,CODE,CODE2)
#  endif /* ACE_LACKS_SIGNED_CHAR */

#define GET_MANIP_CODE  { if (ipfx ()) { (*func) (*this); } isfx (); return *this; }
#define PUT_MANIP_CODE  { if (opfx ()) { (*func) (*this); } osfx (); return *this; }

#define GET_FUNC_SET(MT)        GET_FUNC_SET1(MT,GET_CODE,GET_MANIP_CODE)
#define PUT_FUNC_SET(MT)        PUT_FUNC_SET1(MT,PUT_CODE,PUT_MANIP_CODE)
#define GETPUT_FUNC_SET(MT)     GET_FUNC_SET(MT) PUT_FUNC_SET(MT)

#define GET_SIG_SET(MT)         GET_FUNC_SET1(MT,= 0;,= 0;)
#define PUT_SIG_SET(MT)         PUT_FUNC_SET1(MT,= 0;,= 0;)
#define GETPUT_SIG_SET(MT)      GET_SIG_SET(MT) PUT_SIG_SET(MT)
