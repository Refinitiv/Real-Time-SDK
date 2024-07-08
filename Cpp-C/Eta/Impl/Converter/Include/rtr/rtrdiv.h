/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020 LSEG. All rights reserved.                
 *|-----------------------------------------------------------------------------
 *
 * Decsription:
 * This header file defines a macro that implements that fastest 
 * unsigned long division by 10 including the remainder. It may be
 * different per operating system.
 * __rtr_u32div(value,quo,rem)
 * 		value - unsigned long value to divide by 10.
 * 		quo - unsigned long quotient of result.
 * 		rem - unsigned long remainder of result.
 *
 * The above macro uses the ones defined below depending on OS.
 *
 * Division by 10, let the compiler optimize.
 * __rtr_comp_u32div(value,quo,rem)
 * 		value - unsigned long value to divide by 10.
 * 		quo - unsigned long quotient of result.
 * 		rem - unsigned long remainder of result.
 *
 * Division by 10 using decimal in c-code.
 * __rtr_hand_u32div(value,quo,rem)
 * 		value - unsigned long value to divide by 10.
 * 		quo - unsigned long quotient of result.
 * 		rem - unsigned long remainder of result.
 *
 * Division by 10 using decimal in assembler.
 * __rtr_asm_u32div(value,quo,rem)
 * 		value - unsigned long value to divide by 10.
 * 		quo - unsigned long quotient of result.
 * 		rem - unsigned long remainder of result.
 *
 */

#ifndef __RTR_DIVIDE_H
#define __RTR_DIVIDE_H

#include "rtr/os.h"

