///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.impl.OmmLoggerClient.Severity;
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
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.TransportFactory;
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


class DictionaryCallbackClient extends ConsumerCallbackClient implements RDMDictionaryMsgCallback
{
	private static final String CLIENT_NAME 				= "DictionaryCallbackClient";
	private static final int MAX_DICTIONARY_BUFFER_SIZE 	= 16384;
	
	private List< ChannelDictionary >						_channelDictList;
	private List<ChannelDictionary>							_channelDictPool;
	private com.thomsonreuters.upa.codec.DataDictionary		_rsslLocalDictionary;
	private com.thomsonreuters.upa.codec.RequestMsg 		_rsslRequestMsg;
	private com.thomsonreuters.upa.codec.Buffer 			_rsslWrapBuffer;
	private com.thomsonreuters.upa.codec.Buffer 			_rsslEncBuffer;
	private com.thomsonreuters.upa.transport.Error			_rsslError;
	private com.thomsonreuters.upa.codec.Int 				_rsslCurrentFid;
	private com.thomsonreuters.upa.codec.RefreshMsg 		_rsslRefreshMsg;
	private com.thomsonreuters.upa.codec.StatusMsg			_rsslStatusMsg;
	
	DictionaryCallbackClient(OmmConsumerImpl consumer)
	{
		super(consumer, CLIENT_NAME);
	}
	
	void initialize()
	{
		_consumer.activeConfig().intializeDictReq(CLIENT_NAME);
		
		if (_consumer.activeConfig().dictionaryConfig.isLocalDictionary)
			loadDictionaryFromFile();
		else
		{
			_channelDictList = new ArrayList<ChannelDictionary>();
			_channelDictPool = new ArrayList<ChannelDictionary>();
			_channelDictPool.add(new ChannelDictionary(_consumer));
		}
	}

