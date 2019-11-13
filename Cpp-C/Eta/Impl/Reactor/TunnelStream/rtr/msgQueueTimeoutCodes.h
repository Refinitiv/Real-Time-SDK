/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef _RTR_MSGQUEUE_TIMEOUT_CODES_H
#define _RTR_MSGQUEUE_TIMEOUT_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Special timeout values used when queueing outbound messages.
 */
typedef enum
{
	MSGQUEUE_TO_INFINITE = 0, /*!< The messages has no timeout and does not expire. */

	MSGQUEUE_TO_IMMEDIATE = -1 /*!< The message should expire immediately if the recipient queue is not online. */
} MsgQueueTimeoutCodes;

#ifdef __cplusplus
}
#endif

#endif
