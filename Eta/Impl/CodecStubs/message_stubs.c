#include "rtr/rsslAckMsg.h"
#include "rtr/rsslCloseMsg.h"
#include "rtr/rsslGenericMsg.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslMsg.h"
#include "rtr/rsslMsgBase.h"
#include "rtr/rsslMsgDecoders.h"
#include "rtr/rsslMsgEncoders.h"
#include "rtr/rsslMsgKey.h"
#include "rtr/rsslPostMsg.h"
#include "rtr/rsslRefreshMsg.h"
#include "rtr/rsslRequestMsg.h"
#include "rtr/rsslStatusMsg.h"
#include "rtr/rsslUpdateMsg.h"

/** 
 * @brief Programmatically extracts library and product version information that is compiled into this library
 *
 * User can call this function to programmatically extract version information, or <BR>
 * query version information externally (via 'strings' command or something similar<BR>
 * and grep for the following tags:<BR>
 * 'VERSION' - contains internal library version information such as node number (e.g. rssl1.4.F2)<BR>
 * 'PRODUCT' - contains product information such as load/package naming (e.g. upa7.0.0.L1)<BR>
 * @param pVerInfo RsslLibraryVersionInfo structure to populate with library version information
 * @see RsslLibraryVersionInfo
 */
void rsslQueryMessagesLibraryVersion(RsslLibraryVersionInfo *pVerInfo)
{
	return;
}

/**
 * @brief Validate a populated RsslMsg
 *
 * Validates fully populated RsslMsg structure to ensure that it's flags and data members are properly set, and can be encoded without any issues.
 * @param pMsg Pointer to a fully populated RsslMsg
 * @return \ref RSSL_TRUE if the message is valid, and can be properly encoded. \ref RSSL_FALSE if the message is not valid.
 * @see rsslCopyMsg, rsslReleaseCopiedMsg
 */
RsslBool rsslValidateMsg(const RsslMsg *pMsg)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Retrieve the sequence number from the provided decoded message structure
 *
 * @param pMsg              fully populated RsslMsg
 * @return Sequence number contained in the RsslMsg.
 */
const RsslUInt32* rsslGetSeqNum(const RsslMsg * pMsg)
{
	return NULL;
}

/**
 * @brief Retrieve the RsslMsgKey structure from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the RsslMsgKey structure contained in the RsslMsg.
 */
const RsslMsgKey* rsslGetMsgKey(const RsslMsg * pMsg)
{
	return NULL;
}

/**
 * @brief Retrieve the request RsslMsgKey structure from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the RsslMsgKey structure contained in the RsslMsg.
 */
const RsslMsgKey* rsslGetReqMsgKey(const RsslMsg * pMsg)
{
	return NULL;
}

/**
 * @brief Retrieve flags from the provided decoded message structure
 *
 * @param pMsg				fully populated RsslMsg
 * @return Flag values contained in the RsslMsg.
 */
const RsslUInt16* rsslGetFlags(const RsslMsg * pMsg)
{
	return NULL;
}

/**
 * @brief Retrieve extended header from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the Extended Header buffer contained in the RsslMsg.
 */
const RsslBuffer* rsslGetExtendedHeader(const RsslMsg * pMsg)
{
	return NULL;
}

/**
 * @brief Retrieve state from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the RsslState structure contained in the RsslMsg.
 */
const RsslState* rsslGetState(const RsslMsg * pMsg)
{
	return NULL;
}

/**
 * @brief Retrieve permission expression from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the permission data buffer contained in the RsslMsg.
 */
const RsslBuffer* rsslGetPermData(const RsslMsg * pMsg)
{
	return NULL;
}

/**
 * @brief Retrieve group Id from the provided decoded message structure
 *
 * @param pMsg             fully populated RsslMsg
 * @return Pointer to the GroupId buffer contained in the RsslMsg. 
 */
const RsslBuffer* rsslGetGroupId(const RsslMsg * pMsg)
{
	return NULL;
}

