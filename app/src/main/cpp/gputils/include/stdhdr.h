/* standard defines and includes
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005
   James Bowman, Craig Franklin
   
   Copyright (C) 2016 Molnar Karoly

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


#ifndef __STDHDR_H__
#define __STDHDR_H__

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#if defined(HAVE_STRING_H)
  #define _GNU_SOURCE
  #include <string.h>
#elif defined(HAVE_STRINGS_H)
  #include <strings.h>
#endif

#ifdef HAVE_INTTYPES_H
  #include <inttypes.h>
#else

  #define __WORDSIZE		NATIVE_WORD_SIZE

  #ifdef TYPE_INT8
    typedef signed   TYPE_INT8  int8_t;
    typedef unsigned TYPE_INT8  uint8_t;
  #endif
  #ifdef TYPE_INT16
    typedef signed   TYPE_INT16 int16_t;
    typedef unsigned TYPE_INT16 uint16_t;
  #endif
  #ifdef TYPE_INT32
    typedef signed   TYPE_INT32 int32_t;
    typedef unsigned TYPE_INT32 uint32_t;
  #endif
  #ifdef TYPE_INT64
    typedef signed   TYPE_INT64 int64_t;
    typedef unsigned TYPE_INT64 uint64_t;
  #endif

  #if (__WORDSIZE == 64)
    #define __PRI64_PREFIX	"l"
    #define __INT64_C(Cnt)	Cnt ## L
    #define __UINT64_C(Cnt)	Cnt ## UL
  #else /* (__WORDSIZE == 32) */
    #define __PRI64_PREFIX	"ll"
    #define __INT64_C(Cnt)	Cnt ## LL
    #define __UINT64_C(Cnt)	Cnt ## ULL
  #endif

  #define PRId8			"d"
  #define PRId16		"d"
  #define PRId32		"d"
  #define PRId64		__PRI64_PREFIX "d"

  #define PRIi8			"i"
  #define PRIi16		"i"
  #define PRIi32		"i"
  #define PRIi64		__PRI64_PREFIX "i"

  #define PRIo8			"o"
  #define PRIo16		"o"
  #define PRIo32		"o"
  #define PRIo64		__PRI64_PREFIX "o"

  #define PRIu8			"u"
  #define PRIu16		"u"
  #define PRIu32		"u"
  #define PRIu64		__PRI64_PREFIX "u"

  #define PRIx8			"x"
  #define PRIx16		"x"
  #define PRIx32		"x"
  #define PRIx64		__PRI64_PREFIX "x"

  #define PRIX8			"X"
  #define PRIX16		"X"
  #define PRIX32		"X"
  #define PRIX64		__PRI64_PREFIX "X"

  #define INT8_MIN		(-128)
  #define INT16_MIN		(-32767-1)
  #define INT32_MIN		(-2147483647-1)
  #define INT64_MIN		(-__INT64_C(9223372036854775807)-1)

  #define INT8_MAX		(127)
  #define INT16_MAX		(32767)
  #define INT32_MAX		(2147483647)
  #define INT64_MAX		(__INT64_C(9223372036854775807))

  #define UINT8_MAX		(255)
  #define UINT16_MAX		(65535)
  #define UINT32_MAX		(4294967295U)
  #define UINT64_MAX		(__UINT64_C(18446744073709551615))

#endif /* HAVE_INTTYPES_H */

#ifndef __INT8_FMTd__
  #define __INT8_FMTd__		"hhd"
#endif

#ifndef __INT8_FMTi__
  #define __INT8_FMTi__		"hhi"
#endif

#ifndef __UINT8_FMTo__
  #define __UINT8_FMTo__	"hho"
#endif

#ifndef __UINT8_FMTu__
  #define __UINT8_FMTu__	"hhu"
#endif

#ifndef __UINT8_FMTx__
  #define __UINT8_FMTx__	"hhx"
#endif

#ifndef __UINT8_FMTX__
  #define __UINT8_FMTX__	"hhX"
#endif

#ifndef __INT16_FMTd__
  #define __INT16_FMTd__	"hd"
#endif

#ifndef __INT16_FMTi__
  #define __INT16_FMTi__	"hi"
#endif

#ifndef __UINT16_FMTo__
  #define __UINT16_FMTo__	"ho"
#endif

#ifndef __UINT16_FMTu__
  #define __UINT16_FMTu__	"hu"
#endif

#ifndef __UINT16_FMTx__
  #define __UINT16_FMTx__	"hx"
#endif

#ifndef __UINT16_FMTX__
  #define __UINT16_FMTX__	"hX"
#endif

#ifndef __INT32_FMTd__
  #define __INT32_FMTd__	"d"
#endif

#ifndef __INT32_FMTi__
  #define __INT32_FMTi__	"i"
#endif

#ifndef __UINT32_FMTo__
  #define __UINT32_FMTo__	"o"
#endif

#ifndef __UINT32_FMTu__
  #define __UINT32_FMTu__	"u"
#endif

#ifndef __UINT32_FMTx__
  #define __UINT32_FMTx__	"x"
#endif

#ifndef __UINT32_FMTX__
  #define __UINT32_FMTX__	"X"
#endif

#if (__WORDSIZE == 64)

  #ifndef __INT64_FMTd__
    #define __INT64_FMTd__	"ld"
  #endif

  #ifndef __INT64_FMTi__
    #define __INT64_FMTi__	"li"
  #endif

  #ifndef __UINT64_FMTo__
    #define __UINT64_FMTo__	"lo"
  #endif

  #ifndef __UINT64_FMTu__
    #define __UINT64_FMTu__	"lu"
  #endif

  #ifndef __UINT64_FMTx__
    #define __UINT64_FMTx__	"lx"
  #endif

  #ifndef __UINT64_FMTX__
    #define __UINT64_FMTX__	"lX"
  #endif

  #ifndef __OFF_FMTo__
    #define __OFF_FMTo__	"lo"
  #endif

  #ifndef __OFF_FMTu__
    #define __OFF_FMTu__	"lu"
  #endif

  #ifndef __OFF_FMTx__
    #define __OFF_FMTx__	"lx"
  #endif

  #ifndef __OFF_FMTX__
    #define __OFF_FMTX__	"lX"
  #endif

  #ifndef __SIZE_FMTo__
    #define __SIZE_FMTo__	"lo"
  #endif

  #ifndef __SIZE_FMTu__
    #define __SIZE_FMTu__	"lu"
  #endif

  #ifndef __SIZE_FMTx__
    #define __SIZE_FMTx__	"lx"
  #endif

  #ifndef __SIZE_FMTX__
    #define __SIZE_FMTX__	"lX"
  #endif

#else /* (__WORDSIZE == 32) */

  #ifndef __INT64_FMTd__
    #define __INT64_FMTd__	"lld"
  #endif

  #ifndef __INT64_FMTi__
    #define __INT64_FMTi__	"lli"
  #endif

  #ifndef __UINT64_FMTo__
    #define __UINT64_FMTo__	"llo"
  #endif

  #ifndef __UINT64_FMTu__
    #define __UINT64_FMTu__	"llu"
  #endif

  #ifndef __UINT64_FMTx__
    #define __UINT64_FMTx__	"llx"
  #endif

  #ifndef __UINT64_FMTX__
    #define __UINT64_FMTX__	"llX"
  #endif

  #if (SIZEOF_OFF_T == 4)

    #ifndef __OFF_FMTo__
      #define __OFF_FMTo__	"lo"
    #endif

    #ifndef __OFF_FMTu__
      #define __OFF_FMTu__	"lu"
    #endif

    #ifndef __OFF_FMTx__
      #define __OFF_FMTx__	"lx"
    #endif

    #ifndef __OFF_FMTX__
      #define __OFF_FMTX__	"lX"
    #endif

  #endif

  #if (SIZEOF_OFF_T == 8)

    #ifndef __OFF_FMTo__
      #define __OFF_FMTo__	"llo"
    #endif

    #ifndef __OFF_FMTu__
      #define __OFF_FMTu__	"llu"
    #endif

    #ifndef __OFF_FMTx__
      #define __OFF_FMTx__	"llx"
    #endif

    #ifndef __OFF_FMTX__
      #define __OFF_FMTX__	"llX"
    #endif

  #endif

  #if (SIZEOF_SIZE_T == 4)

    #ifndef __SIZE_FMTo__
      #define __SIZE_FMTo__	"o"
    #endif

    #ifndef __SIZE_FMTu__
      #define __SIZE_FMTu__	"u"
    #endif

    #ifndef __SIZE_FMTx__
      #define __SIZE_FMTx__	"x"
    #endif

    #ifndef __SIZE_FMTX__
      #define __SIZE_FMTX__	"X"
    #endif

  #endif

  #if (SIZEOF_SIZE_T == 8)

    #ifndef __SIZE_FMTo__
      #define __SIZE_FMTo__	"llo"
    #endif

    #ifndef __SIZE_FMTu__
      #define __SIZE_FMTu__	"llu"
    #endif

    #ifndef __SIZE_FMTx__
      #define __SIZE_FMTx__	"llx"
    #endif

    #ifndef __SIZE_FMTX__
      #define __SIZE_FMTX__	"llX"
    #endif

  #endif

#endif

#define INT8_FMTd		__INT8_FMTd__
#define INT8_FMTi		__INT8_FMTi__
#define UINT8_FMTo		__UINT8_FMTo__
#define UINT8_FMTu		__UINT8_FMTu__
#define UINT8_FMTx		__UINT8_FMTx__
#define UINT8_FMTX		__UINT8_FMTX__

#define INT16_FMTd		__INT16_FMTd__
#define INT16_FMTi		__INT16_FMTi__
#define UINT16_FMTo		__UINT16_FMTo__
#define UINT16_FMTu		__UINT16_FMTu__
#define UINT16_FMTx		__UINT16_FMTx__
#define UINT16_FMTX		__UINT16_FMTX__

#define INT32_FMTd		__INT32_FMTd__
#define INT32_FMTi		__INT32_FMTi__
#define UINT32_FMTo		__UINT32_FMTo__
#define UINT32_FMTu		__UINT32_FMTu__
#define UINT32_FMTx		__UINT32_FMTx__
#define UINT32_FMTX		__UINT32_FMTX__

#define INT64_FMTd		__INT64_FMTd__
#define INT64_FMTi		__INT64_FMTi__
#define UINT64_FMTo		__UINT64_FMTo__
#define UINT64_FMTu		__UINT64_FMTu__
#define UINT64_FMTx		__UINT64_FMTx__
#define UINT64_FMTX		__UINT64_FMTX__

#define OFF_FMTo		__OFF_FMTo__
#define OFF_FMTu		__OFF_FMTu__
#define OFF_FMTx		__OFF_FMTx__
#define OFF_FMTX		__OFF_FMTX__

#define SIZE_FMTo		__SIZE_FMTo__
#define SIZE_FMTu		__SIZE_FMTu__
#define SIZE_FMTx		__SIZE_FMTx__
#define SIZE_FMTX		__SIZE_FMTX__

#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>

#ifdef HAVE_STDLIB_H
  #include <stdlib.h>
#endif

#ifndef EXIT_SUCCESS
  #define EXIT_SUCCESS          0
  #define EXIT_FAILURE          1
#endif

#ifdef HAVE_LOCALE_H
  #include <locale.h>

  #ifdef HAVE_LANGINFO_H
    #include <langinfo.h>
  #endif
#endif

#ifdef HAVE_UNISTD_H
  #include <unistd.h>
#endif

#include "getopt.h"

#ifndef HAVE_STDLIB_H
  #ifdef HAVE_MALLOC_H
    #include <malloc.h>
  #endif
#endif

#ifndef HAVE_STRCASECMP
  #ifdef HAVE_STRICMP
    #define strcasecmp          stricmp
  #endif
#endif

#ifndef HAVE_STRNCASECMP
  #ifdef HAVE_STRNICMP
    #define strncasecmp         strnicmp
  #endif
#endif

#ifndef HAVE_VSNPRINTF
/* No system vsnprintf and vasprintf: Use the one implemented in libiberty. */
extern int vsnprintf(char *s, size_t n, const char *format, va_list args);
extern int vasprintf(char **result, const char *format, va_list args);
#endif

#ifndef HAVE_SNPRINTF
/* No system snprintf: Use the one implemented in libiberty. */
extern int snprintf(char *s, size_t n, const char *format, ...);
#endif

#ifndef HAVE_STRNDUP
/* No system strndup: Use the one implemented in libiberty. */
extern char *strndup(const char *s, size_t n);
#endif

#ifndef HAVE_STRVERSCMP
/* No system strverscmp: Use the one implemented in libiberty. */
extern int strverscmp(const char *s1, const char *s2);
#endif

#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#ifdef HAVE_DOS_BASED_FILE_SYSTEM
  #define UNIX_PATH_CHAR        '/'
  #define PATH_SEPARATOR_CHAR   '\\'
  #define PATH_SEPARATOR_STR    "\\"
#else
  #define PATH_SEPARATOR_CHAR   '/'
  #define PATH_SEPARATOR_STR    "/"
#endif

#define GPUTILS_COPYRIGHT_STRING        "Copyright (c) 1998-2016 gputils project"

#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
  #define GPUTILS_GCC_STR(s)            #s
  #define GPUTILS_GCC_JOINSTR(x,y)      GPUTILS_GCC_STR(x ## y)
  #define GPUTILS_GCC_DO_PRAGMA(x)      _Pragma(#x)
  #define GPUTILS_GCC_PRAGMA(x)         GPUTILS_GCC_DO_PRAGMA(GCC diagnostic x)

  #if ((__GNUC__ * 100) + __GNUC_MINOR__) < 406
    /* needed for older GCCs that accept only top of the file diagnostic pragmas */
    #define GPUTILS_GCC_DIAG_TOP(x)     GPUTILS_GCC_PRAGMA(ignored GPUTILS_GCC_JOINSTR(-W,x))
    #define GPUTILS_GCC_DIAG_OFF(x)
    #define GPUTILS_GCC_DIAG_ON(x)
  #else
    #define GPUTILS_GCC_DIAG_TOP(x)

    #define GPUTILS_GCC_DIAG_OFF(x) \
                GPUTILS_GCC_PRAGMA(push) \
                GPUTILS_GCC_PRAGMA(ignored GPUTILS_GCC_JOINSTR(-W,x))

    #define GPUTILS_GCC_DIAG_ON(x)      GPUTILS_GCC_PRAGMA(pop)
  #endif
#else
  #define GPUTILS_GCC_DIAG_TOP(x)
  #define GPUTILS_GCC_DIAG_OFF(x)
  #define GPUTILS_GCC_DIAG_ON(x)
#endif

#endif /* __STDHDR_H__ */
