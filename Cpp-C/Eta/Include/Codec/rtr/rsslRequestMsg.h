/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_REQUESTMSG_H_
#define __RSSL_REQUESTMSG_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "rtr/rsslMsgBase.h"


/**
 * @addtogroup ReqMsgStruct
 * @{
 */



/** 
 * @brief The RsslRequestMsg is used to provide an initial or subsequent data synchronization point, including quality of service and payload content.  When used to respond to an RsslRequestMsg, a solicited RsslRefreshMsg can be used.  If pushing out a content change to downstream applications, an unsolicited RsslRefreshMsg can be used.  When the stream is instantiated because of an RsslRequestMsg, the streamId from the request should be used; when a provider instantiates the stream with an RsslRefreshMsg (e.g. Non-Interactive provider), a negative streamId should be used.
 *
 * @note An RsslRequestMsg can be used to specify batch or view information on a stream or to pause/resume content flow on a stream. 
 * 
 * @see RsslMsg, RsslMsgBase, RsslMsgKey, RsslRequestFlags, RSSL_INIT_REQUEST_MSG, rsslClearRequestMsg
 */
typedef struct {
	RsslMsgBase		msgBase;		/*!< @brief Common message header members (streamId, domainType, msgKey, etc.).  The msgKey is required on an initial request, but is not necessary on reissue requests (changes to an already open stream's properties).   */
	RsslUInt16		flags;			/*!< @brief Flag values used to indicate optional member presence and/or stream behavior.  The available options are defined by values present in  \ref RsslRequestFlags. */
	RsslUInt8		priorityClass;	/*!< @brief The priority class associated with this stream, used to indicate the stream's general importance to the consumer.  A higher class value indicates higher level of importance.  Presence is indicated by \ref RsslRequestFlags::RSSL_RQMF_HAS_PRIORITY.    */
	RsslUInt16		priorityCount;	/*!< @brief Indicates the count at the specified RsslRequestMsg::priorityClass, indicating importance within the class level.  Presence is indicated by \ref RsslRequestFlags::RSSL_RQMF_HAS_PRIORITY.    */
	RsslQos			qos;			/*!< @brief Quality of Service information used to request a specific QoS or, when specified along with RsslRequestMsg::worstQos, a QoS range.  Presence is indicated by \ref RsslRequestFlags::RSSL_RQMF_HAS_QOS.  */
	RsslQos			worstQos;		/*!< @brief Quality of Service information used to specify the least desirable QoS within a range.  Presence is indicated by \ref RsslRequestFlags::RSSL_RQMF_HAS_WORST_QOS.  */
	RsslBuffer		extendedHeader;	/*!< @brief Extended header attributes, used for per-domain defined attributes that are not used when determining stream uniqueness.  Presence is indicated by \ref RsslRequestFlags::RSSL_RQMF_HAS_EXTENDED_HEADER.  */
} RsslRequestMsg;

/**
 * @brief Static initializer for the RsslRequestMsg
 *
 * @note Static clear function does not set RsslRequestMsg::msgBase::msgClass, the rsslClearRequestMsg() function does.  
 *
 * @warning On larger structures, like messages, the clear functions tend to outperform the static initializer.  It is recommended to use the clear function when initializing any messages.
 * @see RsslRequestMsg, rsslClearRequestMsg
 */

#define RSSL_INIT_REQUEST_MSG { RSSL_INIT_MSG_BASE, 0, 0, 0, RSSL_INIT_QOS, RSSL_INIT_QOS, RSSL_INIT_BUFFER }


/**
 * @brief Clears an RsslRequestMsg.  Also sets RsslRequestMsg::msgBase::msgClass.
 *
 * @see RsslRequestMsg, RSSL_INIT_REQUEST_MSG
 */
RTR_C_ALWAYS_INLINE void rsslClearRequestMsg(RsslRequestMsg *pRequestMsg)  
{
	memset(pRequestMsg, 0, sizeof(RsslRequestMsg));
	pRequestMsg->msgBase.msgClass = 1;
}



/** 
 * @brief Flag values for use with the RsslRequestMsg.  (RQMF = RequestMsg Flag)
 * @see RsslRequestMsg, RsslMsg
 */
typedef enum {
	RSSL_RQMF_NONE					= 0x0000,   /*!< (0x000) No RsslRequestMsg flags are present */
	RSSL_RQMF_HAS_EXTENDED_HEADER	= 0x0001,	/*!< (0x0001) This RsslRequestMsg has an extended header buffer, contained in \ref RsslRequestMsg::extendedHeader.  */
	RSSL_RQMF_HAS_PRIORITY			= 0x0002,   /*!< (0x0002) This RsslRequestMsg has priority information, contained in \ref RsslRequestMsg::priorityClass and \ref RsslRequestMsg::priorityCount.  This is used to indicate the importance of this stream. */
	RSSL_RQMF_STREAMING				= 0x0004,   /*!< (0x0004) Indicates that the RsslRequestMsg's item stream should be opened as streaming (e.g. as changes occur, subsequent RsslUpdateMsg will deliver updates).  If a stream is paused, a subsequent RsslRequestMsg with this flag present will resume data flow. */
    RSSL_RQMF_MSG_KEY_IN_UPDATES	= 0x0008,   /*!< (0x0008) Indicates that the user would like to have RsslMsgKey information included in any RsslUpdateMsg content on the stream. */
   	RSSL_RQMF_CONF_INFO_IN_UPDATES	= 0x0010,	/*!< (0x0010) Indicates that the user would like to have conflation information, when conflation is occuring, included in any RsslUpdateMsg content on the stream. */
	RSSL_RQMF_NO_REFRESH			= 0x0020,	/*!< (0x0020) Indicates that the user does not require an RsslRefreshMsg for this request - typically used as part of a reissue to change priority, view information, or pausing/resuming a stream. */
	RSSL_RQMF_HAS_QOS				= 0x0040,	/*!< (0x0040) This RsslRequestMsg contains quality of service information, contained in RsslRequestMsg::qos.  If only \ref RsslRequestMsg::qos is present, this is the QoS that will satisfy the request.  If RsslRequestMsg::qos and RsslRequestMsg::worstQos are both present, this indicates that any QoS in that range will satisfy the request. */
	RSSL_RQMF_HAS_WORST_QOS			= 0x0080,	/*!< (0x0080) This RsslRequestMsg contains a worst quality of service, implying that a range is specified in the message.  When \ref RsslRequestMsg::qos and \ref RsslRequestMsg::worstQos are both present, this indicates that any QoS in that range will satisfy the request. */
	RSSL_RQMF_PRIVATE_STREAM		= 0x0100,	/*!< (0x0100) Indicates that the user would like to open the stream as 'private'.  This requires a response (RsslRefreshMsg, RsslStatusMsg, or RsslAckMsg) to acknowledge that it is open as private.  Once opened as private, all content should only flow between two endpoints of streams - no fanout or caching should be performed. */
	RSSL_RQMF_PAUSE					= 0x0200,	/*!< (0x0200) Indicates that the user would like to pause the stream.  If present on an initial RsslRequestMsg, the stream should be opened in a paused state where the user gets an initial RsslRefreshMsg.  If present on a subsequent RsslRequestMsg, the stream will be paused at that time. Issuing this flag does not guarantee that the stream will be paused.  */
	RSSL_RQMF_HAS_VIEW				= 0x0400,	/*!< (0x0400) Indicates that the payload contains a dynamic view specification, indicating the specific information on the stream the user is interested in receiving.  */
	RSSL_RQMF_HAS_BATCH				= 0x0800,	/*!< (0x0800) This RsslRequestMsg payload contains a batch of items for requesting */
	RSSL_RQMF_QUALIFIED_STREAM		= 0x1000    /*!< (0x1000) Indicates that the user would like to open the stream as 'qualified.' */
} RsslRequestFlags;

/** 
 * @brief General OMM strings associated with the different request message flags
 * @see RsslRequestFlags, rsslRequestFlagsToOmmString
 */
static const RsslBuffer RSSL_OMMSTR_RQMF_HAS_EXTENDED_HEADER = { 17, (char*)"HasExtendedHeader" };
static const RsslBuffer RSSL_OMMSTR_RQMF_HAS_PRIORITY = { 11, (char*)"HasPriority" };
static const RsslBuffer RSSL_OMMSTR_RQMF_STREAMING = { 9, (char*)"Streaming" };
static const RsslBuffer RSSL_OMMSTR_RQMF_MSG_KEY_IN_UPDATES = { 15, (char*)"MsgKeyInUpdates" };
static const RsslBuffer RSSL_OMMSTR_RQMF_CONF_INFO_IN_UPDATES = { 17, (char*)"ConfInfoInUpdates" };
static const RsslBuffer RSSL_OMMSTR_RQMF_NO_REFRESH = { 9, (char*)"NoRefresh" };
static const RsslBuffer RSSL_OMMSTR_RQMF_HAS_QOS = { 6, (char*)"HasQos" };
static const RsslBuffer RSSL_OMMSTR_RQMF_HAS_WORST_QOS = { 11, (char*)"HasWorstQos" };
static const RsslBuffer RSSL_OMMSTR_RQMF_PRIVATE_STREAM = { 13, (char*)"PrivateStream" };
static const RsslBuffer RSSL_OMMSTR_RQMF_PAUSE = { 5, (char*)"Pause" };
static const RsslBuffer RSSL_OMMSTR_RQMF_HAS_VIEW = { 7, (char*)"HasView" };
static const RsslBuffer RSSL_OMMSTR_RQMF_HAS_BATCH = { 8, (char*)"HasBatch" };
static const RsslBuffer RSSL_OMMSTR_RQMF_QUALIFIED_STREAM = { 15, (char*)"QualifiedStream" };

/**
 * @brief Provide general OMM string representation of RsslRequestFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RsslRequestFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RsslRequestFlags
 */
RSSL_API RsslRet rsslRequestFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags);

/**
 * @}
 */


/**
 *	@defgroup ReqMsgHelpers RsslRequestMsg Helper Functions
 *	@{
 */

/**
 * @brief Checks the presence of the ::RSSL_RQMF_HAS_EXTENDED_HEADER flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckHasExtendedHdr(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_HAS_EXTENDED_HEADER) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_HAS_PRIORITY flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckHasPriority(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_HAS_PRIORITY) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_STREAMING flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckStreaming(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_STREAMING) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_MSG_KEY_IN_UPDATES flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckMsgKeyInUpdates(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_MSG_KEY_IN_UPDATES) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_CONF_INFO_IN_UPDATES flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckConfInfoInUpdates(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_CONF_INFO_IN_UPDATES) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_NO_REFRESH flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckNoRefresh(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_NO_REFRESH) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_HAS_QOS flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckHasQos(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_HAS_QOS) ? 
						RSSL_TRUE : RSSL_FALSE );
}
#define rsslRequestMsgCheckHasQoS rsslRequestMsgCheckHasQos

/**
 * @brief Checks the presence of the ::RSSL_RQMF_HAS_WORST_QOS flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckHasWorstQos(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_HAS_WORST_QOS) ? 
						RSSL_TRUE : RSSL_FALSE );
}
#define rsslRequestMsgCheckHasWorstQoS rsslRequestMsgCheckHasWorstQos

/**
 * @brief Checks the presence of the ::RSSL_RQMF_PRIVATE_STREAM flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckPrivateStream(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_PRIVATE_STREAM) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_PAUSE flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckPause(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_PAUSE) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_HAS_VIEW flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckHasView(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_HAS_VIEW) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_HAS_BATCH flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckHasBatch(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_HAS_BATCH) ? 
						RSSL_TRUE : RSSL_FALSE );
}

/**
 * @brief Checks the presence of the ::RSSL_RQMF_QUALIFIED_STREAM flag on the given RsslRequestMsg.
 *
 * @param pRequestMsg Pointer to the RsslRequestMsg to check.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return RSSL_TRUE - if present; RSSL_FALSE if not present.
 */
RTR_C_ALWAYS_INLINE RsslBool rsslRequestMsgCheckQualifiedStream(RsslRequestMsg *pRequestMsg)
{
	return ((pRequestMsg->flags & RSSL_RQMF_QUALIFIED_STREAM) ? 
						RSSL_TRUE : RSSL_FALSE );
}


/**
 * @brief Applies the ::RSSL_RQMF_HAS_EXTENDED_HEADER flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyHasExtendedHdr(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_HAS_EXTENDED_HEADER;
}

/**
 * @brief Applies the ::RSSL_RQMF_HAS_PRIORITY flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyHasPriority(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_HAS_PRIORITY;
}

/**
 * @brief Applies the ::RSSL_RQMF_STREAMING flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyStreaming(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_STREAMING;
}

/**
 * @brief Applies the ::RSSL_RQMF_MSG_KEY_IN_UPDATES flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyMsgKeyInUpdates(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_MSG_KEY_IN_UPDATES;
}

/**
 * @brief Applies the ::RSSL_RQMF_CONF_INFO_IN_UPDATES flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyConfInfoInUpdates(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_CONF_INFO_IN_UPDATES;
}

/**
 * @brief Applies the ::RSSL_RQMF_NO_REFRESH flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyNoRefresh(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_NO_REFRESH;
}

/**
 * @brief Applies the ::RSSL_RQMF_HAS_QOS flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyHasQos(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_HAS_QOS;
}

/**
 * @brief Applies the ::RSSL_RQMF_HAS_WORST_QOS flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyHasWorstQos(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_HAS_WORST_QOS;
}

/**
 * @brief Applies the ::RSSL_RQMF_PRIVATE_STREAM flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyPrivateStream(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_PRIVATE_STREAM;
}

/**
 * @brief Applies the ::RSSL_RQMF_PAUSE flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyPause(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_PAUSE;
}

/**
 * @brief Applies the ::RSSL_RQMF_HAS_VIEW flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyHasView(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_HAS_VIEW;
}

/**
 * @brief Applies the ::RSSL_RQMF_HAS_BATCH flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyHasBatch(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_HAS_BATCH;
}

/**
 * @brief Applies the ::RSSL_RQMF_QUALIFIED_STREAM flag on the given RsslRequestMsg.
 * 
 * @param pRequestMsg Pointer to the RsslRequestMsg to apply flag value to.
 * @see RsslRequestMsg, RsslRequestFlags
 * @return No return value
 */
RTR_C_ALWAYS_INLINE void rsslRequestMsgApplyQualifiedStream(RsslRequestMsg *pRequestMsg)
{
	pRequestMsg->flags |= RSSL_RQMF_QUALIFIED_STREAM;
}


/**
 * @brief Sets the ::RSSL_RQMF_STREAMING flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to apply flag to.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RSSL_API RsslRet rsslSetStreamingFlag( RsslEncodeIterator		*pIter );

/**
 * @brief Unsets the ::RSSL_RQMF_STREAMING flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to remove flag from.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RSSL_API RsslRet rsslUnsetStreamingFlag( RsslEncodeIterator		*pIter );

/**
 * @brief Sets the ::RSSL_RQMF_NO_REFRESH flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to apply flag to.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RSSL_API RsslRet rsslSetNoRefreshFlag( RsslEncodeIterator		*pIter );

/**
 * @brief Unsets the ::RSSL_RQMF_NO_REFRESH flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to remove flag from.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RSSL_API RsslRet rsslUnsetNoRefreshFlag( RsslEncodeIterator		*pIter );


/**
 * @brief Sets the ::RSSL_RQMF_MSG_KEY_IN_UPDATES flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to apply flag to.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RSSL_API RsslRet rsslSetMsgKeyInUpdatesFlag( RsslEncodeIterator		*pIter );

/**
 * @brief Unsets the ::RSSL_RQMF_MSG_KEY_IN_UPDATES flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to remove flag from.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RSSL_API RsslRet rsslUnsetMsgKeyInUpdatesFlag( RsslEncodeIterator		*pIter );

/**
 * @brief Sets the ::RSSL_RQMF_CONF_INFO_IN_UPDATES flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to apply flag to.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RSSL_API RsslRet rsslSetConfInfoInUpdatesFlag( RsslEncodeIterator		*pIter );

/**
 * @brief Unsets the ::RSSL_RQMF_CONF_INFO_IN_UPDATES flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to remove flag from.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RSSL_API RsslRet rsslUnsetConfInfoInUpdatesFlag( RsslEncodeIterator		*pIter );



/**
 * @}
 */





#ifdef __cplusplus
}
#endif


#endif

