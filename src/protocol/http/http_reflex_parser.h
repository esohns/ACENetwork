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

#ifndef YY_HTTP_HTTP_REFLEX_PARSER_H_INCLUDED
# define YY_HTTP_HTTP_REFLEX_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef HTTP_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define HTTP_DEBUG 1
#  else
#   define HTTP_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define HTTP_DEBUG 1
# endif /* ! defined YYDEBUG */
#endif  /* ! defined HTTP_DEBUG */
#if HTTP_DEBUG
extern int http_debug;
#endif
/* "%code requires" blocks.  */

#include <cstdio>
#include <string>

#include "ace/Basic_Types.h"

#include "common_iparser.h"

#include "http_reflex_iparser.h"

// forward declarations
struct HTTP_Record;
typedef void* yyscan_t;

#undef YYTOKENTYPE


/* Token kinds.  */
#ifndef HTTP_TOKENTYPE
# define HTTP_TOKENTYPE
  enum http_tokentype
  {
    HTTP_EMPTY = -2,
    END = 0,                       /* "end"  */
    HTTP_error = 256,              /* error  */
    HTTP_UNDEF = 257,              /* "invalid token"  */
    METHOD = 258,                  /* "method"  */
    URI = 259,                     /* "uri"  */
    _VERSION = 260,                /* "version"  */
    HEADER = 261,                  /* "header"  */
    DELIMITER = 262,               /* "delimiter"  */
    STATUS = 263,                  /* "status"  */
    REASON = 264,                  /* "reason"  */
    BODY = 265,                    /* "body"  */
    CHUNK = 266,                   /* "chunk"  */
    END_OF_FRAGMENT = 267          /* "end_of_fragment"  */
  };
  typedef enum http_tokentype http_token_kind_t;
#endif

/* Value type.  */
#if ! defined HTTP_STYPE && ! defined HTTP_STYPE_IS_DECLARED
union HTTP_STYPE
{

  ACE_INT64    ival;
  std::string* sval;


};
typedef union HTTP_STYPE HTTP_STYPE;
# define HTTP_STYPE_IS_TRIVIAL 1
# define HTTP_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined HTTP_LTYPE && ! defined HTTP_LTYPE_IS_DECLARED
typedef struct HTTP_LTYPE HTTP_LTYPE;
struct HTTP_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define HTTP_LTYPE_IS_DECLARED 1
# define HTTP_LTYPE_IS_TRIVIAL 1
#endif



int http_parse (HTTP_Reflex_IParser* iparser_p, yyscan_t scanner);
/* "%code provides" blocks.  */

#define YYSTYPE HTTP_STYPE
#define YYLTYPE HTTP_LTYPE
void http_error (YYLTYPE*, HTTP_Reflex_IParser*, yyscan_t, const char*);


#endif /* !YY_HTTP_HTTP_REFLEX_PARSER_H_INCLUDED  */
