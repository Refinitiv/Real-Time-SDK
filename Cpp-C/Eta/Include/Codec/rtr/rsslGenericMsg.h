/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_GENERICMSG_H_
#define __RSSL_GENERICMSG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslMsgBase.h"


/**
 * @addtogroup GenericMsgStruct
 * @{
 */


/** 
 * @brief The RsslGenericMsg allows applications to bidirectionally send messages without any implied interaction semantics (e.g. no Request/Response paradigm).  
 *
 * @see RsslMsg, RsslMsgBase, RsslGenericFlags, RSSL_INIT_GENERIC_MSG, rsslClearGenericMsg
 */
typedef struct {
	RsslMsgBase		msgBase;			/*!< @brief Common message header members (streamId, domainType, msgKey, etc.).  The msgKey is optional. */
	RsslUInt16		flags;				/*!< @brief Flag values used to indicate optional member presence and/or stream behavior.  The available options are defined by values present in  \ref RsslGenericFlags. */
	RsslUInt16		partNum;			/*!< @brief Part Number is a 15-bit value typically used with multi-part generic messages. Value should start with 0 to indicate the initial part, each subsequent part in a multi-part generic message should increment the previous partNum by 1. */
	RsslUInt32		seqNum;				/*!< @brief Sequence number intended to help with temporal ordering. Typically, this will be incremented with every message, but may have gaps depending on the sequencing algorithm being used. Presence is indicated by \ref RsslGenericFlags::RSSL_GNMF_HAS_SEQ_NUM. */
	RsslUInt32		secondarySeqNum;	/*!< @brief Secondary Sequence Number often used as an acknowledgment sequence number. Presence is indicated by RsslGenericFlags::RSSL_GNMF_HAS_SECONDARY_SEQ_NUM.  */
	RsslBuffer		permData;			/*!< @brief Contains authorization information for this stream.  Presence is indicated by \ref RsslGenericFlags::RSSL_GNMF_HAS_PERM_DATA. */
	RsslBuffer		extendedHeader;		/*!< @brief Extended header attributes, used for per-domain defined attributes that are not used when determining stream uniqueness.  Presence is indicated by \ref RsslGenericFlags::RSSL_GNMF_HAS_EXTENDED_HEADER. */
	RsslMsgKey	 	reqMsgKey;			/*!< @brief Optional Key providing unique identifier information for an item stream. If present, this contains the original request message key and is used to uniquely identify a stream */
} RsslGenericMsg;

/**
 * @brief Static initializer for the RsslGenericMsg
 *
 * @note Static clear function does not set RsslGenericMsg::msgBase::msgClass, the rsslClearGenericMsg() function does.  
 *
 * @warning On larger structures, like messages, the clear functions tend to outperform the static initializer.  It is recommended to use the clear function when initializing any messages.
 * @see RsslGenericMsg, rsslClearGenericMsg
 */

#define RSSL_INIT_GENERIC_MSG { RSSL_INIT_MSG_BASE, 0, 0, 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_MSG_KEY }


/**
 * @brief Clears RsslGenericMsg
 * @see RsslGenericMsg, RSSL_INIT_GENERIC_MSG
 */
RTR_C_ALWAYS_INLINE void rsslClearGenericMsg(RsslGenericMsg *pGenericMsg) 
{
	memset(pGenericMsg, 0, sizeof(RsslGenericMsg));
	pGenericMsg->msgBase.msgClass = 7;
}


/** 
 * @brief The Generic Message flags (GNMF = GenericMsg Flags)
 * @see RsslGenericMsg, RsslMsg
 */
typedef enum {
	RSSL_GNMF_NONE					= 0x000,		/*!< (0x000) No RsslGenericFlags are present.  */
	RSSL_GNMF_HAS_EXTENDED_HEADER	= 0x001,		/*!< (0x001) This RsslGenericMsg has an extended header buffer, contained in RsslGenericMsg::extendedHeader.  */
	RSSL_GNMF_HAS_PERM_DATA			= 0x002,		/*!< (0x002) This RsslGenericMsg has permission information, contained in RsslGenericMsg::permData. */
	RSSL_GNMF_HAS_MSG_KEY			= 0x004,		/*!< (0x004) This RsslGenericMsg has a message key, contained in RsslGenericMsg::msgBase::msgKey.  */
	RSSL_GNMF_HAS_SEQ_NUM			= 0x008,		/*!< (0x008) This RsslGenericMsg has a sequence number, contained in RsslGenericMsg::seqNum. */
	RSSL_GNMF_MESSAGE_COMPLETE		= 0x010,		/*!< (0x010) Indicates that this RsslGenericMsg is the final part of a multi-part generic message. This flag should be set on both single-part generic messages, as well as the final message in a multi-part generic message sequence. */
	RSSL_GNMF_HAS_SECONDARY_SEQ_NUM	= 0x020,		/*!< (0x020) This RsslGenericMsg has a secondary sequence number, contained in RsslGenericMsg::secondarySeqNum */
	RSSL_GNMF_HAS_PART_NUM			= 0x040,		/*!< (0x040) This RsslGenericMsg has a part number, contained in RsslGenericMsg::partNum. */
	RSSL_GNMF_HAS_REQ_MSG_KEY		= 0x080			/*!< (0x080) The RsslGenericMsg has the original request's message key, contained in \ref RsslGenericMsg::msgBase::reqMsgKey*/
} RsslGenericFlags;

