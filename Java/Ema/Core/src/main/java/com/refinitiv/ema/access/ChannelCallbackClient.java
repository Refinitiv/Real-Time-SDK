///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019-2025 LSEG. All rights reserved.         	--
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;

import com.refinitiv.ema.access.OmmBaseImpl.OmmImplState;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.WriteFlags;
import com.refinitiv.eta.transport.WritePriorities;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.reactor.ConsumerRole;
import com.refinitiv.eta.valueadd.reactor.ConsumerWatchlistOptions;
import com.refinitiv.eta.valueadd.reactor.DictionaryDownloadModes;
import com.refinitiv.eta.valueadd.reactor.NIProviderRole;
import com.refinitiv.eta.valueadd.reactor.Reactor;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEventCallback;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEventTypes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelType;
import com.refinitiv.eta.valueadd.reactor.ReactorConnectInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorConnectOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorOAuthCredential;
import com.refinitiv.eta.valueadd.reactor.ReactorOAuthCredentialEventCallback;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorRole;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServerInfo;

class ChannelInfo
{
	private String				_name;
	private StringBuilder		_toString;
	private boolean				_toStringSet;
	private Reactor				_rsslReactor;
	private ReactorChannel		_rsslReactorChannel;
	private int					_reactorChannelType;
	private ChannelInfo			_parentChannel;
	private ActiveConfig		_activeConfig;
	private SessionChannelInfo<OmmConsumerClient> _sessionChannelInfo;
	protected int _majorVersion;
	protected int _minorVersion;
	protected DataDictionary		_rsslDictionary;
	
	ChannelConfig				_channelConfig;
	
	ChannelInfo(String name, Reactor rsslReactor, int reactorChannelType)
	{
		_name = name;
		_rsslReactor = rsslReactor;
		_reactorChannelType = reactorChannelType;
	}
	
	ChannelInfo(String name, Reactor rsslReactor)
	{
		_name = name;
		_rsslReactor = rsslReactor;
		_reactorChannelType = ReactorChannelType.NORMAL;
	}

	ChannelInfo reset(String name, Reactor rsslReactor, int reactorChannelType)
	{
		_name = name;
		_rsslReactor = rsslReactor;
		_reactorChannelType = reactorChannelType;
		_toStringSet = false;
		return this;
	}
	
	void clear()
	{
		_name = null;
		_rsslReactor = null;
	}
	
	String name()
	{
		return _name;
	}

	Reactor rsslReactor()
	{
		return _rsslReactor;
	}
	
	ReactorChannel rsslReactorChannel()
	{
		return _rsslReactorChannel;
	}
	
	void rsslReactorChannel(ReactorChannel rsslReactorChannel)
	{
		_rsslReactorChannel = rsslReactorChannel;
				
		_majorVersion = rsslReactorChannel.majorVersion();
		_minorVersion = rsslReactorChannel.minorVersion();
	}
		
	DataDictionary rsslDictionary()
	{
		return _rsslDictionary;
	}
	
	ChannelInfo rsslDictionary(DataDictionary rsslDictionary)
	{
		_rsslDictionary = rsslDictionary;
		return this;
	}
	
	void reactorChannelType(int reactorChannelType)
	{
		_reactorChannelType = reactorChannelType;
	}
	
	int getReactorChannelType()
	{
		return _reactorChannelType;
	}
	
	ChannelInfo getParentChannel()
	{
		return _parentChannel;
	}
	
	void setParentChannel(ChannelInfo parentChannel)
	{
		_parentChannel = parentChannel;
	}
	
	SessionChannelInfo<OmmConsumerClient> sessionChannelInfo()
	{
		return _sessionChannelInfo;
	}
	
	void sessionChannelInfo(SessionChannelInfo<OmmConsumerClient> sessionChannelInfo)
	{
		_sessionChannelInfo = sessionChannelInfo;
	}

	ActiveConfig getActiveConfig()
	{
		return _activeConfig;
	}

	void setActiveConfig(ActiveConfig activeConfig)
	{
		_activeConfig = activeConfig;
	}

	@Override
	public String toString()
	{
		if (!_toStringSet)
		{
			_toStringSet = true;
			if (_toString == null)
				_toString = new StringBuilder(1024);
			else
				_toString.setLength(0);
			
			_toString.append("\tRsslReactorChannel name ")
					 .append(_name).append(OmmLoggerClient.CR)
					 .append("\tRsslReactor ")
					 .append(Integer.toHexString(_rsslReactor.hashCode())).append(OmmLoggerClient.CR)
					 .append("\tRsslReactorChannel ")
					 .append(Integer.toHexString(_rsslReactorChannel != null ?  _rsslReactorChannel.hashCode() : 0)).append(OmmLoggerClient.CR);
		}
		
		return _toString.toString();
	}
}

class ChannelCallbackClient<T> implements ReactorChannelEventCallback
{
	private static final String CLIENT_NAME = "ChannelCallbackClient";
	
	private final List<ChannelInfo>		_channelPool = new ArrayList<>();
	private final List<ChannelInfo>		_channelList = new ArrayList<>();
	private final OmmBaseImpl<T>		_baseImpl;
	private final Reactor				_rsslReactor;
	private final ReactorConnectOptions _rsslReactorConnOptions = ReactorFactory.createReactorConnectOptions();
	private ReactorRole 				_rsslReactorRole = null;
	private ChannelInfo					_warmStandbyChannelInfo = null;
	private boolean						_bInitialChannelReadyEventReceived;
	private static String		 		_productVersion = null;
	{
		_productVersion = ChannelCallbackClient.class.getPackage().getImplementationVersion();
		if (_productVersion == null) {
			_productVersion = "EMA Java Edition";
		}
	}
	
