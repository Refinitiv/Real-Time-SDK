/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef TUNNEL_STREAM_H
#define TUNNEL_STREAM_H

#include "rtr/rsslVAExports.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslMemoryBuffer.h"
#include "rtr/rsslErrorInfo.h"
#include "rtr/rsslReactorCallbackReturnCodes.h"
#include "rtr/rsslReactorChannel.h"
#include "rtr/rsslClassOfService.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief Represents a tunnel stream. A tunnel stream is a private stream that supports
  * interactions with guaranteed messaging and other auxiliary services.
  */
typedef struct
{
	char					*name;				/*!< Name of the tunnel stream. */
	RsslInt32				streamId;			/*!< Stream ID of the tunnel stream. */
	RsslUInt8				domainType;			/*!< Domain type of the tunnel stream. */
	RsslUInt16				serviceId;			/*!< Service ID of the tunnel stream. */
	RsslReactorChannel		*pReactorChannel;	/*!< Reactor channel associated with this tunnel stream. */
	void					*userSpecPtr;		/*!< A user-specified pointer associated with the tunnel stream. */
	RsslState				state;				/*!< The last known state of this tunnel stream. */
	RsslClassOfService		classOfService;		/*!< Indicates negotiated parameters associated with the tunnel stream .*/
} RsslTunnelStream;

/**
 * @brief An event containing authentication information for an RsslTunnelStreamStatusEvent.
 * @see RsslTunnelStreamStatusEvent
 */
typedef struct
{
	RsslRDMLoginMsg		*pLoginMsg; 	/*!< The received authenication message, decoded as an RsslRDMLoginMsg. */
} RsslTunnelStreamAuthInfo;

/**
 * @brief An event regarding the state of the tunnel stream.
 * @see RsslTunnelStreamStatusEventCallback
 */
typedef struct
{
	RsslReactorChannel			*pReactorChannel;	/*!< Reactor channel associated with this event. */
	RsslState					*pState;			/*!< The state of the tunnel stream due to this event. */
	RsslMsg						*pRsslMsg;			/*!< Message that resulted in this status event. */
	RsslTunnelStreamAuthInfo	*pAuthInfo;			/*!< (Consumers only) Provides information about a received authentication response. */
} RsslTunnelStreamStatusEvent;

/**
 * @brief An event indicating a message received in this tunnel stream.
 * @see RsslTunnelStreamDefaultMsgCallback
 */
typedef struct
{
	RsslUInt8				containerType;		/*!< Container type of message content in this event. See RsslDataTypes. */
	RsslReactorChannel		*pReactorChannel;	/*!< Reactor channel associated with this event. */
	RsslMsg					*pRsslMsg;			/*!< The RsslMsg structure. Present if decoding of the message was successful. */
	RsslBuffer				*pRsslBuffer;		/*!< Encoded buffer content. Present if the buffer is not an RsslMsg. */
	RsslErrorInfo			*pErrorInfo;		/*!< Error information. Present if a problem was encountered, and provides information about the error and its location in the source code. */
} RsslTunnelStreamMsgEvent;

/**
 * @brief An event indicating a queue message received in this tunnel stream.
 * @see RsslTunnelStreamQueueMsgCallback
 */
typedef struct
{
	RsslTunnelStreamMsgEvent	base; /*!< Base tunnel stream message event. */
	RsslRDMQueueMsg				*pQueueMsg; /*!< The RsslRDMQueueMsg of this event. */
} RsslTunnelStreamQueueMsgEvent;

/**
 * @brief Signature of a Tunnel Stream Status Event Callback function.
 * @see RsslTunnelStream, RsslTunnelStreamStatusEvent
 */
typedef RsslReactorCallbackRet RsslTunnelStreamStatusEventCallback(RsslTunnelStream*, RsslTunnelStreamStatusEvent*);

/**
 * @brief Signature of a Tunnel Stream Message Event Callback function.
 * @see RsslTunnelStream, RsslTunnelStreamMsgEvent
 */
