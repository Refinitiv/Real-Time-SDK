/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef RSSL_RDM_QUEUE_MSG_MSG_H
#define RSSL_RDM_QUEUE_MSG_MSG_H

#include "rtr/rsslVAExports.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslRDM.h"
#include "rtr/rsslRDMMsgBase.h"
#include "rtr/rsslMemoryBuffer.h"
#include "rtr/rsslErrorInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup VARDMQueue
 *	@{
 */

/** 
 * @brief The types of RDM Queue Messages.  When the message is a queue message,
 * The rdmMsgType member may be set to one of these to indicate the specific RsslRDMQueueMsg class.
 * @see RsslRDMQueueMsg, RsslRDMMsgBase, RsslRDMQueueRequest, RsslRDMQueueRefresh, RsslRDMQueueStatus, RsslRDMQueueData, RsslRDMQueueDataExpired, RsslRDMQueueAck, RsslRDMQueueClose
 */
typedef enum
{
	RDM_QMSG_MT_UNKNOWN			= 0, /*!< (0) Unknown type. */
	RDM_QMSG_MT_REQUEST			= 1, /*!< (1) Queue Request. */
	RDM_QMSG_MT_REFRESH			= 2, /*!< (2) Queue Refresh. */
	RDM_QMSG_MT_STATUS			= 3, /*!< (3) Queue Status. */
	RDM_QMSG_MT_DATA			= 4, /*!< (4) Queue Data. */
	RDM_QMSG_MT_DATA_EXPIRED	= 5, /*!< (5) Queue Data (Expired). */
	RDM_QMSG_MT_ACK				= 6, /*!< (6) Queue Ack. */
	RDM_QMSG_MT_CLOSE			= 7  /*!< (7) Queue Close. */
} RsslRDMQueueMsgType;

/**
  * @brief Provides a string representation of a QueueMsg type.
  * @see RsslRDMQueueMsgType
  */
RSSL_VA_API const char* rsslRDMQueueMsgTypeToString(RsslRDMQueueMsgType type);

/**
 * @brief Maximum allowed length for a queue source or destination name.
 */
#define RDM_QMSG_MAX_NAME_LENGTH 200

/**
 * @brief The RDM Queue Request. Used by an OMM Consumer to connect to a QueueProvider.
 * @see RsslRDMMsgBase, RsslRDMQueueMsg, rsslClearRDMQueueRequest
 */
typedef struct
{
	RsslRDMMsgBase				rdmMsgBase;	/*!< The base RDM Message. */
	RsslBuffer					sourceName;	/*!< The name of the queue to open with this request. */
} RsslRDMQueueRequest;

/**
 * @brief Clears an RsslRDMQueueRequest.
 * @see RsslRDMQueueRequest
 */
RTR_C_INLINE void rsslClearRDMQueueRequest(RsslRDMQueueRequest *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMQueueRequest));
	pMsg->rdmMsgBase.rdmMsgType = RDM_QMSG_MT_REQUEST;
}

/**
 * @brief The RDM Queue Refresh.
 * @see RsslRDMMsgBase, RsslRDMQueueMsg, rsslClearRDMQueueRefresh
 */
typedef struct
{
	RsslRDMMsgBase				rdmMsgBase;	/*!< The base RDM Message. */
	RsslBuffer					sourceName;	/*!< Name of the queue that opened this stream. This should match the request. */
	RsslState					state;		/*!< The state of the stream. */
	RsslUInt16					queueDepth;	/*!< The number of messages remaining in the queue for this stream. */
} RsslRDMQueueRefresh;

/**
 * @brief Clears an RsslRDMQueueRefresh.
 * @see RsslRDMQueueRefresh
 */
RTR_C_INLINE void rsslClearRDMQueueRefresh(RsslRDMQueueRefresh *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMQueueRefresh));
	pMsg->rdmMsgBase.rdmMsgType = RDM_QMSG_MT_REFRESH;
}

/**
 * The RDM Queue Status Flags.
 * @see RsslRDMQueueStatus
 */
typedef enum
{
	RDM_QMSG_STF_NONE		= 0x0, /*!< (0) No flags set. */
	RDM_QMSG_STF_HAS_STATE	= 0x1  /*!< (1) Indicates presence of the state member. */
} RsslRDMQueueStatusFlags;

/**
 * @brief The RDM Queue Status.
 * @see RsslRDMMsgBase, RsslRDMQueueMsg, rsslClearRDMQueueStatus
 */
typedef struct
{
	RsslRDMMsgBase				rdmMsgBase;	/*!< The base RDM message. */
	RsslUInt16					flags;		/*!< Queue Status flaggs. See RsslRDMQueueStatusFlags. */
	RsslState					state;		/*!< The state of the stream. */
} RsslRDMQueueStatus;

/**
 * @brief Clears an RsslRDMQueueStatus.
 * @see RsslRDMQueueStatus
 */
RTR_C_INLINE void rsslClearRDMQueueStatus(RsslRDMQueueStatus *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMQueueStatus));
	pMsg->rdmMsgBase.rdmMsgType = RDM_QMSG_MT_STATUS;
}

/**
 * @brief Type used with Queue Data Messages.
 * @see RsslRDMQueueData
 */
typedef enum
{
	RDM_QMSG_DATA_MT_INIT			= 0, /*!< (0) Queue data message initialization value. This should not be used by the application or returned to the application. */
	RDM_QMSG_DATA_MT_DATA			= 1, /*!< (1) Indicates a normal data message. */
	RDM_QMSG_DATA_MT_UNDELIVERABLE	= 2  /*!< (2) Indicates an undeliverable data message. */
} RsslRDMQueueDataMsgType;

/**
  * @brief Provides a string representation of a QueueData message type.
  * @see RsslRDMQueueData, RsslRDMQueueDataMsgType
  */
RSSL_VA_API const char* rsslRDMQueueDataMsgType(RsslRDMQueueDataMsgType type);

/** @brief Codes for undeliverable queue messages. */
typedef enum
{
	RDM_QMSG_UC_UNSPECIFIED		= 0, /*!< (0) Unspecified. */
	RDM_QMSG_UC_EXPIRED			= 1, /*!< (1) The time limit on this message expired. */
	RDM_QMSG_UC_NO_PERMISSION	= 2, /*!< (2) Sender is not permitted to send to this message's destination. */
	RDM_QMSG_UC_INVALID_TARGET	= 3, /*!< (3) Destination of this message does not exist. */
	RDM_QMSG_UC_QUEUE_FULL		= 4, /*!< (4) Destination's message queue is full. */
	RDM_QMSG_UC_QUEUE_DISABLED	= 5, /*!< (5) Destination's queue has been disabled. */
	RDM_QMSG_UC_MAX_MSG_SIZE	= 6, /*!< (6) Message was too large. */
	RDM_QMSG_UC_INVALID_SENDER	= 7, /*!< (7) The sender of this message is now invalid. */
	RDM_QMSG_UC_TARGET_DELETED	= 8 /*!< (8) The target queue was deleted after sending the message, but before it was delivered. */
} RsslRDMQueueUndeliverableCode;

/**
  * @brief Provides a string representation of a QueueData undeliverable code.
  * @see RsslRDMQueueUndeliverableCode
  */
RSSL_VA_API const char* rsslRDMQueueUndeliverableCodeToString(RsslUInt8 code);

/**
 * @brief Special timeout values used when sending queue messages.
 * @see RsslRDMQueueData
 */
typedef enum
{
	RDM_QMSG_TC_PROVIDER_DEFAULT	= -2, /*!< (-2) The message will expire at a value chosen by the provider. */
	RDM_QMSG_TC_INFINITE			= -1, /*!< (-1) The messages has no timeout and does not expire. */
	RDM_QMSG_TC_IMMEDIATE			= 0   /*!< (0) The message should expire immediately if the recipient queue is not online. */
} RsslRDMQueueTimeoutCode;

/**
 * @brief Flags associated with an RsslRDMQueueData message.
 * @see RsslRDMQueueData
 */
typedef enum
{
	RDM_QMSG_DF_NONE				= 0x00,	/*!< (0x00) None */
	RDM_QMSG_DF_POSSIBLE_DUPLICATE	= 0x01	/*!< (0x01) This message may be a duplicate of a previous message. */
} RsslRDMQueueDataFlags;

/**
  * @brief Provides a string representation of a QueueData timeout code.
  * @see RsslRDMQueueData, RsslRDMQueueTimeoutCode
  */
RSSL_VA_API const char* rsslRDMQueueTimeoutCodeToString(RsslRDMQueueTimeoutCode code);

/**
 * @brief The RDM Queue Data.
 * @see RsslRDMMsgBase, RsslRDMQueueMsg, rsslClearRDMQueueData
 */
typedef struct
{
	RsslRDMMsgBase		rdmMsgBase;			/*!< The base RDM message. */
	RsslUInt16			flags;				/*!< Flags associated with this message. See RsslRDMQueueDataFlags. These flags are read-only. */
	RsslUInt8			containerType;		/*!< Type of the contents of the encDataBody. See RsslDataTypes. */
	RsslBuffer			sourceName;			/*!< Name of the source queue. */
	RsslBuffer			destName;			/*!< Name of the destination queue. */
	RsslBuffer			encDataBody;		/*!< The encoded contents of this message. */
	RsslInt64			identifier;			/*!< An identifier associated with this message. */
	RsslInt64			timeout;			/*!< A timeout for this message. Positive values indicate a numerical timeout, in millseconds. For special timeout codes, see RsslRDMQueueTimeoutCode. */
	RsslUInt16			queueDepth;			/*!< The number of messages remaining in the queue for this stream. */
	RsslGenericMsg		_encodeMsg;			/* For internal encoding. */
} RsslRDMQueueData;

/**
 * @brief Clears an RsslRDMQueueData.
 * @see RsslRDMQueueData
 */
RTR_C_INLINE void rsslClearRDMQueueData(RsslRDMQueueData *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMQueueData));
	pMsg->rdmMsgBase.rdmMsgType = RDM_QMSG_MT_DATA;
	pMsg->timeout = RDM_QMSG_TC_INFINITE;
}

/**
  * @brief Checks if an RsslRDMQueueData message may be a duplicate message.
  * @see RsslRDMQueueData
  */
RTR_C_INLINE RsslBool rsslRDMQueueDataCheckIsPossibleDuplicate(RsslRDMQueueData *pMsg)
{
	return (pMsg->flags & RDM_QMSG_DF_POSSIBLE_DUPLICATE);
}


/**
 * @brief The RDM Queue Ack.
 * @see RsslRDMMsgBase, RsslRDMQueueMsg, rsslClearRDMQueueAck
 */
typedef struct
{
	RsslRDMMsgBase				rdmMsgBase;	/*!< The base RDM message. */
	RsslInt64					identifier;	/*!< Identifier of the message acknowledged by this Queue Ack. */
	RsslBuffer					sourceName;	/*!< Source name for Ack message. */
	RsslBuffer					destName;	/*!< Destination name for Ack message. */
} RsslRDMQueueAck;

/**
 * @brief Clears an RsslRDMQueueAck.
 * @see RsslRDMQueueAck
 */
RTR_C_INLINE void rsslClearRDMQueueAck(RsslRDMQueueAck *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMQueueAck));
	pMsg->rdmMsgBase.rdmMsgType = RDM_QMSG_MT_ACK;
}

/**
 * @brief The RDM Queue Data Expired Message. NOTE: Encoding this message is not supported.
 * @see RsslRDMMsgBase, RsslRDMQueueMsg, rsslClearRDMQueueDataExpired
 */
typedef struct
{
	RsslRDMMsgBase		rdmMsgBase;			/*!< The base RDM message. */
	RsslUInt16			flags;				/*!< Flags associated with this message. See RsslRDMQueueDataFlags. These flags are read-only. */
	RsslUInt8			containerType;		/*!< Type of the contents of the encDataBody. See RsslDataTypes. */
	RsslBuffer			sourceName;			/*!< Name of the source queue. */
	RsslBuffer			destName;			/*!< Name of the destination queue. */
	RsslBuffer			encDataBody;		/*!< The encoded contents of this message. */
	RsslInt64			identifier;			/*!< An identifier associated with this message. */
	RsslUInt8			undeliverableCode;	/*!< This code indicates the reason for the expired message. See RsslRDMQueueUndeliverableCode. */
	RsslUInt16			queueDepth;			/*!< The number of messages remaining in the queue for this stream. */
} RsslRDMQueueDataExpired;

/**
 * @brief Clears an RsslRDMQueueDataExpired.
 * @see RsslRDMQueueDataExpired
 */
RTR_C_INLINE void rsslClearRDMQueueDataExpired(RsslRDMQueueDataExpired *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMQueueDataExpired));
	pMsg->rdmMsgBase.rdmMsgType = RDM_QMSG_MT_DATA_EXPIRED;
}

