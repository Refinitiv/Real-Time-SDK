/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|              Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslVAUtils.h"

RSSL_VA_API RsslRet rsslDeepCopyConnectOpts(RsslConnectOptions *destOpts, RsslConnectOptions *sourceOpts)
{
	size_t tempLen = 0;
	memset(destOpts, 0, sizeof(RsslConnectOptions));

	*destOpts = *sourceOpts;

#define DEEP_COPY_CONNECT_OPTS(FIELD)\
	if((sourceOpts-> FIELD) != 0)\
	{\
		tempLen = (strlen(sourceOpts-> FIELD) + 1) * sizeof(char);\
		(destOpts-> FIELD) = (char*)malloc(tempLen);\
		if ((destOpts-> FIELD) == 0)\
		{\
			rsslFreeConnectOpts(destOpts);\
			return RSSL_RET_FAILURE;\
		}\
		memcpy((destOpts-> FIELD), (sourceOpts-> FIELD), tempLen);\
	}
	
	DEEP_COPY_CONNECT_OPTS (hostName)
	DEEP_COPY_CONNECT_OPTS (serviceName)
	DEEP_COPY_CONNECT_OPTS (objectName)
	DEEP_COPY_CONNECT_OPTS (connectionInfo.segmented.recvAddress)
	DEEP_COPY_CONNECT_OPTS (connectionInfo.segmented.recvServiceName)
	DEEP_COPY_CONNECT_OPTS (connectionInfo.segmented.unicastServiceName)
	DEEP_COPY_CONNECT_OPTS (connectionInfo.segmented.interfaceName)
	DEEP_COPY_CONNECT_OPTS (connectionInfo.segmented.sendAddress)
	DEEP_COPY_CONNECT_OPTS (connectionInfo.segmented.sendServiceName)
	DEEP_COPY_CONNECT_OPTS (componentVersion)
	DEEP_COPY_CONNECT_OPTS (multicastOpts.hsmInterface)
	DEEP_COPY_CONNECT_OPTS (multicastOpts.hsmMultAddress)
	DEEP_COPY_CONNECT_OPTS (multicastOpts.hsmPort)
	DEEP_COPY_CONNECT_OPTS (multicastOpts.tcpControlPort)
	DEEP_COPY_CONNECT_OPTS (encryptionOpts.openSSLCAStore)
	DEEP_COPY_CONNECT_OPTS (encryptionOpts.cipherSuite)
	DEEP_COPY_CONNECT_OPTS (encryptionOpts.cipherSuite_TLSV1_3)
	DEEP_COPY_CONNECT_OPTS (wsOpts.protocols)

#undef DEEP_COPY_CONNECT_OPTS

	if (rsslDeepCopyProxyOpts(&destOpts->proxyOpts, &sourceOpts->proxyOpts) != RSSL_RET_SUCCESS)
	{
		rsslFreeConnectOpts(destOpts);
		return RSSL_RET_FAILURE;
	}
	
	return RSSL_RET_SUCCESS;
}
