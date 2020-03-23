/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_BYTE_SWAP_H
#define __RTR_BYTE_SWAP_H

#include "rtr/os.h"

#ifdef COMPILE_64BITS
typedef rtrUInt64 rtrUInt_Ptr;
#else
typedef rtrUInt32 rtrUInt_Ptr;
#endif

#ifdef RTR_BIG_ENDIAN

#define __rtr_bswap16(___from) (___from)
#define __rtr_bswap32(___from) (___from)
#define __rtr_bswap64(___from) (___from)

#define __rtr_bswap32ToFloat(___dptr, ___sval) (*(rtrUInt32*)(___dptr) = __rtr_bswap32(___sval), 4)
#define __rtr_bswap64ToDouble(___dptr, ___sval) (*(rtrUInt64*)(___dptr) = __rtr_bswap64(___sval), 8)


#elif defined(Linux)

RTR_C_ALWAYS_INLINE rtrUInt16 __rtr_bswap16(rtrUInt16 ___from)
{
	__asm__ __volatile__ ("rorw $8, %w0" : "+q" (___from) );
	return ___from;
}

RTR_C_ALWAYS_INLINE rtrUInt32 __rtr_bswap32(rtrUInt32 ___from)
{
	__asm__ __volatile__ ("bswapl %0" : "+r" (___from) );
	return ___from;
}

#ifdef COMPILE_64BITS
RTR_C_ALWAYS_INLINE rtrUInt64 __rtr_bswap64(rtrUInt64 ___from)
{
	__asm__ __volatile__ ("bswapq %0" : "+r" (___from) );
	return ___from;
}
#else
RTR_C_ALWAYS_INLINE rtrUInt64 __rtr_bswap64(rtrUInt64 ___from)
{
	union {
  		struct { rtrUInt32 a,b; } s;
		rtrUInt64 u;
	} ___v;
	___v.u = ___from;
	__asm__ __volatile__ ("bswapl %0; bswapl %1"
				: "=r" (___v.s.b), "=r" (___v.s.a)
				: "0" (___v.s.a), "1" (___v.s.b) );
	return ___v.u;
}
#endif

/* Attempting to store the result of the byteswap directly into a float/double
 * by simple casting may not work correctly when GCC assumes strict-aliasing
 * in its optimizations.  __rtr_bswap32ToFloat and __rtr_bswap64ToDouble 
 * use a technique documented by the GCC team as a solution to this(at least for GCC). 
 * For more info see http://gcc.gnu.org/bugs/#nonbugs_c :
 *   "Casting does not work as expected when optimization is turned on", 
 * or look for documentation on strict aliasing & the fno-strict-aliasing flag. 
 * Issues have not been observed for other platforms. */
RTR_C_ALWAYS_INLINE void __rtr_bswap32ToFloat(rtrFloat *___to, rtrUInt32 ___from)
{
	typedef union
	{
		rtrFloat floatType;
		rtrUInt32 uintType;
	} rtrBswapFloat;

	rtrBswapFloat bswapVal;
	bswapVal.uintType = __rtr_bswap32(___from);
	*___to = bswapVal.floatType;
}

RTR_C_ALWAYS_INLINE void __rtr_bswap64ToDouble(rtrDouble *___to, rtrUInt64 ___from)
{
	typedef union
	{
		rtrDouble doubleType;
		rtrUInt64 uintType;
	} rtrBswapDouble;

	rtrBswapDouble bswapVal;
	bswapVal.uintType = __rtr_bswap64(___from);
	*___to = bswapVal.doubleType;
}


#elif defined(_WIN32)

#include <stdlib.h>
#define __rtr_bswap16(___from) _byteswap_ushort(___from)
#define __rtr_bswap32(___from) _byteswap_ulong(___from)
#define __rtr_bswap64(___from) _byteswap_uint64(___from)

#define __rtr_bswap32ToFloat(___dptr, ___sval) (*(rtrUInt32*)(___dptr) = __rtr_bswap32(___sval), 4)
#define __rtr_bswap64ToDouble(___dptr, ___sval) (*(rtrUInt64*)(___dptr) = __rtr_bswap64(___sval), 8)


#elif defined(RTR_USE_BYTESWAP)

#include <byteswap.h>
#define __rtr_bswap16(___from) bswap_16(___from)
#define __rtr_bswap32(___from) bswap_32(___from)
#define __rtr_bswap64(___from) bswap_64(___from)

#define __rtr_bswap32ToFloat(___dptr, ___sval) (*(rtrUInt32*)(___dptr) = __rtr_bswap32(___sval), 4)
#define __rtr_bswap64ToDouble(___dptr, ___sval) (*(rtrUInt64*)(___dptr) = __rtr_bswap64(___sval), 8)


#else

#define __rtr_bswap16(___from) \
	((((___from) >> 8) & 0xFF) | (((___from) & 0xFF) << 8))

#define __rtr_bswap32(___from) \
	((((___from) & 0xFF000000) >> 24) | (((___from) & 0x00FF0000) >>  8) | \
	 (((___from) & 0x0000FF00) <<  8) | (((___from) & 0x000000FF) << 24))

#define __rtr_bswap64(___from) \
	((((___from) & 0xFF00000000000000ULL) >> 56) | \
	 (((___from) & 0x00FF000000000000ULL) >> 40) | \
	 (((___from) & 0x0000FF0000000000ULL) >> 24) | \
	 (((___from) & 0x000000FF00000000ULL) >> 8) | \
	 (((___from) & 0x00000000FF000000ULL) << 8)  | \
	 (((___from) & 0x0000000000FF0000ULL) << 24) | \
	 (((___from) & 0x000000000000FF00ULL) << 40) | \
	 (((___from) & 0x00000000000000FFULL) << 56) )

#define __rtr_bswap32ToFloat(___dptr, ___sval) (*(rtrUInt32*)(___dptr) = __rtr_bswap32(___sval), 4)
#define __rtr_bswap64ToDouble(___dptr, ___sval) (*(rtrUInt64*)(___dptr) = __rtr_bswap64(___sval), 8)

#endif


#define RTR_SET_BIT(___dptr,___bitn)	( ((rtrUInt8*)(___dptr))[0] |=  (1 << ___bitn) )
#define RTR_UNSET_BIT(___dptr,___bitn)	( ((rtrUInt8*)(___dptr))[0] &= ~(1 <<  ___bitn) )
#define RTR_GET_BIT(___dval,___bitn)	( (___dval >> ___bitn) & 0x01 )

#define RTR_PUT_8(___dptr,___sval) (*((rtrUInt8*)___dptr) = ___sval, 1)
#define RTR_GET_8(___dval,___sptr) (___dval = *((rtrUInt8*)___sptr), 1)


#ifndef RTR_QUAD_32_ALIGN

#define RTR_PUT_16(___dptr,___sval) \
		(*((rtrUInt16*)___dptr) = __rtr_bswap16(___sval), 2)
#define RTR_GET_16(___dval,___sptr) \
		(___dval = __rtr_bswap16(*(rtrUInt16*)___sptr), 2)


#define RTR_PUT_24(___dptr,___sval) \
		(*(rtrUInt32*)___dptr = __rtr_bswap32(___sval << 8),3)
#define RTR_GET_24(___dval, ___sptr) \
		(___dval = __rtr_bswap32(*(rtrUInt32*)___sptr << 8),3)

#define RTR_PUT_32(___dptr,___sval) \
		(*(rtrUInt32*)___dptr = __rtr_bswap32(___sval), 4)
#define RTR_GET_32(___dval,___sptr) \
		(___dval = __rtr_bswap32(*(rtrUInt32*)___sptr), 4)

#define RTR_PUT_64(___dptr,___sval) \
		(*(rtrUInt64*)___dptr = __rtr_bswap64(___sval), 8)
#define RTR_GET_64(___dval,___sptr) \
		(___dval = __rtr_bswap64(*(rtrUInt64*)___sptr), 8)

#define RTR_PUT_FLOAT(___dptr,___sval) \
		(*(rtrUInt32*)___dptr = __rtr_bswap32(*(rtrUInt32*)(&___sval)), 4)
#define RTR_GET_FLOAT(___dval,___sptr) \
		(__rtr_bswap32ToFloat(&___dval, *(rtrUInt32*)___sptr), 4)

#define RTR_PUT_DOUBLE(___dptr,___sval) \
		(*(rtrUInt64*)___dptr = __rtr_bswap64(*(rtrUInt64*)(&___sval)), 8)
#define RTR_GET_DOUBLE(___dval,___sptr) \
		(__rtr_bswap64ToDouble(&___dval, *(rtrUInt64*)___sptr), 8)


#elif defined(RTR_LITTLE_ENDIAN)

	
#define RTR_PUT_16(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & 0x1) ? \
		  (  (char*)(___dptr))[0] = ((char*)(&___sval))[1],\
			((char*)(___dptr))[1] = ((char*)(&___sval))[0], 2 : \
		  (*((rtrUInt16*)___dptr) = __rtr_bswap16(___sval), 2 ) )
#define RTR_GET_16(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & 0x1) ? \
		  (  (char*)(&___dval))[0] = ((char*)(___sptr))[1],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[0], 2 : \
		  (___dval = __rtr_bswap16(*(rtrUInt16*)___sptr), 2) )


#define RTR_PUT_24(___dptr,___sval) \
		(((char*)(___dptr))[0] = ((char*)(&___sval))[2],\
		 ((char*)(___dptr))[1] = ((char*)(&___sval))[1],\
		 ((char*)(___dptr))[2] = ((char*)(&___sval))[0], 3 )
		 

#define RTR_GET_24(___dval,___sptr) \
		( ((char*)(&___dval))[0] = 0, \
		  ((char*)(&___dval))[1] = ((char*)(___sptr))[2], \
		  ((char*)(&___dval))[2] = ((char*)(___sptr))[1], \
		  ((char*)(&___dval))[3] = ((char*)(___sptr))[0], 3 )



#define RTR_PUT_32(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & 0x3) ? \
		  (  (char*)(___dptr))[0] = ((char*)(&___sval))[3],\
			((char*)(___dptr))[1] = ((char*)(&___sval))[2],\
			((char*)(___dptr))[2] = ((char*)(&___sval))[1],\
			((char*)(___dptr))[3] = ((char*)(&___sval))[0], 4 : \
		  (*(rtrUInt32*)___dptr = __rtr_bswap32(___sval), 4) )
#define RTR_GET_32(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & 0x3) ? \
		  (  (char*)(&___dval))[0] = ((char*)(___sptr))[3],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[2],\
			((char*)(&___dval))[2] = ((char*)(___sptr))[1],\
			((char*)(&___dval))[3] = ((char*)(___sptr))[0], 4 : \
		  (___dval = __rtr_bswap32(*(rtrUInt32*)___sptr), 4 ) )

#define RTR_PUT_64(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & RTR_DWORD_ALIGN_BITS) ? \
		  ((char*)(___dptr))[0] = ((char*)(&___sval))[7],\
			((char*)(___dptr))[1] = ((char*)(&___sval))[6],\
			((char*)(___dptr))[2] = ((char*)(&___sval))[5],\
			((char*)(___dptr))[3] = ((char*)(&___sval))[4],\
			((char*)(___dptr))[4] = ((char*)(&___sval))[3],\
			((char*)(___dptr))[5] = ((char*)(&___sval))[2],\
			((char*)(___dptr))[6] = ((char*)(&___sval))[1],\
			((char*)(___dptr))[7] = ((char*)(&___sval))[0], 8 : \
		  (*(rtrUInt64*)___dptr = __rtr_bswap64(___sval), 8) )
#define RTR_GET_64(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & RTR_DWORD_ALIGN_BITS) ? \
		  ((char*)(&___dval))[0] = ((char*)(___sptr))[7],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[6],\
			((char*)(&___dval))[2] = ((char*)(___sptr))[5],\
			((char*)(&___dval))[3] = ((char*)(___sptr))[4],\
			((char*)(&___dval))[4] = ((char*)(___sptr))[3],\
			((char*)(&___dval))[5] = ((char*)(___sptr))[2],\
			((char*)(&___dval))[6] = ((char*)(___sptr))[1],\
			((char*)(&___dval))[7] = ((char*)(___sptr))[0], 8 : \
		  (___dval = __rtr_bswap64(*(rtrUInt64*)___sptr), 8) )

#define RTR_PUT_FLOAT(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & 0x3) ? \
		  (  (char*)(___dptr))[0] = ((char*)(&___sval))[3],\
			((char*)(___dptr))[1] = ((char*)(&___sval))[2],\
			((char*)(___dptr))[2] = ((char*)(&___sval))[1],\
			((char*)(___dptr))[3] = ((char*)(&___sval))[0], 4 : \
		  (*(rtrUInt32*)___dptr = __rtr_bswap32(*(rtrUInt32*)(&___sval)), 4) )