/** 
 * @brief Used to add a two byte identifier to the group ID buffer.
 *
 * @warning <ul>
 *			<li> This should not be run on a group ID buffer contained in a decoded message, as this can cause data corruption.
 *			<li> The user must have allocated enough free space to the buffer to add another two byte identifier.
 *			</ul>
 *			
 * @param[in] groupIdToAdd 	two byte ID to append to the groupID contained in pGroupId buffer
 * @param[out] pGroupId 	RsslBuffer with any existing groupID information. 
 * @return On success, returns RSSL_RET_SUCCESS.
 */
RsslRet rsslAddGroupId(RsslBuffer *pGroupId, RsslUInt16 groupIdToAdd)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Copy RsslMsg to a user-allocated or dynamically allocated buffer.
 *
 * Copies the fully populated RsslMsg structure according to the provided \ref RsslCopyMsgFlags. Depending on the flag 
 * combination provided, this will be either a deep copy if \ref RsslCopyFlags::RSSL_CMF_ALL_FLAGS is used, or a shallow 
 * copy if no flags are present. If a buffer is provided through the pCopyBuffer parameter, this function will copy the 
 * source RsslMsg into the user provided buffer. If a buffer is not provided, the function will dynamically allocate the memory 
 * required to copy the RsslMsg.
 *
 * @note If a user allocated buffer is not provided, the memory allocated for the new copy will not be automatically freed.
 *		    The user must call rsslReleaseCopiedMsg to manually free the data once they are finished with the copy.
 * 
 * @param pSrcMsg          Source RsslMsg
 * @param copyMsgFlags     Copy Message Flags, control how message is copied.  See \ref RsslCopyMsgFlags for more information.
 * @param filterBlocks     Block Mask of Blocks to be included in the final msg
 * @param pCopyBuffer      Optional buffer to copy msg into - in this case user needs to allocate and free memory
 * @return On success, returns a pointer to the copied RsslMsg. On failure, returns 0.
 * @see rsslReleaseCopiedMsg, RsslCopyMsgFlags, rsslValidateMsg, rsslSizeOfMsg
 */
RsslMsg* rsslCopyMsg(const RsslMsg *pSrcMsg, RsslUInt32 copyMsgFlags, RsslUInt32 filterBlocks, RsslBuffer *pCopyBuffer)
{
	return NULL;
}

/**
 * @brief Release allocated RsslMsg memory created by \ref rsslCopyMsg.
 *
 * @warning This function should only be used if rsslCopyMsg allocated memory for the copied RsslMsg. If the user allocated memory
 *			for the copy(using the pCopyBuffer parameter), this function does not need to be called.
 *
 * @param pMsg       RsslMsg to be freed 
 * @see rsslCopyMsg, rsslValidateMsg
 */
void rsslReleaseCopiedMsg(RsslMsg * pMsg)
{
	return;
}

/**
 * @brief This function calculates a deep SizeOf on RsslMsg structure.
 * 
 * This function is intended to assist users who wish to cache or copy the RsslMsg structures by calculating the 
 * fully allocated size of an RsslMsg. In addition to the size of the structure, it also will add in the 
 * size of allocated buffers in the message structure(e.g. \ref RsslMsgKey::name, \ref RsslRefreshMsg::extendedHeader) 
 * according to the \ref RsslCopyMsgFlags options set in the copyMsgFlags parameter.
 *
 * @warning This function will not give an accurate size of an encoded RsslMsg sent on the wire.
 *
 * @param pSrcMsg          	Source RsslMsg
 * @param copyMsgFlags 		Copy Message Flags to control which parts of the message are sized
 * @return Size(in bytes) of the RsslMsg's allocated memory.
 * @see rsslCopyMsg, RsslCopyMsgFlags
 */
RsslUInt32 rsslSizeOfMsg(const RsslMsg *pSrcMsg, RsslUInt32 copyMsgFlags) 
{
	return 0;
}

/** 
 * @brief Converts the provided message class enumeration to a string.
 * @param msgClass message class.
 * @return Null terminated character string containing the name of the message class.
 * @see RsslMsgClasses
 */
const char* rsslMsgClassToString(RsslUInt8 msgClass)
{
	return NULL;
}

/** 
 * @brief Converts the provided domain type enumeration to a string.
 *
 * @param domainType Domain type enumeration to translate to string.
 * @return Null terminated character string containing the name of the domain type.
 * @see RsslDomainTypes
 */
const char* rsslDomainTypeToString(RsslUInt8 domainType)
{
	return NULL;
}

/** 
 * @brief Returns domainType enummeration value from a domain type string.
 * @param domainTypeString domain type string representation.
 * @return Eight bit unsigned integer containing the domain type enumeration that corresponds to the provided string.
 * @see RsslDomainTypes
 */
RsslUInt8 rsslDomainTypeFromString(char *domainTypeString)
{
	return 0;
}

/** 
 * @brief Converts dotted-decimal IP address string(e.g. "127.0.0.1") to integer equivalent.
 * 
 * @param[in] pAddrString 	The IP address string.
 * @param[out] pAddrUInt 	The output integer value, in host byte order.
 * @return RSSL_RET_SUCCESS, or RSSL_RET_FAILURE if the string could not be parsed.
 */
RsslRet rsslIPAddrStringToUInt(const char *pAddrString, RsslUInt32 *pAddrUInt)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Converts IPv4 address in integer format to string equivalent.
 * 
 * @param[in] addrUInt 		The input integer value, in host byte order.
 * @param[out] pAddrString 	The array to fill with the IP Address string. This array must be at least 16 characters in size.
 */
void rsslIPAddrUIntToString(RsslUInt32 addrUInt, char *pAddrString)
{
	return;
}

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
RsslRet rsslDecodeMsg(RsslDecodeIterator * pIter, RsslMsg * pMsg)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslDecodeMsgKeyAttrib(RsslDecodeIterator *pIter, const RsslMsgKey *pKey)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Extract \ref RsslMsgBase::msgClass from a buffer containing an encoded \ref RsslMsg. 
 *
 * @param pIter \ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set on it using \ref rsslSetDecodeIteratorBuffer().
 * @return The \ref RsslMsgBase::msgClass enumeration contained in the buffer. See \ref RsslMsgClasses for enumerations.
 * @see RsslMsgClasses
 */
RsslUInt8 rsslExtractMsgClass(const RsslDecodeIterator *pIter)
{
	return 0;
}

/**
 * @brief Extract \ref RsslMsgBase::domainType from a buffer containing an encoded \ref RsslMsg. 
 *
 * @param pIter 			\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set on it using \ref rsslSetDecodeIteratorBuffer().
 * @return The \ref RsslMsgBase::domainType enumeration contained in the buffer. See \ref RsslDomainTypes for Thomson Reuters Domain Model enumerations.
 * @see RsslMsgTypes
 */
RsslUInt8 rsslExtractDomainType(const RsslDecodeIterator *pIter)
{
	return 0;
}

/**
 * @brief Extract \ref RsslMsgBase::streamId from a buffer containing an encoded \ref RsslMsg. 
 *
 * @param pIter 			\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set on it using \ref rsslSetDecodeIteratorBuffer().
 * @return The \ref RsslMsgBase::streamId value contained in the buffer.
 */
RsslInt32 rsslExtractStreamId(const RsslDecodeIterator *pIter)
{
	return 0;
}

/**
 * @brief Extract the sequence number from a buffer containing an encoded \ref RsslMsg.
 *
 * @param[in] pIter 		\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set.
 * @param[out] pSeqNum 		Pointer to an allocated \ref RsslUInt32.
 * @return If RSSL_RET_SUCCESS, pSeqNum has been populated with the message's sequence number. If RSSL_RET_FAILURE, the operation has failed, and pSeqNum has not been populated.
 * @see \ref RsslUpdateMsg, \ref RsslRefreshMsg, \ref RsslPostMsg, \ref RsslGenericMsg, \ref RsslAckMsg
 */
RsslRet rsslExtractSeqNum(const RsslDecodeIterator *pIter, RsslUInt32 *pSeqNum)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Extract the group Id from a buffer containing an encoded \ref RsslMsg.
 *
 * @param[in] pIter 		\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set.
 * @param[out] pGroupId 	Pointer to an RsslBuffer. This operation will not copy the data out of the encoded buffer.
 * @return If RSSL_RET_SUCCESS, pGroupId has been populated the message's group Id information. If RSSL_RET_FAILURE, the operation has failed, and pGroupId has not been populated.
 * @see \ref RsslRefreshMsg, \ref RsslStatusMsg
 */
RsslRet rsslExtractGroupId(const RsslDecodeIterator *pIter, RsslBuffer *pGroupId)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Extract the post Id from a buffer containing an encoded \ref RsslMsg.
 *
 * @param[in] pIter 		\ref RsslDecodeIterator with the \ref RsslBuffer that contains the encoded \ref RsslMsg set.
 * @param[out] pPostId 		Pointer to an allocated \ref RsslUInt32.
 * @return If RSSL_RET_SUCCESS, pPostId has been populated with the message's sequence number. If RSSL_RET_FAILURE, the operation has failed, and pPostId has not been populated.
 * @see \ref RsslPostMsg
 */
RsslRet rsslExtractPostId(const RsslDecodeIterator *pIter, RsslUInt32 *pPostId)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMsg(RsslEncodeIterator *pIter, RsslMsg *pMsg)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMsgInit(RsslEncodeIterator *pIter, RsslMsg *pMsg, RsslUInt32 dataMaxSize)
{
	return RSSL_RET_FAILURE;
}															

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
RsslRet rsslEncodeMsgKeyAttribComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMsgReqKeyAttribComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeExtendedHeaderComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslEncodeMsgComplete(RsslEncodeIterator *pIter, RsslBool success)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief This function replaces the encoded \ref RsslMsgBase::domainType value in an encoded \ref RsslMsg with the specified domain type enumeration.
 *
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param domainType The new domain type enumeration that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the domain type enumeration has been replaced in the buffer.  If RSSL_RET_INVALID_ARGUMENT, there 
 * 		   was an issue with the encoded buffer's length, and the domain type was not replaced.
 *
 */
RsslRet rsslReplaceDomainType(RsslEncodeIterator *pIter, RsslUInt8 domainType)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief This function replaces the encoded \ref RsslMsgBase::streamId value in an encoded \ref RsslMsg with the specified streamId.
 *
 * @param pIter 		\ref RsslEncodeIterator with an \ref RsslBuffer containing an encoded \ref RsslMsg set on it.
 * @param streamId 		The new stream Id that the user wishes to set on the encoded \ref RsslMsg.
 * @return If RSSL_RET_SUCCESS, the stream Id has been replaced in the buffer.  If RSSL_RET_INVALID_ARGUMENT, there was 
 * 		   an issue with the encoded buffer's length, and the stream Id was not replaced.
 *
 */
RsslRet rsslReplaceStreamId(RsslEncodeIterator *pIter, RsslInt32 streamId)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslReplaceSeqNum(RsslEncodeIterator *pIter, RsslUInt32 seqNum)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslReplaceStreamState(RsslEncodeIterator *pIter, RsslUInt8 streamState)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslReplaceDataState(RsslEncodeIterator *pIter, RsslUInt8 dataState)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslReplaceStateCode(RsslEncodeIterator *pIter, RsslUInt8 stateCode)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslReplaceGroupId(RsslEncodeIterator *pIter, RsslBuffer groupId)
{
	return RSSL_RET_FAILURE;
}

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
RsslRet rsslReplacePostId(RsslEncodeIterator *pIter, RsslUInt32 postId)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Compares two MsgKey structures to determine if they are the same
 * 
 * @param pKey1 First key to compare 
 * @param pKey2 Second key to compare
 * @return RsslRet returns RSSL_RET_SUCCESS if keys match, RSSL_RET_FAILURE otherwise.
 */
