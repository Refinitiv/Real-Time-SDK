/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#include "rtr/tunnelManagerImpl.h"
#include "rtr/tunnelStreamImpl.h"
#include "rtr/msgQueueEncDec.h"

#include <stdlib.h>
#include <assert.h>

RSSL_VA_API RsslBuffer *rsslTunnelStreamGetBuffer(RsslTunnelStream *pTunnel, 
		RsslTunnelStreamGetBufferOptions *pRsslTunnelStreamGetBufferOptions, RsslErrorInfo *pErrorInfo)
{
	TunnelStreamImpl	*pTunnelImpl;
	TunnelBufferImpl	*pBufferImpl;
	RsslReactorImpl		*pReactorImpl;

	if (pTunnel == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStream not provided.");
		return NULL;
	}

	if (pRsslTunnelStreamGetBufferOptions == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStreamGetBufferOptions not provided.");
		return NULL;
	}

	if (pErrorInfo == NULL)
		return NULL;

	pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	pReactorImpl = (RsslReactorImpl*)pTunnelImpl->_manager->_pParentReactor;

	if (reactorLockInterface(pReactorImpl, RSSL_TRUE, pErrorInfo) != RSSL_RET_SUCCESS)
		return NULL;

	pBufferImpl = tunnelStreamGetBuffer(pTunnelImpl, pRsslTunnelStreamGetBufferOptions->size, RSSL_TRUE, RSSL_FALSE, pErrorInfo);
	return (reactorUnlockInterface(pReactorImpl), (RsslBuffer*)pBufferImpl);
}

RSSL_VA_API RsslRet rsslTunnelStreamReleaseBuffer(RsslBuffer *pBuffer, RsslErrorInfo *pErrorInfo)
{
	TunnelBufferImpl	*pBufferImpl;
	TunnelStreamImpl	*pTunnelImpl;
	RsslReactorImpl		*pReactorImpl;
	RsslRet ret;

	if (pBuffer == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslBuffer not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pErrorInfo == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	pBufferImpl = (TunnelBufferImpl*)pBuffer;
	pTunnelImpl = (TunnelStreamImpl*)pBufferImpl->_tunnel;
	pReactorImpl = (RsslReactorImpl*)pTunnelImpl->_manager->_pParentReactor;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	tunnelStreamReleaseBuffer(pTunnelImpl, pBufferImpl);

	return (reactorUnlockInterface(pReactorImpl), RSSL_RET_SUCCESS);
}

RSSL_VA_API RsslRet rsslTunnelStreamGetInfo(RsslTunnelStream *pTunnel, RsslTunnelStreamInfo *pInfo, RsslErrorInfo *pErrorInfo)
{
	TunnelStreamImpl	*pTunnelImpl;
	RsslReactorImpl		*pReactorImpl;
	RsslRet ret;

	if (pErrorInfo == NULL)
		return RSSL_RET_INVALID_ARGUMENT;

	if (pTunnel == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "RsslTunnelStream not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	if (pInfo == NULL)
	{
		rsslSetErrorInfo(pErrorInfo, RSSL_EIC_FAILURE, RSSL_RET_INVALID_ARGUMENT, __FILE__, __LINE__, "ValuePtr not provided.");
		return RSSL_RET_INVALID_ARGUMENT;
	}

	pTunnelImpl = (TunnelStreamImpl*)pTunnel;
	pReactorImpl = (RsslReactorImpl*)pTunnelImpl->_manager->_pParentReactor;

	if ((ret = reactorLockInterface(pReactorImpl, RSSL_TRUE, pErrorInfo)) != RSSL_RET_SUCCESS)
		return ret;

	rsslClearTunnelStreamInfo(pInfo);
	ret = tunnelStreamGetInfo(pTunnelImpl, pInfo, pErrorInfo);

	return (reactorUnlockInterface(pReactorImpl), ret);
}