#define RTR_GET_FLOAT(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & 0x3) ? \
		  (  (char*)(&___dval))[0] = ((char*)(___sptr))[3],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[2],\
			((char*)(&___dval))[2] = ((char*)(___sptr))[1],\
			((char*)(&___dval))[3] = ((char*)(___sptr))[0], 4 : \
		  (*(rtrUInt32*)(&___dval) = __rtr_bswap32(*(rtrUInt32*)___sptr), 4 ) )

#define RTR_PUT_DOUBLE(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & RTR_DWORD_ALIGN_BITS) ? \
		  ((char*)(___dptr))[0] = ((char*)(&___sval))[7],\
			((char*)(___dptr))[1] = ((char*)(&___sval))[6],\
			((char*)(___dptr))[2] = ((char*)(&___sval))[5],\
			((char*)(___dptr))[3] = ((char*)(&___sval))[4],\
			((char*)(___dptr))[4] = ((char*)(&___sval))[3],\
			((char*)(___dptr))[5] = ((char*)(&___sval))[2],\
			((char*)(___dptr))[6] = ((char*)(&___sval))[1],\
			((char*)(___dptr))[7] = ((char*)(&___sval))[0], 8 : \
		  (*(rtrUInt64*)___dptr = __rtr_bswap64(*(rtrUInt64*)(&___sval)), 8) )
#define RTR_GET_DOUBLE(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & RTR_DWORD_ALIGN_BITS) ? \
		  ((char*)(&___dval))[0] = ((char*)(___sptr))[7],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[6],\
			((char*)(&___dval))[2] = ((char*)(___sptr))[5],\
			((char*)(&___dval))[3] = ((char*)(___sptr))[4],\
			((char*)(&___dval))[4] = ((char*)(___sptr))[3],\
			((char*)(&___dval))[5] = ((char*)(___sptr))[2],\
			((char*)(&___dval))[6] = ((char*)(___sptr))[1],\
			((char*)(&___dval))[7] = ((char*)(___sptr))[0], 8 : \
		  (*(rtrUInt64*)(&___dval) = __rtr_bswap64(*(rtrUInt64*)___sptr), 8) )


#else


#define RTR_PUT_16(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & 0x1) ? \
		  (  (char*)(___dptr))[0] = ((char*)(&___sval))[0],\
			((char*)(___dptr))[1] = ((char*)(&___sval))[1], 2 : \
		  (*((rtrUInt16*)___dptr) = ___sval, 2 ) )
#define RTR_GET_16(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & 0x1) ? \
		  (  (char*)(&___dval))[0] = ((char*)(___sptr))[0],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[1], 2 : \
		  (___dval = *(rtrUInt16*)___sptr, 2) ) 


#define RTR_PUT_24(___dptr,___sval) \
		(((char*)(___dptr))[0] = ((char*)(&___sval))[1],\
		 ((char*)(___dptr))[1] = ((char*)(&___sval))[2],\
		 ((char*)(___dptr))[2] = ((char*)(&___sval))[3], 3 )
		 

#define RTR_GET_24(___dval,___sptr) \
		( ((char*)(&___dval))[0] = 0, \
		  ((char*)(&___dval))[1] = ((char*)(___sptr))[0], \
		  ((char*)(&___dval))[2] = ((char*)(___sptr))[1], \
		  ((char*)(&___dval))[3] = ((char*)(___sptr))[2], 3 )


#define RTR_PUT_32(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & 0x3) ? \
		  (  (char*)(___dptr))[0] = ((char*)(&___sval))[0],\
			((char*)(___dptr))[1] = ((char*)(&___sval))[1],\
			((char*)(___dptr))[2] = ((char*)(&___sval))[2],\
			((char*)(___dptr))[3] = ((char*)(&___sval))[3], 4 : \
		  (*(rtrUInt32*)___dptr = ___sval, 4 ) )
#define RTR_GET_32(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & 0x3) ? \
		  (  (char*)(&___dval))[0] = ((char*)(___sptr))[0],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[1],\
			((char*)(&___dval))[2] = ((char*)(___sptr))[2],\
			((char*)(&___dval))[3] = ((char*)(___sptr))[3], 4 : \
		  (___dval = *(rtrUInt32*)___sptr, 4 ) )

#define RTR_PUT_64(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & RTR_DWORD_ALIGN_BITS) ? \
		  (  (char*)(___dptr))[0] = ((char*)(&___sval))[0],\
		 	((char*)(___dptr))[1] = ((char*)(&___sval))[1],\
		 	((char*)(___dptr))[2] = ((char*)(&___sval))[2],\
		 	((char*)(___dptr))[3] = ((char*)(&___sval))[3],\
		 	((char*)(___dptr))[4] = ((char*)(&___sval))[4],\
		 	((char*)(___dptr))[5] = ((char*)(&___sval))[5],\
		 	((char*)(___dptr))[6] = ((char*)(&___sval))[6],\
		 	((char*)(___dptr))[7] = ((char*)(&___sval))[7], 8 : \
		   (*(rtrUInt64*)___dptr = ___sval, 8 ) )
#define RTR_GET_64(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & RTR_DWORD_ALIGN_BITS) ? \
		  (  (char*)(&___dval))[0] = ((char*)(___sptr))[0],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[1],\
			((char*)(&___dval))[2] = ((char*)(___sptr))[2],\
			((char*)(&___dval))[3] = ((char*)(___sptr))[3],\
			((char*)(&___dval))[4] = ((char*)(___sptr))[4],\
			((char*)(&___dval))[5] = ((char*)(___sptr))[5],\
			((char*)(&___dval))[6] = ((char*)(___sptr))[6],\
			((char*)(&___dval))[7] = ((char*)(___sptr))[7], 8 : \
		  (___dval = *(rtrUInt64*)___sptr, 8 ) )

#define RTR_PUT_FLOAT(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & 0x3) ? \
		  (  (char*)(___dptr))[0] = ((char*)(&___sval))[0],\
			((char*)(___dptr))[1] = ((char*)(&___sval))[1],\
			((char*)(___dptr))[2] = ((char*)(&___sval))[2],\
			((char*)(___dptr))[3] = ((char*)(&___sval))[3], 4 : \
		  (*(rtrUInt32*)___dptr = *(rtrUInt32*)(&___sval), 4 ) )
#define RTR_GET_FLOAT(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & 0x3) ? \
		  (  (char*)(&___dval))[0] = ((char*)(___sptr))[0],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[1],\
			((char*)(&___dval))[2] = ((char*)(___sptr))[2],\
			((char*)(&___dval))[3] = ((char*)(___sptr))[3], 4 : \
		  (*(rtrUInt32*)(&___dval) = *(rtrUInt32*)___sptr, 4 ) )

#define RTR_PUT_DOUBLE(___dptr,___sval) \
		(((rtrUInt_Ptr)___dptr & RTR_DWORD_ALIGN_BITS) ? \
		  (  (char*)(___dptr))[0] = ((char*)(&___sval))[0],\
		 	((char*)(___dptr))[1] = ((char*)(&___sval))[1],\
		 	((char*)(___dptr))[2] = ((char*)(&___sval))[2],\
		 	((char*)(___dptr))[3] = ((char*)(&___sval))[3],\
		 	((char*)(___dptr))[4] = ((char*)(&___sval))[4],\
		 	((char*)(___dptr))[5] = ((char*)(&___sval))[5],\
		 	((char*)(___dptr))[6] = ((char*)(&___sval))[6],\
		 	((char*)(___dptr))[7] = ((char*)(&___sval))[7], 8 : \
		   (*(rtrUInt64*)___dptr = *(rtrUInt64*)(&___sval), 8 ) )
#define RTR_GET_DOUBLE(___dval,___sptr) \
		(((rtrUInt_Ptr)___sptr & RTR_DWORD_ALIGN_BITS) ? \
		  (  (char*)(&___dval))[0] = ((char*)(___sptr))[0],\
			((char*)(&___dval))[1] = ((char*)(___sptr))[1],\
			((char*)(&___dval))[2] = ((char*)(___sptr))[2],\
			((char*)(&___dval))[3] = ((char*)(___sptr))[3],\
			((char*)(&___dval))[4] = ((char*)(___sptr))[4],\
			((char*)(&___dval))[5] = ((char*)(___sptr))[5],\
			((char*)(&___dval))[6] = ((char*)(___sptr))[6],\
			((char*)(&___dval))[7] = ((char*)(___sptr))[7], 8 : \
		  (*(rtrUInt64*)(&___dval) = *(rtrUInt64*)___sptr, 8 ) )

#endif


#endif /* #ifndef __RTR_BYTE_SWAP_H */

