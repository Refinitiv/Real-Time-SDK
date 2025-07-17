/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.ema.access.DirectoryServiceStore.ServiceIdInteger;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.refinitiv.eta.valueadd.reactor.RDMDictionaryMsgCallback;
import com.refinitiv.eta.valueadd.reactor.RDMDictionaryMsgEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

class DictionaryCallbackClient<T> extends CallbackClient<T> implements RDMDictionaryMsgCallback
{
	private static final String CLIENT_NAME 				= "DictionaryCallbackClient";
	private static final int MAX_DICTIONARY_BUFFER_SIZE 	= 448000;
	protected static final String DICTIONARY_RWFFID = "RWFFld";
	protected static final String DICTIONARY_RWFENUM = "RWFEnum";
	
	private List<ChannelDictionary<T>>						_channelDictList;
	private ArrayDeque<ChannelDictionary<T>>							_channelDictPool;
	private com.refinitiv.eta.codec.DataDictionary		_rsslLocalDictionary;
	private ChannelDictionary<T> 						_channelDictionary;
	private com.refinitiv.eta.codec.Buffer 			_rsslEncBuffer;
	private com.refinitiv.eta.transport.Error			_rsslError;
	private com.refinitiv.eta.codec.Int 				_rsslCurrentFid;
	private OmmBaseImpl<T>									_ommBaseImpl;
	
	DictionaryCallbackClient(OmmBaseImpl<T> baseImpl)
	{
		super(baseImpl, CLIENT_NAME);
		
		_ommBaseImpl = baseImpl;
	}
	
	void initialize()
	{
		if (_ommBaseImpl.activeConfig().rsslFldDictRequest != null && _ommBaseImpl.activeConfig().rsslEnumDictRequest != null)
			_ommBaseImpl.activeConfig().dictionaryConfig.isLocalDictionary = false;
		else if (_ommBaseImpl.activeConfig().rsslFldDictRequest != null && _ommBaseImpl.activeConfig().rsslEnumDictRequest == null)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			temp.append("Invalid dictionary configuration was specified through the addAdminMsg() method")
				.append(OmmLoggerClient.CR)
				.append("Enumeration type definition request message was not populated.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
												Severity.ERROR).toString());

			throw (_ommBaseImpl.ommIUExcept().message(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT));
		}
		else if (_ommBaseImpl.activeConfig().rsslFldDictRequest == null && _ommBaseImpl.activeConfig().rsslEnumDictRequest != null)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			temp.append("Invalid dictionary configuration was specified through the addAdminMsg() method")
				.append(OmmLoggerClient.CR)
				.append("RDM Field Dictionary request message was not populated.");
			
			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
												Severity.ERROR).toString());

			throw (_ommBaseImpl.ommIUExcept().message(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT));
		}

		com.refinitiv.ema.rdm.DataDictionary dictionary = _ommBaseImpl.activeConfig().dictionaryConfig.dataDictionary;
		if((dictionary instanceof DataDictionaryImpl) &&
				((DataDictionaryImpl) dictionary).rsslDataDictionary() != null)
		{
			_rsslLocalDictionary = ((DataDictionaryImpl) dictionary).rsslDataDictionary();
		}
		else if (_ommBaseImpl.activeConfig().dictionaryConfig.isLocalDictionary)
			loadDictionaryFromFile();
		else
		{
			_channelDictList = new ArrayList<>();
			_channelDictPool = new ArrayDeque<>();
			_channelDictionary = new ChannelDictionary<T>(_ommBaseImpl);
			_channelDictPool.add(_channelDictionary);
		}
	}

	@SuppressWarnings("unchecked")
	@Override
	public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
	{
		_baseImpl.eventReceived();
		
		if (_channelDictList != null)
		{
			for (ChannelDictionary<T> entry : _channelDictList)
			{
				if (entry ==  event.streamInfo().userSpecObject())
				{
					return entry.processCallback(event);
				}
			}
		}
			
		return processCallback(event,(DictionaryItem<T>) (event.streamInfo().userSpecObject()));
	}
	
	int processCallback(RDMDictionaryMsgEvent event, DictionaryItem<T> item)
	{
		Msg msg = event.msg();
		ReactorChannel rsslChannel = event.reactorChannel();
		
		if (msg == null)
		{
        	if (_baseImpl.loggerClient().isErrorEnabled())
        	{
        		_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DictionaryCallbackClient.CLIENT_NAME,
	        			"Internal error. Received RsslRDMDictionaryMsgEvent with no RsslRDMDictionaryMsg in DictionaryCallbackClient.processCallback()", Severity.ERROR));
			}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		_eventImpl._channel = rsslChannel;
		
		switch (msg.msgClass())
		{
			case MsgClasses.REFRESH:
			{	
				if (_refreshMsg == null)
					_refreshMsg = new RefreshMsgImpl(_baseImpl.objManager());
				
				_refreshMsg.decode(msg, rsslChannel.majorVersion(), rsslChannel.minorVersion(), null);
				
				processRefreshMsg(_refreshMsg, item );
				
				return ReactorCallbackReturnCodes.SUCCESS;
			}
			case MsgClasses.STATUS:
			{	
				if (_statusMsg == null)
					_statusMsg = new StatusMsgImpl(_baseImpl.objManager());
				
				_statusMsg.decode(msg, rsslChannel.majorVersion(), rsslChannel.minorVersion(), null );
				
				processStatusMsg(_statusMsg, item );
				
				return ReactorCallbackReturnCodes.SUCCESS;
			}
			default:
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					
					StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("Received unknown RDMDictionary message type").append(OmmLoggerClient.CR)
		        		.append("message type ").append(msg.msgClass()).append(OmmLoggerClient.CR)
		        		.append("streamId ").append(msg.streamId()).append(OmmLoggerClient.CR);
	        	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DictionaryCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	}
				break;
			}
		}

		return ReactorCallbackReturnCodes.SUCCESS;
}
	
	void loadDictionaryFromFile()
	{
		if (_rsslLocalDictionary == null)
			_rsslLocalDictionary = CodecFactory.createDataDictionary();
		else
			_rsslLocalDictionary.clear();

		rsslError();
		if (_rsslLocalDictionary.loadFieldDictionary(_ommBaseImpl.activeConfig().dictionaryConfig.rdmfieldDictionaryFileName, _rsslError) < 0)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				temp.append("Unable to load RDMFieldDictionary from file named ")
					.append(_ommBaseImpl.activeConfig().dictionaryConfig.rdmfieldDictionaryFileName)
					.append(OmmLoggerClient.CR)
					.append("Current working directory ")
					.append(System.getProperty("user.dir"))  
					.append(OmmLoggerClient.CR)
					.append("Error text ")
					.append(_rsslError.toString());
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
			}
			
			throw (_ommBaseImpl.ommIUExcept().message(temp.toString(), _rsslError.errorId()));
		}
		
		if (_rsslLocalDictionary.loadEnumTypeDictionary(_ommBaseImpl.activeConfig().dictionaryConfig.enumtypeDefFileName, _rsslError) < 0)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				temp.append(_ommBaseImpl.activeConfig().dictionaryConfig.enumtypeDefFileName)
					.append(OmmLoggerClient.CR)
					.append("Current working directory ")
					.append(System.getProperty("user.dir"))  
					.append(OmmLoggerClient.CR)
					.append("Error text ")
					.append(_rsslError.toString());
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
			}
			
			throw (_ommBaseImpl.ommIUExcept().message(temp.toString(), _rsslError.errorId()));
		}

		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Successfully loaded local dictionaries: ")
				.append(OmmLoggerClient.CR)
				.append("RDMFieldDictionary file named ")
				.append(_ommBaseImpl.activeConfig().dictionaryConfig.rdmfieldDictionaryFileName)
				.append(OmmLoggerClient.CR)
				.append("EnumTypeDef file named ")
				.append(_ommBaseImpl.activeConfig().dictionaryConfig.enumtypeDefFileName);
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																		Severity.TRACE).toString());
		}

	}
	
	void processRefreshMsg(RefreshMsgImpl refreshMsg, DictionaryItem<T> dictItem)
	{	
		if ( refreshMsg.hasServiceId() )
		{
			specifyServiceNameFromId(refreshMsg);
		}

		_eventImpl._item = dictItem;
		
		notifyOnAllMsg(refreshMsg);
		notifyOnRefreshMsg();

		if (refreshMsg.state().streamState() == OmmState.StreamState.NON_STREAMING)
		{
			if (refreshMsg.complete())
				_eventImpl._item.remove();
		}
		else if (refreshMsg.state().streamState() != OmmState.StreamState.OPEN)
			_eventImpl._item.remove();

		return;
	}
	
	void processStatusMsg(StatusMsgImpl statusMsg, DictionaryItem<T> dictItem)
	{	
		if ( statusMsg.hasServiceId() )
		{
			specifyServiceNameFromId(statusMsg);
		}

		_eventImpl._item = dictItem;
		
		notifyOnAllMsg(statusMsg);
		notifyOnStatusMsg();

		if (statusMsg.state().streamState() != OmmState.StreamState.OPEN)
			_eventImpl._item.remove();

		return;
	}
	
	private void specifyServiceNameFromId(MsgImpl msgImpl)
	{
		Directory<T> directory = _ommBaseImpl.directoryCallbackClient().directory(msgImpl._rsslMsg.msgKey().serviceId());
			
		if ( directory != null )
		{
			int flags = msgImpl._rsslMsg.msgKey().flags(); 
			
			flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
		
			msgImpl._rsslMsg.msgKey().flags(flags);

			msgImpl.msgServiceName(directory.serviceName());

			msgImpl._rsslMsg.msgKey().flags( flags | MsgKeyFlags.HAS_SERVICE_ID);
		}
	}
	
	
	ChannelDictionary<T> pollChannelDict(OmmBaseImpl<T> baseImpl)
	{
		if (_channelDictPool != null && !_channelDictPool.isEmpty())
		{
			ChannelDictionary<T> channelDict = _channelDictPool.removeFirst();
			channelDict.clear();
			
			return channelDict;
		}
		else
		{
			return (new ChannelDictionary<T>(baseImpl));
		}
	}
	
	void returnToChannelDictPool(ChannelDictionary<T> channelDict)
	{
		_channelDictPool.add(channelDict);
	}
	
	DataDictionary defaultRsslDictionary()
	{
		if (isLocalDictionary()) {
			return _rsslLocalDictionary;
		}
		return _channelDictionary.rsslDictionary();
	}
	
	int fldStreamId()
	{
		if (_channelDictList != null && !_channelDictList.isEmpty())
			return _channelDictList.get(0).fldStreamId();
		else
			return 3;
	}
	
	int enumStreamId()
	{
		if (_channelDictList != null && !_channelDictList.isEmpty())
			return _channelDictList.get(0).enumStreamId();
		else
			return 4;
	}
	
	List< ChannelDictionary<T> > channelDictionaryList()
	{
		return _channelDictList;
	}
	
	DictionaryItem<T> dictionaryItem(ReqMsg reqMsg , T client , Object obj)
	{
		return null;
	}

	boolean downloadDictionary(Directory<T> directory, ChannelDictionary<T> dictionary)
	{
		if (_ommBaseImpl.activeConfig().dictionaryConfig.isLocalDictionary)
		{
			if (_rsslLocalDictionary != null && _rsslLocalDictionary.numberOfEntries() > 0)
			{
				if (directory.channelInfo().getParentChannel() != null)
				{
					directory.channelInfo().getParentChannel().rsslDictionary(_rsslLocalDictionary);
				}
				else
					directory.channelInfo().rsslDictionary(_rsslLocalDictionary);
			}
				
			return true;
		}
		
		if (directory.channelInfo().rsslDictionary() != null || _ommBaseImpl.activeConfig().dictionaryConfig.isLocalDictionary)
			return true;
		
		if (_ommBaseImpl.activeConfig().rsslFldDictRequest != null && _ommBaseImpl.activeConfig().rsslEnumDictRequest != null)
		{
			if (_ommBaseImpl.activeConfig().rsslFldDictRequest.serviceId() == directory.service().serviceId() ||
					( _ommBaseImpl.activeConfig().fldDictReqServiceName != null && _ommBaseImpl.activeConfig().fldDictReqServiceName.equals(directory.serviceName())))
				downloadDictionaryFromService(directory, dictionary);
			
			return true;
		}
		
		com.refinitiv.eta.codec.RequestMsg  rsslRequestMsg = rsslRequestMsg();
		
		rsslRequestMsg.domainType(DomainTypes.DICTIONARY);
		rsslRequestMsg.containerType(com.refinitiv.eta.codec.DataTypes.NO_DATA);
		rsslRequestMsg.applyStreaming();
		MsgKey msgKey = rsslRequestMsg.msgKey();
		msgKey.applyHasName();
		msgKey.applyHasFilter();
		msgKey.filter(com.refinitiv.eta.rdm.Dictionary.VerbosityValues.NORMAL);

		//ChannelDictionary<T> dictionary = pollChannelDict(_ommBaseImpl);
		dictionary.channelInfo(directory.channelInfo());
		
		ReactorSubmitOptions rsslSubmitOptions = _ommBaseImpl.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _ommBaseImpl.rsslErrorInfo();
		ReactorChannel rsslChannel = directory.channelInfo().rsslReactorChannel();
		
		rsslSubmitOptions.serviceName(directory.serviceName());
		rsslSubmitOptions.requestMsgOptions().userSpecObj(dictionary);

		int streamId = 3;

		List<String> dictionariesUsed = directory.service().info().dictionariesUsedList();
		for (String dictName : dictionariesUsed)
		{
			if(!directory.service().info().dictionariesProvidedList().contains(dictName)){
				continue;
			}
			msgKey.name().data(dictName);
			rsslRequestMsg.streamId(streamId++);

	        rsslErrorInfo.clear();
	        if (ReactorReturnCodes.SUCCESS > rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo))
	        {
	        	if (_baseImpl.loggerClient().isErrorEnabled())
				{
					com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
					
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Internal error: rsslChannel.submit() failed").append(OmmLoggerClient.CR)
						.append(directory.channelInfo().toString()).append(OmmLoggerClient.CR)
						.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)).append(OmmLoggerClient.CR)
						.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
						.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
						.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
						.append("Error Text ").append(error.text());
					
					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																				Severity.ERROR).toString());
					
					returnToChannelDictPool(dictionary);
				}
				
				return false;
			}
			else
			{
				if (_baseImpl.loggerClient().isTraceEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Requested Dictionary ")
						.append(dictName).append(OmmLoggerClient.CR)
						.append("from Service ").append(directory.serviceName()).append(OmmLoggerClient.CR)
						.append("on Channel ").append(OmmLoggerClient.CR)
						.append(directory.channelInfo().toString());
					_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																				Severity.TRACE).toString());
				}
			}
		}

		_channelDictList.add(dictionary);

		return true;
	}
	
	boolean downloadDictionaryFromService(Directory<T> directory, ChannelDictionary<T> dictionary)
	{
		com.refinitiv.eta.codec.RequestMsg rsslRequestMsg = rsslRequestMsg();
		
		rsslRequestMsg.domainType(DomainTypes.DICTIONARY);
		rsslRequestMsg.containerType(com.refinitiv.eta.codec.DataTypes.NO_DATA);
		
		DictionaryRequest rsslDictRequest = _ommBaseImpl.activeConfig().rsslFldDictRequest;
		if (rsslDictRequest.checkStreaming())
			rsslRequestMsg.applyStreaming();
		
		MsgKey msgKey = rsslRequestMsg.msgKey();
		msgKey.applyHasName();
		msgKey.applyHasFilter();
		msgKey.filter(rsslDictRequest.verbosity());
		msgKey.name(rsslDictRequest.dictionaryName());
		rsslRequestMsg.streamId(3);

		//ChannelDictionary<T> dictionary = pollChannelDict(_ommBaseImpl);
		dictionary.channelInfo(directory.channelInfo());
		
		ReactorSubmitOptions rsslSubmitOptions = _ommBaseImpl.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _ommBaseImpl.rsslErrorInfo();
		ReactorChannel rsslChannel = directory.channelInfo().rsslReactorChannel();
		
		rsslSubmitOptions.serviceName(directory.serviceName());
		rsslSubmitOptions.requestMsgOptions().userSpecObj(dictionary);

        rsslErrorInfo.clear();
        if (ReactorReturnCodes.SUCCESS > rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo))
        {
        	if (_baseImpl.loggerClient().isErrorEnabled())
			{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Internal error: rsslChannel.submit() failed").append(OmmLoggerClient.CR)
					.append(directory.channelInfo().toString()).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
					.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
					.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
					.append("Error Text ").append(error.text());
				
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
				
				returnToChannelDictPool(dictionary);
			}
			
			return false;
		}
		else
		{
			if (_baseImpl.loggerClient().isTraceEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Requested Dictionary ")
					.append(rsslDictRequest.dictionaryName().toString()).append(OmmLoggerClient.CR)
					.append("from Service ").append(directory.serviceName()).append(OmmLoggerClient.CR)
					.append("on Channel ").append(OmmLoggerClient.CR)
					.append(directory.channelInfo().toString());
				_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.TRACE).toString());
			}
		}
	
        rsslRequestMsg.clear();
        rsslRequestMsg.msgClass(MsgClasses.REQUEST);
        rsslRequestMsg.domainType(DomainTypes.DICTIONARY);
        rsslRequestMsg.containerType(com.refinitiv.eta.codec.DataTypes.NO_DATA);
		
		DictionaryRequest rsslEnumDictRequest = _ommBaseImpl.activeConfig().rsslEnumDictRequest;
		if (rsslEnumDictRequest.checkStreaming())
			rsslRequestMsg.applyStreaming();
		
		msgKey = rsslRequestMsg.msgKey();
		msgKey.applyHasName();
		msgKey.applyHasFilter();
		msgKey.filter(rsslEnumDictRequest.verbosity());
		msgKey.name(rsslEnumDictRequest.dictionaryName());
		rsslRequestMsg.streamId(4);

        rsslErrorInfo.clear();
        if (ReactorReturnCodes.SUCCESS > rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo))
        {
        	if (_baseImpl.loggerClient().isErrorEnabled())
			{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Internal error: rsslChannel.submit() failed").append(OmmLoggerClient.CR)
					.append(directory.channelInfo().toString()).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
					.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
					.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
					.append("Error Text ").append(error.text());
				
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
				
				returnToChannelDictPool(dictionary);
			}
			
			return false;
		}
		else
		{
			if (_baseImpl.loggerClient().isTraceEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Requested Dictionary ")
					.append(rsslDictRequest.dictionaryName().toString()).append(OmmLoggerClient.CR)
					.append("from Service ").append(directory.serviceName()).append(OmmLoggerClient.CR)
					.append("on Channel ").append(OmmLoggerClient.CR)
					.append(directory.channelInfo().toString());
				_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.TRACE).toString());
			}
		}

		_channelDictList.add(dictionary);

		return true;
	}
	
	boolean isDictionaryReady()
	{
		if (_rsslLocalDictionary != null && _rsslLocalDictionary.numberOfEntries() > 0
				&& _rsslLocalDictionary.enumTableCount() > 0)
			return true;
		else
		{
			if (_channelDictList == null || _channelDictList.isEmpty())
				return false;

			for (ChannelDictionary<T> entry : _channelDictList)
			{
				if (!(entry.isLoaded()))
					return false;
			}
			return true;
		}
	}
	
	boolean isLocalDictionary()
	{
		return _rsslLocalDictionary == null ? false : true;
	}
	
	Int rsslCurrentFid()
	{
		if (_rsslCurrentFid == null)
			_rsslCurrentFid = CodecFactory.createInt();
		else
			_rsslCurrentFid.clear();
		
		return _rsslCurrentFid;
	}
	
	Buffer rsslDictEncBuffer()
	{
		if (_rsslEncBuffer == null)
		{
			_rsslEncBuffer = CodecFactory.createBuffer();
			_rsslEncBuffer.data(ByteBuffer.allocate(MAX_DICTIONARY_BUFFER_SIZE));
		}
		else
		{
			ByteBuffer byteBuf = _rsslEncBuffer.data();
        	byteBuf.clear();
        	_rsslEncBuffer.data(byteBuf, 0, byteBuf.capacity()); 
		}
		
		return _rsslEncBuffer;
	}
	
	com.refinitiv.eta.transport.Error rsslError()
	{
		if (_rsslError == null)
			_rsslError = com.refinitiv.eta.transport.TransportFactory.createError();
		else
			_rsslError.clear();
		
		return _rsslError;
	}
}

class DictionaryCallbackClientConsumer extends DictionaryCallbackClient<OmmConsumerClient>
{
	DictionaryCallbackClientConsumer(OmmBaseImpl<OmmConsumerClient> baseImpl) {
		super(baseImpl);
	}
	
	@Override
	void notifyOnAllMsg(com.refinitiv.ema.access.Msg msg)
	{
		_eventImpl._item.client().onAllMsg(msg, _eventImpl);
	}
	
	@Override
    void notifyOnRefreshMsg()
	{
		_eventImpl._item.client().onRefreshMsg(_refreshMsg, _eventImpl);
	}
	
	@Override
	void notifyOnStatusMsg() 
	{
		_eventImpl._item.client().onStatusMsg(_statusMsg, _eventImpl);
	}
}

class DictionaryCallbackClientProvider extends DictionaryCallbackClient<OmmProviderClient>
{
	DictionaryCallbackClientProvider(OmmBaseImpl<OmmProviderClient> baseImpl) {
		super(baseImpl);
		
		_eventImpl._ommProvider = (OmmProvider)baseImpl;
	}
	
	@Override
	void notifyOnAllMsg(com.refinitiv.ema.access.Msg msg)
	{
		_eventImpl._item.client().onAllMsg(msg, _eventImpl);
	}
	
	@Override
    void notifyOnRefreshMsg()
	{
		_eventImpl._item.client().onRefreshMsg(_refreshMsg, _eventImpl);
	}
	
	@Override
	void notifyOnStatusMsg() 
	{
		_eventImpl._item.client().onStatusMsg(_statusMsg, _eventImpl);
	}
}

class ChannelDictionary<T>
{
	private static final String CLIENT_NAME = "ChannelDictionary";
	
	private OmmBaseImpl<T>				_baseImpl;
	private ChannelInfo					_channelInfo;
	private boolean						_isFldLoaded;
	private boolean						_isEnumLoaded;
	private int 						_fldStreamId;
	private int 						_enumStreamId;
	private ReentrantLock 				_channelDictLock;
	private List<DictionaryItem<T>>		_listenerList;
	private DataDictionary				_rsslDictionary = CodecFactory.createDataDictionary();
	
	
	ChannelDictionary(OmmBaseImpl<T> baseImpl)
	{
		_baseImpl = baseImpl;
	}
	
	ChannelInfo channelInfo()
	{
		return _channelInfo;
	}
	
	ChannelDictionary<T> channelInfo(ChannelInfo channelInfo)
	{
		_channelInfo = channelInfo;
		_channelInfo.rsslDictionary(_rsslDictionary);
		return this;
	}
	
	ChannelDictionary<T> clear()
	{
		_channelInfo = null;
		_isFldLoaded = false;
		_isEnumLoaded = false;
		_fldStreamId = 0;
		_enumStreamId = 0;
		_rsslDictionary.clear();
		
		if (_listenerList != null && _listenerList.size() > 0)
		{
			for (DictionaryItem<T> entry : _listenerList)
			{
				entry.returnToPool();
			}
			
			_listenerList.clear();
		}
		
		return this;
	}
	
	DataDictionary rsslDictionary()
	{
		return _rsslDictionary;
	}

	boolean isLoaded()
	{
		return _isEnumLoaded && _isFldLoaded;
	}

	int fldStreamId()
	{
		return _fldStreamId;
	}
	
	int enumStreamId()
	{
		return _enumStreamId;
	}
	
	ReentrantLock channelDictionaryLock()
	{
		if (_channelDictLock == null)
			_channelDictLock = new java.util.concurrent.locks.ReentrantLock();
		
		return _channelDictLock;
	}
	
	void addListener(DictionaryItem<T> item)
	{
		if (_listenerList == null)
			_listenerList = new ArrayList<>();
		
		_listenerList.add(item);
	}

	void removeListener(DictionaryItem<T> item)
	{
		if (_listenerList == null || _listenerList.isEmpty())
			return;
		
		_listenerList.remove(item);
	}
	
	void notifyStatusToListener(OmmBaseImpl<T> baseImpl, com.refinitiv.eta.codec.State rsslStatus, int streamId)
	{
		channelDictionaryLock().lock();

		if (_listenerList == null || _listenerList.isEmpty())
		{
			channelDictionaryLock().unlock();
			return;
		}

		DictionaryItem<T> dictItem = null;
		ReactorChannel rsslChannel = baseImpl.rsslReactorChannel();
		EncodeIterator rsslEncIter = baseImpl.rsslEncIter();
		DictionaryCallbackClient<T> dictCallbackClient = baseImpl.dictionaryCallbackClient();
		StatusMsg rsslStatusMsg = dictCallbackClient.rsslStatusMsg();
		Buffer rsslEncDictBuf;
		
		int numOfListeners = _listenerList.size();
		for(int index = 0 ; index < numOfListeners; index++)
		{
			dictItem = _listenerList.get(index);

			if (dictItem.streamId() != streamId)
				continue;
			
			    rsslStatusMsg.msgClass(MsgClasses.STATUS);
		        rsslStatusMsg.streamId(streamId);
		        rsslStatusMsg.domainType(DomainTypes.DICTIONARY);
		        rsslStatusMsg.containerType(DataTypes.NO_DATA);
		        rsslStatusMsg.applyHasState();
			    rsslStatusMsg.state().streamState(rsslStatus.streamState());
			    rsslStatusMsg.state().dataState(rsslStatus.dataState());
			    rsslStatusMsg.state().code(rsslStatus.code());
			    rsslStatusMsg.state().text(rsslStatus.text());
			    rsslStatusMsg.applyHasMsgKey();
			    rsslStatusMsg.msgKey().applyHasName();
			    rsslStatusMsg.msgKey().name().data(dictItem.name());
			       
				rsslEncIter.clear();
				rsslEncDictBuf = dictCallbackClient.rsslDictEncBuffer();
				int retCode = rsslEncIter.setBufferAndRWFVersion(rsslEncDictBuf, rsslChannel.majorVersion(), rsslChannel.minorVersion());
				if (retCode != CodecReturnCodes.SUCCESS)
				{
					if (baseImpl.loggerClient().isErrorEnabled())
		        	{
						baseImpl.loggerClient().error(baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, 
			        			"Internal error. Failed to set encode iterator with buffer in ChannelDictionary.notifyStatusToListener()",
			        									Severity.ERROR));
		        	}
					return;
				}
				
			    if ((retCode = rsslStatusMsg.encode(rsslEncIter)) != CodecReturnCodes.SUCCESS)
			    {
			    	if (baseImpl.loggerClient().isErrorEnabled())
		        	{
			    		baseImpl.loggerClient().error(baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, 
			        			"Internal error. Failed to encode msg in ChannelDictionary.notifyStatusToListener()",
			        									Severity.ERROR));
		        	}
			    	return;
			    }
			    
			    if (dictCallbackClient._statusMsg == null)
			    	dictCallbackClient._statusMsg = new StatusMsgImpl(_baseImpl._objManager);
				
			    dictCallbackClient._statusMsg.decode(rsslEncDictBuf, rsslChannel.majorVersion(), rsslChannel.minorVersion(), null, null);
			    	
			    dictCallbackClient.processStatusMsg(dictCallbackClient._statusMsg, dictItem );
				
			if (rsslStatus.streamState() != StreamStates.OPEN)
			{
				_listenerList.remove(index);
				--index;
			}
			