RsslRet rsslCompareMsgKeys(const RsslMsgKey* pKey1, const RsslMsgKey* pKey2)
{
	return RSSL_RET_FAILURE;
}
 
/** 
 * @brief Performs a deep copy of a MsgKey.  Expects all memory to be owned and managed by user.  
 * 
 * @param pDestKey Destination to copy into.  Should have sufficient memory to contain copy of sourceKey.
 * @param pSourceKey Source to copy from.  
 * @return RsslRet returns RSSL_RET_SUCCESS if copy succeeds, RSSL_RET_FAILURE otherwise.
 */
RsslRet rsslCopyMsgKey(RsslMsgKey* pDestKey, const RsslMsgKey* pSourceKey)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief generates a hash ID from a message key.  
 * 
 * @returns hash ID
 */
RsslUInt32 rsslMsgKeyHash(const RsslMsgKey* pMsgKey)
{
	return 0;
}

/** 
 * @brief Adds a FilterId to the key filter
 * 
 * @param filterId The FilterId you want added to the filter. (e.g. RSSL_SERVICE_STATE_ID)
 * @param pFilter The filter to add the FilterId to
 */
RsslRet rsslAddFilterIdToFilter(const RsslUInt8 filterId, RsslUInt32 *pFilter)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Checks if FilterId is present in key filter
 * 
 * @param filterId The FilterId you want to check for
 * @param filter The filter to check for the FilterId
 */
RsslBool rsslCheckFilterForFilterId(const RsslUInt8 filterId, const RsslUInt32 filter)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Set the RSSL_RFMF_SOLICITED flag on an encoded RsslRefreshMsg buffer
 * @see RsslRefreshFlags
 */
RsslRet rsslSetSolicitedFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Unset the RSSL_RFMF_SOLICITED flag on an encoded RsslRefreshMsg buffer
 * @see RsslRefreshFlags
 */
RsslRet rsslUnsetSolicitedFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Set the RSSL_RFMF_REFRESH_COMPLETE flag on an encoded RsslRefreshMsg buffer
 * @see RsslRefreshFlags
 */
RsslRet rsslSetRefreshCompleteFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Unset the RSSL_RFMF_REFRESH_COMPLETE flag on an encoded RsslRefreshMsg buffer
 * @see RsslRefreshFlags
 */
RsslRet rsslUnsetRefreshCompleteFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Sets the ::RSSL_RQMF_STREAMING flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to apply flag to.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RsslRet rsslSetStreamingFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Unsets the ::RSSL_RQMF_STREAMING flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to remove flag from.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RsslRet rsslUnsetStreamingFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Sets the ::RSSL_RQMF_NO_REFRESH flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to apply flag to.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RsslRet rsslSetNoRefreshFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Unsets the ::RSSL_RQMF_NO_REFRESH flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to remove flag from.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RsslRet rsslUnsetNoRefreshFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Sets the ::RSSL_RQMF_MSG_KEY_IN_UPDATES flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to apply flag to.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RsslRet rsslSetMsgKeyInUpdatesFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Unsets the ::RSSL_RQMF_MSG_KEY_IN_UPDATES flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to remove flag from.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RsslRet rsslUnsetMsgKeyInUpdatesFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Sets the ::RSSL_RQMF_CONF_INFO_IN_UPDATES flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to apply flag to.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RsslRet rsslSetConfInfoInUpdatesFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Unsets the ::RSSL_RQMF_CONF_INFO_IN_UPDATES flag in the encoded message contained in the \ref RsslBuffer associated with the \ref RsslEncodeIterator.
 * 
 * @param pIter RsslEncodeIterator that contains the encoded message to remove flag from.
 * @see RsslRequestMsg, RsslRequestFlags, RsslEncodeIterator
 * @return RsslRet ::RSSL_RET_FAILURE if an error occurs, ::RSSL_RET_SUCCESS when encoded message is successfully modified.
 */
RsslRet rsslUnsetConfInfoInUpdatesFlag(RsslEncodeIterator *pIter)
{
	return RSSL_RET_FAILURE;
}

