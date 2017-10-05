

#ifndef __RSSL_ACKMSG_H_
#define __RSSL_ACKMSG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslMsgBase.h"

/**
 * @addtogroup AckMsgStruct
 * @{
 */



/** 
 * @brief The RsslAckMsg is used to acknowledge success/failure or completion of an RsslPostMsg or RsslCloseMsg.  
 * @see RsslMsg, RsslMsgBase, RsslCloseMsg, RsslPostMsg, RsslAckFlags, RSSL_INIT_ACK_MSG, rsslClearAckMsg
 */
typedef struct {
	RsslMsgBase		msgBase;		/*!< @brief Common message header members (streamId, domainType, msgKey, etc.), msgKey is optional and typically not sent as streamId can be used to match updates to a specific item stream  */
	RsslUInt16		flags;			/*!< @brief Flag values are used to indicate optional member presence.  The available options are defined by values present in  \ref RsslAckFlags. */
	RsslUInt32		ackId;			/*!< @brief ID used to associate this Ack with the message it is acknowledging. */
	RsslUInt8		nakCode;		/*!< @brief This enumeration indicates the reason for a negative acknowledgement(NAK). See \ref RsslNakCodes for values. Presence is indicated by \ref RsslAckFlags::RSSL_AKMF_HAS_NAK_CODE.  */
	RsslUInt32		seqNum;			/*!< @brief Sequence number intended to help with temporal ordering. Typically, this will be incremented with every message, but may have gaps depending on the sequencing algorithm being used. Presence is indicated by \ref RsslAckFlags::RSSL_AKMF_HAS_SEQ_NUM. */
	RsslBuffer		text;			/*!< @brief This provides additional information about the acceptance or rejection of the message being acknowledged. */
	RsslBuffer		extendedHeader;	/*!< @brief Extended header attributes, used for per-domain defined attributes that are not used when determining stream uniqueness.  Presence is indicated by \ref RsslAckFlags::RSSL_AKMF_HAS_EXTENDED_HEADER. */
} RsslAckMsg;

/**
 * @brief Static initializer for the RsslAckMsg
 *
 * @note Static clear function does not set RsslAckMsg::msgBase::msgClass, the rsslClearAckMsg() function does.  
 *
 * @warning On larger structures, like messages, the clear functions tend to outperform the static initializer.  It is recommended to use the clear function when initializing any messages.
 * @see RsslAckMsg, rsslClearAckMsg
 */

#define RSSL_INIT_ACK_MSG { RSSL_INIT_MSG_BASE, 0, 0, 0, 0, RSSL_INIT_BUFFER, RSSL_INIT_BUFFER }

/**
 * @brief Clears RsslAckMsg
 * @see RsslAckMsg, RSSL_INIT_ACK_MSG
 */
RTR_C_ALWAYS_INLINE void rsslClearAckMsg(RsslAckMsg *pAckMsg)
{
	memset(pAckMsg, 0, sizeof(RsslAckMsg));
	pAckMsg->msgBase.msgClass = 6;
}



/** 
 * @brief The Acknowledgement Message flags (AKMF = AckMsg Flag)
 * @see RsslAckMsg, RsslMsg
 */
typedef enum {
	RSSL_AKMF_NONE					= 0x00,		/*!< (0x00) No Flags have been set  */
	RSSL_AKMF_HAS_EXTENDED_HEADER	= 0x01,		/*!< (0x01) Indicates that this RsslAckMsg has an extended header buffer, contained in RsslAckMsg::extendedHeader. */
	RSSL_AKMF_HAS_TEXT 				= 0x02,		/*!< (0x02) Indicates that this RsslAckMsg has a text buffer, contained in \ref RsslAckMsg::text.  */
	RSSL_AKMF_PRIVATE_STREAM		= 0x04,		/*!< (0x04) Indicates that this RsslAckMsg acknowledges the establishment of a private stream */
	RSSL_AKMF_HAS_SEQ_NUM			= 0x08,		/*!< (0x08) Indicates that this RsslAckMsg has a sequence number, contained in \ref RsslAckMsg::seqNum.  */
	RSSL_AKMF_HAS_MSG_KEY			= 0x10,		/*!< (0x10) Indicates that this RsslAckMsg has a Message Key, contained in \ref RsslAckMsg::msgBase::msgKey. */
	RSSL_AKMF_HAS_NAK_CODE			= 0x20,		/*!< (0x20) Indicates that this RsslAckMsg has a NAK Code, contained in \ref RsslAckMsg::nakCode.  See \ref RsslNakCodes. */
	RSSL_AKMF_QUALIFIED_STREAM		= 0x40		/*!< (0x40) Indicates that this RsslAckMsg acknowledges the establishment of a qualified stream request. */
} RsslAckFlags;

/** 
 * @brief General OMM strings associated with the different ack message flags.
 * @see RsslAckFlags, rsslAckFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_AKMF_HAS_EXTENDED_HEADER = { 17, (char*)"HasExtendedHeader" };
static const RsslBuffer RSSL_OMMSTR_AKMF_HAS_TEXT = { 7, (char*)"HasText" };
static const RsslBuffer RSSL_OMMSTR_AKMF_PRIVATE_STREAM = { 13, (char*)"PrivateStream" };
static const RsslBuffer RSSL_OMMSTR_AKMF_HAS_SEQ_NUM = { 9, (char*)"HasSeqNum" };
static const RsslBuffer RSSL_OMMSTR_AKMF_HAS_MSG_KEY = { 9, (char*)"HasMsgKey" };
static const RsslBuffer RSSL_OMMSTR_AKMF_HAS_NAK_CODE = { 10, (char*)"HasNakCode" };
static const RsslBuffer RSSL_OMMSTR_AKMF_QUALIFIED_STREAM = { 15, (char*)"QualifiedStream" };

/**
 * @brief Provide general OMM string representation of RsslAckFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslAckFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslAckFlags
 */
RSSL_API RsslRet rsslAckFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags);

/** 
 * @brief The Ack Message nakCodes, used to indicate a reason for a negative acknowledgment (NAKC = Nak Code)
 * @see RsslAckMsg, RsslMsg
 */
typedef enum {
	RSSL_NAKC_NONE			   = 0,		/*!< (0) No Nak Code */
	RSSL_NAKC_ACCESS_DENIED    = 1,		/*!< (1) Access Denied. The user not properly permissioned for posting on the item or service. */
	RSSL_NAKC_DENIED_BY_SRC    = 2,		/*!< (2) Denied by source. The source being posted to has denied accepting this post message. */
	RSSL_NAKC_SOURCE_DOWN      = 3,		/*!< (3) Source Down. Source being posted to is down or not available. */
	RSSL_NAKC_SOURCE_UNKNOWN   = 4,		/*!< (4) Source Unknown. The source being posted to is unknown and is unreachable. */
	RSSL_NAKC_NO_RESOURCES     = 5,		/*!< (5) No Resources. A component along the path of the post message does not have appropriate resources available to continue processing the post. */
	RSSL_NAKC_NO_RESPONSE      = 6,		/*!< (6) No Response. This code may mean that the source is unavailable or there is a delay in processing the posted information. */
	RSSL_NAKC_GATEWAY_DOWN	   = 7,     /*!< (7) Gateway is Down. The gateway device for handling posted or contributed information is down or not available. */
										/*!< (8) Reserved */
										/*!< (9) Reserved */
	RSSL_NAKC_SYMBOL_UNKNOWN   = 10,	/*!< (10) Unknown Symbol. The item information provided within the post message is not recognized by the system. */
	RSSL_NAKC_NOT_OPEN         = 11,	/*!< (11) Item not open. The item being posted to does not have an available stream open. */
	RSSL_NAKC_INVALID_CONTENT  = 12		/*!< (12) Nak being sent due to invalid content. The content of the post message is invalid and cannot be posted, it does not match the expected formatting for this post. */
} RsslNakCodes;

/** 
 * @brief General OMM strings associated with the different nak codes.
 * @see RsslNakCodes, rsslNakCodeToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_NAKC_NONE = { 4, (char*)"None" };
static const RsslBuffer RSSL_OMMSTR_NAKC_ACCESS_DENIED = { 12, (char*)"AccessDenied" };
static const RsslBuffer RSSL_OMMSTR_NAKC_DENIED_BY_SRC = { 11, (char*)"DeniedBySrc" };
static const RsslBuffer RSSL_OMMSTR_NAKC_SOURCE_DOWN = { 10, (char*)"SourceDown" };
static const RsslBuffer RSSL_OMMSTR_NAKC_SOURCE_UNKNOWN = { 13, (char*)"SourceUnknown" };
static const RsslBuffer RSSL_OMMSTR_NAKC_NO_RESOURCES = { 11, (char*)"NoResources" };
static const RsslBuffer RSSL_OMMSTR_NAKC_NO_RESPONSE = { 10, (char*)"NoResponse" };
static const RsslBuffer RSSL_OMMSTR_NAKC_GATEWAY_DOWN = { 11, (char*)"GatewayDown" };
static const RsslBuffer RSSL_OMMSTR_NAKC_SYMBOL_UNKNOWN = { 13, (char*)"SymbolUnknown" };
static const RsslBuffer RSSL_OMMSTR_NAKC_NOT_OPEN = { 7, (char*)"NotOpen" };
static const RsslBuffer RSSL_OMMSTR_NAKC_INVALID_CONTENT = { 14, (char*)"InvalidContent" };

/**
 * @brief Provide a general OMM string representation for an AckMsg NakCode enumeartion
 * @see RsslAckMsg, RsslNakCodes
 */
RSSL_API const char* rsslNakCodeToOmmString(RsslUInt8 code);


/**
 * @}
 */


/**
 *	@defgroup AckMsgHelpers RsslAckMsg Helper Functions
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_AKMF_HAS_EXTENDED_HEADER flag on the given RsslAckMsg.
 *
 * @param pAckMsg Pointer to the acknowledgement message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslAckMsgCheckHasExtendedHdr(RsslAckMsg *pAckMsg)
{
	return ((pAckMsg->flags & RSSL_AKMF_HAS_EXTENDED_HEADER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_AKMF_HAS_TEXT flag on the given RsslAckMsg.
 *
 * @param pAckMsg Pointer to the acknowledgement message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslAckMsgCheckHasText(RsslAckMsg *pAckMsg)
{
	return ((pAckMsg->flags & RSSL_AKMF_HAS_TEXT) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_AKMF_PRIVATE_STREAM flag on the given RsslAckMsg.
 *
 *  @param pAckMsg Pointer to the acknowledgement message.
 *  @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 **/
