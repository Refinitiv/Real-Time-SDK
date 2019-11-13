/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef	__rtr_custmem_h
#define	__rtr_custmem_h

#include <stdlib.h>
#include <string.h>
#include "intcustmem.h"


/*****************************************/
/* MemCopyByByte() simple while loop */
#define MEM_CHG 42

RTR_C_ALWAYS_INLINE void *MemCopyByByte(void *dest, const void *src, size_t size)
{
#if defined(LinuxVersion) && LinuxVersion <= 4
	return( (size <= MEM_CHG) ? doSimpleMemCopy(dest,src,size) :
			memcpy(dest, src, size) );
#else
	return memcpy(dest, src, size);
#endif
}
/*****************************************/


/*****************************************/
/* MemCopyByInt() works on quad boundaries */
RTR_C_ALWAYS_INLINE void *MemCopyByInt(void *dest, const void *src, size_t size)
{
#if defined(LinuxVersion) && LinuxVersion <= 4
	return ( (size < 8) ? doSimpleMemCopy(dest,src,size) :
			 ((size <= 128) ? doNonSimpleMemCopy4(dest,src,size) :
			 	memcpy(dest, src, size) )  );
#else
	return memcpy(dest, src, size);		// The memcpy with Linux 5 and 6 is much faster, windows memcpy is faster too
#endif
}
/*****************************************/

/*****************************************/
/* MemCopyLongLong() works on quad boundaries, but copies long long 8 bytes */
RTR_C_ALWAYS_INLINE void *MemCopyByLongLong(void *dest, const void *src, size_t size)
{
#if defined(LinuxVersion) && LinuxVersion <= 4
	return ( (size < 8) ? doSimpleMemCopy(dest,src,size) :
			 ((size <= 128) ? doNonSimpleMemCopy8(dest,src,size) :
			 	memcpy(dest, src, size) )  );
#else
	return memcpy(dest, src, size);
#endif
}
/*****************************************/




/*****************************************/
/* MemSetByByte() simple while loop */
RTR_C_ALWAYS_INLINE void *MemSetByByte(void *dest, int c, size_t size)
{
	return ( (size <= MEM_CHG) ? doSimpleMemSet(dest, c, size) :
				memset(dest, c, size) );
}
/*****************************************/

/*****************************************/
/* MemSetByInt() works on quad boundaries and copies int 4 bytes */
RTR_C_ALWAYS_INLINE void *MemSetByInt(void *dest, int c, size_t size)
{
	return ( (size < 8) ? doSimpleMemSet(dest,c,size) :
			 ((size <= 128) ? doNonSimpleMemSet4(dest,c,size) :
			 	memset(dest, c, size) )  );
}
/*****************************************/

/*****************************************/
/* MemSetLongLong() works on quad boundaries, but copies long long 8 bytes */
RTR_C_ALWAYS_INLINE void *MemSetByLongLong(void *dest, int c, size_t size)
{
	return ( (size < 8) ? doSimpleMemSet(dest,c,size) :
			 ((size <= 128) ? doNonSimpleMemSet8(dest,c,size) :
			 	memset(dest, c, size) )  );
}
/*****************************************/



/*****************************************/
/* MemCmpByByte() simple while loop */
RTR_C_ALWAYS_INLINE int MemCmpByByte(void *dest, const void *src, size_t size)
{
	return( (size <= MEM_CHG) ? doSimpleMemCmp(dest,src,size) :
			memcmp(dest, src, size) );
}
/*****************************************/


/*****************************************/
/* MemCmpByInt() works on quad boundaries */
RTR_C_ALWAYS_INLINE int MemCmpByInt(void *dest, const void *src, size_t size)
{
	return ( (size < 8) ? doSimpleMemCmp(dest,src,size) :
			 ((size <= 128) ? doNonSimpleMemCmp4(dest,src,size) :
			 	memcmp(dest, src, size) )  );
}
/*****************************************/

/*****************************************/
/* MemCmpLongLong() works on quad boundaries, but copies long long 8 bytes */
RTR_C_ALWAYS_INLINE int MemCmpByLongLong(void *dest, const void *src, size_t size)
{
	return ( (size < 8) ? doSimpleMemCmp(dest,src,size) :
			 ((size <= 128) ? doNonSimpleMemCmp8(dest,src,size) :
			 	memcmp(dest, src, size) )  );
}
/*****************************************/

#endif