typedef RsslReactorCallbackRet RsslTunnelStreamDefaultMsgCallback(RsslTunnelStream*, RsslTunnelStreamMsgEvent*);

/**
 * @brief Signature of a Tunnel Stream QueueMsg Event Callback function.
 * @see RsslTunnelStream, RsslTunnelStreamQueueMsgEvent
 */
typedef RsslReactorCallbackRet RsslTunnelStreamQueueMsgCallback(RsslTunnelStream*, RsslTunnelStreamQueueMsgEvent*);

/**
 * @brief An event indicating a new tunnel stream from a consumer.
 * @see RsslTunnelStreamListenerCallback
 */
typedef struct
{
	RsslReactorChannel	*pReactorChannel;	  	/*!< RsslReactorChannel on which this event was received. */
	RsslInt32			streamId;				/*!< Stream ID of the requested tunnel stream. */
	RsslUInt8			domainType;				/*!< Domain type of the requested tunnel stream. */
	RsslInt16			serviceId;				/*!< Service ID  of the requested tunnel stream. */
	char*				name;					/*!< Name of the requested tunnel stream. */
	RsslUInt16			classOfServiceFilter;	/*!< Filter indicating what categories of information are present in the request. See RDMClassOfServiceFilterFlags. */
} RsslTunnelStreamRequestEvent;

/**
 * @brief Decodes the ClassOfService information contained in an RsslTunnelStreamRequestEvent
 * @see RsslTunnelStreamQueueMsgCallback
 */
RSSL_VA_API RsslRet rsslTunnelStreamRequestGetCos(RsslTunnelStreamRequestEvent *pEvent, 
																RsslClassOfService *pCos, RsslErrorInfo *pError);

/**
 * @brief Signature of a Tunnel Stream Listener Event Callback function.
 * @see RsslTunnelStreamRequestEvent
 */
typedef RsslReactorCallbackRet RsslTunnelStreamListenerCallback(RsslTunnelStreamRequestEvent *pEvent, RsslErrorInfo *pErrorInfo);

/**
 * @brief Options for submitting a message to a tunnel Stream.
 * @see rsslTunnelStreamSubmitMsg
 */
typedef struct
{
	RsslRDMMsg	*pRDMMsg; /*!< RsslRDMMsg to submit (use only one of pRsslMsg and pRDMMsg). */
	RsslMsg		*pRsslMsg; /*!< RsslMsg to submit (use only one of pRsslMsg and pRDMMsg). */
} RsslTunnelStreamSubmitMsgOptions;

/**
 * @brief Clears a RsslTunnelStreamSubmitMsgOptions object.
 * @see RsslTunnelStreamSubmitMsgOptions
 */
RTR_C_INLINE void rsslClearTunnelStreamSubmitMsgOptions(RsslTunnelStreamSubmitMsgOptions *pOpts)
{
	pOpts->pRDMMsg = NULL;
	pOpts->pRsslMsg = NULL;
}


/**
 * @brief Sends an RsslMsg or RsslRDM message to the Tunnel Stream.
 * @param pTunnelStream The Tunnel Stream to send the RsslRDM message to.
 * @param pRsslTunnelStreamSubmitMsgOptions The send options (includes the message).
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if submitting of the RsslRDM message succeeded.
 * @return failure codes, if the RsslRDM message could not be submitted due to a failure.
 * @see RsslTunnelStream, RsslTunnelStreamSubmitMsgOptions, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslTunnelStreamSubmitMsg(RsslTunnelStream *pTunnelStream, RsslTunnelStreamSubmitMsgOptions *pRsslTunnelStreamSubmitMsgOptions, RsslErrorInfo *pError);

/**
 * @brief Options for getting a buffer for sending a message through the tunnel stream.
 * @see rsslTunnelStreamGetBuffer
 */
typedef struct
{
	RsslUInt32 size;	/*!< The size(in bytes) of the buffer to get. */
} RsslTunnelStreamGetBufferOptions;

/**
 * @brief Clears a RsslTunnelStreamGetBufferOptions object.
 * @see RsslTunnelStreamGetBufferOptions
 */
RTR_C_INLINE void rsslClearTunnelStreamGetBufferOptions(RsslTunnelStreamGetBufferOptions *pOpts)
{
	pOpts->size = 0;
}

/**
 * @brief Gets a buffer from the TunnelStream for writing a message.
 * @param pTunnel The Tunnel Stream of the buffer to get.
 * @param pRsslTunnelStreamGetBufferOptions Options for getting the buffer (includes the length).
 * @param pError Error structure to be populated in the event of failure.
 * @return the RsslBuffer for writing the message or
 *         NULL, if an error occurred (errorInfo will be populated with information).
 * @see RsslBuffer, RsslTunnelStream, RsslErrorInfo
 */
RSSL_VA_API RsslBuffer *rsslTunnelStreamGetBuffer(RsslTunnelStream *pTunnel,
		RsslTunnelStreamGetBufferOptions *pRsslTunnelStreamGetBufferOptions, RsslErrorInfo *pError);

/**
 * @brief Releases a RsslBuffer.
 * @param pTunnel The Tunnel Stream of the buffer to release.
 * @param pBuffer The buffer to release.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if releasing the buffer succeeded.
 * @return failure codes, if an error occurred (errorInfo will be populated with information).
 * @see RsslBuffer, RsslTunnelStream, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslTunnelStreamReleaseBuffer(RsslBuffer *pBuffer, RsslErrorInfo *pError);

/**
 * @brief Structure to obtain the tunnel stream information.
 * @see rsslTunnelStreamGetInfo, rsslClearTunnelStreamInfo
 */
typedef struct
{
	RsslUInt buffersUsed;	/*!< The number of the buffers are in use. */
} RsslTunnelStreamInfo;

/**
 * @brief Clears a RsslTunnelStreamInfo object.
 * @see rsslTunnelStreamGetInfo, RsslTunnelStreamInfo
 */
RTR_C_INLINE void rsslClearTunnelStreamInfo(RsslTunnelStreamInfo *pInfo)
{
	pInfo->buffersUsed = 0;
}

/**
 * @brief Retrieves the tunnel stream information.
 * @param pTunnel The Tunnel Stream to retreive the tunnel stream information.
 * @param pInfo Structure to obtain the tunnel stream information.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if retreiving of the tunnel stream information succeeded.
 * @return failure codes, if an error occurred (errorInfo will be populated with information).
 * @see RsslBuffer, RsslTunnelStream, RsslTunnelStreamInfo, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslTunnelStreamGetInfo(RsslTunnelStream *pTunnel, RsslTunnelStreamInfo *pInfo, RsslErrorInfo *pError);

/**
 * @brief Options for submitting a buffer to a tunnel Stream.
 * @see rsslTunnelStreamSubmit
 */
typedef struct
{
	RsslUInt8	containerType;	/*!< containterType of encoded data. See RsslDataTypes */
} RsslTunnelStreamSubmitOptions;

/**
 * @brief Clears a RsslTunnelStreamSubmitOptions object.
 * @see RsslTunnelStreamSubmitOptions
 */
RTR_C_INLINE void rsslClearTunnelStreamSubmitOptions(RsslTunnelStreamSubmitOptions *pOpts)
{
	pOpts->containerType = RSSL_DT_UNKNOWN;
}


/**
 * @brief Sends an encoded buffer to the Tunnel Stream.
 * @param pTunnelStream The Tunnel Stream to send the encoded buffer to.
 * @param pBuffer The tunnel stream buffer to submit.
 * @param pRsslTunnelStreamSubmitOptions options for sending the buffer.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if submitting of the encoded buffer succeeded.
 * @return failure codes, if the encoded buffer could not be submitted due to a failure.
 * @see RsslTunnelStream, RsslTunnelStreamSubmitOptions, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslTunnelStreamSubmit(RsslTunnelStream *pTunnelStream, RsslBuffer *pBuffer, RsslTunnelStreamSubmitOptions *pRsslTunnelStreamSubmitOptions, RsslErrorInfo *pError);

#ifdef __cplusplus
};
#endif

#endif
