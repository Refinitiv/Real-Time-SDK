/*
 *|---------------------------------------------------------------
 *|                Copyright (C) 2010 Thomson Reuters,          --
 *|                                                             --
 *|         1111 W. 22nd Street, Oak Brook, IL. 60521           --
 *|                                                             --
 *| All rights reserved. Duplication or distribution prohibited --
 *|---------------------------------------------------------------
 *
 */

#ifndef __ANSI_PLATFORM_H
#define __ANSI_PLATFORM_H

#include "dev/platform.h"

/* default macro defines */
#define ASSERT(x)


#if defined(x86_WindowsNT_3X) || defined(_WIN32)
	#include <assert.h>
#endif

#endif
