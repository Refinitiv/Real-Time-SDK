/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmIProviderConfig.h"
#include "OmmIProviderConfigImpl.h"
#include "ExceptionTranslator.h"

using namespace thomsonreuters::ema::access;

OmmIProviderConfig::OmmIProviderConfig() :
	_pImpl( 0 )
{
	try {
		_pImpl = new OmmIProviderConfigImpl( EmaString() );
	}
	catch ( std::bad_alloc ) {}

	if ( !_pImpl )
		throwMeeException( "Failed to allocate memory for OmmIProviderConfigImpl in OmmIProviderConfig()" );
}

OmmIProviderConfig::OmmIProviderConfig( const EmaString & path ) :
	_pImpl( 0 )
{
	try {
		_pImpl = new OmmIProviderConfigImpl( path );
	}
	catch ( std::bad_alloc ) {}

	if ( !_pImpl )
		throwMeeException( "Failed to allocate memory for OmmIProviderConfigImpl in OmmIProviderConfig()" );
}

OmmIProviderConfig::~OmmIProviderConfig()
{
	if (_pImpl)
	{
		delete _pImpl;
		_pImpl = 0;
	}
}

OmmProviderConfig::ProviderRole OmmIProviderConfig::getProviderRole() const
{
	return OmmProviderConfig::InteractiveEnum;
}

OmmIProviderConfigImpl* OmmIProviderConfig::getConfigImpl() const
{
	return _pImpl;
}

OmmIProviderConfig& OmmIProviderConfig::clear()
{
	_pImpl->clear();
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::port( const EmaString& host )
{
	_pImpl->port( host );
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::operationModel( OperationModel operationModel )
{
	_pImpl->operationModel( operationModel );
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::adminControlDirectory( AdminControl control )
{
	_pImpl->adminControlDirectory( control );
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::adminControlDictionary( AdminControl control )
{
	_pImpl->adminControlDictionary( control );
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::providerName( const EmaString& providerName )
{
	_pImpl->providerName( providerName );
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::config( const Data& config )
{
	_pImpl->config( config );
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::addAdminMsg( const RefreshMsg& refreshMsg )
{
	_pImpl->addAdminMsg( refreshMsg );
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::libSslName(const EmaString& libSslName)
{
	_pImpl->libsslName(libSslName);
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::libCryptoName(const EmaString& libCryptoName)
{
	_pImpl->libcryptoName(libCryptoName);
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::libCurlName (const EmaString& libCurlName)
{
	_pImpl->libcurlName(libCurlName);
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::serverCert(const EmaString& serverCert)
{
	_pImpl->serverCert(serverCert);
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::serverPrivateKey(const EmaString& serverPrivateKey)
{
	_pImpl->serverPrivateKey(serverPrivateKey);
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::cipherSuite(const EmaString& cipherSuite)
{
	_pImpl->cipherSuite(cipherSuite);
	return *this;
}

OmmIProviderConfig& OmmIProviderConfig::dhParams(const EmaString& dhParams)
{
	_pImpl->dhParams(dhParams);
	return *this;
}

