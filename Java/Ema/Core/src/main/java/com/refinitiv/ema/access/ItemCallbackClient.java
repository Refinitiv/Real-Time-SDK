/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import java.util.*;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.ema.access.DirectoryServiceStore.ServiceIdInteger;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.access.OmmState.StreamState;
import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.rdm.SymbolList;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.common.VaNode;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.reactor.*;

import static com.refinitiv.eta.codec.Codec.majorVersion;

interface CallbackRsslMsgPool
{
	com.refinitiv.eta.codec.RequestMsg rsslRequestMsg();
	
	com.refinitiv.eta.codec.RefreshMsg rsslRefreshMsg();
	
	com.refinitiv.eta.codec.StatusMsg rsslStatusMsg();
	
	com.refinitiv.eta.codec.CloseMsg rsslCloseMsg();
}

class CallbackClient<T> implements CallbackRsslMsgPool
{
	protected RefreshMsgImpl			_refreshMsg;
	protected UpdateMsgImpl				_updateMsg;
	protected StatusMsgImpl				_statusMsg;
	protected GenericMsgImpl			_genericMsg;
	protected AckMsgImpl				_ackMsg;
	protected OmmEventImpl<T>			_eventImpl;
	protected OmmCommonImpl				_baseImpl;
	protected RefreshMsg				_rsslRefreshMsg;  
	protected CloseMsg					_rsslCloseMsg;
	protected RequestMsg 				_rsslRequestMsg;
	protected StatusMsg					_rsslStatusMsg;
	protected LoginRefresh				_loginRefresh;
	

	CallbackClient(OmmBaseImpl<T> baseImpl, String clientName)
	{
		_baseImpl = baseImpl;
		_eventImpl = new OmmEventImpl<T>(baseImpl);
		_refreshMsg = new RefreshMsgImpl(_baseImpl.objManager());
		
		if  ( _baseImpl.loggerClient().isTraceEnabled() )
		{
			String temp = "Created " + clientName;
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(clientName,
											temp, Severity.TRACE).toString());
		}
	}
	
	CallbackClient(OmmServerBaseImpl baseImpl, String clientName)
	{
		_baseImpl = baseImpl;
		_eventImpl = new OmmEventImpl<T>();
		_refreshMsg = new RefreshMsgImpl(_baseImpl.objManager());
		
		if  ( _baseImpl.loggerClient().isTraceEnabled() )
		{
			String temp = "Created " + clientName;
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(clientName,
											temp, Severity.TRACE).toString());
		}
	}
	
	public com.refinitiv.eta.codec.RequestMsg rsslRequestMsg()
	{
		if (_rsslRequestMsg == null)
			_rsslRequestMsg = (RequestMsg)CodecFactory.createMsg();
		else
			_rsslRequestMsg.clear();
		
		_rsslRequestMsg.msgClass(MsgClasses.REQUEST);
		return _rsslRequestMsg;
	}
	
	public com.refinitiv.eta.codec.RefreshMsg rsslRefreshMsg()
	{
		if (_rsslRefreshMsg == null)
			_rsslRefreshMsg = (RefreshMsg)CodecFactory.createMsg();
		else
			_rsslRefreshMsg.clear();
		
		_rsslRefreshMsg.msgClass(MsgClasses.REFRESH);
		return _rsslRefreshMsg;
	}
	
	public com.refinitiv.eta.codec.StatusMsg rsslStatusMsg()
	{
		if (_rsslStatusMsg == null)
			_rsslStatusMsg = (StatusMsg)CodecFactory.createMsg();
		else
			_rsslStatusMsg.clear();
		
		_rsslStatusMsg.msgClass(MsgClasses.STATUS);
		return _rsslStatusMsg;
	}
	
	public com.refinitiv.eta.codec.CloseMsg rsslCloseMsg()
	{
		if (_rsslCloseMsg == null)
			_rsslCloseMsg = (CloseMsg)CodecFactory.createMsg();
		else
			_rsslCloseMsg.clear();
		
		_rsslCloseMsg.msgClass(MsgClasses.CLOSE);
		return _rsslCloseMsg;
	}
	
	public com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh loginRefreshMsg()
	{
		if (_loginRefresh == null)
		{
			_loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
			_loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
		}
		
		return _loginRefresh;
	}
	
	void notifyOnAllMsg(com.refinitiv.ema.access.Msg msg) {}
    void notifyOnRefreshMsg() {}
	void notifyOnUpdateMsg() {}
	void notifyOnStatusMsg() {}
	void notifyOnGenericMsg() {} 
	void notifyOnAckMsg() {}
}

class TunnelItem<T> extends Item<T> {

	private static final String CLIENT_NAME = "TunnelItem";
	private int _nextSubItemStreamId;
	private LinkedList<IntObject> _returnedSubItemStreamIds;
	private Directory<T> _directory;
	private TunnelStream _rsslTunnelStream;
	private ArrayList<Item<T>> _subItems;
	private static final int STARTING_SUBITEM_STREAMID = 5;
	protected OmmBaseImpl<T>	_baseImpl;

	TunnelItem()
	{
	}

	TunnelItem(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> parent)
	{
		super(client, closure, null);
		
		_baseImpl = baseImpl;

		_directory = null;
		_rsslTunnelStream = null;
		_nextSubItemStreamId = STARTING_SUBITEM_STREAMID;
		_subItems = new ArrayList<Item<T>>(32);
		while (STARTING_SUBITEM_STREAMID >= _subItems.size())
			_subItems.add(null);
		_returnedSubItemStreamIds = new LinkedList<IntObject>();
	}

	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> parent)
	{
		super.reset(client, closure, null);

		_baseImpl = baseImpl;
		
		_directory = null;
		_rsslTunnelStream = null;
		_nextSubItemStreamId = STARTING_SUBITEM_STREAMID;
		_subItems = new ArrayList<Item<T>>(32);
		while (STARTING_SUBITEM_STREAMID >= _subItems.size())
			_subItems.add(null);
		_returnedSubItemStreamIds = new LinkedList<IntObject>();
	}

	int subItemStreamId()
	{
		if (_returnedSubItemStreamIds.isEmpty())
		{
			return ++_nextSubItemStreamId;
		}

		IntObject streamId = _returnedSubItemStreamIds.pop();
		streamId.returnToPool();
		return streamId.value();
	}

	void returnSubItemStreamId(int subItemStreamId)
	{
		IntObject streamId = _baseImpl._objManager.createIntObject().value(subItemStreamId);
		_returnedSubItemStreamIds.push(streamId);
	}

	int addSubItem(Item<T> subItem, int streamId)
	{

		if (streamId == 0)
		{
			streamId = subItemStreamId();
		} else
		{
			if (streamId < STARTING_SUBITEM_STREAMID)
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append(
						"Invalid attempt to open a sub stream with streamId smaller than starting stream id. Passed in stream id is ")
						.append(streamId);

				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);

			}

			Boolean foundReturnedStreamId = false;

			Iterator<IntObject> iter = _returnedSubItemStreamIds.iterator();
			while (iter.hasNext()) 
			{
			    IntObject intObj = iter.next();
			    if (intObj.value() == streamId) 
			    {
			    	iter.remove();
			        intObj.returnToPool();
			        foundReturnedStreamId = true;
			        break;
			    }
			}

			if (!foundReturnedStreamId)
			{
				if ((streamId < _subItems.size()) && (_subItems.get(streamId) != null))
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Invalid attempt to open a substream: substream streamId (").append(streamId)
							.append(") is already in use");

					if (_baseImpl.loggerClient().isErrorEnabled())
						_baseImpl.loggerClient().error(
								_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

					_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
			}
		}

		while (streamId >= _subItems.size())
			_subItems.add(null);

		_subItems.set(streamId, subItem);

		return streamId;
	}

	void removeSubItem(int streamId)
	{
		if (streamId < STARTING_SUBITEM_STREAMID)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal error. Current stream Id in removeSubItem is less than the starting stream id.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);
		}

		if (streamId >= _subItems.size())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal error. Current stream Id in removeSubItem is greater than the starting stream id.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);
		}

		if (_subItems.get(streamId) != null)
			_subItems.set(streamId, null);
	}

	Item<T> getSubItem(int streamId)
	{
		if (streamId < STARTING_SUBITEM_STREAMID)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal error. Current stream Id in getSubItem is less than the starting stream id.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);

			return null;
		}

		if (streamId >= _subItems.size())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal error. Current stream Id in getSubItem is greater than the starting stream id.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);

			return null;
		}

		return _subItems.get(streamId);

	}

	void scheduleItemClosedStatus(TunnelStreamRequest tunnelStreamRequest, String text)
	{
		if (_closedStatusClient != null) return;

		_closedStatusClient = new ClosedStatusClient<T>(_baseImpl.itemCallbackClient(), this, tunnelStreamRequest,
				text);

		_baseImpl.addTimeoutEvent(1000, _closedStatusClient);

	}

	void rsslTunnelStream(TunnelStream rsslTunnelStream)
	{
		_rsslTunnelStream = rsslTunnelStream;
	}

	boolean open(TunnelStreamRequest tunnelStreamRequest)
	{
		Directory<T> directory = null;
		ConsumerSession<T> consumerSession = _baseImpl.consumerSession();
		SessionDirectory<T> sessionDirectory = null;

		if (tunnelStreamRequest.hasServiceName())
		{
			if(consumerSession != null)
			{
				sessionDirectory = consumerSession.sessionDirectoryByName(tunnelStreamRequest.serviceName());
				
				if(sessionDirectory != null)
				{
					directory = sessionDirectory.directory();
				}
				
				if(directory == null)
				{
					StringBuilder temp = new StringBuilder();
					temp.append("Service name of  ").append(tunnelStreamRequest.serviceName())
							.append(" is not found.");

					if (_baseImpl.loggerClient().isErrorEnabled())
						_baseImpl.loggerClient()
								.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
					
					/* Assign a valid handle to this request.  This will be valid until the closed status event is given to the user */
					_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);

					scheduleItemClosedStatus(tunnelStreamRequest, temp.toString());

					return true;
				}
			}
			else
			{
				directory = _baseImpl.directoryCallbackClient().directory(tunnelStreamRequest.serviceName());
				
				if (directory == null && (!_baseImpl.loginCallbackClient().loginRefreshMsg().attrib().checkHasSingleOpen() || _baseImpl.loginCallbackClient().loginRefreshMsg().attrib().singleOpen() == 0))
				{
					StringBuilder temp = new StringBuilder();
					temp.append("Service name of  ").append(tunnelStreamRequest.serviceName())
							.append(" is not found.");

					if (_baseImpl.loggerClient().isErrorEnabled())
						_baseImpl.loggerClient()
								.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
					
					/* Assign a valid handle to this request.  This will be valid until the closed status event is given to the user */
					_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);

					scheduleItemClosedStatus(tunnelStreamRequest, temp.toString());

					return true;
				}
			}
		} 
		else if (tunnelStreamRequest.hasServiceId())
		{
			if(consumerSession != null)
			{
				sessionDirectory = consumerSession.sessionDirectoryById(tunnelStreamRequest.serviceId());
				
				if(sessionDirectory != null)
				{
					directory = sessionDirectory.directory();
				}
				
				if (directory == null)
				{
					StringBuilder temp = new StringBuilder();
					temp.append("Service id of  ").append(tunnelStreamRequest.serviceId()).append(" is not found.");

					if (_baseImpl.loggerClient().isErrorEnabled())
						_baseImpl.loggerClient()
								.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
					
					/* Assign a valid handle to this request.  This will be valid until the closed status event is given to the user */
					_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);

					scheduleItemClosedStatus(tunnelStreamRequest, temp.toString());

					return true;
				}
			}
			else
			{
				directory = _baseImpl.directoryCallbackClient().directory(tunnelStreamRequest.serviceId());
				if (directory == null && (!_baseImpl.loginCallbackClient().loginRefreshMsg().attrib().checkHasSingleOpen() || _baseImpl.loginCallbackClient().loginRefreshMsg().attrib().singleOpen() == 0))
				{
					StringBuilder temp = new StringBuilder();
					temp.append("Service id of  ").append(tunnelStreamRequest.serviceId()).append(" is not found.");

					if (_baseImpl.loggerClient().isErrorEnabled())
						_baseImpl.loggerClient()
								.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
					
					/* Assign a valid handle to this request.  This will be valid until the closed status event is given to the user */
					_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);

					scheduleItemClosedStatus(tunnelStreamRequest, temp.toString());

					return true;
				}
			}
		}

		_directory = directory;

		return submit(tunnelStreamRequest);
	}

	@Override
	boolean open(ReqMsg reqMsg)
	{

		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to open tunnel stream using ReqMsg.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	boolean modify(ReqMsg reqMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to reissue tunnel stream using ReqMsg.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	boolean submit(PostMsg postMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit PostMsg on tunnel stream.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	boolean submit(GenericMsg genericMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit GenericMsg on tunnel stream.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	boolean submit(TunnelStreamRequest tunnelStreamRequest)
	{
		_domainType = tunnelStreamRequest.domainType();
		_streamId = getNextStreamId(0);
		_baseImpl._itemCallbackClient.addToMap(_baseImpl.nextLongId(), this);

		_baseImpl.rsslErrorInfo().clear();

		TunnelStreamOpenOptions tsOpenOptions = ReactorFactory.createTunnelStreamOpenOptions();
		
		tsOpenOptions.clear();

		tsOpenOptions.domainType(_domainType);
		tsOpenOptions.name(tunnelStreamRequest.name());
		ChannelInfo channelInfo = null;
		ReactorChannel rsslChannel = null;
		if(_baseImpl.consumerSession() != null)
		{
			channelInfo = _directory.channelInfo();
			rsslChannel = channelInfo.rsslReactorChannel();
		}
		else
		{
			channelInfo = _baseImpl.loginCallbackClient().activeChannelInfo();
			rsslChannel = (channelInfo != null) ? channelInfo.rsslReactorChannel() : null;
		}

		if (tunnelStreamRequest.hasLoginReqMsg())
		{

			DecodeIterator dIter = _baseImpl.rsslDecIter();
			dIter.clear();

			if (ReactorReturnCodes.SUCCESS > (dIter.setBufferAndRWFVersion(
					((TunnelStreamRequestImpl) tunnelStreamRequest).tunnelStreamLoginReqMsg().buffer(),
					channelInfo._majorVersion, channelInfo._minorVersion)))
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Internal Error. Failed to set decode iterator version in TunnelItem.submit");

				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);

				return false;
			}

			if (_baseImpl._itemCallbackClient._rsslRDMLoginMsg == null)
				_baseImpl._itemCallbackClient._rsslRDMLoginMsg = LoginMsgFactory.createMsg();
			else
				_baseImpl._itemCallbackClient._rsslRDMLoginMsg.clear();

			_baseImpl._itemCallbackClient._rsslRDMLoginMsg.rdmMsgType(LoginMsgType.REQUEST);

			if (ReactorReturnCodes.SUCCESS > _baseImpl._itemCallbackClient._rsslRDMLoginMsg.decode(dIter,
					((TunnelStreamRequestImpl) tunnelStreamRequest).tunnelStreamLoginReqMsg().rsslMsg()))
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Internal Error. Failed to decode login request in TunnelItem.submit");

				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);

				return false;
			}

			tsOpenOptions.authLoginRequest((LoginRequest) _baseImpl._itemCallbackClient._rsslRDMLoginMsg);
		}

		tsOpenOptions.guaranteedOutputBuffers(tunnelStreamRequest.guaranteedOutputBuffers());
		tsOpenOptions.responseTimeout(tunnelStreamRequest.responseTimeOut());
		if (_directory != null)
			tsOpenOptions.serviceId(_directory.service().serviceId());
		else
			tsOpenOptions.serviceId(tunnelStreamRequest.serviceId());
		tsOpenOptions.streamId(_streamId);

		tsOpenOptions.userSpecObject(this);

		tsOpenOptions.defaultMsgCallback(_baseImpl.itemCallbackClient());
		tsOpenOptions.queueMsgCallback(_baseImpl.itemCallbackClient());
		tsOpenOptions.statusEventCallback(_baseImpl.itemCallbackClient());

		ClassOfService cos = tunnelStreamRequest.classOfService();
		tsOpenOptions.classOfService().common().maxMsgSize(cos.common().maxMsgSize());

		tsOpenOptions.classOfService().authentication().type(cos.authentication().type());

		tsOpenOptions.classOfService().dataIntegrity().type(cos.dataIntegrity().type());

		tsOpenOptions.classOfService().flowControl().recvWindowSize(cos.flowControl().recvWindowSize());
		tsOpenOptions.classOfService().flowControl().type(cos.flowControl().type());

		tsOpenOptions.classOfService().guarantee().type(cos.guarantee().type());
		tsOpenOptions.classOfService().guarantee().persistLocally(cos.guarantee().persistedLocally());
		tsOpenOptions.classOfService().guarantee().persistenceFilePath(cos.guarantee().persistenceFilePath());

		if (rsslChannel == null)
		{
			StringBuilder message = _baseImpl.strBuilder();

			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				message.append("Internal error: rsslChannel.Submit() failed in TunnelItem.submit(TunnelStreamRequest)").append(OmmLoggerClient.CR)
					.append("\tReactorChannel is not available");

				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, message.toString(), Severity.ERROR));

				message.setLength(0);
			}

			message.append("Failed to open tunnel stream request. Reason: ReactorChannel is not available");

			_baseImpl.handleInvalidUsage(message.toString(), ReactorReturnCodes.FAILURE);

			return false;
		}

		if (rsslChannel.openTunnelStream(tsOpenOptions, _baseImpl.rsslErrorInfo()) != ReactorCallbackReturnCodes.SUCCESS)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append(
					"Failed to open tunnel stream request in TunnelItem.submit(TunnelStreamRequest ")
					.append(_baseImpl.rsslErrorInfo().toString());

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), _baseImpl.rsslErrorInfo().code());

			return false;
		}

		return true;
	}

	@Override
	boolean close()
	{
		_baseImpl.rsslErrorInfo().clear();

		if (_rsslTunnelStream.close(false, _baseImpl.rsslErrorInfo()) != ReactorReturnCodes.SUCCESS)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal Error. TunnelStream.close() failed in TunnelItem.close()").append(OmmLoggerClient.CR)
					.append("Error Id").append(_baseImpl.rsslErrorInfo().error().errorId()).append(OmmLoggerClient.CR)
					.append("Internal sysError").append(_baseImpl.rsslErrorInfo().error().sysError())
					.append(OmmLoggerClient.CR).append("Error Text").append(_baseImpl.rsslErrorInfo().error().text())
					.append(OmmLoggerClient.CR);

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), _baseImpl.rsslErrorInfo().code());

			return false;
		}

		remove();
		return true;

	}

	boolean submitSubItemMsg(Msg msg)
	{
		int bufSize = 256;
		int retCode = 0;
		_baseImpl.rsslErrorInfo().clear();

		if (_rsslTunnelStream == null)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal Error. Tunnel stream is not open yet");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);

			return false;
		}
		
		TransportBuffer tunnelStreamBuf = _rsslTunnelStream.getBuffer(bufSize, _baseImpl.rsslErrorInfo());

		if (tunnelStreamBuf == null)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal Error. Failed to allocated TransportBuffer in TunnelItem.submitSubItemMsg");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), _baseImpl.rsslErrorInfo().code());

			return false;
		}

		_baseImpl.rsslEncIter().clear();
		int ret;
		
		 ChannelInfo channelInfo = null;
		 
        if(_baseImpl.consumerSession() != null)
        {
        	channelInfo = _directory.channelInfo();
        }
        else
        {
        	channelInfo = _baseImpl.loginCallbackClient().activeChannelInfo();
        }

		
		if (ReactorReturnCodes.SUCCESS > (ret = _baseImpl.rsslEncIter().setBufferAndRWFVersion(tunnelStreamBuf,
				channelInfo._majorVersion, channelInfo._minorVersion)))
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal Error. Failed to set encode iterator in TunnelItem.submitSubItemMsg");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), ret);

			return false;
		}

		while ((retCode = msg.encode(_baseImpl.rsslEncIter())) == CodecReturnCodes.BUFFER_TOO_SMALL)
		{
			_rsslTunnelStream.releaseBuffer(tunnelStreamBuf, _baseImpl.rsslErrorInfo());

			bufSize += bufSize;
			tunnelStreamBuf = _rsslTunnelStream.getBuffer(bufSize, _baseImpl.rsslErrorInfo());
			if (tunnelStreamBuf == null)
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Internal Error. Failed to allocated TransportBuffer in TunnelItem.submitSubItemMsg");

				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), _baseImpl.rsslErrorInfo().code());

				return false;
			}
			_baseImpl.rsslEncIter().clear();
			if (ReactorReturnCodes.SUCCESS > (ret = _baseImpl.rsslEncIter().setBufferAndRWFVersion(tunnelStreamBuf,
					channelInfo._majorVersion, channelInfo._minorVersion)))
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Internal Error. Failed to set encode iterator in in TunnelItem.submitSubItemMsg");

				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), ret);

				return false;
			}

		}

		if (retCode != ReactorReturnCodes.SUCCESS)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal Error. Failed to encode message in TunnelItem.submitSubItemMsg");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), retCode);

			return false;
		}

		TunnelStreamSubmitOptions tunnelStreamSubmitOptions = _baseImpl.rsslTunnelStreamSubmitOptions();
		tunnelStreamSubmitOptions.containerType(DataTypes.MSG);

		retCode = _rsslTunnelStream.submit(tunnelStreamBuf, tunnelStreamSubmitOptions, _baseImpl.rsslErrorInfo());
		if (retCode != ReactorReturnCodes.SUCCESS)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Internal Error. Failed to submit message in TunnelItem.submitSubItemMsg");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient()
						.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidUsage(temp.toString(), retCode);

			return false;
		}

		return true;
	}

	@Override
	void remove()
	{
		int subItemSize = _subItems.size();
		SubItem<T> item;
		for (int i = 0; i < subItemSize; i++)
		{
			item = ((SubItem<T>) _subItems.get(i));
			if (item != null)
				item.remove();
		}
		_subItems.clear();

		_baseImpl.itemCallbackClient().removeFromMap(this, true);
	}

	@Override
	int type()
	{
		return Item.ItemType.TUNNEL_ITEM;
	}

	@Override
	Directory<T> directory()
	{
		return _directory;
	}

	@Override
	boolean submit(com.refinitiv.ema.access.RefreshMsg refreshMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit RefreshMsg on tunnel stream.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	boolean submit(UpdateMsg updateMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit UpdateMsg on tunnel stream.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	boolean submit(com.refinitiv.ema.access.StatusMsg statusMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit StatusMsg on tunnel stream.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(TunnelItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	int getNextStreamId(int numOfItem) {
		return _baseImpl._itemCallbackClient.nextStreamId(numOfItem);
	}
}

class SubItem<T> extends Item<T>
{

	private static final String CLIENT_NAME = "SubItem";
	Directory<T> _directory;
	protected OmmBaseImpl<T>	_baseImpl;

	SubItem()
	{
	}

	SubItem(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> parent)
	{
		super(client, closure, parent);
		_baseImpl = baseImpl;
		_directory = null;
	}

	
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> parent)
	{
		super.reset(client, closure, parent);
		_baseImpl = baseImpl;
		_directory = null;
	}

	@Override
	boolean open(ReqMsg reqMsg)
	{
		if (reqMsg.hasServiceName())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Invalid attempt to open sub stream using serviceName.");
			
			/* Assign a valid handle to this request.  This will be valid until the closed status event is given to the user */
			_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);

			scheduleItemClosedStatus(_baseImpl.itemCallbackClient(), this, ((ReqMsgImpl) reqMsg).rsslMsg(),
					temp.toString(), reqMsg.serviceName());

			return true;
		}
		
		if (reqMsg.streamId() == 0)
		{
			_streamId = ((TunnelItem<T>) (_parent)).addSubItem(this, 0);
			reqMsg.streamId(_streamId);
		} 
		else
		{
			if (reqMsg.streamId() < 0)
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Invalid attempt to assign negative streamid to a substream.");

				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(SubItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);

				return false;
			}
			else
			{
				_streamId = ((TunnelItem<T>) (_parent)).addSubItem(this, reqMsg.streamId());
			}
		}

		_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);
		_domainType = reqMsg.domainType();

		return ((TunnelItem<T>) (_parent)).submitSubItemMsg(((ReqMsgImpl) reqMsg).rsslMsg());
	}

	@Override
	boolean modify(ReqMsg reqMsg)
	{
		reqMsg.streamId(_streamId);
		return ((TunnelItem<T>) (_parent)).submitSubItemMsg(((ReqMsgImpl) reqMsg).rsslMsg());
	}

	@Override
	boolean submit(PostMsg postMsg)
	{
		postMsg.streamId(_streamId);
		return ((TunnelItem<T>) (_parent)).submitSubItemMsg(((PostMsgImpl) postMsg).rsslMsg());
	}

	@Override
	boolean submit(GenericMsg genericMsg)
	{
		genericMsg.streamId(_streamId);
		if (genericMsg.domainType() == 0)
			genericMsg.domainType(_domainType);

		return ((TunnelItem<T>) (_parent)).submitSubItemMsg(((GenericMsgImpl) genericMsg).rsslMsg());
	}

	@Override
	boolean close()
	{
		CloseMsg rsslCloseMsg = _baseImpl.itemCallbackClient().rsslCloseMsg();
		rsslCloseMsg.containerType(DataTypes.NO_DATA);
		rsslCloseMsg.domainType(_domainType);
		rsslCloseMsg.streamId(_streamId);
		rsslCloseMsg.streamId(_streamId);

		boolean retCode = ((TunnelItem<T>) (_parent)).submitSubItemMsg((rsslCloseMsg));

		remove();

		return retCode;

	}

	@Override
	void remove()
	{
		((TunnelItem<T>) (_parent)).removeSubItem(_streamId);
		((TunnelItem<T>) (_parent)).returnSubItemStreamId(_streamId);
		_baseImpl.itemCallbackClient().removeFromMap(this, true);
	}

	@Override
	int type()
	{
		return ItemType.SUB_ITEM;
	}

	@Override
	Directory<T> directory()
	{
		return _directory;
	}

	@Override
	boolean submit(com.refinitiv.ema.access.RefreshMsg refreshMsg)
	{
		refreshMsg.streamId(_streamId);
		return ((TunnelItem<T>) (_parent)).submitSubItemMsg((Msg) refreshMsg);
	}

	@Override
	boolean submit(UpdateMsg updateMsg)
	{
		updateMsg.streamId(_streamId);
		return ((TunnelItem<T>) (_parent)).submitSubItemMsg((Msg) updateMsg);
	}

	@Override
	boolean submit(com.refinitiv.ema.access.StatusMsg statusMsg)
	{
		statusMsg.streamId(_streamId);
		return ((TunnelItem<T>) (_parent)).submitSubItemMsg((Msg) statusMsg);
	}
	
	void scheduleItemClosedStatus(CallbackClient<T> client, Item<T> item, Msg rsslMsg, String statusText, String serviceName)
	{
		if (_closedStatusClient != null) return;
    	
		_closedStatusClient = new ClosedStatusClient<T>(client, item, rsslMsg, statusText, serviceName);
		_baseImpl.addTimeoutEvent(1000, _closedStatusClient);
	}
	
	@Override
	int getNextStreamId(int numOfItem) {
		return _baseImpl._itemCallbackClient.nextStreamId(numOfItem);
	}

}

