/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

/*
 * flip.h - defines swapping macros.
 */

#ifndef	_IPC_FLIP_H
#define	_IPC_FLIP_H

#include "rtr/ripcplat.h"


#define	host2net_u32(x) (u32)(h2n_u32((u32)(x)))
#define	host2net_u16(x) (u16)(h2n_u16((u16)(x)))

#define	net2host_u32(x)	(u32)(n2h_u32((u32)(x)))
#define	net2host_u16(x)	(u16)(n2h_u16((u16)(x)))

#ifdef	RTR_LITTLE_ENDIAN
#define	h2n_u32(p) \
   (u32)(((p>>24)&0xff)|((p>>8)&0xff00)|((p<<8)&0xff0000)|((p<<24)&0xff000000))
#define	h2n_u16(p) (u16)(((p<<8)&(u16)0xff00)|((p>>8)&(u16)0x00ff))
#define	h2n_a16(p,n)  byte_swap(p,(u16)sizeof(u16)*n)

#define	n2h_u32(p) \
   (u32)(((p>>24)&0xff)|((p>>8)&0xff00)|((p<<8)&0xff0000)|((p<<24)&0xff000000))
#define n2h_u16(p) (u16)(((p<<8)& (u16) 0xff00)|((p>>8)& (u16) 0x00ff))
#define	n2h_a16(p,n)  byte_swap(p,(u16)sizeof(u16)*n)

#else	/* Not RTR_LITTLE_ENDIAN */
#define	h2n_u32(p)     ((u32)(p))
#define	h2n_u16(p)     ((u16)(p))
#define	h2n_a16(p,n)     ((u16*)(p))
#define	n2h_u32(p)     ((u32)(p))
#define n2h_u16(p)     ((u16)(p))
#define	n2h_a16(p,n)     ((u16*)(p))
#endif	/* RTR_LITTLE_ENDIAN */


#ifdef RTR_QUAD_32_ALIGN
#define move_u16(d,s) \
	((char*)(d))[0] = ((char*)(s))[0];\
	((char*)(d))[1] = ((char*)(s))[1]
#define move_u32(d,s) \
	((char*)(d))[0] = ((char*)(s))[0];\
	((char*)(d))[1] = ((char*)(s))[1];\
	((char*)(d))[2] = ((char*)(s))[2];\
	((char*)(d))[3] = ((char*)(s))[3]
#ifdef RTR_LITTLE_ENDIAN
#define _move_u16_swap(d,s) \
	((char*)(d))[0] = ((char*)(s))[1];\
	((char*)(d))[1] = ((char*)(s))[0]
#define _move_u32_swap(d,s) \
	((char*)(d))[0] = ((char*)(s))[3];\
	((char*)(d))[1] = ((char*)(s))[2];\
	((char*)(d))[2] = ((char*)(s))[1];\
	((char*)(d))[3] = ((char*)(s))[0]
#else /* not RTR_LITTLE_ENDIAN */
#define _move_u16_swap(d,s) \
	((char*)(d))[0] = ((char*)(s))[0];\
	((char*)(d))[1] = ((char*)(s))[1]
#define _move_u32_swap(d,s) \
	((char*)(d))[0] = ((char*)(s))[0];\
	((char*)(d))[1] = ((char*)(s))[1];\
	((char*)(d))[2] = ((char*)(s))[2];\
	((char*)(d))[3] = ((char*)(s))[3]
#endif /* if RTR_LITTLE_ENDIAN */
	
#else /* not RTR_QUAD_32_ALIGN */
#ifdef RTR_LITTLE_ENDIAN
#define _move_u16_swap(d,s) \
	((char*)(d))[0] = ((char*)(s))[1];\
	((char*)(d))[1] = ((char*)(s))[0]
#define _move_u32_swap(d,s) \
	((char*)(d))[0] = ((char*)(s))[3];\
	((char*)(d))[1] = ((char*)(s))[2];\
	((char*)(d))[2] = ((char*)(s))[1];\
	((char*)(d))[3] = ((char*)(s))[0]
#else /* not RTR_LITTLE_ENDIAN */
#define _move_u16_swap(d,s) \
	*(u16 *)(d) = *(u16 *)(s)
#define _move_u32_swap(d,s) \
	*(u32 *)(d) = *(u32 *)(s)
#endif /* if RTR_LITTLE_ENDIAN */
	
#define move_u16(d,s) \
	*(u16 *)(d) = *(u16 *)(s)
#define move_u32(d,s) \
	*(u32 *)(d) = *(u32 *)(s)
#endif /* if RTR_QUAD_32_ALIGN */

#endif
