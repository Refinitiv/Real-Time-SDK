/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef	__INTCUSTMEM_H__
#define	__INTCUSTMEM_H__


#include <stdlib.h>
#include "rtr/os.h"



/* Used for typecasting pointer to int if needed */
#ifdef COMPILE_64BITS
typedef rtrInt64 rtrIntPtr;
typedef rtrUInt64 rtrUIntPtr;
#else
typedef rtrInt32 rtrIntPtr;
typedef rtrUInt32 rtrUIntPtr;
#endif

#ifdef LINUX 
#define PtrToUIntCast(v) ((unsigned long)(v))
#else
#define PtrToUIntCast(v) ((rtrUInt64)(v))
#endif

#define ALIGNMENT(v) ((size_t)v & 0x03)
#define ALIGN4BYTE(v) ((size_t)v & 0x03)
#define ALIGN8BYTE(v) ((size_t)v & 0x07)

/*****************************************/
/* Internal helper functions, do not use */
RTR_C_ALWAYS_INLINE void *doSimpleMemCopy(void *dest, const void *src, size_t size)
{
	register int cnt=-1;
	while (++cnt < (long) size)
		((char*)dest)[cnt] = ((char*)src)[cnt];
	return(dest);
}

RTR_C_ALWAYS_INLINE void doLess4Copy(void *dest, const void *src, size_t size)
{
	switch(size)
	{
	case 3:
		((char*)dest)[2] = ((char*)src)[2];
	case 2:
		((char*)dest)[1] = ((char*)src)[1];
	case 1:
		((char*)dest)[0] = ((char*)src)[0];
	}
}

static size_t alignbytes[4] = { 0, 3, 2, 1 };
static size_t align8bytes[8] = { 0, 7, 6, 5, 4, 3, 2, 1 };

/*****************************************/



/*****************************************/
/* MemCopyByInt() works on quad boundaries */
/* dest and src must align to 4 byte boundary */
/* copy 4 bytes at a time if possible */
/* uses doLess4Copy for any remainder of size/4 */
/* should return same as memcpy (original value of dest) */

RTR_C_ALWAYS_INLINE void *doQuadAlignMemCopy4(void *dest, const void *src, size_t size)
{
	register int cnt=-1;
	register int allreadydone;

	while (++cnt < (((long) size)>>2))
		((rtrUInt32*)dest)[cnt] = ((rtrUInt32*)src)[cnt];


	allreadydone = (size & ~0x03);
	if (size & 0x03)
		doLess4Copy((char*)dest + allreadydone,
			(char*)src + allreadydone, (size & 0x03));

	return(dest);
}

/* no assumptions on dest and src alignment */
/* copy 4 bytes at a time if possible */
/* if src and dest align the same, use doQuadAlignMemCopy4 */
/* should return same as memcpy (original value of dest) */
RTR_C_ALWAYS_INLINE void *doNonQuadAlignMemCopy4(void *dest, const void *src, size_t size)
{
	register size_t bytestoalign = alignbytes[((size_t)dest & 0x03)];
	if ((ALIGN4BYTE(dest) == ALIGN4BYTE(src)) &&
		(size >= 4+bytestoalign))
	{

		doLess4Copy(dest, src, bytestoalign);

		doQuadAlignMemCopy4((char*)dest+bytestoalign,
			(char*)src+bytestoalign,((long)size)-(long)bytestoalign);
		return(dest);
	}
	else
		return(doSimpleMemCopy(dest,src,size));
}

/* no assumptions on dest and src alignment */
/* copy 4 bytes at a time if possible */
/* should return same as memcpy (original value of dest) */
RTR_C_ALWAYS_INLINE void *doNonSimpleMemCopy4(void *dest, const void *src, size_t size)
{
	return( ALIGN4BYTE(dest) || ALIGN4BYTE(src) ?
		doNonQuadAlignMemCopy4(dest,src,size) :
		doQuadAlignMemCopy4(dest,src,size) );
}

/*****************************************/


/*****************************************/
/* MemCopyByLongLong() works on quad boundaries, but copies (long long) or 8 bytes at a time */
/* dest and src must align to 8 byte boundary */
/* copy 8 bytes at a time if possible */
/* use (1) 32 bit and (1) doLess4Copy for any remainder of size/8 */
/* should return same as memcpy would (original value of dest) */

