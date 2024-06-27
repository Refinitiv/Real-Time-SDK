/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#ifndef _RSSL_RDM_PACKAGE_H
#define _RSSL_RDM_PACKAGE_H

#include "rtr/rsslRDMMsgBase.h"
#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslRDMDirectoryMsg.h"
#include "rtr/rsslRDMDictionaryMsg.h"
#include "rtr/rsslRDMQueueMsg.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup VARDMCommon
 *	@{
 */
 

/**
 *	@brief RSSL Message Union contains all messages. This includes constructs, functions, and values. 
 */
typedef union {
	RsslRDMMsgBase				rdmMsgBase;
	RsslRDMLoginMsg				loginMsg;
	RsslRDMDirectoryMsg			directoryMsg;
	RsslRDMDictionaryMsg		dictionaryMsg;
	RsslRDMQueueMsg				queueMsg;
} RsslRDMMsg;

RTR_C_INLINE void rsslClearRDMMsg(RsslRDMMsg *pMsg)
{
	memset(pMsg, 0, sizeof(RsslRDMMsg));
}

/**
 *	@brief This function encodes the given RDM Message.
 */
RTR_C_INLINE RsslRet rsslEncodeRDMMsg(RsslEncodeIterator *pIter, RsslRDMMsg *pRDMMsg, RsslUInt32 *pBytesWritten, RsslErrorInfo *pError)
{
	switch(pRDMMsg->rdmMsgBase.domainType)
	{
	case RSSL_DMT_LOGIN: return rsslEncodeRDMLoginMsg(pIter, (RsslRDMLoginMsg*)pRDMMsg, pBytesWritten, pError);
		case RSSL_DMT_SOURCE: return rsslEncodeRDMDirectoryMsg(pIter, (RsslRDMDirectoryMsg*)pRDMMsg, pBytesWritten, pError);
		case RSSL_DMT_DICTIONARY:return rsslEncodeRDMDictionaryMsg(pIter, (RsslRDMDictionaryMsg*)pRDMMsg, pBytesWritten, pError);
		default: return RSSL_RET_INVALID_ARGUMENT;
	}
}


/**
 *	@brief Attempts to decode an Rssl message buffer into a domain message structure. 
 */
RTR_C_INLINE RsslRet rsslDecodeRDMMsg(RsslDecodeIterator *pIter, RsslMsg *pMsg, RsslRDMMsg *pRDMMsg, RsslBuffer *pMemoryBuffer, RsslErrorInfo *pError)
{
	RsslRet ret;

	ret = rsslDecodeMsg(pIter, pMsg);

	if (ret != RSSL_RET_SUCCESS)
		return ret;

	switch(pMsg->msgBase.domainType)
	{
		case RSSL_DMT_LOGIN: return rsslDecodeRDMLoginMsg(pIter, pMsg, (RsslRDMLoginMsg*)pRDMMsg, pMemoryBuffer, pError);
		case RSSL_DMT_SOURCE: return rsslDecodeRDMDirectoryMsg(pIter, pMsg, (RsslRDMDirectoryMsg*)pRDMMsg, pMemoryBuffer, pError);
		case RSSL_DMT_DICTIONARY:return rsslDecodeRDMDictionaryMsg(pIter, pMsg, (RsslRDMDictionaryMsg*)pRDMMsg, pMemoryBuffer, pError);
		default: pRDMMsg->rdmMsgBase.domainType = pMsg->msgBase.domainType; return RSSL_RET_SUCCESS;
	}
}

/**
 *	@brief Copies the pOldMsg to pNewMsg.
 */
RTR_C_INLINE RsslRet rsslCopyRDMMsg(RsslRDMMsg *pNewMsg, RsslRDMMsg *pOldMsg, RsslBuffer *pNewMemoryBuffer)
{
	switch(pOldMsg->rdmMsgBase.domainType)
	{
		case RSSL_DMT_LOGIN: return rsslCopyRDMLoginMsg((RsslRDMLoginMsg*)pNewMsg, (RsslRDMLoginMsg*)pOldMsg, pNewMemoryBuffer);
		case RSSL_DMT_SOURCE: return rsslCopyRDMDirectoryMsg((RsslRDMDirectoryMsg*)pNewMsg, (RsslRDMDirectoryMsg*)pOldMsg, pNewMemoryBuffer);
		case RSSL_DMT_DICTIONARY: return rsslCopyRDMDictionaryMsg((RsslRDMDictionaryMsg*)pNewMsg, (RsslRDMDictionaryMsg*)pOldMsg, pNewMemoryBuffer);
		default: return RSSL_RET_INVALID_ARGUMENT;
	}
}

/**
 *	@}
 */

#ifdef __cplusplus
}
#endif

#endif