	@Override
	public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
	{
		boolean found = false;
		ChannelDictionary one = null;

		for (ChannelDictionary entry : _channelDictList)
		{
			if (entry ==  event.streamInfo().userSpecObject())
			{
				found = true;
				one = entry;
				break;
			}
		}

		if (found)
			return one.processCallback(event);
		else
		{
			if (_consumer.loggerClient().isErrorEnabled())
				_consumer.loggerClient().error(_consumer.formatLogMessage(CLIENT_NAME, "Received dictionary which is not what has been requested",
																			Severity.ERROR).toString());
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
	}
	
	void loadDictionaryFromFile()
	{
		if (_rsslLocalDictionary == null)
			_rsslLocalDictionary = CodecFactory.createDataDictionary();
		else
			_rsslLocalDictionary.clear();

		rsslError();
		if (_rsslLocalDictionary.loadFieldDictionary(_consumer.activeConfig().dictionaryConfig.rdmfieldDictionaryFileName, _rsslError) < 0)
		{
			StringBuilder temp = _consumer.consumerStrBuilder();
			
			if (_consumer.loggerClient().isErrorEnabled())
			{
				temp.append("Unable to load RDMFieldDictionary from file named ")
					.append(_consumer.activeConfig().dictionaryConfig.rdmfieldDictionaryFileName)
					.append(OmmLoggerClient.CR)
					.append("Current working directory ")
					.append(System.getProperty("user.dir"))  
					.append(OmmLoggerClient.CR)
					.append("Error text ")
					.append(_rsslError.toString());
				_consumer.loggerClient().error(_consumer.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
			}
			
			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
			else
				throw (_consumer.ommIUExcept().message(temp.toString()));
		
			return;
		}
		
		if (_rsslLocalDictionary.loadEnumTypeDictionary(_consumer.activeConfig().dictionaryConfig.enumtypeDefFileName, _rsslError) < 0)
		{
			StringBuilder temp = _consumer.consumerStrBuilder();
			if (_consumer.loggerClient().isErrorEnabled())
			{
				temp.append(_consumer.activeConfig().dictionaryConfig.enumtypeDefFileName)
					.append(OmmLoggerClient.CR)
					.append("Current working directory ")
					.append(System.getProperty("user.dir"))  
					.append(OmmLoggerClient.CR)
					.append("Error text ")
					.append(_rsslError.toString());
				_consumer.loggerClient().error(_consumer.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
			}
			
			if (_consumer.hasConsumerErrorClient())
				_consumer.consumerErrorClient().onInvalidUsage(temp.toString());
			else
				throw (_consumer.ommIUExcept().message(temp.toString()));
			
			return;
		}

		if (_consumer.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _consumer.consumerStrBuilder();
			temp.append("Successfully loaded local dictionaries: ")
				.append(OmmLoggerClient.CR)
				.append("RDMFieldDictionary file named ")
				.append(_consumer.activeConfig().dictionaryConfig.rdmfieldDictionaryFileName)
				.append(OmmLoggerClient.CR)
				.append("EnumTypeDef file named ")
				.append(_consumer.activeConfig().dictionaryConfig.enumtypeDefFileName);
			_consumer.loggerClient().trace(_consumer.formatLogMessage(CLIENT_NAME, temp.toString(),
																		Severity.TRACE).toString());
		}

	}
	
	void processRefreshMsg(Buffer rsslBuffer, int majVer, int minVer, DictionaryItem dictItem)
	{
		if (_refreshMsg == null)
			_refreshMsg = new RefreshMsgImpl();
		
		_refreshMsg.decode(rsslBuffer, majVer, minVer, null, null);

		_event._item = dictItem;
		
		_event._item.client().onAllMsg(_refreshMsg, _event);
		_event._item.client().onRefreshMsg(_refreshMsg, _event);

		if (_refreshMsg.state().streamState() == OmmState.StreamState.NON_STREAMING)
		{
			if (_refreshMsg.complete())
				_event._item.remove();
		}
		else if (_refreshMsg.state().streamState() != OmmState.StreamState.OPEN)
			_event._item.remove();

		return;
	}
	
	void processStatusMsg(Buffer rsslBuffer, int majVer, int minVer, DictionaryItem dictItem)
	{
		if (_statusMsg == null)
			_statusMsg = new StatusMsgImpl();
		
		_statusMsg.decode(rsslBuffer, majVer, minVer, null, null);

		_event._item = dictItem;
		
		_event._item.client().onAllMsg(_statusMsg, _event);
		_event._item.client().onStatusMsg(_statusMsg, _event);

		if (_statusMsg.state().streamState() != OmmState.StreamState.OPEN)
			_event._item.remove();

		return;
	}
	
	ChannelDictionary pollChannelDict(OmmConsumerImpl consumer)
	{
		if (_channelDictPool != null && !_channelDictPool.isEmpty())
			return (_channelDictPool.get(0).clear());
		else
			return (new ChannelDictionary(consumer));
	}
	
	void returnToChannelDictPool(ChannelDictionary channelDict)
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
	
	List< ChannelDictionary > channelDictionaryList()
	{
		return _channelDictList;
	}
	
	DictionaryItem dictionaryItem(ReqMsg reqMsg , OmmConsumerClient consumerClient , Object obj)
	{
		return null;
	}

	boolean downloadDictionary(Directory directory)
	{
		if (directory.channelInfo().rsslDictionary() != null)
			return true;
		
		if (_consumer.activeConfig().rsslFldDictRequest != null && _consumer.activeConfig().rsslEnumDictRequest != null)
		{
			if (_consumer.activeConfig().rsslFldDictRequest.serviceId() == directory.service().serviceId())
				downloadDictionaryFromService(directory);
			
			return true;
		}
		else if (_rsslLocalDictionary != null && _rsslLocalDictionary.numberOfEntries() > 0)
		{
			directory.channelInfo().rsslDictionary(_rsslLocalDictionary);
			return true;
		}
		
		if (_rsslRequestMsg == null)
			_rsslRequestMsg = (RequestMsg)CodecFactory.createMsg();
		else
			_rsslRequestMsg.clear();
		
		_rsslRequestMsg.msgClass(MsgClasses.REQUEST);
		_rsslRequestMsg.domainType(DomainTypes.DICTIONARY);
		_rsslRequestMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		_rsslRequestMsg.applyStreaming();
		MsgKey msgKey = _rsslRequestMsg.msgKey();
		msgKey.applyHasName();
		msgKey.applyHasFilter();
		msgKey.filter(com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues.NORMAL);

		ChannelDictionary dictionary = pollChannelDict(_consumer);
		dictionary.channelInfo(directory.channelInfo());
		
		ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
		ReactorChannel rsslChannel = directory.channelInfo().rsslReactorChannel();
		
		rsslSubmitOptions.clear();
		rsslSubmitOptions.serviceName(directory.serviceName());
		rsslSubmitOptions.requestMsgOptions().userSpecObj(dictionary);

		int streamId = 3;

		if (_rsslWrapBuffer == null)
			_rsslWrapBuffer = CodecFactory.createBuffer();
		
		List<String> dictionariesUsed = directory.service().info().dictionariesUsedList();
		for (String dictName : dictionariesUsed)
		{
			_rsslWrapBuffer.data(dictName);
			msgKey.name(_rsslWrapBuffer);
			_rsslRequestMsg.streamId(streamId++);

	        rsslErrorInfo.clear();
	        if (ReactorReturnCodes.SUCCESS > rsslChannel.submit(_rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo))
	        {
	        	if (_consumer.loggerClient().isErrorEnabled())
				{
					com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
					
					StringBuilder temp = _consumer.consumerStrBuilder();
					temp.append("Internal error: rsslChannel.submit() failed").append(OmmLoggerClient.CR)
						.append(directory.channelInfo().toString()).append(OmmLoggerClient.CR)
						.append("RsslChannel ").append(Integer.toHexString(error.channel().hashCode())).append(OmmLoggerClient.CR)
						.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
						.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
						.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
						.append("Error Text ").append(error.text());
					
					_consumer.loggerClient().error(_consumer.formatLogMessage(CLIENT_NAME, temp.toString(),
																				Severity.ERROR).toString());
					
					returnToChannelDictPool(dictionary);
				}
				
				return false;
			}
			else
			{
				if (_consumer.loggerClient().isTraceEnabled())
				{
					StringBuilder temp = _consumer.consumerStrBuilder();
					temp.append("Requested Dictionary ")
						.append(dictName).append(OmmLoggerClient.CR)
						.append("from Service ").append(directory.serviceName()).append(OmmLoggerClient.CR)
						.append("on Channel ").append(OmmLoggerClient.CR)
						.append(directory.channelInfo().toString());
					_consumer.loggerClient().trace(_consumer.formatLogMessage(CLIENT_NAME, temp.toString(),
																				Severity.TRACE).toString());
				}
			}
		}

		_channelDictList.add(dictionary);

		return true;
	}
	
	boolean downloadDictionaryFromService(Directory directory)
	{
		if (_rsslRequestMsg == null)
			_rsslRequestMsg = (RequestMsg)CodecFactory.createMsg();
		else
			_rsslRequestMsg.clear();
		
		_rsslRequestMsg.msgClass(MsgClasses.REQUEST);
		_rsslRequestMsg.domainType(DomainTypes.DICTIONARY);
		_rsslRequestMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		
		DictionaryRequest rsslDictRequest = _consumer.activeConfig().rsslFldDictRequest;
		if (rsslDictRequest.checkStreaming())
			_rsslRequestMsg.applyStreaming();
		
		MsgKey msgKey = _rsslRequestMsg.msgKey();
		msgKey.applyHasName();
		msgKey.applyHasFilter();
		msgKey.filter(rsslDictRequest.verbosity());
		msgKey.name(rsslDictRequest.dictionaryName());
		_rsslRequestMsg.streamId(3);

		ChannelDictionary dictionary = pollChannelDict(_consumer);
		dictionary.channelInfo(directory.channelInfo());
		
		ReactorSubmitOptions rsslSubmitOptions = _consumer.rsslSubmitOptions();
		ReactorErrorInfo rsslErrorInfo = _consumer.rsslErrorInfo();
		ReactorChannel rsslChannel = directory.channelInfo().rsslReactorChannel();
		
		rsslSubmitOptions.clear();
		rsslSubmitOptions.serviceName(directory.serviceName());
		rsslSubmitOptions.requestMsgOptions().userSpecObj(dictionary);

        rsslErrorInfo.clear();
        if (ReactorReturnCodes.SUCCESS > rsslChannel.submit(_rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo))
        {
        	if (_consumer.loggerClient().isErrorEnabled())
			{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
				StringBuilder temp = _consumer.consumerStrBuilder();
				temp.append("Internal error: rsslChannel.submit() failed").append(OmmLoggerClient.CR)
					.append(directory.channelInfo().toString()).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append(Integer.toHexString(error.channel().hashCode())).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
					.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
					.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
					.append("Error Text ").append(error.text());
				
				_consumer.loggerClient().error(_consumer.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
				
				returnToChannelDictPool(dictionary);
			}
			
			return false;
		}
		else
		{
			if (_consumer.loggerClient().isTraceEnabled())
			{
				StringBuilder temp = _consumer.consumerStrBuilder();
				temp.append("Requested Dictionary ")
					.append(rsslDictRequest.dictionaryName().toString()).append(OmmLoggerClient.CR)
					.append("from Service ").append(directory.serviceName()).append(OmmLoggerClient.CR)
					.append("on Channel ").append(OmmLoggerClient.CR)
					.append(directory.channelInfo().toString());
				_consumer.loggerClient().trace(_consumer.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.TRACE).toString());
			}
		}
	
		_rsslRequestMsg.clear();
		_rsslRequestMsg.msgClass(MsgClasses.REQUEST);
		_rsslRequestMsg.domainType(DomainTypes.DICTIONARY);
		_rsslRequestMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		
		DictionaryRequest rsslEnumDictRequest = _consumer.activeConfig().rsslEnumDictRequest;
		if (rsslEnumDictRequest.checkStreaming())
			_rsslRequestMsg.applyStreaming();
		
		msgKey = _rsslRequestMsg.msgKey();
		msgKey.applyHasName();
		msgKey.applyHasFilter();
		msgKey.filter(rsslEnumDictRequest.verbosity());
		msgKey.name(rsslEnumDictRequest.dictionaryName());
		_rsslRequestMsg.streamId(4);

        rsslErrorInfo.clear();
        if (ReactorReturnCodes.SUCCESS > rsslChannel.submit(_rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo))
        {
        	if (_consumer.loggerClient().isErrorEnabled())
			{
				com.thomsonreuters.upa.transport.Error error = rsslErrorInfo.error();
				
				StringBuilder temp = _consumer.consumerStrBuilder();
				temp.append("Internal error: rsslChannel.submit() failed").append(OmmLoggerClient.CR)
					.append(directory.channelInfo().toString()).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append(Integer.toHexString(error.channel().hashCode())).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
					.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
					.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
					.append("Error Text ").append(error.text());
				
				_consumer.loggerClient().error(_consumer.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.ERROR).toString());
				
				returnToChannelDictPool(dictionary);
			}
			
			return false;
		}
		else
		{
			if (_consumer.loggerClient().isTraceEnabled())
			{
				StringBuilder temp = _consumer.consumerStrBuilder();
				temp.append("Requested Dictionary ")
					.append(rsslDictRequest.dictionaryName().toString()).append(OmmLoggerClient.CR)
					.append("from Service ").append(directory.serviceName()).append(OmmLoggerClient.CR)
					.append("on Channel ").append(OmmLoggerClient.CR)
					.append(directory.channelInfo().toString());
				_consumer.loggerClient().trace(_consumer.formatLogMessage(CLIENT_NAME, temp.toString(),
																			Severity.TRACE).toString());
			}
		}

		_channelDictList.add(dictionary);

		return true;
	}
	
	boolean isDictionaryReady()
	{
		if (_rsslLocalDictionary != null && _rsslLocalDictionary.numberOfEntries() > 0 &&
			_rsslLocalDictionary.enumTableCount() > 0)
			return true;
		else 
		{
			if (_channelDictList.isEmpty())
				return false;
			
			for (ChannelDictionary entry : _channelDictList)
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
        	_rsslEncBuffer.data(byteBuf, 0, 0); 
		}
		
		return _rsslEncBuffer;
	}
	
	Buffer rsslWrapBuffer()
	{
		if (_rsslWrapBuffer == null)
			_rsslWrapBuffer = CodecFactory.createBuffer();
		else 
			_rsslWrapBuffer.clear();
		
		return _rsslWrapBuffer;
	}
	
	com.thomsonreuters.upa.transport.Error rsslError()
	{
		if (_rsslError == null)
			_rsslError = TransportFactory.createError();
		else
			_rsslError.clear();
		
		return _rsslError;
	}
	
	com.thomsonreuters.upa.codec.RefreshMsg rsslRefreshMsg()
	{
		if (_rsslRefreshMsg == null)
			_rsslRefreshMsg = (RefreshMsg)CodecFactory.createMsg();
		else
			_rsslRefreshMsg.clear();
		
		return _rsslRefreshMsg;
	}
	
	com.thomsonreuters.upa.codec.StatusMsg rsslStatusMsg()
	{
		if (_rsslStatusMsg == null)
			_rsslStatusMsg = (StatusMsg)CodecFactory.createMsg();
		else
			_rsslStatusMsg.clear();
		
		return _rsslStatusMsg;
	}
}

class ChannelDictionary
{
	private static final String CLIENT_NAME = "ChannelDictionary";
	
	private OmmConsumerImpl				_consumer;
	private ChannelInfo					_channelInfo;
	private boolean						_isFldLoaded;
	private boolean						_isEnumLoaded;
	private int 						_fldStreamId;
	private int 						_enumStreamId;
	private ReentrantLock 				_channelDictLock;
	private List<DictionaryItem>		_listenerList;
	private DataDictionary				_rsslDictionary = CodecFactory.createDataDictionary();
	
	
	ChannelDictionary(OmmConsumerImpl consumer)
	{
		_consumer = consumer;
	}
	
	ChannelInfo channelInfo()
	{
		return _channelInfo;
	}
	
	ChannelDictionary channelInfo(ChannelInfo channelInfo)
	{
		_channelInfo = channelInfo;
		_channelInfo.rsslDictionary(_rsslDictionary);
		return this;
	}
	
	ChannelDictionary clear()
	{
		_channelInfo = null;
		_isFldLoaded = false;
		_isEnumLoaded = false;
		_fldStreamId = 0;
		_enumStreamId = 0;
		_rsslDictionary.clear();
		
		if (_listenerList != null && _listenerList.size() > 0)
		{
			for (DictionaryItem entry : _listenerList)
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
	
	void addListener(DictionaryItem item)
	{
		if (_listenerList == null)
			_listenerList = new ArrayList<DictionaryItem>();
		
		_listenerList.add(item);
	}

	void removeListener(DictionaryItem item)
	{
		if (_listenerList == null || _listenerList.isEmpty())
			return;
		
		_listenerList.remove(item);
	}
	
	void notifyStatusToListener(OmmConsumerImpl consumer, com.thomsonreuters.upa.codec.State rsslStatus, int streamId)
	{
		channelDictionaryLock().lock();

		if (_listenerList == null || _listenerList.isEmpty())
		{
			channelDictionaryLock().unlock();
			return;
		}

		DictionaryItem dictItem = null;
		ReactorChannel rsslChannel = consumer.rsslReactorChannel();
		EncodeIterator rsslEncIter = consumer.rsslEncIter();
		DictionaryCallbackClient dictCallbackClient = consumer.dictionaryCallbackClient();
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
			       
				rsslEncIter.clear();
				rsslEncDictBuf = dictCallbackClient.rsslDictEncBuffer();
				int retCode = rsslEncIter.setBufferAndRWFVersion(rsslEncDictBuf, rsslChannel.majorVersion(), rsslChannel.minorVersion());
				if (retCode != CodecReturnCodes.SUCCESS)
				{
					if (consumer.loggerClient().isErrorEnabled())
		        	{
						consumer.loggerClient().error(consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, 
			        			"Internal error. Failed to set encode iterator with buffer in ChannelDictionary.notifyStatusToListener()",
			        									Severity.ERROR));
		        	}
					return;
				}
				
			    if ((retCode = rsslStatusMsg.encode(rsslEncIter)) != CodecReturnCodes.SUCCESS)
			    {
			    	if (consumer.loggerClient().isErrorEnabled())
		        	{
			    		consumer.loggerClient().error(consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, 
			        			"Internal error. Failed to encode msg in ChannelDictionary.notifyStatusToListener()",
			        									Severity.ERROR));
		        	}
			    	return;
			    }
			    	
			    dictCallbackClient.processStatusMsg(rsslEncDictBuf, rsslChannel.majorVersion(),
														rsslChannel.minorVersion(),	dictItem);
				
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
        	
        	if (_consumer.loggerClient().isErrorEnabled())
        	{
	        	StringBuilder temp = _consumer.consumerStrBuilder();
				
	        	temp.append("Received event without RDMDictionary message").append(OmmLoggerClient.CR)
				    .append("ChannelInfo ").append(OmmLoggerClient.CR)
					.append(channelInfo.toString()).append(OmmLoggerClient.CR)
					.append("RsslChannel ").append(Integer.toHexString(error.channel().hashCode())).append(OmmLoggerClient.CR)
					.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(event.errorInfo().location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());

	        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			_consumer.closeRsslChannel(event.reactorChannel());
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		switch (msg.msgClass())
		{
		case MsgClasses.REFRESH:
		{
			com.thomsonreuters.upa.codec.RefreshMsg rsslMsg = (com.thomsonreuters.upa.codec.RefreshMsg)msg;

			com.thomsonreuters.upa.codec.State state = rsslMsg.state();

			if (state.streamState() != StreamStates.OPEN)
			{
				if (_consumer.loggerClient().isErrorEnabled())
	        	{
					
					StringBuilder temp = _consumer.consumerStrBuilder();
		        	temp.append("RDMDictionary stream was closed with refresh message").append(OmmLoggerClient.CR)
		        		.append("ChannelInfo").append(OmmLoggerClient.CR)
						.append(channelInfo.toString()).append(OmmLoggerClient.CR)
						.append("Reason ").append(state.toString());
	        	
		        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	}
				break;
			}
			else if (state.dataState() == DataStates.SUSPECT)
			{
				if (_consumer.loggerClient().isWarnEnabled())
	        	{
					
					StringBuilder temp = _consumer.consumerStrBuilder();
		        	temp.append("RDMDictionary stream state was changed to suspect with refresh message").append(OmmLoggerClient.CR)
		        		.append("ChannelInfo").append(OmmLoggerClient.CR)
						.append(channelInfo.toString()).append(OmmLoggerClient.CR)
						.append("Reason ").append(state.toString());
	        	
		        	_consumer.loggerClient().warn(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.WARNING));
	        	}
				break;
			}
			
			if (_consumer.loggerClient().isTraceEnabled())
        	{
				
				StringBuilder temp = _consumer.consumerStrBuilder();
	        	temp.append("Received RDMDictionary refresh message").append(OmmLoggerClient.CR)
	        		.append("Dictionary name ").append(rsslMsg.msgKey().name().toString()).append(OmmLoggerClient.CR)
					.append("streamId ").append(rsslMsg.streamId());
        	
	        	_consumer.loggerClient().trace(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.TRACE));
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
							if (_consumer.loggerClient().isErrorEnabled())
				        	{
								
								StringBuilder temp = _consumer.consumerStrBuilder();
					        	temp.append("Received RDMDictionary refresh message with FieldDefinitions but changed streamId")
					        		.append(OmmLoggerClient.CR)
					        		.append("Initial streamId ").append(_fldStreamId).append(OmmLoggerClient.CR)
					        		.append("New streamId ").append(rsslMsg.streamId());
				        	
					        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME,
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
							if (_consumer.loggerClient().isErrorEnabled())
				        	{
								
								StringBuilder temp = _consumer.consumerStrBuilder();
					        	temp.append("Received RDMDictionary refresh message with EnumTables but changed streamId")
					        		.append(OmmLoggerClient.CR)
					        		.append("Initial streamId ").append(_fldStreamId).append(OmmLoggerClient.CR)
					        		.append("New streamId ").append(rsslMsg.streamId());
				        	
					        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME,
					        									temp.toString(), Severity.ERROR));
				        	}
							return ReactorCallbackReturnCodes.SUCCESS;
						}
						break;
					}
					default: 
					{
						if (_consumer.loggerClient().isErrorEnabled())
			        	{
							StringBuilder temp = _consumer.consumerStrBuilder();
				        	temp.append("Received RDMDictionary message with unknown dictionary type").append(OmmLoggerClient.CR)
				        		.append("Dictionary type ").append(rsslRefresh.dictionaryType());
			        	
				        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME,
				        									temp.toString(), Severity.ERROR));
			        	}
						return ReactorCallbackReturnCodes.SUCCESS;
					}
				}
			}

			DecodeIterator dIter = _consumer.rsslDecIter();
			dIter.clear();
			if (CodecReturnCodes.SUCCESS != dIter.setBufferAndRWFVersion(rsslMsg.encodedDataBody(), rsslChannel.majorVersion(), rsslChannel.minorVersion()))
			{
				if (_consumer.loggerClient().isErrorEnabled())
	        	{
					StringBuilder temp = _consumer.consumerStrBuilder();
		        	temp.append("Internal error: failed to set buffer while decoding dictionary").append(OmmLoggerClient.CR)
		        		.append("Trying to set ").append(rsslChannel.majorVersion())
		        		.append(".").append(rsslChannel.minorVersion());;
	        	
		        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME,
		        									temp.toString(), Severity.ERROR));
	        	}
				return ReactorCallbackReturnCodes.SUCCESS;
			}

			com.thomsonreuters.upa.transport.Error rsslError = _consumer.dictionaryCallbackClient().rsslError();
			if (_fldStreamId == rsslMsg.streamId())
			{
				_rsslDictionary.clear();
	    		if (CodecReturnCodes.SUCCESS == _rsslDictionary.decodeFieldDictionary(dIter, 
	    																				com.thomsonreuters.upa.rdm.Dictionary.VerbosityValues.VERBOSE,
	    																				rsslError))
				{
					if (rsslRefresh.checkRefreshComplete())
					{
						_isFldLoaded = true;

						if (_consumer.loggerClient().isTraceEnabled())
			        	{
							StringBuilder temp = _consumer.consumerStrBuilder();
				        	temp.append("Received RDMDictionary refresh complete message").append(OmmLoggerClient.CR)
				        		.append("dictionary name ").append(rsslRefresh.dictionaryName().toString()).append(OmmLoggerClient.CR)
								.append("streamId ").append(rsslMsg.streamId());
			        	
				        	_consumer.loggerClient().trace(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME,
				        									temp.toString(), Severity.TRACE));
			        	}
					}
					else
						_isFldLoaded = false;
				}
				else
	    		{
					_isFldLoaded = false;

					if (_consumer.loggerClient().isErrorEnabled())
		        	{
						StringBuilder temp = _consumer.consumerStrBuilder();
			        	temp.append("Internal error: failed to decode FieldDictionary").append(OmmLoggerClient.CR)
			        		.append("Error text ").append(rsslError.text());
		        	
			        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME,
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
						
						if (_consumer.loggerClient().isTraceEnabled())
			        	{
							StringBuilder temp = _consumer.consumerStrBuilder();
				        	temp.append("Received RDMDictionary refresh complete message").append(OmmLoggerClient.CR)
				        		.append("dictionary name ").append(rsslRefresh.dictionaryName().toString()).append(OmmLoggerClient.CR)
								.append("streamId ").append(rsslMsg.streamId());
			        	
				        	_consumer.loggerClient().trace(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME,
				        									temp.toString(), Severity.TRACE));
			        	}
					}
					else
						_isEnumLoaded = false;
				}
				else
	    		{
					_isEnumLoaded = false;

					if (_consumer.loggerClient().isErrorEnabled())
		        	{
						StringBuilder temp = _consumer.consumerStrBuilder();
			        	temp.append("Internal error: failed to decode EnumTable dictionary").append(OmmLoggerClient.CR)
			        		.append("Error text ").append(rsslError.text());
		        	
			        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME,
			        									temp.toString(), Severity.ERROR));
		        	}
					
					return ReactorCallbackReturnCodes.SUCCESS;
	    		}
			}
			else
			{
				if (_consumer.loggerClient().isErrorEnabled())
	        	{
					StringBuilder temp = _consumer.consumerStrBuilder();
		        	temp.append("Received unexpected RDMDictionary refresh message on streamId ").append(OmmLoggerClient.CR)
		        		.append(rsslMsg.streamId());
	        	
		        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME,
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
					if (_consumer.loggerClient().isWarnEnabled())
		        	{
						StringBuilder temp = _consumer.consumerStrBuilder();
			        	temp.append("RDMDictionary stream was closed with status message").append(OmmLoggerClient.CR)
							.append("streamId ").append(rsslMsg.streamId()).append(OmmLoggerClient.CR)
							.append("Reason ").append(state.toString());
		        	
			        	_consumer.loggerClient().warn(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					notifyStatusToListener(_consumer, rsslStatus.state(), rsslMsg.streamId());
					break;
				}
				else if (state.dataState() == DataStates.SUSPECT)
				{
					if (_consumer.loggerClient().isWarnEnabled())
		        	{
						StringBuilder temp = _consumer.consumerStrBuilder();
			        	temp.append("RDMDictionary stream state was changed to suspect with status message").append(OmmLoggerClient.CR)
							.append("streamId ").append(rsslMsg.streamId()).append(OmmLoggerClient.CR)
							.append("Reason ").append(state.toString());
		        	
			        	_consumer.loggerClient().warn(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.WARNING));
		        	}
					
					notifyStatusToListener(_consumer, rsslStatus.state(), rsslMsg.streamId());
					break;
				}

				if (_consumer.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _consumer.consumerStrBuilder();
		        	temp.append("RDMDictionary stream was open with status message").append(OmmLoggerClient.CR)
						.append("streamId ").append(rsslMsg.streamId()).append(OmmLoggerClient.CR)
						.append("Reason ").append(state.toString());
	        	
		        	_consumer.loggerClient().trace(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.TRACE));
	        	}
			}
			else
			{
				if (_consumer.loggerClient().isWarnEnabled())
	        	{
					StringBuilder temp = _consumer.consumerStrBuilder();
		        	temp.append("Received RDMDictionary status message without the state").append(OmmLoggerClient.CR)
						.append("streamId ").append(rsslMsg.streamId()).append(OmmLoggerClient.CR);
	        	
		        	_consumer.loggerClient().warn(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.WARNING));
	        	}
			}
			break;
		}
		default:
		{
			if (_consumer.loggerClient().isErrorEnabled())
        	{
				
				StringBuilder temp = _consumer.consumerStrBuilder();
	        	temp.append("Received unknown RDMDictionary message type").append(OmmLoggerClient.CR)
	        		.append("message type ").append(msg.msgClass()).append(OmmLoggerClient.CR)
	        		.append("streamId ").append(msg.streamId()).append(OmmLoggerClient.CR);
        	
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(ChannelDictionary.CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
			break;
		}
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}
}

