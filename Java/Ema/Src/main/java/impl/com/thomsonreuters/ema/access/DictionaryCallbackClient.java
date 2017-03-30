///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.thomsonreuters.upa.valueadd.reactor.RDMDictionaryMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.RDMDictionaryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;


class DictionaryCallbackClient<T> extends CallbackClient<T> implements RDMDictionaryMsgCallback
{
	private static final String CLIENT_NAME 				= "DictionaryCallbackClient";
	private static final int MAX_DICTIONARY_BUFFER_SIZE 	= 448000;
	protected static final String DICTIONARY_RWFFID = "RWFFld";
	protected static final String DICTIONARY_RWFENUM = "RWFEnum";
	
	private List<ChannelDictionary<T>>						_channelDictList;
	private List<ChannelDictionary<T>>							_channelDictPool;
	private com.thomsonreuters.upa.codec.DataDictionary		_rsslLocalDictionary;
	private com.thomsonreuters.upa.codec.Buffer 			_rsslEncBuffer;
	private com.thomsonreuters.upa.transport.Error			_rsslError;
	private com.thomsonreuters.upa.codec.Int 				_rsslCurrentFid;
	
	DictionaryCallbackClient(OmmBaseImpl<T> baseImpl)
	{
		super(baseImpl, CLIENT_NAME);
	}
	
	void initialize()
	{
		if (_baseImpl.activeConfig().rsslFldDictRequest != null && _baseImpl.activeConfig().rsslEnumDictRequest != null)
			_baseImpl.activeConfig().dictionaryConfig.isLocalDictionary = false;
		else if (_baseImpl.activeConfig().rsslFldDictRequest != null && _baseImpl.activeConfig().rsslEnumDictRequest == null)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			temp.append("Invalid dictionary configuration was specified through the addAdminMsg() method")
				.append(OmmLoggerClient.CR)
				.append("Enumeration type definition request message was not populated.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
												Severity.ERROR).toString());

			throw (_baseImpl.ommIUExcept().message(temp.toString()));
		}
		else if (_baseImpl.activeConfig().rsslFldDictRequest == null && _baseImpl.activeConfig().rsslEnumDictRequest != null)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			temp.append("Invalid dictionary configuration was specified through the addAdminMsg() method")
				.append(OmmLoggerClient.CR)
				.append("RDM Field Dictionary request message was not populated.");
			
			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
												Severity.ERROR).toString());

			throw (_baseImpl.ommIUExcept().message(temp.toString()));
		}
		
		if (_baseImpl.activeConfig().dictionaryConfig.isLocalDictionary)
			loadDictionaryFromFile();
		else
		{
			_channelDictList = new ArrayList<>();
			_channelDictPool = new ArrayList<>();
			_channelDictPool.add(new ChannelDictionary<T>(_baseImpl));
		}
	}

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
		
		switch (msg.msgClass())
		{
			case MsgClasses.REFRESH:
			{	
				if (_refreshMsg == null)
					_refreshMsg = new RefreshMsgImpl(_baseImpl._objManager);
				
				_refreshMsg.decode(msg, rsslChannel.majorVersion(), rsslChannel.minorVersion(), null);
				
				_baseImpl.dictionaryCallbackClient().processRefreshMsg(_refreshMsg, item );
				
				return ReactorCallbackReturnCodes.SUCCESS;
			}
			case MsgClasses.STATUS:
			{	
				if (_statusMsg == null)
					_statusMsg = new StatusMsgImpl(_baseImpl._objManager);
				
				_statusMsg.decode(msg, rsslChannel.majorVersion(), rsslChannel.minorVersion(), null );
				
				_baseImpl.dictionaryCallbackClient().processStatusMsg(_statusMsg, item );
				
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
		if (_rsslLocalDictionary.loadFieldDictionary(_baseImpl.activeConfig().dictionaryConfig.rdmfieldDictionaryFileName, _rsslError) < 0)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				temp.append("Unable to load RDMFieldDictionary from file named ")
					.append(_baseImpl.activeConfig().dictionaryConfig.rdmfieldDictionaryFileName)
					.append(OmmLoggerClient.CR)
					.append("Current working directory ")
					.append(System.getProperty("user.dir"))  
					.append(OmmLoggerClient.CR)
					.append("Error text ")
					.append(_rsslError.toString());
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
			}
			

			_baseImpl.handleInvalidUsage(temp.toString());
		
			return;
		}
		
		if (_rsslLocalDictionary.loadEnumTypeDictionary(_baseImpl.activeConfig().dictionaryConfig.enumtypeDefFileName, _rsslError) < 0)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				temp.append(_baseImpl.activeConfig().dictionaryConfig.enumtypeDefFileName)
					.append(OmmLoggerClient.CR)
					.append("Current working directory ")
					.append(System.getProperty("user.dir"))  
					.append(OmmLoggerClient.CR)
					.append("Error text ")
					.append(_rsslError.toString());
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
			}
			
			_baseImpl.handleInvalidUsage(temp.toString());
			
			return;
		}

		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Successfully loaded local dictionaries: ")
				.append(OmmLoggerClient.CR)
				.append("RDMFieldDictionary file named ")
				.append(_baseImpl.activeConfig().dictionaryConfig.rdmfieldDictionaryFileName)
				.append(OmmLoggerClient.CR)
				.append("EnumTypeDef file named ")
				.append(_baseImpl.activeConfig().dictionaryConfig.enumtypeDefFileName);
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
		Directory directory = _baseImpl.directoryCallbackClient().directory(msgImpl._rsslMsg.msgKey().serviceId());
			
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
			return (_channelDictPool.get(0).clear());
		else
			return (new ChannelDictionary<T>(baseImpl));
	}
	
	void returnToChannelDictPool(ChannelDictionary<T> channelDict)
	{
		_channelDictPool.add(channelDict);
	}
	
	DataDictionary defaultRsslDictionary()
	{
		if (_channelDictList != null && !_channelDictList.isEmpty())
		{
			if (_channelDictList.get(0).isLoaded())
				return _channelDictList.get(0).rsslDictionary();
		}
		else
			return _rsslLocalDictionary;
		
		return null;
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

	boolean downloadDictionary(Directory directory)
	{
		if (_baseImpl.activeConfig().dictionaryConfig.isLocalDictionary)
		{
			if (_rsslLocalDictionary != null && _rsslLocalDictionary.numberOfEntries() > 0)
				directory.channelInfo().rsslDictionary(_rsslLocalDictionary);
			return true;
		}
		
		if (directory.channelInfo().rsslDictionary() != null || _baseImpl.activeConfig().dictionaryConfig.isLocalDictionary)
			return true;
		
		if (_baseImpl.activeConfig().rsslFldDictRequest != null && _baseImpl.activeConfig().rsslEnumDictRequest != null)
		{
			if (_baseImpl.activeConfig().rsslFldDictRequest.serviceId() == directory.service().serviceId() ||
					( _baseImpl.activeConfig().fldDictReqServiceName != null && _baseImpl.activeConfig().fldDictReqServiceName.equals(directory.serviceName())))
				downloadDictionaryFromService(directory);
			
			return true;
		}
		
		com.thomsonreuters.upa.codec.RequestMsg  rsslRequestMsg = rsslRequestMsg();
		
		rsslRequestMsg.domainType(DomainTypes.DICTIONARY);
		rsslRequestMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		rsslRequestMsg.applyStreaming();
		MsgKey msgKey = rsslRequestMsg.msgKey();
		msgKey.applyHasName();
		msgKey.applyHasFilter();
		msgKey.filter(com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues.NORMAL);

		ChannelDictionary<T> dictionary = pollChannelDict(_baseImpl);
		dictionary.channelInfo(directory.channelInfo());
		
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		ReactorChannel rsslChannel = directory.channelInfo().rsslReactorChannel();
		
		rsslSubmitOptions.serviceName(directory.serviceName());
		rsslSubmitOptions.requestMsgOptions().userSpecObj(dictionary);

		int streamId = 3;

		List<String> dictionariesUsed = directory.service().info().dictionariesUsedList();
		for (String dictName : dictionariesUsed)
		{
			msgKey.name().data(dictName);
			rsslRequestMsg.streamId(streamId++);

	        rsslErrorInfo.clear();
	        if (ReactorReturnCodes.SUCCESS > rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo))
	        {
	        	if (_baseImpl.loggerClient().isErrorEnabled())
				{
					com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
					
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
	
	boolean downloadDictionaryFromService(Directory directory)
	{
		com.thomsonreuters.upa.codec.RequestMsg rsslRequestMsg = rsslRequestMsg();
		
		rsslRequestMsg.domainType(DomainTypes.DICTIONARY);
		rsslRequestMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		
		DictionaryRequest rsslDictRequest = _baseImpl.activeConfig().rsslFldDictRequest;
		if (rsslDictRequest.checkStreaming())
			rsslRequestMsg.applyStreaming();
		
		MsgKey msgKey = rsslRequestMsg.msgKey();
		msgKey.applyHasName();
		msgKey.applyHasFilter();
		msgKey.filter(rsslDictRequest.verbosity());
		msgKey.name(rsslDictRequest.dictionaryName());
		rsslRequestMsg.streamId(3);

		ChannelDictionary<T> dictionary = pollChannelDict(_baseImpl);
		dictionary.channelInfo(directory.channelInfo());
		
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		ReactorChannel rsslChannel = directory.channelInfo().rsslReactorChannel();
		
		rsslSubmitOptions.serviceName(directory.serviceName());
		rsslSubmitOptions.requestMsgOptions().userSpecObj(dictionary);

        rsslErrorInfo.clear();
        if (ReactorReturnCodes.SUCCESS > rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo))
        {
        	if (_baseImpl.loggerClient().isErrorEnabled())
			{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
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
        rsslRequestMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		
		DictionaryRequest rsslEnumDictRequest = _baseImpl.activeConfig().rsslEnumDictRequest;
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
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
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
	
	com.thomsonreuters.upa.transport.Error rsslError()
	{
		if (_rsslError == null)
			_rsslError = com.thomsonreuters.upa.transport.TransportFactory.createError();
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
	void notifyOnAllMsg(com.thomsonreuters.ema.access.Msg msg)
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
	}
	
	@Override
	void notifyOnAllMsg(com.thomsonreuters.ema.access.Msg msg)
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
	
	void notifyStatusToListener(OmmBaseImpl<T> baseImpl, com.thomsonreuters.upa.codec.State rsslStatus, int streamId)
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
		
		if (msg == null)
		{
			com.thomsonreuters.upa.transport.Error error = event.errorInfo().error();
        	
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
			com.thomsonreuters.upa.codec.RefreshMsg rsslMsg = (com.thomsonreuters.upa.codec.RefreshMsg)msg;

			com.thomsonreuters.upa.codec.State state = rsslMsg.state();

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
					case com.thomsonreuters.upa.rdm.Dictionary.Types.FIELD_DEFINITIONS :
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
					case com.thomsonreuters.upa.rdm.Dictionary.Types.ENUM_TABLES :
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

			com.thomsonreuters.upa.transport.Error rsslError = _baseImpl.dictionaryCallbackClient().rsslError();
			if (_fldStreamId == rsslMsg.streamId())
			{
				if (_isFldLoaded == true && _isEnumLoaded == true)
					_rsslDictionary.clear();
				
	    		if (CodecReturnCodes.SUCCESS == _rsslDictionary.decodeFieldDictionary(dIter, 
	    																				com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues.VERBOSE,
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
	    										com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues.VERBOSE, rsslError))
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
			com.thomsonreuters.upa.codec.StatusMsg rsslMsg = (com.thomsonreuters.upa.codec.StatusMsg)msg;
			DictionaryStatus rsslStatus = (DictionaryStatus)event.rdmDictionaryMsg();
			
			if (rsslMsg.checkHasState())
			{
				State state =((com.thomsonreuters.upa.codec.StatusMsg)rsslMsg).state();

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

		if ( reqMsg.hasServiceName() || rsslReqMsg.msgKey().checkHasServiceId() )
			return super.open( reqMsg );
		else
		{
			DataDictionary rsslDictionary = dictCBClient.defaultRsslDictionary();
	
			if (rsslDictionary != null)
			{
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

		_baseImpl.handleInvalidUsage(temp.toString());

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

		_baseImpl.handleInvalidUsage(temp.toString());

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

		_baseImpl.handleInvalidUsage(temp.toString());

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
				_baseImpl.itemCallbackClient().removeFromMap(this);
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
		com.thomsonreuters.upa.transport.Error rsslError = dictCallbackClient.rsslError();

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
	
	