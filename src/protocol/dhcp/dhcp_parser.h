/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2015 Free Software Foundation, Inc.

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

#ifndef YY_YY_DHCP_PARSER_H_INCLUDED
# define YY_YY_DHCP_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */


#include <cstdio>
#include <string>

//enum yytokentype
//{
//  END = 0,
//  OP = 258,
//  HTYPE = 259,
//  HLEN = 260,
//  HOPS = 261,
//  XID = 262,
//  SECS = 263,
//  FLAGS = 264,
//  CIADDR = 265,
//  YIADDR = 266,
//  SIADDR = 267,
//  GIADDR = 268,
//  CHADDR = 269,
//  SNAME = 270,
//  FILE_ = 271,
//  COOKIE = 272,
//  OPTION_KEY = 273,
//  OPTION_VALUE = 274
//};
//#define YYTOKENTYPE
class DHCP_ParserDriver;
//class DHCP_Scanner;
struct YYLTYPE;
//union YYSTYPE;

union yytoken
{
  unsigned char aval[16];
  int           ival;
  std::string*  sval;
};
typedef yytoken yytoken_t;
#define YYSTYPE yytoken_t

typedef void* yyscan_t;

//#define YYERROR_VERBOSE
extern void yyerror (YYLTYPE*, DHCP_ParserDriver*, yyscan_t, const char*);
extern int yyparse (DHCP_ParserDriver*, yyscan_t);

#undef YYPRINT
//extern void yyprint (FILE*, yytokentype, YYSTYPE);



/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 0,
    OP = 258,
    HTYPE = 259,
    HLEN = 260,
    HOPS = 261,
    XID = 262,
    SECS = 263,
    FLAGS = 264,
    CIADDR = 265,
    YIADDR = 266,
    SIADDR = 267,
    GIADDR = 268,
    CHADDR = 269,
    SNAME = 270,
    FILE_ = 271,
    COOKIE = 272,
    OPTION_TAG = 273,
    OPTION_VALUE = 274
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


  unsigned char aval[16];
  int           ival;
  std::string*  sval;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (DHCP_ParserDriver* driver, yyscan_t yyscanner);

#endif /* !YY_YY_DHCP_PARSER_H_INCLUDED  */
