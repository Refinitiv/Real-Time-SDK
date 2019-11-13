/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 Refinitiv. All rights reserved.
*/

#ifndef _RSSL_RDM_DICTIONARY_MSG_H
#define _RSSL_RDM_DICTIONARY_MSG_H

#include "rtr/rsslRDMMsgBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup VARDMDictionary
 *	@{
 */

/** 
 * @brief The types of RDM Dictionary Messages.  When the rdmMsgBase's domainType is RSSL_DMT_DICTIONARY, 
 * the rdmMsgType member may be set to one of these to indicate the specific RsslRDMDictionaryMsg class.
 * @see RsslRDMDictionaryMsg, RsslRDMMsgBase, RsslRDMDictionaryRequest, RsslRDMDictionaryClose, RsslRDMDictionaryRefresh, RsslRDMDictionaryStatus
 */
typedef enum {
	RDM_DC_MT_UNKNOWN	= 0,	/*!< (0) Unknown type. */
	RDM_DC_MT_REQUEST	= 1,	/*!< (1) Dictionary Request. */
	RDM_DC_MT_CLOSE		= 2,	/*!< (2) Dictionary Close. */
	RDM_DC_MT_REFRESH	= 3,	/*!< (3) Dictionary Refresh. */
	RDM_DC_MT_STATUS	= 4		/*!< (4) Dictionary Status. */
} RsslRDMDictionaryMsgType;

/**
 * @brief The RDM Dictionary Request Flags
 * @see RsslRDMDictionaryRequest
 */
typedef enum {
	RDM_DC_RQF_NONE			= 0x0,	/*!< (0x0) No flags set. */
	RDM_DC_RQF_STREAMING	= 0x1	/*!< (0x1) Indicates this is to be a streaming request. */
} RsslRDMDictionaryRequestFlags;

/**
 * @brief The RDM Dictionary Request.  Used by an OMM Consumer to request a dictionary from a services that provides it.
 * @see RsslRDMMsgBase, RsslRDMDictionaryMsg, rsslClearRDMDictionaryRequest
 */
typedef struct {
	RsslRDMMsgBase					rdmMsgBase;			/*!< The Base RDM Message. */
	RsslUInt32						flags;				/*!< The RDM Dictionary Request Flags. */
	RsslUInt16						serviceId;			/*!< The ID of the service to request the dictionary from. */
	RsslUInt32						verbosity;			/*!< The verbosity of information desired.  Populated by RDMDictionaryVerbosityValues. */
	RsslBuffer						dictionaryName;		/*!< The name of the requested dictionary. */
} RsslRDMDictionaryRequest;

/**
 * @brief Clears an RsslRDMDictionaryRequest.
 * @see RsslRDMDictionaryRequest
 */
RTR_C_INLINE void rsslClearRDMDictionaryRequest(RsslRDMDictionaryRequest *pRequest)
{
	rsslClearRDMMsgBase(&pRequest->rdmMsgBase);
	pRequest->rdmMsgBase.rdmMsgType = RDM_DC_MT_REQUEST;
	pRequest->rdmMsgBase.domainType = RSSL_DMT_DICTIONARY;
	pRequest->flags = RDM_DC_RQF_NONE;
	pRequest->serviceId = 0;
	pRequest->verbosity = RDM_DICTIONARY_NORMAL;
	rsslClearBuffer(&pRequest->dictionaryName);
}

/**
 * @brief The RDM Dictionary Refresh Flags
 * @see RsslRDMDictionaryRefresh
 */
typedef enum {
	RDM_DC_RFF_NONE			= 0x00,	/*!< (0x00) No flags set. */
	RDM_DC_RFF_HAS_INFO		= 0x01, /*!< (0x01) When decoding, indicates presence of the dictionaryId, version, and type members. This flag is not used while encoding as this information is automatically added by the encode function when appropriate. */
	RDM_DC_RFF_IS_COMPLETE	= 0x02, /*!< (0x02) When decoding, indicates that this is the final part of the dictionary refresh. This flag is not used while encoding as it is automatically set by the encode function set when appropriate. */
	RDM_DC_RFF_SOLICITED	= 0x04,	/*!< (0x04) Indicates that this refresh is being provided in response to a request. */
	RDM_DC_RFF_HAS_SEQ_NUM	= 0x08,	/*!< (0x08) Indicates presence of the sequenceNumber member. */
	RDM_DC_RFF_CLEAR_CACHE	= 0x10	/*!< (0x10) Indicates the receiver of the refresh should clear any existing cached information. */
} RsslRDMDictionaryRefreshFlags;

/**
 * @brief The RDM Dictionary Refresh. Used by an OMM Provider to provide the content of a Dictionary.<BR>
 * NOTE: The Field dictionary type supports being split into fragments, each containing some of the fields.
 * rsslEncodeRDMDictionaryMsg() may need to be called multiple times to send the entire message. The startFid parameter
 * will be updated with each call.
 * @see RsslRDMMsgBase, RsslRDMMsg, RsslRDMDictionaryRefreshFlags, rsslClearRDMDictionaryRefresh
 */
typedef struct
{
	RsslRDMMsgBase					rdmMsgBase;			/*!< The Base RDM Message. */
	RsslUInt32						flags;				/*!< The RDM Dictionary Refresh flags. */
	RsslState						state;				/*!< The current state of the stream. */
	RsslBuffer						dictionaryName;		/*!< The name of the dictionary being provided. This should match what was requested. */
	RsslUInt16						serviceId;			/*!< The ID of the service providing the dictionary. */
	RsslUInt32						verbosity;			/*!< The verbosity of the dictionary being provided. Populated by RDMDictionaryVerbosityValues. */
	RsslUInt						type;				/*!< The type of the dictionary. Populated by RDMDictionaryTypes. Only the values RDM_DICTIONARY_FIELD_DEFINITIONS and RDM_DICTIONARY_ENUM_TABLES are supported by the encoder. */
	RsslUInt32						sequenceNumber;		/*!< The sequence number of this message. */
	RsslDataDictionary				*pDictionary;		/*!< When encoding the message, this points to the dictionary object that is being encoded. When decoding, this is not used. */
	RsslInt32						startFid;			/*!< When encoding the message, this represents the first Field ID that will be encoded in the current fragment.  When decoding, this is not used. */
	RsslInt32						enumStartFid;		/*!< When encoding the message, this represents the first Enum Field ID that will be encoded in the current fragment.  When decoding, this is not used. */
	RsslInt							dictionaryId;		/*!< The ID of this dictionary's family. */
	RsslBuffer						version;			/*!< The Version of this dictionary. */
	RsslBuffer						dataBody;			/*!< When decoding, points to the payload of the message.  The application should set the iterator to 
														 * this buffer and call the appropriate decode function. This will add the data present
														 * to the RsslDataDictionary object. When encoding, this member is not used. */
} RsslRDMDictionaryRefresh;

/**
 * @brief Clears an RsslRDMDictionaryRefresh.
 * @see RsslRDMDictionaryRefresh
 */
RTR_C_INLINE void rsslClearRDMDictionaryRefresh(RsslRDMDictionaryRefresh *pDictionaryRefresh)
{
	rsslClearRDMMsgBase(&pDictionaryRefresh->rdmMsgBase);
	pDictionaryRefresh->rdmMsgBase.rdmMsgType = RDM_DC_MT_REFRESH;
	pDictionaryRefresh->rdmMsgBase.domainType = RSSL_DMT_DICTIONARY;
	pDictionaryRefresh->flags = RDM_DC_RFF_NONE;
	rsslClearState(&pDictionaryRefresh->state);
	pDictionaryRefresh->state.streamState = RSSL_STREAM_OPEN;
	pDictionaryRefresh->state.dataState = RSSL_DATA_OK;
	pDictionaryRefresh->state.code = RSSL_SC_NONE;
	rsslClearBuffer(&pDictionaryRefresh->dictionaryName);
	pDictionaryRefresh->serviceId = 0;
	pDictionaryRefresh->verbosity = RDM_DICTIONARY_NORMAL;
	pDictionaryRefresh->type = 0;
	pDictionaryRefresh->sequenceNumber = 0;
	pDictionaryRefresh->pDictionary = NULL;
	pDictionaryRefresh->startFid = RSSL_MIN_FID;
	pDictionaryRefresh->enumStartFid = 0;
	pDictionaryRefresh->dictionaryId = 1;
	rsslClearBuffer(&pDictionaryRefresh->version);
	rsslClearBuffer(&pDictionaryRefresh->dataBody);
}

/**
 * @brief The RDM Dictionary Close.  Used by an OMM Consumer to close a Dictionary stream.
 * @see RsslRDMMsgBase, RsslRDMDictionary, rsslClearRDMDictionaryClose
 */
typedef struct {
	RsslRDMMsgBase rdmMsgBase;	/*!< The Base RDM Message. */
} RsslRDMDictionaryClose;

/**
 * @brief Clears an RsslRDMDictionaryClose.
 * @see RsslRDMDictionaryClose
 */
RTR_C_INLINE void rsslClearRDMDictionaryClose(RsslRDMDictionaryClose *pClose)
{
	rsslClearRDMMsgBase(&pClose->rdmMsgBase);
	pClose->rdmMsgBase.rdmMsgType = RDM_DC_MT_CLOSE;
	pClose->rdmMsgBase.domainType = RSSL_DMT_DICTIONARY;
}


/**
 * @brief The RDM Dictionary Status Flags
 * @see RsslRDMDictionaryStatus
 */
typedef enum
{
	RDM_DC_STF_NONE			= 0x00,	/*!< (0x00) No flags set. */
	RDM_DC_STF_HAS_STATE	= 0x01	/*!< (0x01) Indicates presence of the state member. */
} RsslRDMDictionaryStatusFlags;

/**
 * @brief The RDM Dictionary Status. Used by an OMM Provider to indicate changes to the Dictionary stream.
 * @see RsslRDMMsgBase, RsslRDMDictionaryMsg, rsslClearRDMDictionaryStatus
 */
typedef struct {
	RsslRDMMsgBase					rdmMsgBase;	/*!< The base RDM Message. */
	RsslUInt32						flags;		/*!< The RDM Dictionary Status flags. Populated by RsslRDMDictionaryStatusFlags. */
	RsslState						state;		/*!< The current state of the Dictionary stream. */
} RsslRDMDictionaryStatus;

/**
 * @brief Clears an RsslRDMDictionaryStatus.
 * @see RsslRDMDictionaryStatus
 */
RTR_C_INLINE void rsslClearRDMDictionaryStatus(RsslRDMDictionaryStatus *pStatus)
{
	rsslClearRDMMsgBase(&pStatus->rdmMsgBase);
	pStatus->rdmMsgBase.rdmMsgType = RDM_DC_MT_STATUS;
	pStatus->rdmMsgBase.domainType = RSSL_DMT_DICTIONARY;
	pStatus->flags = RDM_DC_STF_NONE;
	rsslClearState(&pStatus->state);
	pStatus->state.streamState = RSSL_STREAM_OPEN;
	pStatus->state.dataState = RSSL_DATA_OK;
	pStatus->state.code = RSSL_SC_NONE;
}

/**
 * @brief The RDM Dictionary Msg.  The Dictionary Message encoder and decoder functions expect this type.
 * It is a group of the classes of message that the Dictionary domain supports.  Any 
 * specific message class may be cast to an RsslRDMDictionaryMsg, and an RsslRDMDictionaryMsg may be cast 
 * to any specific message class.  The RsslRDMMsgBase contains members common to each class
 * that may be used to identify the class of message.
 * @see RsslRDMDictionaryMsgType, RsslRDMMsgBase, RsslRDMDictionaryRequest, RsslRDMDictionaryClose, RsslRDMDictionaryRefresh, RsslRDMDictionaryStatus, rsslClearRDMDictionaryMsg
 */
typedef union {
	RsslRDMMsgBase				rdmMsgBase;	/*!< The Base RDM Message. */
	RsslRDMDictionaryRequest	request;	/*!< Dictionary Request. */
	RsslRDMDictionaryClose		close;		/*!< Dictionary Close. */
	RsslRDMDictionaryRefresh	refresh;	/*!< Dictionary Refresh. */
	RsslRDMDictionaryStatus		status;		/*!< Dictionary Status. */
} RsslRDMDictionaryMsg;

/**
 * @brief Clears an RsslRDMDictionaryMsg.
 * @see RsslRDMDictionaryMsg
 */
RTR_C_INLINE void rsslClearRDMDictionaryMsg(RsslRDMDictionaryMsg *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMDictionaryMsg));
	pMsg->rdmMsgBase.domainType = RSSL_DMT_DICTIONARY;
}

