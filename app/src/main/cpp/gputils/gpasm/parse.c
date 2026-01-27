/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30706

/* Bison version string.  */
#define YYBISON_VERSION "3.7.6"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parse.y"

/* Parser for gpasm
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005
   James Bowman, Craig Franklin

This file is part of gputils.

gputils is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

gputils is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with gputils; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "stdhdr.h"

#include "libgputils.h"
#include "gpasm.h"
#include "evaluate.h"
#include "gpmsg.h"
#include "directive.h"
#include "lst.h"
#include "macro.h"
#include "coff.h"
#include "scan.h"

/* #define DEBUG */

#ifdef DEBUG
/* enable debugging */
#include "parse.h"

int yydebug = 1;

#define YYPRINT(file, type, value)   yyprint(file, type, value)

static void
yyprint(FILE *File, int Type, YYSTYPE Value)
{
  switch (Type) {
    case LABEL:
    case IDENTIFIER:
    case IDENT_BRACKET:
    case DEBUG_LINE:
    case ERRORLEVEL:
    case FILL:
    case LIST:
    case PROCESSOR:
    case DEFINE:
      fprintf(File, "%s", Value.s);
      break;

    case STRING:
      fprintf(File, "\"%s\"", Value.s);
      break;

    case NUMBER:
    case UPPER:
    case HIGH:
    case LOW:
    case LSH:
    case RSH:
    case GREATER_EQUAL:
    case LESS_EQUAL:
    case EQUAL:
    case NOT_EQUAL:
    case '<':
    case '>':
    case '&':
    case '|':
    case '^':
    case LOGICAL_AND:
    case LOGICAL_OR:
    case '=':
    case ASSIGN_PLUS:
    case ASSIGN_MINUS:
    case ASSIGN_MULTIPLY:
    case ASSIGN_DIVIDE:
    case ASSIGN_MODULUS:
    case ASSIGN_LSH:
    case ASSIGN_RSH:
    case ASSIGN_AND:
    case ASSIGN_OR:
    case ASSIGN_XOR:
    case INCREMENT:
    case DECREMENT:
    case POSTINCREMENT:
    case POSTDECREMENT:
    case INDFOFFSET:
    case TBL_NO_CHANGE:
    case TBL_POST_INC:
    case TBL_POST_DEC:
    case TBL_PRE_INC:
    case '[':
    case ']':
      fprintf(File, "%d", Value.i);
      break;

    case CBLOCK:
    case ENDC:
    default:
      break;
  }
}
#endif

int yylex(void);

/************************************************************************/

/* Some simple functions for building parse trees */

static pnode_t *
mk_pnode(enum pnode_tag Tag)
{
  pnode_t *new = (pnode_t *)GP_Calloc(1, sizeof(pnode_t));

  new->tag = Tag;
  return new;
}

pnode_t *
mk_constant(int Value)
{
  pnode_t *new = mk_pnode(PTAG_CONSTANT);

  PnConstant(new) = Value;
  return new;
}

pnode_t *
mk_offset(pnode_t *Pnode)
{
  pnode_t *new = mk_pnode(PTAG_OFFSET);

  PnOffset(new) = Pnode;
  return new;
}

pnode_t *
mk_symbol(const char *String)
{
  pnode_t *new = mk_pnode(PTAG_SYMBOL);

  PnSymbol(new) = String;
  return new;
}

pnode_t *
mk_string(char *String)
{
  pnode_t *new = mk_pnode(PTAG_STRING);

  PnString(new) = String;
  return new;
}

pnode_t *
mk_list(pnode_t *Head, pnode_t *Tail)
{
  pnode_t *new = mk_pnode(PTAG_LIST);

  PnListHead(new) = Head;
  PnListTail(new) = Tail;
  return new;
}

pnode_t *
mk_2op(int Op, pnode_t *Pnode0, pnode_t *Pnode1)
{
  pnode_t *new = mk_pnode(PTAG_BINOP);

  PnBinOpOp(new) = Op;
  PnBinOpP0(new) = Pnode0;
  PnBinOpP1(new) = Pnode1;
  return new;
}

pnode_t *
mk_1op(int Op, pnode_t *Pnode)
{
  pnode_t *new = mk_pnode(PTAG_UNOP);

  PnUnOpOp(new) = Op;
  PnUnOpP0(new) = Pnode;
  return new;
}

/************************************************************************/
/* shared functions */

gpasmVal
set_label(const char *Label, pnode_t *Parms)
{
  gpasmVal value = 0;

  if (asm_enabled()) {
    if ((state.mode == MODE_RELOCATABLE) && !(IN_MACRO_WHILE_DEFINITION) &&
        !(SECTION_FLAGS & (STYP_TEXT | STYP_RAM_AREA | STYP_BPACK)))
      gpmsg_verror(GPE_LABEL_IN_SECTION, NULL);

    value = do_or_append_insn("set", Parms);
    if (!(IN_MACRO_WHILE_DEFINITION)) {
      set_global(Label, value, VAL_VARIABLE, false, false);
    }
  }

  return value;
}

int return_op(int Operation);

void
next_line(int Value)
{
  source_context_t *ctx;

  ctx = state.src_list.last;

  if ((state.pass == 2) && (state.lst.line.linetype == LTY_DOLIST_DIR)) {
    state.lst.line.linetype = LTY_NONE;
    lst_format_line(ctx->curr_src_line.line, Value);
  }

  if ((IN_WHILE_EXPANSION) || (IN_MACRO_EXPANSION)) {
    if (!(IN_WHILE_DEFINITION) && state.lst.expand &&
        (state.pass == 2) &&
        (state.lst.line.linetype != LTY_DOLIST_DIR) &&
        (state.lst.line.linetype != LTY_NOLIST_DIR)) {

      if (ctx->curr_src_line.line != NULL) {
        /* Empty macro. */
        lst_format_line(ctx->curr_src_line.line, Value);
      }
      preproc_emit();
    }

    /* While loops can be defined inside a macro or nested. */
    if (IN_MACRO_WHILE_DEFINITION) {
      state.lst.line.linetype = LTY_NONE;

      if (state.mac_body != NULL) {
        /* Empty macro. */
        state.mac_body->src_line = GP_Strdup(ctx->mac_body->src_line);
      }
    }

    if (ctx->mac_body != NULL) {
      ctx->mac_body = ctx->mac_body->next;
    }
  }
  else if (IN_FILE_EXPANSION) {
    if (!(IN_WHILE_DEFINITION) && (state.pass == 2) &&
        (state.lst.line.linetype != LTY_DOLIST_DIR) &&
        (state.lst.line.linetype != LTY_NOLIST_DIR)) {
      lst_format_line(ctx->curr_src_line.line, Value);

      if (!(IN_MACRO_WHILE_DEFINITION)) {
        preproc_emit();
      }
    }

    if (IN_MACRO_WHILE_DEFINITION) {
      state.lst.line.linetype = LTY_NONE;

      if (state.mac_body != NULL) {
        state.mac_body->src_line = GP_Strdup(ctx->curr_src_line.line);
      }
    }

   if (state.next_state == STATE_INCLUDE) {
      /* Includes have to be evaluated here and not in the following
         switch statements so that the errors are reported correctly. */
      ++(ctx->line_number);
      open_src(state.next_buffer.file, true);
      free(state.next_buffer.file);
    }
  }

  ctx = state.src_list.last;
  switch (state.next_state) {
    case STATE_EXITMACRO:
      ++(ctx->line_number);
      execute_exitm();
      break;

    case STATE_MACRO:
      ++(ctx->line_number);
      /* push the label for local directive */
      state.stTop = macro_push_symbol_table(state.stTop);
      execute_macro(state.next_buffer.macro, false);
      break;

    case STATE_SECTION:
      ++(ctx->line_number);
      /* create a new coff section */
      coff_new_section(state.obj.new_sect_name, state.obj.new_sect_addr, state.obj.new_sect_flags);
      break;

    case STATE_WHILE:
      ++(ctx->line_number);
      execute_macro(state.next_buffer.macro, true);
      break;

    case STATE_INCLUDE:
      /* already evaluated */
      break;

    default:
      ++(ctx->line_number);
      break;
  }
}

void
yyerror(const char *Message)
{
  if (!(IN_MACRO_WHILE_DEFINITION)) {
    /* throw error if not in macro definition */
    gpmsg_verror(GPE_PARSER, NULL, Message);
  }
}

/************************************************************************/


#line 406 "parse.c"

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

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_PARSE_H_INCLUDED
# define YY_YY_PARSE_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    LABEL = 258,                   /* LABEL  */
    IDENTIFIER = 259,              /* IDENTIFIER  */
    IDENT_BRACKET = 260,           /* IDENT_BRACKET  */
    CBLOCK = 261,                  /* CBLOCK  */
    DEBUG_LINE = 262,              /* DEBUG_LINE  */
    ENDC = 263,                    /* ENDC  */
    ERRORLEVEL = 264,              /* ERRORLEVEL  */
    FILL = 265,                    /* FILL  */
    LIST = 266,                    /* LIST  */
    NUMBER = 267,                  /* NUMBER  */
    PROCESSOR = 268,               /* PROCESSOR  */
    STRING = 269,                  /* STRING  */
    DEFINE = 270,                  /* DEFINE  */
    UPPER = 271,                   /* UPPER  */
    HIGH = 272,                    /* HIGH  */
    LOW = 273,                     /* LOW  */
    LSH = 274,                     /* LSH  */
    RSH = 275,                     /* RSH  */
    GREATER_EQUAL = 276,           /* GREATER_EQUAL  */
    LESS_EQUAL = 277,              /* LESS_EQUAL  */
    EQUAL = 278,                   /* EQUAL  */
    NOT_EQUAL = 279,               /* NOT_EQUAL  */
    LOGICAL_AND = 280,             /* LOGICAL_AND  */
    LOGICAL_OR = 281,              /* LOGICAL_OR  */
    ASSIGN_PLUS = 282,             /* ASSIGN_PLUS  */
    ASSIGN_MINUS = 283,            /* ASSIGN_MINUS  */
    ASSIGN_MULTIPLY = 284,         /* ASSIGN_MULTIPLY  */
    ASSIGN_DIVIDE = 285,           /* ASSIGN_DIVIDE  */
    ASSIGN_MODULUS = 286,          /* ASSIGN_MODULUS  */
    ASSIGN_LSH = 287,              /* ASSIGN_LSH  */
    ASSIGN_RSH = 288,              /* ASSIGN_RSH  */
    ASSIGN_AND = 289,              /* ASSIGN_AND  */
    ASSIGN_OR = 290,               /* ASSIGN_OR  */
    ASSIGN_XOR = 291,              /* ASSIGN_XOR  */
    INCREMENT = 292,               /* INCREMENT  */
    DECREMENT = 293,               /* DECREMENT  */
    POSTINCREMENT = 294,           /* POSTINCREMENT  */
    POSTDECREMENT = 295,           /* POSTDECREMENT  */
    INDFOFFSET = 296,              /* INDFOFFSET  */
    TBL_NO_CHANGE = 297,           /* TBL_NO_CHANGE  */
    TBL_POST_INC = 298,            /* TBL_POST_INC  */
    TBL_POST_DEC = 299,            /* TBL_POST_DEC  */
    TBL_PRE_INC = 300              /* TBL_PRE_INC  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define LABEL 258
#define IDENTIFIER 259
#define IDENT_BRACKET 260
#define CBLOCK 261
#define DEBUG_LINE 262
#define ENDC 263
#define ERRORLEVEL 264
#define FILL 265
#define LIST 266
#define NUMBER 267
#define PROCESSOR 268
#define STRING 269
#define DEFINE 270
#define UPPER 271
#define HIGH 272
#define LOW 273
#define LSH 274
#define RSH 275
#define GREATER_EQUAL 276
#define LESS_EQUAL 277
#define EQUAL 278
#define NOT_EQUAL 279
#define LOGICAL_AND 280
#define LOGICAL_OR 281
#define ASSIGN_PLUS 282
#define ASSIGN_MINUS 283
#define ASSIGN_MULTIPLY 284
#define ASSIGN_DIVIDE 285
#define ASSIGN_MODULUS 286
#define ASSIGN_LSH 287
#define ASSIGN_RSH 288
#define ASSIGN_AND 289
#define ASSIGN_OR 290
#define ASSIGN_XOR 291
#define INCREMENT 292
#define DECREMENT 293
#define POSTINCREMENT 294
#define POSTDECREMENT 295
#define INDFOFFSET 296
#define TBL_NO_CHANGE 297
#define TBL_POST_INC 298
#define TBL_POST_DEC 299
#define TBL_PRE_INC 300

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 338 "parse.y"

  gpasmVal  i;
  char     *s;
  pnode_t  *p;

#line 555 "parse.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSE_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_LABEL = 3,                      /* LABEL  */
  YYSYMBOL_IDENTIFIER = 4,                 /* IDENTIFIER  */
  YYSYMBOL_IDENT_BRACKET = 5,              /* IDENT_BRACKET  */
  YYSYMBOL_CBLOCK = 6,                     /* CBLOCK  */
  YYSYMBOL_DEBUG_LINE = 7,                 /* DEBUG_LINE  */
  YYSYMBOL_ENDC = 8,                       /* ENDC  */
  YYSYMBOL_ERRORLEVEL = 9,                 /* ERRORLEVEL  */
  YYSYMBOL_FILL = 10,                      /* FILL  */
  YYSYMBOL_LIST = 11,                      /* LIST  */
  YYSYMBOL_NUMBER = 12,                    /* NUMBER  */
  YYSYMBOL_PROCESSOR = 13,                 /* PROCESSOR  */
  YYSYMBOL_STRING = 14,                    /* STRING  */
  YYSYMBOL_DEFINE = 15,                    /* DEFINE  */
  YYSYMBOL_UPPER = 16,                     /* UPPER  */
  YYSYMBOL_HIGH = 17,                      /* HIGH  */
  YYSYMBOL_LOW = 18,                       /* LOW  */
  YYSYMBOL_LSH = 19,                       /* LSH  */
  YYSYMBOL_RSH = 20,                       /* RSH  */
  YYSYMBOL_GREATER_EQUAL = 21,             /* GREATER_EQUAL  */
  YYSYMBOL_LESS_EQUAL = 22,                /* LESS_EQUAL  */
  YYSYMBOL_EQUAL = 23,                     /* EQUAL  */
  YYSYMBOL_NOT_EQUAL = 24,                 /* NOT_EQUAL  */
  YYSYMBOL_25_ = 25,                       /* '<'  */
  YYSYMBOL_26_ = 26,                       /* '>'  */
  YYSYMBOL_27_ = 27,                       /* '&'  */
  YYSYMBOL_28_ = 28,                       /* '|'  */
  YYSYMBOL_29_ = 29,                       /* '^'  */
  YYSYMBOL_LOGICAL_AND = 30,               /* LOGICAL_AND  */
  YYSYMBOL_LOGICAL_OR = 31,                /* LOGICAL_OR  */
  YYSYMBOL_32_ = 32,                       /* '='  */
  YYSYMBOL_ASSIGN_PLUS = 33,               /* ASSIGN_PLUS  */
  YYSYMBOL_ASSIGN_MINUS = 34,              /* ASSIGN_MINUS  */
  YYSYMBOL_ASSIGN_MULTIPLY = 35,           /* ASSIGN_MULTIPLY  */
  YYSYMBOL_ASSIGN_DIVIDE = 36,             /* ASSIGN_DIVIDE  */
  YYSYMBOL_ASSIGN_MODULUS = 37,            /* ASSIGN_MODULUS  */
  YYSYMBOL_ASSIGN_LSH = 38,                /* ASSIGN_LSH  */
  YYSYMBOL_ASSIGN_RSH = 39,                /* ASSIGN_RSH  */
  YYSYMBOL_ASSIGN_AND = 40,                /* ASSIGN_AND  */
  YYSYMBOL_ASSIGN_OR = 41,                 /* ASSIGN_OR  */
  YYSYMBOL_ASSIGN_XOR = 42,                /* ASSIGN_XOR  */
  YYSYMBOL_INCREMENT = 43,                 /* INCREMENT  */
  YYSYMBOL_DECREMENT = 44,                 /* DECREMENT  */
  YYSYMBOL_POSTINCREMENT = 45,             /* POSTINCREMENT  */
  YYSYMBOL_POSTDECREMENT = 46,             /* POSTDECREMENT  */
  YYSYMBOL_INDFOFFSET = 47,                /* INDFOFFSET  */
  YYSYMBOL_TBL_NO_CHANGE = 48,             /* TBL_NO_CHANGE  */
  YYSYMBOL_TBL_POST_INC = 49,              /* TBL_POST_INC  */
  YYSYMBOL_TBL_POST_DEC = 50,              /* TBL_POST_DEC  */
  YYSYMBOL_TBL_PRE_INC = 51,               /* TBL_PRE_INC  */
  YYSYMBOL_52_ = 52,                       /* '['  */
  YYSYMBOL_53_ = 53,                       /* ']'  */
  YYSYMBOL_54_ = 54,                       /* '+'  */
  YYSYMBOL_55_ = 55,                       /* '-'  */
  YYSYMBOL_56_ = 56,                       /* '*'  */
  YYSYMBOL_57_ = 57,                       /* '/'  */
  YYSYMBOL_58_ = 58,                       /* '%'  */
  YYSYMBOL_59_ = 59,                       /* '!'  */
  YYSYMBOL_60_ = 60,                       /* '~'  */
  YYSYMBOL_61_n_ = 61,                     /* '\n'  */
  YYSYMBOL_62_ = 62,                       /* ')'  */
  YYSYMBOL_63_ = 63,                       /* ','  */
  YYSYMBOL_64_ = 64,                       /* ':'  */
  YYSYMBOL_65_ = 65,                       /* '$'  */
  YYSYMBOL_66_ = 66,                       /* '('  */
  YYSYMBOL_YYACCEPT = 67,                  /* $accept  */
  YYSYMBOL_program = 68,                   /* program  */
  YYSYMBOL_69_1 = 69,                      /* $@1  */
  YYSYMBOL_line = 70,                      /* line  */
  YYSYMBOL_decimal_ops = 71,               /* decimal_ops  */
  YYSYMBOL_statement = 72,                 /* statement  */
  YYSYMBOL_73_2 = 73,                      /* $@2  */
  YYSYMBOL_74_3 = 74,                      /* $@3  */
  YYSYMBOL_75_4 = 75,                      /* $@4  */
  YYSYMBOL_76_5 = 76,                      /* $@5  */
  YYSYMBOL_77_6 = 77,                      /* $@6  */
  YYSYMBOL_const_block = 78,               /* const_block  */
  YYSYMBOL_const_line = 79,                /* const_line  */
  YYSYMBOL_const_def_list = 80,            /* const_def_list  */
  YYSYMBOL_81_7 = 81,                      /* $@7  */
  YYSYMBOL_const_def = 82,                 /* const_def  */
  YYSYMBOL_assign_equal_ops = 83,          /* assign_equal_ops  */
  YYSYMBOL_parameter_list = 84,            /* parameter_list  */
  YYSYMBOL_expr = 85,                      /* expr  */
  YYSYMBOL_e9op = 86,                      /* e9op  */
  YYSYMBOL_e8 = 87,                        /* e8  */
  YYSYMBOL_e8op = 88,                      /* e8op  */
  YYSYMBOL_e7 = 89,                        /* e7  */
  YYSYMBOL_e7op = 90,                      /* e7op  */
  YYSYMBOL_e6 = 91,                        /* e6  */
  YYSYMBOL_e6op = 92,                      /* e6op  */
  YYSYMBOL_e5 = 93,                        /* e5  */
  YYSYMBOL_e5op = 94,                      /* e5op  */
  YYSYMBOL_e4 = 95,                        /* e4  */
  YYSYMBOL_e4op = 96,                      /* e4op  */
  YYSYMBOL_e3 = 97,                        /* e3  */
  YYSYMBOL_e3op = 98,                      /* e3op  */
  YYSYMBOL_e2 = 99,                        /* e2  */
  YYSYMBOL_e2op = 100,                     /* e2op  */
  YYSYMBOL_e1 = 101,                       /* e1  */
  YYSYMBOL_e1op = 102,                     /* e1op  */
  YYSYMBOL_e0 = 103,                       /* e0  */
  YYSYMBOL_cidentifier = 104,              /* cidentifier  */
  YYSYMBOL_list_block = 105,               /* list_block  */
  YYSYMBOL_list_expr = 106,                /* list_expr  */
  YYSYMBOL_107_8 = 107,                    /* $@8  */
  YYSYMBOL_list_args = 108                 /* list_args  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
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

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   396

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  42
/* YYNRULES -- Number of rules.  */
#define YYNRULES  130
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  200

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   300


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
      61,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    59,     2,     2,    65,    58,    27,     2,
      66,    62,    56,    54,    63,    55,     2,    57,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    64,     2,
      25,    32,    26,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    52,     2,    53,    29,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    28,     2,    60,     2,     2,     2,
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
      30,    31,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   435,   435,   439,   438,   444,   460,   470,   479,   488,
     497,   587,   631,   631,   634,   646,   646,   653,   658,   658,
     665,   665,   672,   677,   682,   687,   692,   697,   702,   707,
     724,   742,   741,   762,   761,   781,   788,   790,   798,   800,
     807,   818,   831,   831,   836,   840,   847,   855,   855,   855,
     856,   856,   856,   856,   857,   857,   857,   860,   865,   870,
     875,   880,   885,   890,   895,   900,   905,   912,   914,   922,
     925,   927,   935,   938,   940,   948,   951,   953,   961,   961,
     961,   964,   966,   974,   974,   974,   974,   974,   974,   977,
     979,   987,   987,   990,   992,  1000,  1000,  1003,  1005,  1013,
    1013,  1013,  1016,  1018,  1025,  1025,  1025,  1025,  1025,  1025,
    1025,  1028,  1033,  1038,  1043,  1048,  1053,  1058,  1063,  1068,
    1073,  1078,  1083,  1090,  1097,  1102,  1110,  1109,  1121,  1128,
    1133
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "LABEL", "IDENTIFIER",
  "IDENT_BRACKET", "CBLOCK", "DEBUG_LINE", "ENDC", "ERRORLEVEL", "FILL",
  "LIST", "NUMBER", "PROCESSOR", "STRING", "DEFINE", "UPPER", "HIGH",
  "LOW", "LSH", "RSH", "GREATER_EQUAL", "LESS_EQUAL", "EQUAL", "NOT_EQUAL",
  "'<'", "'>'", "'&'", "'|'", "'^'", "LOGICAL_AND", "LOGICAL_OR", "'='",
  "ASSIGN_PLUS", "ASSIGN_MINUS", "ASSIGN_MULTIPLY", "ASSIGN_DIVIDE",
  "ASSIGN_MODULUS", "ASSIGN_LSH", "ASSIGN_RSH", "ASSIGN_AND", "ASSIGN_OR",
  "ASSIGN_XOR", "INCREMENT", "DECREMENT", "POSTINCREMENT", "POSTDECREMENT",
  "INDFOFFSET", "TBL_NO_CHANGE", "TBL_POST_INC", "TBL_POST_DEC",
  "TBL_PRE_INC", "'['", "']'", "'+'", "'-'", "'*'", "'/'", "'%'", "'!'",
  "'~'", "'\\n'", "')'", "','", "':'", "'$'", "'('", "$accept", "program",
  "$@1", "line", "decimal_ops", "statement", "$@2", "$@3", "$@4", "$@5",
  "$@6", "const_block", "const_line", "const_def_list", "$@7", "const_def",
  "assign_equal_ops", "parameter_list", "expr", "e9op", "e8", "e8op", "e7",
  "e7op", "e6", "e6op", "e5", "e5op", "e4", "e4op", "e3", "e3op", "e2",
  "e2op", "e1", "e1op", "e0", "cidentifier", "list_block", "list_expr",
  "$@8", "list_args", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    60,    62,    38,   124,    94,
     280,   281,    61,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,    91,    93,    43,    45,    42,    47,    37,    33,
     126,    10,    41,    44,    58,    36,    40
};
#endif

