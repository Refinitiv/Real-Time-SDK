/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019-2020,2023-2024 LSEG. All rights reserved.     
*/

#ifndef RSSL_VA_UTILS_H
#define RSSL_VA_UTILS_H
#include "rtr/rsslTransport.h"
#include "rtr/rsslVAExports.h"
#include "rtr/rsslReactor.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *	@addtogroup RSSLVAUtils
 *	@{
 */

/**
 *	@brief Performs a deep copy of a RsslProxyOpts structure.
 *
 *	@param destProxyOpts RsslProxyOpts to be copied to.
 *	@param sourceProxyOpts RsslProxyOpts to be copied from.
 *	@return RSSL_RET_SUCCESS if successful, RSSL_RET_FAILURE if an error occurred.
 */
RTR_C_INLINE RsslRet rsslDeepCopyProxyOpts(RsslProxyOpts* destProxyOpts, RsslProxyOpts* sourceProxyOpts)
{
	size_t tempLen = 0;

	if (sourceProxyOpts->proxyHostName != 0)
	{
		tempLen = (strlen(sourceProxyOpts->proxyHostName) + 1) * sizeof(char);
		destProxyOpts->proxyHostName = (char*)malloc(tempLen);

		if (destProxyOpts->proxyHostName == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destProxyOpts->proxyHostName, sourceProxyOpts->proxyHostName, tempLen);
	}

	if (sourceProxyOpts->proxyPort != 0)
	{
		tempLen = (strlen(sourceProxyOpts->proxyPort) + 1) * sizeof(char);
		destProxyOpts->proxyPort = (char*)malloc(tempLen);

		if (destProxyOpts->proxyPort == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destProxyOpts->proxyPort, sourceProxyOpts->proxyPort, tempLen);
	}

	if (sourceProxyOpts->proxyUserName != 0)
	{
		tempLen = (strlen(sourceProxyOpts->proxyUserName) + 1) * sizeof(char);
		destProxyOpts->proxyUserName = (char*)malloc(tempLen);

		if (destProxyOpts->proxyUserName == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destProxyOpts->proxyUserName, sourceProxyOpts->proxyUserName, tempLen);
	}

	if (sourceProxyOpts->proxyPasswd != 0)
	{
		tempLen = (strlen(sourceProxyOpts->proxyPasswd) + 1) * sizeof(char);
		destProxyOpts->proxyPasswd = (char*)malloc(tempLen);

		if (destProxyOpts->proxyPasswd == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destProxyOpts->proxyPasswd, sourceProxyOpts->proxyPasswd, tempLen);
	}

	if (sourceProxyOpts->proxyDomain != 0)
	{
		tempLen = (strlen(sourceProxyOpts->proxyDomain) + 1) * sizeof(char);
		destProxyOpts->proxyDomain = (char*)malloc(tempLen);

		if (destProxyOpts->proxyDomain == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destProxyOpts->proxyDomain, sourceProxyOpts->proxyDomain, tempLen);
	}

	return RSSL_RET_SUCCESS;
}

/**
 *	@brief Performs a deep copy of a RsslPreferredHostOptions structure.
 *
 *	@param destPrefHostOpts RsslPreferredHostOptions to be copied to.
 *	@param sourcePrefHostOpts RsslPreferredHostOptions to be copied from.
 *	@return RSSL_RET_SUCCESS if successful, RSSL_RET_FAILURE if an error occurred.
 */
RTR_C_INLINE RsslRet rsslDeepCopyPreferredHostOpts(RsslPreferredHostOptions* destPrefHostOpts,
												RsslPreferredHostOptions* sourcePrefHostOpts)
{
	if (sourcePrefHostOpts->enablePreferredHostOptions)
	{
		destPrefHostOpts->enablePreferredHostOptions = sourcePrefHostOpts->enablePreferredHostOptions;

		if (sourcePrefHostOpts->detectionTimeSchedule.data != 0 && sourcePrefHostOpts->detectionTimeSchedule.length > 0)
		{
			destPrefHostOpts->detectionTimeSchedule.length = sourcePrefHostOpts->detectionTimeSchedule.length;
			destPrefHostOpts->detectionTimeSchedule.data = (char*)malloc(destPrefHostOpts->detectionTimeSchedule.length + 1);

			if (destPrefHostOpts->detectionTimeSchedule.data == 0)
			{
				return RSSL_RET_FAILURE;
			}

			memset(destPrefHostOpts->detectionTimeSchedule.data, 0, destPrefHostOpts->detectionTimeSchedule.length);
			strncpy(destPrefHostOpts->detectionTimeSchedule.data, sourcePrefHostOpts->detectionTimeSchedule.data,
				destPrefHostOpts->detectionTimeSchedule.length);
		}

		destPrefHostOpts->detectionTimeInterval = sourcePrefHostOpts->detectionTimeInterval;
		destPrefHostOpts->connectionListIndex = sourcePrefHostOpts->connectionListIndex;
		destPrefHostOpts->warmStandbyGroupListIndex = sourcePrefHostOpts->warmStandbyGroupListIndex;
		destPrefHostOpts->fallBackWithInWSBGroup = sourcePrefHostOpts->fallBackWithInWSBGroup;
	}
	else
	{
		rsslClearRsslPreferredHostOptions(destPrefHostOpts);
	}

	return RSSL_RET_SUCCESS;
}

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

	*destOpts = *sourceOpts;
	
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
	
	if (sourceOpts->componentVersion != 0)
	{
		tempLen = (strlen(sourceOpts->componentVersion)+1)*sizeof(char);
		destOpts->componentVersion = (char*)malloc(tempLen);

		if (destOpts->componentVersion == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destOpts->componentVersion, sourceOpts->componentVersion, tempLen);
	}
	
	if (sourceOpts->multicastOpts.hsmInterface != 0)
	{
		tempLen = (strlen(sourceOpts->multicastOpts.hsmInterface)+1)*sizeof(char);
		destOpts->multicastOpts.hsmInterface = (char*)malloc(tempLen);

		if (destOpts->multicastOpts.hsmInterface == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destOpts->multicastOpts.hsmInterface, sourceOpts->multicastOpts.hsmInterface, tempLen);
	}
	
	if (sourceOpts->multicastOpts.hsmMultAddress != 0)
	{
		tempLen = (strlen(sourceOpts->multicastOpts.hsmMultAddress)+1)*sizeof(char);
		destOpts->multicastOpts.hsmMultAddress = (char*)malloc(tempLen);

		if (destOpts->multicastOpts.hsmMultAddress == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destOpts->multicastOpts.hsmMultAddress, sourceOpts->multicastOpts.hsmMultAddress, tempLen);
	}
	
	if (sourceOpts->multicastOpts.hsmPort != 0)
	{
		tempLen = (strlen(sourceOpts->multicastOpts.hsmPort)+1)*sizeof(char);
		destOpts->multicastOpts.hsmPort = (char*)malloc(tempLen);

		if (destOpts->multicastOpts.hsmPort == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destOpts->multicastOpts.hsmPort, sourceOpts->multicastOpts.hsmPort, tempLen);
	}
	
	if (sourceOpts->multicastOpts.tcpControlPort != 0)
	{
		tempLen = (strlen(sourceOpts->multicastOpts.tcpControlPort)+1)*sizeof(char);
		destOpts->multicastOpts.tcpControlPort = (char*)malloc(tempLen);

		if (destOpts->multicastOpts.tcpControlPort == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destOpts->multicastOpts.tcpControlPort, sourceOpts->multicastOpts.tcpControlPort, tempLen);
	}

	if (sourceOpts->encryptionOpts.openSSLCAStore != 0)
	{
		tempLen = (strlen(sourceOpts->encryptionOpts.openSSLCAStore) + 1) * sizeof(char);
		destOpts->encryptionOpts.openSSLCAStore = (char*)malloc(tempLen);

		if (destOpts->encryptionOpts.openSSLCAStore == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strncpy(destOpts->encryptionOpts.openSSLCAStore, sourceOpts->encryptionOpts.openSSLCAStore, tempLen);
	}

	if (sourceOpts->wsOpts.protocols != 0)
	{
		tempLen = (strlen(sourceOpts->wsOpts.protocols) + 1) * sizeof(char);
		destOpts->wsOpts.protocols = (char*)malloc(tempLen);

		if (destOpts->wsOpts.protocols == 0)
		{
			return RSSL_RET_FAILURE;
		}

		strcpy(destOpts->wsOpts.protocols, sourceOpts->wsOpts.protocols);
	}

	if (rsslDeepCopyProxyOpts(&destOpts->proxyOpts, &sourceOpts->proxyOpts) != RSSL_RET_SUCCESS)
	{
		return RSSL_RET_FAILURE;
	}
	
	return RSSL_RET_SUCCESS;
}

RTR_C_INLINE void rsslFreeProxyOpts(RsslProxyOpts* proxyOpts)
{
	if (proxyOpts->proxyHostName != 0)
	{
		free(proxyOpts->proxyHostName);
	}

	if (proxyOpts->proxyPort != 0)
	{
		free(proxyOpts->proxyPort);
	}

	if (proxyOpts->proxyUserName != 0)
	{
		free(proxyOpts->proxyUserName);
	}

	if (proxyOpts->proxyPasswd != 0)
	{
		free(proxyOpts->proxyPasswd);
	}

	if (proxyOpts->proxyDomain != 0)
	{
		free(proxyOpts->proxyDomain);
	}

	memset(proxyOpts, 0, sizeof(RsslProxyOpts));
}

RTR_C_INLINE void rsslFreePreferredHostOpts(RsslPreferredHostOptions* prefHostOpts)
{
	if (prefHostOpts->detectionTimeSchedule.data != NULL)
	{
		// The pointer will be set to NULL in the clear call below.
		free(prefHostOpts->detectionTimeSchedule.data);
	}

	rsslClearRsslPreferredHostOptions(prefHostOpts);
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
	
	if(connOpts->componentVersion != 0)
	{
		free(connOpts->componentVersion);
	}
	
	if(connOpts->multicastOpts.hsmInterface != 0)
	{
		free(connOpts->multicastOpts.hsmInterface);
	}
	
	if(connOpts->multicastOpts.hsmMultAddress != 0)
	{
		free(connOpts->multicastOpts.hsmMultAddress);
	}
	
	if(connOpts->multicastOpts.hsmPort != 0)
	{
		free(connOpts->multicastOpts.hsmPort);
	}
	
	if(connOpts->multicastOpts.tcpControlPort != 0)
	{
		free(connOpts->multicastOpts.tcpControlPort);
	}

	if(connOpts->encryptionOpts.openSSLCAStore != 0)
	{
		free(connOpts->encryptionOpts.openSSLCAStore);
	}

	if (connOpts->wsOpts.protocols != 0)
	{
		free(connOpts->wsOpts.protocols);
	}

	rsslFreeProxyOpts(&connOpts->proxyOpts);

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
 *	@brief Performs a deep copy of a RsslBuffer instance.
 *
 *	@param destBuffer RsslBuffer to be copied to.
 *	@param sourceBuffer RsslBuffer to be copied from.
 *	@return RSSL_RET_SUCCESS if successful, RSSL_RET_FAILURE if an error occurred.
 */
RTR_C_INLINE RsslRet rsslDeepCopyRsslBuffer(RsslBuffer* destBuffer, RsslBuffer* sourceBuffer)
{
	if (sourceBuffer->data != 0 && sourceBuffer->length > 0)
	{
		destBuffer->length = sourceBuffer->length;
		destBuffer->data = (char*)malloc(destBuffer->length);

		if (destBuffer->data == 0)
		{
			return RSSL_RET_FAILURE;
		}

		memset(destBuffer->data, 0, destBuffer->length);
		memcpy(destBuffer->data, sourceBuffer->data, destBuffer->length);
	}

	return RSSL_RET_SUCCESS;
}

/**
 *	@}
 */

#ifdef __cplusplus
};
#endif

#endif
