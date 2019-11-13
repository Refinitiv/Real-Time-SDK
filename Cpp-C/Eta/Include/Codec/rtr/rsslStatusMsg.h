/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_STATUSMSG_H_
#define __RSSL_STATUSMSG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslMsgBase.h"


/**
 * @addtogroup StatusMsgStruct
 * @{
 */

/** 
 * @brief The RsslStatusMsg is used to convey state information, permission change, or group Id change information on a stream. An interactive or non-interactive providing application can also close a stream with a status message. 
 *
 * @see RsslMsg, RsslMsgBase, RsslStatusFlags, RSSL_INIT_STATUS_MSG
 */
typedef struct {
	RsslMsgBase		 msgBase;			/*!< @brief Common message header members (streamId, domainType, msgKey, etc.), msgKey is optional and typically not sent as streamId can be used to match updates to a specific item stream */
	RsslUInt16		 flags;				/*!< @brief Flag values used to indicate optional member presence and/or stream behavior.  The available options are defined by values present in  \ref RsslStatusFlags. */
	RsslState		 state;				/*!< @brief Contains stream or data state information. When present in an RsslStatusMsg, this usually indicates a change in the state of the item stream.  See \ref RsslState for more details.  Presence is indicated by \ref RsslStatusFlags::RSSL_STMF_HAS_STATE */
	RsslBuffer		 groupId;			/*!< @brief Group identifier.  This contains information about the item group this stream belongs to.  This is typically present on a status message when changing the item group associations.  Presence is indicated by \ref RsslStatusFlags::RSSL_STMF_HAS_GROUP_ID */
	RsslBuffer		 permData;			/*!< @brief Contains permission authorization information for this stream. This typically replaces the permission information received in a \ref RsslRefreshMsg::permData.  Presence is indicated by \ref RsslStatusFlags::RSSL_STMF_HAS_PERM_DATA. */ 
	RsslPostUserInfo postUserInfo;		/*!< @brief Information used to identify the user that posted this update to an upstream provider. Presence is indicated by \ref RsslStatusFlags::RSSL_STMF_HAS_POST_USER_INFO */
	RsslBuffer		 extendedHeader;	/*!< @brief Extended header attributes, used for per-domain defined attributes that are not used when determining stream uniqueness.  Presence is indicated by \ref RsslStatusFlags::RSSL_STMF_HAS_EXTENDED_HEADER. */
	RsslMsgKey	 	 reqMsgKey;			/*!< @brief Optional Key providing unique identifier information for an item stream. If present, this contains the original request message key and is used to uniquely identify a stream */
} RsslStatusMsg;


/**
 * @brief Static initializer for the RsslStatusMsg
 *
 * @note Static clear function does not set RsslStatusMsg::msgBase::msgClass, the rsslClearStatusMsg() function does.  
 *
 * @warning On larger structures, like messages, the clear functions tend to outperform the static initializer.  It is recommended to use the clear function when initializing any messages.
 * @see RsslStatusMsg, rsslClearStatusMsg
 */

#define RSSL_INIT_STATUS_MSG { RSSL_INIT_MSG_BASE, 0, RSSL_INIT_STATE, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER, RSSL_INIT_POST_USER_INFO, RSSL_INIT_BUFFER, RSSL_INIT_MSG_KEY}


/**
 * @brief Clears RsslStatusMsg
 * @see RsslStatusMsg, RSSL_INIT_STATUS_MSG
 */
RTR_C_ALWAYS_INLINE void rsslClearStatusMsg(RsslStatusMsg *pStatusMsg)  
{
	memset(pStatusMsg, 0, sizeof(RsslStatusMsg));
	pStatusMsg->msgBase.msgClass = 3;
}


/** 
 * @brief The Status Message flags (STMF = StatusMsg Flag)
 * @see RsslStatusMsg, RsslMsg
 */
typedef enum {
	RSSL_STMF_NONE					= 0x000,		/*!< (0x000) No RsslStateMsg flags are present  */
	RSSL_STMF_HAS_EXTENDED_HEADER	= 0x001,		/*!< (0x001) Indicates that this RsslStatusMsg has an extended header buffer, contained in RsslStatusMsg::extendedHeader. */
	RSSL_STMF_HAS_PERM_DATA			= 0x002,		/*!< (0x002) Indicates that this RsslStatusMsg has permission data, contained in RsslStatusMsg::permData. */
	RSSL_STMF_HAS_MSG_KEY			= 0x008,		/*!< (0x008) Indicates that this RsslStatusMsg has a msgKey, contained in RsslStatusMsg::msgBase::msgKey.  */
	RSSL_STMF_HAS_GROUP_ID			= 0x010,		/*!< (0x010) Indicates that this RsslStatusMsg has a GroupId, contained in RsslStatusMsg::GroupId */
	RSSL_STMF_HAS_STATE	    		= 0x020,		/*!< (0x020) Indicates that this RsslStatusMsg has stream or group state information, contained in RsslStatusMsg::state.  */
	RSSL_STMF_CLEAR_CACHE			= 0x040,		/*!< (0x040) Indicates that all cached header or payload data associated with this item stream should be cleared. */	
	RSSL_STMF_PRIVATE_STREAM		= 0x080,		/*!< (0x080) Indicates that this RsslStatusMsg is an acknowledgement of a private stream establishment.  Or, if \ref RsslStatusMsg::state::streamState's value is \ref RSSL_STREAM_REDIRECTED, the presence of this flag indicates that the current stream can only be opened as a private stream. */
	RSSL_STMF_HAS_POST_USER_INFO	= 0x100,		/*!< (0x100) Indicates that this RsslStatusMsg has post user info, contained in RsslStatusMsg::postUserInfo. */
	RSSL_STMF_HAS_REQ_MSG_KEY		= 0x200,		/*!< (0x200) Indicates that this RsslStatusMsg has the original request's message key, contained in \ref RsslStatusMsg::msgBase::reqMsgKey*/
	RSSL_STMF_QUALIFIED_STREAM		= 0x400			/*!< (0x400) Indicates that this RsslStatusMsg is an acknowledgement of a qualified stream establishment. */
} RsslStatusFlags;