			break;
		}

		channelDictionaryLock().unlock();
	}

	int processCallback(RDMDictionaryMsgEvent event)
	{
		Msg msg = event.msg();
		ReactorChannel rsslChannel = event.reactorChannel();
		ChannelInfo channelInfo = (ChannelInfo)rsslChannel.userSpecObj();
		
		if (channelInfo.getParentChannel() != null)
			channelInfo = channelInfo.getParentChannel();
		
		if (msg == null)
		{
			com.refinitiv.eta.transport.Error error = event.errorInfo().error();
        	
        	if (_baseImpl.loggerClient().isErrorEnabled())
        	{
	        	StringBuilder temp = _baseImpl.strBuilder();
				
	        	temp.append("Received event without RDMDictionary message").append(OmmLoggerClient.CR)
				    .append("ChannelInfo ").append(OmmLoggerClient.CR)
					.append(channelInfo.toString()).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(event.errorInfo().location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());

	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

        	_baseImpl.closeRsslChannel(event.reactorChannel());
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		switch (msg.msgClass())
		{
		case MsgClasses.REFRESH:
		{
			com.refinitiv.eta.codec.RefreshMsg rsslMsg = (com.refinitiv.eta.codec.RefreshMsg)msg;

			com.refinitiv.eta.codec.State state = rsslMsg.state();

			if (state.streamState() != StreamStates.OPEN  && state.streamState() != StreamStates.NON_STREAMING)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					
					StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("RDMDictionary stream was closed with refresh message").append(OmmLoggerClient.CR)
		        		.append("ChannelInfo").append(OmmLoggerClient.CR)
						.append(channelInfo.toString()).append(OmmLoggerClient.CR)
						.append("Reason ").append(state.toString());
	        	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	}
				break;
			}
			else if (state.dataState() == DataStates.SUSPECT)
			{
				if (_baseImpl.loggerClient().isWarnEnabled())
	        	{
					
					StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("RDMDictionary stream state was changed to suspect with refresh message").append(OmmLoggerClient.CR)
		        		.append("ChannelInfo").append(OmmLoggerClient.CR)
						.append(channelInfo.toString()).append(OmmLoggerClient.CR)
						.append("Reason ").append(state.toString());
	        	
		        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.WARNING));
	        	}
				break;
			}
			
			if (_baseImpl.loggerClient().isTraceEnabled())
        	{
				
				StringBuilder temp = _baseImpl.strBuilder();
	        	temp.append("Received RDMDictionary refresh message").append(OmmLoggerClient.CR)
	        		.append("Dictionary name ").append(rsslMsg.msgKey().name().toString()).append(OmmLoggerClient.CR)
					.append("streamId ").append(rsslMsg.streamId());
        	
	        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.TRACE));
        	}
			
			DictionaryRefresh rsslRefresh = (DictionaryRefresh)event.rdmDictionaryMsg();

			if (rsslRefresh.checkHasInfo())
			{
				switch (rsslRefresh.dictionaryType())
				{
					case com.refinitiv.eta.rdm.Dictionary.Types.FIELD_DEFINITIONS :
					{
						if (_fldStreamId == 0)
							_fldStreamId = rsslMsg.streamId();
						else if (_fldStreamId != rsslMsg.streamId())
						{
							if (_baseImpl.loggerClient().isErrorEnabled())
				        	{
								
								StringBuilder temp = _baseImpl.strBuilder();
					        	temp.append("Received RDMDictionary refresh message with FieldDefinitions but changed streamId")
					        		.append(OmmLoggerClient.CR)
					        		.append("Initial streamId ").append(_fldStreamId).append(OmmLoggerClient.CR)
					        		.append("New streamId ").append(rsslMsg.streamId());
				        	
					        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME,
					        									temp.toString(), Severity.ERROR));
				        	}
							return ReactorCallbackReturnCodes.SUCCESS;
						}
						break;
					}
					case com.refinitiv.eta.rdm.Dictionary.Types.ENUM_TABLES :
					{
						if (_enumStreamId == 0)
							_enumStreamId = rsslMsg.streamId();
						else if (_enumStreamId != rsslMsg.streamId())
						{
							if (_baseImpl.loggerClient().isErrorEnabled())
				        	{
								
								StringBuilder temp = _baseImpl.strBuilder();
					        	temp.append("Received RDMDictionary refresh message with EnumTables but changed streamId")
					        		.append(OmmLoggerClient.CR)
					        		.append("Initial streamId ").append(_fldStreamId).append(OmmLoggerClient.CR)
					        		.append("New streamId ").append(rsslMsg.streamId());
				        	
					        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME,
					        									temp.toString(), Severity.ERROR));
				        	}
							return ReactorCallbackReturnCodes.SUCCESS;
						}
						break;
					}
					default: 
					{
						if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
							StringBuilder temp = _baseImpl.strBuilder();
				        	temp.append("Received RDMDictionary message with unknown dictionary type").append(OmmLoggerClient.CR)
				        		.append("Dictionary type ").append(rsslRefresh.dictionaryType());
			        	
				        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME,
				        									temp.toString(), Severity.ERROR));
			        	}
						return ReactorCallbackReturnCodes.SUCCESS;
					}
				}
			}

			DecodeIterator dIter = _baseImpl.rsslDecIter();
			dIter.clear();
			if (CodecReturnCodes.SUCCESS != dIter.setBufferAndRWFVersion(rsslMsg.encodedDataBody(), rsslChannel.majorVersion(), rsslChannel.minorVersion()))
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("Internal error: failed to set buffer while decoding dictionary").append(OmmLoggerClient.CR)
		        		.append("Trying to set ").append(rsslChannel.majorVersion())
		        		.append(".").append(rsslChannel.minorVersion());;
	        	
		        		_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME,
		        									temp.toString(), Severity.ERROR));
	        	}
				return ReactorCallbackReturnCodes.SUCCESS;
			}

			com.refinitiv.eta.transport.Error rsslError = _baseImpl.dictionaryCallbackClient().rsslError();
			if (_fldStreamId == rsslMsg.streamId())
			{
				if (_isFldLoaded == true && _isEnumLoaded == true)
					_rsslDictionary.clear();
				
	    		if (CodecReturnCodes.SUCCESS == _rsslDictionary.decodeFieldDictionary(dIter, 
	    																				com.refinitiv.eta.rdm.Dictionary.VerbosityValues.VERBOSE,
	    																				rsslError))
				{
					if (rsslRefresh.checkRefreshComplete())
					{
						_isFldLoaded = true;

						if (_baseImpl.loggerClient().isTraceEnabled())
			        	{
							StringBuilder temp = _baseImpl.strBuilder();
				        	temp.append("Received RDMDictionary refresh complete message").append(OmmLoggerClient.CR)
				        		.append("dictionary name ").append(rsslRefresh.dictionaryName().toString()).append(OmmLoggerClient.CR)
								.append("streamId ").append(rsslMsg.streamId());
			        	
				        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME,
				        									temp.toString(), Severity.TRACE));
			        	}
					}
					else
						_isFldLoaded = false;
				}
				else
	    		{
					_isFldLoaded = false;

					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
						StringBuilder temp = _baseImpl.strBuilder();
			        	temp.append("Internal error: failed to decode FieldDictionary").append(OmmLoggerClient.CR)
			        		.append("Error text ").append(rsslError.text());
		        	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME,
			        									temp.toString(), Severity.ERROR));
		        	}
					
					return ReactorCallbackReturnCodes.SUCCESS;
	    		}
			}
			else if (_enumStreamId == rsslMsg.streamId())
			{
				rsslError.clear();
	    		if (CodecReturnCodes.SUCCESS == _rsslDictionary.decodeEnumTypeDictionary(dIter,
	    										com.refinitiv.eta.rdm.Dictionary.VerbosityValues.VERBOSE, rsslError))
				{
					if (rsslRefresh.checkRefreshComplete())
					{
						_isEnumLoaded = true;
						
						if (_baseImpl.loggerClient().isTraceEnabled())
			        	{
							StringBuilder temp = _baseImpl.strBuilder();
				        	temp.append("Received RDMDictionary refresh complete message").append(OmmLoggerClient.CR)
				        		.append("dictionary name ").append(rsslRefresh.dictionaryName().toString()).append(OmmLoggerClient.CR)
								.append("streamId ").append(rsslMsg.streamId());
			        	
				        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME,
				        									temp.toString(), Severity.TRACE));
			        	}
					}
					else
						_isEnumLoaded = false;
				}
				else
	    		{
					_isEnumLoaded = false;

					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
						StringBuilder temp = _baseImpl.strBuilder();
			        	temp.append("Internal error: failed to decode EnumTable dictionary").append(OmmLoggerClient.CR)
			        		.append("Error text ").append(rsslError.text());
		        	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME,
			        									temp.toString(), Severity.ERROR));
		        	}
					
					return ReactorCallbackReturnCodes.SUCCESS;
	    		}
			}
			else
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("Received unexpected RDMDictionary refresh message on streamId ").append(OmmLoggerClient.CR)
		        		.append(rsslMsg.streamId());
	        	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME,
		        									temp.toString(), Severity.ERROR));
	        	}
				
				return ReactorCallbackReturnCodes.SUCCESS;
			}

			break;
		}
		case MsgClasses.STATUS:
		{
			com.refinitiv.eta.codec.StatusMsg rsslMsg = (com.refinitiv.eta.codec.StatusMsg)msg;
			DictionaryStatus rsslStatus = (DictionaryStatus)event.rdmDictionaryMsg();
			
			if (rsslMsg.checkHasState())
			{
				State state =rsslMsg.state();

				if (state.streamState() != StreamStates.OPEN)
				{
					if (_baseImpl.loggerClient().isWarnEnabled())
		        	{
						StringBuilder temp = _baseImpl.strBuilder();
			        	temp.append("RDMDictionary stream was closed with status message").append(OmmLoggerClient.CR)
							.append("streamId ").append(rsslMsg.streamId()).append(OmmLoggerClient.CR)
							.append("Reason ").append(state.toString());
		        	
			        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					notifyStatusToListener(_baseImpl, rsslStatus.state(), rsslMsg.streamId());
					break;
				}
				else if (state.dataState() == DataStates.SUSPECT)
				{
					if (_baseImpl.loggerClient().isWarnEnabled())
		        	{
						StringBuilder temp = _baseImpl.strBuilder();
			        	temp.append("RDMDictionary stream state was changed to suspect with status message").append(OmmLoggerClient.CR)
							.append("streamId ").append(rsslMsg.streamId()).append(OmmLoggerClient.CR)
							.append("Reason ").append(state.toString());
		        	
			        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					notifyStatusToListener(_baseImpl, rsslStatus.state(), rsslMsg.streamId());
					break;
				}

				if (_baseImpl.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("RDMDictionary stream was open with status message").append(OmmLoggerClient.CR)
						.append("streamId ").append(rsslMsg.streamId()).append(OmmLoggerClient.CR)
						.append("Reason ").append(state.toString());
	        	
		        	_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.TRACE));
	        	}
			}
			else
			{
				if (_baseImpl.loggerClient().isWarnEnabled())
	        	{
					StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("Received RDMDictionary status message without the state").append(OmmLoggerClient.CR)
						.append("streamId ").append(rsslMsg.streamId()).append(OmmLoggerClient.CR);
	        	
		        	_baseImpl.loggerClient().warn(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.WARNING));
	        	}
			}
			break;
		}
		default:
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				
				StringBuilder temp = _baseImpl.strBuilder();
	        	temp.append("Received unknown RDMDictionary message type").append(OmmLoggerClient.CR)
	        		.append("message type ").append(msg.msgClass()).append(OmmLoggerClient.CR)
	        		.append("streamId ").append(msg.streamId()).append(OmmLoggerClient.CR);
        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
			break;
		}
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}
}

class DictionaryItem<T> extends SingleItem<T> implements TimeoutClient
{
	private static final String 	CLIENT_NAME = "DictionaryItem";
	
	private int	_rsslFilter;
	private int 	_currentFid;
	private boolean	_needRemoved;
	private boolean	_removed;
	private String _name;
	
	DictionaryItem(OmmBaseImpl<T> baseImpl, T client, Object closure)
	{
		super(baseImpl, client, closure, null);
		_rsslFilter = 0;
		_currentFid = 0;
		_needRemoved = false;
		_removed = false;
	}
	
	@Override
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> item)
	{
		super.reset(baseImpl, client, closure, null);
		
		_rsslFilter = 0; 
		_currentFid = 0;
		_needRemoved = false;
	}

	@Override
	boolean open(ReqMsg reqMsg)
	{
		RequestMsg rsslReqMsg = ((ReqMsgImpl)reqMsg).rsslMsg();
		_name =  rsslReqMsg.msgKey().name().toString();
		DictionaryCallbackClient<T> dictCBClient = _baseImpl.dictionaryCallbackClient();

		if (rsslReqMsg.msgKey().checkHasFilter())
			_rsslFilter = (int)rsslReqMsg.msgKey().filter();

		/* User is asking for a specific service's dictionary, submit the request to the Reactor */
		if ( reqMsg.hasServiceName() || rsslReqMsg.msgKey().checkHasServiceId() )
			return super.open( reqMsg );
		else
		{
			/* This ensures that a valid handle is assigned to the request. */
			_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);

			DataDictionary rsslDictionary = dictCBClient.defaultRsslDictionary();
	
			if (rsslDictionary != null)
			{
				/* EMA will generate the Dictionary from the cached values */  
				if (_name.equals(DictionaryCallbackClient.DICTIONARY_RWFFID))
				{
					_currentFid = rsslDictionary.minFid();
					_streamId = dictCBClient.fldStreamId();
				}
				else if (_name.equals(DictionaryCallbackClient.DICTIONARY_RWFENUM))
				{
					_currentFid = 0;
					_streamId = dictCBClient.enumStreamId();
				}
				else
				{
					StringBuilder temp = _baseImpl.strBuilder();
					
		        	temp.append("Invalid ReqMsg's name : ")
		        		.append(_name)
		        		.append("\nReqMsg's name must be \"").append(DictionaryCallbackClient.DICTIONARY_RWFFID)
		        		.append("\" or \"").append(DictionaryCallbackClient.DICTIONARY_RWFENUM).append("\" for MMT_DICTIONARY domain type. ")
						.append("Instance name='").append(_baseImpl.instanceName()).append("'.");

		        	if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
		        		_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DictionaryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	}

		        	_baseImpl.handleInvalidUsage( temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );

					return false;
				}
				
				if (!dictCBClient.isLocalDictionary())
				{
					ChannelDictionary<T> channelDict = dictCBClient.channelDictionaryList().get(0);
				
					channelDict.channelDictionaryLock().lock();
	
					channelDict.addListener(this);
	
					channelDict.channelDictionaryLock().unlock();
				}
				
	
				_baseImpl.addTimeoutEvent(500, this);
				
				return true;
			}
			else
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DictionaryItem.CLIENT_NAME,
		        										"Ema must have to receive a dictionary before open a dictionary request",
		        										Severity.ERROR));
				return false;
			}
		}
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to modify dictionary stream. ").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(DictionaryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}
	
	@Override
	boolean submit(PostMsg postMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit PostMsg on dictionary stream. ").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(DictionaryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	boolean submit(GenericMsg genericMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit GenericMsg on dictionary stream. ").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(DictionaryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	boolean close()
	{
		if (_streamId > 4)
		{
			super.close();
		}
		else
		{
			if (_needRemoved == false)
			{
				_needRemoved = true;

				DictionaryCallbackClient<T> dictCBClient = _baseImpl.dictionaryCallbackClient();
				if (dictCBClient.channelDictionaryList() != null && !dictCBClient.channelDictionaryList().isEmpty())
				{
					ChannelDictionary<T> channelDict = dictCBClient.channelDictionaryList().get(0);
				
					channelDict.channelDictionaryLock().lock();

					channelDict.removeListener(this);

					channelDict.channelDictionaryLock().unlock();
				}

				_baseImpl.addTimeoutEvent(2000, this);
			}
		}

		return true;
	}
	
	@Override
	void remove()
	{
		if (!_needRemoved)
		{
			_needRemoved = true;
			_baseImpl.addTimeoutEvent(2000, this);
		}
	}
	
	@Override
	public void handleTimeoutEvent()
	{
		if (_needRemoved)
		{
			if(!_removed) 
			{
				_baseImpl.itemCallbackClient().removeFromMap(this, true);
				this.itemIdObj().returnToPool();
				this.returnToPool();
				_removed = true;
			}
			return;
		}
		
		DictionaryCallbackClient<T> dictCallbackClient = _baseImpl.dictionaryCallbackClient();
		DataDictionary rsslDictionary = dictCallbackClient.defaultRsslDictionary();
		ReactorChannel rsslChannel = _baseImpl.rsslReactorChannel();
		boolean firstPart = false;
		int ret = CodecReturnCodes.FAILURE;
		
		Buffer rsslDictEncBuffer = _baseImpl.dictionaryCallbackClient().rsslDictEncBuffer();

		if (rsslDictionary != null && (rsslDictionary.enumTableCount() > 0 || rsslDictionary.numberOfEntries() > 0))
		{
			if (_name.equals(DictionaryCallbackClient.DICTIONARY_RWFFID))
			{
				if (_currentFid == rsslDictionary.minFid())
					firstPart = true;
				
				ret = encodeDataDictionaryResp(	firstPart, rsslDictionary, rsslDictEncBuffer);
			}
			else if (_name.equals(DictionaryCallbackClient.DICTIONARY_RWFENUM))
			{
				if (_currentFid == 0)
					firstPart = true;

				ret = encodeDataDictionaryResp(	firstPart, rsslDictionary, rsslDictEncBuffer);
			}

			if ((ret == CodecReturnCodes.SUCCESS) || (ret == CodecReturnCodes.DICT_PART_ENCODED))
			{
				if (dictCallbackClient._refreshMsg == null)
					dictCallbackClient._refreshMsg = new RefreshMsgImpl(_baseImpl._objManager);
				
				dictCallbackClient._refreshMsg.decode(rsslDictEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion(), null, null);
				
				if (ret == CodecReturnCodes.SUCCESS)
					dictCallbackClient._refreshMsg.complete(true);
				
				dictCallbackClient.processRefreshMsg(dictCallbackClient._refreshMsg, this);
			}

			if (ret == CodecReturnCodes.DICT_PART_ENCODED)
			{
				_baseImpl.addTimeoutEvent(500,  this);
				return;
			}
			
			if (ret != CodecReturnCodes.SUCCESS)
			{
				EncodeIterator rsslEncIter = _baseImpl.rsslEncIter();
				StatusMsg rsslStatusMsg = dictCallbackClient.rsslStatusMsg();
				
		        rsslStatusMsg.streamId(_streamId);
		        rsslStatusMsg.domainType(DomainTypes.DICTIONARY);
		        rsslStatusMsg.containerType(DataTypes.NO_DATA);
		        rsslStatusMsg.applyHasState();
			    rsslStatusMsg.state().streamState(StreamStates.CLOSED);
			    rsslStatusMsg.state().dataState(DataStates.SUSPECT);
			    rsslStatusMsg.state().code(StateCodes.NONE);
			    rsslStatusMsg.state().text().data("Failed to provide data dictionary: Internal error.");
			    rsslStatusMsg.applyHasMsgKey();
			    rsslStatusMsg.msgKey().applyHasName();
			    rsslStatusMsg.msgKey().name().data(_name);
			       
				rsslEncIter.clear();
				int retCode = rsslEncIter.setBufferAndRWFVersion(rsslDictEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion());
				if (retCode != CodecReturnCodes.SUCCESS)
				{
					if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
						_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DictionaryItem.CLIENT_NAME, 
			        			"Internal error. Failed to set encode iterator RWF version in DictionatyItem.handleTimeoutEvent()",
			        									Severity.ERROR));
		        	}
					return;
				}
				
			    if ((retCode = rsslStatusMsg.encode(rsslEncIter)) != CodecReturnCodes.SUCCESS)
			    {
			    	if (_baseImpl.loggerClient().isErrorEnabled())
		        	{
			    		_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DictionaryItem.CLIENT_NAME, 
			        			"Internal error. Failed to encode msg in DictionatyItem.handleTimeoutEvent()",
			        									Severity.ERROR));
		        	}
			    	return;
			    }
			    
			    if (dictCallbackClient._statusMsg == null)
			    	dictCallbackClient._statusMsg = new StatusMsgImpl(_baseImpl._objManager);
				
			    dictCallbackClient._statusMsg.decode(rsslDictEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion(), null, null);
			    	
				dictCallbackClient.processStatusMsg(dictCallbackClient._statusMsg,	this);
				return;
			}
		}
		else
		{
			EncodeIterator rsslEncIter = _baseImpl.rsslEncIter();
					
			StatusMsg rsslStatusMsg = dictCallbackClient.rsslStatusMsg();
	        rsslStatusMsg.streamId(_streamId);
	        rsslStatusMsg.domainType(DomainTypes.DICTIONARY);
	        rsslStatusMsg.containerType(DataTypes.NO_DATA);
	        rsslStatusMsg.applyHasState();
		    rsslStatusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
		    rsslStatusMsg.state().dataState(DataStates.SUSPECT);
		    rsslStatusMsg.state().code(StateCodes.NONE);
		    rsslStatusMsg.state().text().data("Data dictionary is not ready to provide.");
		       
			rsslEncIter.clear();
			int retCode = rsslEncIter.setBufferAndRWFVersion(rsslDictEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion());
			if (retCode != CodecReturnCodes.SUCCESS)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DictionaryItem.CLIENT_NAME, 
		        			"Internal error. Failed to set encode iterator RWF version in DictionatyItem.handleTimeoutEvent()",
		        									Severity.ERROR));
	        	}
				return;
			}
			
		    if ((retCode = rsslStatusMsg.encode(rsslEncIter)) != CodecReturnCodes.SUCCESS)
		    {
		    	if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
		    		_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(DictionaryItem.CLIENT_NAME, 
		        			"Internal error. Failed to encode msg in DictionatyItem.handleTimeoutEvent()",
		        									Severity.ERROR));
	        	}
		    	return;
		    }
		    
		    if (dictCallbackClient._statusMsg == null)
		    	dictCallbackClient._statusMsg = new StatusMsgImpl(_baseImpl._objManager);
			
		    dictCallbackClient._statusMsg.decode(rsslDictEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion(), null, null);
		    	
			dictCallbackClient.processStatusMsg(dictCallbackClient._statusMsg,	this);
			
			return;
		}
	}

	@Override
	public ReentrantLock userLock()
	{
		return _baseImpl.userLock();
	}

	int currentFid()
	{
		return _currentFid;
	}
	
	String name()
	{
		return _name;
	}
	
	int rsslFilters()
	{
		return _rsslFilter;
	}
	
	int encodeDataDictionaryResp(boolean firstMultiRefresh, DataDictionary rsslDataDictionary, Buffer rsslDictEncBuffer)
	{
		ReactorChannel rsslChannel = _baseImpl.rsslReactorChannel();
		EncodeIterator rsslEncIter = _baseImpl.rsslEncIter();
		DictionaryCallbackClient<T> dictCallbackClient = _baseImpl.dictionaryCallbackClient();
		
		rsslEncIter.clear();
		int retCode = rsslEncIter.setBufferAndRWFVersion(rsslDictEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion());
		if (retCode != CodecReturnCodes.SUCCESS)
			return retCode;
		
		RefreshMsg rsslRefreshMsg = dictCallbackClient.rsslRefreshMsg();
		com.refinitiv.eta.transport.Error rsslError = dictCallbackClient.rsslError();

		rsslRefreshMsg.domainType(DomainTypes.DICTIONARY);
		rsslRefreshMsg.containerType(DataTypes.SERIES);
		rsslRefreshMsg.state().streamState(StreamStates.OPEN);
		rsslRefreshMsg.state().dataState(DataStates.OK);
		rsslRefreshMsg.state().code(StateCodes.NONE);
		rsslRefreshMsg.applySolicited();
		rsslRefreshMsg.applyHasMsgKey();
		rsslRefreshMsg.msgKey().filter(_rsslFilter);
		rsslRefreshMsg.msgKey().applyHasFilter();

		if (firstMultiRefresh)
			rsslRefreshMsg.applyClearCache();
		
		rsslRefreshMsg.msgKey().applyHasName();
		rsslRefreshMsg.msgKey().name().data(_name);
		
		rsslRefreshMsg.streamId(_streamId);
		
		Int rsslCurrentFid = dictCallbackClient.rsslCurrentFid();
		rsslCurrentFid.value(_currentFid);
		
		boolean complete = false;
		
		if ((retCode = rsslRefreshMsg.encodeInit(rsslEncIter, 0)) < CodecReturnCodes.SUCCESS)
			return retCode;

		if (_name.equals(DictionaryCallbackClient.DICTIONARY_RWFFID))
		{
			retCode = rsslDataDictionary.encodeFieldDictionary(rsslEncIter, rsslCurrentFid, _rsslFilter, rsslError);
		}
		else if (_name.equals(DictionaryCallbackClient.DICTIONARY_RWFENUM))
		{
			retCode = rsslDataDictionary.encodeEnumTypeDictionaryAsMultiPart(rsslEncIter, rsslCurrentFid, _rsslFilter, rsslError); 
		}	
		else
			return CodecReturnCodes.FAILURE;
			

		if (retCode != CodecReturnCodes.SUCCESS)
		{
			if (retCode == CodecReturnCodes.DICT_PART_ENCODED)
				_currentFid = (int)rsslCurrentFid.toLong();
			else
				return retCode;
		}
		else
		{
			complete = true;
		}
		
		if ((retCode = rsslRefreshMsg.encodeComplete(rsslEncIter, true)) < CodecReturnCodes.SUCCESS)
			return retCode;
		
		return complete ? CodecReturnCodes.SUCCESS : CodecReturnCodes.DICT_PART_ENCODED;
	}
}

class NiProviderDictionaryItem<T> extends SingleItem<T> implements ProviderItem
{
	private static final String 	CLIENT_NAME = "NiProviderDictionaryItem";
	
	protected MsgKey 			_rsslMsgKey = CodecFactory.createMsgKey();
	protected ItemWatchList		_itemWatchList;
	protected int				_serviceId;
	protected boolean			_isPrivateStream;
	protected boolean 			_specifiedServiceInReq;
	private TimeoutEvent		_reqTimeoutEvent;
	private boolean				_receivedInitResp;
	
	NiProviderDictionaryItem(OmmBaseImpl<T> baseImpl, T client, Object closure)
	{
		super(baseImpl, client, closure, null);
		_itemWatchList = ((OmmNiProviderImpl)baseImpl).itemWatchList();
	}
	
	@Override
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> item)
	{
		super.reset(baseImpl, client, closure, null);
		
		_itemWatchList = ((OmmNiProviderImpl)baseImpl).itemWatchList();
	}

	@Override
	boolean open(ReqMsg reqMsg)
	{
		ReqMsgImpl reqMsgImpl = ((ReqMsgImpl)reqMsg);
		
		String serviceName = null;
		
		if ( reqMsgImpl.hasServiceName() )
		{
			ServiceIdInteger serviceId = ((OmmNiProviderImpl)_baseImpl).directoryServiceStore().serviceId(reqMsgImpl.serviceName());
			
			if ( serviceId == null )
			{
				StringBuilder temp = _baseImpl.strBuilder();
	        	temp.append("Service name of '")
	        		.append(reqMsg.serviceName())
	        		.append("' is not found.");
	     
				_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);

	        	scheduleItemClosedStatus(_baseImpl.itemCallbackClient(),
											this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
											temp.toString(), reqMsg.serviceName());
	        	
	        	return true;
			}
			else if (serviceId != null)
			{
				_serviceId = serviceId.value();
				reqMsgImpl.rsslMsg().msgKey().applyHasServiceId();
				reqMsgImpl.rsslMsg().msgKey().serviceId(_serviceId);
				serviceName = reqMsgImpl.serviceName();
				_specifiedServiceInReq = true;
			}
		}
		else if ( reqMsgImpl.hasServiceId() )
		{
			serviceName = ((OmmNiProviderImpl)_baseImpl).directoryServiceStore().serviceName(reqMsgImpl.serviceId());
			
			if ( serviceName == null )
			{
				StringBuilder temp = _baseImpl.strBuilder();
				
	        	temp.append("Service id of '")
	        		.append(reqMsg.serviceId())
	        		.append("' is not found.");
	        	
				_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);
	        	
	        	scheduleItemClosedStatus(_baseImpl.itemCallbackClient(),
						this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
						temp.toString(), null);
	        	
	        	return true;
			}
			
			_serviceId = reqMsgImpl.serviceId();
			_specifiedServiceInReq = true;
		}
		
		_isPrivateStream = reqMsgImpl.privateStream();
		
		_directory = new Directory<T>(serviceName);
			
		_itemWatchList.addItem(this);
		
		reqMsgImpl.rsslMsg().msgKey().copy(_rsslMsgKey);
		
		return rsslSubmit(((ReqMsgImpl)reqMsg).rsslMsg(), true);
	}
	
	@Override
	void remove()
	{
		cancelReqTimerEvent();
		
		super.remove();
		
		((OmmNiProviderImpl)_baseImpl).returnProviderStreamId(_streamId);
		
		_itemWatchList.removeItem(this);
	}
	
	@Override
	boolean modify(com.refinitiv.ema.access.ReqMsg reqMsg)
	{
		if (_closedStatusClient != null) return false;
		
		ReqMsgImpl reqMsgImpl = (ReqMsgImpl)reqMsg;
		
		if ( reqMsgImpl.hasServiceName() )
		{
			if ( _specifiedServiceInReq && ( _directory != null ) && reqMsgImpl.serviceName().equals(_directory.serviceName()) )
			{
				reqMsgImpl.rsslMsg().msgKey().applyHasServiceId();
				reqMsgImpl.rsslMsg().msgKey().serviceId(_serviceId);
			}
			else
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Service name of '")
        		.append(reqMsg.serviceName())
        		.append("' does not match existing request.").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");

				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(NiProviderDictionaryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				
				return false;
			}
		}
		else if ( reqMsgImpl.hasServiceId() )
		{
			if  ( !_specifiedServiceInReq || ( reqMsgImpl.serviceId() != _serviceId ) )
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Service id of '")
        		.append(reqMsg.serviceId())
        		.append("' does not match existing request.").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");
				
				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(NiProviderDictionaryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				
				return false;
			}
		}
		else
		{
			if ( _specifiedServiceInReq )
			{
				reqMsgImpl.rsslMsg().msgKey().applyHasServiceId();
				reqMsgImpl.rsslMsg().msgKey().serviceId(_serviceId);
			}
		}
		
		if ( reqMsgImpl.hasName() )
		{
			if ( reqMsgImpl.name().equals(_rsslMsgKey.name().toString()) == false )
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Name of '")
        		.append(reqMsgImpl.name())
        		.append("' does not match existing request.").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");
				
				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(NiProviderDictionaryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				
				return false;
			}
		}
		else
		{
			reqMsgImpl.name(_rsslMsgKey.name().toString());
			reqMsgImpl.nameType(_rsslMsgKey.nameType());
		}
		
		((ReqMsgImpl)reqMsg).rsslMsg().domainType(EmaRdm.MMT_DICTIONARY);
		
		return super.modify(reqMsgImpl);
	}
	
	@Override
	int getNextStreamId(int numOfItem) {
		return ((OmmNiProviderImpl)_baseImpl).nextProviderStreamId();
	}
	
	@Override
	boolean rsslSubmit(com.refinitiv.eta.codec.RequestMsg rsslRequestMsg, boolean reportError)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.serviceName(null);
		
		int domainType =  rsslRequestMsg.domainType();
		
		if (_streamId == 0)
		{
			rsslRequestMsg.streamId(getNextStreamId(0));
			_streamId = rsslRequestMsg.streamId();
			
			_baseImpl._itemCallbackClient.addToMap(_baseImpl.nextLongId(), this);
		}
		else
			rsslRequestMsg.streamId(_streamId);

		if (_domainType == 0)
			_domainType = domainType;
		else
			rsslRequestMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		
		ChannelInfo activeChannel = _baseImpl.loginCallbackClient().activeChannelInfo();
		ReactorChannel rsslChannel = (activeChannel != null) ? activeChannel.rsslReactorChannel() : null;

		if (rsslChannel == null)
		{
			StringBuilder message = _baseImpl.strBuilder();

			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				message.append("Internal error: rsslChannel.Submit() failed in NiProviderDictionaryItem.submit(TunnelStreamRequest)").append(OmmLoggerClient.CR)
					.append("\tReactorChannel is not available");

				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(NiProviderDictionaryItem.CLIENT_NAME, message.toString(), Severity.ERROR));

				message.setLength(0);
			}

			message.append("Failed to open or modify item request. Reason: ReactorChannel is not available");

			_baseImpl.handleInvalidUsage(message.toString(), ReactorReturnCodes.FAILURE);

			return false;
		}

		int ret;

		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in NiProviderDictionaryItem.submit(RequestMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(NiProviderDictionaryItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to open or modify item request. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			_baseImpl.handleInvalidUsage(temp.toString(), ret);
			return false;
	    }
		
		int requestTimeout = ((OmmNiProviderImpl)_baseImpl).requestTimeout();
		
		if ( requestTimeout > 0 )
		{
			cancelReqTimerEvent();
			_reqTimeoutEvent = _baseImpl.addTimeoutEvent(requestTimeout * 1000, new ItemTimeOut( this ) );
		}

		return true;
	}
	
	@Override
	boolean submit(com.refinitiv.ema.access.GenericMsg genericMsg)
	{
		return false;
	}
	
	public TimeoutEvent reqTimeoutEvent()
	{
		return _reqTimeoutEvent;
	}

	@Override
	public void scheduleItemClosedRecoverableStatus(String statusText, boolean initiateTimeout) {

		if (_closedStatusClient != null) return;
		
		cancelReqTimerEvent();
    	
		_closedStatusClient = new ClosedStatusClient<T>(_baseImpl.itemCallbackClient(), this, _rsslMsgKey, _isPrivateStream, statusText, _directory.serviceName());
		
		if ( initiateTimeout )
			_baseImpl.addTimeoutEvent(100, _closedStatusClient);
		else
			_closedStatusClient.handleTimeoutEvent();
	}

	@Override
	public MsgKey rsslMsgKey() {
		return _rsslMsgKey;
	}

	@Override
	public void sendCloseMsg() {
		CloseMsg rsslCloseMsg = _baseImpl.itemCallbackClient().rsslCloseMsg();
		rsslCloseMsg.containerType(DataTypes.NO_DATA);
		rsslCloseMsg.domainType(_domainType);

		rsslSubmit(rsslCloseMsg);
	}

	@Override
	public ClientSession clientSession() {
		return null;
	}

	@Override
	public int serviceId() {
		return _serviceId;
	}
	
	@Override
	int type()
	{
		return ItemType.NIPROVIDER_DICTIONARY_ITEM;
	}

	@Override
	public boolean processInitialResp(RefreshMsg refreshMsg)
	{
		boolean result = true;
		
		if ( _receivedInitResp == false )
		{
			if ( _domainType != refreshMsg.domainType() )
				result = false;
			
			_isPrivateStream = refreshMsg.checkPrivateStream();
			
			if (!_rsslMsgKey.equals(refreshMsg.msgKey()) )
			{
				result = false;
			}
			
			_receivedInitResp = true;
		}
		
		return result;
	}

	@Override
	public ItemWatchList itemWatchList()
	{
		return _itemWatchList;
	}

	@Override
	public void cancelReqTimerEvent()
	{
		if ( _reqTimeoutEvent != null )
		{
			if ( _reqTimeoutEvent.cancelled() == false )
			{
				_reqTimeoutEvent.cancel();
			}
		}
	}
	
	@Override
	public boolean requestWithService()
	{
		return _specifiedServiceInReq;
	}

	@Override
	public ReentrantLock userLock()
	{
		return _baseImpl.userLock();
	}
}