#ifdef __cplusplus
extern "C" {
#endif



 /*
 * This header file defines macros that implements division by
 * 10 using a multiply and a shift. This is much more efficient than
 * division.
 * It works by using decimal arithmetic in binary. In effect a
 * 32 bit division can be turned into a 64 bit multiply and shift.
 * BASE10_MULTIPLIER represents the binary 1/10, after shift and
 * rounding.
 *
 * In some cases there needs to be some correction (noted below).
 * In these cases all you have to do is check the remainder and
 * see if it is less than the dividend. If not then correct.
 */

#define RTR_MAX_U32_EXP 9
#define RTR_MAX_U64_EXP 14

static unsigned RTR_LONGLONG __rtr_multTable[RTR_MAX_U32_EXP+1] =
{
	0x1UL,
	0xCCCCCCCDUL,
	0x51EB851FUL,
	0x83126E98UL,
	0xD1B71759UL,
	0x53E2D624UL,	/* Need Correction */
	0x8637BD06UL,
	0xD6BF94D6UL,
	0xABCC7712UL,
	0x44B82FA1UL	/* Need Correction */
};

static unsigned long __rtr_shiftTable[RTR_MAX_U32_EXP+1] =
{
	0x00,
	0x03,
	0x05,
	0x09,
	0x0D,
	0x0F,
	0x13,
	0x17,
	0x1A,
	0x1C
};

static unsigned long __rtr_powTable[RTR_MAX_U32_EXP+1] =
{
	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000
};

#ifdef  x86_Linux_4X

static rtrUInt64 __rtr_dpowTable[RTR_MAX_U64_EXP+1] =
{
	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000,
	10000000000ULL,
	100000000000ULL,
	1000000000000ULL,
	10000000000000ULL,
	100000000000000ULL
};

#else

static rtrUInt64 __rtr_dpowTable[RTR_MAX_U64_EXP+1] =
{
	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000,
	10000000000,
	100000000000,
	1000000000000,
	10000000000000,
	100000000000000

};

#endif

/* Compiler optimized unsigned division by a negative exponent.
 * value - unsigned long value to divide.
 * exponent - negative of the exponent to divide by (e.g. 2 = 10, 4 = 1000)
 * quo - quotient of result.
 * rem - remainder of result.
 */
#define __rtr_comp_u32div( ___value, ___exponent, ___quo, ___rem ) \
{ \
	___quo = ___value/(__rtr_powTable[___exponent]); \
	___rem = ___value - ___quo * (__rtr_powTable[___exponent]); \
}



#ifdef RTR_I64

/* Compiler optimized unsigned division by a negative exponent.
 * value - unsigned long value to divide.
 * exponent - negative of the exponent to divide by (e.g. 2 = 10, 4 = 1000)
 * quo - quotient of result.
 * rem - remainder of result.
 */
#define __rtr_hand_u32div( ___value, ___exponent, ___quo, ___rem ) \
{ \
	___quo = (unsigned long)((((unsigned RTR_LONGLONG)___value * __rtr_multTable[___exponent]) >> (__rtr_shiftTable[___exponent] + 32) ));\
	___rem = ___value - ___quo * (__rtr_powTable[___exponent]); \
	if (___rem >= __rtr_powTable[___exponent]) \
	{ \
		___quo--; \
		___rem = ___value - ___quo * (__rtr_powTable[___exponent]); \
	} \
}

#endif


#ifdef Linux

#ifdef COMPILE_64BITS
#define __rtr_asm_u32mul(___value, ___quo, ___mmmult, ___mmmshift) \
{ \
	__asm__ ("imul %%rax,%%rdx;shr %%cl,%%rdx" \
					: "=d" (___quo) \
					: "d" (___value), \
					  "A" (___mmmult), \
					  "c" (___mmmshift + 32) ); \
}
#else
#define __rtr_asm_u32mul(___value, ___quo, ___mmmult, ___mmmshift) \
{ \
	__asm__ ("mull %%ebx;shr %%cl,%%edx" \
					: "=d" (___quo) \
					: "b" (___value), \
					  "A" (___mmmult), \
					  "c" (___mmmshift) ); \
}
#endif
					  
/* Assembler optimized unsigned division by a negative exponent.
 * value - unsigned long value to divide.
 * exponent - negative of the exponent to divide by (e.g. 2 = 10, 4 = 1000)
 * quo - quotient of result.
 * rem - remainder of result.
 */
#define __rtr_asm_u32div(___value, ___exponent, ___quo, ___rem) \
{ \
	unsigned RTR_LONGLONG __mymult = __rtr_multTable[___exponent];\
	unsigned long __myshift = __rtr_shiftTable[___exponent];\
	__rtr_asm_u32mul(___value,___quo,__mymult,__myshift); \
	___rem = ___value - ___quo * (__rtr_powTable[___exponent]); \
	if (___rem >= __rtr_powTable[___exponent]) \
	{ \
		___quo--; \
		___rem = ___value - ___quo * (__rtr_powTable[___exponent]); \
	} \
}
#endif


#ifdef _WIN32

#ifndef _WIN64

#define __rtr_asm_u32mul(___value, ___quo, ___mmmult, ___mmmshift) __asm \
{ \
	__asm mov esi, dword ptr ___mmmult \
	__asm mov eax, dword ptr ___value \
	__asm mul esi \
	__asm mov ecx, dword ptr ___mmmshift \
	__asm shr edx, cl \
	__asm mov dword ptr ___quo,edx \
}

/* Assembler optimized unsigned division by a negative exponent.
 * value - unsigned long value to divide.
 * exponent - negative of the exponent to divide by (e.g. 2 = 10, 4 = 1000)
 * quo - quotient of result.
 * rem - remainder of result.
 */
#define __rtr_asm_u32div(___value, ___exponent, ___quo, ___rem) \
{ \
	unsigned RTR_LONGLONG __mymult = __rtr_multTable[___exponent];\
	unsigned long __myshift = __rtr_shiftTable[___exponent];\
	__rtr_asm_u32mul(___value, ___quo, __mymult, __myshift); \
	___rem = ___value - ___quo * (__rtr_powTable[___exponent]); \
	if (___rem >= __rtr_powTable[___exponent]) \
	{ \
		___quo--; \
		___rem = ___value - ___quo * (__rtr_powTable[___exponent]); \
	} \
}

#endif

#endif



#if defined(Linux)
#ifdef COMPILE_64BITS
/* Hand multiply optimized unsigned division is fastest for this OS. */
#define __rtr_u32div(___value, ___exponent, ___quo, ___rem) \
	__rtr_hand_u32div(___value, ___exponent, ___quo, ___rem)
#else
/* Assembler optimized unsigned division is fastest for this OS. */
#define __rtr_u32div(___value, ___exponent, ___quo, ___rem) \
	__rtr_asm_u32div(___value, ___exponent, ___quo, ___rem)
#endif
#endif


#ifdef _WIN64
/* Hand multiply optimized unsigned division is fastest for this OS. */
#define __rtr_u32div(___value, ___exponent, ___quo, ___rem) \
	__rtr_hand_u32div(___value, ___exponent, ___quo, ___rem)
#elif defined (_WIN32)
/* Assembler optimized unsigned division is fastest for this OS. */
#define __rtr_u32div(___value, ___exponent, ___quo, ___rem) \
	__rtr_asm_u32div(___value, ___exponent, ___quo, ___rem)
#endif


#if defined(SOLARIS2)
/* Hand multiply optimized unsigned division is fastest for this OS. */
#define __rtr_u32div(___value, ___exponent, ___quo, ___rem) \
	__rtr_hand_u32div( ___value, ___exponent, ___quo, ___rem )
#endif


#ifndef __rtr_u32div
#define __rtr_u32div(___value, ___exponent, ___quo, ___rem) \
	__rtr_comp_u32div( ___value, ___exponent, ___quo, ___rem )
#endif



#ifdef __cplusplus
};
#endif

#endif
