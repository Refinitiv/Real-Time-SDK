///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.util.List;
import com.rtsdk.ema.access.OmmProviderConfig.ProviderRole;
import com.rtsdk.eta.valueadd.reactor.ReactorChannel;

class OmmEventImpl<T> implements OmmConsumerEvent, OmmProviderEvent
{
	Item<T> _item;
	OmmProvider _ommProvider;
	LongObject _clientHandle = new LongObject().value(0);
	Object 	_closure;
	LongObject _handle;
	ReactorChannel _channel;
	ChannelInformationImpl _channelInfo;
	
	@Override
	public long handle()
	{
		if(_item == null)
			return _handle.value();
		else
			return _item.itemId();
	}

	@Override
	public Object closure()
	{
		if(_item == null )
			return _closure;
		else
			return _item.closure();
	}

	@Override
	public long parentHandle()
	{
		if ( _item.parent() != null )
			return _item.parent().itemId();
		else
			return 0;
	}

	@Override
	public OmmProvider provider()
	{
		return _ommProvider;
	}

	@Override
	public long clientHandle()
	{
		return _clientHandle.value();
	}

	@Override
	public ChannelInformation channelInformation()
	{
		if ( _channelInfo == null )
			_channelInfo = new ChannelInformationImpl();
		else
			_channelInfo.clear();
		
		// this should work for consumers and interactive providers
		if (_channel != null) {
			_channelInfo.set(_channel);
			if (_ommProvider == null) {
				_channelInfo.ipAddress("not available for OmmConsumer connections");
				_channelInfo.port(_channel.port());
			} else if (_ommProvider != null && _ommProvider.providerRole() == ProviderRole.NON_INTERACTIVE)
				_channelInfo.ipAddress("not available for OmmNiProvider connections");			
			return _channelInfo;
		}

		// for NiProviders, the only channel is the login channel so we'll just use
		// the channel from the LoginCallbackClient
		if (_ommProvider != null && _ommProvider.providerRole() == ProviderRole.NON_INTERACTIVE) {
			if (((OmmNiProviderImpl)(_ommProvider))._loginCallbackClient != null) {
				List<ChannelInfo> chInfo = (((OmmNiProviderImpl)(_ommProvider))._loginCallbackClient).loginChannelList();
				if (!chInfo.isEmpty()) {
					if (chInfo.get(0).rsslReactorChannel() != null) {
						_channelInfo.set(chInfo.get(0).rsslReactorChannel());
						_channelInfo.ipAddress("not available for OmmNiProvider connections");
						return _channelInfo;
					}
				}
			}
		}

		// at this point, something wasn't set
		return _channelInfo;
	}
}
