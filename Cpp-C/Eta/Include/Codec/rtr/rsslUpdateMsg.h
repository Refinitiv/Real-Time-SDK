

#ifndef __RSSL_UPDATEMSG_H_
#define __RSSL_UPDATEMSG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslMsgBase.h"

/**
 * @addtogroup UpdateMsgStruct
 * @{
 */


/** 
 * @brief The RsslUpdateMsg is used by providers(both interactive and non-interactive) to convey changes to data associated with an item stream.
 *
 * @see RsslMsg, RsslMsgBase, RsslUpdateFlags, RSSL_INIT_UPDATE_MSG, rsslClearUpdateMsg
 */
typedef struct {
	RsslMsgBase			msgBase;		/*!< @brief Common message header members (streamId, domainType, msgKey, etc.), msgKey is optional and typically not sent as streamId can be used to match updates to a specific item stream */
	RsslUInt16			flags;			/*!< @brief Flag values used to indicate optional member presence and/or stream behavior.  The available options are defined by values present in  \ref RsslUpdateFlags. */
	RsslUInt8			updateType;		/*!< @brief Indicates domain-specific information about the type of content contained in this update. See rsslRDM.h for domain-specific enumerations for usage with the Thomson Reuters Domain Models. */
	RsslUInt32			seqNum;			/*!< @brief Sequence number intended to help with temporal ordering. Typically, this will be incremented with every message, but may have gaps depending on the sequencing algorithm being used.  Presence is indicated by \ref RsslUpdateFlags::RSSL_UPMF_HAS_SEQ_NUM. */
	RsslUInt16			conflationCount;/*!< @brief Number of updates conflated into this update.  A value of 0 means that this update is not conflated.  Presence is indicated by \ref RsslUpdateFlags::RSSL_UPMF_HAS_CONF_INFO. */
	RsslUInt16			conflationTime;	/*!< @brief Time period(in miliseconds) over which updates were conflated.  Presence is indicated by \ref RsslUpdateFlags::RSSL_UPMF_HAS_CONF_INFO. */
	RsslBuffer			permData;		/*!< @brief Contains permissioning information for this update.  Presence is indicated by \ref RsslUpdateFlags::RSSL_UPMF_HAS_PERM_DATA. */
	RsslPostUserInfo	postUserInfo;	/*!< @brief Information used to identify the user that posted this update to an upstream provider.  Presence is indicated by \ref RsslUpdateFlags::RSSL_UPMF_HAS_POST_USER_INFO */
	RsslBuffer			extendedHeader;	/*!< @brief Extended header attributes, used for per-domain defined attributes that are not used when determining stream uniqueness.  Presence is indicated by \ref RsslUpdatetFlags::RSSL_UPMF_HAS_EXTENDED_HEADER. */
} RsslUpdateMsg;

/**
 * @brief Static initializer for the RsslUpdateMsg
 *
 * @note Static clear function does not set RsslUpdateMsg::msgBase::msgClass, the rsslClearUpdateMsg() function does.  
 *
 * @warning On larger structures, like messages, the clear functions tend to outperform the static initializer.  It is recommended to use the clear function when initializing any messages.
 * @see RsslUpdateMsg, rsslClearUpdateMsg
 */

#define RSSL_INIT_UPDATE_MSG { RSSL_INIT_MSG_BASE, 0, 0, 0, 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_POST_USER_INFO, RSSL_INIT_BUFFER }


/**
 * @brief Clears RsslUpdateMsg
 * @see RsslUpdateMsg, RSSL_INIT_UPDATE_MSG
 */
RTR_C_ALWAYS_INLINE void rsslClearUpdateMsg(RsslUpdateMsg *pUpdateMsg) 
{
	memset(pUpdateMsg, 0, sizeof(RsslUpdateMsg));
	pUpdateMsg->msgBase.msgClass = 4;
}