/** 
 * @brief General OMM strings associated with the different status message flags.
 * @see RsslStatusFlags, rsslStatusFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_STMF_HAS_EXTENDED_HEADER = { 17, (char*)"HasExtendedHeader" };
static const RsslBuffer RSSL_OMMSTR_STMF_HAS_PERM_DATA = { 11, (char*)"HasPermData" };
static const RsslBuffer RSSL_OMMSTR_STMF_HAS_MSG_KEY = { 9, (char*)"HasMsgKey" };
static const RsslBuffer RSSL_OMMSTR_STMF_HAS_GROUP_ID = { 10, (char*)"HasGroupID" };
static const RsslBuffer RSSL_OMMSTR_STMF_HAS_STATE = { 8, (char*)"HasState" };
static const RsslBuffer RSSL_OMMSTR_STMF_CLEAR_CACHE = { 10, (char*)"ClearCache" };
static const RsslBuffer RSSL_OMMSTR_STMF_PRIVATE_STREAM = { 13, (char*)"PrivateStream" };
static const RsslBuffer RSSL_OMMSTR_STMF_HAS_POST_USER_INFO = { 15, (char*)"HasPostUserInfo" };
static const RsslBuffer RSSL_OMMSTR_STMF_HAS_REQ_MSG_KEY = { 12, (char*)"HasReqMsgKey" };
static const RsslBuffer RSSL_OMMSTR_STMF_QUALIFIED_STREAM = { 15, (char*)"QualifiedStream" };

/**
 * @brief Provide general OMM string representation of RsslStatusFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslStatusFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslStatusFlags
 */
RSSL_API RsslRet rsslStatusFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags);

/**
 * @}
 */


/**
 *	@defgroup StatusMsgHelpers RsslStatusMsg Helper Functions
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_STMF_HAS_EXTENDED_HEADER flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckHasExtendedHdr(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_HAS_EXTENDED_HEADER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_STMF_HAS_PERM_DATA flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckHasPermData(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_HAS_PERM_DATA) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_STMF_HAS_MSG_KEY flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckHasMsgKey(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_HAS_MSG_KEY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_STMF_HAS_GROUP_ID flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckHasGroupId(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_HAS_GROUP_ID) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_STMF_HAS_STATE flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckHasState(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_HAS_STATE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_STMF_CLEAR_CACHE flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckClearCache(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_CLEAR_CACHE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_STMF_PRIVATE_STREAM flag on the given RsslStatusMsg.
 *
 *  @param pStatusMsg Pointer to the status message.
 *  @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckPrivateStream(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_PRIVATE_STREAM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_STMF_HAS_POST_USER_INFO flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckHasPostUserInfo(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_HAS_POST_USER_INFO) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_STMF_HAS_REQ_MSG_KEY flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckHasReqMsgKey(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_HAS_REQ_MSG_KEY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_STMF_QUALIFIED_STREAM flag on the given RsslStatusMsg.
 *
 *  @param pStatusMsg Pointer to the status message.
 *  @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslStatusMsgCheckQualifiedStream(RsslStatusMsg *pStatusMsg)
{
	return ((pStatusMsg->flags & RSSL_STMF_QUALIFIED_STREAM) ? 
						RSSL_TRUE : RSSL_FALSE );
}


/**
 * @brief Applies the ::RSSL_STMF_HAS_EXTENDED_HEADER flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyHasExtendedHdr(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_HAS_EXTENDED_HEADER;
}

/**
 * @brief Applies the ::RSSL_STMF_HAS_PERM_DATA flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyHasPermData(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_HAS_PERM_DATA;
}

/**
 * @brief Applies the ::RSSL_STMF_HAS_MSG_KEY flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyHasMsgKey(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_HAS_MSG_KEY;
}

/**
 * @brief Applies the ::RSSL_STMF_HAS_GROUP_ID flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyHasGroupId(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_HAS_GROUP_ID;
}

/**
 * @brief Applies the ::RSSL_STMF_HAS_STATE flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyHasState(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_HAS_STATE;
}

/**
 * @brief Applies the ::RSSL_STMF_CLEAR_CACHE flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyClearCache(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_CLEAR_CACHE;
}

/**
 * @brief Applies the ::RSSL_STMF_PRIVATE_STREAM flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyPrivateStream(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_PRIVATE_STREAM;
}

/**
 * @brief Applies the ::RSSL_STMF_HAS_POST_USER_INFO flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyHasPostUserInfo(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_HAS_POST_USER_INFO;
}

/**
 * @brief Applies the ::RSSL_STMF_HAS_REQ_MSG_KEY flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyHasReqMsgKey(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_HAS_REQ_MSG_KEY;
}

/**
 * @brief Applies the ::RSSL_STMF_QUALIFIED_STREAM flag on the given RsslStatusMsg.
 *
 * @param pStatusMsg Pointer to the status message.
 */
RTR_C_ALWAYS_INLINE void rsslStatusMsgApplyQualifiedStream(RsslStatusMsg *pStatusMsg)
{
	pStatusMsg->flags |= RSSL_STMF_QUALIFIED_STREAM;
}

/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif

