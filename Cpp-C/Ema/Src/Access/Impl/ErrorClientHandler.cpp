/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ErrorClientHandler.h"
#include "ConsumerSessionInfo.h"
#include "ProviderSessionInfo.h"
#include "ChannelInfoImpl.h"

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

void ErrorClientHandler::onInvalidUsage( const EmaString& text, Int32 errorCode )
{
	if ( _pConsumerErrorClient )
		_pConsumerErrorClient->onInvalidUsage( text, errorCode );
	else if ( _pProviderErrorClient )
		_pProviderErrorClient->onInvalidUsage( text, errorCode );
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

void ErrorClientHandler::onJsonConverter(const char* text, Int32 errorCode, RsslReactorChannel* reactorChannel, ClientSession* clientSession, OmmProvider* pProvider)
{
	if (_pConsumerErrorClient)
	{
		ConsumerSessionInfo sessionInfo;
		getChannelInformationImpl(reactorChannel, OmmCommonImpl::ConsumerEnum, const_cast<ChannelInformation&>(sessionInfo._channelInfo));
		_pConsumerErrorClient->onJsonConverter(text, errorCode, sessionInfo);
	}
	else if (_pProviderErrorClient)
	{
		ProviderSessionInfo sessionInfo;

		if (clientSession != NULL)
		{
			sessionInfo._clientHandle = clientSession->getClientHandle();
			sessionInfo._handle = clientSession->getLoginHandle();
			sessionInfo._provider = pProvider;
			getChannelInformationImpl(reactorChannel, OmmCommonImpl::IProviderEnum, const_cast<ChannelInformation&>(sessionInfo._channelInfo));
		}
		else
		{
			sessionInfo._provider = pProvider;

			getChannelInformationImpl(reactorChannel, OmmCommonImpl::NiProviderEnum, const_cast<ChannelInformation&>(sessionInfo._channelInfo));
		}

		_pProviderErrorClient->onJsonConverter(text, errorCode, sessionInfo);
	}
}
