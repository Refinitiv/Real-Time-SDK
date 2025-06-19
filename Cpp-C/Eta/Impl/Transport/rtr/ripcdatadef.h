/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef	_LIBDEF__
#define	_LIBDEF__

/* Windows 3.x is the only 16-bit system supported */
#ifndef	u32

#if defined(_WIN16)
typedef	unsigned long	u32;
#else /* rest of the platforms: UNIX, NT, etc. */
typedef	unsigned int	u32;
#endif  /* defined(_WIN16) */

#endif

#ifndef	u16
typedef	unsigned short	u16;
#endif

#ifndef	u8
typedef	unsigned char	u8;
#endif

#endif
