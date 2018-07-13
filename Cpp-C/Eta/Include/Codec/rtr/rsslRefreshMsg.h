/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_REFRESHMSG_H_
#define __RSSL_REFRESHMSG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslMsgBase.h"

/**
 * @addtogroup RefreshMsgStruct
 * @{
 */


/** 
 * @brief The RsslRefreshMsg is provided as an initial response or when an upstream source requires a data resynchronization point.
 *
 * @see RsslMsg, RsslMsgBase, RsslRefreshFlags, RSSL_INIT_REFRESH_MSG,
 * rsslClearRefreshMsg
 */
typedef struct {
	RsslMsgBase		 msgBase;		/*!< @brief Common message header members (streamId, domainType, msgKey, etc.). The msgKey is optional and typically not sent as StreamId is used to determine the specific item stream. */
	RsslUInt16		 flags;			/*!< @brief Flag values used to indicate optional member presence and/or stream behavior. The available options are defined by values present in  \ref RsslRefreshFlags. */
	RsslUInt16	 	 partNum;		/*!< @brief Part number is a 15-bit value typically used with multi-part refresh messages, value should start with 0 to indicate the initial part, each subsequent part in a multi-part refresh message should increment the previous partNum by 1. Presence is indicated by \ref RsslRefreshFlags::RSSL_RFMF_HAS_PART_NUM */
	RsslUInt32		 seqNum;		/*!< @brief Sequence number intended to help with temporal ordering. Typically, this will be incremented with every message, but may have gaps depending on the sequencing algorithm being used. Presence is indicated by \ref RsslRefreshFlags::RSSL_RFMF_HAS_SEQ_NUM. */
	RsslState		 state;			/*!< @brief Contains state information for the current item stream.  See \ref RsslState for more details. */
	RsslBuffer		 groupId;		/*!< @brief Group identifier.  This contains information about the item group this stream belongs to. This typically contains a sequence of two-byte integers, where system components can append additional values to ensure uniqueness. */
	RsslBuffer		 permData;		/*!< @brief Contains permission authorization information for all content provided on this stream. Presence is indicated by \ref RsslRefreshFlags::RSSL_RFMF_HAS_PERM_DATA. */ 
	RsslPostUserInfo postUserInfo;	/*!< @brief Information used to identify the user that posted this update to an upstream provider. Presence is indicated by \ref RsslRefreshFlags::RSSL_RFMF_HAS_POST_USER_INFO */
	RsslQos		 	 qos;			/*!< @brief Quality of Service information for the current item stream. See \ref RsslQos for more details.  Presence is indicated by \ref RsslRefreshFlags::RSSL_RFMF_HAS_QOS.  */
	RsslBuffer		 extendedHeader;/*!< @brief Extended header attributes, used for per-domain defined attributes that are not used when determining stream uniqueness.  Presence is indicated by \ref RsslRefreshFlags::RSSL_RFMF_HAS_EXTENDED_HEADER. */
	RsslMsgKey	 	 reqMsgKey;		/*!< @brief Optional Key providing unique identifier information for an item stream. If present, this contains the original request message key and is used to uniquely identify a stream */
} RsslRefreshMsg;

/**
 * @brief Static initializer for the RsslRefreshMsg
 *
 * @note Static clear function does not set RsslRefreshMsg::msgBase::msgClass, the rsslClearRefreshMsg() function does.  
 *
 * @warning On larger structures, like messages, the clear functions tend to outperform the static initializer.  It is recommended to use the clear function when initializing any messages.
 * @see RsslRefreshMsg, rsslClearRefreshMsg
 */

#define RSSL_INIT_REFRESH_MSG { RSSL_INIT_MSG_BASE, 0, 0, 0, RSSL_INIT_STATE, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_POST_USER_INFO, RSSL_INIT_QOS, RSSL_INIT_BUFFER, RSSL_INIT_MSG_KEY }