/** 
 * @brief The Update Message flags (UPMF = UpdateMsg Flag)
 * @see RsslUpdateMsg, RsslMsg
 */
typedef enum {
	RSSL_UPMF_NONE					= 0x000,		/*!< (0x000) No RsslUpdateMsg flags are present */
	RSSL_UPMF_HAS_EXTENDED_HEADER	= 0x001,		/*!< (0x001) This RsslUpdateMsg has an extended header buffer, contained in \ref RsslUpdateMsg::extendedHeader. */
	RSSL_UPMF_HAS_PERM_DATA			= 0x002,		/*!< (0x002) This RsslUpdateMsg has permission information, contained in \ref RsslUpdateMsg::permData. */
	RSSL_UPMF_HAS_MSG_KEY			= 0x008,		/*!< (0x008) This RsslUpdateMsg has an RsslMsgKey included in the update, contained in \ref RsslUpdateMsg::msgBase::msgKey */
	RSSL_UPMF_HAS_SEQ_NUM			= 0x010,		/*!< (0x010) This RsslUpdateMsg contains a sequence number, contained in \ref RsslUpdateMsg::seqNum. */
	RSSL_UPMF_HAS_CONF_INFO			= 0x020,		/*!< (0x020) Indicates that this RsslUpdateMsg contains conflation information. This information is contained in \ref RsslUpdateMsg::conflationCount and RsslUpdateMsg::conflationTime. */
	RSSL_UPMF_DO_NOT_CACHE    		= 0x040,		/*!< (0x040) Indicates that this RsslUpdateMsg is transient and that this message's payload information should not be cached  */
	RSSL_UPMF_DO_NOT_CONFLATE		= 0x080,		/*!< (0x080) Indicates that this RsslUpdateMsg's payload information should not be conflated or aggregated. */
	RSSL_UPMF_DO_NOT_RIPPLE			= 0x100,		/*!< (0x100) Indicates that rippling should not be applied to this RsslUpdateMsg's contents. */
	RSSL_UPMF_HAS_POST_USER_INFO	= 0x200,		/*!< (0x200) This RsslUpdateMsg has been posted by the user identified in \ref RsslUpdateMsg::postUserInfo. */
	RSSL_UPMF_DISCARDABLE			= 0x400			/*!< (0x400) Indicates that this RsslUpdateMsg can be discarded.  This flag is commonly used with options that have no open interest. */	
} RsslUpdateFlags;

/** 
 * @brief General OMM strings associated with the different update message flags.
 * @see RsslUpdateFlags, rsslUpdateFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_UPMF_HAS_EXTENDED_HEADER = { 17, (char*)"HasExtendedHeader" };
static const RsslBuffer RSSL_OMMSTR_UPMF_HAS_PERM_DATA = { 11, (char*)"HasPermData" };
static const RsslBuffer RSSL_OMMSTR_UPMF_HAS_MSG_KEY = { 9, (char*)"HasMsgKey" };
static const RsslBuffer RSSL_OMMSTR_UPMF_HAS_SEQ_NUM = { 9, (char*)"HasSeqNum" };
static const RsslBuffer RSSL_OMMSTR_UPMF_HAS_CONF_INFO = { 11, (char*)"HasConfInfo" };
static const RsslBuffer RSSL_OMMSTR_UPMF_DO_NOT_CACHE = { 10, (char*)"DoNotCache" };
static const RsslBuffer RSSL_OMMSTR_UPMF_DO_NOT_CONFLATE = { 13, (char*)"DoNotConflate" };
static const RsslBuffer RSSL_OMMSTR_UPMF_DO_NOT_RIPPLE = { 11, (char*)"DoNotRipple" };
static const RsslBuffer RSSL_OMMSTR_UPMF_HAS_POST_USER_INFO = { 15, (char*)"HasPostUserInfo" };
static const RsslBuffer RSSL_OMMSTR_UPMF_DISCARDABLE = { 11, (char*)"Discardable" };

/**
 * @brief Provide general OMM string representation of RsslUpdateFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslUpdateFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslUpdateFlags
 */
RSSL_API RsslRet rsslUpdateFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags);

/**
 * @}
 */


/**
 *	@defgroup UpdateMsgHelpers RsslUpdateMsg Helper Functions
 *	@{
 */


/**
 * @brief Checks the presence of the ::RSSL_UPMF_HAS_EXTENDED_HEADER flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckHasExtendedHdr(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_HAS_EXTENDED_HEADER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_UPMF_HAS_MSG_KEY flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckHasPermData(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_HAS_PERM_DATA) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_UPMF_HAS_PERM_DATA flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckHasMsgKey(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_HAS_MSG_KEY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_UPMF_HAS_SEQ_NUM flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckHasSeqNum(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_HAS_SEQ_NUM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_UPMF_HAS_CONF_INFO flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckHasConfInfo(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_HAS_CONF_INFO) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_UPMF_DO_NOT_CACHE flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckDoNotCache(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_DO_NOT_CACHE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_UPMF_DO_NOT_CONFLATE flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckDoNotConflate(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_DO_NOT_CONFLATE) ? 
						RSSL_TRUE : RSSL_FALSE );
}
/**
 * @brief Checks the presence of the ::RSSL_UPMF_DO_NOT_RIPPLE flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckDoNotRipple(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_DO_NOT_RIPPLE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_UPMF_HAS_POST_USER_INFO flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckHasPostUserInfo(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_HAS_POST_USER_INFO) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_UPMF_DISCARDABLE flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslUpdateMsgCheckHasDiscardable(RsslUpdateMsg *pUpdateMsg)
{
	return ((pUpdateMsg->flags & RSSL_UPMF_DISCARDABLE) ?
						RSSL_TRUE : RSSL_FALSE );
}


/**
 * @brief Applies the ::RSSL_UPMF_HAS_EXTENDED_HEADER flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyHasExtendedHdr(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_HAS_EXTENDED_HEADER;
}

/**
 * @brief Applies the ::RSSL_UPMF_HAS_PERM_DATA flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyHasPermData(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_HAS_PERM_DATA;
}

/**
 * @brief Applies the ::RSSL_UPMF_HAS_MSG_KEY flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyHasMsgKey(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_HAS_MSG_KEY;
}

/**
 * @brief Applies the ::RSSL_UPMF_HAS_SEQ_NUM flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyHasSeqNum(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_HAS_SEQ_NUM;
}

/**
 * @brief Applies the ::RSSL_UPMF_HAS_CONF_INFO flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyHasConfInfo(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_HAS_CONF_INFO;
}

/**
 * @brief Applies the ::RSSL_UPMF_DO_NOT_CACHE flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyDoNotCache(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_DO_NOT_CACHE;
}

/**
 * @brief Applies the ::RSSL_UPMF_DO_NOT_CONFLATE flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyDoNotConflate(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_DO_NOT_CONFLATE;
}

/**
 * @brief Applies the ::RSSL_UPMF_DO_NOT_RIPPLE flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyDoNotRipple(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_DO_NOT_RIPPLE;
}

/**
 * @brief Applies the ::RSSL_UPMF_HAS_POST_USER_INFO flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyHasPostUserInfo(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_HAS_POST_USER_INFO;
}

/**
 * @brief Applies the ::RSSL_UPMF_DISCARDABLE flag on the given RsslUpdateMsg.
 *
 * @param pUpdateMsg Pointer to the update message.
 */
RTR_C_ALWAYS_INLINE void rsslUpdateMsgApplyDiscardable(RsslUpdateMsg *pUpdateMsg)
{
	pUpdateMsg->flags |= RSSL_UPMF_DISCARDABLE;
}

/**
 * @}
 */




#ifdef __cplusplus
}
#endif


#endif

