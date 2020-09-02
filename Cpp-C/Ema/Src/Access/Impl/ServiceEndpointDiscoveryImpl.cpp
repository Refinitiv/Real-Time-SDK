/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscoveryImpl.h"
#include "ExceptionTranslator.h"
#include "Utilities.h"
#include "OmmInvalidUsageException.h"

using namespace rtsdk::ema::access;

ServiceEndpointDiscoveryImpl::ServiceEndpointDiscoveryImpl(ServiceEndpointDiscovery *pServiceEndpointDiscovery, const EmaString& tokenServiceURL, 
																const EmaString& serviceDiscoveryURL) :
_pServiceEndpointDiscovery(0),
_pClient(0)
{
	RsslCreateReactorOptions reactorOpts;
	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);

	rsslClearCreateReactorOptions(&reactorOpts);
	reactorOpts.tokenServiceURL.data = const_cast<char*>(tokenServiceURL.c_str());
	reactorOpts.tokenServiceURL.length = tokenServiceURL.length();
	reactorOpts.serviceDiscoveryURL.data = const_cast<char*>(serviceDiscoveryURL.c_str());
	reactorOpts.serviceDiscoveryURL.length = serviceDiscoveryURL.length();
	reactorOpts.userSpecPtr = this;

	_pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo);
	if (!_pReactor)
	{
		EmaString temp("Failed to initialize ServiceEndpointDiscoveryImpl (rsslCreateReactor).");
		temp.append("' Error Id='").append(rsslErrorInfo.rsslError.rsslErrorId)
			.append("' Internal sysError='").append(rsslErrorInfo.rsslError.sysError)
			.append("' Error Location='").append(rsslErrorInfo.errorLocation)
			.append("' Error Text='").append(rsslErrorInfo.rsslError.text).append("'. ");
		throwIueException( temp, OmmInvalidUsageException::InternalErrorEnum );
	}

	_pServiceEndpointDiscovery = pServiceEndpointDiscovery;
}

ServiceEndpointDiscoveryImpl::~ServiceEndpointDiscoveryImpl()
{
	if (_pReactor)
	{
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo(&rsslErrorInfo);
		rsslDestroyReactor(_pReactor, &rsslErrorInfo);

		_pReactor = 0;
	}
}

