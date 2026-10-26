/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

#ifndef YY_ZZ_WS_PARSER_H_INCLUDED
# define YY_ZZ_WS_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef ZZDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define ZZDEBUG 1
#  else
#   define ZZDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ZZDEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ZZDEBUG */
#if ZZDEBUG
extern int zzdebug;
#endif
/* "%code requires" blocks.  */

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#ifndef WEBSOCKET_PARSER_H
//#define WEBSOCKET_PARSER_H

#include <cstdio>
#include <string>

#include "common_iparser.h"

/* enum yytokentype
{
  END = 0,
  FRAME = 265,
}; */
//#define YYTOKENTYPE
/*#undef YYTOKENTYPE*/
/* enum yytokentype; */
class WebSocket_IParser;
struct WebSocket_Record;
//class WebSocket_Scanner;
//struct YYLTYPE;

/* #define YYSTYPE
typedef union YYSTYPE
{
  ACE_INT64 ival;
} YYSTYPE; */
#undef ZZSTYPE
//union ZZSTYPE;

typedef void* yyscan_t;

// *NOTE*: on current versions of bison, this needs to be inserted into the
//         header manually, as there is no way to add the export symbol to
//         the declaration
#define ZZDEBUG 1
extern int zzdebug;
#define ZZERROR_VERBOSE 1
//#define YYLTYPE_IS_DECLARED 1

//#undef YYTOKENTYPE


/* Token kinds.  */
#ifndef ZZTOKENTYPE
# define ZZTOKENTYPE
  enum zztokentype
  {
    ZZEMPTY = -2,
    END_ = 0,                      /* "end"  */
    ZZerror = 256,                 /* error  */
    ZZUNDEF = 257,                 /* "invalid token"  */
    FRAME = 258                    /* "frame"  */
  };
  typedef enum zztokentype zztoken_kind_t;
#endif

/* Value type.  */
#if ! defined ZZSTYPE && ! defined ZZSTYPE_IS_DECLARED
union ZZSTYPE
{

  ACE_INT64 ival;


};
typedef union ZZSTYPE ZZSTYPE;
# define ZZSTYPE_IS_TRIVIAL 1
# define ZZSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined ZZLTYPE && ! defined ZZLTYPE_IS_DECLARED
typedef struct ZZLTYPE ZZLTYPE;
struct ZZLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define ZZLTYPE_IS_DECLARED 1
# define ZZLTYPE_IS_TRIVIAL 1
#endif



int zzparse (WebSocket_IParser* iparser_p, yyscan_t yyscanner);
/* "%code provides" blocks.  */

/*void yysetdebug (int);*/
void zzerror (ZZLTYPE*, WebSocket_IParser*, yyscan_t, const char*);
//void yyerror (WebSocket_IParser*, yyscan_t, const char*);
/*int yyparse (WebSocket_IParser*, yyscan_t);*/
/*void yyprint (FILE*, enum yytokentype, YYSTYPE);*/

// *NOTE*: add double include protection, required for GNU Bison 2.4.2
// *TODO*: remove this ASAP
//#endif // WebSocket_PARSER_H


#endif /* !YY_ZZ_WS_PARSER_H_INCLUDED  */