	ChannelCallbackClient(OmmBaseImpl<T> baseImpl, Reactor rsslReactor)
	{
		_baseImpl = baseImpl;
		_rsslReactor = rsslReactor;
		_bInitialChannelReadyEventReceived = false;
		
		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME,
																		"Created ChannelCallbackClient",
																		Severity.TRACE));
		}
	}

	@SuppressWarnings("unchecked")
	@Override
	public int reactorChannelEventCallback(ReactorChannelEvent event)
	{
		_baseImpl.eventReceived();
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		ReactorChannel rsslReactorChannel  = event.reactorChannel();
		ChannelConfig channelConfig = chnlInfo._channelConfig;
		SessionChannelInfo<OmmConsumerClient> sessionChannelInfo = chnlInfo.sessionChannelInfo();
		chnlInfo.reactorChannelType(event.reactorChannel().reactorChannelType());

		_baseImpl._rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);
		if (channelConfig.rsslConnectionType == ConnectionTypes.SOCKET &&
				((SocketChannelConfig) channelConfig).directWrite)
			_baseImpl._rsslSubmitOptions.writeArgs().flags( _baseImpl._rsslSubmitOptions.writeArgs().flags() |  WriteFlags.DIRECT_SOCKET_WRITE);

		switch(event.eventType())
		{
		
			case ReactorChannelEventTypes.CHANNEL_OPENED :
			{
				if (_baseImpl.loggerClient().isTraceEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
    	        	temp.append("Received ChannelOpened on channel ");
					temp.append(chnlInfo.name()).append(OmmLoggerClient.CR)
						.append("Instance Name ").append(_baseImpl.instanceName());
					_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
				}
				
				if (sessionChannelInfo != null)
				{
					sessionChannelInfo.reactorChannel(event.reactorChannel());
					chnlInfo.rsslReactorChannel(event.reactorChannel());
				}
				
				return ReactorCallbackReturnCodes.SUCCESS;
			}
    		case ReactorChannelEventTypes.CHANNEL_UP:
    		{
    			ReactorErrorInfo rsslReactorErrorInfo = ReactorFactory.createReactorErrorInfo();
                ReactorChannelInfo reactorChannelInfo = ReactorFactory.createReactorChannelInfo();
    			
				if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					
					/* Add all the current selectable channels */
					for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().size(); i++)
					{
						try
						{
							event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i).register(_baseImpl.selector(),
									SelectionKey.OP_READ,
									event.reactorChannel());
						}
		    	        catch (ClosedChannelException e)
		    	        {
		    	        	if (_baseImpl.loggerClient().isErrorEnabled())
		    	        	{
			    	        	StringBuilder temp = _baseImpl.strBuilder();
			    	        	temp.append("Selector failed to register channel ")
									.append(chnlInfo.name()).append(OmmLoggerClient.CR)
									.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
			    	        		if (rsslReactorChannel != null && rsslReactorChannel.channel() != null )
										temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
										.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
									else
										temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
				    	        	
			    	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
		    	        	}
		    	        	return ReactorCallbackReturnCodes.FAILURE;
		    			}
						
					}
					
					setRsslReactorChannelOnChnlInfo(event.reactorChannel(), chnlInfo);
					event.reactorChannel().info(reactorChannelInfo, rsslReactorErrorInfo);

					if(sessionChannelInfo != null)
					{
						sessionChannelInfo.reactorChannel(event.reactorChannel());
						chnlInfo.rsslReactorChannel(event.reactorChannel());
						sessionChannelInfo.consumerSession().watchlist().submitItemCloseForChannel(event.reactorChannel());
					}
				}
				else
				{
					
					try
	    	        {
	    				event.reactorChannel().selectableChannel().register(_baseImpl.selector(),
	    																	SelectionKey.OP_READ,
	    																	event.reactorChannel());
	    			}
	    	        catch (ClosedChannelException e)
	    	        {
	    	        	if (_baseImpl.loggerClient().isErrorEnabled())
	    	        	{
		    	        	StringBuilder temp = _baseImpl.strBuilder();
		    	        	temp.append("Selector failed to register channel ")
								.append(chnlInfo.name()).append(OmmLoggerClient.CR)
								.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
		    	        		if (rsslReactorChannel != null && rsslReactorChannel.channel() != null )
									temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
									.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
								else
									temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
			    	        	
		    	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	    	        	}
	    	        	return ReactorCallbackReturnCodes.FAILURE;
	    			}
					
					setRsslReactorChannel(event.reactorChannel(), reactorChannelInfo, rsslReactorErrorInfo);

					if(sessionChannelInfo != null)
					{
						sessionChannelInfo.reactorChannel(event.reactorChannel());
						chnlInfo.rsslReactorChannel(event.reactorChannel());
						sessionChannelInfo.consumerSession().watchlist().submitItemCloseForChannel(event.reactorChannel());
					}
				}

    	        if (rsslReactorChannel.ioctl(com.refinitiv.eta.transport.IoctlCodes.SYSTEM_WRITE_BUFFERS, channelConfig.sysSendBufSize, rsslReactorErrorInfo) != ReactorReturnCodes.SUCCESS)
                {
    	        	if (_baseImpl.loggerClient().isErrorEnabled())
    	        	{
	    	        	StringBuilder temp = _baseImpl.strBuilder();
	    	        	temp.append("Failed to set send buffer size on channel ")
							.append(chnlInfo.name()).append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
	    	        	    if (rsslReactorChannel.channel() != null )
								temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
								.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
							else
								temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
		    	        	
							temp.append("Error Id ").append(rsslReactorErrorInfo.error().errorId()).append(OmmLoggerClient.CR)
							.append("Internal sysError ").append(rsslReactorErrorInfo.error().sysError()).append(OmmLoggerClient.CR)
							.append("Error Location ").append(rsslReactorErrorInfo.location()).append(OmmLoggerClient.CR)
							.append("Error text ").append(rsslReactorErrorInfo.error().text());

	    	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
    	        	}
    	        	
    	        	_baseImpl.closeRsslChannel(rsslReactorChannel);
    	        	
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
    	        
                if (rsslReactorChannel.ioctl(com.refinitiv.eta.transport.IoctlCodes.SYSTEM_READ_BUFFERS, channelConfig.sysRecvBufSize, rsslReactorErrorInfo) != ReactorReturnCodes.SUCCESS)
                {
                	if (_baseImpl.loggerClient().isErrorEnabled())
    	        	{
	    	        	StringBuilder temp = _baseImpl.strBuilder();
	    	        	temp.append("Failed to set recv buffer size on channel ").append(chnlInfo.name()).append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
	    	        	if (rsslReactorChannel.channel() != null)
							temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
							.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
						else
							temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
	    	        	
						temp.append("Error Id ").append(rsslReactorErrorInfo.error().errorId()).append(OmmLoggerClient.CR)
							.append("Internal sysError ").append(rsslReactorErrorInfo.error().sysError()).append(OmmLoggerClient.CR)
							.append("Error Location ").append(rsslReactorErrorInfo.location()).append(OmmLoggerClient.CR)
							.append("Error text ").append(rsslReactorErrorInfo.error().text());

	    	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
    	        	}
                	
                	_baseImpl.closeRsslChannel(rsslReactorChannel);
                	
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                
                if (channelConfig.compressionThresholdSet)
                {
	                if (rsslReactorChannel.ioctl(com.refinitiv.eta.transport.IoctlCodes.COMPRESSION_THRESHOLD, channelConfig.compressionThreshold, rsslReactorErrorInfo) != ReactorReturnCodes.SUCCESS)
	                {
	                	if (_baseImpl.loggerClient().isErrorEnabled())
	    	        	{
		    	        	StringBuilder temp = _baseImpl.strBuilder();
							
		    	        	temp.append("Failed to set compression threshold on channel ")
								.append(chnlInfo.name()).append(OmmLoggerClient.CR)
								.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
			    	        	if (rsslReactorChannel.channel() != null)
									temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
									.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
								else
									temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
			    	        	
								temp.append("Error Id ").append(rsslReactorErrorInfo.error().errorId()).append(OmmLoggerClient.CR)
								.append("Internal sysError ").append(rsslReactorErrorInfo.error().sysError()).append(OmmLoggerClient.CR)
								.append("Error Location ").append(rsslReactorErrorInfo.location()).append(OmmLoggerClient.CR)
								.append("Error text ").append(rsslReactorErrorInfo.error().text());
	
		    	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	    	        	}
	                	
	                	_baseImpl.closeRsslChannel(rsslReactorChannel);
	                	
	                    return ReactorCallbackReturnCodes.SUCCESS;
	                }
                }

				if (_baseImpl.loggerClient().isInfoEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
    	        	temp.append("Received ChannelUp event on channel ");
					temp.append(chnlInfo.name()).append(OmmLoggerClient.CR)
						.append("Instance Name ").append(_baseImpl.instanceName());

					if (reactorChannelInfo.channelInfo().componentInfo() != null)
					{
						int count = reactorChannelInfo.channelInfo().componentInfo().size();
						if (count != 0) {
							temp.append(OmmLoggerClient.CR).append("Component Version ");
							for (int i = 0; i < count; ++i) {
								temp.append(reactorChannelInfo.channelInfo().componentInfo().get(i).componentVersion());
								if (i < count - 1)
									temp.append(", ");
							}
						}
					}

					_baseImpl.loggerClient().info(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.INFO));
				}
				
				if(sessionChannelInfo != null)
				{
					sessionChannelInfo.state(OmmImplState.RSSLCHANNEL_UP);
					
    				sessionChannelInfo.consumerSession().processChannelEvent(sessionChannelInfo, event);
    				
    				if(sessionChannelInfo.consumerSession().checkAllSessionChannelHasState(OmmImplState.RSSLCHANNEL_UP))
					{
						_baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_UP);
					}
				}
				else
				{
					_baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_UP);
				}
				
				if (channelConfig.highWaterMark > 0)
				{
	                if (rsslReactorChannel.ioctl(com.refinitiv.eta.transport.IoctlCodes.HIGH_WATER_MARK, channelConfig.highWaterMark, rsslReactorErrorInfo) != ReactorReturnCodes.SUCCESS)
	                {
	                	if (_baseImpl.loggerClient().isErrorEnabled())
	    	        	{
		    	        	StringBuilder temp = _baseImpl.strBuilder();
							
		    	        	temp.append("Failed to set high water mark on channel ")
								.append(chnlInfo.name()).append(OmmLoggerClient.CR)
								.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
			    	        	if (rsslReactorChannel.channel() != null)
									temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
									.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
								else
									temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
			    	        	
								temp.append("Error Id ").append(rsslReactorErrorInfo.error().errorId()).append(OmmLoggerClient.CR)
								.append("Internal sysError ").append(rsslReactorErrorInfo.error().sysError()).append(OmmLoggerClient.CR)
								.append("Error Location ").append(rsslReactorErrorInfo.location()).append(OmmLoggerClient.CR)
								.append("Error text ").append(rsslReactorErrorInfo.error().text());
	
		    	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	    	        	}
	                	
	             	_baseImpl.closeRsslChannel(rsslReactorChannel);
	                	
	                    return ReactorCallbackReturnCodes.SUCCESS;
	                }
	                else if (_baseImpl.loggerClient().isInfoEnabled())
					{
						StringBuilder temp = _baseImpl.strBuilder();
	    	        	temp.append("high water mark set on channel ");
						temp.append(chnlInfo.name()).append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName());
						_baseImpl.loggerClient().info(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.INFO));
					}
				}
                
				return ReactorCallbackReturnCodes.SUCCESS;
    		}
    		case ReactorChannelEventTypes.FD_CHANGE:
    		{
    			
    			if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
    				setRsslReactorChannelOnChnlInfo(event.reactorChannel(), chnlInfo);
    				
					/* Remove all the old keys from the selectable key list */
					event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().forEach((oldSelectChannel) -> {
						// cancel old reactorChannel select if it's not in the current list
						if(!event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().contains(oldSelectChannel))
						{
							SelectionKey key = oldSelectChannel.keyFor(_baseImpl.selector());
							if (key != null)
								key.cancel();
						}
					});
					
					/* Add all the current selectable channels */
					for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().size(); i++)
					{
						try
						{
							if(event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i).keyFor(_baseImpl.selector()) == null)
							{
								event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i).register(_baseImpl.selector(),
										SelectionKey.OP_READ,
										event.reactorChannel());
							}
						}
						catch (ClosedChannelException e)
						{
		    	        	if (_baseImpl.loggerClient().isErrorEnabled())
		    	        	{
			    	        	StringBuilder temp = _baseImpl.strBuilder();
			    	        	temp.append("Selector failed to register channel ")
									.append(chnlInfo.name()).append(OmmLoggerClient.CR);
				    	        	if (rsslReactorChannel != null && rsslReactorChannel.channel() != null)
										temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
										.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
									else
										temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
				    	        	
			    	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
		    	        	}
		    	        	return ReactorCallbackReturnCodes.FAILURE;
						}
						
					}
				}
				else
				{
	    			
	    	        try
	    	        {
	    	            SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(_baseImpl.selector());
	    	            if (key != null)
	                       	key.cancel();
	    	        }
	    	        catch (Exception e) {}
	    
	    	        try
	    	        {
	    	        	event.reactorChannel().selectableChannel().register(_baseImpl.selector(),
	    	        													SelectionKey.OP_READ,
	    	        													event.reactorChannel());
	    	        }
	    	        catch (Exception e)
	    	        {
	    	        	if (_baseImpl.loggerClient().isErrorEnabled())
	    	        	{
		    	        	StringBuilder temp = _baseImpl.strBuilder();
		    	        	temp.append("Selector failed to register channel ")
								.append(chnlInfo.name()).append(OmmLoggerClient.CR);
			    	        	if (rsslReactorChannel != null && rsslReactorChannel.channel() != null)
									temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
									.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
								else
									temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
			    	        	
		    	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	    	        	}
	    	        	return ReactorCallbackReturnCodes.FAILURE;
	    	        }
				}
    	        
    	        if (_baseImpl.loggerClient().isTraceEnabled())
    			{
    	        	StringBuilder temp = _baseImpl.strBuilder();
    	        	temp.append("Received FD Change event on channel ")
						.append(chnlInfo.name()).append(OmmLoggerClient.CR)
						.append("Instance Name ").append(_baseImpl.instanceName());
    	        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
    			}

				return ReactorCallbackReturnCodes.SUCCESS;
    		}
    		case ReactorChannelEventTypes.CHANNEL_READY:
    		{
    			if (_baseImpl.loggerClient().isTraceEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
    	        	temp.append("Received ChannelReady event on channel ");
					temp.append(chnlInfo.name()).append(OmmLoggerClient.CR)
						.append("Instance Name ").append(_baseImpl.instanceName());
					_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
				}
    			
    			_baseImpl.processChannelEvent( event );
    			
    			if(sessionChannelInfo != null)
				{
    				sessionChannelInfo.consumerSession().processChannelEvent(sessionChannelInfo, event);
				}
    			else
    			{
	    			if ( _bInitialChannelReadyEventReceived )
	    			{
	    				_baseImpl.loginCallbackClient().processChannelEvent(event);
	    			}
	    			else
	    				_bInitialChannelReadyEventReceived = true;
				}
    			
    			return ReactorCallbackReturnCodes.SUCCESS;
    		}
    		case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
    		{
				// unregister selectableChannel from Selector
				if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					/* Remove all the old keys from the selectable key list */
					event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().forEach((oldSelectChannel) -> {
						// cancel old reactorChannel select if it's not in the current list
						if(!event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().contains(oldSelectChannel))
						{
							SelectionKey key = oldSelectChannel.keyFor(_baseImpl.selector());
							if (key != null)
								key.cancel();
						}
					});
				}
				else
				{
	                try
	                {
	                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(_baseImpl.selector());
	                    if (key != null)
	                    	key.cancel();
	                }
	                catch (Exception e) { }
				}

				if (_baseImpl.loggerClient().isWarnEnabled())
				{
					ReactorErrorInfo errorInfo = event.errorInfo();

					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received ChannelDownReconnecting event on channel ")
							.append(chnlInfo.name()).append(OmmLoggerClient.CR);
					if (rsslReactorChannel != null && rsslReactorChannel.channel() != null)
						temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
								.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
					else
						temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);

					temp.append("Error Id ").append(event.errorInfo().error().errorId()).append(OmmLoggerClient.CR)
							.append("Internal sysError ").append(errorInfo.error().sysError()).append(OmmLoggerClient.CR)
							.append("Error Location ").append(errorInfo.location()).append(OmmLoggerClient.CR)
							.append("Error text ").append(errorInfo.error().text());

					_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
				}

				if(sessionChannelInfo != null)
				{
					sessionChannelInfo.state(OmmImplState.RSSLCHANNEL_DOWN);
					
					if(sessionChannelInfo.consumerSession().checkAllSessionChannelHasState(OmmImplState.RSSLCHANNEL_DOWN))
					{
						_baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_DOWN);
					}
					
					sessionChannelInfo.consumerSession().processChannelEvent(sessionChannelInfo, event);
				}
        		else
        		{
        			_baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_DOWN);
        			
        			_baseImpl.loginCallbackClient().processChannelEvent(event);
        		}
        		
         	   _baseImpl.processChannelEvent(event);
            	
            	return ReactorCallbackReturnCodes.SUCCESS;
    		}
    		case ReactorChannelEventTypes.CHANNEL_DOWN:
            {
				// unregister selectableChannel from Selector
				if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					/* Remove all the old keys from the selectable key list */
					event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().forEach((oldSelectChannel) -> {
						// cancel old reactorChannel select if it's not in the current list
						if(!event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().contains(oldSelectChannel))
						{
							SelectionKey key = oldSelectChannel.keyFor(_baseImpl.selector());
							if (key != null)
								key.cancel();
						}
					});
				}
				else
				{
		        	   try
		               {
		                   SelectionKey key = rsslReactorChannel.selectableChannel().keyFor(_baseImpl.selector());
		                   if (key != null)
		                      	key.cancel();
		               }
		               catch (Exception e) {}
				}

				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					ReactorErrorInfo errorInfo = event.errorInfo();
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received ChannelDown event on channel ")
							.append(chnlInfo.name()).append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
					if (rsslReactorChannel != null && rsslReactorChannel.channel() != null && rsslReactorChannel.reactor() != null)
						temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
								.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
					else
						temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);

					temp.append("Error Id ").append(event.errorInfo().error().errorId()).append(OmmLoggerClient.CR)
							.append("Internal sysError ").append(errorInfo.error().sysError()).append(OmmLoggerClient.CR)
							.append("Error Location ").append(errorInfo.location()).append(OmmLoggerClient.CR)
							.append("Error text ").append(errorInfo.error().text());

					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
				}

        	   if(sessionChannelInfo != null)
        	   {
        		   sessionChannelInfo.state(OmmImplState.RSSLCHANNEL_DOWN);
        		   
        		   if(sessionChannelInfo.consumerSession().checkAllSessionChannelHasState(OmmImplState.RSSLCHANNEL_DOWN))
					{
						_baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_DOWN);
					}
        		   
        		   sessionChannelInfo.consumerSession().processChannelEvent(sessionChannelInfo, event);        		   
        		   _baseImpl.closeSessionChannel((SessionChannelInfo<T>) sessionChannelInfo);
        		   
        		   sessionChannelInfo.onChannelClose(chnlInfo);
        	   }
        	   else
        	   {
        		   _baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_DOWN);
            	   
            	   _baseImpl.processChannelEvent(event);
        		   
        		   _baseImpl.loginCallbackClient().processChannelEvent(event);
        		   _baseImpl.closeRsslChannel(event.reactorChannel());
        	   }
			
        	   return ReactorCallbackReturnCodes.SUCCESS;
            }
            case ReactorChannelEventTypes.WARNING:
            {
            	if (_baseImpl.loggerClient().isWarnEnabled())
          	   	{
            		ReactorErrorInfo errorInfo = event.errorInfo();
            		 
  					StringBuilder temp = _baseImpl.strBuilder();
  		        	temp.append("Received Channel warning event on channel ")
  						.append(chnlInfo.name()).append(OmmLoggerClient.CR)
  						.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
	    	        	if (rsslReactorChannel != null && rsslReactorChannel.channel() != null)
							temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
							.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
						else
							temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
	    	        	
						temp.append("Error Id ").append(event.errorInfo().error().errorId()).append(OmmLoggerClient.CR)
  						.append("Internal sysError ").append(errorInfo.error().sysError()).append(OmmLoggerClient.CR)
  						.append("Error Location ").append(errorInfo.location()).append(OmmLoggerClient.CR)
  						.append("Error text ").append(errorInfo.error().text());
  	
  		        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.WARNING));
          	   	}
            	return ReactorCallbackReturnCodes.SUCCESS;
            }
			case ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE:
			{
				if (_baseImpl.loggerClient().isInfoEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received PreferredHostSwitchoverComplete event on channel ");
					temp.append(chnlInfo.name()).append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName());
					_baseImpl.loggerClient().info(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.INFO));
				}

				_baseImpl.processChannelEvent(event);

				if (sessionChannelInfo != null)
				{
					sessionChannelInfo.phOperationInProgress(false);
					sessionChannelInfo.consumerSession().processChannelEvent(sessionChannelInfo, event);
				}
				else
				{
					_baseImpl.loginCallbackClient().processChannelEvent(event);
				}

				return ReactorCallbackReturnCodes.SUCCESS;
			}
			case ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK:
			{
				if (_baseImpl.loggerClient().isInfoEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received PreferredHostStartFallback event on channel ");
					temp.append(chnlInfo.name()).append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName());
					_baseImpl.loggerClient().info(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.INFO));
				}

				_baseImpl.processChannelEvent(event);

				if (sessionChannelInfo != null)
				{
					sessionChannelInfo.phOperationInProgress(true);
					sessionChannelInfo.consumerSession().processChannelEvent(sessionChannelInfo, event);
				}
				else
				{
					_baseImpl.loginCallbackClient().processChannelEvent(event);
				}

				return ReactorCallbackReturnCodes.SUCCESS;
			}
            default:
            {
            	if (_baseImpl.loggerClient().isErrorEnabled())
         	   	{
         		    ReactorErrorInfo errorInfo = event.errorInfo();
 					StringBuilder temp = _baseImpl.strBuilder();
 		        	temp.append("Received unknown channel event type ")
 						.append(chnlInfo.name()).append(OmmLoggerClient.CR)
 						.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
	    	        	if (rsslReactorChannel != null && rsslReactorChannel.channel() != null )
							temp.append("RsslReactor ").append("@").append(Integer.toHexString(rsslReactorChannel.reactor().hashCode() )).append(OmmLoggerClient.CR)
							.append("RsslChannel ").append("@").append(Integer.toHexString(rsslReactorChannel.channel().hashCode())).append(OmmLoggerClient.CR);
						else
							temp.append("RsslReactor Channel is null").append(OmmLoggerClient.CR);
	    	        	
						temp.append("Error Id ").append(event.errorInfo().error().errorId()).append(OmmLoggerClient.CR)
 						.append("Internal sysError ").append(errorInfo.error().sysError()).append(OmmLoggerClient.CR)
 						.append("Error Location ").append(errorInfo.location()).append(OmmLoggerClient.CR)
 						.append("Error text ").append(errorInfo.error().text());
 	
 		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
         	   	}
            	return ReactorCallbackReturnCodes.FAILURE;
            }
		}
	}

	@SuppressWarnings("fallthrough")
	private String channelParametersToString(ActiveConfig activeConfig,  ChannelConfig channelCfg )
	{
		boolean bValidChType = true;
		StringBuilder cfgParameters = new StringBuilder(512);
		String compType;
		String strConnectionType = "SOCKET";
		switch (channelCfg.compressionType)
		{
		case com.refinitiv.eta.transport.CompressionTypes.ZLIB:
			{
				compType = "ZLib";
				break;
			}
		case com.refinitiv.eta.transport.CompressionTypes.LZ4:
			{
				compType = "LZ4";
				break;
			}
		case com.refinitiv.eta.transport.CompressionTypes.NONE:
			{
				compType = "None";
				break;
			}
		default:
			{	
				compType = "Unknown Compression Type";
				break;
			}
		}
		
		switch (channelCfg.rsslConnectionType)
		{
		case com.refinitiv.eta.transport.ConnectionTypes.WEBSOCKET:
			strConnectionType = "WEBSOCKET";
			//fallthrough because WEBSOCKET channel should be handled similarly to SOCKET
		case com.refinitiv.eta.transport.ConnectionTypes.SOCKET: {
			SocketChannelConfig tempChannelCfg = (SocketChannelConfig) channelCfg;
			cfgParameters.append( "hostName " ).append( tempChannelCfg.hostName ).append( OmmLoggerClient.CR )
					.append( "port " ).append( tempChannelCfg.serviceName ).append( OmmLoggerClient.CR )
					.append( "CompressionType " ).append( compType ).append( OmmLoggerClient.CR )
					.append( "tcpNodelay " ).append( ( tempChannelCfg.tcpNodelay ? "true" : "false" ) ).append( OmmLoggerClient.CR );

			break;
		}
		case com.refinitiv.eta.transport.ConnectionTypes.HTTP:
		case com.refinitiv.eta.transport.ConnectionTypes.ENCRYPTED:
			{
				HttpChannelConfig tempChannelCfg = (HttpChannelConfig) channelCfg;
				strConnectionType = ConnectionTypes.toString(tempChannelCfg.rsslConnectionType);
				cfgParameters.append( "hostName " ).append( tempChannelCfg.hostName ).append( OmmLoggerClient.CR )
				.append( "port " ).append( tempChannelCfg.serviceName ).append( OmmLoggerClient.CR )
				.append( "CompressionType " ).append( compType ).append( OmmLoggerClient.CR )
				.append( "tcpNodelay " ).append( ( tempChannelCfg.tcpNodelay ? "true" : "false" ) ).append( OmmLoggerClient.CR )
				.append( "ObjectName " ).append( tempChannelCfg.objectName ).append( OmmLoggerClient.CR )
				.append( "EnableSessionMgnt " ).append( ( tempChannelCfg.enableSessionMgnt ? "true" : "false" ) ).append( OmmLoggerClient.CR );
				
				// Provides additional logging information for encrypted connection.
				if(channelCfg.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.ENCRYPTED)
				{
					EncryptedChannelConfig tempEncryptedChannelCfg = (EncryptedChannelConfig)channelCfg;
					cfgParameters.append( "EncryptedProtocolType " ).append( tempEncryptedChannelCfg.encryptedProtocolType ).append( OmmLoggerClient.CR )
					.append( "SecurityProtocol " ).append( tempEncryptedChannelCfg.encryptionConfig.SecurityProtocol).append( OmmLoggerClient.CR )
					.append( "SecurityProtocolVersions " ).append( Arrays.toString(tempEncryptedChannelCfg.encryptionConfig.SecurityProtocolVersions)).append( OmmLoggerClient.CR )
					.append( "EnableSessionMgnt " ).append( ( tempEncryptedChannelCfg.enableSessionMgnt ? "true" : "false" ) ).append( OmmLoggerClient.CR )
					.append( "Location " ).append( tempEncryptedChannelCfg.location ).append( OmmLoggerClient.CR );
				}
				break;
			}
		default:
			{	
				strConnectionType = "Invalid ChannelType: ";
				strConnectionType += com.refinitiv.eta.transport.ConnectionTypes.toString(channelCfg.rsslConnectionType);
				bValidChType = false;
				break;
			}
		}

		if (channelCfg.rsslConnectionType == ConnectionTypes.WEBSOCKET
				|| (channelCfg.rsslConnectionType == ConnectionTypes.ENCRYPTED && channelCfg.encryptedProtocolType == ConnectionTypes.WEBSOCKET)) {
			cfgParameters
					.append("WsMaxMsgSize ").append(channelCfg.wsMaxMsgSize).append(OmmLoggerClient.CR)
					.append("WsProtocols ").append(channelCfg.wsProtocols).append(OmmLoggerClient.CR);
		}

		StringBuilder tempBlder = _baseImpl.strBuilder();
		tempBlder.append( strConnectionType ).append( OmmLoggerClient.CR )
		.append( "Channel name " ).append( channelCfg.name ).append( OmmLoggerClient.CR )
		.append( "Instance Name " ).append( _baseImpl.instanceName() ).append( OmmLoggerClient.CR );

		if (bValidChType)
		{
			tempBlder.append("RsslReactor ").append("@").append(Integer.toHexString(_rsslReactor.hashCode())).append(OmmLoggerClient.CR)
			.append( "InterfaceName " ).append( channelCfg.interfaceName ).append( OmmLoggerClient.CR )
			.append( cfgParameters )
			.append( "reconnectAttemptLimit " ).append( activeConfig.reconnectAttemptLimit ).append( OmmLoggerClient.CR )
			.append( "reconnectMinDelay " ).append( activeConfig.reconnectMinDelay ).append( " msec" ).append( OmmLoggerClient.CR )
			.append( "reconnectMaxDelay " ).append( activeConfig.reconnectMaxDelay ).append( " msec" ).append( OmmLoggerClient.CR )
			.append( "guaranteedOutputBuffers " ).append( channelCfg.guaranteedOutputBuffers ).append( OmmLoggerClient.CR )
			.append( "numInputBuffers " ).append( channelCfg.numInputBuffers ).append( OmmLoggerClient.CR )
			.append( "sysRecvBufSize " ).append( channelCfg.sysRecvBufSize ).append( OmmLoggerClient.CR )
			.append( "sysSendBufSize " ).append( channelCfg.sysSendBufSize ).append( OmmLoggerClient.CR )
			.append( "connectionPingTimeout " ).append( channelCfg.connectionPingTimeout ).append( " msec" ).append( OmmLoggerClient.CR )
			.append( "initializationTimeout " ).append( channelCfg.initializationTimeout ).append( " sec" ).append( OmmLoggerClient.CR );
		}
		
		return tempBlder.toString();
	}

	@SuppressWarnings("unchecked")
	private ReactorConnectInfo channelConfigToReactorConnectInfo(ChannelConfig channelConfig, List<ChannelConfig> activeConfigChannelSet, SessionChannelInfo<T> sessionChannelInfo,
			ChannelInfo wsbChannelInfo)
	{
		int connectionType = channelConfig.rsslConnectionType;
		ReactorConnectInfo reactorConnectInfo = ReactorFactory.createReactorConnectInfo();

		ChannelInfo channelInfo = channelInfo(channelConfig.name, _rsslReactor);
		channelInfo.setActiveConfig(_baseImpl.activeConfig());
		channelInfo._channelConfig = channelConfig;
		channelConfig.channelInfo = channelInfo;
		channelInfo.setParentChannel(wsbChannelInfo);

		reactorConnectInfo.connectOptions().userSpecObject(channelInfo);
		
		if(sessionChannelInfo != null)
		{
			channelInfo.sessionChannelInfo((SessionChannelInfo<OmmConsumerClient>)sessionChannelInfo);
			sessionChannelInfo.channelInfoList().add(channelInfo);
		}
		else
		{
			_channelList.add(channelInfo);
		}

		reactorConnectInfo.connectOptions().majorVersion(com.refinitiv.eta.codec.Codec.majorVersion());
		reactorConnectInfo.connectOptions().minorVersion(com.refinitiv.eta.codec.Codec.minorVersion());
		reactorConnectInfo.connectOptions().protocolType(com.refinitiv.eta.codec.Codec.protocolType());
		
		reactorConnectInfo.connectOptions().compressionType(channelConfig.compressionType);
		reactorConnectInfo.connectOptions().connectionType(connectionType);
		reactorConnectInfo.connectOptions().pingTimeout(channelConfig.connectionPingTimeout/1000);
		reactorConnectInfo.connectOptions().guaranteedOutputBuffers(channelConfig.guaranteedOutputBuffers);
		reactorConnectInfo.connectOptions().sysRecvBufSize(channelConfig.sysRecvBufSize);
		reactorConnectInfo.connectOptions().sysSendBufSize(channelConfig.sysSendBufSize);
		reactorConnectInfo.connectOptions().numInputBuffers(channelConfig.numInputBuffers);
		reactorConnectInfo.connectOptions().componentVersion(_productVersion);

		switch (reactorConnectInfo.connectOptions().connectionType())
		{
		case com.refinitiv.eta.transport.ConnectionTypes.ENCRYPTED:
		{
			if(channelConfig.encryptedProtocolType == com.refinitiv.eta.transport.ConnectionTypes.HTTP || 
					channelConfig.encryptedProtocolType == com.refinitiv.eta.transport.ConnectionTypes.SOCKET ||
					channelConfig.encryptedProtocolType == com.refinitiv.eta.transport.ConnectionTypes.WEBSOCKET)
			{
				reactorConnectInfo.enableSessionManagement(channelConfig.enableSessionMgnt);
				reactorConnectInfo.location(channelConfig.location);

				reactorConnectInfo.connectOptions().unifiedNetworkInfo().address(((HttpChannelConfig) channelConfig).hostName);
				try
				{
					reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName(((HttpChannelConfig) channelConfig).serviceName);
				}
				catch(Exception e) 
				{
	        	   if (_baseImpl.loggerClient().isErrorEnabled())
	        	   {
	        		   StringBuilder tempErr = _baseImpl.strBuilder();
						tempErr.append("Failed to set service name on channel options, received exception: '")
	        				     .append(e.getLocalizedMessage())
	        				     .append( "'. ");
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, tempErr.toString(), Severity.ERROR));
	        	   }
				}
				reactorConnectInfo.connectOptions().tcpOpts().tcpNoDelay(((HttpChannelConfig) channelConfig).tcpNodelay);
				reactorConnectInfo.connectOptions().tunnelingInfo().objectName(((HttpChannelConfig) channelConfig).objectName);
				reactorConnectInfo.connectOptions().encryptionOptions().connectionType(channelConfig.encryptedProtocolType);
				tunnelingConfiguration(reactorConnectInfo.connectOptions(), (HttpChannelConfig)channelConfig);
            }
			else /* Multiple checks prior to this point, so we don't need additional verification */
			{
				reactorConnectInfo.connectOptions().unifiedNetworkInfo().address(((SocketChannelConfig) channelConfig).hostName);
				try
				{
					reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName(((SocketChannelConfig) channelConfig).serviceName);
				}
				catch (Exception e)
				{
					if (_baseImpl.loggerClient().isErrorEnabled())
					{
						StringBuilder tempErr = _baseImpl.strBuilder();
						tempErr.append("Failed to set service name on channel options, received exception: '")
								.append(e.getLocalizedMessage())
								.append("'. ");
						_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, tempErr.toString(), Severity.ERROR));
					}
				}
				reactorConnectInfo.connectOptions().tcpOpts().tcpNoDelay(((SocketChannelConfig) channelConfig).tcpNodelay);
				socketProxyConfiguration(reactorConnectInfo.connectOptions(), (SocketChannelConfig)channelConfig);
            }
            break;
        }
		case com.refinitiv.eta.transport.ConnectionTypes.WEBSOCKET:
		case com.refinitiv.eta.transport.ConnectionTypes.SOCKET:
		{
			if ( channelConfig.channelInfo != null &&
                    Objects.equals(((ChannelInfo) reactorConnectInfo.connectOptions().userSpecObject())._channelConfig.name, channelConfig.name))
			{
				try
				{
					reactorConnectInfo.connectOptions().unifiedNetworkInfo().address(((SocketChannelConfig)  channelConfig).hostName);
					reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName(((SocketChannelConfig)  channelConfig).serviceName);
				}
				catch (Exception e)
				{
					if (_baseImpl.loggerClient().isErrorEnabled())
					{
						StringBuilder tempErr = _baseImpl.strBuilder();
						tempErr.append("Failed to set service name on channel options, received exception: '")
								.append(e.getLocalizedMessage())
								.append("'. ");
						_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, tempErr.toString(), Severity.ERROR));
					}
				}
				reactorConnectInfo.connectOptions().tcpOpts().tcpNoDelay(((SocketChannelConfig) channelConfig).tcpNodelay);
				socketProxyConfiguration(reactorConnectInfo.connectOptions(), (SocketChannelConfig) channelConfig);

				reactorConnectInfo.enableSessionManagement(channelConfig.enableSessionMgnt);
			}
			break;
		}
		
		case com.refinitiv.eta.transport.ConnectionTypes.HTTP:
			{
				reactorConnectInfo.connectOptions().unifiedNetworkInfo().address(((HttpChannelConfig) channelConfig).hostName);
				try
				{
					reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName(((HttpChannelConfig) channelConfig).serviceName);
				}
				catch(Exception e) 
				{
	        	   if (_baseImpl.loggerClient().isErrorEnabled())
	        	   {
	        		   StringBuilder tempErr = _baseImpl.strBuilder();
						tempErr.append("Failed to set service name on channel options, received exception: '")
	        				     .append(e.getLocalizedMessage())
	        				     .append( "'. ");
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, tempErr.toString(), Severity.ERROR));
	        	   }
				}
				reactorConnectInfo.connectOptions().tcpOpts().tcpNoDelay(((HttpChannelConfig) channelConfig).tcpNodelay);
				reactorConnectInfo.connectOptions().tunnelingInfo().objectName(((HttpChannelConfig) channelConfig).objectName);
				tunnelingConfiguration(reactorConnectInfo.connectOptions(), (HttpChannelConfig)channelConfig);
			break;
			}
		default :
			break;
		}

		reactorConnectInfo.connectOptions().unifiedNetworkInfo().interfaceName( channelConfig.interfaceName);
		reactorConnectInfo.connectOptions().unifiedNetworkInfo().unicastServiceName("");

		if (reactorConnectInfo.connectOptions().connectionType() == ConnectionTypes.WEBSOCKET
				|| reactorConnectInfo.connectOptions().encryptionOptions().connectionType() == ConnectionTypes.WEBSOCKET) {
			reactorConnectInfo.connectOptions().wSocketOpts().protocols(channelConfig.wsProtocols);
			reactorConnectInfo.connectOptions().wSocketOpts().maxMsgSize(channelConfig.wsMaxMsgSize);
		}

		return reactorConnectInfo;
	}
	
	private void initalizeSessionChannel()
	{
		ActiveConfig activeConfig = _baseImpl.activeConfig();
		List<SessionChannelConfig>	activeSessionChannelSet = activeConfig.configSessionChannelSet;

		ConsumerSession<T> consumerSession = _baseImpl.consumerSession();
		
		StringBuilder temp = new StringBuilder();
		temp.append("Attempt to connect using session channels");
		
		String channelParams;

		StringBuilder errorStrUnsupportedConnectionType = new StringBuilder();		
		errorStrUnsupportedConnectionType.append( "Unsupported connection type. Passed in type is ");
		
		for(int i = 0;i < activeSessionChannelSet.size(); i++)
		{
			SessionChannelConfig sessionChannelConfig = activeSessionChannelSet.get(i);
			
			sessionChannelConfig.connectOptions().reconnectAttemptLimit(sessionChannelConfig.reconnectAttemptLimit);
			sessionChannelConfig.connectOptions().reconnectMinDelay(sessionChannelConfig.reconnectMinDelay);
			sessionChannelConfig.connectOptions().reconnectMaxDelay(sessionChannelConfig.reconnectMaxDelay);
			
			SessionChannelInfo<T> sessionChannelInfo = new SessionChannelInfo<>(sessionChannelConfig, consumerSession);
			
			consumerSession.addSessionChannelInfo(sessionChannelInfo);
			
			ReactorConnectInfo reactorConnectInfo;
			ChannelConfig activeChannelConfig;
			int rsslReactorConnListSize;
			String channelNames = "";
			int supportedConnectionTypeChannelCount = 0;
			
			// Handle the channel set of SessionChannel
			for (int j = 0; j < sessionChannelConfig.configChannelSet.size(); j++)
			{
				activeChannelConfig = sessionChannelConfig.configChannelSet.get(j);
				int channelCfgSetLastIndex = sessionChannelConfig.configChannelSet.size() - 1;
				
				if (activeChannelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.SOCKET  ||
						activeChannelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.HTTP ||
						activeChannelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.ENCRYPTED ||
						activeChannelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.WEBSOCKET)
				{	
					reactorConnectInfo = channelConfigToReactorConnectInfo(activeChannelConfig, sessionChannelConfig.configChannelSet, sessionChannelInfo, null);
					supportedConnectionTypeChannelCount++;
					channelNames += (activeChannelConfig.name + " ");
					rsslReactorConnListSize = sessionChannelConfig.connectOptions().connectionList().size();
                    if (j >= rsslReactorConnListSize) {
                        sessionChannelConfig.connectOptions().connectionList().add(reactorConnectInfo);
                    }
                    reactorConnectInfo.connectOptions().copy(sessionChannelConfig.connectOptions().connectionList().get(j).connectOptions());
                    sessionChannelConfig.connectOptions().connectionList().get(j).location(reactorConnectInfo.location());
                    sessionChannelConfig.connectOptions().connectionList().get(j).enableSessionManagement(reactorConnectInfo.enableSessionManagement());
                    sessionChannelConfig.connectOptions().connectionList().get(j).initTimeout(activeChannelConfig.initializationTimeout);
                    sessionChannelConfig.connectOptions().connectionList().get(j).serviceDiscoveryRetryCount(activeChannelConfig.serviceDiscoveryRetryCount);

                    if (_baseImpl.loggerClient().isTraceEnabled())
					{
						channelParams = channelParametersToString( _baseImpl.activeConfig(), activeChannelConfig );
						temp.append( OmmLoggerClient.CR ).append( i + 1 ).append( "] " ).append( channelParams );
						if ( j == ( channelCfgSetLastIndex ) )				
							_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
					}	
				}
				else
				{
					errorStrUnsupportedConnectionType.append( ConnectionTypes.toString(activeChannelConfig.rsslConnectionType)).append( " for " )
					.append( activeChannelConfig.name );
					if ( i < sessionChannelConfig.configChannelSet.size() - 1 )
						errorStrUnsupportedConnectionType.append( ", " );
				}
			}
			
			// Handling the WSB channel set of SessionChannel
			WarmStandbyChannelConfig warmStandbyChannelConfig;
			ReactorConnectInfo wsbStartingActiveServerReactorConnectInfo = null;
			
			if (!sessionChannelConfig.configWarmStandbySet.isEmpty())
			{		
				for (int k = 0; k < sessionChannelConfig.configWarmStandbySet.size(); ++k)
				{
					sessionChannelConfig.connectOptions().reactorWarmStandbyGroupList().add(ReactorFactory.createReactorWarmStandbyGroup());
				}
			}

			// Handle active c going to Warm Standby Groups
			for (int k = 0; k < sessionChannelConfig.configWarmStandbySet.size(); ++k)
			{				
				warmStandbyChannelConfig = sessionChannelConfig.configWarmStandbySet.get(k);
				
				_warmStandbyChannelInfo = new ChannelInfo(warmStandbyChannelConfig.name, _rsslReactor, ReactorChannelType.WARM_STANDBY);
				// Handle active config going to Starting Active Server of this Warm Standby Group
				if (warmStandbyChannelConfig.startingActiveServer != null)
				{
					if (warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.SOCKET  ||
						warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.HTTP ||
						warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.ENCRYPTED ||
						warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.WEBSOCKET)
					{
						wsbStartingActiveServerReactorConnectInfo = channelConfigToReactorConnectInfo(warmStandbyChannelConfig.startingActiveServer.channelConfig, null, sessionChannelInfo, _warmStandbyChannelInfo);
						supportedConnectionTypeChannelCount++;
						channelNames += (warmStandbyChannelConfig.startingActiveServer.channelConfig.name + " ");	
					}
					else
					{
						errorStrUnsupportedConnectionType.append( ConnectionTypes.toString(warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType)).append( " for " )
						.append( warmStandbyChannelConfig.startingActiveServer.channelConfig.name ).append( ", " );
					}
				
					if (!warmStandbyChannelConfig.startingActiveServer.perServiceNameSet.isEmpty())
					{
						sessionChannelConfig.connectOptions().reactorWarmStandbyGroupList().get(k).startingActiveServer().perServiceBasedOptions().serviceNameList(new ArrayList<Buffer>());
						
						for (int index = 0; index < warmStandbyChannelConfig.startingActiveServer.perServiceNameSet.size(); index++)
						{
							Buffer serviceName = CodecFactory.createBuffer();
							serviceName.data(warmStandbyChannelConfig.startingActiveServer.perServiceNameSet.get(index));
							sessionChannelConfig.connectOptions().reactorWarmStandbyGroupList().get(k).startingActiveServer().perServiceBasedOptions().serviceNameList().add(serviceName);
						}
					}

					// Copy Reactor Connect Info into Warm Standby Channel Group's specific Reactor Connect Info
					sessionChannelConfig.connectOptions().reactorWarmStandbyGroupList().get(k).startingActiveServer().reactorConnectInfo(wsbStartingActiveServerReactorConnectInfo);
				}
				
				// Handle active config going to StandbyServerSet of this Warm Standby Group
				if (!warmStandbyChannelConfig.standbyServerSet.isEmpty())
				{
					for (int j = 0; j < warmStandbyChannelConfig.standbyServerSet.size(); ++j)
					{
						sessionChannelConfig.connectOptions().reactorWarmStandbyGroupList().get(k).standbyServerList().add(ReactorFactory.createReactorWarmStandbyServerInfo());
					}
				}
				
				ReactorWarmStandbyServerInfo warmStandbyServerInfo;
				ReactorConnectInfo wsbStandbyChannelReactorConnectInfo = null;
				
				// Handle active config going to each Standby Server of this Warm Standby Group
				for (int j = 0; j < sessionChannelConfig.connectOptions().reactorWarmStandbyGroupList().get(k).standbyServerList().size(); ++j)
				{
					warmStandbyServerInfo = sessionChannelConfig.connectOptions().reactorWarmStandbyGroupList().get(k).standbyServerList().get(j);
					warmStandbyServerInfo.clear();

					if (warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.SOCKET  ||
							warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.HTTP ||
							warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.ENCRYPTED ||
							warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.WEBSOCKET)
						{
							wsbStandbyChannelReactorConnectInfo = channelConfigToReactorConnectInfo(warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig, null, sessionChannelInfo, _warmStandbyChannelInfo);
							supportedConnectionTypeChannelCount++;
							channelNames += (warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.name + " ");
						}
						else
						{
							errorStrUnsupportedConnectionType.append( ConnectionTypes.toString(warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType)).append( " for " )
							.append( warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.name );
							if ( j < warmStandbyChannelConfig.standbyServerSet.size() - 1 )
								errorStrUnsupportedConnectionType.append( ", " );
						}
					
					if (!warmStandbyChannelConfig.standbyServerSet.get(j).perServiceNameSet.isEmpty())
					{
						warmStandbyServerInfo.perServiceBasedOptions().serviceNameList(new ArrayList<Buffer>());
						
						for (int index = 0; index < warmStandbyChannelConfig.standbyServerSet.get(j).perServiceNameSet.size(); index++)
						{
							Buffer serviceName = CodecFactory.createBuffer();
							serviceName.data(warmStandbyChannelConfig.standbyServerSet.get(j).perServiceNameSet.get(index));
							warmStandbyServerInfo.perServiceBasedOptions().serviceNameList().add(serviceName);
						}
					}
					
					// Copy Reactor Connect Info into Warm Standby Channel Group's specific Reactor Connect Info
					sessionChannelConfig.connectOptions().reactorWarmStandbyGroupList().get(k).standbyServerList().get(j).reactorConnectInfo(wsbStandbyChannelReactorConnectInfo);
				}
				
				sessionChannelConfig.connectOptions().reactorWarmStandbyGroupList().get(k).warmStandbyMode(warmStandbyChannelConfig.warmStandbyMode);
			}
			
			if(supportedConnectionTypeChannelCount > 0)
			{
				ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
				
				_baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_DOWN);
				
				if (ReactorReturnCodes.SUCCESS > _rsslReactor.connect(sessionChannelConfig.connectOptions(), _rsslReactorRole, rsslErrorInfo))
				{
					com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
					StringBuilder tempErr = _baseImpl.strBuilder();
					tempErr.append("Failed to add RsslChannel(s) to RsslReactor. Channel name(s) ")
					    .append( channelNames ).append(OmmLoggerClient.CR)
						.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
						.append("RsslReactor ").append("@").append(Integer.toHexString(_rsslReactor.hashCode())).append(OmmLoggerClient.CR)
						.append("RsslChannel ").append(error.channel()).append(OmmLoggerClient.CR)
						.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
						.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
						.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
						.append("Error Text ").append(error.text());

					if (_baseImpl.loggerClient().isErrorEnabled())
						_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, tempErr.toString(), Severity.ERROR));
					
					for(int k = 0; i <  sessionChannelConfig.connectOptions().connectionList().size(); k++ )
					{
						ChannelInfo checkChannelInfo = (ChannelInfo) sessionChannelConfig.connectOptions().connectionList().get(k).connectOptions().userSpecObject();
						if( checkChannelInfo != null)
						{
							removeChannel( checkChannelInfo );
						}
					}
					
					throw _baseImpl.ommIUExcept().message(tempErr.toString(), rsslErrorInfo.code());
			    }

				if (_baseImpl.loggerClient().isTraceEnabled())
				{
					StringBuilder tempTrace = _baseImpl.strBuilder();
					tempTrace.append("Successfully created a Reactor and Channel(s)")
		            	.append(OmmLoggerClient.CR)
					    .append(" Channel name(s) ").append( channelNames ).append(OmmLoggerClient.CR)
						.append("Instance Name ").append(_baseImpl.instanceName());
					_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, tempTrace.toString(), Severity.TRACE));
				}	
			}
			else
			{
				if(sessionChannelConfig.configChannelSet.isEmpty() && sessionChannelConfig.configWarmStandbySet.isEmpty())
				{
					errorStrUnsupportedConnectionType.setLength(0);
					errorStrUnsupportedConnectionType.append("There is no valid channel for session name: ").append(sessionChannelConfig.name);
				}
				
				if (_baseImpl.loggerClient().isErrorEnabled())
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, errorStrUnsupportedConnectionType.toString(), Severity.ERROR));

				throw _baseImpl.ommIUExcept().message( errorStrUnsupportedConnectionType.toString(), OmmInvalidUsageException.ErrorCode.UNSUPPORTED_CHANNEL_TYPE );
			}
		}
	}
	
	private void initializeReactor()
	{
		ActiveConfig activeConfig = _baseImpl.activeConfig();
		List<ChannelConfig>	activeConfigChannelSet = activeConfig.channelConfigSet;
		StringBuilder channelNames = new StringBuilder();
		int supportedConnectionTypeChannelCount = 0;
		
		StringBuilder temp = new StringBuilder();
		if( activeConfigChannelSet.size() > 1 )
			temp.append("Attempt to connect using the following list");
		else
			temp.append("Attempt to connect using ");
		
		int channelCfgSetLastIndex = activeConfigChannelSet.size() - 1;

		StringBuilder errorStrUnsupportedConnectionType = new StringBuilder();
		errorStrUnsupportedConnectionType.append( "Unsupported connection type. Passed in type is ");

		
		List<WarmStandbyChannelConfig> warmStandbyChannelSet = _baseImpl.activeConfig().configWarmStandbySet;
		
		if (!warmStandbyChannelSet.isEmpty())
		{		
			for (int i = 0; i < warmStandbyChannelSet.size(); ++i)
			{
				_rsslReactorConnOptions.reactorWarmStandbyGroupList().add(ReactorFactory.createReactorWarmStandbyGroup());
			}
		}
		_rsslReactorConnOptions.reconnectAttemptLimit(activeConfig.reconnectAttemptLimit);
		_rsslReactorConnOptions.reconnectMinDelay(activeConfig.reconnectMinDelay);
		_rsslReactorConnOptions.reconnectMaxDelay(activeConfig.reconnectMaxDelay);

		// Preferred Host fallback attributes
		_rsslReactorConnOptions.reactorPreferredHostOptions().isPreferredHostEnabled(activeConfig.enablePreferredHostOptions);
		_rsslReactorConnOptions.reactorPreferredHostOptions().detectionTimeSchedule(activeConfig.detectionTimeSchedule);
		_rsslReactorConnOptions.reactorPreferredHostOptions().detectionTimeInterval(activeConfig.detectionTimeInterval);
		_rsslReactorConnOptions.reactorPreferredHostOptions().connectionListIndex(activeConfig.connectionListIndex);
		_rsslReactorConnOptions.reactorPreferredHostOptions().warmStandbyGroupListIndex(activeConfig.warmStandbyGroupListIndex);
		_rsslReactorConnOptions.reactorPreferredHostOptions().fallBackWithInWSBGroup(activeConfig.fallBackWithInWSBGroup);

		ReactorConnectInfo reactorConnectInfo;
		ChannelConfig activeChannelConfig;

		// Handle activeConfig information going to channelList
		for (int i = 0; i < activeConfigChannelSet.size(); i++)
		{
			activeChannelConfig = activeConfigChannelSet.get(i);
			
			if (activeChannelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.SOCKET  ||
					activeChannelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.HTTP ||
					activeChannelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.ENCRYPTED ||
					activeChannelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.WEBSOCKET)
			{
				reactorConnectInfo = channelConfigToReactorConnectInfo(activeChannelConfig, activeConfigChannelSet, null, null);
				supportedConnectionTypeChannelCount++;
				channelNames.append(activeChannelConfig.name);
				int rsslReactorConnListSize = _rsslReactorConnOptions.connectionList().size();
                if (i >= rsslReactorConnListSize) {
                    _rsslReactorConnOptions.connectionList().add(reactorConnectInfo);
                }
                reactorConnectInfo.connectOptions().copy(_rsslReactorConnOptions.connectionList().get(i).connectOptions());
                _rsslReactorConnOptions.connectionList().get(i).location(reactorConnectInfo.location());
                _rsslReactorConnOptions.connectionList().get(i).enableSessionManagement(reactorConnectInfo.enableSessionManagement());
                _rsslReactorConnOptions.connectionList().get(i).initTimeout(activeChannelConfig.initializationTimeout);
                _rsslReactorConnOptions.connectionList().get(i).serviceDiscoveryRetryCount(activeChannelConfig.serviceDiscoveryRetryCount);

                if (_baseImpl.loggerClient().isTraceEnabled())
				{
					String channelParams = channelParametersToString( _baseImpl.activeConfig(), activeConfigChannelSet.get( i ) );
					temp.append( OmmLoggerClient.CR ).append( i + 1 ).append( "] " ).append( channelParams );
					if ( i == ( channelCfgSetLastIndex ) )				
						_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
				}	
			}
			else
			{
				errorStrUnsupportedConnectionType.append( ConnectionTypes.toString(activeChannelConfig.rsslConnectionType)).append( " for " )
				.append( activeConfigChannelSet.get(i).name );
				if ( i < activeConfigChannelSet.size() - 1 )
					errorStrUnsupportedConnectionType.append( ", " );
			}
		}

		WarmStandbyChannelConfig warmStandbyChannelConfig;
		ReactorConnectInfo wsbStartingActiveServerReactorConnectInfo = null;

		// Handle active config going to Warm Standby Groups
		for (int i = 0; i < _rsslReactorConnOptions.reactorWarmStandbyGroupList().size(); ++i)
		{
			_warmStandbyChannelInfo = new ChannelInfo("", _rsslReactor, ReactorChannelType.WARM_STANDBY);
			_warmStandbyChannelInfo.setActiveConfig(activeConfig);
			
			warmStandbyChannelConfig = warmStandbyChannelSet.get(i);
			
			_warmStandbyChannelInfo = new ChannelInfo(warmStandbyChannelConfig.name, _rsslReactor, ReactorChannelType.WARM_STANDBY);
			
			// Handle active config going to Starting Active Server of this Warm Standby Group
			if (warmStandbyChannelConfig.startingActiveServer != null)
			{
				if (warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.SOCKET  ||
					warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.HTTP ||
					warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.ENCRYPTED ||
					warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.WEBSOCKET)
				{
					wsbStartingActiveServerReactorConnectInfo = channelConfigToReactorConnectInfo(warmStandbyChannelConfig.startingActiveServer.channelConfig, activeConfigChannelSet, null, _warmStandbyChannelInfo);
					supportedConnectionTypeChannelCount++;
					channelNames.append(warmStandbyChannelConfig.startingActiveServer.channelConfig.name);
				}
				else
				{
					errorStrUnsupportedConnectionType.append( ConnectionTypes.toString(warmStandbyChannelConfig.startingActiveServer.channelConfig.rsslConnectionType)).append( " for " )
					.append( activeConfigChannelSet.get(i).name );
					if ( i < activeConfigChannelSet.size() - 1 )
						errorStrUnsupportedConnectionType.append( ", " );
				}
			
				if (!warmStandbyChannelConfig.startingActiveServer.perServiceNameSet.isEmpty())
				{
					_rsslReactorConnOptions.reactorWarmStandbyGroupList().get(i).startingActiveServer().perServiceBasedOptions().serviceNameList(new ArrayList<Buffer>());
					
					for (int index = 0; index < warmStandbyChannelConfig.startingActiveServer.perServiceNameSet.size(); index++)
					{
						Buffer serviceName = CodecFactory.createBuffer();
						serviceName.data(warmStandbyChannelConfig.startingActiveServer.perServiceNameSet.get(index));
						_rsslReactorConnOptions.reactorWarmStandbyGroupList().get(i).startingActiveServer().perServiceBasedOptions().serviceNameList().add(serviceName);
					}
				}

				// Copy Reactor Connect Info into Warm Standby Channel Group's specific Reactor Connect Info
				_rsslReactorConnOptions.reactorWarmStandbyGroupList().get(i).startingActiveServer().reactorConnectInfo(wsbStartingActiveServerReactorConnectInfo);
			}
			
			// Handle active config going to StandbyServerSet of this Warm Standby Group
			if (!warmStandbyChannelConfig.standbyServerSet.isEmpty())
			{
				for (int j = 0; j < warmStandbyChannelConfig.standbyServerSet.size(); ++j)
				{
					_rsslReactorConnOptions.reactorWarmStandbyGroupList().get(i).standbyServerList().add(ReactorFactory.createReactorWarmStandbyServerInfo());
				}
			}
			
			ReactorWarmStandbyServerInfo warmStandbyServerInfo;
			ReactorConnectInfo wsbStandbyChannelReactorConnectInfo = null;
			
			// Handle active config going to each Standby Server of this Warm Standby Group
			for (int j = 0; j < _rsslReactorConnOptions.reactorWarmStandbyGroupList().get(i).standbyServerList().size(); ++j)
			{
				warmStandbyServerInfo = _rsslReactorConnOptions.reactorWarmStandbyGroupList().get(i).standbyServerList().get(j);
				warmStandbyServerInfo.clear();

				if (warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.SOCKET  ||
						warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.HTTP ||
						warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.ENCRYPTED ||
						warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType == com.refinitiv.eta.transport.ConnectionTypes.WEBSOCKET)
					{
						wsbStandbyChannelReactorConnectInfo = channelConfigToReactorConnectInfo(warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig, activeConfigChannelSet, null, _warmStandbyChannelInfo);
						supportedConnectionTypeChannelCount++;
						channelNames.append(warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.name);
					}
					else
					{
						errorStrUnsupportedConnectionType.append( ConnectionTypes.toString(warmStandbyChannelConfig.standbyServerSet.get(j).channelConfig.rsslConnectionType)).append( " for " )
						.append( activeConfigChannelSet.get(i).name );
						if ( i < activeConfigChannelSet.size() - 1 )
							errorStrUnsupportedConnectionType.append( ", " );
					}
				
				if (!warmStandbyChannelConfig.standbyServerSet.get(j).perServiceNameSet.isEmpty())
				{
					warmStandbyServerInfo.perServiceBasedOptions().serviceNameList(new ArrayList<Buffer>());
					
					for (int index = 0; index < warmStandbyChannelConfig.standbyServerSet.get(j).perServiceNameSet.size(); index++)
					{
						Buffer serviceName = CodecFactory.createBuffer();
						serviceName.data(warmStandbyChannelConfig.standbyServerSet.get(j).perServiceNameSet.get(index));
						warmStandbyServerInfo.perServiceBasedOptions().serviceNameList().add(serviceName);
					}
				}
				
				// Copy Reactor Connect Info into Warm Standby Channel Group's specific Reactor Connect Info
				_rsslReactorConnOptions.reactorWarmStandbyGroupList().get(i).standbyServerList().get(j).reactorConnectInfo(wsbStandbyChannelReactorConnectInfo);
			}
			
			_rsslReactorConnOptions.reactorWarmStandbyGroupList().get(i).warmStandbyMode(warmStandbyChannelConfig.warmStandbyMode);
		}

		// Check to make sure we have support connections and call connect
		if( supportedConnectionTypeChannelCount > 0 )
		{
			ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
			if (ReactorReturnCodes.SUCCESS > _rsslReactor.connect(_rsslReactorConnOptions, _rsslReactorRole, rsslErrorInfo))
			{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				StringBuilder tempErr = _baseImpl.strBuilder();
				tempErr.append("Failed to add RsslChannel(s) to RsslReactor. Channel name(s) ")
				    .append( channelNames ).append(OmmLoggerClient.CR)
					.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
					.append("RsslReactor ").append("@").append(Integer.toHexString(_rsslReactor.hashCode())).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append(error.channel()).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
					.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
					.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
					.append("Error Text ").append(error.text());

				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, tempErr.toString(), Severity.ERROR));
				
				for(int i = 0; i <  _rsslReactorConnOptions.connectionList().size(); i++ )
				{
					ChannelInfo checkChannelInfo = (ChannelInfo) _rsslReactorConnOptions.connectionList().get(i).connectOptions().userSpecObject();
					if( checkChannelInfo != null)
					{
						removeChannel( checkChannelInfo );
					}
				}
				
				throw _baseImpl.ommIUExcept().message(tempErr.toString(), rsslErrorInfo.code());
			}

			_baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_DOWN);

			if (_baseImpl.loggerClient().isTraceEnabled())
			{
				StringBuilder tempTrace = _baseImpl.strBuilder();
				tempTrace.append("Successfully created a Reactor and Channel(s)")
	            	.append(OmmLoggerClient.CR)
				    .append(" Channel name(s) ").append( channelNames ).append(OmmLoggerClient.CR)
					.append("Instance Name ").append(_baseImpl.instanceName());
				_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, tempTrace.toString(), Severity.TRACE));
			}	
		}
		else
		{
			 if (_baseImpl.loggerClient().isErrorEnabled())
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, errorStrUnsupportedConnectionType.toString(), Severity.ERROR));

			 throw _baseImpl.ommIUExcept().message( errorStrUnsupportedConnectionType.toString(), OmmInvalidUsageException.ErrorCode.UNSUPPORTED_CHANNEL_TYPE );
		}
			
	}

	void initializeConsumerRole(LoginRequest loginReq, DirectoryRequest dirReq, EmaConfigImpl configImpl, ReactorOAuthCredentialEventCallback credentialCallback)
	{
        ConsumerRole consumerRole = ReactorFactory.createConsumerRole();
		
        loginReq.applyHasRole();
		loginReq.role(Login.RoleTypes.CONS);
		consumerRole.rdmLoginRequest(loginReq);
		consumerRole.rdmDirectoryRequest(dirReq);
		consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
		consumerRole.loginMsgCallback(_baseImpl.loginCallbackClient());
		consumerRole.dictionaryMsgCallback(_baseImpl.dictionaryCallbackClient());
		consumerRole.directoryMsgCallback(_baseImpl.directoryCallbackClient());
		consumerRole.channelEventCallback(_baseImpl.channelCallbackClient());
		consumerRole.defaultMsgCallback(_baseImpl.itemCallbackClient());
		
		ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
		
		oAuthCredential.takeExclusiveSignOnControl(configImpl.takeExclusiveSignOnControl());
		
		if(credentialCallback != null)
		{
			oAuthCredential.reactorOAuthCredentialEventCallback(credentialCallback);
		}
		
		/* The Client ID is required parameter to enable the session management */
		if(configImpl.clientId().length() != 0 )
		{				
			oAuthCredential.clientId(configImpl.clientId());
		}
		
		if(configImpl.clientSecret().length() != 0 )
		{				
			oAuthCredential.clientSecret(configImpl.clientSecret());
		}
		
		if(configImpl.tokenScope().length() != 0 )
		{				
			oAuthCredential.tokenScope(configImpl.tokenScope());
		}
		
		if(configImpl.clientJwk().length() != 0 )
		{				
			oAuthCredential.clientJwk(configImpl.clientJwk());
		}
		
		if(configImpl.audience().length() != 0 )
		{				
			oAuthCredential.audience(configImpl.audience());
		}
		
		
		oAuthCredential.reactorOAuthCredentialEventCallback(credentialCallback);
		
		consumerRole.reactorOAuthCredential(oAuthCredential);
		
		ConsumerWatchlistOptions watchlistOptions = consumerRole.watchlistOptions();
		watchlistOptions.channelOpenCallback(this);
		watchlistOptions.enableWatchlist(true);
		watchlistOptions.itemCountHint(_baseImpl.activeConfig().itemCountHint);
		watchlistOptions.obeyOpenWindow(_baseImpl.activeConfig().obeyOpenWindow > 0);
		watchlistOptions.postAckTimeout(_baseImpl.activeConfig().postAckTimeout);
		watchlistOptions.requestTimeout(_baseImpl.activeConfig().requestTimeout);
		watchlistOptions.maxOutstandingPosts(_baseImpl.activeConfig().maxOutstandingPosts);
		
		_rsslReactorRole = consumerRole;
		
		
		if(!_baseImpl.activeConfig().configSessionChannelSet.isEmpty())
		{
			if (_baseImpl.loggerClient().isTraceEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Initializing session channels: ");
	        	
	        	int count = _baseImpl.activeConfig().configSessionChannelSet.size();
	        	for(SessionChannelConfig config : _baseImpl.activeConfig().configSessionChannelSet)
	        	{
	        		if(--count > 0)
	        			temp.append(config.name).append(", ");
					else
						temp.append(config.name).append(OmmLoggerClient.CR);
	        	}
	      
				_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
			}
			
			initalizeSessionChannel();
		}
		else
		{
			initializeReactor();
		}
	}
	
	void initializeNiProviderRole(LoginRequest loginReq, DirectoryRefresh directoryRefresh)
	{
		NIProviderRole niProviderRole = ReactorFactory.createNIProviderRole();
	
		loginReq.applyHasRole();
		loginReq.role(Login.RoleTypes.PROV);
		niProviderRole.rdmLoginRequest(loginReq);
		niProviderRole.loginMsgCallback(_baseImpl.loginCallbackClient());
		niProviderRole.channelEventCallback(this);
		niProviderRole.defaultMsgCallback(_baseImpl.itemCallbackClient());
		niProviderRole.rdmDirectoryRefresh(directoryRefresh);
		
		_rsslReactorRole = niProviderRole;
		
		initializeReactor();
	}

	private void tunnelingConfiguration(com.refinitiv.eta.transport.ConnectOptions rsslOptions, HttpChannelConfig channelConfig)
    {    	
        if (channelConfig.httpProxy)
        {
            if (channelConfig.httpProxyHostName == null)
            {
            	if (_baseImpl.loggerClient().isErrorEnabled())
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, "Proxy hostname not provided", Severity.ERROR));

            	throw _baseImpl.ommIUExcept().message( "Proxy hostname not provided", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );
            }           
            if (channelConfig.httpProxyPort == null)
            {
            	if (_baseImpl.loggerClient().isErrorEnabled())
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, "Proxy port number not provided", Severity.ERROR));

            	throw _baseImpl.ommIUExcept().message( "Proxy port number not provided", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );
            }                             	
            
            rsslOptions.tunnelingInfo().HTTPproxy(true);
            rsslOptions.tunnelingInfo().HTTPproxyHostName(channelConfig.httpProxyHostName);
            try
            {
            	rsslOptions.tunnelingInfo().HTTPproxyPort(Integer.parseInt(channelConfig.httpProxyPort));
            }
            catch(Exception e)
            {
            	if (_baseImpl.loggerClient().isErrorEnabled())
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, "Proxy port number not provided", Severity.ERROR));

            	throw _baseImpl.ommIUExcept().message( "Proxy port number not provided", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );
            }
            
            // set credentials
            if ( channelConfig.httpProxyUserName != null)
            {
            	 rsslOptions.credentialsInfo().HTTPproxyUsername(channelConfig.httpProxyUserName);
            }         	
            if ( channelConfig.httpproxyPasswd != null)
            {
            	 rsslOptions.credentialsInfo().HTTPproxyPasswd(channelConfig.httpproxyPasswd);
            }     
            if ( channelConfig.httpProxyDomain != null)
            {
            	rsslOptions.credentialsInfo().HTTPproxyDomain(channelConfig.httpProxyDomain);		        		        		
            }
          
            if (channelConfig.httpProxyLocalHostName == null)
            {
            	 String localIPaddress = null;
                 String localHostName;
            	   try
                   {
                       localIPaddress = InetAddress.getLocalHost().getHostAddress();
                       localHostName = InetAddress.getLocalHost().getHostName();
                   }
                   catch (UnknownHostException e)
                   {
                	   localHostName = localIPaddress;
                   }
            	 rsslOptions.credentialsInfo().HTTPproxyLocalHostname(localHostName);     		        		        		
            }
            else
            {
            	rsslOptions.credentialsInfo().HTTPproxyLocalHostname(channelConfig.httpProxyLocalHostName);
            }
        	    	
            if (channelConfig.httpProxyKRB5ConfigFile != null)
            {
            	 rsslOptions.credentialsInfo().HTTPproxyKRB5configFile(channelConfig.httpProxyKRB5ConfigFile);      		        		        		
            }                       
        }
        
        if (channelConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
        {
			readEncryptedChannelConfig(rsslOptions, channelConfig.encryptionConfig);
		}
        else
        	rsslOptions.tunnelingInfo().tunnelingType("http"); 
        
        if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder tempTrace = _baseImpl.strBuilder();
			tempTrace.append("Successfully set a tunneling ")
            	.append(OmmLoggerClient.CR)
			    .append(" Channel name ").append( channelConfig.name ).append(OmmLoggerClient.CR)
				.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
				.append("with the following tunneling configurations:")
				.append( OmmLoggerClient.CR );
			if (rsslOptions.tunnelingInfo().tunnelingType().equals("encrypted"))
			{
				tempTrace.append(rsslOptions.tunnelingInfo().toString()).append( OmmLoggerClient.CR );
			}
			else if (rsslOptions.tunnelingInfo().tunnelingType().equals("None"))
			{
				tempTrace.append(rsslOptions.encryptionOptions().toString());
				
				if (rsslOptions.tunnelingInfo().HTTPproxy())
				{
					tempTrace.append("\tHTTP Proxy").append( OmmLoggerClient.CR );
					tempTrace.append("\t\tHTTPproxy: ").append("true").append( OmmLoggerClient.CR );
					tempTrace.append("\t\tHTTPproxyHostName: " ).append(rsslOptions.tunnelingInfo().HTTPproxyHostName()).append( OmmLoggerClient.CR );
					tempTrace.append("\t\tHTTPproxyPort: " ).append(rsslOptions.tunnelingInfo().HTTPproxyPort()).append( OmmLoggerClient.CR );
				}
			}
			else
			{
				tempTrace.append(rsslOptions.tunnelingInfo().toString()).append( OmmLoggerClient.CR );
			}
			
				tempTrace.append(rsslOptions.credentialsInfo().toString()).append( OmmLoggerClient.CR );
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, tempTrace.toString(), Severity.TRACE));
		}	
	}
	
	private void socketProxyConfiguration(com.refinitiv.eta.transport.ConnectOptions rsslOptions, SocketChannelConfig channelConfig)
    {    	
        if (channelConfig.httpProxy)
        {
            if (channelConfig.httpProxyHostName == null)
            {
            	if (_baseImpl.loggerClient().isErrorEnabled())
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, "Proxy hostname not provided", Severity.ERROR));

            	throw _baseImpl.ommIUExcept().message( "Proxy hostname not provided", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );
            }           
            if (channelConfig.httpProxyPort == null)
            {
            	if (_baseImpl.loggerClient().isErrorEnabled())
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, "Proxy port number not provided", Severity.ERROR));

            	throw _baseImpl.ommIUExcept().message( "Proxy port number not provided", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );
            }                             	
            
            rsslOptions.tunnelingInfo().HTTPproxy(true);
            rsslOptions.tunnelingInfo().HTTPproxyHostName(channelConfig.httpProxyHostName);
            try
            {
            	rsslOptions.tunnelingInfo().HTTPproxyPort(Integer.parseInt(channelConfig.httpProxyPort));
            }
            catch(Exception e)
            {
            	if (_baseImpl.loggerClient().isErrorEnabled())
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, "Proxy port number not provided", Severity.ERROR));

            	throw _baseImpl.ommIUExcept().message( "Proxy port number not provided", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );
            }
            
            // set credentials
            if ( channelConfig.httpProxyUserName != null)
            {
            	 rsslOptions.credentialsInfo().HTTPproxyUsername(channelConfig.httpProxyUserName);
            }         	
            if ( channelConfig.httpproxyPasswd != null)
            {
            	 rsslOptions.credentialsInfo().HTTPproxyPasswd(channelConfig.httpproxyPasswd);
            }     
            if ( channelConfig.httpProxyDomain != null)
            {
            	rsslOptions.credentialsInfo().HTTPproxyDomain(channelConfig.httpProxyDomain);		        		        		
            }   
          
            if (channelConfig.httpProxyLocalHostName == null)
            {
            	 String localIPaddress = null;
                 String localHostName;
            	   try
                   {
                       localIPaddress = InetAddress.getLocalHost().getHostAddress();
                       localHostName = InetAddress.getLocalHost().getHostName();
                   }
                   catch (UnknownHostException e)
                   {
                	   localHostName = localIPaddress;
                   }
            	 rsslOptions.credentialsInfo().HTTPproxyLocalHostname(localHostName);     		        		        		
            }
            else
            {
            	rsslOptions.credentialsInfo().HTTPproxyLocalHostname(channelConfig.httpProxyLocalHostName);
            }
        	    	
            if (channelConfig.httpProxyKRB5ConfigFile != null)
            {
            	 rsslOptions.credentialsInfo().HTTPproxyKRB5configFile(channelConfig.httpProxyKRB5ConfigFile);      		        		        		
            }                       
        }
        
        if (channelConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
        {
        	if(channelConfig.encryptedProtocolType == ConnectionTypes.HTTP)
        	{
        		rsslOptions.tunnelingInfo().tunnelingType("HTTP");
	        	rsslOptions.tunnelingInfo().KeystoreFile(channelConfig.encryptionConfig.KeyStoreFile);
	    		rsslOptions.tunnelingInfo().KeystorePasswd(channelConfig.encryptionConfig.KeyStorePasswd);
	    		if (channelConfig.encryptionConfig.KeyStoreType != null)
	    			 rsslOptions.tunnelingInfo().KeystoreType(channelConfig.encryptionConfig.KeyStoreType);
	    		if (channelConfig.encryptionConfig.SecurityProtocol != null)
	    			 rsslOptions.tunnelingInfo().SecurityProtocol(channelConfig.encryptionConfig.SecurityProtocol);
	    		if (channelConfig.encryptionConfig.SecurityProtocolVersions != null)
	    			 rsslOptions.tunnelingInfo().SecurityProtocolVersions(channelConfig.encryptionConfig.SecurityProtocolVersions);
	    		if (channelConfig.encryptionConfig.SecurityProvider != null)
	    			 rsslOptions.tunnelingInfo().SecurityProvider(channelConfig.encryptionConfig.SecurityProvider);
	    		if (channelConfig.encryptionConfig.TrustManagerAlgorithm != null)
	    			 rsslOptions.tunnelingInfo().TrustManagerAlgorithm(channelConfig.encryptionConfig.TrustManagerAlgorithm);
	    		if (channelConfig.encryptionConfig.KeyManagerAlgorithm != null)
	    			 rsslOptions.tunnelingInfo().KeyManagerAlgorithm(channelConfig.encryptionConfig.KeyManagerAlgorithm);
        	}
        	else
        	{
        		rsslOptions.tunnelingInfo().tunnelingType("None");
        		rsslOptions.encryptionOptions().connectionType(ConnectionTypes.SOCKET);
        		rsslOptions.encryptionOptions().KeystoreFile(channelConfig.encryptionConfig.KeyStoreFile);
	    		rsslOptions.encryptionOptions().KeystorePasswd(channelConfig.encryptionConfig.KeyStorePasswd);
	    		if (channelConfig.encryptionConfig.KeyStoreType != null)
	    			 rsslOptions.encryptionOptions().KeystoreType(channelConfig.encryptionConfig.KeyStoreType);
	    		if (channelConfig.encryptionConfig.SecurityProtocol != null)
	    			 rsslOptions.encryptionOptions().SecurityProtocol(channelConfig.encryptionConfig.SecurityProtocol);
	    		if (channelConfig.encryptionConfig.SecurityProtocolVersions != null)
	    			rsslOptions.encryptionOptions().SecurityProtocolVersions(channelConfig.encryptionConfig.SecurityProtocolVersions);
	    		if (channelConfig.encryptionConfig.SecurityProvider != null)
	    			 rsslOptions.encryptionOptions().SecurityProvider(channelConfig.encryptionConfig.SecurityProvider);
	    		if (channelConfig.encryptionConfig.TrustManagerAlgorithm != null)
	    			 rsslOptions.encryptionOptions().TrustManagerAlgorithm(channelConfig.encryptionConfig.TrustManagerAlgorithm);
	    		if (channelConfig.encryptionConfig.KeyManagerAlgorithm != null)
	    			 rsslOptions.encryptionOptions().KeyManagerAlgorithm(channelConfig.encryptionConfig.KeyManagerAlgorithm);
        	}
		}
        
        if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder tempTrace = _baseImpl.strBuilder();
			tempTrace.append("Successfully set a tunneling ")
            	.append(OmmLoggerClient.CR)
			    .append(" Channel name ").append( channelConfig.name ).append(OmmLoggerClient.CR)
				.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
				.append("with the following tunneling configurations:")
				.append( OmmLoggerClient.CR ).append(rsslOptions.tunnelingInfo().toString()).append( OmmLoggerClient.CR )
				.append(rsslOptions.credentialsInfo().toString()).append( OmmLoggerClient.CR );
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, tempTrace.toString(), Severity.TRACE));
		}	
	}

	private void readEncryptedChannelConfig(ConnectOptions rsslOptions, EncryptionConfig channelConfig)
	{
		if(rsslOptions.encryptionOptions().connectionType() == ConnectionTypes.HTTP)
		{
			rsslOptions.tunnelingInfo().tunnelingType("encrypted");
			
			rsslOptions.tunnelingInfo().KeystoreFile(channelConfig.KeyStoreFile);
			rsslOptions.tunnelingInfo().KeystorePasswd(channelConfig.KeyStorePasswd);
			if (channelConfig.KeyStoreType != null)
				 rsslOptions.tunnelingInfo().KeystoreType(channelConfig.KeyStoreType);
			if (channelConfig.SecurityProtocol != null)
				 rsslOptions.tunnelingInfo().SecurityProtocol(channelConfig.SecurityProtocol);
			if (channelConfig.SecurityProtocolVersions != null)
				 rsslOptions.tunnelingInfo().SecurityProtocolVersions(channelConfig.SecurityProtocolVersions);
			if (channelConfig.SecurityProvider != null)
				 rsslOptions.tunnelingInfo().SecurityProvider(channelConfig.SecurityProvider);
			if (channelConfig.TrustManagerAlgorithm != null)
				 rsslOptions.tunnelingInfo().TrustManagerAlgorithm(channelConfig.TrustManagerAlgorithm);
			if (channelConfig.KeyManagerAlgorithm != null)
				 rsslOptions.tunnelingInfo().KeyManagerAlgorithm(channelConfig.KeyManagerAlgorithm);
		}
		else
		{
			rsslOptions.tunnelingInfo().tunnelingType("None");
			rsslOptions.encryptionOptions().connectionType(rsslOptions.encryptionOptions().connectionType());
			rsslOptions.encryptionOptions().KeystoreFile(channelConfig.KeyStoreFile);
			rsslOptions.encryptionOptions().KeystorePasswd(channelConfig.KeyStorePasswd);
			if (channelConfig.KeyStoreType != null)
   			     rsslOptions.encryptionOptions().KeystoreType(channelConfig.KeyStoreType);
			if (channelConfig.SecurityProtocol != null)
  			     rsslOptions.encryptionOptions().SecurityProtocol(channelConfig.SecurityProtocol);
			if (channelConfig.SecurityProtocolVersions != null)
				rsslOptions.encryptionOptions().SecurityProtocolVersions(channelConfig.SecurityProtocolVersions);
			if (channelConfig.SecurityProvider != null)
  			     rsslOptions.encryptionOptions().SecurityProvider(channelConfig.SecurityProvider);
			if (channelConfig.TrustManagerAlgorithm != null)
  			     rsslOptions.encryptionOptions().TrustManagerAlgorithm(channelConfig.TrustManagerAlgorithm);
			if (channelConfig.KeyManagerAlgorithm != null)
  			     rsslOptions.encryptionOptions().KeyManagerAlgorithm(channelConfig.KeyManagerAlgorithm);
		}
	}

	private void setRsslReactorChannel(ReactorChannel rsslReactorChannel, ReactorChannelInfo rsslReactorChannlInfo, ReactorErrorInfo rsslReactorErrorInfo)
	{
		for (int index = _channelList.size() -1; index >= 0; index--)
		{
			_channelList.get(index).rsslReactorChannel(rsslReactorChannel);
			_channelList.get(index).rsslReactorChannel().info(rsslReactorChannlInfo, rsslReactorErrorInfo);
		}
	}
	
	private void setRsslReactorChannelOnChnlInfo(ReactorChannel rsslReactorChannel, ChannelInfo chnlInfo)
	{
		if (chnlInfo.getParentChannel() != null)
			chnlInfo.getParentChannel().rsslReactorChannel(rsslReactorChannel);
		chnlInfo.rsslReactorChannel(rsslReactorChannel);
	}
	
	private ChannelInfo channelInfo(String name, Reactor rsslReactor)
	{
		if (_channelPool.isEmpty())
			return new ChannelInfo(name, rsslReactor, ReactorChannelType.NORMAL);
		else 
			return (_channelPool.get(0).reset(name, rsslReactor, ReactorChannelType.NORMAL));
	}
	
	void removeChannel(ChannelInfo chanInfo)
	{
		if (chanInfo != null)
		{
			_baseImpl.loginCallbackClient().removeChannelInfo(chanInfo.rsslReactorChannel());
			_channelList.remove( chanInfo );
			chanInfo.clear();
			_channelPool.add( chanInfo );
		}
	}

	void closeChannels()
	{
		if(_baseImpl.consumerSession() == null)
		{
			for (int index = _channelList.size() -1; index >= 0; index--)
				_baseImpl.closeRsslChannel(_channelList.get(index).rsslReactorChannel());
		}
		else
		{
			_baseImpl.closeConsumerSession();
		}
	}

	List<ChannelInfo>  channelList()
	{
		return _channelList;
	}

	public void removeSessionChannel(SessionChannelInfo<T> sessionChannelInfo)
	{
		if(sessionChannelInfo != null)
		{
			_baseImpl.loginCallbackClient().removeSessionChannelInfo(sessionChannelInfo);
			
		}
		
	}
}