/** 
 * @brief General OMM strings associated with the different generic message flags.
 * @see RsslGenericFlags, rsslGenericFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_GNMF_HAS_EXTENDED_HEADER = { 17, (char*)"HasExtendedHeader" };
static const RsslBuffer RSSL_OMMSTR_GNMF_HAS_PERM_DATA = { 11, (char*)"HasPermData" };
static const RsslBuffer RSSL_OMMSTR_GNMF_HAS_MSG_KEY = { 9, (char*)"HasMsgKey" };
static const RsslBuffer RSSL_OMMSTR_GNMF_HAS_SEQ_NUM = { 9, (char*)"HasSeqNum" };
static const RsslBuffer RSSL_OMMSTR_GNMF_MESSAGE_COMPLETE = { 15, (char*)"MessageComplete" };
static const RsslBuffer RSSL_OMMSTR_GNMF_HAS_SECONDARY_SEQ_NUM = { 18, (char*)"HasSecondarySeqNum" };
static const RsslBuffer RSSL_OMMSTR_GNMF_HAS_PART_NUM = { 10, (char*)"HasPartNum" };
static const RsslBuffer RSSL_OMMSTR_GNMF_HAS_REQ_MSG_KEY = { 12, (char*)"HasReqMsgKey" };

/**
 * @brief Provide general OMM string representation of RsslGenericFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslGenericFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslGenericFlags
 */
RSSL_API RsslRet rsslGenericFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags);

/**
 * @}
 */


/**
 *	@defgroup GenericMsgHelpers RsslGenericMsg Helper Functions
 *	@{
 */
 
/**
 * @brief Checks the presence of the ::RSSL_GNMF_HAS_EXTENDED_HEADER flag on the given RsslGenericMsg.
 *
 * @param pGenericMsg Pointer to the generic message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslGenericMsgCheckHasExtendedHdr(RsslGenericMsg *pGenericMsg)
{
	return ((pGenericMsg->flags & RSSL_GNMF_HAS_EXTENDED_HEADER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_GNMF_HAS_PERM_DATA flag on the given RsslGenericMsg.
 *
 * @param pGenericMsg Pointer to the generic message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslGenericMsgCheckHasPermData(RsslGenericMsg *pGenericMsg)
{
	return ((pGenericMsg->flags & RSSL_GNMF_HAS_PERM_DATA) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_GNMF_HAS_MSG_KEY flag on the given RsslGenericMsg.
 *
 * @param pGenericMsg Pointer to the generic message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslGenericMsgCheckHasMsgKey(RsslGenericMsg *pGenericMsg)
{
	return ((pGenericMsg->flags & RSSL_GNMF_HAS_MSG_KEY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_GNMF_HAS_SEQ_NUM flag on the given RsslGenericMsg.
 *
 * @param pGenericMsg Pointer to the generic message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslGenericMsgCheckHasSeqNum(RsslGenericMsg *pGenericMsg)
{
	return ((pGenericMsg->flags & RSSL_GNMF_HAS_SEQ_NUM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_GNMF_HAS_PART_NUM flag on the given RsslGenericMsg.
 *
 * @param pGenericMsg Pointer to the generic message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslGenericMsgCheckHasPartNum(RsslGenericMsg *pGenericMsg)
{
	return ((pGenericMsg->flags & RSSL_GNMF_HAS_PART_NUM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_GNMF_MESSAGE_COMPLETE flag on the given RsslGenericMsg.
 *
 * @param pGenericMsg Pointer to the generic message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslGenericMsgCheckMessageComplete(RsslGenericMsg *pGenericMsg)
{
	return ((pGenericMsg->flags & RSSL_GNMF_MESSAGE_COMPLETE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_GNMF_HAS_SECONDARY_SEQ_NUM flag on the given RsslGenericMsg.
 *
 * @param pGenericMsg Pointer to the generic message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslGenericMsgCheckHasSecondarySeqNum(RsslGenericMsg *pGenericMsg)
{
	return ((pGenericMsg->flags & RSSL_GNMF_HAS_SECONDARY_SEQ_NUM) ? 
						RSSL_TRUE : RSSL_FALSE );
}
#define rsslGenericMsgCheckHasSecSeqNum rsslGenericMsgCheckHasSecondarySeqNum

/**
 * @brief Checks the presence of the ::RSSL_GNMF_HAS_REQ_MSG_KEY flag on the given RsslGenericMsg.
 *
 * @param pGenericMsg Pointer to the generic message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslGenericMsgCheckHasReqMsgKey(RsslGenericMsg *pGenericMsg)
{
	return ((pGenericMsg->flags & RSSL_GNMF_HAS_REQ_MSG_KEY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Applies the ::RSSL_GNMF_HAS_EXTENDED_HEADER flag on the given RsslGenericMsg.
 * 
 * @param pGenericMsg Pointer to the generic message.
 */