RTR_C_ALWAYS_INLINE void *doQuadAlignMemCopy8(void *dest, const void *src, size_t size)
{
	register int cnt=-1;
	register int allreadydone = 0;

	while (++cnt < (((long) size)>>3))
	{
		((rtrUInt64*)dest)[cnt] = ((rtrUInt64*)src)[cnt];
		allreadydone += 8;
	}

	/* Check for greater than 4 more bytes left, use int copy if so */
	if (size & 0x04)
		((rtrUInt32*)dest)[cnt<<1] = ((rtrUInt32*)src)[cnt<<1];

	if (size & 0x03)
	{
		allreadydone = (size & ~0x03);
		doLess4Copy((char*)dest + allreadydone,
			(char*)src + allreadydone, (size & 0x03));
	}

	return(dest);
}

/* assumes dest or src alignment NOT on 8 byte boundary */
/* copy 8 bytes at a time if possible */
/* should return same as memcpy would (original value of dest) */
RTR_C_ALWAYS_INLINE void *doNonQuadAlignMemCopy8(void *dest, const void *src, size_t size)
{
	register size_t remainder = align8bytes[((size_t)dest & 0x07)];
	if ((ALIGN8BYTE(dest) == ALIGN8BYTE(src)) &&
		(size >= 8+remainder))
	{
		register size_t bytestoalign = alignbytes[((size_t)dest & 0x03)];

		/* first copy up to 4 (possibly 8) byte alignment */
		doLess4Copy(dest, src, bytestoalign);
		/* then, if enough left and 4 byte align, copy up to 8 byte align */
		if (remainder & 0x04)
		{
			*((rtrUInt32*)((char *)(dest) + bytestoalign)) =
				*((rtrUInt32*)((char *)(src) + bytestoalign));
		}

		doQuadAlignMemCopy8((char*)(dest)+remainder,
			(char*)(src)+remainder,((long)size)-remainder);
		return(dest);
	}
	else
		return(doSimpleMemCopy(dest,src,size));
}

RTR_C_ALWAYS_INLINE void *doNonSimpleMemCopy8(void *dest, const void *src, size_t size)
{
	return( ALIGN8BYTE(dest) || ALIGN8BYTE(src) ?
		doNonQuadAlignMemCopy8(dest,src,size) :
		doQuadAlignMemCopy8(dest,src,size) );
}

/*****************************************/




/*****************************************/
/* Internal helper functions, do not use */
RTR_C_ALWAYS_INLINE void *doSimpleMemSet(void *dest, int c, size_t size)
{
	register int cnt=-1;
	while (++cnt < (long) size)
		((char*)dest)[cnt] = c;
	return(dest);
}

RTR_C_ALWAYS_INLINE void doLess4Set(void *dest, int c, size_t size)
{
	switch(size)
	{
	case 3:
		((char*)dest)[2] = c;
	case 2:
		((char*)dest)[1] = c;
	case 1:
		((char*)dest)[0] = c;
	}
}
/*****************************************/



/*****************************************/
/* MemSetByInt() works on quad boundaries and copies (int) 4 bytes at a time */

RTR_C_ALWAYS_INLINE void *doQuadAlignMemSet4(void *dest, int c, size_t size)
{
	register int cnt=-1;
	register unsigned int setIntChar=0;

	if (c != 0)
	{
		setIntChar = (unsigned char) c;
		setIntChar |= setIntChar << 8;
		setIntChar |= setIntChar << 16;
	}

	while (++cnt < (((long) size)>>2))
		((rtrUInt32*)dest)[cnt] = setIntChar;

	/* Copy any left over bytes, always less than 4 */
	if (size & 0x03)
		doLess4Set((char*)dest + (size & ~0x03), c, (size & 0x03));

	return(dest);
}

RTR_C_ALWAYS_INLINE void *doNonQuadAlignMemSet4(void *dest, int c, size_t size)
{

	if (size < 4)
	{
		doLess4Set(dest, c, size);
	}
	else
	{
		register size_t bytestoalign = alignbytes[((size_t)dest & 0x03)];
		doLess4Set(dest, c, bytestoalign);

		doQuadAlignMemSet4((char*)dest+bytestoalign,c,((long)size)-bytestoalign);
	}
	return(dest);
}

