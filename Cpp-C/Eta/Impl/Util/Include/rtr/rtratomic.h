/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//  description:
//	Atomic Operations
//
//	This header file defines the macros to be used to perform atomic
//	operations on variables of type long. An atomic operation guarantees
//	that only one thread will perform the operation at a time.
//
//		These versions don't return any values.
//	RTR_ATOMIC_INCREMENT(var) - atomically increment, by 1, the variable.
//	RTR_ATOMIC_DECREMENT(var) - atomically decrement, by 1, the variable.
//	RTR_ATOMIC_ADD(var,incr) - atomically add incr to the variable.
//	RTR_ATOMIC_SET(var,newval) - atomically set var to newval.
//
//		These versions return the new value of the variable.
//	RTR_ATOMIC_INCREMENT_RET(var) - atomically increment, by 1, the variable.
//	RTR_ATOMIC_DECREMENT_RET(var) - atomically decrement, by 1, the variable.
//	RTR_ATOMIC_ADD_RET(var,incr) - atomically add incr to the variable.
//
//		These versions return the old value of the variable.
//	RTR_ATOMIC_SET_RETOLD(var,newval) - atomically set var to new value.
//
//
//  Compare and swap routines:
//  CAS(pvar,compval,newval) 
//    if (*pvar == compval)
//    {
//      origpval = *pvar;
//      *pvar = newval;
//      return origpval;
//    }
//    else
//      return *pvar
//    
//  RTR_ATOMIC_COMPARE_AND_SWAP(pvar,compval,newval) - CAS for 32 bit values.
//	RTR_ATOMIC_COMPARE_AND_SWAPPTR(ppvar,pcompval,pnewval) - CAS for pointers.
//


#ifndef __rtratomic_h
#define __rtratomic_h

#include "rtr/rtrdefs.h"
#include "rtr/os.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _WIN32
#include <windows.h>

typedef long rtr_atomic_val;


	/**********************************************/
	/* Windows atomic operations VC7 and above    */
	/**********************************************/

typedef _int64 rtr_atomic_val64;

extern LONG  __cdecl _InterlockedIncrement(LONG volatile *Addend);
extern LONG  __cdecl _InterlockedDecrement(LONG volatile *Addend);
extern LONG  __cdecl _InterlockedCompareExchange(LONG volatile *Dest, LONG Exchange, LONG Comp);
extern LONG  __cdecl _InterlockedExchange(LONG volatile *Dest, LONG Value);
extern LONG  __cdecl _InterlockedExchangeAdd(LONG volatile *Addend, LONG Value);

extern LONGLONG  __cdecl _InterlockedCompareExchange64(LONGLONG volatile *Addend, LONGLONG Exchange, LONGLONG Comparand);

#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#pragma intrinsic (_InterlockedCompareExchange)
#pragma intrinsic (_InterlockedExchange)
#pragma intrinsic (_InterlockedExchangeAdd)

#define RTR_ATOMIC_INCREMENT(___var)		(void)_InterlockedIncrement(&___var)
#define RTR_ATOMIC_DECREMENT(___var)		(void)_InterlockedDecrement(&___var)
#define RTR_ATOMIC_ADD(___var,___incr)		(void)_InterlockedExchangeAdd(&___var,___incr)
#define RTR_ATOMIC_SET(___var,___newval)	(void)_InterlockedExchange(&___var,___newval)

#define RTR_ATOMIC_INCREMENT_RET(___var)	_InterlockedIncrement(&___var)
#define RTR_ATOMIC_DECREMENT_RET(___var)	_InterlockedDecrement(&___var)
#define RTR_ATOMIC_ADD_RET(___var,___incr)	_InterlockedExchangeAdd(&___var,___incr)

#define RTR_ATOMIC_INCREMENT_PTR(___var)		(void)_InterlockedIncrement(___var)
#define RTR_ATOMIC_DECREMENT_PTR(___var)		(void)_InterlockedDecrement(___var)
#define RTR_ATOMIC_ADD_PTR(___var,___incr)		(void)_InterlockedExchangeAdd(___var,___incr)
#define RTR_ATOMIC_SET_PTR(___var,___newval)	(void)_InterlockedExchange(___var,___newval)

#define RTR_ATOMIC_INCREMENT_RET_PTR(___var)	_InterlockedIncrement(___var)
#define RTR_ATOMIC_DECREMENT_RET_PTR(___var)	_InterlockedDecrement(___var)
#define RTR_ATOMIC_ADD_RET_PTR(___var,___incr)	_InterlockedExchangeAdd(___var,___incr)