#define YYPACT_NINF (-93)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-127)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -93,   140,   -93,   -30,    23,   -93,   335,    58,     0,   -93,
     -93,    79,    24,   -93,     3,   -93,   -93,   -93,   -93,   254,
     -93,   -93,   -93,   -93,   -93,   -93,   -93,   -93,   -93,   -93,
      38,    42,   -93,   254,   -93,   -93,   -93,   -93,   -93,   -93,
     178,   178,   -93,   -93,   -93,   254,   -93,   -93,   -93,   -93,
     -93,   -93,   178,   -93,   254,    84,   -10,    66,    75,    61,
     181,    73,    41,   -17,   -93,   254,   -93,   -93,   114,   -93,
     -93,   -93,   -26,   121,   -93,   272,   122,     7,     5,   -93,
     178,   -13,   -93,   -93,    39,    72,    72,   -29,    72,    -4,
     -93,   -93,   -93,   -93,   254,   178,   254,   -93,   254,   -93,
     254,   -93,   -93,   -93,   254,   -93,   -93,   -93,   -93,   -93,
     -93,   254,   -93,   -93,   254,   -93,   -93,   254,   -93,   -93,
     -93,   254,   -93,    68,   -93,   -93,    67,    36,    99,    66,
      71,    89,    81,    93,   -93,    95,   120,    74,   113,   -93,
     -93,   -93,   -93,    25,    72,    66,    75,    61,   181,    73,
      41,   -17,   -93,   -93,     2,   -93,   254,    96,   129,   -93,
     272,   -93,   -93,   158,   102,   152,   -93,   -93,   196,   106,
     -93,   -93,   -16,   165,    12,    45,   254,   254,   -93,   -93,
     -93,   117,   -93,    50,   -93,   -93,   165,   -93,   115,   124,
     -93,    59,    66,   -93,   -93,   -93,   254,   -93,   -93,   129
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,     0,     5,     0,     0,     0,    13,
      12,     0,    18,    15,     0,    14,     4,    20,    11,     0,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
       0,     0,    10,     0,   123,   112,   114,   104,   105,   106,
     121,   122,   118,   119,   120,     0,   110,   107,   117,   108,
     109,    27,    62,   113,     0,     0,    58,    67,    70,    73,
      76,    81,    89,    93,    97,     0,   102,   111,     0,   121,
     122,    33,     0,     0,    17,     0,     0,     0,     0,    26,
       0,     0,     9,     8,     0,    63,    64,     0,    61,     0,
      65,    66,    28,    69,     0,    59,     0,    72,     0,    75,
       0,    78,    79,    80,     0,    87,    88,    85,    86,    83,
      84,     0,    91,    92,     0,    95,    96,     0,    99,   100,
     101,     0,   103,     0,    36,    31,     0,     0,   123,   128,
       0,   124,     0,     0,    25,   129,     0,     0,     0,     7,
       6,   116,   115,     0,    60,    68,    71,    74,    77,    82,
      90,    94,    98,    35,    42,    36,     0,     0,     0,    19,
       0,    16,    22,     0,     0,     0,    21,    57,     0,     0,
      38,    37,     0,     0,    42,     0,     0,     0,   125,   130,
      24,     0,    40,     0,    34,    39,     0,    43,    45,     0,
      29,     0,   127,    23,    41,    44,     0,    32,    30,    46
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -93,   -93,   -93,   -93,   -93,   183,   -93,   -93,   -93,   -93,
     -93,    43,   -93,   -93,   -93,    11,   -93,   260,    -8,    33,
     -73,   -93,   101,   -93,   109,   -93,   107,   -93,    82,   -93,
     103,   -93,    98,   -93,   -52,   -93,   -93,   -92,    56,   -93,
     -93,    55
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,     4,    16,    17,    18,    76,    75,    80,   155,
     124,   154,   171,   172,   173,   187,    33,    55,    56,    96,
      57,    98,    58,   100,    59,   104,    60,   111,    61,   114,
      62,   117,    63,   121,    64,    65,    66,    67,   130,   131,
     158,   137
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      72,    68,   129,    93,    34,   168,    93,    77,    78,   135,
     169,    81,    35,   122,    36,   168,    37,    38,    39,    93,
     189,   133,    93,   145,   141,    84,     6,     7,    93,     8,
       9,     5,    10,    11,    12,   125,    13,    87,    14,   118,
     119,   120,    94,    69,    70,   185,    89,   186,   139,    42,
      43,    44,    45,    95,    46,    47,    48,    93,   142,    49,
      50,    71,    34,   170,    79,    53,    54,   136,   134,   152,
      35,    93,    36,   170,    37,    38,    39,    93,   167,    90,
      91,   188,    93,    73,    15,    74,   143,   129,   101,   102,
     103,    93,   112,   113,   188,   115,   116,    97,   157,    82,
     140,    40,    41,    83,   192,    99,   190,    42,    43,    44,
      45,   194,    46,    47,    48,    90,    91,    49,    50,    51,
     198,    52,   123,    53,    54,    34,   132,    90,    91,   153,
     156,  -126,   159,    35,   164,    36,   165,    37,    38,    39,
       2,     3,   161,    -3,    -3,    92,    -3,    -3,   175,    -3,
      -3,    -3,   160,    -3,   162,    -3,    90,    91,   163,   176,
     183,    93,   135,   180,    40,    41,   181,   184,   191,    34,
      42,    43,    44,    45,   166,    46,    47,    48,   193,   196,
      49,    50,    34,   126,    52,   197,    53,    54,   199,    32,
      35,   177,    36,   149,    37,    38,    39,   195,   174,   146,
      34,    -3,   105,   106,   107,   108,   109,   110,    35,   147,
      36,   148,    37,    38,    39,   151,   178,   150,   179,     0,
       0,    40,    41,     0,     0,     0,     0,    42,    43,    44,
      45,     0,    46,    47,    48,     0,     0,    49,    50,    69,
      70,    52,     0,    53,    54,    42,    43,    44,    45,     0,
      46,    47,    48,     0,     0,    49,    50,   182,    34,     0,
       0,    53,    54,     0,     0,     0,    35,     0,    36,     0,
      37,    38,    39,     0,     0,     0,   128,     0,     0,     0,
       0,     0,     0,     0,    35,     0,    36,     0,    37,    38,
      39,     0,     0,     0,     0,     0,     0,    69,    70,     0,
      85,    86,     0,    42,    43,    44,    45,     0,    46,    47,
      48,     0,    88,    49,    50,    69,    70,     0,     0,    53,
      54,    42,    43,    44,    45,     0,    46,    47,    48,     0,
       0,    49,    50,   127,     0,     0,     0,    53,    54,     7,
     138,     8,     9,     0,    10,    11,    12,     0,    13,     0,
      14,     0,     0,     0,     0,   144,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15
};

static const yytype_int16 yycheck[] =
{
       8,     1,    75,    32,     4,     3,    32,     4,     5,     4,
       8,    19,    12,    65,    14,     3,    16,    17,    18,    32,
       8,    14,    32,    96,    53,    33,     3,     4,    32,     6,
       7,    61,     9,    10,    11,    61,    13,    45,    15,    56,
      57,    58,    52,    43,    44,    61,    54,    63,    61,    49,
      50,    51,    52,    63,    54,    55,    56,    32,    62,    59,
      60,    61,     4,    61,    61,    65,    66,    62,    61,   121,
      12,    32,    14,    61,    16,    17,    18,    32,    53,    43,
      44,   173,    32,     4,    61,    61,    94,   160,    27,    28,
      29,    32,    19,    20,   186,    54,    55,    31,    62,    61,
      61,    43,    44,    61,   177,    30,    61,    49,    50,    51,
      52,    61,    54,    55,    56,    43,    44,    59,    60,    61,
      61,    63,     8,    65,    66,     4,     4,    43,    44,    61,
      63,    32,    61,    12,    14,    14,    62,    16,    17,    18,
       0,     1,    61,     3,     4,    61,     6,     7,   156,     9,
      10,    11,    63,    13,    61,    15,    43,    44,    63,    63,
     168,    32,     4,    61,    43,    44,    14,    61,   176,     4,
      49,    50,    51,    52,    61,    54,    55,    56,    61,    64,
      59,    60,     4,    62,    63,    61,    65,    66,   196,     6,
      12,   158,    14,   111,    16,    17,    18,   186,   155,    98,
       4,    61,    21,    22,    23,    24,    25,    26,    12,   100,
      14,   104,    16,    17,    18,   117,   160,   114,   163,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    50,    51,
      52,    -1,    54,    55,    56,    -1,    -1,    59,    60,    43,
      44,    63,    -1,    65,    66,    49,    50,    51,    52,    -1,
      54,    55,    56,    -1,    -1,    59,    60,    61,     4,    -1,
      -1,    65,    66,    -1,    -1,    -1,    12,    -1,    14,    -1,
      16,    17,    18,    -1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    12,    -1,    14,    -1,    16,    17,
      18,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    -1,
      40,    41,    -1,    49,    50,    51,    52,    -1,    54,    55,
      56,    -1,    52,    59,    60,    43,    44,    -1,    -1,    65,
      66,    49,    50,    51,    52,    -1,    54,    55,    56,    -1,
      -1,    59,    60,    73,    -1,    -1,    -1,    65,    66,     4,
      80,     6,     7,    -1,     9,    10,    11,    -1,    13,    -1,
      15,    -1,    -1,    -1,    -1,    95,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    61
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    68,     0,     1,    69,    61,     3,     4,     6,     7,
       9,    10,    11,    13,    15,    61,    70,    71,    72,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    72,    83,     4,    12,    14,    16,    17,    18,
      43,    44,    49,    50,    51,    52,    54,    55,    56,    59,
      60,    61,    63,    65,    66,    84,    85,    87,    89,    91,
      93,    95,    97,    99,   101,   102,   103,   104,     1,    43,
      44,    61,    85,     4,    61,    74,    73,     4,     5,    61,
      75,    85,    61,    61,    85,    84,    84,    85,    84,    85,
      43,    44,    61,    32,    52,    63,    86,    31,    88,    30,
      90,    27,    28,    29,    92,    21,    22,    23,    24,    25,
      26,    94,    19,    20,    96,    54,    55,    98,    56,    57,
      58,   100,   101,     8,    77,    61,    62,    84,     4,    87,
     105,   106,     4,    14,    61,     4,    62,   108,    84,    61,
      61,    53,    62,    85,    84,    87,    89,    91,    93,    95,
      97,    99,   101,    61,    78,    76,    63,    62,   107,    61,
      63,    61,    61,    63,    14,    62,    61,    53,     3,     8,
      61,    79,    80,    81,    78,    85,    63,    86,   105,   108,
      61,    14,    61,    85,    61,    61,    63,    82,   104,     8,
      61,    85,    87,    61,    61,    82,    64,    61,    61,    85
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    67,    68,    69,    68,    68,    70,    70,    70,    70,
      70,    70,    71,    71,    72,    73,    72,    72,    74,    72,
      75,    72,    72,    72,    72,    72,    72,    72,    72,    72,
      72,    76,    72,    77,    72,    72,    78,    78,    79,    79,
      79,    79,    81,    80,    80,    82,    82,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    85,    85,    86,
      87,    87,    88,    89,    89,    90,    91,    91,    92,    92,
      92,    93,    93,    94,    94,    94,    94,    94,    94,    95,
      95,    96,    96,    97,    97,    98,    98,    99,    99,   100,
     100,   100,   101,   101,   102,   102,   102,   102,   102,   102,
     102,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   104,   105,   105,   107,   106,   106,   108,
     108
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     0,     3,     3,     4,     4,     3,     3,
       2,     1,     1,     1,     1,     0,     4,     2,     0,     4,
       0,     4,     4,     6,     5,     3,     2,     2,     3,     6,
       7,     0,     7,     0,     6,     4,     0,     2,     1,     2,
       2,     3,     0,     2,     3,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     2,
       3,     2,     1,     2,     2,     2,     2,     1,     3,     1,
       1,     3,     1,     1,     3,     1,     1,     3,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     1,     1,     3,     1,     1,     1,     3,     1,
       1,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     0,     4,     1,     1,
       3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 3: /* $@1: %empty  */
#line 439 "parse.y"
        {
          state.lst.line.was_byte_addr = state.byte_addr;
          state.lst.line.linetype      = LTY_NONE;
          state.next_state             = STATE_NOCHANGE;
        }
#line 1845 "parse.c"
    break;

  case 5: /* program: program error '\n'  */
#line 445 "parse.y"
        {
          state.lst.line.was_byte_addr = state.byte_addr;
          state.lst.line.linetype      = LTY_NONE;
          state.next_state             = STATE_NOCHANGE;

          yyerrok;  /* generate multiple errors */
          if (IN_MACRO_WHILE_DEFINITION) {
            /* in macro definition: append the macro */
            macro_append();
          }
          next_line(0);
        }
#line 1862 "parse.c"
    break;

  case 6: /* line: LABEL assign_equal_ops expr '\n'  */
#line 461 "parse.y"
        {
          pnode_t *parms;
          int      exp_result;

          exp_result = do_insn("set", mk_list((yyvsp[-1].p), NULL));
          parms = mk_list(mk_2op(return_op((yyvsp[-2].i)), mk_symbol((yyvsp[-3].s)), mk_constant(exp_result)), NULL);
          next_line(set_label((yyvsp[-3].s), parms));
        }
#line 1875 "parse.c"
    break;

  case 7: /* line: LABEL '=' expr '\n'  */
#line 471 "parse.y"
        {
          pnode_t *parms;

          /* implements i = 6 + 1 */
          parms = mk_list((yyvsp[-1].p), NULL);
          next_line(set_label((yyvsp[-3].s), parms));
        }
#line 1887 "parse.c"
    break;

  case 8: /* line: LABEL DECREMENT '\n'  */
#line 480 "parse.y"
        {
          pnode_t *parms;

          /* implements i-- */
          parms = mk_list(mk_1op(DECREMENT, mk_symbol((yyvsp[-2].s))), NULL);
          next_line(set_label((yyvsp[-2].s), parms));
        }
#line 1899 "parse.c"
    break;

  case 9: /* line: LABEL INCREMENT '\n'  */
#line 489 "parse.y"
        {
          pnode_t *parms;

          /* implements i++ */
          parms = mk_list(mk_1op(INCREMENT, mk_symbol((yyvsp[-2].s))), NULL);
          next_line(set_label((yyvsp[-2].s), parms));
        }
#line 1911 "parse.c"
    break;

  case 10: /* line: LABEL statement  */
#line 498 "parse.y"
        {
          if (asm_enabled()) {
            if (state.lst.line.linetype == LTY_NONE) {
              if (IS_RAM_ORG) {
                /* alias to next definition */
                state.lst.line.linetype = LTY_RES;
              }
              else {
                state.lst.line.linetype = LTY_INSN;
              }
            }

            if (state.mac_head != NULL) {
              /* This is a macro definition. Set it up. */
              symbol_t     *mac;
              macro_head_t *head;

              mac  = gp_sym_get_symbol(state.stMacros, (yyvsp[-1].s));
              head = (mac != NULL) ? (macro_head_t *)gp_sym_get_symbol_annotation(mac) : NULL;

              /* It's not an error if macro was defined on pass 1 and we're in pass 2. */
              if ((head != NULL) && !((head->pass == 1) && (state.pass == 2))) {
                gpmsg_verror(GPE_DUPLICATE_MACRO, NULL);
              }
              else {
                if (mac == NULL) {
                  mac = gp_sym_add_symbol(state.stMacros, (yyvsp[-1].s));
                }

                gp_sym_annotate_symbol(mac, state.mac_head);
                head = state.mac_head;
                head->line_number = state.src_list.last->line_number;
                head->file_symbol = state.src_list.last->file_symbol;
              }

              head->pass = state.pass;

              /* The macro is defined so allow calls. */
              if (state.pass == 2) {
                head->defined = true;
              }

              state.mac_head = NULL;
            }
            else if (!(IN_MACRO_WHILE_DEFINITION)) {
              /* Outside a macro definition, just define the label. */
              switch (state.lst.line.linetype) {
                case LTY_SEC:
                  gp_strncpy(state.obj.new_sect_name, (yyvsp[-1].s), sizeof(state.obj.new_sect_name));
                  break;

                case LTY_SET:
                  set_global((yyvsp[-1].s), (yyvsp[0].i), VAL_VARIABLE, false, false);
                  break;

                case LTY_ORG:
                case LTY_EQU:
                  set_global((yyvsp[-1].s), (yyvsp[0].i), VAL_CONSTANT, false, false);
                  break;

                case LTY_INSN:
                case LTY_DATA:
                case LTY_RES: {
                  if ((state.mode == MODE_RELOCATABLE) && !(IN_MACRO_WHILE_DEFINITION) &&
                      !(SECTION_FLAGS & (STYP_TEXT | STYP_RAM_AREA | STYP_BPACK))) {
                    gpmsg_verror(GPE_LABEL_IN_SECTION, NULL);
                  }

                  if (IS_RAM_ORG) {
                    set_global((yyvsp[-1].s), (yyvsp[0].i), VAL_STATIC, false, false);
                  }
                  else {
                    set_global((yyvsp[-1].s), (yyvsp[0].i), VAL_ADDRESS, false, false);
                  }
                  break;
                }

                case LTY_DIR:
                  gpmsg_verror(GPE_ILLEGAL_LABEL, NULL, (yyvsp[-1].s));
                  break;

                default:
                  break;
              }
            }
          }
          next_line((yyvsp[0].i));
        }
#line 2004 "parse.c"
    break;

  case 11: /* line: statement  */
#line 588 "parse.y"
        {
          if (state.mac_head != NULL) {
            /* This is a macro definition, but the label was missing. */
            state.mac_head = NULL;
            gpmsg_verror(GPE_NO_MACRO_NAME, NULL);
          }
          else {
            if (state.found_end) {
              switch (state.src_list.last->type) {
                case SRC_WHILE:
                  gpmsg_error(GPE_EXPECTED, "Expected (ENDW)");
                  break;

                case SRC_MACRO:
                  gpmsg_error(GPW_EXPECTED, "Expected (ENDM)");
                  /* fall through */

                default:
                  break;
              }

              if (state.astack != NULL) {
                amode_t *old;

                while (state.astack != NULL) {
                  old = state.astack;
                  state.astack = state.astack->upper;
                  free(old);
                }
                gpmsg_warning(GPW_EXPECTED, "Expected (ENDIF)");
              }
            }

            next_line((yyvsp[0].i));

            if (state.found_end) {
              found_end();
              YYACCEPT;
            }
          }
        }
#line 2050 "parse.c"
    break;

  case 14: /* statement: '\n'  */
#line 635 "parse.y"
        {
          if (!(IN_MACRO_WHILE_DEFINITION)) {
            (yyval.i) = (IS_RAM_ORG) ? state.byte_addr :
                                /* We want to have r as the value to assign to label. */
                                gp_processor_insn_from_byte_p(state.processor, state.byte_addr);
          }
          else {
            macro_append();
          }
        }
#line 2065 "parse.c"
    break;

  case 15: /* $@2: %empty  */
#line 646 "parse.y"
                  { force_ident = true; }
#line 2071 "parse.c"
    break;

  case 16: /* statement: PROCESSOR $@2 IDENTIFIER '\n'  */
#line 648 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-3].s), mk_list(mk_symbol((yyvsp[-1].s)), NULL));
          force_ident = false;
        }
#line 2080 "parse.c"
    break;

  case 17: /* statement: LIST '\n'  */
#line 654 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-1].s), NULL);
        }
#line 2088 "parse.c"
    break;

  case 18: /* $@3: %empty  */
#line 658 "parse.y"
             { force_decimal = true; }
#line 2094 "parse.c"
    break;

  case 19: /* statement: LIST $@3 list_block '\n'  */
#line 660 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-3].s), (yyvsp[-1].p));
          force_decimal = false;
        }
#line 2103 "parse.c"
    break;

  case 20: /* $@4: %empty  */
#line 665 "parse.y"
                    { force_decimal = true; }
#line 2109 "parse.c"
    break;

  case 21: /* statement: decimal_ops $@4 parameter_list '\n'  */
#line 667 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-3].s), (yyvsp[-1].p));
          force_decimal = false;
        }
#line 2118 "parse.c"
    break;

  case 22: /* statement: DEFINE IDENTIFIER STRING '\n'  */
#line 673 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-3].s), mk_list(mk_string((yyvsp[-2].s)), mk_list(mk_string((yyvsp[-1].s)), NULL)));
        }
#line 2126 "parse.c"
    break;

  case 23: /* statement: DEFINE IDENT_BRACKET list_args ')' STRING '\n'  */
#line 678 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-5].s), mk_list(mk_string((yyvsp[-4].s)), mk_list(mk_string((yyvsp[-1].s)), (yyvsp[-3].p))));
        }
#line 2134 "parse.c"
    break;

  case 24: /* statement: DEFINE IDENT_BRACKET ')' STRING '\n'  */
#line 683 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-4].s), mk_list(mk_string((yyvsp[-3].s)), mk_list(mk_string((yyvsp[-1].s)), NULL)));
        }
#line 2142 "parse.c"
    break;

  case 25: /* statement: DEFINE IDENTIFIER '\n'  */
#line 688 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-2].s), mk_list(mk_string((yyvsp[-1].s)), NULL));
        }
#line 2150 "parse.c"
    break;

  case 26: /* statement: DEFINE '\n'  */
#line 693 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-1].s), NULL);
        }
#line 2158 "parse.c"
    break;

  case 27: /* statement: IDENTIFIER '\n'  */
#line 698 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-1].s), NULL);
        }
#line 2166 "parse.c"
    break;

  case 28: /* statement: IDENTIFIER parameter_list '\n'  */
#line 703 "parse.y"
        {
          (yyval.i) = do_or_append_insn((yyvsp[-2].s), (yyvsp[-1].p));
        }
#line 2174 "parse.c"
    break;

  case 29: /* statement: FILL IDENTIFIER ')' ',' expr '\n'  */