/**
 * @brief Clears RsslRefreshMsg
 * @see RsslRefreshMsg, RSSL_INIT_REFRESH_MSG, rsslClearStatusMsg
 */
RTR_C_ALWAYS_INLINE void rsslClearRefreshMsg(RsslRefreshMsg *pRefreshMsg)
{
	memset(pRefreshMsg, 0, sizeof(RsslRefreshMsg));
	pRefreshMsg->msgBase.msgClass = 2;
}


/** 
 * @brief The Refresh Message flags (RFMF = RefreshMsg Flag)
 * @see RsslRefreshMsg, RsslMsg
 */
typedef enum {
	RSSL_RFMF_NONE					= 0x0000,		/*!< (0x0000) No RsslRefreshMsg flags are present */
	RSSL_RFMF_HAS_EXTENDED_HEADER	= 0x0001,		/*!< (0x0001) The RsslRefreshMsg has an extended header buffer, contained in \ref RsslRefreshMsg::extendedHeader.  */
	RSSL_RFMF_HAS_PERM_DATA			= 0x0002,		/*!< (0x0002) The RsslRefreshMsg has permission authorization information, contained in RsslRefreshMsg::permData. */
	RSSL_RFMF_HAS_MSG_KEY			= 0x0008,		/*!< (0x0008) The RsslRefreshMsg has a message key, contained in \ref RsslRefreshMsg::msgBase::msgKey. */
	RSSL_RFMF_HAS_SEQ_NUM			= 0x0010,		/*!< (0x0010) The RsslRefreshMsg has a sequence number, contained in \ref RsslRefreshMsg::seqNum. */
	RSSL_RFMF_SOLICITED				= 0x0020,		/*!< (0x0020) Indicates that this RsslRefreshMsg is a solicited response to a consumer's request. */
	RSSL_RFMF_REFRESH_COMPLETE		= 0x0040,		/*!< (0x0040) Indicates that this is the final part of a refresh. This flag should be set on both single-part response messages, as well as the final message in a multi-part response message sequence. */
	RSSL_RFMF_HAS_QOS				= 0x0080,		/*!< (0x0080) The RsslRefreshMsg has quality of service information, contained in RsslRefreshMsg::qos. */
	RSSL_RFMF_CLEAR_CACHE			= 0x0100,		/*!< (0x0100) Indicates that any cached header or payload information associated with the RsslRefreshMsg's item stream should be cleared. */
	RSSL_RFMF_DO_NOT_CACHE	   		= 0x0200,		/*!< (0x0200) Indicates that this RsslRefreshMsg is transient and should not be cached.  */
	RSSL_RFMF_PRIVATE_STREAM		= 0x0400,		/*!< (0x0400) Indicates that this RsslRefreshMsg is an acknowledgement of a private stream request. Or, if \ref RsslRefreshMsg::state::streamState's value is \ref RSSL_STREAM_REDIRECTED, the presence of this flag indicates that the current stream can only be opened as a private stream. */
	RSSL_RFMF_HAS_POST_USER_INFO	= 0x0800,		/*!< (0x0800) The RsslRefreshMsg contains data posted by the user with the identifying information contained in \ref RsslRefreshMsg::postUserInfo */
	RSSL_RFMF_HAS_PART_NUM			= 0x1000,		/*!< (0x1000) The RsslRefreshMsg has a part number, contained in \ref RsslRefreshMsg::partNum. */
	RSSL_RFMF_HAS_REQ_MSG_KEY		= 0x2000,		/*!< (0x2000) The RsslRefreshMsg has the original request's message key, contained in \ref RsslRefreshMsg::reqMsgBase::msgKey*/
	RSSL_RFMF_QUALIFIED_STREAM		= 0x4000		/*!< (0x4000) Indicates that this RsslRefreshMsg is an acknowledgement of a qualified stream request. */
} RsslRefreshFlags;

