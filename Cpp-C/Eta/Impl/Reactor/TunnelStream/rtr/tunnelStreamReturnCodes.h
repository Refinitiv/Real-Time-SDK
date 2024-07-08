/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef TUNNEL_STREAM_RETURN_CODES_H
#define TUNNEL_STREAM_RETURN_CODES_H

#include "rtr/rsslRetCodes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Additional return codes that may be returned to the reactor by the Tunnel Manager,
 * in addition to RSSL_RET_SUCCESS and RSSL_RET_BUFFER_NO_BUFFERS.  */
typedef enum 
{
	RSSL_RET_CHANNEL_ERROR		= -30000,	/* A fatal channel error was encountered while reading from or writing to that channel. */
	RSSL_RET_NO_TUNNEL_STREAM	= -30001	/* The StreamID of a passed-in message is not in the table of currently-open tunnel streams. */
} TunnelStreamReturnCodes;

#ifdef __cplusplus
};
#endif

#endif
