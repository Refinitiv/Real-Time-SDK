/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_MSG_ENCODERS_H_
#define __RSSL_MSG_ENCODERS_H_


#ifdef __cplusplus
extern "C" {
#endif

/* Data Package Headers */
#include "rtr/rsslDataPackage.h"


/* Message Package Headers */
#include "rtr/rsslMessagePackage.h"


/** 
 * @addtogroup MsgEncoders
 * @{
 */


/** 
 * @brief 	Encodes an RsslMsg where there is no payload or any payload is pre-encoded and set on \ref RsslMsgBase::encDataBody, no extended header or it is pre-encoded, and no message key attributes or they are pre-encoded and populated on \ref RsslMsgKey::encAttrib.
 *
 * Encodes an RsslMsg<BR>
 * Typical use:<BR>
 *  1. Populate desired members on the RsslMsg (RsslMsg union or RsslRefreshMsg, RsslStatusMsg, RsslUpdateMsg, RsslRequestMsg, RsslPostMsg, RsslAckMsg, RsslGenericMsg, RsslCloseMsg)<BR>
 *  2. If the message contains message key attributes, call appropriate \ref RsslMsgKey::attribContainerType encoder functions and populate on \ref RsslMsgKey::encAttrib<BR>
 *  3. If the message contains extended header information, encode as needed and populate on the respective message's extendedHeader \ref RsslBuffer<BR>
 *  4. If the message contains message request key attributes, call appropriate \ref RsslMsgKey::attribContainerType encoder functions and populate on \ref RsslMsgKey::encAttrib<BR>
 *  5. If the message contains any payload, encode using the specified \ref RsslMsgBase::containerType encode functions and populate on the \ref RsslMsgBase::encDataBody<BR>
 *  6. Call rsslEncodeMsg() when all content is populated to encode the entire message and all payload<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pMsg populated RsslMsg to encode.  If any payload, extended header, or message key attributes are required, they must already be populated on this message. 
 * @see RsslEncodeIterator, rsslEncodeMsgInit, RsslMsg, RsslMsgKey, RsslMsgBase, RsslRequestMsg, RsslRefreshMsg, RsslStatusMsg, RsslUpdateMsg, RsslGenericMsg, RsslPostMsg, RsslAckMsg, RsslCloseMsg
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMsg( 
					RsslEncodeIterator		*pIter,
					RsslMsg			        *pMsg 
                    );


/** 
 * @brief 	Begin encoding process for an RsslMsg.
 *
 * Begins encoding of an RsslMsg<BR>
 * Typical use:<BR>
 *  1. Populate desired members on the RsslMsg<BR>
 *  2. Call rsslEncodeMsgInit() to begin message encoding<BR>
 *  3. If the RsslMsg requires any message key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgKeyAttribComplete() to continue with message encoding<BR>
 *  4. If the RsslMsg requires any extended header information, but it is not pre-encoded and populated in the extendedHeader \ref RsslBuffer, the rsslEncodeMsgInit() (or when also encoding attributes, the rsslEncodeMsgKeyAttribComplete()) function will return ::RSSL_RET_ENCODE_EXTENDED_HEADER.  Call any necessary extended header encoding functions; when completed call rsslEncodeExtendedHeaderComplete() to continue with message encoding<BR>
 *  5. If the RsslMsg requires any message request key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgReqKeyAttribComplete() to continue with message encoding<BR>
 *  6. If the RsslMsg requires any payload, but it is not pre-encoded and populated in the \ref RsslMsgBase::encDataBody, the rsslEncodeMsgInit() (or when encoding message key attributes or extended header, rsslEncodeMsgKeyAttribComplete() or rsslEncodeExtendedHeaderComplete() )  function will return ::RSSL_RET_ENCODE_CONTAINER.  Call appropriate payload encode functions, as indicated by \ref RsslMsgBase::containerType.  If no payload is required or it is provided as pre-encoded, this function will return ::RSSL_RET_SUCCESS<BR>
 *  7. Call rsslEncodeMsgComplete() when all content is completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param pMsg	Partially populated RsslMsg structure to encode
 * @param dataMaxSize Max expected encoding size of the payload, if encoding.  
 * @see RsslEncodeIterator, rsslEncodeMsg, rsslEncodeMsgComplete, rsslEncodeMsgKeyAttribComplete, rsslEncodeExtendedHeaderComplete, RsslMsg, RsslMsgKey, RsslMsgBase, RsslRequestMsg, RsslRefreshMsg, RsslStatusMsg, RsslUpdateMsg, RsslGenericMsg, RsslPostMsg, RsslAckMsg, RsslCloseMsg
 * @return Returns an RsslRet to provide success or failure information.
 */
RSSL_API RsslRet rsslEncodeMsgInit( 
					RsslEncodeIterator		*pIter,
					RsslMsg			        *pMsg,
					RsslUInt32				dataMaxSize
                    );
															

/** 
 * @brief 	Completes encoding of any non-pre-encoded message key attributes when encoding an RsslMsg using rsslEncodeMsgInit()
 *
 * Completes non-pre-encoded message key attribute encoding when using rsslEncodeMsgInit() <BR>
 * Typical use:<BR>
 *  1. Populate desired members on the RsslMsg<BR>
 *  2. Call rsslEncodeMsgInit() to begin message encoding<BR>
 *  3. If the RsslMsg requires any message key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgKeyAttribComplete() to continue with message encoding<BR>
 *  4. If the RsslMsg requires any extended header information, but it is not pre-encoded and populated in the extendedHeader \ref RsslBuffer, the rsslEncodeMsgInit() (or when also encoding attributes, the rsslEncodeMsgKeyAttribComplete()) function will return ::RSSL_RET_ENCODE_EXTENDED_HEADER.  Call any necessary extended header encoding functions; when completed call rsslEncodeExtendedHeaderComplete() to continue with message encoding<BR>
 *  5. If the RsslMsg requires any message request key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgReqKeyAttribComplete() to continue with message encoding<BR>
 *  6. If the RsslMsg requires any payload, but it is not pre-encoded and populated in the \ref RsslMsgBase::encDataBody, the rsslEncodeMsgInit() (or when encoding message key attributes or extended header, rsslEncodeMsgKeyAttribComplete() or rsslEncodeExtendedHeaderComplete() )  function will return ::RSSL_RET_ENCODE_CONTAINER.  Call appropriate payload encode functions, as indicated by \ref RsslMsgBase::containerType.  If no payload is required or it is provided as pre-encoded, this function will return ::RSSL_RET_SUCCESS<BR>
 *  7. Call rsslEncodeMsgComplete() when all content is completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the message encoding, if false - roll back encoding.
 * @see RsslEncodeIterator, rsslEncodeMsg, rsslEncodeMsgComplete, rsslEncodeMsgKeyAttribComplete, rsslEncodeExtendedHeaderComplete, RsslMsg, RsslMsgKey, RsslMsgBase, RsslRequestMsg, RsslRefreshMsg, RsslStatusMsg, RsslUpdateMsg, RsslGenericMsg, RsslPostMsg, RsslAckMsg, RsslCloseMsg
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMsgKeyAttribComplete( 
                   	RsslEncodeIterator		*pIter,
				    RsslBool				success
				   );


/** 
 * @brief 	Completes encoding of any non-pre-encoded message request key attributes when encoding an RsslMsg using rsslEncodeMsgInit()
 *
 * Completes non-pre-encoded message key attribute encoding when using rsslEncodeMsgInit() <BR>
 * Typical use:<BR>
 *  1. Populate desired members on the RsslMsg<BR>
 *  2. Call rsslEncodeMsgInit() to begin message encoding<BR>
 *  3. If the RsslMsg requires any message key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgKeyAttribComplete() to continue with message encoding<BR>
 *  4. If the RsslMsg requires any extended header information, but it is not pre-encoded and populated in the extendedHeader \ref RsslBuffer, the rsslEncodeMsgInit() (or when also encoding attributes, the rsslEncodeMsgKeyAttribComplete()) function will return ::RSSL_RET_ENCODE_EXTENDED_HEADER.  Call any necessary extended header encoding functions; when completed call rsslEncodeExtendedHeaderComplete() to continue with message encoding<BR>
 *  5. If the RsslMsg requires any message request key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgReqKeyAttribComplete() to continue with message encoding<BR>
 *  6. If the RsslMsg requires any payload, but it is not pre-encoded and populated in the \ref RsslMsgBase::encDataBody, the rsslEncodeMsgInit() (or when encoding message key attributes or extended header, rsslEncodeMsgKeyAttribComplete() or rsslEncodeExtendedHeaderComplete() )  function will return ::RSSL_RET_ENCODE_CONTAINER.  Call appropriate payload encode functions, as indicated by \ref RsslMsgBase::containerType.  If no payload is required or it is provided as pre-encoded, this function will return ::RSSL_RET_SUCCESS<BR>
 *  7. Call rsslEncodeMsgComplete() when all content is completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the message encoding, if false - roll back encoding.
 * @see RsslEncodeIterator, rsslEncodeMsg, rsslEncodeMsgComplete, rsslEncodeMsgKeyAttribComplete, rsslEncodeExtendedHeaderComplete, RsslMsg, RsslMsgKey, RsslMsgBase, RsslRequestMsg, RsslRefreshMsg, RsslStatusMsg, RsslUpdateMsg, RsslGenericMsg, RsslPostMsg, RsslAckMsg, RsslCloseMsg
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMsgReqKeyAttribComplete( 
                   	RsslEncodeIterator		*pIter,
				    RsslBool				success
				   );


/** 
 * @brief 	Completes encoding of any non-pre-encoded extended header information when encoding an RsslMsg using rsslEncodeMsgInit()
 *
 * Completes non-pre-encoded extended header information when using rsslEncodeMsgInit() <BR>
 * Typical use:<BR>
 *  1. Populate desired members on the RsslMsg<BR>
 *  2. Call rsslEncodeMsgInit() to begin message encoding<BR>
 *  3. If the RsslMsg requires any message key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgKeyAttribComplete() to continue with message encoding<BR>
 *  4. If the RsslMsg requires any extended header information, but it is not pre-encoded and populated in the extendedHeader \ref RsslBuffer, the rsslEncodeMsgInit() (or when also encoding attributes, the rsslEncodeMsgKeyAttribComplete()) function will return ::RSSL_RET_ENCODE_EXTENDED_HEADER.  Call any necessary extended header encoding functions; when completed call rsslEncodeExtendedHeaderComplete() to continue with message encoding<BR>
 *  5. If the RsslMsg requires any message request key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgReqKeyAttribComplete() to continue with message encoding<BR>
 *  6. If the RsslMsg requires any payload, but it is not pre-encoded and populated in the \ref RsslMsgBase::encDataBody, the rsslEncodeMsgInit() (or when encoding message key attributes or extended header, rsslEncodeMsgKeyAttribComplete() or rsslEncodeExtendedHeaderComplete() )  function will return ::RSSL_RET_ENCODE_CONTAINER.  Call appropriate payload encode functions, as indicated by \ref RsslMsgBase::containerType.  If no payload is required or it is provided as pre-encoded, this function will return ::RSSL_RET_SUCCESS<BR>
 *  7. Call rsslEncodeMsgComplete() when all content is completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the extended header encoding, if false - roll back encoding.
 * @see RsslEncodeIterator, rsslEncodeMsg, rsslEncodeMsgComplete, rsslEncodeMsgKeyAttribComplete, rsslEncodeExtendedHeaderComplete, RsslMsg, RsslMsgKey, RsslMsgBase, RsslRequestMsg, RsslRefreshMsg, RsslStatusMsg, RsslUpdateMsg, RsslGenericMsg, RsslPostMsg, RsslAckMsg, RsslCloseMsg
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeExtendedHeaderComplete( 
					RsslEncodeIterator		*pIter,
					RsslBool				success
                    );


/** 
 * @brief 	Completes encoding of an RsslMsg
 *
 * Completes RsslMsg encoding <BR>
 * Typical use:<BR>
 *  1. Populate desired members on the RsslMsg<BR>
 *  2. Call rsslEncodeMsgInit() to begin message encoding<BR>
 *  3. If the RsslMsg requires any message key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgKeyAttribComplete() to continue with message encoding<BR>
 *  4. If the RsslMsg requires any extended header information, but it is not pre-encoded and populated in the extendedHeader \ref RsslBuffer, the rsslEncodeMsgInit() (or when also encoding attributes, the rsslEncodeMsgKeyAttribComplete()) function will return ::RSSL_RET_ENCODE_EXTENDED_HEADER.  Call any necessary extended header encoding functions; when completed call rsslEncodeExtendedHeaderComplete() to continue with message encoding<BR>
 *  5. If the RsslMsg requires any message request key attributes, but they are not pre-encoded and populated on the RsslMsgKey::encAttrib, the rsslEncodeMsgInit() function will return ::RSSL_RET_ENCODE_MSG_KEY_OPAQUE.  Call appropriate encode functions, as indicated by RsslMsgKey::attribContainerType.  When attribute encoding is completed, followed with rsslEncodeMsgReqKeyAttribComplete() to continue with message encoding<BR>
 *  6. If the RsslMsg requires any payload, but it is not pre-encoded and populated in the \ref RsslMsgBase::encDataBody, the rsslEncodeMsgInit() (or when encoding message key attributes or extended header, rsslEncodeMsgKeyAttribComplete() or rsslEncodeExtendedHeaderComplete() )  function will return ::RSSL_RET_ENCODE_CONTAINER.  Call appropriate payload encode functions, as indicated by \ref RsslMsgBase::containerType.  If no payload is required or it is provided as pre-encoded, this function will return ::RSSL_RET_SUCCESS<BR>
 *  7. Call rsslEncodeMsgComplete() when all content is completed<BR>
 *
 * @param pIter	Pointer to the encode iterator.
 * @param success If true - successfully complete the message encoding, if false - roll back encoding.
 * @see RsslEncodeIterator, rsslEncodeMsg, rsslEncodeMsgComplete, rsslEncodeMsgKeyAttribComplete, rsslEncodeExtendedHeaderComplete, RsslMsg, RsslMsgKey, RsslMsgBase, RsslRequestMsg, RsslRefreshMsg, RsslStatusMsg, RsslUpdateMsg, RsslGenericMsg, RsslPostMsg, RsslAckMsg, RsslCloseMsg
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslEncodeMsgComplete( 
                    RsslEncodeIterator		*pIter,
					RsslBool				success 
                    );




/**
 * @}
 */


/**
 * @addtogroup MsgEncodeUtilsHelpers
 * @{
 */



/**
 * @brief This function replaces the encoded \ref RsslMsgBase::domainType value in an encoded \ref RsslMsg with the specified domain type enumeration.
 *
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param domainType The new domain type enumeration that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the domain type enumeration has been replaced in the buffer.  If RSSL_RET_INVALID_ARGUMENT, there 
 * 		   was an issue with the encoded buffer's length, and the domain type was not replaced.
 *
 */
RSSL_API RsslRet rsslReplaceDomainType(
                     RsslEncodeIterator		*pIter,
                     RsslUInt8				domainType );


/**
 * @brief This function replaces the encoded \ref RsslMsgBase::streamId value in an encoded \ref RsslMsg with the specified streamId.
 *
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param streamId 		The new stream Id that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the stream Id has been replaced in the buffer.  If RSSL_RET_INVALID_ARGUMENT, there was 
 * 		   an issue with the encoded buffer's length, and the stream Id was not replaced.
 *
 */
RSSL_API RsslRet rsslReplaceStreamId(
                     RsslEncodeIterator		*pIter,
                     RsslInt32             streamId );


/**
 * @brief This function replaces the encoded Sequence Number in an encoded \ref RsslMsg with the specified streamId.
 * 
 * @note This function will only succeed if the encoded message contains a sequence number.  It cannot add a sequence number 
 * 	 	 to an encoded message that does not have the information.
 * 
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param seqNum 		The new sequence number that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the sequence number has been replaced in the buffer.  If RSSL_RET_FAILURE, this indicates that 
 * 		   the encoded message did not have a sequence number, and nothing was changed in the encoded buffer. If RSSL_RET_INVALID_ARGUMENT, 
 *		   there was an issue with the encoded RsslBuffer, and the encoded buffer was not changed.
 * @see \ref RsslUpdateMsg, \ref RsslRefreshMsg, \ref RsslPostMsg, \ref RsslGenericMsg, \ref RsslAckMsg
 */
RSSL_API RsslRet rsslReplaceSeqNum(
                     RsslEncodeIterator		*pIter,
                     RsslUInt32             seqNum );


/**
 * @brief This function replaces the encoded stream state in an encoded \ref RsslMsg with the specified stream state.
 * 
 * @note This function will only succeed if the encoded message contains stream state information.  It cannot add 
 *		 state information to an encoded message that does not have the information.
 *
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param streamState 	The new stream state that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the stream state has been replaced in the buffer.  If RSSL_RET_FAILURE, this indicates that 
 *		   the encoded message did not have a stream state, so nothing was changed in the encoded buffer.  If RSSL_RET_INVALID_ARGUMENT, 
 *		   there was an issue with the encoded RsslBuffer or streamState, and the encoded buffer was not changed.
 * @see \ref RsslRefreshMsg, \ref RsslStatusMsg
 */
RSSL_API RsslRet rsslReplaceStreamState(
                     RsslEncodeIterator		*pIter,
                     RsslUInt8				streamState);



/**
 * @brief This function replaces the encoded data state in an encoded \ref RsslMsg with the specified data state.
 * 
 * @note This function will only succeed if the encoded message contains data state information.  It cannot add 
 *		 state information to an encoded message that does not have the information.
 *
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param dataState 	The new data state that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the stream state has been replaced in the buffer.  If RSSL_RET_FAILURE, this indicates that 
 *		   the encoded message did not have a data state, so nothing was changed in the encoded buffer.  If RSSL_RET_INVALID_ARGUMENT, 
 *		   there was an issue with the encoded RsslBuffer, and the encoded buffer was not changed.
 * @see \ref RsslRefreshMsg, \ref RsslStatusMsg
 */
RSSL_API RsslRet rsslReplaceDataState(
                     RsslEncodeIterator		*pIter,
                     RsslUInt8				dataState);

/**
 * @brief This function replaces the encoded state code in an encoded \ref RsslMsg with the specified state code.
 * 
 * @note This function will only succeed if the encoded message contains a state code.  It cannot add 
 *		 state information to an encoded message that does not have the information.
 *
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param stateCode 	The new state code that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the state code has been replaced in the buffer.  If RSSL_RET_FAILURE, this indicates that 
 *		   the encoded message did not have a state code, so nothing was changed in the encoded buffer.  If RSSL_RET_INVALID_ARGUMENT, 
 *		   there was an issue with the encoded RsslBuffer, and the encoded buffer was not changed.
 * @see \ref RsslRefreshMsg, \ref RsslStatusMsg
 */
RSSL_API RsslRet rsslReplaceStateCode(
                     RsslEncodeIterator		*pIter,
                     RsslUInt8				stateCode);


/**
 * @brief This function replaces the encoded group Id in an encoded \ref RsslMsg with the specified group Id.
 * 
 * @note This function will only succeed if the encoded message contains a group Id.  It cannot add 
 *		 state information to an encoded message that does not have the information.
 *
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param groupId 		The new group Id that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the group Id has been replaced in the buffer.  If RSSL_RET_FAILURE, this indicates that 
 *		   the encoded message did not have a group Id, so nothing was changed in the encoded buffer.  If RSSL_RET_INVALID_ARGUMENT, 
 *		   there was an issue with the encoded RsslBuffer, and the encoded buffer was not changed.
 * @see \ref RsslRefreshMsg, \ref RsslStatusMsg
 */
RSSL_API RsslRet rsslReplaceGroupId(
                     RsslEncodeIterator		*pIter,
                     RsslBuffer 			groupId);



/**
 * @brief This function replaces the encoded post Id in an encoded \ref RsslMsg with the specified post Id.
 * 
 * @note This function will only succeed if the encoded message contains a post Id.  It cannot add 
 *		 state information to an encoded message that does not have the information.
 *
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param postId 		The new post Id that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the post Id has been replaced in the buffer.  If RSSL_RET_FAILURE, this indicates that 
 *		   the encoded message did not have a post Id, so nothing was changed in the encoded buffer.  If RSSL_RET_INVALID_ARGUMENT, 
 *		   there was an issue with the encoded RsslBuffer, and the encoded buffer was not changed.
 * @see \ref RsslPostMsg
 */
RSSL_API RsslRet rsslReplacePostId(
                     RsslEncodeIterator		*pIter,
                     RsslUInt32				postId);



/**
 * @}
 */


#ifdef __cplusplus
}
#endif


#endif