RTR_C_ALWAYS_INLINE void *doNonSimpleMemSet4(void *dest, int c, size_t size)
{
	return( ALIGN4BYTE(dest) ? doNonQuadAlignMemSet4(dest,c,size) :
		doQuadAlignMemSet4(dest,c,size) );
}

/*****************************************/



/*****************************************/
/* MemSetLongLong() works on quad boundaries, but copies (long long) or 8 bytes at a time */

RTR_C_ALWAYS_INLINE void *doQuadAlignMemSet8(void *dest, int c, size_t size)
{
	register int cnt=-1;
	register rtrUInt64 setIntChar=0;

	if (c != 0)
	{
		setIntChar = (unsigned char) c;
		setIntChar |= setIntChar << 8;
		setIntChar |= setIntChar << 16;
		setIntChar |= (setIntChar << 16) << 16;
	}

	while (++cnt < (((long) size)>>3))
		((rtrUInt64*)dest)[cnt] = setIntChar;

		/* Check for greater than 4 more bytes and perform int instruction */
	if (size & 0x04)
		((rtrUInt32*)dest)[cnt<<1] = (rtrUInt32)setIntChar;

	/* Copy any left over bytes, always less than 4 */
	if (size & 0x03)
		doLess4Set((char*)dest + (size & ~0x03), c, (size & 0x03));

	return(dest);
}

RTR_C_ALWAYS_INLINE void *doNonQuadAlignMemSet8(void *dest, int c, size_t size)
{
	register size_t remainder = align8bytes[((size_t)dest & 0x07)];
	register size_t bytestoalign = alignbytes[((size_t)dest & 0x03)];

	if (size < 8 + remainder)
	{
		doNonQuadAlignMemSet4(dest,c,size);
	}
	else
	{
		/* first set up to 4 byte alignment */
		doLess4Set(dest, c, bytestoalign);
		/* then, if necessary, set up to 8 byte alignment */
		if (remainder & 0x04)
		{
			register rtrUInt64 setIntChar=0;
			if (c != 0)
			{
				setIntChar = (unsigned char) c;
				setIntChar |= setIntChar << 8;
				setIntChar |= setIntChar << 16;
			}
			*((rtrUInt32*)((char *)(dest) + bytestoalign)) =
				(rtrUInt32) setIntChar;
		}

		doQuadAlignMemSet8((char*)dest+remainder,c,
			((long)size)-(long)remainder);
	}
	return(dest);
}

RTR_C_ALWAYS_INLINE void *doNonSimpleMemSet8(void *dest, int c, size_t size)
{
	return( ALIGN8BYTE(dest) ? doNonQuadAlignMemSet8(dest,c,size) :
		doQuadAlignMemSet8(dest,c,size) );
}
/*****************************************/



/*****************************************/
/* Internal helper functions, do not use */
RTR_C_ALWAYS_INLINE int doSimpleMemCmp(void *dest, const void *src, size_t size)
{
	register size_t cnt;
	for (cnt = 0; cnt < size; ++cnt)
		if (((char*)dest)[cnt] != ((char*)src)[cnt])
			return(((char*)dest)[cnt] - ((char*)src)[cnt]);
	return(0);
}

RTR_C_ALWAYS_INLINE int doLess4Cmp(void *dest, const void *src, size_t size)
{
	switch(size)
	{
	case 3:
		if (((char*)dest)[2] != ((char*)src)[2])
			return((((char*)dest)[2] - ((char*)src)[2]));
	case 2:
		if (((char*)dest)[1] != ((char*)src)[1])
			return((((char*)dest)[1] - ((char*)src)[1]));
	case 1:
		if (((char*)dest)[0] != ((char*)src)[0])
			return((((char*)dest)[0] - ((char*)src)[0]));
	}
	return(0);
}

/*****************************************/

/*****************************************/
/* MemCmpByInt() works on quad boundaries */

RTR_C_ALWAYS_INLINE int doQuadAlignMemCmp4(void *dest, const void *src, size_t size)
{
	register int cnt=-1;
	register int allreadydone;

	while (++cnt < (((long) size)>>2))
		if (((rtrUInt32*)dest)[cnt] != ((rtrUInt32*)src)[cnt])
			return(((rtrUInt32*)dest)[cnt] - ((rtrUInt32*)src)[cnt]);

	if (size & 0x03)
	{
		allreadydone = (size & ~0x03);
		return(doLess4Cmp((char*)dest + allreadydone,
			(char*)src + allreadydone, (size & 0x03)));
	}
	return(0);
}

RTR_C_ALWAYS_INLINE int doNonQuadAlignMemCmp4(void *dest, const void *src, size_t size)
{
	register size_t bytestoalign = alignbytes[((size_t)dest & 0x03)];
	if ((ALIGN4BYTE(dest) == ALIGN4BYTE(src)) &&
		(size >= 4+bytestoalign))
	{
		register int ret;

		if ((ret = doLess4Cmp(dest, src, bytestoalign)) != 0)
			return(ret);

		return(doQuadAlignMemCmp4((char*)dest+bytestoalign,
			(char*)src+bytestoalign,((long)size)-bytestoalign));
	}
	else
		return(doSimpleMemCmp(dest,src,size));
}

RTR_C_ALWAYS_INLINE int doNonSimpleMemCmp4(void *dest, const void *src, size_t size)
{
	return( ALIGN4BYTE(dest) || ALIGN4BYTE(src) ?
		doNonQuadAlignMemCmp4(dest,src,size) :
		doQuadAlignMemCmp4(dest,src,size) );
}

/*****************************************/

/*****************************************/
/* MemCmpByLongLong() works on quad boundaries, but copies (long long) or 8 bytes at a time */

RTR_C_ALWAYS_INLINE int doQuadAlignMemCmp8(void *dest, const void *src, size_t size)
{
	register int cnt=-1;
	register int allreadydone;

	while (++cnt < (((long) size)>>3))
		if (((rtrUInt64*)dest)[cnt] != ((rtrUInt64*)src)[cnt])
			return (int)(((rtrUInt64*)dest)[cnt] - ((rtrUInt64*)src)[cnt]);

		/* Check for greater than 4 more bytes and perform int instruction */
	if ((size & 0x04) && (((rtrUInt32*)dest)[cnt<<1] != ((rtrUInt32*)src)[cnt<<1]))
		return(((rtrUInt32*)dest)[cnt<<1] - ((rtrUInt32*)src)[cnt<<1]);

	if (size & 0x03)
	{
		allreadydone = (size & ~0x03);
		return(doLess4Cmp((char*)dest + allreadydone,
			(char*)src + allreadydone, (size & 0x03)));
	}
	return(0);
}

RTR_C_ALWAYS_INLINE int doNonQuadAlignMemCmp8(void *dest, const void *src, size_t size)
{
	register size_t remainder = align8bytes[((size_t)dest & 0x07)];
	if ((ALIGN8BYTE(dest) == ALIGN8BYTE(src)) &&
		(size >= 8+remainder))
	{
		register size_t bytestoalign = alignbytes[((size_t)dest & 0x03)];
		register int ret;

		if ((ret = doLess4Cmp(dest, src, bytestoalign)) != 0)
			return(ret);
		/* if not 8 byte aligned, check next 4 bytes with int instruction */
		if ((remainder & 0x04) &&
			(*((rtrUInt32*) ((char *) (dest) + bytestoalign)) !=
				*((rtrUInt32*) ((char *) (src) + bytestoalign))))
		{
			return (*((rtrUInt32*) ((char *) (dest) + bytestoalign)) -
				*((rtrUInt32*) ((char *) (src) + bytestoalign)));
		}

		return(doQuadAlignMemCmp8((char*)dest+remainder,
			(char*)src+remainder,((long)size)-remainder));
	}
	else
		return(doSimpleMemCmp(dest,src,size));
}

RTR_C_ALWAYS_INLINE int doNonSimpleMemCmp8(void *dest, const void *src, size_t size)
{
	return( ALIGN8BYTE(dest) || ALIGN8BYTE(src) ?
		doNonQuadAlignMemCmp8(dest,src,size) :
		doQuadAlignMemCmp8(dest,src,size) );
}

/*****************************************/

#endif	//	__INTCUSTMEM_H__
