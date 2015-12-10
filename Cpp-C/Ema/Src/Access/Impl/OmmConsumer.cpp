/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumer.h"
#include "OmmConsumerConfig.h"
#include "OmmConsumerImpl.h"

using namespace thomsonreuters::ema::access;

OmmConsumer::OmmConsumer( const OmmConsumerConfig& config ) :
 _pImpl( 0 )
{
	try {
		_pImpl = new OmmConsumerImpl( config );
	} catch ( std::bad_alloc ) {}

	if ( !_pImpl )
	{
		const char* temp = "Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& ).";
		throwMeeException( temp );
	}
}

OmmConsumer::OmmConsumer( const OmmConsumerConfig& config, OmmConsumerErrorClient& client ) :
 _pImpl( 0 )
{
	try {
		_pImpl = new OmmConsumerImpl( config, client );
	} catch ( std::bad_alloc ) {}

	if ( !_pImpl )
	{
		const char* temp = "Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& , OmmConsumerErrorClient& ).";
		client.onMemoryExhaustion( temp );
	}
}

OmmConsumer::~OmmConsumer()
{
	if ( _pImpl )
		delete _pImpl;
}

UInt64 OmmConsumer::registerClient( const ReqMsg& reqMsg, OmmConsumerClient& client, void* closure, UInt64 parentHandle ) 
{
	return _pImpl->registerClient( reqMsg, client, closure, parentHandle );
}

UInt64 OmmConsumer::registerClient( const TunnelStreamRequest& tunnelStreamRequest, OmmConsumerClient& client, void* closure )
{
	return _pImpl->registerClient( tunnelStreamRequest, client, closure );
}

void OmmConsumer::reissue( const ReqMsg& reqMsg, UInt64 handle ) 
{
	return _pImpl->reissue( reqMsg, handle );
}

void OmmConsumer::submit( const GenericMsg& genericMsg, UInt64 handle )
{
	_pImpl->submit( genericMsg, handle );
}

void OmmConsumer::submit( const PostMsg& postMsg, UInt64 handle )
{
	_pImpl->submit( postMsg, handle );
}

Int64 OmmConsumer::dispatch( Int64 timeOut )
{
	return _pImpl->dispatch( timeOut );
}

void OmmConsumer::unregister( UInt64 handle )
{
	_pImpl->unregister( handle );
}