RTR_C_ALWAYS_INLINE void rsslGenericMsgApplyHasExtendedHdr(RsslGenericMsg *pGenericMsg)
{
	pGenericMsg->flags |= RSSL_GNMF_HAS_EXTENDED_HEADER;
}

/**
 * @brief Applies the ::RSSL_GNMF_HAS_PERM_DATA flag on the given RsslGenericMsg.
 * 
 * @param pGenericMsg Pointer to the generic message.
 */
RTR_C_ALWAYS_INLINE void rsslGenericMsgApplyHasPermData(RsslGenericMsg *pGenericMsg)
{
	pGenericMsg->flags |= RSSL_GNMF_HAS_PERM_DATA;
}

/**
 * @brief Applies the ::RSSL_GNMF_HAS_MSG_KEY flag on the given RsslGenericMsg.
 * 
 * @param pGenericMsg Pointer to the generic message.
 */
RTR_C_ALWAYS_INLINE void rsslGenericMsgApplyHasMsgKey(RsslGenericMsg *pGenericMsg)
{
	pGenericMsg->flags |= RSSL_GNMF_HAS_MSG_KEY;
}

/**
 * @brief Applies the ::RSSL_GNMF_HAS_SEQ_NUM flag on the given RsslGenericMsg.
 * 
 * @param pGenericMsg Pointer to the generic message.
 */
RTR_C_ALWAYS_INLINE void rsslGenericMsgApplyHasSeqNum(RsslGenericMsg *pGenericMsg)
{
	pGenericMsg->flags |= RSSL_GNMF_HAS_SEQ_NUM;
}

/**
 * @brief Applies the ::RSSL_GNMF_HAS_PART_NUM flag on the given RsslGenericMsg.
 * 
 * @param pGenericMsg Pointer to the generic message.
 */
RTR_C_ALWAYS_INLINE void rsslGenericMsgApplyHasPartNum(RsslGenericMsg *pGenericMsg)
{
	pGenericMsg->flags |= RSSL_GNMF_HAS_PART_NUM;
}


/**
 * @brief Applies the ::RSSL_GNMF_MESSAGE_COMPLETE flag on the given RsslGenericMsg.
 * 
 * @param pGenericMsg Pointer to the generic message.
 */
RTR_C_ALWAYS_INLINE void rsslGenericMsgApplyMessageComplete(RsslGenericMsg *pGenericMsg)
{
	pGenericMsg->flags |= RSSL_GNMF_MESSAGE_COMPLETE;
}

/**
 * @brief Applies the ::RSSL_GNMF_HAS_SECONDARY_SEQ_NUM flag on the given RsslGenericMsg.
 * 
 * @param pGenericMsg Pointer to the generic message.
 */
RTR_C_ALWAYS_INLINE void rsslGenericMsgApplyHasSecondarySeqNum(RsslGenericMsg *pGenericMsg)
{
	pGenericMsg->flags |= RSSL_GNMF_HAS_SECONDARY_SEQ_NUM;
}

/**
 * @brief Applies the ::RSSL_GNMF_HAS_REQ_MSG_KEY flag on the given RsslGenericMsg.
 * 
 * @param pGenericMsg Pointer to the generic message.
 */
RTR_C_ALWAYS_INLINE void rsslGenericMsgApplyHasReqMsgKey(RsslGenericMsg *pGenericMsg)
{
	pGenericMsg->flags |= RSSL_GNMF_HAS_REQ_MSG_KEY;
}

/**
* @brief Set the RSSL_GNMF_MESSAGE_COMPLETE flag on an encoded RsslGenericMsg buffer
* @see RsslGenericFlags
*/
RSSL_API RsslRet rsslSetGenericCompleteFlag(RsslEncodeIterator		*pIter);

/**
* @brief Unset the RSSL_GNMF_MESSAGE_COMPLETE flag on an encoded RsslGenericMsg buffer
* @see RsslGenericFlags
*/
RSSL_API RsslRet rsslUnsetGenericCompleteFlag(RsslEncodeIterator		*pIter);

#define rsslGenericMsgApplyHasSecSeqNum rsslGenericMsgApplyHasSecondarySeqNum

/**
 *	@}
 */



#ifdef __cplusplus
}
#endif


#endif