/** 
 * @brief General OMM strings associated with the different refresh message flags
 * @see RsslRefreshFlags, rsslRefreshFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_RFMF_HAS_EXTENDED_HEADER = { 17, (char*)"HasExtendedHeader" };
static const RsslBuffer RSSL_OMMSTR_RFMF_HAS_PERM_DATA = { 11, (char*)"HasPermData" };
static const RsslBuffer RSSL_OMMSTR_RFMF_HAS_MSG_KEY = { 9, (char*)"HasMsgKey" };
static const RsslBuffer RSSL_OMMSTR_RFMF_HAS_SEQ_NUM = { 9, (char*)"HasSeqNum" };
static const RsslBuffer RSSL_OMMSTR_RFMF_SOLICITED = { 9, (char*)"Solicited" };
static const RsslBuffer RSSL_OMMSTR_RFMF_REFRESH_COMPLETE = { 15, (char*)"RefreshComplete" };
static const RsslBuffer RSSL_OMMSTR_RFMF_HAS_QOS = { 6, (char*)"HasQos" };
static const RsslBuffer RSSL_OMMSTR_RFMF_CLEAR_CACHE = { 10, (char*)"ClearCache" };
static const RsslBuffer RSSL_OMMSTR_RFMF_DO_NOT_CACHE = { 10, (char*)"DoNotCache" };
static const RsslBuffer RSSL_OMMSTR_RFMF_PRIVATE_STREAM = { 13, (char*)"PrivateStream" };
static const RsslBuffer RSSL_OMMSTR_RFMF_HAS_POST_USER_INFO = { 15, (char*)"HasPostUserInfo" };
static const RsslBuffer RSSL_OMMSTR_RFMF_HAS_PART_NUM = { 10, (char*)"HasPartNum" };
static const RsslBuffer RSSL_OMMSTR_RFMF_HAS_REQ_MSG_KEY = { 12, (char*)"HasReqMsgKey" };
static const RsslBuffer RSSL_OMMSTR_RFMF_QUALIFIED_STREAM = { 15, (char*)"QualifiedStream" };

/**
 * @brief Provide general OMM string representation of RsslRefreshFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslRefreshFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslRefreshFlags
 */
RSSL_API RsslRet rsslRefreshFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags);

/**
 * @}
 */


