/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_NETWORK_H
#define __RTR_NETWORK_H

#include "rtr/byteswap.h"
#include "rtr/rwfNet.h"

#ifndef RTR_QUAD_32_ALIGN

#define rwfPut8(___dptr,___sval) RTR_PUT_8(___dptr,___sval)
#define rwfGet8(___dval,___sptr) RTR_GET_8(___dval,___sptr)
#define rwfPut16(___dptr,___sval) RTR_PUT_16(___dptr,___sval)
#define rwfGet16(___dval,___sptr) RTR_GET_16(___dval,___sptr)
#define rwfPut24(___dptr,___sval) RTR_PUT_24(___dptr,___sval)
#define rwfGet24(___dval,___sptr) RTR_GET_24(___dval,___sptr)
#define rwfPut32(___dptr,___sval) RTR_PUT_32(___dptr,___sval)
#define rwfGet32(___dval,___sptr) RTR_GET_32(___dval,___sptr)
#define rwfPut64(___dptr,___sval) RTR_PUT_64(___dptr,___sval)
#define rwfGet64(___dval,___sptr) RTR_GET_64(___dval,___sptr)
#define rwfPutFloat(___dptr,___sval) RTR_PUT_FLOAT(___dptr,___sval)
#define rwfGetFloat(___dval,___sptr) RTR_GET_FLOAT(___dval,___sptr)
#define rwfPutDouble(___dptr,___sval) RTR_PUT_DOUBLE(___dptr,___sval)
#define rwfGetDouble(___dval,___sptr) RTR_GET_DOUBLE(___dval,___sptr)

#define RWF_PUT_8_8(___dptr,___fval,___sval) \
	(*((rtrUInt16*)___dptr) = ((___sval << 8) | ___fval))



