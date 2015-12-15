/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef _RTR_RSSL_EVENTS_H
#define _RTR_RSSL_EVENTS_H

#include "rtr/rsslReactorChannel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup VAReactorEvent
 *	@{
 */

/**
 * @brief Type indicating the type of RsslReactorChannelEvent.
 * @see RsslReactorChannelEvent
 */
typedef enum
{
	RSSL_RC_CET_MIN				= -128,	/*!< Minimum value expected - this enumeration is for range checking */
	RSSL_RC_CET_INIT			= 0,	/*!< Unknown event type. */
	RSSL_RC_CET_CHANNEL_UP		= 1,	/*!< Channel has successfully initialized and can be dispatched. If the application presented any messages for setting
										 * up the session in their RsslReactorChannelRole structure in rsslReactorConnect() or rsslReactorAccept(),
										 * they will now be sent. */
	RSSL_RC_CET_CHANNEL_READY	= 2,	/*!< Channel has sent and received all messages expected for setting up the session. Normal use(such as item requests) can now be done. */
	RSSL_RC_CET_CHANNEL_DOWN	= 3,	/*!< Channel has failed(e.g. the connection was lost or a ping timeout expired) and will no longer be processed.  The application should call rsslReactorCloseChannel() to clean up the channel. */
	RSSL_RC_CET_FD_CHANGE		= 4,	/*!< The file descriptor representing this channel has changed. The new and old Socket ID can be found on the RsslReactorChannel. */
	RSSL_RC_CET_WARNING			= 5,		/*!< An event has occurred that did not result in channel failure, but may require attention by the application. */
	RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING = 6,
	RSSL_RC_CET_CHANNEL_OPENED	= 7		/*!< Channel was opened by the application and can be used (occurs when watchlist is enabled and only appears in the channelOpenCallback). */
} RsslReactorChannelEventType;

/**
 * @brief An event that has occurred on an RsslReactorChannel.
 * @see RsslReactorChannel
 */
typedef struct
{
	RsslReactorChannelEventType	channelEventType;	/*!< The type of the event. Populated by RsslReactorChannelEventType. */
	RsslReactorChannel			*pReactorChannel;	/*!< The channel associated with this event. */
	RsslErrorInfo				*pError; 			/*!< Present on warnings and channel down events. Contains information about the error that occurred. */
} RsslReactorChannelEvent;

/**
 * @brief Clears an RsslReactorChannelEvent.
 * @see RsslReactorChannelEvent
 */
RTR_C_INLINE void rsslClearReactorChannelEvent(RsslReactorChannelEvent *pEvent)
{
	memset(pEvent, 0, sizeof(RsslReactorChannelEvent));
}

/** 
 * @brief Information about the stream associated with a message.
 * Only used when a watchlist is enabled. */
typedef struct
{
	const RsslBuffer	*pServiceName;	/*!< Name of service associated with the stream, if any. */
	void				*pUserSpec;		/*!< User-specified pointer given when the stream was opened. */
} RsslStreamInfo;

/**
 * @brief Base event provided to Message Callback functions.  This structure is provided to defaultMsgCallback functions
 * and is present in structures provided to other Message Callback functions such as loginMsgCallback and directoryMsgCallback. 
 */
typedef struct
{
	RsslBuffer		*pRsslMsgBuffer;	/*!< The raw message that was read and processed. */
	RsslMsg			*pRsslMsg;			/*!< The RsslMsg structure. Present if decoding of the message was successful. */
	RsslStreamInfo	*pStreamInfo;		/*!< Stream information (watchlist enabled only). */
	RsslErrorInfo	*pErrorInfo;		/*!< Error information. Present if a problem was encountered, and provides information about the error and its location in the source code. */
	RsslUInt32		*pSeqNum;			/*!< Sequence number associated with this message. */
	RsslUInt8		*pFTGroupId;		/*!< FTGroupId associated with this message. */
} RsslMsgEvent;

/**
 * @brief Clears an RsslMsgEvent.
 * @see RsslMsgEvent
 */
RTR_C_INLINE void rsslClearMsgEvent(RsslMsgEvent *pEvent)
{
	pEvent->pRsslMsgBuffer = NULL;
	pEvent->pRsslMsg = NULL;
	pEvent->pErrorInfo = NULL;
	pEvent->pStreamInfo = NULL;
	pEvent->pSeqNum = NULL;
	pEvent->pFTGroupId = NULL;
}

/**
 * @brief Event provided to Login Message Callback functions.  
 * @see RsslMsgEvent
 */
typedef struct
{
	RsslMsgEvent	baseMsgEvent;		/*!< Base Message event. */
	RsslRDMLoginMsg		*pRDMLoginMsg;	/*!< The RDM Representation of the decoded Login message.  If not present, an error was encountered while decoding and information will be in baseMsgEvent.pErrorInfo */
} RsslRDMLoginMsgEvent;

/**
 * @brief Clears an RsslRDMLoginMsgEvent
 * @see RsslRDMLoginMsgEvent
 */
RTR_C_INLINE void rsslClearRDMLoginMsgEvent(RsslRDMLoginMsgEvent *pEvent)
{
	rsslClearMsgEvent(&pEvent->baseMsgEvent);
	pEvent->pRDMLoginMsg = NULL;
}

/**
 * @brief Event provided to Directory Message Callback functions.  
 * @see RsslMsgEvent
 */
typedef struct
{
	RsslMsgEvent	baseMsgEvent;			/*!< Base Message event. */
	RsslRDMDirectoryMsg	*pRDMDirectoryMsg;	/*!< The RDM Representation of the decoded Directory message.  If not present, an error was encountered while decoding and information will be in baseMsgEvent.pErrorInfo */
} RsslRDMDirectoryMsgEvent;


/**
 * @brief Clears an RsslRDMDirectoryMsgEvent
 * @see RsslRDMDirectoryMsgEvent
 */
RTR_C_INLINE void rsslClearRDMDirectoryMsgEvent(RsslRDMDirectoryMsgEvent *pEvent)
{
	rsslClearMsgEvent(&pEvent->baseMsgEvent);
	pEvent->pRDMDirectoryMsg = NULL;
}

/**
 * @brief Event provided to Dicionary Message Callback functions.  
 * @see RsslMsgEvent
 */
typedef struct
{
	RsslMsgEvent		baseMsgEvent;			/*!< Base Message event. */
	RsslRDMDictionaryMsg	*pRDMDictionaryMsg;	/*!< The RDM Representation of the decoded Dicionary message.  If not present, an error was encountered while decoding and information will be in baseMsgEvent.pErrorInfo */
} RsslRDMDictionaryMsgEvent;


/**
 * @brief Clears an RsslRDMDicionaryMsgEvent
 * @see RsslRDMDicionaryMsgEvent
 */
RTR_C_INLINE void rsslClearRDMDictionaryMsgEvent(RsslRDMDictionaryMsgEvent *pEvent)
{
	rsslClearMsgEvent(&pEvent->baseMsgEvent);
	pEvent->pRDMDictionaryMsg = NULL;
}


/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif
