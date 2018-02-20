/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

/* This header file contains platform specific definitions for
   various sytem types and mechanisms.  It is used for 
   cross-platform development and allows compiling 
   code on multiple operating systems and system
   architectures without maintaining multiple code
   bases.  Definitions contained in this file can
   be used and are leveraged by multiple code bases
   and products.  Please use caution when modifying, 
   adding, or removing anything defined in this file */

/*
 *	RTR_I64			- Syntax for 64 bit signed integer
 *	unsigned RTRLLD	- Syntax for 64 bit unsigned interger
 *	RTR_LLD			- Signed 64 bit integer specifier for xprintf()
 *	RTR_LLU			- Unsigned 64 bit integer specifier for xprintf()
 *	RTR_LLX			- Hex 64 bit integer specifier for xprintf()
 *
 *	RTR_BIG_ENDIAN | RTR_LITTLE_ENDIAN - Endianess of machine
 *
 *	RTR_API_EXPORT	- Shared library function export declaration
 *	RTR_API_IMPORT	- Shared library function import declaration
 *
 *	RTR_C_INLINE	- C Function inline specification
 *
 */

#ifndef __RTR_OS_H
#define __RTR_OS_H


#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
	#include	<malloc.h>
#else
	#include	<alloca.h>
#endif


/* Type definitions necessary for Platform Independence  */

/* NOTE: This area contains common type definitions to allow for 
 * platform independence of types.  As future platforms and
 * operating systems are added this file may evolve
 */


#define RTR_LITTLE_ENDIAN
#define RTR_DWORD_ALIGN_BITS 0x3


/* Platform specific definitions for Linux 32 and 64-bit */
#ifdef Linux
	#define RTR_I64 long long
	#define RTR_ULL(___val) (___val##ULL)
	#define RTR_LL(___val) (___val##LL)

	#define RTR_API_EXPORT
	#define RTR_API_IMPORT

	#define RTR_C_INLINE static inline

#if __GNUC__ == 3
	#define RTR_C_ALWAYS_INLINE static __attribute__((always_inline)) inline
	#if !defined(COMPILE_64BITS) && (__GNUC_MINOR__ > 3)
		#define RTR_FASTCALL __attribute((fastcall))
	#else
		#define RTR_FASTCALL
	#endif
#elif __GNUC__ >= 4
	#define RTR_C_ALWAYS_INLINE static __attribute__((always_inline)) inline
	#if !defined(COMPILE_64BITS)
		#define RTR_FASTCALL __attribute((fastcall))
	#else
		#define RTR_FASTCALL
	#endif
#else
	#define RTR_C_ALWAYS_INLINE static inline
	#define RTR_FASTCALL
#endif

	#define RTR_LLD "%lld"
	#define RTR_LLU "%llu"
	#define RTR_LLX "%llx"

#define rtrLikely(expr) __builtin_expect((expr), 1)
#define rtrUnlikely(expr) __builtin_expect((expr), 0)

#endif	/* #ifdef Linux */


/* Platform specific definitions for Windows 32 and 64-bit */
#ifdef WIN32
#ifndef _WIN32
#define _WIN32
#endif
#endif

#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#endif

#ifdef WIN32
	#define RTR_I64 __int64
	#define RTR_ULL(___val) (___val##Ui64)
	#define RTR_LL(___val) (___val##i64)

	
	#ifndef WINVER
	#define WINVER			0x0400
	#endif

	#ifndef _WIN32_WINNT
	#define _WIN32_WINNT	WINVER
	#endif

	#define RTR_API_EXPORT	__declspec(dllexport)
	#define RTR_API_IMPORT	__declspec(dllimport)

	#define RTR_C_INLINE static __inline
	#define RTR_C_ALWAYS_INLINE static __forceinline
	#define RTR_FASTCALL __fastcall

	#define RTR_LLD "%I64d"
	#define RTR_LLU "%I64u"
	#define RTR_LLX "%I64X"

#define rtrLikely(expr) (expr)
#define rtrUnlikely(expr) (expr)

#endif	/* #ifdef WIN32 */

	/* Old macro definitions */
#define RTR_LONGLONG RTR_I64
#define RTR_LONGLONG_SPEC RTR_LLD
#define RTR_ULONGLONG_SPEC RTR_LLU



/* RTR Data Types */
typedef signed char rtrInt8;
typedef unsigned char rtrUInt8;

typedef short rtrInt16;
typedef unsigned short rtrUInt16;

typedef int rtrInt32;
typedef unsigned int rtrUInt32;

typedef RTR_LONGLONG rtrInt64;
typedef unsigned RTR_LONGLONG rtrUInt64;

typedef float rtrFloat; 
typedef double rtrDouble;

/** 
 * @brief RwfBuffer type
 * 
 * Used by transport layer and data encoder/decoder
 */
typedef struct
{
	rtrUInt32	length;		/*!< The length of the buffer. */
	char		*data;		/*!< The actual data buffer. */
} RwfBuffer;


/**
 * @brief Clears an RwfBuffer 
 * @see RwfBuffer
 */

RTR_C_INLINE void rwfClearBuffer(RwfBuffer *pBuffer)
{
	pBuffer->length = 0;
	pBuffer->data = 0;
}

typedef rtrUInt8	RwfBool;



#ifdef __cplusplus
}
#endif


#endif	/* #ifndef __RTR_OS_H */
