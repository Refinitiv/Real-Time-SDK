/*
//
//|---------------------------------------------------------------
//|                Copyright (C) 1995 Reuters,                  --
//|                                                             --
//|         1400 Kensington Road, Oak Brook, IL. 60521          --
//|                                                             --
//| All rights reserved. Duplication or distribution prohibited --
//|---------------------------------------------------------------
//
//  created:     May 9, 1995
//  version:
//
//  description:
//  This header file is meant to define platform specific
//  definitions needed for compiles.
//
//  NOTE : This header file MUST use old style comments since
//  it is used by K&R, ANSI, and C++ compilers.
//
//
//  Definitions                 Descriptions
//-------------------------------------------------------------------------
//	DEV_UNIX                    It is a UNIX machine. As apposed to
//                              NT, VMS.
//  DEV_SVR4                    System V Release 4 definitions.
//                              DEV_SVR4 implies all DEV_SVR4_* definitions.
//	DEV_SVR4_SIGNALS            System V Release 4 signal definitions.
//	DEV_USE_O_NONBLOCK          Use O_NONBLOCK when setting non blocking
//                              IO on a file descriptor.
//                              NOTE- DEV_SVR4 implies DEV_USE_O_NONBLOCK.
//	DEV_QUAD_32_ALIGN           Quad 32 alignment.
//	DEV_IOCTL_ENABLE_ASYNC      Use ioctl(FIOASYNC) to enable
//                              asynchronous reading
//	DEV_FCNTL_ENABLE_ASYNC      Use fcntl(F_GETFL) to enbable
//                              asynchronous reading
//	DEV_SELECT_BITMAP_CAST      Some machines need typecasting for the
//                              bitmaps used in the select calls.
//  DEV_LITTLE_ENDIAN           The machine uses a little endian architecture.
//                              Byte swapping is needed for correct network
//                              order.
//	DEV_HAS_POLL				The poll() system call exists.
//	DEV_SH_LIB_EXPORT			If a shared library is being built, should
//								this function/data be exported? You put
//								this macro in front of the function declaration.
//	DEV_SH_LIB_IMPORT			If a shared library is being built, should
//								a function/data be imported? You put this
//								macro in front of the function declaration.
//
*/

#ifndef __rtr_dev_platform
#define __rtr_dev_platform


	/* Default to nothing. Redefine if needed. */
#define DEV_SELECT_BITMAP_CAST
#define STATIC_THREAD_PRIVATE
#define DEV_SH_LIB_EXPORT
#define DEV_THREAD_LOCAL
#define DEV_SH_LIB_IMPORT


#if defined(DLL_IMPORT) 
#undef DLL_IMPORT	/* also properly defined in ssl.h */
#endif
#define DLL_IMPORT
#define DLL_EXPORT



	/* Definitions for Sun hardware. */

#if defined(sun4) || defined(i86pc)
#undef DEV_THREAD_LOCAL
#ifdef _RDEV_NO_TLS_
#define DEV_THREAD_LOCAL
#else
#define DEV_THREAD_LOCAL __thread
#endif

#define DEV_UNIX
#ifdef __sparc
#define DEV_QUAD_32_ALIGN
#endif
#define DEV_FCNTL_ENABLE_ASYNC
#define DEV_HAS_POLL

		/* Definitions for Solaris 1 */
#ifdef sun4_SunOS_41X
#define DEV_SVR4_SIGNALS
#endif

		/* Definitions for Solaris 2 */
#ifdef sun4_SunOS_5X 
#define DEV_SVR4
#endif

#ifdef __GNUC__
#define __INCLUDE_TEMPLATE_IMPL__
#endif

#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#endif



	/* Definitions for HP hardware. */
#ifdef hp

#define DEV_UNIX
#define DEV_SVR4
#define DEV_QUAD_32_ALIGN
#define DEV_IOCTL_ENABLE_ASYNC
#undef DEV_SELECT_BITMAP_CAST
#define DEV_HAS_POLL

		/* Definitions for hp-ux versions 9.x */
#ifdef hp_HPUX_09X
#define DEV_SELECT_BITMAP_CAST (int *)
#endif

		/* Definitions for hp-ux versions 10.x */
#ifdef hp_HPUX_10X
#define DEV_SELECT_BITMAP_CAST (int *)
#endif

		/* Definitions for hp-ux versions 11.x */