RTR_C_ALWAYS_INLINE rtrInt32 rwfPutLenSpecBlank(char *dptr)
{
	return rwfPut8(dptr,0);
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfPutLenSpecU16(char *dptr, rtrUInt16 sval)
{
	if (sval <= (rtrUInt16)0xFF) {
		RWF_PUT_8_8(dptr,0x01,sval);
		return 2;
	}
	*(rtrUInt8*)dptr = 2;
	RTR_PUT_16((dptr+1),sval);
	return 3;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecU16(rtrUInt16 *dvalptr, char *sptr)
{
	register rtrUInt32 tval = *(rtrUInt32*)sptr;
	switch (tval & 0xFF)
	{
		case 0:
			*dvalptr = 0;
			return 1;
		case 1:
			*dvalptr = (tval & 0xFF00) >> 8;
			return 2;
		case 2:
			*dvalptr = __rtr_bswap16( (rtrUInt16)(tval >> 8) );
			return 3;
	}

	return -1;
}


RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecU16_Size(rtrUInt16 *dvalptr, char *sptr, rtrUInt16 size)
{
	register rtrUInt16 tval = *(rtrUInt16*)sptr;
	switch (size)
	{
		case 0:
			*dvalptr = 0;
			return 0;
		case 1:
			*dvalptr = (tval & 0x00FF);
			return 1;
		case 2:
			*dvalptr = __rtr_bswap16( tval );
			return 2;
	}
	return -1;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfPutOptByteU16(char *dptr, rtrUInt16 sval)
{
	if (sval < (rtrUInt16)0xFE) {
		*(rtrUInt8*)dptr = (rtrUInt8)sval;
		return 1;
	}
	*(rtrUInt8*)dptr = 0xFE;
	*(rtrUInt16*)(dptr+1) = __rtr_bswap16(sval);
	return 3;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetOptByteU16(rtrUInt16 *dvalptr, char *sptr)
{
	register rtrUInt8 temp = (*(rtrUInt8*)sptr);
	if (temp < 0xFE) {
		*dvalptr = (rtrUInt16)temp;
		return 1;
	}
	*dvalptr = __rtr_bswap16(*(rtrUInt16*)(sptr+1));
	return 3;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfPutResBitU15(char *dptr, rtrUInt16 sval)
{
	if (sval < (rtrUInt16)0x80) {
		*(rtrUInt8*)dptr = (rtrUInt8)sval;
		return 1;
	}
	sval |= 0x8000;
	RTR_PUT_16(dptr,sval);
	return 2;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetResBitU15(rtrUInt16 *dvalptr, char *sptr)
{
	register rtrUInt8 temp = (*(rtrUInt8*)sptr);
	if (temp & 0x80) {
		*dvalptr = __rtr_bswap16(*(rtrUInt16*)sptr) & 0x7FFF;
		return 2;
	}
	*dvalptr = (rtrUInt16)temp;
	return 1;
}

/* ResBitU22 -- Used by TRWF */

RTR_C_ALWAYS_INLINE rtrInt32 rwfPutResBitU22(char *dptr, const rtrUInt32 *sptr)
{
	register rtrUInt32 temp = (*(rtrUInt32*)sptr);
	if (temp < (rtrUInt32)0x80) 
	{
		*(rtrUInt8*)dptr = temp;
		return 1;
	}
	
	if (temp < (rtrUInt32)0x4000)
	{
		temp |= 0x8000;
		RTR_PUT_16(dptr,temp);
		return 2;
	}
	
	if (temp < (rtrUInt32)0x400000)
	{
		//rtrUInt32 temp2 = *sptr | 0xC00000;
		temp |= 0xC00000;
		RTR_PUT_24(dptr, temp);
		return 3;
	}
	
	return -1;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetResBitU22(rtrUInt32 *dvalptr, const char *sptr)
{
	register rtrUInt8 temp = *sptr;
	if ((temp & (rtrUInt8)0x80) == 0)
	{
		*dvalptr = temp & 0x7F; 
		return 1;
	}
	else if ((temp & 0xC0) == (rtrUInt8)0x80)
	{
		*dvalptr = __rtr_bswap16(*(rtrUInt16*)sptr) & 0x3FFF;
		return 2;
	}
	
	RTR_GET_24(*dvalptr, sptr); *dvalptr &= 0x3FFFFF;
	return 3;
	
}

/* ResBitU30 - used by TRWF and RWF */
/* ResBitU31 -- Used by TRWF */
/* 00 -- 1 byte (6 bits)
 * 10 -- 2 bytes (14 bits)
 * 01 -- 3 bytes (22 bits)
 * 11 -- 4 bytes (30 bits) */
RTR_C_ALWAYS_INLINE rtrInt32 rwfPutResBitU30(char *dptr, const rtrUInt32 *sptr)
{
	if (*sptr < (rtrUInt32)0x40) /* 6b */
	{
		register rtrUInt8 temp = (*(rtrUInt8*)sptr);
		rwfPut8(dptr, temp);
		return 1;
	}
	else if (*sptr < (rtrUInt32)0x4000) /* 14 b */
	{
		register rtrUInt16 temp = ((*(rtrUInt16*)sptr) | 0x8000);
		rwfPut16(dptr, temp);
		return 2;
	}
	else if (*sptr < (rtrUInt32)0x400000) /* 22b */
	{
		/* Note: RTR_PUT_24 still puts 4 bytes.  
           If this is replacing memory (like the size marks)
  		   this could overwrite valid bytes.  This was not seen
  		   in testing, but if it is seen this should be changed
		   to RWF_PUT_24 which only puts 3 bytes */
		RTR_PUT_24(dptr, *sptr);
		return 3;
	}
	else if (*sptr < (rtrUInt32)0x80000000) /* 30b */
	{
		rtrUInt32 temp = *sptr | 0xC0000000;
		rwfPut32(dptr, temp);
		return 4;
	}

	return -1;
}

/* Used by TRWF and RWF */
RTR_C_ALWAYS_INLINE rtrInt32 rwfGetResBitU30(rtrUInt32 *dvalptr, const char *sptr)
{
	register rtrUInt8 temp = *sptr;
	
	switch(temp & 0xC0)
	{
		case 0: /* 6b */
			*dvalptr = (rtrUInt32)temp;
			return 1;
		break;
		case 64: /* 22b */
			RTR_GET_24(*dvalptr, sptr);
			*dvalptr &= 0x003FFFFF;
			return 3;
		break;
		case 128: /* 14b */
			*dvalptr = (rtrUInt32)__rtr_bswap16( (*(rtrUInt16*)(sptr)) );
			*dvalptr &= 0x00003FFF;
			return 2;
		break;
		case 192: /* 30b */
			rwfGet32(*dvalptr, sptr); 
			*dvalptr &= 0x3FFFFFFF;
			return 4;
		break;
	}
	return -1;

}


/* ResBitU31 -- Used by TRWF */
/* 0 -- 3 bytes(23 bits)
 * 1 -- 4 bytes(31 bits) */
RTR_C_ALWAYS_INLINE rtrInt32 rwfPutResBitU31(char *dptr, const rtrUInt32 *sptr)
{
	if (*sptr < (rtrUInt32)0x800000) /* 23b */
	{
		/* Note: RTR_PUT_24 still puts 4 bytes.  
           If this is replacing memory (like the size marks)
  		   this could overwrite valid bytes.  This was not seen
  		   in testing, but if it is seen this should be changed
		   to RWF_PUT_24 which only puts 3 bytes */
		RTR_PUT_24(dptr, *sptr);
		return 3;
	}
	else if (*sptr < (rtrUInt32)0x80000000) /* 31b */
	{
		rtrUInt32 temp = *sptr | 0x80000000;
		rwfPut32(dptr, temp);
		return 4;
	}

	return -1;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetResBitU31(rtrUInt32 *dvalptr, const char *sptr)
{
	register rtrUInt8 temp = *sptr;

	if ((temp & (rtrUInt8)0x80) == 0) /* 23b */
	{
		RTR_GET_24(*dvalptr, sptr);
		return 3;
	}

	/* 31b */
	rwfGet32(*dvalptr, sptr); *dvalptr &= ~(0x80000000);
	return 4;
}

/* ResBitI30 -- Used by TRWF */
/* 0 -- 3 bytes(22 bits)
 * 1 -- 4 bytes(30 bits) */
/* 2nd from top bit is a sign bit */
RTR_C_ALWAYS_INLINE rtrInt32 rwfPutResBitI30(char *dptr, const rtrInt32 *sptr)
{
	rtrInt32 temp = *sptr;

	if ( temp < (rtrInt32)0xC0000000 || temp >= (rtrInt32)0x40000000) /* Invalid */
		return -1;

	if ( temp < (rtrInt32)0xFFC00000 || temp > (rtrInt32)0x003FFFFF) 
	{
		/* 30b */
		temp |= 0x80000000; /* Add resbit */
		rwfPut32(dptr, temp);
		return 4;
	}

	/* 22b */
	temp &= ~0x800000; /* set res bit = 0 */
	/* Same note as above - if problems, replace with
 	   RWF_PUT_24 */
	RTR_PUT_24(dptr, temp);
	return 3;
}


RTR_C_ALWAYS_INLINE rtrInt32 rwfReplaceResBitI30(char *dptr, const rtrInt32 *sptr)
{
	rtrInt32 temp = *sptr;

	if ( temp < (rtrInt32)0xC0000000 || temp >= (rtrInt32)0x40000000)
		return -1; /* Invalid */

	if (*dptr & (rtrInt8)0x80)
	{
		/* 30b can fit any valid value */
		temp |= 0x80000000; /* Add resbit */
		rwfPut32(dptr, temp);
		return 4;
	}

	if ( temp < (rtrInt32)0xFFC00000 || temp > (rtrInt32)0x003FFFFF)
		return 0; /* Too big -- 22b can't fit 30b value */

	temp &= ~0x800000; /* set res bit = 0 */
	/* Since this is a 'replace' function, 
   	   assuming it replaces a value already on the 
	   wire so it should use RWF_PUT_24 */
	   RWF_PUT_24(dptr, &temp); 
	return 3;
}


RTR_C_ALWAYS_INLINE rtrInt32 rwfGetResBitI30(rtrInt32 *dvalptr, const char *sptr)
{
	if (*sptr & (rtrInt8)0x80)
	{
		/* 30b */
		rwfGet32(*dvalptr, sptr);

		if (*dvalptr & 0x40000000) 
			*dvalptr |= 0xC0000000; /* sign extend */
		else
			*dvalptr &= ~0x80000000; /* take resbit off */

		return 4;
	}

	/* 22b */
	RTR_GET_24(*dvalptr, sptr);

	if (*dvalptr & 0x00400000) 
		*dvalptr |= 0xFFC00000; /* sign extend */
	/* else res bit already 0 */

	return 3;

}

/* Missing Buffer Put functions */
/* Missing Buffer15 */


	/* This function does not perform any buffer size checking.
	 * Can use the _rsslValidBufferPtr(), _rsslInvalidBufferPointer(),
	 * _rsslValidBufferPointerEndPtr() or _rsslInvalidBufferPointerEndPtr()
	 * after to check since no data copy is happening.
	 */
RTR_C_ALWAYS_INLINE rtrInt32 rwfGetBuffer16(RwfBuffer *buf, char *sptr)
{
	register rtrUInt8 temp = (*(rtrUInt8*)sptr);
	if (temp < (rtrUInt8)0xFE) {
		buf->length = (rtrInt32)temp;
		buf->data = sptr + 1;
		return 1 + buf->length;
	} else if (temp != 0) {
		buf->length = (rtrInt32)__rtr_bswap16(*(rtrUInt16*)(sptr+1));
		buf->data = sptr + 3;
		return 3 + buf->length;
	} 

	buf->length = 0;
	buf->data = 0;
	return 1;
	
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetBuffer8(RwfBuffer *buf, char *sptr)
{
	register rtrUInt8 temp = (*(rtrUInt8*)sptr);
	if (temp > 0)
	{
		buf->length = (rtrInt32)temp;
		buf->data = sptr + 1;
		return 1 + temp;
	}
	buf->length = 0;
	buf->data = 0;
	return 1;
}


/* .................
 */



/* Put functions for LenSpec RWF Int/UInt32s */
/* Performing the write of the length, moving dptr and shifting the value appears to perform
   better than or'ing the length and swapping, due to the need to mask out the type byte */
RTR_C_ALWAYS_INLINE rtrInt32 trwfPutLenSpecI32(char *dptr, rtrInt32 sval)
{
	register rtrUInt32 ckval = (sval >= 0) ? (sval << 1) : (((-sval) - 1) << 1);  // shift here instead of each if/else if statement
	if (ckval & 0xFF000000 ) {
		*((rtrUInt8*)dptr) = 0x80;  // top three bits set to 100
		*((rtrUInt32*)(dptr+1)) = __rtr_bswap32( (rtrUInt32)sval);
		return 5;
	} else if (ckval & 0x00FF0000) { 
		*((rtrUInt8*)dptr) = 0x60;  // top 3 bits set to 011
		*((rtrUInt32*)(dptr+1)) = __rtr_bswap32((rtrUInt32)sval << 8);
		//*((rtrUInt32*)(dptr)) = __rtr_bswap32(( (((rtrUInt32)sval )) & 0x00FFFFFF) | 0x60000000);
		return 4;
	} else if (ckval & 0x0000FF00 ) {
		*((rtrUInt8*)dptr) = 0x40;  // top 3 bits set to 010
		*((rtrUInt16*)(dptr+1)) = __rtr_bswap16( (rtrUInt16)sval);
		return 3;
	} 
	
	*((rtrUInt16*)dptr) = (((rtrUInt16) sval << 8) | 0x20);  // top 3 bits set to 001
	return 2;

}

/* Performing the write of the length, moving dptr and shifting the value appears to perform
   better than or'ing the length and swapping, due to the need to mask out the type byte */
RTR_C_ALWAYS_INLINE rtrInt32 trwfPutLenSpecI64(char *dptr, rtrInt64 sval)
{
	if ((sval >= RTR_LL(-2147483648)) && (sval <= RTR_LL(2147483647))) {
		return trwfPutLenSpecI32(dptr,(rtrInt32)sval);
	} else {
		register rtrUInt64 ckval = (sval >= 0) ? (sval << 1) : ((-sval - 1) << 1); // shift once here instead of each if/else if
		if (ckval & RTR_ULL(0xFF00000000000000) ) {
			*((rtrUInt16*)dptr) = (rtrUInt16)0x081F;  // should give us a first byte of 00011111 (primitive code bailout and second byte of 0x08 telling us 8 bytes follow 
			*((rtrUInt64*)(dptr+2)) = __rtr_bswap64( (rtrUInt64)sval);
			return 10;
		} else if (ckval & RTR_ULL(0x00FF000000000000) ) {
			*((rtrUInt8*)dptr) = 0xE0;  // set top three bits to 111
			*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval << 8);
			//*((rtrUInt64*)(dptr)) = __rtr_bswap64( (((rtrUInt64)sval) & RTR_ULL(0x00FFFFFFFFFFFFFF)) | 0xE000000000000000);
			return 8;
		} else if (ckval & RTR_ULL(0x0000FF0000000000) ) {
			*((rtrUInt8*)dptr) = 0xC0; // set top three bits to 110
			*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval << 16);
			//*((rtrUInt64*)(dptr)) = __rtr_bswap64( (((rtrUInt64)sval << 8) & RTR_ULL(0x00FFFFFFFFFFFFFF)) | 0xC000000000000000);
			return 7;
		} 

		*((rtrUInt8*)dptr) = 0xA0;  // set top three bits to 101
		*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval << 24);
		//*((rtrUInt64*)(dptr)) = __rtr_bswap64( (((rtrUInt64)sval << 16) & 0x00FFFFFFFFFFFFFF) | 0xA000000000000000);
		return 6;
		
	}
}



/* Put functions for LenSpec RWF Int/UInt32s */
RTR_C_ALWAYS_INLINE rtrInt32 rwfPutLenSpecI32(char *dptr, rtrInt32 sval)
{
	register rtrUInt32 ckval = (sval >= 0) ? (sval << 1) : (((-sval) - 1) << 1);  // shift here instead of each if/else if statement
	if (ckval & 0xFF000000 ) {
		*((rtrUInt8*)dptr) = 0x04;
		*((rtrUInt32*)(dptr+1)) = __rtr_bswap32( (rtrUInt32)sval);
		return 5;
	} else if (ckval & 0x00FF0000) { 
		*((rtrUInt8*)dptr) = 0x03;
		*((rtrUInt32*)(dptr+1)) = __rtr_bswap32( (rtrUInt32)sval << 8);
		return 4;
	} else if (ckval & 0x0000FF00 ) {
		*((rtrUInt8*)dptr) = 0x02;
		*((rtrUInt16*)(dptr+1)) = __rtr_bswap16( (rtrUInt16)sval);
		return 3;
	} 
		
	*((rtrUInt16*)dptr) = (((rtrUInt16) sval << 8) | 0x01);
	return 2;
	
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecI32(rtrInt32 *svalptr, char *sptr)
{
	register rtrUInt8 len = (*(rtrUInt8*)(sptr));
		
	switch (len)
	{
		case 0:
			*svalptr = 0;
			return 1;
		break;
		case 1:
			*svalptr = (rtrInt32)((*(rtrUInt8*)(sptr+1)));
			if (*svalptr & 0x80)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFFFF00);
			return 2;
		break;
		case 2:
			*svalptr = (rtrInt32)__rtr_bswap16( *((rtrUInt16*)(sptr+1)));
			if (*svalptr & 0x8000)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFF0000);
			return 3;
		break;
		case 3:
			// there are actually 4 bytes on wire.  mask out length and swap
			*svalptr = (rtrInt32)__rtr_bswap32( (*(rtrUInt32*)(sptr)) & 0xFFFFFF00);
			if (*svalptr & 0x800000)
				*svalptr |= RTR_LL(0xFFFFFFFFFF000000);
			return 4;
		break;
		case 4:
			*svalptr = (rtrInt32)__rtr_bswap32( *((rtrUInt32*)(sptr+1)));
			if (*svalptr & 0x80000000)
				*svalptr |= RTR_LL(0xFFFFFFFF00000000);
			return 5;
		break;
		default: 
			return -1;
	}
}





RTR_C_ALWAYS_INLINE rtrInt32 rwfPutLenSpecI64(char *dptr, rtrInt64 sval)
{
	if ((sval >= RTR_LL(-2147483648)) && (sval <= RTR_LL(2147483647))) {
		return rwfPutLenSpecI32(dptr,(rtrInt32)sval);
	} else {
		register rtrUInt64 ckval = (sval >= 0) ? (sval << 1) : (((-sval) - 1) << 1); // shift once here instead of each if/else if
		if (ckval & RTR_ULL(0xFF00000000000000) ) {
			*((rtrUInt8*)dptr) = 0x08;
			*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval);
			return 9;
		} else if (ckval & RTR_ULL(0x00FF000000000000) ) {
			*((rtrUInt8*)dptr) = 0x07;
			*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval << 8);
			return 8;
		} else if (ckval & RTR_ULL(0x0000FF0000000000) ) {
			*((rtrUInt8*)dptr) = 0x06;
			*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval << 16) ;
			return 7;
		} 

		*((rtrUInt8*)dptr) = 0x05;
		*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval << 24)  ;
		return 6;
		
	}
}


RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecI64(rtrInt64 *svalptr, char *sptr)
{
	register rtrUInt8 len = (*(rtrUInt8*)(sptr));
		
	switch (len)
	{
		case 0:
			*svalptr = 0;
			return 1;
		case 1:
			*svalptr = (rtrInt64)((*(rtrUInt8*)(sptr+1)));
			if (*svalptr & 0x80)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFFFF00);
			return 2;
		case 2:
			*svalptr = (rtrInt64)__rtr_bswap16( *((rtrUInt16*)(sptr+1) ));
			if (*svalptr & 0x8000)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFF0000);
			return 3;
		case 3:
			// there are actually 4 bytes on wire.  mask out length and swap
			*svalptr = (rtrInt64)__rtr_bswap32( (*(rtrUInt32*)(sptr)) & 0xFFFFFF00);
			if (*svalptr & 0x800000)
				*svalptr |= RTR_LL(0xFFFFFFFFFF000000);
			return 4;
		case 4:
			*svalptr = (rtrInt64)__rtr_bswap32( *((rtrUInt32*)(sptr+1)));
			if (*svalptr & 0x80000000)
				*svalptr |= RTR_LL(0xFFFFFFFF00000000);
			return 5;
		case 5:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_I64_SIZE(svalptr,(sptr+1),len);
//#else 
			// only 6 bytes on wire including length.  swap 5 byte value and shift over three bytes before swap
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+1)) << 24) );
			if (*svalptr & RTR_LL(0x8000000000))
				*svalptr |= RTR_LL(0xFFFFFF0000000000);
			return 6;