class DictionaryItem extends SingleItem implements TimeoutClient
{
	private static final String 	CLIENT_NAME = "DictionaryItem";
	
	private int						_rsslFilter;
	private int 					_currentFid;
	private boolean					_needRemoved;
	private boolean					_removed;
	private String	 				_name;
	
	
	DictionaryItem(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure)
	{
		super(consumer, consumerClient, closure, null);
		_rsslFilter = 0;
		_currentFid = 0;
		_needRemoved = false;
		_removed = false;
	}
	
	@Override
	void reset(OmmConsumerImpl consumer, OmmConsumerClient consumerClient, Object closure, Item item)
	{
		super.reset(consumer, consumerClient, closure, null);
		
		_rsslFilter = 0; 
		_currentFid = 0;
		_needRemoved = false;
	}

	@Override
	boolean open(ReqMsg reqMsg)
	{
		RequestMsg rsslReqMsg = ((ReqMsgImpl)reqMsg).rsslMsg();
		_name =  rsslReqMsg.msgKey().name().toString();
		DictionaryCallbackClient dictCBClient = _consumer.dictionaryCallbackClient();

		if (rsslReqMsg.msgKey().checkHasFilter())
			_rsslFilter = (int)rsslReqMsg.msgKey().filter();

		if (_name.equals("RWFFld"))
		{
			_currentFid = dictCBClient.defaultRsslDictionary().minFid();
			_streamId = dictCBClient.fldStreamId();
		}
		else if (_name.equals("RWFEnum"))
		{
			_currentFid = 0;
			_streamId = dictCBClient.enumStreamId();
		}

		DataDictionary rsslDictionary = dictCBClient.defaultRsslDictionary();

		if (rsslDictionary != null)
		{
			if (!dictCBClient.isLocalDictionary())
			{
				ChannelDictionary channelDict = dictCBClient.channelDictionaryList().get(0);
			
				channelDict.channelDictionaryLock().lock();

				channelDict.addListener(this);

				channelDict.channelDictionaryLock().unlock();
			}

			_consumer.addTimeoutEvent(500, this);
			
			return true;
		}
		else
		{
			if (_consumer.loggerClient().isErrorEnabled())
	        	_consumer.loggerClient().error(_consumer.formatLogMessage(DictionaryItem.CLIENT_NAME,
	        										"Consumer must have to receive a dictionary before open a dictionary request",
	        										Severity.ERROR));
			return false;
		}
	}
	
