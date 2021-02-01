/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2020 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_HASH_FUNCS_H
#define __RSSL_HASH_FUNCS_H

#include "rtr/rsslTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief Implements a basic hash function.
 */
RSSL_API RsslUInt32 rsslHashingEntityId(const char *buf, const RsslUInt32 length, const RsslUInt32 numberOfHashingEntities);

#ifdef __cplusplus
}
#endif

#endif