/**
  * @brief Checks if an RsslRDMQueueDataExpired message may be a duplicate message.
  * @see RsslRDMQueueDataExpired
  */
RTR_C_INLINE RsslBool rsslRDMQueueDataExpiredCheckIsPossibleDuplicate(RsslRDMQueueDataExpired *pMsg)
{
	return (pMsg->flags & RDM_QMSG_DF_POSSIBLE_DUPLICATE);
}



/**
 * @brief The RDM Queue Close.
 * @see RsslRDMMsgBase, RsslRDMQueueMsg, rsslClearRDMQueueClose
 */
typedef struct
{
	RsslRDMMsgBase	rdmMsgBase;	/*!< The base RDM message. */
} RsslRDMQueueClose;

/**
 * @brief Clears an RsslRDMQueueClose.
 * @see RsslRDMQueueClose
 */
RTR_C_INLINE void rsslClearRDMQueueClose(RsslRDMQueueClose *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMQueueClose));
	pMsg->rdmMsgBase.rdmMsgType = RDM_QMSG_MT_CLOSE;
}

/**
 * @brief The RDM Queue Msg. The Queue Message encoder and decoder functions expect this type.
 * It is a group of the classes of message that the Queue domain supports. Any specific message
 * class may be cast to an RsslRDMQueueMsg, and an RsslRDMQueueMsg may be cast to any specific
 * message class. The RsslRDMMsgBase contains members common to each class that may be used to
 * identify the class of message.
 * @see RsslRDMQueueMsgType, RsslRDMMsgBase, RsslRDMQueueRequest, RsslRDMQueueRefresh, RsslRDMQueueStatus, RsslRDMQueueData, RsslRDMQueueAck, RsslRDMQueueClose, rsslClearRDMQueueMsg
 */
typedef union
{
	RsslRDMMsgBase			rdmMsgBase; 	/*!< The base RDM message. */
	RsslRDMQueueRequest		request;		/*!< Queue Request */
	RsslRDMQueueRefresh		refresh;		/*!< Queue Refresh */
	RsslRDMQueueStatus		status;			/*!< Queue Status */
	RsslRDMQueueData		data;			/*!< Queue Data */
	RsslRDMQueueDataExpired	dataExpired;	/*!< Queue Data (Expired) */
	RsslRDMQueueAck			ack;			/*!< Queue Ack */
	RsslRDMQueueClose		close;			/*!< Queue Close */
} RsslRDMQueueMsg;

/**
 * @brief Clears an RsslRDMQueueMsg.
 * @see RsslRDMQueueMsg
 */
RTR_C_INLINE void rsslClearRDMQueueMsg(RsslRDMQueueMsg *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMQueueMsg));
}

/**
 * @brief Encodes an RsslRDMQueueMsg.
 * @param pEncodeIter The Encode Iterator.
 * @param pQueueMsg The RDM Queue Message to Encode.
 * @param pBytesWritten Returns the total number of bytes used to encode the message.
 * @param pError Error structure to be populated in the event of failure.
 * @see RsslRDMQueueMsg
 */
RSSL_VA_API RsslRet rsslEncodeRDMQueueMsg(RsslEncodeIterator *pEncodeIter, RsslRDMQueueMsg *pQueueMsg, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError);

/**
 * @brief Begins encoding an RsslRDMQueueMsg.
 * @param pEncodeIter The Encode Iterator.
 * @param pQueueMsg The RDM Queue Message to Encode.
 * @param pError Error structure to be populated in the event of failure.
 * @see RsslRDMQueueMsg
 */
RSSL_VA_API RsslRet rsslEncodeRDMQueueMsgInit(RsslEncodeIterator *pEncodeIter, RsslRDMQueueMsg *pQueueMsg, RsslErrorInfo *pError);

/**
 * @brief Completes encoding of an RsslRDMQueueMsg.
 * @param pEncodeIter The Encode Iterator.
 * @param success If true - successfully complete the message encoding, if false - roll back encoding.
 * @param pBytesWritten Returns the total number of bytes used to encode the message. If the success parameter is set to false, this is not set.
 * @param pError Error structure to be populated in the event of failure.
 * @see RsslRDMQueueMsg
 */
RSSL_VA_API RsslRet rsslEncodeRDMQueueMsgComplete(RsslEncodeIterator *pEncodeIter, RsslBool success, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError);

/**
 *	@}
 */

#ifdef __cplusplus
};
#endif

#endif