//#endif
		case 6:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_I64_SIZE(svalptr, (sptr+1),len);
//#else
			// only 7 bytes on wire - causes problem on windows
			// byteswap 8 byte value (2 bytes arent ours), but we need to shift over 16 before we swap
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+1)) << 16 ) );
			if (*svalptr & RTR_LL(0x800000000000))
				*svalptr |= RTR_LL(0xFFFF000000000000);
			return 7;
//#endif
		case 7:
			// there are a full 8 bytes on wire, byte swap with length, but mask it off
			*svalptr = (rtrInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) & RTR_LL(0xFFFFFFFFFFFFFF00) );
			if (*svalptr & RTR_LL(0x80000000000000))
				*svalptr |= RTR_LL(0xFF00000000000000);
			return 8;
		case 8:
			// full 8 bytes on wire for value
			*svalptr = (rtrInt64)__rtr_bswap64( *((rtrUInt64*)(sptr+1)) );
			return 9;
	}
	return -1;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecI64Size(rtrInt64 *svalptr, char *sptr, rtrUInt8 size)
{
		
	switch (size)
	{
		case 0:
			*svalptr = 0;
			return 0;
		case 1:
			*svalptr = (rtrInt64)((*(rtrUInt8*)(sptr)));
			if (*svalptr & 0x80)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFFFF00);
			return 1;
		case 2:
			*svalptr = (rtrInt64)__rtr_bswap16( *((rtrUInt16*)(sptr) ));
			if (*svalptr & 0x8000)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFF0000);
			return 2;
		case 3:
			// there are actually 4 bytes on wire.  mask out length and swap
			*svalptr = (rtrInt64)__rtr_bswap32( (*(rtrUInt32*)(sptr)) << 8);
			if (*svalptr & 0x800000)
				*svalptr |= RTR_LL(0xFFFFFFFFFF000000);
			return 3;
		case 4:
			*svalptr = (rtrInt64)__rtr_bswap32( *((rtrUInt32*)(sptr)));
			if (*svalptr & 0x80000000)
				*svalptr |= RTR_LL(0xFFFFFFFF00000000);
			return 4;
		case 5:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_I64_SIZE(svalptr,(sptr+1),len);
//#else 
			// only 6 bytes on wire including length.  swap 5 byte value and shift over three bytes before swap
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr)) << 24) );
			if (*svalptr & RTR_LL(0x8000000000))
				*svalptr |= RTR_LL(0xFFFFFF0000000000);
			return 5;
//#endif
		case 6:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_I64_SIZE(svalptr, (sptr+1),len);
//#else
			// only 7 bytes on wire - causes problem on windows
			// byteswap 8 byte value (2 bytes arent ours), but we need to shift over 16 before we swap
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr)) << 16 ) );
			if (*svalptr & RTR_LL(0x800000000000))
				*svalptr |= RTR_LL(0xFFFF000000000000);
			return 6;
//#endif
		case 7:
			// there are a full 8 bytes on wire, byte swap with length, but mask it off
			*svalptr = (rtrInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) << 8 );
			if (*svalptr & RTR_LL(0x80000000000000))
				*svalptr |= RTR_LL(0xFF00000000000000);
			return 7;
		case 8:
			// full 8 bytes on wire for value
			*svalptr = (rtrInt64)__rtr_bswap64( *((rtrUInt64*)(sptr)) );
			return 8;
	}
	return -1;
}



RTR_C_ALWAYS_INLINE rtrInt32 rwfPutLenSpecU32(char *dptr, rtrUInt32 sval)
{
	if (sval & 0xFF000000 ) {
		*((rtrUInt8*)dptr) = 0x04;
		*((rtrUInt32*)(dptr+1)) = __rtr_bswap32( (rtrUInt32)sval);
		return 5;
	} else if (sval & 0x00FF0000) { 
		//*((rtrUInt8*)dptr) = 0x03;
		*((rtrUInt32*)(dptr)) = __rtr_bswap32( (rtrUInt32)sval |0x03000000);
		return 4;
	} else if (sval & 0x0000FF00 ) {
		*((rtrUInt8*)dptr) = 0x02;
		*((rtrUInt16*)(dptr+1)) = __rtr_bswap16( (rtrUInt16)sval);
		return 3;
	} 

	*((rtrUInt16*)dptr) = __rtr_bswap16((rtrUInt16)sval | 0x0100); //(((rtrUInt16) sval << 8) | 0x01);
	return 2;
	
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecU32(rtrUInt32 *svalptr, char *sptr)
{
	register rtrUInt8 len = (*(rtrUInt8*)(sptr));
		
	switch (len)
	{
		case 0:
			*svalptr = 0;
			return 1;
		case 1:
			*svalptr = (rtrUInt32)((*(rtrUInt8*)(sptr+1)));
			return 2;
		case 2:
			*svalptr = (rtrUInt32)__rtr_bswap16( *((rtrUInt16*)(sptr+1) ));
			return 3;
		case 3: 
			// there are 4 bytes on wire including length.  mask that out and swap
			*svalptr = (rtrUInt32)__rtr_bswap32((*(rtrUInt32*)(sptr)) & 0xFFFFFF00) ;
			return 4;
		case 4:
			*svalptr = (rtrUInt32)__rtr_bswap32( *((rtrUInt32*)(sptr+1)) );
			return 5;
	}
	return -1;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfPutLenSpecU64(char *dptr, rtrUInt64 sval)
{
	if (sval <= RTR_LL(4294967295)) {
		return rwfPutLenSpecU32(dptr,(rtrUInt32)sval);
	} else {
		if (sval & RTR_ULL(0xFF00000000000000) ) {
			*((rtrUInt8*)dptr) = 0x08;
			*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval);
			return 9;
		} else if (sval & RTR_ULL(0x00FF000000000000) ) {
		//	*((rtrUInt8*)dptr) = 0x07;
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval | RTR_ULL(0x0700000000000000));
			return 8;
		} else if (sval & RTR_ULL(0x0000FF0000000000) ) {
			//*((rtrUInt8*)dptr) = 0x06;
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( ((rtrUInt64)sval << 8) | RTR_ULL(0x0600000000000000)) ;
			return 7;
		} 
		
		//*((rtrUInt8*)dptr) = 0x05;
		*((rtrUInt64*)(dptr)) = __rtr_bswap64( ((rtrUInt64)sval << 16) | RTR_ULL(0x0500000000000000)) ;
		return 6;
		

	}
}


RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecU64(rtrUInt64 *svalptr, char *sptr)
{
	register rtrUInt8 len = (*(rtrUInt8*)(sptr));
		
	switch (len)
	{
		case 0:
			*svalptr = 0;
			return 1;
		case 1:
			*svalptr = (rtrUInt64)((*(rtrUInt8*)(sptr+1)));
			return 2;
		case 2:
			*svalptr = (rtrUInt64)__rtr_bswap16( *((rtrUInt16*)(sptr+1) ));
			return 3;
		case 3:
			// there are 4 bytes on the wire.  mask out length and swap all 4
			*svalptr = (rtrUInt64)__rtr_bswap32( (*(rtrUInt32*)(sptr)) & 0xFFFFFF00) ;
			return 4;
		case 4:
			*svalptr = (rtrUInt64)__rtr_bswap32( *((rtrUInt32*)(sptr+1)) );
			return 5;
		case 5:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_U64_SIZE(svalptr,(sptr+1),len);
//#else 
			// only 6 bytes on wire including length.  swap 5 byte value and shift over three bytes before swap
			*svalptr = (rtrUInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+1)) << 24) );
			return 6;
//#endif
		case 6:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_U64_SIZE(svalptr, (sptr+1),len);
//#else
			// only 7 bytes on wire - causes problem on windows
			// byteswap 8 byte value (2 bytes arent ours), but we need to shift over 16 before we swap
			*svalptr = (rtrUInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+1)) << 16 ) );
			return 7;
//#endif
		case 7:
			// there are a full 8 bytes on wire, byte swap with length, but mask it off
			*svalptr = (rtrUInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) & RTR_LL(0xFFFFFFFFFFFFFF00) );
			return 8;
		case 8:
			// full 8 bytes on wire for value
			*svalptr = (rtrUInt64)__rtr_bswap64( *((rtrUInt64*)(sptr+1)) );
			return 9;
	}
	return -1;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecU64Size(rtrUInt64 *svalptr, char *sptr, rtrUInt8 size)
{
		
	switch (size)
	{
		case 0:
			*svalptr = 0;
			return 0;
		case 1:
			*svalptr = (rtrUInt64)((*(rtrUInt8*)(sptr)));
			return 1;
		case 2:
			*svalptr = (rtrUInt64)__rtr_bswap16( *((rtrUInt16*)(sptr) ));
			return 2;
		case 3: /*to do*/
			// there are 4 bytes on the wire.  mask out length and swap all 4
			*svalptr = (rtrUInt64)__rtr_bswap32( (*(rtrUInt32*)(sptr)) << 8) ;
			return 3;
		case 4:
			*svalptr = (rtrUInt64)__rtr_bswap32( *((rtrUInt32*)(sptr)) );
			return 4;
		case 5:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_U64_SIZE(svalptr,(sptr+1),len);
//#else 
			// only 6 bytes on wire including length.  swap 5 byte value and shift over three bytes before swap
			*svalptr = (rtrUInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr)) << 24) );
			return 5;
//#endif
		case 6:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_U64_SIZE(svalptr, (sptr+1),len);
//#else
			// only 7 bytes on wire - causes problem on windows
			// byteswap 8 byte value (2 bytes arent ours), but we need to shift over 16 before we swap
			*svalptr = (rtrUInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr)) << 16) );
			return 6;
//#endif
		case 7:
			// there are a full 8 bytes on wire, byte swap with length, but mask it off
			*svalptr = (rtrUInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) << 8 );
			return 7;
		case 8:
			// full 8 bytes on wire for value
			*svalptr = (rtrUInt64)__rtr_bswap64( *((rtrUInt64*)(sptr)) );
			return 8;
	}
	return -1;
}
/* Need get functions, possibly functions with no length as well? */



/* this function assumes that if this is a primitive code it was already 
   handled - a new function can be written to return primitive codes as well if needed */
/* Because the switch values are not consecutive will this still get optimized out by the compiler?
   Is it better to shift the length bytes and just use 1, 2, 3, ...?  Yes */
RTR_C_ALWAYS_INLINE rtrInt32 trwfGetLenSpecI64(rtrInt64 *svalptr, char *sptr)
{
	
	register rtrUInt8 len = (*(rtrUInt8*)(sptr));
	/* shift length over 5 bits so that it will be 1, 2, .... */
	switch (len >> 5)
	{
		case 1:
			*svalptr = (rtrInt64)((*(rtrUInt8*)(sptr+1)));
			if (*svalptr & 0x80)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFFFF00);
			return 2;
		case 2:
			*svalptr = (rtrInt64)__rtr_bswap16( *((rtrUInt16*)(sptr+1) ));
			if (*svalptr & 0x8000)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFF0000);
			return 3;
		case 3:
			// there are actually 4 bytes on wire.  mask out length and swap
			*svalptr = (rtrInt64)__rtr_bswap32( (*(rtrUInt32*)(sptr)) & 0xFFFFFF00) ;
			if (*svalptr & 0x800000)
				*svalptr |= RTR_LL(0xFFFFFFFFFF000000);
			return 4;
		case 4:
			*svalptr = (rtrInt64)__rtr_bswap32( *((rtrUInt32*)(sptr+1)) );
			if (*svalptr & 0x80000000)
				*svalptr |= RTR_LL(0xFFFFFFFF00000000);
			return 5;
		case 5:
//#ifdef WIN32
//			return TRWF_GET_LENSPEC_I64_SIZE(svalptr,(sptr+1),len);
//#else 
			// only 6 bytes on wire including length.  swap 5 byte value and shift over three bytes before swap
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+1)) << 24) );
			if (*svalptr & RTR_LL(0x8000000000))
				*svalptr |= RTR_LL(0xFFFFFF0000000000);
			return 6;
//#endif
		case 6:
//#ifdef WIN32
//			return TRWF_GET_LENSPEC_I64_SIZE(svalptr, (sptr+1),len);
//#else
			// only 7 bytes on wire - causes problem on windows
			// byteswap 8 byte value (2 bytes arent ours), but we need to shift over 16 before we swap
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+1)) << 16 ) );
			if (*svalptr & RTR_LL(0x800000000000))
				*svalptr |= RTR_LL(0xFFFF000000000000);
			return 7;
//#endif
		case 7:
			// there are a full 8 bytes on wire, byte swap with length, but mask it off
			*svalptr = (rtrInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) & RTR_LL(0xFFFFFFFFFFFFFF00) );
			if (*svalptr & RTR_LL(0x80000000000000))
				*svalptr |= RTR_LL(0xFF00000000000000);
			return 8;

	}
	
	/* handle 8 byte escape case */
	if (len == 31)
	{
		// full 8 bytes on wire for value 
		// first byte is 31, second byte should be 8 but is unnecessary
		*svalptr = (rtrInt64)__rtr_bswap64( *((rtrUInt64*)(sptr+2)) );
		return 10;
	}
	
	return -1;
}

/* This function assumes size still needs to be shifted, like the #define version */
RTR_C_ALWAYS_INLINE rtrInt32 trwfGetLenSpecI64_Size(rtrInt64 *svalptr, char *sptr, rtrUInt8 size)
{
	
	switch (size >> 5)
	{
		case 1:
			*svalptr = (rtrInt64)(*(rtrUInt8*)(sptr));
			if (*svalptr & 0x80)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFFFF00);
			return 1;
		case 2:
			*svalptr = (rtrInt64)__rtr_bswap16( *(rtrUInt16*)(sptr));
			if (*svalptr & 0x8000)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFF0000);
			return 2;
		case 3:
			/* need to shift out byte that isnt ours */
			*svalptr = (rtrInt64)__rtr_bswap32( (*(rtrUInt32*)(sptr)) << 8) ;
			if (*svalptr & 0x800000)
				*svalptr |= RTR_LL(0xFFFFFFFFFF000000);
			return 3;
		case 4:
			*svalptr = (rtrInt64)__rtr_bswap32( *(rtrUInt32*)(sptr));
			if (*svalptr & 0x80000000)
				*svalptr |= RTR_LL(0xFFFFFFFF00000000);
			return 4;
		case 5:
//#ifdef WIN32
//			return TRWF_GET_LENSPEC_I64_SIZE(svalptr,(sptr+1),len);
//#else 
			// only 5 bytes on wire including length.  swap 5 byte value and shift over three bytes before swap
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr)) << 24) );
			if (*svalptr & RTR_LL(0x8000000000))
				*svalptr |= RTR_LL(0xFFFFFF0000000000);
			return 5;
//#endif
		case 6:
//#ifdef WIN32
//			return TRWF_GET_LENSPEC_I64_SIZE(svalptr, (sptr+1),len);
//#else
			// only 6 bytes on wire - causes problem on windows
			// byteswap 8 byte value (2 bytes arent ours), but we need to shift over 16 before we swap
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr)) << 16 ) );
			if (*svalptr & RTR_LL(0x800000000000))
				*svalptr |= RTR_LL(0xFFFF000000000000);
			return 6;
