/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_RSSL_UNIDIRECTION_SHMEM_TRANSPORT_H
#define __RTR_RSSL_UNIDIRECTION_SHMEM_TRANSPORT_H

/* Contains function declarations necessary for to hook in
 * unidirectional shared memory connection type 
 * (typically used for Tier 0 shared memory connection type).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslChanManagement.h"
#include "rtr/rsslTypes.h"
#include <stdio.h>

/* Initializes shared memory transport and function pointers */
RsslRet rsslUniShMemInitialize(RsslLockingTypes lockingType, RsslError *error);

/* Uninitializes transport */
RsslRet rsslUniShMemUninitialize();


#ifdef __cplusplus
};
#endif


#endif