/**
 * @brief Encodes an RsslRDMDictionaryMsg.<BR>
 * When encoding an RsslRDMDictionaryRefresh of type RDM_DICTIONARY_FIELD_DEFINITIONS, this function may need 
 * to be called multiple times to send the entire refresh. This is because the Field Dictionary supports
 * being split into fragments when encoding.  When this occurs, RSSL_RET_DICT_PART_ENCODED is returned instead
 * of RSSL_RET_SUCCESS and the RsslRDMDictionaryRefresh structure is updated.  To continue encoding,
 * set pEncodeIter to a new RsslBuffer and call rsslEncodeRDMDictionaryMsg() again.
 * @param pEncodeIter The Encode Iterator
 * @param pDictionaryMsg The RDM Dictionary Message to Encode
 * @param pBytesWritten Returns the total number of bytes used to encode the message.
 * @return RSSL_RET_SUCCESS when successful.
 * @return RSSL_RET_DICT_PART_ENCODED, when encoding was successful but there is still more data in the dictionary to send.
 * @see RsslRDMDictionaryMsg
 */
RSSL_VA_API RsslRet rsslEncodeRDMDictionaryMsg(RsslEncodeIterator *pEncodeIter, RsslRDMDictionaryMsg *pDictionaryMsg, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError);