class ItemCallbackClient<T> extends CallbackClient<T> implements DefaultMsgCallback, TunnelStreamDefaultMsgCallback, TunnelStreamQueueMsgCallback, 
TunnelStreamStatusEventCallback
{
	private static final String CLIENT_NAME = "ItemCallbackClient";
	private static final int  CONSUMER_STARTING_STREAM_ID = 4;
	private static final int  PROVIDER_STARTING_STREAM_ID = 0;
	private static final int CONSUMER_MAX_STREAM_ID_MINUSONE = Integer.MAX_VALUE -1;
	
	private Map<LongObject, Item<T>>	_itemMap;
	private Map<IntObject, Item<T>> _streamIdMap;
	private LongObject _longObjHolder;
	private IntObject _intObjHolder;
	protected LoginMsg _rsslRDMLoginMsg;
	private int	_nextStreamId;
	boolean	_nextStreamIdWrapAround;
	private ReentrantLock _streamIdAccessLock;
	private ConsumerSession<T> _consumerSession; /* This is used when there is a consumer session */

	HashMap<ReactorChannel, HashMap<Integer, Item<T>>> _providerDrivenItemsByReactorChannel = new HashMap<>();
	int _nextProviderStreamId = 0;

	ItemCallbackClient(OmmBaseImpl<T> baseImpl)
	{
		super(baseImpl, CLIENT_NAME);
		
		_consumerSession = baseImpl.consumerSession();
		
		if(_consumerSession != null)
		{	/* Gets HashMap from the watchlist when the consumer session is enabled. */
			_itemMap = _consumerSession.watchlist().itemHandleMap();
			_streamIdMap = _consumerSession.watchlist().streamIdMap();
			_consumerSession.watchlist().callbackClient(this);
		}
		else
		{
			if(baseImpl.activeConfig().userDispatch == OmmConsumerConfig.OperationModel.API_DISPATCH)
			{
				_itemMap = new ConcurrentHashMap<>( baseImpl.activeConfig().itemCountHint == 0 ? 1024 : baseImpl.activeConfig().itemCountHint);
				_streamIdMap = new ConcurrentHashMap<>( baseImpl.activeConfig().itemCountHint == 0 ? 1024 : baseImpl.activeConfig().itemCountHint);
			}
			else
			{
				_itemMap = new HashMap<>( baseImpl.activeConfig().itemCountHint == 0 ? 1024 : baseImpl.activeConfig().itemCountHint);
				_streamIdMap = new HashMap<>( baseImpl.activeConfig().itemCountHint == 0 ? 1024 : baseImpl.activeConfig().itemCountHint);
			}
		}

		_updateMsg = new UpdateMsgImpl(_baseImpl.objManager());
		
		_longObjHolder = new LongObject();
		_intObjHolder = new IntObject();
		
		if ( baseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER )
		{
			_nextStreamId = CONSUMER_STARTING_STREAM_ID;
		}
		else
		{
			_nextStreamId = PROVIDER_STARTING_STREAM_ID;
		}
		
		_nextStreamIdWrapAround = false;
	}
	
	ItemCallbackClient(OmmServerBaseImpl baseImpl)
	{
		super(baseImpl, CLIENT_NAME);

		if(baseImpl.activeConfig().userDispatch == OmmConsumerConfig.OperationModel.API_DISPATCH)
		{
			_itemMap = new ConcurrentHashMap<>( baseImpl.activeConfig().itemCountHint == 0 ? 1024 : baseImpl.activeConfig().itemCountHint);
			_streamIdMap = new ConcurrentHashMap<>( baseImpl.activeConfig().itemCountHint == 0 ? 1024 : baseImpl.activeConfig().itemCountHint);
		}
		else
		{
			_itemMap = new HashMap<>( baseImpl.activeConfig().itemCountHint == 0 ? 1024 : baseImpl.activeConfig().itemCountHint);
			_streamIdMap = new HashMap<>( baseImpl.activeConfig().itemCountHint == 0 ? 1024 : baseImpl.activeConfig().itemCountHint);
		}

		_updateMsg = new UpdateMsgImpl(_baseImpl.objManager());
		
		_longObjHolder = new LongObject();
		_intObjHolder = new IntObject();
		
		_nextStreamId = PROVIDER_STARTING_STREAM_ID;
		_nextStreamIdWrapAround = false;
	}

	void initialize() {}

	@SuppressWarnings("unchecked")
	@Override
	public int statusEventCallback(TunnelStreamStatusEvent tunnelStreamStatusEvent)
	{
		_baseImpl.eventReceived();

		if (tunnelStreamStatusEvent.tunnelStream() == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append(
						"Received a tunnel stream message event without the tunnel stream object in ItemCallbackClient.statusEventCallback")
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(tunnelStreamStatusEvent.reactorChannel().reactor().hashCode()));

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			return ReactorCallbackReturnCodes.SUCCESS;
		}

		ChannelInfo channelInfo = (ChannelInfo) tunnelStreamStatusEvent.reactorChannel().userSpecObj();
		_eventImpl._item = (Item<T>) (tunnelStreamStatusEvent.tunnelStream().userSpecObject());
		if (_eventImpl._item == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				com.refinitiv.eta.transport.Error error = tunnelStreamStatusEvent.errorInfo().error();

				StringBuilder temp = _baseImpl.strBuilder();
				temp.append(
						"Received a tunnel stream status event without the userSpecObject ItemCallbackClient.statusEventCallback")
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(channelInfo.rsslReactor().hashCode())).append(OmmLoggerClient.CR)
						.append("RsslChannel ")
						.append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0))
						.append(OmmLoggerClient.CR).append("Error Id ").append(error.errorId())
						.append(OmmLoggerClient.CR).append("Internal sysError ").append(error.sysError())
						.append(OmmLoggerClient.CR).append("Error Location ")
						.append(tunnelStreamStatusEvent.errorInfo().location()).append(OmmLoggerClient.CR)
						.append("Error Text ").append(error.text());

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			return ReactorCallbackReturnCodes.SUCCESS;
		}

		StatusMsg rsslStatusMsg = rsslStatusMsg();
		rsslStatusMsg.flags(StatusMsgFlags.PRIVATE_STREAM | StatusMsgFlags.CLEAR_CACHE | StatusMsgFlags.HAS_MSG_KEY);
		rsslStatusMsg.containerType(DataTypes.NO_DATA);
		rsslStatusMsg.domainType(tunnelStreamStatusEvent.tunnelStream().domainType());
		rsslStatusMsg.streamId(tunnelStreamStatusEvent.tunnelStream().streamId());

		if (tunnelStreamStatusEvent.state() != null)
		{
			rsslStatusMsg.state(tunnelStreamStatusEvent.state());
			rsslStatusMsg.flags(rsslStatusMsg.flags() | StatusMsgFlags.HAS_STATE);
		}

		rsslStatusMsg.msgKey().flags(MsgKeyFlags.HAS_NAME);
		rsslStatusMsg.msgKey().name().data(tunnelStreamStatusEvent.tunnelStream().name());

		rsslStatusMsg.msgKey().flags(rsslStatusMsg.msgKey().flags() | MsgKeyFlags.HAS_SERVICE_ID);
		rsslStatusMsg.msgKey().serviceId(tunnelStreamStatusEvent.tunnelStream().serviceId());

		if (_statusMsg == null)
			_statusMsg = new StatusMsgImpl(_baseImpl.objManager());

		_statusMsg.decode(rsslStatusMsg, tunnelStreamStatusEvent.reactorChannel().majorVersion(),
				tunnelStreamStatusEvent.reactorChannel().minorVersion(), channelInfo.rsslDictionary());

		((TunnelItem<T>) (_eventImpl._item)).rsslTunnelStream(tunnelStreamStatusEvent.tunnelStream());
		
		if(_eventImpl._item.directory() != null)
		{
			_statusMsg.service(_eventImpl._item.directory().serviceName());
			
			if(_eventImpl._item.directory().hasGeneratedServiceId()) /* Gets the mapping alternate service Id if any */
				_statusMsg.serviceId(_eventImpl._item.directory().generatedServiceId());
		}
		else
			_statusMsg.service(null);

		notifyOnAllMsg(_statusMsg);
		notifyOnStatusMsg();

		if (tunnelStreamStatusEvent.state() != null)
		{
			if (_statusMsg.state().streamState() != StreamState.OPEN)
			{
				_eventImpl._item.remove();
			}
		}

		return ReactorCallbackReturnCodes.SUCCESS;

	};

	@Override
	public int queueMsgCallback(TunnelStreamQueueMsgEvent tunnelStreamQueueMsgEvent)
	{
		_baseImpl.eventReceived();

		if (tunnelStreamQueueMsgEvent.tunnelStream() == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append(
						"Received a tunnel stream queueMsg event without the tunnel stream object in ItemCallbackClient.queueEventCallback")
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(tunnelStreamQueueMsgEvent.reactorChannel().reactor().hashCode()));

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			return ReactorCallbackReturnCodes.SUCCESS;
		}

		ChannelInfo channelInfo = (ChannelInfo) tunnelStreamQueueMsgEvent.reactorChannel().userSpecObj();
		if (tunnelStreamQueueMsgEvent.tunnelStream().userSpecObject() == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				com.refinitiv.eta.transport.Error error = tunnelStreamQueueMsgEvent.errorInfo().error();

				StringBuilder temp = _baseImpl.strBuilder();
				temp.append(
						"Received a tunnel stream queueMsg event without the userSpecObject  in ItemCallbackClient.queueMsgCallback")
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(channelInfo.rsslReactor().hashCode())).append(OmmLoggerClient.CR)
						.append("RsslChannel ")
						.append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0))
						.append(OmmLoggerClient.CR).append("Error Id ").append(error.errorId())
						.append(OmmLoggerClient.CR).append("Internal sysError ").append(error.sysError())
						.append(OmmLoggerClient.CR).append("Error Location ")
						.append(tunnelStreamQueueMsgEvent.errorInfo().location()).append(OmmLoggerClient.CR)
						.append("Error Text ").append(error.text());

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			return ReactorCallbackReturnCodes.SUCCESS;
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	@Override
	@SuppressWarnings("unchecked")
	public int defaultMsgCallback(TunnelStreamMsgEvent tunnelStreamMsgEvent)
	{
		_baseImpl.eventReceived();

		TunnelStream tunnelStream = tunnelStreamMsgEvent.tunnelStream();
		if (tunnelStream == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append(
						"Received a null tunnel stream defaultMsg event without the tunnel stream object in ItemCallbackClient.defaultMsgCallback")
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(tunnelStreamMsgEvent.reactorChannel().reactor().hashCode()));

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			return ReactorCallbackReturnCodes.SUCCESS;
		}

		ChannelInfo channelInfo = (ChannelInfo) tunnelStreamMsgEvent.reactorChannel().userSpecObj();
		if (tunnelStream.userSpecObject() == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				com.refinitiv.eta.transport.Error error = tunnelStreamMsgEvent.errorInfo().error();
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append(
						"Received a tunnel stream message event without the userSpecObject in ItemCallbackClient.defaultMsgCallback")
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(channelInfo.rsslReactor().hashCode())).append(OmmLoggerClient.CR)
						.append("RsslChannel ")
						.append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0))
						.append(OmmLoggerClient.CR).append("Error Id ").append(error.errorId())
						.append(OmmLoggerClient.CR).append("Internal sysError ").append(error.sysError())
						.append(OmmLoggerClient.CR).append("Error Location ")
						.append(tunnelStreamMsgEvent.errorInfo().location()).append(OmmLoggerClient.CR)
						.append("Error Text ").append(error.text());

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			return ReactorCallbackReturnCodes.SUCCESS;
		}

		if (tunnelStreamMsgEvent.containerType() != DataTypes.MSG)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Received a tunnel stream message event containing an unsupported data type of ")
						.append(DataTypes.toString(tunnelStreamMsgEvent.containerType()))
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
						.append("Tunnel Stream Handle ")
						.append(Integer.toHexString(tunnelStream.userSpecObject().hashCode()))
						.append(OmmLoggerClient.CR).append("Tunnel Stream name ").append(tunnelStream.name())
						.append(OmmLoggerClient.CR).append("Tunnel Stream serviceId ").append(tunnelStream.serviceId())
						.append(OmmLoggerClient.CR).append("Tunnel Stream streamId ").append(tunnelStream.streamId());

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			return ReactorCallbackReturnCodes.SUCCESS;
		}

		if (tunnelStreamMsgEvent.msg() == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append(
						"Received a tunnel stream message event containing no sub stream message in ItemCallbackClient.defaultMsgCallback")
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
						.append("Tunnel Stream Handle ")
						.append(Integer.toHexString(tunnelStream.userSpecObject().hashCode()))
						.append(OmmLoggerClient.CR).append("Tunnel Stream name ").append(tunnelStream.name())
						.append(OmmLoggerClient.CR).append("Tunnel Stream serviceId ").append(tunnelStream.serviceId())
						.append(OmmLoggerClient.CR).append("Tunnel Stream streamId ").append(tunnelStream.streamId());

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			return ReactorCallbackReturnCodes.SUCCESS;
		}

		_eventImpl._item = ((TunnelItem<T>) (tunnelStream.userSpecObject()))
				.getSubItem(tunnelStreamMsgEvent.msg().streamId());

		if (_eventImpl._item == null)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append(
						"Received a tunnel stream message event containing sub stream message with unknown streamId ")
						.append(tunnelStreamMsgEvent.msg().streamId()).append(".  Message is dropped.")
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
						.append("Tunnel Stream Handle ")
						.append(Integer.toHexString(tunnelStream.userSpecObject().hashCode()))
						.append(OmmLoggerClient.CR).append("Tunnel Stream name ").append(tunnelStream.name())
						.append(OmmLoggerClient.CR).append("Tunnel Stream serviceId ").append(tunnelStream.serviceId())
						.append(OmmLoggerClient.CR).append("Tunnel Stream streamId ").append(tunnelStream.streamId());

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}

			return ReactorCallbackReturnCodes.SUCCESS;
		}

		switch (tunnelStreamMsgEvent.msg().msgClass())
		{
		case MsgClasses.GENERIC:
			return processTunnelStreamGenericMsg(tunnelStreamMsgEvent.msg(), channelInfo);
		case MsgClasses.ACK:
			return processTunnelStreamAckMsg(tunnelStreamMsgEvent.msg(), channelInfo);
		case MsgClasses.REFRESH:
			return processTunnelStreamRefreshMsg(tunnelStreamMsgEvent.msg(), channelInfo);
		case MsgClasses.UPDATE:
			return processTunnelStreamUpdateMsg(tunnelStreamMsgEvent.msg(), channelInfo);
		case MsgClasses.STATUS:
			return processTunnelStreamStatusMsg(tunnelStreamMsgEvent.msg(), channelInfo);
		default:
			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Received a tunnel stream message event containing an unsupported message type of ")
						.append(DataType.asString(Utilities.toEmaMsgClass[tunnelStreamMsgEvent.msg().msgClass()]))
						.append(OmmLoggerClient.CR).append("Instance Name ").append(_baseImpl.instanceName())
						.append(OmmLoggerClient.CR).append("RsslReactor ")
						.append(Integer.toHexString(channelInfo.rsslReactor().hashCode()));

				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			}
		}

		return ReactorCallbackReturnCodes.SUCCESS;

	}

	int processTunnelStreamAckMsg(Msg rsslMsg, ChannelInfo channelInfo)
	{
		if (_ackMsg == null)
			_ackMsg = new AckMsgImpl(_baseImpl.objManager());

		_ackMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);

		notifyOnAllMsg(_ackMsg);
		notifyOnAckMsg();

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processTunnelStreamGenericMsg(Msg rsslMsg, ChannelInfo channelInfo)
	{
		if (_genericMsg == null)
			_genericMsg = new GenericMsgImpl(_baseImpl.objManager());

		_genericMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);

		notifyOnAllMsg(_genericMsg);
		notifyOnGenericMsg();

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processTunnelStreamStatusMsg(Msg rsslMsg, ChannelInfo channelInfo)
	{
		if (_statusMsg == null)
			_statusMsg = new StatusMsgImpl(_baseImpl.objManager());

		_statusMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);

		notifyOnAllMsg(_statusMsg);
		notifyOnStatusMsg();

		if (((com.refinitiv.eta.codec.StatusMsg) rsslMsg).checkHasState()
				&& ((com.refinitiv.eta.codec.StatusMsg) rsslMsg).state().streamState() != StreamStates.OPEN)
			_eventImpl._item.remove();

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processTunnelStreamRefreshMsg(Msg rsslMsg, ChannelInfo channelInfo)
	{
		_refreshMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);

		notifyOnAllMsg(_refreshMsg);
		notifyOnRefreshMsg();

		int rsslStreamState = ((com.refinitiv.eta.codec.RefreshMsg) rsslMsg).state().streamState();
		if (rsslStreamState == StreamStates.NON_STREAMING)
		{
			if (((com.refinitiv.eta.codec.RefreshMsg) rsslMsg).checkRefreshComplete())
				_eventImpl._item.remove();
		} else if (rsslStreamState != StreamStates.OPEN)
		{
			_eventImpl._item.remove();
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processTunnelStreamUpdateMsg(Msg rsslMsg, ChannelInfo channelInfo)
	{
		_updateMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);

		notifyOnAllMsg(_updateMsg);
		notifyOnUpdateMsg();

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	@SuppressWarnings("unchecked")
	public int defaultMsgCallback(ReactorMsgEvent event)
	{
		_baseImpl.eventReceived();
		
		Msg msg = event.msg();
		ChannelInfo channelInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		if (channelInfo.getParentChannel() != null)
			channelInfo = channelInfo.getParentChannel();
        if (msg == null)
        {
        	com.refinitiv.eta.transport.Error error = event.errorInfo().error();
        	
        	if (_baseImpl.loggerClient().isErrorEnabled())
        	{
	        	StringBuilder temp = _baseImpl.strBuilder();
	        	temp.append("Received an item event without RsslMsg message")
	        		.append(OmmLoggerClient.CR)
	    			.append("Instance Name ").append(_baseImpl.instanceName())
	    			.append(OmmLoggerClient.CR)
	    			.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
	    			.append(OmmLoggerClient.CR)
	    			.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(event.errorInfo().location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
        	}
        	
    		return ReactorCallbackReturnCodes.SUCCESS;
        }
        
        _eventImpl._item = (Item<T>)(event.streamInfo() != null ? event.streamInfo().userSpecObject() : null);
		_eventImpl._channel = event.reactorChannel();

        if (_eventImpl._item == null && msg.streamId() != 1)
        {
        	if ( _baseImpl.implType() != OmmCommonImpl.ImplementationType.CONSUMER )
        	{
        		_eventImpl._item = _streamIdMap.get(_intObjHolder.value(msg.streamId()));
        		
        		if ( _eventImpl._item == null )
        		{
        			if (_baseImpl.loggerClient().isErrorEnabled())
            		{
            			StringBuilder temp = _baseImpl.strBuilder();
            			temp.append("Received an item event without a matching stream Id  ")
            				.append(msg.streamId())
    	        			.append(OmmLoggerClient.CR)
    	        			.append("Instance Name ").append(_baseImpl.instanceName())
    	        			.append(OmmLoggerClient.CR)
    	        			.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
    	        			.append(OmmLoggerClient.CR);
            			if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
            				temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
            				.append(OmmLoggerClient.CR)
            				.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
            			else
            				temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
    	        	
            			_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
            		}
        			
        			return ReactorCallbackReturnCodes.SUCCESS;
        		}
        		
        		ProviderItem	providerItem = ((ProviderItem)_eventImpl._item);
        		
        		providerItem.cancelReqTimerEvent();
        		
        		int streamId = msg.streamId();

        		msg = ItemWatchList.processRsslMsg(msg, providerItem);
        		
        		if ( msg == null )
        		{
        			if (_baseImpl.loggerClient().isErrorEnabled())
            		{
            			StringBuilder temp = _baseImpl.strBuilder();
            			temp.append("Received response is mismatch with the initlal request for stream Id  ")
            				.append(streamId)
                			.append(OmmLoggerClient.CR)
                			.append("Instance Name ").append(_baseImpl.instanceName())
                			.append(OmmLoggerClient.CR)
                			.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
                			.append(OmmLoggerClient.CR);
            			if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
            				temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
            				.append(OmmLoggerClient.CR)
            				.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
            			else
            				temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
                	
            			_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
            		}
        			
        			return ReactorCallbackReturnCodes.SUCCESS;
        		}
        	}
        	else
        	{
        		if (_baseImpl.loggerClient().isErrorEnabled())
        		{
        			StringBuilder temp = _baseImpl.strBuilder();
        			temp.append("Received an item event without user specified pointer or stream info")
	        			.append(OmmLoggerClient.CR)
	        			.append("Instance Name ").append(_baseImpl.instanceName())
	        			.append(OmmLoggerClient.CR)
	        			.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
	        			.append(OmmLoggerClient.CR);
        			if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
        				temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
        				.append(OmmLoggerClient.CR)
        				.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
        			else
        				temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
	        	
        			_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
        		}

        		return ReactorCallbackReturnCodes.SUCCESS;
        	}
        }

    	switch (msg.msgClass())
    	{
	    	case MsgClasses.ACK :
	    		if( _baseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER )
	    		{
	    			if (msg.streamId() == 1)
	    				return ((OmmBaseImpl<T>)_baseImpl).loginCallbackClient().processAckMsg(msg, channelInfo);
	    			else
	    				return processAckMsg(msg, channelInfo);
	    		}
	    		else
	    		{
	    			StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("Received an item event with the ACK message class for provider")
		        		.append(OmmLoggerClient.CR)
		        		.append("Instance Name ").append(_baseImpl.instanceName())
		        		.append(OmmLoggerClient.CR)
		        		.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
		        		.append(OmmLoggerClient.CR);
		        	if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
	        			temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
	        			.append(OmmLoggerClient.CR)
	        			.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
		        	else
		        		temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
		        	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	    			
	    			return ReactorCallbackReturnCodes.SUCCESS;
	    		}
	    	case MsgClasses.GENERIC :
	    		if (msg.streamId() == 1)
	    			return ((OmmBaseImpl<T>)_baseImpl).loginCallbackClient().processGenericMsg(msg, channelInfo);
	    		else
	    			return processGenericMsg(msg, channelInfo);
	    	case MsgClasses.REFRESH :
	    		return processRefreshMsg(msg, channelInfo.rsslReactorChannel(), channelInfo.rsslDictionary());
	    	case MsgClasses.STATUS :
	    		return processStatusMsg(msg, channelInfo.rsslReactorChannel(), channelInfo.rsslDictionary());
	    	case MsgClasses.UPDATE :
	    		return processUpdateMsg(msg, channelInfo.rsslReactorChannel(), channelInfo.rsslDictionary());
	    	default :
	    		if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
		        	StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("Received an item event with message containing unhandled message class")
		        		.append(OmmLoggerClient.CR)
		        		.append("Instance Name ").append(_baseImpl.instanceName())
		        		.append(OmmLoggerClient.CR)
		        		.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
		        		.append(OmmLoggerClient.CR);
			        	if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
		        			temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
		        			.append(OmmLoggerClient.CR)
		        			.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
			        	else
			        		temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
		        	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	}
	    		break;
    	}
        
		return ReactorCallbackReturnCodes.SUCCESS;
	}

	Item<T> getProviderDrivenItem(MsgImpl rsslMsg, ReactorChannel reactorChannel, Object userSpecObject, boolean createIfNotFound)
	{
		Item<T> item = null;
		HashMap<Integer, Item<T>> map = null;

		if (_providerDrivenItemsByReactorChannel.containsKey(reactorChannel))
		{
			map = _providerDrivenItemsByReactorChannel.get(reactorChannel);
		}
		else
		{
			map = new HashMap<>();
			_providerDrivenItemsByReactorChannel.put(reactorChannel, map);
		}

		if (map.containsKey(rsslMsg.streamId()))
		{
			item = map.get(rsslMsg.streamId());
		}
		else // this streamId has not been yet processed for the given reactorChannel, create new Item
		{
			item = createProviderDrivenItem(rsslMsg, userSpecObject, map, reactorChannel, createIfNotFound);
		}

		return item;
	}

	Item<T> createProviderDrivenItem(MsgImpl rsslMsg, Object userSpecObject, HashMap<Integer, Item<T>> map, ReactorChannel channel, boolean createIfNotFound)
	{
		SingleItemWithSource<T> item = null;

		if (userSpecObject != null)
		{
			if (userSpecObject instanceof SourceItem) // streamId of an item requested by the API from the Symbol List Refresh, streamId should be negative
			{
				@SuppressWarnings("unchecked")
				SourceItem<T> sourceItem = (SourceItem<T>)userSpecObject;
				int streamId = rsslMsg.streamId() < 0 ? getNextProviderStreamId() : rsslMsg.streamId();
				item = sourceItem.getItem(rsslMsg, streamId,true);
				item._reactorChannel = channel;

				map.put(rsslMsg.streamId(), item);
			}
		}

		return item;
	}

	public int processIProviderMsgCallback(ReactorMsgEvent event, DataDictionary dataDictionary)
	{
		Msg msg = event.msg();
		ClientSession clientSession = (ClientSession)event.reactorChannel().userSpecObj();
        
		_eventImpl._item = _streamIdMap.get(_intObjHolder.value(msg.streamId()));
		_eventImpl._clientHandle = clientSession.clientHandle();
		_eventImpl._ommProvider = ((OmmServerBaseImpl)_baseImpl).provider();
		
		if ( _eventImpl._item == null )
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
    		{
    			StringBuilder temp = _baseImpl.strBuilder();
    			temp.append("Received an item event without a matching stream Id  ")
    				.append(msg.streamId())
        			.append(OmmLoggerClient.CR)
        			.append("Instance Name ").append(_baseImpl.instanceName())
        			.append(OmmLoggerClient.CR)
        			.append("RsslReactor ").append(Integer.toHexString(clientSession.channel().reactor().hashCode()))
        			.append(OmmLoggerClient.CR);
    			if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
    				temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
    				.append(OmmLoggerClient.CR)
    				.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
    			else
    				temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
        	
    			_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
    		}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
		
		ProviderItem	providerItem = ((ProviderItem)_eventImpl._item);
		
		providerItem.cancelReqTimerEvent();
		
		int streamId = msg.streamId();
		
		msg = ItemWatchList.processRsslMsg(msg, providerItem);
		
		if ( msg == null )
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
    		{
    			StringBuilder temp = _baseImpl.strBuilder();
    			temp.append("Received response is mismatch with the initial request for stream Id  ")
    				.append(streamId)
        			.append(OmmLoggerClient.CR)
        			.append("Instance Name ").append(_baseImpl.instanceName())
        			.append(OmmLoggerClient.CR)
        			.append("RsslReactor ").append(Integer.toHexString(clientSession.channel().reactor().hashCode()))
        			.append(OmmLoggerClient.CR);
    			if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
    				temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
    				.append(OmmLoggerClient.CR)
    				.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
    			else
    				temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
        	
    			_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
    		}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}

    	switch (msg.msgClass())
    	{
	    	case MsgClasses.REFRESH :	
	    		return processRefreshMsg(msg, clientSession.channel(), dataDictionary);
	    	case MsgClasses.STATUS :
	    		return processStatusMsg(msg, clientSession.channel(), dataDictionary);
	    	default :
	    		if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
		        	StringBuilder temp = _baseImpl.strBuilder();
		        	temp.append("Received an item event with message containing unhandled message class")
		        		.append(OmmLoggerClient.CR)
		        		.append("Instance Name ").append(_baseImpl.instanceName())
		        		.append(OmmLoggerClient.CR)
		        		.append("RsslReactor ").append(Integer.toHexString(event.reactorChannel().hashCode()))
		        		.append(OmmLoggerClient.CR);
			        	if (event.reactorChannel() != null && event.reactorChannel().selectableChannel() != null)
		        			temp.append("RsslReactorChannel ").append(Integer.toHexString(event.reactorChannel().hashCode()))
		        			.append(OmmLoggerClient.CR)
		        			.append("RsslSelectableChannel ").append(Integer.toHexString(event.reactorChannel().selectableChannel().hashCode()));
			        	else
			        		temp.append("RsslReactorChannel is null").append(OmmLoggerClient.CR);
		        	
			        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	}
	    		break;
    	}
        
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	int processRefreshMsg(Msg rsslMsg, ReactorChannel reactorChannel, DataDictionary dataDictionary)
	{
		_refreshMsg.decode(rsslMsg, reactorChannel.majorVersion(), reactorChannel.minorVersion(), dataDictionary);
	
		boolean fromBatchRequest = false;
		boolean fromSymbolListRequest = false;
		if (_eventImpl._item.type() == Item.ItemType.BATCH_ITEM)
		{
			fromBatchRequest = true;
			_eventImpl._item = ((BatchItem<T>) _eventImpl._item).singleItem(rsslMsg.streamId());
			if (_eventImpl._item == null)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received an item event with invalid message stream").append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
							.append("RsslReactor ").append(Integer.toHexString(reactorChannel.hashCode()))
							.append(OmmLoggerClient.CR);

					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}

				return ReactorCallbackReturnCodes.FAILURE;
			}
		}
		else if (_eventImpl._item.type() == Item.ItemType.SYMBOL_LIST_SOURCE_ITEM)
		{
			fromSymbolListRequest = true;
			_eventImpl._item = rsslMsg.streamId() > 0
					? ((SourceItem<T>)_eventImpl._item).getItem(_refreshMsg, true)
					: getProviderDrivenItem(_refreshMsg, reactorChannel, _eventImpl._item, true);
			_refreshMsg.streamId(_eventImpl._item._streamId);
		}

		if (_eventImpl._item.directory() != null)
		{
			_refreshMsg.service(_eventImpl._item.directory().serviceName());
			
			if (isSingleItem())
			{
				SingleItem<T> singleItem = ((SingleItem<T>)_eventImpl._item);
				
				singleItem._itemClosedDirHash = null;
				
				if(singleItem._serviceList != null)
				{
					_refreshMsg.service(singleItem._serviceList.name());
					_refreshMsg.serviceId(singleItem._serviceList.serviceId());
				}
				else if(singleItem.directory().hasGeneratedServiceId()) /* Gets the mapping alternate service Id if any */
					_refreshMsg.serviceId(singleItem.directory().generatedServiceId());
				
				if(singleItem.directory().channelInfo().getReactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					if(singleItem.state() == SingleItem.ItemStates.RECOVERING)
					{
						@SuppressWarnings("unchecked")
						SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>)singleItem.directory().channelInfo().sessionChannelInfo();
						
						if(sessionChannelInfo != null)
						{
							singleItem.state(SingleItem.ItemStates.NORMAL);
							
							int wsbMode = sessionChannelInfo.getWarmStandbyMode(sessionChannelInfo.reactorChannel());

							if(wsbMode == ReactorWarmStandbyMode.LOGIN_BASED)
							{
								sessionChannelInfo.removeWSBStaleItem(singleItem);
							}
							else if (wsbMode == ReactorWarmStandbyMode.SERVICE_BASED)
							{
								sessionChannelInfo.removeWSBServiceBasedStaleItem(singleItem.directory().serviceName(), singleItem);
							}
						}
					}
				}
			}

			/* Adds the item to the item map of SessionDirectory when the item is requested by batch request. */
			if(fromBatchRequest || fromSymbolListRequest)
			{
				if( _eventImpl._item.directory().sessionDirectory() != null && isSingleItem())
				{
					if(_refreshMsg.hasName())
					{
						_eventImpl._item.directory().sessionDirectory().putDirectoryByItemName(_refreshMsg.name(),((SingleItem<T>)_eventImpl._item));
					}
				}
			}
		}
		else if (isSingleItem())
		{
			_refreshMsg.service(((SingleItem<T>)_eventImpl._item)._serviceName);
		}
		else
			_refreshMsg.service(null);
		
		notifyOnAllMsg(_refreshMsg);
		notifyOnRefreshMsg();
		
		int rsslStreamState = ((com.refinitiv.eta.codec.RefreshMsg)rsslMsg).state().streamState();
		if (rsslStreamState == StreamStates.NON_STREAMING)
		{
			if (((com.refinitiv.eta.codec.RefreshMsg)rsslMsg).checkRefreshComplete())
				_eventImpl._item.remove();
		}
		else if (rsslStreamState != StreamStates.OPEN)
		{
			_eventImpl._item.remove();
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processUpdateMsg(Msg rsslMsg,  ReactorChannel reactorChannel, DataDictionary dataDictionary)
	{
		_updateMsg.decode(rsslMsg, reactorChannel.majorVersion(), reactorChannel.minorVersion(), dataDictionary);

		if (_eventImpl._item.type() == Item.ItemType.BATCH_ITEM)
		{
			_eventImpl._item = ((BatchItem<T>) _eventImpl._item).singleItem(rsslMsg.streamId());
			if (_eventImpl._item == null)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received an item event with invalid message stream").append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
							.append("RsslReactor ").append(Integer.toHexString(reactorChannel.hashCode()))
							.append(OmmLoggerClient.CR);

					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}

				return ReactorCallbackReturnCodes.FAILURE;
			}
		}
		else if (_eventImpl._item.type() == Item.ItemType.SYMBOL_LIST_SOURCE_ITEM)
		{
			_eventImpl._item = ((SourceItem<T>)_eventImpl._item).getItem(_updateMsg.streamId());
			if (_eventImpl._item == null)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received an item event (update) with invalid message stream").append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
							.append("RsslReactor ").append(Integer.toHexString(reactorChannel.hashCode()))
							.append(OmmLoggerClient.CR);

					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}

				return ReactorCallbackReturnCodes.FAILURE;
			}
			else
			{
				_updateMsg.streamId(_eventImpl._item._streamId);
			}
		}

		if (_eventImpl._item.directory() != null)
		{
			_updateMsg.service(_eventImpl._item.directory().serviceName());
			
			if (isSingleItem())
			{
				SingleItem<T> singleItem = ((SingleItem<T>)_eventImpl._item);
				
				if(singleItem._serviceList != null)
				{
					_updateMsg.service(singleItem._serviceList.name());
					_updateMsg.serviceId(singleItem._serviceList.serviceId());
					
					singleItem._itemClosedDirHash = null;
				}
				else if(_eventImpl._item.directory().hasGeneratedServiceId()) /* Gets the mapping alternate service Id if any */
				{
					_updateMsg.serviceId(_eventImpl._item.directory().generatedServiceId());
					
					singleItem._itemClosedDirHash = null;
				}
				
				if (singleItem.directory().channelInfo().getReactorChannelType() == ReactorChannelType.WARM_STANDBY)
				{
					if(singleItem.state() == SingleItem.ItemStates.RECOVERING)
					{
						@SuppressWarnings("unchecked")
						SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>)singleItem.directory().channelInfo().sessionChannelInfo();
						
						if(sessionChannelInfo != null)
						{
							singleItem.state(SingleItem.ItemStates.NORMAL);
							
							int wsbMode = sessionChannelInfo.getWarmStandbyMode(sessionChannelInfo.reactorChannel());
							
							if(wsbMode == ReactorWarmStandbyMode.LOGIN_BASED)
							{
								sessionChannelInfo.removeWSBStaleItem(singleItem);
							}
							else if (wsbMode == ReactorWarmStandbyMode.SERVICE_BASED)
							{
								sessionChannelInfo.removeWSBServiceBasedStaleItem(singleItem.directory().serviceName(), singleItem);
							}
						}
					}
				}
			}
		}
		else if (isSingleItem())
		{
			_updateMsg.service(((SingleItem<T>)_eventImpl._item)._serviceName);
		}
		else
			_updateMsg.service(null);

		notifyOnAllMsg(_updateMsg);
		notifyOnUpdateMsg();

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processStatusMsg(Msg rsslMsg,  ReactorChannel reactorChannel, DataDictionary dataDictionary)
	{
		if (_statusMsg == null)
			_statusMsg = new StatusMsgImpl(_baseImpl.objManager());
		
		_statusMsg.decode(rsslMsg, reactorChannel.majorVersion(), reactorChannel.minorVersion(), dataDictionary);
		if (_eventImpl._item.type() == Item.ItemType.BATCH_ITEM)
		{
			if((_eventImpl._item._streamId == _statusMsg.streamId()) &&
					_statusMsg.hasState() && _statusMsg.state().streamState() == StreamState.CLOSED)
				((BatchItem<T>) _eventImpl._item)._statusFlag = true;

			_eventImpl._item = ((BatchItem<T>)_eventImpl._item).singleItem(rsslMsg.streamId());

			if  (_eventImpl._item == null)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received an item event with invalid message stream").append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
							.append("RsslReactor ").append(Integer.toHexString(reactorChannel.hashCode()))
							.append(OmmLoggerClient.CR);

					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}
				return ReactorCallbackReturnCodes.FAILURE;
			}
		}
		else if (_eventImpl._item.type() == Item.ItemType.SYMBOL_LIST_SOURCE_ITEM)
		{
			_eventImpl._item = rsslMsg.streamId() > 0
					? ((SourceItem<T>)_eventImpl._item).getItem(_statusMsg, true)
					: getProviderDrivenItem(_statusMsg, reactorChannel, _eventImpl._item, true);
			_statusMsg.streamId(_eventImpl._item._streamId);
		}

		if (_eventImpl._item.directory() != null)
		{
			_statusMsg.service(_eventImpl._item.directory().serviceName());
		}
		else if (isSingleItem())
		{
			_statusMsg.service(((SingleItem<T>)_eventImpl._item)._serviceName);
		}
		else
			_statusMsg.service(null);
		
		if (_consumerSession != null)
		{
			return _consumerSession.watchlist().handleItemStatus(((com.refinitiv.eta.codec.StatusMsg)rsslMsg), 
					_statusMsg, _eventImpl._item);
		}
		
		notifyOnAllMsg(_statusMsg);
		notifyOnStatusMsg();

		if (((com.refinitiv.eta.codec.StatusMsg)rsslMsg).checkHasState() &&  
				((com.refinitiv.eta.codec.StatusMsg)rsslMsg).state().streamState() != StreamStates.OPEN)
		{
			_eventImpl._item.remove();
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processGenericMsg(Msg rsslMsg,  ChannelInfo channelInfo)
	{
		if (_genericMsg == null)
			_genericMsg = new GenericMsgImpl(_baseImpl.objManager());

		_genericMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);
		
		if (_eventImpl._item.type() == Item.ItemType.BATCH_ITEM)
		{
			_eventImpl._item = ((BatchItem<T>) _eventImpl._item).singleItem(rsslMsg.streamId());
			if (_eventImpl._item == null)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received an item event with invalid message stream (generic)").append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
							.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
							.append(OmmLoggerClient.CR);

					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}

				return ReactorCallbackReturnCodes.FAILURE;
			}
		}
		else if (_eventImpl._item.type() == Item.ItemType.SYMBOL_LIST_SOURCE_ITEM)
		{
			_eventImpl._item = ((SourceItem<T>)_eventImpl._item).getItem(_genericMsg.streamId());
			if (_eventImpl._item == null)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received an item event with invalid message stream").append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
							.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
							.append(OmmLoggerClient.CR);

					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}

				return ReactorCallbackReturnCodes.FAILURE;
			}
			else
			{
				_genericMsg.streamId(_eventImpl._item._streamId);
			}
		}
		
		if(_eventImpl._item.directory() != null)
		{
			if(rsslMsg.msgKey().checkHasServiceId())
			{
				int serviceId = rsslMsg.msgKey().serviceId();
				if(serviceId == _eventImpl._item.directory().service().serviceId())
				{
					/* Gets the mapping alternate service Id if any */
					if(_eventImpl._item.directory().hasGeneratedServiceId())
						_genericMsg.serviceId(_eventImpl._item.directory().generatedServiceId());
				}
			}
		}

		notifyOnAllMsg(_genericMsg);
		notifyOnGenericMsg();

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	int processAckMsg(Msg rsslMsg,  ChannelInfo channelInfo)
	{
		if (_ackMsg == null)
			_ackMsg = new AckMsgImpl(_baseImpl.objManager());
		
		_ackMsg.decode(rsslMsg, channelInfo._majorVersion, channelInfo._minorVersion, channelInfo._rsslDictionary);

		if (_eventImpl._item.type() == Item.ItemType.BATCH_ITEM)
		{
			_eventImpl._item = ((BatchItem<T>) _eventImpl._item).singleItem(rsslMsg.streamId());
			if (_eventImpl._item == null)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received an item event with invalid message stream").append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
							.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
							.append(OmmLoggerClient.CR);

					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}

				return ReactorCallbackReturnCodes.FAILURE;
			}
		}
		else if (_eventImpl._item.type() == Item.ItemType.SYMBOL_LIST_SOURCE_ITEM)
		{
			_eventImpl._item = ((SourceItem<T>)_eventImpl._item).getItem(rsslMsg.streamId());
			if (_eventImpl._item == null)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					StringBuilder temp = _baseImpl.strBuilder();
					temp.append("Received an item event with invalid message stream (ack)").append(OmmLoggerClient.CR)
							.append("Instance Name ").append(_baseImpl.instanceName()).append(OmmLoggerClient.CR)
							.append("RsslReactor ").append(Integer.toHexString(channelInfo.rsslReactor().hashCode()))
							.append(OmmLoggerClient.CR);

					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}

				return ReactorCallbackReturnCodes.FAILURE;
			}
			else
			{
				_ackMsg.streamId(_eventImpl._item._streamId);
			}
		}

		if (_eventImpl._item.directory() != null)
		{
			_ackMsg.service(_eventImpl._item.directory().serviceName());
			
			if(rsslMsg.msgKey().checkHasServiceId())
			{
				int serviceId = rsslMsg.msgKey().serviceId();
				if(serviceId == _eventImpl._item.directory().service().serviceId())
				{
					/* Gets the mapping alternate service Id if any */
					if(_eventImpl._item.directory().hasGeneratedServiceId())
						_ackMsg.serviceId(_eventImpl._item.directory().generatedServiceId());
				}
			}
		}
		else
			_ackMsg.service(null);
		
		notifyOnAllMsg(_ackMsg);
		notifyOnAckMsg();

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	boolean isSingleItem()
	{
		return _eventImpl._item.type() == Item.ItemType.SINGLE_ITEM || _eventImpl._item.type() == Item.ItemType.SINGLE_ITEM_WITH_SOURCE;
	}

	@SuppressWarnings("unchecked")
	long registerClient(ReqMsg reqMsg, T client, Object closure , long parentHandle)
	{
		if (parentHandle == 0)
		{
			com.refinitiv.eta.codec.RequestMsg requestMsg = ((ReqMsgImpl)reqMsg).rsslMsg();

			switch (requestMsg.domainType())
			{
				case DomainTypes.LOGIN :
				{
					SingleItem<T> item = ((OmmBaseImpl<T>)_baseImpl).loginCallbackClient().loginItem(reqMsg, client, closure);
					/* Assign an available handle to the request */
					return addToItemMap(_baseImpl.nextLongId(), item);
				}
				case DomainTypes.DICTIONARY :
				{
					int nameType = requestMsg.msgKey().nameType();
					if ((nameType != InstrumentNameTypes.UNSPECIFIED) && (nameType != InstrumentNameTypes.RIC))
					{
						StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("Invalid ReqMsg's name type : ")
			        		.append(nameType)
			        		.append(". Instance name='").append(_baseImpl.instanceName()).append("'.");
		
			        	if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
			        		_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}

						_baseImpl.handleInvalidUsage( temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );

						return 0;
					}

					if (!requestMsg.msgKey().checkHasName())
					{
						StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("ReqMsg's name is not defined. ")
							.append("Instance name='").append(_baseImpl.instanceName()).append("'.");

			        	if (_baseImpl.loggerClient().isErrorEnabled())
			        	{
			        		_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));
			        	}

			        	_baseImpl.handleInvalidUsage( temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );

						return 0;
					}

					Item<T> item = null;
					
					if ( _baseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER )
					{
						if ((item = (DictionaryItem<T>)_baseImpl.objManager()._dictionaryItemPool.poll()) == null)
						{
							item = new DictionaryItem<T>((OmmBaseImpl<T>)_baseImpl, client, closure);
							_baseImpl.objManager()._dictionaryItemPool.updatePool(item);
						}
						else
							((DictionaryItem<T>)item).reset((OmmBaseImpl<T>)_baseImpl, client, closure, null);
					}
					else if ( _baseImpl.implType() == OmmCommonImpl.ImplementationType.NIPROVIDER )
					{
						if ((item = (NiProviderDictionaryItem<T>)_baseImpl.objManager()._niproviderDictionaryItemPool.poll()) == null)
						{
							item = new NiProviderDictionaryItem<T>((OmmBaseImpl<T>)_baseImpl, client, closure);
							_baseImpl.objManager()._niproviderDictionaryItemPool.updatePool(item);
						}
						else
							((NiProviderDictionaryItem<T>)item).reset((OmmBaseImpl<T>)_baseImpl, client, closure, null);
					}
					else
					{
						if ((item = (Item<T>)_baseImpl.objManager()._iproviderDictionaryItemPool.poll()) == null)
						{
							item = (Item<T>)(new IProviderDictionaryItem((OmmServerBaseImpl)_baseImpl, (OmmProviderClient)client, closure));
							_baseImpl.objManager()._iproviderDictionaryItemPool.updatePool(item);
						}
						else
							((IProviderDictionaryItem)item).reset((OmmServerBaseImpl)_baseImpl, (OmmProviderClient)client, closure, null);
					}
					
					if (!item.open(reqMsg))
					{
						removeFromMap(item, true);
						return 0;
					}
					else
						return item.itemId();
				}
				case DomainTypes.SOURCE :
				{
					OmmBaseImpl<T> ommBaseImpl = ((OmmBaseImpl<T>)_baseImpl);
					ConsumerSession<T> consumerSession = ommBaseImpl.consumerSession();
					
					if(consumerSession != null)
					{
						SingleItem<T> item = ((OmmBaseImpl<T>)_baseImpl).directoryCallbackClient().directoryItem(consumerSession, reqMsg, client, closure);
						/* Assign an available handle to the request */
						return addToItemMap(_baseImpl.nextLongId(), item);
					}
					
					ChannelInfo channel = ommBaseImpl.loginCallbackClient().activeChannelInfo();
					if (channel == null)
					{
						StringBuilder temp = _baseImpl.strBuilder();
	
						temp.append("Failed to send a directory request due to no active channel")
						.append(". Instance name='").append(_baseImpl.instanceName()).append("' in registerClient().");;
	
						if (_baseImpl.loggerClient().isErrorEnabled())
						{
							_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
									temp.toString(), Severity.ERROR));
						}
	
						_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL);						
						return 0;
					}
						
					DirectoryItem<T> item;
					if ((item = (DirectoryItem<T>)_baseImpl.objManager()._directoryItemPool.poll()) == null)
					{
						item = new DirectoryItem<T>(ommBaseImpl, client, closure);
						_baseImpl.objManager()._directoryItemPool.updatePool(item);
					}
					else
						item.reset(ommBaseImpl, client, closure, null);
					
					item.channelInfo(channel);
					
					if (!item.open(reqMsg))
					{
						removeFromMap(item, true);
						return 0;
					}
					else
						return item.itemId();
				}
				case DomainTypes.SYMBOL_LIST:
					SingleItemWithSource<T> itemWithSource = null;
					if ((itemWithSource = (SingleItemWithSource<T>)_baseImpl.objManager()._singleItemWithSourcePool.poll()) == null)
					{
						itemWithSource = new SingleItemWithSource<>((OmmBaseImpl<T>)_baseImpl, client, closure);
						_baseImpl.objManager()._singleItemWithSourcePool.updatePool(itemWithSource);
					}
					else
					{
						itemWithSource.reset((OmmBaseImpl<T>)_baseImpl, client, closure, null);
					}

					if (!itemWithSource.open(reqMsg))
					{
						removeFromMap(itemWithSource, true);
						return 0;
					}
					else
						return itemWithSource.itemId();
				default :
				{					
					if (requestMsg.checkHasBatch())
					{
						BatchItem<T> batchItem;
						if ((batchItem = (BatchItem<T>)_baseImpl.objManager()._batchItemPool.poll()) == null)
						{
							batchItem = new BatchItem<T>((OmmBaseImpl<T>)_baseImpl, client, closure);
							_baseImpl.objManager()._batchItemPool.updatePool(batchItem);
						}
						else
							batchItem.reset((OmmBaseImpl<T>)_baseImpl, client, closure, null);
						
						/* Start splitting the batch request into individual item request if _nextStreamIdWrapAround is true. */
						if (((OmmBaseImpl<T>)_baseImpl)._itemCallbackClient.nextStreamIdWrapAround(((ReqMsgImpl)reqMsg).batchItemList().size()))
						{
							List<String> itemList = ((ReqMsgImpl)reqMsg).batchItemList();
							SingleItem<T> item;
							int flags =  requestMsg.flags();
							flags &= ~RequestMsgFlags.HAS_BATCH;
							requestMsg.flags(flags);
							requestMsg.msgKey().applyHasName();
							for (String itemName : itemList)
							{
								if ((item = (SingleItem<T>)_baseImpl.objManager()._singleItemPool.poll()) == null)
								{
									item = new SingleItem<T>((OmmBaseImpl<T>)_baseImpl, client, closure, null);
									_baseImpl.objManager()._singleItemPool.updatePool(item);
								}
								else
									item.reset((OmmBaseImpl<T>)_baseImpl, client, closure, null);
								
								requestMsg.msgKey().name().data(itemName);

								if (!item.open(reqMsg))
								{
									removeFromMap(item, true);
									return 0;
								}
							}
							
							addToItemMap(LongIdGenerator.nextLongId(), batchItem);
							
							/* Send stream close status for the batch stream */
							int keyFlags = requestMsg.msgKey().flags();
							keyFlags &= ~MsgKeyFlags.HAS_NAME;
							requestMsg.msgKey().flags(keyFlags);
							batchItem.scheduleItemClosedStatus(((OmmBaseImpl<T>)_baseImpl).itemCallbackClient(),
									batchItem, requestMsg, "Stream closed for batch", reqMsg.serviceName());
							
							return batchItem.itemId();
						}
						else
						{
							batchItem.addBatchItems( ((ReqMsgImpl)reqMsg).batchItemList());
							List<SingleItem<T>> items = batchItem.singleItemList();
							int numOfItem = items.size();
							
							if ( !batchItem.open( reqMsg ) )
							{
								SingleItem<T> item;
								for ( int i = 1 ; i < numOfItem ; i++ )
								{
									item = items.get(i);
									removeFromMap(item, true);
								}
							
								removeFromMap(batchItem, true);
								
								return 0;
							}
							else
								return batchItem.itemId();
						}
					}
					else
					{
						SingleItem<T> item = null;
						if ((item = (SingleItem<T>)_baseImpl.objManager()._singleItemPool.poll()) == null)
						{
							item = new SingleItem<T>((OmmBaseImpl<T>)_baseImpl, client, closure, null);
							_baseImpl.objManager()._singleItemPool.updatePool(item);
						}
						else
							item.reset((OmmBaseImpl<T>)_baseImpl, client, closure, null);
					
						if (!item.open(reqMsg))
						{
							removeFromMap(item, true);
							return 0;
						}
						else
							return item.itemId();
					}
				}
			}
		}
		else 
		{
			Item<T> parent = _itemMap.get(_longObjHolder.value(parentHandle));
			if (parent == null)
			{
				StringBuilder temp = _baseImpl.strBuilder();

				temp.append("Attempt to get item for parent handle from itemMap failed in registerClient(). ");

				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);

				return 0;
			}
			if (parent.type() != Item.ItemType.TUNNEL_ITEM)
			{
				StringBuilder temp = _baseImpl.strBuilder();

				temp.append("Attempt to invalid type as parentHandle on registerClient(). ");

				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME,
							temp.toString(), Severity.ERROR));
				}

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);

				return 0;
			}

			SubItem<T> subItem;
			if ((subItem = (SubItem<T>)_baseImpl.objManager()._subItemPool.poll()) == null)
			{
				subItem = new SubItem<T>((OmmBaseImpl<T>)_baseImpl, client, closure, parent);
				_baseImpl.objManager()._subItemPool.updatePool(subItem);
			} else
				subItem.reset((OmmBaseImpl<T>)_baseImpl, client, closure, parent);

			if (!subItem.open(reqMsg))
			{
				if (subItem.streamId() > 0)
				{
					((TunnelItem<T>) (parent)).removeSubItem(subItem.streamId());
					((TunnelItem<T>) (parent)).returnSubItemStreamId(subItem.streamId());
				}
				
				removeFromMap(subItem, true);

				return 0;
			}

			return subItem.itemId();
		}
	}
	
	@SuppressWarnings("unchecked")
	long registerClient(TunnelStreamRequest tunnelStreamReq, T client, Object closure)
	{
		TunnelItem<T> item;
		if ((item = (TunnelItem<T>) _baseImpl.objManager()._tunnelItemPool.poll()) == null)
		{
			item = new TunnelItem<T>((OmmBaseImpl<T>)_baseImpl, client, closure, null);
			_baseImpl.objManager()._tunnelItemPool.updatePool(item);
		} 
		else
			item.reset((OmmBaseImpl<T>)_baseImpl, client, closure, null);

		if (!item.open(tunnelStreamReq))
		{
			removeFromMap(item, true);
			return 0;
		}
		return item.itemId();
	}
	
	void reissue(com.refinitiv.ema.access.ReqMsg reqMsg, long handle)
	{
		Item<T> item = _itemMap.get(_longObjHolder.value(handle));
		if (item == null || item._closedStatusClient != null)
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Attempt to use invalid Handle on reissue(). ").append("Instance name='")
					.append(_baseImpl.instanceName()).append("'.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidHandle(handle, temp.toString());

			return;
		}

		item.modify(reqMsg);
	}

	void unregister(long handle)
	{
		Item<T> item = _itemMap.get(_longObjHolder.value(handle));
		if (item != null)
			item.close();
	}
	
	void submit(com.refinitiv.ema.access.RefreshMsg refreshMsg, long handle)
	{
		Item<T> found = _itemMap.get(_longObjHolder.value(handle));
		if ( found == null )
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Attempt to use invalid Handle on submit(RefreshMsg). ").append("name='")
					.append(_baseImpl.instanceName()).append("'.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidHandle(handle, temp.toString());
			
			return;
		}

		found.submit( refreshMsg );
	}
	
	void submit(com.refinitiv.ema.access.UpdateMsg updateMsg, long handle)
	{
		Item<T> found = _itemMap.get(_longObjHolder.value(handle));
		if ( found == null )
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Attempt to use invalid Handle on submit(UpdateMsg). ").append("name='")
					.append(_baseImpl.instanceName()).append("'.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidHandle(handle, temp.toString());
			
			return;
		}

		found.submit( updateMsg );
	}
	
	void submit(com.refinitiv.ema.access.StatusMsg statusMsg, long handle)
	{
		Item<T> found = _itemMap.get(_longObjHolder.value(handle));
		if ( found == null )
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Attempt to use invalid Handle on submit(StatusMsg). ").append("name='")
					.append(_baseImpl.instanceName()).append("'.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidHandle(handle, temp.toString());
			
			return;
		}

		found.submit(statusMsg);
	}

	void submit(com.refinitiv.ema.access.PostMsg postMsg, long handle)
	{
		Item<T> found = _itemMap.get(_longObjHolder.value(handle));
		if ( found == null )
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Attempt to use invalid Handle on submit(PostMsg). ").append("Instance name='")
					.append(_baseImpl.instanceName()).append("'.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidHandle(handle, temp.toString());
			
			return;
		}

		found.submit( postMsg );
	}

	void submit(com.refinitiv.ema.access.GenericMsg genericMsg, long handle)
	{
		Item<T> found = _itemMap.get(_longObjHolder.value(handle));
		if ( found == null )
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Attempt to use invalid Handle on submit(GenericMsg). ").append("Instance name='")
					.append(_baseImpl.instanceName()).append("'.");

			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(
						_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.ERROR));

			_baseImpl.handleInvalidHandle(handle, temp.toString());
			
			return;
		}

		found.submit( genericMsg );
	}

	long addToMap(long itemId, Item<T> item)
	{
		LongObject itemIdObj = _baseImpl.objManager().createLongObject().value(itemId);
		IntObject streamIdObj = _baseImpl.objManager().createIntObject().value(item._streamId); // this has to be ema-facing streamId because it has to be unique on EMA level
		item.itemId(itemIdObj, streamIdObj);
		_itemMap.put(itemIdObj, item);
		_streamIdMap.put(streamIdObj, item);
		
		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Added Item ").append(itemId).append(" of StreamId ").append(streamIdObj.value()).append(" to item map" ).append( OmmLoggerClient.CR )
			.append( "Instance name " ).append( _baseImpl .instanceName() );
			
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		}
		
		return itemId;
	}
	
	long addToItemMap(long itemId, Item<T> item)
	{
		LongObject itemIdObj = _baseImpl.objManager().createLongObject().value(itemId);
		item.itemId(itemIdObj, null);
		_itemMap.put(itemIdObj, item);
		
		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			temp.append("Added Item ").append(itemId).append(" to item map" ).append( OmmLoggerClient.CR )
			.append( "Instance name " ).append( _baseImpl .instanceName() );
			
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		}
		
		return itemId;
	}
	
	Item<T> getItem(long handle)
	{
		return _itemMap.get(_longObjHolder.value(handle));
	}
	
	void removeFromMap(Item<T> item, boolean returnToPool)
	{
		if (_baseImpl.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = _baseImpl.strBuilder();
			if (item.streamIdObj() != null)
				temp.append("Removed Item ").append(item._itemId).append(" of StreamId ").append(item.streamIdObj().value()).append(" from item map" ).append( OmmLoggerClient.CR )
			.append( "Instance name " ).append( _baseImpl .instanceName() );
			else
				temp.append("Removed Item ").append(item._itemId).append(" from item map" ).append( OmmLoggerClient.CR )
			.append( "Instance name " ).append( _baseImpl .instanceName() );
			
			_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(ItemCallbackClient.CLIENT_NAME, temp.toString(), Severity.TRACE));
		}

		if (item.itemIdObj() != null)
		{
			_itemMap.remove(item.itemIdObj());
		}

		if (item.streamIdObj() != null)
		{
			_streamIdMap.remove(item.streamIdObj());
		}


		if (returnToPool)
		{
			item.backToPool();
		}
	}

	boolean isStreamIdInUse(int nextStreamId)
	{
		return (_streamIdMap.containsKey(_intObjHolder.value(nextStreamId)));
	}
	
	int nextStreamId(int numOfItem)
	{
		if (_nextStreamId > CONSUMER_MAX_STREAM_ID_MINUSONE - numOfItem)
		{
			if ( _baseImpl.implType() == OmmCommonImpl.ImplementationType.CONSUMER )
			{
				_nextStreamId = CONSUMER_STARTING_STREAM_ID;
			}
			else
			{
				_nextStreamId = PROVIDER_STARTING_STREAM_ID;
			}
			
			_nextStreamIdWrapAround = true;
			
			if (_streamIdAccessLock == null)
				_streamIdAccessLock = new java.util.concurrent.locks.ReentrantLock();
			
			if (_baseImpl.loggerClient().isTraceEnabled())
				_baseImpl.loggerClient().trace(_baseImpl.formatLogMessage(CLIENT_NAME,
						"Reach max number available for next stream id, will wrap around", Severity.TRACE));
		}
		
		if (!_nextStreamIdWrapAround)
		{
			if ( numOfItem > 0 )
			{
				int retVal = ++_nextStreamId;
				_nextStreamId += numOfItem;
				return retVal;
			}
	
			return ++_nextStreamId;
		}
		else
		{
			_streamIdAccessLock.lock();
			while (isStreamIdInUse(++_nextStreamId));
			_streamIdAccessLock.unlock();
			
			if ( _nextStreamId < 0 )
			{
				StringBuilder tempErr = _baseImpl.strBuilder();
				tempErr.append("Unable to obtain next available stream id for item request.");
				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, tempErr.toString(), Severity.ERROR));
				
				_baseImpl.handleInvalidUsage(tempErr.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);
			}
				
			return _nextStreamId;
		}
	}
	
	boolean nextStreamIdWrapAround(int numOfItem)
	{
		return (_nextStreamId > (CONSUMER_MAX_STREAM_ID_MINUSONE - numOfItem));
	}


	int getNextProviderStreamId()
	{
		if (_nextProviderStreamId == Integer.MAX_VALUE) _nextProviderStreamId = 0;

		int startStreamId = _nextProviderStreamId;
		int nextProviderStreamId = ++_nextProviderStreamId;

		while (isStreamIdInUse(-nextProviderStreamId) && _nextProviderStreamId < Integer.MAX_VALUE)
		{
			nextProviderStreamId = ++_nextProviderStreamId;
		}

		if (isStreamIdInUse(-nextProviderStreamId)) // start from the beginning to startStreamId to make the full circle
		{
			_nextProviderStreamId = 0;
			while (isStreamIdInUse(-nextProviderStreamId) && _nextProviderStreamId < startStreamId)
			{
				nextProviderStreamId = ++_nextProviderStreamId;
			}

			if (isStreamIdInUse(-nextProviderStreamId)) // free streamId not found
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Unable to obtain next available stream id to retrieve provider-driven item.");
				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR);
			}
		}

		return -nextProviderStreamId;
	}
}