#define RTR_ATOMIC_SET_RETOLD(___var,___newval) \
	_InterlockedExchange(&___var,___newval)

#define RTR_ATOMIC_COMPARE_AND_SWAP(___var,___compval,___newval) \
	_InterlockedCompareExchange((LPLONG)&___var, ___newval, ___compval)

#ifdef COMPILE_64BITS
#define RTR_ATOMIC_COMPARE_AND_SWAPPTR(___pvar,___pcompval,___pnewval) \
	InterlockedCompareExchangePointer((PVOID*)&___pvar, ___pnewval, ___pcompval)
#define RTR_ATOMIC_READ64(___pvar)			(*(___pvar)) // this assumes the variable is 64-bit aligned
#else
#define RTR_ATOMIC_COMPARE_AND_SWAPPTR(___pvar,___pcompval,___pnewval) \
	(void*)_InterlockedCompareExchange((LPLONG)&___pvar,(LONG)___pnewval,(LONG)___pcompval)

// This function uses compare/exchange to do a 64-bit atomic read
// this assumes the variable is 64-bit aligned
#define RTR_ATOMIC_READ64(___pvar)			rtrAtomicRead64((___pvar))
RTR_C_ALWAYS_INLINE rtr_atomic_val64 rtrAtomicRead64(volatile rtr_atomic_val64 *val)
{
	rtr_atomic_val64 dummyPrevValue = 0, dummyNewValue = 0;
	return (rtr_atomic_val64)(_InterlockedCompareExchange64(val, dummyNewValue, dummyPrevValue));
};

#endif

/* Most 64-bit atomic functions(e.g. InterlockedIncrement64) are not available on Windows XP(Minimum is Vista/2003). 
 * The only atomic 64-bit function we can use for this appears to be _InterlockedCompareExchange64. */
RTR_C_ALWAYS_INLINE rtr_atomic_val64 rtrInterExch64(rtr_atomic_val64 *val, rtr_atomic_val64 newval)
{
	rtr_atomic_val64 prev;
	do prev = *val;
	while (prev != _InterlockedCompareExchange64(val, newval, prev));
	return *val;
};

#define RTR_ATOMIC_INCREMENT64(___var)		(void)rtrInterExch64(&___var, ___var + 1)
#define RTR_ATOMIC_DECREMENT64(___var)		(void)rtrInterExch64(&___var, ___var - 1)
#define RTR_ATOMIC_SET64(___var,___newval)	(void)rtrInterExch64(&___var,___newval)
#define RTR_ATOMIC_ADD64(___var, ___addend) (void)rtrInterExch64(&___var, ___var + ___addend)

#define RTR_ATOMIC_INCREMENT64_PTR(___var)		(void)rtrInterExch64(___var, ___var + 1)
#define RTR_ATOMIC_DECREMENT64_PTR(___var)		(void)rtrInterExch64(___var, ___var - 1)
#define RTR_ATOMIC_SET64_PTR(___var,___newval)	(void)rtrInterExch64(___var,___newval)
#define RTR_ATOMIC_ADD64_PTR(___var, ___addend) (void)rtrInterExch64(___var, ___var + ___addend)



#elif defined(LINUX)
	/**********************************************/
	/* Linux atomic operations                    */
	/**********************************************/

typedef int rtr_atomic_val;
typedef rtrInt64 rtr_atomic_val64;

RTR_C_ALWAYS_INLINE rtr_atomic_val rtrInterIncr(rtr_atomic_val *var)
{
	__asm__ __volatile__ ( "lock incl %0" : "=m"(*var) : "m"(*var));
	return *var;
}
RTR_C_ALWAYS_INLINE rtr_atomic_val rtrInterDecr(rtr_atomic_val *var)
{
	__asm__ __volatile__ ( "lock decl %0" : "=m"(*var) : "m"(*var));
	return *var;
}
RTR_C_ALWAYS_INLINE rtr_atomic_val rtrInterAdd(rtr_atomic_val *var, rtr_atomic_val incr)
{
	__asm__ __volatile__ ( "lock addl %1, %0" :
								"=m"(*var) :
								"ir"(incr), "m"(*var));
	return *var;
}
RTR_C_ALWAYS_INLINE rtr_atomic_val rtrInterExchOld(rtr_atomic_val *var, rtr_atomic_val newval)
{
	register rtr_atomic_val prev;
	__asm__ __volatile__ ( "xchgl %0, %1" :
			"=r"(prev) : "m"(*var), "0"(newval) : "memory");
	return prev;
}
RTR_C_ALWAYS_INLINE rtr_atomic_val rtrInterCompAndSwap(rtr_atomic_val *var, rtr_atomic_val compval, rtr_atomic_val newval)
{
	register rtr_atomic_val prev;
	__asm__ __volatile__ ( "lock cmpxchgl %1, %2" :
		"=a"(prev) : "r"(newval), "m"(*var), "0"(compval) : "memory" );
	return prev;
};

