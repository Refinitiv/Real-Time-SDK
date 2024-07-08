/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_POSTMSG_H_
#define __RSSL_POSTMSG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslMsgBase.h"


/**
 * @addtogroup PostMsgStruct
 * @{
 */



/** 
 * @brief The RsslPostMsg is used to contribute content from a consumer into the platform.  The post message payload can contain another OMM message, OMM container, or other opaque content.  
 * @see RsslMsg, RsslMsgBase, RsslPostFlags, RSSL_INIT_POST_MSG, rsslClearPostMsg
 */
typedef struct {
	RsslMsgBase		 msgBase;		/*!< @brief Common message header members (streamId, domainType, msgKey, etc.).  The msgKey is optional for on-stream posting, as the streamId is used to identify the stream. However, for off-stream posting, the msgKey is typically required. */
	RsslUInt16		 flags;			/*!< @brief Flag values used to indicate optional member presence and/or stream behavior. The available options are defined by values present in \ref RsslPostFlags. */
	RsslUInt16		 partNum;		/*!< @brief Part number is a 15-bit value typically used with multi-part refresh messages, value should start with 0 to indicate the initial part, each subsequent part in a multi-part refresh message should increment the previous partNum by 1. Presence is indicated by \ref RsslPostFlags::RSSL_PSMF_HAS_PART_NUM */
	RsslUInt16       postUserRights;/*!< @brief Conveys the rights or abilities of the user posting this content.  See \ref RsslPostUserRights for the specific values and their meaning. Presence is indicated by \ref RsslPostFlags::RSSL_PSMF_HAS_POST_USER_RIGHTS */
	RsslUInt32		 seqNum;		/*!< @brief Sequence number intended to help with temporal ordering. Typically, this will be incremented with every message, but may have gaps depending on the sequencing algorithm being used. Presence is indicated by \ref RsslPostFlags::RSSL_PSMF_HAS_SEQ_NUM. */
	RsslUInt32		 postId;		/*!< @brief This is a consumer-assigned identifier that is used by upstream devices to distinguish different post messages. Each RsslPostMsg in a multi-part post must use the same postId value. Presence is indicated by \ref RsslPostFlags::RSSL_PSMF_HAS_POST_ID. */
	RsslPostUserInfo postUserInfo;	/*!< @brief Information used to identify the user that posted this content to an upstream provider. */
	RsslBuffer		 permData;		/*!< @brief Contains permission authorization information for this RsslPostMsg. Presence is indicated by \ref RsslPostFlags::RSSL_PSMF_HAS_PERM_DATA. */ 
	RsslBuffer		 extendedHeader;/*!< @brief Extended header attributes, used for per-domain defined attributes that are not used when determining stream uniqueness.  Presence is indicated by \ref RsslPostFlags::RSSL_PSMF_HAS_EXTENDED_HEADER. */
} RsslPostMsg;

/**
 * @brief The RSSL Post Message initializer
 * @see RsslPostMsg, rsslClearPostMsg
 */

#define RSSL_INIT_POST_MSG { RSSL_INIT_MSG_BASE, 0, 0, 0, 0, 0, RSSL_INIT_POST_USER_INFO, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER }


/**
 * @brief Clears RsslPostMsg
 * @see RsslPostMsg, RSSL_INIT_POST_MSG
 */
RTR_C_ALWAYS_INLINE void rsslClearPostMsg(RsslPostMsg *pPostMsg)
{
	memset(pPostMsg, 0, sizeof(RsslPostMsg));
	pPostMsg->msgBase.msgClass = 8;
}



/** 
 * @brief The Post Message flags (PSMF - PostMsg Flag)
 * @see RsslPostMsg, RsslMsg
 */
typedef enum {
	RSSL_PSMF_NONE					= 0x000,    /*!< (0x000) No RsslPostMsg flags are present */
	RSSL_PSMF_HAS_EXTENDED_HEADER	= 0x001,	/*!< (0x001) This RsslPostMsg has an extended header buffer, contained in \ref RsslPostMsg::extendedHeader.  */
	RSSL_PSMF_HAS_POST_ID			= 0x002,    /*!< (0x002) This RsslPostMsg has a Post ID, contained in \ref RsslPostMsg::postId. */
	RSSL_PSMF_HAS_MSG_KEY			= 0x004,    /*!< (0x004) This RsslPostMsg has a message key, contained in \ref RsslPostMsg::msgBase::msgKey. */
    RSSL_PSMF_HAS_SEQ_NUM			= 0x008,    /*!< (0x008) This RsslPostMsg has a sequence number, contained in \ref RsslPostMsg:: */
   	RSSL_PSMF_POST_COMPLETE			= 0x020,	/*!< (0x020) Indicates that this RsslPostMsg is the final part of the post message. This flag should also be set on single-part post messages, as well as the final message in a multi-part RsslPostMsg sequence. */
	RSSL_PSMF_ACK					= 0x040,	/*!< (0x040) Indicates that the provider receiving this RsslPostMsg should send an RsslAckMsg to indicate that this RsslPostMsg has been received and processed. The provider must include the \ref RsslPostMsg::postId in the RsslAckMsg's \ref RsslAckMsg::ackId. */
	RSSL_PSMF_HAS_PERM_DATA			= 0x080,	/*!< (0x080) This RsslPostMsg has permission authorization data, contained in \ref RsslPostMsg::permData. */
	RSSL_PSMF_HAS_PART_NUM			= 0x100,	/*!< (0x100) This RsslPostMsg has a part number, contained in \ref RsslPostMsg::partNum. */
	RSSL_PSMF_HAS_POST_USER_RIGHTS  = 0x200     /*!< (0x200) This RsslPostMsg has post user rights, contained in \ref RsslPostMsg::postUserRights. */
} RsslPostFlags;

/** 
 * @brief General OMM strings associated with the different post message flags
 * @see RsslPostFlags, rsslPostFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_PSMF_HAS_EXTENDED_HEADER = { 17, (char*)"HasExtendedHeader" };
static const RsslBuffer RSSL_OMMSTR_PSMF_HAS_POST_ID = { 9, (char*)"HasPostID" };
static const RsslBuffer RSSL_OMMSTR_PSMF_HAS_MSG_KEY = { 9, (char*)"HasMsgKey" };
static const RsslBuffer RSSL_OMMSTR_PSMF_HAS_SEQ_NUM = { 9, (char*)"HasSeqNum" };
static const RsslBuffer RSSL_OMMSTR_PSMF_POST_COMPLETE = { 12, (char*)"PostComplete" };
static const RsslBuffer RSSL_OMMSTR_PSMF_ACK = { 3, (char*)"Ack" };
static const RsslBuffer RSSL_OMMSTR_PSMF_HAS_PERM_DATA = { 11, (char*)"HasPermData" };
static const RsslBuffer RSSL_OMMSTR_PSMF_HAS_PART_NUM = { 10, (char*)"HasPartNum" };
static const RsslBuffer RSSL_OMMSTR_PSMF_HAS_POST_USER_RIGHTS = { 17, (char*)"HasPostUserRights" };

/**
 * @brief Provide general OMM string representation of RsslPostFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslPostFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslPostFlags
 */
RSSL_API RsslRet rsslPostFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags);

/** 
 * @brief The Post User Rights values (PSUR - Post User Rights).  These can be combined to allow for multiple rights. 
 * @see RsslPostMsg, RsslMsg
 */
typedef enum {
	RSSL_PSUR_NONE					= 0x00, 	/*!< (0x00) No user rights */
	RSSL_PSUR_CREATE				= 0x01, 	/*!< (0x01) User is allowed to create records in cache with this post */
	RSSL_PSUR_DELETE				= 0x02, 	/*!< (0x02) User is allowed to delete/remove records from cache with this post */
	RSSL_PSUR_MODIFY_PERM			= 0x04		/*!< (0x04) User is allowed to modify the permData for records already in cache with this post */
} RsslPostUserRights;