class ItemCallbackClientConsumer extends ItemCallbackClient<OmmConsumerClient>
{
	private static final String CLIENT_NAME = "ItemCallbackClientConsumer";
	
	ItemCallbackClientConsumer(OmmBaseImpl<OmmConsumerClient> baseImpl) {
		super(baseImpl);
	}
	
	@Override
	void notifyOnAllMsg(com.refinitiv.ema.access.Msg msg)
	{
		if (_eventImpl._item.client() == null)	// Consumer client is already closed, ignore this message
		{
			StringBuilder tempErr = _baseImpl.strBuilder();
			tempErr.append("An incoming Msg to a closed OmmConsumerClient has been dropped.");
			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, tempErr.toString(), Severity.WARNING));
			return;
		}
		_eventImpl._item.client().onAllMsg(msg, _eventImpl);
	}
	
	@Override
    void notifyOnRefreshMsg()
	{
		if (_eventImpl._item.client() == null)	// Consumer client is already closed, ignore this message
		{
			StringBuilder tempErr = _baseImpl.strBuilder();
			tempErr.append("An incoming RefreshMsg to a closed OmmConsumerClient has been dropped.");
			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, tempErr.toString(), Severity.WARNING));
			return;
		}
		_eventImpl._item.client().onRefreshMsg(_refreshMsg, _eventImpl);
	}
	
	@Override
	void notifyOnUpdateMsg()
	{
		if (_eventImpl._item.client() == null)	// Consumer client is already closed, ignore this message
		{
			StringBuilder tempErr = _baseImpl.strBuilder();
			tempErr.append("An incoming UpdateMsg to a closed OmmConsumerClient has been dropped.");
			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, tempErr.toString(), Severity.WARNING));
			return;
		}
		_eventImpl._item.client().onUpdateMsg(_updateMsg, _eventImpl);
	}
	
	@Override
	void notifyOnStatusMsg() 
	{
		if (_eventImpl._item.client() == null)	// Consumer client is already closed, ignore this message
		{
			StringBuilder tempErr = _baseImpl.strBuilder();
			tempErr.append("An incoming StatusMsg to a closed OmmConsumerClient has been dropped.");
			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, tempErr.toString(), Severity.WARNING));
			return;
		}
		_eventImpl._item.client().onStatusMsg(_statusMsg, _eventImpl);
	}
	
	@Override
	void notifyOnGenericMsg()
	{
		if (_eventImpl._item.client() == null)	// Consumer client is already closed, ignore this message
		{
			StringBuilder tempErr = _baseImpl.strBuilder();
			tempErr.append("An incoming GenericMsg to a closed OmmConsumerClient has been dropped.");
			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, tempErr.toString(), Severity.WARNING));
			return;
		}
		_eventImpl._item.client().onGenericMsg(_genericMsg, _eventImpl);
	} 
	
	@Override
	void notifyOnAckMsg()
	{
		if (_eventImpl._item.client() == null)	// Consumer client is already closed, ignore this message
		{
			StringBuilder tempErr = _baseImpl.strBuilder();
			tempErr.append("An incoming AckMsg to a closed OmmConsumerClient has been dropped.");
			if (_baseImpl.loggerClient().isErrorEnabled())
				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, tempErr.toString(), Severity.WARNING));
			return;
		}
		_eventImpl._item.client().onAckMsg(_ackMsg, _eventImpl);
	}
}

class ItemCallbackClientProvider extends ItemCallbackClient<OmmProviderClient>
{
	ItemCallbackClientProvider(OmmBaseImpl<OmmProviderClient> baseImpl) {
		super(baseImpl);
		
		_eventImpl._ommProvider = (OmmProvider)baseImpl;
	}
	
	ItemCallbackClientProvider(OmmServerBaseImpl baseImpl) {
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
	
	@Override
	void notifyOnGenericMsg()
	{
		_eventImpl._item.client().onGenericMsg(_genericMsg, _eventImpl);
	} 
}

abstract class Item<T> extends VaNode
{
	// non private stream copy flags
    static final int DEFAULT_COPY_FLAGS = CopyMsgFlags.ALL_FLAGS & ~CopyMsgFlags.EXTENDED_HEADER & ~CopyMsgFlags.DATA_BODY;
	
	static final class ItemType
	{
		final static int SINGLE_ITEM = 0;
		final static int BATCH_ITEM  = 1;
		final static int LOGIN_ITEM = 2;
		final static int DICTIONARY_ITEM = 3;
		final static int DIRECTORY_ITEM = 4;
		final static int TUNNEL_ITEM = 5;
		final static int SUB_ITEM	 = 6;
		final static int NIPROVIDER_DICTIONARY_ITEM = 7;
		final static int IPROVIDER_SINGLE_ITEM = 8;
		final static int IPROVIDER_DICTIONARY_ITEM = 9;
		final static int SYMBOL_LIST_SOURCE_ITEM = 10;
		final static int SINGLE_ITEM_WITH_SOURCE = 11;
	}
	
	int						_domainType;
	int						_streamId;     // ema-facing streamId
	Object					_closure;
	Item<T>					_parent;
	T						_client;
	long 					_itemId;
	boolean					_assignedItemId;
	LongObject _itemIdObj;
	IntObject _streamIdObj;
	ClosedStatusClient<T>	_closedStatusClient;
	int 					_etaStreamId;  				// eta-facing streamId (for provider-driven items)
	boolean 				_hasEtaStreamId = false;	// is true only with SingleItemWithSource items that are requested on behalf of the user

	Item() {}

	Item(T client, Object closure, Item<T> parent)
	{
		_domainType = 0;
		_streamId = 0;
		_etaStreamId = 0;
		_hasEtaStreamId = false;
		_closure = closure;
		_parent = parent;
		_client = client;
	}

	T client()
	{
		return _client;
	}
	
	Object closure()
	{
		return _closure;
	}
	
	Item<T> parent()
	{
		return _parent;
	}
	
	void itemId(LongObject itemIdObj, IntObject streamIdObj)
	{
		_itemId = itemIdObj.value();
		_itemIdObj = itemIdObj;
		_streamIdObj = streamIdObj;
	}
	
	void backToPool()
	{
		if (_itemIdObj != null) _itemIdObj.returnToPool();
		if (_streamIdObj != null) _streamIdObj.returnToPool();
	
		_closure = null;
		_parent = null;
		_client = null;
		_closedStatusClient = null;
		_assignedItemId = false;
		
		returnToPool();
	}
	
	long itemId()
	{
		return _itemId;
	}
	
	LongObject itemIdObj()
	{
		return _itemIdObj;
	}
	
	IntObject streamIdObj()
	{
		return _streamIdObj;
	}
	
	void reset(T client, Object closure, Item<T> parent)
	{
		_domainType = 0;
		_streamId = 0;
		_etaStreamId = 0;
		_hasEtaStreamId = false;
		_closure = closure;
		_parent = parent;
		_client = client;
		_closedStatusClient = null;
		_assignedItemId = false;
	}
	
	int streamId()
	{
		return _streamId;
	}

	int etaStreamId()
	{
		return _hasEtaStreamId ? _etaStreamId : _streamId;
	}

	abstract boolean open(com.refinitiv.ema.access.ReqMsg reqMsg);
	abstract boolean modify(com.refinitiv.ema.access.ReqMsg reqMsg);
	boolean submit(com.refinitiv.ema.access.RefreshMsg refreshMsg)
	{
		return false;
	}
	boolean submit(com.refinitiv.ema.access.UpdateMsg updateMsg)
	{
		return false;
	}
	boolean submit(com.refinitiv.ema.access.StatusMsg statusMsg)
	{
		return false;
	}
	boolean submit(com.refinitiv.ema.access.PostMsg postMsg)
	{
		return false;
	}
	boolean submit(com.refinitiv.ema.access.GenericMsg genericMsg)
	{
		return false;
	}

	void setUserSpecObject(ReactorSubmitOptions rsslSubmitOptions)
	{
		rsslSubmitOptions.requestMsgOptions().userSpecObj(this);
	}

	abstract boolean close();
	abstract void remove();
	abstract int type();
	abstract Directory<T> directory();
	abstract int getNextStreamId(int numOfItem);
}

class SingleItem<T> extends Item<T>
{
	private static final String 	CLIENT_NAME = "SingleItem";
	
	protected Directory<T>	_directory;
	protected String _serviceName;
	protected OmmBaseImpl<T>			_baseImpl;
	protected RequestMsg  _requestMsg;
	protected int _state =  ItemStates.NORMAL;
	protected ServiceListImpl _serviceList;
	protected boolean _retrytosameChannel = false;
	protected OpenSuspectClient<T> _openSuspectClient;
	String _itemName = "";
	
	/* The following is used to handle the closed status message from provider. */
	HashSet<Directory<T>> _itemClosedDirHash; /* This keep a list of Directory objects which sends item closed status for this item */
	int _lastDataState = 0;
	int _lastStatusCode = 0;
	String _lastStatusText = "";
	
	/* Defines Item states for handling item watchlist by EMA */
	public static class ItemStates
	{
		public final static int NORMAL = 0;
		
		/* This indicates that this item is being recovered */
		public final static int RECOVERING = 1;
		
		/* This indicates that this item is being recovered but there is no matching service */
		public final static int RECOVERING_NO_MATHCING = 2;
		
		/* This is used to close this stream with the watchlist only without removing this item */
		public final static int CLOSING_STREAM = 3;
		
		/* This is used to indicate that this item is removed so it is not recovered */
		public final static int REMOVED = 4;
		
		/* This indicates that this item is being recovered by watchlist when the requested service is operational. */
		public final static int RECOVERING_BY_WATCHLIST = 5;
	}

	SingleItem() {}
	
	SingleItem(OmmBaseImpl<T> baseImpl,T client, Object closure , Item<T> batchItem)
	{
		super(client, closure, batchItem);
		_baseImpl = baseImpl;
		
		_state = ItemStates.NORMAL;
	}
	
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure , Item<T> batchItem)
	{
		super.reset(client, closure, batchItem);
		_baseImpl = baseImpl;
		
		_directory = null;
		
		_serviceList = null;
		
		_state = ItemStates.NORMAL;
		
		_retrytosameChannel = false;
		
		_itemName = "";
		
		_requestMsg = null;
		
		_serviceName = null;
		
		_itemClosedDirHash = null;
		
		_lastDataState = 0;
		_lastStatusCode = 0;
		_lastStatusText = "";
		
		_openSuspectClient = null;
	}
	
	int state()
	{
		return _state;
	}
	
	void state(int state)
	{
		_state = state;
	}
	
	ConsumerSession<T> session()
	{
		return _baseImpl.consumerSession();
	}
	
	@Override
	Directory<T> directory()
	{
		return _directory;
	}
	
	@Override
	int type()
	{
		return ItemType.SINGLE_ITEM;
	}

	@SuppressWarnings("unchecked")
	@Override
	boolean open(com.refinitiv.ema.access.ReqMsg reqMsg)
	{
		Directory<T> directory = null;
		ConsumerSession<T> consumerSession = session();
		LoginRefresh loginRefreshMsg = null;
		RequestMsg rsslRequestMsg = ((ReqMsgImpl)reqMsg).rsslMsg();
		SessionDirectory<T> sessionDirectory = null;

		if (reqMsg.hasServiceName())
		{	
			if(consumerSession != null)
			{
				sessionDirectory = consumerSession.sessionDirectoryByName(reqMsg.serviceName());
				
				if(sessionDirectory != null)
				{
					HashSet<SingleItem<T>> existingItemSet = null;
					
					if (reqMsg.hasName())
					{
						_itemName = reqMsg.name();
						
						existingItemSet = sessionDirectory.getDirectoryByItemName(reqMsg.name());
						
						directory = sessionDirectory.matchingWithExistingDirectory(existingItemSet, rsslRequestMsg);
					}
					
					/* Joins the same ReactorChannel with the existing item */
					if(directory != null && existingItemSet != null)
					{
						sessionDirectory.putDirectoryByHashSet(existingItemSet, this);
					}
					else
					{
						directory = sessionDirectory.directory(rsslRequestMsg);
						
						if(reqMsg.hasName() && directory != null)
						{
							_itemName = reqMsg.name();
							sessionDirectory.putDirectoryByItemName(_itemName, this);
						}
					}
				}
				
				loginRefreshMsg = consumerSession.loginRefresh();
			}
			else
			{
				directory = _baseImpl.directoryCallbackClient().directory(reqMsg.serviceName());
				loginRefreshMsg = _baseImpl.loginCallbackClient().loginRefreshMsg();
			}
			
			if (directory == null && (!loginRefreshMsg.attrib().checkHasSingleOpen() || loginRefreshMsg.attrib().singleOpen() == 0))
			{
	        	/* Assign a handle to this request.  This will be valid until the closed status event is given to the user */
				_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);
				
				/* EMA is generating the status down response because the service is not valid */
				StringBuilder temp = _baseImpl.strBuilder();
	        	temp.append("Service name of '")
	        		.append(reqMsg.serviceName())
	        		.append("' is not found.");

	        	scheduleItemClosedStatus(_baseImpl.itemCallbackClient(),
											this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
											temp.toString(), reqMsg.serviceName());
	        	
	        	return true;
			}
			else if (directory == null && consumerSession != null)
			{
				/* Add this item into the pending queue of the requested service name */
				consumerSession.addPendingRequestByServiceName(reqMsg.serviceName(), this, reqMsg);
				
				if (reqMsg.hasName())
				{
					_itemName = reqMsg.name();
				}
				
				/* Specify service name */
				_serviceName = reqMsg.serviceName();
	        	
	        	/* Assign a handle to this request.  This will be valid until the user close this handle */
				_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);
				_assignedItemId = true;
				
				StringBuilder temp = _baseImpl.strBuilder();
				if(sessionDirectory == null)
				{
					temp.append("No matching service present.");
				}
				else
				{
					temp.append(sessionDirectory.watchlistResult().resultText);
				}

				scheduleItemOpenOkStatus(_baseImpl.itemCallbackClient(),
											this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
											temp.toString(), reqMsg.serviceName());
	        	return true;
			}
		}
		else if (reqMsg.hasServiceListName())
		{	
			if(consumerSession != null)
			{
				ServiceListImpl serviceList = consumerSession.serviceList(reqMsg.serviceListName());
				
				if(serviceList != null)
				{
					_serviceList = serviceList;
					
					for(String serviceName : serviceList.concreteServiceList())
					{
						sessionDirectory = consumerSession.sessionDirectoryByName(serviceName);
						
						if(sessionDirectory != null)
						{
							HashSet<SingleItem<T>> existingItemSet = null;
							
							if(reqMsg.hasName())
							{
								_itemName = reqMsg.name();
								
								existingItemSet = sessionDirectory.getDirectoryByItemName(reqMsg.name());
							
								directory = sessionDirectory.matchingWithExistingDirectory(existingItemSet, rsslRequestMsg);
							}
							
							/* Joins the same ReactorChannel with the existing item */
							if(directory != null && existingItemSet != null)
							{
								sessionDirectory.putDirectoryByHashSet(existingItemSet, this);
								break;
							}
							else
							{
								directory = consumerSession.sessionDirectoryByName(serviceName).directory(rsslRequestMsg);
								
								if(directory != null)
								{	
									if(reqMsg.hasName())
									{
										_itemName = reqMsg.name();
										sessionDirectory.putDirectoryByItemName(_itemName, this);
									}
									
									break;
								}
							}
						}
					}
					
					if(directory == null)
					{	
						serviceList.addPendingRequest((SingleItem<OmmConsumerClient>) this, reqMsg);
						
						if(reqMsg.hasName())
						{
							_itemName = reqMsg.name();
						}
						
						/* Specify service list name */
						_serviceName = reqMsg.serviceListName();
						
			        	/* Assign a handle to this request.  This will be valid until the closed status event is given to the user */
						_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);
						
						StringBuilder temp = _baseImpl.strBuilder();
						if(sessionDirectory == null)
						{
							temp.append("No matching service present.");
						}
						else
						{
							temp.append(sessionDirectory.watchlistResult().resultText);
						}

						scheduleItemOpenOkStatus(_baseImpl.itemCallbackClient(),
													this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
													temp.toString(), reqMsg.serviceListName());
			        	return true;
					}
				}
				else
				{
					StringBuilder temp = _baseImpl.strBuilder();
					
					temp.append("The service list name '" + reqMsg.serviceListName() + "' does not exist.");
					
					_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
					
					return false;
				}
			}
			else
			{				
				ServiceListImpl serviceList = _baseImpl.serviceList(reqMsg.serviceListName());
				
				if(serviceList != null)
				{
					for(String serviceName : serviceList.concreteServiceList())
					{
						directory = _baseImpl.directoryCallbackClient().directory(serviceName);
						
						if(directory != null)
						{
							break;
						}
					}
				}
				else
				{
					StringBuilder temp = _baseImpl.strBuilder();
					
					temp.append("The service list name '" + reqMsg.serviceListName() + "' does not exist.");
					
					_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
					
					return false;
				}
			}
		}
		else
		{
			if (reqMsg.hasServiceId())
			{
				if(consumerSession != null)
				{
					sessionDirectory = consumerSession.sessionDirectoryById(reqMsg.serviceId());
					
					if(sessionDirectory != null)
					{
						HashSet<SingleItem<T>> existingItemSet = null;
						
						if(reqMsg.hasName())
						{
							_itemName = reqMsg.name();
							
							existingItemSet = sessionDirectory.getDirectoryByItemName(reqMsg.name());
							
							directory = sessionDirectory.matchingWithExistingDirectory(existingItemSet, rsslRequestMsg);
						}
						
						/* Joins the same ReactorChannel with the existing item */
						if(directory != null && existingItemSet != null)
						{
							sessionDirectory.putDirectoryByHashSet(existingItemSet, this);
						}
						else
						{
							directory = sessionDirectory.directory(rsslRequestMsg);
							
							if(reqMsg.hasName() && directory != null)
							{
								_itemName = reqMsg.name();
								sessionDirectory.putDirectoryByItemName(_itemName, this);
							}
						}
						
						if(directory != null)
						{
							/* Overrides with the actual service ID from source directory */
							reqMsg.serviceId(directory.service().serviceId());
						}
					}
					
					loginRefreshMsg = consumerSession.loginRefresh();
				}
				else
				{
					directory = _baseImpl.directoryCallbackClient().directory(reqMsg.serviceId());
					
					loginRefreshMsg = _baseImpl.loginCallbackClient().loginRefreshMsg();
				}
			}
			else
			{
				/* Assign a handle to this request.  This will be valid until the closed status event is given to the user */
				_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);
				scheduleItemClosedStatus(_baseImpl.itemCallbackClient(),
						this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
						"Passed in request message does not identify any service.",
						null);
	        	
				return true;
			}
			
			if (consumerSession == null)
			{
				if(directory == null && (!loginRefreshMsg.attrib().checkHasSingleOpen() || loginRefreshMsg.attrib().singleOpen() == 0))
				{		        	
		        	/* Assign a handle to this request.  This will be valid until the closed status event is given to the user */
					_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);
					
					StringBuilder temp = _baseImpl.strBuilder();
					
		        	temp.append("Service id of '")
		        		.append(reqMsg.serviceId())
		        		.append("' is not found.");
		        	
		        	scheduleItemClosedStatus(_baseImpl.itemCallbackClient(),
							this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
							temp.toString(), null);
		        	
		        	return true;
				}
			}
			else
			{	/* ConsumerSession is enabled */
				if (directory == null)
				{
					if(sessionDirectory == null)
					{
			        	/* Assign a handle to this request.  This will be valid until the closed status event is given to the user */
						_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);
						
						StringBuilder temp = _baseImpl.strBuilder();
						
			        	temp.append("Service id of '")
			        		.append(reqMsg.serviceId())
			        		.append("' is not found.");
			        	
			        	scheduleItemClosedStatus(_baseImpl.itemCallbackClient(),
								this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
								temp.toString(), null);
			        	
			        	return true;
					}
					else
					{
						StringBuilder temp = _baseImpl.strBuilder();
						
						/* Add this item into the pending queue of the requested service Id */	
						consumerSession.addPendingRequestByServiceId(sessionDirectory, this, reqMsg);
						
						if(reqMsg.hasName())
						{
							_itemName = reqMsg.name();
						}
						
						/* Specify service name */
						_serviceName = sessionDirectory.serviceName();
			        	
			        	/* Assign a handle to this request.  This will be valid until the user close this handle */
						_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);
						_assignedItemId = true;
						
						temp.append(sessionDirectory.watchlistResult().resultText);
		
						scheduleItemOpenOkStatus(_baseImpl.itemCallbackClient(),
													this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
													temp.toString(), sessionDirectory.serviceName());
			        	return true;
					}
				}
			}
		}

		_directory = directory;
		
		_serviceName = reqMsg.hasServiceName() ? reqMsg.serviceName() : null;

		return rsslSubmit(rsslRequestMsg, true);
	}
	
	@Override
	boolean modify(com.refinitiv.ema.access.ReqMsg reqMsg)
	{
		return rsslSubmit(((ReqMsgImpl) reqMsg).rsslMsg(), true);
	}

	@Override
	boolean submit(com.refinitiv.ema.access.PostMsg postMsg)
	{
		return rsslSubmit(((PostMsgImpl) postMsg).rsslMsg(), postMsg.hasServiceName() ? postMsg.serviceName() : null);
	}

	@Override
	boolean submit(com.refinitiv.ema.access.GenericMsg genericMsg)
	{
		ConsumerSession<T> consumerSession = session();
		if(consumerSession != null)
		{
			if(genericMsg.hasServiceId())
			{
				int serviceId =genericMsg.serviceId();
				
				if(serviceId == _directory.generatedServiceId())
				{
					/* Translate to the actual service Id */
					genericMsg.serviceId(_directory.service().serviceId());
				}
			}
			
			return rsslSubmit(((GenericMsgImpl) genericMsg).rsslMsg());
		}
		else
		{
			return rsslSubmit(((GenericMsgImpl) genericMsg).rsslMsg());
		}
	}
	
	@Override
	boolean submit(com.refinitiv.ema.access.RefreshMsg refreshMsg) 
	{
		return rsslSubmit( ((RefreshMsgImpl)refreshMsg).rsslMsg());
	}

	@Override
	boolean submit(com.refinitiv.ema.access.UpdateMsg updateMsg)
	{
		return rsslSubmit( ((UpdateMsgImpl)updateMsg).rsslMsg());
	}

	@Override
	boolean submit(com.refinitiv.ema.access.StatusMsg statusMsg)
	{
		return rsslSubmit(((StatusMsgImpl)statusMsg).rsslMsg());
	}
	
	@Override
	boolean close()
	{
		boolean retCode = true;
		
		/* Don't send a close message to the ReactorChannel when the item is being recovered by EMA */
		if (_state == ItemStates.NORMAL || _state == ItemStates.CLOSING_STREAM)
		{
			CloseMsg rsslCloseMsg = _baseImpl.itemCallbackClient().rsslCloseMsg();
			rsslCloseMsg.containerType(DataTypes.NO_DATA);
			rsslCloseMsg.domainType(_domainType);
			
			retCode = rsslSubmit(rsslCloseMsg);
			
			if(_state == ItemStates.CLOSING_STREAM)
				return retCode;
		}

		remove();
		
		return retCode;
	}
	
	@Override
	void remove()
	{
		if (type() != ItemType.BATCH_ITEM)
		{
			if (_parent != null)
			{
				if (_parent.type() == ItemType.BATCH_ITEM)
					((BatchItem<T>)_parent).decreaseItemCount();
			}
			
			if(_directory != null)
			{
				SessionDirectory<T> sessionDirectory = _directory.sessionDirectory();
				if(_directory.sessionDirectory() != null)
				{
					sessionDirectory.removeDirectoryByItemName(_itemName, this);
					
					@SuppressWarnings("unchecked")
					SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>)_directory.channelInfo().sessionChannelInfo();
					if(sessionChannelInfo != null)
					{
						 if (_directory.channelInfo().getReactorChannelType() == ReactorChannelType.WARM_STANDBY)
						 {
							 int wsbMode = sessionChannelInfo.getWarmStandbyMode(sessionChannelInfo.reactorChannel());
								
								if(wsbMode == ReactorWarmStandbyMode.LOGIN_BASED)
								{
									sessionChannelInfo.removeWSBStaleItem(this);
								}
								else if (wsbMode == ReactorWarmStandbyMode.SERVICE_BASED)
								{
									sessionChannelInfo.removeWSBServiceBasedStaleItem(_directory.serviceName(), this);
								}
						 }
					}
				}
				
				_requestMsg = null;
				_itemClosedDirHash = null;
			}
			
			boolean returnToPool = !(state() == SingleItem.ItemStates.RECOVERING || state() == SingleItem.ItemStates.RECOVERING_NO_MATHCING || state() == SingleItem.ItemStates.RECOVERING_BY_WATCHLIST);

			_baseImpl.itemCallbackClient().removeFromMap(this, returnToPool);
			
			state(SingleItem.ItemStates.REMOVED);
		}
	}

	boolean rsslSubmit(com.refinitiv.eta.codec.RequestMsg rsslRequestMsg, boolean reportError)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		
		if (!rsslRequestMsg.msgKey().checkHasServiceId() && _directory != null)
			rsslSubmitOptions.serviceName(_directory.serviceName());
		
		if (_directory == null)
			rsslSubmitOptions.serviceName(_serviceName);

		if (!rsslRequestMsg.checkHasQos())
		{
			rsslRequestMsg.applyHasQos();
			rsslRequestMsg.applyHasWorstQos();
			rsslRequestMsg.qos().dynamic(false);
			rsslRequestMsg.qos().timeliness(QosTimeliness.REALTIME);
			rsslRequestMsg.qos().rate(QosRates.TICK_BY_TICK);
			rsslRequestMsg.worstQos().rate(QosRates.TIME_CONFLATED);
			rsslRequestMsg.worstQos().timeliness(QosTimeliness.DELAYED_UNKNOWN);
			rsslRequestMsg.worstQos().rateInfo(65535);
		}	
		
		if (_baseImpl.activeConfig().msgKeyInUpdates)
			rsslRequestMsg.applyMsgKeyInUpdates();

		setUserSpecObject(rsslSubmitOptions);
		
		int domainType =  rsslRequestMsg.domainType();
		
		if (_streamId == 0) // if streamId is zero, this is not the case of the provider-driven item, so we can operate with the _streamId
		{
			if (rsslRequestMsg.checkHasBatch())
			{
				List<SingleItem<T>> items = ((BatchItem<T>)this).singleItemList();
				int numOfItem = items.size();

				rsslRequestMsg.streamId(getNextStreamId(numOfItem));
				_streamId = rsslRequestMsg.streamId();    // this is batch, ok to use this streamId since we are not dealing with provider-driven items

				if (!_assignedItemId)
				{
					_baseImpl._itemCallbackClient.addToMap(_baseImpl.nextLongId(), this);
				}
				else
				{
					_baseImpl._itemCallbackClient.addToMap(_itemId, this);
				}
				
				SingleItem<T> item;
				int itemStreamIdStart = _streamId; // this is batch, _streamId is both ema-facing and eta-facing
				int originalBatchFlags = rsslRequestMsg.flags();
				for ( int index = 0; index < numOfItem; index++)
				{
					item = items.get(index);
					item._directory = _directory;
					item._streamId = ++itemStreamIdStart;
					item._domainType = domainType;
					item._serviceList = _serviceList;
					_baseImpl._itemCallbackClient.addToMap(_baseImpl.nextLongId(), item);
					
					if(session() != null)
					{
						int flags =  rsslRequestMsg.flags();
						flags &= ~RequestMsgFlags.HAS_BATCH;
						rsslRequestMsg.flags(flags);
						
						if(item._requestMsg == null)
						{
							item._requestMsg  = (RequestMsg)CodecFactory.createMsg();
							
							item._requestMsg .msgClass(MsgClasses.REQUEST);
							rsslRequestMsg.copy(item._requestMsg , CopyMsgFlags.ALL_FLAGS);
							item._requestMsg.msgKey().applyHasName();
							item._requestMsg.msgKey().name().data(item._itemName);
							item._requestMsg.streamId(item._streamId);
						}
					}
				}
				
				/* Restore the original flags for the batch request */
				rsslRequestMsg.flags(originalBatchFlags);
			}
			else
			{
				rsslRequestMsg.streamId(getNextStreamId(0));
				_streamId = rsslRequestMsg.streamId(); // streamId is never 0 for SingleItemWithSource instances, so _streamId is ok (this is item for which _streamId and _etaStreamId are the same)
				/* Here need to add the item to hashmap FIRST because the response for this item driven by dispatch thread could comes back before open() returns.
				 * If it is the case, the response of closed state could call remove() without removing anything from the hashmap. It will leads to mem growth finally.
				 */
				if(!_assignedItemId)
				{
					_baseImpl._itemCallbackClient.addToMap(_baseImpl.nextLongId(), this);
				}
				else
				{
					_baseImpl._itemCallbackClient.addToMap(_itemId, this);
				}
			}
		}
		else
			rsslRequestMsg.streamId(etaStreamId()); // we are sending out this message to eta layer, need to assign eta-facing streamId in case this is SingleItemWithSource

		if (_domainType == 0)
			_domainType = domainType;
		else
			rsslRequestMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel; 
				
		if(session() != null)
		{
		    rsslChannel = _directory.channelInfo().rsslReactorChannel();
		}
		else
		{
			ChannelInfo activeChannel = _baseImpl.loginCallbackClient().activeChannelInfo();
			rsslChannel = (activeChannel != null) ? activeChannel.rsslReactorChannel() : null;
		}

		if (rsslChannel == null)
		{
			StringBuilder message = _baseImpl.strBuilder();

			if(reportError)
			{
				if (_baseImpl.loggerClient().isErrorEnabled())
				{
					message.append("Internal error: rsslChannel.Submit() failed in SingleItem.submit(RequestMsg)").append(OmmLoggerClient.CR)
						.append("\tReactorChannel is not available");
	
					_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, message.toString(), Severity.ERROR));
	
					message.setLength(0);
				}
	
				message.append("Failed to open or modify item request. Reason: ReactorChannel is not available");
	
				_baseImpl.handleInvalidUsage(message.toString(), ReactorReturnCodes.FAILURE);
			}

			return false;
		}

		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			if(reportError)
			{
				StringBuilder temp = _baseImpl.strBuilder();
				if (_baseImpl.loggerClient().isErrorEnabled())
	        	{
					com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
					
		        	temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(RequestMsg)")
		        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
		    			.append(OmmLoggerClient.CR)
		    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
		    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
		    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
		    			.append("Error Text ").append(error.text());
		        	
		        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
		        	
		        	temp.setLength(0);
	        	}
				
				temp.append("Failed to open or modify item request. Reason: ")
					.append(ReactorReturnCodes.toString(ret))
					.append(". Error text: ")
					.append(rsslErrorInfo.error().text());
					
				_baseImpl.handleInvalidUsage(temp.toString(), ret);
			}
			
			return false;
	    }

        /* Store the request message for recovery and view tracking.
         * reportError is used to determine whether this is an application submit or Watchlist submit.
         * Watchlist Submits do not require copying. */
        if(session() != null && reportError && !rsslRequestMsg.checkHasBatch())
        {
            if(_requestMsg == null)
            {
                _requestMsg = (RequestMsg)CodecFactory.createMsg();
                _requestMsg.msgClass(MsgClasses.REQUEST);
            }

            /* Always update the stored request message with the latest view information */
            rsslRequestMsg.copy(_requestMsg, CopyMsgFlags.ALL_FLAGS);

        }
		

		return true;
	}

	boolean rsslSubmit(com.refinitiv.eta.codec.CloseMsg rsslCloseMsg)
	{	
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.serviceName(null);

		rsslSubmitOptions.requestMsgOptions().userSpecObj(this);
	
		if (_streamId == 0)
		{
			if (_baseImpl.loggerClient().isErrorEnabled())
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SingleItem.CLIENT_NAME,
	        									"Invalid streamId for this item in in SingleItem.submit(CloseMsg)",
	        									Severity.ERROR));
		}
		else
			rsslCloseMsg.streamId(etaStreamId()); // we are sending out this message to eta layer, need to assign eta-facing streamId
	
		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel;
		
		if(session() == null)
		{	ChannelInfo activeChannel = _baseImpl.loginCallbackClient().activeChannelInfo();
			rsslChannel = (activeChannel != null) ? activeChannel.rsslReactorChannel() : null;
		}
		else
			rsslChannel = _directory.channelInfo().rsslReactorChannel();
		
		if (rsslChannel == null)
		{
			StringBuilder message = _baseImpl.strBuilder();

			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				message.append("Internal error: rsslChannel.Submit() failed in SingleItem.submit(CloseMsg)").append(OmmLoggerClient.CR)
					.append("\tReactorChannel is not available");

				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, message.toString(), Severity.ERROR));

				message.setLength(0);
			}

			message.append("Failed to close item request. Reason: ReactorChannel is not available");

			_baseImpl.handleInvalidUsage(message.toString(), ReactorReturnCodes.FAILURE);

			return false;
		}

		int ret;
			
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslCloseMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			
			if (_baseImpl.loggerClient().isErrorEnabled())
	    	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: ReactorChannel.submit() failed in SingleItem.submit(CloseMsg)")
	        	.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
	    	}
			
			temp.append("Failed to close item request. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				

			_baseImpl.handleInvalidUsage(temp.toString(), ret);
	
			return false;
	    }
	
		return true;
	}

	boolean rsslSubmit(com.refinitiv.eta.codec.PostMsg rsslPostMsg, String serviceName)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		if(!validateServiceName(rsslPostMsg, serviceName)){
			return false;
		}
		rsslSubmitOptions.serviceName(serviceName);
		rsslSubmitOptions.requestMsgOptions().clear();
		
		rsslPostMsg.streamId(etaStreamId()); // we are sending out this message to eta layer, need to assign eta-facing streamId
		rsslPostMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel;
		
		if(session() == null)
		{
			ChannelInfo activeChannel = _baseImpl.loginCallbackClient().activeChannelInfo();
			rsslChannel = (activeChannel != null) ? activeChannel.rsslReactorChannel() : null;
		}
		else
			rsslChannel = _directory.channelInfo().rsslReactorChannel();
		
		if (rsslChannel == null)
		{
			StringBuilder message = _baseImpl.strBuilder();

			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				message.append("Internal error: rsslChannel.Submit() failed in SingleItem.submit(PostMsg)").append(OmmLoggerClient.CR)
					.append("\tReactorChannel is not available");

				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, message.toString(), Severity.ERROR));

				message.setLength(0);
			}

			message.append("Failed to submit PostMsg on item stream. Reason: ReactorChannel is not available");

			_baseImpl.handleInvalidUsage(message.toString(), ReactorReturnCodes.FAILURE);

			return false;
		}

		int ret;
			
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslPostMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(PostMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit PostMsg on item stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				

			_baseImpl.handleInvalidUsage(temp.toString(), ret);

			return false;
	    }
        
		return true;
	}

	private boolean validateServiceName(com.refinitiv.eta.codec.PostMsg rsslPostMsg, String serviceName)
	{
		Directory<T> directory = null;
		String errorText = null;
		
		if(_baseImpl.consumerSession() != null)
		{
			@SuppressWarnings("unchecked")
			SessionChannelInfo<T> sessionChannelInfo = (SessionChannelInfo<T>) _directory.channelInfo().sessionChannelInfo();
			
			if(serviceName != null)
			{
				directory = sessionChannelInfo.getDirectoryByName(serviceName);
				
				if(directory != null && directory.service().action() != MapEntryActions.DELETE)
				{
					return true;
				}
				
				errorText = "Message submitted with unknown service name " + serviceName;
			}
			else if(rsslPostMsg.checkHasMsgKey() && rsslPostMsg.msgKey().checkHasServiceId())
			{
				/* Search from the generated service Id */
				SessionDirectory<T> sessionDirectory =_baseImpl.consumerSession().sessionDirectoryById(rsslPostMsg.msgKey().serviceId());
				
				if(sessionDirectory != null)
				{
					/* Search from service name whether this SessionChannelInfo has it. */ 
				    directory = sessionChannelInfo.getDirectoryByName(sessionDirectory.serviceName());
				
				    if(directory != null && directory.service().action() != MapEntryActions.DELETE)
				    {
				    	/* Translate to the actual service ID from the provider */
				    	rsslPostMsg.msgKey().serviceId(directory.service().serviceId());
				    	return true;
				    }
				}
				
				errorText = "Message submitted with unknown service Id " + rsslPostMsg.msgKey().serviceId();
			}
			else
			{
				errorText = "Message submitted with unspecified service Name and Id";
			}
		}
		else
		{
			directory = _baseImpl.directoryCallbackClient().directory(serviceName);
			
			if (serviceName == null || directory != null)
			{
				return true;
			}
			
			errorText = "Message submitted with unknown service name " + serviceName;
		}

		StringBuilder temp = _baseImpl.strBuilder();
		if (_baseImpl.loggerClient().isErrorEnabled())
		{
			temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(PostMsg)")
					.append(OmmLoggerClient.CR)
					.append("Error Id ").append(ReactorReturnCodes.INVALID_USAGE).append(OmmLoggerClient.CR)
					.append("Error Location ").append("ItemCallbackClient.rsslSubmit(PostMsg,String)").append(OmmLoggerClient.CR)
					.append("Error Text ").append(errorText);

			_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

			temp.setLength(0);
		}

		temp.append("Failed to submit PostMsg on item stream. Reason: ")
				.append(ReactorReturnCodes.toString(ReactorReturnCodes.INVALID_USAGE))
				.append(". Error text: ")
				.append(errorText);

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		return false;
	}
	
	boolean rsslSubmit(com.refinitiv.eta.codec.GenericMsg rsslGenericMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.serviceName(null);
		rsslSubmitOptions.requestMsgOptions().clear();
		
		rsslGenericMsg.streamId(etaStreamId()); // we are sending out this message to eta layer, need to assign eta-facing streamId
		if (rsslGenericMsg.domainType() == 0)
			rsslGenericMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel;
		
		if(session() == null)
		{
			ChannelInfo activeChannel = _baseImpl.loginCallbackClient().activeChannelInfo();
			rsslChannel =(activeChannel != null) ? activeChannel.rsslReactorChannel() : null;
		}
		else
			rsslChannel = _directory.channelInfo().rsslReactorChannel();
		
		if (rsslChannel == null)
		{
			StringBuilder message = _baseImpl.strBuilder();

			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				message.append("Internal error: rsslChannel.Submit() failed in SingleItem.submit(GenericMsg)").append(OmmLoggerClient.CR)
					.append("\tReactorChannel is not available");

				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, message.toString(), Severity.ERROR));

				message.setLength(0);
			}

			message.append("Failed to submit GenericMsg on item stream. Reason: ReactorChannel is not available");

			_baseImpl.handleInvalidUsage(message.toString(), ReactorReturnCodes.FAILURE);

			return false;
		}

		int ret;
			
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslGenericMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(GenericMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit GenericMsg on item stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				

			_baseImpl.handleInvalidUsage(temp.toString(), ret);

			return false;
	    }
        
		return true;
	}
	
	boolean rsslSubmit(com.refinitiv.eta.codec.RefreshMsg rsslRefreshMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.serviceName(null);
		rsslSubmitOptions.requestMsgOptions().clear();
		
		rsslRefreshMsg.streamId(etaStreamId()); // we are sending out this message to eta layer, need to assign eta-facing streamId
		rsslRefreshMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();

		ChannelInfo activeChannel = _baseImpl.loginCallbackClient().activeChannelInfo();
		ReactorChannel rsslChannel = (activeChannel != null) ? activeChannel.rsslReactorChannel() : null;

		if (rsslChannel == null)
		{
			StringBuilder message = _baseImpl.strBuilder();

			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				message.append("Internal error: rsslChannel.Submit() failed in SingleItem.submit(RefreshMsg)").append(OmmLoggerClient.CR)
					.append("\tReactorChannel is not available");

				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, message.toString(), Severity.ERROR));

				message.setLength(0);
			}

			message.append("Failed to submit RefreshMsg on item stream. Reason: ReactorChannel is not available");

			_baseImpl.handleInvalidUsage(message.toString(), ReactorReturnCodes.FAILURE);

			return false;
		}

		int ret;
			
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslRefreshMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(RefreshMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit RefreshMsg on item stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				

			_baseImpl.handleInvalidUsage(temp.toString(), ret);

			return false;
	    }
        
		return true;
	}
	
	boolean rsslSubmit(com.refinitiv.eta.codec.UpdateMsg rsslUpdateMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.serviceName(null);
		rsslSubmitOptions.requestMsgOptions().clear();
		
		rsslUpdateMsg.streamId(etaStreamId()); // we are sending out this message to eta layer, need to assign eta-facing streamId
		rsslUpdateMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();

		ChannelInfo activeChannel = _baseImpl.loginCallbackClient().activeChannelInfo();
		ReactorChannel rsslChannel = (activeChannel != null) ? activeChannel.rsslReactorChannel() : null;

		if (rsslChannel == null)
		{
			StringBuilder message = _baseImpl.strBuilder();

			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				message.append("Internal error: rsslChannel.Submit() failed in SingleItem.submit(UpdateMsg)").append(OmmLoggerClient.CR)
					.append("\tReactorChannel is not available");

				_baseImpl.loggerClient().error(CLIENT_NAME, message.toString());

				message.setLength(0);
			}

			message.append("Failed to submit UpdateMsg on item stream. Reason: ReactorChannel is not available");

			_baseImpl.handleInvalidUsage(message.toString(), ReactorReturnCodes.FAILURE);

			return false;
		}

		int ret;
		
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslUpdateMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(UpdateMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit UpdateMsg on item stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				

			_baseImpl.handleInvalidUsage(temp.toString(), ret);

			return false;
	    }
        
		return true;
	}
	
	boolean rsslSubmit(com.refinitiv.eta.codec.StatusMsg rsslStatusMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.requestMsgOptions().clear();
		
		rsslStatusMsg.streamId(etaStreamId()); // we are sending out this message to eta layer, need to assign eta-facing streamId
		rsslStatusMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();

		ChannelInfo activeChannel = _baseImpl.loginCallbackClient().activeChannelInfo();
		ReactorChannel rsslChannel = (activeChannel != null) ? activeChannel.rsslReactorChannel() : null;

		if (rsslChannel == null)
		{
			StringBuilder message = _baseImpl.strBuilder();

			if (_baseImpl.loggerClient().isErrorEnabled())
			{
				message.append("Internal error: rsslChannel.Submit() failed in SingleItem.submit(StatusMsg)").append(OmmLoggerClient.CR)
					.append("\tReactorChannel is not available");

				_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, message.toString(), Severity.ERROR));

				message.setLength(0);
			}

			message.append("Failed to submit StatusMsg on item stream. Reason: ReactorChannel is not available");

			_baseImpl.handleInvalidUsage(message.toString(), ReactorReturnCodes.FAILURE);

			return false;
		}

		int ret;
			
		if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslStatusMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in SingleItem.submit(StatusMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(SingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to submit StatusMsg on item stream. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				

			_baseImpl.handleInvalidUsage(temp.toString(), ret);

			return false;
	    }
        
		return true;
	}
	
	void scheduleItemClosedStatus(CallbackClient<T> client, Item<T> item, Msg rsslMsg, String statusText, String serviceName)
	{
		if (_closedStatusClient != null) return;
    	
		_closedStatusClient = new ClosedStatusClient<T>(client, item, rsslMsg, statusText, serviceName);
		_baseImpl.addTimeoutEvent(1000, _closedStatusClient);
	}
	
	void scheduleItemOpenOkStatus(CallbackClient<T> client, SingleItem<T> item, Msg rsslMsg, String statusText, String serviceName)
	{
		if(_openSuspectClient != null) return;
		
		_openSuspectClient = new OpenSuspectClient<T>(client, item, rsslMsg, statusText, serviceName);
		_baseImpl.addTimeoutEvent(1000, _openSuspectClient);
	}
		
	@Override
	int getNextStreamId(int numOfItem) {
		return _baseImpl._itemCallbackClient.nextStreamId(numOfItem);
	}

	OmmBaseImpl<T> baseImpl()
	{
		return _baseImpl;
	}

}

class SourceItem<T> extends Item<T>
{
	RequestMsg _initialRequestMsg = (RequestMsg)CodecFactory.createMsg();
	Map<Integer, SingleItemWithSource> _streamIdItemsMap = new HashMap<>(); // stores items by msg.streamId (from ValueAdd layer)
	SingleItemWithSource<T> symbolListItem;

	boolean _hasBehavior = false;
	int _symbolListFlags;

	@Override
	boolean open(ReqMsg reqMsg) { return false; }

	@Override
	boolean modify(ReqMsg reqMsg) { return false; }

	@Override
	boolean close() { return false; }

	@Override
	void remove() { }

	@Override
	int type()
	{
		return ItemType.SYMBOL_LIST_SOURCE_ITEM;
	}

	@Override
	Directory<T> directory() {
		return null;
	}

	@Override
	int getNextStreamId(int numOfItem) {
		return 0;
	}

	@SuppressWarnings("unchecked")
	Item<T> getItem(int streamId)
	{
		SingleItemWithSource<T> item = null;

		if (_streamIdItemsMap.containsKey(streamId))
		{
			item = _streamIdItemsMap.get(streamId);
		}

		return item;
	}

	SingleItemWithSource<T> getItem(MsgImpl msg, boolean createIfNotFound)
	{
		return getItem(msg, msg.streamId(), createIfNotFound);
	}

	@SuppressWarnings("unchecked")
	SingleItemWithSource<T> getItem(MsgImpl msg, int streamId, boolean createIfNotFound)
	{
		SingleItemWithSource<T> item = null;

		if (_streamIdItemsMap.containsKey(msg.streamId()))
		{
			item = _streamIdItemsMap.get(msg.streamId());
		}
		else if (symbolListItem != null && createIfNotFound)
		{
			if ((item = (SingleItemWithSource<T>)symbolListItem._baseImpl.objManager()._singleItemWithSourcePool.poll()) == null)
			{
				item = new SingleItemWithSource<T>(symbolListItem._baseImpl, symbolListItem._client, symbolListItem._closure);
				symbolListItem._baseImpl.objManager()._singleItemWithSourcePool.updatePool(item);
			}
			else
				item.reset(symbolListItem._baseImpl, symbolListItem._client, symbolListItem._closure, null);

			item._streamId = streamId;
			item._etaStreamId = msg.streamId();
			item._hasEtaStreamId = true;
			_streamIdItemsMap.put(msg.streamId(), item);
			item.source = this;


			item._directory = symbolListItem._directory;
			item._domainType = msg.domainType();
			item._serviceList = symbolListItem._serviceList;
			item._serviceName = symbolListItem._serviceName;

			item._requestMsg = (RequestMsg)CodecFactory.createMsg();
			item._requestMsg.msgClass(MsgClasses.REQUEST);
			item._requestMsg.streamId(msg._rsslMsg.streamId());

			if (msg._rsslMsg.msgKey().checkHasNameType())
			{
				item._requestMsg.msgKey().applyHasNameType();
				item._requestMsg.msgKey().nameType(msg._rsslMsg.msgKey().nameType());
			}

			item._requestMsg.domainType(msg.domainType());
			item._requestMsg.containerType(DataTypes.NO_DATA);
			if ((_symbolListFlags & SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS) > 0) item._requestMsg.applyStreaming();

			int serviceId = 0;
			Directory<T> directory = null;
			if (symbolListItem._directory != null)
			{
				directory = symbolListItem._directory;
			}
			else
			{
				if (_initialRequestMsg.msgKey().checkHasServiceId())
				{
					serviceId = _initialRequestMsg.msgKey().serviceId();
					directory = symbolListItem._baseImpl.directoryCallbackClient().directory(_initialRequestMsg.msgKey().serviceId());
				}
				else if (symbolListItem._serviceName != null)
				{
					directory = symbolListItem._baseImpl.directoryCallbackClient().directory(symbolListItem._serviceName);
					serviceId = directory.service().serviceId();
				}
			}

			if (serviceId != 0)
			{
				item._requestMsg.msgKey().applyHasServiceId();
				item._requestMsg.msgKey().serviceId(serviceId);
			}

			Qos itemQos = null;

			item._requestMsg.applyHasQos();
			if (directory != null && !directory.service().info().qosList().isEmpty())
			{
				itemQos = directory.service().info().bestQos();
				itemQos.copy(item._requestMsg.qos());
			}
			else
			{
				item._requestMsg.qos().rate(QosRates.TICK_BY_TICK);
				item._requestMsg.qos().timeliness(QosTimeliness.REALTIME);
			}

			if (msg._rsslMsg.msgKey() != null && msg._rsslMsg.msgKey().checkHasName())
			{
				ByteBuffer name = ByteBuffer.allocate(msg._rsslMsg.msgKey().name().length());
				for (int i = msg._rsslMsg.msgKey().name().position(); i < msg._rsslMsg.msgKey().name().position() + msg._rsslMsg.msgKey().name().length(); i++)
				{
					name.put(msg._rsslMsg.msgKey().name().data().get(i));
				}
				name.flip();
				item._requestMsg.msgKey().applyHasName();
				item._requestMsg.msgKey().name().data(name);

				item._itemName = msg._rsslMsg.msgKey().name().toString();
			}

			long handle = symbolListItem._baseImpl.nextLongId();
			symbolListItem._baseImpl.itemCallbackClient().addToMap(handle, item);
		}

		return item;
	}
}

class SingleItemWithSource<T> extends SingleItem<T>
{
	com.refinitiv.eta.codec.ElementList _elementList;
	com.refinitiv.eta.codec.ElementEntry _elementEntry;
	com.refinitiv.eta.codec.ElementList _behaviourElementList;
	com.refinitiv.eta.codec.ElementEntry _behaviourEntry;
	com.refinitiv.eta.codec.UInt _dataStreamFlag;
	com.refinitiv.eta.codec.DecodeIterator _dIter;

	boolean _initialized = false;

	SourceItem<T> source;
	ReactorChannel _reactorChannel;

	SingleItemWithSource()
	{
		super();
	}

	SingleItemWithSource(OmmBaseImpl<T> baseImpl,T client, Object closure)
	{
		super(baseImpl, client, closure, null);
	}

	@Override
	int type()
	{
		return ItemType.SINGLE_ITEM_WITH_SOURCE;
	}

	@Override
	void setUserSpecObject(ReactorSubmitOptions rsslSubmitOptions)
	{
		SourceItem<T> source = this.source;

		if (source == null)
		{
			source = new SourceItem<T>();
			source.symbolListItem = this.copy();
			this.source = source;
		}

		rsslSubmitOptions.requestMsgOptions().userSpecObj(source);
	}

	@Override
	void remove()
	{
		if (source != null)
		{
			source._streamIdItemsMap.remove(etaStreamId());
			_initialized = false;
			this.source = null;
		}
		if (_baseImpl._itemCallbackClient._providerDrivenItemsByReactorChannel.containsKey(_reactorChannel))
		{
			_baseImpl._itemCallbackClient._providerDrivenItemsByReactorChannel.get(_reactorChannel).remove(this.etaStreamId());
		}
		_reactorChannel = null;

		super.remove();
	}

	@Override
	boolean rsslSubmit(com.refinitiv.eta.codec.RequestMsg rsslRequestMsg, boolean reportError)
	{
		boolean res = super.rsslSubmit(rsslRequestMsg, reportError);

		if (!_initialized)
		{
			_initialized = true;
			extractFlags(rsslRequestMsg);
			source._initialRequestMsg = (RequestMsg)CodecFactory.createMsg();
			rsslRequestMsg.copy(source._initialRequestMsg, CopyMsgFlags.ALL_FLAGS);
			source._streamIdItemsMap.put(rsslRequestMsg.streamId(), this);
		}

		return res;
	}

	@Override
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure , Item<T> batchItem)
	{
		super.reset(baseImpl, client, closure, batchItem);
		_initialized = false;
		source = null;
		_reactorChannel = null;
	}

	SingleItemWithSource<T> copy()
	{
		// this object should not be taken from pool
		SingleItemWithSource<T> res = new SingleItemWithSource<T>(this._baseImpl, this._client, this._closure);

		res._directory = this._directory;
		res._serviceList = this._serviceList;
		res._domainType = this._domainType;

		return res;
	}

	void extractFlags(com.refinitiv.eta.codec.RequestMsg requestMsg)
	{
		if (requestMsg.containerType() != DataTypes.ELEMENT_LIST)
			return; // nothing to extract


		if (_dIter == null) _dIter = CodecFactory.createDecodeIterator();
		if (_elementList == null) _elementList = CodecFactory.createElementList();
		_elementList.clear();

		_dIter.clear();
		_dIter.setBufferAndRWFVersion(requestMsg.encodedDataBody(), Codec.majorVersion(), Codec.minorVersion());

		int ret = _elementList.decode(_dIter, null);
		if (ret != CodecReturnCodes.SUCCESS)
		{
			return;
		}

		if (_elementEntry == null) _elementEntry = CodecFactory.createElementEntry();
		_elementEntry.clear();
		if (_behaviourElementList == null) _behaviourElementList = CodecFactory.createElementList();
		_behaviourElementList.clear();
		if (_behaviourEntry == null) _behaviourEntry = CodecFactory.createElementEntry();
		_behaviourEntry.clear();

		while ((ret = _elementEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret != CodecReturnCodes.SUCCESS)
			{
				return;
			}

			if (_elementEntry.name().equals(SymbolList.ElementNames.SYMBOL_LIST_BEHAVIORS))
			{
				source._hasBehavior = true;

				if (_elementEntry.dataType() != DataTypes.ELEMENT_LIST)
				{
					// Nothing to extract
					return;
				}

				ret = _behaviourElementList.decode(_dIter, null);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					while ((ret = _behaviourEntry.decode(_dIter)) != CodecReturnCodes.END_OF_CONTAINER)
					{
						if (ret < CodecReturnCodes.SUCCESS)
						{
							return;
						}
						else
						{
							if (_behaviourEntry.name().equals(SymbolList.ElementNames.SYMBOL_LIST_DATA_STREAMS))
							{
								if (_behaviourEntry.dataType() != DataTypes.UINT)
								{
									return;
								}

								if (_dataStreamFlag == null) _dataStreamFlag = CodecFactory.createUInt();

								ret = _dataStreamFlag.decode(_dIter);
								if (ret != CodecReturnCodes.SUCCESS)
								{
									return;
								}

								if ((int)_dataStreamFlag.toLong() < SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_NAMES_ONLY
										|| (int)_dataStreamFlag.toLong() > SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS )
								{
									return;
								}
								source._symbolListFlags = (int)_dataStreamFlag.toLong();
							}
						}
					}
				}
			}
		}
	}
}

interface ProviderItem
{
	void scheduleItemClosedRecoverableStatus(String statusText, boolean initiateTimeout);
	
	MsgKey rsslMsgKey();
	
	ClientSession clientSession();
	
	void sendCloseMsg();
	
	int serviceId();
	
	TimeoutEvent reqTimeoutEvent();
	
	boolean processInitialResp(com.refinitiv.eta.codec.RefreshMsg refreshMsg);
	
	ItemWatchList itemWatchList();
	
	void cancelReqTimerEvent();
	
	boolean requestWithService();

	ReentrantLock userLock();
}

abstract class IProviderSingleItem extends Item<OmmProviderClient> implements ProviderItem
{
	private static final String 	CLIENT_NAME = "IProviderSingleItem";
	
	protected Directory<OmmProviderClient>									_directory;
	protected OmmServerBaseImpl						    _baseImpl;
	protected MsgKey									_rsslMsgKey = CodecFactory.createMsgKey();
	protected ItemWatchList 							_itemWatchList;
	protected ClientSession								_clientSession;
	protected int										_serviceId;
	protected boolean									_isPrivateStream;
	protected boolean                                   _specifiedServiceInReq = false;
	private TimeoutEvent								_reqTimeoutEvent;

	IProviderSingleItem() {}
	
	IProviderSingleItem(OmmServerBaseImpl baseImpl,OmmProviderClient client, Object closure , Item<OmmProviderClient> batchItem)
	{
		super(client, closure, batchItem);
		_baseImpl = baseImpl;
		_itemWatchList = ((OmmIProviderImpl)baseImpl).itemWatchList();
	}
	
	void reset(OmmServerBaseImpl baseImpl, OmmProviderClient client, Object closure , Item<OmmProviderClient> batchItem)
	{
		super.reset(client, closure, batchItem);
		_baseImpl = baseImpl;
		_itemWatchList = ((OmmIProviderImpl)baseImpl).itemWatchList();
	}
	
	@Override
	public ClientSession clientSession()
	{
		return _clientSession;
	}
	
	void clientSession(ClientSession clientSession)
	{
		_clientSession = clientSession;
	}
	
	@Override
	Directory<OmmProviderClient> directory()
	{
		return _directory;
	}
	
	@Override
	int type()
	{
		return ItemType.IPROVIDER_SINGLE_ITEM;
	}

	@Override
	boolean open(com.refinitiv.ema.access.ReqMsg reqMsg)
	{
		ReqMsgImpl reqMsgImpl = ((ReqMsgImpl)reqMsg);
		
		String serviceName = null;
		
		if ( reqMsgImpl.hasServiceName() )
		{
			ServiceIdInteger serviceId = ((OmmIProviderImpl)_baseImpl).directoryServiceStore().serviceId(reqMsgImpl.serviceName());
			
			if ( serviceId == null )
			{
				StringBuilder temp = _baseImpl.strBuilder();
	        	temp.append("Service name of '")
	        		.append(reqMsg.serviceName())
	        		.append("' is not found.");
	     
				_baseImpl._itemCallbackClient.addToItemMap(_baseImpl.nextLongId(), this);

	        	scheduleItemClosedStatus(_baseImpl.<OmmProviderClient>itemCallbackClient(),
											this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
											temp.toString(), reqMsg.serviceName());
	        	
	        	return true;
			}
			else
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
			serviceName = ((OmmIProviderImpl)_baseImpl).directoryServiceStore().serviceName(reqMsgImpl.serviceId());
			
			if ( serviceName == null )
			{
				StringBuilder temp = _baseImpl.strBuilder();
				
	        	temp.append("Service id of '")
	        		.append(reqMsg.serviceId())
	        		.append("' is not found.");
	        	
				_baseImpl._itemCallbackClient.addToItemMap(LongIdGenerator.nextLongId(), this);
	        	
	        	scheduleItemClosedStatus(_baseImpl.<OmmProviderClient>itemCallbackClient(),
						this, ((ReqMsgImpl)reqMsg).rsslMsg(), 
						temp.toString(), null);
	        	
	        	return true;
			}
			
			_serviceId = reqMsgImpl.serviceId();
			_specifiedServiceInReq = true;
		}
		
		_isPrivateStream = reqMsgImpl.privateStream();
		
		_directory = new Directory<OmmProviderClient>(serviceName);
		
		reqMsgImpl.rsslMsg().msgKey().copy(_rsslMsgKey);	
		_itemWatchList.addItem(this);
		
		return rsslSubmit(reqMsgImpl.rsslMsg());
	}
	
	private void scheduleItemClosedStatus(ItemCallbackClient<OmmProviderClient> client,
			IProviderSingleItem item, RequestMsg rsslMsg, String statusText, String serviceName) {
		
		if (_closedStatusClient != null) return;
    	
		_closedStatusClient = new ClosedStatusClient<OmmProviderClient>(client, item, rsslMsg, statusText, serviceName);
    	_baseImpl.addTimeoutEvent(100, _closedStatusClient);
	}

