/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "OmmNiProviderConfig.h"
#include "OmmNiProviderConfigImpl.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmNiProviderConfig::OmmNiProviderConfig() :
	_pImpl( 0 )
{
	try {
		_pImpl = new OmmNiProviderConfigImpl( EmaString() );
	}
	catch ( std::bad_alloc& ) {}

	if ( !_pImpl )
		throwMeeException( "Failed to allocate memory for OmmNiProviderConfigImpl in OmmNiProviderConfig()" );
}

OmmNiProviderConfig::OmmNiProviderConfig( const EmaString & path ) :
	_pImpl( 0 )
{
	try {
		_pImpl = new OmmNiProviderConfigImpl( path );
	}
	catch ( std::bad_alloc& ) {}

	if ( !_pImpl )
		throwMeeException( "Failed to allocate memory for OmmNiProviderConfigImpl in OmmNiProviderConfig()" );
}

OmmNiProviderConfig::~OmmNiProviderConfig()
{
	if ( _pImpl )
		delete _pImpl;
}

OmmProviderConfig::ProviderRole OmmNiProviderConfig::getProviderRole() const
{
	return OmmProviderConfig::NonInteractiveEnum;
}

OmmNiProviderConfigImpl* OmmNiProviderConfig::getConfigImpl() const
{
	return _pImpl;
}

OmmNiProviderConfig& OmmNiProviderConfig::clear()
{
	_pImpl->clear();
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::username( const EmaString& username )
{
	_pImpl->username( username );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::password( const EmaString& password )
{
	_pImpl->password( password );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::position( const EmaString& position )
{
	_pImpl->position( position );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::applicationId( const EmaString& applicationId )
{
	_pImpl->applicationId( applicationId );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::instanceId( const EmaString& instanceId )
{
	_pImpl->instanceId( instanceId );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::host( const EmaString& host )
{
	_pImpl->host( host );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::operationModel( OperationModel operationModel )
{
	_pImpl->operationModel( operationModel );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::adminControlDirectory( AdminControl control )
{
	_pImpl->adminControlDirectory( control );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::providerName( const EmaString& providerName )
{
	_pImpl->providerName( providerName );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::tunnelingProxyHostName(const EmaString& proxyHostName)
{
	_pImpl->proxyHostName(proxyHostName);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::tunnelingProxyPort(const EmaString& proxyPort)
{
	_pImpl->proxyPort(proxyPort);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::tunnelingSecurityProtocol(int securityProtocol)
{
	_pImpl->securityProtocol(securityProtocol);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::tunnelingObjectName(const EmaString& objectName)
{
	_pImpl->objectName(objectName);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::tunnelingLibSslName(const EmaString& libsslName)
{
	_pImpl->libsslName(libsslName);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::tunnelingLibCryptoName(const EmaString& libcryptoName)
{
	_pImpl->libcryptoName(libcryptoName);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::libcurlName(const EmaString& libcurlName)
{
	_pImpl->libcurlName(libcurlName);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::proxyUserName(const EmaString& proxyUserName)
{
	_pImpl->proxyUserName(proxyUserName);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::proxyPasswd(const EmaString& proxyPasswd)
{
	_pImpl->proxyPasswd(proxyPasswd);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::proxyDomain(const EmaString& proxyDomain)
{
	_pImpl->proxyDomain(proxyDomain);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::sslCAStore(const EmaString& sslCAStore)
{
	_pImpl->sslCAStore(sslCAStore);
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::config( const Data& config )
{
	_pImpl->config( config );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::addAdminMsg( const ReqMsg& reqMsg )
{
	_pImpl->addAdminMsg( reqMsg );
	return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::addAdminMsg( const RefreshMsg& refreshMsg )
{
	_pImpl->addAdminMsg( refreshMsg );
	return *this;
}
