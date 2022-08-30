/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2022 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumerConfig.h"
#include "OmmConsumerConfigImpl.h"

using namespace refinitiv::ema::access;

OmmConsumerConfig::OmmConsumerConfig() :
	_pImpl(0)
{
	try {
		_pImpl = new OmmConsumerConfigImpl(EmaString());
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for OmmConsumerConfigImpl in OmmConsumerConfig().");
	}
}

OmmConsumerConfig::OmmConsumerConfig(const EmaString& path) :
	_pImpl(0)
{
	try {
		_pImpl = new OmmConsumerConfigImpl(path);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for OmmConsumerConfigImpl in OmmConsumerConfig().");
	}
}

OmmConsumerConfig::~OmmConsumerConfig()
{
	if (_pImpl)
		delete _pImpl;
}

OmmConsumerConfig& OmmConsumerConfig::clear()
{
	_pImpl->clear();
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::username(const EmaString& username)
{
	_pImpl->username(username);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::password(const EmaString& password)
{
	_pImpl->password(password);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::position(const EmaString& position)
{
	_pImpl->position(position);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::applicationId(const EmaString& applicationId)
{
	_pImpl->applicationId(applicationId);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::clientId(const EmaString& clientId)
{
	_pImpl->clientId(clientId);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::clientSecret(const EmaString& clientSecret)
{
	_pImpl->clientSecret(clientSecret);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tokenScope(const EmaString& tokenScope)
{
	_pImpl->tokenScope(tokenScope);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::takeExclusiveSignOnControl(bool takeExclusiveSignOnControl)
{
	_pImpl->takeExclusiveSignOnControl(takeExclusiveSignOnControl);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tokenServiceUrl(const EmaString& tokenServiceUrl)
{
	_pImpl->tokenServiceUrl(tokenServiceUrl);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tokenServiceUrlV1(const EmaString& tokenServiceUrl)
{
	_pImpl->tokenServiceUrlV1(tokenServiceUrl);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tokenServiceUrlV2(const EmaString& tokenServiceUrl)
{
	_pImpl->tokenServiceUrlV2(tokenServiceUrl);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::serviceDiscoveryUrl(const EmaString& serviceDiscoveryUrl)
{
	_pImpl->serviceDiscoveryUrl(serviceDiscoveryUrl);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::host(const EmaString& host)
{
	_pImpl->host(host);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::operationModel(OperationModel operationModel)
{
	_pImpl->operationModel(operationModel);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::consumerName(const EmaString& consumerName)
{
	_pImpl->consumerName(consumerName);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tunnelingProxyHostName(const EmaString& proxyHostName)
{
	_pImpl->proxyHostName(proxyHostName);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tunnelingProxyPort(const EmaString& proxyPort)
{
	_pImpl->proxyPort(proxyPort);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tunnelingSecurityProtocol(int securityProtocol)
{
	_pImpl->securityProtocol(securityProtocol);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tunnelingObjectName(const EmaString& objectName)
{
	_pImpl->objectName(objectName);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tunnelingLibSslName(const EmaString& libsslName)
{
	_pImpl->libsslName(libsslName);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::tunnelingLibCryptoName(const EmaString& libcryptoName)
{
	_pImpl->libcryptoName(libcryptoName);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::libcurlName(const EmaString& libcurlName)
{
	_pImpl->libcurlName(libcurlName);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::proxyUserName(const EmaString& proxyUserName)
{
	_pImpl->proxyUserName(proxyUserName);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::proxyPasswd(const EmaString& proxyPasswd)
{
	_pImpl->proxyPasswd(proxyPasswd);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::proxyDomain(const EmaString& proxyDomain)
{
	_pImpl->proxyDomain(proxyDomain);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::sslCAStore(const EmaString& sslCAStore)
{
	_pImpl->sslCAStore(sslCAStore);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::config(const Data& config)
{
	_pImpl->config(config);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::addAdminMsg(const ReqMsg& reqMsg)
{
	_pImpl->addAdminMsg(reqMsg);
	return *this;
}

OmmConsumerConfigImpl* OmmConsumerConfig::getConfigImpl() const
{
	return _pImpl;
}

OmmConsumerConfig& OmmConsumerConfig::addOAuth2Credential(const OAuth2Credential& credential)
{
	_pImpl->addOAuth2Credential(credential);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::addOAuth2Credential(const OAuth2Credential& credential, const OmmOAuth2ConsumerClient& client)
{
	_pImpl->addOAuth2Credential(credential, client);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::addOAuth2Credential(const OAuth2Credential& credential, const OmmOAuth2ConsumerClient& client, void* closure)
{
	_pImpl->addOAuth2Credential(credential, client, closure);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::addLoginMsgCredential(const ReqMsg& reqMsg, const EmaString& channelList)
{
	_pImpl->addLoginMsgCredential(reqMsg, channelList);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::addLoginMsgCredential(const ReqMsg& reqMsg, const EmaString& channelList, const OmmLoginCredentialConsumerClient& client)
{
	_pImpl->addLoginMsgCredential(reqMsg, channelList, client);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::addLoginMsgCredential(const ReqMsg& reqMsg, const EmaString& channelList, const OmmLoginCredentialConsumerClient& client, void* closure)
{
	_pImpl->addLoginMsgCredential(reqMsg, channelList, client, closure);
	return *this;
}


OmmConsumerConfig& OmmConsumerConfig::restLoggingCallback(OmmRestLoggingClient& ommRestLoggingClient, void* closure)
{
	_pImpl->setOmmRestLoggingClient(&ommRestLoggingClient, closure);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::workerThreadBind(const EmaString& cpuString)
{
	_pImpl->setCpuWorkerThreadBind(cpuString);
	return *this;
}

OmmConsumerConfig& OmmConsumerConfig::apiThreadBind(const EmaString& cpuString)
{
	_pImpl->setCpuApiThreadBind(cpuString);
	return *this;
}
