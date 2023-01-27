/* A Bison parser, made by GNU Bison 3.7.4.  */

/* Skeleton implementation for Bison GLR parsers in C

   Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C GLR parser skeleton written by Paul Hilfinger.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30704

/* Bison version string.  */
#define YYBISON_VERSION "3.7.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "glr.c"

/* Pure parsers.  */
#define YYPURE 1


/* "%code top" blocks.  */

#include "stdafx.h"
#undef YYTOKENTYPE





# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "pcp_parser.h"

/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_VERSION = 3,                    /* "version"  */
  YYSYMBOL_OPCODE = 4,                     /* "opcode"  */
  YYSYMBOL_RESERVED = 5,                   /* "reserved"  */
  YYSYMBOL_RESULT_CODE = 6,                /* "result_code"  */
  YYSYMBOL_LIFETIME = 7,                   /* "lifetime"  */
  YYSYMBOL_EPOCH_TIME = 8,                 /* "epoch_time"  */
  YYSYMBOL_RESERVED_2 = 9,                 /* "reserved_2"  */
  YYSYMBOL_OPTION_MAP_NONCE = 10,          /* "option_map_nonce"  */
  YYSYMBOL_OPTION_MAP_PROTOCOL = 11,       /* "option_map_protocol"  */
  YYSYMBOL_OPTION_MAP_RESERVED = 12,       /* "option_map_reserved"  */
  YYSYMBOL_OPTION_MAP_INTERNAL_PORT = 13,  /* "option_map_internal_port"  */
  YYSYMBOL_OPTION_MAP_ASSIGNED_EXTERNAL_PORT = 14, /* "option_map_assigned_external_port"  */
  YYSYMBOL_OPTION_MAP_ASSIGNED_EXTERNAL_IP_ADDRESS = 15, /* "option_map_assigned_external_ip_address"  */
  YYSYMBOL_OPTION_PEER_NONCE = 16,         /* "option_peer_nonce"  */
  YYSYMBOL_OPTION_PEER_PROTOCOL = 17,      /* "option_peer_protocol"  */
  YYSYMBOL_OPTION_PEER_RESERVED = 18,      /* "option_peer_reserved"  */
  YYSYMBOL_OPTION_PEER_INTERNAL_PORT = 19, /* "option_peer_internal_port"  */
  YYSYMBOL_OPTION_PEER_ASSIGNED_EXTERNAL_PORT = 20, /* "option_peer_assigned_external_port"  */
  YYSYMBOL_OPTION_PEER_ASSIGNED_EXTERNAL_IP_ADDRESS = 21, /* "option_peer_assigned_external_ip_address"  */
  YYSYMBOL_OPTION_PEER_REMOTE_PEER_PORT = 22, /* "option_peer_remote_port"  */
  YYSYMBOL_OPTION_PEER_RESERVED_2 = 23,    /* "option_peer_reserved_2"  */
  YYSYMBOL_OPTION_PEER_REMOTE_PEER_IP_ADDRESS = 24, /* "option_peer_remote_peer_address"  */
  YYSYMBOL_OPTION_AUTHENTICATION_SESSION_ID = 25, /* "option_authentication_session_id"  */
  YYSYMBOL_OPTION_AUTHENTICATION_SEQUENCE_NUMBER = 26, /* "option_authentication_sequence_number"  */
  YYSYMBOL_OPTION_CODE = 27,               /* "option_code"  */
  YYSYMBOL_OPTION_RESERVED = 28,           /* "option_reserved"  */
  YYSYMBOL_OPTION_LENGTH = 29,             /* "option_length"  */
  YYSYMBOL_OPTION_THIRD_PARTY_ADDRESS = 30, /* "option_third_party_address"  */
  YYSYMBOL_OPTION_FILTER_RESERVED = 31,    /* "option_filter_reserved"  */
  YYSYMBOL_OPTION_FILTER_PREFIX_LENGTH = 32, /* "option_filter_prefix_length"  */
  YYSYMBOL_OPTION_FILTER_REMOTE_PEER_PORT = 33, /* "option_filter_remote_peer_port"  */
  YYSYMBOL_OPTION_FILTER_REMOTE_PEER_IP_ADDRESS = 34, /* "option_filter_remote_peer_address"  */
  YYSYMBOL_OPTION_NONCE_NONCE = 35,        /* "option_nonce_nonce"  */
  YYSYMBOL_OPTION_AUTHENTICATION_TAG_SESSION_ID = 36, /* "option_authentication_tag_session_id"  */
  YYSYMBOL_OPTION_AUTHENTICATION_TAG_SEQUENCE_NUMBER = 37, /* "option_authentication_tag_sequence_number"  */
  YYSYMBOL_OPTION_AUTHENTICATION_TAG_KEY_ID = 38, /* "option_authentication_tag_key_id"  */
  YYSYMBOL_OPTION_AUTHENTICATION_TAG_DATA = 39, /* "option_authentication_tag_data"  */
  YYSYMBOL_OPTION_PA_AUTHENTICATION_TAG_KEY_ID = 40, /* "option_pa_authentication_tag_key_id"  */
  YYSYMBOL_OPTION_PA_AUTHENTICATION_TAG_DATA = 41, /* "option_pa_authentication_tag_data"  */
  YYSYMBOL_OPTION_EAP_PAYLOAD_DATA = 42,   /* "option_eap_payload_data"  */
  YYSYMBOL_OPTION_PSEUDO_RANDOM_FUNCTION_ID = 43, /* "option_pseudo_random_function_id"  */
  YYSYMBOL_OPTION_MAC_ALGORITHM_ID = 44,   /* "option_mac_algorithm_id"  */
  YYSYMBOL_OPTION_SESSION_LIFETIME_LIFETIME = 45, /* "option_session_lifetime_lifetime"  */
  YYSYMBOL_OPTION_RECEIVED_PAK_SEQUENCE_NUMBER = 46, /* "option_received_pak_sequence_number"  */
  YYSYMBOL_OPTION_ID_INDICATOR_DATA = 47,  /* "option_id_indicator_data"  */
  YYSYMBOL_YYACCEPT = 48,                  /* $accept  */
  YYSYMBOL_message = 49,                   /* message  */
  YYSYMBOL_header = 50,                    /* header  */
  YYSYMBOL_51_1 = 51,                      /* $@1  */
  YYSYMBOL_52_2 = 52,                      /* $@2  */
  YYSYMBOL_53_3 = 53,                      /* $@3  */
  YYSYMBOL_54_4 = 54,                      /* $@4  */
  YYSYMBOL_55_5 = 55,                      /* $@5  */
  YYSYMBOL_56_6 = 56,                      /* $@6  */
  YYSYMBOL_opcode_specific = 57,           /* opcode_specific  */
  YYSYMBOL_opcode_specific_map = 58,       /* opcode_specific_map  */
  YYSYMBOL_59_7 = 59,                      /* $@7  */
  YYSYMBOL_60_8 = 60,                      /* $@8  */
  YYSYMBOL_61_9 = 61,                      /* $@9  */
  YYSYMBOL_62_10 = 62,                     /* $@10  */
  YYSYMBOL_63_11 = 63,                     /* $@11  */
  YYSYMBOL_opcode_specific_peer = 64,      /* opcode_specific_peer  */
  YYSYMBOL_65_12 = 65,                     /* $@12  */
  YYSYMBOL_66_13 = 66,                     /* $@13  */
  YYSYMBOL_67_14 = 67,                     /* $@14  */
  YYSYMBOL_68_15 = 68,                     /* $@15  */
  YYSYMBOL_69_16 = 69,                     /* $@16  */
  YYSYMBOL_70_17 = 70,                     /* $@17  */
  YYSYMBOL_71_18 = 71,                     /* $@18  */
  YYSYMBOL_72_19 = 72,                     /* $@19  */
  YYSYMBOL_opcode_specific_authentication = 73, /* opcode_specific_authentication  */
  YYSYMBOL_74_20 = 74,                     /* $@20  */
  YYSYMBOL_options = 75,                   /* options  */
  YYSYMBOL_option = 76,                    /* option  */
  YYSYMBOL_77_21 = 77,                     /* $@21  */
  YYSYMBOL_78_22 = 78,                     /* $@22  */
  YYSYMBOL_79_23 = 79,                     /* $@23  */
  YYSYMBOL_option_data = 80,               /* option_data  */
  YYSYMBOL_option_data_third_party = 81,   /* option_data_third_party  */
  YYSYMBOL_option_data_filter = 82,        /* option_data_filter  */
  YYSYMBOL_83_24 = 83,                     /* $@24  */
  YYSYMBOL_84_25 = 84,                     /* $@25  */
  YYSYMBOL_85_26 = 85,                     /* $@26  */
  YYSYMBOL_option_data_nonce = 86,         /* option_data_nonce  */
  YYSYMBOL_option_data_authentication_tag = 87, /* option_data_authentication_tag  */
  YYSYMBOL_88_27 = 88,                     /* $@27  */
  YYSYMBOL_89_28 = 89,                     /* $@28  */
  YYSYMBOL_90_29 = 90,                     /* $@29  */
  YYSYMBOL_option_data_pa_authentication_tag = 91, /* option_data_pa_authentication_tag  */
  YYSYMBOL_92_30 = 92,                     /* $@30  */
  YYSYMBOL_option_data_eap_payload = 93,   /* option_data_eap_payload  */
  YYSYMBOL_option_data_pseudo_random_function = 94, /* option_data_pseudo_random_function  */
  YYSYMBOL_option_data_mac_algorithm = 95, /* option_data_mac_algorithm  */
  YYSYMBOL_option_data_session_lifetime = 96, /* option_data_session_lifetime  */
  YYSYMBOL_option_data_received_pak = 97,  /* option_data_received_pak  */
  YYSYMBOL_option_data_id_indicator = 98   /* option_data_id_indicator  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;


/* Default (constant) value used for initialization for null
   right-hand sides.  Unlike the standard yacc.c template, here we set
   the default value of $$ to a zeroed-out value.  Since the default
   value is undefined, this behavior is technically correct.  */
static YYSTYPE yyval_default;
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;


/* Unqualified %code blocks.  */

// *NOTE*: necessary only if %debug is set in the definition file (see: parser.y)
#if defined (YYDEBUG)
#include <iostream>
#endif // YYDEBUG

// *WORKAROUND*
using namespace std;
// *IMPORTANT NOTE*: several ACE headers inclue ace/iosfwd.h, which introduces
//                   a problem in conjunction with the standard include headers
//                   when ACE_USES_OLD_IOSTREAMS is defined
//                   --> include the necessary headers manually (see above), and
//                       prevent ace/iosfwd.h from causing any harm
#define ACE_IOSFWD_H

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "net_common_tools.h"
#include "net_macros.h"

#include "pcp_common.h"
#include "pcp_defines.h"
#include "pcp_parser_driver.h"
#include "pcp_scanner.h"
#include "pcp_tools.h"

// *TODO*: this shouldn't be necessary
#define yylex PCP_Scanner_lex

//#define YYPRINT(file, type, value) yyprint (file, type, value)


#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif
#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YYFREE
# define YYFREE free
#endif
#ifndef YYMALLOC
# define YYMALLOC malloc
#endif
#ifndef YYREALLOC
# define YYREALLOC realloc
#endif

#ifdef __cplusplus
  typedef bool yybool;
# define yytrue true
# define yyfalse false
#else
  /* When we move to stdbool, get rid of the various casts to yybool.  */
  typedef signed char yybool;
# define yytrue 1
# define yyfalse 0
#endif

#ifndef YYSETJMP
# include <setjmp.h>
# define YYJMP_BUF jmp_buf
# define YYSETJMP(Env) setjmp (Env)
/* Pacify Clang and ICC.  */
# define YYLONGJMP(Env, Val)                    \
 do {                                           \
   longjmp (Env, Val);                          \
   YY_ASSERT (0);                               \
 } while (yyfalse)
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* The _Noreturn keyword of C11.  */
#ifndef _Noreturn
# if (defined __cplusplus \
      && ((201103 <= __cplusplus && !(__GNUC__ == 4 && __GNUC_MINOR__ == 7)) \
          || (defined _MSC_VER && 1900 <= _MSC_VER)))
#  define _Noreturn [[noreturn]]
# elif (!defined __cplusplus                     \
        && (201112 <= (defined __STDC_VERSION__ ? __STDC_VERSION__ : 0)  \
            || 4 < __GNUC__ + (7 <= __GNUC_MINOR__) \
            || (defined __apple_build_version__ \
                ? 6000000 <= __apple_build_version__ \
                : 3 < __clang_major__ + (5 <= __clang_minor__))))
   /* _Noreturn works as-is.  */
# elif 2 < __GNUC__ + (8 <= __GNUC_MINOR__) || 0x5110 <= __SUNPRO_C
#  define _Noreturn __attribute__ ((__noreturn__))
# elif 1200 <= (defined _MSC_VER ? _MSC_VER : 0)
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   50

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  48
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  51
/* YYNRULES -- Number of rules.  */
#define YYNRULES  66
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  98
/* YYMAXRHS -- Maximum number of symbols on right-hand side of rule.  */
#define YYMAXRHS 17
/* YYMAXLEFT -- Maximum number of symbols to the left of a handle
   accessed by $0, $-1, etc., in any rule.  */
#define YYMAXLEFT 0

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   302

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47
};

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   203,   203,   204,   206,   207,   208,   210,   211,   204,
     214,   215,   216,   217,   218,   219,   220,   221,   222,   218,
     234,   235,   236,   237,   238,   239,   249,   250,   234,   262,
     262,   265,   266,   267,   270,   272,   267,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     299,   301,   303,   299,   317,   320,   322,   324,   320,   329,
     329,   334,   337,   340,   343,   346,   349
};
#endif

#define YYPACT_NINF (-31)
#define YYTABLE_NINF (-1)

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -1,   -31,     4,    -7,     3,   -31,   -31,   -31,   -31,   -19,
     -31,   -31,   -31,   -31,     0,     2,    -6,   -31,   -31,   -19,
      16,   -31,   -31,   -31,    -5,   -31,   -31,    10,     6,   -31,
      19,   -31,   -31,    -3,   -31,    14,     9,   -31,    22,   -31,
     -31,   -30,   -31,    17,    12,   -31,   -31,   -31,   -31,   -31,
     -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,
     -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,    25,   -31,
     -31,    -2,     1,    -4,   -31,    20,    13,   -31,   -31,   -31,
      27,   -31,   -31,     7,     5,   -31,    23,   -31,   -31,   -31,
       8,    11,    18,   -31,   -31,   -31,    15,   -31
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     3,     0,    13,     0,     1,    14,    20,    29,    32,
      10,    11,    12,     4,     0,     0,     0,    33,     2,    32,
       0,    15,    21,    30,     0,    31,     5,     0,     0,    34,
       0,    16,    22,     0,     6,     0,     0,    35,     0,    17,
      23,    48,     7,     0,     0,    49,    50,    54,    55,    59,
      61,    62,    63,    64,    65,    66,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,     0,    18,
      24,     0,     0,     0,     8,     0,     0,    51,    56,    60,
       0,    19,    25,     0,     0,     9,     0,    52,    57,    26,
       0,     0,     0,    53,    58,    27,     0,    28
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,
     -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,
     -31,   -31,   -31,   -31,   -31,   -31,   -31,    28,   -31,   -31,
     -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,
     -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,   -31,
     -31
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     2,     3,     4,    20,    30,    38,    68,    80,     9,
      10,    14,    27,    35,    43,    75,    11,    15,    28,    36,
      44,    76,    86,    92,    96,    12,    16,    18,    19,    24,
      33,    41,    56,    57,    58,    71,    83,    90,    59,    60,
      72,    84,    91,    61,    73,    62,    63,    64,    65,    66,
      67
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      45,    46,     1,     6,     5,    47,    48,    13,    17,     7,
      49,    21,    50,    51,    52,    53,    54,    55,     8,    22,
      23,    26,    31,    29,    32,    34,    37,    39,    40,    42,
      77,    69,    70,    74,    82,    81,    85,    79,    78,    97,
      87,    95,    93,    88,     0,    89,     0,    25,     0,     0,
      94
};

static const yytype_int8 yycheck[] =
{
      30,    31,     3,    10,     0,    35,    36,     4,    27,    16,
      40,    11,    42,    43,    44,    45,    46,    47,    25,    17,
      26,     5,    12,    28,    18,     6,    29,    13,    19,     7,
      32,    14,    20,     8,    21,    15,     9,    41,    37,    24,
      33,    23,    34,    38,    -1,    22,    -1,    19,    -1,    -1,
      39
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,    49,    50,    51,     0,    10,    16,    25,    57,
      58,    64,    73,     4,    59,    65,    74,    27,    75,    76,
      52,    11,    17,    26,    77,    75,     5,    60,    66,    28,
      53,    12,    18,    78,     6,    61,    67,    29,    54,    13,
      19,    79,     7,    62,    68,    30,    31,    35,    36,    40,
      42,    43,    44,    45,    46,    47,    80,    81,    82,    86,
      87,    91,    93,    94,    95,    96,    97,    98,    55,    14,
      20,    83,    88,    92,     8,    63,    69,    32,    37,    41,
      56,    15,    21,    84,    89,     9,    70,    33,    38,    22,
      85,    90,    71,    34,    39,    23,    72,    24
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    48,    49,    51,    52,    53,    54,    55,    56,    50,
      57,    57,    57,    57,    59,    60,    61,    62,    63,    58,
      65,    66,    67,    68,    69,    70,    71,    72,    64,    74,
      73,    75,    75,    77,    78,    79,    76,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    81,
      83,    84,    85,    82,    86,    88,    89,    90,    87,    92,
      91,    93,    94,    95,    96,    97,    98
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     0,     0,     0,     0,     0,     0,    13,
       1,     1,     1,     0,     0,     0,     0,     0,     0,    11,
       0,     0,     0,     0,     0,     0,     0,     0,    17,     0,
       3,     2,     0,     0,     0,     0,     7,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       0,     0,     0,     7,     1,     0,     0,     0,     7,     0,
       3,     1,     1,     1,     1,     1,     1
};


/* YYDPREC[RULE-NUM] -- Dynamic precedence of rule #RULE-NUM (0 if none).  */
static const yytype_int8 yydprec[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0
};

/* YYMERGER[RULE-NUM] -- Index of merging function for rule #RULE-NUM.  */
static const yytype_int8 yymerger[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0
};

/* YYIMMEDIATE[RULE-NUM] -- True iff rule #RULE-NUM is not to be deferred, as
   in the case of predicates.  */
static const yybool yyimmediate[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0
};

/* YYCONFLP[YYPACT[STATE-NUM]] -- Pointer into YYCONFL of start of
   list of conflicting reductions corresponding to action entry for
   state STATE-NUM in yytable.  0 means no conflicts.  The list in
   yyconfl is terminated by a rule number of 0.  */
static const yytype_int8 yyconflp[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0
};

/* YYCONFL[I] -- lists of conflicting rule numbers, each terminated by
   0, pointed into by YYCONFLP.  */
static const short yyconfl[] =
{
       0
};


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

# define YYRHSLOC(Rhs, K) ((Rhs)[K].yystate.yyloc)



#undef yynerrs
#define yynerrs (yystackp->yyerrcnt)
#undef yychar
#define yychar (yystackp->yyrawchar)
#undef yylval
#define yylval (yystackp->yyval)
#undef yylloc
#define yylloc (yystackp->yyloc)


enum { YYENOMEM = -2 };

typedef enum { yyok, yyaccept, yyabort, yyerr } YYRESULTTAG;

#define YYCHK(YYE)                              \
  do {                                          \
    YYRESULTTAG yychk_flag = YYE;               \
    if (yychk_flag != yyok)                     \
      return yychk_flag;                        \
  } while (0)

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYMAXDEPTH * sizeof (GLRStackItem)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

/* Minimum number of free items on the stack allowed after an
   allocation.  This is to allow allocation and initialization
   to be completed by functions that call yyexpandGLRStack before the
   stack is expanded, thus insuring that all necessary pointers get
   properly redirected to new data.  */
#define YYHEADROOM 2

#ifndef YYSTACKEXPANDABLE
#  define YYSTACKEXPANDABLE 1
#endif

#if YYSTACKEXPANDABLE
# define YY_RESERVE_GLRSTACK(Yystack)                   \
  do {                                                  \
    if (Yystack->yyspaceLeft < YYHEADROOM)              \
      yyexpandGLRStack (Yystack);                       \
  } while (0)
#else
# define YY_RESERVE_GLRSTACK(Yystack)                   \
  do {                                                  \
    if (Yystack->yyspaceLeft < YYHEADROOM)              \
      yyMemoryExhausted (Yystack);                      \
  } while (0)
#endif

/** State numbers. */
typedef int yy_state_t;

/** Rule numbers. */
typedef int yyRuleNum;

/** Item references. */
typedef short yyItemNum;

typedef struct yyGLRState yyGLRState;
typedef struct yyGLRStateSet yyGLRStateSet;
typedef struct yySemanticOption yySemanticOption;
typedef union yyGLRStackItem yyGLRStackItem;
typedef struct yyGLRStack yyGLRStack;

struct yyGLRState {
  /** Type tag: always true.  */
  yybool yyisState;
  /** Type tag for yysemantics.  If true, yysval applies, otherwise
   *  yyfirstVal applies.  */
  yybool yyresolved;
  /** Number of corresponding LALR(1) machine state.  */
  yy_state_t yylrState;
  /** Preceding state in this stack */
  yyGLRState* yypred;
  /** Source position of the last token produced by my symbol */
  YYPTRDIFF_T yyposn;
  union {
    /** First in a chain of alternative reductions producing the
     *  nonterminal corresponding to this state, threaded through
     *  yynext.  */
    yySemanticOption* yyfirstVal;
    /** Semantic value for this state.  */
    YYSTYPE yysval;
  } yysemantics;
  /** Source location for this state.  */
  YYLTYPE yyloc;
};

struct yyGLRStateSet {
  yyGLRState** yystates;
  /** During nondeterministic operation, yylookaheadNeeds tracks which
   *  stacks have actually needed the current lookahead.  During deterministic
   *  operation, yylookaheadNeeds[0] is not maintained since it would merely
   *  duplicate yychar != YYEMPTY.  */
  yybool* yylookaheadNeeds;
  YYPTRDIFF_T yysize;
  YYPTRDIFF_T yycapacity;
};

struct yySemanticOption {
  /** Type tag: always false.  */
  yybool yyisState;
  /** Rule number for this reduction */
  yyRuleNum yyrule;
  /** The last RHS state in the list of states to be reduced.  */
  yyGLRState* yystate;
  /** The lookahead for this reduction.  */
  int yyrawchar;
  YYSTYPE yyval;
  YYLTYPE yyloc;
  /** Next sibling in chain of options.  To facilitate merging,
   *  options are chained in decreasing order by address.  */
  yySemanticOption* yynext;
};

/** Type of the items in the GLR stack.  The yyisState field
 *  indicates which item of the union is valid.  */
union yyGLRStackItem {
  yyGLRState yystate;
  yySemanticOption yyoption;
};

struct yyGLRStack {
  int yyerrState;
  /* To compute the location of the error token.  */
  yyGLRStackItem yyerror_range[3];

  int yyerrcnt;
  int yyrawchar;
  YYSTYPE yyval;
  YYLTYPE yyloc;

  YYJMP_BUF yyexception_buffer;
  yyGLRStackItem* yyitems;
  yyGLRStackItem* yynextFree;
  YYPTRDIFF_T yyspaceLeft;
  yyGLRState* yysplitPoint;
  yyGLRState* yylastDeleted;
  yyGLRStateSet yytops;
};

#if YYSTACKEXPANDABLE
static void yyexpandGLRStack (yyGLRStack* yystackp);
#endif

_Noreturn static void
yyFail (yyGLRStack* yystackp, YYLTYPE *yylocp, PCP_ParserDriver* driver, yyscan_t yyscanner, const char* yymsg)
{
  if (yymsg != YY_NULLPTR)
    yyerror (yylocp, driver, yyscanner, yymsg);
  YYLONGJMP (yystackp->yyexception_buffer, 1);
}

_Noreturn static void
yyMemoryExhausted (yyGLRStack* yystackp)
{
  YYLONGJMP (yystackp->yyexception_buffer, 2);
}

/** Accessing symbol of state YYSTATE.  */
static inline yysymbol_kind_t
yy_accessing_symbol (yy_state_t yystate)
{
  return YY_CAST (yysymbol_kind_t, yystos[yystate]);
}

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end\"", "error", "\"invalid token\"", "\"version\"", "\"opcode\"",
  "\"reserved\"", "\"result_code\"", "\"lifetime\"", "\"epoch_time\"",
  "\"reserved_2\"", "\"option_map_nonce\"", "\"option_map_protocol\"",
  "\"option_map_reserved\"", "\"option_map_internal_port\"",
  "\"option_map_assigned_external_port\"",
  "\"option_map_assigned_external_ip_address\"", "\"option_peer_nonce\"",
  "\"option_peer_protocol\"", "\"option_peer_reserved\"",
  "\"option_peer_internal_port\"",
  "\"option_peer_assigned_external_port\"",
  "\"option_peer_assigned_external_ip_address\"",
  "\"option_peer_remote_port\"", "\"option_peer_reserved_2\"",
  "\"option_peer_remote_peer_address\"",
  "\"option_authentication_session_id\"",
  "\"option_authentication_sequence_number\"", "\"option_code\"",
  "\"option_reserved\"", "\"option_length\"",
  "\"option_third_party_address\"", "\"option_filter_reserved\"",
  "\"option_filter_prefix_length\"", "\"option_filter_remote_peer_port\"",
  "\"option_filter_remote_peer_address\"", "\"option_nonce_nonce\"",
  "\"option_authentication_tag_session_id\"",
  "\"option_authentication_tag_sequence_number\"",
  "\"option_authentication_tag_key_id\"",
  "\"option_authentication_tag_data\"",
  "\"option_pa_authentication_tag_key_id\"",
  "\"option_pa_authentication_tag_data\"", "\"option_eap_payload_data\"",
  "\"option_pseudo_random_function_id\"", "\"option_mac_algorithm_id\"",
  "\"option_session_lifetime_lifetime\"",
  "\"option_received_pak_sequence_number\"",
  "\"option_id_indicator_data\"", "$accept", "message", "header", "$@1",
  "$@2", "$@3", "$@4", "$@5", "$@6", "opcode_specific",
  "opcode_specific_map", "$@7", "$@8", "$@9", "$@10", "$@11",
  "opcode_specific_peer", "$@12", "$@13", "$@14", "$@15", "$@16", "$@17",
  "$@18", "$@19", "opcode_specific_authentication", "$@20", "options",
  "option", "$@21", "$@22", "$@23", "option_data",
  "option_data_third_party", "option_data_filter", "$@24", "$@25", "$@26",
  "option_data_nonce", "option_data_authentication_tag", "$@27", "$@28",
  "$@29", "option_data_pa_authentication_tag", "$@30",
  "option_data_eap_payload", "option_data_pseudo_random_function",
  "option_data_mac_algorithm", "option_data_session_lifetime",
  "option_data_received_pak", "option_data_id_indicator", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#if YYDEBUG

# ifndef YYFPRINTF
#  define YYFPRINTF fprintf
# endif

# define YY_FPRINTF                             \
  YY_IGNORE_USELESS_CAST_BEGIN YY_FPRINTF_

# define YY_FPRINTF_(Args)                      \
  do {                                          \
    YYFPRINTF Args;                             \
    YY_IGNORE_USELESS_CAST_END                  \
  } while (0)

# define YY_DPRINTF                             \
  YY_IGNORE_USELESS_CAST_BEGIN YY_DPRINTF_

# define YY_DPRINTF_(Args)                      \
  do {                                          \
    if (yydebug)                                \
      YYFPRINTF Args;                           \
    YY_IGNORE_USELESS_CAST_END                  \
  } while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YY_LOCATION_PRINT
#  if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#   define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

#  else
#   define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#  endif
# endif /* !defined YY_LOCATION_PRINT */



/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (yylocationp);
  YYUSE (driver);
  YYUSE (yyscanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_YYEOF: /* "end"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_VERSION: /* "version"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPCODE: /* "opcode"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_RESERVED: /* "reserved"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_RESULT_CODE: /* "result_code"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_LIFETIME: /* "lifetime"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_EPOCH_TIME: /* "epoch_time"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_RESERVED_2: /* "reserved_2"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_MAP_NONCE: /* "option_map_nonce"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_MAP_PROTOCOL: /* "option_map_protocol"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_MAP_RESERVED: /* "option_map_reserved"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_MAP_INTERNAL_PORT: /* "option_map_internal_port"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_MAP_ASSIGNED_EXTERNAL_PORT: /* "option_map_assigned_external_port"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_MAP_ASSIGNED_EXTERNAL_IP_ADDRESS: /* "option_map_assigned_external_ip_address"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PEER_NONCE: /* "option_peer_nonce"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PEER_PROTOCOL: /* "option_peer_protocol"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PEER_RESERVED: /* "option_peer_reserved"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PEER_INTERNAL_PORT: /* "option_peer_internal_port"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PEER_ASSIGNED_EXTERNAL_PORT: /* "option_peer_assigned_external_port"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PEER_ASSIGNED_EXTERNAL_IP_ADDRESS: /* "option_peer_assigned_external_ip_address"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PEER_REMOTE_PEER_PORT: /* "option_peer_remote_port"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PEER_RESERVED_2: /* "option_peer_reserved_2"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PEER_REMOTE_PEER_IP_ADDRESS: /* "option_peer_remote_peer_address"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_SESSION_ID: /* "option_authentication_session_id"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_SEQUENCE_NUMBER: /* "option_authentication_sequence_number"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_CODE: /* "option_code"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_RESERVED: /* "option_reserved"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_LENGTH: /* "option_length"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_THIRD_PARTY_ADDRESS: /* "option_third_party_address"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_FILTER_RESERVED: /* "option_filter_reserved"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_FILTER_PREFIX_LENGTH: /* "option_filter_prefix_length"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_FILTER_REMOTE_PEER_PORT: /* "option_filter_remote_peer_port"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_FILTER_REMOTE_PEER_IP_ADDRESS: /* "option_filter_remote_peer_address"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_NONCE_NONCE: /* "option_nonce_nonce"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_TAG_SESSION_ID: /* "option_authentication_tag_session_id"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_TAG_SEQUENCE_NUMBER: /* "option_authentication_tag_sequence_number"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_TAG_KEY_ID: /* "option_authentication_tag_key_id"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_TAG_DATA: /* "option_authentication_tag_data"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), (((*yyvaluep).aval))); }
        break;

    case YYSYMBOL_OPTION_PA_AUTHENTICATION_TAG_KEY_ID: /* "option_pa_authentication_tag_key_id"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_PA_AUTHENTICATION_TAG_DATA: /* "option_pa_authentication_tag_data"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), (((*yyvaluep).aval))); }
        break;

    case YYSYMBOL_OPTION_EAP_PAYLOAD_DATA: /* "option_eap_payload_data"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), (((*yyvaluep).aval))); }
        break;

    case YYSYMBOL_OPTION_PSEUDO_RANDOM_FUNCTION_ID: /* "option_pseudo_random_function_id"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_MAC_ALGORITHM_ID: /* "option_mac_algorithm_id"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_SESSION_LIFETIME_LIFETIME: /* "option_session_lifetime_lifetime"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_RECEIVED_PAK_SEQUENCE_NUMBER: /* "option_received_pak_sequence_number"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_OPTION_ID_INDICATOR_DATA: /* "option_id_indicator_data"  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %s"), (((*yyvaluep).aval))); }
        break;

    case YYSYMBOL_message: /* message  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_header: /* header  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_opcode_specific: /* opcode_specific  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_opcode_specific_map: /* opcode_specific_map  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_opcode_specific_peer: /* opcode_specific_peer  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_opcode_specific_authentication: /* opcode_specific_authentication  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_options: /* options  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option: /* option  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data: /* option_data  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_third_party: /* option_data_third_party  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_filter: /* option_data_filter  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_nonce: /* option_data_nonce  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_authentication_tag: /* option_data_authentication_tag  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_pa_authentication_tag: /* option_data_pa_authentication_tag  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_eap_payload: /* option_data_eap_payload  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_pseudo_random_function: /* option_data_pseudo_random_function  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_mac_algorithm: /* option_data_mac_algorithm  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_session_lifetime: /* option_data_session_lifetime  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_received_pak: /* option_data_received_pak  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

    case YYSYMBOL_option_data_id_indicator: /* option_data_id_indicator  */
                          { ACE_OS::fprintf (yyoutput, ACE_TEXT (" %u"), ((*yyvaluep).ival)); }
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YY_LOCATION_PRINT (yyo, *yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, driver, yyscanner);
  YYFPRINTF (yyo, ")");
}

# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                  \
  do {                                                                  \
    if (yydebug)                                                        \
      {                                                                 \
        YY_FPRINTF ((stderr, "%s ", Title));                            \
        yy_symbol_print (stderr, Kind, Value, Location, driver, yyscanner);        \
        YY_FPRINTF ((stderr, "\n"));                                    \
      }                                                                 \
  } while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;

static void yypstack (yyGLRStack* yystackp, YYPTRDIFF_T yyk)
  YY_ATTRIBUTE_UNUSED;
static void yypdumpstack (yyGLRStack* yystackp)
  YY_ATTRIBUTE_UNUSED;

#else /* !YYDEBUG */

# define YY_DPRINTF(Args) do {} while (yyfalse)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)

#endif /* !YYDEBUG */

#ifndef yystrlen
# define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


/** Fill in YYVSP[YYLOW1 .. YYLOW0-1] from the chain of states starting
 *  at YYVSP[YYLOW0].yystate.yypred.  Leaves YYVSP[YYLOW1].yystate.yypred
 *  containing the pointer to the next state in the chain.  */
static void yyfillin (yyGLRStackItem *, int, int) YY_ATTRIBUTE_UNUSED;
static void
yyfillin (yyGLRStackItem *yyvsp, int yylow0, int yylow1)
{
  int i;
  yyGLRState *s = yyvsp[yylow0].yystate.yypred;
  for (i = yylow0-1; i >= yylow1; i -= 1)
    {
#if YYDEBUG
      yyvsp[i].yystate.yylrState = s->yylrState;
#endif
      yyvsp[i].yystate.yyresolved = s->yyresolved;
      if (s->yyresolved)
        yyvsp[i].yystate.yysemantics.yysval = s->yysemantics.yysval;
      else
        /* The effect of using yysval or yyloc (in an immediate rule) is
         * undefined.  */
        yyvsp[i].yystate.yysemantics.yyfirstVal = YY_NULLPTR;
      yyvsp[i].yystate.yyloc = s->yyloc;
      s = yyvsp[i].yystate.yypred = s->yypred;
    }
}


/** If yychar is empty, fetch the next token.  */
static inline yysymbol_kind_t
yygetToken (int *yycharp, yyGLRStack* yystackp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  yysymbol_kind_t yytoken;
  YYUSE (driver);
  YYUSE (yyscanner);
  if (*yycharp == YYEMPTY)
    {
      YY_DPRINTF ((stderr, "Reading a token\n"));
      *yycharp = yylex (&yylval, &yylloc, driver, yyscanner);
    }
  if (*yycharp <= END)
    {
      *yycharp = END;
      yytoken = YYSYMBOL_YYEOF;
      YY_DPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (*yycharp);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }
  return yytoken;
}

/* Do nothing if YYNORMAL or if *YYLOW <= YYLOW1.  Otherwise, fill in
 * YYVSP[YYLOW1 .. *YYLOW-1] as in yyfillin and set *YYLOW = YYLOW1.
 * For convenience, always return YYLOW1.  */
static inline int yyfill (yyGLRStackItem *, int *, int, yybool)
     YY_ATTRIBUTE_UNUSED;
static inline int
yyfill (yyGLRStackItem *yyvsp, int *yylow, int yylow1, yybool yynormal)
{
  if (!yynormal && yylow1 < *yylow)
    {
      yyfillin (yyvsp, *yylow, yylow1);
      *yylow = yylow1;
    }
  return yylow1;
}

/** Perform user action for rule number YYN, with RHS length YYRHSLEN,
 *  and top stack item YYVSP.  YYLVALP points to place to put semantic
 *  value ($$), and yylocp points to place for location information
 *  (@$).  Returns yyok for normal return, yyaccept for YYACCEPT,
 *  yyerr for YYERROR, yyabort for YYABORT.  */
