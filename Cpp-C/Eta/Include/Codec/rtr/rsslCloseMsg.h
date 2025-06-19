/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2017-2019,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_CLOSEMSG_H_
#define __RSSL_CLOSEMSG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslMsgBase.h"

/**
 * @addtogroup CloseMsgStruct
 * @{
 */

/** 
 * @brief The RsslCloseMsg is used to indicate that a consuming application is no longer interested in content on the stream being closed.
 * @see RsslMsg, RsslMsgBase, RSSL_INIT_CLOSE_MSG, rsslClearCloseMsg
 */
typedef struct {
	RsslMsgBase		msgBase;		/*!< @brief Common message header members (streamId, domainType, msgKey, etc.). */
	RsslUInt16		flags;			/*!< @brief Flag values are used to indicate optional member presence.  The available options are defined by values present in  \ref RsslCloseFlags. */
	RsslBuffer		extendedHeader;	/*!< @brief Extended header attributes, used for per-domain defined attributes that are not used when determining stream uniqueness.  Presence is indicated by \ref RsslCloseFlags::RSSL_CLMF_HAS_EXTENDED_HEADER. */
} RsslCloseMsg;

/**
 * @brief Static initializer for the RsslCloseMsg
 *
 * @note Static clear function does not set RsslCloseMsg::msgBase::msgClass, the rsslClearCloseMsg() function does.  
 *
 * @warning On larger structures, like messages, the clear functions tend to outperform the static initializer.  It is recommended to use the clear function when initializing any messages.
 * @see RsslCloseMsg, rsslClearCloseMsg
 */

#define RSSL_INIT_CLOSE_MSG { RSSL_INIT_MSG_BASE, 0, RSSL_INIT_BUFFER }


/**
 * @brief Clears RsslCloseMsg
 * @see RsslCloseMsg, RSSL_INIT_CLOSE_MSG
 */
RTR_C_ALWAYS_INLINE void rsslClearCloseMsg(RsslCloseMsg *pCloseMsg) 
{
	memset(pCloseMsg, 0, sizeof(RsslCloseMsg));
	pCloseMsg->msgBase.msgClass = 5;
}


/** 
 * @brief The Close Message flags (CLMF = CloseMsg Flags)
 * @see RsslCloseMsg, RsslMsg
 */
typedef enum {
	RSSL_CLMF_NONE					= 0x00,		/*!< (0x00) No RsslCloseMsg flags are present. */
	RSSL_CLMF_HAS_EXTENDED_HEADER	= 0x01,		/*!< (0x01) The RsslCloseMsg has an extended header bffer, contained in RsslCloseMsg::extendedHeader. */
	RSSL_CLMF_ACK					= 0x02,		/*!< (0x02) Indicates that the consumer that is sending this RsslCloseMsg wants the provider to send an RsslAckMsg to indicate that this RsslCloseMsg has been received, processed correctly, and the stream has been properly closed. */				
	RSSL_CLMF_HAS_BATCH				= 0x04		/*!< (0x04) Support batching with header flags. */ 
} RsslCloseFlags;

/** 
 * @brief General OMM strings associated with the different close message flags.
 * @see RsslCloseFlags, rsslCloseFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_CLMF_HAS_EXTENDED_HEADER = { 17, (char*)"HasExtendedHeader" };
static const RsslBuffer RSSL_OMMSTR_CLMF_ACK = { 3, (char*)"Ack" };
static const RsslBuffer RSSL_OMMSTR_CLMF_HAS_BATCH = { 8, (char*)"HasBatch" };

/**
 * @brief Provide general OMM string representation of RsslCloseFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslCloseFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslCloseFlags
 */
RSSL_API RsslRet rsslCloseFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags);

/**
 * @}
 */


/**
 *	@defgroup CloseMsgHelpers RsslCloseMsg Helper Functions
 *	@{
 */


/**
 * @brief Checks the presence of the ::RSSL_CLMF_HAS_EXTENDED_HEADER flag on the given RsslCloseMsg.
 *
 * @param pCloseMsg Pointer to the close message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslCloseMsgCheckHasExtendedHdr(RsslCloseMsg *pCloseMsg)
{
	return ((pCloseMsg->flags & RSSL_CLMF_HAS_EXTENDED_HEADER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_CLMF_ACK flag on the given RsslCloseMsg.
 *
 * @param pCloseMsg Pointer to the close message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslCloseMsgCheckAck(RsslCloseMsg *pCloseMsg)
{
	return ((pCloseMsg->flags & RSSL_CLMF_ACK) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_CLMF_HAS_BATCH flag on the given RsslCloseMsg.
 *
 * @param pCloseMsg Pointer to the close message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslCloseMsgCheckHasBatch(RsslCloseMsg *pCloseMsg)
{
	return ((pCloseMsg->flags & RSSL_CLMF_HAS_BATCH) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Applies the ::RSSL_CLMF_HAS_EXTENDED_HEADER flag on the given RsslCloseMsg.
 * 
 * @param pCloseMsg Pointer to the close message.
 */
RTR_C_ALWAYS_INLINE void rsslCloseMsgSetHasExtendedHdr(RsslCloseMsg *pCloseMsg)
{
	pCloseMsg->flags |= RSSL_CLMF_HAS_EXTENDED_HEADER;
}

/**
 * @brief Applies the ::RSSL_CLMF_ACK flag on the given RsslCloseMsg.
 * 
 * @param pCloseMsg Pointer to the close message.
 */
RTR_C_ALWAYS_INLINE void rsslCloseMsgSetAck(RsslCloseMsg *pCloseMsg)
{
	pCloseMsg->flags |= RSSL_CLMF_ACK;
}

/**
 * @brief Applies the ::RSSL_CLMF_HAS_BATCH flag on the given RsslCloseMsg.
 * 
 * @param pCloseMsg Pointer to the close message.
 */
RTR_C_ALWAYS_INLINE void rsslCloseMsgSetHasBatch(RsslCloseMsg *pCloseMsg)
{
	pCloseMsg->flags |= RSSL_CLMF_HAS_BATCH;
}

/**
 *	@}
 */




#ifdef __cplusplus
}
#endif


#endif