class IProviderDictionaryItem extends IProviderSingleItem
{
	private boolean						_receivedInitResp;
	
	IProviderDictionaryItem(OmmServerBaseImpl baseImpl, OmmProviderClient client, Object closure)
	{
		super(baseImpl, client, closure, null);
	}
	
	@Override
	void reset(OmmServerBaseImpl baseImpl, OmmProviderClient client, Object closure, Item<OmmProviderClient> item)
	{
		super.reset(baseImpl, client, closure, null);
	}

	@Override
	int type()
	{
		return ItemType.IPROVIDER_DICTIONARY_ITEM;
	}
	
	@Override
	boolean modify(com.refinitiv.ema.access.ReqMsg reqMsg)
	{
		if (_closedStatusClient != null) return false;
		
		((ReqMsgImpl)reqMsg).rsslMsg().domainType(EmaRdm.MMT_DICTIONARY);
		
		return super.modify(reqMsg);
	}
	
	@Override
	public boolean processInitialResp(RefreshMsg refreshMsg)
	{
		boolean result = true;
		
		if ( _receivedInitResp == false )
		{
			if ( _domainType != refreshMsg.domainType() )
				result = false;
			
			_isPrivateStream = refreshMsg.checkPrivateStream();
			
			if (!_rsslMsgKey.equals(refreshMsg.msgKey()) )
			{
				result = false;
			}
			
			_receivedInitResp = true;
		}
		
		return result;
	}

	@Override
	public ReentrantLock userLock()
	{
		return _baseImpl.userLock();
	}
}
	
	
