/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _RTR_RSSL_REACTOR_CALLBACK_RETURN_CODES_H
#define _RTR_RSSL_REACTOR_CALLBACK_RETURN_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup VAReactorStruct
 *	@{
 */

/**
 * @brief Codes that may be returned from Callback functions.
 * @see loginMsgCallback, directoryMsgCallback, dictionaryMsgCallback, defaultMsgCallback, channelEventCallback
 */
typedef enum 
{
	RSSL_RC_CRET_SUCCESS		= 0,	/*!< (0) The message or event has been handled. */
	RSSL_RC_CRET_FAILURE		= -1,	/*!< (-1) The message or event failed to be handled. Returning this code will cause the RsslReactor to shutdown. */
	RSSL_RC_CRET_RAISE			= -2	/*!< (-2) Used from domain-specific callbacks such as loginMsgCallback. Causes the defaultMsgCallback function to be called with the same message. */
} RsslReactorCallbackRet;



/**
 *	@}
 */


#ifdef __cplusplus
}
#endif

#endif