//#endif
		case 7:
			// there are 7 bytes on wire, byte swap with length, but mask it off
			*svalptr = (rtrInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) << 8 );
			if (*svalptr & RTR_LL(0x80000000000000))
				*svalptr |= RTR_LL(0xFF00000000000000);
			return 7;


	}

	if (size == 31)
	{
		// full 8 bytes on wire and the extra byte.  skip extra byte 
		*svalptr = (rtrInt64)__rtr_bswap64( *((rtrUInt64*)(sptr+1)));
		return 9;
	}

	return -1;
}


/* The NoLength functions - these are used when encoding TRWF Reals */
/* Format matches LenSpec encoding, without the length byte(s) */
/* Real decoding uses the existing functionality */
RTR_C_ALWAYS_INLINE rtrInt32 trwfPutLenSpecI32NoLength(char *dptr, rtrInt32 sval)
{
	register rtrUInt32 ckval = (sval >= 0) ? (sval << 1) : (((-sval) - 1) << 1);  // shift here instead of each if/else if statement
	if (ckval & 0xFF000000 ) {
		*((rtrUInt32*)(dptr)) = __rtr_bswap32( (rtrUInt32)sval);
		return 4;
	} else if (ckval & 0x00FF0000) { 
		*((rtrUInt32*)(dptr)) = __rtr_bswap32((rtrUInt32)sval << 8);
		//*((rtrUInt32*)(dptr)) = __rtr_bswap32(( (((rtrUInt32)sval )) & 0x00FFFFFF) | 0x60000000);
		return 3;
	} else if (ckval & 0x0000FF00 ) {
		*((rtrUInt16*)(dptr)) = __rtr_bswap16( (rtrUInt16)sval);
		return 2;
	} 
	
	*((rtrUInt8*)dptr) = ((rtrUInt8) sval);  // top 3 bits set to 001
	return 1;

}

RTR_C_ALWAYS_INLINE rtrInt32 trwfPutLenSpecU32NoLength(char *dptr, rtrUInt32 sval)
{
	if (sval & 0xFF000000 ) {
		*((rtrUInt32*)(dptr)) = __rtr_bswap32( (rtrUInt32)sval);
		return 4;
	} else if (sval & 0x00FF0000) { 
		*((rtrUInt32*)(dptr)) = __rtr_bswap32((rtrUInt32)sval << 8);
		//*((rtrUInt32*)(dptr)) = __rtr_bswap32(( (((rtrUInt32)sval )) & 0x00FFFFFF) | 0x60000000);
		return 3;
	} else if (sval & 0x0000FF00 ) {
		*((rtrUInt16*)(dptr)) = __rtr_bswap16( (rtrUInt16)sval);
		return 2;
	} 
	
	*((rtrUInt8*)dptr) = ((rtrUInt8) sval);  // top 3 bits set to 001
	return 1;

}

/* Performing the write of the length, moving dptr and shifting the value appears to perform
   better than or'ing the length and swapping, due to the need to mask out the type byte */
RTR_C_ALWAYS_INLINE rtrInt32 trwfPutLenSpecI64NoLength(char *dptr, rtrInt64 sval)
{
	if ((sval >= RTR_LL(-2147483648)) && (sval <= RTR_LL(2147483647))) {
		return trwfPutLenSpecI32NoLength(dptr,(rtrInt32)sval);
	} else {
		register rtrUInt64 ckval = (sval >= 0) ? (sval << 1) : ((-sval - 1) << 1); // shift once here instead of each if/else if
		if (ckval & RTR_ULL(0xFF00000000000000) ) {
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval);
			return 8;
		} else if (ckval & RTR_ULL(0x00FF000000000000) ) {
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval << 8);
			//*((rtrUInt64*)(dptr)) = __rtr_bswap64( (((rtrUInt64)sval) & RTR_ULL(0x00FFFFFFFFFFFFFF)) | 0xE000000000000000);
			return 7;
		} else if (ckval & RTR_ULL(0x0000FF0000000000) ) {
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval << 16);
			//*((rtrUInt64*)(dptr)) = __rtr_bswap64( (((rtrUInt64)sval << 8) & RTR_ULL(0x00FFFFFFFFFFFFFF)) | 0xC000000000000000);
			return 6;
		} 

		*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval << 24);
		//*((rtrUInt64*)(dptr)) = __rtr_bswap64( (((rtrUInt64)sval << 16) & 0x00FFFFFFFFFFFFFF) | 0xA000000000000000);
		return 5;
		
	}
}

RTR_C_ALWAYS_INLINE rtrInt32 trwfPutLenSpecU64NoLength(char *dptr, rtrUInt64 sval)
{
	if (sval <= RTR_ULL(0xFFFFFFFF)) {
		return trwfPutLenSpecU32NoLength(dptr,(rtrUInt32)sval);
	} else {
		if (sval & RTR_ULL(0xFF00000000000000) ) {
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval);
			return 8;
		} else if (sval & RTR_ULL(0x00FF000000000000) ) {
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval << 8);
			//*((rtrUInt64*)(dptr)) = __rtr_bswap64( (((rtrUInt64)sval) & RTR_ULL(0x00FFFFFFFFFFFFFF)) | 0xE000000000000000);
			return 7;
		} else if (sval & RTR_ULL(0x0000FF0000000000) ) {
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval << 16);
			//*((rtrUInt64*)(dptr)) = __rtr_bswap64( (((rtrUInt64)sval << 8) & RTR_ULL(0x00FFFFFFFFFFFFFF)) | 0xC000000000000000);
			return 6;
		} 

		*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval << 24);
		//*((rtrUInt64*)(dptr)) = __rtr_bswap64( (((rtrUInt64)sval << 16) & 0x00FFFFFFFFFFFFFF) | 0xA000000000000000);
		return 5;
		
	}
}



RTR_C_ALWAYS_INLINE rtrInt32 trwfPutLenSpecU32(char *dptr, rtrUInt32 sval)
{
	if (sval & 0xFF000000 ) {
		*((rtrUInt8*)dptr) = 0x80; // top three bits set to 100
		*((rtrUInt32*)(dptr+1)) = __rtr_bswap32( (rtrUInt32)sval);
		return 5;
	} else if (sval & 0x00FF0000) { 
		//*((rtrUInt8*)dptr) = 0x60; // top three bits set to 011
		/* Or the length with the value into the top byte, then swap all 4 */
		*((rtrUInt32*)(dptr)) = __rtr_bswap32( (rtrUInt32)sval | 0x60000000);
		return 4;
	} else if (sval & 0x0000FF00 ) {
		*((rtrUInt8*)dptr) = 0x40;  // top three bits set to 010
		*((rtrUInt16*)(dptr+1)) = __rtr_bswap16( (rtrUInt16)sval);
		return 3;
	} 
	
	/* Or length with the value into the top byte, then swap both */
	*((rtrUInt16*)dptr) = __rtr_bswap16( (rtrUInt16) sval | 0x2000);  // top three bits set to 001
	return 2;
	
}


RTR_C_ALWAYS_INLINE rtrInt32 trwfPutLenSpecU64(char *dptr, rtrUInt64 sval)
{
	if (sval <= RTR_ULL(0xFFFFFFFF)) {
		return trwfPutLenSpecU32(dptr,(rtrUInt32)sval);
	} else {
		if (sval & RTR_ULL(0xFF00000000000000) ) {
			*((rtrUInt16*)dptr) = (rtrUInt16)0x081F;  // should give us a first byte of 00011111 (primitive code bailout and second byte of 0x08 telling us 8 bytes follow 
			*((rtrUInt64*)(dptr+2)) = __rtr_bswap64( (rtrUInt64)sval);
			return 10;
		} else if (sval & RTR_ULL(0x00FF000000000000) ) {
			// set top three bits to 111 by Or'ing with sval and swapping
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( (rtrUInt64)sval | RTR_ULL(0xE000000000000000));
			return 8;
		} else if (sval & RTR_ULL(0x0000FF0000000000) ) {
			//*((rtrUInt8*)dptr) = 0xC0; // set top three bits to 110
			*((rtrUInt64*)(dptr)) = __rtr_bswap64( ((rtrUInt64)sval << 8) | RTR_ULL(0xC000000000000000)) ;
			return 7;
		} 
		//	*((rtrUInt8*)dptr) = 0xA0;  // set top three bits to 101
		*((rtrUInt64*)(dptr)) = __rtr_bswap64( ((rtrUInt64)sval << 16) | RTR_ULL(0xA000000000000000)) ;
		return 6;
		
	}
}

/* this function assumes that if this is a primitive code it was already 
   handled - a new function can be written to return primitive codes as well if needed */
/* Because the switch values are not consecutive will this still get optimized out by the compiler?
   Is it better to shift the length bytes and just use 1, 2, 3, ...? */
RTR_C_ALWAYS_INLINE rtrInt32 trwfGetLenSpecU64(rtrUInt64 *svalptr, char *sptr)
{
	
	register rtrUInt8 len = (*(rtrUInt8*)(sptr));
		
	switch (len >> 5)
	{
		case 1:
			*svalptr = (rtrUInt64)((*(rtrUInt8*)(sptr+1)));
			return 2;
		case 2:
			*svalptr = (rtrUInt64)__rtr_bswap16( *((rtrUInt16*)(sptr+1) ));
			return 3;
		case 3:
			// there are actually 4 bytes on wire.  mask out length and swap
			*svalptr = (rtrUInt64)__rtr_bswap32( (*(rtrUInt32*)(sptr)) & 0xFFFFFF00) ;
			return 4;
		case 4:
			*svalptr = (rtrUInt64)__rtr_bswap32( *((rtrUInt32*)(sptr+1)) );
			return 5;
		case 5:
//#ifdef WIN32
//			return TRWF_GET_LENSPEC_U64_SIZE(svalptr,(sptr+1),len);
//#else 
			// only 6 bytes on wire including length.  swap 5 byte value and shift over three bytes before swap
			*svalptr = (rtrUInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+1)) << 24) );
			return 6;
//#endif
		case 6:
//#ifdef WIN32
//			return TRWF_GET_LENSPEC_U64_SIZE(svalptr, (sptr+1),len);
//#else
			// only 7 bytes on wire - causes problem on windows
			// byteswap 8 byte value (2 bytes arent ours), but we need to shift over 16 before we swap
			*svalptr = (rtrUInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+1)) << 16 ) );
			return 7;
//#endif
		case 7:
			// there are a full 8 bytes on wire, byte swap with length, but mask it off
			*svalptr = (rtrUInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) & RTR_LL(0xFFFFFFFFFFFFFF00) );
			return 8;
	}
	
	/* handle 8 byte escape */
	if (len == 31)
	{
		// full 8 bytes on wire for value 
		// first byte is 31, second byte should be 8 but is unnecessary
		*svalptr = (rtrUInt64)__rtr_bswap64( *((rtrUInt64*)(sptr+2)) );
		return 10;
	}

	return -1;
}


RTR_C_ALWAYS_INLINE rtrInt32 trwfGetLenSpecU64_Size(rtrUInt64 *svalptr, char *sptr, rtrUInt8 size)
{
	
	switch (size >> 5)
	{
		case 1:
			*svalptr = (rtrUInt64)((*(rtrUInt8*)(sptr)));
			return 1;
		case 2:
			*svalptr = (rtrUInt64)__rtr_bswap16( *((rtrUInt16*)(sptr) ));
			return 2;
		case 3:
			// shift over 8 since one byte isnt ours
			*svalptr = (rtrUInt64)__rtr_bswap32( (*(rtrUInt32*)(sptr)) << 8) ;
			return 3;
		case 4:
			*svalptr = (rtrUInt64)__rtr_bswap32( *((rtrUInt32*)(sptr)) );
			return 4;
		case 5:
//#ifdef WIN32
//			return TRWF_GET_LENSPEC_U64_SIZE(svalptr,(sptr+1),len);
//#else 
			// only 5 bytes on wire including length.  swap 5 byte value and shift over three bytes before swap
			*svalptr = (rtrUInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr)) << 24) );
			return 5;
//#endif
		case 6:
//#ifdef WIN32
//			return TRWF_GET_LENSPEC_U64_SIZE(svalptr, (sptr+1),len);
//#else
			// only 6 bytes on wire - causes problem on windows
			// byteswap 8 byte value (2 bytes arent ours), but we need to shift over 16 before we swap
			*svalptr = (rtrUInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr)) << 16 ) );
			return 6;
//#endif
		case 7:
			// there are 7 bytes on wire, 
			*svalptr = (rtrUInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) << 8 );
			return 7;
	}
	
	/* handle 8 byte escape */
	if (size == 31)
	{
		// full 8 bytes on wire for value 
		// first byte is 31, second byte should be 8 but is unnecessary
		*svalptr = (rtrUInt64)__rtr_bswap64( *((rtrUInt64*)(sptr+1)) );
		return 9;
	}

	return -1;
}





/* Missing PutResbitReal32, PutResBitReal64, GetResBitReal32, GetResBitReal64 */


RTR_C_ALWAYS_INLINE rtrInt32 rwfPutLenSpecReal32(char *dptr, rtrInt32 sval, rtrUInt8 fval)
{
	register rtrUInt32 ckval = (sval >= 0) ? (sval << 1) : (((-sval) - 1) << 1);  // shift here instead of each if/else if statement
	if (ckval & 0xFF000000 ) {
		*((rtrUInt16*)dptr) = ((rtrUInt16)fval << 8 | 0x05);
		*((rtrUInt32*)(dptr+2)) = __rtr_bswap32( (rtrUInt32)sval);
		return 6;
	} else if (ckval & 0x00FF0000 ) {
		*((rtrUInt16*)dptr) = ((rtrUInt16)fval << 8 | 0x04);
		*((rtrUInt32*)(dptr+2)) = __rtr_bswap32( (rtrUInt32)sval << 8);
		return 5;
	} else if (ckval & 0x0000FF00 ) {
		*((rtrUInt16*)dptr) = ((rtrUInt16)fval << 8 | 0x03);
		*((rtrUInt16*)(dptr+2)) = __rtr_bswap16( (rtrUInt16)sval);
		return 4;
	} 
	
	*((rtrUInt16*)dptr) = ((rtrUInt16)fval << 8 | 0x02);
	*((rtrUInt8*)(dptr+2)) = (rtrUInt8)sval;
	return 3;
	
}



RTR_C_ALWAYS_INLINE rtrInt32 rwfPutReal32(char *dptr, rtrInt32 sval, rtrUInt8 fval)
{
	register rtrUInt32 ckval = (sval >= 0) ? (sval << 1) : (((-sval) - 1) << 1);  // shift here instead of each if/else if statement
	if (ckval & 0xFF000000 ) {
		*((rtrUInt8*)dptr) = fval;
		*((rtrUInt32*)(dptr+1)) = __rtr_bswap32( (rtrUInt32)sval);
		return 5;
	} else if (ckval & 0x00FF0000 ) {
		*((rtrUInt8*)dptr) = fval;
		*((rtrUInt32*)(dptr+1)) = __rtr_bswap32( (rtrUInt32)sval << 8);
		return 4;
	} else if (ckval & 0x0000FF00 ) {
		*((rtrUInt8*)dptr) = fval;
		*((rtrUInt16*)(dptr+1)) = __rtr_bswap16( (rtrUInt16)sval);
		return 3;
	} 

	*((rtrUInt16*)dptr) = (((rtrUInt16) sval << 8) | fval);
	return 2;
	
}

/* A problem was uncovered with this function on Windows where
   we are reading into memory that is not ours.  This is due to
   the cast and shift of the char* sptr into the 4 byte temp 
   (this may take bytes that are not ours in the 2 and 3 byte case */
RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecReal32(rtrInt32 *svalptr, rtrUInt8 *fvalptr, char *sptr)
{
	register rtrUInt8 len = (rtrUInt8)(*(rtrUInt8*)(sptr));
	*fvalptr = (rtrUInt8)(*(rtrUInt8*)(sptr+1));
	
	switch (len)
	{
		case 1:
			*fvalptr = 0x20;
			return 2;
		case 2:
			*svalptr = (rtrInt32)(*(rtrUInt8*)(sptr+2));
			if (*svalptr & 0x80)
				*svalptr |= 0xFFFFFF00;
			return 3;
		case 3:
			*svalptr = (rtrInt32)__rtr_bswap16( *(rtrUInt16*)(sptr+2) );
			if (*svalptr & 0x8000)
				*svalptr |= 0xFFFF0000;
			return 4;
		case 4:
			*svalptr = (rtrInt32)__rtr_bswap32( *(rtrUInt32*)(sptr+1) & 0xFFFFFF00 );
			if (*svalptr & 0x800000)
				*svalptr |= 0xFF000000;
			return 5;
		case 5:
			*svalptr = (rtrInt32)__rtr_bswap32( *((rtrUInt32*)(sptr+2)) );
			return 6;

	}

	return -1;
}

/* A problem was uncovered with this function on Windows where
   we are reading into memory that is not ours.  This is due to
   the cast and shift of the char* sptr into the 4 byte temp 
   (this may take bytes that are not ours in the 2 and 3 byte case */
RTR_C_ALWAYS_INLINE rtrInt32 rwfGetReal32(rtrInt32 *svalptr, rtrUInt8 *fvalptr, rtrUInt16 length, char *sptr)
{
	*fvalptr = (rtrUInt8)(*(rtrUInt8*)(sptr));

	switch (length)
	{
		case 1:
			*fvalptr = 0x20;
			return 1;
		case 2:
			*svalptr = (rtrInt32)(*(rtrUInt8*)(sptr+1));
			if (*svalptr & 0x80)
				*svalptr |= 0xFFFFFF00;
			return 2;
		case 3:
			*svalptr = (rtrInt32)__rtr_bswap16( *((rtrUInt16*)(sptr+1)) );
			if (*svalptr & 0x8000)
				*svalptr |= 0xFFFF0000;
			return 3;
		case 4:
			*svalptr = (rtrInt32)__rtr_bswap32( (*(rtrUInt32*)(sptr)) & 0xFFFFFF00 );
			if (*svalptr & 0x800000)
				*svalptr |= 0xFF000000;
			return 4;
		case 5:
			*svalptr = (rtrInt32)__rtr_bswap32( *((rtrUInt32*)(sptr+1)) );
			return 5;
	}
	return -1;
}

