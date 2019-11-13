///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class RefreshMsgImpl extends MsgImpl implements RefreshMsg
{
	private  byte[] defaultGroupId = { 0, 0};
	private OmmStateImpl 	_state = new OmmStateImpl();
	private OmmQosImpl 		_qos;
	private boolean 		_stateSet;
	private boolean 		_qosSet;
	
	RefreshMsgImpl()
	{
		super(DataTypes.REFRESH_MSG, null);
		initialEncoding();
	}

	RefreshMsgImpl(EmaObjectManager objManager)
	{
		super(DataTypes.REFRESH_MSG, objManager);
	}
	
	RefreshMsgImpl(RefreshMsg other)
	{
		super(DataTypes.REFRESH_MSG, new EmaObjectManager());
		
		_objManager.initialize();
		
		MsgImpl.cloneBufferToMsg(this, (MsgImpl)other, "com.thomsonreuters.ema.access.RefreshMsgImpl.RefreshMsgImpl(RefreshMsg other)");
		
		// Set the decoded values from the clone buffer to the encoder
		if(!hasMsgKey() && other.hasMsgKey())
			cloneMsgKey((MsgImpl)other, _rsslMsg.msgKey(), _rsslMsg.flags(), "com.thomsonreuters.ema.access.RefreshMsgImpl.RefreshMsgImpl(UpdateMsg other)");

		if(hasMsgKey() || other.hasMsgKey())
		{
			if (hasName())
				name(name());

			if (hasNameType())
				nameType(nameType());

			if (hasServiceId())
				serviceId(serviceId());

			if (hasId())
				id(id());

			if (hasFilter())
				filter(filter());

			if(attrib().dataType() != DataTypes.NO_DATA)
				attrib(attrib().data());
		}
		
		domainType(domainType());

		if (hasExtendedHeader())
			extendedHeader(extendedHeader());
		
		if (hasQos())
			qos(qos().timeliness(), qos().rate());

		if (other.hasServiceName())
			serviceName(other.serviceName());

		if (other.hasSeqNum())
			seqNum(seqNum());

		if (other.hasPermissionData())
			permissionData(permissionData());
		
		if (hasPartNum())
			partNum(partNum());

		if (other.hasPublisherId())
			publisherId(publisherIdUserId(), publisherIdUserAddress());
		
		state(other.state().streamState(), other.state().dataState(), other.state().statusCode(), other.state().statusText());

		itemGroup(other.itemGroup());
		
		solicited(solicited());
		
		complete(complete());
		
		clearCache(clearCache());
		
		privateStream(privateStream());
		
		doNotCache(doNotCache());
				
		payload(other.payload().data());
		
		decodeCloneAttribPayload((MsgImpl)other);
	}
	
	@Override
	public RefreshMsg clear()
	{
		msgClear();
		initialEncoding();
		return this;
	}

	@Override
	public boolean hasQos()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasQos(); 
	}
	
	@Override
	public boolean hasSeqNum()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasSeqNum();
	}

	@Override
	public boolean hasPartNum()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasPartNum();
	}

	@Override
	public boolean hasPermissionData()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasPermData();
	}

	@Override
	public boolean hasPublisherId()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkHasPostUserInfo();
	}

	@Override
	public OmmState state()
	{
		if (_stateSet) 	return _state;
		
		_state.decode(((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state());
		_stateSet = true;
		
		return _state;
	}

	@Override
	public OmmQos qos()
	{
		if (!hasQos())
			throw ommIUExcept().message("Attempt to qos() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		if (_qosSet) return _qos;
		
		if (_qos == null)
			_qos = new OmmQosImpl();
		
		_qos.decode(((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).qos());
		_qosSet = true;
		
		return _qos;
	}

	@Override
	public long seqNum()
	{
		if (!hasSeqNum())
			throw ommIUExcept().message("Attempt to seqNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).seqNum();
	}

	@Override
	public int partNum()
	{
		if (!hasPartNum())
			throw ommIUExcept().message("Attempt to partNum() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).partNum();
	}

	@Override
	public ByteBuffer itemGroup()
	{
		_itemGroup = Utilities.copyFromPool( ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).groupId(), _itemGroup, _objManager);
		return _itemGroup;
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		_permissionData = Utilities.copyFromPool( ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).permData(), _permissionData, _objManager);
		return _permissionData;
	}

	@Override
	public long publisherIdUserId()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserId() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).postUserInfo().userId();
	}

	@Override
	public long publisherIdUserAddress()
	{
		if (!hasPublisherId())
			throw ommIUExcept().message("Attempt to publisherIdUserAddress() while it is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).postUserInfo().userAddr();
	}

	@Override
	public boolean solicited()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkSolicited();
	}

	@Override
	public boolean doNotCache()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkDoNotCache();
	}

	@Override
	public boolean complete()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkRefreshComplete();
	}

	@Override
	public boolean clearCache()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkClearCache();
	}

	@Override
	public boolean privateStream()
	{
		return ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).checkPrivateStream();
	}
	
	@Override
	public RefreshMsg streamId(int streamId)
	{
		msgStreamId(streamId);
		return this;
	}

	@Override
	public RefreshMsg domainType(int domainType)
	{
		msgDomainType(domainType);
		return this;
	}

	@Override
	public RefreshMsg name(String name)
	{
		msgName(name);
		return this;
	}

	@Override
	public RefreshMsg nameType(int nameType)
	{
		msgNameType(nameType);
		return this;
	}

	@Override
	public RefreshMsg serviceName(String serviceName)
	{
		msgServiceName(serviceName);
		return this;
	}

	@Override
	public RefreshMsg serviceId(int serviceId)
	{
		msgServiceId(serviceId);
		return this;
	}

	@Override
	public RefreshMsg id(int id)
	{
		msgId(id);
		return this;
	}

	@Override
	public RefreshMsg filter(long filter)
	{
		msgFilter(filter);
		return this;
	}

	@Override
	public RefreshMsg qos(int timeliness, int rate)
	{
		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasQos();
		Utilities.toRsslQos(rate, timeliness, ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).qos());
		
		return this;
	}

	@Override
	public RefreshMsg state(int streamState, int dataState, int statusCode, String statusText)
	{
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state().streamState(streamState) ||
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state().dataState(dataState) || 
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state().code(statusCode) || 
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state().text().data(statusText))
		{
			String errText = errorString().append("Attempt to specify invalid state. Passed in value is='" )
										.append( streamState ).append( " / " )
										.append( dataState ).append( " / " )
										.append( statusCode ).append( " / " )
										.append( statusCode ).append( "'." ).toString();
			throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		}
		
		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state().text().data(statusText);

		return this;
	}
	
	@Override
	public RefreshMsg state(int streamState, int dataState)
	{
		state(streamState, dataState, OmmState.StatusCode.NONE, DataImpl.EMPTY_STRING);
		
		return this;
	}

	@Override
	public RefreshMsg state(int streamState, int dataState, int statusCode)
	{
		state(streamState, dataState, statusCode, DataImpl.EMPTY_STRING);
		
		return this;
	}

	@Override
	public RefreshMsg seqNum(long seqNum)
	{
		msgSeqNum(seqNum);
		return this;
	}

	@Override
	public RefreshMsg partNum(int partNum)
	{
		msgPartNum(partNum);
		return this;
	}

	@Override
	public RefreshMsg itemGroup(ByteBuffer itemGroup)
	{
		if (itemGroup == null)
			throw ommIUExcept().message("Passed in itemGroup is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		Utilities.copy(itemGroup, ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).groupId());
		return this;
	}

	@Override
	public RefreshMsg permissionData(ByteBuffer permissionData)
	{
		msgPermissionData(permissionData);
		return this;
	}

	@Override
	public RefreshMsg publisherId(long userId, long userAddress)
	{
		msgPublisherId(userId, userAddress);
		return this;
	}

	@Override
	public RefreshMsg attrib(ComplexType data)
	{
		msgAttrib(data);
		return this;
	}

	@Override
	public RefreshMsg payload(ComplexType data)
	{
		msgPayload(data);
		return this;
	}

	@Override
	public RefreshMsg extendedHeader(ByteBuffer buffer)
	{
		msgExtendedHeader(buffer);
		return this;
	}
	
	@Override
	public RefreshMsg solicited(boolean solicited)
	{
		if (solicited)
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applySolicited();
	
		return this;
	}

	@Override
	public RefreshMsg doNotCache(boolean doNotCache)
	{
		if (doNotCache)
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyDoNotCache();
		
		return this;
	}

	@Override
	public RefreshMsg clearCache(boolean clearCache)
	{
		if (clearCache)
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyClearCache();
		
		return this;
	}

	@Override
	public RefreshMsg complete(boolean complete)
	{
		if (complete)
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyRefreshComplete();

		return this;
	}

	@Override
	public RefreshMsg privateStream(boolean privateStream)
	{
		if (privateStream)
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyPrivateStream();
		
		return this;
	}

	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		if ( _objManager == null )
			return "\nDecoding of just encoded object in the same application is not supported\n";
		
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent++).append("RefreshMsg");
		Utilities.addIndent(_toString, indent, true).append("streamId=\"")
													 .append(streamId())
													 .append("\"");
		Utilities.addIndent(_toString, indent, true).append("domain=\"")
													 .append(Utilities.rdmDomainAsString(domainType()))
													 .append("\"");			

		if (solicited())
			Utilities.addIndent(_toString, indent, true).append("solicited");

		if (complete())
			Utilities.addIndent(_toString, indent, true).append("RefreshComplete");

		if (privateStream())
			Utilities.addIndent(_toString, indent, true).append("privateStream");

		Utilities.addIndent(_toString, indent, true).append("state=\"")
													 .append(state().toString())
													 .append("\"");

		Utilities.addIndent(_toString, indent, true).append("itemGroup=\"");
		Utilities.asHexString(_toString, itemGroup()).append("\"");

		if (hasPermissionData())
		{
			Utilities.addIndent(_toString, indent, true).append("permissionData=\"");
			Utilities.asHexString(_toString, permissionData()).append("\"");
		}

		indent--;
		if (hasMsgKey())
		{
			indent++;
			
			if (hasName())
				Utilities.addIndent(_toString, indent, true).append("name=\"")
															 .append(name())
															 .append("\"");
															 

			if (hasNameType())
				Utilities.addIndent(_toString, indent, true).append("nameType=\"")
															 .append(nameType())
															 .append("\"");

			if (hasServiceId())
				Utilities.addIndent(_toString, indent, true).append("serviceId=\"")
															 .append(serviceId())
															 .append("\"");

			if (hasServiceName())
				Utilities.addIndent(_toString, indent, true).append("serviceName=\"")
															 .append(serviceName())
															 .append("\"");

			if (hasFilter())
				Utilities.addIndent(_toString, indent, true).append("filter=\"")
															 .append(filter())
															 .append("\"");

			if (hasId())
				Utilities.addIndent(_toString, indent, true).append("id=\"")
															 .append(id())
															 .append("\"");

			indent--;

			if (hasAttrib())
			{
				indent++;
				Utilities.addIndent(_toString, indent, true).append("Attrib dataType=\"")
															 .append(DataType.asString(attribData().dataType()))
															 .append("\"\n");

				indent++;
				_toString.append(attribData().toString(indent));
				indent--;

				Utilities.addIndent(_toString, indent, false).append("AttribEnd");
				indent--;
			}
		}
			
		if (hasExtendedHeader())
		{
			indent++;
			Utilities.addIndent(_toString, indent, true).append("ExtendedHeader\n");

			indent++;
			Utilities.addIndent(_toString, indent);
			Utilities.asHexString(_toString, extendedHeader()).append("\"");
			indent--;

			Utilities.addIndent(_toString, indent, true).append("ExtendedHeaderEnd");
			indent--;
		}

		if (hasPayload())
		{
			indent++;
			Utilities.addIndent(_toString, indent, true).append("Payload dataType=\"")
														 .append(DataType.asString(payloadData().dataType()))
														 .append("\"\n");

			indent++;
			_toString.append(payloadData().toString(indent));
			indent--;

			Utilities.addIndent(_toString, indent).append("PayloadEnd");
			indent--;
		}
				
		Utilities.addIndent(_toString, indent, true).append("RefreshMsgEnd\n");

		return _toString.toString();
	}
	
	com.thomsonreuters.upa.codec.RefreshMsg rsslMsg()
	{
		return ((_rsslEncodeIter != null) ? (com.thomsonreuters.upa.codec.RefreshMsg)(_rsslMsg) : null);
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Msg rsslMsg, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary)
	{
		_rsslMsg = rsslMsg;

		_rsslBuffer = _rsslMsg.encodedMsgBuffer();
		
		_rsslDictionary = rsslDictionary;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_serviceNameSet = false;

		_stateSet = false;

		_qosSet = false;

		decodeAttribPayload();
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object obj)
	{
		_rsslNestedMsg.clear();

		_rsslMsg = _rsslNestedMsg;
		
		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_serviceNameSet = false;

		_stateSet = false;

		_qosSet = false;

		_rsslDecodeIter.clear();

		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (CodecReturnCodes.SUCCESS != retCode)
		{
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}

		retCode = _rsslMsg.decode(_rsslDecodeIter);
		switch (retCode)
		{
			case CodecReturnCodes.SUCCESS:
				_errorCode = ErrorCode.NO_ERROR;
				decodeAttribPayload();
				return;
			case CodecReturnCodes.ITERATOR_OVERRUN:
				_errorCode = ErrorCode.ITERATOR_OVERRUN;
				dataInstance(_attribDecoded, DataTypes.ERROR).decode(_rsslBuffer, _errorCode);
				dataInstance(_payloadDecoded, DataTypes.ERROR).decode(_rsslBuffer, _errorCode);
				return;
			case CodecReturnCodes.INCOMPLETE_DATA:
				_errorCode = ErrorCode.INCOMPLETE_DATA;
				dataInstance(_attribDecoded, DataTypes.ERROR).decode(_rsslBuffer, _errorCode);
				dataInstance(_payloadDecoded, DataTypes.ERROR).decode(_rsslBuffer, _errorCode);
				return;
			default:
				_errorCode = ErrorCode.UNKNOWN_ERROR;
				dataInstance(_attribDecoded, DataTypes.ERROR).decode(_rsslBuffer, _errorCode);
				dataInstance(_payloadDecoded, DataTypes.ERROR).decode(_rsslBuffer, _errorCode);
				return;
		}
	}
	
	void initialEncoding()
	{
		 ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state().streamState(com.thomsonreuters.upa.codec.StreamStates.OPEN);
		 ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state().dataState(com.thomsonreuters.upa.codec.DataStates.OK);
		 ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).state().code(com.thomsonreuters.upa.codec.StateCodes.NONE);

		 ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).groupId().data( ByteBuffer.wrap(defaultGroupId));
	}
}