	@Override
	boolean modify(ReqMsg reqMsg)
	{
		return true;
	}
	
	@Override
	boolean submit(PostMsg postMsg)
	{
		return true;
	}
	
	@Override
	boolean submit(GenericMsg genericMsg)
	{
		return true;
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

				DictionaryCallbackClient dictCBClient = _consumer.dictionaryCallbackClient();
				if (dictCBClient.channelDictionaryList() != null && !dictCBClient.channelDictionaryList().isEmpty())
				{
					ChannelDictionary channelDict = dictCBClient.channelDictionaryList().get(0);
				
					channelDict.channelDictionaryLock().lock();

					channelDict.removeListener(this);

					channelDict.channelDictionaryLock().unlock();
				}

				_consumer.addTimeoutEvent(2000, this);
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
			_consumer.addTimeoutEvent(2000, this);
		}
	}
	
	@Override
	public void handleTimeoutEvent()
	{
		if (_needRemoved)
		{
			if(!_removed) 
			{
				_consumer.itemCallbackClient().removeFromMap(this);
				this.returnToPool();
				_removed = true;
			}
			return;
		}
		
		DictionaryCallbackClient dictCallbackClient = _consumer.dictionaryCallbackClient();
		DataDictionary rsslDictionary = dictCallbackClient.defaultRsslDictionary();
		ReactorChannel rsslChannel = _consumer.rsslReactorChannel();
		boolean firstPart = false;
		int ret = CodecReturnCodes.FAILURE;
		
		Buffer rsslDictEncBuffer = _consumer.dictionaryCallbackClient().rsslDictEncBuffer();

		if (rsslDictionary != null && rsslDictionary.enumTableCount() > 0 && rsslDictionary.numberOfEntries() > 0)
		{
			if (_name.equals("RWFFld"))
			{
				if (_currentFid == rsslDictionary.minFid())
					firstPart = true;
				
				ret = encodeDataDictionaryResp(	firstPart, rsslDictionary, rsslDictEncBuffer);
			}
			else if (_name.equals("RWFEnum"))
			{
				if (_currentFid == 0)
					firstPart = true;

				ret = encodeDataDictionaryResp(	firstPart, rsslDictionary, rsslDictEncBuffer);
			}

			if ((ret == CodecReturnCodes.SUCCESS) || (ret == CodecReturnCodes.DICT_PART_ENCODED))
			{
				dictCallbackClient.processRefreshMsg(rsslDictEncBuffer,
														rsslChannel.majorVersion(),
														rsslChannel.minorVersion(),
														this);
			}

			if (ret == CodecReturnCodes.DICT_PART_ENCODED)
			{
				_consumer.addTimeoutEvent(500,  this);
				return;
			}
			
			if (ret != CodecReturnCodes.SUCCESS)
			{
				EncodeIterator rsslEncIter = _consumer.rsslEncIter();
				StatusMsg rsslStatusMsg = dictCallbackClient.rsslStatusMsg();
				Buffer textBuffer = dictCallbackClient.rsslWrapBuffer();
				
			    rsslStatusMsg.msgClass(MsgClasses.STATUS);
		        rsslStatusMsg.streamId(_streamId);
		        rsslStatusMsg.domainType(DomainTypes.DICTIONARY);
		        rsslStatusMsg.containerType(DataTypes.NO_DATA);
		        rsslStatusMsg.applyHasState();
			    rsslStatusMsg.state().streamState(StreamStates.CLOSED);
			    rsslStatusMsg.state().dataState(DataStates.SUSPECT);
			    rsslStatusMsg.state().code(StateCodes.NONE);
			    textBuffer.data("Failed to provide data dictionary: Internal error.");
			    rsslStatusMsg.state().text(textBuffer);
			       
				rsslEncIter.clear();
				int retCode = rsslEncIter.setBufferAndRWFVersion(rsslDictEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion());
				if (retCode != CodecReturnCodes.SUCCESS)
				{
					if (_consumer.loggerClient().isErrorEnabled())
		        	{
			        	_consumer.loggerClient().error(_consumer.formatLogMessage(DictionaryItem.CLIENT_NAME, 
			        			"Internal error. Failed to set encode iterator RWF version in DictionatyItem.handleTimeoutEvent()",
			        									Severity.ERROR));
		        	}
					return;
				}
				
			    if ((retCode = rsslStatusMsg.encode(rsslEncIter)) != CodecReturnCodes.SUCCESS)
			    {
			    	if (_consumer.loggerClient().isErrorEnabled())
		        	{
			        	_consumer.loggerClient().error(_consumer.formatLogMessage(DictionaryItem.CLIENT_NAME, 
			        			"Internal error. Failed to encode msg in DictionatyItem.handleTimeoutEvent()",
			        									Severity.ERROR));
		        	}
			    	return;
			    }
			    	
				dictCallbackClient.processStatusMsg(rsslDictEncBuffer,	rsslChannel.majorVersion(),
													rsslChannel.minorVersion(),	this);
				return;
			}
		}
		else
		{
			EncodeIterator rsslEncIter = _consumer.rsslEncIter();
					
			StatusMsg rsslStatusMsg = dictCallbackClient.rsslStatusMsg();
			Buffer textBuffer = dictCallbackClient.rsslWrapBuffer();
		    rsslStatusMsg.msgClass(MsgClasses.STATUS);
	        rsslStatusMsg.streamId(_streamId);
	        rsslStatusMsg.domainType(DomainTypes.DICTIONARY);
	        rsslStatusMsg.containerType(DataTypes.NO_DATA);
	        rsslStatusMsg.applyHasState();
		    rsslStatusMsg.state().streamState(StreamStates.CLOSED_RECOVER);
		    rsslStatusMsg.state().dataState(DataStates.SUSPECT);
		    rsslStatusMsg.state().code(StateCodes.NONE);
		    textBuffer.data("Data dictionary is not ready to provide.");
		    rsslStatusMsg.state().text(textBuffer);
		       
			rsslEncIter.clear();
			int retCode = rsslEncIter.setBufferAndRWFVersion(rsslDictEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion());
			if (retCode != CodecReturnCodes.SUCCESS)
			{
				if (_consumer.loggerClient().isErrorEnabled())
	        	{
		        	_consumer.loggerClient().error(_consumer.formatLogMessage(DictionaryItem.CLIENT_NAME, 
		        			"Internal error. Failed to set encode iterator RWF version in DictionatyItem.handleTimeoutEvent()",
		        									Severity.ERROR));
	        	}
				return;
			}
			
		    if ((retCode = rsslStatusMsg.encode(rsslEncIter)) != CodecReturnCodes.SUCCESS)
		    {
		    	if (_consumer.loggerClient().isErrorEnabled())
	        	{
		        	_consumer.loggerClient().error(_consumer.formatLogMessage(DictionaryItem.CLIENT_NAME, 
		        			"Internal error. Failed to encode msg in DictionatyItem.handleTimeoutEvent()",
		        									Severity.ERROR));
	        	}
		    	return;
		    }
		    	
			dictCallbackClient.processStatusMsg(rsslDictEncBuffer,	rsslChannel.majorVersion(),
												rsslChannel.minorVersion(),	this);
			return;
		}
	}
	
	int encodeDataDictionaryResp(boolean firstMultiRefresh, DataDictionary rsslDataDictionary, Buffer rsslDictEncBuffer)
	{
		ReactorChannel rsslChannel = _consumer.rsslReactorChannel();
		EncodeIterator rsslEncIter = _consumer.rsslEncIter();
		DictionaryCallbackClient dictCallbackClient = _consumer.dictionaryCallbackClient();
		
		rsslEncIter.clear();
		int retCode = rsslEncIter.setBufferAndRWFVersion(rsslDictEncBuffer, rsslChannel.majorVersion(), rsslChannel.minorVersion());
		if (retCode != CodecReturnCodes.SUCCESS)
			return retCode;
		
		RefreshMsg rsslRefreshMsg = dictCallbackClient.rsslRefreshMsg();
		com.thomsonreuters.upa.transport.Error rsslError = dictCallbackClient.rsslError();

		rsslRefreshMsg.msgClass(MsgClasses.REFRESH);
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
		Buffer rsslDictBuffer = dictCallbackClient.rsslWrapBuffer();
		rsslDictBuffer.data(_name);
		rsslRefreshMsg.msgKey().name(rsslDictBuffer);
		
		rsslRefreshMsg.streamId(_streamId);
		
		Int rsslCurrentFid = dictCallbackClient.rsslCurrentFid();
		rsslCurrentFid.value(_currentFid);
		
		boolean complete = false;
		
		if ((retCode = rsslRefreshMsg.encodeInit(rsslEncIter, 0)) < CodecReturnCodes.SUCCESS)
			return retCode;

		if (_name.equals("RWFFld"))
		{
			retCode = rsslDataDictionary.encodeFieldDictionary(rsslEncIter, rsslCurrentFid, _rsslFilter, rsslError);
		}
		else if (_name.equals("RWFEnum"))
		{
			retCode = rsslDataDictionary.encodeEnumTypeDictionary(rsslEncIter, _rsslFilter, rsslError); 
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
			complete = true;
		
		if ((retCode = rsslRefreshMsg.encodeComplete(rsslEncIter, true)) < CodecReturnCodes.SUCCESS)
			return retCode;
		
		return complete ? CodecReturnCodes.SUCCESS : CodecReturnCodes.DICT_PART_ENCODED;
	}
}
	
	