/** 
 * @brief General OMM strings associated with the different post user rights.
 * @see RsslPostUserRights, rsslPostUserRightsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_PSUR_CREATE = { 6, (char*)"Create" };
static const RsslBuffer RSSL_OMMSTR_PSUR_DELETE = { 6, (char*)"Delete" };
static const RsslBuffer RSSL_OMMSTR_PSUR_MODIFY_PERM = { 10, (char*)"ModifyPerm" };

/**
 * @brief Provide general OMM string representation of RsslPostUserRights
 * If multiple rights are set, they will be separated by a '|' delimiter.
 * Unrecognized rights will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param rights RsslPostUserRights value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslPostUserRights
 */
RSSL_API RsslRet rsslPostUserRightsToOmmString(RsslBuffer *oBuffer, RsslUInt16 rights);

/**
 * @}
 */


/**
 *	@defgroup PostMsgHelpers RsslPostMsg Helper Functions
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_PSMF_HAS_EXTENDED_HEADER flag on the given RsslPostMsg.
 *
 * @param pPostMsg Pointer to the post message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslPostMsgCheckHasExtendedHdr(RsslPostMsg *pPostMsg)
{
	return ((pPostMsg->flags & RSSL_PSMF_HAS_EXTENDED_HEADER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_PSMF_HAS_POST_ID flag on the given RsslPostMsg.
 *
 * @param pPostMsg Pointer to the post message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslPostMsgCheckHasPostId(RsslPostMsg *pPostMsg)
{
	return ((pPostMsg->flags & RSSL_PSMF_HAS_POST_ID) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_PSMF_HAS_MSG_KEY flag on the given RsslPostMsg.
 *
 * @param pPostMsg Pointer to the post message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslPostMsgCheckHasMsgKey(RsslPostMsg *pPostMsg)
{
	return ((pPostMsg->flags & RSSL_PSMF_HAS_MSG_KEY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_PSMF_HAS_SEQ_NUM flag on the given RsslPostMsg.
 *
 * @param pPostMsg Pointer to the post message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslPostMsgCheckHasSeqNum(RsslPostMsg *pPostMsg)
{
	return ((pPostMsg->flags & RSSL_PSMF_HAS_SEQ_NUM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_PSMF_HAS_PART_NUM flag on the given RsslPostMsg.
 *
 * @param pPostMsg Pointer to the post message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslPostMsgCheckHasPartNum(RsslPostMsg *pPostMsg)
{
	return ((pPostMsg->flags & RSSL_PSMF_HAS_PART_NUM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_PSMF_HAS_POST_USER_RIGHTS flag on the given RsslPostMsg.
 *
 * @param pPostMsg Pointer to the post message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslPostMsgCheckHasPostUserRights(RsslPostMsg *pPostMsg)
{
	return ((pPostMsg->flags & RSSL_PSMF_HAS_POST_USER_RIGHTS) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_PSMF_POST_COMPLETE flag on the given RsslPostMsg.
 *
 * @param pPostMsg Pointer to the post message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslPostMsgCheckPostComplete(RsslPostMsg *pPostMsg)
{
	return ((pPostMsg->flags & RSSL_PSMF_POST_COMPLETE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_PSMF_POST_COMPLETE flag on the given RsslPostMsg.
 *
 * @param pPostMsg Pointer to the close message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslPostMsgCheckAck(RsslPostMsg *pPostMsg)
{
	return ((pPostMsg->flags & RSSL_PSMF_ACK) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_PSMF_HAS_PERM_DATA flag on the given RsslPostMsg.
 *
 * @param pPostMsg Pointer to the post message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslPostMsgCheckHasPermData(RsslPostMsg *pPostMsg)
{
	return ((pPostMsg->flags & RSSL_PSMF_HAS_PERM_DATA) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Applies the ::RSSL_PSMF_HAS_EXTENDED_HEADER flag on the given RsslCloseMsg.
 * 
 * @param pPostMsg Pointer to the post message.
 */
