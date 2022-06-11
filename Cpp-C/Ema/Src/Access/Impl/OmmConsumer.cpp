/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2022 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumer.h"
#include "OmmConsumerConfig.h"
#include "OmmConsumerConfigImpl.h"
#include "OmmConsumerImpl.h"
#include "ExceptionTranslator.h"

using namespace refinitiv::ema::access;

OmmConsumer::OmmConsumer( const OmmConsumerConfig& config ) :
	_pImpl( 0 )
{
	try
	{
		config._pImpl->validateSpecifiedSessionName();
		_pImpl = new OmmConsumerImpl( config );
	}
	catch ( std::bad_alloc& )
	{
		throwMeeException( "Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& )." );
	}
}

OmmConsumer::OmmConsumer(const OmmConsumerConfig& config, OmmConsumerClient& client, void* closure ) :
	_pImpl(0)
{
	try
	{
		config._pImpl->validateSpecifiedSessionName();
		_pImpl = new OmmConsumerImpl(config, client, closure);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& ).");
	}
}
OmmConsumer::OmmConsumer(const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, void* closure) :
	_pImpl(0)
{
	try
	{
		config._pImpl->validateSpecifiedSessionName();
		_pImpl = new OmmConsumerImpl(config, adminClient, oAuthClient, closure);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& ).");
	}
}

OmmConsumer::OmmConsumer(const OmmConsumerConfig& config, OmmOAuth2ConsumerClient& oAuthClient, void* closure) :
	_pImpl(0)
{
	try
	{
		config._pImpl->validateSpecifiedSessionName();
		_pImpl = new OmmConsumerImpl(config, oAuthClient, closure);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& ).");
	}
}

OmmConsumer::OmmConsumer(const OmmConsumerConfig& config, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& errorClient, void* closure) :
	_pImpl(0)
{
	try
	{
		config._pImpl->validateSpecifiedSessionName();
		_pImpl = new OmmConsumerImpl(config, oAuthClient, errorClient, closure);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& ).");
	}
}

OmmConsumer::OmmConsumer( const OmmConsumerConfig& config, OmmConsumerErrorClient& client ) :
	_pImpl( 0 )
{
	try
	{
		config._pImpl->validateSpecifiedSessionName();
		_pImpl = new OmmConsumerImpl( config, client );
	}
	catch ( std::bad_alloc& )
	{
		client.onMemoryExhaustion("Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& , OmmConsumerErrorClient& ).");
	}
}

OmmConsumer::OmmConsumer(const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmConsumerErrorClient& errorClient, void* closure ) :
	_pImpl(0)
{
	try
	{
		config._pImpl->validateSpecifiedSessionName();
		_pImpl = new OmmConsumerImpl(config, adminClient, errorClient, closure);
	}
	catch (std::bad_alloc&)
	{
		errorClient.onMemoryExhaustion("Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& , OmmConsumerErrorClient& ).");
	}
}


OmmConsumer::OmmConsumer(const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& errorClient, void* closure) :
	_pImpl(0)
{
	try
	{
		config._pImpl->validateSpecifiedSessionName();
		_pImpl = new OmmConsumerImpl(config, adminClient, oAuthClient, errorClient, closure);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for OmmConsumerImpl in OmmConsumer( const OmmConsumerConfig& ).");
	}
}

OmmConsumer::~OmmConsumer()
{
	if ( _pImpl )
		delete _pImpl;
}

const EmaString& OmmConsumer::getConsumerName() const
{
	return _pImpl->getInstanceName();
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

void OmmConsumer::getChannelInformation(ChannelInformation& ci) {
  // this function can be called during the OmmConsumer constructor (usually from an event
  // received during that process). If so, just have to return 0.
  if (_pImpl)
	_pImpl->getChannelInformation(ci);
  else
	ci.clear();
}

void OmmConsumer::getChannelStatistics(ChannelStatistics& cs) {
  // this function can be called during the OmmConsumer constructor (usually from an event
  // received during that process). If so, just have to return 0.
  if (_pImpl)
	_pImpl->getChannelStatistics(cs);
  else
	cs.clear();
}

void OmmConsumer::modifyIOCtl(Int32 code, Int32 value)
{
	_pImpl->modifyIOCtl(code, value);
}

void OmmConsumer::modifyReactorIOCtl(Int32 code, Int32 value)
{
	_pImpl->modifyReactorIOCtl(code, value);
}

void OmmConsumer::renewOAuth2Credentials(OAuth2CredentialRenewal& credentials)
{
	_pImpl->renewOAuth2Credentials(credentials);
}

void OmmConsumer::renewLoginCredentials(LoginMsgCredentialRenewal& credentials)
{
	_pImpl->renewLoginMsgCredentials(credentials);
}