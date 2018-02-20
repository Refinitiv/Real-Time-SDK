/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_MSG_DECODERS_H_
#define __RSSL_MSG_DECODERS_H_


#ifdef __cplusplus
extern "C" {
#endif

/* Data Package Headers */
#include "rtr/rsslDataPackage.h"

/* Message Package Headers */
#include "rtr/rsslMessagePackage.h"



/** 
 * @addtogroup MsgDecoders
 * @{
 */


/**
 * @brief Decodes an RsslMsg.  
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeMsg()<BR>
 *  2. If there are any message key attributes and the application wishes to decode them using the same \ref RsslDecodeIterator, call rsslDecodeMsgKeyAttrib() for RWF container types and continue decoding using the appropriate container type decode functions, as indicated by RsslMsgKey::attribContainerType.<BR>
 *  3. If payload is present and the application wishes to decode it, use the appropriate decode functions, as specified in \ref RsslMsgBase::containerType<BR>
 *
 *	@note This does not decode any contained payload, message key attributes, or extended header information. The user can choose whether it requires decoding of those portions of the message.  
 *
 * @param pIter Decode iterator to use for decode process
 * @param pMsg RsslMsg structure to populate with decoded contents.  RsslMsgBase::encDataBody will point to any encoded payload.  RsslMsgKey::encAttrib will point to any encoded message key attributes.  The extendedHeader \ref RsslBuffer will point to any encoded extended header information.  
 * @see rsslDecodeMsgKeyAttrib, RsslMsg, RsslMsgKey, RsslMsgBase, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information
 */
RSSL_API RsslRet rsslDecodeMsg(RsslDecodeIterator * pIter, RsslMsg * pMsg);



/**
 * @brief Allows the user to continue decoding of any message key attributes with the same \ref RsslDecodeIterator used when calling rsslDecodeMsg
 * 
 * Typical use:<BR>
 *  1. Call rsslDecodeMsg()<BR>
 *  2. If there are any message key attributes that are of RWF container type and the application wishes to decode them using the same \ref RsslDecodeIterator, call rsslDecodeMsgKeyAttrib() and continue decoding using the appropriate container type decode functions, as indicated by RsslMsgKey::attribContainerType. This should not be used if RsslMsgKey::attribContainerType is a Non-RWF type.<BR>
 *  3. If payload is present and the application wishes to decode it, use the appropriate decode functions, as specified in \ref RsslMsgBase::containerType<BR>
 *
 *	@note A seperate \ref RsslDecodeIterator can be used to decode the message key attributes. This function is only required if using the same \ref RsslDecodeIterator as used with rsslDecodeMsg().
 *
 * @param pIter Decode iterator to use for decode process
 * @param pKey RsslMsgKey pointer from the RsslMsg structure populated by rsslDecodeMsg().
 * @see rsslDecodeMsg, RsslMsg, RsslMsgKey, RsslMsgBase, RsslDecodeIterator
 * @return Returns an RsslRet to provide success or failure information. RSSL_RET_INVALID_ARGUMENT is returned if RsslMsgKey::attribContainerType is a Non-RWF type.
 */
RSSL_API RsslRet rsslDecodeMsgKeyAttrib(RsslDecodeIterator *pIter, const RsslMsgKey *pKey);


/**
 * @}
 */





/**
 * @addtogroup MsgDecodeUtilsHelpers
 * @{
 */


/**
 * @brief Extract \ref RsslMsgBase::msgClass from a buffer containing an encoded \ref RsslMsg. 
 *
 * @param pIter \ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set on it using \ref rsslSetDecodeIteratorBuffer().
 * @return The \ref RsslMsgBase::msgClass enumeration contained in the buffer. See \ref RsslMsgClasses for enumerations.
 * @see RsslMsgClasses
 */
RSSL_API RsslUInt8 rsslExtractMsgClass( const RsslDecodeIterator *pIter );


/**
 * @brief Extract \ref RsslMsgBase::domainType from a buffer containing an encoded \ref RsslMsg. 
 *
 * @param pIter 			\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set on it using \ref rsslSetDecodeIteratorBuffer().
 * @return The \ref RsslMsgBase::domainType enumeration contained in the buffer. See \ref RsslDomainTypes for Thomson Reuters Domain Model enumerations.
 * @see RsslMsgTypes
 */
RSSL_API RsslUInt8 rsslExtractDomainType( const RsslDecodeIterator *pIter );


/**
 * @brief Extract \ref RsslMsgBase::streamId from a buffer containing an encoded \ref RsslMsg. 
 *
 * @param pIter 			\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set on it using \ref rsslSetDecodeIteratorBuffer().
 * @return The \ref RsslMsgBase::streamId value contained in the buffer.
 */
RSSL_API RsslInt32 rsslExtractStreamId( const RsslDecodeIterator *pIter );

/**
 * @brief Extract the sequence number from a buffer containing an encoded \ref RsslMsg.
 *
 * @param[in] pIter 		\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set.
 * @param[out] pSeqNum 		Pointer to an allocated \ref RsslUInt32.
 * @return If RSSL_RET_SUCCESS, pSeqNum has been populated with the message's sequence number. If RSSL_RET_FAILURE, the operation has failed, and pSeqNum has not been populated.
 * @see \ref RsslUpdateMsg, \ref RsslRefreshMsg, \ref RsslPostMsg, \ref RsslGenericMsg, \ref RsslAckMsg
 */
RSSL_API RsslRet rsslExtractSeqNum( const RsslDecodeIterator *pIter, RsslUInt32 *pSeqNum );


/**
 * @brief Extract the group Id from a buffer containing an encoded \ref RsslMsg.
 *
 * @param[in] pIter 		\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set.
 * @param[out] pGroupId 	Pointer to an RsslBuffer. This operation will not copy the data out of the encoded buffer.
 * @return If RSSL_RET_SUCCESS, pGroupId has been populated the message's group Id information. If RSSL_RET_FAILURE, the operation has failed, and pGroupId has not been populated.
 * @see \ref RsslRefreshMsg, \ref RsslStatusMsg
 */
RSSL_API RsslRet rsslExtractGroupId( const RsslDecodeIterator *pIter, RsslBuffer *pGroupId);

/**
 * @brief Extract the post Id from a buffer containing an encoded \ref RsslMsg.
 *
 * @param[in] pIter 		\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set.
 * @param[out] pPostId 		Pointer to an allocated \ref RsslUInt32.
 * @return If RSSL_RET_SUCCESS, pPostId has been populated with the message's sequence number. If RSSL_RET_FAILURE, the operation has failed, and pPostId has not been populated.
 * @see \ref RsslPostMsg
 */
RSSL_API RsslRet rsslExtractPostId( const RsslDecodeIterator *pIter, RsslUInt32 *pPostId );

/**
 * @}
 */



#ifdef __cplusplus
}
#endif


#endif

