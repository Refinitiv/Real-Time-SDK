///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.access.OmmNiProviderImpl.StreamInfo;
import com.refinitiv.ema.access.OmmNiProviderImpl.StreamType;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;

class PackedMsgImpl implements PackedMsg {
	private TransportBuffer transportBuffer;
	private ReactorChannel reactorChannel;
	private int maxSize;
	private int remainingSize;
	private int packedMsgCount;
	private long clientHandle;
	private long itemHandle;
	private OmmIProviderImpl iProviderImpl;
	private OmmNiProviderImpl niProviderImpl;
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private ReactorErrorInfo errorInfo;
	EncodeIterator encIter;

	final static int DEFAULT_MAX_SIZE = 6000;
	
	public PackedMsgImpl(OmmProvider provider) {
		errorInfo = ReactorFactory.createReactorErrorInfo();
		
		if (provider instanceof OmmIProviderImpl)
		{
			iProviderImpl = (OmmIProviderImpl)provider;
		}
		else if (provider instanceof OmmNiProviderImpl)
		{
			niProviderImpl = (OmmNiProviderImpl)provider;
		}
		maxSize = DEFAULT_MAX_SIZE;
	}

	// Retrieves the reactor channel and gets transportBuffer for niProvider applications
	public PackedMsg initBuffer() {
		clear();
		maxSize = DEFAULT_MAX_SIZE;
		remainingSize = maxSize;

		if (iProviderImpl != null)
		{
			String temp = "This method is used for Non-Interactive Provider only. Setting a client handle with initBuffer(long clientHandle) is required when using an Interactive Provider." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		else if (niProviderImpl != null)
		{
			this.reactorChannel = niProviderImpl._channelCallbackClient.channelList().get(0).rsslReactorChannel();
		}
		
		getBuffer();
		return this;
	}
	
	// Retrieves the reactor channel and gets transportBuffer for niProvider applications
	public PackedMsg initBuffer(int maxSize) {
		clear();
		this.maxSize = maxSize;
		remainingSize = maxSize;
		
		if (iProviderImpl != null)
		{
			String temp = "This method is used for Non-Interactive Provider only. Setting a client handle with initBuffer(long clientHandle, int maxSize) is required when using an Interactive Provider." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		else if (niProviderImpl != null)
		{
			this.reactorChannel = niProviderImpl._channelCallbackClient.channelList().get(0).rsslReactorChannel();
		}
		
		getBuffer();
		return this;
	}
	
	// Sets the client handle associated with this PackedMsg for IProvider to retrieve the channel and transportBuffer
	public PackedMsg initBuffer(long clientHandle) {
		clear();
		ClientSession clientSession;
		this.clientHandle = clientHandle;
		maxSize = DEFAULT_MAX_SIZE;
		remainingSize = maxSize;

		if (iProviderImpl != null)
		{
			LongObject handleObject = new LongObject();
			handleObject.value(clientHandle);
			clientSession = iProviderImpl.serverChannelHandler().getClientSession(handleObject);

			this.reactorChannel = clientSession.channel();
		}
		else if (niProviderImpl != null)
		{
			String temp = "This method is used for Interactive Provider only. Using initBuffer() is required when using a Non-Interactive Provider, as it does not use a client handle." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}

		getBuffer();
		return this;
	}
	
	// Sets the client handle associated with this PackedMsg for IProvider to retrieve the channel and transportBuffer
	public PackedMsg initBuffer(long clientHandle, int maxSize) {
		clear();
		ClientSession clientSession;
		this.clientHandle = clientHandle;
		this.maxSize = maxSize;
		remainingSize = maxSize;

		if (iProviderImpl != null)
		{
			LongObject handleObject = new LongObject();
			handleObject.value(clientHandle);
			clientSession = iProviderImpl.serverChannelHandler().getClientSession(handleObject);

			this.reactorChannel = clientSession.channel();
		}
		else if (niProviderImpl != null)
		{
			String temp = "This method is used for Interactive Provider only. Using initBuffer(int maxSize) is required when using a Non-Interactive Provider, as it does not use a client handle." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		
		getBuffer();
		
		return this;
	}
	
	void getBuffer()
	{
		if (errorInfo == null)
			errorInfo = ReactorFactory.createReactorErrorInfo();
		
		if (reactorChannel.channel() == null)
		{
			String temp = "Failed to retrieve transport buffer. No active channel exists." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL);
		}
		transportBuffer = reactorChannel.getBuffer(maxSize, true, errorInfo);
		if (transportBuffer == null)
		{
			String temp = "Failed to retrieve transport buffer from channel." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.NO_BUFFERS);
		}
	}
	
	public PackedMsg addMsg(Msg msg, long handle) {
		// If this reactorChannel has no channel set, our connection is not established anymore
		if (reactorChannel.channel() == null)
		{
			String temp = "addMsg() failed because connection is not established." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL);
		}
		
		if (handle == 0)
		{
			String temp = "Item handle must be set when calling addMsg()." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		
		if (transportBuffer == null)
		{
			String temp = "addMsg() failed because initBuffer() was not called." ;
			throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
		}
		
		itemHandle = handle;
		
		int retCode = 0;
		MsgImpl msgImpl = (MsgImpl)msg;
		
		if (encIter == null)
			encIter = CodecFactory.createEncodeIterator();
		else
			encIter.clear();
		
		encIter.setBufferAndRWFVersion(transportBuffer, reactorChannel.majorVersion(), reactorChannel.minorVersion());
		
		// Set StreamId of the message
		ItemInfo itemInfo = null;
		boolean niProviderStreamAdded = false;	// Check if stream was added for NiProvider
		StreamInfo niProviderStream = null;
		
		if (iProviderImpl != null)
		{
			iProviderImpl.userLock().lock();
			itemInfo = iProviderImpl.getItemInfo(handle);
			if (itemInfo == null)	// Stream is down
			{
				releaseBuffer();
				String temp = "No item info exists for this handle." ;
				throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
			}
			iProviderImpl.userLock().unlock();
			
			if (msgImpl == null || msgImpl._rsslMsg == null)
			{
				releaseBuffer();
				String temp = "Incoming message to pack was null." ;
				throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_USAGE);
			}
			msgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			if (msgImpl.hasServiceName())
			{
				if ((iProviderImpl.directoryServiceStore().serviceId(msgImpl.serviceName())) != null)
				{
					msgImpl.msgServiceId(iProviderImpl.directoryServiceStore().serviceId(msgImpl.serviceName()).value());
				}
				else
				{
					releaseBuffer();
					String temp = "Attempt to add " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + " with service name of " + msgImpl.serviceName() +
							" that was not included in the SourceDirectory. Dropping this " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + ".";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
			}
			else if (msgImpl.hasServiceId())
			{
				if ( iProviderImpl.directoryServiceStore().serviceName(msgImpl.serviceId()) == null )
				{
					releaseBuffer();
					String temp = "Attempt to add " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + " with service id of " + msgImpl.serviceId() +
							" that was not included in the SourceDirectory. Dropping this " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + ".";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
			}
		}
		else if (niProviderImpl != null)
		{
			StreamInfo streamInfo = niProviderImpl.getStreamInfo(handle);
			
			if (streamInfo != null)
			{
				 msgImpl._rsslMsg.streamId(niProviderImpl.getStreamInfo(handle).streamId());
			}
			else
			{
				streamInfo = (StreamInfo)niProviderImpl._objManager._streamInfoPool.poll();
		    	if (streamInfo == null)
		    	{
		    		streamInfo = niProviderImpl.new StreamInfo(StreamType.PROVIDING, niProviderImpl.nextProviderStreamId());
		    		streamInfo.handle(handle);
		    		niProviderImpl._objManager._streamInfoPool.updatePool(streamInfo);
		    	}
		    	else
		    	{
		    		streamInfo.clear();
		    		streamInfo.set(StreamType.PROVIDING, niProviderImpl.nextProviderStreamId());
		    		streamInfo.handle(handle);
		    	}
		    	
		    	msgImpl._rsslMsg.streamId(streamInfo.streamId());
				niProviderImpl._handleToStreamInfo.put(streamInfo.handle(), streamInfo);
				niProviderStreamAdded = true;
				niProviderStream = streamInfo;
			}
			
			if (msgImpl.hasServiceName())
			{
				if ((niProviderImpl.directoryServiceStore().serviceId(msgImpl.serviceName())) != null)
				{
					msgImpl.msgServiceId(niProviderImpl.directoryServiceStore().serviceId(msgImpl.serviceName()).value());
				}
				else
				{
					releaseBuffer();
					String temp = "Attempt to add " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + " with service name of " + msgImpl.serviceName() +
							" that was not included in the SourceDirectory. Dropping this " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + ".";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
			}
			else if (msgImpl.hasServiceId())
			{		
				if ( niProviderImpl.directoryServiceStore().serviceName(msgImpl.serviceId()) == null )
				{
					releaseBuffer();
					String temp = "Attempt to add " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + " with service id of " + msgImpl.serviceId() +
							" that was not included in the SourceDirectory. Dropping this " + DataType.asString(Utilities.toEmaMsgClass[msgImpl._rsslMsg.msgClass()]) + ".";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
			}
		}

		retCode = msgImpl._rsslMsg.encode(encIter);
		if (retCode < CodecReturnCodes.SUCCESS)
		{
			if (niProviderStreamAdded && niProviderStream != null)
			{
				niProviderImpl._handleToStreamInfo.remove(niProviderStream.handle());
				niProviderStream.returnToPool();
				niProviderStream = null;
			}
			
			if (retCode == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				String temp = "Not enough space remaining in this PackedMsg buffer." ;
				throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.BUFFER_TOO_SMALL);
			}
			else
			{
				releaseBuffer();
				String temp = "Failed to encode message during addMsg()." ;
				throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE);
			}
		}

		if ((retCode = reactorChannel.packBuffer(transportBuffer, errorInfo)) < ReactorReturnCodes.SUCCESS) 
		{
			if (niProviderStreamAdded && niProviderStream != null)
			{
				niProviderImpl._handleToStreamInfo.remove(niProviderStream.handle());
				niProviderStream.returnToPool();
				niProviderStream = null;
			}
			
			String temp;
			switch(errorInfo.code())
			{
				case ReactorReturnCodes.FAILURE:
					releaseBuffer();
					temp = "Failed to pack buffer during addMsg().";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE);
				case ReactorReturnCodes.SHUTDOWN:
					releaseBuffer();
					temp = "Failed to pack buffer during addMsg().";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.CHANNEL_ERROR);
				default: 
					releaseBuffer();
					temp = "Failed to pack buffer during addMsg().";
					throw ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE);
			}
		}
		
		remainingSize = retCode;	// Set remainingSize to our return from packBuffer
		packedMsgCount++;

		return this;
	}

	public int remainingSize() {
		if (transportBuffer == null)
			return -1;
		return remainingSize;
	}

	public int packedMsgCount() {
		return packedMsgCount;
	}

	public int maxSize() {
		return maxSize;
	}

	public PackedMsg clear() {
		remainingSize = 0;
		packedMsgCount = 0;
		releaseBuffer();
		return this;
	}
	
	public TransportBuffer getTransportBuffer()
	{
		return transportBuffer;
	}
	
	public void setTransportBuffer(TransportBuffer transportBuffer)
	{
		this.transportBuffer = transportBuffer;
	}
	
	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _ommIUExcept;
	}
	
	long getClientHandle()
	{
		return clientHandle;
	}
	
	long getItemHandle()
	{
		return itemHandle;
	}
	
	void releaseBuffer()
	{
		if (transportBuffer != null)
		{
			reactorChannel.releaseBuffer(transportBuffer, errorInfo);
			transportBuffer = null;
		}
	}
}