static YYRESULTTAG
yyuserAction (yyRuleNum yyn, int yyrhslen, yyGLRStackItem* yyvsp,
              yyGLRStack* yystackp,
              YYSTYPE* yyvalp, YYLTYPE *yylocp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  yybool yynormal YY_ATTRIBUTE_UNUSED = yystackp->yysplitPoint == YY_NULLPTR;
  int yylow;
  YYUSE (yyvalp);
  YYUSE (yylocp);
  YYUSE (driver);
  YYUSE (yyscanner);
  YYUSE (yyrhslen);
# undef yyerrok
# define yyerrok (yystackp->yyerrState = 0)
# undef YYACCEPT
# define YYACCEPT return yyaccept
# undef YYABORT
# define YYABORT return yyabort
# undef YYERROR
# define YYERROR return yyerrok, yyerr
# undef YYRECOVERING
# define YYRECOVERING() (yystackp->yyerrState != 0)
# undef yyclearin
# define yyclearin (yychar = YYEMPTY)
# undef YYFILL
# define YYFILL(N) yyfill (yyvsp, &yylow, (N), yynormal)
# undef YYBACKUP
# define YYBACKUP(Token, Value)                                              \
  return yyerror (yylocp, driver, yyscanner, YY_("syntax error: cannot back up")),     \
         yyerrok, yyerr

  yylow = 1;
  if (yyrhslen == 0)
    *yyvalp = yyval_default;
  else
    *yyvalp = yyvsp[YYFILL (1-yyrhslen)].yystate.yysemantics.yysval;
  /* Default location. */
  YYLLOC_DEFAULT ((*yylocp), (yyvsp - yyrhslen), yyrhslen);
  yystackp->yyerror_range[1].yystate.yyloc = *yylocp;

  switch (yyn)
    {
  case 2: /* message: header opcode_specific options  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 3: /* $@1: %empty  */
                                                                    { driver->record_->version =
                                                                        static_cast<enum PCP_Codes::VersionType> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 4: /* $@2: %empty  */
                                                                    { driver->record_->opcode = static_cast<ACE_UINT8> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 5: /* $@3: %empty  */
                                                                    { driver->record_->reserved = static_cast<ACE_UINT8> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 6: /* $@4: %empty  */
                                                                    { driver->record_->result_code =
                                                                        static_cast<enum PCP_Codes::ResultCodeType> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 7: /* $@5: %empty  */
                                                                    { driver->record_->lifetime = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 8: /* $@6: %empty  */
                                                                    { driver->record_->epoch_time = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 9: /* header: "version" $@1 "opcode" $@2 "reserved" $@3 "result_code" $@4 "lifetime" $@5 "epoch_time" $@6 "reserved_2"  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-12)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival);
                                                                      driver->record_->reserved_2 = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 10: /* opcode_specific: opcode_specific_map  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 11: /* opcode_specific: opcode_specific_peer  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 12: /* opcode_specific: opcode_specific_authentication  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 13: /* opcode_specific: %empty  */
                                                                    { ((*yyvalp).ival) = 0; }
    break;

  case 14: /* $@7: %empty  */
                                                                    { driver->record_->map.nonce = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 15: /* $@8: %empty  */
                                                                    { driver->record_->map.protocol = static_cast<ACE_UINT8> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 16: /* $@9: %empty  */
                                                                    { driver->record_->map.reserved = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 17: /* $@10: %empty  */
                                                                    { driver->record_->map.internal_port = static_cast<ACE_UINT16> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 18: /* $@11: %empty  */
                                                                    { driver->record_->map.external_port = static_cast<ACE_UINT16> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 19: /* opcode_specific_map: "option_map_nonce" $@7 "option_map_protocol" $@8 "option_map_reserved" $@9 "option_map_internal_port" $@10 "option_map_assigned_external_port" $@11 "option_map_assigned_external_ip_address"  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival);
                                                                      ACE_NEW_NORETURN (driver->record_->map.external_address,
                                                                                        ACE_INET_Addr (driver->record_->map.external_port,
                                                                                                       static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival))));
                                                                      if (!driver->record_->map.external_address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
    break;

  case 20: /* $@12: %empty  */
                                                                    { driver->record_->peer.nonce = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 21: /* $@13: %empty  */
                                                                    { driver->record_->peer.protocol = static_cast<ACE_UINT8> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 22: /* $@14: %empty  */
                                                                    { driver->record_->peer.reserved = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 23: /* $@15: %empty  */
                                                                    { driver->record_->peer.internal_port = static_cast<ACE_UINT16> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 24: /* $@16: %empty  */
                                                                    { driver->record_->peer.external_port = static_cast<ACE_UINT16> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 25: /* $@17: %empty  */
                                                                    { ACE_NEW_NORETURN (driver->record_->peer.external_address,
                                                                                        ACE_INET_Addr (driver->record_->peer.external_port,
                                                                                                       static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival))));
                                                                      if (!driver->record_->peer.external_address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
    break;

  case 26: /* $@18: %empty  */
                                                                    { driver->record_->peer.remote_peer_port = static_cast<ACE_UINT16> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 27: /* $@19: %empty  */
                                                                    { driver->record_->peer.reserved_2 = static_cast<ACE_UINT16> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 28: /* opcode_specific_peer: "option_peer_nonce" $@12 "option_peer_protocol" $@13 "option_peer_reserved" $@14 "option_peer_internal_port" $@15 "option_peer_assigned_external_port" $@16 "option_peer_assigned_external_ip_address" $@17 "option_peer_remote_port" $@18 "option_peer_reserved_2" $@19 "option_peer_remote_peer_address"  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-16)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-14)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-12)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-10)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-8)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival);
                                                                      ACE_NEW_NORETURN (driver->record_->peer.remote_peer_address,
                                                                                        ACE_INET_Addr (driver->record_->peer.remote_peer_port,
                                                                                                       static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival))));
                                                                      if (!driver->record_->peer.remote_peer_address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
    break;

  case 29: /* $@20: %empty  */
                                                                    { driver->record_->authentication.session_id = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 30: /* opcode_specific_authentication: "option_authentication_session_id" $@20 "option_authentication_sequence_number"  */
                                                                        { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); // rfc7652
                                                                          driver->record_->authentication.sequence_number = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 31: /* options: option options  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-1)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 32: /* options: %empty  */
                                                                    { ((*yyvalp).ival) = 0; }
    break;

  case 33: /* $@21: %empty  */
                                                                    { struct PCPOption option_s;
                                                                      option_s.code = static_cast<enum PCP_Codes::OptionType> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival));
                                                                      driver->record_->options.push_back (option_s); }
    break;

  case 34: /* $@22: %empty  */
                                                                    { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.reserved = static_cast<ACE_UINT8> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 35: /* $@23: %empty  */
                                                                    { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.length = static_cast<ACE_UINT16> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 36: /* option: "option_code" $@21 "option_reserved" $@22 "option_length" $@23 option_data  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 37: /* option_data: option_data_third_party  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 38: /* option_data: option_data_filter  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 39: /* option_data: option_data_nonce  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 40: /* option_data: option_data_authentication_tag  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 41: /* option_data: option_data_pa_authentication_tag  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 42: /* option_data: option_data_eap_payload  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 43: /* option_data: option_data_pseudo_random_function  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 44: /* option_data: option_data_mac_algorithm  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 45: /* option_data: option_data_session_lifetime  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 46: /* option_data: option_data_received_pak  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 47: /* option_data: option_data_id_indicator  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); }
    break;

  case 48: /* option_data: %empty  */
                                                                    { ((*yyvalp).ival) = 0; }
    break;

  case 49: /* option_data_third_party: "option_third_party_address"  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival);
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      ACE_NEW_NORETURN (option_r.third_party.address,
                                                                                        ACE_INET_Addr (static_cast<u_short> (0),
                                                                                                       static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival))));
                                                                      if (!option_r.third_party.address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
    break;

  case 50: /* $@24: %empty  */
                                                                    { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.filter.reserved = static_cast<ACE_UINT8> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 51: /* $@25: %empty  */
                                                                    { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.filter.prefix_length = static_cast<ACE_UINT8> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 52: /* $@26: %empty  */
                                                                    { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.filter.remote_peer_port = static_cast<ACE_UINT16> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 53: /* option_data_filter: "option_filter_reserved" $@24 "option_filter_prefix_length" $@25 "option_filter_remote_peer_port" $@26 "option_filter_remote_peer_address"  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival);
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      ACE_NEW_NORETURN (option_r.filter.remote_peer_address,
                                                                                        ACE_INET_Addr (option_r.filter.remote_peer_port,
                                                                                                       static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival))));
                                                                      if (!option_r.filter.remote_peer_address)
                                                                      {
                                                                        ACE_DEBUG ((LM_CRITICAL,
                                                                                    ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
                                                                        YYABORT;
                                                                      } // end IF
                                                                    }
    break;

  case 54: /* option_data_nonce: "option_nonce_nonce"  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); // rfc7652
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.nonce.nonce = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 55: /* $@27: %empty  */
                                                                       { struct PCPOption& option_r = driver->record_->options.back (); // rfc7652
                                                                         option_r.authentication_tag.session_id = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 56: /* $@28: %empty  */
                                                                            { struct PCPOption& option_r = driver->record_->options.back ();
                                                                              option_r.authentication_tag.sequence_number = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 57: /* $@29: %empty  */
                                                                    { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.authentication_tag.key_id = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 58: /* option_data_authentication_tag: "option_authentication_tag_session_id" $@27 "option_authentication_tag_sequence_number" $@28 "option_authentication_tag_key_id" $@29 "option_authentication_tag_data"  */
                                                                    { struct PCPOption& option_r = driver->record_->options.back ();
                                                                      ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-6)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-4)].yystate.yysemantics.yysval.ival) + (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.ival) + (option_r.length - 12);
                                                                      option_r.authentication_tag.data = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.aval); }
    break;

  case 59: /* $@30: %empty  */
                                                                         { struct PCPOption& option_r = driver->record_->options.back (); // rfc7652
                                                                           option_r.pa_authentication_tag.key_id = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 60: /* option_data_pa_authentication_tag: "option_pa_authentication_tag_key_id" $@30 "option_pa_authentication_tag_data"  */
                                                                       { struct PCPOption& option_r = driver->record_->options.back ();
                                                                         ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (-2)].yystate.yysemantics.yysval.ival) + (option_r.length - 4);
                                                                         option_r.pa_authentication_tag.data = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.aval); }
    break;

  case 61: /* option_data_eap_payload: "option_eap_payload_data"  */
                                                                    { struct PCPOption& option_r = driver->record_->options.back (); // rfc7652
                                                                      ((*yyvalp).ival) = option_r.length;
                                                                      option_r.eap_payload.data = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.aval); }
    break;

  case 62: /* option_data_pseudo_random_function: "option_pseudo_random_function_id"  */
                                                                       { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); // rfc7652
                                                                         struct PCPOption& option_r = driver->record_->options.back ();
                                                                         option_r.pseudo_random_function.id = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 63: /* option_data_mac_algorithm: "option_mac_algorithm_id"  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); // rfc7652
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.mac_algorithm.id = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 64: /* option_data_session_lifetime: "option_session_lifetime_lifetime"  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); // rfc7652
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.session_lifetime.lifetime = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 65: /* option_data_received_pak: "option_received_pak_sequence_number"  */
                                                                    { ((*yyvalp).ival) = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival); // rfc7652
                                                                      struct PCPOption& option_r = driver->record_->options.back ();
                                                                      option_r.received_pak.sequence_number = static_cast<ACE_UINT32> ((YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.ival)); }
    break;

  case 66: /* option_data_id_indicator: "option_id_indicator_data"  */
                                                                    { struct PCPOption& option_r = driver->record_->options.back (); // rfc7652
                                                                      ((*yyvalp).ival) = option_r.length;
                                                                      option_r.id_indicator.data = (YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL (0)].yystate.yysemantics.yysval.aval); }
    break;



      default: break;
    }

  return yyok;
# undef yyerrok
# undef YYABORT
# undef YYACCEPT
# undef YYERROR
# undef YYBACKUP
# undef yyclearin
# undef YYRECOVERING
}


static void
yyuserMerge (int yyn, YYSTYPE* yy0, YYSTYPE* yy1)
{
  YYUSE (yy0);
  YYUSE (yy1);

  switch (yyn)
    {

      default: break;
    }
}

                              /* Bison grammar-table manipulation.  */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (driver);
  YYUSE (yyscanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_YYEOF: /* "end"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_VERSION: /* "version"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPCODE: /* "opcode"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_RESERVED: /* "reserved"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_RESULT_CODE: /* "result_code"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_LIFETIME: /* "lifetime"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_EPOCH_TIME: /* "epoch_time"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_RESERVED_2: /* "reserved_2"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_MAP_NONCE: /* "option_map_nonce"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_MAP_PROTOCOL: /* "option_map_protocol"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_MAP_RESERVED: /* "option_map_reserved"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_MAP_INTERNAL_PORT: /* "option_map_internal_port"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_MAP_ASSIGNED_EXTERNAL_PORT: /* "option_map_assigned_external_port"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_MAP_ASSIGNED_EXTERNAL_IP_ADDRESS: /* "option_map_assigned_external_ip_address"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PEER_NONCE: /* "option_peer_nonce"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PEER_PROTOCOL: /* "option_peer_protocol"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PEER_RESERVED: /* "option_peer_reserved"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PEER_INTERNAL_PORT: /* "option_peer_internal_port"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PEER_ASSIGNED_EXTERNAL_PORT: /* "option_peer_assigned_external_port"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PEER_ASSIGNED_EXTERNAL_IP_ADDRESS: /* "option_peer_assigned_external_ip_address"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PEER_REMOTE_PEER_PORT: /* "option_peer_remote_port"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PEER_RESERVED_2: /* "option_peer_reserved_2"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PEER_REMOTE_PEER_IP_ADDRESS: /* "option_peer_remote_peer_address"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_SESSION_ID: /* "option_authentication_session_id"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_SEQUENCE_NUMBER: /* "option_authentication_sequence_number"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_CODE: /* "option_code"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_RESERVED: /* "option_reserved"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_LENGTH: /* "option_length"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_THIRD_PARTY_ADDRESS: /* "option_third_party_address"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_FILTER_RESERVED: /* "option_filter_reserved"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_FILTER_PREFIX_LENGTH: /* "option_filter_prefix_length"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_FILTER_REMOTE_PEER_PORT: /* "option_filter_remote_peer_port"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_FILTER_REMOTE_PEER_IP_ADDRESS: /* "option_filter_remote_peer_address"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_NONCE_NONCE: /* "option_nonce_nonce"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_TAG_SESSION_ID: /* "option_authentication_tag_session_id"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_TAG_SEQUENCE_NUMBER: /* "option_authentication_tag_sequence_number"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_TAG_KEY_ID: /* "option_authentication_tag_key_id"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_AUTHENTICATION_TAG_DATA: /* "option_authentication_tag_data"  */
                          { delete[] ((*yyvaluep).aval); ((*yyvaluep).aval) = NULL; }
        break;

    case YYSYMBOL_OPTION_PA_AUTHENTICATION_TAG_KEY_ID: /* "option_pa_authentication_tag_key_id"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_PA_AUTHENTICATION_TAG_DATA: /* "option_pa_authentication_tag_data"  */
                          { delete[] ((*yyvaluep).aval); ((*yyvaluep).aval) = NULL; }
        break;

    case YYSYMBOL_OPTION_EAP_PAYLOAD_DATA: /* "option_eap_payload_data"  */
                          { delete[] ((*yyvaluep).aval); ((*yyvaluep).aval) = NULL; }
        break;

    case YYSYMBOL_OPTION_PSEUDO_RANDOM_FUNCTION_ID: /* "option_pseudo_random_function_id"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_MAC_ALGORITHM_ID: /* "option_mac_algorithm_id"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_SESSION_LIFETIME_LIFETIME: /* "option_session_lifetime_lifetime"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_RECEIVED_PAK_SEQUENCE_NUMBER: /* "option_received_pak_sequence_number"  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_OPTION_ID_INDICATOR_DATA: /* "option_id_indicator_data"  */
                          { delete[] ((*yyvaluep).aval); ((*yyvaluep).aval) = NULL; }
        break;

    case YYSYMBOL_message: /* message  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_header: /* header  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_opcode_specific: /* opcode_specific  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_opcode_specific_map: /* opcode_specific_map  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_opcode_specific_peer: /* opcode_specific_peer  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_opcode_specific_authentication: /* opcode_specific_authentication  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_options: /* options  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option: /* option  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data: /* option_data  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_third_party: /* option_data_third_party  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_filter: /* option_data_filter  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_nonce: /* option_data_nonce  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_authentication_tag: /* option_data_authentication_tag  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_pa_authentication_tag: /* option_data_pa_authentication_tag  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_eap_payload: /* option_data_eap_payload  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_pseudo_random_function: /* option_data_pseudo_random_function  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_mac_algorithm: /* option_data_mac_algorithm  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_session_lifetime: /* option_data_session_lifetime  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_received_pak: /* option_data_received_pak  */
                          { ((*yyvaluep).ival) = 0; }
        break;

    case YYSYMBOL_option_data_id_indicator: /* option_data_id_indicator  */
                          { ((*yyvaluep).ival) = 0; }
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}

/** Number of symbols composing the right hand side of rule #RULE.  */
static inline int
yyrhsLength (yyRuleNum yyrule)
{
  return yyr2[yyrule];
}

static void
yydestroyGLRState (char const *yymsg, yyGLRState *yys, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  if (yys->yyresolved)
    yydestruct (yymsg, yy_accessing_symbol (yys->yylrState),
                &yys->yysemantics.yysval, &yys->yyloc, driver, yyscanner);
  else
    {
#if YYDEBUG
      if (yydebug)
        {
          if (yys->yysemantics.yyfirstVal)
            YY_FPRINTF ((stderr, "%s unresolved", yymsg));
          else
            YY_FPRINTF ((stderr, "%s incomplete", yymsg));
          YY_SYMBOL_PRINT ("", yy_accessing_symbol (yys->yylrState), YY_NULLPTR, &yys->yyloc);
        }
#endif

      if (yys->yysemantics.yyfirstVal)
        {
          yySemanticOption *yyoption = yys->yysemantics.yyfirstVal;
          yyGLRState *yyrh;
          int yyn;
          for (yyrh = yyoption->yystate, yyn = yyrhsLength (yyoption->yyrule);
               yyn > 0;
               yyrh = yyrh->yypred, yyn -= 1)
            yydestroyGLRState (yymsg, yyrh, driver, yyscanner);
        }
    }
}

/** Left-hand-side symbol for rule #YYRULE.  */
static inline yysymbol_kind_t
yylhsNonterm (yyRuleNum yyrule)
{
  return YY_CAST (yysymbol_kind_t, yyr1[yyrule]);
}

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

/** True iff LR state YYSTATE has only a default reduction (regardless
 *  of token).  */
static inline yybool
yyisDefaultedState (yy_state_t yystate)
{
  return yypact_value_is_default (yypact[yystate]);
}

/** The default reduction for YYSTATE, assuming it has one.  */
static inline yyRuleNum
yydefaultAction (yy_state_t yystate)
{
  return yydefact[yystate];
}

#define yytable_value_is_error(Yyn) \
  0

/** The action to take in YYSTATE on seeing YYTOKEN.
 *  Result R means
 *    R < 0:  Reduce on rule -R.
 *    R = 0:  Error.
 *    R > 0:  Shift to state R.
 *  Set *YYCONFLICTS to a pointer into yyconfl to a 0-terminated list
 *  of conflicting reductions.
 */
static inline int
yygetLRActions (yy_state_t yystate, yysymbol_kind_t yytoken, const short** yyconflicts)
{
  int yyindex = yypact[yystate] + yytoken;
  if (yytoken == YYSYMBOL_YYerror)
    {
      // This is the error token.
      *yyconflicts = yyconfl;
      return 0;
    }
  else if (yyisDefaultedState (yystate)
           || yyindex < 0 || YYLAST < yyindex || yycheck[yyindex] != yytoken)
    {
      *yyconflicts = yyconfl;
      return -yydefact[yystate];
    }
  else if (! yytable_value_is_error (yytable[yyindex]))
    {
      *yyconflicts = yyconfl + yyconflp[yyindex];
      return yytable[yyindex];
    }
  else
    {
      *yyconflicts = yyconfl + yyconflp[yyindex];
      return 0;
    }
}

/** Compute post-reduction state.
 * \param yystate   the current state
 * \param yysym     the nonterminal to push on the stack
 */
static inline yy_state_t
yyLRgotoState (yy_state_t yystate, yysymbol_kind_t yysym)
{
  int yyr = yypgoto[yysym - YYNTOKENS] + yystate;
  if (0 <= yyr && yyr <= YYLAST && yycheck[yyr] == yystate)
    return yytable[yyr];
  else
    return yydefgoto[yysym - YYNTOKENS];
}

static inline yybool
yyisShiftAction (int yyaction)
{
  return 0 < yyaction;
}

static inline yybool
yyisErrorAction (int yyaction)
{
  return yyaction == 0;
}

                                /* GLRStates */

/** Return a fresh GLRStackItem in YYSTACKP.  The item is an LR state
 *  if YYISSTATE, and otherwise a semantic option.  Callers should call
 *  YY_RESERVE_GLRSTACK afterwards to make sure there is sufficient
 *  headroom.  */

static inline yyGLRStackItem*
yynewGLRStackItem (yyGLRStack* yystackp, yybool yyisState)
{
  yyGLRStackItem* yynewItem = yystackp->yynextFree;
  yystackp->yyspaceLeft -= 1;
  yystackp->yynextFree += 1;
  yynewItem->yystate.yyisState = yyisState;
  return yynewItem;
}

/** Add a new semantic action that will execute the action for rule
 *  YYRULE on the semantic values in YYRHS to the list of
 *  alternative actions for YYSTATE.  Assumes that YYRHS comes from
 *  stack #YYK of *YYSTACKP. */
static void
yyaddDeferredAction (yyGLRStack* yystackp, YYPTRDIFF_T yyk, yyGLRState* yystate,
                     yyGLRState* yyrhs, yyRuleNum yyrule)
{
  yySemanticOption* yynewOption =
    &yynewGLRStackItem (yystackp, yyfalse)->yyoption;
  YY_ASSERT (!yynewOption->yyisState);
  yynewOption->yystate = yyrhs;
  yynewOption->yyrule = yyrule;
  if (yystackp->yytops.yylookaheadNeeds[yyk])
    {
      yynewOption->yyrawchar = yychar;
      yynewOption->yyval = yylval;
      yynewOption->yyloc = yylloc;
    }
  else
    yynewOption->yyrawchar = YYEMPTY;
  yynewOption->yynext = yystate->yysemantics.yyfirstVal;
  yystate->yysemantics.yyfirstVal = yynewOption;

  YY_RESERVE_GLRSTACK (yystackp);
}

                                /* GLRStacks */

/** Initialize YYSET to a singleton set containing an empty stack.  */
static yybool
yyinitStateSet (yyGLRStateSet* yyset)
{
  yyset->yysize = 1;
  yyset->yycapacity = 16;
  yyset->yystates
    = YY_CAST (yyGLRState**,
               YYMALLOC (YY_CAST (YYSIZE_T, yyset->yycapacity)
                         * sizeof yyset->yystates[0]));
  if (! yyset->yystates)
    return yyfalse;
  yyset->yystates[0] = YY_NULLPTR;
  yyset->yylookaheadNeeds
    = YY_CAST (yybool*,
               YYMALLOC (YY_CAST (YYSIZE_T, yyset->yycapacity)
                         * sizeof yyset->yylookaheadNeeds[0]));
  if (! yyset->yylookaheadNeeds)
    {
      YYFREE (yyset->yystates);
      return yyfalse;
    }
  memset (yyset->yylookaheadNeeds,
          0,
          YY_CAST (YYSIZE_T, yyset->yycapacity) * sizeof yyset->yylookaheadNeeds[0]);
  return yytrue;
}

static void yyfreeStateSet (yyGLRStateSet* yyset)
{
  YYFREE (yyset->yystates);
  YYFREE (yyset->yylookaheadNeeds);
}

/** Initialize *YYSTACKP to a single empty stack, with total maximum
 *  capacity for all stacks of YYSIZE.  */
static yybool
yyinitGLRStack (yyGLRStack* yystackp, YYPTRDIFF_T yysize)
{
  yystackp->yyerrState = 0;
  yynerrs = 0;
  yystackp->yyspaceLeft = yysize;
  yystackp->yyitems
    = YY_CAST (yyGLRStackItem*,
               YYMALLOC (YY_CAST (YYSIZE_T, yysize)
                         * sizeof yystackp->yynextFree[0]));
  if (!yystackp->yyitems)
    return yyfalse;
  yystackp->yynextFree = yystackp->yyitems;
  yystackp->yysplitPoint = YY_NULLPTR;
  yystackp->yylastDeleted = YY_NULLPTR;
  return yyinitStateSet (&yystackp->yytops);
}


#if YYSTACKEXPANDABLE
# define YYRELOC(YYFROMITEMS, YYTOITEMS, YYX, YYTYPE)                   \
  &((YYTOITEMS)                                                         \
    - ((YYFROMITEMS) - YY_REINTERPRET_CAST (yyGLRStackItem*, (YYX))))->YYTYPE

/** If *YYSTACKP is expandable, extend it.  WARNING: Pointers into the
    stack from outside should be considered invalid after this call.
    We always expand when there are 1 or fewer items left AFTER an
    allocation, so that we can avoid having external pointers exist
    across an allocation.  */
static void
yyexpandGLRStack (yyGLRStack* yystackp)
{
  yyGLRStackItem* yynewItems;
  yyGLRStackItem* yyp0, *yyp1;
  YYPTRDIFF_T yynewSize;
  YYPTRDIFF_T yyn;
  YYPTRDIFF_T yysize = yystackp->yynextFree - yystackp->yyitems;
  if (YYMAXDEPTH - YYHEADROOM < yysize)
    yyMemoryExhausted (yystackp);
  yynewSize = 2*yysize;
  if (YYMAXDEPTH < yynewSize)
    yynewSize = YYMAXDEPTH;
  yynewItems
    = YY_CAST (yyGLRStackItem*,
               YYMALLOC (YY_CAST (YYSIZE_T, yynewSize)
                         * sizeof yynewItems[0]));
  if (! yynewItems)
    yyMemoryExhausted (yystackp);
  for (yyp0 = yystackp->yyitems, yyp1 = yynewItems, yyn = yysize;
       0 < yyn;
       yyn -= 1, yyp0 += 1, yyp1 += 1)
    {
      *yyp1 = *yyp0;
      if (*YY_REINTERPRET_CAST (yybool *, yyp0))
        {
          yyGLRState* yys0 = &yyp0->yystate;
          yyGLRState* yys1 = &yyp1->yystate;
          if (yys0->yypred != YY_NULLPTR)
            yys1->yypred =
              YYRELOC (yyp0, yyp1, yys0->yypred, yystate);
          if (! yys0->yyresolved && yys0->yysemantics.yyfirstVal != YY_NULLPTR)
            yys1->yysemantics.yyfirstVal =
              YYRELOC (yyp0, yyp1, yys0->yysemantics.yyfirstVal, yyoption);
        }
      else
        {
          yySemanticOption* yyv0 = &yyp0->yyoption;
          yySemanticOption* yyv1 = &yyp1->yyoption;
          if (yyv0->yystate != YY_NULLPTR)
            yyv1->yystate = YYRELOC (yyp0, yyp1, yyv0->yystate, yystate);
          if (yyv0->yynext != YY_NULLPTR)
            yyv1->yynext = YYRELOC (yyp0, yyp1, yyv0->yynext, yyoption);
        }
    }
  if (yystackp->yysplitPoint != YY_NULLPTR)
    yystackp->yysplitPoint = YYRELOC (yystackp->yyitems, yynewItems,
                                      yystackp->yysplitPoint, yystate);

  for (yyn = 0; yyn < yystackp->yytops.yysize; yyn += 1)
    if (yystackp->yytops.yystates[yyn] != YY_NULLPTR)
      yystackp->yytops.yystates[yyn] =
        YYRELOC (yystackp->yyitems, yynewItems,
                 yystackp->yytops.yystates[yyn], yystate);
  YYFREE (yystackp->yyitems);
  yystackp->yyitems = yynewItems;
  yystackp->yynextFree = yynewItems + yysize;
  yystackp->yyspaceLeft = yynewSize - yysize;
}
#endif

static void
yyfreeGLRStack (yyGLRStack* yystackp)
{
  YYFREE (yystackp->yyitems);
  yyfreeStateSet (&yystackp->yytops);
}

/** Assuming that YYS is a GLRState somewhere on *YYSTACKP, update the
 *  splitpoint of *YYSTACKP, if needed, so that it is at least as deep as
 *  YYS.  */
static inline void
yyupdateSplit (yyGLRStack* yystackp, yyGLRState* yys)
{
  if (yystackp->yysplitPoint != YY_NULLPTR && yystackp->yysplitPoint > yys)
    yystackp->yysplitPoint = yys;
}

/** Invalidate stack #YYK in *YYSTACKP.  */
static inline void
yymarkStackDeleted (yyGLRStack* yystackp, YYPTRDIFF_T yyk)
{
  if (yystackp->yytops.yystates[yyk] != YY_NULLPTR)
    yystackp->yylastDeleted = yystackp->yytops.yystates[yyk];
  yystackp->yytops.yystates[yyk] = YY_NULLPTR;
}

/** Undelete the last stack in *YYSTACKP that was marked as deleted.  Can
    only be done once after a deletion, and only when all other stacks have
    been deleted.  */
static void
yyundeleteLastStack (yyGLRStack* yystackp)
{
  if (yystackp->yylastDeleted == YY_NULLPTR || yystackp->yytops.yysize != 0)
    return;
  yystackp->yytops.yystates[0] = yystackp->yylastDeleted;
  yystackp->yytops.yysize = 1;
  YY_DPRINTF ((stderr, "Restoring last deleted stack as stack #0.\n"));
  yystackp->yylastDeleted = YY_NULLPTR;
}

static inline void
yyremoveDeletes (yyGLRStack* yystackp)
{
  YYPTRDIFF_T yyi, yyj;
  yyi = yyj = 0;
  while (yyj < yystackp->yytops.yysize)
    {
      if (yystackp->yytops.yystates[yyi] == YY_NULLPTR)
        {
          if (yyi == yyj)
            YY_DPRINTF ((stderr, "Removing dead stacks.\n"));
          yystackp->yytops.yysize -= 1;
        }
      else
        {
          yystackp->yytops.yystates[yyj] = yystackp->yytops.yystates[yyi];
          /* In the current implementation, it's unnecessary to copy
             yystackp->yytops.yylookaheadNeeds[yyi] since, after
             yyremoveDeletes returns, the parser immediately either enters
             deterministic operation or shifts a token.  However, it doesn't
             hurt, and the code might evolve to need it.  */
          yystackp->yytops.yylookaheadNeeds[yyj] =
            yystackp->yytops.yylookaheadNeeds[yyi];
          if (yyj != yyi)
            YY_DPRINTF ((stderr, "Rename stack %ld -> %ld.\n",
                        YY_CAST (long, yyi), YY_CAST (long, yyj)));
          yyj += 1;
        }
      yyi += 1;
    }
}

/** Shift to a new state on stack #YYK of *YYSTACKP, corresponding to LR
 * state YYLRSTATE, at input position YYPOSN, with (resolved) semantic
 * value *YYVALP and source location *YYLOCP.  */
static inline void
yyglrShift (yyGLRStack* yystackp, YYPTRDIFF_T yyk, yy_state_t yylrState,
            YYPTRDIFF_T yyposn,
            YYSTYPE* yyvalp, YYLTYPE* yylocp)
{
  yyGLRState* yynewState = &yynewGLRStackItem (yystackp, yytrue)->yystate;

  yynewState->yylrState = yylrState;
  yynewState->yyposn = yyposn;
  yynewState->yyresolved = yytrue;
  yynewState->yypred = yystackp->yytops.yystates[yyk];
  yynewState->yysemantics.yysval = *yyvalp;
  yynewState->yyloc = *yylocp;
  yystackp->yytops.yystates[yyk] = yynewState;

  YY_RESERVE_GLRSTACK (yystackp);
}

/** Shift stack #YYK of *YYSTACKP, to a new state corresponding to LR
 *  state YYLRSTATE, at input position YYPOSN, with the (unresolved)
 *  semantic value of YYRHS under the action for YYRULE.  */
static inline void
yyglrShiftDefer (yyGLRStack* yystackp, YYPTRDIFF_T yyk, yy_state_t yylrState,
                 YYPTRDIFF_T yyposn, yyGLRState* yyrhs, yyRuleNum yyrule)
{
  yyGLRState* yynewState = &yynewGLRStackItem (yystackp, yytrue)->yystate;
  YY_ASSERT (yynewState->yyisState);

  yynewState->yylrState = yylrState;
  yynewState->yyposn = yyposn;
  yynewState->yyresolved = yyfalse;
  yynewState->yypred = yystackp->yytops.yystates[yyk];
  yynewState->yysemantics.yyfirstVal = YY_NULLPTR;
  yystackp->yytops.yystates[yyk] = yynewState;

  /* Invokes YY_RESERVE_GLRSTACK.  */
  yyaddDeferredAction (yystackp, yyk, yynewState, yyrhs, yyrule);
}

#if !YYDEBUG
# define YY_REDUCE_PRINT(Args)
#else
# define YY_REDUCE_PRINT(Args)          \
  do {                                  \
    if (yydebug)                        \
      yy_reduce_print Args;             \
  } while (0)

/*----------------------------------------------------------------------.
| Report that stack #YYK of *YYSTACKP is going to be reduced by YYRULE. |
`----------------------------------------------------------------------*/

static inline void
yy_reduce_print (yybool yynormal, yyGLRStackItem* yyvsp, YYPTRDIFF_T yyk,
                 yyRuleNum yyrule, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  int yynrhs = yyrhsLength (yyrule);
  int yylow = 1;
  int yyi;
  YY_FPRINTF ((stderr, "Reducing stack %ld by rule %d (line %d):\n",
               YY_CAST (long, yyk), yyrule - 1, yyrline[yyrule]));
  if (! yynormal)
    yyfillin (yyvsp, 1, -yynrhs);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YY_FPRINTF ((stderr, "   $%d = ", yyi + 1));
      yy_symbol_print (stderr,
                       yy_accessing_symbol (yyvsp[yyi - yynrhs + 1].yystate.yylrState),
                       &yyvsp[yyi - yynrhs + 1].yystate.yysemantics.yysval,
                       &(YY_CAST (yyGLRStackItem const *, yyvsp)[YYFILL ((yyi + 1) - (yynrhs))].yystate.yyloc)                       , driver, yyscanner);
      if (!yyvsp[yyi - yynrhs + 1].yystate.yyresolved)
        YY_FPRINTF ((stderr, " (unresolved)"));
      YY_FPRINTF ((stderr, "\n"));
    }
}
#endif

/** Pop the symbols consumed by reduction #YYRULE from the top of stack
 *  #YYK of *YYSTACKP, and perform the appropriate semantic action on their
 *  semantic values.  Assumes that all ambiguities in semantic values
 *  have been previously resolved.  Set *YYVALP to the resulting value,
 *  and *YYLOCP to the computed location (if any).  Return value is as
 *  for userAction.  */
static inline YYRESULTTAG
yydoAction (yyGLRStack* yystackp, YYPTRDIFF_T yyk, yyRuleNum yyrule,
            YYSTYPE* yyvalp, YYLTYPE *yylocp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  int yynrhs = yyrhsLength (yyrule);

  if (yystackp->yysplitPoint == YY_NULLPTR)
    {
      /* Standard special case: single stack.  */
      yyGLRStackItem* yyrhs
        = YY_REINTERPRET_CAST (yyGLRStackItem*, yystackp->yytops.yystates[yyk]);
      YY_ASSERT (yyk == 0);
      yystackp->yynextFree -= yynrhs;
      yystackp->yyspaceLeft += yynrhs;
      yystackp->yytops.yystates[0] = & yystackp->yynextFree[-1].yystate;
      YY_REDUCE_PRINT ((yytrue, yyrhs, yyk, yyrule, driver, yyscanner));
      return yyuserAction (yyrule, yynrhs, yyrhs, yystackp,
                           yyvalp, yylocp, driver, yyscanner);
    }
  else
    {
      yyGLRStackItem yyrhsVals[YYMAXRHS + YYMAXLEFT + 1];
      yyGLRState* yys = yyrhsVals[YYMAXRHS + YYMAXLEFT].yystate.yypred
        = yystackp->yytops.yystates[yyk];
      int yyi;
      if (yynrhs == 0)
        /* Set default location.  */
        yyrhsVals[YYMAXRHS + YYMAXLEFT - 1].yystate.yyloc = yys->yyloc;
      for (yyi = 0; yyi < yynrhs; yyi += 1)
        {
          yys = yys->yypred;
          YY_ASSERT (yys);
        }
      yyupdateSplit (yystackp, yys);
      yystackp->yytops.yystates[yyk] = yys;
      YY_REDUCE_PRINT ((yyfalse, yyrhsVals + YYMAXRHS + YYMAXLEFT - 1, yyk, yyrule, driver, yyscanner));
      return yyuserAction (yyrule, yynrhs, yyrhsVals + YYMAXRHS + YYMAXLEFT - 1,
                           yystackp, yyvalp, yylocp, driver, yyscanner);
    }
}

/** Pop items off stack #YYK of *YYSTACKP according to grammar rule YYRULE,
 *  and push back on the resulting nonterminal symbol.  Perform the
 *  semantic action associated with YYRULE and store its value with the
 *  newly pushed state, if YYFORCEEVAL or if *YYSTACKP is currently
 *  unambiguous.  Otherwise, store the deferred semantic action with
 *  the new state.  If the new state would have an identical input
 *  position, LR state, and predecessor to an existing state on the stack,
 *  it is identified with that existing state, eliminating stack #YYK from
 *  *YYSTACKP.  In this case, the semantic value is
 *  added to the options for the existing state's semantic value.
 */
static inline YYRESULTTAG
yyglrReduce (yyGLRStack* yystackp, YYPTRDIFF_T yyk, yyRuleNum yyrule,
             yybool yyforceEval, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  YYPTRDIFF_T yyposn = yystackp->yytops.yystates[yyk]->yyposn;

  if (yyforceEval || yystackp->yysplitPoint == YY_NULLPTR)
    {
      YYSTYPE yysval;
      YYLTYPE yyloc;

      YYRESULTTAG yyflag = yydoAction (yystackp, yyk, yyrule, &yysval, &yyloc, driver, yyscanner);
      if (yyflag == yyerr && yystackp->yysplitPoint != YY_NULLPTR)
        YY_DPRINTF ((stderr,
                     "Parse on stack %ld rejected by rule %d (line %d).\n",
                     YY_CAST (long, yyk), yyrule - 1, yyrline[yyrule - 1]));
      if (yyflag != yyok)
        return yyflag;
      YY_SYMBOL_PRINT ("-> $$ =", yylhsNonterm (yyrule), &yysval, &yyloc);
      yyglrShift (yystackp, yyk,
                  yyLRgotoState (yystackp->yytops.yystates[yyk]->yylrState,
                                 yylhsNonterm (yyrule)),
                  yyposn, &yysval, &yyloc);
    }
  else
    {
      YYPTRDIFF_T yyi;
      int yyn;
      yyGLRState* yys, *yys0 = yystackp->yytops.yystates[yyk];
      yy_state_t yynewLRState;

      for (yys = yystackp->yytops.yystates[yyk], yyn = yyrhsLength (yyrule);
           0 < yyn; yyn -= 1)
        {
          yys = yys->yypred;
          YY_ASSERT (yys);
        }
      yyupdateSplit (yystackp, yys);
      yynewLRState = yyLRgotoState (yys->yylrState, yylhsNonterm (yyrule));
      YY_DPRINTF ((stderr,
                   "Reduced stack %ld by rule %d (line %d); action deferred.  "
                   "Now in state %d.\n",
                   YY_CAST (long, yyk), yyrule - 1, yyrline[yyrule - 1],
                   yynewLRState));
      for (yyi = 0; yyi < yystackp->yytops.yysize; yyi += 1)
        if (yyi != yyk && yystackp->yytops.yystates[yyi] != YY_NULLPTR)
          {
            yyGLRState *yysplit = yystackp->yysplitPoint;
            yyGLRState *yyp = yystackp->yytops.yystates[yyi];
            while (yyp != yys && yyp != yysplit && yyp->yyposn >= yyposn)
              {
                if (yyp->yylrState == yynewLRState && yyp->yypred == yys)
                  {
                    yyaddDeferredAction (yystackp, yyk, yyp, yys0, yyrule);
                    yymarkStackDeleted (yystackp, yyk);
                    YY_DPRINTF ((stderr, "Merging stack %ld into stack %ld.\n",
                                 YY_CAST (long, yyk), YY_CAST (long, yyi)));
                    return yyok;
                  }
                yyp = yyp->yypred;
              }
          }
      yystackp->yytops.yystates[yyk] = yys;
      yyglrShiftDefer (yystackp, yyk, yynewLRState, yyposn, yys0, yyrule);
    }
  return yyok;
}

static YYPTRDIFF_T
yysplitStack (yyGLRStack* yystackp, YYPTRDIFF_T yyk)
{
  if (yystackp->yysplitPoint == YY_NULLPTR)
    {
      YY_ASSERT (yyk == 0);
      yystackp->yysplitPoint = yystackp->yytops.yystates[yyk];
    }
  if (yystackp->yytops.yycapacity <= yystackp->yytops.yysize)
    {
      YYPTRDIFF_T state_size = YYSIZEOF (yystackp->yytops.yystates[0]);
      YYPTRDIFF_T half_max_capacity = YYSIZE_MAXIMUM / 2 / state_size;
      if (half_max_capacity < yystackp->yytops.yycapacity)
        yyMemoryExhausted (yystackp);
      yystackp->yytops.yycapacity *= 2;

      {
        yyGLRState** yynewStates
          = YY_CAST (yyGLRState**,
                     YYREALLOC (yystackp->yytops.yystates,
                                (YY_CAST (YYSIZE_T, yystackp->yytops.yycapacity)
                                 * sizeof yynewStates[0])));
        if (yynewStates == YY_NULLPTR)
          yyMemoryExhausted (yystackp);
        yystackp->yytops.yystates = yynewStates;
      }

      {
        yybool* yynewLookaheadNeeds
          = YY_CAST (yybool*,
                     YYREALLOC (yystackp->yytops.yylookaheadNeeds,
                                (YY_CAST (YYSIZE_T, yystackp->yytops.yycapacity)
                                 * sizeof yynewLookaheadNeeds[0])));
        if (yynewLookaheadNeeds == YY_NULLPTR)
          yyMemoryExhausted (yystackp);
        yystackp->yytops.yylookaheadNeeds = yynewLookaheadNeeds;
      }
    }
  yystackp->yytops.yystates[yystackp->yytops.yysize]
    = yystackp->yytops.yystates[yyk];
  yystackp->yytops.yylookaheadNeeds[yystackp->yytops.yysize]
    = yystackp->yytops.yylookaheadNeeds[yyk];
  yystackp->yytops.yysize += 1;
  return yystackp->yytops.yysize - 1;
}

/** True iff YYY0 and YYY1 represent identical options at the top level.
 *  That is, they represent the same rule applied to RHS symbols
 *  that produce the same terminal symbols.  */
static yybool
yyidenticalOptions (yySemanticOption* yyy0, yySemanticOption* yyy1)
{
  if (yyy0->yyrule == yyy1->yyrule)
    {
      yyGLRState *yys0, *yys1;
      int yyn;
      for (yys0 = yyy0->yystate, yys1 = yyy1->yystate,
           yyn = yyrhsLength (yyy0->yyrule);
           yyn > 0;
           yys0 = yys0->yypred, yys1 = yys1->yypred, yyn -= 1)
        if (yys0->yyposn != yys1->yyposn)
          return yyfalse;
      return yytrue;
    }
  else
    return yyfalse;
}

/** Assuming identicalOptions (YYY0,YYY1), destructively merge the
 *  alternative semantic values for the RHS-symbols of YYY1 and YYY0.  */
static void
yymergeOptionSets (yySemanticOption* yyy0, yySemanticOption* yyy1)
{
  yyGLRState *yys0, *yys1;
  int yyn;
  for (yys0 = yyy0->yystate, yys1 = yyy1->yystate,
       yyn = yyrhsLength (yyy0->yyrule);
       0 < yyn;
       yys0 = yys0->yypred, yys1 = yys1->yypred, yyn -= 1)
    {
      if (yys0 == yys1)
        break;
      else if (yys0->yyresolved)
        {
          yys1->yyresolved = yytrue;
          yys1->yysemantics.yysval = yys0->yysemantics.yysval;
        }
      else if (yys1->yyresolved)
        {
          yys0->yyresolved = yytrue;
          yys0->yysemantics.yysval = yys1->yysemantics.yysval;
        }
      else
        {
          yySemanticOption** yyz0p = &yys0->yysemantics.yyfirstVal;
          yySemanticOption* yyz1 = yys1->yysemantics.yyfirstVal;
          while (yytrue)
            {
              if (yyz1 == *yyz0p || yyz1 == YY_NULLPTR)
                break;
              else if (*yyz0p == YY_NULLPTR)
                {
                  *yyz0p = yyz1;
                  break;
                }
              else if (*yyz0p < yyz1)
                {
                  yySemanticOption* yyz = *yyz0p;
                  *yyz0p = yyz1;
                  yyz1 = yyz1->yynext;
                  (*yyz0p)->yynext = yyz;
                }
              yyz0p = &(*yyz0p)->yynext;
            }
          yys1->yysemantics.yyfirstVal = yys0->yysemantics.yyfirstVal;
        }
    }
}

/** Y0 and Y1 represent two possible actions to take in a given
 *  parsing state; return 0 if no combination is possible,
 *  1 if user-mergeable, 2 if Y0 is preferred, 3 if Y1 is preferred.  */
static int
yypreference (yySemanticOption* y0, yySemanticOption* y1)
{
  yyRuleNum r0 = y0->yyrule, r1 = y1->yyrule;
  int p0 = yydprec[r0], p1 = yydprec[r1];

  if (p0 == p1)
    {
      if (yymerger[r0] == 0 || yymerger[r0] != yymerger[r1])
        return 0;
      else
        return 1;
    }
  if (p0 == 0 || p1 == 0)
    return 0;
  if (p0 < p1)
    return 3;
  if (p1 < p0)
    return 2;
  return 0;
}

static YYRESULTTAG yyresolveValue (yyGLRState* yys,
                                   yyGLRStack* yystackp, PCP_ParserDriver* driver, yyscan_t yyscanner);


/** Resolve the previous YYN states starting at and including state YYS
 *  on *YYSTACKP. If result != yyok, some states may have been left
 *  unresolved possibly with empty semantic option chains.  Regardless
 *  of whether result = yyok, each state has been left with consistent
 *  data so that yydestroyGLRState can be invoked if necessary.  */
static YYRESULTTAG
yyresolveStates (yyGLRState* yys, int yyn,
                 yyGLRStack* yystackp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  if (0 < yyn)
    {
      YY_ASSERT (yys->yypred);
      YYCHK (yyresolveStates (yys->yypred, yyn-1, yystackp, driver, yyscanner));
      if (! yys->yyresolved)
        YYCHK (yyresolveValue (yys, yystackp, driver, yyscanner));
    }
  return yyok;
}

/** Resolve the states for the RHS of YYOPT on *YYSTACKP, perform its
 *  user action, and return the semantic value and location in *YYVALP
 *  and *YYLOCP.  Regardless of whether result = yyok, all RHS states
 *  have been destroyed (assuming the user action destroys all RHS
 *  semantic values if invoked).  */
static YYRESULTTAG
yyresolveAction (yySemanticOption* yyopt, yyGLRStack* yystackp,
                 YYSTYPE* yyvalp, YYLTYPE *yylocp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  yyGLRStackItem yyrhsVals[YYMAXRHS + YYMAXLEFT + 1];
  int yynrhs = yyrhsLength (yyopt->yyrule);
  YYRESULTTAG yyflag =
    yyresolveStates (yyopt->yystate, yynrhs, yystackp, driver, yyscanner);
  if (yyflag != yyok)
    {
      yyGLRState *yys;
      for (yys = yyopt->yystate; yynrhs > 0; yys = yys->yypred, yynrhs -= 1)
        yydestroyGLRState ("Cleanup: popping", yys, driver, yyscanner);
      return yyflag;
    }

  yyrhsVals[YYMAXRHS + YYMAXLEFT].yystate.yypred = yyopt->yystate;
  if (yynrhs == 0)
    /* Set default location.  */
    yyrhsVals[YYMAXRHS + YYMAXLEFT - 1].yystate.yyloc = yyopt->yystate->yyloc;
  {
    int yychar_current = yychar;
    YYSTYPE yylval_current = yylval;
    YYLTYPE yylloc_current = yylloc;
    yychar = yyopt->yyrawchar;
    yylval = yyopt->yyval;
    yylloc = yyopt->yyloc;
    yyflag = yyuserAction (yyopt->yyrule, yynrhs,
                           yyrhsVals + YYMAXRHS + YYMAXLEFT - 1,
                           yystackp, yyvalp, yylocp, driver, yyscanner);
    yychar = yychar_current;
    yylval = yylval_current;
    yylloc = yylloc_current;
  }
  return yyflag;
}

#if YYDEBUG
static void
yyreportTree (yySemanticOption* yyx, int yyindent)
{
  int yynrhs = yyrhsLength (yyx->yyrule);
  int yyi;
  yyGLRState* yys;
  yyGLRState* yystates[1 + YYMAXRHS];
  yyGLRState yyleftmost_state;

  for (yyi = yynrhs, yys = yyx->yystate; 0 < yyi; yyi -= 1, yys = yys->yypred)
    yystates[yyi] = yys;
  if (yys == YY_NULLPTR)
    {
      yyleftmost_state.yyposn = 0;
      yystates[0] = &yyleftmost_state;
    }
  else
    yystates[0] = yys;

  if (yyx->yystate->yyposn < yys->yyposn + 1)
    YY_FPRINTF ((stderr, "%*s%s -> <Rule %d, empty>\n",
                 yyindent, "", yysymbol_name (yylhsNonterm (yyx->yyrule)),
                 yyx->yyrule - 1));
  else
    YY_FPRINTF ((stderr, "%*s%s -> <Rule %d, tokens %ld .. %ld>\n",
                 yyindent, "", yysymbol_name (yylhsNonterm (yyx->yyrule)),
                 yyx->yyrule - 1, YY_CAST (long, yys->yyposn + 1),
                 YY_CAST (long, yyx->yystate->yyposn)));
  for (yyi = 1; yyi <= yynrhs; yyi += 1)
    {
      if (yystates[yyi]->yyresolved)
        {
          if (yystates[yyi-1]->yyposn+1 > yystates[yyi]->yyposn)
            YY_FPRINTF ((stderr, "%*s%s <empty>\n", yyindent+2, "",
                         yysymbol_name (yy_accessing_symbol (yystates[yyi]->yylrState))));
          else
            YY_FPRINTF ((stderr, "%*s%s <tokens %ld .. %ld>\n", yyindent+2, "",
                         yysymbol_name (yy_accessing_symbol (yystates[yyi]->yylrState)),
                         YY_CAST (long, yystates[yyi-1]->yyposn + 1),
                         YY_CAST (long, yystates[yyi]->yyposn)));
        }
      else
        yyreportTree (yystates[yyi]->yysemantics.yyfirstVal, yyindent+2);
    }
}
#endif

static YYRESULTTAG
yyreportAmbiguity (yySemanticOption* yyx0,
                   yySemanticOption* yyx1, YYLTYPE *yylocp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  YYUSE (yyx0);
  YYUSE (yyx1);

#if YYDEBUG
  YY_FPRINTF ((stderr, "Ambiguity detected.\n"));
  YY_FPRINTF ((stderr, "Option 1,\n"));
  yyreportTree (yyx0, 2);
  YY_FPRINTF ((stderr, "\nOption 2,\n"));
  yyreportTree (yyx1, 2);
  YY_FPRINTF ((stderr, "\n"));
#endif

  yyerror (yylocp, driver, yyscanner, YY_("syntax is ambiguous"));
  return yyabort;
}

/** Resolve the locations for each of the YYN1 states in *YYSTACKP,
 *  ending at YYS1.  Has no effect on previously resolved states.
 *  The first semantic option of a state is always chosen.  */
static void
yyresolveLocations (yyGLRState *yys1, int yyn1,
                    yyGLRStack *yystackp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  if (0 < yyn1)
    {
      yyresolveLocations (yys1->yypred, yyn1 - 1, yystackp, driver, yyscanner);
      if (!yys1->yyresolved)
        {
          yyGLRStackItem yyrhsloc[1 + YYMAXRHS];
          int yynrhs;
          yySemanticOption *yyoption = yys1->yysemantics.yyfirstVal;
          YY_ASSERT (yyoption);
          yynrhs = yyrhsLength (yyoption->yyrule);
          if (0 < yynrhs)
            {
              yyGLRState *yys;
              int yyn;
              yyresolveLocations (yyoption->yystate, yynrhs,
                                  yystackp, driver, yyscanner);
              for (yys = yyoption->yystate, yyn = yynrhs;
                   yyn > 0;
                   yys = yys->yypred, yyn -= 1)
                yyrhsloc[yyn].yystate.yyloc = yys->yyloc;
            }
          else
            {
              /* Both yyresolveAction and yyresolveLocations traverse the GSS
                 in reverse rightmost order.  It is only necessary to invoke
                 yyresolveLocations on a subforest for which yyresolveAction
                 would have been invoked next had an ambiguity not been
                 detected.  Thus the location of the previous state (but not
                 necessarily the previous state itself) is guaranteed to be
                 resolved already.  */
              yyGLRState *yyprevious = yyoption->yystate;
              yyrhsloc[0].yystate.yyloc = yyprevious->yyloc;
            }
          YYLLOC_DEFAULT ((yys1->yyloc), yyrhsloc, yynrhs);
        }
    }
}

/** Resolve the ambiguity represented in state YYS in *YYSTACKP,
 *  perform the indicated actions, and set the semantic value of YYS.
 *  If result != yyok, the chain of semantic options in YYS has been
 *  cleared instead or it has been left unmodified except that
 *  redundant options may have been removed.  Regardless of whether
 *  result = yyok, YYS has been left with consistent data so that
 *  yydestroyGLRState can be invoked if necessary.  */
static YYRESULTTAG
yyresolveValue (yyGLRState* yys, yyGLRStack* yystackp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  yySemanticOption* yyoptionList = yys->yysemantics.yyfirstVal;
  yySemanticOption* yybest = yyoptionList;
  yySemanticOption** yypp;
  yybool yymerge = yyfalse;
  YYSTYPE yysval;
  YYRESULTTAG yyflag;
  YYLTYPE *yylocp = &yys->yyloc;

  for (yypp = &yyoptionList->yynext; *yypp != YY_NULLPTR; )
    {
      yySemanticOption* yyp = *yypp;

      if (yyidenticalOptions (yybest, yyp))
        {
          yymergeOptionSets (yybest, yyp);
          *yypp = yyp->yynext;
        }
      else
        {
          switch (yypreference (yybest, yyp))
            {
            case 0:
              yyresolveLocations (yys, 1, yystackp, driver, yyscanner);
              return yyreportAmbiguity (yybest, yyp, yylocp, driver, yyscanner);
              break;
            case 1:
              yymerge = yytrue;
              break;
            case 2:
              break;
            case 3:
              yybest = yyp;
              yymerge = yyfalse;
              break;
            default:
              /* This cannot happen so it is not worth a YY_ASSERT (yyfalse),
                 but some compilers complain if the default case is
                 omitted.  */
              break;
            }
          yypp = &yyp->yynext;
        }
    }

  if (yymerge)
    {
      yySemanticOption* yyp;
      int yyprec = yydprec[yybest->yyrule];
      yyflag = yyresolveAction (yybest, yystackp, &yysval, yylocp, driver, yyscanner);
      if (yyflag == yyok)
        for (yyp = yybest->yynext; yyp != YY_NULLPTR; yyp = yyp->yynext)
          {
            if (yyprec == yydprec[yyp->yyrule])
              {
                YYSTYPE yysval_other;
                YYLTYPE yydummy;
                yyflag = yyresolveAction (yyp, yystackp, &yysval_other, &yydummy, driver, yyscanner);
                if (yyflag != yyok)
                  {
                    yydestruct ("Cleanup: discarding incompletely merged value for",
                                yy_accessing_symbol (yys->yylrState),
                                &yysval, yylocp, driver, yyscanner);
                    break;
                  }
                yyuserMerge (yymerger[yyp->yyrule], &yysval, &yysval_other);
              }
          }
    }
  else
    yyflag = yyresolveAction (yybest, yystackp, &yysval, yylocp, driver, yyscanner);

  if (yyflag == yyok)
    {
      yys->yyresolved = yytrue;
      yys->yysemantics.yysval = yysval;
    }
  else
    yys->yysemantics.yyfirstVal = YY_NULLPTR;
  return yyflag;
}

static YYRESULTTAG
yyresolveStack (yyGLRStack* yystackp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  if (yystackp->yysplitPoint != YY_NULLPTR)
    {
      yyGLRState* yys;
      int yyn;

      for (yyn = 0, yys = yystackp->yytops.yystates[0];
           yys != yystackp->yysplitPoint;
           yys = yys->yypred, yyn += 1)
        continue;
      YYCHK (yyresolveStates (yystackp->yytops.yystates[0], yyn, yystackp
                             , driver, yyscanner));
    }
  return yyok;
}

static void
yycompressStack (yyGLRStack* yystackp)
{
  yyGLRState* yyp, *yyq, *yyr;

  if (yystackp->yytops.yysize != 1 || yystackp->yysplitPoint == YY_NULLPTR)
    return;

  for (yyp = yystackp->yytops.yystates[0], yyq = yyp->yypred, yyr = YY_NULLPTR;
       yyp != yystackp->yysplitPoint;
       yyr = yyp, yyp = yyq, yyq = yyp->yypred)
    yyp->yypred = yyr;

  yystackp->yyspaceLeft += yystackp->yynextFree - yystackp->yyitems;
  yystackp->yynextFree = YY_REINTERPRET_CAST (yyGLRStackItem*, yystackp->yysplitPoint) + 1;
  yystackp->yyspaceLeft -= yystackp->yynextFree - yystackp->yyitems;
  yystackp->yysplitPoint = YY_NULLPTR;
  yystackp->yylastDeleted = YY_NULLPTR;

  while (yyr != YY_NULLPTR)
    {
      yystackp->yynextFree->yystate = *yyr;
      yyr = yyr->yypred;
      yystackp->yynextFree->yystate.yypred = &yystackp->yynextFree[-1].yystate;
      yystackp->yytops.yystates[0] = &yystackp->yynextFree->yystate;
      yystackp->yynextFree += 1;
      yystackp->yyspaceLeft -= 1;
    }
}

static YYRESULTTAG
yyprocessOneStack (yyGLRStack* yystackp, YYPTRDIFF_T yyk,
                   YYPTRDIFF_T yyposn, YYLTYPE *yylocp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  while (yystackp->yytops.yystates[yyk] != YY_NULLPTR)
    {
      yy_state_t yystate = yystackp->yytops.yystates[yyk]->yylrState;
      YY_DPRINTF ((stderr, "Stack %ld Entering state %d\n",
                   YY_CAST (long, yyk), yystate));

      YY_ASSERT (yystate != YYFINAL);

      if (yyisDefaultedState (yystate))
        {
          YYRESULTTAG yyflag;
          yyRuleNum yyrule = yydefaultAction (yystate);
          if (yyrule == 0)
            {
              YY_DPRINTF ((stderr, "Stack %ld dies.\n", YY_CAST (long, yyk)));
              yymarkStackDeleted (yystackp, yyk);
              return yyok;
            }
          yyflag = yyglrReduce (yystackp, yyk, yyrule, yyimmediate[yyrule], driver, yyscanner);
          if (yyflag == yyerr)
            {
              YY_DPRINTF ((stderr,
                           "Stack %ld dies "
                           "(predicate failure or explicit user error).\n",
                           YY_CAST (long, yyk)));
              yymarkStackDeleted (yystackp, yyk);
              return yyok;
            }
          if (yyflag != yyok)
            return yyflag;
        }
      else
        {
          yysymbol_kind_t yytoken = yygetToken (&yychar, yystackp, driver, yyscanner);
          const short* yyconflicts;
          const int yyaction = yygetLRActions (yystate, yytoken, &yyconflicts);
          yystackp->yytops.yylookaheadNeeds[yyk] = yytrue;

          for (/* nothing */; *yyconflicts; yyconflicts += 1)
            {
              YYRESULTTAG yyflag;
              YYPTRDIFF_T yynewStack = yysplitStack (yystackp, yyk);
              YY_DPRINTF ((stderr, "Splitting off stack %ld from %ld.\n",
                           YY_CAST (long, yynewStack), YY_CAST (long, yyk)));
              yyflag = yyglrReduce (yystackp, yynewStack,
                                    *yyconflicts,
                                    yyimmediate[*yyconflicts], driver, yyscanner);
              if (yyflag == yyok)
                YYCHK (yyprocessOneStack (yystackp, yynewStack,
                                          yyposn, yylocp, driver, yyscanner));
              else if (yyflag == yyerr)
                {
                  YY_DPRINTF ((stderr, "Stack %ld dies.\n", YY_CAST (long, yynewStack)));
                  yymarkStackDeleted (yystackp, yynewStack);
                }
              else
                return yyflag;
            }

          if (yyisShiftAction (yyaction))
            break;
          else if (yyisErrorAction (yyaction))
            {
              YY_DPRINTF ((stderr, "Stack %ld dies.\n", YY_CAST (long, yyk)));
              yymarkStackDeleted (yystackp, yyk);
              break;
            }
          else
            {
              YYRESULTTAG yyflag = yyglrReduce (yystackp, yyk, -yyaction,
                                                yyimmediate[-yyaction], driver, yyscanner);
              if (yyflag == yyerr)
                {
                  YY_DPRINTF ((stderr,
                               "Stack %ld dies "
                               "(predicate failure or explicit user error).\n",
                               YY_CAST (long, yyk)));
                  yymarkStackDeleted (yystackp, yyk);
                  break;
                }
              else if (yyflag != yyok)
                return yyflag;
            }
        }
    }
  return yyok;
}

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYSTACKP, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  */
static int
yypcontext_expected_tokens (const yyGLRStack* yystackp,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[yystackp->yytops.yystates[0]->yylrState];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}

static int
yy_syntax_error_arguments (const yyGLRStack* yystackp,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  yysymbol_kind_t yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.b4_lac_if([
       In the first two cases, it might appear that the current syntax
       error should have been detected in the previous state when yy_lac
       was invoked.  However, at that time, there might have been a
       different syntax error that discarded a different initial context
       during error recovery, leaving behind the current lookahead.], [
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.])
  */
  if (yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yystackp,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}



static void
yyreportSyntaxError (yyGLRStack* yystackp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  if (yystackp->yyerrState != 0)
    return;
  {
  yybool yysize_overflow = yyfalse;
  char* yymsg = YY_NULLPTR;
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount
    = yy_syntax_error_arguments (yystackp, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    yyMemoryExhausted (yystackp);

  switch (yycount)
    {
#define YYCASE_(N, S)                   \
      case N:                           \
        yyformat = S;                   \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysz
          = yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (YYSIZE_MAXIMUM - yysize < yysz)
          yysize_overflow = yytrue;
        else
          yysize += yysz;
      }
  }

  if (!yysize_overflow)
    yymsg = YY_CAST (char *, YYMALLOC (YY_CAST (YYSIZE_T, yysize)));

  if (yymsg)
    {
      char *yyp = yymsg;
      int yyi = 0;
      while ((*yyp = *yyformat))
        {
          if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
            {
              yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
              yyformat += 2;
            }
          else
            {
              ++yyp;
              ++yyformat;
            }
        }
      yyerror (&yylloc, driver, yyscanner, yymsg);
      YYFREE (yymsg);
    }
  else
    {
      yyerror (&yylloc, driver, yyscanner, YY_("syntax error"));
      yyMemoryExhausted (yystackp);
    }
  }
  yynerrs += 1;
}

/* Recover from a syntax error on *YYSTACKP, assuming that *YYSTACKP->YYTOKENP,
   yylval, and yylloc are the syntactic category, semantic value, and location
   of the lookahead.  */
static void
yyrecoverSyntaxError (yyGLRStack* yystackp, PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  if (yystackp->yyerrState == 3)
    /* We just shifted the error token and (perhaps) took some
       reductions.  Skip tokens until we can proceed.  */
    while (yytrue)
      {
        yysymbol_kind_t yytoken;
        int yyj;
        if (yychar == END)
          yyFail (yystackp, &yylloc, driver, yyscanner, YY_NULLPTR);
        if (yychar != YYEMPTY)
          {
            /* We throw away the lookahead, but the error range
               of the shifted error token must take it into account.  */
            yyGLRState *yys = yystackp->yytops.yystates[0];
            yyGLRStackItem yyerror_range[3];
            yyerror_range[1].yystate.yyloc = yys->yyloc;
            yyerror_range[2].yystate.yyloc = yylloc;
            YYLLOC_DEFAULT ((yys->yyloc), yyerror_range, 2);
            yytoken = YYTRANSLATE (yychar);
            yydestruct ("Error: discarding",
                        yytoken, &yylval, &yylloc, driver, yyscanner);
            yychar = YYEMPTY;
          }
        yytoken = yygetToken (&yychar, yystackp, driver, yyscanner);
        yyj = yypact[yystackp->yytops.yystates[0]->yylrState];
        if (yypact_value_is_default (yyj))
          return;
        yyj += yytoken;
        if (yyj < 0 || YYLAST < yyj || yycheck[yyj] != yytoken)
          {
            if (yydefact[yystackp->yytops.yystates[0]->yylrState] != 0)
              return;
          }
        else if (! yytable_value_is_error (yytable[yyj]))
          return;
      }

  /* Reduce to one stack.  */
  {
    YYPTRDIFF_T yyk;
    for (yyk = 0; yyk < yystackp->yytops.yysize; yyk += 1)
      if (yystackp->yytops.yystates[yyk] != YY_NULLPTR)
        break;
    if (yyk >= yystackp->yytops.yysize)
      yyFail (yystackp, &yylloc, driver, yyscanner, YY_NULLPTR);
    for (yyk += 1; yyk < yystackp->yytops.yysize; yyk += 1)
      yymarkStackDeleted (yystackp, yyk);
    yyremoveDeletes (yystackp);
    yycompressStack (yystackp);
  }

  /* Pop stack until we find a state that shifts the error token.  */
  yystackp->yyerrState = 3;
  while (yystackp->yytops.yystates[0] != YY_NULLPTR)
    {
      yyGLRState *yys = yystackp->yytops.yystates[0];
      int yyj = yypact[yys->yylrState];
      if (! yypact_value_is_default (yyj))
        {
          yyj += YYSYMBOL_YYerror;
          if (0 <= yyj && yyj <= YYLAST && yycheck[yyj] == YYSYMBOL_YYerror
              && yyisShiftAction (yytable[yyj]))
            {
              /* Shift the error token.  */
              int yyaction = yytable[yyj];
              /* First adjust its location.*/
              YYLTYPE yyerrloc;
              yystackp->yyerror_range[2].yystate.yyloc = yylloc;
              YYLLOC_DEFAULT (yyerrloc, (yystackp->yyerror_range), 2);
              YY_SYMBOL_PRINT ("Shifting", yy_accessing_symbol (yyaction),
                               &yylval, &yyerrloc);
              yyglrShift (yystackp, 0, yyaction,
                          yys->yyposn, &yylval, &yyerrloc);
              yys = yystackp->yytops.yystates[0];
              break;
            }
        }
      yystackp->yyerror_range[1].yystate.yyloc = yys->yyloc;
      if (yys->yypred != YY_NULLPTR)
        yydestroyGLRState ("Error: popping", yys, driver, yyscanner);
      yystackp->yytops.yystates[0] = yys->yypred;
      yystackp->yynextFree -= 1;
      yystackp->yyspaceLeft += 1;
    }
  if (yystackp->yytops.yystates[0] == YY_NULLPTR)
    yyFail (yystackp, &yylloc, driver, yyscanner, YY_NULLPTR);
}

#define YYCHK1(YYE)                                                          \
  do {                                                                       \
    switch (YYE) {                                                           \
    case yyok:                                                               \
      break;                                                                 \
    case yyabort:                                                            \
      goto yyabortlab;                                                       \
    case yyaccept:                                                           \
      goto yyacceptlab;                                                      \
    case yyerr:                                                              \
      goto yyuser_error;                                                     \
    default:                                                                 \
      goto yybuglab;                                                         \
    }                                                                        \
  } while (0)

/*----------.
| yyparse.  |
`----------*/

int
yyparse (PCP_ParserDriver* driver, yyscan_t yyscanner)
{
  int yyresult;
  yyGLRStack yystack;
  yyGLRStack* const yystackp = &yystack;
  YYPTRDIFF_T yyposn;

  YY_DPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY;
  yylval = yyval_default;
  yylloc = yyloc_default;

  /* User initialization code.  */
{
  // initialize the location
  //@$.initialize (YY_NULLPTR, 1, 1);
  //@$.begin.filename = @$.end.filename = &driver->file;
  ACE_OS::memset (&yylloc, 0, sizeof (YYLTYPE));

  // initialize the token value container
  yylval.ival = 0;
}



  if (! yyinitGLRStack (yystackp, YYINITDEPTH))
    goto yyexhaustedlab;
  switch (YYSETJMP (yystack.yyexception_buffer))
    {
    case 0: break;
    case 1: goto yyabortlab;
    case 2: goto yyexhaustedlab;
    default: goto yybuglab;
    }
  yyglrShift (&yystack, 0, 0, 0, &yylval, &yylloc);
  yyposn = 0;

  while (yytrue)
    {
      /* For efficiency, we have two loops, the first of which is
         specialized to deterministic operation (single stack, no
         potential ambiguity).  */
      /* Standard mode. */
      while (yytrue)
        {
          yy_state_t yystate = yystack.yytops.yystates[0]->yylrState;
          YY_DPRINTF ((stderr, "Entering state %d\n", yystate));
          if (yystate == YYFINAL)
            goto yyacceptlab;
          if (yyisDefaultedState (yystate))
            {
              yyRuleNum yyrule = yydefaultAction (yystate);
              if (yyrule == 0)
                {
                  yystack.yyerror_range[1].yystate.yyloc = yylloc;
                  yyreportSyntaxError (&yystack, driver, yyscanner);
                  goto yyuser_error;
                }
              YYCHK1 (yyglrReduce (&yystack, 0, yyrule, yytrue, driver, yyscanner));
            }
          else
            {
              yysymbol_kind_t yytoken = yygetToken (&yychar, yystackp, driver, yyscanner);
              const short* yyconflicts;
              int yyaction = yygetLRActions (yystate, yytoken, &yyconflicts);
              if (*yyconflicts)
                /* Enter nondeterministic mode.  */
                break;
              if (yyisShiftAction (yyaction))
                {
                  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
                  yychar = YYEMPTY;
                  yyposn += 1;
                  yyglrShift (&yystack, 0, yyaction, yyposn, &yylval, &yylloc);
                  if (0 < yystack.yyerrState)
                    yystack.yyerrState -= 1;
                }
              else if (yyisErrorAction (yyaction))
                {
                  yystack.yyerror_range[1].yystate.yyloc = yylloc;
                  /* Issue an error message unless the scanner already
                     did. */
                  if (yychar != YYerror)
                    yyreportSyntaxError (&yystack, driver, yyscanner);
                  goto yyuser_error;
                }
              else
                YYCHK1 (yyglrReduce (&yystack, 0, -yyaction, yytrue, driver, yyscanner));
            }
        }

      /* Nondeterministic mode. */
      while (yytrue)
        {
          yysymbol_kind_t yytoken_to_shift;
          YYPTRDIFF_T yys;

          for (yys = 0; yys < yystack.yytops.yysize; yys += 1)
            yystackp->yytops.yylookaheadNeeds[yys] = yychar != YYEMPTY;

          /* yyprocessOneStack returns one of three things:

              - An error flag.  If the caller is yyprocessOneStack, it
                immediately returns as well.  When the caller is finally
                yyparse, it jumps to an error label via YYCHK1.

              - yyok, but yyprocessOneStack has invoked yymarkStackDeleted
                (&yystack, yys), which sets the top state of yys to NULL.  Thus,
                yyparse's following invocation of yyremoveDeletes will remove
                the stack.

              - yyok, when ready to shift a token.

             Except in the first case, yyparse will invoke yyremoveDeletes and
             then shift the next token onto all remaining stacks.  This
             synchronization of the shift (that is, after all preceding
             reductions on all stacks) helps prevent double destructor calls
             on yylval in the event of memory exhaustion.  */

          for (yys = 0; yys < yystack.yytops.yysize; yys += 1)
            YYCHK1 (yyprocessOneStack (&yystack, yys, yyposn, &yylloc, driver, yyscanner));
          yyremoveDeletes (&yystack);
          if (yystack.yytops.yysize == 0)
            {
              yyundeleteLastStack (&yystack);
              if (yystack.yytops.yysize == 0)
                yyFail (&yystack, &yylloc, driver, yyscanner, YY_("syntax error"));
              YYCHK1 (yyresolveStack (&yystack, driver, yyscanner));
              YY_DPRINTF ((stderr, "Returning to deterministic operation.\n"));
              yystack.yyerror_range[1].yystate.yyloc = yylloc;
              yyreportSyntaxError (&yystack, driver, yyscanner);
              goto yyuser_error;
            }

          /* If any yyglrShift call fails, it will fail after shifting.  Thus,
             a copy of yylval will already be on stack 0 in the event of a
             failure in the following loop.  Thus, yychar is set to YYEMPTY
             before the loop to make sure the user destructor for yylval isn't
             called twice.  */
          yytoken_to_shift = YYTRANSLATE (yychar);
          yychar = YYEMPTY;
          yyposn += 1;
          for (yys = 0; yys < yystack.yytops.yysize; yys += 1)
            {
              yy_state_t yystate = yystack.yytops.yystates[yys]->yylrState;
              const short* yyconflicts;
              int yyaction = yygetLRActions (yystate, yytoken_to_shift,
                              &yyconflicts);
              /* Note that yyconflicts were handled by yyprocessOneStack.  */
              YY_DPRINTF ((stderr, "On stack %ld, ", YY_CAST (long, yys)));
              YY_SYMBOL_PRINT ("shifting", yytoken_to_shift, &yylval, &yylloc);
              yyglrShift (&yystack, yys, yyaction, yyposn,
                          &yylval, &yylloc);
              YY_DPRINTF ((stderr, "Stack %ld now in state #%d\n",
                           YY_CAST (long, yys),
                           yystack.yytops.yystates[yys]->yylrState));
            }

          if (yystack.yytops.yysize == 1)
            {
              YYCHK1 (yyresolveStack (&yystack, driver, yyscanner));
              YY_DPRINTF ((stderr, "Returning to deterministic operation.\n"));
              yycompressStack (&yystack);
              break;
            }
        }
      continue;
    yyuser_error:
      yyrecoverSyntaxError (&yystack, driver, yyscanner);
      yyposn = yystack.yytops.yystates[0]->yyposn;
    }

 yyacceptlab:
  yyresult = 0;
  goto yyreturn;

 yybuglab:
  YY_ASSERT (yyfalse);
  goto yyabortlab;

 yyabortlab:
  yyresult = 1;
  goto yyreturn;

 yyexhaustedlab:
  yyerror (&yylloc, driver, yyscanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;

 yyreturn:
  if (yychar != YYEMPTY)
    yydestruct ("Cleanup: discarding lookahead",
                YYTRANSLATE (yychar), &yylval, &yylloc, driver, yyscanner);

  /* If the stack is well-formed, pop the stack until it is empty,
     destroying its entries as we go.  But free the stack regardless
     of whether it is well-formed.  */
  if (yystack.yyitems)
    {
      yyGLRState** yystates = yystack.yytops.yystates;
      if (yystates)
        {
          YYPTRDIFF_T yysize = yystack.yytops.yysize;
          YYPTRDIFF_T yyk;
          for (yyk = 0; yyk < yysize; yyk += 1)
            if (yystates[yyk])
              {
                while (yystates[yyk])
                  {
                    yyGLRState *yys = yystates[yyk];
                    yystack.yyerror_range[1].yystate.yyloc = yys->yyloc;
                    if (yys->yypred != YY_NULLPTR)
                      yydestroyGLRState ("Cleanup: popping", yys, driver, yyscanner);
                    yystates[yyk] = yys->yypred;
                    yystack.yynextFree -= 1;
                    yystack.yyspaceLeft += 1;
                  }
                break;
              }
        }
      yyfreeGLRStack (&yystack);
    }

  return yyresult;
}

/* DEBUGGING ONLY */
#if YYDEBUG
static void
yy_yypstack (yyGLRState* yys)
{
  if (yys->yypred)
    {
      yy_yypstack (yys->yypred);
      YY_FPRINTF ((stderr, " -> "));
    }
  YY_FPRINTF ((stderr, "%d@%ld", yys->yylrState, YY_CAST (long, yys->yyposn)));
}

static void
yypstates (yyGLRState* yyst)
{
  if (yyst == YY_NULLPTR)
    YY_FPRINTF ((stderr, "<null>"));
  else
    yy_yypstack (yyst);
  YY_FPRINTF ((stderr, "\n"));
}

static void
yypstack (yyGLRStack* yystackp, YYPTRDIFF_T yyk)
{
  yypstates (yystackp->yytops.yystates[yyk]);
}

static void
yypdumpstack (yyGLRStack* yystackp)
{
#define YYINDEX(YYX)                                                    \
  YY_CAST (long,                                                        \
           ((YYX)                                                       \
            ? YY_REINTERPRET_CAST (yyGLRStackItem*, (YYX)) - yystackp->yyitems \
            : -1))

  yyGLRStackItem* yyp;
  for (yyp = yystackp->yyitems; yyp < yystackp->yynextFree; yyp += 1)
    {
      YY_FPRINTF ((stderr, "%3ld. ",
                   YY_CAST (long, yyp - yystackp->yyitems)));
      if (*YY_REINTERPRET_CAST (yybool *, yyp))
        {
          YY_ASSERT (yyp->yystate.yyisState);
          YY_ASSERT (yyp->yyoption.yyisState);
          YY_FPRINTF ((stderr, "Res: %d, LR State: %d, posn: %ld, pred: %ld",
                       yyp->yystate.yyresolved, yyp->yystate.yylrState,
                       YY_CAST (long, yyp->yystate.yyposn),
                       YYINDEX (yyp->yystate.yypred)));
          if (! yyp->yystate.yyresolved)
            YY_FPRINTF ((stderr, ", firstVal: %ld",
                         YYINDEX (yyp->yystate.yysemantics.yyfirstVal)));
        }
      else
        {
          YY_ASSERT (!yyp->yystate.yyisState);
          YY_ASSERT (!yyp->yyoption.yyisState);
          YY_FPRINTF ((stderr, "Option. rule: %d, state: %ld, next: %ld",
                       yyp->yyoption.yyrule - 1,
                       YYINDEX (yyp->yyoption.yystate),
                       YYINDEX (yyp->yyoption.yynext)));
        }
      YY_FPRINTF ((stderr, "\n"));
    }

  YY_FPRINTF ((stderr, "Tops:"));
  {
    YYPTRDIFF_T yyi;
    for (yyi = 0; yyi < yystackp->yytops.yysize; yyi += 1)
      YY_FPRINTF ((stderr, "%ld: %ld; ", YY_CAST (long, yyi),
                   YYINDEX (yystackp->yytops.yystates[yyi])));
    YY_FPRINTF ((stderr, "\n"));
  }
#undef YYINDEX
}
#endif

#undef yylval
#undef yychar
#undef yynerrs
#undef yylloc






/* void
yy::PCP_Parser::error (const location_type& location_in,
                       const std::string& message_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Parser::error"));

  driver->error (location_in, message_in);
}

void
yy::PCP_Parser::set (yyscan_t context_in)
{
  NETWORK_TRACE (ACE_TEXT ("PCP_Parser::set"));

  yyscanner = context_in;
} */

void
yyerror (YYLTYPE* location_in,
         PCP_ParserDriver* driver_in,
         yyscan_t context_in,
         const char* message_in)
{
  NETWORK_TRACE (ACE_TEXT ("::yyerror"));

  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (location_in);
  ACE_ASSERT (driver_in);

  driver_in->error (*location_in, std::string (message_in));
}

//void
//yyprint (FILE* file_in,
//         yytokentype type_in,
//         YYSTYPE value_in)
//{
//  NETWORK_TRACE (ACE_TEXT ("::yyprint"));
//
//  int result = -1;
//
//  std::string format_string;
//  switch (type_in)
//  {
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown token type (was: %d), returning\n"),
//                  type_in));
//      return;
//    }
//  } // end SWITCH
//}