#ifdef COMPILE_64BITS
RTR_C_ALWAYS_INLINE void* rtrInterCompAndSwapPtr(void **var, void *compval, void *newval)
{
	register void* prev;
	__asm__ __volatile__ ( "lock cmpxchgq %1, %2" :
		"=a"(prev) : "r"(newval), "m"(*var), "0"(compval) : "memory");
	return prev;
};
#else
RTR_C_ALWAYS_INLINE void* rtrInterCompAndSwapPtr(void **var, void *compval, void *newval)
{
	register void* prev;
	__asm__ __volatile__ ( "lock cmpxchgl %1, %2" :
		"=a"(prev) : "r"(newval), "m"(*var), "0"(compval) : "memory" );
	return prev;
};
#endif

#ifdef COMPILE_64BITS

RTR_C_ALWAYS_INLINE rtr_atomic_val64 rtrInterIncr64(rtr_atomic_val64 *var)
{
	__asm__ __volatile__ ( "lock incq %0" : "=m"(*var) : "m"(*var) : "cc");
	return *var;
};

RTR_C_ALWAYS_INLINE rtr_atomic_val64 rtrInterDecr64(rtr_atomic_val64 *var)
{
	__asm__ __volatile__ ( "lock decq %0" : "=m"(*var) : "m"(*var) : "cc");
	return *var;
};

RTR_C_ALWAYS_INLINE rtr_atomic_val64 rtrInterAdd64(rtr_atomic_val64 *var, rtr_atomic_val64 addend)
{
	__asm__ __volatile__ ( "lock addq %1, %0" : "=m"(*var) : "r"(addend) : "cc");
	return *var;
};

RTR_C_ALWAYS_INLINE rtr_atomic_val64 rtrInterExch64(rtr_atomic_val64 *var, rtr_atomic_val64 newval)
{
	register rtr_atomic_val64 prev;
	__asm__ __volatile__ ( "lock xchgq %0, %1" :
			"=r"(prev) : "m"(*var), "0"(newval) : "cc");
	return prev;
}

#define RTR_ATOMIC_INCREMENT64(___var)		(void)rtrInterIncr64(&___var)
#define RTR_ATOMIC_DECREMENT64(___var)		(void)rtrInterDecr64(&___var)
#define RTR_ATOMIC_SET64(___var,___newval)	(void)rtrInterExch64(&___var,___newval)
#define RTR_ATOMIC_ADD64(___var,___addend)	(void)rtrInterAdd64(&___var,___addend)
#define RTR_ATOMIC_READ64(___pvar)			(*(___pvar)) // this assumes the variable is 64-bit aligned

#define RTR_ATOMIC_INCREMENT64_PTR(___var)		(void)rtrInterIncr64(___var)
#define RTR_ATOMIC_DECREMENT64_PTR(___var)		(void)rtrInterDecr64(___var)
#define RTR_ATOMIC_SET64_PTR(___var,___newval)	(void)rtrInterExch64(___var,___newval)
#define RTR_ATOMIC_ADD64_PTR(___var,___addend)	(void)rtrInterAdd64(___var,___addend)

#else

RTR_C_ALWAYS_INLINE rtr_atomic_val64 rtrInterExch64(volatile rtr_atomic_val64 *volatile var, volatile rtr_atomic_val64 newVal)
{
	volatile rtrInt32* volatile p1 = (rtrInt32*)var;
	volatile rtrInt32* volatile p2 = ((rtrInt32*)var) + 1;

	volatile rtrInt32* volatile pn1 = (rtrInt32*)(&newVal);
	volatile rtrInt32* volatile pn2 = ((rtrInt32*)(&newVal)) + 1;

	__asm__ __volatile__ ( "1: lock cmpxchg8b %0 ; jnz 1"
	: "=m"(*var) : "a" (*p1), "d"(*p2),  "b" (*pn1), "c" (*pn2) : "cc");
	return *var;
}

// This function uses compare/exchange to do a 64-bit atomic read
// this assumes the variable is 64-bit aligned
RTR_C_ALWAYS_INLINE rtr_atomic_val64 rtrAtomicRead64(volatile rtr_atomic_val64 *val)
{
	rtr_atomic_val64 prevValue = 0, newValue = 0;

	rtrInt32* p1PrevValue = (rtrInt32*)&prevValue;
	rtrInt32* p2PrevValue = ((rtrInt32*)&prevValue) + 1;

	rtrInt32* p1NewValue = (rtrInt32*)(&newValue);
	rtrInt32* p2NewValue = ((rtrInt32*)(&newValue)) + 1;

	__asm__ __volatile__ ( "lock cmpxchg8b %0"
	: "=m"(*val), "=a"(*p1PrevValue), "=d"(*p2PrevValue) : "1"(*p1PrevValue), "2"(*p2PrevValue), "b"(*p1NewValue), "c"(*p2NewValue) : "cc");
	return prevValue;
}

#define RTR_ATOMIC_READ64(___pvar)			(*(___pvar))
#define RTR_ATOMIC_INCREMENT64(___var)		(void)rtrInterExch64(&___var,___var+1)
#define RTR_ATOMIC_DECREMENT64(___var)		(void)rtrInterExch64(&___var,___var-1)
#define RTR_ATOMIC_SET64(___var,___newval)	(void)rtrInterExch64(&___var,___newval)
#define RTR_ATOMIC_ADD64(___var,___addend)	(void)rtrInterExch64(&___var,___var + ___addend)

#define RTR_ATOMIC_INCREMENT64_PTR(___var)		(void)rtrInterExch64(___var,___var+1)
#define RTR_ATOMIC_DECREMENT64_PTR(___var)		(void)rtrInterExch64(___var,___var-1)
#define RTR_ATOMIC_SET64_PTR(___var,___newval)	(void)rtrInterExch64(___var,___newval)
#define RTR_ATOMIC_ADD64_PTR(___var,___addend)	(void)rtrInterExch64(___var,___var + ___addend)


#endif


#define RTR_ATOMIC_INCREMENT(___var)		(void)rtrInterIncr(&___var)
#define RTR_ATOMIC_DECREMENT(___var)		(void)rtrInterDecr(&___var)
#define RTR_ATOMIC_ADD(___var,___incr)		(void)rtrInterAdd(&___var,___incr)
#define RTR_ATOMIC_SET(___var,___newval)	(void)rtrInterExchOld(&___var,___newval)

#define RTR_ATOMIC_INCREMENT_RET(___var)	rtrInterIncr(&___var)
#define RTR_ATOMIC_DECREMENT_RET(___var)	rtrInterDecr(&___var)
#define RTR_ATOMIC_ADD_RET(___var,___incr)	rtrInterAdd(&___var,___incr)

#define RTR_ATOMIC_INCREMENT_PTR(___var)		(void)rtrInterIncr(___var)
#define RTR_ATOMIC_DECREMENT_PTR(___var)		(void)rtrInterDecr(___var)
#define RTR_ATOMIC_ADD_PTR(___var,___incr)		(void)rtrInterAdd(___var,___incr)
#define RTR_ATOMIC_SET_PTR(___var,___newval)	(void)rtrInterExchOld(___var,___newval)

#define RTR_ATOMIC_INCREMENT_RET_PTR(___var)	rtrInterIncr(___var)
#define RTR_ATOMIC_DECREMENT_RET_PTR(___var)	rtrInterDecr(___var)
#define RTR_ATOMIC_ADD_RET_PTR(___var,___incr)	rtrInterAdd(___var,___incr)

#define RTR_ATOMIC_SET_RETOLD(___var,___newval) \
		rtrInterExchOld(&___var,___newval)

#define RTR_ATOMIC_COMPARE_AND_SWAP(___var,___compval,___newval) \
		rtrInterCompAndSwap(&___var,___compval,___newval)

#define RTR_ATOMIC_COMPARE_AND_SWAPPTR(___pvar,___pcompval,___pnewval) \
		rtrInterCompAndSwapPtr(&___pvar,(void*)___pcompval,(void*)___pnewval)

#else

typedef int rtr_atomic_val;
typedef rtrInt64 rtr_atomic_val64;
#error	"RTR_ATOMIC operations are not implemented on this platform."


#endif


#ifdef __cplusplus
}
#endif

#endif
