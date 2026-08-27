/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_PATH_H
#define __RTR_PATH_H

#include "rtr/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Returns a pointer to the filename part of a path.
 * Supports both '/' and '\\' separators.
 * If no separator exists, returns the original input pointer.
 * If path is NULL, returns NULL.
 * If both separators exist, returns the one that appears last in the string.
 */
RTR_C_ALWAYS_INLINE const char *rtr_basename(const char *path)
{
	const char *last_slash = NULL;
	const char *p;

	if (!path)
		return NULL;

	// Single pass scan for both separators
	for (p = path; *p; p++)
	{
		if (*p == '/' || *p == '\\')
			last_slash = p;
	}

	// If a separator was found, return the substring after it,
	// otherwise return the input.
	return last_slash ? last_slash + 1 : path;
}

#ifdef __cplusplus
}
#endif

#endif /* __RTR_PATH_H */
