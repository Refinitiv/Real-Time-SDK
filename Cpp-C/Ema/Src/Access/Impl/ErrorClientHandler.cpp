/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ErrorClientHandler.h"

using namespace thomsonreuters::ema::access;

ErrorClientHandler::ErrorClientHandler( OmmConsumerErrorClient& client ) :
	_pConsumerErrorClient( &client ),
	_pProviderErrorClient( 0 )
{
}
	
ErrorClientHandler::ErrorClientHandler( OmmProviderErrorClient& client ) :
	_pConsumerErrorClient( 0 ),
	_pProviderErrorClient( &client )
{
}

ErrorClientHandler::~ErrorClientHandler()
{
}

bool ErrorClientHandler::hasErrorClientHandler()
{
	return ( _pConsumerErrorClient || _pProviderErrorClient ) ? true : false;
}

void ErrorClientHandler::onInaccessibleLogFile( const EmaString& filename, const EmaString& text )
{
	if ( _pConsumerErrorClient )
		_pConsumerErrorClient->onInaccessibleLogFile( filename, text );
	else if ( _pProviderErrorClient )
		_pProviderErrorClient->onInaccessibleLogFile( filename, text );
}

void ErrorClientHandler::onInvalidHandle( UInt64 handle, const EmaString& text )
{
	if ( _pConsumerErrorClient )
		_pConsumerErrorClient->onInvalidHandle( handle, text );
	else if ( _pProviderErrorClient )
		_pProviderErrorClient->onInvalidHandle( handle, text );
}

void ErrorClientHandler::onInvalidUsage( const EmaString& text )
{
	if ( _pConsumerErrorClient )
		_pConsumerErrorClient->onInvalidUsage( text );
	else if ( _pProviderErrorClient )
		_pProviderErrorClient->onInvalidUsage( text );
}

void ErrorClientHandler::onMemoryExhaustion( const EmaString& text )
{
	if ( _pConsumerErrorClient )
		_pConsumerErrorClient->onMemoryExhaustion( text );
	else if ( _pProviderErrorClient )
		_pProviderErrorClient->onMemoryExhaustion( text );
}

void ErrorClientHandler::onSystemError( Int64 code, void* ptr, const EmaString& text )
{
	if ( _pConsumerErrorClient )
		_pConsumerErrorClient->onSystemError( code, ptr, text );
	else if ( _pProviderErrorClient )
		_pProviderErrorClient->onSystemError( code, ptr, text );
}
