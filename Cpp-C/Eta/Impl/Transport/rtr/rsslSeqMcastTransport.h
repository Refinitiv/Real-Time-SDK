/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RTR_RSSL_SEQ_MCAST_TRANSPORT_H
#define __RTR_RSSL_SEQ_MCAST_TRANSPORT_H

/* Contains function declarations necessary to hook in the
 * bi-directional use of Sequence Multicast
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslTypes.h"
#include "rtr/rsslChanManagement.h"
#include <stdio.h>

#define RSSL_SEQ_MCAST_ERROR_TEXT_SIZE 255

typedef struct
{
	rtrUInt16		maxLength;
	rtrUInt16		length;
	char*			buffer;	/* Pointer to the data portion of the buffer */
	rtrUInt16		hdrOffset;
} rtrSeqMcastBuffer;
/* This should be a statically allocated part of the channel.  If we add buffer pooling at a later date, this is where we should try to do it. */

typedef enum {
	SEQ_MCAST_NO_FLAGS				= 0x00, /*!< @brief None. */
	SEQ_MCAST_FLAGS_RETRANSMIT		= 0x02
} SEQ_MCAST_FLAGS;

/* Initializes Sequence Multicast transport and sets up function pointers */
RsslRet rsslSeqMcastInitialize(RsslLockingTypes lockingType, RsslError *error);

/* Uninitializes Sequence Multicast transport */
RsslRet rsslSeqMcastUninitialize();

#ifdef __cplusplus
};
#endif


#endif