RTR_C_ALWAYS_INLINE rtrInt32 rwfPutLenSpecReal64(char *dptr, rtrInt64 sval, rtrUInt8 fval)
{
	if ((sval >= RTR_LL(-2147483648)) && (sval <= RTR_LL(2147483647))) {
		return rwfPutLenSpecReal32(dptr,(rtrInt32)sval,fval);
	} else {
		register rtrUInt64 ckval = (sval >= 0) ? (sval << 1) : (((-sval) - 1) << 1);  // shift once here instead of each if/else if
		if (ckval & RTR_ULL(0xFF00000000000000) ) {
			*((rtrUInt16*)dptr) = ((rtrUInt16)fval << 8 | 0x09);
			*((rtrUInt64*)(dptr+2)) = __rtr_bswap64( (rtrUInt64)sval);
			return 10;
		} else if (ckval & RTR_ULL(0x00FF000000000000) ) {
			*((rtrUInt16*)dptr) = ((rtrUInt16)fval << 8 | 0x08);
			*((rtrUInt64*)(dptr+2)) = __rtr_bswap64( (rtrUInt64)sval << 8);
			return 9;
		} else if (ckval & RTR_ULL(0x0000FF0000000000) ) {
			*((rtrUInt16*)dptr) = ((rtrUInt16)fval << 8 | 0x07);
			*((rtrUInt64*)(dptr+2)) = __rtr_bswap64( (rtrUInt64)sval << 16);
			return 8;
		} 

		*((rtrUInt16*)dptr) = ((rtrUInt16)fval << 8 | 0x06);
		*((rtrUInt64*)(dptr+2)) = __rtr_bswap64( (rtrUInt64)sval << 24);
		return 7;
		
	}
}


RTR_C_ALWAYS_INLINE rtrInt32 rwfPutReal64(char *dptr, rtrInt64 sval, rtrUInt8 fval)
{
	if ((sval >= RTR_LL(-2147483648)) && (sval <= RTR_LL(2147483647))) {
		return rwfPutReal32(dptr,(rtrInt32)sval,fval);
	} else {
		register rtrUInt64 ckval = (sval >= 0) ? (sval << 1) : (((-sval) - 1) << 1); // shift once here instead of each if/else if
		if (ckval & RTR_ULL(0xFF00000000000000) ) {
			*((rtrUInt8*)dptr) = fval;
			*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval);
			return 9;
		} else if (ckval & RTR_ULL(0x00FF000000000000) ) {
			*((rtrUInt8*)dptr) = fval;
			*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval << 8);
			return 8;
		} else if (ckval & RTR_ULL(0x0000FF0000000000) ) {
			*((rtrUInt8*)dptr) = fval;
			*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval << 16);
			return 7;
		} 

		*((rtrUInt8*)dptr) = fval;
		*((rtrUInt64*)(dptr+1)) = __rtr_bswap64( (rtrUInt64)sval << 24);
		return 6;
	}
}

/* A problem was uncovered with this function on Windows where
   we are reading into memory that is not ours.  This is due to
   the cast and shift of the char* sptr into the 4 byte temp 
   (this may take bytes that are not ours in the 2 and 3 byte case */
RTR_C_ALWAYS_INLINE rtrInt32 rwfGetLenSpecReal64(rtrInt64 *svalptr, rtrUInt8 *fvalptr, char *sptr)
{
	register rtrUInt8 len = (*(rtrUInt8*)(sptr));
	*fvalptr = (rtrUInt8)(*(rtrUInt8*)(sptr+1));
	
	switch (len)
	{
		case 1:
			*fvalptr = 0x20;
			*svalptr = 0;
			return 2;
		case 2:
			*svalptr = (rtrInt64)((*(rtrUInt8*)(sptr+2)));
			if (*svalptr & 0x80)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFFFF00);
			return 3;
		case 3:
			*svalptr = (rtrInt64)__rtr_bswap16( (*(rtrUInt16*)(sptr+2)) );
			if (*svalptr & 0x8000)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFF0000);
			return 4;
		case 4:
			*svalptr = (rtrInt64)__rtr_bswap32( *((rtrUInt32*)(sptr+1)) & 0xFFFFFF00 );
			if (*svalptr & 0x800000)
				*svalptr |= RTR_LL(0xFFFFFFFFFF000000);
			return 5;
		case 5:
			*svalptr = (rtrInt64)__rtr_bswap32( *((rtrUInt32*)(sptr+2)) );
			if (*svalptr & 0x80000000)
				*svalptr |= RTR_LL(0xFFFFFFFF00000000);
			return 6;
		case 6:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_NUMERIC64_SIZE(svalptr,fvalptr,(sptr+1),len);
//#else 
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+2)) << 24) );
			if (*svalptr & RTR_LL(0x8000000000))
				*svalptr |= RTR_LL(0xFFFFFF0000000000);
			return 7;
//#endif
		case 7:
			*svalptr = (rtrInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) & RTR_LL(0xFFFFFFFFFFFF0000) );
			if (*svalptr & RTR_LL(0x800000000000))
				*svalptr |= RTR_LL(0xFFFF000000000000);
			return 8;
		case 8:
			*svalptr = (rtrInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr+1)) & RTR_LL(0xFFFFFFFFFFFFFF00) );
			if (*svalptr & RTR_LL(0x80000000000000))
				*svalptr |= RTR_LL(0xFF00000000000000);
			return 9;
		case 9:
			*svalptr = (rtrInt64)__rtr_bswap64( *((rtrUInt64*)(sptr+2)) );
			return 10;

	}
	return -1;
}

/* A problem was uncovered with this function on Windows where
   we are reading into memory that is not ours.  This is due to
   the cast and shift of the char* sptr into the 4 byte temp 
   (this may take bytes that are not ours in the 2 and 3 byte case */
RTR_C_ALWAYS_INLINE rtrInt32 rwfGetReal64(rtrInt64 *svalptr, rtrUInt8 *fvalptr, rtrUInt16 length, char *sptr)
{
	*fvalptr = (rtrUInt8)((*(rtrUInt8*)(sptr)));

	switch (length)
	{
		case 1:
			*fvalptr = 0x20;
			*svalptr = 0;
			return 1;
		case 2:
			*svalptr = (rtrInt64)((*(rtrUInt8*)(sptr+1)));
			if (*svalptr & 0x80)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFFFF00);
			return 2;
		case 3:
			*svalptr = (rtrInt64)__rtr_bswap16( (*(rtrUInt16*)(sptr+1)) );
			if (*svalptr & 0x8000)
				*svalptr |= RTR_LL(0xFFFFFFFFFFFF0000);
			return 3;
		case 4:
			
			*svalptr = (rtrInt64)__rtr_bswap32( (*(rtrUInt32*)(sptr)) & 0xFFFFFF00);
			if (*svalptr & 0x800000)
				*svalptr |= RTR_LL(0xFFFFFFFFFF000000);
			return 4;
		case 5:
			*svalptr = (rtrInt64)__rtr_bswap32( *((rtrUInt32*)(sptr+1)) );
			if (*svalptr & 0x80000000)
				*svalptr |= RTR_LL(0xFFFFFFFF00000000);
			return 5;
		case 6:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_NUMERIC64_SIZE(svalptr,fvalptr,(sptr),length);
//#else
			*svalptr = (rtrInt64)__rtr_bswap64( ((*(rtrUInt64*)(sptr+1)) << 24) );
			if (*svalptr & RTR_LL(0x8000000000))
				*svalptr |= RTR_LL(0xFFFFFF0000000000);
			return 6;
//#endif
		case 7:
//#ifdef WIN32
//			return RWF_GET_LENSPEC_NUMERIC64_SIZE(svalptr,fvalptr,(sptr),length);
//#else
			*svalptr = (rtrInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr+1)) << 16 );
			if (*svalptr & RTR_LL(0x800000000000))
				*svalptr |= RTR_LL(0xFFFF000000000000);
			return 7;
//#endif
		case 8:
			*svalptr = (rtrInt64)__rtr_bswap64( (*(rtrUInt64*)(sptr)) & RTR_LL(0xFFFFFFFFFFFFFF00) );
			if (*svalptr & RTR_LL(0x80000000000000))
				*svalptr |= RTR_LL(0xFF00000000000000);
			return 8;
		case 9:
			*svalptr = (rtrInt64)__rtr_bswap64( *((rtrUInt64*)(sptr+1)) );
			return 9;

	}
	return -1;

}


/* need to get this implemented for performance */
#define rwfPutResBitI15(___dptr,___sval) RWF_PUT_RESBIT_I15(___dptr,&___sval)
#define rwfGetResBitI15(___dvalptr,___sptr) RWF_GET_RESBIT_I15(___dvalptr,___sptr)

#else



#define rwfPut8(___dptr,___sval) RTR_PUT_8(___dptr,___sval)
#define rwfGet8(___dval,___sptr) RTR_GET_8(___dval,___sptr)
#define rwfPut16(___dptr,___sval) RTR_PUT_16(___dptr,___sval)
#define rwfGet16(___dval,___sptr) RTR_GET_16(___dval,___sptr)
#define rwfPut24(___dptr,___sval) RTR_PUT_24(___dptr,___sval)
#define rwfGet24(___dval,___sptr) RTR_GET_24(___dval,___sptr)
#define rwfPut32(___dptr,___sval) RTR_PUT_32(___dptr,___sval)
#define rwfGet32(___dval,___sptr) RTR_GET_32(___dval,___sptr)
#define rwfPut64(___dptr,___sval) RTR_PUT_64(___dptr,___sval)
#define rwfGet64(___dval,___sptr) RTR_GET_64(___dval,___sptr)
#define rwfPutFloat(___dptr,___sval) RTR_PUT_FLOAT(___dptr,___sval)
#define rwfGetFloat(___dval,___sptr) RTR_GET_FLOAT(___dval,___sptr)
#define rwfPutDouble(___dptr,___sval) RTR_PUT_DOUBLE(___dptr,___sval)
#define rwfGetDouble(___dval,___sptr) RTR_GET_DOUBLE(___dval,___sptr)

#define rwfPutLenSpecBlank(___dptr) RWF_PUT_LENSPEC_BLANK(___dptr)
#define rwfPutLenSpecU16(___dptr,___sval) RWF_PUT_LENSPEC_U16(___dptr,&___sval)
#define rwfGetLenSpecU16(___dvalptr,___sptr) RWF_GET_LENSPEC_U16(___dvalptr,___sptr)
#define rwfGetLenSpecU16_Size(___dvalptr,___sptr,___size) \
			RWF_GET_LENSPEC_U16_SIZE(___dvalptr,___sptr,___size)

#define rwfPutOptByteU16(___dptr,___sval) RWF_PUT_OPTBYTE_U16(___dptr,&___sval)
#define rwfGetOptByteU16(___dvalptr,___sptr) RWF_GET_OPTBYTE_U16(___dvalptr,___sptr)

#define rwfPutResBitU15(___dptr,___sval) RWF_PUT_RESBIT_U15(___dptr,&___sval)
#define rwfGetResBitU15(___dvalptr,___sptr) RWF_GET_RESBIT_U15(___dvalptr,___sptr)

/* ResBitU22 -- Used by TRWF */
#define rwfPutResBitU22(___dptr,___sptr) RWF_PUT_RESBIT_U22(___dptr,___sptr)
#define rwfGetResBitU22(___dvalptr,___sptr) RWF_GET_RESBIT_U22(___dvalptr,___sptr)

/* ResBitU30 -- used by TRWF and RWF */
#define rwfPutResBitU30(___dptr,___sptr) RWF_PUT_RESBIT_U30(___dptr,___sptr)
#define rwfGetResBitU30(___dvalptr,___sptr) RWF_GET_RESBIT_U30(___dvalptr,___sptr)

/* ResBitU31 -- Used by TRWF */
#define rwfPutResBitU31(___dptr,___sptr) RWF_PUT_RESBIT_U31(___dptr,___sptr)
#define rwfGetResBitU31(___dvalptr,___sptr) RWF_GET_RESBIT_U31(___dvalptr,___sptr)

/* ResBitI30 -- Implementation for non-x86 will be added as time permits. They are defined so the trwf apps can still build. */
#define rwfPutResBitI30(___dptr,___sptr) RWF_PUT_RESBIT_I30(___dptr,___sptr)
#define rwfGetResBitI30(___dptr,___sptr) RWF_GET_RESBIT_I30(___dptr,___sptr)
#define rwfReplaceResBitI30(___dptr,___sptr) RWF_REPLACE_RESBIT_I30(___dptr,___sptr)

#define rwfPutResBitI15(___dptr,___sval) RWF_PUT_RESBIT_I15(___dptr,&___sval)
#define rwfGetResBitI15(___dvalptr,___sptr) RWF_GET_RESBIT_I15(___dvalptr,___sptr)


#ifdef RTR_LITTLE_ENDIAN
#define __rwfGetU16As32(___dvalptr,___svalptr) \
	(((char*)(___dvalptr))[3] = 0, \
	 ((char*)(___dvalptr))[2] = 0, \
	 ((char*)(___dvalptr))[1] = ((char*)(___svalptr))[0], \
	 ((char*)(___dvalptr))[0] = ((char*)(___svalptr))[1] )
#else
#define __rwfGetU16As32(___dvalptr,___svalptr) \
	(((char*)(___dvalptr))[0] = 0, \
	 ((char*)(___dvalptr))[1] = 0, \
	 ((char*)(___dvalptr))[2] = ((char*)(___svalptr))[0], \
	 ((char*)(___dvalptr))[3] = ((char*)(___svalptr))[1] )
#endif


#define rwfGetBuffer8(___bufptr,___sptr) \
	( (___bufptr)->length = (rtrUInt8)___sptr[0], \
		(((___bufptr)->length == 0) ? \
			((___bufptr)->data = 0, 1 )  \
			: \
		   ((___bufptr)->data = ___sptr+1, \
			(1 + (___bufptr)->length)) \
	     ) \
	)	
		

	/* This function does not perform any buffer size checking.
	 * Can use the _rsslValidBufferPtr(), _rsslInvalidBufferPointer(),
	 * _rsslValidBufferPointerEndPtr() or _rsslInvalidBufferPointerEndPtr()
	 * after to check since no data copy is happening.
	 */
#define rwfGetBuffer16(___bufptr,___sptr) \
	( \
		((rtrUInt8)___sptr[0] < (rtrUInt8)0xFE) ? \
			  ( (___bufptr)->length = (rtrUInt8)___sptr[0], \
			    (___bufptr)->data = (___sptr + 1), \
			  	(1 + (rtrUInt8)___sptr[0]) ) \
			  : \
	  		    (__rwfGetU16As32(&((___bufptr)->length), (___sptr + 1)), \
			    (___bufptr)->data = ___sptr+3, \
			    (3 + (___bufptr)->length) )\
	)

#define rwfPutLenSpecI32(___dptr, ___sval) \
		RWF_PUT_LENSPEC_I32(___dptr, &(___sval))
#define rwfPutLenSpecU32(___dptr, ___sval) \
		RWF_PUT_LENSPEC_U32(___dptr, &(___sval))
#define rwfPutLenSpecI64(___dptr, ___sval) \
		RWF_PUT_LENSPEC_I64(___dptr, &(___sval))
#define rwfPutLenSpecU64(___dptr, ___sval) \
		RWF_PUT_LENSPEC_U64(___dptr, &(___sval))
#define rwfGetLenSpecI64Size(___svalptr, ___sptr, ___size) \
		RWF_GET_LENSPEC_I64_SIZE(___svalptr, (___sptr), ___size)
#define rwfGetLenSpecU64Size(___svalptr, ___sptr, ___size) \
		RWF_GET_LENSPEC_U64_SIZE(___svalptr, (___sptr), ___size)
#define rwfGetLenSpecI64(___dptr, ___sptr) \
		RWF_GET_LENSPEC_I64(___dptr, (___sptr))
#define rwfGetLenSpecU64(___svalptr, ___sptr) \
		RWF_GET_LENSPEC_U64(___svalptr, (___sptr))

/* TRWF does not directly call any 32 bit encode functions, so we do not need to define those here */
#define trwfPutLenSpecI64(___dptr, ___sval) \
		TRWF_PUT_LENSPEC_I64(___dptr, &(___sval))
#define trwfPutLenSpecU64(___dptr, ___sval) \
		TRWF_PUT_LENSPEC_U64(___dptr, &(___sval))
#define trwfPutLenSpecI64NoLength(___dptr,___sval) \
		TRWF_PUT_LENSPEC_I64_NO_LENGTH(___dptr,&(___sval))
#define trwfPutLenSpecU64NoLength(___dptr,___sval) \
		RWF_PUT_LENSPEC_U64_NO_LENGTH(___dptr,&(___sval))

#define trwfGetLenSpecI64(___dptr, ___sval) \
		TRWF_GET_LENSPEC_I64(___dptr, &(___sval))
#define trwfGetLenSpecU64(___dptr, ___sval) \
		TRWF_GET_LENSPEC_U64(___dptr, &(___sval))
#define trwfGetLenSpecI64_Size(___dptr, ___sval, ___lval) \
		TRWF_GET_LENSPEC_I64_SIZE(___dptr, &(___sval), ___lval)
#define trwfGetLenSpecU64_Size(___dptr, ___sval, ___lval) \
		TRWF_GET_LENSPEC_U64_SIZE(___dptr, &(___sval), ___lval)


#define rwfPutLenSpecReal32(___dptr,___sval,___fval) \
			RWF_PUT_LENSPEC_NUMERIC32(___dptr,&(___fval),&(___sval))
#define rwfGetLenSpecReal32(___svalptr,___fvalptr,___sptr) \
			RWF_GET_LENSPEC_NUMERIC32(___svalptr,___fvalptr,___sptr)
#define rwfGetReal32(___svalptr,___fvalptr,___lptr,___ivalptr) \
			RWF_GET_LENSPEC_NUMERIC32_SIZE(___svalptr,___fvalptr,___ivalptr,___lptr)
#define rwfPutReal32(___dptr,___sval,___fval) \
			RWF_PUT_RESBIT_NUMERIC32(___dptr,&(___fval),&(___sval))
#define rwfPutResBitReal32(___dptr,___sval,___fval) \
			RWF_PUT_RESBIT_NUMERIC32(___dptr,&(___fval),&(___sval))
#define rwfGetResBitReal32(___svalptr,___fvalptr,___sptr) \
			RWF_GET_RESBIT_NUMERIC32(___svalptr,___fvalptr,___sptr)

#define rwfPutLenSpecReal64(___dptr,___sval,___fval) \
			RWF_PUT_LENSPEC_NUMERIC64(___dptr,&(___fval),&(___sval))
#define rwfGetLenSpecReal64(___svalptr,___fvalptr,___sptr) \
			RWF_GET_LENSPEC_NUMERIC64(___svalptr,___fvalptr,___sptr)
#define rwfGetReal64(___svalptr,___fvalptr,___lptr,___ivalptr) \
			RWF_GET_LENSPEC_NUMERIC64_SIZE(___svalptr,___fvalptr,___ivalptr,___lptr)
#define rwfPutReal64(___dptr,___sval,___fval) \
			RWF_PUT_RESBIT_NUMERIC64(___dptr,&(___fval),&(___sval))
#define rwfPutResBitReal64(___dptr,___sval,___fval) \
			RWF_PUT_RESBIT_NUMERIC64(___dptr,&(___fval),&(___sval))
#define rwfGetResBitReal64(___svalptr,___fvalptr,___sptr) \
			RWF_GET_RESBIT_NUMERIC64(___svalptr,___fvalptr,___sptr)

#endif


#endif

