/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

/* Decsription:
 * This header file defines a macro that implements that fastest 
 * unsigned long division by 10 including the remainder. It may be
 * different per operating system.
 * __rtr_u32div10(value,quo,rem)
 * 		value - unsigned long value to divide by 10.
 * 		quo - unsigned long quotient of result.
 * 		rem - unsigned long remainder of result.
 *
 * The above macro uses the ones defined below depending on OS.
 *
 * Division by 10, let the compiler optimize.
 * __rtr_comp_u32div10(value,quo,rem)
 * 		value - unsigned long value to divide by 10.
 * 		quo - unsigned long quotient of result.
 * 		rem - unsigned long remainder of result.
 *
 * Division by 10 using decimal in c-code.
 * __rtr_hand_u32div10(value,quo,rem)
 * 		value - unsigned long value to divide by 10.
 * 		quo - unsigned long quotient of result.
 * 		rem - unsigned long remainder of result.
 *
 */

#ifndef __RTR_DIVIDE10_H
#define __RTR_DIVIDE10_H

#include "rtr/os.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Compiler optimized unsigned division by 10.
 * value - unsigned long value to divide by 10.
 * quo - quotient of result.
 * rem - remainder of result.
 */
#define __rtr_comp_u32div10( value, quo, rem ) \
	quo = value/10; rem = value - quo * 10


 /*
 * This header file defines macros that implements division by
 * 10 using a multiply and a shift. This is much more efficient than
 * division.
 * It works by using decimal arithmetic in binary. In effect a
 * 32 bit division can be turned into a 64 bit multiply and shift.
 * BASE10_MULTIPLIER represents the binary 1/10, after shift and
 * rounding.
 */
#define BASE10_MULTIPLIER   0xcccccccdUL
#define BASE10_POSTSHIFT    3


#ifdef RTR_I64

/* By hand optimized unsigned division by 10.
 * value - unsigned long value to divide by 10.
 * quo - quotient of result.
 * rem - remainder of result.
	quo = (unsigned long)((((unsigned RTR_LONGLONG)value * BASE10_MULTIPLIER) >> BASE10_POSTSHIFT) + 32);\
 */
#define __rtr_hand_u32div10( value, quo, rem ) \
	quo = (unsigned long)((unsigned RTR_LONGLONG)(((unsigned RTR_LONGLONG)value * (unsigned RTR_LONGLONG)BASE10_MULTIPLIER) >> (BASE10_POSTSHIFT + 32)));\
	rem = value - quo * 10
#endif


#define __rtr_u32div10(___value, ___quo, ___rem) __rtr_hand_u32div10(___value, ___quo, ___rem)


#ifndef __rtr_u32div10
#define __rtr_u32div10(___value, ___quo, ___rem) __rtr_comp_u32div10( ___value, ___quo, ___rem )
#endif



#ifdef __cplusplus
};
#endif

#endif