void ServiceEndpointDiscoveryImpl::registerClient(const ServiceEndpointDiscoveryOption& params, ServiceEndpointDiscoveryClient& client, 
														void *closure)
{
	_userLock.lock();

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	rsslClearReactorServiceDiscoveryOptions(&_serviceDiscoveryOpts);

	_serviceDiscoveryOpts.userName.data = const_cast<char*>(params._username.c_str());
	_serviceDiscoveryOpts.userName.length = params._username.length();
	_serviceDiscoveryOpts.password.data = const_cast<char*>(params._password.c_str());
	_serviceDiscoveryOpts.password.length = params._password.length();
	_serviceDiscoveryOpts.takeExclusiveSignOnControl = (RsslBool)params._takeExclusiveSignOnControl;

	if (!params._proxyHostName.empty())
	{
		_serviceDiscoveryOpts.proxyHostName.data = const_cast<char*>(params._proxyHostName.c_str());
		_serviceDiscoveryOpts.proxyHostName.length = params._proxyHostName.length();
	}

	if (!params._proxyPort.empty())
	{
		_serviceDiscoveryOpts.proxyPort.data = const_cast<char*>(params._proxyPort.c_str());
		_serviceDiscoveryOpts.proxyPort.length = params._proxyPort.length();
	}

	if (!params._proxyUserName.empty())
	{
		_serviceDiscoveryOpts.proxyUserName.data = const_cast<char*>(params._proxyUserName.c_str());
		_serviceDiscoveryOpts.proxyUserName.length = params._proxyUserName.length();
	}

	if (!params._proxyPassword.empty())
	{
		_serviceDiscoveryOpts.proxyPasswd.data = const_cast<char*>(params._proxyPassword.c_str());
		_serviceDiscoveryOpts.proxyPasswd.length = params._proxyPassword.length();
	}

	if (!params._proxyDomain.empty())
	{
		_serviceDiscoveryOpts.proxyDomain.data = const_cast<char*>(params._proxyDomain.c_str());
		_serviceDiscoveryOpts.proxyDomain.length = params._proxyDomain.length();
	}

	if (!params._clientId.empty())
	{
		_serviceDiscoveryOpts.clientId.data = const_cast<char*>(params._clientId.c_str());
		_serviceDiscoveryOpts.clientId.length = params._clientId.length();
	}

	if (!params._clientSecret.empty())
	{
		_serviceDiscoveryOpts.clientSecret.data = const_cast<char*>(params._clientSecret.c_str());
		_serviceDiscoveryOpts.clientSecret.length = params._clientSecret.length();
	}

	if (!params._tokenScope.empty() && params._tokenScope != _serviceDiscoveryOpts.tokenScope.data)
	{
		_serviceDiscoveryOpts.tokenScope.data = const_cast<char*>(params._tokenScope.c_str());
		_serviceDiscoveryOpts.tokenScope.length = params._tokenScope.length();
	}

	_pClient = &client;
	_serviceDiscoveryOpts.userSpecPtr = closure;
	_serviceDiscoveryOpts.pServiceEndpointEventCallback = ServiceEndpointDiscoveryImpl::serviceEndpointEventCallback;

	EmaString temp;
	switch (params._transport)
	{
	case ServiceEndpointDiscoveryOption::UnknownTransportEnum:
		break;
	case ServiceEndpointDiscoveryOption::TcpEnum:
		_serviceDiscoveryOpts.transport = RSSL_RD_TP_TCP;
		break;
	case ServiceEndpointDiscoveryOption::WebsocketEnum:
		_serviceDiscoveryOpts.transport = RSSL_RD_TP_WEBSOCKET;
		break;
	default:
		temp.set("Invalid transport protocol ").append(params._transport)
			.append(" specified in ServiceEndpointDiscoveryOption::transport()");

		_userLock.unlock();
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		break;
	}

	switch (params._dataFormat)
	{
	case ServiceEndpointDiscoveryOption::UnknownDataFormatEnum:
		break;
	case ServiceEndpointDiscoveryOption::RwfEnum:
		_serviceDiscoveryOpts.dataFormat = RSSL_RD_DP_RWF;
		break;
	case ServiceEndpointDiscoveryOption::Json2Enum:
		_serviceDiscoveryOpts.dataFormat = RSSL_RD_DP_JSON2;
		break;
	default:
		temp.set("Invalid dataformat protocol ").append(params._dataFormat)
			.append(" specified in ServiceEndpointDiscoveryOption::dataFormat()");

		_userLock.unlock();
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		break;
	}

	if (rsslReactorQueryServiceDiscovery(_pReactor, &_serviceDiscoveryOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		EmaString temp("Failed to query service discovery (rsslReactorQueryServiceDiscovery).");
		temp.append("' Error Id='").append(rsslErrorInfo.rsslError.rsslErrorId)
			.append("' Internal sysError='").append(rsslErrorInfo.rsslError.sysError)
			.append("' Error Location='").append(rsslErrorInfo.errorLocation)
			.append("' Error Text='").append(rsslErrorInfo.rsslError.text).append("'. ");

		_userLock.unlock();
		throwIueException( temp, rsslErrorInfo.rsslError.rsslErrorId );
	}

	_userLock.unlock();
}

RsslReactorCallbackRet ServiceEndpointDiscoveryImpl::serviceEndpointEventCallback(RsslReactor* pReactor, RsslReactorServiceEndpointEvent* pEvent)
{
	UInt32 index, valueIndex;
	ServiceEndpointDiscoveryImpl *pImpl = (ServiceEndpointDiscoveryImpl*)pReactor->userSpecPtr;
	RsslReactorServiceEndpointInfo *reactorServiceEndpointInfo;
	ServiceEndpointDiscoveryInfo serviceEndpointInfo;
	EmaString value;

	pImpl->_serviceDiscoveryEvent._pClosure = pEvent->userSpecPtr;
	pImpl->_serviceDiscoveryEvent._pServiceEndpointDiscovery = pImpl->_pServiceEndpointDiscovery;

	if (pEvent->pErrorInfo == 0)
	{
		pImpl->_serviceDiscoveryResp._pServiceEndpointDiscoveryInfoList->clear();
		for (index = 0; index < pEvent->serviceEndpointInfoCount; index++)
		{
			reactorServiceEndpointInfo = &pEvent->serviceEndpointInfoList[index];

			serviceEndpointInfo._pDataFormatList->clear();
			serviceEndpointInfo._pLocationList->clear();
			for (valueIndex = 0; valueIndex < reactorServiceEndpointInfo->dataFormatCount; valueIndex++)
			{
				value.set(reactorServiceEndpointInfo->dataFormatList[valueIndex].data, reactorServiceEndpointInfo->dataFormatList[valueIndex].length);
				serviceEndpointInfo._pDataFormatList->push_back(value);
			}

			for (valueIndex = 0; valueIndex < reactorServiceEndpointInfo->locationCount; valueIndex++)
			{
				value.set(reactorServiceEndpointInfo->locationList[valueIndex].data, reactorServiceEndpointInfo->locationList[valueIndex].length);
				serviceEndpointInfo._pLocationList->push_back(value);
			}

			serviceEndpointInfo._endPoint.set(reactorServiceEndpointInfo->endPoint.data, reactorServiceEndpointInfo->endPoint.length);
			serviceEndpointInfo._port.set(reactorServiceEndpointInfo->port.data, reactorServiceEndpointInfo->port.length);
			serviceEndpointInfo._provider.set(reactorServiceEndpointInfo->provider.data, reactorServiceEndpointInfo->provider.length);
			serviceEndpointInfo._transport.set(reactorServiceEndpointInfo->transport.data, reactorServiceEndpointInfo->provider.length);
			pImpl->_serviceDiscoveryResp._pServiceEndpointDiscoveryInfoList->push_back(serviceEndpointInfo);
		}

		static_cast<ServiceEndpointDiscoveryImpl*>(pReactor->userSpecPtr)->_pClient->onSuccess(pImpl->_serviceDiscoveryResp, pImpl->_serviceDiscoveryEvent);
	}
	else
	{
		static_cast<ServiceEndpointDiscoveryImpl*>(pReactor->userSpecPtr)->_pClient->onError(value.set(pEvent->pErrorInfo->rsslError.text), pImpl->_serviceDiscoveryEvent);
	}

	return RSSL_RC_CRET_SUCCESS;
}
