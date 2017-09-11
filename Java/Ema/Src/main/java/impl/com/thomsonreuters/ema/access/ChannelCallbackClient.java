///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.ema.access.OmmBaseImpl.OmmImplState;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.WriteFlags;
import com.thomsonreuters.upa.transport.WritePriorities;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.reactor.ConsumerRole;
import com.thomsonreuters.upa.valueadd.reactor.ConsumerWatchlistOptions;
import com.thomsonreuters.upa.valueadd.reactor.DictionaryDownloadModes;
import com.thomsonreuters.upa.valueadd.reactor.NIProviderRole;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventCallback;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventTypes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorConnectOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorRole;
import com.thomsonreuters.upa.valueadd.reactor.ReactorConnectInfo;

class ChannelInfo
{
	private String				_name;
	private StringBuilder		_toString;
	private boolean				_toStringSet;
	private Reactor				_rsslReactor;
	private ReactorChannel		_rsslReactorChannel;		
	protected int _majorVersion;
	protected int _minorVersion;
	protected DataDictionary		_rsslDictionary;
	
	ChannelConfig				_channelConfig;
	
	ChannelInfo(String name, Reactor rsslReactor)
	{
		_name = name;
		_rsslReactor = rsslReactor;
	}

	ChannelInfo reset(String name, Reactor rsslReactor)
	{
		_name = name;
		_rsslReactor = rsslReactor;
		_toStringSet = false;
		return this;
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
	
	private List<ChannelInfo>			_channelPool = new ArrayList<ChannelInfo>();
	private List<ChannelInfo>			_channelList = new ArrayList<ChannelInfo>();
	private OmmBaseImpl<T>				_baseImpl;
	private Reactor						_rsslReactor;
	private ReactorConnectOptions 		_rsslReactorConnOptions = ReactorFactory.createReactorConnectOptions();
	private ReactorRole 				_rsslReactorRole = null;
	private boolean						_bInitialChannelReadyEventReceived;
    Package 							_package = Package.getPackage("com.thomsonreuters.ema.access");
	String 								_productVersion;
	
	ChannelCallbackClient(OmmBaseImpl<T> baseImpl, Reactor rsslReactor)
	{
		_baseImpl = baseImpl;
		_rsslReactor = rsslReactor;
		_rsslReactorConnOptions.connectionList().add(ReactorFactory.createReactorConnectInfo());
		_bInitialChannelReadyEventReceived = false;
		
		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME,
																		"Created ChannelCallbackClient",
																		Severity.TRACE).toString());
		}

        _productVersion = _package.getImplementationVersion();
        if (_productVersion == null)
        	_productVersion = "EMA Java Edition";
	}

	@Override
	public int reactorChannelEventCallback(ReactorChannelEvent event)
	{
		_baseImpl.eventReceived();
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		ReactorChannel rsslReactorChannel  = event.reactorChannel();
		ChannelConfig channelConfig = chnlInfo._channelConfig;
		
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
				return ReactorCallbackReturnCodes.SUCCESS;
			}
    		case ReactorChannelEventTypes.CHANNEL_UP:
    		{
    			ReactorErrorInfo rsslReactorErrorInfo = _baseImpl.rsslErrorInfo();
                ReactorChannelInfo reactorChannelInfo = new ReactorChannelInfo();
    			
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
    	       
    	        chnlInfo.rsslReactorChannel(event.reactorChannel());
                chnlInfo.rsslReactorChannel().info(reactorChannelInfo, rsslReactorErrorInfo);
    	        
    	        int sendBufSize = 65535;
    	        if (rsslReactorChannel.ioctl(com.thomsonreuters.upa.transport.IoctlCodes.SYSTEM_WRITE_BUFFERS, sendBufSize, rsslReactorErrorInfo) != ReactorReturnCodes.SUCCESS)
                {
    	        	if (_baseImpl.loggerClient().isErrorEnabled())
    	        	{
	    	        	StringBuilder temp = _baseImpl.strBuilder();
	    	        	temp.append("Failed to set send buffer size on channel ")
							.append(chnlInfo.name()).append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
	    	        	    if (rsslReactorChannel != null && rsslReactorChannel.channel() != null )
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

    	    	int rcvBufSize = 65535;
                if (rsslReactorChannel.ioctl(com.thomsonreuters.upa.transport.IoctlCodes.SYSTEM_READ_BUFFERS, rcvBufSize, rsslReactorErrorInfo) != ReactorReturnCodes.SUCCESS)
                {
                	if (_baseImpl.loggerClient().isErrorEnabled())
    	        	{
	    	        	StringBuilder temp = _baseImpl.strBuilder();
	    	        	temp.append("Failed to set recv buffer size on channel ").append(chnlInfo.name()).append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
	    	        	if (rsslReactorChannel != null && rsslReactorChannel.channel() != null)
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
                
                               
                if (rsslReactorChannel.ioctl(com.thomsonreuters.upa.transport.IoctlCodes.COMPRESSION_THRESHOLD, channelConfig.compressionThreshold, rsslReactorErrorInfo) != ReactorReturnCodes.SUCCESS)
                {
                	if (_baseImpl.loggerClient().isErrorEnabled())
    	        	{
	    	        	StringBuilder temp = _baseImpl.strBuilder();
						
	    	        	temp.append("Failed to set compression threshold on channel ")
							.append(chnlInfo.name()).append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
		    	        	if (rsslReactorChannel != null && rsslReactorChannel.channel() != null)
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

				if (_baseImpl.loggerClient().isInfoEnabled())
				{
					int count = reactorChannelInfo.channelInfo().componentInfo().size();
					
					StringBuilder temp = _baseImpl.strBuilder();
    	        	temp.append("Received ChannelUp event on channel ");
					temp.append(chnlInfo.name()).append(OmmLoggerClient.CR)
						.append("Instance Name ").append(_baseImpl.instanceName());
						
						if ( count != 0 )
						{
							temp.append(OmmLoggerClient.CR).append("Component Version ");
							for (int i = 0; i < count; ++i)
							{
								temp.append(reactorChannelInfo.channelInfo().componentInfo().get(i).componentVersion());
								if (i < count - 1)
									temp.append(", ");
							}
						}
					
					_baseImpl.loggerClient().info(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.INFO));
				}
	
				_baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_UP);
				
				if (channelConfig.highWaterMark > 0)
				{
	                if (rsslReactorChannel.ioctl(com.thomsonreuters.upa.transport.IoctlCodes.HIGH_WATER_MARK, channelConfig.highWaterMark, rsslReactorErrorInfo) != ReactorReturnCodes.SUCCESS)
	                {
	                	if (_baseImpl.loggerClient().isErrorEnabled())
	    	        	{
		    	        	StringBuilder temp = _baseImpl.strBuilder();
							
		    	        	temp.append("Failed to set high water mark on channel ")
								.append(chnlInfo.name()).append(OmmLoggerClient.CR)
								.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR);
			    	        	if (rsslReactorChannel != null && rsslReactorChannel.channel() != null)
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
    	        
    	        if (_baseImpl.loggerClient().isTraceEnabled())
    			{
    	        	StringBuilder temp = _baseImpl.strBuilder();
    	        	temp.append("Received FD Change event on channel ")
						.append(chnlInfo.name()).append(OmmLoggerClient.CR)
						.append("Instance Name ").append(_baseImpl.instanceName());
    	        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
    			}

    			chnlInfo.rsslReactorChannel(event.reactorChannel());

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
    			
    			if ( _bInitialChannelReadyEventReceived )
    			{
    				_baseImpl.loginCallbackClient().processChannelEvent(event);
    			}
    			else
    				_bInitialChannelReadyEventReceived = true;
    			
    			return ReactorCallbackReturnCodes.SUCCESS;
    		}
    		case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
    		{
                try
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(_baseImpl.selector());
                    if (key != null)
                    	key.cancel();
                }
                catch (Exception e) { }
    			
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
                
        		_baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_DOWN);
        		
         	   _baseImpl.processChannelEvent(event);
        	   
         	   _baseImpl.loginCallbackClient().processChannelEvent(event);
            	
            	return ReactorCallbackReturnCodes.SUCCESS;
    		}
    		case ReactorChannelEventTypes.CHANNEL_DOWN:
            {
        	   try
               {
                   SelectionKey key = rsslReactorChannel.selectableChannel().keyFor(_baseImpl.selector());
                   if (key != null)
                      	key.cancel();
               }
               catch (Exception e) {}

        	   if (_baseImpl.loggerClient().isErrorEnabled())
        	   {
        		    ReactorErrorInfo errorInfo = event.errorInfo();
					StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("Received ChannelDown event on channel ")
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
	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
        	   }

        	   _baseImpl.ommImplState(OmmImplState.RSSLCHANNEL_DOWN);
        	   
        	   _baseImpl.processChannelEvent(event);
        	   
        	   _baseImpl.loginCallbackClient().processChannelEvent(event);

        	   _baseImpl.closeRsslChannel(event.reactorChannel());
			
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
	
	private String channelParametersToString(ActiveConfig activeConfig,  ChannelConfig channelCfg )
	{
		boolean bValidChType = true;
		StringBuilder cfgParameters = new StringBuilder(512);
		String compType;
		String strConnectionType;
		switch (channelCfg.compressionType)
		{
		case com.thomsonreuters.upa.transport.CompressionTypes.ZLIB:
			{
				compType = "ZLib";
				break;
			}
		case com.thomsonreuters.upa.transport.CompressionTypes.LZ4:
			{
				compType = "LZ4";
				break;
			}
		case com.thomsonreuters.upa.transport.CompressionTypes.NONE:
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
		case com.thomsonreuters.upa.transport.ConnectionTypes.SOCKET:
			{
				SocketChannelConfig tempChannelCfg = (SocketChannelConfig) channelCfg;
				strConnectionType = "SOCKET";
				cfgParameters.append( "hostName " ).append( tempChannelCfg.hostName ).append( OmmLoggerClient.CR )
				.append( "port " ).append( tempChannelCfg.serviceName ).append( OmmLoggerClient.CR )
				.append( "CompressionType " ).append( compType ).append( OmmLoggerClient.CR )
				.append( "tcpNodelay " ).append( ( tempChannelCfg.tcpNodelay ? "true" : "false" ) ).append( OmmLoggerClient.CR );

				break;
			}
		case com.thomsonreuters.upa.transport.ConnectionTypes.HTTP:
			{
				HttpChannelConfig tempChannelCfg = (HttpChannelConfig) channelCfg;
				strConnectionType = "HTTP";
				cfgParameters.append( "hostName " ).append( tempChannelCfg.hostName ).append( OmmLoggerClient.CR )
				.append( "port " ).append( tempChannelCfg.serviceName ).append( OmmLoggerClient.CR )
				.append( "CompressionType " ).append( compType ).append( OmmLoggerClient.CR )
				.append( "tcpNodelay " ).append( ( tempChannelCfg.tcpNodelay ? "true" : "false" ) ).append( OmmLoggerClient.CR )
				.append( "ObjectName " ).append( tempChannelCfg.objectName ).append( OmmLoggerClient.CR );
				break;
			}
		case com.thomsonreuters.upa.transport.ConnectionTypes.ENCRYPTED:
			{
				EncryptedChannelConfig tempChannelCfg = (EncryptedChannelConfig) channelCfg;
				strConnectionType = "ENCRYPTED";
				cfgParameters.append( "hostName " ).append( tempChannelCfg.hostName ).append( OmmLoggerClient.CR )
				.append( "port " ).append( tempChannelCfg.serviceName ).append( OmmLoggerClient.CR )
				.append( "CompressionType " ).append( compType ).append( OmmLoggerClient.CR )
				.append( "tcpNodelay " ).append( ( tempChannelCfg.tcpNodelay ? "true" : "false" ) ).append( OmmLoggerClient.CR )
				.append( "ObjectName " ).append( tempChannelCfg.objectName ).append( OmmLoggerClient.CR );
				break;
			}
		default:
			{	
				strConnectionType = "Invalid ChannelType: ";
				strConnectionType += com.thomsonreuters.upa.transport.ConnectionTypes.toString(channelCfg.rsslConnectionType);
				bValidChType = false;
				break;
			}
		}
		
		StringBuilder tempBlder = _baseImpl.strBuilder();
		tempBlder.append( strConnectionType ).append( OmmLoggerClient.CR )
		.append( "Channel name " ).append( channelCfg.name ).append( OmmLoggerClient.CR )
		.append( "Instance Name " ).append( _baseImpl.instanceName() ).append( OmmLoggerClient.CR );

		if( bValidChType == true)
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
			.append( "connectionPingTimeout " ).append( channelCfg.connectionPingTimeout ).append( " msec" ).append( OmmLoggerClient.CR );
		}
		
		return tempBlder.toString();
	}
	
	private void initializeReactor()
	{
		ActiveConfig activeConfig = _baseImpl.activeConfig();
		List<ChannelConfig>	activeConfigChannelSet = activeConfig.channelConfigSet;
		int channelCfgSetLastIndex = activeConfigChannelSet.size() - 1;
		int rsslReactorConnListSize = _rsslReactorConnOptions.connectionList().size();
		int supportedConnectionTypeChannelCount = 0;
		String channelParams = "";
		String channelNames = "";
		StringBuilder temp = new StringBuilder();

		if( activeConfigChannelSet.size() > 1 )
			temp.append("Attempt to connect using the following list");
		else
			temp.append("Attempt to connect using ");
		
		StringBuilder errorStrUnsupportedConnectionType = new StringBuilder();		
		errorStrUnsupportedConnectionType.append( "Unsupported connection type. Passed in type is ");

		
		_rsslReactorConnOptions.reconnectAttemptLimit(activeConfig.reconnectAttemptLimit);
		_rsslReactorConnOptions.reconnectMinDelay(activeConfig.reconnectMinDelay);
		_rsslReactorConnOptions.reconnectMaxDelay(activeConfig.reconnectMaxDelay);
		com.thomsonreuters.upa.transport.ConnectOptions connectOptions = null;
		
		for(int i = 0; i < activeConfigChannelSet.size(); i++)
		{
			ChannelConfig channelConfig = activeConfigChannelSet.get(i);
			int connectionType = channelConfig.rsslConnectionType;
			
			if (connectionType == com.thomsonreuters.upa.transport.ConnectionTypes.SOCKET  ||
				connectionType == com.thomsonreuters.upa.transport.ConnectionTypes.HTTP ||
				connectionType == com.thomsonreuters.upa.transport.ConnectionTypes.ENCRYPTED)
			{
				ChannelInfo channelInfo = channelInfo(channelConfig.name, _rsslReactor);
				channelInfo._channelConfig = channelConfig;
				channelConfig.channelInfo = channelInfo;
				if( i < rsslReactorConnListSize )
				{
					connectOptions = _rsslReactorConnOptions.connectionList().get(i).connectOptions();
					connectOptions.userSpecObject(channelInfo);
					_rsslReactorConnOptions.connectionList().get(i).initTimeout(5);
				}
				else
				{
					
					ReactorConnectInfo newReactConnInfo = ReactorFactory.createReactorConnectInfo();
					newReactConnInfo.initTimeout(5);
					connectOptions = newReactConnInfo.connectOptions();
					connectOptions.userSpecObject(channelInfo);
					_rsslReactorConnOptions.connectionList().add(newReactConnInfo);					
					rsslReactorConnListSize = _rsslReactorConnOptions.connectionList().size();
				}
				
				
	
				connectOptions.majorVersion(com.thomsonreuters.upa.codec.Codec.majorVersion());
				connectOptions.minorVersion(com.thomsonreuters.upa.codec.Codec.minorVersion());
				connectOptions.protocolType(com.thomsonreuters.upa.codec.Codec.protocolType());
	
	
				connectOptions.compressionType(channelConfig.compressionType);
				connectOptions.connectionType(connectionType);
				connectOptions.pingTimeout(channelConfig.connectionPingTimeout/1000);
				connectOptions.guaranteedOutputBuffers(channelConfig.guaranteedOutputBuffers);
				connectOptions.sysRecvBufSize(channelConfig.sysRecvBufSize);
				connectOptions.sysSendBufSize(channelConfig.sysSendBufSize);
				connectOptions.numInputBuffers(channelConfig.numInputBuffers);
				connectOptions.componentVersion(_productVersion);
	
				switch (connectOptions.connectionType())
				{
				case com.thomsonreuters.upa.transport.ConnectionTypes.SOCKET:
					{
						connectOptions.unifiedNetworkInfo().address(((SocketChannelConfig) channelConfig).hostName);
						try
						{
						connectOptions.unifiedNetworkInfo().serviceName(((SocketChannelConfig) channelConfig).serviceName);
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
						connectOptions.tcpOpts().tcpNoDelay(((SocketChannelConfig) channelConfig).tcpNodelay);
						connectOptions.unifiedNetworkInfo().interfaceName(((SocketChannelConfig) channelConfig).interfaceName);
						connectOptions.unifiedNetworkInfo().unicastServiceName("");
					break;
					}
				case com.thomsonreuters.upa.transport.ConnectionTypes.ENCRYPTED:
					{
						connectOptions.unifiedNetworkInfo().address(((EncryptedChannelConfig) channelConfig).hostName);
						connectOptions.unifiedNetworkInfo().serviceName(((EncryptedChannelConfig) channelConfig).serviceName);
						connectOptions.tcpOpts().tcpNoDelay(((EncryptedChannelConfig) channelConfig).tcpNodelay);
						connectOptions.tunnelingInfo().objectName(((EncryptedChannelConfig) channelConfig).objectName);
						connectOptions.tunnelingInfo().tunnelingType("encrypted"); 
						connectOptions.unifiedNetworkInfo().interfaceName(((EncryptedChannelConfig) channelConfig).interfaceName);
						connectOptions.unifiedNetworkInfo().unicastServiceName("");
						encryptedConfiguration(connectOptions);
					break;
					}
				case com.thomsonreuters.upa.transport.ConnectionTypes.HTTP:
					{
						connectOptions.unifiedNetworkInfo().address(((HttpChannelConfig) channelConfig).hostName);
						connectOptions.unifiedNetworkInfo().serviceName(((HttpChannelConfig) channelConfig).serviceName);
						connectOptions.tcpOpts().tcpNoDelay(((HttpChannelConfig) channelConfig).tcpNodelay);
						connectOptions.tunnelingInfo().objectName(((HttpChannelConfig) channelConfig).objectName);
						connectOptions.tunnelingInfo().tunnelingType("http"); 
						connectOptions.unifiedNetworkInfo().interfaceName(((HttpChannelConfig) channelConfig).interfaceName);
						connectOptions.unifiedNetworkInfo().unicastServiceName("");
						httpConfiguration(connectOptions);
					break;
					}
				default :
					break;
				}
	
				connectOptions.unifiedNetworkInfo().interfaceName( channelConfig.interfaceName);
				connectOptions.unifiedNetworkInfo().unicastServiceName("");
				channelNames.concat(channelConfig.name);

				if (_baseImpl.loggerClient().isTraceEnabled())
				{
					channelParams = channelParametersToString( activeConfig, activeConfigChannelSet.get( i ) );
					temp.append( OmmLoggerClient.CR ).append( i + 1 ).append( "] " ).append( channelParams );
					if ( i == ( channelCfgSetLastIndex ) )				
						_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
				}	
	
				_channelList.add(channelInfo);
				supportedConnectionTypeChannelCount++;
			}
			else
			{
				errorStrUnsupportedConnectionType.append( ConnectionTypes.toString(channelConfig.rsslConnectionType)).append( " for " )
				.append( activeConfigChannelSet.get(i).name );
				if ( i < channelCfgSetLastIndex )
					errorStrUnsupportedConnectionType.append( ", " );				
			}
		}
		
		if( supportedConnectionTypeChannelCount > 0 )
		{
			ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
			if (ReactorReturnCodes.SUCCESS > _rsslReactor.connect(_rsslReactorConnOptions, (ReactorRole)_rsslReactorRole, rsslErrorInfo))
			{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
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
					ChannelInfo channelInfo = (ChannelInfo) _rsslReactorConnOptions.connectionList().get(i).connectOptions().userSpecObject();
					if( channelInfo != null)
					{
						removeChannel( channelInfo );
					}
				}
				throw _baseImpl.ommIUExcept().message(tempErr.toString());
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

			 throw _baseImpl.ommIUExcept().message( errorStrUnsupportedConnectionType.toString() );
		}
	}
	
	void initializeConsumerRole(LoginRequest loginReq, DirectoryRequest dirReq)
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
		
		ConsumerWatchlistOptions watchlistOptions = consumerRole.watchlistOptions();
		watchlistOptions.channelOpenCallback(this);
		watchlistOptions.enableWatchlist(true);
		watchlistOptions.itemCountHint(_baseImpl.activeConfig().itemCountHint);
		watchlistOptions.obeyOpenWindow(_baseImpl.activeConfig().obeyOpenWindow > 0 ? true : false);
		watchlistOptions.postAckTimeout(_baseImpl.activeConfig().postAckTimeout);
		watchlistOptions.requestTimeout(_baseImpl.activeConfig().requestTimeout);
		watchlistOptions.maxOutstandingPosts(_baseImpl.activeConfig().maxOutstandingPosts);
		
		_rsslReactorRole = consumerRole;
		
		initializeReactor();
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

	private void httpConfiguration(com.thomsonreuters.upa.transport.ConnectOptions rsslOptions)
    {    	
	}    
	 
	private void encryptedConfiguration(com.thomsonreuters.upa.transport.ConnectOptions rsslOptions)
	{
	}

	private ChannelInfo channelInfo(String name, Reactor rsslReactor)
	{
		if (_channelPool.isEmpty())
			return new ChannelInfo(name, rsslReactor);
		else 
			return (_channelPool.get(0).reset(name, rsslReactor));
	}
	
	void removeChannel(ChannelInfo chanInfo)
	{
		if (chanInfo != null)
		{
			_baseImpl.loginCallbackClient().removeChannelInfo(chanInfo.rsslReactorChannel());
			_channelList.remove( chanInfo );
			_channelPool.add( chanInfo );
		}
	}

	void closeChannels()
	{
		for (int index = _channelList.size() -1; index >= 0; index--)
			_baseImpl.closeRsslChannel(_channelList.get(index).rsslReactorChannel());
	}

	List<ChannelInfo>  channelList()
	{
		return _channelList;
	}
}