/**
 *	@defgroup RefreshMsgHelpers RsslRefreshMsg Helper Functions
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_RFMF_HAS_EXTENDED_HEADER flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckHasExtendedHdr(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_HAS_EXTENDED_HEADER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_HAS_PERM_DATA flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckHasPermData(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_HAS_PERM_DATA) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_HAS_MSG_KEY flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckHasMsgKey(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_HAS_MSG_KEY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_HAS_SEQ_NUM flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckHasSeqNum(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_HAS_SEQ_NUM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_HAS_PART_NUM flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckHasPartNum(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_HAS_PART_NUM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_SOLICITED flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckSolicited(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_SOLICITED) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_REFRESH_COMPLETE flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckRefreshComplete(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_REFRESH_COMPLETE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_HAS_QOS flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckHasQos(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_HAS_QOS) ? 
						RSSL_TRUE : RSSL_FALSE );
}
#define rsslRefreshMsgCheckHasQoS rsslRefreshMsgCheckHasQos
/**
 * @brief Checks the presence of the ::RSSL_RFMF_CLEAR_CACHE flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckClearCache(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_CLEAR_CACHE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_DO_NOT_CACHE flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckDoNotCache(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_DO_NOT_CACHE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_PRIVATE_STREAM flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckPrivateStream(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_PRIVATE_STREAM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_HAS_POST_USER_INFO flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckHasPostUserInfo(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_HAS_POST_USER_INFO) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_HAS_REQ_MSG_KEY flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckHasReqMsgKey(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_HAS_REQ_MSG_KEY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RFMF_QUALIFIED_STREAM flag on the given RsslRefreshMsg.
 *
 * @param pRefreshMsg Pointer to the refresh message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRefreshMsgCheckQualifiedStream(RsslRefreshMsg *pRefreshMsg)
{
	return ((pRefreshMsg->flags & RSSL_RFMF_QUALIFIED_STREAM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Applies the ::RSSL_RFMF_HAS_EXTENDED_HEADER flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyHasExtendedHdr(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_HAS_EXTENDED_HEADER;
}

/**
 * @brief Applies the ::RSSL_RFMF_HAS_PERM_DATA flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyHasPermData(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_HAS_PERM_DATA;
}

/**
 * @brief Applies the ::RSSL_RFMF_HAS_MSG_KEY flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyHasMsgKey(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_HAS_MSG_KEY;
}

/**
 * @brief Applies the ::RSSL_RFMF_HAS_SEQ_NUM flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyHasSeqNum(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_HAS_SEQ_NUM;
}

/**
 * @brief Applies the ::RSSL_RFMF_HAS_PART_NUM flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyHasPartNum(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_HAS_PART_NUM;
}


/**
 * @brief Applies the ::RSSL_RFMF_SOLICITED flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplySolicited(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_SOLICITED;
}

/**
 * @brief Applies the ::RSSL_RFMF_REFRESH_COMPLETE flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyRefreshComplete(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_REFRESH_COMPLETE;
}

/**
 * @brief Applies the ::RSSL_RFMF_HAS_QOS flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyHasQos(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_HAS_QOS;
}
#define rsslRefreshMsgApplyHasQoS rsslRefreshMsgApplyHasQos

/**
 * @brief Applies the ::RSSL_RFMF_CLEAR_CACHE flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyClearCache(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_CLEAR_CACHE;
}

/**
 * @brief Applies the ::RSSL_RFMF_DO_NOT_CACHE flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyDoNotCache(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_DO_NOT_CACHE;
}

/**
 * @brief Applies the ::RSSL_RFMF_PRIVATE_STREAM flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyPrivateStream(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_PRIVATE_STREAM;
}

/**
 * @brief Applies the ::RSSL_RFMF_HAS_POST_USER_INFO flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyHasPostUserInfo(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_HAS_POST_USER_INFO;
}

/**
 * @brief Applies the ::RSSL_RFMF_HAS_REQ_MSG_KEY flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyHasReqMsgKey(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_HAS_REQ_MSG_KEY;
}

/**
 * @brief Applies the ::RSSL_RFMF_QUALIFIED_STREAM flag on the given RsslRefreshMsg.
 * 
 * @param pRefreshMsg Pointer to the refresh message.
 */
RTR_C_ALWAYS_INLINE void rsslRefreshMsgApplyQualifiedStream(RsslRefreshMsg *pRefreshMsg)
{
	pRefreshMsg->flags |= RSSL_RFMF_QUALIFIED_STREAM;
}

/** 
 * @brief Set the RSSL_RFMF_SOLICITED flag on an encoded RsslRefreshMsg buffer
 * @see RsslRefreshFlags
 */
RSSL_API RsslRet rsslSetSolicitedFlag( RsslEncodeIterator		*pIter );

/** 
 * @brief Unset the RSSL_RFMF_SOLICITED flag on an encoded RsslRefreshMsg buffer
 * @see RsslRefreshFlags
 */
RSSL_API RsslRet rsslUnsetSolicitedFlag( RsslEncodeIterator		*pIter );


/** 
 * @brief Set the RSSL_RFMF_REFRESH_COMPLETE flag on an encoded RsslRefreshMsg buffer
 * @see RsslRefreshFlags
 */
RSSL_API RsslRet rsslSetRefreshCompleteFlag( RsslEncodeIterator		*pIter );

/** 
 * @brief Unset the RSSL_RFMF_REFRESH_COMPLETE flag on an encoded RsslRefreshMsg buffer
 * @see RsslRefreshFlags
 */
RSSL_API RsslRet rsslUnsetRefreshCompleteFlag( RsslEncodeIterator		*pIter );


/**
 * @}
 */



#ifdef __cplusplus
}
#endif


#endif

