///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|          Copyright (C) 2019-2020,2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.List;
import com.refinitiv.ema.access.OmmProviderConfig.ProviderRole;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;

class OmmEventImpl<T> implements OmmConsumerEvent, OmmProviderEvent
{
	Item<T> _item;
	OmmProvider _ommProvider;
	LongObject _clientHandle = new LongObject().value(0);
	Object 	_closure;
	LongObject _handle;
	ReactorChannel _channel;
	ChannelInformationImpl _channelInfo;
	OmmBaseImpl<T> _ommBaseImpl;
	
	OmmEventImpl(OmmBaseImpl<T> baseImpl)
	{
		_ommBaseImpl = baseImpl;
	}
	
	OmmEventImpl()
	{
	}
	
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
	
	void populateChannelInfomation(ChannelInformationImpl channelInfoImpl, ReactorChannel reactorChannel, ChannelInfo channelInfo)
	{
		if (reactorChannel != null) {
			channelInfoImpl.set(reactorChannel);
			
			if(channelInfo != null)
			{
				@SuppressWarnings("unchecked")
				SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>) channelInfo.sessionChannelInfo();
				
				channelInfoImpl.channelName(channelInfo._channelConfig.name);
				
				if(sessionChannelInfo != null)
				{
					channelInfoImpl.sessionChannelName(sessionChannelInfo.sessionChannelConfig().name);
				}
			}
			
			if (_ommProvider == null) {
				channelInfoImpl.ipAddress("not available for OmmConsumer connections");
				channelInfoImpl.port(reactorChannel.port());
			} else if (_ommProvider != null && _ommProvider.providerRole() == ProviderRole.NON_INTERACTIVE)
				channelInfoImpl.ipAddress("not available for OmmNiProvider connections");		
		}
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

			if (_ommProvider != null && _ommProvider.providerRole() == ProviderRole.INTERACTIVE) {
				_channelInfo.set(_channel);
			}
			else { //Consumer & NiProvider
				ChannelInfo channelInfo = (ChannelInfo)_channel.userSpecObj();

				populateChannelInfomation(_channelInfo, _channel, channelInfo);
			}

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

	@Override
	public void sessionChannelInfo(List<ChannelInformation> channelInfoList) 
	{
		if(channelInfoList == null)
			return;
		
		channelInfoList.clear();
		
		ChannelInfo channelInfo = null;
		ChannelInformationImpl channelInfoImpl;
		
		if(_ommBaseImpl != null && _ommBaseImpl.consumerSession() != null)
		{
			List<SessionChannelInfo<T>> sessionChannelInfoList = _ommBaseImpl.consumerSession().sessionChannelList();
			
			for(SessionChannelInfo<T> sessionChInfo : sessionChannelInfoList)
			{
				ReactorChannel reactorChannel = sessionChInfo.reactorChannel();
				
				if(reactorChannel != null)
				{
					channelInfo = (ChannelInfo)reactorChannel.userSpecObj();
					
					if(channelInfo != null)
					{
						channelInfoImpl = new ChannelInformationImpl();
						
						populateChannelInfomation(channelInfoImpl, reactorChannel, channelInfo);
						
						channelInfoList.add(channelInfoImpl);
					}
				}
			}
		}		
	}
}
