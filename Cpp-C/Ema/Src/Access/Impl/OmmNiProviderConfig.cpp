/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmNiProviderConfig.h"
#include "OmmNiProviderConfigImpl.h"
#include "ExceptionTranslator.h"

#include <new>

using namespace thomsonreuters::ema::access;

OmmNiProviderConfig::OmmNiProviderConfig() : _pImpl( 0 )
{
	try {
		_pImpl = new OmmNiProviderConfigImpl();
	} catch ( std::bad_alloc ) {}

	if ( !_pImpl )
		throwMeeException( "Failed to allocate memory for OmmNiProviderConfigImpl in OmmNiProviderConfig()" );
}

OmmNiProviderConfig::~OmmNiProviderConfig()
{
	delete _pImpl;
}

OmmNiProviderConfigImpl* OmmNiProviderConfig::getConfigImpl() const
{
	return _pImpl;
}

OmmNiProviderConfig& OmmNiProviderConfig::username( const EmaString& username )
{
	_pImpl->username( username );
        return *this;
}

OmmNiProviderConfig& OmmNiProviderConfig::host( const EmaString& host )
{
  _pImpl->host( host );
        return *this;
}


OmmNiProviderConfig& OmmNiProviderConfig::niProviderName( const EmaString& niProviderName )
{
        return *this;
}
