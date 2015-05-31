/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef RSSL_VA_UTILS_H
#define RSSL_VA_UTILS_H
#include "rtr/rsslTransport.h"
#include "rtr/rsslVAExports.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup RSSLVAUtils
 *	@{
 */

/**
 *	@brief Performs a deep copy of a rsslConnectOpts structure.
 *
 *	@param destOpts RsslConnectOpts to be copied to.
 *	@param sourceOpts RsslConnectOpts to be copied from.
 *	@return RSSL_RET_SUCCESS if successful, RSSL_RET_FAILURE if an error occurred.
 */
RTR_C_INLINE RsslRet rsslDeepCopyConnectOpts(RsslConnectOptions *destOpts, RsslConnectOptions *sourceOpts)
{
	size_t tempLen = 0;
	memset(destOpts, 0, sizeof(RsslConnectOptions));

	destOpts->connectionType = sourceOpts->connectionType;
	destOpts->compressionType = sourceOpts->compressionType;
	destOpts->blocking = sourceOpts->blocking;
	destOpts->tcp_nodelay = sourceOpts->tcp_nodelay;
	destOpts->pingTimeout = sourceOpts->pingTimeout;
	destOpts->guaranteedOutputBuffers = sourceOpts->guaranteedOutputBuffers;
	destOpts->numInputBuffers = sourceOpts->numInputBuffers;
	destOpts->protocolType = sourceOpts->protocolType;
	destOpts->majorVersion = sourceOpts->majorVersion;
	destOpts->minorVersion = sourceOpts->minorVersion;
	destOpts->sysSendBufSize = sourceOpts->sysSendBufSize;
	destOpts->sysRecvBufSize = sourceOpts->sysRecvBufSize;
	destOpts->userSpecPtr = sourceOpts->userSpecPtr;

	if(sourceOpts->hostName != 0)
	{
		tempLen = (strlen(sourceOpts->hostName)+1)*sizeof(char);
		destOpts->hostName = (char*)malloc(tempLen);

		if(destOpts->hostName == 0)
			return RSSL_RET_FAILURE;

		strncpy(destOpts->hostName, sourceOpts->hostName, tempLen);
	}

	if(sourceOpts->serviceName != 0)
	{
		tempLen = (strlen(sourceOpts->serviceName)+1)*sizeof(char);
		destOpts->serviceName = (char*)malloc(tempLen);

		if(destOpts->serviceName == 0)
			return RSSL_RET_FAILURE;

		strncpy(destOpts->serviceName, sourceOpts->serviceName, tempLen);
	}

	if(sourceOpts->objectName != 0)
	{
		tempLen = (strlen(sourceOpts->objectName)+1)*sizeof(char);
		destOpts->objectName = (char*)malloc(tempLen);

		if(destOpts->objectName == 0)
			return RSSL_RET_FAILURE;

		strncpy(destOpts->objectName, sourceOpts->objectName, tempLen);
	}

	if(sourceOpts->connectionInfo.segmented.recvAddress != 0)
	{
		tempLen = (strlen(sourceOpts->connectionInfo.segmented.recvAddress)+1)*sizeof(char);
		destOpts->connectionInfo.segmented.recvAddress = (char*)malloc(tempLen);

		if(destOpts->connectionInfo.segmented.recvAddress == 0)
			return RSSL_RET_FAILURE;

		strncpy(destOpts->connectionInfo.segmented.recvAddress, sourceOpts->connectionInfo.segmented.recvAddress, tempLen);
	}

	if(sourceOpts->connectionInfo.segmented.recvServiceName != 0)
	{
		tempLen = (strlen(sourceOpts->connectionInfo.segmented.recvServiceName)+1)*sizeof(char);
		destOpts->connectionInfo.segmented.recvServiceName = (char*)malloc(tempLen);

		if(destOpts->connectionInfo.segmented.recvServiceName == 0)
			return RSSL_RET_FAILURE;
		strncpy(destOpts->connectionInfo.segmented.recvServiceName, sourceOpts->connectionInfo.segmented.recvServiceName, tempLen);
	}

	if(sourceOpts->connectionInfo.segmented.unicastServiceName != 0)
	{
		tempLen = (strlen(sourceOpts->connectionInfo.segmented.unicastServiceName)+1)*sizeof(char);
		destOpts->connectionInfo.segmented.unicastServiceName = (char*)malloc(tempLen);

		if(destOpts->connectionInfo.segmented.unicastServiceName == 0)
			return RSSL_RET_FAILURE;


		strncpy(destOpts->connectionInfo.segmented.unicastServiceName, sourceOpts->connectionInfo.segmented.unicastServiceName, tempLen);
	}

	if(sourceOpts->connectionInfo.segmented.interfaceName != 0)
	{
		tempLen = (strlen(sourceOpts->connectionInfo.segmented.interfaceName)+1)*sizeof(char);
		destOpts->connectionInfo.segmented.interfaceName = (char*)malloc(tempLen);

		if(destOpts->connectionInfo.segmented.interfaceName == 0)
			return RSSL_RET_FAILURE;

		strncpy(destOpts->connectionInfo.segmented.interfaceName, sourceOpts->connectionInfo.segmented.interfaceName, tempLen);
	}

	if(sourceOpts->connectionInfo.segmented.sendAddress != 0)
	{
		tempLen = (strlen(sourceOpts->connectionInfo.segmented.sendAddress)+1)*sizeof(char);
		destOpts->connectionInfo.segmented.sendAddress = (char*)malloc(tempLen);

		if(destOpts->connectionInfo.segmented.sendAddress == 0)
			return RSSL_RET_FAILURE;

		strncpy(destOpts->connectionInfo.segmented.sendAddress, sourceOpts->connectionInfo.segmented.sendAddress, tempLen);
	}

	if(sourceOpts->connectionInfo.segmented.sendServiceName != 0)
	{
		tempLen = (strlen(sourceOpts->connectionInfo.segmented.sendServiceName)+1)*sizeof(char);
		destOpts->connectionInfo.segmented.sendServiceName = (char*)malloc(tempLen);


		if(destOpts->connectionInfo.segmented.sendServiceName == 0)
			return RSSL_RET_FAILURE;

		strncpy(destOpts->connectionInfo.segmented.sendServiceName, sourceOpts->connectionInfo.segmented.sendServiceName, tempLen);
	}

	destOpts->tcpOpts.tcp_nodelay = sourceOpts->tcpOpts.tcp_nodelay;

	destOpts->multicastOpts = sourceOpts->multicastOpts;

	destOpts->shmemOpts.maxReaderLag = sourceOpts->shmemOpts.maxReaderLag;

	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE void rsslFreeConnectOpts(RsslConnectOptions *connOpts)
{
	if(connOpts->hostName != 0)
	{
		free(connOpts->hostName);
	}

	if(connOpts->serviceName != 0)
	{
		free(connOpts->serviceName);
	}

	if(connOpts->objectName != 0)
	{
		free(connOpts->objectName);
	}

	if(connOpts->connectionInfo.segmented.recvAddress != 0)
	{
		free(connOpts->connectionInfo.segmented.recvAddress);
	}

	if(connOpts->connectionInfo.segmented.recvServiceName != 0)
	{
		free(connOpts->connectionInfo.segmented.recvServiceName);
	}

	if(connOpts->connectionInfo.segmented.unicastServiceName != 0)
	{
		free(connOpts->connectionInfo.segmented.unicastServiceName);
	}

	if(connOpts->connectionInfo.segmented.interfaceName != 0)
	{
		free(connOpts->connectionInfo.segmented.interfaceName);
	}

	if(connOpts->connectionInfo.segmented.sendAddress != 0)
	{
		free( connOpts->connectionInfo.segmented.sendAddress);
	}

	if(connOpts->connectionInfo.segmented.sendServiceName != 0)
	{
		free(connOpts->connectionInfo.segmented.sendServiceName);
	}

	memset(connOpts, 0, sizeof(RsslConnectOptions));
}

/**
 *	@brief Creates a deep copy of an RsslRDMMsg.
 *
 *	@param pRdmMsg RsslRDMMsg to copy.
 *	@param lengthHint An initial size for the memory buffer created. If this is too small a larger size will be allocated.
 *	@returns The created RDMMsg, or NULL if an error occurred. If so, pRet is populated with: <br>
 *  - RSSL_RET_INVALID_ARGUMENT, if the message appears invalid.<br>
 *  - RSSL_RET_BUFFER_NO_BUFFERS, if memory could not be allocated.<br>
 *  - RSSL_RET_FAILURE, if the message could not be copied for some other reason.<br>
 */
static RsslRDMMsg *rsslCreateRDMMsgCopy(RsslRDMMsg *pRdmMsg, RsslUInt32 lengthHint, RsslRet *pRet)
{
	RsslUInt32 msgSize = sizeof(RsslRDMMsg);

	if (lengthHint == 0)
		lengthHint = 1;

	/* Copy the message, resizing if necessary. */
	while (1)
	{
		RsslRDMMsg *pNewRdmMsg;
		RsslRet ret;
		char *pData;
		RsslBuffer memoryBuffer;

		if ((pData = (char*)malloc(msgSize + lengthHint)) == NULL)
		{
			*pRet = RSSL_RET_BUFFER_NO_BUFFERS;
			return NULL;
		}

		/* Message struct goes first. */
		pNewRdmMsg = (RsslRDMMsg*)pData;

		/* Remaining memory is used for the deep copy. */
		memoryBuffer.data = pData + msgSize;
		memoryBuffer.length = lengthHint;

		ret = rsslCopyRDMMsg(pNewRdmMsg, pRdmMsg, &memoryBuffer);

		switch(ret)
		{
			case RSSL_RET_SUCCESS:
				*pRet = RSSL_RET_SUCCESS;
				return pNewRdmMsg;

			case RSSL_RET_BUFFER_TOO_SMALL:
				free(pData);
				lengthHint *= 2;
				continue;

			default:
				free(pData);
				*pRet = RSSL_RET_FAILURE;
				return NULL;
		}
	}
}

/**
 *	@}
 */

#ifdef __cplusplus
};
#endif

#endif
