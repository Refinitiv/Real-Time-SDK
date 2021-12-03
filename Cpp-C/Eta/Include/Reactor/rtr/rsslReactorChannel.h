/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2020 Refinitiv. All rights reserved.
*/

#ifndef _RTR_REACTOR_CHANNEL_H
#define _RTR_REACTOR_CHANNEL_H

#include "rtr/rsslVAExports.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define REACTOR_INVALID_SOCKET INVALID_SOCKET
#else
#define REACTOR_INVALID_SOCKET -1
#endif

/**
 *	@addtogroup VAReactorChnl
 *	@{
 */

 /**
 * @brief Channel representing a connection handled by an RsslReactor.
 * @see RsslReactor, RsslChannel, RsslServer, RsslSocket, rsslReactorConnect, rsslReactorAccept, rsslReactorCloseChannel
 */
typedef struct
{
	RsslChannel	*pRsslChannel;	/*!< Rssl Channel handled by this RsslReactorChannel. */
	RsslServer	*pRsslServer;	/*!< The RsslServer that was used to accept the connection, if the channel was accepted with rsslReactorAccept. */
	RsslSocket	socketId;		/*!< Socket ID of the channel. Used for notification of available data for this channel.  */
	RsslSocket	oldSocketId;	/*!< Previous Socket Id of the channel, if an FD_CHANGE event has occurred. */
	RsslUInt32	majorVersion;	/*!< The major version that should be set on iterators when encoding and decoding messages for this channel. */
	RsslUInt32	minorVersion;	/*!< The minior version that should be set on iterators when encoding and decoding messages for this channel. */
	RsslUInt32	protocolType;	/*!< The protocol type of the encoder & decoder that should be used. */
	void	*userSpecPtr;		/*!< A user specified pointer associated with this RsslReactorChannel. */
} RsslReactorChannel;

/**
 * @brief Statistics returned by the rsslReactorGetChannelInfo() call.
 * @see RsslChannelInfo, rsslReactorGetChannelInfo
 */
typedef struct
{
	RsslChannelInfo	rsslChannelInfo;	/*!< RsslChannel information. */
} RsslReactorChannelInfo;

/**
 * @brief Returns information about the RsslReactorChannel.
 * @param pReactorChannel The channel to get information from.
 * @param pInfo RsslReactorChannelInfo structure to be populated with information.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if successful.
 * @return RsslRet failure codes, if an error occurred.
 * @see RsslReactor, RsslReactorChannel, RsslReactorChannelInfo, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslReactorGetChannelInfo(RsslReactorChannel* pReactorChannel, RsslReactorChannelInfo* pInfo, RsslErrorInfo* pError);

/**
 * @brief Statistics returned by the rsslReactorGetChannelStats() call.
 * @see RsslChannelInfo, rsslReactorGetChannelInfo
 */
typedef struct
{
	RsslChannelStats rsslChannelStats;	/*!< RsslChannel statistics. */
} RsslReactorChannelStats;

/**
 * @brief Returns statistical information about the RsslReactorChannel.
 * @param pReactorChannel The channel to get information from.
 * @param pInfo RsslReactorChannelStats structure to be populated with information.
 * @param pError Error structure to be populated in the event of failure.
 * @return RSSL_RET_SUCCESS, if successful.
 * @return RsslRet failure codes, if an error occurred.
 * @see RsslReactor, RsslReactorChannel, RsslReactorChannelInfo, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslReactorGetChannelStats(RsslReactorChannel* pReactorChannel, RsslReactorChannelStats* pInfo, RsslErrorInfo* pError);

/**
 * @brief Retrieve the total number of used buffers for an RsslReactorChannel.
 * @param pReactorChannel The channel to be queried.
 * @param pError Error structure to be populated in the event of failure.
 * @return If the value is positive, it is total number of buffers in use by this channel.
 * @return If the value is negative, it is an RsslRet failure code.
 * @see RsslReactor, RsslReactorChannel, RsslIoctlCodes, RsslErrorInfo
 */
RSSL_VA_API RsslInt32 rsslReactorChannelBufferUsage(RsslReactorChannel* pReactorChannel, RsslErrorInfo* pError);

/**
 *	@addtogroup VAReactorChnlOps
 *	@{
 */

/**
 * @brief Gets a buffer from the RsslReactorChannel for writing a message.
 * @param pChannel The channel to get a buffer for.
 * @param size The size(in bytes) of the buffer to get.
 * @param packedBuffer whether the buffer allows packing multiple messages via rsslReactorPackBuffer().
 * @param pError Error structure to be populated in the event of failure.
 * @return The buffer for writing the message.
 * @return NULL, if an error occurred. pError will be populated with information.
 * @see RsslReactor, RsslErrorInfo, rsslReactorPackBuffer, rsslReactorSubmit, rsslReactorReleaseBuffer
 */
RSSL_VA_API RsslBuffer* rsslReactorGetBuffer(RsslReactorChannel *pReactorChannel, RsslUInt32 size, RsslBool packedBuffer, RsslErrorInfo *pError);

/**
 * @brief Returns an unwritten buffer to the RsslReactorChannel.
 * @param pChannel The channel to get a buffer for.
 * @param pBuffer The buffer to release.
 * @param pError Error structure to be populated in the event of failure.
 * @return RsslRet codes.
 * @see RsslReactor, RsslReactorChannel, RsslErrorInfo, rsslReactorGetBuffer, rsslReactorSubmit
 */
RSSL_VA_API RsslRet rsslReactorReleaseBuffer(RsslReactorChannel *pReactorChannel, RsslBuffer *pBuffer, RsslErrorInfo *pError);

/**
 * @brief Packs an RsslBuffer and returns a new position in the buffer for writing another message.
 * @param pReactorChannel The channel that owns the RsslBuffer.
 * @param pBuffer The buffer to be packed. The length should be set to the size of the currently written message.
 * @param pError Error structure to be populated in the event of failure.
 * @return A new buffer position to begin writing to.
 * @return NULL, if an error occurred.
 * @see RsslReactor, RsslReactorChannel, RsslBuffer, RsslErrorInfo
 */
RSSL_VA_API RsslBuffer* rsslReactorPackBuffer(RsslReactorChannel *pReactorChannel, RsslBuffer *pBuffer, RsslErrorInfo *pError);

/**
 * @brief Changes some aspects of the RsslReactorChannel.
 * @param pReactorChannel The channel to be modified.
 * @param code Code indicating the option to change. See RsslIoctlCodes.
 * @param value Value to change the option to.
 * @param pError Error structure to be populated in the event of failure.
 * @return RsslRet return codes
 * @see RsslReactor, RsslReactorChannel, RsslIoctlCodes, RsslErrorInfo
 */
RSSL_VA_API RsslRet rsslReactorChannelIoctl(RsslReactorChannel* pReactorChannel, int code, void* value, RsslErrorInfo* pError);

/**
 *	@}
 */


/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif
