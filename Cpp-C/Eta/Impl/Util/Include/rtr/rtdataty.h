/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_DATA_TYPES
#define __RTR_DATA_TYPES

// Set default flags if none have been set.
// This is for current Sun/IBMRS/HP configurations
#ifndef SHORT_8_BIT
#ifndef SHORT_16_BIT
#define SHORT_16_BIT
#endif
#endif

#ifndef INT_16_BIT
#ifndef INT_32_BIT
#define INT_32_BIT
#endif
#endif

/* presume LP64 model if COMPILE_64BITS is set */
#ifdef COMPILE_64BITS

#ifndef LONG_64_BIT
#define LONG_64_BIT
#endif
#ifndef PTR_64_BIT
#define PTR_64_BIT
#endif

#undef LONG_32_BIT
#undef PTR_32_BIT
/* assume ILP32 model if COMPILE_64BITS is not defined */
#else
#ifndef LONG_32_BIT
#define LONG_32_BIT
#endif
#ifndef PTR_32_BIT
#define PTR_32_BIT
#endif
#undef LONG_64_BIT
#undef PTR_64_BIT
#endif

// END SETTING DEFAULTS




#ifdef SHORT_8_BIT
	typedef unsigned short u_8;
	typedef short s_8;
#endif

#ifdef SHORT_16_BIT
	typedef unsigned char u_8;
	typedef char s_8;
	typedef unsigned short u_16;
	typedef short s_16;
#endif

#ifdef INT_16_BIT
#ifndef SHORT_16_BIT
	typedef unsigned int u_16;
	typedef int s_16;
#endif
#endif

#ifdef INT_32_BIT
	typedef unsigned int u_32;
	typedef int s_32;
#endif

#ifdef LONG_32_BIT
#ifndef INT_32_BIT
	typedef unsigned long u_32;
	typedef long s_32;
#endif
#endif



#ifdef LONG_64_BIT
#endif

#if defined(LONG_64_BIT) || defined(PTR_64_BIT)
	typedef long intptr_type;
	/* typedef unsigned long uintptr_type; */
#else
	typedef int intptr_type;
	/* typedef unsigned int uintptr_type; */
#endif

#endif
