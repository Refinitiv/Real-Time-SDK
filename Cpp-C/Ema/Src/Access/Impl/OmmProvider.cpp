/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "OmmProvider.h"
#include "OmmProviderConfig.h"
#include "OmmNiProviderImpl.h"
#include "OmmIProviderImpl.h"
#include "OmmNiProviderConfig.h"
#include "OmmIProviderConfig.h"
#include "OmmNiProviderConfigImpl.h"
#include "OmmIProviderConfigImpl.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

OmmProvider::OmmProvider( const OmmProviderConfig& config ) :
  _pImpl( 0 ),
  _role(config.getProviderRole())
{
	if ( config.getProviderRole() == OmmProviderConfig::InteractiveEnum )
		throwIueException( "Attempt to pass an OmmIProvConfig instance to non interactive provider OmmProvider constructor.", OmmInvalidUsageException::InvalidArgumentEnum );
	else
	{
		try
		{
			static_cast<const OmmNiProviderConfig&>(config)._pImpl->validateSpecifiedSessionName();
			_pImpl = new OmmNiProviderImpl( this, static_cast<const OmmNiProviderConfig&>( config ) );
		}
		catch ( std::bad_alloc& ) {}

		if ( !_pImpl )
			throwMeeException( "Failed to allocate memory in OmmProvider( const OmmProviderConfig& )." );
	}
}

OmmProvider::OmmProvider( const OmmProviderConfig& config, OmmProviderClient& client, void* closure) :
  _pImpl(0),
  _role(config.getProviderRole())
{
	if (config.getProviderRole() == OmmProviderConfig::NonInteractiveEnum)
	{
		try
		{
			static_cast<const OmmNiProviderConfig&>(config)._pImpl->validateSpecifiedSessionName();
			_pImpl = new OmmNiProviderImpl(this, static_cast<const OmmNiProviderConfig&>(config), client, closure);
		}
		catch (std::bad_alloc&) {}

		if (!_pImpl)
			throwMeeException("Failed to allocate memory in OmmNiProvider( const OmmProviderConfig&, OmmProviderClient& client ).");
	}
	else
	{
		try
		{
			static_cast<const OmmIProviderConfig&>(config)._pImpl->validateSpecifiedSessionName();
			_pImpl = new OmmIProviderImpl( this, static_cast<const OmmIProviderConfig&>(config), client, closure);
		}
		catch (std::bad_alloc&) {}

		if (!_pImpl)
			throwMeeException("Failed to allocate memory in OmmProvider( const OmmProviderConfig&, OmmProviderClient& client ).");
	}
}

OmmProvider::OmmProvider( const OmmProviderConfig& config, OmmProviderErrorClient& errorClient ) :
  _pImpl( 0 ),
  _role(config.getProviderRole())
{
	if ( config.getProviderRole() == OmmProviderConfig::InteractiveEnum )
		throwIueException( "Attempt to pass an OmmIProvConfig instance to non interactive provider OmmProvider constructor.", OmmInvalidUsageException::InvalidArgumentEnum );
	else
	{
		try
		{
			static_cast<const OmmNiProviderConfig&>(config)._pImpl->validateSpecifiedSessionName();
			_pImpl = new OmmNiProviderImpl( this, static_cast<const OmmNiProviderConfig&>( config ), errorClient);
		}
		catch ( std::bad_alloc& ) {}

		if ( !_pImpl )
			errorClient.onMemoryExhaustion( "Failed to allocate memory in OmmProvider( const OmmProviderConfig& , OmmProviderErrorClient& )." );
	}
}

OmmProvider::OmmProvider( const OmmProviderConfig& config, OmmProviderClient& client, OmmProviderErrorClient& errorClient, void* closure) :
  _pImpl(0),
  _role(config.getProviderRole())
{
	if (config.getProviderRole() == OmmProviderConfig::NonInteractiveEnum)
	{
		try
		{
			static_cast<const OmmNiProviderConfig&>(config)._pImpl->validateSpecifiedSessionName();
			_pImpl = new OmmNiProviderImpl(this, static_cast<const OmmNiProviderConfig&>(config), client, errorClient, closure);
		}
		catch (std::bad_alloc&) {}

		if (!_pImpl)
			errorClient.onMemoryExhaustion("Failed to allocate memory in OmmNiProvider( const OmmProviderConfig& , OmmProviderErrorClient& ).");
	}
	else
	{
		try
		{
			static_cast<const OmmIProviderConfig&>(config)._pImpl->validateSpecifiedSessionName();
			_pImpl = new OmmIProviderImpl( this, static_cast<const OmmIProviderConfig&>(config), client, errorClient, closure);
		}
		catch (std::bad_alloc&) {}

		if (!_pImpl)
			errorClient.onMemoryExhaustion("Failed to allocate memory in OmmProvider( const OmmProviderConfig& , OmmProviderErrorClient& ).");
	}
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

OmmProviderConfig::ProviderRole OmmProvider::getProviderRole() const {
  return _role;
}

UInt64 OmmProvider::registerClient( const ReqMsg& reqMsg, OmmProviderClient& client, void* closure ) 
{
	return _pImpl->registerClient( reqMsg, client, closure );
}

void OmmProvider::reissue(const ReqMsg& reqMsg, UInt64 handle)
{
	return _pImpl->reissue(reqMsg, handle);
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

void OmmProvider::submit( const AckMsg& ackMsg, UInt64 handle )
{
	_pImpl->submit( ackMsg, handle );
}

void OmmProvider::getConnectedClientChannelInfo( EmaVector<ChannelInformation>& ci ) {
	return _pImpl->getConnectedClientChannelInfo( ci );
}

void OmmProvider::getConnectedClientChannelStats( UInt64 clientHandle, ChannelStatistics& cs ) {
	return _pImpl->getConnectedClientChannelStats( clientHandle, cs );
}

void OmmProvider::getChannelInformation( ChannelInformation& ci ) {
	// this function can be called during the OmmProvider constructor (usually from an event
	// received during that process). If so, just have to return 0.
	if ( _pImpl )
		_pImpl->getChannelInformation( ci );
	else
		ci.clear();
}

void OmmProvider::modifyIOCtl( Int32 code, Int32 value, UInt64 handle )
{
	_pImpl->modifyIOCtl(code, value, handle);
}

void OmmProvider::closeChannel( UInt64 clientHandle )
{
	_pImpl->closeChannel(clientHandle);
}