RTR_C_ALWAYS_INLINE RsslBool rsslAckMsgCheckPrivateStream(RsslAckMsg *pAckMsg)
{
	return ((pAckMsg->flags & RSSL_AKMF_PRIVATE_STREAM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_AKMF_HAS_SEQ_NUM flag on the given RsslAckMsg.
 *
 * @param pAckMsg Pointer to the acknowledgement message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslAckMsgCheckHasSeqNum(RsslAckMsg *pAckMsg)
{
	return ((pAckMsg->flags & RSSL_AKMF_HAS_SEQ_NUM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_AKMF_HAS_MSG_KEY flag on the given RsslAckMsg.
 *
 * @param pAckMsg Pointer to the acknowledgement message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslAckMsgCheckHasMsgKey(RsslAckMsg *pAckMsg)
{
	return ((pAckMsg->flags & RSSL_AKMF_HAS_MSG_KEY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_AKMF_HAS_NAK_CODE flag on the given RsslAckMsg.
 *
 * @param pAckMsg Pointer to the acknowledgement message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslAckMsgCheckHasNakCode(RsslAckMsg *pAckMsg)
{
	return ((pAckMsg->flags & RSSL_AKMF_HAS_NAK_CODE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_AKMF_QUALIFIED_STREAM flag on the given RsslAckMsg.
 *
 * @param pAckMsg Pointer to the acknowledgement message.
 * @return RSSL_TRUE - if exists; RSSL_FALSE if does not exist.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslAckMsgCheckQualifiedStream(RsslAckMsg *pAckMsg)
{
	return ((pAckMsg->flags & RSSL_AKMF_QUALIFIED_STREAM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Applies the ::RSSL_AKMF_HAS_EXTENDED_HEADER flag on the given RsslAckMsg.
 * 
 * @param pAckMsg Pointer to the acknowledgement message.
 */
RTR_C_ALWAYS_INLINE void rsslAckMsgApplyHasExtendedHdr(RsslAckMsg *pAckMsg)
{
	pAckMsg->flags |= RSSL_AKMF_HAS_EXTENDED_HEADER;
}

/**
 * @brief Applies the ::RSSL_AKMF_HAS_TEXT flag on the given RsslAckMsg.
 * 
 * @param pAckMsg Pointer to the acknowledgement message.
 */
RTR_C_ALWAYS_INLINE void rsslAckMsgApplyHasText(RsslAckMsg *pAckMsg)
{
	pAckMsg->flags |= RSSL_AKMF_HAS_TEXT;
}

/**
 * @brief Applies the ::RSSL_AKMF_PRIVATE_STREAM flag on the given RsslAckMsg.
 * 
 * @param pAckMsg Pointer to the acknowledgement message.
 **/
RTR_C_ALWAYS_INLINE void rsslAckMsgApplyPrivateStream(RsslAckMsg *pAckMsg)
{
	pAckMsg->flags |= RSSL_AKMF_PRIVATE_STREAM;
}

/**
 * @brief Applies the ::RSSL_AKMF_HAS_SEQ_NUM flag on the given RsslAckMsg.
 * 
 * @param pAckMsg Pointer to the acknowledgement message.
 */
RTR_C_ALWAYS_INLINE void rsslAckMsgApplyHasSeqNum(RsslAckMsg *pAckMsg)
{
	pAckMsg->flags |= RSSL_AKMF_HAS_SEQ_NUM;
}

/**
 * @brief Applies the ::RSSL_AKMF_HAS_MSG_KEY flag on the given RsslAckMsg.
 * 
 * @param pAckMsg Pointer to the acknowledgement message.
 */
RTR_C_ALWAYS_INLINE void rsslAckMsgApplyHasMsgKey(RsslAckMsg *pAckMsg)
{
	pAckMsg->flags |= RSSL_AKMF_HAS_MSG_KEY;
}

/**
 * @brief Applies the ::RSSL_AKMF_HAS_NAK_CODE flag on the given RsslAckMsg.
 * 
 * @param pAckMsg Pointer to the acknowledgement message.
 */
RTR_C_ALWAYS_INLINE void rsslAckMsgApplyHasNakCode(RsslAckMsg *pAckMsg)
{
	pAckMsg->flags |= RSSL_AKMF_HAS_NAK_CODE;
}

/**
 * @brief Applies the ::RSSL_AKMF_QUALIFIED_STREAM flag on the given RsslAckMsg.
 * 
 * @param pAckMsg Pointer to the acknowledgement message.
 */
RTR_C_ALWAYS_INLINE void rsslAckMsgApplyQualifiedStream(RsslAckMsg *pAckMsg)
{
	pAckMsg->flags |= RSSL_AKMF_QUALIFIED_STREAM;
}

/**
 * @}
 */



#ifdef __cplusplus
}
#endif


#endif

