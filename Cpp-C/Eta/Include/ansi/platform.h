/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __ANSI_PLATFORM_H
#define __ANSI_PLATFORM_H

#include "rtr/platform.h"

/* default macro defines */
#define ASSERT(x)


#if defined(x86_WindowsNT_3X) || defined(_WIN32)
	#include <assert.h>
#endif

#endif