#line 708 "parse.y"
        {
          int number;
          int i;

          if (!(IN_MACRO_WHILE_DEFINITION)) {
            number = eval_fill_number((yyvsp[-1].p));

            for (i = 0; i < number; i++) {
              (yyval.i) = do_insn((yyvsp[-4].s), NULL);
            }
          }
          else {
            macro_append();
          }
        }
#line 2194 "parse.c"
    break;

  case 30: /* statement: FILL IDENTIFIER parameter_list ')' ',' expr '\n'  */
#line 725 "parse.y"
        {
          int number;
          int i;

          if (!(IN_MACRO_WHILE_DEFINITION)) {
            number = eval_fill_number((yyvsp[-1].p));

            for (i = 0; i < number; i++) {
              (yyval.i) = do_insn((yyvsp[-5].s), (yyvsp[-4].p));
            }
          }
          else {
            macro_append();
          }
        }
#line 2214 "parse.c"
    break;

  case 31: /* $@5: %empty  */
#line 742 "parse.y"
        {
          if (!(IN_MACRO_WHILE_DEFINITION)) {
            begin_cblock((yyvsp[-1].p));
          }
          else {
            macro_append();
          }
          next_line(0);
        }
#line 2228 "parse.c"
    break;

  case 32: /* statement: CBLOCK expr '\n' $@5 const_block ENDC '\n'  */
#line 753 "parse.y"
        {
          state.lst.line.linetype = LTY_NONE;
          if (IN_MACRO_WHILE_DEFINITION) {
            macro_append();
          }
          (yyval.i) = 0;
        }
#line 2240 "parse.c"
    break;

  case 33: /* $@6: %empty  */
#line 762 "parse.y"
        {
          if (!(IN_MACRO_WHILE_DEFINITION)) {
            continue_cblock();
          }
          else {
            macro_append();
          }
          next_line(0);
        }
#line 2254 "parse.c"
    break;

  case 34: /* statement: CBLOCK '\n' $@6 const_block ENDC '\n'  */
#line 773 "parse.y"
        {
          state.lst.line.linetype = LTY_NONE;
          if (IN_MACRO_WHILE_DEFINITION) {
            macro_append();
          }
          (yyval.i) = 0;
        }
#line 2266 "parse.c"
    break;

  case 35: /* statement: CBLOCK error ENDC '\n'  */
#line 782 "parse.y"
        {
          state.lst.line.linetype = LTY_NONE;
          (yyval.i) = 0;
        }
#line 2275 "parse.c"
    break;

  case 37: /* const_block: const_block const_line  */
#line 791 "parse.y"
        {
          state.lst.line.linetype = LTY_SET;
          next_line(state.lst.cblock_lst);
        }
#line 2284 "parse.c"
    break;

  case 39: /* const_line: const_def_list '\n'  */
#line 801 "parse.y"
        {
          if (IN_MACRO_WHILE_DEFINITION) {
            macro_append();
          }
        }
#line 2294 "parse.c"
    break;

  case 40: /* const_line: LABEL '\n'  */
#line 808 "parse.y"
        {
          if (!(IN_MACRO_WHILE_DEFINITION)) {
            state.lst.cblock_lst = state.cblock;
            cblock_expr(mk_symbol((yyvsp[-1].s)));
          }
          else {
            macro_append();
          }
        }
#line 2308 "parse.c"
    break;

  case 41: /* const_line: LABEL expr '\n'  */
#line 819 "parse.y"
        {
          if (!(IN_MACRO_WHILE_DEFINITION)) {
            state.lst.cblock_lst = state.cblock;
            cblock_expr_incr(mk_symbol((yyvsp[-2].s)), (yyvsp[-1].p));
          }
          else {
            macro_append();
          }
        }
#line 2322 "parse.c"
    break;

  case 42: /* $@7: %empty  */
#line 831 "parse.y"
        {
          state.lst.cblock_lst = state.cblock;
        }
#line 2330 "parse.c"
    break;

  case 45: /* const_def: cidentifier  */
#line 841 "parse.y"
        {
          if (!(IN_MACRO_WHILE_DEFINITION)) {
            cblock_expr((yyvsp[0].p));
          }
        }
#line 2340 "parse.c"
    break;

  case 46: /* const_def: cidentifier ':' expr  */
#line 848 "parse.y"
        {
          if (!(IN_MACRO_WHILE_DEFINITION)) {
            cblock_expr_incr((yyvsp[-2].p), (yyvsp[0].p));
          }
        }
#line 2350 "parse.c"
    break;

  case 57: /* parameter_list: expr '[' expr ']'  */
#line 861 "parse.y"
        {
          (yyval.p) = mk_list(mk_constant(INDFOFFSET), mk_list((yyvsp[-1].p), (yyvsp[-3].p)));
        }
#line 2358 "parse.c"
    break;

  case 58: /* parameter_list: expr  */
#line 866 "parse.y"
        {
          (yyval.p) = mk_list((yyvsp[0].p), NULL);
        }
#line 2366 "parse.c"
    break;

  case 59: /* parameter_list: expr ','  */
#line 871 "parse.y"
        {
          (yyval.p) = mk_list((yyvsp[-1].p), mk_list(mk_symbol(""), NULL));
        }
#line 2374 "parse.c"
    break;

  case 60: /* parameter_list: expr ',' parameter_list  */
#line 876 "parse.y"
        {
          (yyval.p) = mk_list((yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2382 "parse.c"
    break;

  case 61: /* parameter_list: ',' parameter_list  */
#line 881 "parse.y"
        {
          (yyval.p) = mk_list(mk_symbol(""), (yyvsp[0].p));
        }
#line 2390 "parse.c"
    break;

  case 62: /* parameter_list: ','  */
#line 886 "parse.y"
        {
          (yyval.p) = mk_list(mk_symbol(""), mk_list(mk_symbol(""), NULL));
        }
#line 2398 "parse.c"
    break;

  case 63: /* parameter_list: INCREMENT parameter_list  */
#line 891 "parse.y"
        {
          (yyval.p) = mk_list(mk_constant((yyvsp[-1].i)), (yyvsp[0].p));
        }
#line 2406 "parse.c"
    break;

  case 64: /* parameter_list: DECREMENT parameter_list  */
#line 896 "parse.y"
        {
          (yyval.p) = mk_list(mk_constant((yyvsp[-1].i)), (yyvsp[0].p));
        }
#line 2414 "parse.c"
    break;

  case 65: /* parameter_list: parameter_list INCREMENT  */
#line 901 "parse.y"
        {
          (yyval.p) = mk_list(mk_constant(POSTINCREMENT), (yyvsp[-1].p));
        }
#line 2422 "parse.c"
    break;

  case 66: /* parameter_list: parameter_list DECREMENT  */
#line 906 "parse.y"
        {
          (yyval.p) = mk_list(mk_constant(POSTDECREMENT), (yyvsp[-1].p));
        }
#line 2430 "parse.c"
    break;

  case 68: /* expr: expr e9op e8  */
#line 915 "parse.y"
        {
          coerce_str1((yyvsp[-2].p));
          coerce_str1((yyvsp[0].p));
          (yyval.p) = mk_2op((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2440 "parse.c"
    break;

  case 71: /* e8: e8 e8op e7  */
#line 928 "parse.y"
        {
          coerce_str1((yyvsp[-2].p));
          coerce_str1((yyvsp[0].p));
          (yyval.p) = mk_2op((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2450 "parse.c"
    break;

  case 74: /* e7: e7 e7op e6  */
#line 941 "parse.y"
        {
          coerce_str1((yyvsp[-2].p));
          coerce_str1((yyvsp[0].p));
          (yyval.p) = mk_2op((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2460 "parse.c"
    break;

  case 77: /* e6: e6 e6op e5  */
#line 954 "parse.y"
        {
          coerce_str1((yyvsp[-2].p));
          coerce_str1((yyvsp[0].p));
          (yyval.p) = mk_2op((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2470 "parse.c"
    break;

  case 82: /* e5: e5 e5op e4  */
#line 967 "parse.y"
        {
          coerce_str1((yyvsp[-2].p));
          coerce_str1((yyvsp[0].p));
          (yyval.p) = mk_2op((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2480 "parse.c"
    break;

  case 90: /* e4: e4 e4op e3  */
#line 980 "parse.y"
        {
          coerce_str1((yyvsp[-2].p));
          coerce_str1((yyvsp[0].p));
          (yyval.p) = mk_2op((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2490 "parse.c"
    break;

  case 94: /* e3: e3 e3op e2  */
#line 993 "parse.y"
        {
          coerce_str1((yyvsp[-2].p));
          coerce_str1((yyvsp[0].p));
          (yyval.p) = mk_2op((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2500 "parse.c"
    break;

  case 98: /* e2: e2 e2op e1  */
#line 1006 "parse.y"
        {
          coerce_str1((yyvsp[-2].p));
          coerce_str1((yyvsp[0].p));
          (yyval.p) = mk_2op((yyvsp[-1].i), (yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2510 "parse.c"
    break;

  case 103: /* e1: e1op e1  */
#line 1019 "parse.y"
        {
          coerce_str1((yyvsp[0].p));
          (yyval.p) = mk_1op((yyvsp[-1].i), (yyvsp[0].p));
        }
#line 2519 "parse.c"
    break;

  case 111: /* e0: cidentifier  */
#line 1029 "parse.y"
        {
          (yyval.p) = (yyvsp[0].p);
        }
#line 2527 "parse.c"
    break;

  case 112: /* e0: NUMBER  */
#line 1034 "parse.y"
        {
          (yyval.p) = mk_constant((yyvsp[0].i));
        }
#line 2535 "parse.c"
    break;

  case 113: /* e0: '$'  */
#line 1039 "parse.y"
        {
          (yyval.p) = mk_symbol("$");
        }
#line 2543 "parse.c"
    break;

  case 114: /* e0: STRING  */
#line 1044 "parse.y"
        {
          (yyval.p) = mk_string((yyvsp[0].s));
        }
#line 2551 "parse.c"
    break;

  case 115: /* e0: '(' expr ')'  */
#line 1049 "parse.y"
        {
          (yyval.p) = (yyvsp[-1].p);
        }
#line 2559 "parse.c"
    break;

  case 116: /* e0: '[' expr ']'  */
#line 1054 "parse.y"
        {
          (yyval.p) = mk_offset((yyvsp[-1].p));
        }
#line 2567 "parse.c"
    break;

  case 117: /* e0: '*'  */
#line 1059 "parse.y"
        {
          (yyval.p) = mk_constant(TBL_NO_CHANGE);
        }
#line 2575 "parse.c"
    break;

  case 118: /* e0: TBL_POST_INC  */
#line 1064 "parse.y"
        {
          (yyval.p) = mk_constant((yyvsp[0].i));
        }
#line 2583 "parse.c"
    break;

  case 119: /* e0: TBL_POST_DEC  */
#line 1069 "parse.y"
        {
          (yyval.p) = mk_constant((yyvsp[0].i));
        }
#line 2591 "parse.c"
    break;

  case 120: /* e0: TBL_PRE_INC  */
#line 1074 "parse.y"
        {
          (yyval.p) = mk_constant((yyvsp[0].i));
        }
#line 2599 "parse.c"
    break;

  case 121: /* e0: INCREMENT  */
#line 1079 "parse.y"
        {
          (yyval.p) = mk_constant((yyvsp[0].i));
        }
#line 2607 "parse.c"
    break;

  case 122: /* e0: DECREMENT  */
#line 1084 "parse.y"
        {
          (yyval.p) = mk_constant((yyvsp[0].i));
        }
#line 2615 "parse.c"
    break;

  case 123: /* cidentifier: IDENTIFIER  */
#line 1091 "parse.y"
        {
          (yyval.p) = mk_symbol((yyvsp[0].s));
        }
#line 2623 "parse.c"
    break;

  case 124: /* list_block: list_expr  */
#line 1098 "parse.y"
        {
          (yyval.p) = mk_list((yyvsp[0].p), NULL);
        }
#line 2631 "parse.c"
    break;

  case 125: /* list_block: list_expr ',' list_block  */
#line 1103 "parse.y"
        {
          (yyval.p) = mk_list((yyvsp[-2].p), (yyvsp[0].p));
        }
#line 2639 "parse.c"
    break;

  case 126: /* $@8: %empty  */
#line 1110 "parse.y"
        {
          if ((strcasecmp((yyvsp[0].s), "p") == 0) || (strcasecmp((yyvsp[0].s), "pe") == 0)) {
            force_ident = true;
          }
        }
#line 2649 "parse.c"
    break;

  case 127: /* list_expr: IDENTIFIER $@8 e9op e8  */
#line 1116 "parse.y"
        {
          (yyval.p) = mk_2op((yyvsp[-1].i), mk_symbol((yyvsp[-3].s)), (yyvsp[0].p));
          force_ident = false;
        }
#line 2658 "parse.c"
    break;

  case 128: /* list_expr: e8  */
#line 1122 "parse.y"
        {
          (yyval.p) = (yyvsp[0].p);
        }
#line 2666 "parse.c"
    break;

  case 129: /* list_args: IDENTIFIER  */
#line 1129 "parse.y"
        {
          (yyval.p) = mk_list(mk_symbol((yyvsp[0].s)), NULL);
        }
#line 2674 "parse.c"
    break;

  case 130: /* list_args: IDENTIFIER ',' list_args  */
#line 1134 "parse.y"
        {
          (yyval.p) = mk_list(mk_symbol((yyvsp[-2].s)), (yyvsp[0].p));
        }
#line 2682 "parse.c"
    break;


#line 2686 "parse.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1138 "parse.y"


int
return_op(int Operation)
{
  /* returns an operator for the replacement of i+=1 with i=i+1*/
  switch (Operation) {
    case ASSIGN_PLUS:     return '+';
    case ASSIGN_MINUS:    return '-';
    case ASSIGN_MULTIPLY: return '*';
    case ASSIGN_DIVIDE:   return '/';
    case ASSIGN_MODULUS:  return '%';
    case ASSIGN_LSH:      return LSH;
    case ASSIGN_RSH:      return RSH;
    case ASSIGN_AND:      return '&';
    case ASSIGN_OR:       return '|';
    case ASSIGN_XOR:      return '^';
    default:
      assert(0); /* Unhandled operator */
  }

  return 0;
}