/**
 * @brief Decodes an RsslRDMDictionaryMsg.
 * @param pEncodeIter The Decode Iterator
 * @param pDictionaryMsg The RDM Dictionary Message to be populated
 * @param pMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the decoded message.
 * @param pError An Error Structure, which will be populated if the decoding fails.
 * @return RSSL_RET_SUCCESS, if the message was succesfully decoded and correctly followed the RDM.
 * @return RSSL_RET_FAILURE, if the message was not successfully decoded or did  follow the RDM.  Information about the error will be stored in pError.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDictionaryMsg
 */
RSSL_VA_API RsslRet rsslDecodeRDMDictionaryMsg(RsslDecodeIterator *pIter, RsslMsg *pMsg, RsslRDMDictionaryMsg *pDictionaryMsg, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError);

/**
 *	@addtogroup VARDMDictionaryHelper
 *	@{
 */

/**
 * @brief Fully copies an RsslRDMDictionaryMsg.
 * @param pNewMsg The resulting copy of the RDM Dictionary Message
 * @param pOldMsg The RDM Dictionary Message to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDictionaryMsg
 */
RSSL_VA_API RsslRet rsslCopyRDMDictionaryMsg(RsslRDMDictionaryMsg *pNewMsg, RsslRDMDictionaryMsg *pOldMsg, RsslBuffer *pNewMemoryBuffer);


/**
 * @brief Fully copies an RsslRDMDictionaryRequest.
 * @param pNewRequest The resulting copy of the RDM Dictionary Request
 * @param pOldRequest The RDM Dictionary Request to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDictionaryRequest
 */
RSSL_VA_API RsslRet rsslCopyRDMDictionaryRequest(RsslRDMDictionaryRequest *pNewRequest, RsslRDMDictionaryRequest *pOldRequest, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMDictionaryRefresh.
 * @param pNewRefresh The resulting copy of the RDM Dictionary Refresh
 * @param pOldRefresh The RDM Dictionary Refresh to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDictionaryRefresh
 */
RSSL_VA_API RsslRet rsslCopyRDMDictionaryRefresh(RsslRDMDictionaryRefresh *pNewRefresh, RsslRDMDictionaryRefresh *pOldRefresh, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMDictionaryClose.
 * @param pNewClose The resulting copy of the RDM Dictionary Close
 * @param pOldClose The RDM Dictionary Close to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDictionaryClose
 */
RSSL_VA_API RsslRet rsslCopyRDMDictionaryClose(RsslRDMDictionaryClose *pNewClose, RsslRDMDictionaryClose *pOldClose, RsslBuffer *pNewMemoryBuffer);

/**
 * @brief Fully copies an RsslRDMDictionaryStatus.
 * @param pNewStatus The resulting copy of the RDM Dictionary Status
 * @param pOldStatus The RDM Dictionary Status to be copied
 * @param pNewMemoryBuffer An RsslBuffer pointing to space that may be used as additional storage space for the new message.
 * @return RSSL_RET_SUCCESS, if the message was succesfully copied.
 * @return RSSL_RET_BUFFER_TOO_SMALL, if the size of pNewMemoryBuffer was not large enough to store all additional data. 
 * @see RsslRDMDictionaryStatus
 */
RSSL_VA_API RsslRet rsslCopyRDMDictionaryStatus(RsslRDMDictionaryStatus *pNewStatus, RsslRDMDictionaryStatus *pOldStatus, RsslBuffer *pNewMemoryBuffer);

/**
 *	@}
 */

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif
