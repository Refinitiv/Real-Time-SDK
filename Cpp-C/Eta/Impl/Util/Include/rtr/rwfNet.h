/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_RWF_NET_H
#define __RTR_RWF_NET_H

#include "rtr/os.h"


#define RWF_MAX_8	0xFF
#define RWF_MAX_16	0xFFFF
#define RWF_MAX_32	0xFFFFFFFF
#define RWF_MAX_64	0xFFFFFFFFFFFFFFFF
#define RWF_MAX_U15	0x7FFF
#define RWF_MAX_U22 0x3FFFFF
#define RWF_MAX_U30	0x3FFFFFFF
#define RWF_MAX_U31 0x7FFFFFFF
#define RWF_MAX_I30 ((rtrInt32)0x3FFFFFFF)
#define RWF_MIN_I30 ((rtrInt32)0xC0000000)



/* Note: 
	   The case where the length specification is set to 0 signifies a blank.  The application
	   can look at the returned size and see that we read 1 byte (the length) and infer that is it 
	   a blank entry.  This only works for length specified types. 
*/


#define RWF_MOVE_8(d,s) \
	(((char*)(d))[0] = ((char*)(s))[0], 1)

#ifdef RTR_LITTLE_ENDIAN
	/* The following are for LE machines (i.e. low-byte to high-byte ordering) */

	#define RWF_MOVE_16(d,s) \
		(((char*)(d))[0] = ((char*)(s))[1],\
		 ((char*)(d))[1] = ((char*)(s))[0], 2)

	#define RWF_MOVE_16_NO_SWAP(d,s) \
		(((char*)(d))[0] = ((char*)(s))[1],\
		 ((char*)(d))[1] = ((char*)(s))[0], 2)

	#define RWF_MOVE_32(d,s) \
		(((char*)(d))[0] = ((char*)(s))[3],\
		 ((char*)(d))[1] = ((char*)(s))[2],\
		 ((char*)(d))[2] = ((char*)(s))[1],\
		 ((char*)(d))[3] = ((char*)(s))[0], 4 )

	#define RWF_MOVE_64(d,s)\
		(((char*)(d))[0] = ((char*)(s))[7],\
		 ((char*)(d))[1] = ((char*)(s))[6],\
		 ((char*)(d))[2] = ((char*)(s))[5],\
		 ((char*)(d))[3] = ((char*)(s))[4],\
		 ((char*)(d))[4] = ((char*)(s))[3],\
		 ((char*)(d))[5] = ((char*)(s))[2],\
		 ((char*)(d))[6] = ((char*)(s))[1],\
		 ((char*)(d))[7] = ((char*)(s))[0], 8 )

	#define RWF_MOVE_FLOAT(d,s)\
		(((char*)(d))[0] = ((char*)(s))[3],\
		 ((char*)(d))[1] = ((char*)(s))[2],\
		 ((char*)(d))[2] = ((char*)(s))[1],\
		 ((char*)(d))[3] = ((char*)(s))[0], 4 )

	#define RWF_MOVE_DOUBLE(d,s)\
		(((char*)(d))[0] = ((char*)(s))[7],\
		 ((char*)(d))[1] = ((char*)(s))[6],\
		 ((char*)(d))[2] = ((char*)(s))[5],\
		 ((char*)(d))[3] = ((char*)(s))[4],\
		 ((char*)(d))[4] = ((char*)(s))[3],\
		 ((char*)(d))[5] = ((char*)(s))[2],\
		 ((char*)(d))[6] = ((char*)(s))[1],\
		 ((char*)(d))[7] = ((char*)(s))[0], 8 )


	/* Length specified scalar values */
	
	/* sets the length specifier to 0 to signify blank value.  This
	   should work for any length specified type */
	#define RWF_PUT_LENSPEC_BLANK(d) \
		(  ((char*)(d))[0] = 0, 1 )


	#define RWF_PUT_LENSPEC_U16(d,s)\
		( ( *((rtrUInt16*)s) <= (rtrUInt16)0xFF )  ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			) \
		)


	#define RWF_GET_LENSPEC_U16(d,s) \
		(( (unsigned char)((char*)(s))[0] == 0 )  ? \
			( ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : -1 \
		  ) \
		) \
       )

	#define RWF_GET_LENSPEC_U16_SIZE(d,s,size) \
		  (( size == 0 )  ? \
			( ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 0 \
			) : \
		  (( size == 1 )  ? \
			( ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 \
			) : \
		 ( ( size == 2 )  ? \
		 	( ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : -1 \
		  ) \
		 ) \
		)


#define RWF_PUT_LENSPEC_I16(d,s) \
		( ( *((rtrInt16*)s) > 0 )  ? \
		  ( ( *((rtrInt16*)s) < 128 ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 \
			) : \
		    ( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			)  \
		  ) : \
		 ( ( *((rtrInt16*)s) >= -128 ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 \
            ) : \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			) \
         ) \
	   )


	#define RWF_GET_LENSPEC_I16(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
		 	( ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 2 ) ? \
			( ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : -1 \
		   ) \
		  ) \
   	    )

	#define RWF_GET_LENSPEC_I16_SIZE(d,s,size) \
		  (( size == 0 )  ? \
		 	 ( ((char*)(d))[1] = 0, \
			   ((char*)(d))[0] = 0, 0 \
			 ) : \
		  (( size == 1 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[0], 1 \
			  ) : \
			  ( ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[0], 1 \
			  ) \
			) : \
		   (( size == 2 ) ? \
			( ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : -1 \
		   ) \
		  ) \
	    )



	#define RWF_PUT_LENSPEC_U32(d,s)\
		( ( *((rtrUInt32*)s) <= 0xFF )  ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		 ( ( *((rtrUInt32*)s) <= 0xFFFF ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : \
		  ( ( *((rtrUInt32*)s) <= 0xFFFFFF ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) \
		  ) \
		 ) \
		)

	#define RWF_GET_LENSPEC_U32(d,s) \
		(( (unsigned char)((char*)(s))[0] == 0 )  ? \
		    ( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		( ((unsigned char)((char*)(s))[0] == 1 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
		 	( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 4 ) ? \
			( ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[4], 5 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )

	#define RWF_GET_LENSPEC_U32_SIZE(d,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 0 \
			) : \
		 (( size == 1 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 \
			) : \
		 ( ( size == 2 )  ? \
		 	( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		  ( ( size == 3 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : \
		   (( size == 4 ) ? \
			( ((char*)(d))[3] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		 ) \
		)


#define RWF_PUT_LENSPEC_I32(d,s) \
		( ( *((rtrInt32*)s) > 0 )  ? \
		  ( ( *((rtrInt32*)s) < 128 ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		  ( ( *((rtrInt32*)s) < 32768 ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt32*)s) < 8388608 ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) \
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt32*)s) >= -128 ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		  ( ( *((rtrInt32*)s) >= -32768 ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt32*)s) >= -8388608 ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) \
		  ) \
		 ) \
		) \
	   )


	#define RWF_GET_LENSPEC_I32(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
		 	( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[1], \
			     ((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
			    ((char*)(d))[1] = ((char*)(s))[2], \
			    ((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 4 ) ? \
			( ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[4], 5 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )

	#define RWF_GET_LENSPEC_I32_SIZE(d,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 0 \
			) : \
         (( size == 1 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[0], 1 \
			  ) : \
			  ( ((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[0], 1 \
			  ) \
			) : \
		 (( size == 2 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[0], \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			   ) : \
		 	   ( ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[0], \
			     ((char*)(d))[0] = ((char*)(s))[1], 2 \
			   ) \
			) : \
		  (( size == 3 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[0], \
			    ((char*)(d))[1] = ((char*)(s))[1], \
			    ((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		   (( size == 4 ) ? \
			( ((char*)(d))[3] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )



	#define RWF_PUT_LENSPEC_U64(d,s)\
	   (  ( *((rtrUInt64*)s) <= RTR_ULL(0xFF) )  ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFF) )  ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		 ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFF)   ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		 ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFF)   ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6\
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) : \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[8] = ((char*)(s))[0], 9 \
			) \
		  ) \
		 ) \
		) \
	   ) \
	  ) \
     ) \
    ) 
      


	#define RWF_GET_LENSPEC_U64(d,s) \
		(( (unsigned char)((char*)(s))[0] == 0 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		( ( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
		 	( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 4 )  ? \
		 	( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[4], 5 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 5 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[0] = ((char*)(s))[5], 6 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 6 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[0] = ((char*)(s))[6], 7 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 7 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[0] = ((char*)(s))[7], 8 \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 8 ) ? \
			( ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[0] = ((char*)(s))[8], 9 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		)\
		)\
		)\
		) \
		)

/* used to write default header sid bitmap */
	#define RWF_PUT_LENSPEC_U64_SIZE(d,s,size)\
	   (  ( size == 1 )  ? \
			( ((char*)(d))[0] = ((char*)(s))[0], 1 ) : \
		( ( size == 2 )  ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		 ( ( size == 3   ) ? \
			( ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : \
		 ( ( size == 4 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		  ( ( size == 5   ) ? \
			( ((char*)(d))[0] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( size == 6 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( size == 7 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
			( ((char*)(d))[0] = ((char*)(s))[7], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) \
		  ) \
		 ) \
		) \
	   ) \
	  ) \
     ) \
    ) 


	#define RWF_GET_LENSPEC_U64_SIZE(d,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 0 \
			) : \
		 (( size == 1 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 \
			) : \
		(( size == 2 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		 ( ( size == 3 )  ? \
		 	( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : \
		 ( ( size == 4 )  ? \
		 	( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) : \
		 ( ( size == 5 )  ? \
		 	( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[4], 5 \
			) : \
		  ( ( size == 6 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = ((char*)(s))[0], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[0] = ((char*)(s))[5], 6 \
			) : \
			( ( size == 7 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = ((char*)(s))[0], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[0] = ((char*)(s))[6], 7 \
			) : \
		   (( size == 8 ) ? \
			( ((char*)(d))[7] = ((char*)(s))[0], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[0] = ((char*)(s))[7], 8 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
		)


/* THE FOLLOWING ARE RWF1.0 specific and should only be
   used with RWF 1.0 - for TRWF there are different 
   functions below that correspond to the bit layouts
   for trwf */

#define RWF_PUT_LENSPEC_I64(d,s) \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(128) ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(8388608) ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(549755813888) ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(36028797018963968) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) : \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[8] = ((char*)(s))[0], 9 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt64*)s) >= RTR_LL(-128) ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-8388608) ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-549755813888) ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-36028797018963968) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) : \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[8] = ((char*)(s))[0], 9 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		)  \
	   )


	#define RWF_GET_LENSPEC_I64(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(d))[7] = 0, \
				 ((char*)(d))[6] = 0, \
				 ((char*)(d))[5] = 0, \
				 ((char*)(d))[4] = 0, \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[1], \
			     ((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
			    ((char*)(d))[1] = ((char*)(s))[2], \
			    ((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 4 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[1] = ((char*)(s))[3], \
			    ((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 5 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[1] = ((char*)(s))[4], \
				((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
			    ((char*)(d))[1] = ((char*)(s))[4], \
			    ((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 6 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[1] = ((char*)(s))[5], \
				((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
			    ((char*)(d))[1] = ((char*)(s))[5], \
			    ((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 7 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
			    ((char*)(d))[2] = ((char*)(s))[5], \
				((char*)(d))[1] = ((char*)(s))[6], \
				((char*)(d))[0] = ((char*)(s))[7], 8 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
			    ((char*)(d))[2] = ((char*)(s))[5], \
			    ((char*)(d))[1] = ((char*)(s))[6], \
			    ((char*)(d))[0] = ((char*)(s))[7], 8 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 8 ) ? \
			( ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[0] = ((char*)(s))[8], 9 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
	   )

	#define RWF_GET_LENSPEC_I64_SIZE(d,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 0 \
			) : \
		 (( size == 1 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[0], 1 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[0], 1 \
			  ) \
			) : \
		 (( size == 2 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[0], \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			   ) : \
		 	   ( ((char*)(d))[7] = 0, \
				 ((char*)(d))[6] = 0, \
				 ((char*)(d))[5] = 0, \
				 ((char*)(d))[4] = 0, \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[0], \
			     ((char*)(d))[0] = ((char*)(s))[1], 2 \
			   ) \
			) : \
		  (( size == 3 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[0], \
			    ((char*)(d))[1] = ((char*)(s))[1], \
			    ((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		  (( size == 4 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[0], \
			    ((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = ((char*)(s))[0], \
			    ((char*)(d))[2] = ((char*)(s))[1], \
			    ((char*)(d))[1] = ((char*)(s))[2], \
			    ((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  (( size == 5 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[1] = ((char*)(s))[3], \
			    ((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  (( size == 6 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[5] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[1] = ((char*)(s))[4], \
				((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
			    ((char*)(d))[1] = ((char*)(s))[4], \
			    ((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  (( size == 7 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[0], \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[1] = ((char*)(s))[5], \
				((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = ((char*)(s))[0], \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
			    ((char*)(d))[1] = ((char*)(s))[5], \
			    ((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		   (( size == 8 ) ? \
			( ((char*)(d))[7] = ((char*)(s))[0], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[0] = ((char*)(s))[7], 8 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
	   )
/* End RWF 1.0 specific */


/* THE FOLLOWING ARE USED BY TRWF and are specifically written
   to work with TRWF bit and byte layouts - they should not
   be used with RWF 1.0 encoding or decoding */



	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 (31) 00001000 (8) = 8 byte length */
#define TRWF_PUT_LENSPEC_I64(d,s) \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(128) ) ? \
			( ((char*)(d))[0] = 32, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = 64, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(8388608) ) ? \
			( ((char*)(d))[0] = 96, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((unsigned char*)(d))[0] = 128, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(549755813888) ) ? \
			( ((unsigned char*)(d))[0] = 160, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((unsigned char*)(d))[0] = 192, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(36028797018963968) ) ? \
			( ((unsigned char*)(d))[0] = 224, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) : \
			( ((char*)(d))[0] = 31, \
			  ((char*)(d))[1] = 8, \
			  ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[2], \
			  ((char*)(d))[8] = ((char*)(s))[1], \
			  ((char*)(d))[9] = ((char*)(s))[0], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt64*)s) >= RTR_LL(-128) ) ? \
			( ((char*)(d))[0] = 32, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = 64, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-8388608) ) ? \
			( ((char*)(d))[0] = 96, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((char*)(d))[0] = 128, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-549755813888) ) ? \
			( ((char*)(d))[0] = 160, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((char*)(d))[0] = 192, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-36028797018963968) ) ? \
			( ((char*)(d))[0] = 224, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) : \
			( ((char*)(d))[0] = 31, \
			  ((char*)(d))[1] = 8, \
			  ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[2], \
			  ((char*)(d))[8] = ((char*)(s))[1], \
			  ((char*)(d))[9] = ((char*)(s))[0], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		)  \
	   )

	/* Almost the same as RWF_PUT_LENSPEC_U64, but uses TRWF sizes */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 (31) 00001000 (8) = 8 byte length */
	#define TRWF_PUT_LENSPEC_U64(d,s)\
	   (  ( *((rtrUInt64*)s) <= RTR_ULL(0xFF) )  ? \
			( ((char*)(d))[0] = 32, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFF) )  ? \
			( ((char*)(d))[0] = 64, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		 ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFF)   ) ? \
			( ((char*)(d))[0] = 96, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		 ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 128, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFF)   ) ? \
			( ((char*)(d))[0] = 160, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6\
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 192, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 224, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) : \
			( ((char*)(d))[0] = 31, \
			  ((char*)(d))[1] = 8, \
			  ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[2], \
			  ((char*)(d))[8] = ((char*)(s))[1], \
			  ((char*)(d))[9] = ((char*)(s))[0], 10 \
			) \
		  ) \
		 ) \
		) \
	   ) \
	  ) \
     ) \
    ) 
      

	/* Like RWF_GET_LENSPEC_U64, uses TRWF buffer lengths */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 (31) 00001000 (8) = 8 byte length */
	#define TRWF_GET_LENSPEC_U64(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 32 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		( ( (unsigned char)((char*)(s))[0] == 64 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 96 )  ? \
		 	( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 128 )  ? \
		 	( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[4], 5 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 160 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[0] = ((char*)(s))[5], 6 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 192 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[0] = ((char*)(s))[6], 7 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 224 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[0] = ((char*)(s))[7], 8 \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 31 ) ? \
			( ((char*)(d))[7] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[1] = ((char*)(s))[8], \
			  ((char*)(d))[0] = ((char*)(s))[9], 10 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		)\
		)\
		)\
		)

/* This function works like TRWF_PUT_LENSPEC_I64,
 * but puts the bytes on starting from the top byte, rather than the bottom byte
 *
 * This lets us save some bandwidth for floating point numbers --
 * Double-precision floats converted from single-precision floats
 * typically have all their meaningful bits on the top end, usually
 * leaving bytes of zero on the bottom end(as opposed to integers where the 0's are
 * on the high end).
 */

/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 (31) 00001000 (8) = 8 byte length */
#define TRWF_PUT_LENSPEC_I64_TOP(d,s) \
		  ( \
			( !( *((rtrUInt64*)s) & 0x00FFFFFFFFFFFFFFULL ) ? \
			( ((char*)(d))[0] = 32, \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x0000FFFFFFFFFFFFULL ) ? \
			( ((char*)(d))[0] = 64, \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], 3 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x000000FFFFFFFFFFULL ) ? \
			( ((char*)(d))[0] = 96, \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], 4 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x00000000FFFFFFFFULL ) ? \
			( ((unsigned char*)(d))[0] = 128, \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], 5 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x0000000000FFFFFFULL ) ? \
			( ((unsigned char*)(d))[0] = 160, \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], 6 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x000000000000FFFFULL ) ? \
			( ((unsigned char*)(d))[0] = 192, \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[2], 7 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x00000000000000FFULL ) ? \
			( ((unsigned char*)(d))[0] = 224, \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[1], 8 \
			) : \
			( ((char*)(d))[0] = 31, \
			  ((char*)(d))[1] = 8, \
			  ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[2], \
			  ((char*)(d))[8] = ((char*)(s))[1], \
			  ((char*)(d))[9] = ((char*)(s))[0], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
			) \
			)

/* This still finds the length boundaries and only puts on the 
   necessary number of bytes, however it does not put the length 
   on.  It is assumed this will be done outside this function */
#define TRWF_PUT_LENSPEC_I64_NO_LENGTH(d,s) \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(128) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0], 1 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(8388608) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(549755813888) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(36028797018963968) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
			( ((char*)(d))[0] = ((char*)(s))[7], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt64*)s) >= RTR_LL(-128) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0], 1 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-8388608) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-549755813888) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-36028797018963968) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
			( ((char*)(d))[0] = ((char*)(s))[7], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		)  \
	   )

	#define RWF_PUT_LENSPEC_U64_NO_LENGTH(d,s)\
	   (  ( *((rtrUInt64*)s) <= RTR_ULL(0xFF))  ? \
			( ((char*)(d))[0] = ((char*)(s))[0], 1 ) : \
		( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFF))  ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		 ( (*((rtrUInt64*)s) <= RTR_ULL(0xFFFFFF) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : \
		 ( (*((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFF) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFF)) ? \
			( ((char*)(d))[0] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFF)) ? \
			( ((char*)(d))[0] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFFFF)) ? \
			( ((char*)(d))[0] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
			( ((char*)(d))[0] = ((char*)(s))[7], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) \
		  ) \
		 ) \
		) \
	   ) \
	  ) \
     ) \
    ) 


	/* took out 0 case */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 00001000 (31 8) = 8 byte length */
	#define TRWF_GET_LENSPEC_I64(d,s) \
		 (( (unsigned char)((char*)(s))[0] == 32 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 64 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(d))[7] = 0, \
				 ((char*)(d))[6] = 0, \
				 ((char*)(d))[5] = 0, \
				 ((char*)(d))[4] = 0, \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[1], \
			     ((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 96 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
			    ((char*)(d))[1] = ((char*)(s))[2], \
			    ((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  ( ( (unsigned char)((unsigned char*)(s))[0] == 128 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[1] = ((char*)(s))[3], \
			    ((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  ( ( (unsigned char)((unsigned char*)(s))[0] == 160 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[1] = ((char*)(s))[4], \
				((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
			    ((char*)(d))[1] = ((char*)(s))[4], \
			    ((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  ( ( (unsigned char)((unsigned char*)(s))[0] == 192 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[1] = ((char*)(s))[5], \
				((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
			    ((char*)(d))[1] = ((char*)(s))[5], \
			    ((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		  ( ( (unsigned char)((unsigned char*)(s))[0] == 224 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
			    ((char*)(d))[2] = ((char*)(s))[5], \
				((char*)(d))[1] = ((char*)(s))[6], \
				((char*)(d))[0] = ((char*)(s))[7], 8 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
			    ((char*)(d))[2] = ((char*)(s))[5], \
			    ((char*)(d))[1] = ((char*)(s))[6], \
			    ((char*)(d))[0] = ((char*)(s))[7], 8 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 31 ) ? \
			( ((char*)(d))[7] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[1] = ((char*)(s))[8], \
			  ((char*)(d))[0] = ((char*)(s))[9], 10 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
	   )

/* see TRWF_PUT_LENSPEC_I64_TOP for information */
	/* took out 0 case */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 00001000 (31 8) = 8 byte length */
	#define TRWF_GET_LENSPEC_I64_TOP(d,s) \
		 ( \
			( ((unsigned char)((char*)(s))[0] == 32 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = 0 ,\
				((char*)(d))[5] = 0 ,\
				((char*)(d))[4] = 0 ,\
				((char*)(d))[3] = 0 ,\
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 2 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 64 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = 0 ,\
				((char*)(d))[4] = 0 ,\
				((char*)(d))[3] = 0 ,\
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 3 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 96 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = 0 ,\
				((char*)(d))[3] = 0 ,\
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 4 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 128 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = 0 ,\
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 5 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 160 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 6 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 192 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 7 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 224 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
				((char*)(d))[1] = ((char*)(s))[7], \
				((char*)(d))[0] = 0 , 8 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 31 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[3], \
				((char*)(d))[5] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
				((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], \
				((char*)(d))[1] = ((char*)(s))[8], \
				((char*)(d))[0] = ((char*)(s))[9], 10 \
			) : - 1\
			)))))))))

	/* see TRWF_PUT_LENSPEC_I64_TOP for information */
	/* took out 0 case */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 00001000 (31 8) = 8 byte length */
	#define TRWF_GET_LENSPEC_I64_TOP_SIZE(d,s,size) \
		  ((( (size) == 32 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = 0 ,\
				((char*)(d))[5] = 0 ,\
				((char*)(d))[4] = 0 ,\
				((char*)(d))[3] = 0 ,\
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 2 \
			) : \
			(( (size) == 64 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = 0 ,\
				((char*)(d))[4] = 0 ,\
				((char*)(d))[3] = 0 ,\
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 3 \
			) : \
			(( (size) == 96 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = 0 ,\
				((char*)(d))[3] = 0 ,\
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 4 \
			) : \
			(( (size) == 128 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = 0 ,\
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 5 \
			) : \
			(( (size) == 160 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = 0 ,\
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 6 \
			) : \
			(( (size) == 192 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
				((char*)(d))[1] = 0 ,\
				((char*)(d))[0] = 0 , 7 \
			) : \
			(( (size) == 224 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
				((char*)(d))[1] = ((char*)(s))[7], \
				((char*)(d))[0] = 0 , 8 \
			) : \
			(( (size) == 31 )  ? \
			  (((char*)(d))[7] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[3], \
				((char*)(d))[5] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
				((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], \
				((char*)(d))[1] = ((char*)(s))[8], \
				((char*)(d))[0] = ((char*)(s))[9], 10 \
			) : - 1\
			)))))))))


	/* took out 0 case */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 00001000 (31 8) = 8 byte length */
	#define TRWF_GET_LENSPEC_I64_SIZE(d,s,size) \
		 (( size == 32 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[0], 1 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[0], 1 \
			  ) \
			) : \
		 (( size == 64 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[0], \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			   ) : \
		 	   ( ((char*)(d))[7] = 0, \
				 ((char*)(d))[6] = 0, \
				 ((char*)(d))[5] = 0, \
				 ((char*)(d))[4] = 0, \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[0], \
			     ((char*)(d))[0] = ((char*)(s))[1], 2 \
			   ) \
			) : \
		  (( size == 96 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[0], \
			    ((char*)(d))[1] = ((char*)(s))[1], \
			    ((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		  (( size == 128 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[0], \
			    ((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = ((char*)(s))[0], \
			    ((char*)(d))[2] = ((char*)(s))[1], \
			    ((char*)(d))[1] = ((char*)(s))[2], \
			    ((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  (( size == 160 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[1] = ((char*)(s))[3], \
			    ((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  (( size == 192 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[5] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[1] = ((char*)(s))[4], \
				((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
			    ((char*)(d))[1] = ((char*)(s))[4], \
			    ((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  (( size == 224 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[7] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[0], \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[1] = ((char*)(s))[5], \
				((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = ((char*)(s))[0], \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
			    ((char*)(d))[1] = ((char*)(s))[5], \
			    ((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		   (( size == 31 ) ? \
			( ((char*)(d))[7] = ((char*)(s))[0], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[0] = ((char*)(s))[7], 8 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
	  )

/* Like RWF_GET_LENSPEC_U64_SIZE, but uses TRWF sizes */
#define TRWF_GET_LENSPEC_U64_SIZE(d,s,size) \
		 (( size == 32 )  ? \
			( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 \
			) : \
		 (( size == 64 )  ? \
			( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		  (( size == 96 )  ? \
			( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : \
		  (( size == 128 )  ? \
			( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) : \
		  (( size == 160 )  ? \
			( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[4], 5 \
			) : \
		  (( size == 192 )  ? \
			( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[0] = ((char*)(s))[5], 6 \
			) : \
		  (( size == 224 )  ? \
			( ((char*)(d))[7] = 0, \
				((char*)(d))[6] = ((char*)(s))[0], \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[0] = ((char*)(s))[6], 7 \
			) : \
		   (( size == 31 ) ? \
			( ((char*)(d))[7] = ((char*)(s))[0], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[0] = ((char*)(s))[7], 8 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
	  )

/* END of TRWF Specific codes */


	#define RWF_PUT_24(d,s) \
		(((char*)(d))[0] = ((char*)(s))[2],\
		 ((char*)(d))[1] = ((char*)(s))[1],\
		 ((char*)(d))[2] = ((char*)(s))[0], 3 )
		 

	#define RWF_GET_24(d,s) \
		( ((char*)(d))[3] = 0, \
		  ((char*)(d))[2] = ((char*)(s))[0], \
		  ((char*)(d))[1] = ((char*)(s))[1], \
		  ((char*)(d))[0] = ((char*)(s))[2], 3 )
		
	#define RWF_PUT_40(d,s) \
		(((char*)(d))[0] = ((char*)(s))[4],\
		 ((char*)(d))[1] = ((char*)(s))[3],\
		 ((char*)(d))[2] = ((char*)(s))[2],\
		 ((char*)(d))[3] = ((char*)(s))[1],\
		 ((char*)(d))[4] = ((char*)(s))[0], 5 )

	#define RWF_GET_40(d,s) \
		( ((char*)(d))[7] = 0, \
		  ((char*)(d))[6] = 0, \
		  ((char*)(d))[5] = 0, \
		  ((char*)(d))[4] = ((char*)(s))[0], \
		  ((char*)(d))[3] = ((char*)(s))[1], \
		  ((char*)(d))[2] = ((char*)(s))[2], \
		  ((char*)(d))[1] = ((char*)(s))[3], \
		  ((char*)(d))[0] = ((char*)(s))[4], 5 )

	#define RWF_PUT_48(d,s) \
		(((char*)(d))[0] = ((char*)(s))[5],\
		 ((char*)(d))[1] = ((char*)(s))[4],\
		 ((char*)(d))[2] = ((char*)(s))[3],\
		 ((char*)(d))[3] = ((char*)(s))[2],\
		 ((char*)(d))[4] = ((char*)(s))[1],\
		 ((char*)(d))[5] = ((char*)(s))[0], 6 )

	#define RWF_GET_48(d,s) \
		( ((char*)(d))[7] = 0, \
		  ((char*)(d))[6] = 0, \
		  ((char*)(d))[5] = ((char*)(s))[0], \
		  ((char*)(d))[4] = ((char*)(s))[1], \
		  ((char*)(d))[3] = ((char*)(s))[2], \
		  ((char*)(d))[2] = ((char*)(s))[3], \
		  ((char*)(d))[1] = ((char*)(s))[4], \
		  ((char*)(d))[0] = ((char*)(s))[5], 6 )


	#define RWF_PUT_56(d,s) \
		(((char*)(d))[0] = ((char*)(s))[6],\
		 ((char*)(d))[1] = ((char*)(s))[5],\
		 ((char*)(d))[2] = ((char*)(s))[4],\
		 ((char*)(d))[3] = ((char*)(s))[3],\
		 ((char*)(d))[4] = ((char*)(s))[2],\
		 ((char*)(d))[5] = ((char*)(s))[1],\
		 ((char*)(d))[6] = ((char*)(s))[0], 7 )

	#define RWF_GET_56(d,s) \
		( ((char*)(d))[7] = 0, \
		  ((char*)(d))[6] = ((char*)(s))[0], \
		  ((char*)(d))[5] = ((char*)(s))[1], \
		  ((char*)(d))[4] = ((char*)(s))[2], \
		  ((char*)(d))[3] = ((char*)(s))[3], \
		  ((char*)(d))[2] = ((char*)(s))[4], \
		  ((char*)(d))[1] = ((char*)(s))[5], \
		  ((char*)(d))[0] = ((char*)(s))[6], 7 )


	#define RWF_GET_16_AS_32(d,s) \
		( ((char*)(d))[3] = 0, \
		  ((char*)(d))[2] = 0, \
		  ((char*)(d))[1] = ((char*)(s))[0], \
		  ((char*)(d))[0] = ((char*)(s))[1], 2 )




/* these macros will put the length specified numerics into the 
   buffer - we need the length to be before the formatting and the
   value.  Thus, we take three parameters.  d - destination, 
   f - format bit, s - source.  Things will be written into destination
   and for GET_LENSPEC, things will be written into format bit */
 
#define RWF_PUT_LENSPEC_NUMERIC32(d,f,s)\
	 ( (*((rtrUInt8*)f) & 0x20) ? \
		   ( ((char*)(d))[0] = 0, 1 \
		   ) : \
		 ( ( *((rtrInt32*)s) > 0 )  ? \
		  ( ( *((rtrInt32*)s) < 128 ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : \
		  ( ( *((rtrInt32*)s) < 32768 ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
		  ( ( *((rtrInt32*)s) < 8388608 ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) \
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt32*)s) >= -128 ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt32*)s) >= -32768 ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 ) : \
		  ( ( *((rtrInt32*)s) >= -8388608 ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) \
		  ) \
		 ) \
		) \
	  ) \
	 )





	#define RWF_GET_LENSPEC_NUMERIC32(d,f,s) \
	( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
		 	( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( (((unsigned char*)(s))[2] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
                ((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( (((unsigned char*)(s))[2] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[1], \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[2], \
			     ((char*)(d))[0] = ((char*)(s))[3], 4 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 4 )  ? \
			( (((unsigned char*)(s))[2] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[1] = ((char*)(s))[3], \
			    ((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 5 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[0] = ((char*)(s))[5], 6 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )
	


	#define RWF_GET_LENSPEC_NUMERIC32_SIZE(d,f,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 0 \
			) : \
         (( size == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
			    ((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
			    ((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 (( size == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[0], \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[1], \
			     ((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  (( size == 4 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
			    ((char*)(d))[1] = ((char*)(s))[2], \
			    ((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		   (( size == 5 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[4], 5 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )
	


#define RWF_PUT_LENSPEC_NUMERIC64(d,f,s)\
		( (*((rtrUInt8*)f) & 0x20) ? \
		   ( ((char*)(d))[0] = 0, 1 \
		   ) : \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(128) ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(8388608) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(549755813888) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL( 36028797018963968 ) ) ? \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[8] = ((char*)(s))[0], 9 \
			) : \
			( ((char*)(d))[0] = 9, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[2], \
			  ((char*)(d))[8] = ((char*)(s))[1], \
			  ((char*)(d))[9] = ((char*)(s))[0], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt64*)s) >= RTR_LL(-128) ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-8388608) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[0], 6 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-549755813888) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[0], 8 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-36028797018963968) ) ? \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[8] = ((char*)(s))[0], 9 \
			) : \
			( ((char*)(d))[0] = 9, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[2], \
			  ((char*)(d))[8] = ((char*)(s))[1], \
			  ((char*)(d))[9] = ((char*)(s))[0], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		)  \
	   ) \
	  )
	




	#define RWF_GET_LENSPEC_NUMERIC64(d,f,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
			( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
			    ((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[1], \
				 ((char*)(d))[7] = 0, \
				 ((char*)(d))[6] = 0, \
				 ((char*)(d))[5] = 0, \
				 ((char*)(d))[4] = 0, \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[2], \
			     ((char*)(d))[0] = ((char*)(s))[3], 4 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 4 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[1] = ((char*)(s))[3], \
			    ((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 5 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[1] = ((char*)(s))[4], \
				((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
			    ((char*)(d))[1] = ((char*)(s))[4], \
			    ((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 6 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[1] = ((char*)(s))[5], \
				((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
			    ((char*)(d))[1] = ((char*)(s))[5], \
			    ((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 7 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
			    ((char*)(d))[2] = ((char*)(s))[5], \
				((char*)(d))[1] = ((char*)(s))[6], \
				((char*)(d))[0] = ((char*)(s))[7], 8 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
			    ((char*)(d))[2] = ((char*)(s))[5], \
			    ((char*)(d))[1] = ((char*)(s))[6], \
			    ((char*)(d))[0] = ((char*)(s))[7], 8 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 8 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[7] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
			    ((char*)(d))[2] = ((char*)(s))[6], \
				((char*)(d))[1] = ((char*)(s))[7], \
				((char*)(d))[0] = ((char*)(s))[8], 9 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
			    ((char*)(d))[2] = ((char*)(s))[6], \
			    ((char*)(d))[1] = ((char*)(s))[7], \
			    ((char*)(d))[0] = ((char*)(s))[8], 9 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 9 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[1] = ((char*)(s))[8], \
			  ((char*)(d))[0] = ((char*)(s))[9], 10 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
	   )
	


	#define RWF_GET_LENSPEC_NUMERIC64_SIZE(d,f,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 0 \
			) : \
		 (( size == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 (( size == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[0], \
				 ((char*)(d))[7] = 0, \
				 ((char*)(d))[6] = 0, \
				 ((char*)(d))[5] = 0, \
				 ((char*)(d))[4] = 0, \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[1], \
			     ((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  (( size == 4 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
			    ((char*)(d))[1] = ((char*)(s))[2], \
			    ((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  (( size == 5 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[1] = ((char*)(s))[3], \
			    ((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  (( size == 6 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[1] = ((char*)(s))[4], \
				((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
			    ((char*)(d))[1] = ((char*)(s))[4], \
			    ((char*)(d))[0] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  (( size == 7 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[1] = ((char*)(s))[5], \
				((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
			    ((char*)(d))[1] = ((char*)(s))[5], \
			    ((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		  (( size == 8 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
			    ((char*)(d))[2] = ((char*)(s))[5], \
				((char*)(d))[1] = ((char*)(s))[6], \
				((char*)(d))[0] = ((char*)(s))[7], 8 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
			    ((char*)(d))[2] = ((char*)(s))[5], \
			    ((char*)(d))[1] = ((char*)(s))[6], \
			    ((char*)(d))[0] = ((char*)(s))[7], 8 \
			  ) \
			) : \
		   (( size == 9 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[0], \
			  ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[0] = ((char*)(s))[8], 9 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
	   )	


/* Reserved Bit Numerics */
/* These macros write numeric types as reserved bit.  The top two bits
   of the format byte are reserved for length.  On the 32 bit values, 
   0x00 = 1 byte value, 0x40 = 2 byte value, 0x80 = 3 byte value, 0xC0 = 
   4 byte value.  For 64 bit values 0x00 = 2 byte value, 0x40 = 4 byte value, 
   0x80 = 6 byte value, and 0xC0 = 8 byte value.  The values are encoded
   as a signed integer value */
#define RWF_PUT_RESBIT_NUMERIC32(d,f,s)\
	 ( (*((rtrUInt8*)f) == 0x20) ? \
		   ( ((char*)(d))[0] = 0x20, 1 \
		   ) : \
		 ( ( *((rtrInt32*)s) > 0 )  ? \
		  ( ( *((rtrInt32*)s) < 128 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x00), \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 \
			) : \
		  ( ( *((rtrInt32*)s) < 32768 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x40), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : \
		  ( ( *((rtrInt32*)s) < 8388608 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x80), \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0xC0), \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) \
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt32*)s) >= -128 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x00), \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 ) : \
		  ( ( *((rtrInt32*)s) >= -32768 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x40), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt32*)s) >= -8388608 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x80), \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0xC0), \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) \
		  ) \
		 ) \
		) \
	  ) \
	 )





	#define RWF_GET_RESBIT_NUMERIC32(d,f,s) \
	( ( (unsigned char)((char*)(s))[0] == 0x20 )  ? \
		 	( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		 (( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x00 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
                ((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[0] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x40 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[0], \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[1], \
			     ((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x80 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[3] = 0, \
			    ((char*)(d))[2] = ((char*)(s))[1], \
			    ((char*)(d))[1] = ((char*)(s))[2], \
			    ((char*)(d))[0] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		   (( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0xC0 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[4], 5 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )
	



#define RWF_PUT_RESBIT_NUMERIC64(d,f,s)\
		( (*((rtrUInt8*)f) == 0x20) ? \
		   ( ((char*)(d))[0] = 0x20, 1 \
		   ) : \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x00), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x40), \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x80), \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0xC0), \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[8] = ((char*)(s))[0], 9 \
		    ) \
		   ) \
		 ) \
		) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x00), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x40), \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x80), \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[0], 7 \
			) : \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0xC0), \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[8] = ((char*)(s))[0], 9 \
		   )\
		  ) \
		 ) \
		)  \
	   ) \
	  )
	




	#define RWF_GET_RESBIT_NUMERIC64(d,f,s) \
		( ( (unsigned char)((char*)(s))[0] == 0x20 )  ? \
			( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x00 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
			    ((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[0], \
				 ((char*)(d))[7] = 0, \
				 ((char*)(d))[6] = 0, \
				 ((char*)(d))[5] = 0, \
				 ((char*)(d))[4] = 0, \
				 ((char*)(d))[3] = 0, \
			     ((char*)(d))[2] = 0, \
			     ((char*)(d))[1] = ((char*)(s))[1], \
			     ((char*)(d))[0] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x40 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = 0, \
				((char*)(d))[4] = 0, \
				((char*)(d))[3] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[1] = ((char*)(s))[3], \
			    ((char*)(d))[0] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x80 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[7] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[1] = ((char*)(s))[5], \
				((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[7] = 0, \
				((char*)(d))[6] = 0, \
				((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
			    ((char*)(d))[2] = ((char*)(s))[4], \
			    ((char*)(d))[1] = ((char*)(s))[5], \
			    ((char*)(d))[0] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		   (( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0xC0 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[0], \
			  ((char*)(d))[7] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[0] = ((char*)(s))[8], 9 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )
	

		
	/* These macros implement a reserved bit strategy. They allow
	 * for a maximum unsigned 15 bit value to be represented. If
	 * the high order bit (0x80) is set in the first byte, then the
	 * second byte is part of the value. The high order bit is
	 * not part of the value.
	 */
	#define RWF_PUT_RESBIT_U15(d,s) \
		( ( *((rtrUInt16*)s) < (rtrUInt16)0x80 )  ? \
			( ((char*)(d))[0] = ((char*)(s))[0], 1 ) : \
		 ( ( *((rtrUInt16*)s) < (rtrUInt16)0x8000 )  ? \
			( ((char*)(d))[0] = (((char*)(s))[1] | (rtrUInt8)0x80), \
			  ( (char*)(d))[1] = ((char*)(s))[0], 2 \
			) : -1 \
		 ) \
		)

	#define RWF_GET_RESBIT_U15(d,s) \
		( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x80 )  ? \
			( ((char*)(d))[1] = (((char*)(s))[0] & (rtrUInt8)0x7F), \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
			( ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 \
			) \
		)

	#define RWF_GET_RESBIT_U15_AS32(d,s) \
		( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x80 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = (((char*)(s))[0] & (rtrUInt8)0x7F), \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 ) : \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 ) )




	/* resbit 15 signed - 14 bits of precision plus sign extension */
	#define RWF_PUT_RESBIT_I15(d,s) \
		( ( ( *((rtrInt16*)s) >= (rtrInt16) -0x40 ) && \
		    ( *((rtrInt16*)s) < (rtrInt16) 0x40 ) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0] & (rtrUInt8)0x7F, 1 ) : \
		  ( ( ( *((rtrInt16*)s) >= (rtrInt16) -0x4000 ) && \
		      ( *((rtrInt16*)s) < (rtrInt16) 0x4000 ) )  ? \
			( ((char*)(d))[0] = (((char*)(s))[1] | (rtrUInt8)0x80), \
			  ( (char*)(d))[1] = ((char*)(s))[0], 2 \
			) : -1 \
		  ) \
		)

	#define RWF_GET_RESBIT_I15(d,s) \
		( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x80 )  ? \
			( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x40 ) ? \
				( ((char*)(d))[1] = ((char*)(s))[0], \
				  ((char*)(d))[0] = ((char*)(s))[1], 2 \
				) : \
				( ((char*)(d))[1] = (((char*)(s))[0] & (rtrUInt8)0x7F), \
				  ((char*)(d))[0] = ((char*)(s))[1], 2 \
				) \
			) : \
			( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x40 ) ? \
				( ((char*)(d))[1] = ((char) 0xFF), \
				  ((char*)(d))[0] = (((char*)(s))[0] | (rtrUInt8)0x80), 1 \
				) : \
				( ((char*)(d))[1] = 0, \
				  ((char*)(d))[0] = ((char*)(s))[0], 1 \
				) \
			) \
		)

	#define RWF_GET_RESBIT_I15_AS32(d,s) \
		( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x80 )  ? \
			( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x40 ) ? \
				( ((char*)(d))[3] = ((char)0xFF), \
				  ((char*)(d))[2] = ((char)0xFF), \
				  ((char*)(d))[1] = ((char*)(s))[0], \
				  ((char*)(d))[0] = ((char*)(s))[1], 2 \
				) : \
				( ((char*)(d))[3] = 0, \
				  ((char*)(d))[2] = 0, \
				  ((char*)(d))[1] = (((char*)(s))[0] & (rtrUInt8)0x7F), \
				  ((char*)(d))[0] = ((char*)(s))[1], 2 \
				) \
			) : \
			( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x40 ) ? \
				( ((char*)(d))[3] = ((char) 0xFF), \
				  ((char*)(d))[2] = ((char) 0xFF), \
				  ((char*)(d))[1] = ((char) 0xFF), \
				  ((char*)(d))[0] = (((char*)(s))[0] | (rtrUInt8)0x80), 1 \
				) : \
				( ((char*)(d))[3] = 0, \
				  ((char*)(d))[2] = 0, \
				  ((char*)(d))[1] = 0, \
				  ((char*)(d))[0] = ((char*)(s))[0], 1 \
				) \
			) \
		)


/* USED BY TRWF */

	/* These macros implement a reserved bits strategy. They allow
	 * for a maximum unsigned 22 bit value to be represented. The
	 * high order 1 or 2 bits are used to represent the number of bytes
	 * in the value. High order order 1 or 2 bits are not part of the value.
	 * 0x00 - 1 byte (7 bits) (uses top bit as 0 - 0x0)
	 * 0x80 - 2 bytes (14 bits) (uses top two bits as 10 - 0x80)
	 * 0x40 - 3 bytes (22 bits) (uses top two bits as 11 - 0xC0)
	 */
	#define RWF_PUT_RESBIT_U22(d,s) \
		( ( *((rtrUInt32*)s) < 0x80 )  ? \
			( ((char*)(d))[0] = ((char*)(s))[0], 1 ) : \
		 ( ( *((rtrUInt32*)s) < 0x4000 ) ? \
			( ((char*)(d))[0] = (((char*)(s))[1] | (rtrUInt8)0x80), \
				((char*)(d))[1] = ((char*)(s))[0], 2 \
			) : \
		  ( ( *((rtrUInt32*)s) < 0x400000 ) ? \
			( ((char*)(d))[0] = (((char*)(s))[2] | (rtrUInt8)0xC0), \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : -1 \
		  ) \
		 ) \
		 )

	#define RWF_GET_RESBIT_U22(d,s) \
		(( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0x80) == (rtrUInt8)0x00 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x80 )  ? \
		 	( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			)\
		  ) \
		)

	/* ResBitU31 -- 
	 * 0x00 - 3 bytes(23 bits)
	 * 0x80 - 4 bytes(31 bits) 
	 */
	#define RWF_PUT_RESBIT_U31(d,s) \
		( ( *((rtrUInt32*)s) < 0x800000 ) ? \
			(   ((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[0] = ((char*)(s))[2], 3 \
		) : ( ( *((rtrUInt32*)s) < 0x80000000 ) ? \
			(   ((char*)(d))[3] = ((char*)(s))[0], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[0] = (((char*)(s))[3] | (rtrUInt8)0x80), 4 \
		) : -1 ) )
		

	#define RWF_GET_RESBIT_U31(d,s) \
		( ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0x80))  ? \
		 	( ((char*)(d))[3] = (((char*)(s))[0] & (rtrUInt8)0x7F), \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) : \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			)\
		  ) \
		)
		
/* ResBitI30
 * 0x80 is the resBit(1 => 30b, 0 => 22b), 0x40 is a sign bit 
 * Used by TRWF */

#define RWF_PUT_RESBIT_I30(d,s) \
	( \
	  ( *((rtrInt32*)s) < RWF_MIN_I30 || *((rtrInt32*)s) > RWF_MAX_I30) ?  -1 : /* Invalid */ \
	  ( \
		*((rtrInt32*)s) < (rtrInt32)0xFFC00000 || *((rtrInt32*)s) > (rtrInt32)0x003FFFFF) ? \
	  ( /* 30b */ \
		((char*)(d))[3] = ((char*)(s))[0], \
		((char*)(d))[2] = ((char*)(s))[1], \
		((char*)(d))[1] = ((char*)(s))[2], \
		((char*)(d))[0] = ((char*)(s))[3] | (rtrInt8)0x80, 4 /* Add resBit */ \
	  ) \
	  : \
	  ( /* 22b */ \
		((char*)(d))[2] = ((char*)(s))[0], \
		((char*)(d))[1] = ((char*)(s))[1], \
		((char*)(d))[0] = ((char*)(s))[2] & (rtrInt8)~0x80, 3 /* Take off resBit */ \
	  ) \
	)

#define RWF_GET_RESBIT_I30(d,s) \
	( \
	  ( *((rtrInt8*)s) & (rtrInt8)0x80 ) ? \
	  ( /* 30b */ \
		( \
		  (((char*)(s))[0] & (rtrInt8)0x40) ? \
		  (((char*)(d))[3] = ((char*)(s))[0] | (rtrInt8)0xC0) /* Negative, sign extend */\
		  :\
		  (((char*)(d))[3] = ((char*)(s))[0] & (rtrInt8)~0x80) /* Positive, take off resBit */\
		), \
		((char*)(d))[2] = ((char*)(s))[1], \
		((char*)(d))[1] = ((char*)(s))[2], \
		((char*)(d))[0] = ((char*)(s))[3], 4 \
	  ) \
	  : \
	  ( /* 20b */ \
		( (((char*)(s))[0] & (rtrInt8)0x40) ? \
		  (\
		   ((unsigned char*)(d))[3] = 0xFF, \
		   ((char*)(d))[2] = ((char*)(s))[0] | (rtrInt8)0xC0 /* Negative, sign extend */\
		  )\
		  :\
		  (\
		   ((char*)(d))[3] = 0, \
		   ((char*)(d))[2] = ((char*)(s))[0] /* Positive, resBit already ok */\
		  )\
		), \
		((char*)(d))[1] = ((char*)(s))[1], \
		((char*)(d))[0] = ((char*)(s))[2], 3 \
	  ) \
) 

/* Replaces the value in the current position, so long as the new value is a valid fit 
 * -- returns -1 if invalid, 0 if the new value doesn't fit */
#define RWF_REPLACE_RESBIT_I30(d,s) \
	(\
	 ( *((rtrInt32*)s) < RWF_MIN_I30 || *((rtrInt32*)s) > RWF_MAX_I30) ?  -1 : /* Invalid */ \
	 (\
	  (((char*)(d))[0] & (rtrInt8)0x80) ? /* Check size of current value */ \
	  ( /* 30b can fit any valid value */ \
		((char*)(d))[3] = ((char*)(s))[0], \
		((char*)(d))[2] = ((char*)(s))[1], \
		((char*)(d))[1] = ((char*)(s))[2], \
		((char*)(d))[0] = ((char*)(s))[3] | (rtrInt8)0x80, 4 /* Add resBit */ \
	  )\
	  :\
	  ( /*22b can only fit other 22b values */\
		(*((rtrInt32*)s)< (rtrInt32)0xFFC00000 || *((rtrInt32*)s)> (rtrInt32)0x003FFFFF) ? 0 : /* Too big */ \
		(\
		 ((char*)(d))[2] = ((char*)(s))[0], \
		 ((char*)(d))[1] = ((char*)(s))[1], \
		 ((char*)(d))[0] = ((char*)(s))[2] & (rtrInt8)~0x80, 3 /* Take off resBit */ \
		)\
	  )\
	 )\
	)

/* End TRWF Section */


	/* These macros implement a reserved bits strategy. They allow
	 * for a maximum unsigned 30 bit value to be represented. The
	 * high order 2 bits are used to represent the number of bytes
	 * in the value. High order order 2 bits are not part of the value.
	 * 0x00 - 1 byte (6 bits)
	 * 0x80 - 2 bytes (14 bits)
	 * 0x40 - 3 bytes (22 bits)
	 */
	#define RWF_PUT_RESBIT_U30(d,s) \
		( ( *((rtrUInt32*)s) < 0x40 )  ? \
			( ((char*)(d))[0] = ((char*)(s))[0], 1 ) : \
		 ( ( *((rtrUInt32*)s) < 0x4000 ) ? \
			( ((char*)(d))[0] = (((char*)(s))[1] | (rtrUInt8)0x80), \
				((char*)(d))[1] = ((char*)(s))[0], 2 \
			) : \
		  ( ( *((rtrUInt32*)s) < 0x400000 ) ? \
			( ((char*)(d))[0] = (((char*)(s))[2] | (rtrUInt8)0x40), \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : \
		   ( ( *((rtrUInt32*)s) < 0x40000000 ) ? \
			( ((char*)(d))[0] = (((char*)(s))[3] | (rtrUInt8)0xC0), \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		)

	#define RWF_GET_RESBIT_U30(d,s) \
		(( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x00 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x80 )  ? \
		 	( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & 0xC0) == (rtrUInt8)0x40 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : \
			( ((char*)(d))[3] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			)\
		  ) \
		 ) \
		)

/* USED BY TRWF */

/* These macros implement a reserved bits strategy. They allow
	 * for a maximum unsigned 27 bit value to be represented. The
	 * 5th & 6th bits are used to represent the number of bytes
	 * in the value. The 2 bits are not part of the value.
	 * 0x10 - 1 byte (8 bits)
	 * 0x20 - 2 bytes (16 bits)
     * 0x30 - 3B+3b (27 bits)
	 *
     * Format:
     *   ffrr0sss	f = format bits
	 *   ssssssss	r = reserved bits
	 *   ssssssss	s = source bits
	 *   ssssssss
	 * The top three value bits are ONLY used in the biggest case. Otherwise they are set to 0.
	 */

	#define RWF_PUT_RESBIT_U27(d,s,f) \
		 ( ( *((rtrUInt32*)s) <= 0xFF ) ? \
			( ((char*)(d))[0] = (rtrUInt8)0x10 | ((rtrUInt8)(f << 6) & 0xC0), \
				((char*)(d))[1] = ((char*)(s))[0], 2 \
			) : \
		  ( ( *((rtrUInt32*)s) <= 0xFFFF ) ? \
			( ((char*)(d))[0] = (rtrUInt8)0x20 | ((rtrUInt8)(f << 6) & 0xC0), \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[0], 3 \
			) : \
		   ( ( *((rtrUInt32*)s) <= 0x7FFFFFF ) ? \
			( ((char*)(d))[0] = ((((char*)(s))[3] & 0x7) | (rtrUInt8)0x30 | ((rtrUInt8)(f << 6) & 0xC0)), \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[0], 4 \
			) : -1 \
		   )))

	// Currently does not include the format bits as they are not needed when this is used.
	// Be wary of changing that behavior.
	#define RWF_GET_RESBIT_U27(d,s) \
		(( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0x30) == (rtrUInt8)0x10 )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0x30) == (rtrUInt8)0x20 ) ? \
		 	( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : \
			( ((char*)(d))[3] = (((char*)(s))[0] & 0x7), \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) \
			) \
			)

/* END TRWF Section */


	/* These macros implement a reserved bits strategy. They allow
	 * for a maximum unsigned 62 bit value to be represented. The
	 * high order 2 bits are used to represent the number of bytes
	 * in the value. High order order 2 bits are not part of the value.
	 * 0x00 - 2 byte (14 bits)
	 * 0x40 - 4 bytes (30 bits)
	 * 0x80 - 6 bytes (46 bits)
	 * 0xC0 - 8 bytes (62 bits)
	 */
		#define RWF_PUT_RESBIT_U62(d,s) \
		( ( *((rtrUInt64*)s) < RTR_ULL(0x4000) )  ? \
			(	((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[0], 2  \
			) : \
		 ( ( *((rtrUInt64*)s) < RTR_ULL(0x40000000) ) ? \
			(	((char*)(d))[0] = (((char*)(s))[3] | (rtrUInt8)0x40), \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[0], 4  \
			) : \
		  ( ( *((rtrUInt64*)s) < RTR_ULL(0x400000000000) ) ? \
			(	((char*)(d))[0] = (((char*)(s))[5] | (rtrUInt8)0x80), \
				((char*)(d))[1] = ((char*)(s))[4], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[1], \
				((char*)(d))[5] = ((char*)(s))[0], 6  \
			) : \
		   ( ( *((rtrUInt64*)s) < RTR_ULL(0x4000000000000000) ) ? \
			(	((char*)(d))[0] = (((char*)(s))[7] | (rtrUInt8)0xC0), \
				((char*)(d))[1] = ((char*)(s))[6], \
				((char*)(d))[2] = ((char*)(s))[5], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[3], \
				((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[7] = ((char*)(s))[0], 8  \
			) : -1 \
		   ) \
		  ) \
		 ) \
		)

	#define RWF_GET_RESBIT_U62(d,s) \
		(( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x00 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], 2 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x40 )  ? \
		 	( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[3] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[3], 4 \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & 0xC0) == (rtrUInt8)0x80 )  ? \
			( ((char*)(d))[7] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[5] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[0] = ((char*)(s))[5], 6 \
			) : \
			( ((char*)(d))[7] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[0] = ((char*)(s))[7], 8 \
			)\
		  ) \
		 ) \
		)



	/* These macros implement a reserved value scalar optimization
	 * strategy. The values are always represented by a minimum
	 * number of byte(s). If the values of these byte(s) is some
	 * reserved value(s), then there are more bytes that represent
	 * the value.
	 */

	/* These macros implement an optmized byte, with max u16 reserved
	 * value stragety. There is always one byte, if the value of this
	 * byte tells you how to interpret the value.  This has been changed
	 * to 0xFE so it is compatable with the U32 encoding as well.
	 *    byte value < 0xFE - The byte is the value.
	 *    byte value = 0xFE | 0xFF - Ignore the first byte, the next two bytes
	 *                        are the value.
	 * Ex:
	 *   0xFD = 0xFD00 = 253
	 *   0xFE00FE = 0xFE00 = 254
	 */
	#define RWF_PUT_OPTBYTE_U16(d,s) \
		( ( *((rtrUInt16*)s) < (rtrUInt16)0xFE )  ? \
			( ( (char*)(d))[0] = ((char*)(s))[0], 1 ) : \
			  ( ((unsigned char*)(d))[0] = (rtrUInt8)0xFE, \
			    ((char*)(d))[1] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[0], 3 ) )

	#define RWF_GET_OPTBYTE_U16(d,s) \
		( ( (unsigned char)((char*)(s))[0] < (rtrUInt16)0xFE )  ? \
			( ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 ) : \
			( ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 ) )

	/* These macros implement an optmized byte, with max u32 reserved
	 * value stragety. There is always one byte, if the value of this
	 * byte tells you how to interpret the value.
	 *    byte value < 0xFE - The byte is the value.
	 *    byte value = 0xFE - Ignroe the first byte, the next two bytes
	 *                        are the value.
	 *    byte value = 0xFF - Ignore the first byte, the next four bytes
	 *                        are the value.
	 * Ex:
	 *   0xFD = 0xFD00 - 253
	 *   0xFE00FE - 0xFE00 - 254
	 *   0xFEFFFF = 0xFFFF - 65535
	 *   0xFF00010000 = 0x00000100 - 65536
	 */
	#define RWF_PUT_OPTBYTE_U32(d,s) \
		( ( *((rtrUInt32*)s) < (rtrUInt32)0xFE )  ? \
			( ( (char*)(d))[0] = ((char*)(s))[0], 1 ) : \
		 ( ( *((rtrUInt32*)s) <= 0xFFFF )  ? \
		 	( ((unsigned char*)(d))[0] = (rtrUInt8)0xFE, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[0], 3 ) : \
			( ((char*)(d))[0] = (rtrUInt8)0xFF, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[0], 5 \
			) \
		 ) \
		)

	#define RWF_GET_OPTBYTE_U32(d,s) \
		( ( (unsigned char)((char*)(s))[0] < (rtrUInt8)0xFE )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == (rtrUInt8)0xFE ) ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 \
			) : \
			( ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[4], 5 \
			) \
		 ) \
		)

	#define RWF_GET_OPTBYTE_U16_AS32(d,s) \
		( ( (unsigned char)((char*)(s))[0] < (rtrUInt8)0xFE )  ? \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = ((char*)(s))[0], 1 ) : \
			( ((char*)(d))[3] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], 3 ) )



#else/* RTR_BIG_ENDIAN */
	/* The following are for BE machines (i.e. high-byte to low-byte ordering) */


	#define RWF_MOVE_16(d,s) \
		(((char*)(d))[0] = ((char*)(s))[0],\
		 ((char*)(d))[1] = ((char*)(s))[1], 2 )



	#define RWF_MOVE_16_NO_SWAP(d,s) \
		(((char*)(d))[0] = ((char*)(s))[0],\
		 ((char*)(d))[1] = ((char*)(s))[1], 2 )

	#define RWF_MOVE_32(d,s) \
		(((char*)(d))[0] = ((char*)(s))[0],\
		 ((char*)(d))[1] = ((char*)(s))[1],\
		 ((char*)(d))[2] = ((char*)(s))[2],\
		 ((char*)(d))[3] = ((char*)(s))[3], 4 )

	#define RWF_MOVE_64(d,s)\
		(((char*)(d))[0] = ((char*)(s))[0],\
		 ((char*)(d))[1] = ((char*)(s))[1],\
		 ((char*)(d))[2] = ((char*)(s))[2],\
		 ((char*)(d))[3] = ((char*)(s))[3],\
		 ((char*)(d))[4] = ((char*)(s))[4],\
		 ((char*)(d))[5] = ((char*)(s))[5],\
		 ((char*)(d))[6] = ((char*)(s))[6],\
		 ((char*)(d))[7] = ((char*)(s))[7], 8 )

	#define RWF_MOVE_FLOAT(d,s)\
		(((char*)(d))[0] = ((char*)(s))[0],\
		 ((char*)(d))[1] = ((char*)(s))[1],\
		 ((char*)(d))[2] = ((char*)(s))[2],\
		 ((char*)(d))[3] = ((char*)(s))[3], 4 )

	#define RWF_MOVE_DOUBLE(d,s)\
		(((char*)(d))[0] = ((char*)(s))[0],\
		 ((char*)(d))[1] = ((char*)(s))[1],\
		 ((char*)(d))[2] = ((char*)(s))[2],\
		 ((char*)(d))[3] = ((char*)(s))[3],\
		 ((char*)(d))[4] = ((char*)(s))[4],\
		 ((char*)(d))[5] = ((char*)(s))[5],\
		 ((char*)(d))[6] = ((char*)(s))[6],\
		 ((char*)(d))[7] = ((char*)(s))[7], 8 )

	/* Length specified scalar values */

	/* sets the length specifier to 0 to signify blank value.  This
	   should work for any length specified type */
	#define RWF_PUT_LENSPEC_BLANK(d) \
		(  ((char*)(d))[0] = 0, 1 )


	#define RWF_PUT_LENSPEC_U16(d,s)\
		( ( *((rtrUInt16*)s) <= (rtrUInt16)0xFF )  ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[1], 2 ) : \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], 3 \
			) \
		)

	#define RWF_GET_LENSPEC_U16(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
			( ((char*)(d))[1] = 0, \
			  ((char*)(d))[0] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], 2 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[1], 3 \
			) : -1 \
		  ) \
		) \
	   )

	#define RWF_GET_LENSPEC_U16_SIZE(d,s,size) \
		 (( size == 0 )  ? \
		    ( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, 0 \
			) : \
		 (( size == 1 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], 1 \
			) : \
		 ( ( size == 2 )  ? \
		 	( ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[0], 2 \
			) : -1 \
		  ) \
		 ) \
		)


#define RWF_PUT_LENSPEC_I16(d,s) \
		( ( *((rtrInt16*)s) > 0 )  ? \
		  ( ( *((rtrInt16*)s) < 128 ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[1], 2 \
			) : \
		    ( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], 3 \
			)  \
		  ) : \
		 ( ( *((rtrInt16*)s) >= -128 ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[1], 2 \
            ) : \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], 3 \
			) \
         ) \
	   )


	#define RWF_GET_LENSPEC_I16(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 2 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], 3 \
			) : -1 \
		   ) \
		  ) \
   	    )

	#define RWF_GET_LENSPEC_I16_SIZE(d,s,size) \
		   (( size == 0 )  ? \
		    ( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, 0 \
			) : \
		  (( size == 1 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[0], 1 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = ((char*)(s))[0], 1 \
			  ) \
			) : \
		   (( size == 2 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], 2 \
			) : -1 \
		   ) \
		  ) \
	    )




	#define RWF_PUT_LENSPEC_U32(d,s)\
		( ( *((rtrUInt32*)s) <= 0xFF )  ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[3], 2 ) : \
		 ( ( *((rtrUInt32*)s) <= 0xFFFF ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], 3 \
			) : \
		  ( ( *((rtrUInt32*)s) <= 0xFFFFFF ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], 5 \
			) \
		  ) \
		 ) \
		)

	#define RWF_GET_LENSPEC_U32(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[1], 2 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], 3 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 4 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], 5 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )

	#define RWF_GET_LENSPEC_U32_SIZE(d,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, 0 \
			) : \
		 (( size == 1 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[0], 1 \
			) : \
		 ( ( size == 2 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], 2 \
			) : \
		  ( ( size == 3 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], 3 \
			) : \
		   (( size == 4 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		 ) \
		)

#define RWF_PUT_LENSPEC_I32(d,s) \
		( ( *((rtrInt32*)s) > 0 )  ? \
		  ( ( *((rtrInt32*)s) < 128 ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[3], 2 ) : \
		  ( ( *((rtrInt32*)s) < 32768 ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], 3 ) : \
		  ( ( *((rtrInt32*)s) < 8388608 ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], 5 \
			) \
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt32*)s) >= -128 ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[3], 2 ) : \
		  ( ( *((rtrInt32*)s) >= -32768 ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], 3 ) : \
		  ( ( *((rtrInt32*)s) >= -8388608 ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], 5 \
			) \
		  ) \
		 ) \
		) \
	   )


	#define RWF_GET_LENSPEC_I32(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[3] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(d))[0] = 0, \
			     ((char*)(d))[1] = 0, \
			     ((char*)(d))[2] = ((char*)(s))[1], \
			     ((char*)(d))[3] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
			    ((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[3] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 4 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], 5 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )

	#define RWF_GET_LENSPEC_I32_SIZE(d,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, 0 \
			) : \
         (( size == 1 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[0], 1 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[3] = ((char*)(s))[0], 1 \
			  ) \
			) : \
		 (( size == 2 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], 2 \
			   ) : \
		 	   ( ((char*)(d))[0] = 0, \
			     ((char*)(d))[1] = 0, \
			     ((char*)(d))[2] = ((char*)(s))[0], \
			     ((char*)(d))[3] = ((char*)(s))[1], 2 \
			   ) \
			) : \
		  (( size == 3 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
			    ((char*)(d))[1] = ((char*)(s))[0], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = ((char*)(s))[0], \
			    ((char*)(d))[2] = ((char*)(s))[1], \
			    ((char*)(d))[3] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		   (( size == 4 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )


#define RWF_PUT_LENSPEC_U64(d,s)\
	   (  ( *((rtrUInt64*)s) <= RTR_ULL(0xFF) )  ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 ) : \
		( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFF) )  ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		 ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFF)   ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 \
			) : \
		 ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFF)   ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6\
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], \
			  ((char*)(d))[8] = ((char*)(s))[7], 9 \
			) \
		  ) \
		 ) \
		) \
	   ) \
	  ) \
     ) \
    ) 
      


	#define RWF_GET_LENSPEC_U64(d,s) \
		(( (unsigned char)((char*)(s))[0] == 0 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = 0, 1 \
			) : \
		((  (unsigned char)((char*)(s))[0] == 1 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = ((char*)(s))[1], 2 \
			) : \
		( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[2], 3 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[3], 4 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 4 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[4], 5 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 5 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], 6 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 6 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], 7 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 7 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 8 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], \
			  ((char*)(d))[7] = ((char*)(s))[8], 9 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		)\
		)\
		)\
		) \
		)


/* used to write default sidding bitmaps after stop bits are injected */
#define RWF_PUT_LENSPEC_U64_SIZE(d,s, size)\
	   (  ( size == 1 )  ? \
			(((char*)(d))[0] = ((char*)(s))[7], 1 ) : \
		( ( size == 2 )  ? \
			( ((char*)(d))[0] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 ) : \
		 ( ( size == 3   ) ? \
			( ((char*)(d))[0] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 \
			) : \
		 ( ( size == 4 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 \
			) : \
		  ( ( size == 5   ) ? \
			( ((char*)(d))[0] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5\
			) : \
		  ( ( size == 6 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6 \
			) : \
		  ( ( size == 7 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) \
		  ) \
		 ) \
		) \
	   ) \
	  ) \
     ) \
    ) 





	#define RWF_GET_LENSPEC_U64_SIZE(d,s,size) \
	    (( size == 0 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = 0, 0 \
			) : \
	    (( size == 1 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = ((char*)(s))[0], 1 \
			) : \
		(( size == 2 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = ((char*)(s))[0], \
			  ((char*)(d))[7] = ((char*)(s))[1], 2 \
			) : \
		 ( ( size == 3 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = ((char*)(s))[0], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[2], 3 \
			) : \
		 ( ( size == 4 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = ((char*)(s))[0], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[3], 4 \
			) : \
		 ( ( size == 5 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[0], \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[4], 5 \
			) : \
		  ( ( size == 6 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], 6 \
			) : \
			( ( size == 7 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], 7 \
			) : \
		   (( size == 8 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
		)



#define RWF_PUT_LENSPEC_I64(d,s) \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(128) ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(8388608) ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(549755813888) ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(36028797018963968) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], \
			  ((char*)(d))[8] = ((char*)(s))[7], 9 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt64*)s) >= RTR_LL(-128) ) ? \
			( ((char*)(d))[0] = 1, \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-8388608) ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-549755813888) ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-36028797018963968) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], \
			  ((char*)(d))[8] = ((char*)(s))[7], 9 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		)  \
	   )


	#define RWF_GET_LENSPEC_I64(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 1 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[7] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = 0, \
			    ((char*)(d))[6] = 0, \
			    ((char*)(d))[7] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((char*)(d))[5] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[7] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(d))[0] = 0, \
				 ((char*)(d))[1] = 0, \
				 ((char*)(d))[2] = 0, \
				 ((char*)(d))[3] = 0, \
				 ((char*)(d))[4] = 0, \
			     ((char*)(d))[5] = 0, \
			     ((char*)(d))[6] = ((char*)(s))[1], \
			     ((char*)(d))[7] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = ((char*)(s))[1], \
			    ((char*)(d))[6] = ((char*)(s))[2], \
			    ((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 4 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[3], \
				((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
			    ((char*)(d))[6] = ((char*)(s))[3], \
			    ((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 5 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[6] = ((char*)(s))[4], \
				((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
			    ((char*)(d))[6] = ((char*)(s))[4], \
			    ((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 6 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
				((char*)(d))[6] = ((char*)(s))[5], \
				((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
			    ((char*)(d))[6] = ((char*)(s))[5], \
			    ((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 7 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
			    ((char*)(d))[5] = ((char*)(s))[5], \
				((char*)(d))[6] = ((char*)(s))[6], \
				((char*)(d))[7] = ((char*)(s))[7], 8 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
			    ((char*)(d))[5] = ((char*)(s))[5], \
			    ((char*)(d))[6] = ((char*)(s))[6], \
			    ((char*)(d))[7] = ((char*)(s))[7], 8 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 8 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], \
			  ((char*)(d))[7] = ((char*)(s))[8], 9 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
	   )

	#define RWF_GET_LENSPEC_I64_SIZE(d,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = 0, 0 \
			) : \
		  (( size == 1 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[7] = ((char*)(s))[0], 1 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = 0, \
			    ((char*)(d))[6] = 0, \
			    ((char*)(d))[7] = ((char*)(s))[0], 1 \
			  ) \
			) : \
		 (( size == 2 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[0], \
				((char*)(d))[7] = ((char*)(s))[1], 2 \
			   ) : \
		 	   ( ((char*)(d))[0] = 0, \
				 ((char*)(d))[1] = 0, \
				 ((char*)(d))[2] = 0, \
				 ((char*)(d))[3] = 0, \
				 ((char*)(d))[4] = 0, \
			     ((char*)(d))[5] = 0, \
			     ((char*)(d))[6] = ((char*)(s))[0], \
			     ((char*)(d))[7] = ((char*)(s))[1], 2 \
			   ) \
			) : \
		  (( size == 3 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((char*)(d))[5] = ((char*)(s))[0], \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[7] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = ((char*)(s))[0], \
			    ((char*)(d))[6] = ((char*)(s))[1], \
			    ((char*)(d))[7] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		  (( size == 4 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[0], \
			    ((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = ((char*)(s))[0], \
			    ((char*)(d))[5] = ((char*)(s))[1], \
			    ((char*)(d))[6] = ((char*)(s))[2], \
			    ((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  (( size == 5 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[3], \
				((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
			    ((char*)(d))[6] = ((char*)(s))[3], \
			    ((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  (( size == 6 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[6] = ((char*)(s))[4], \
				((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
			    ((char*)(d))[6] = ((char*)(s))[4], \
			    ((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  (( size == 7 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[0], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
				((char*)(d))[6] = ((char*)(s))[5], \
				((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = ((char*)(s))[0], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
			    ((char*)(d))[6] = ((char*)(s))[5], \
			    ((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		   (( size == 8 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
	   )

/* TRWF Macros - these are used by TRWF and are 
   written to lay out bits that are specific
   to TRWF */

	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 (31) 00001000 (8) = 8 byte length */
#define TRWF_PUT_LENSPEC_I64(d,s) \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(128) ) ? \
			( ((char*)(d))[0] = 32, \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = 64, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(8388608) ) ? \
			( ((char*)(d))[0] = 96, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((unsigned char*)(d))[0] = 128, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(549755813888) ) ? \
			( ((unsigned char*)(d))[0] = 160, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((unsigned char*)(d))[0] = 192, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(36028797018963968) ) ? \
			( ((unsigned char*)(d))[0] = 224, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
			( ((char*)(d))[0] = 31, \
			  ((char*)(d))[1] = 8, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], \
			  ((char*)(d))[8] = ((char*)(s))[6], \
			  ((char*)(d))[9] = ((char*)(s))[7], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt64*)s) >= RTR_LL(-128) ) ? \
			( ((char*)(d))[0] = 32, \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = 64, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-8388608) ) ? \
			( ((char*)(d))[0] = 96, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((unsigned char*)(d))[0] = 128, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-549755813888) ) ? \
			( ((unsigned char*)(d))[0] = 160, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((unsigned char*)(d))[0] = 192, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-36028797018963968) ) ? \
			( ((unsigned char*)(d))[0] = 224, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
			( ((char*)(d))[0] = 31, \
			  ((char*)(d))[1] = 8, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], \
			  ((char*)(d))[8] = ((char*)(s))[6], \
			  ((char*)(d))[9] = ((char*)(s))[7], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		)  \
	   )

/* Almost the same as RWF_PUT_LENSPEC_U64, but uses TRWF sizes */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 (31) 00001000 (8) = 8 byte length */
	#define TRWF_PUT_LENSPEC_U64(d,s)\
	   (  ( *((rtrUInt64*)s) <= RTR_ULL(0xFF) )  ? \
			( ((char*)(d))[0] = 32, \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 ) : \
		( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFF) )  ? \
			( ((char*)(d))[0] = 64, \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		 ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFF)   ) ? \
			( ((char*)(d))[0] = 96, \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 \
			) : \
		 ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 128, \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFF)   ) ? \
			( ((char*)(d))[0] = 160, \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6\
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 192, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFFFF) ) ? \
			( ((char*)(d))[0] = 224, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
			( ((char*)(d))[0] = 31, \
			  ((char*)(d))[1] = 8, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], \
			  ((char*)(d))[8] = ((char*)(s))[6], \
			  ((char*)(d))[9] = ((char*)(s))[7], 10 \
			) \
		  ) \
		 ) \
		) \
	   ) \
	  ) \
     ) \
    ) 
      

	/* Like RWF_GET_LENSPEC_U64, uses TRWF buffer lengths */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 (31) 00001000 (8) = 8 byte length */
	#define TRWF_GET_LENSPEC_U64(d,s) \
		( ( (unsigned char)((char*)(s))[0] == 32 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = ((char*)(s))[1], 2 \
			) : \
		( ( (unsigned char)((char*)(s))[0] == 64 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[2], 3 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 96 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[3], 4 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 128 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[4], 5 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 160 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], 6 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 192 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], 7 \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 224 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 31 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], \
			  ((char*)(d))[6] = ((char*)(s))[8], \
			  ((char*)(d))[7] = ((char*)(s))[9], 10 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		)\
		)\
		)\
		)


	/* This function works like TRWF_PUT_LENSPEC_I64,
	 * but puts the bytes on starting from the top byte, rather than the bottom byte
	 *
	 * This lets us save some bandwidth for floating point numbers --
	 * Double-precision floats converted from single-precision floats
	 * typically have all their meaningful bits on the top end, usually
	 * leaving bytes of zero on the bottom end(as opposed to integers where the 0's are
	 * on the high end).
	 */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 (31) 00001000 (8) = 8 byte length */
#define TRWF_PUT_LENSPEC_I64_TOP(d,s) \
		  ( \
			( !( *((rtrUInt64*)s) & 0x00FFFFFFFFFFFFFFULL ) ? \
			( ((char*)(d))[0] = 32, \
			  ((char*)(d))[1] = ((char*)(s))[0], 2 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x0000FFFFFFFFFFFFULL ) ? \
			( ((char*)(d))[0] = 64, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], 3 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x000000FFFFFFFFFFULL ) ? \
			( ((char*)(d))[0] = 96, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], 4 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x00000000FFFFFFFFULL ) ? \
			( ((unsigned char*)(d))[0] = 128, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], 5 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x0000000000FFFFFFULL ) ? \
			( ((unsigned char*)(d))[0] = 160, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], 6 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x000000000000FFFFULL ) ? \
			( ((unsigned char*)(d))[0] = 192, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], 7 \
			) : \
		  ( !( *((rtrUInt64*)s) & 0x00000000000000FFULL ) ? \
			( ((unsigned char*)(d))[0] = 224, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], 8 \
			) : \
			( ((char*)(d))[0] = 31, \
			  ((char*)(d))[1] = 8, \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], \
			  ((char*)(d))[8] = ((char*)(s))[6], \
			  ((char*)(d))[9] = ((char*)(s))[7], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
			) \
			)
/* This still finds the length boundaries and only puts on the 
   necessary number of bytes, however it does not put the length 
   on.  It is assumed this will be done outside this function */
#define TRWF_PUT_LENSPEC_I64_NO_LENGTH(d,s) \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(128) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[7], 1 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(8388608) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(549755813888) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(36028797018963968) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt64*)s) >= RTR_LL(-128) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[7], 1 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[7], 2 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-8388608) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-549755813888) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-36028797018963968) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		)  \
	   )

#define RWF_PUT_LENSPEC_U64_NO_LENGTH(d,s)\
	   (  ( *((rtrUInt64*)s) <= RTR_ULL(0xFF))  ? \
			( ((char*)(d))[0] = ((char*)(s))[7], 1 ) : \
		( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFF))  ? \
			( ((char*)(d))[1] = ((char*)(s))[7], \
			  ((char*)(d))[0] = ((char*)(s))[6], 2 ) : \
		 ( (*((rtrUInt64*)s) <= RTR_ULL(0xFFFFFF) ) ? \
			( ((char*)(d))[2] = ((char*)(s))[7], \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[0] = ((char*)(s))[5], 3 \
			) : \
		 ( (*((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFF) ) ? \
			( ((char*)(d))[3] = ((char*)(s))[7], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[1] = ((char*)(s))[5], \
			  ((char*)(d))[0] = ((char*)(s))[4], 4 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFF)) ? \
			( ((char*)(d))[4] = ((char*)(s))[7], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[0] = ((char*)(s))[3], 5 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFF)) ? \
			( ((char*)(d))[5] = ((char*)(s))[7], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[0] = ((char*)(s))[2], 6 \
			) : \
		  ( ( *((rtrUInt64*)s) <= RTR_ULL(0xFFFFFFFFFFFFFF)) ? \
			( ((char*)(d))[6] = ((char*)(s))[7], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[0] = ((char*)(s))[1], 7 \
			) : \
			( ((char*)(d))[7] = ((char*)(s))[7], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[0], 8 \
			) \
		  ) \
		 ) \
		) \
	   ) \
	  ) \
     ) \
    ) 



	/* took out 0 case */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 00001000 (31 8) = 8 byte length */
	#define TRWF_GET_LENSPEC_I64(d,s) \
		 (( (unsigned char)((char*)(s))[0] == 32 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[7] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = 0, \
			    ((char*)(d))[6] = 0, \
			    ((char*)(d))[7] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 64 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[7] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(d))[0] = 0, \
				 ((char*)(d))[1] = 0, \
				 ((char*)(d))[2] = 0, \
				 ((char*)(d))[3] = 0, \
				 ((char*)(d))[4] = 0, \
			     ((char*)(d))[5] = 0, \
			     ((char*)(d))[6] = ((char*)(s))[1], \
			     ((char*)(d))[7] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 96 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = ((char*)(s))[1], \
			    ((char*)(d))[6] = ((char*)(s))[2], \
			    ((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  ( ( (unsigned char)((unsigned char*)(s))[0] == 128 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[3], \
				((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
			    ((char*)(d))[6] = ((char*)(s))[3], \
			    ((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  ( ( (unsigned char)((unsigned char*)(s))[0] == 160 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[6] = ((char*)(s))[4], \
				((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
			    ((char*)(d))[6] = ((char*)(s))[4], \
			    ((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  ( ( (unsigned char)((unsigned char*)(s))[0] == 192 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
				((char*)(d))[6] = ((char*)(s))[5], \
				((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
			    ((char*)(d))[6] = ((char*)(s))[5], \
			    ((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		  ( ( (unsigned char)((unsigned char*)(s))[0] == 224 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
			    ((char*)(d))[5] = ((char*)(s))[5], \
				((char*)(d))[6] = ((char*)(s))[6], \
				((char*)(d))[7] = ((char*)(s))[7], 8 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
			    ((char*)(d))[5] = ((char*)(s))[5], \
			    ((char*)(d))[6] = ((char*)(s))[6], \
			    ((char*)(d))[7] = ((char*)(s))[7], 8 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 31 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], \
			  ((char*)(d))[6] = ((char*)(s))[8], \
			  ((char*)(d))[7] = ((char*)(s))[9], 10 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
	   )

	/* see TRWF_PUT_LENSPEC_I64_TOP for information */
	/* took out 0 case */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 00001000 (31 8) = 8 byte length */
	#define TRWF_GET_LENSPEC_I64_TOP(d,s) \
		 ( \
			( ((unsigned char)((char*)(s))[0] == 32 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = 0 ,\
				((char*)(d))[2] = 0 ,\
				((char*)(d))[3] = 0 ,\
				((char*)(d))[4] = 0 ,\
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 2 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 64 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = 0 ,\
				((char*)(d))[3] = 0 ,\
				((char*)(d))[4] = 0 ,\
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 3 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 96 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = 0 ,\
				((char*)(d))[4] = 0 ,\
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 4 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 128 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = 0 ,\
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 5 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 160 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 6 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 192 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 7 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 224 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
				((char*)(d))[6] = ((char*)(s))[7], \
				((char*)(d))[7] = 0 , 8 \
			) : \
			( ((unsigned char)((char*)(s))[0] == 31 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
				((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], \
				((char*)(d))[6] = ((char*)(s))[8], \
				((char*)(d))[7] = ((char*)(s))[9], 10 \
			) : - 1\
			)))))))))

	/* see TRWF_PUT_LENSPEC_I64_TOP for information */
	/* took out 0 case */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 00001000 (31 8) = 8 byte length */
	#define TRWF_GET_LENSPEC_I64_TOP_SIZE(d,s,size) \
			((( (size) == 32 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = 0 ,\
				((char*)(d))[2] = 0 ,\
				((char*)(d))[3] = 0 ,\
				((char*)(d))[4] = 0 ,\
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 2 \
			) : \
			(( (size) == 64 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = 0 ,\
				((char*)(d))[3] = 0 ,\
				((char*)(d))[4] = 0 ,\
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 3 \
			) : \
			(( (size) == 96 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = 0 ,\
				((char*)(d))[4] = 0 ,\
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 4 \
			) : \
			(( (size) == 128 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = 0 ,\
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 5 \
			) : \
			(( (size) == 160 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = 0 ,\
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 6 \
			) : \
			(( (size) == 192 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
				((char*)(d))[6] = 0 ,\
				((char*)(d))[7] = 0 , 7 \
			) : \
			(( (size) == 224 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
				((char*)(d))[6] = ((char*)(s))[7], \
				((char*)(d))[7] = 0 , 8 \
			) : \
			(( (size) == 31 )  ? \
			  (((char*)(d))[0] = ((char*)(s))[2], \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
				((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], \
				((char*)(d))[6] = ((char*)(s))[8], \
				((char*)(d))[7] = ((char*)(s))[9], 10 \
			) : - 1\
			)))))))))

	/* took out 0 case */
	/* 00100000 (32) = 1 byte length
	 * 01000000 (64) = 2 byte length
	 * 01100000 (96) = 3 byte length 
	 * 10000000 (128) = 4 byte length
	 * 10100000 (160) = 5 byte length
	 * 11000000 (192) = 6 byte length
	 * 11100000 (224) = 7 byte length 
	 * 00011111 00001000 (31) = 8 byte length */
	#define TRWF_GET_LENSPEC_I64_SIZE(d,s,size) \
		  (( size == 32 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[7] = ((char*)(s))[0], 1 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = 0, \
			    ((char*)(d))[6] = 0, \
			    ((char*)(d))[7] = ((char*)(s))[0], 1 \
			  ) \
			) : \
		 (( size == 64 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[0], \
				((char*)(d))[7] = ((char*)(s))[1], 2 \
			   ) : \
		 	   ( ((char*)(d))[0] = 0, \
				 ((char*)(d))[1] = 0, \
				 ((char*)(d))[2] = 0, \
				 ((char*)(d))[3] = 0, \
				 ((char*)(d))[4] = 0, \
			     ((char*)(d))[5] = 0, \
			     ((char*)(d))[6] = ((char*)(s))[0], \
			     ((char*)(d))[7] = ((char*)(s))[1], 2 \
			   ) \
			) : \
		  (( size == 96 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((char*)(d))[5] = ((char*)(s))[0], \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[7] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = ((char*)(s))[0], \
			    ((char*)(d))[6] = ((char*)(s))[1], \
			    ((char*)(d))[7] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		  (( size == 128 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[0], \
			    ((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = ((char*)(s))[0], \
			    ((char*)(d))[5] = ((char*)(s))[1], \
			    ((char*)(d))[6] = ((char*)(s))[2], \
			    ((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  (( size == 160 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[3], \
				((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
			    ((char*)(d))[6] = ((char*)(s))[3], \
			    ((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  (( size == 192 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[6] = ((char*)(s))[4], \
				((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
			    ((char*)(d))[6] = ((char*)(s))[4], \
			    ((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  (( size == 224 )  ? \
			( (((unsigned char*)(s))[0] >= 0x80) ? \
			  ( ((unsigned char*)(d))[0] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[0], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
				((char*)(d))[6] = ((char*)(s))[5], \
				((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = ((char*)(s))[0], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
			    ((char*)(d))[6] = ((char*)(s))[5], \
			    ((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		   (( size == 31 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
	   )

/* Like RWF_GET_LENSPEC_U64_SIZE, but uses TRWF sizes */
#define TRWF_GET_LENSPEC_U64_SIZE(d,s,size) \
		 (( size == 32 )  ? \
			( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = ((char*)(s))[0], 1 \
			) : \
		 (( size == 64 )  ? \
			( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = ((char*)(s))[0], \
			  ((char*)(d))[7] = ((char*)(s))[1], 2 \
			) : \
		  (( size == 96 )  ? \
			( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = ((char*)(s))[0], \
			  ((char*)(d))[6] = ((char*)(s))[1], \
			  ((char*)(d))[7] = ((char*)(s))[2], 3 \
			) : \
		  (( size == 128 )  ? \
			( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = ((char*)(s))[0], \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[3], 4 \
			) : \
		  (( size == 160 )  ? \
			( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = ((char*)(s))[0], \
				((char*)(d))[4] = ((char*)(s))[1], \
			  ((char*)(d))[5] = ((char*)(s))[2], \
			  ((char*)(d))[6] = ((char*)(s))[3], \
			  ((char*)(d))[7] = ((char*)(s))[4], 5 \
			) : \
		  (( size == 192 )  ? \
			( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = ((char*)(s))[0], \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], 6 \
			) : \
		  (( size == 224 )  ? \
			( ((char*)(d))[0] = 0, \
				((char*)(d))[1] = ((char*)(s))[0], \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], 7 \
			) : \
		   (( size == 31 ) ? \
			( ((char*)(d))[0] = ((char*)(s))[0], \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
	  )

/* END TRWF */





/* these macros will put the length specified numerics into the 
   buffer - we need the length to be before the formatting and the
   value.  Thus, we take three parameters.  d - destination, 
   f - format bit, s - source.  Things will be written into destination
   and for GET_LENSPEC, things will be written into format bit */
	#define RWF_PUT_LENSPEC_NUMERIC32(d,f,s)\
		 ( (*((rtrUInt8*)f) & 0x20) ? \
		   ( ((char*)(d))[0] = 0, 1 \
		   ) : \
		 ( ( *((rtrInt32*)s) > 0 )  ? \
		  ( ( *((rtrInt32*)s) < 128 ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[3], 3 \
			) : \
		  ( ( *((rtrInt32*)s) < 32768 ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : \
		  ( ( *((rtrInt32*)s) < 8388608 ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], 5 \
			) : \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], 6 \
			) \
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt32*)s) >= -128 ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[3], 3 ) : \
		  ( ( *((rtrInt32*)s) >= -32768 ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 ) : \
		  ( ( *((rtrInt32*)s) >= -8388608 ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], 5 \
			) : \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], 6 \
			) \
		  ) \
		 ) \
		) \
	  ) \
	 )




	#define RWF_GET_LENSPEC_NUMERIC32(d,f,s) \
	( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
		 	( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( (((unsigned char*)(s))[2] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
                ((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[3] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( (((unsigned char*)(s))[2] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], 4 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[1], \
				 ((char*)(d))[0] = 0, \
			     ((char*)(d))[1] = 0, \
			     ((char*)(d))[2] = ((char*)(s))[2], \
			     ((char*)(d))[3] = ((char*)(s))[3], 4 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 4 )  ? \
			( (((unsigned char*)(s))[2] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[0] = 0xFF, \
			    ((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = ((char*)(s))[2], \
			    ((char*)(d))[2] = ((char*)(s))[3], \
			    ((char*)(d))[3] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 5 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], 6 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )
	
  

	#define RWF_GET_LENSPEC_NUMERIC32_SIZE(d,f,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, 0 \
			) : \
         (( size == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
			    ((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
			    ((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[3] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 (( size == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[0], \
				 ((char*)(d))[0] = 0, \
			     ((char*)(d))[1] = 0, \
			     ((char*)(d))[2] = ((char*)(s))[1], \
			     ((char*)(d))[3] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  (( size == 4 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
			    ((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[3] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		   (( size == 5 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], 5 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )
		



	#define RWF_PUT_LENSPEC_NUMERIC64(d,f,s)\
		( (*((rtrUInt8*)f) & 0x20) ? \
		   ( ((char*)(d))[0] = 0, 1 \
		   ) : \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(128) ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(8388608) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(549755813888) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(36028797018963968) ) ? \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], \
			  ((char*)(d))[8] = ((char*)(s))[7], 9 \
			) : \
			( ((char*)(d))[0] = 9, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], \
			  ((char*)(d))[8] = ((char*)(s))[6], \
			  ((char*)(d))[9] = ((char*)(s))[7], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt64*)s) >= RTR_LL(-128) ) ? \
			( ((char*)(d))[0] = 2, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = 3, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[6], \
			  ((char*)(d))[3] = ((char*)(s))[7], 4 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-8388608) ) ? \
			( ((char*)(d))[0] = 4, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((char*)(d))[0] = 5, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], 6 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-549755813888) ) ? \
			( ((char*)(d))[0] = 6, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((char*)(d))[0] = 7, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-36028797018963968) ) ? \
			( ((char*)(d))[0] = 8, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], \
			  ((char*)(d))[8] = ((char*)(s))[7], 9 \
			) : \
			( ((char*)(d))[0] = 9, \
			  ((char*)(d))[1] = ((char*)(f))[0], \
			  ((char*)(d))[2] = ((char*)(s))[0], \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], \
			  ((char*)(d))[8] = ((char*)(s))[6], \
			  ((char*)(d))[9] = ((char*)(s))[7], 10 \
			)  \
            ) \
			) \
			) \
		   )\
		  ) \
		 ) \
		)  \
	   ) \
	  )
	
  

 	#define RWF_GET_LENSPEC_NUMERIC64(d,f,s) \
		( ( (unsigned char)((char*)(s))[0] == 0 )  ? \
			( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = 0, 1 \
			) : \
		 (( (unsigned char)((char*)(s))[0] == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
			    ((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[7] = ((char*)(s))[2], 3 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = 0, \
			    ((char*)(d))[6] = 0, \
			    ((char*)(d))[7] = ((char*)(s))[2], 3 \
			  ) \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[7] = ((char*)(s))[3], 4 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[1], \
				 ((char*)(d))[0] = 0, \
				 ((char*)(d))[1] = 0, \
				 ((char*)(d))[2] = 0, \
				 ((char*)(d))[3] = 0, \
				 ((char*)(d))[4] = 0, \
			     ((char*)(d))[5] = 0, \
			     ((char*)(d))[6] = ((char*)(s))[2], \
			     ((char*)(d))[7] = ((char*)(s))[3], 4 \
			   ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 4 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[3], \
				((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = ((char*)(s))[2], \
			    ((char*)(d))[6] = ((char*)(s))[3], \
			    ((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 5 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[6] = ((char*)(s))[4], \
				((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
			    ((char*)(d))[6] = ((char*)(s))[4], \
			    ((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 6 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
				((char*)(d))[6] = ((char*)(s))[5], \
				((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
			    ((char*)(d))[6] = ((char*)(s))[5], \
			    ((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 7 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
			    ((char*)(d))[5] = ((char*)(s))[5], \
				((char*)(d))[6] = ((char*)(s))[6], \
				((char*)(d))[7] = ((char*)(s))[7], 8 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
			    ((char*)(d))[5] = ((char*)(s))[5], \
			    ((char*)(d))[6] = ((char*)(s))[6], \
			    ((char*)(d))[7] = ((char*)(s))[7], 8 \
			  ) \
			) : \
		  ( ( (unsigned char)((char*)(s))[0] == 8 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((unsigned char*)(d))[0] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
			    ((char*)(d))[5] = ((char*)(s))[6], \
				((char*)(d))[6] = ((char*)(s))[7], \
				((char*)(d))[7] = ((char*)(s))[8], 9 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[1], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], \
				((char*)(d))[3] = ((char*)(s))[4], \
				((char*)(d))[4] = ((char*)(s))[5], \
			    ((char*)(d))[5] = ((char*)(s))[6], \
			    ((char*)(d))[6] = ((char*)(s))[7], \
			    ((char*)(d))[7] = ((char*)(s))[8], 9 \
			  ) \
			) : \
		   (( (unsigned char)((char*)(s))[0] == 9 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[1], \
			  ((char*)(d))[0] = ((char*)(s))[2], \
			  ((char*)(d))[1] = ((char*)(s))[3], \
			  ((char*)(d))[2] = ((char*)(s))[4], \
			  ((char*)(d))[3] = ((char*)(s))[5], \
			  ((char*)(d))[4] = ((char*)(s))[6], \
			  ((char*)(d))[5] = ((char*)(s))[7], \
			  ((char*)(d))[6] = ((char*)(s))[8], \
			  ((char*)(d))[7] = ((char*)(s))[9], 10 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
	   )		




	#define RWF_GET_LENSPEC_NUMERIC64_SIZE(d,f,s,size) \
		 (( size == 0 )  ? \
			( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = 0, 0 \
			) : \
		 (( size == 2 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((unsigned char*)(d))[6] = 0xFF, \
				((char*)(d))[7] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = 0, \
			    ((char*)(d))[6] = 0, \
			    ((char*)(d))[7] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 (( size == 3 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[7] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[0], \
				 ((char*)(d))[0] = 0, \
				 ((char*)(d))[1] = 0, \
				 ((char*)(d))[2] = 0, \
				 ((char*)(d))[3] = 0, \
				 ((char*)(d))[4] = 0, \
			     ((char*)(d))[5] = 0, \
			     ((char*)(d))[6] = ((char*)(s))[1], \
			     ((char*)(d))[7] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  (( size == 4 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((char*)(d))[5] = ((char*)(s))[1], \
				((char*)(d))[6] = ((char*)(s))[2], \
				((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = 0, \
			    ((char*)(d))[5] = ((char*)(s))[1], \
			    ((char*)(d))[6] = ((char*)(s))[2], \
			    ((char*)(d))[7] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		  (( size == 5 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[3], \
				((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
			    ((char*)(d))[6] = ((char*)(s))[3], \
			    ((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  (( size == 6 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
				((char*)(d))[6] = ((char*)(s))[4], \
				((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = ((char*)(s))[1], \
				((char*)(d))[4] = ((char*)(s))[2], \
			    ((char*)(d))[5] = ((char*)(s))[3], \
			    ((char*)(d))[6] = ((char*)(s))[4], \
			    ((char*)(d))[7] = ((char*)(s))[5], 6 \
			  ) \
			) : \
		  (( size == 7 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
				((char*)(d))[6] = ((char*)(s))[5], \
				((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
			    ((char*)(d))[6] = ((char*)(s))[5], \
			    ((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		  (( size == 8 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
			    ((char*)(d))[5] = ((char*)(s))[5], \
				((char*)(d))[6] = ((char*)(s))[6], \
				((char*)(d))[7] = ((char*)(s))[7], 8 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
			    ((char*)(d))[5] = ((char*)(s))[5], \
			    ((char*)(d))[6] = ((char*)(s))[6], \
			    ((char*)(d))[7] = ((char*)(s))[7], 8 \
			  ) \
			) : \
		   (( size == 9 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], \
			  ((char*)(d))[7] = ((char*)(s))[8], 9 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
		) \
		) \
		) \
		) \
	   )	

/* Reserved Bit Numerics */
/* These macros write numeric types as reserved bit.  The top two bits
   of the format byte are reserved for length.  On the 32 bit values, 
   0x00 = 1 byte value, 0x40 = 2 byte value, 0x80 = 3 byte value, 0xC0 = 
   4 byte value.  For 64 bit values 0x00 = 2 byte value, 0x40 = 4 byte value, 
   0x80 = 6 byte value, and 0xC0 = 8 byte value.  The values are encoded
   as a signed integer value */
#define RWF_PUT_RESBIT_NUMERIC32(d,f,s)\
	 ( (*((rtrUInt8*)f) == 0x20) ? \
		   ( ((char*)(d))[0] = 0x20, 1 \
		   ) : \
		 ( ( *((rtrInt32*)s) > 0 )  ? \
		  ( ( *((rtrInt32*)s) < 128 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x00), \
			  ((char*)(d))[1] = ((char*)(s))[3], 2 \
			) : \
		  ( ( *((rtrInt32*)s) < 32768 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x40), \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], 3 \
			) : \
		  ( ( *((rtrInt32*)s) < 8388608 ) ? \
			( ((unsigned char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x80), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0xC0), \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], 5 \
			) \
		  ) \
		 ) \
		) : \
		 ( ( *((rtrInt32*)s) >= -128 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x00), \
			  ((char*)(d))[1] = ((char*)(s))[3], 2 ) : \
		  ( ( *((rtrInt32*)s) >= -32768 ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x40), \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], 3 ) : \
		  ( ( *((rtrInt32*)s) >= -8388608 ) ? \
			( ((unsigned char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x80), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0xC0), \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], 5 \
			) \
		  ) \
		 ) \
		) \
	  ) \
	 )





	#define RWF_GET_RESBIT_NUMERIC32(d,f,s) \
	( ( (unsigned char)((char*)(s))[0] == 0x20 )  ? \
		 	( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, 1 \
			) : \
		 (( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x00 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
                ((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((char*)(d))[3] = ((char*)(s))[1], 2 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = 0, \
			    ((char*)(d))[2] = 0, \
			    ((char*)(d))[3] = ((char*)(s))[1], 2 \
			  ) \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x40 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
			    ((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[0], \
				 ((char*)(d))[0] = 0, \
			     ((char*)(d))[1] = 0, \
			     ((char*)(d))[2] = ((char*)(s))[1], \
			     ((char*)(d))[3] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x80 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
			    ((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], 4 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
			    ((char*)(d))[1] = ((char*)(s))[1], \
			    ((char*)(d))[2] = ((char*)(s))[2], \
			    ((char*)(d))[3] = ((char*)(s))[3], 4 \
			  ) \
			) : \
		   (( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0xC0 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], 5 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )
	



#define RWF_PUT_RESBIT_NUMERIC64(d,f,s)\
		( (*((rtrUInt8*)f) == 0x20) ? \
		   ( ((char*)(d))[0] = 0x20, 1 \
		   ) : \
		( ( *((rtrInt64*)s) > 0 )  ? \
		  ( ( *((rtrInt64*)s) < RTR_LL(32768) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x00), \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(2147483648) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x40), \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) < RTR_LL(140737488355328) ) ? \
			( ((unsigned char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x80), \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0xC0), \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], \
			  ((char*)(d))[8] = ((char*)(s))[7], 9 \
		    ) \
		   ) \
		 ) \
		) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-32768) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x00), \
			  ((char*)(d))[1] = ((char*)(s))[6], \
			  ((char*)(d))[2] = ((char*)(s))[7], 3 ) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-2147483648) ) ? \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x40), \
			  ((char*)(d))[1] = ((char*)(s))[4], \
			  ((char*)(d))[2] = ((char*)(s))[5], \
			  ((char*)(d))[3] = ((char*)(s))[6], \
			  ((char*)(d))[4] = ((char*)(s))[7], 5 \
			) : \
		  ( ( *((rtrInt64*)s) >= RTR_LL(-140737488355328) ) ? \
			( ((unsigned char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0x80), \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], 7 \
			) : \
			( ((char*)(d))[0] = (((char*)(f))[0] | (rtrUInt8)0xC0), \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], \
			  ((char*)(d))[5] = ((char*)(s))[4], \
			  ((char*)(d))[6] = ((char*)(s))[5], \
			  ((char*)(d))[7] = ((char*)(s))[6], \
			  ((char*)(d))[8] = ((char*)(s))[7], 9 \
		   )\
		  ) \
		 ) \
		)  \
	   ) \
	  )
	




	#define RWF_GET_RESBIT_NUMERIC64(d,f,s) \
		( ( (unsigned char)((char*)(s))[0] == 0x20 )  ? \
			( ((char*)(f))[0] = 0x20, \
			  ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = 0, \
			  ((char*)(d))[7] = 0, 1 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x00 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((unsigned char*)(d))[4] = 0xFF, \
			    ((unsigned char*)(d))[5] = 0xFF, \
				((char*)(d))[6] = ((char*)(s))[1], \
				((char*)(d))[7] = ((char*)(s))[2], 3 \
			   ) : \
		 	   ( ((char*)(f))[0] = ((char*)(s))[0], \
				 ((char*)(d))[0] = 0, \
				 ((char*)(d))[1] = 0, \
				 ((char*)(d))[2] = 0, \
				 ((char*)(d))[3] = 0, \
				 ((char*)(d))[4] = 0, \
			     ((char*)(d))[5] = 0, \
			     ((char*)(d))[6] = ((char*)(s))[1], \
			     ((char*)(d))[7] = ((char*)(s))[2], 3 \
			   ) \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x40 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((unsigned char*)(d))[2] = 0xFF, \
				((unsigned char*)(d))[3] = 0xFF, \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
				((char*)(d))[6] = ((char*)(s))[3], \
				((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = 0, \
				((char*)(d))[3] = 0, \
				((char*)(d))[4] = ((char*)(s))[1], \
			    ((char*)(d))[5] = ((char*)(s))[2], \
			    ((char*)(d))[6] = ((char*)(s))[3], \
			    ((char*)(d))[7] = ((char*)(s))[4], 5 \
			  ) \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x80 )  ? \
			( (((unsigned char*)(s))[1] >= 0x80) ? \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((unsigned char*)(d))[0] = 0xFF, \
				((unsigned char*)(d))[1] = 0xFF, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
				((char*)(d))[6] = ((char*)(s))[5], \
				((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) : \
			  ( ((char*)(f))[0] = ((char*)(s))[0], \
				((char*)(d))[0] = 0, \
				((char*)(d))[1] = 0, \
				((char*)(d))[2] = ((char*)(s))[1], \
				((char*)(d))[3] = ((char*)(s))[2], \
				((char*)(d))[4] = ((char*)(s))[3], \
			    ((char*)(d))[5] = ((char*)(s))[4], \
			    ((char*)(d))[6] = ((char*)(s))[5], \
			    ((char*)(d))[7] = ((char*)(s))[6], 7 \
			  ) \
			) : \
		   (( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0xC0 ) ? \
			( ((char*)(f))[0] = ((char*)(s))[0], \
			  ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], \
			  ((char*)(d))[4] = ((char*)(s))[5], \
			  ((char*)(d))[5] = ((char*)(s))[6], \
			  ((char*)(d))[6] = ((char*)(s))[7], \
			  ((char*)(d))[7] = ((char*)(s))[8], 9 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		) \
	   )
		


/* Macros used by TRWF */

	#define RWF_PUT_24(d,s) \
		(((char*)(d))[0] = ((char*)(s))[1],\
		 ((char*)(d))[1] = ((char*)(s))[2],\
		 ((char*)(d))[2] = ((char*)(s))[3], 3 )
		 

	#define RWF_GET_24(d,s) \
		( ((char*)(d))[0] = 0, \
		  ((char*)(d))[1] = ((char*)(s))[0], \
		  ((char*)(d))[2] = ((char*)(s))[1], \
		  ((char*)(d))[3] = ((char*)(s))[2], 3 )

	#define RWF_PUT_40(d,s) \
		(((char*)(d))[0] = ((char*)(s))[3],\
		 ((char*)(d))[1] = ((char*)(s))[4],\
		 ((char*)(d))[2] = ((char*)(s))[5],\
		 ((char*)(d))[3] = ((char*)(s))[6],\
		 ((char*)(d))[4] = ((char*)(s))[7], 5 )

	#define RWF_GET_40(d,s) \
		( ((char*)(d))[0] = 0, \
		  ((char*)(d))[1] = 0, \
		  ((char*)(d))[2] = 0, \
		  ((char*)(d))[3] = ((char*)(s))[0], \
		  ((char*)(d))[4] = ((char*)(s))[1], \
		  ((char*)(d))[5] = ((char*)(s))[2], \
		  ((char*)(d))[6] = ((char*)(s))[3], \
		  ((char*)(d))[7] = ((char*)(s))[4], 5 )

	#define RWF_PUT_48(d,s) \
		(((char*)(d))[0] = ((char*)(s))[2],\
		 ((char*)(d))[1] = ((char*)(s))[3],\
		 ((char*)(d))[2] = ((char*)(s))[4],\
		 ((char*)(d))[3] = ((char*)(s))[5],\
		 ((char*)(d))[4] = ((char*)(s))[6],\
		 ((char*)(d))[5] = ((char*)(s))[7], 6 )

	#define RWF_GET_48(d,s) \
		 (((char*)(d))[0] = 0, \
		  ((char*)(d))[1] = 0, \
		  ((char*)(d))[2] = ((char*)(s))[0], \
		  ((char*)(d))[3] = ((char*)(s))[1], \
		  ((char*)(d))[4] = ((char*)(s))[2], \
		  ((char*)(d))[5] = ((char*)(s))[3], \
		  ((char*)(d))[6] = ((char*)(s))[4], \
		  ((char*)(d))[7] = ((char*)(s))[5], 6 )


	#define RWF_PUT_56(d,s) \
	    (((char*)(d))[0] = ((char*)(s))[1],\
		 ((char*)(d))[1] = ((char*)(s))[2],\
		 ((char*)(d))[2] = ((char*)(s))[3],\
		 ((char*)(d))[3] = ((char*)(s))[4],\
		 ((char*)(d))[4] = ((char*)(s))[5],\
		 ((char*)(d))[5] = ((char*)(s))[6],\
		 ((char*)(d))[6] = ((char*)(s))[7], 7 )

	#define RWF_GET_56(d,s) \
		 (((char*)(d))[0] = 0, \
		  ((char*)(d))[1] = ((char*)(s))[0], \
		  ((char*)(d))[2] = ((char*)(s))[1], \
		  ((char*)(d))[3] = ((char*)(s))[2], \
		  ((char*)(d))[4] = ((char*)(s))[3], \
		  ((char*)(d))[5] = ((char*)(s))[4], \
		  ((char*)(d))[6] = ((char*)(s))[5], \
		  ((char*)(d))[7] = ((char*)(s))[6], 7 )


	#define RWF_GET_16_AS_32(d,s) \
		( ((char*)(d))[0] = 0, \
		  ((char*)(d))[1] = 0, \
		  ((char*)(d))[2] = ((char*)(s))[0], \
		  ((char*)(d))[3] = ((char*)(s))[1], 2 )

/* END TRWF */
	

	/* These macros implement a reserved bit strategy. They allow
	 * for a maximum unsigned 15 bit value to be represented. If
	 * the high order bit (0x80) is set in the first byte, then the
	 * second byte is part of the value. The high order bit is
	 * not part of the value.
	 */
	#define RWF_PUT_RESBIT_U15(d,s) \
		( ( *((rtrUInt16*)s) < (rtrUInt16)0x80 )  ? \
			( ((char*)(d))[0] = ((char*)(s))[1], 1 ) : \
		 ( ( *((rtrUInt16*)s) < (rtrUInt16)0x8000 )  ? \
			( ((unsigned char*)(d))[0] = (((char*)(s))[0] | (rtrUInt8)0x80), \
			  ((char*)(d))[1] = ((char*)(s))[1], 2 \
			) : -1 \
		 ) \
		)

	#define RWF_GET_RESBIT_U15(d,s) \
		( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x80 )  ? \
			( ((char*)(d))[0] = (((char*)(s))[0] & 0x7F), \
			  ((char*)(d))[1] = ((char*)(s))[1], 2 \
			) : \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], 1 \
			) \
		)


	#define RWF_GET_RESBIT_U15_AS32(d,s) \
		( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x80 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = (((char*)(s))[0] & 0x7F), \
			  ((char*)(d))[3] = ((char*)(s))[1], 2 ) : \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[0], 1 ) )



	/* resbit 15 signed - 14 bits of precision plus sign extension */
	#define RWF_PUT_RESBIT_I15(d,s) \
		( ( ( *((rtrInt16*)s) >= (rtrInt16) -0x40 ) && \
		    ( *((rtrInt16*)s) < (rtrInt16) 0x40 ) ) ? \
			( ((char*)(d))[0] = ((char*)(s))[1] & (rtrUInt8)0x7F, 1 ) : \
		  ( ( ( *((rtrInt16*)s) >= (rtrInt16) -0x4000 ) && \
		      ( *((rtrInt16*)s) < (rtrInt16) 0x4000 ) )  ? \
			( ((unsigned char*)(d))[0] = (((char*)(s))[0] | (rtrUInt8)0x80), \
			  ( (char*)(d))[1] = ((char*)(s))[1], 2 \
			) : -1 \
		  ) \
		)

	#define RWF_GET_RESBIT_I15(d,s) \
		( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x80 )  ? \
			( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x40 ) ? \
				( ((char*)(d))[0] = ((char*)(s))[0], \
				  ((char*)(d))[1] = ((char*)(s))[1], 2 \
				) : \
				( ((char*)(d))[0] = (((char*)(s))[0] & (rtrUInt8)0x7F), \
				  ((char*)(d))[1] = ((char*)(s))[1], 2 \
				) \
			) : \
			( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x40 ) ? \
				( ((unsigned char*)(d))[0] = ((char) 0xFF), \
				  ((unsigned char*)(d))[1] = (((char*)(s))[0] | (rtrUInt8)0x80), 1 \
				) : \
				( ((char*)(d))[0] = 0, \
				  ((char*)(d))[1] = ((char*)(s))[0], 1 \
				) \
			) \
		)

	#define RWF_GET_RESBIT_I15_AS32(d,s) \
		( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x80 )  ? \
			( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x40 ) ? \
				( ((unsigned char*)(d))[0] = ((char)0xFF), \
				  ((unsigned char*)(d))[1] = ((char)0xFF), \
				  ((char*)(d))[2] = ((char*)(s))[0], \
				  ((char*)(d))[3] = ((char*)(s))[1], 2 \
				) : \
				( ((char*)(d))[0] = 0, \
				  ((char*)(d))[1] = 0, \
				  ((char*)(d))[2] = (((char*)(s))[0] & (rtrUInt8)0x7F), \
				  ((char*)(d))[3] = ((char*)(s))[1], 2 \
				) \
			) : \
			( ( (unsigned char)((char*)(s))[0] & (rtrUInt8)0x40 ) ? \
				( ((unsigned char*)(d))[0] = ((char) 0xFF), \
				  ((unsigned char*)(d))[1] = ((char) 0xFF), \
				  ((unsigned char*)(d))[2] = ((char) 0xFF), \
				  ((unsigned char*)(d))[3] = (((char*)(s))[0] | (rtrUInt8)0x80), 1 \
				) : \
				( ((char*)(d))[0] = 0, \
				  ((char*)(d))[1] = 0, \
				  ((char*)(d))[2] = 0, \
				  ((char*)(d))[3] = ((char*)(s))[0], 1 \
				) \
			) \
		)



	/* These macros implement a reserved bits strategy. They allow
	 * for a maximum unsigned 22 bit value to be represented. The
	 * high order 1 or 2 bits are used to represent the number of bytes
	 * in the value. High order order 1 or 2 bits are not part of the value.
	 * 0x00 - 1 byte (7 bits) (uses top bit as 0 - 0x0)
	 * 0x80 - 2 bytes (14 bits) (uses top two bits as 10 - 0x80)
	 * 0x40 - 3 bytes (22 bits) (uses top two bits as 11 - 0xC0)
	 */
	#define RWF_PUT_RESBIT_U22(d,s) \
		( ( *((rtrUInt32*)s) < 0x80 )  ? \
			( ((char*)(d))[0] = ((char*)(s))[3], 1 ) : \
		 ( ( *((rtrUInt32*)s) < 0x4000 ) ? \
			( ((unsigned char*)(d))[0] = (((char*)(s))[2] | (rtrUInt8)0x80), \
				((char*)(d))[1] = ((char*)(s))[3], 2 \
			) : \
		  ( ( *((rtrUInt32*)s) < 0x400000 ) ? \
			( ((char*)(d))[0] = (((char*)(s))[1] | (rtrUInt8)0xC0), \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], 3 \
			) : -1 \
		   ) \
		  ) \
		 )

	#define RWF_GET_RESBIT_U22(d,s) \
		(( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0x80) == (rtrUInt8)0x00 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[0], 1 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x80 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[3] = ((char*)(s))[1], 2 \
			) : \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], 3 \
			)\
		  ) \
		  )

	/* ResBitU31 -- 
	 * 0x00 - 3 bytes(23 bits)
	 * 0x80 - 4 bytes(31 bits) 
	 */
	#define RWF_PUT_RESBIT_U31(d,s) \
		( ( *((rtrUInt32*)s) < 0x800000 ) ? \
			(   ((char*)(d))[0] = ((char*)(s))[1], \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], 3 \
		) : ( ( *((rtrUInt32*)s) < 0x80000000 ) ? \
			(   ((unsigned char*)(d))[0] = (((char*)(s))[0]  | (rtrUInt8)0x80), \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], 4 \
		) : -1 ) )
		

	#define RWF_GET_RESBIT_U31(d,s) \
		( ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0x80))  ? \
		 	( ((char*)(d))[0] = (((char*)(s))[0] & (rtrUInt8)0x7F), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], 3 \
			)\
		  ) \
		)


/* ResBitI30
 * 0x80 is the resBit(1 => 30b, 0 => 22b), 0x40 is a sign bit 
 * Used by TRWF */

#define RWF_PUT_RESBIT_I30(d,s) \
	( \
	  ( *((rtrInt32*)s) < RWF_MIN_I30 || *((rtrInt32*)s) > RWF_MAX_I30) ?  -1 : /* Invalid */ \
	  ( \
		*((rtrInt32*)s) < (rtrInt32)0xFFC00000 || *((rtrInt32*)s) > (rtrInt32)0x003FFFFF) ? \
	  ( /* 30b */ \
		((unsigned char*)(d))[0] = ((char*)(s))[0] | (rtrInt8)0x80, /* Add resBit */ \
		((char*)(d))[1] = ((char*)(s))[1], \
		((char*)(d))[2] = ((char*)(s))[2], \
		((char*)(d))[3] = ((char*)(s))[3], 4 \
	  ) \
	  : \
	  ( /* 22b */ \
		((char*)(d))[0] = ((char*)(s))[1] & (rtrInt8)~0x80, /* Take off resBit */\
		((char*)(d))[1] = ((char*)(s))[2], \
		((char*)(d))[2] = ((char*)(s))[3], 3  \
	  ) \
	)

#define RWF_GET_RESBIT_I30(d,s) \
	( \
	  ( *((rtrInt8*)s) & (rtrInt8)0x80 ) ? \
	  ( /* 30b */ \
		( \
		  (((char*)(s))[0] & (rtrInt8)0x40) ? \
		  (((char*)(d))[0] = ((char*)(s))[0] | (rtrInt8)0xC0) /* Negative, sign extend */\
		  :\
		  (((char*)(d))[0] = ((char*)(s))[0] & (rtrInt8)~0x80) /* Positive, take off resBit */\
		), \
		((char*)(d))[1] = ((char*)(s))[1], \
		((char*)(d))[2] = ((char*)(s))[2], \
		((char*)(d))[3] = ((char*)(s))[3], 4 \
	  ) \
	  : \
	  ( /* 20b */ \
		( (((char*)(s))[0] & (rtrInt8)0x40) ? \
		  (\
		   ((unsigned char*)(d))[0] = 0xFF, \
		   ((char*)(d))[1] = ((char*)(s))[0] | (rtrInt8)0xC0 /* Negative, sign extend */\
		  )\
		  :\
		  (\
		   ((char*)(d))[0] = 0, \
		   ((char*)(d))[1] = ((char*)(s))[0] /* Positive, resBit already ok */\
		  )\
		), \
		((char*)(d))[2] = ((char*)(s))[1], \
		((char*)(d))[3] = ((char*)(s))[2], 3 \
	  ) \
) 

/* Replaces the value in the current position, so long as the new value is a valid fit 
 * -- returns -1 if invalid, 0 if the new value doesn't fit */
#define RWF_REPLACE_RESBIT_I30(d,s) \
	(\
	 ( *((rtrInt32*)s) < RWF_MIN_I30 || *((rtrInt32*)s) > RWF_MAX_I30) ?  -1 : /* Invalid */ \
	 (\
	  (((char*)(d))[0] & (rtrInt8)0x80) ? /* Check size of current value */ \
	  ( /* 30b can fit any valid value */ \
		((unsigned char*)(d))[0] = ((char*)(s))[0] | (rtrInt8)0x80, /* Add resBit */ \
		((char*)(d))[1] = ((char*)(s))[1], \
		((char*)(d))[2] = ((char*)(s))[2], \
		((char*)(d))[3] = ((char*)(s))[3], 4 \
	  )\
	  :\
	  ( /*22b can only fit other 22b values */\
		(*((rtrInt32*)s)< (rtrInt32)0xFFC00000 || *((rtrInt32*)s)> (rtrInt32)0x003FFFFF) ? 0 : /* Too big */ \
		(\
		 ((char*)(d))[0] = ((char*)(s))[1] & (rtrInt8)~0x80, /* Take off resBit */ \
		 ((char*)(d))[1] = ((char*)(s))[2], \
		 ((char*)(d))[2] = ((char*)(s))[3], 3 \
		)\
	  )\
	 )\
	)


	/* These macros implement a reserved bits strategy. They allow
	 * for a maximum unsigned 30 bit value to be represented. The
	 * high order 2 bits are used to represent the number of bytes
	 * in the value. High order order 2 bits are not part of the value.
	 * 0x00 - 1 byte (6 bits)
 	 * 0x80 - 2 bytes (14 bits)
	 * 0x40 - 3 bytes (22 bits)
	 */
	#define RWF_PUT_RESBIT_U30(d,s) \
		( ( *((rtrUInt32*)s) < 0x40 )  ? \
			( ((char*)(d))[0] = ((char*)(s))[3], 1 ) : \
		 ( ( *((rtrUInt32*)s) < 0x4000 ) ? \
			( ((unsigned char*)(d))[0] = (((char*)(s))[2] | (rtrUInt8)0x80), \
				((char*)(d))[1] = ((char*)(s))[3], 2 \
			) : \
		  ( ( *((rtrUInt32*)s) < 0x400000 ) ? \
			( ((char*)(d))[0] = (((char*)(s))[1] | (rtrUInt8)0x40), \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], 3 \
			) : \
		   ( ( *((rtrUInt32*)s) < 0x40000000 ) ? \
			( ((char*)(d))[0] = (((char*)(s))[0] | (rtrUInt8)0xC0), \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : -1 \
		   ) \
		  ) \
		 ) \
		)

	#define RWF_GET_RESBIT_U30(d,s) \
		(( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x00 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[0], 1 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x80 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[3] = ((char*)(s))[1], 2 \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x40 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], 3 \
			) : \
			( ((char*)(d))[0] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			)\
		  ) \
		 ) \
		)


/* These macros implement a reserved bits strategy. They allow
	 * for a maximum unsigned 27 bit value to be represented. The
	 * 5th & 6th bits are used to represent the number of bytes
	 * in the value. The 2 bits are not part of the value.
	 * 0x10 - 1 byte (8 bits)
	 * 0x20 - 2 bytes (16 bits)
     * 0x30 - 3B+3b (27 bits)
	 *
     * Format:
     *   ffrr0sss	f = format bits
	 *   ssssssss	r = reserved bits
	 *   ssssssss	s = source bits
	 *   ssssssss
	 * The top three value bits are ONLY used in the biggest case. Otherwise they are set to 0.
	 */

	#define RWF_PUT_RESBIT_U27(d,s,f) \
		 ( ( *((rtrUInt32*)s) <= 0xFF ) ? \
			( ((char*)(d))[0] = (rtrUInt8)0x10 | ((rtrUInt8)(f << 6) & 0xC0), \
				((char*)(d))[1] = ((char*)(s))[3], 2 \
			) : \
		  ( ( *((rtrUInt32*)s) <= 0xFFFF ) ? \
			( ((char*)(d))[0] = (rtrUInt8)0x20 | ((rtrUInt8)(f << 6) & 0xC0), \
				((char*)(d))[1] = ((char*)(s))[2], \
				((char*)(d))[2] = ((char*)(s))[3], 3 \
			) : \
		   ( ( *((rtrUInt32*)s) <= 0x7FFFFFF ) ? \
			( ((char*)(d))[0] = ((((char*)(s))[0] & 0x7) | (rtrUInt8)0x30 | ((rtrUInt8)(f << 6) & 0xC0)), \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], 4 \
			) : -1 \
		   )))

	// Currently does not include the format bits as they are not needed when this is used.
	// Be wary of changing that behavior.
	#define RWF_GET_RESBIT_U27(d,s) \
		(( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0x30) == (rtrUInt8)0x10 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[1], 2 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0x30) == (rtrUInt8)0x20 ) ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], 3 \
			) : \
			( ((char*)(d))[0] = (((char*)(s))[0] & 0x7), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], 4 \
			) \
			) \
			)

	/* These macros implement a reserved bits strategy. They allow
	 * for a maximum unsigned 62 bit value to be represented. The
	 * high order 2 bits are used to represent the number of bytes
	 * in the value. High order order 2 bits are not part of the value.
	 * 0x00 - 2 byte (14 bits)
	 * 0x40 - 4 bytes (30 bits)
	 * 0x80 - 6 bytes (46 bits)
	 * 0xC0 - 8 bytes (62 bits)
	 */
		#define RWF_PUT_RESBIT_U62(d,s) \
		( ( *((rtrUInt64*)s) < RTR_ULL(0x4000) )  ? \
			(	((char*)(d))[0] = ((char*)(s))[6], \
				((char*)(d))[1] = ((char*)(s))[7], 2  \
			) : \
		 ( ( *((rtrUInt64*)s) < RTR_ULL(0x40000000) ) ? \
			(	((char*)(d))[0] = (((char*)(s))[4] | (rtrUInt8)0x40), \
				((char*)(d))[1] = ((char*)(s))[5], \
				((char*)(d))[2] = ((char*)(s))[6], \
				((char*)(d))[3] = ((char*)(s))[7], 4  \
			) : \
		  ( ( *((rtrUInt64*)s) < RTR_ULL(0x400000000000) ) ? \
			(	((char*)(d))[0] = (((char*)(s))[2] | (rtrUInt8)0x80), \
				((char*)(d))[1] = ((char*)(s))[3], \
				((char*)(d))[2] = ((char*)(s))[4], \
				((char*)(d))[3] = ((char*)(s))[5], \
				((char*)(d))[4] = ((char*)(s))[6], \
				((char*)(d))[5] = ((char*)(s))[7], 6  \
			) : \
		   ( ( *((rtrUInt64*)s) < RTR_ULL(0x4000000000000000) ) ? \
			(	((char*)(d))[0] = (((char*)(s))[0] | (rtrUInt8)0xC0), \
				((char*)(d))[1] = ((char*)(s))[1], \
				((char*)(d))[2] = ((char*)(s))[2], \
				((char*)(d))[3] = ((char*)(s))[3], \
				((char*)(d))[4] = ((char*)(s))[4], \
				((char*)(d))[5] = ((char*)(s))[5], \
				((char*)(d))[6] = ((char*)(s))[6], \
				((char*)(d))[7] = ((char*)(s))[7], 8  \
			) : -1 \
		   ) \
		  ) \
		 ) \
		)

	#define RWF_GET_RESBIT_U62(d,s) \
		(( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x00 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = 0, \
			  ((char*)(d))[5] = 0, \
			  ((char*)(d))[6] = ((char*)(s))[0], \
			  ((char*)(d))[7] = ((char*)(s))[1], 2 \
			) : \
		 ( ( (unsigned char)(((char*)(s))[0] & (rtrUInt8)0xC0) == (rtrUInt8)0x40 )  ? \
		 	( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = 0, \
			  ((char*)(d))[4] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[5] = ((char*)(s))[1], \
			  ((char*)(d))[6] = ((char*)(s))[2], \
			  ((char*)(d))[7] = ((char*)(s))[3], 4 \
			) : \
		  ( ( (unsigned char)(((char*)(s))[0] & 0xC0) == (rtrUInt8)0x80 )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[3] = ((char*)(s))[1], \
			  ((char*)(d))[4] = ((char*)(s))[2], \
			  ((char*)(d))[5] = ((char*)(s))[3], \
			  ((char*)(d))[6] = ((char*)(s))[4], \
			  ((char*)(d))[7] = ((char*)(s))[5], 6 \
			) : \
			( ((char*)(d))[0] = (((char*)(s))[0] & (rtrUInt8)0x3F), \
			  ((char*)(d))[1] = ((char*)(s))[1], \
			  ((char*)(d))[2] = ((char*)(s))[2], \
			  ((char*)(d))[3] = ((char*)(s))[3], \
			  ((char*)(d))[4] = ((char*)(s))[4], \
			  ((char*)(d))[5] = ((char*)(s))[5], \
			  ((char*)(d))[6] = ((char*)(s))[6], \
			  ((char*)(d))[7] = ((char*)(s))[7], 8 \
			)\
		  ) \
		 ) \
		)






	/* These macros implement a reserved value scalar optimization
	 * strategy. The values are always represented by a minimum
	 * number of byte(s). If the values of these byte(s) is some
	 * reserved value(s), then there are more bytes that represent
	 * the value.
	 */

	/* These macros implement an optmized byte, with max u16 reserved
	 * value stragety. There is always one byte, if the value of this
	 * byte tells you how to interpret the value.
	 *    byte value < 0xFE - The byte is the value.
	 *    byte value = 0xFE - Ignore the first byte, the next two bytes
	 *                        are the value.
	 * Ex:
	 *   0xFD = 0xFD = 253
	 *   0xFE00FF = 0x00FF = 255
	 */
	#define RWF_PUT_OPTBYTE_U16(d,s) \
		( ( *((rtrUInt16*)s) < 0xFE )  ? \
			( ( (char*)(d))[0] = ((char*)(s))[1], 1 ) : \
			  ( ((unsigned char*)(d))[0] = 0xFE, \
			    ((char*)(d))[1] = ((char*)(s))[0], \
			    ((char*)(d))[2] = ((char*)(s))[1], 3 ) )

	#define RWF_GET_OPTBYTE_U16(d,s) \
		( ( (unsigned char)((char*)(s))[0] < (rtrUInt8)0xFE )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = ((char*)(s))[0], 1 ) : \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], 3 ) )

	/* These macros implement an optmized byte, with max u32 reserved
	 * value stragety. There is always one byte, if the value of this
	 * byte tells you how to interpret the value.
	 *    byte value < 0xFE - The byte is the value.
	 *    byte value = 0xFE - Ignroe the first byte, the next two bytes
	 *                        are the value.
	 *    byte value = 0xFF - Ignore the first byte, the next four bytes
	 *                        are the value.
	 * Ex:
	 *   0xFD = 0xFD - 253
	 *   0xFE00FE = 0x00FE - 254
	 *   0xFEFFFF = 0xFFFF - 65535
	 *   0xFF00010000 = 0x00010000 - 65536
	 */
	#define RWF_PUT_OPTBYTE_U32(d,s) \
		( ( *((rtrUInt32*)s) < 0xFE )  ? \
			( ( (char*)(d))[0] = ((char*)(s))[3], 1 ) : \
		 ( ( *((rtrUInt32*)s) <= 0xFFFF )  ? \
		 	( ((unsigned char*)(d))[0] = (rtrUInt8)0xFE, \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], 3 ) : \
			( ((unsigned char*)(d))[0] = (rtrUInt8)0xFF, \
			  ((char*)(d))[1] = ((char*)(s))[0], \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], \
			  ((char*)(d))[4] = ((char*)(s))[3], 5 \
			) \
		 ) \
		)

	#define RWF_GET_OPTBYTE_U32(d,s) \
		( ( (unsigned char)((char*)(s))[0] < 0xFE )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[0], 1 \
			) : \
		 ( ( (unsigned char)((char*)(s))[0] == 0xFE ) ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], 3 \
			) : \
			( ((char*)(d))[0] = ((char*)(s))[1], \
			  ((char*)(d))[1] = ((char*)(s))[2], \
			  ((char*)(d))[2] = ((char*)(s))[3], \
			  ((char*)(d))[3] = ((char*)(s))[4], 5 \
			) \
		 ) \
		)

	#define RWF_GET_OPTBYTE_U16_AS32(d,s) \
		( ( (unsigned char)((char*)(s))[0] < (rtrUInt8)0xFE )  ? \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = 0, \
			  ((char*)(d))[3] = ((char*)(s))[0], 1 ) : \
			( ((char*)(d))[0] = 0, \
			  ((char*)(d))[1] = 0, \
			  ((char*)(d))[2] = ((char*)(s))[1], \
			  ((char*)(d))[3] = ((char*)(s))[2], 3 ) )


#endif /* #ifdef RTR_LITTLE_ENDIAN */


#endif /* #ifndef __RTR_RWF_NET_H */