RTR_C_ALWAYS_INLINE void rsslPostMsgApplyHasExtendedHdr(RsslPostMsg *pPostMsg)
{
	pPostMsg->flags |= RSSL_PSMF_HAS_EXTENDED_HEADER;
}

/**
 * @brief Applies the ::RSSL_PSMF_HAS_POST_ID flag on the given RsslCloseMsg.
 *
 * @param pPostMsg Pointer to the post message.
 */
RTR_C_ALWAYS_INLINE void rsslPostMsgApplyHasPostId(RsslPostMsg *pPostMsg)
{
	pPostMsg->flags |= RSSL_PSMF_HAS_POST_ID;
}

/**
 * @brief Applies the ::RSSL_PSMF_HAS_MSG_KEY flag on the given RsslCloseMsg.
 *
 * @param pPostMsg Pointer to the post message.
 */
RTR_C_ALWAYS_INLINE void rsslPostMsgApplyHasMsgKey(RsslPostMsg *pPostMsg)
{
	pPostMsg->flags |= RSSL_PSMF_HAS_MSG_KEY;
}

/**
 * @brief Applies the ::RSSL_PSMF_HAS_SEQ_NUM flag on the given RsslCloseMsg.
 *
 * @param pPostMsg Pointer to the post message.
 */
RTR_C_ALWAYS_INLINE void rsslPostMsgApplyHasSeqNum(RsslPostMsg *pPostMsg)
{
	pPostMsg->flags |= RSSL_PSMF_HAS_SEQ_NUM;
}

/**
 * @brief Applies the ::RSSL_PSMF_HAS_PART_NUM flag on the given RsslCloseMsg.
 *
 * @param pPostMsg Pointer to the post message.
 */
RTR_C_ALWAYS_INLINE void rsslPostMsgApplyHasPartNum(RsslPostMsg *pPostMsg)
{
	pPostMsg->flags |= RSSL_PSMF_HAS_PART_NUM;
}

/**
 * @brief Applies the ::RSSL_PSMF_HAS_POST_USER_RIGHTS flag on the given RsslCloseMsg.
 *
 * @param pPostMsg Pointer to the post message.
 */
RTR_C_ALWAYS_INLINE void rsslPostMsgApplyHasPostUserRights(RsslPostMsg *pPostMsg)
{
	pPostMsg->flags |= RSSL_PSMF_HAS_POST_USER_RIGHTS;
}

/**
 * @brief Applies the ::RSSL_PSMF_POST_COMPLETE flag on the given RsslCloseMsg.
 *
 * @param pPostMsg Pointer to the post message.
 */
RTR_C_ALWAYS_INLINE void rsslPostMsgApplyPostComplete(RsslPostMsg *pPostMsg)
{
	pPostMsg->flags |= RSSL_PSMF_POST_COMPLETE;
}

/**
 * @brief Applies the ::RSSL_PSMF_ACK flag on the given RsslCloseMsg.
 *
 * @param pPostMsg Pointer to the close message.
 */
RTR_C_ALWAYS_INLINE void rsslPostMsgApplyAck(RsslPostMsg *pPostMsg)
{
	pPostMsg->flags |= RSSL_PSMF_ACK;
}

/**
 * @brief Applies the ::RSSL_PSMF_HAS_PERM_DATA flag on the given RsslCloseMsg.
 *
 * @param pPostMsg Pointer to the post message.
 */
RTR_C_ALWAYS_INLINE void rsslPostMsgApplyHasPermData(RsslPostMsg *pPostMsg)
{
	pPostMsg->flags |= RSSL_PSMF_HAS_PERM_DATA;
}

/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif

