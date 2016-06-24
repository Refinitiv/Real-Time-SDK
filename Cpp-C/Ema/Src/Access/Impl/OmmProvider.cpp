/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmProvider.h"
#include "OmmProviderConfig.h"
#include "OmmNiProviderImpl.h"
#include "OmmNiProviderConfig.h"

#include <new>

using namespace thomsonreuters::ema::access;

OmmProvider::OmmProvider( const OmmProviderConfig& config ) :
	_pImpl( 0 )
{
	try {
	  _pImpl = new OmmNiProviderImpl( static_cast< const OmmNiProviderConfig& >( config ) );
	}
	catch ( std::bad_alloc ) {}

	if ( !_pImpl )
		throwMeeException( "Failed to allocate memory in OmmProvider( const OmmProviderConfig& )." );
}

OmmProvider::OmmProvider( const OmmProviderConfig& config, OmmProviderErrorClient& client ) :
	_pImpl( 0 )
{
	try {
	  _pImpl = new OmmNiProviderImpl( static_cast< const OmmNiProviderConfig& >( config ), client );
	} catch ( std::bad_alloc ) {}

	if ( !_pImpl )
		client.onMemoryExhaustion( "Failed to allocate memory in OmmProvider( const OmmProviderConfig& , OmmProviderErrorClient& )." );
}

OmmProvider::~OmmProvider()
{
	if ( _pImpl )
		delete _pImpl;
}
const EmaString& OmmProvider::getProviderName() const
{
	return _pImpl->getInstanceName();
}

UInt64 OmmProvider::registerClient( const ReqMsg& reqMsg, OmmProviderClient& client, void* closure ) 
{
	return _pImpl->registerClient( reqMsg, client, closure );
}

void OmmProvider::submit( const RefreshMsg& refreshMsg, UInt64 handle )
{
	_pImpl->submit( refreshMsg, handle );
}

void OmmProvider::submit( const UpdateMsg& updateMsg, UInt64 handle )
{
	_pImpl->submit( updateMsg, handle );
}

void OmmProvider::submit( const StatusMsg& statusMsg, UInt64 handle )
{
	_pImpl->submit( statusMsg, handle );
}

void OmmProvider::submit( const GenericMsg& genericMsg, UInt64 handle )
{
	_pImpl->submit( genericMsg, handle );
}

Int64 OmmProvider::dispatch( Int64 timeOut )
{
	return _pImpl->dispatch( timeOut );
}

void OmmProvider::unregister( UInt64 handle )
{
	_pImpl->unregister( handle );
}