	@Override
	boolean modify(com.refinitiv.ema.access.ReqMsg reqMsg)
	{
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
							.error(_baseImpl.formatLogMessage(IProviderSingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				
				return false;
			}
		}
		else if ( reqMsgImpl.hasServiceId() )
		{
			if ( !_specifiedServiceInReq || ( reqMsgImpl.serviceId() != _serviceId ) )
			{
				StringBuilder temp = _baseImpl.strBuilder();
				temp.append("Service id of '")
        		.append(reqMsg.serviceId())
        		.append("' does not match existing request.").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");
				
				if (_baseImpl.loggerClient().isErrorEnabled())
					_baseImpl.loggerClient()
							.error(_baseImpl.formatLogMessage(IProviderSingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

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
							.error(_baseImpl.formatLogMessage(IProviderSingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

				_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				
				return false;
			}
		}
		else
		{
			reqMsgImpl.name(_rsslMsgKey.name().toString());
			reqMsgImpl.nameType(_rsslMsgKey.nameType());
		}
		
		return rsslSubmit(((ReqMsgImpl) reqMsg).rsslMsg());
	}
	
	@Override
	boolean close()
	{
		CloseMsg rsslCloseMsg = _baseImpl.itemCallbackClient().rsslCloseMsg();
		rsslCloseMsg.containerType(DataTypes.NO_DATA);
		rsslCloseMsg.domainType(_domainType);

		boolean retCode = rsslSubmit(rsslCloseMsg);

		remove();
		return retCode;
	}
	
	@Override
	void remove()
	{
		cancelReqTimerEvent();
		
		_baseImpl.<OmmProviderClient>itemCallbackClient().removeFromMap( this, true );
		
		_itemWatchList.removeItem(this);
	}

	boolean rsslSubmit(com.refinitiv.eta.codec.RequestMsg rsslRequestMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.serviceName(null);
		
		int domainType =  rsslRequestMsg.domainType();
		
		if (_streamId == 0)
		{
			rsslRequestMsg.streamId(getNextStreamId(0));
			_streamId = rsslRequestMsg.streamId();
			_baseImpl.<OmmProviderClient>itemCallbackClient().addToMap(_baseImpl.nextLongId(), this);
		}
		else
			rsslRequestMsg.streamId(_streamId);

		if (_domainType == 0)
			_domainType = domainType;
		else
			rsslRequestMsg.domainType(_domainType);
		
	    ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		
		if ( _clientSession == null )
		{
			_clientSession = _baseImpl.serverChannelHandler().clientSessionForDictReq();
		}
		
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = _clientSession.channel().submit(rsslRequestMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();
			if (_baseImpl.loggerClient().isErrorEnabled())
        	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();
				
	        	temp.append("Internal error: rsslChannel.submit() failed in IProviderSingleItem.submit(RequestMsg)")
	        		.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0)) 
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(IProviderSingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));
	        	
	        	temp.setLength(0);
        	}
			
			temp.append("Failed to open or modify item request. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());
				
			_baseImpl.handleInvalidUsage(temp.toString(), ret);
			return false;
	    }
		
		int requestTimeout = ((OmmIProviderImpl)_baseImpl).requestTimeout();
		
		if ( requestTimeout > 0 )
		{
			cancelReqTimerEvent();
			_reqTimeoutEvent = _baseImpl.addTimeoutEvent(requestTimeout * 1000, new ItemTimeOut( this ) );
		}
		
		return true;
	}
	
	public TimeoutEvent reqTimeoutEvent()
	{
		return _reqTimeoutEvent;
	}
	
	boolean rsslSubmit(com.refinitiv.eta.codec.CloseMsg rsslCloseMsg)
	{
		ReactorSubmitOptions rsslSubmitOptions = _baseImpl.rsslSubmitOptions();
		rsslSubmitOptions.clear();

		rsslCloseMsg.streamId(_streamId);

		ReactorErrorInfo rsslErrorInfo = _baseImpl.rsslErrorInfo();
		rsslErrorInfo.clear();
		ReactorChannel rsslChannel = _clientSession.channel();
		int ret;

        if (rsslChannel == null)
        {
            StringBuilder temp = _baseImpl.strBuilder();

            if (_baseImpl.loggerClient().isErrorEnabled())
            {
                temp.append("Internal error: ReactorChannel.submit() failed in IProviderSingleItem.submit(CloseMsg)").append(OmmLoggerClient.CR)
                        .append("\tReactorChannel is not available");

                _baseImpl.loggerClient().error(_baseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));

                temp.setLength(0);
            }

            temp.append("Failed to close item request. Reason: ReactorChannel is not available");

            _baseImpl.handleInvalidUsage(temp.toString(), ReactorReturnCodes.FAILURE);

            return false;
        }

        if (ReactorReturnCodes.SUCCESS > (ret = rsslChannel.submit(rsslCloseMsg, rsslSubmitOptions, rsslErrorInfo)))
	    {
			StringBuilder temp = _baseImpl.strBuilder();

			if (_baseImpl.loggerClient().isErrorEnabled())
	    	{
				com.refinitiv.eta.transport.Error error = rsslErrorInfo.error();

	        	temp.append("Internal error: ReactorChannel.submit() failed in IProviderSingleItem.submit(CloseMsg)")
	        	.append("RsslChannel ").append(Integer.toHexString(error.channel() != null ? error.channel().hashCode() : 0))
	    			.append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());

	        	_baseImpl.loggerClient().error(_baseImpl.formatLogMessage(IProviderSingleItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

	        	temp.setLength(0);
	    	}

			temp.append("Failed to close item request. Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(rsslErrorInfo.error().text());


			_baseImpl.handleInvalidUsage(temp.toString(), ret);

			return false;
	    }
	
		return true;
	}

	@Override
	int getNextStreamId(int numOfItem) {
		return -_baseImpl.itemCallbackClient().nextStreamId(numOfItem);
	}

	@Override
	public void scheduleItemClosedRecoverableStatus(String statusText, boolean initiateTimeout) {

		if (_closedStatusClient != null) return;
		
		cancelReqTimerEvent();
    	
		_closedStatusClient = new ClosedStatusClient<OmmProviderClient>(_baseImpl.<OmmProviderClient>itemCallbackClient(), this, _rsslMsgKey, _isPrivateStream, statusText, _directory.serviceName());
		
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
	public int serviceId() {
		return _serviceId;
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
}

class BatchItem<T> extends SingleItem<T>
{
	private static final String 	CLIENT_NAME = "BatchItem";
	
	private List<SingleItem<T>>		_singleItemList = new ArrayList<>();
	int	 _itemCount;
	boolean _statusFlag;
	
	BatchItem() {}
			
	BatchItem(OmmBaseImpl<T> baseImpl, T client, Object closure)
	{
		super(baseImpl, client, closure, null);
		
		_singleItemList = new ArrayList<>();
		_itemCount = 1;
	}
	
	@Override
	void reset(OmmBaseImpl<T> baseImpl, T client, Object closure, Item<T> item)
	{
		super.reset(baseImpl, client, closure, null);
		
		_singleItemList.clear();
		_itemCount = 1;
		_statusFlag = false;
	}

	@Override
	boolean open(com.refinitiv.ema.access.ReqMsg reqMsg)
	{
		return super.open(reqMsg);
	}
	
	@Override
	boolean modify(com.refinitiv.ema.access.ReqMsg reqMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to modify batch stream. ").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(BatchItem.CLIENT_NAME, temp.toString(), Severity.ERROR));


		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	boolean submit(com.refinitiv.ema.access.PostMsg postMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit PostMsg on batch stream. ").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(BatchItem.CLIENT_NAME, temp.toString(), Severity.ERROR));


		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	boolean submit(com.refinitiv.ema.access.GenericMsg genericMsg)
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to submit GenericMsg on batch stream. ").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(BatchItem.CLIENT_NAME, temp.toString(), Severity.ERROR));

		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}
	
	@Override
	boolean close()
	{
		StringBuilder temp = _baseImpl.strBuilder();
		temp.append("Invalid attempt to close batch stream. ").append("Instance name='")
				.append(_baseImpl.instanceName()).append("'.");

		if (_baseImpl.loggerClient().isErrorEnabled())
			_baseImpl.loggerClient()
					.error(_baseImpl.formatLogMessage(BatchItem.CLIENT_NAME, temp.toString(), Severity.ERROR));


		_baseImpl.handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return false;
	}

	@Override
	int type()
	{
		return ItemType.BATCH_ITEM;
	
	}
	
	@SuppressWarnings("unchecked")
	SingleItem<T> createSingleItem()
	{
		SingleItem<T> item;
		if ((item = (SingleItem<T>)_baseImpl._objManager._singleItemPool.poll()) == null)
		{
			item = new SingleItem<T>(_baseImpl, _client, 0, this);
			_baseImpl._objManager._singleItemPool.updatePool(item);
		}
		else
			item.reset(_baseImpl, _client, 0, this);
		
		return item;
	}
	
	@SuppressWarnings("unchecked")
	void addBatchItems(List<String> itemList)
	{
		SingleItem<T> item;
		for( int i = 0 ; i < itemList.size() ; i++ )
		{
			if ((item = (SingleItem<T>)_baseImpl._objManager._singleItemPool.poll()) == null)
			{
				item = new SingleItem<T>(_baseImpl, _client, _closure, this);
				_baseImpl._objManager._singleItemPool.updatePool(item);
			}
			else
				item.reset(_baseImpl, _client, _closure, this);
			
			item._itemName = itemList.get(i);
			
			_singleItemList.add( item );
		}
		
		_itemCount = itemList.size();
	}
	
	List<SingleItem<T>> singleItemList()
	{
		return _singleItemList;
	}

	SingleItem<T> singleItem(int streamId)
	{
		int index = streamId - _streamId;
		if (index < 0)
			return null;
	
		return (index == 0) ? this : _singleItemList.get(index-1);
	}

	void decreaseItemCount()
	{
		if ( --_itemCount == 0 && _statusFlag )
			_baseImpl.itemCallbackClient().removeFromMap(this, true);
	}

	@Override
	void remove() {
		if(_itemCount == 0)
			_baseImpl.itemCallbackClient().removeFromMap(this, true);
	}
}


class ClosedStatusClient<T> implements TimeoutClient
{
	private MsgKey 		_rsslMsgKey = CodecFactory.createMsgKey();
	private Buffer 		_statusText =  CodecFactory.createBuffer();
	private String 		_serviceName;
	private int 		_domainType;
	private int 		_streamId;
	private Item<T> 		_item;
	private boolean 	_isPrivateStream; 
	private CallbackClient<T> _client;
	private com.refinitiv.eta.codec.State		_rsslState = CodecFactory.createState();
	
	ClosedStatusClient(CallbackClient<T> client, Item<T> item, Msg rsslMsg, String statusText, String serviceName)
	{
		_client = client;
		_item = item;
		_statusText.data(statusText);
		_domainType = rsslMsg.domainType();
		_rsslMsgKey.clear();
		_serviceName = serviceName;
		
		if (rsslMsg.msgKey() != null)
			rsslMsg.msgKey().copy(_rsslMsgKey);
		
		_rsslState.streamState(StreamStates.CLOSED);
		_rsslState.dataState(DataStates.SUSPECT);
		_rsslState.code(StateCodes.NONE);
		
		switch (rsslMsg.msgClass())
	    {
	     	case MsgClasses.REFRESH :
	           	_isPrivateStream = (rsslMsg.flags() & RefreshMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	           	break;
	        case MsgClasses.STATUS :
	        	_isPrivateStream = (rsslMsg.flags() & StatusMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	        	break;
	        case MsgClasses.REQUEST :
	           	_isPrivateStream = (rsslMsg.flags() & RequestMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	        	break;
	        case MsgClasses.ACK :
	           	_isPrivateStream = (rsslMsg.flags() & AckMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	        	break;
	        default :
	           	_isPrivateStream = false;
	        	break;
	    }
	}
	
	ClosedStatusClient(CallbackClient<T> client, Item<T> item, TunnelStreamRequest tunnelStreamRequest, String text)
	{
		_client = client;
		_item = item;
		_statusText.data(text);
		_domainType = tunnelStreamRequest.domainType();
		
		_rsslMsgKey.clear();
		
		_rsslState.streamState(StreamStates.CLOSED);
		_rsslState.dataState(DataStates.SUSPECT);
		_rsslState.code(StateCodes.NONE);
		
		if(tunnelStreamRequest.hasName())
		{
			Buffer tunnelStreamNameBuf = CodecFactory.createBuffer();
			tunnelStreamNameBuf.data(tunnelStreamRequest.name());
			_rsslMsgKey.name(tunnelStreamNameBuf);
		}
		
		if(tunnelStreamRequest.hasServiceId() == true)
		{
			_rsslMsgKey.serviceId(tunnelStreamRequest.serviceId());
			_rsslMsgKey.applyHasServiceId();
		}
		
		if(tunnelStreamRequest.hasServiceName())
		{
			_serviceName = tunnelStreamRequest.serviceName();
		}
	}
	
	ClosedStatusClient(CallbackClient<T> client, Item<T> item, MsgKey msgKey, boolean privateStream, String statusText, String serviceName)
	{
		_client = client;
		_item = item;
		_statusText.data(statusText);
		_domainType = item._domainType;
		_rsslMsgKey.clear();
		_serviceName = serviceName;
		_rsslMsgKey = msgKey;
		
		_rsslState.streamState(StreamStates.CLOSED_RECOVER);
		_rsslState.dataState(DataStates.SUSPECT);
		_rsslState.code(StateCodes.NONE);
	}

	@Override
	public void handleTimeoutEvent()
	{
		StatusMsg rsslStatusMsg = _client.rsslStatusMsg();

		rsslStatusMsg.streamId(_streamId);
		rsslStatusMsg.domainType(_domainType);
		rsslStatusMsg.containerType(DataTypes.NO_DATA);
	
		rsslStatusMsg.applyHasState();
		rsslStatusMsg.state().streamState(_rsslState.streamState());
		if (_item.type() !=  Item.ItemType.BATCH_ITEM)
			rsslStatusMsg.state().dataState(_rsslState.dataState());
		else
			rsslStatusMsg.state().dataState(DataStates.OK);
		rsslStatusMsg.state().code(_rsslState.code());
		rsslStatusMsg.state().text(_statusText);

		rsslStatusMsg.applyHasMsgKey();
		_rsslMsgKey.copy(rsslStatusMsg.msgKey()); 
		
		if (_isPrivateStream)
			rsslStatusMsg.applyPrivateStream();

		if (_client._statusMsg == null)
			_client._statusMsg = new StatusMsgImpl(_client._baseImpl.objManager());
		
		_client._statusMsg.decode(rsslStatusMsg, majorVersion(), majorVersion(), null);

		if(_serviceName != null)
			_client._statusMsg.service(_serviceName);

		_client._eventImpl._item = _item;
		
		_client.notifyOnAllMsg(_client._statusMsg);
		_client.notifyOnStatusMsg();

		if(_item.type() != Item.ItemType.BATCH_ITEM)
			_client._eventImpl._item.remove();
		else
			((ItemCallbackClient<T>)_client).removeFromMap(_item, true);
	}

	@Override
	public ReentrantLock userLock()
	{
		return _client._baseImpl.userLock();
	}
}

class OpenSuspectClient<T> implements TimeoutClient
{
	private MsgKey 			_rsslMsgKey = CodecFactory.createMsgKey();
	private Buffer 			_statusText =  CodecFactory.createBuffer();
	private String 			_serviceName;
	private int 			_domainType;
	private int 			_streamId;
	private SingleItem<T> 	_singleItem;
	private boolean 	_isPrivateStream; 
	private CallbackClient<T> _client;
	private com.refinitiv.eta.codec.State		_rsslState = CodecFactory.createState();
	
	public OpenSuspectClient(CallbackClient<T> client, SingleItem<T> item, Msg rsslMsg, String statusText,
			String serviceName) 
	{
		_client = client;
		_singleItem = item;
		_statusText.data(statusText);
		_domainType = rsslMsg.domainType();
		_rsslMsgKey.clear();
		_serviceName = serviceName;
		
		if (rsslMsg.msgKey() != null)
			rsslMsg.msgKey().copy(_rsslMsgKey);
		
		_rsslState.streamState(StreamStates.OPEN);
		_rsslState.dataState(DataStates.SUSPECT);
		_rsslState.code(StateCodes.NONE);
		
		switch (rsslMsg.msgClass())
	    {
	     	case MsgClasses.REFRESH :
	           	_isPrivateStream = (rsslMsg.flags() & RefreshMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	           	break;
	        case MsgClasses.STATUS :
	        	_isPrivateStream = (rsslMsg.flags() & StatusMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	        	break;
	        case MsgClasses.REQUEST :
	           	_isPrivateStream = (rsslMsg.flags() & RequestMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	        	break;
	        case MsgClasses.ACK :
	           	_isPrivateStream = (rsslMsg.flags() & AckMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
	        	break;
	        default :
	           	_isPrivateStream = false;
	        	break;
	    }
	}

	@Override
	public void handleTimeoutEvent()
	{
		StatusMsg rsslStatusMsg = _client.rsslStatusMsg();

		rsslStatusMsg.streamId(_streamId);
		rsslStatusMsg.domainType(_domainType);
		rsslStatusMsg.containerType(DataTypes.NO_DATA);
	
		rsslStatusMsg.applyHasState();
		rsslStatusMsg.state().streamState(_rsslState.streamState());
		if (_singleItem.type() !=  Item.ItemType.BATCH_ITEM)
			rsslStatusMsg.state().dataState(_rsslState.dataState());
		else
			rsslStatusMsg.state().dataState(DataStates.OK);
		rsslStatusMsg.state().code(_rsslState.code());
		rsslStatusMsg.state().text(_statusText);
		    
		rsslStatusMsg.applyHasMsgKey();
		_rsslMsgKey.copy(rsslStatusMsg.msgKey()); 
		
		if (_isPrivateStream)
			rsslStatusMsg.applyPrivateStream();

		if (_client._statusMsg == null)
			_client._statusMsg = new StatusMsgImpl(_client._baseImpl.objManager());
		
		_client._statusMsg.decode(rsslStatusMsg, Codec.majorVersion(), Codec.majorVersion(), null);

		if(_serviceName != null)
			_client._statusMsg.service(_serviceName);

		_client._eventImpl._item = _singleItem;
		
		_client.notifyOnAllMsg(_client._statusMsg);
		_client.notifyOnStatusMsg();
	}

	@Override
	public ReentrantLock userLock()
	{
		return _client._baseImpl.userLock();
	}
}

class ItemWatchList
{
	private List<ProviderItem>		_itemList;
	private CallbackRsslMsgPool		_callBackRsslMsgPool;
	private final static int		INITIAL_ITEM_WATCHLIST_SIZE = 10;
	
	ItemWatchList(CallbackRsslMsgPool callBackRsslMsgPool)
	{
		_itemList = new ArrayList<>(INITIAL_ITEM_WATCHLIST_SIZE);
		_callBackRsslMsgPool = callBackRsslMsgPool;
	}
	
	CallbackRsslMsgPool callBackRsslMsgPool()
	{
		return _callBackRsslMsgPool;
	}
	
	void addItem(ProviderItem providerItem)
	{
		_itemList.add(providerItem);
	}
	
	void removeItem(ProviderItem providerItem)
	{
		_itemList.remove(providerItem);
	}
	
	void processIProviderChannelEvent(ReactorChannelEvent reactorChannelEvent)
	{
		switch ( reactorChannelEvent.eventType() )
		{
			case ReactorChannelEventTypes.CHANNEL_DOWN:
			case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
				notifyClosedRecoverableStatusMessage();
				break;
			default:
				break;
		}
	}

	// This is strictly for the NiProvider handling of dictionary requests.
	@SuppressWarnings("unchecked")
	void processNiProvChannelEvent(ReactorChannelEvent reactorChannelEvent)
	{
		switch ( reactorChannelEvent.eventType() )
		{
			case ReactorChannelEventTypes.CHANNEL_DOWN:
			case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
				for(int index = 0; index < _itemList.size(); index++ )
				{
					NiProviderSessionChannelInfo<OmmProviderClient> sessionChannel = ((ChannelInfo)reactorChannelEvent.reactorChannel().userSpecObj()).niProviderSessionChannelInfo();
					// For NiProviders, all items in _itemList should be NiProviderDictionaryItem objects.
					NiProviderDictionaryItem<OmmProviderClient> dictionaryItem =  (NiProviderDictionaryItem<OmmProviderClient>)_itemList.get(index);

					// Schedule the closed recoverable status for the item if the sessionChannel is null, or if the session channel is the same as the dictionary item's niProv session channel info
					if(sessionChannel == null || (sessionChannel == dictionaryItem.niProviderSessionChannelInfo()))
						dictionaryItem.scheduleItemClosedRecoverableStatus("channel down", true);
				}
				break;
			default:
				break;
		}
	}

	void processCloseLogin(ClientSession clientSession)
	{
		for(int index = 0; index < _itemList.size(); index++ )
		{
			 ProviderItem providerItem = _itemList.get(index);
			 
			if ( providerItem.clientSession() == clientSession )
			{
				providerItem.scheduleItemClosedRecoverableStatus("channel is closed", true);
			}
		}
	}
	
	void processServiceDelete(ClientSession clientSession, int serviceId)
	{
		for(int index = 0; index < _itemList.size(); index++ )
		{
			 ProviderItem providerItem = _itemList.get(index);
			 
			 if ( providerItem.requestWithService() )
			 {
				 if ( clientSession != null )
				 {
					 if ( providerItem.clientSession() == clientSession && providerItem.serviceId() == serviceId )
					{
						 providerItem.scheduleItemClosedRecoverableStatus("service is deleted", true);
					}
				 }
				 else if ( providerItem.serviceId() == serviceId )
				 {
					 providerItem.scheduleItemClosedRecoverableStatus("service is deleted", true);
				 }
			 }
		}
	}
	
	static Msg processRsslMsg(Msg eventMsg, ProviderItem providerItem)
	{	
		switch(eventMsg.msgClass())
		{
			case MsgClasses.REFRESH:
			{
				RefreshMsg refreshMsg = (RefreshMsg)eventMsg;
				
				if ( providerItem.processInitialResp(refreshMsg) == false )
				{
					providerItem.scheduleItemClosedRecoverableStatus("received response is mismatch with the initlal request", true);
					
					return null;
				}
					
				if ( (eventMsg.flags() & RefreshMsgFlags.HAS_MSG_KEY)  == 0)
				{	
	    			RefreshMsg modifyRefreshMsg = providerItem.itemWatchList().callBackRsslMsgPool().rsslRefreshMsg();
	   
	    			int ret = refreshMsg.copy( modifyRefreshMsg, CopyMsgFlags.ALL_FLAGS & ~( CopyMsgFlags.KEY | CopyMsgFlags.MSG_BUFFER ) );
	    			
	    			if ( ret == CodecReturnCodes.SUCCESS)
	    			{
	    				modifyRefreshMsg.applyHasMsgKey();
	    				MsgKey msgKey = modifyRefreshMsg.msgKey();
	    			
	    				ret = providerItem.rsslMsgKey().copy(msgKey);
	    				
	    				if ( ret != CodecReturnCodes.SUCCESS)
	    					return eventMsg;
	    			}
	    			else
	    			{
	    				return eventMsg;
	    			}
	    					
	    			return modifyRefreshMsg;
				}
	
				break;
			}	
			case MsgClasses.STATUS:
			{	
				if ( (eventMsg.flags() & StatusMsgFlags.HAS_MSG_KEY)  == 0)
				{
	    			StatusMsg statusMsg = (StatusMsg)eventMsg;
	    			
	    			StatusMsg modifyStatusMsg = providerItem.itemWatchList().callBackRsslMsgPool().rsslStatusMsg();
	    			
	    			int ret = statusMsg.copy(modifyStatusMsg, CopyMsgFlags.ALL_FLAGS & ~( CopyMsgFlags.KEY | CopyMsgFlags.MSG_BUFFER ) );
	    	
	    			if ( ret == CodecReturnCodes.SUCCESS)
	    			{
	    				modifyStatusMsg.applyHasMsgKey();
	    				MsgKey msgKey = modifyStatusMsg.msgKey();
	    		
	    				ret = providerItem.rsslMsgKey().copy(msgKey);
	    				
	    				if ( ret != CodecReturnCodes.SUCCESS)
	    					return eventMsg;
	    			}
	    			else
	    			{
	    				return eventMsg;
	    			}
	    			
	    			return modifyStatusMsg;
				}
				
				break;
			}
			default:
				return null;
		}
		
		return eventMsg;
	}
	
	private void notifyClosedRecoverableStatusMessage()
	{
		for(int index = 0; index < _itemList.size(); index++ )
		{
			_itemList.get(index).scheduleItemClosedRecoverableStatus("channel down", true);
		}
	}
}

class ItemTimeOut implements TimeoutClient
{
	private ProviderItem _providerItem;
	
	ItemTimeOut(ProviderItem providerItem)
	{
		_providerItem = providerItem;
	}
	
	ProviderItem providerItem()
	{
		return _providerItem;
	}
	
	@Override
	public void handleTimeoutEvent() {
		
		_providerItem.sendCloseMsg();
		_providerItem.scheduleItemClosedRecoverableStatus("request is timeout", false);
	}

	@Override
	public ReentrantLock userLock() {
		return _providerItem.userLock();
	}
}