#ifdef hp_HPUX_11X
#include	<ctype.h>
#include	<string.h>
#include	<unistd.h>
#define DEV_SELECT_BITMAP_CAST (fd_set *)
#define HP_ACC
#define	_SYS_GETHOSTNAME_DEFINED
#define __INCLUDE_TEMPLATE_IMPL__
#endif
#endif



	/* Definitions for Ibmrs hardware. */
#ifdef ibmrs

#define DEV_UNIX
#define DEV_SVR4
#define DEV_QUAD_32_ALIGN
#define DEV_IOCTL_ENABLE_ASYNC
#define DEV_HAS_POLL

#define __INCLUDE_TEMPLATE_IMPL__

#if DEV_COMPILER_SUITE == cset
#ifndef __CSET__
#define __CSET__
#endif
#endif

		/* Definitions for aix 3.2 */
#ifdef ibmrs_AIX_32
#endif

#endif
	
	/* Definitions for Linux */
#ifdef Linux
#undef DEV_THREAD_LOCAL

#ifdef _RDEV_NO_TLS_
#define DEV_THREAD_LOCAL
#else
#define DEV_THREAD_LOCAL __thread
#endif

#define DEV_UNIX 
#define DEV_SVR4
#define DEV_HAS_POLL
#define __INCLUDE_TEMPLATE_IMPL__
#if defined(x86_Linux_S9X) || defined(x86_Linux_4X)
#include <unistd.h>		/* SuSE and AS4x defines gethostname */
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#define	_SYS_GETHOSTNAME_DEFINED
#endif
#endif

	/* Definitions for x86_Linux_2X and subsequent Linux platforms*/
#if defined(Linux)
#ifndef MEM_FUNCS_FOR_LINUX
#define MEM_FUNCS_FOR_LINUX
#endif
#endif


	/* Definitions for alpha_OSF1 hardware. */
#ifdef alpha_OSF1

#define DEV_UNIX
#define DEV_SVR4
#define DEV_LITTLE_ENDIAN
#define DEV_IOCTL_ENABLE_ASYNC
#define DEV_FCNTL_ENABLE_ASYNC
#define DEV_HAS_POLL

#endif

#ifdef _MSC_VER
#define __INCLUDE_TEMPLATE_IMPL__
#endif

	/* Definitions for MS-Windows Platforms (Win3.1) (WinNT) */
#if defined( _WIN16 ) || defined( _WIN32 )

typedef	char * 	caddr_t;

#define DEV_LITTLE_ENDIAN

#if defined(_WIN16)  
#define	DEV_SMALL_INT
#endif

#if defined(_WIN32)
	#undef DEV_THREAD_LOCAL
	#ifdef _RDEV_NO_TLS_
	#define DEV_THREAD_LOCAL
	#else
	#define DEV_THREAD_LOCAL        __declspec(thread)
	#endif

	#undef	STATIC_THREAD_PRIVATE
	#ifdef _RDEV_NO_TLS_
	#define STATIC_THREAD_PRIVATE
	#else
	#define STATIC_THREAD_PRIVATE	__declspec(thread)
	#endif

	#undef	DLL_IMPORT				
	#define DLL_IMPORT				__declspec(dllimport)

	#undef	DLL_EXPORT
	#define DLL_EXPORT				__declspec(dllexport)

#if defined(DEV_SH_LIB)
#undef DEV_SH_LIB_EXPORT
#undef DEV_SH_LIB_IMPORT
#define DEV_SH_LIB_EXPORT	DLL_EXPORT
#define DEV_SH_LIB_IMPORT	DLL_IMPORT
#endif

#endif

#endif


	/* Definitions for VAX/VMS hardware. */
#ifdef VMS

#define DEV_LITTLE_ENDIAN
#define DEV_QUAD_32_ALIGN
#undef DEV_SELECT_BITMAP_CAST
#define DEV_SELECT_BITMAP_CAST	(int *)

#endif /* VMS */


#ifndef MEM_FUNCS_FOR_LINUX
#if defined(_WIN32) || defined(DEV_SVR4) && !defined(_KERNEL)
#ifndef bcopy
#define bcopy(a,b,c)    memmove(b,a,c)
#endif
#ifndef bcmp
#define bcmp(a,b,c)     memcmp(a,b,c)
#endif
#ifndef bzero
#define bzero(a,b)      memset(a,0,b)
#endif
#endif
#endif /* MEM_FUNCS_FOR_LINUX */

#endif
