/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmNiProvider.h"
#include "OmmNiProviderConfig.h"
#include "OmmNiProviderImpl.h"

using namespace thomsonreuters::ema::access;

OmmNiProvider::OmmNiProvider( const OmmNiProviderConfig& config ) :
 _pImpl( 0 )
{
	try {
		_pImpl = new OmmNiProviderImpl( config );
	} catch ( std::bad_alloc ) {}

	if ( !_pImpl )
	{
		const char* temp = "Failed to allocate memory for OmmNiProviderImpl in OmmNiProvider( const OmmNiProviderConfig& ).";
		throwMeeException( temp );
	}
}

OmmNiProvider::OmmNiProvider( const OmmNiProviderConfig& config, OmmNiProviderErrorClient& client ) :
 _pImpl( 0 )
{
	try {
		_pImpl = new OmmNiProviderImpl( config, client );
	} catch ( std::bad_alloc ) {}

	if ( !_pImpl )
	{
		const char* temp = "Failed to allocate memory for OmmNiProviderImpl in OmmNiProvider( const OmmNiProviderConfig& , OmmNiProviderErrorClient& ).";
		client.onMemoryExhaustion( temp );
	}
}

OmmNiProvider::~OmmNiProvider()
{
  if ( _pImpl )
      _pImpl->uninitialize();
  delete _pImpl;
}

UInt64 OmmNiProvider::registerClient( const ReqMsg& reqMsg, OmmNiProviderClient& client, void* closure ) 
{
	return _pImpl->registerClient( reqMsg, client, closure );
}

void OmmNiProvider::submit( const RefreshMsg& refreshMsg, UInt64 handle )
{
  _pImpl->submit( refreshMsg, handle );
}

void OmmNiProvider::submit( const UpdateMsg& updateMsg, UInt64 handle )
{
  _pImpl->submit( updateMsg, handle );
}

void OmmNiProvider::submit( const StatusMsg& statusMsg, UInt64 handle )
{
  _pImpl->submit( statusMsg, handle );
}