///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.ema.access.ConfigReader.XMLnode;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.DirectoryServiceStore.ServiceIdInteger;
import com.thomsonreuters.ema.access.OmmException.ExceptionType;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FilterEntryActions;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.WriteFlags;
import com.thomsonreuters.upa.transport.WritePriorities;
import com.thomsonreuters.upa.valueadd.common.VaNode;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventTypes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;

public class OmmNiProviderImpl extends OmmBaseImpl<OmmProviderClient> implements OmmProvider {
	
	private OmmProviderErrorClient _providerErrorClient = null;
	private OmmNiProviderActiveConfig _activeConfig = null;
	private HashMap<Long, StreamInfo> _handleToStreamInfo = new HashMap<>();
	private boolean _bIsStreamIdZeroRefreshSubmitted = false;
	private DirectoryServiceStore directoryServiceStore = new DirectoryServiceStore(_objManager);
	private DecodeIterator userStoreDecodeIt = null;
	HashMap<String, UInt> _deletedServicesNameAndIdTable = new HashMap<>();
	
	OmmNiProviderImpl(OmmProviderConfig config)
	{
		super();
		_activeConfig = new OmmNiProviderActiveConfig();
		super.initialize(_activeConfig, (OmmNiProviderConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);
		if (_activeConfig.channelConfig.rsslConnectionType == ConnectionTypes.SOCKET &&
				((SocketChannelConfig)_activeConfig.channelConfig).directWrite)
			_rsslSubmitOptions.writeArgs().flags( _rsslSubmitOptions.writeArgs().flags() |  WriteFlags.DIRECT_SOCKET_WRITE);
	}

	OmmNiProviderImpl(OmmProviderConfig config, OmmProviderErrorClient client)
	{
		super();
		_activeConfig = new OmmNiProviderActiveConfig();
		super.initialize(_activeConfig, (OmmNiProviderConfigImpl)config);
		_providerErrorClient = client;
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);
		if (_activeConfig.channelConfig.rsslConnectionType == ConnectionTypes.SOCKET &&
				((SocketChannelConfig)_activeConfig.channelConfig).directWrite)
			_rsslSubmitOptions.writeArgs().flags( _rsslSubmitOptions.writeArgs().flags() |  WriteFlags.DIRECT_SOCKET_WRITE);
	}
	
	@Override
	public void uninitialize()
	{
		super.uninitialize();
	}

	@Override
	public String providerName() {
		return _activeConfig.instanceName;
	}
	
	@Override
	public long registerClient(ReqMsg reqMsg, OmmProviderClient client)
	{
		return registerClient(reqMsg, client, null);
	}
	
	@Override
	public long registerClient(ReqMsg reqMsg, OmmProviderClient client, Object closure)
	{
		userLock().lock();
		
		long handle = super.registerClient(reqMsg, client, closure);
		
		if (handle != 0)
		{
			StreamInfo streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
	    	if (streamInfo == null)
	    	{
	    		streamInfo = new StreamInfo(_itemCallbackClient.getItem(handle).streamId());
	    		_objManager._streamInfoPool.updatePool(streamInfo);
	    	}
	    	else
	    	{
	    		streamInfo.clear();
	    		streamInfo.set(_itemCallbackClient.getItem(handle).streamId());
	    	}
	    	
	    	_handleToStreamInfo.put(handle, streamInfo);
		}
		
		userLock().unlock();
		return handle;
	}
	
	@Override
	public long dispatch(long timeOut)
	{
		return super.dispatch(timeOut);
	}
	
	@Override
	public void unregister(long handle)
	{
		userLock().lock();
		
		StreamInfo streamInfo = _handleToStreamInfo.get(handle);
		
		if ( streamInfo == null )
		{
			userLock().unlock();
			return;
		}
		
		if ( streamInfo.streamId() < 0 )
		{
			userLock().unlock();
			handleInvalidHandle(handle, "Attempt to unregister a handle that was not registered.");
			return;
		}
		
		_handleToStreamInfo.remove(handle);
		
		super.unregister(handle);
		
		userLock().unlock();
	}

	@Override
	public void submit(RefreshMsg refreshMsg, long handle)
	{
		boolean bHandleAdded = false;
		StreamInfo streamInfo;
		
		userLock().lock();
		
		if ( _channelCallbackClient == null )
		{
			userLock().unlock();
			return;
		}
		
		ChannelInfo channel = _channelCallbackClient.channelList().get(0);
		
		RefreshMsgImpl refreshMsgImpl = (RefreshMsgImpl)refreshMsg;
		
		if ( refreshMsgImpl.domainType() == EmaRdm.MMT_DIRECTORY)
		{
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace(formatLogMessage(instanceName() , strBuilder().append("Received RefreshMsg with SourceDirectory domain; Handle = ")
						.append(handle).append(", user assigned streamId = ").append(refreshMsgImpl.streamId()).append(".").toString(), Severity.TRACE));
			}
		
			if ( refreshMsgImpl.rsslMsg().containerType() != com.thomsonreuters.upa.codec.DataTypes.MAP )
			{
				userLock().unlock();
				handleInvalidUsage(strBuilder().append("Attempt to submit RefreshMsg with SourceDirectory domain using container with wrong data type. Expected container data type is Map. Passed in is ")
						.append(  DataType.asString(Utilities.toEmaDataType[refreshMsgImpl.payload().dataType()])).toString());
			}
			
			Buffer outputBuffer = CodecFactory.createBuffer();
			
			if ( !decodeSourceDirectory(refreshMsgImpl._rsslMsg, outputBuffer, strBuilder()) )
			{
				userLock().unlock();
				handleInvalidUsage(_strBuilder.toString());
				return;
			}
			
			int flags = refreshMsgImpl._rsslMsg.flags();
			flags &= ~RefreshMsgFlags.SOLICITED;
			refreshMsgImpl._rsslMsg.flags(flags);
			
			if ( _activeConfig.mergeSourceDirectoryStreams)
			{
				refreshMsgImpl._rsslMsg.streamId(0);
			}
			else
			{
				if (_handleToStreamInfo.containsKey(handle))
				{
					streamInfo = _handleToStreamInfo.get(handle);
					refreshMsgImpl._rsslMsg.streamId(streamInfo.streamId());
				}
				else
				{
					streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
			    	if (streamInfo == null)
			    	{
			    		streamInfo = new StreamInfo(channel.nextProviderStreamId());
			    		_objManager._streamInfoPool.updatePool(streamInfo);
			    	}
			    	else
			    	{
			    		streamInfo.clear();
			    		streamInfo.set(channel.nextProviderStreamId());
			    	}
			    	
			    	refreshMsgImpl._rsslMsg.streamId(streamInfo.streamId());
					_handleToStreamInfo.put(handle, streamInfo);
					
					bHandleAdded = true;
				}
			}
		}
		else
		{
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace(formatLogMessage(instanceName() , strBuilder().append("Received RefreshMsg with market domain; Handle = ")
					.append(handle).append(", user assigned streamId = ").append(refreshMsgImpl.streamId()).append(".").toString(), Severity.TRACE));
			}
			
			streamInfo = _handleToStreamInfo.get(handle);
			
			if ( streamInfo != null )
			{
				refreshMsgImpl._rsslMsg.streamId(streamInfo.streamId());
				
				if ( ( refreshMsgImpl._rsslMsg.flags() & com.thomsonreuters.upa.codec.RefreshMsgFlags.HAS_MSG_KEY) != 0 )
				{
					refreshMsgImpl._rsslMsg.msgKey().serviceId(streamInfo.serviceId());
					refreshMsgImpl._rsslMsg.msgKey().applyHasServiceId();
				}
			}
			else if ( refreshMsgImpl.hasServiceName())
			{
				String serviceName = refreshMsgImpl.serviceName();
				
				ServiceIdInteger serviceId = directoryServiceStore.serviceId(serviceName);
				
				if ( serviceId == null )
				{
					userLock().unlock();
					
					strBuilder().append("Attempt to submit initial RefreshMsg with service name of ")
					.append(serviceName).append(" that was not included in the SourceDirectory. Dropping this RefreshMsg.");
					handleInvalidUsage(_strBuilder.toString());
					return;
				}
				
				refreshMsgImpl._rsslMsg.msgKey().serviceId(serviceId.value());
				refreshMsgImpl._rsslMsg.msgKey().applyHasServiceId();
				
				int flags = refreshMsgImpl.rsslMsg().flags();
				flags &= ~RefreshMsgFlags.SOLICITED;
				refreshMsgImpl.rsslMsg().flags(flags);
				
				streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
		    	if (streamInfo == null)
		    	{
		    		streamInfo = new StreamInfo(channel.nextProviderStreamId(),serviceId.value());
		    		_objManager._streamInfoPool.updatePool(streamInfo);
		    	}
		    	else
		    	{
		    		streamInfo.clear();
		    		streamInfo.set(channel.nextProviderStreamId(),serviceId.value());
		    	}
		    	
		    	refreshMsgImpl._rsslMsg.streamId(streamInfo.streamId());
		    	_handleToStreamInfo.put(handle, streamInfo);
				
				bHandleAdded = true;
				
			}
			else if ( refreshMsgImpl.hasServiceId())
			{
				int serviceId = refreshMsgImpl.serviceId();
				String serviceName = directoryServiceStore.serviceName(serviceId);
				
				if ( serviceName == null )
				{
					userLock().unlock();
					strBuilder().append("Attempt to submit initial RefreshMsg with service id of ")
					.append(serviceId).append(" that was not included in the SourceDirectory. Dropping this RefreshMsg.");
					handleInvalidUsage(_strBuilder.toString());
				}
				
				int flags = refreshMsgImpl.rsslMsg().flags();
				flags &= ~RefreshMsgFlags.SOLICITED;
				refreshMsgImpl.rsslMsg().flags(flags);
				
				streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
		    	if (streamInfo == null)
		    	{
		    		streamInfo = new StreamInfo(channel.nextProviderStreamId(),serviceId);
		    		_objManager._streamInfoPool.updatePool(streamInfo);
		    	}
		    	else
		    	{
		    		streamInfo.clear();
		    		streamInfo.set(channel.nextProviderStreamId(),serviceId);
		    	}
		    	
		    	refreshMsgImpl._rsslMsg.streamId(streamInfo.streamId());
		    	_handleToStreamInfo.put(handle, streamInfo);
				
		    	bHandleAdded = true;
			}
			else
			{
				userLock().unlock();
				handleInvalidUsage("Attempt to submit initial RefreshMsg without service name or id. Dropping this RefreshMsg.");
				return;
			}
		}
		
		_rsslErrorInfo.clear();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = channel.rsslReactorChannel().submit(refreshMsgImpl._rsslMsg, _rsslSubmitOptions, _rsslErrorInfo)))
	    {
			if (bHandleAdded)
			{
				_handleToStreamInfo.remove(handle);
				channel.returnStreamId(refreshMsgImpl._rsslMsg.streamId());
			}
			
			if (loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
				
	        	strBuilder().append("Internal error: rsslChannel.submit() failed in OmmNiProviderImpl.submit(RefreshMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(_rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	loggerClient().error(formatLogMessage(instanceName() , _strBuilder.toString(), Severity.ERROR));
        	}
			
			userLock().unlock();
			strBuilder().append("Failed to submit RefreshMsg. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(_rsslErrorInfo.error().text());
			
			handleInvalidUsage(_strBuilder.toString());
			return;
	    }
		
		if (refreshMsgImpl.state().streamState() == OmmState.StreamState.CLOSED || 
				refreshMsgImpl.state().streamState() == OmmState.StreamState.CLOSED_RECOVER || 
				refreshMsgImpl.state().streamState() == OmmState.StreamState.CLOSED_REDIRECTED)
		{
			_handleToStreamInfo.remove(handle);
			channel.returnStreamId(refreshMsgImpl._rsslMsg.streamId());
		}
		
		if (refreshMsgImpl._rsslMsg.streamId() == 0)
		{
			_bIsStreamIdZeroRefreshSubmitted = true;
		}
		
		userLock().unlock();
	}

	@Override
	public void submit(UpdateMsg updateMsg, long handle)
	{
		boolean bHandleAdded = false;
		StreamInfo streamInfo;
		
		userLock().lock();
		
		if ( _channelCallbackClient == null )
		{
			userLock().unlock();
			return;
		}
		
		ChannelInfo channel = _channelCallbackClient.channelList().get(0);
		
		UpdateMsgImpl updateMsgImpl = (UpdateMsgImpl)updateMsg;
		
		if ( updateMsg.domainType() == EmaRdm.MMT_DIRECTORY)
		{
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace(formatLogMessage(instanceName() , strBuilder().append("Received UpdateMsg with SourceDirectory domain; Handle = ")
						.append(handle).append(", user assigned streamId = ").append(updateMsgImpl.streamId()).append(".").toString(), Severity.TRACE));
			}
			
			if ( updateMsgImpl.rsslMsg().containerType() != com.thomsonreuters.upa.codec.DataTypes.MAP )
			{
				userLock().unlock();
				handleInvalidUsage(strBuilder().append("Attempt to submit UpdateMsg with SourceDirectory domain using container with wrong data type. Expected container data type is Map. Passed in is ")
						.append(  DataType.asString(Utilities.toEmaDataType[updateMsgImpl.payload().dataType()])).toString());
				return;
			}
			
			Buffer outputBuffer = CodecFactory.createBuffer();
			
			if ( !decodeSourceDirectory(updateMsgImpl._rsslMsg, outputBuffer, strBuilder()) )
			{
				userLock().unlock();
				handleInvalidUsage(_strBuilder.toString());
				return;
			}
			
			if ( _activeConfig.mergeSourceDirectoryStreams)
			{
				if ( _activeConfig.refreshFirstRequired && !_bIsStreamIdZeroRefreshSubmitted )
				{
					userLock().unlock();
					strBuilder().append("Attempt to submit UpdateMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = ")
					.append(handle).append(".");
					handleInvalidHandle(handle, _strBuilder.toString());
					return;
				}
				
				updateMsgImpl._rsslMsg.streamId(0);
			}
			else
			{
				if (_handleToStreamInfo.containsKey(handle))
				{
					streamInfo = _handleToStreamInfo.get(handle);
					updateMsgImpl._rsslMsg.streamId(streamInfo.streamId());
				}
				else
				{
					streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
			    	if (streamInfo == null)
			    	{
			    		streamInfo = new StreamInfo(channel.nextProviderStreamId());
			    		_objManager._streamInfoPool.updatePool(streamInfo);
			    	}
			    	else
			    	{
			    		streamInfo.clear();
			    		streamInfo.set(channel.nextProviderStreamId());
			    	}
			    	
			    	updateMsgImpl._rsslMsg.streamId(streamInfo.streamId());
					_handleToStreamInfo.put(handle, streamInfo);
					
					bHandleAdded = true;
				}
			}
		}
		else
		{
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace(formatLogMessage(instanceName() , strBuilder().append("Received UpdateMsg with market domain; Handle = ")
						.append(handle).append(", user assigned streamId = ").append(updateMsgImpl.streamId()).append(".").toString(), Severity.TRACE));
			}
			
			streamInfo = _handleToStreamInfo.get(handle);
			
			if ( streamInfo != null )
			{
				updateMsgImpl._rsslMsg.streamId(streamInfo.streamId());
				
				if ( ( updateMsgImpl._rsslMsg.flags() & com.thomsonreuters.upa.codec.UpdateMsgFlags.HAS_MSG_KEY) != 0 )
				{
					updateMsgImpl._rsslMsg.msgKey().serviceId(streamInfo.serviceId());
					updateMsgImpl._rsslMsg.msgKey().applyHasServiceId();
				}
			}
			else if ( updateMsgImpl.hasServiceName())
			{
				String serviceName = updateMsgImpl.serviceName();
				
				ServiceIdInteger serviceId = directoryServiceStore.serviceId(serviceName);
				
				if ( serviceId == null )
				{
					userLock().unlock();
					
					strBuilder().append("Attempt to submit initial UpdateMsg with service name of ")
					.append(serviceName).append(" that was not included in the SourceDirectory. Dropping this UpdateMsg.");
					handleInvalidUsage(_strBuilder.toString());
					return;
				}
				
				updateMsgImpl._rsslMsg.msgKey().serviceId(serviceId.value());
				updateMsgImpl._rsslMsg.msgKey().applyHasServiceId();
				
				streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
		    	if (streamInfo == null)
		    	{
		    		streamInfo = new StreamInfo(channel.nextProviderStreamId(),serviceId.value());
		    		_objManager._streamInfoPool.updatePool(streamInfo);
		    	}
		    	else
		    	{
		    		streamInfo.clear();
		    		streamInfo.set(channel.nextProviderStreamId(),serviceId.value());
		    	}
		    	
		    	updateMsgImpl._rsslMsg.streamId(streamInfo.streamId());
		    	_handleToStreamInfo.put(handle, streamInfo);
				
				bHandleAdded = true;
				
			}
			else if ( updateMsgImpl.hasServiceId())
			{
				int serviceId = updateMsgImpl.serviceId();
				String serviceName = directoryServiceStore.serviceName(serviceId);
				
				if ( serviceName == null )
				{
					userLock().unlock();
					strBuilder().append("Attempt to submit initial UpdateMsg with service id of ")
					.append(serviceId).append(" that was not included in the SourceDirectory. Dropping this UpdateMsg.");
					handleInvalidUsage(_strBuilder.toString());
					return;
				}
				
				streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
		    	if (streamInfo == null)
		    	{
		    		streamInfo = new StreamInfo(channel.nextProviderStreamId(),serviceId);
		    		_objManager._streamInfoPool.updatePool(streamInfo);
		    	}
		    	else
		    	{
		    		streamInfo.clear();
		    		streamInfo.set(channel.nextProviderStreamId(),serviceId);
		    	}
		    	
		    	updateMsgImpl._rsslMsg.streamId(streamInfo.streamId());
		    	_handleToStreamInfo.put(handle, streamInfo);
				
		    	bHandleAdded = true;
			}
			else
			{
				userLock().unlock();
				handleInvalidUsage("Attempt to submit initial UpdateMsg without service name or id. Dropping this UpdateMsg.");
				return;
			}
		}
		
		_rsslErrorInfo.clear();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = channel.rsslReactorChannel().submit(updateMsgImpl._rsslMsg, _rsslSubmitOptions, _rsslErrorInfo)))
	    {
			if (bHandleAdded)
			{
				_handleToStreamInfo.remove(handle);
				channel.returnStreamId(updateMsgImpl._rsslMsg.streamId());
			}
			
			if (loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
				
	        	strBuilder().append("Internal error: rsslChannel.submit() failed in OmmNiProviderImpl.submit(UpdateMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(_rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	loggerClient().error(formatLogMessage(instanceName() , _strBuilder.toString(), Severity.ERROR));
        	}
			
			userLock().unlock();
			strBuilder().append("Failed to submit UpdateMsg. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(_rsslErrorInfo.error().text());
			
			handleInvalidUsage(_strBuilder.toString());
			return;
	    }
		
		userLock().unlock();
	}

	@Override
	public void submit(StatusMsg statusMsg, long handle)
	{
		boolean bHandleAdded = false;
		StreamInfo streamInfo;
		
        userLock().lock();
		
		if ( _channelCallbackClient == null )
		{
			userLock().unlock();
			return;
		}
		
		ChannelInfo channel = _channelCallbackClient.channelList().get(0);
		
		StatusMsgImpl statusMsgImpl = (StatusMsgImpl)statusMsg;
		
		if ( statusMsg.domainType() == EmaRdm.MMT_DIRECTORY)
		{
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace(formatLogMessage(instanceName() , strBuilder().append("Received StatusMsg with SourceDirectory domain; Handle = ")
					.append(handle).append(", user assigned streamId = ").append(statusMsgImpl.streamId()).append(".").toString(), Severity.TRACE));
			}
			
			if ( statusMsgImpl.rsslMsg().containerType() != com.thomsonreuters.upa.codec.DataTypes.MAP )
			{
				userLock().unlock();
				handleInvalidUsage(strBuilder().append("Attempt to submit StatusMsg with SourceDirectory domain using container with wrong data type. Expected container data type is Map. Passed in is ")
						.append(  DataType.asString(Utilities.toEmaDataType[statusMsgImpl.payload().dataType()])).toString());
			}
			
			Buffer outputBuffer = CodecFactory.createBuffer();
			
			if ( !decodeSourceDirectory(statusMsgImpl._rsslMsg, outputBuffer, strBuilder()) )
			{
				userLock().unlock();
				handleInvalidUsage(_strBuilder.toString());
				return;
			}
			
			if ( _activeConfig.mergeSourceDirectoryStreams)
			{
				if ( _activeConfig.refreshFirstRequired && !_bIsStreamIdZeroRefreshSubmitted )
				{
					userLock().unlock();
					strBuilder().append("Attempt to submit StatusMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = ")
					.append(handle).append(".");
					handleInvalidHandle(handle, _strBuilder.toString());
					return;
				}
				
				statusMsgImpl._rsslMsg.streamId(0);
			}
			else
			{
				if (_handleToStreamInfo.containsKey(handle))
				{
					streamInfo = _handleToStreamInfo.get(handle);
					statusMsgImpl._rsslMsg.streamId(streamInfo.streamId());
				}
				else
				{
					streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
			    	if (streamInfo == null)
			    	{
			    		streamInfo = new StreamInfo(channel.nextProviderStreamId());
			    		_objManager._streamInfoPool.updatePool(streamInfo);
			    	}
			    	else
			    	{
			    		streamInfo.clear();
			    		streamInfo.set(channel.nextProviderStreamId());
			    	}
			    	
			    	statusMsgImpl._rsslMsg.streamId(streamInfo.streamId());
					_handleToStreamInfo.put(handle, streamInfo);
					
					bHandleAdded = true;
				}
			}
		}
		else
		{
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace(formatLogMessage(instanceName() , strBuilder().append("Received StatusMsg with market domain; Handle = ")
						.append(handle).append(", user assigned streamId = ").append(statusMsgImpl.streamId()).append(".").toString(), Severity.TRACE));
			}
			
			streamInfo = _handleToStreamInfo.get(handle);
			
			if ( streamInfo != null )
			{
				statusMsgImpl._rsslMsg.streamId(streamInfo.streamId());
				
				if ( ( statusMsgImpl._rsslMsg.flags() & com.thomsonreuters.upa.codec.RefreshMsgFlags.HAS_MSG_KEY) != 0 )
				{
					statusMsgImpl._rsslMsg.msgKey().serviceId(streamInfo.serviceId());
					statusMsgImpl._rsslMsg.msgKey().applyHasServiceId();
				}
			}
			else if ( statusMsgImpl.hasServiceName())
			{
				String serviceName = statusMsgImpl.serviceName();
				
				ServiceIdInteger serviceId = directoryServiceStore.serviceId(serviceName);
				
				if ( serviceId == null )
				{
					userLock().unlock();
					strBuilder().append("Attempt to submit initial StatusMsg with service name of ")
					.append(serviceName).append(" that was not included in the SourceDirectory. Dropping this StatusMsg.");
					handleInvalidUsage(_strBuilder.toString());
					return;
				}
				
				statusMsgImpl._rsslMsg.msgKey().serviceId(serviceId.value());
				statusMsgImpl._rsslMsg.msgKey().applyHasServiceId();
				
				streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
		    	if (streamInfo == null)
		    	{
		    		streamInfo = new StreamInfo(channel.nextProviderStreamId(),serviceId.value());
		    		_objManager._streamInfoPool.updatePool(streamInfo);
		    	}
		    	else
		    	{
		    		streamInfo.clear();
		    		streamInfo.set(channel.nextProviderStreamId(),serviceId.value());
		    	}
		    	
		    	statusMsgImpl._rsslMsg.streamId(streamInfo.streamId());
		    	_handleToStreamInfo.put(handle, streamInfo);
				
				bHandleAdded = true;
				
			}
			else if ( statusMsgImpl.hasServiceId())
			{
				int serviceId = statusMsgImpl.serviceId();
				String serviceName = directoryServiceStore.serviceName(serviceId);
				
				if ( serviceName == null )
				{
					userLock().unlock();
					strBuilder().append("Attempt to submit initial StatusMsg with service id of ")
					.append(serviceId).append(" that was not included in the SourceDirectory. Dropping this StatusMsg.");
					handleInvalidUsage(_strBuilder.toString());
					return;
				}
				
				streamInfo = (StreamInfo)_objManager._streamInfoPool.poll();
		    	if (streamInfo == null)
		    	{
		    		streamInfo = new StreamInfo(channel.nextProviderStreamId(),serviceId);
		    		_objManager._streamInfoPool.updatePool(streamInfo);
		    	}
		    	else
		    	{
		    		streamInfo.clear();
		    		streamInfo.set(channel.nextProviderStreamId(),serviceId);
		    	}
		    	
		    	statusMsgImpl._rsslMsg.streamId(streamInfo.streamId());
		    	_handleToStreamInfo.put(handle, streamInfo);
				
		    	bHandleAdded = true;
			}
			else
			{
				userLock().unlock();
				handleInvalidUsage("Attempt to submit initial StatusMsg without service name or id. Dropping this StatusMsg.");
				return;
			}
		}
		
		_rsslErrorInfo.clear();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = channel.rsslReactorChannel().submit(statusMsgImpl._rsslMsg, _rsslSubmitOptions, _rsslErrorInfo)))
	    {
			if (bHandleAdded)
			{
				_handleToStreamInfo.remove(handle);
				channel.returnStreamId(statusMsgImpl._rsslMsg.streamId());
			}
			
			if (loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
				
	        	strBuilder().append("Internal error: rsslChannel.submit() failed in OmmNiProviderImpl.submit(StatusMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(_rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	loggerClient().error(formatLogMessage(instanceName() , _strBuilder.toString(), Severity.ERROR));
        	}
			
			userLock().unlock();
			strBuilder().append("Failed to submit StatusMsg. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(_rsslErrorInfo.error().text());
			
			handleInvalidUsage(_strBuilder.toString());
			return;
	    }
		
		if (statusMsgImpl.state().streamState() == OmmState.StreamState.CLOSED || 
				statusMsgImpl.state().streamState() == OmmState.StreamState.CLOSED_RECOVER || 
						statusMsgImpl.state().streamState() == OmmState.StreamState.CLOSED_REDIRECTED)
		{
			_handleToStreamInfo.remove(handle);
			channel.returnStreamId(statusMsgImpl._rsslMsg.streamId());
		}
		
		userLock().unlock();
	}
	
	@Override
	public void submit(GenericMsg genericMsg, long handle)
	{
		userLock().lock();
			
		if ( _channelCallbackClient == null )
		{
			userLock().unlock();
			return;
		}
		
		ChannelInfo channel = _channelCallbackClient.channelList().get(0);
		
		if ( loggerClient().isTraceEnabled() )
		{
			loggerClient().trace(formatLogMessage(instanceName() , strBuilder().append("Received GenericMsg; Handle = ")
					.append(handle).append(", user assigned streamId = ").append(genericMsg.streamId()).append(".").toString(), Severity.TRACE));
		}
		
		StreamInfo streamInfo = _handleToStreamInfo.get(handle);
		
		if ( streamInfo != null )
		{
			((GenericMsgImpl)genericMsg).streamId(streamInfo.streamId());
		}
		else
		{
			userLock().unlock();
			strBuilder().append("Attempt to submit GenericMsg on stream that is not open yet. Handle = ")
			.append(handle).append(".");
			handleInvalidHandle(handle, _strBuilder.toString());
			return;
		}
		
		_rsslErrorInfo.clear();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = channel.rsslReactorChannel().submit(((GenericMsgImpl)genericMsg)._rsslMsg, _rsslSubmitOptions, _rsslErrorInfo)))
	    {
			if (loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
				
	        	strBuilder().append("Internal error: rsslChannel.submit() failed in OmmNiProviderImpl.submit(GenericMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(_rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	loggerClient().error(formatLogMessage(instanceName() , _strBuilder.toString(), Severity.ERROR));
        	}
			
			userLock().unlock();
			strBuilder().append("Failed to submit GenericMsg. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(_rsslErrorInfo.error().text());
				
			handleInvalidUsage(_strBuilder.toString());
			return;
	    }
		
		userLock().unlock();
	}

	@Override
	void notifyErrorClient(OmmException ommException) {
		switch (ommException.exceptionType())
		{
		case ExceptionType.OmmInvalidHandleException:
			_providerErrorClient.onInvalidHandle(((OmmInvalidHandleException) ommException).handle(), ommException.getMessage());
			break;
		case ExceptionType.OmmInvalidUsageException:
			_providerErrorClient.onInvalidUsage(ommException.getMessage());
			break;
		default:
			break;
		}
	}

	@Override
	String formatLogMessage(String clientName, String temp, int level) {
		strBuilder().append("loggerMsg\n").append("    ClientName: ").append(clientName).append("\n")
        .append("    Severity: ").append(OmmLoggerClient.loggerSeverityAsString(level)).append("\n")
        .append("    Text:    ").append(temp).append("\n").append("loggerMsgEnd\n\n");

		return _strBuilder.toString();
	}

	@Override
	String instanceName() 
	{
		return _activeConfig.instanceName;
	}

	@Override
	boolean hasErrorClient() {
		return _providerErrorClient != null ? true : false;
	}

	@Override
	void readCustomConfig(EmaConfigImpl config)
	{
		_activeConfig.dictionaryConfig = (DictionaryConfig) new DictionaryConfig(true);
		
		_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName = "RDMFieldDictionary";
		_activeConfig.dictionaryConfig.enumtypeDefFileName = "enumtype.def";
	
		_activeConfig.directoryAdminControl = ((OmmNiProviderConfigImpl)config).adminControlDirectory();
		
		if ( _activeConfig.directoryAdminControl == OmmNiProviderConfig.AdminControl.API_CONTROL)
		{
			_activeConfig.directoryConfig.directoryName = ((OmmNiProviderConfigImpl)config).directoryName(_activeConfig.configuredName);
			
			if ( _activeConfig.directoryConfig.directoryName == null || _activeConfig.directoryConfig.directoryName.isEmpty() )
				_activeConfig.directoryConfig.directoryName = (String)config.xmlConfig().getDefaultDirectoryName();
			
			if ( _activeConfig.directoryConfig.directoryName == null || _activeConfig.directoryConfig.directoryName.isEmpty() )
				_activeConfig.directoryConfig.directoryName = (String)config.xmlConfig().getFirstDirectory();
			
			if ( _activeConfig.directoryConfig.directoryName == null || _activeConfig.directoryConfig.directoryName.isEmpty() )
			{
				config.errorTracker().append("no configuration exists for ni provider directory [")
				.append(_activeConfig.instanceName).append("]. Will use directory defaults.").create(Severity.WARNING);
				
				useDefaultService(config);
			}
			else
			{
				XMLnode directoryNode = config.xmlConfig().getDirectory(_activeConfig.directoryConfig.directoryName);
				
				if (directoryNode != null)
				{
					int numberofservice = 0;
					Set<String> serviceNameSet = new LinkedHashSet<>();
					Set<Integer> serviceIdSet = new LinkedHashSet<>();
					List<Service> unspecifiedIdList = new ArrayList<>();
					boolean result = false;
					Service service = null;
					String serviceName = null;
					
					for(int i = 0; i < directoryNode.children().size() ; i++ )
					{
						XMLnode childNode = directoryNode.children().get(i);
						
						if( childNode != null && childNode.tagId() == ConfigManager.Service )
						{	
							if ( childNode.attributeList() != null )
							{
								serviceName = (String)childNode.attributeList().getValue(ConfigManager.ServiceName);
							}
							
							if ( serviceName != null && !serviceName.isEmpty() )
							{
								if( serviceNameSet.contains(serviceName))
								{
									config.errorTracker().append("service[").append(serviceName)
									.append("] is already specified by another service. Will drop this service.").create(Severity.ERROR);
									continue;
								}
								
								if ( ++numberofservice > ConfigManager.MAX_UINT16 )
								{
									config.errorTracker().append("Number of configured services is greater than allowed maximum(")
									.append(ConfigManager.MAX_UINT16).append("). Some services will be dropped.").create(Severity.ERROR);
									break;
								}
								
								service = DirectoryMsgFactory.createService();
								service.applyHasInfo();
								service.info().action(FilterEntryActions.SET);
								service.info().serviceName().data(serviceName);
								
								XMLnode infoFilterNode = childNode.getChild(ConfigManager.ServiceInfoFilter);
								
								if ( infoFilterNode != null)
								{
									result = readServiceInfoFilter(config, serviceIdSet, unspecifiedIdList, service, infoFilterNode);
								}
								
								XMLnode stateFilterNode = childNode.getChild(ConfigManager.ServiceStateFilter);
								
								if (result && stateFilterNode != null)
								{
									result = readServiceStateFilter(config, service,stateFilterNode);
								}
								
								if ( result )
								{
									serviceNameSet.add(serviceName);
									directoryServiceStore.addToMap(service);
									_activeConfig.directoryConfig.addService(service);
								}
							}
						}
					}
					
					if ( _activeConfig.directoryConfig.serviceList().size() == 0 )
					{
						config.errorTracker().append("specified ni provider directory [ [")
						.append(_activeConfig.directoryConfig.directoryName).append("] contains no services. Will use directory defaults").create(Severity.WARNING);
						
						useDefaultService(config);
					}
					
					if( unspecifiedIdList.size() > 0 )
					{
						int serviceId = 0;
						
						for(int index = 0; index < unspecifiedIdList.size(); ++index )
						{
							while(serviceIdSet.contains(serviceId))
							{
								++serviceId;
							}
							
							if( serviceId > ConfigManager.MAX_UINT16 )
							{
								config.errorTracker().append("EMA ran out of assignable service ids. Will drop rest of the services").create(Severity.ERROR);
								break;
							}
							
							unspecifiedIdList.get(index).serviceId(serviceId);
						}
					}
				}
				else
				{
					config.errorTracker().append("specified ni provider directory [ [")
					.append(_activeConfig.directoryConfig.directoryName).append("] contains no services. Will use directory defaults").create(Severity.WARNING);
					
					useDefaultService(config);
				}
			}
		}
		
		ConfigAttributes niProviderAttributes = getAttributes(config);
		
		if(niProviderAttributes != null)
		{
			ConfigElement element = (ConfigElement)niProviderAttributes.getElement(ConfigManager.NiProviderRefreshFirstRequired);
			
			if (element != null)
			{
				_activeConfig.refreshFirstRequired = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)niProviderAttributes.getElement(ConfigManager.NiProviderMergeSourceDirectoryStreams);
			
			if (element != null)
			{
				_activeConfig.mergeSourceDirectoryStreams = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)niProviderAttributes.getElement(ConfigManager.NiProviderRecoverUserSubmitSourceDirectory);
			
			if (element != null)
			{
				_activeConfig.recoverUserSubmitSourceDirectory = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)niProviderAttributes.getElement(ConfigManager.NiProviderRemoveItemsOnDisconnect);
			
			if (element != null)
			{
				_activeConfig.removeItemsOnDisconnect = element.intLongValue() > 0 ? true : false;
			}
		}
		
		if ( _activeConfig.recoverUserSubmitSourceDirectory )
		{
			userStoreDecodeIt = CodecFactory.createDecodeIterator();
		}
		
		// TODO: add handling for programmatic configuration
	}
	
	boolean readServiceInfoFilter(EmaConfigImpl config, Set<Integer> serviceIdSet, List<Service> unspecifiedIdList, Service service, XMLnode infoFilterNode)
	{
		ConfigAttributes infoAttributes =  infoFilterNode.attributeList();
		
		ConfigElement element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterServiceId);
		
		if (element != null)
		{
			int serviceId = element.intLongValue();
			
			if (serviceId > ConfigManager.MAX_UINT16)
			{
				config.errorTracker().append("service[").append(service.info().serviceName().toString())
				.append("] specifies out of range ServiceId (value of ").append(serviceId)
				.append("). Will drop this service.").create(Severity.ERROR);
				return false;
			}
			
			if (serviceIdSet.contains(serviceId))
			{
				config.errorTracker().append("service[").append(service.info().serviceName().toString())
				.append("] specifies the same ServiceId (value of ").append(serviceId)
				.append(") as already specified by another service. Will drop this service.").create(Severity.ERROR);
				return false;
			}
			
			service.serviceId(serviceId);
			serviceIdSet.add(serviceId);
		}
		else
		{
			unspecifiedIdList.add(service);
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterVendor);
		
		if (element != null)
		{
			service.info().applyHasVendor();
			service.info().vendor().data(element.asciiValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterIsSource);
		
		if (element != null)
		{			
			 service.info().applyHasIsSource();
		     service.info().isSource(element.intLongValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterSupportsQoSRange);
		
		if (element != null)
		{
			 service.info().applyHasSupportsQosRange();
		     service.info().supportsQosRange(element.intLongValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterItemList);
		
		if (element != null)
		{			
			service.info().applyHasItemList();
		    service.info().itemList().data(element.asciiValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterAcceptingConsumerStatus);
		
		if (element != null)
		{
		     service.info().applyHasAcceptingConsumerStatus();
		     service.info().acceptingConsumerStatus(element.intLongValue());
		}
		
		element = (ConfigElement) infoAttributes.getElement(ConfigManager.ServiceInfoFilterSupportsOutOfBandSnapshots);
		
		if (element != null)
		{
			service.info().applyHasSupportsOutOfBandSnapshots();
	        service.info().supportsOutOfBandSnapshots(element.intLongValue());
		}
		
		for(int i = 0; i < infoFilterNode.children().size() ; i++ )
		{
			XMLnode node = infoFilterNode.children().get(i);
			
			List<ConfigElement> configElementList;
			
			if ( node.tagId() == ConfigManager.ServiceInfoFilterCapabilities )
			{
				Integer domainTypeInt;
				
				configElementList =  node.attributeList().getConfigElementList(ConfigManager.ServiceInfoFilterCapabilitiesCapabilitiesEntry);
				
				for(int index = 0 ; index < configElementList.size(); ++index )
				{
					element = configElementList.get(index);
					
					if (element != null)
					{
						domainTypeInt = ConfigManager.convertDomainType(element.asciiValue());
						
						if ( domainTypeInt != null )
						{
							if ( domainTypeInt > ConfigManager.MAX_UINT16 )
							{
								config.errorTracker().append("specified service [")
								.append(service.info().serviceName().toString()).append("] contains out of range capability = ")
								.append(domainTypeInt).append(". Will drop this capability.").create(Severity.ERROR);
								continue;
							}
							
							service.info().capabilitiesList().add(domainTypeInt.longValue());
						}
						else
						{
							config.errorTracker().append("failed to read or convert a capability from the specified service [")
							.append(service.info().serviceName().toString())
							.append("]. Will drop this capability. Its value is = ").append(element.asciiValue()).create(Severity.ERROR);
						}
					}
				}
			}
			
		    readServiceInfoFilterDictionary(config, service, node, ConfigManager.ServiceInfoFilterDictionariesProvided, 
		    		ConfigManager.ServiceInfoFilterDictionariesProvidedDictionariesProvidedEntry);
		    
		    readServiceInfoFilterDictionary(config, service, node, ConfigManager.ServiceInfoFilterDictionariesUsed, 
		    		ConfigManager.ServiceInfoFilterDictionariesUsedDictionariesUsedEntry);
		    
		    if ( node.tagId() == ConfigManager.ServiceInfoFilterQoS )
		    {
		    	Long timeliness;
		    	Long rate;
		    
		    	if ( node.children().size() == 0 )
		    	{
		    		config.errorTracker().append("no configuration QoSEntry exists for service QoS [")
					.append(service.info().serviceName().toString())
					.append("|InfoFilter|QoS]. Will use default QoS.").create(Severity.WARNING);
		    		
		    		Qos qos = CodecFactory.createQos();
					Utilities.toRsslQos(OmmQos.Rate.TICK_BY_TICK, OmmQos.Timeliness.REALTIME, qos);
					service.info().applyHasQos();
					service.info().qosList().add(qos);
		    	}
		    	else
		    	{
			    	for(int index = 0; index < node.children().size() ; index++ )
					{
			    		timeliness = new Long(OmmQos.Timeliness.REALTIME);
				    	rate = new Long(OmmQos.Rate.TICK_BY_TICK);
			    		
						XMLnode childNode = node.children().get(index);
						
						ConfigAttributes qosAttributes = childNode.attributeList();
						
						element = (ConfigElement) qosAttributes.getElement(ConfigManager.ServiceInfoFilterQoSEntryTimeliness);
						
						if (element != null)
						{
							timeliness = ConfigManager.convertQosTimeliness(element.asciiValue());
							
							if( timeliness == null )
							{
								config.errorTracker().append("failed to read or convert a QoS Timeliness from the specified service [")
								.append(service.info().serviceName().toString())
								.append("|InfoFilter|QoS|QoSEntry|Timeliness]. Will use default Timeliness.")
								.append(" Suspect Timeliness value is ").append(element.asciiValue()).create(Severity.WARNING);
								
								timeliness = new Long(OmmQos.Timeliness.REALTIME);
							}
							else if ( timeliness > Integer.MAX_VALUE )
							{
								config.errorTracker().append("specified service QoS::Timeliness [")
								.append(service.info().serviceName().toString())
								.append("|InfoFilter|QoS|QoSEntry|Timeliness] is greater than allowed maximum. Will use maximum Timeliness.")
								.append(" Suspect Timeliness value is ").append(element.asciiValue()).create(Severity.WARNING);
								
								timeliness = new Long(OmmQos.Timeliness.INEXACT_DELAYED);
							}
						}
						else
						{
							config.errorTracker().append("no configuration exists for service QoS Timeliness [")
							.append(service.info().serviceName().toString())
							.append("|InfoFilter|QoS|QoSEntry|Timeliness]. Will use default Timeliness.").create(Severity.WARNING);
						}
						
						element = (ConfigElement) qosAttributes.getElement(ConfigManager.ServiceInfoFilterQoSEntryRate);
						
						if (element != null)
						{
							rate = ConfigManager.convertQosRate(element.asciiValue());
							
							if( rate == null )
							{
								config.errorTracker().append("failed to read or convert a QoS Rate from the specified service [")
								.append(service.info().serviceName().toString())
								.append("|InfoFilter|QoS|QoSEntry|Rate]. Will use default Rate.")
								.append(" Suspect Rate value is ").append(element.asciiValue()).create(Severity.WARNING);
								
								rate = new Long(OmmQos.Rate.TICK_BY_TICK);
							}
							else if ( rate > Integer.MAX_VALUE )
							{
								config.errorTracker().append("specified service QoS::Rate [")
								.append(service.info().serviceName().toString())
								.append("|InfoFilter|QoS|QoSEntry|Rate] is greater than allowed maximum. Will use maximum Rate.")
								.append(" Suspect Rate value is ").append(element.asciiValue()).create(Severity.WARNING);
								
								rate = new Long(OmmQos.Rate.JUST_IN_TIME_CONFLATED);
							}
						}
						else
						{
							config.errorTracker().append("no configuration exists for service QoS Rate [")
							.append(service.info().serviceName().toString())
							.append("|InfoFilter|QoS|QoSEntry|Timeliness]. Will use default Rate").create(Severity.WARNING);
						}
						
						Qos qos = CodecFactory.createQos();
						Utilities.toRsslQos(rate.intValue(), timeliness.intValue(), qos);
						service.info().applyHasQos();
						service.info().qosList().add(qos);
					}
		    	}
		    }
		}
		
		if ( service.info().capabilitiesList().size() == 0 )
		{
			config.errorTracker().append("specified service [")
			.append(service.info().serviceName().toString())
			.append("] contains no capabilities. Will drop this service.").create(Severity.ERROR);
			return false;
		}
		
		return true;
	}
	
	void readServiceInfoFilterDictionary(EmaConfigImpl config, Service service, XMLnode node, int nodeId, int entryId)
	{
		String dictionaryName = null;
		List<ConfigElement> configElementList;
		ConfigElement element;
		String rdmEnumTypeItemName;
		String rdmFieldDictItemName;
		
		if ( node.tagId() == nodeId )
		{
			configElementList =  node.attributeList().getConfigElementList(entryId);
			
			for(int index = 0 ; index < configElementList.size(); ++index )
			{
				dictionaryName = configElementList.get(index).asciiValue();
				
				ConfigAttributes dictionaryAttributes = config.xmlConfig().getDictionaryAttributes(dictionaryName);
				
				if ( dictionaryAttributes != null)
				{
					element = (ConfigElement) dictionaryAttributes.getElement(ConfigManager.DictionaryEnumTypeDefItemName);
					
					if ( element == null || ( rdmEnumTypeItemName = element.asciiValue()).isEmpty() )
					{
						rdmEnumTypeItemName = DictionaryCallbackClient.DICTIONARY_RWFENUM;
						
						config.errorTracker().append("no configuration exists or unspecified name for EnumTypeDefItemName in dictionary [")
						.append(dictionaryName).append("]. Will use default value of ").append(rdmEnumTypeItemName)
						.create(Severity.WARNING);
					}
		
					element = (ConfigElement) dictionaryAttributes.getElement(ConfigManager.DictionaryRdmFieldDictionaryItemName);
					
					if ( element == null || ( rdmFieldDictItemName = element.asciiValue()).isEmpty() )
					{
						rdmFieldDictItemName = DictionaryCallbackClient.DICTIONARY_RWFFID;
						
						config.errorTracker().append("no configuration exists or unspecified name for RdmFieldDictionaryItemName in dictionary [")
						.append(dictionaryName).append("]. Will use default value of ").append(rdmFieldDictItemName)
						.create(Severity.WARNING);
					}
				}
				else
				{
					config.errorTracker().append("no configuration exists for dictionary [")
					.append(dictionaryName).append("]. Will use dictionary defaults").create(Severity.WARNING);
					
					rdmEnumTypeItemName = DictionaryCallbackClient.DICTIONARY_RWFENUM;
					rdmFieldDictItemName = DictionaryCallbackClient.DICTIONARY_RWFFID;
				}
				

				if ( nodeId == ConfigManager.ServiceInfoFilterDictionariesProvided )
				{
					service.info().applyHasDictionariesProvided();
					service.info().dictionariesProvidedList().add(rdmEnumTypeItemName);
					service.info().dictionariesProvidedList().add(rdmFieldDictItemName);
				}
				else if ( nodeId == ConfigManager.ServiceInfoFilterDictionariesUsed )
				{
					service.info().applyHasDictionariesUsed();
					service.info().dictionariesUsedList().add(rdmEnumTypeItemName);
					service.info().dictionariesUsedList().add(rdmFieldDictItemName);
				}
			}
		}
	}
	
	boolean readServiceStateFilter(EmaConfigImpl config, Service service, XMLnode stateFilterNode)
	{
		ConfigAttributes stateAttributes =  stateFilterNode.attributeList();
		
		ConfigElement element = (ConfigElement) stateAttributes.getElement(ConfigManager.ServiceStateFilterServiceState);
		
		if (element != null)
		{
			service.applyHasState();
		    service.state().action(FilterEntryActions.SET);
			service.state().serviceState(element.intLongValue());
		}
		
		element = (ConfigElement) stateAttributes.getElement(ConfigManager.ServiceStateFilterAcceptingRequests);
		
		if (element != null)
		{
			service.state().applyHasAcceptingRequests();
			service.state().acceptingRequests(element.intLongValue());
		}
		
		for(int j = 0; j < stateFilterNode.children().size() ; j++ )
		{
			XMLnode node = stateFilterNode.children().get(j);
			
			ConfigAttributes statusAttributes =  node.attributeList();
			
			element = (ConfigElement) statusAttributes.getElement(ConfigManager.ServiceStateFilterStatusStreamState);
			
			if (element != null)
			{			
				service.state().applyHasStatus();
	            service.state().status().streamState(element.intValue());
			}
			
			element = (ConfigElement) statusAttributes.getElement(ConfigManager.ServiceStateFilterStatusDataState);
			
			if (element != null)
			{
				service.state().applyHasStatus();
				service.state().status().dataState(element.intValue());
			}
			
			element = (ConfigElement) statusAttributes.getElement(ConfigManager.ServiceStateFilterStatusStatusCode);
			
			if (element != null)
			{
				service.state().applyHasStatus();
	            service.state().status().code(element.intValue());
			}
			
			element = (ConfigElement) statusAttributes.getElement(ConfigManager.ServiceStateFilterStatusStatusText);
			
			if (element != null)
			{			
				service.state().applyHasStatus();
	            service.state().status().text().data(element.asciiValue());
			}
		}
		
		return true;
	}
	
	void useDefaultService(EmaConfigImpl config)
	{		
		Service service = DirectoryMsgFactory.createService();
		populateDefaultService(service);
		directoryServiceStore.addToMap(service);
		_activeConfig.directoryConfig.addService(service);
	}
	
	void populateDefaultService(Service service)
	{
		service.clear();
	    service.action(MapEntryActions.ADD);
		service.serviceId(OmmNiProviderActiveConfig.DEFAULT_SERVICE_ID);
		
		service.applyHasInfo();
		service.info().action(FilterEntryActions.SET);

		service.info().applyHasVendor();
		service.info().vendor().data("");
		
		service.info().serviceName().data(OmmNiProviderActiveConfig.DEFAULT_SERVICE_NAME);

        service.info().applyHasSupportsQosRange();
        service.info().supportsQosRange(OmmNiProviderActiveConfig.DEFAULT_SERVICE_SUPPORTS_QOS_RANGE);
      
        service.info().capabilitiesList().add((long)com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE);
        service.info().capabilitiesList().add((long)com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_BY_ORDER);
        service.info().capabilitiesList().add((long)com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_BY_PRICE);
        service.info().capabilitiesList().add((long)com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_MAKER);

        service.info().applyHasQos();
        Qos qos = CodecFactory.createQos();
        qos.rate(QosRates.TICK_BY_TICK);
        qos.timeliness(QosTimeliness.REALTIME);
        service.info().qosList().add(qos);

        service.info().applyHasDictionariesUsed();
        service.info().dictionariesUsedList().add(DictionaryCallbackClient.DICTIONARY_RWFFID);
        service.info().dictionariesUsedList().add(DictionaryCallbackClient.DICTIONARY_RWFENUM);

        service.info().applyHasIsSource();
        service.info().isSource(OmmNiProviderActiveConfig.DEFAULT_SERVICE_IS_SOURCE);
        
        service.info().applyHasItemList();
        service.info().itemList().data("");

        service.info().applyHasAcceptingConsumerStatus();
        service.info().acceptingConsumerStatus(OmmNiProviderActiveConfig.DEFAULT_SERVICE_ACCEPTING_CONSUMER_SERVICE);
        
        service.info().applyHasSupportsOutOfBandSnapshots();
        service.info().supportsOutOfBandSnapshots(OmmNiProviderActiveConfig.DEFAULT_SERVICE_SUPPORTS_OUT_OF_BAND_SNAPSHATS);
  
        service.applyHasState();
        service.state().action(FilterEntryActions.SET);
        service.state().serviceState(OmmNiProviderActiveConfig.DEFAULT_SERVICE_STATE);
        
        service.state().applyHasAcceptingRequests();
        service.state().acceptingRequests(OmmNiProviderActiveConfig.DEFAULT_ACCEPTING_REQUESTS);
	}

	@Override
	void processChannelEvent(ReactorChannelEvent reactorChannelEvent) {
		switch ( reactorChannelEvent.eventType() )
		{
		case ReactorChannelEventTypes.CHANNEL_DOWN:
		case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
			if ( _activeConfig.removeItemsOnDisconnect )
				removeItems();
			break;
		default:
			break;
		}
		
	}
	
	void removeItems()
	{
		_bIsStreamIdZeroRefreshSubmitted = false;
		
		Set<Entry<Long, StreamInfo>> entrySet = _handleToStreamInfo.entrySet();
		
		for (Entry<Long, StreamInfo> entry : entrySet)
		{
			_objManager._streamInfoPool.add(entry.getValue());
		}
		
		_handleToStreamInfo.clear();
		
		directoryServiceStore.clearMap();
	}

	@Override
	Logger createLoggerClient() {
		return LoggerFactory.getLogger(OmmNiProviderImpl.class);
	}

	@Override
	ConfigAttributes getAttributes(EmaConfigImpl config)
	{
		return config.xmlConfig().getNiProviderAttributes(_activeConfig.configuredName);
	}
	
	@Override
	Object getAttributeValue(EmaConfigImpl config, int attributeKey)
	{
		return config.xmlConfig().getNiProviderAttributeValue(_activeConfig.configuredName, attributeKey);
	}

	@Override
	void handleAdminDomains() {
		
		_loginCallbackClient = new LoginCallbackClientProvider(this);
		_loginCallbackClient.initialize();

		_itemCallbackClient = new ItemCallbackClientProvider(this);
		_itemCallbackClient.initialize();

		_channelCallbackClient = new ChannelCallbackClient<>(this,_rsslReactor);
		
		if ( _activeConfig.directoryAdminControl == OmmNiProviderConfig.AdminControl.API_CONTROL)
			_channelCallbackClient.initializeNiProviderRole(_loginCallbackClient.rsslLoginRequest(), _activeConfig.directoryConfig.getDirectoryRefresh());
		else
		{
			if (loggerClient().isTraceEnabled())
			{
				loggerClient().trace(formatLogMessage(_activeConfig.instanceName, "DirectoryAdminControl = UserControl", Severity.TRACE));
			}
			
			_channelCallbackClient.initializeNiProviderRole(_loginCallbackClient.rsslLoginRequest(), null);
		}

		handleLoginReqTimeout();
	}

	@Override
	void handleInvalidUsage(String text)
	{
		if ( hasErrorClient() )
			_providerErrorClient.onInvalidUsage(text);
		else
			throw (ommIUExcept().message(text.toString()));
		
	}

	@Override
	void handleInvalidHandle(long handle, String text)
	{	
		if ( hasErrorClient() )
			_providerErrorClient.onInvalidHandle(handle, text);
		else
			throw (ommIHExcept().message(text, handle));
	}
	
	boolean decodeSourceDirectory(com.thomsonreuters.upa.codec.Msg rsslMsg, Buffer outputBuffer, StringBuilder errorText)
	{
		int retCode = CodecReturnCodes.SUCCESS;
		DecodeIterator decodeIt = CodecFactory.createDecodeIterator();
		decodeIt.clear();
		
		Buffer inputBuffer = rsslMsg.encodedDataBody();
		
		retCode = decodeIt.setBufferAndRWFVersion(inputBuffer, Codec.majorVersion(), Codec.minorVersion());
		
		if( retCode !=  CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error. Failed to set decode iterator buffer and version in OmmNiProviderImpl.decodeSourceDirectory(). Reason = ")
			.append( CodecReturnCodes.toString(retCode) ).append(".");
			return false;
		}
		
		com.thomsonreuters.upa.codec.Map map = CodecFactory.createMap();
		map.clear();
		
		if ( loggerClient().isTraceEnabled() )
		{
			loggerClient().trace( formatLogMessage(_activeConfig.instanceName, "Begin decoding of SourceDirectory.", Severity.TRACE) );
		}
		
		retCode = map.decode(decodeIt);
		
		if( retCode <  CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error. Failed to decode Map in OmmNiProviderImpl.decodeSourceDirectory(). Reason = ")
			.append( CodecReturnCodes.toString(retCode) ).append(".");
			return false;
		}
		else if ( retCode == CodecReturnCodes.NO_DATA )
		{
			if ( loggerClient().isWarnEnabled() )
			{
				loggerClient().warn( formatLogMessage(_activeConfig.instanceName, "Passed in SourceDirectory map contains no entries"
						+ " (e.g. there is no service specified).", Severity.WARNING) );
			}
			
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace( formatLogMessage(_activeConfig.instanceName, "End decoding of SourceDirectory.", Severity.TRACE) );
			}
			
			return true;
		}
		
		switch( map.keyPrimitiveType())
		{
		case com.thomsonreuters.upa.codec.DataTypes.UINT:
			if( !decodeSourceDirectoryKeyUInt(map, decodeIt, errorText) )
				return false;
			break;
		case com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING:
		{
			errorText.append("Attempt to specify SourceDirectory info with a Map using key DataType of ")
			.append( DataType.asString(Utilities.toEmaDataType[map.keyPrimitiveType()]))
			.append(" while the expected key DataType is ")
			.append( DataType.asString(DataType.DataTypes.UINT));
			
			if ( loggerClient().isErrorEnabled() )
			{
				loggerClient().error( formatLogMessage(_activeConfig.instanceName, errorText.toString(), Severity.ERROR) );
			}
			
			return false;
		}
		default:
			errorText.append("Attempt to specify SourceDirectory info with a Map using key DataType of  ")
			.append( DataType.asString(Utilities.toEmaDataType[map.keyPrimitiveType()]))
			.append(" while the expected key DataType is ")
			.append( DataType.asString(DataType.DataTypes.UINT) + " or " + DataType.asString(DataType.DataTypes.ASCII) );
			return false;
		}
		
		if ( loggerClient().isTraceEnabled() )
		{
			loggerClient().trace( formatLogMessage(_activeConfig.instanceName, "End decoding of SourceDirectory.", Severity.TRACE) );
		}
		
		return true;
	}
	
	boolean decodeSourceDirectoryKeyUInt(com.thomsonreuters.upa.codec.Map map, DecodeIterator decodeIt, StringBuilder errorText)
	{
		int retCode = CodecReturnCodes.SUCCESS;
		com.thomsonreuters.upa.codec.UInt serviceId = CodecFactory.createUInt();
		com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
		StringBuilder text = new StringBuilder();
		com.thomsonreuters.upa.codec.FilterList filterList = CodecFactory.createFilterList();
		com.thomsonreuters.upa.codec.FilterEntry filterEntry = CodecFactory.createFilterEntry();
		com.thomsonreuters.upa.codec.ElementList elementList = CodecFactory.createElementList();
		com.thomsonreuters.upa.codec.ElementEntry elementEntry = CodecFactory.createElementEntry();
		Service service = null;
		
		while ( ( retCode = mapEntry.decode(decodeIt, serviceId) ) != CodecReturnCodes.END_OF_CONTAINER )
		{
			if ( retCode != CodecReturnCodes.SUCCESS )
			{
				errorText.append( "Internal error: Failed to Decode Map Entry. Reason = " )
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			
			text.setLength(0);
			text.append( "Begin decoding of Service with id of " );
			text.append( serviceId ).append(". Action= ");
			switch ( mapEntry.action() )
			{
			case com.thomsonreuters.upa.codec.MapEntryActions.UPDATE:
				text.append("Upate");
				break;
			case com.thomsonreuters.upa.codec.MapEntryActions.ADD:
				text.append("Add");
				break;
			case com.thomsonreuters.upa.codec.MapEntryActions.DELETE:
				text.append("Delete");
				break;
			}
			
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
			}
			
			if ( mapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.DELETE )
			{
				String serviceName = directoryServiceStore.serviceName(serviceId.toBigInteger().intValue());
				
				if ( serviceName != null )
				{
					directoryServiceStore.remove(serviceId.toBigInteger().intValue());
				}
				
				text.setLength(0);
				text.append("End decoding of Service with id of ").append(serviceId);
				if ( loggerClient().isTraceEnabled() )
				{
					loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
				}
				
				continue;
			}
			else if ( mapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.ADD )
			{
				String serviceName = directoryServiceStore.serviceName(serviceId.toBigInteger().intValue());
				
				if ( serviceName != null )
				{
					errorText.append("Attempt to add a service with name of ");
					errorText.append( serviceName ).append( " and id of ").append( serviceId ).append( " while a service with the same id is already added." );
					return false;
				}
			}
			
			if( map.containerType() != com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST )
			{
				errorText.append( "Attempt to specify Service with a container of " )
				.append(DataType.asString(Utilities.toEmaDataType[map.containerType()]))
				.append("  rather than the expected  ").append( DataType.asString(DataTypes.FILTER_LIST));
				return false;
			}
			
			if ( _activeConfig.recoverUserSubmitSourceDirectory )
			{
				if ( service == null )
				{
					service = DirectoryMsgFactory.createService();
				}
				else
				{
					service.clear();
				}
				
				service.serviceId(serviceId.toBigInteger().intValue());
			}
			
			filterList.clear();
			filterEntry.clear();
			
			retCode = filterList.decode(decodeIt);
			
			if ( retCode < CodecReturnCodes.SUCCESS )
			{
				errorText.append("Internal error: Failed to Decode FilterList. Reason")
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			else if ( retCode == CodecReturnCodes.NO_DATA )
			{
				text.setLength(0);
				text.append("Service with id of ").append(serviceId)
				.append(" contains no FilterEntries. Skipping this service.");
				
				if ( loggerClient().isWarnEnabled() )
				{
					loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.WARNING) );
				}
				
				text.setLength(0);
				text.append("End decoding of Service with id of ").append(serviceId);
				
				if ( loggerClient().isTraceEnabled() )
				{
					loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
				}
				
				continue;
			}
			
			while ( ( retCode = filterEntry.decode(decodeIt) ) != CodecReturnCodes.END_OF_CONTAINER )
			{	
				if ( retCode < CodecReturnCodes.SUCCESS )
				{
					errorText.append("Internal error: Failed to Decode Filter Entry. Reason = ");
					errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
					return false;
				}
				
				text.setLength(0);
				text.append("Begin decoding of FilterEntry with id of ").append(filterEntry.id());
				
				if ( loggerClient().isTraceEnabled() )
				{
					loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
				}
				
				if ( filterEntry.id() == com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds.INFO )
				{
					if( mapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.UPDATE )
					{
						errorText.append("Attempt to update Infofilter of service with id of ").append(serviceId)
						.append("  while this is not allowed.");
						return false;
					}
					
					if ( filterEntry.checkHasContainerType() && ( filterEntry.containerType() != com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )
							&& filterList.containerType() != com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )
					{
						int containerType = filterEntry.checkHasContainerType() ? filterEntry.containerType() : filterList.containerType();
						errorText.append("Attempt to specify Service InfoFilter with a container of ");
						errorText.append(DataType.asString(Utilities.toEmaDataType[containerType]));
						errorText.append(" rather than the expected ").append(DataType.asString(DataTypes.ELEMENT_LIST));
						return false;
					}
					
					if ( _activeConfig.recoverUserSubmitSourceDirectory )
					{
						userStoreDecodeIt.clear();
						retCode = userStoreDecodeIt.setBufferAndRWFVersion(filterEntry.encodedData(), Codec.majorVersion(), Codec.minorVersion());
						
						if (  ( retCode < CodecReturnCodes.SUCCESS ) || ( ( retCode = service.info().decode(userStoreDecodeIt) ) < CodecReturnCodes.SUCCESS ) ) 
						{
							errorText.append("Internal error: Failed to decode ServiceInfo. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						service.applyHasInfo();
					}
					
					elementList.clear();
					elementEntry.clear();
					
					if ( ( retCode = elementList.decode(decodeIt, null) ) < CodecReturnCodes.SUCCESS )
					{
						errorText.append("Internal error: Failed to Decode Element List. Reason = ");
						errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
						return false;
					}
					
					boolean bServiceNameEntryFound = false;
					
					while ( ( retCode = elementEntry.decode(decodeIt)) != CodecReturnCodes.END_OF_CONTAINER )
					{
						if ( retCode < CodecReturnCodes.SUCCESS )
						{
							errorText.append("Internal error: Failed to Decode ElementEntry. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						
						text.setLength(0);
						text.append("Decoding of ElementEntry with name of ");
						text.append(elementEntry.name().toString());
						
						if ( loggerClient().isTraceEnabled() )
						{
							loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
						}
						
						if( !bServiceNameEntryFound && elementEntry.name().equals(com.thomsonreuters.upa.rdm.ElementNames.NAME) )
						{
							if ( elementEntry.dataType() != com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING )
							{
								errorText.append("Attempt to specify Service Name with a ")
								.append( DataType.asString(Utilities.toEmaDataType[elementEntry.dataType()]) )
								.append(" rather than the expected ").append( DataType.asString(DataTypes.ASCII));
								return false;
							}
							
							Buffer serviceNameBuffer = CodecFactory.createBuffer();
							serviceNameBuffer.clear();
							
							retCode = serviceNameBuffer.decode(decodeIt);
							if( retCode < CodecReturnCodes.SUCCESS)
							{
								errorText.append("Internal error: Failed to Decode Buffer. Reason = ");
								errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
								return false;
							}
							else if ( retCode == CodecReturnCodes.BLANK_DATA )
							{
								errorText.append("Attempt to specify Service Name with a blank ascii string for service id of ");
								errorText.append(serviceId);
								return false;
							}
							
							bServiceNameEntryFound = true;
							
							if ( directoryServiceStore.serviceId(serviceNameBuffer.toString()) != null )
							{
								errorText.append("Attempt to add a service with name of ")
								.append(serviceNameBuffer.toString()).append(" and id of" )
								.append( serviceId ).append(" while a service with the same id is already added.");
								return false;
							}
							
							directoryServiceStore.addToMap(serviceId.toBigInteger().intValue(), serviceNameBuffer.toString());
							
							text.setLength(0);
							text.append("Detected Service with name of ")
							.append(serviceNameBuffer.toString()).append(" and id of ").append( serviceId );
							
							if ( loggerClient().isTraceEnabled() )
							{
								loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
							}
						}
					}
					
					if( !bServiceNameEntryFound )
					{
						errorText.append("Attempt to specify service InfoFilter without required Service Name for service id of ")
						.append(serviceId);
						return false;
					}
					
					
				}
				else if ( filterEntry.id() == com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds.STATE )
				{
					if ( _activeConfig.recoverUserSubmitSourceDirectory )
					{
						if ( ( retCode = service.state().decode(decodeIt) ) < CodecReturnCodes.SUCCESS ) 
						{
							errorText.append("Internal error: Failed to decode ServiceState. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						service.applyHasState();
					}
				}
			}
			
			text.setLength(0);
			text.append("End decoding of FilterEntry with id of ");
			text.append(filterEntry.id());
			
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
			}
			
			if ( _activeConfig.recoverUserSubmitSourceDirectory )
			{
				service = directoryServiceStore.addToStore(service);
			}
		}
		
		return true;
	}
	
	boolean decodeSourceDirectoryKeyAscii(com.thomsonreuters.upa.codec.Map map, DecodeIterator decodeIt, StringBuilder errorText)
	{
		int retCode = CodecReturnCodes.SUCCESS;
		Buffer serviceNameBuffer = CodecFactory.createBuffer();
		serviceNameBuffer.clear();
		StringBuilder text = new StringBuilder();
		int emaAssignedServiceId = 0;
		com.thomsonreuters.upa.codec.MapEntry mapEntry = CodecFactory.createMapEntry();
		com.thomsonreuters.upa.codec.FilterList filterList = CodecFactory.createFilterList();
		com.thomsonreuters.upa.codec.FilterEntry filterEntry = CodecFactory.createFilterEntry();
		com.thomsonreuters.upa.codec.ElementList elementList = CodecFactory.createElementList();
		com.thomsonreuters.upa.codec.ElementEntry elementEntry = CodecFactory.createElementEntry();
		Service service = null;
		
		while ( ( retCode = mapEntry.decode(decodeIt, serviceNameBuffer) ) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if ( retCode != CodecReturnCodes.SUCCESS )
			{
				errorText.append( "Internal error: Failed to Decode Map Entry. Reason = " )
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			
			text.setLength(0);
			text.append("Begin decoding of Service with name of ");
			text.append(serviceNameBuffer.toString()).append(". Action = ");
			switch ( mapEntry.action() )
			{
			case com.thomsonreuters.upa.codec.MapEntryActions.UPDATE:
				text.append("Upate");
				break;
			case com.thomsonreuters.upa.codec.MapEntryActions.ADD:
				text.append("Add");
				
				break;
			case com.thomsonreuters.upa.codec.MapEntryActions.DELETE:
				text.append("Delete");
				
				break;
			}
			
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
			}
			
			if ( mapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.DELETE )
			{
				DirectoryServiceStore.ServiceIdInteger serviceIdInteger = directoryServiceStore.serviceId(serviceNameBuffer.toString());
				
				if ( serviceIdInteger != null )
				{
					directoryServiceStore.remove(serviceIdInteger.value());
					
					com.thomsonreuters.upa.codec.UInt serviceId = CodecFactory.createUInt();
					serviceId.value(serviceIdInteger.value());
					_deletedServicesNameAndIdTable.put(serviceNameBuffer.toString(), serviceId);
				}
				
				text.setLength(0);
				text.append("End decoding of Service with name of ").append( serviceNameBuffer.toString());
				
				if ( loggerClient().isTraceEnabled() )
				{
					loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
				}
				
				continue;
			}
			else if (mapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.ADD)
			{
				DirectoryServiceStore.ServiceIdInteger serviceIdInteger = directoryServiceStore.serviceId(serviceNameBuffer.toString());
				
				if ( serviceIdInteger != null )
				{
					errorText.append("Attempt to add a service with name of ")
					.append( serviceNameBuffer.toString() ).append( " and id of ").append( serviceIdInteger.value() )
					.append( " while a service with the same name is already added." );
					return false;
				}
			}
			
			if( map.containerType() != com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST )
			{
				errorText.append( "Attempt to specify Service with a container of " )
				.append(DataType.asString(Utilities.toEmaDataType[map.containerType()]))
				.append("  rather than the expected  ").append( DataType.asString(DataTypes.FILTER_LIST));
				return false;
			}
			
			if ( _activeConfig.recoverUserSubmitSourceDirectory )
			{
				if ( service == null )
					service = DirectoryMsgFactory.createService();
				else
					service.clear();
			}
			
			filterList.clear();
			filterEntry.clear();
			
			if ( ( retCode = filterList.decode(decodeIt) ) < CodecReturnCodes.SUCCESS )
			{
				errorText.append("Internal error: Failed to Decode FilterList. Reason")
				.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			else if ( retCode == CodecReturnCodes.NO_DATA )
			{
				text.setLength(0);
				text.append("Service with name of ").append(serviceNameBuffer.toString())
				.append(" contains no FilterEntries. Skipping this service.");
		
				if ( loggerClient().isWarnEnabled() )
				{
					loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.WARNING) );
				}
				
				text.setLength(0);
				text.append("End decoding of Service with name of ").append(serviceNameBuffer.toString());
				
				if ( loggerClient().isTraceEnabled() )
				{
					loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
				}
				
				continue;
			}
			
			while ( ( retCode = filterEntry.decode(decodeIt) ) != CodecReturnCodes.END_OF_CONTAINER )
			{
				if ( retCode < CodecReturnCodes.SUCCESS )
				{
					errorText.append("Internal error: Failed to Decode Filter Entry. Reason = ");
					errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
					return false;
				}
				
				text.setLength(0);
				text.append("Begin decoding of FilterEntry with id of ").append(filterEntry.id());
				
				if ( loggerClient().isTraceEnabled() )
				{
					loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
				}
				
				if ( filterEntry.id() == com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds.INFO )
				{
					if( mapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.UPDATE )
					{
						errorText.append("Attempt to update Infofilter of service with name of ").append(serviceNameBuffer.toString())
						.append("  while this is not allowed.");
						return false;
					}
					
					if ( filterEntry.checkHasContainerType() && ( filterEntry.containerType() != com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )
							&& filterList.containerType() != com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST )
					{
						int containerType = filterEntry.checkHasContainerType() ? filterEntry.containerType() : filterList.containerType();
						errorText.append("Attempt to specify Service InfoFilter with a container of ");
						errorText.append(DataType.asString(Utilities.toEmaDataType[containerType]));
						errorText.append(" rather than the expected ").append(DataType.asString(DataTypes.ELEMENT_LIST));
						return false;
					}
					
					if ( _activeConfig.recoverUserSubmitSourceDirectory )
					{
						userStoreDecodeIt.clear();
						retCode = userStoreDecodeIt.setBufferAndRWFVersion(filterEntry.encodedData(), Codec.majorVersion(), Codec.minorVersion());
						
						if (  ( retCode < CodecReturnCodes.SUCCESS ) || ( ( retCode = service.info().decode(userStoreDecodeIt) ) < CodecReturnCodes.SUCCESS ) ) 
						{
							errorText.append("Internal error: Failed to decode ServiceInfo. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						service.applyHasInfo();
					}
					
					elementList.clear();
					elementEntry.clear();
					
					if ( ( retCode = elementList.decode(decodeIt, null) ) < CodecReturnCodes.SUCCESS )
					{
						errorText.append("Internal error: Failed to Decode Element List. Reason = ");
						errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
						return false;
					}
					
					boolean bServiceIdEntryFound = false;
					
					while ( ( retCode = elementEntry.decode(decodeIt)) != CodecReturnCodes.END_OF_CONTAINER )
					{
						if ( retCode < CodecReturnCodes.SUCCESS )
						{
							errorText.append("Internal error: Failed to Decode ElementEntry. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						
						text.setLength(0);
						text.append("Decoding of ElementEntry with name of ");
						text.append(elementEntry.name().toString());
						
						if ( loggerClient().isTraceEnabled() )
						{
							loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
						}
						
						if( !bServiceIdEntryFound && elementEntry.name().toString().equals(EmaRdm.ENAME_SERVICE_ID) )
						{
							if ( elementEntry.dataType() != com.thomsonreuters.upa.codec.DataTypes.UINT )
							{
								errorText.append("Attempt to specify Service Id with a ")
								.append( DataType.asString(Utilities.toEmaDataType[elementEntry.dataType()]) )
								.append(" rather than the expected ").append( DataType.asString(DataTypes.UINT));
								return false;
							}
							
							UInt serviceId = CodecFactory.createUInt();
							serviceId.clear();
							
							retCode = serviceId.decode(decodeIt);
							if( retCode < CodecReturnCodes.SUCCESS)
							{
								errorText.append("Internal error: Failed to Decode UInt. Reason = ");
								errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
								return false;
							}
							else if ( retCode == CodecReturnCodes.BLANK_DATA )
							{
								errorText.append("Attempt to specify Service Id with a blank UInt string for service id of ");
								errorText.append(serviceId);
								return false;
							}
							
							bServiceIdEntryFound = true;
							
							if ( directoryServiceStore.serviceName(serviceId.toBigInteger().intValue()) != null )
							{
								errorText.append("Attempt to add a service with name of ")
								.append( serviceNameBuffer.toString() ).append( " and id of " ).append( serviceId.toString() )
								.append( " while a service with the same id is already added." );
								return false;
							}
							
							directoryServiceStore.addToMap(serviceId.toBigInteger().intValue(), serviceNameBuffer.toString());
							
							if ( _activeConfig.recoverUserSubmitSourceDirectory )
							{
								service.serviceId(serviceId.toBigInteger().intValue());
								service.info().serviceName(serviceNameBuffer);
							}
						
							text.setLength(0);
							text.append("Detected Service with name of ")
							.append(serviceNameBuffer.toString()).append(" and id of ").append( serviceId );
							
							if ( loggerClient().isTraceEnabled() )
							{
								loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
							}
						}
					}
					
					if( !bServiceIdEntryFound )
					{
						boolean found = true;
						while ( directoryServiceStore.serviceName(emaAssignedServiceId) != null )
						{
							if ( emaAssignedServiceId == ConfigManager.MAX_UINT16 )
							{
								found = false;
								break;
							}
							++emaAssignedServiceId;
						}
						
						if ( !found )
						{
							errorText.append("All service ids are used.");
							return false;
						}
						
						directoryServiceStore.addToMap(emaAssignedServiceId, serviceNameBuffer.toString());
						
						if ( _activeConfig.recoverUserSubmitSourceDirectory )
						{
							service.serviceId(emaAssignedServiceId);
							service.info().serviceName(serviceNameBuffer);
						}
						
						text.setLength(0);
						text.append("Assigned service id of ").append(emaAssignedServiceId)
						.append(" to service with name of ").append(serviceNameBuffer.toString());
						
						if ( loggerClient().isTraceEnabled() )
						{
							loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
						}
					}
				}
				else if ( filterEntry.id() == com.thomsonreuters.upa.rdm.Directory.ServiceFilterIds.STATE )
				{
					if ( _activeConfig.recoverUserSubmitSourceDirectory )
					{
						if ( ( retCode = service.state().decode(decodeIt) ) < CodecReturnCodes.SUCCESS ) 
						{
							errorText.append("Internal error: Failed to decode ServiceState. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						service.applyHasState();
					}
				}
			}
			
			text.setLength(0);
			text.append("End decoding of FilterEntry with id of ");
			text.append(filterEntry.id());
			
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace( formatLogMessage(_activeConfig.instanceName, text.toString(), Severity.TRACE) );
			}
			
			if ( _activeConfig.recoverUserSubmitSourceDirectory )
			{
				service = directoryServiceStore.addToStore(service);
			}
		}
		
		return true;
	}
	
	boolean swapServiceNameAndId(Buffer inputBuffer, Buffer outputBuffer, StringBuilder errorText)
	{
		DecodeIterator decodeIt = CodecFactory.createDecodeIterator();
		decodeIt.clear();
		decodeIt.setBufferAndRWFVersion(inputBuffer, Codec.majorVersion(), Codec.minorVersion());
		
		com.thomsonreuters.upa.codec.Map inMap = CodecFactory.createMap();
		inMap.clear();
		
		outputBuffer.data(ByteBuffer.allocate(inputBuffer.length() + 512));
		
		EncodeIterator encodeIt = CodecFactory.createEncodeIterator();
		encodeIt.clear();
		
		encodeIt.setBufferAndRWFVersion(outputBuffer, Codec.majorVersion(),Codec.minorVersion());
		
		com.thomsonreuters.upa.codec.Map outMap = CodecFactory.createMap();
		outMap.clear();
		
		Buffer serviceNameBuffer = CodecFactory.createBuffer();
		serviceNameBuffer.clear();
		
		com.thomsonreuters.upa.codec.MapEntry inMapEntry = CodecFactory.createMapEntry();
		inMapEntry.clear();
		
		com.thomsonreuters.upa.codec.MapEntry outMapEntry = CodecFactory.createMapEntry();
		
		com.thomsonreuters.upa.codec.FilterList inFilterList = CodecFactory.createFilterList();
		inFilterList.clear();
		
		com.thomsonreuters.upa.codec.FilterEntry inFilterEntry = CodecFactory.createFilterEntry();
		inFilterEntry.clear();
		
		com.thomsonreuters.upa.codec.FilterList outFilterList = CodecFactory.createFilterList();
		outFilterList.clear();
		
		com.thomsonreuters.upa.codec.FilterEntry outFilterEntry = CodecFactory.createFilterEntry();
		com.thomsonreuters.upa.codec.ElementList inElementList = CodecFactory.createElementList();
		com.thomsonreuters.upa.codec.ElementEntry inElementEntry = CodecFactory.createElementEntry();
		com.thomsonreuters.upa.codec.ElementList outElementList = CodecFactory.createElementList();
		com.thomsonreuters.upa.codec.ElementEntry outElementEntry = CodecFactory.createElementEntry();
		
		outMap.keyPrimitiveType(com.thomsonreuters.upa.codec.DataTypes.UINT);
		outMap.flags(com.thomsonreuters.upa.codec.MapFlags.NONE);
		outMap.containerType(com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
		
		int retCode;
		
		if ( ( retCode = outMap.encodeInit(encodeIt, 0, 0) ) != CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error: Failed to Encode Map. Reason = ");
			errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
			return false;
		}
		
		if ( inMap.decode(decodeIt) == CodecReturnCodes.NO_DATA )
		{
			if ( ( retCode = outMap.encodeComplete(encodeIt, true) ) < CodecReturnCodes.SUCCESS )
			{
				errorText.append("Internal error: Failed to Encode Map. Reason = ");
				errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
				return false;
			}
			
			return true;
		}
		
		while ( inMapEntry.decode(decodeIt, serviceNameBuffer)  != CodecReturnCodes.END_OF_CONTAINER )
		{
			outMapEntry.clear();
			outMapEntry.flags(com.thomsonreuters.upa.codec.MapEntryFlags.NONE);
			outMapEntry.action(inMapEntry.action());
			String serviceName = serviceNameBuffer.toString();
			
			if ( outMapEntry.action() == com.thomsonreuters.upa.codec.MapEntryActions.DELETE )
			{	
				while( ( retCode = outMapEntry.encode(encodeIt, _deletedServicesNameAndIdTable.get(serviceName)) ) == CodecReturnCodes.BUFFER_TOO_SMALL )
				{
					outputBuffer.data(ByteBuffer.allocate(outputBuffer.capacity()*2));
					encodeIt.realignBuffer(outputBuffer);
				}
				
				if ( retCode != CodecReturnCodes.SUCCESS )
				{
					errorText.append("Internal error: Failed to Encode MapEntry. Reason = ");
					errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
					return false;
				}
				
				continue;
			}
			else
			{
				DirectoryServiceStore.ServiceIdInteger serviceId = directoryServiceStore.serviceId(serviceName);
				com.thomsonreuters.upa.codec.UInt serviceIdUInt = CodecFactory.createUInt();
				serviceIdUInt.value(serviceId.value());
			
				outFilterList.containerType(com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
				outFilterList.flags(com.thomsonreuters.upa.codec.FilterListFlags.NONE);
				
				while( ( retCode = outMapEntry.encodeInit(encodeIt, serviceIdUInt, 0) ) == CodecReturnCodes.BUFFER_TOO_SMALL )
				{
					outputBuffer.data(ByteBuffer.allocate(outputBuffer.capacity()*2));
					encodeIt.realignBuffer(outputBuffer);
				}
				
				if ( retCode != CodecReturnCodes.SUCCESS )
				{
					errorText.append("Internal error: Failed to Encode MapEntry. Reason = ");
					errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
					return false;
				}
				
				while ( ( retCode = outFilterList.encodeInit(encodeIt)) == CodecReturnCodes.BUFFER_TOO_SMALL )
				{
					outputBuffer.data(ByteBuffer.allocate(outputBuffer.capacity()*2));
					encodeIt.realignBuffer(outputBuffer);
				}
				
				if ( retCode != CodecReturnCodes.SUCCESS )
				{
					errorText.append("Internal error: Failed to Encode FilterList. Reason = ");
					errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
					return false;
				}
				
				if ( inFilterList.decode(decodeIt) == CodecReturnCodes.NO_DATA )
				{
					outFilterList.encodeComplete(encodeIt, true);
					outMapEntry.encodeComplete(encodeIt, true);
					continue;
				}
				
				while( inFilterEntry.decode(decodeIt) != CodecReturnCodes.END_OF_CONTAINER )
				{
					outFilterEntry.clear();
					outFilterEntry.action(inFilterEntry.action());
					outFilterEntry.containerType(inFilterEntry.containerType());
					outFilterEntry.flags(inFilterEntry.flags());
					outFilterEntry.id(inFilterEntry.id());
					
					if ( inFilterEntry.checkHasPermData() )
					{
						outFilterEntry.permData(inFilterEntry.permData());
					}
					
					if ( inFilterEntry.id() == com.thomsonreuters.upa.rdm.Directory.ServiceFilterFlags.INFO )
					{
						inElementList.clear();
						inElementEntry.clear();
						
						outElementList.clear();
						
						while( ( retCode = outFilterEntry.encodeInit(encodeIt, 0) ) == CodecReturnCodes.BUFFER_TOO_SMALL )
						{
							outputBuffer.data(ByteBuffer.allocate(outputBuffer.capacity()*2));
							encodeIt.realignBuffer(outputBuffer);
						}
						
						if ( retCode != CodecReturnCodes.SUCCESS )
						{
							errorText.append("Internal error: Failed to Encode FilterEntry. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						
						if( inElementList.decode(decodeIt, null) == CodecReturnCodes.NO_DATA )
						{
							if ( ( retCode = outElementList.encodeInit(encodeIt, null, 0) ) < CodecReturnCodes.SUCCESS )
							{
								errorText.append("Internal error: Failed to Encode ElementList. Reason = ");
								errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
								return false;
							}
							
							outElementList.encodeComplete(encodeIt, true);
							outFilterEntry.encodeComplete(encodeIt, true);
							
							continue;
						}
						
						outElementList.applyHasStandardData();
						
						while ( ( retCode = outElementList.encodeInit(encodeIt, null, 0) ) == CodecReturnCodes.BUFFER_TOO_SMALL )
						{
							outputBuffer.data(ByteBuffer.allocate(outputBuffer.capacity()*2));
							encodeIt.realignBuffer(outputBuffer);
						}
						
						if ( retCode != CodecReturnCodes.SUCCESS )
						{
							errorText.append("Internal error: Failed to Encode ElementList. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						
						outElementEntry.clear();
						outElementEntry.dataType(com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING);
						outElementEntry.name(com.thomsonreuters.upa.rdm.ElementNames.NAME);
						
						while ( ( retCode = outElementEntry.encode(encodeIt,serviceNameBuffer)) == CodecReturnCodes.BUFFER_TOO_SMALL )
						{
							outputBuffer.data(ByteBuffer.allocate(outputBuffer.capacity()*2));
							encodeIt.realignBuffer(outputBuffer);
						}
						
						if ( retCode != CodecReturnCodes.SUCCESS )
						{
							errorText.append("Internal error: Failed to Encode ElementEntry. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
						
						while ( inElementEntry.decode(decodeIt) != CodecReturnCodes.END_OF_CONTAINER )
						{	
							if ( inElementEntry.name().toString().equals( com.thomsonreuters.ema.rdm.EmaRdm.ENAME_SERVICE_ID) == false )
							{
								outElementEntry.name(inElementEntry.name());
								outElementEntry.dataType(inElementEntry.dataType());
								outElementEntry.encodedData(inElementEntry.encodedData());
								
								while ( ( retCode = outElementEntry.encode(encodeIt)) == CodecReturnCodes.BUFFER_TOO_SMALL)
								{	
									outputBuffer.data(ByteBuffer.allocate(outputBuffer.capacity()*2));
									encodeIt.realignBuffer(outputBuffer);
								}
								
								if ( retCode != CodecReturnCodes.SUCCESS)
								{
									errorText.append("Internal error: Failed to Encode ElementEntry. Reason = ");
									errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
									return false;
								}
								
								inElementEntry.clear();
							}
						}
						
						outElementList.encodeComplete(encodeIt, true);
						outFilterEntry.encodeComplete(encodeIt, true);
					}
					else
					{
						outFilterEntry.encodedData(inFilterEntry.encodedData());
						
						while ( ( retCode = outFilterEntry.encode(encodeIt) ) == CodecReturnCodes.BUFFER_TOO_SMALL )
						{
							outputBuffer.data(ByteBuffer.allocate(outputBuffer.capacity()*2));
							encodeIt.realignBuffer(outputBuffer);
						}
						
						if ( retCode != CodecReturnCodes.SUCCESS )
						{
							errorText.append("Internal error: Failed to Encode FilterEntry. Reason = ");
							errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
							return false;
						}
					}
				}
				
				outFilterList.encodeComplete(encodeIt, true);
				outMapEntry.encodeComplete(encodeIt, true);
			}
		}
		
		outMap.encodeComplete(encodeIt, true);
		
		return true;
	}
	
	void remapServiceIdAndServcieName(DirectoryRefresh directoryRefresh)
	{
		List<Service> serviceList = directoryRefresh.serviceList();
		
		for(int index = 0; index < serviceList.size(); index++ )
		{
			directoryServiceStore.addToMap(serviceList.get(index));
		}	
	}
	
	int submitDirectoryRefresh(DirectoryRefresh directoryRefresh)
	{
		int retCode = CodecReturnCodes.SUCCESS;
		
		Buffer encodedBuffer = CodecFactory.createBuffer();
		
		encodedBuffer.data(ByteBuffer.allocate(1024));
		
		EncodeIterator encodeIt = CodecFactory.createEncodeIterator();
		encodeIt.clear();
		
		StringBuilder errorText = new StringBuilder();
		
		if ( encodeIt.setBufferAndRWFVersion(encodedBuffer, Codec.majorVersion(),Codec.minorVersion()) != CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error. Failed to set encode iterator buffer and version in OmmNiProviderImpl.reLoadConfigSourceDirectory().");
			handleInvalidUsage(errorText.toString());
		}
		
		while ( ( retCode = directoryRefresh.encode(encodeIt) ) == CodecReturnCodes.BUFFER_TOO_SMALL )
		{
			encodedBuffer.data(ByteBuffer.allocate(encodedBuffer.capacity()*2));
			encodeIt.realignBuffer(encodedBuffer);
		}
		
		if ( retCode != CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error. Failed to encode buffer from DirectoryRefresh in OmmNiProviderImpl.reLoadConfigSourceDirectory().").append(" Reason = ");
			errorText.append( CodecReturnCodes.toString(retCode) ).append(".");
			handleInvalidUsage(errorText.toString());
		}
		
		DecodeIterator decodeIt = CodecFactory.createDecodeIterator();
		decodeIt.clear();
		
		if ( decodeIt.setBufferAndRWFVersion(encodedBuffer, Codec.majorVersion(), Codec.minorVersion()) != CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error. Failed to set decode iterator buffer and version in OmmNiProviderImpl.reLoadConfigSourceDirectory().");
			handleInvalidUsage(errorText.toString());
		}
	
		com.thomsonreuters.upa.codec.RefreshMsg rsslRefreshMsg = (com.thomsonreuters.upa.codec.RefreshMsg)CodecFactory.createMsg();
		rsslRefreshMsg.clear();
		
		if ( rsslRefreshMsg.decode(decodeIt) != CodecReturnCodes.SUCCESS )
		{
			errorText.append("Internal error. Failed to decode message in OmmNiProviderImpl.reLoadConfigSourceDirectory().");
			handleInvalidUsage(errorText.toString());
		}
		
		int flags = rsslRefreshMsg.flags();
		flags &= ~RefreshMsgFlags.SOLICITED;
		rsslRefreshMsg.flags(flags);
		
		if( _activeConfig.removeItemsOnDisconnect )
		{
			remapServiceIdAndServcieName(directoryRefresh);
		}
		
		_rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _channelCallbackClient.channelList().get(0).rsslReactorChannel();
		if (ReactorReturnCodes.SUCCESS > (retCode = rsslChannel.submit(rsslRefreshMsg, _rsslSubmitOptions, _rsslErrorInfo)))
	    {			
			StringBuilder temp = strBuilder();
			if (loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in OmmNiProviderImpl.reLoadConfigSourceDirectory().")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(_rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	loggerClient().error(formatLogMessage(instanceName() , temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit RefreshMsg. Reason: ")
				.append(ReactorReturnCodes.toString(retCode))
				.append(". Error text: ")
				.append(_rsslErrorInfo.error().text());
				
			handleInvalidUsage(temp.toString());
	    }
		
		return retCode;
	}
	
	void reLoadConfigSourceDirectory()
	{
		if ( _activeConfig.directoryAdminControl != OmmNiProviderConfig.AdminControl.API_CONTROL )
			return;
		
		DirectoryRefresh directoryRefresh = _activeConfig.directoryConfig.getDirectoryRefresh();
		
		if ( directoryRefresh.serviceList().size() == 0 )
			return;
		
		if ( loggerClient().isTraceEnabled() )
		{
			loggerClient().trace( formatLogMessage(_activeConfig.instanceName, "Reload of configured source directories.", Severity.TRACE) );
		}
		
		if( _activeConfig.removeItemsOnDisconnect )
		{
			remapServiceIdAndServcieName(directoryRefresh);
		}
		
		_bIsStreamIdZeroRefreshSubmitted = true;
		
		if ( loggerClient().isTraceEnabled() )
		{
			loggerClient().trace( formatLogMessage(_activeConfig.instanceName, "Configured source directoies were sent out on the wire after reconnect.", Severity.TRACE) );
		}
	}
	
	void reLoadUserSubmitSourceDirectory()
	{
		if ( !_activeConfig.recoverUserSubmitSourceDirectory )
			return;
		
		DirectoryRefresh directoryRefresh = directoryServiceStore.getDirectoryRefreshMsg();
		
		if ( directoryRefresh.serviceList().size() == 0 )
			return;
		
		if ( loggerClient().isTraceEnabled() )
		{
			loggerClient().trace( formatLogMessage(_activeConfig.instanceName, "Reload of user submitted source directories.", Severity.TRACE) );
		}
		
		if ( submitDirectoryRefresh(directoryRefresh) == ReactorReturnCodes.SUCCESS )
		{
			_bIsStreamIdZeroRefreshSubmitted = true;
			
			if ( loggerClient().isTraceEnabled() )
			{
				loggerClient().trace( formatLogMessage(_activeConfig.instanceName, "User submitted source directoies were sent out on the wire after reconnect.", Severity.TRACE) );
			}
		}
	}
	
	@Override
	void reLoadDirectory()
	{
		reLoadConfigSourceDirectory();
		reLoadUserSubmitSourceDirectory();
	}
	
	class StreamInfo extends VaNode
	{
		private int _streamId;
		private int _serviceId;
		
		StreamInfo(int streamId)
		{
			_streamId = streamId;
			_serviceId = 0;
		}
		
		StreamInfo(int streamId, int serviceId)
		{
			_streamId = streamId;
			_serviceId = serviceId;
		}
		
		void set(int streamId)
		{
			_streamId = streamId;
		}
		
		void set(int streamId, int serviceId)
		{
			_streamId = streamId;
			_serviceId = serviceId;
		}
		
		void clear()
		{
			_streamId = 0;
			_serviceId = 0;
		}
		
		StreamInfo(StreamInfo other)
		{
			_streamId = other._streamId;
			_serviceId = other._serviceId;
		}
		
		int streamId()
		{
			return _streamId;
		}
		
		int serviceId()
		{
			return _serviceId;
		}
	}
}
